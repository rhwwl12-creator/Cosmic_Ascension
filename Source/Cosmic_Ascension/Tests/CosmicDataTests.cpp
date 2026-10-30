// Cosmic Ascension - automation tests of the data pipeline. Design: guideline.md section 7.3, 9.3, 10.1.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/DataTable.h"
#include "Events/CosmicEventSubsystem.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "HAL/FileManager.h"
#include "Internationalization/Regex.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/Class.h"
#include "UObject/EnumProperty.h"
#include "UObject/Package.h"
#include "UObject/TextProperty.h"
#include "UObject/UnrealType.h"

#include <initializer_list>
#include <limits>

namespace CosmicEventImpl
{
	// Defined in Events/CosmicEventSubsystem.cpp, in a named namespace (external linkage): the effects a per-tick source
	// applies. Declared here and not in a header: the only reader outside that file is the test below, which holds the
	// list of Tools/data/schema.json to the list of the code.
	bool IsPerTickEffect(EEffectType Type);
}

namespace CosmicDataTests
{
	// SHA-1 of a file without its carriage returns, so that the line ending conversion of a checkout does not matter.
	// Tools/make_data_tables.py hashes the same way. Empty when the file cannot be read.
	FString HashOfSource(const FString& Path)
	{
		TArray<uint8> Bytes;
		if (!FFileHelper::LoadFileToArray(Bytes, *Path))
		{
			return FString();
		}
		Bytes.RemoveAll([](uint8 Byte) { return Byte == 0x0D; });
		return FSHA1::HashBuffer(Bytes.GetData(), static_cast<uint64>(Bytes.Num())).ToString();
	}

	// ---- Data.Integrity: Tools/data/schema.json against the code and against loaded tables (section 7.3, 9.3) ----
	//
	// The schema is checked in full, rule by rule, by Tools/cosmic_data_schema.py on the sources when the assets are built.
	// This side reads what Python cannot see or cannot be trusted with: the reflected row structs and enums, the per-tick
	// list of the code, the words the evaluator resolves, and in the loaded assets the things that break silently at run
	// time (a row name that names nothing, a tag the ini does not declare, an array of the wrong length, a knob out of
	// range). The usage of conditions and effects, the text rules and the row rules stay with the sources:
	// Data.SourcesInSync ties the assets to them. What it cannot tie them to is the ini of today (the ini is not in the
	// stamp), so every tag of a loaded row is held to the ini here, the tags inside conditions and effects too.
	// Names are compared to the letter: FString compares without regard to case and a field of a JSON object is found
	// without regard to it, and a schema that differs from the code in case only would turn away rows written the way the
	// code spells them. (The words of the stats and of the civ selector are the exception: they are held by asking the
	// evaluator, which reads them as FNames, case ignored.)

	using FJsonObjectPtr = TSharedPtr<FJsonObject>;

	FJsonObjectPtr ParseJson(const FString& Text)
	{
		FJsonObjectPtr Object;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
		return FJsonSerializer::Deserialize(Reader, Object) ? Object : FJsonObjectPtr();
	}

	FJsonObjectPtr ObjectOf(const FJsonObject& Parent, const TCHAR* Field)
	{
		const FJsonObjectPtr* Found = nullptr;
		return Parent.TryGetObjectField(Field, Found) ? *Found : FJsonObjectPtr();
	}

	TArray<FString> StringsOf(const FJsonObject& Parent, const TCHAR* Field)
	{
		TArray<FString> Out;
		Parent.TryGetStringArrayField(Field, Out);
		return Out;
	}

	// A "root" of a tag column: one string or a list of them. Empty = any root.
	TArray<FString> RootsOf(const FJsonObject& Column)
	{
		TArray<FString> Roots = StringsOf(Column, TEXT("root"));
		FString One;
		if (Roots.IsEmpty() && Column.TryGetStringField(TEXT("root"), One))
		{
			Roots.Add(One);
		}
		return Roots;
	}

	TArray<TSharedPtr<FJsonValue>> JsonStrings(std::initializer_list<const TCHAR*> Strings)
	{
		TArray<TSharedPtr<FJsonValue>> Values;
		for (const TCHAR* String : Strings)
		{
			Values.Add(MakeShared<FJsonValueString>(FString(String)));
		}
		return Values;
	}

	TArray<FString> KeysOf(const FJsonObjectPtr& Object)
	{
		TArray<FString> Keys;
		if (Object.IsValid())
		{
			for (const auto& Pair : Object->Values)
			{
				Keys.Add(FString(*Pair.Key));
			}
		}
		return Keys;
	}

	TArray<FString> EnumeratorsOf(const UEnum* Enum)
	{
		TArray<FString> Names;
		// The last entry of a UENUM is the generated _MAX.
		for (int32 Index = 0; Enum && Index < Enum->NumEnums() - 1; ++Index)
		{
			Names.Add(Enum->GetNameStringByIndex(Index));
		}
		return Names;
	}

	const UScriptStruct* RowStructOf(const FString& StructName)
	{
		return FindObject<UScriptStruct>(nullptr, *(FString(TEXT("/Script/Cosmic_Ascension.")) + StructName));
	}

	bool SameText(const FString& A, const FString& B)
	{
		return A.Equals(B, ESearchCase::CaseSensitive);
	}

	bool HasText(const TArray<FString>& List, const FString& Text)
	{
		return List.ContainsByPredicate([&Text](const FString& Each) { return SameText(Each, Text); });
	}

	// Every TEXT("...") of a header, comments left out (a constant that is commented out is not called). The same
	// reading as load_code_rows of Tools/cosmic_data_schema.py.
	TArray<FString> RowNamesInCode(const FString& Header)
	{
		FString Code;
		int32 From = 0;
		FRegexMatcher Comments(FRegexPattern(TEXT("(?s)/\\*.*?\\*/|//[^\\n]*")), Header);
		while (Comments.FindNext())
		{
			Code += Header.Mid(From, Comments.GetMatchBeginning() - From);
			From = Comments.GetMatchEnding();
		}
		Code += Header.Mid(From);
		TArray<FString> Names;
		FRegexMatcher Matcher(FRegexPattern(TEXT("TEXT\\(\"([^\"]*)\"\\)")), Code);
		while (Matcher.FindNext())
		{
			Names.Add(Matcher.GetCaptureGroup(1));
		}
		return Names;
	}

	// A table the schema declares under exactly this name.
	bool DeclaresTable(const FJsonObjectPtr& Tables, const FString& TableName)
	{
		return Tables.IsValid() && HasText(KeysOf(Tables), TableName) && ObjectOf(*Tables, *TableName).IsValid();
	}

	// The kinds of column (the closed list of schema.json) a property may be declared as. A resource pool is a pool and a
	// list of other structs is structs (its columns are declared and checked one level down, CheckColumnsOf); a struct the
	// checker does not read is plain.
	TArray<FString> KindsFor(const FProperty* Property)
	{
		if (CastField<FNameProperty>(Property))
		{
			return { TEXT("row") };
		}
		if (CastField<FTextProperty>(Property))
		{
			return { TEXT("text") };
		}
		if (CastField<FEnumProperty>(Property))
		{
			return { TEXT("enum") };
		}
		if (const FByteProperty* ByteProperty = CastField<FByteProperty>(Property); ByteProperty && ByteProperty->Enum)
		{
			return { TEXT("enum") };
		}
		if (CastField<FBoolProperty>(Property))
		{
			return { TEXT("plain") };
		}
		if (CastField<FNumericProperty>(Property))
		{
			return { TEXT("number") };
		}
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			if (StructProperty->Struct == FGameplayTag::StaticStruct())
			{
				return { TEXT("tag") };
			}
			if (StructProperty->Struct == FGameplayTagContainer::StaticStruct())
			{
				return { TEXT("tags") };
			}
			if (StructProperty->Struct == FResourcePool::StaticStruct())
			{
				return { TEXT("pool") };
			}
			return { TEXT("plain") };
		}
		if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			const FProperty* Inner = ArrayProperty->Inner;
			if (CastField<FTextProperty>(Inner))
			{
				return { TEXT("texts") };
			}
			if (CastField<FNumericProperty>(Inner) && !CastField<FByteProperty>(Inner))
			{
				return { TEXT("numbers") };
			}
			if (const FStructProperty* InnerStruct = CastField<FStructProperty>(Inner))
			{
				if (InnerStruct->Struct == FGameplayTag::StaticStruct())
				{
					return { TEXT("tag_list") };
				}
				if (InnerStruct->Struct == FCosmicCondition::StaticStruct())
				{
					return { TEXT("conditions") };
				}
				if (InnerStruct->Struct == FCosmicEffect::StaticStruct())
				{
					return { TEXT("effects") };
				}
				return { TEXT("structs") };
			}
			if (CastField<FNameProperty>(Inner))
			{
				return { TEXT("rows") };
			}
			return { TEXT("plain") };
		}
		if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
		{
			const FStructProperty* KeyStruct = CastField<FStructProperty>(MapProperty->KeyProp);
			if (KeyStruct && KeyStruct->Struct == FGameplayTag::StaticStruct())
			{
				return { TEXT("tag_map") };
			}
			if (CastField<FNameProperty>(MapProperty->KeyProp))
			{
				return { TEXT("row_map"), TEXT("word_map") };
			}
		}
		return { TEXT("plain") };
	}

	// A column whose numbers are whole by its type: an integer property, an array of them, a map to them.
	bool HoldsWholeNumbers(const FProperty* Property)
	{
		const FProperty* Number = Property;
		if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			Number = ArrayProperty->Inner;
		}
		else if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
		{
			Number = MapProperty->ValueProp;
		}
		const FByteProperty* ByteProperty = CastField<FByteProperty>(Number);
		const FNumericProperty* Numeric = CastField<FNumericProperty>(Number);
		return Numeric && Numeric->IsInteger() && !(ByteProperty && ByteProperty->Enum);
	}

	void CompareNames(TArray<FString>& Problems, const FString& What, TArray<FString> InSchema, TArray<FString> InCode)
	{
		const auto Before = [](const FString& A, const FString& B) { return A.Compare(B, ESearchCase::CaseSensitive) < 0; };
		InSchema.Sort(Before);
		InCode.Sort(Before);
		bool bSame = InSchema.Num() == InCode.Num();
		for (int32 Index = 0; bSame && Index < InSchema.Num(); ++Index)
		{
			bSame = SameText(InSchema[Index], InCode[Index]);
		}
		if (!bSame)
		{
			Problems.Add(FString::Printf(TEXT("%s: the schema lists [%s], the code has [%s]"), *What, *FString::Join(InSchema, TEXT(", ")), *FString::Join(InCode, TEXT(", "))));
		}
	}

	// The declared columns of a struct against its properties, both ways and to the letter: a kind that fits the type, a
	// column of whole numbers that says "integer", an enum column that names its enum. A list of structs (kind structs) is
	// checked one level down against the struct of its entries, with the columns it declares.
	void CheckColumnsOf(TArray<FString>& Problems, const FString& Where, const FJsonObjectPtr& Columns, const UStruct* Struct)
	{
		const TArray<FString> Declared = KeysOf(Columns);
		TArray<FString> PropertyNames;
		for (TFieldIterator<FProperty> It(Struct, EFieldIteratorFlags::ExcludeSuper); It; ++It)
		{
			const FString ColumnName = It->GetName();
			PropertyNames.Add(ColumnName);
			const FJsonObjectPtr Column = HasText(Declared, ColumnName) ? ObjectOf(*Columns, *ColumnName) : FJsonObjectPtr();
			if (!Column.IsValid())
			{
				Problems.Add(FString::Printf(TEXT("%s.%s: the struct has this column and the schema does not declare it"), *Where, *ColumnName));
				continue;
			}
			FString Kind;
			Column->TryGetStringField(TEXT("kind"), Kind);
			const TArray<FString> Allowed = KindsFor(*It);
			if (!HasText(Allowed, Kind))
			{
				Problems.Add(FString::Printf(TEXT("%s.%s: kind '%s' does not fit the type of the column (%s)"), *Where, *ColumnName, *Kind, *FString::Join(Allowed, TEXT(" or "))));
			}
			// A column of whole numbers says so: the sources are held to whole numbers by that option alone.
			bool bInteger = false;
			Column->TryGetBoolField(TEXT("integer"), bInteger);
			if (HoldsWholeNumbers(*It) && !bInteger)
			{
				Problems.Add(FString::Printf(TEXT("%s.%s: the column holds whole numbers and the schema does not say \"integer\": true"), *Where, *ColumnName));
			}
			// An enum column names the enum of its property: its cells and the tests of the rules are held to that list.
			const UEnum* Enum = nullptr;
			if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(*It))
			{
				Enum = EnumProperty->GetEnum();
			}
			else if (const FByteProperty* ByteProperty = CastField<FByteProperty>(*It))
			{
				Enum = ByteProperty->Enum;
			}
			FString EnumName;
			Column->TryGetStringField(TEXT("enum"), EnumName);
			if (Enum && !SameText(EnumName, Enum->GetName()))
			{
				Problems.Add(FString::Printf(TEXT("%s.%s: the column is of the enum %s and the schema says '%s'"), *Where, *ColumnName, *Enum->GetName(), *EnumName));
			}
			// The columns of a list of structs are the members of its entries.
			const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(*It);
			const FStructProperty* Entry = ArrayProperty ? CastField<FStructProperty>(ArrayProperty->Inner) : nullptr;
			if (Kind == TEXT("structs") && Entry && HasText(Allowed, Kind))
			{
				const FJsonObjectPtr EntryColumns = ObjectOf(*Column, TEXT("columns"));
				if (EntryColumns.IsValid())
				{
					CheckColumnsOf(Problems, FString::Printf(TEXT("%s.%s"), *Where, *ColumnName), EntryColumns, Entry->Struct);
				}
				else
				{
					Problems.Add(FString::Printf(TEXT("%s.%s: a list of structs declares the columns of its entries"), *Where, *ColumnName));
				}
			}
		}
		for (const FString& ColumnName : Declared)
		{
			if (!HasText(PropertyNames, ColumnName))
			{
				Problems.Add(FString::Printf(TEXT("%s.%s: the schema declares a column the struct does not have"), *Where, *ColumnName));
			}
		}
	}

	// The schema against the reflected code: every column of every declared row struct with a kind that fits its type,
	// the enumerators of the two vocabularies and of the enums it names, the word lists the code has a list for, the
	// tags the code asks for by name, and the per-tick list. No table is read.
	TArray<FString> CheckSchemaAgainstCode(const FJsonObject& Schema)
	{
		TArray<FString> Problems;
		const FJsonObjectPtr Tables = ObjectOf(Schema, TEXT("tables"));
		if (!Tables.IsValid())
		{
			Problems.Add(TEXT("schema: no tables"));
			return Problems;
		}
		for (const auto& TablePair : Tables->Values)
		{
			const FString TableName(*TablePair.Key);
			const FJsonObjectPtr Spec = TablePair.Value->AsObject();
			FString StructName;
			Spec->TryGetStringField(TEXT("struct"), StructName);
			const UScriptStruct* Struct = RowStructOf(StructName);
			if (!Struct)
			{
				Problems.Add(FString::Printf(TEXT("schema.%s: '%s' is no row struct of the module"), *TableName, *StructName));
				continue;
			}
			const FJsonObjectPtr Columns = ObjectOf(*Spec, TEXT("columns"));
			if (!Columns.IsValid())
			{
				// Declared with the first row of the table.
				continue;
			}
			CheckColumnsOf(Problems, FString::Printf(TEXT("schema.%s"), *TableName), Columns, Struct);
		}

		CompareNames(Problems, TEXT("schema.conditions"), KeysOf(ObjectOf(Schema, TEXT("conditions"))), EnumeratorsOf(StaticEnum<EConditionType>()));
		CompareNames(Problems, TEXT("schema.effects"), KeysOf(ObjectOf(Schema, TEXT("effects"))), EnumeratorsOf(StaticEnum<EEffectType>()));
		const FJsonObjectPtr Enums = ObjectOf(Schema, TEXT("enums"));
		for (const FString& EnumName : KeysOf(Enums))
		{
			const UEnum* Enum = FindObject<UEnum>(nullptr, *(FString(TEXT("/Script/Cosmic_Ascension.")) + EnumName));
			if (!Enum)
			{
				Problems.Add(FString::Printf(TEXT("schema.enums.%s: no such enum in the module"), *EnumName));
				continue;
			}
			CompareNames(Problems, FString::Printf(TEXT("schema.enums.%s"), *EnumName), StringsOf(*Enums, *EnumName), EnumeratorsOf(Enum));
		}

		// The word lists that have a list in the code are that list: the resources, the roles, the ethics axes and the
		// policies of the lure knob. (The words of the stats and of the civ selector have none; the test asks the
		// evaluator for those.)
		const FJsonObjectPtr Words = ObjectOf(Schema, TEXT("words"));
		if (Words.IsValid())
		{
			CompareNames(Problems, TEXT("schema.words.resource"), StringsOf(*Words, TEXT("resource")), EnumeratorsOf(StaticEnum<ECosmicResource>()));
			// The sources hold a resource pool (kind pool) to the words of that list, member by member.
			TArray<FString> PoolMembers;
			for (TFieldIterator<FProperty> It(FResourcePool::StaticStruct()); It; ++It)
			{
				PoolMembers.Add(It->GetName());
			}
			CompareNames(Problems, TEXT("schema.words.resource (the members of a resource pool)"), StringsOf(*Words, TEXT("resource")), PoolMembers);
			CompareNames(Problems, TEXT("schema.words.role"), StringsOf(*Words, TEXT("role")), EnumeratorsOf(StaticEnum<EPlayerRole>()));
			TArray<FString> Axes;
			for (TFieldIterator<FProperty> It(FEthicsAxes::StaticStruct()); It; ++It)
			{
				Axes.Add(It->GetName());
			}
			CompareNames(Problems, TEXT("schema.words.axis"), StringsOf(*Words, TEXT("axis")), Axes);
			TArray<FString> Policies;
			for (const TPair<FName, float>& Policy : FCosmicRulesDef().LureAcceptByPolicy)
			{
				Policies.Add(Policy.Key.ToString());
			}
			CompareNames(Problems, TEXT("schema.words.policy"), StringsOf(*Words, TEXT("policy")), Policies);
		}
		else
		{
			Problems.Add(TEXT("schema: no words"));
		}

		// The tags the schema requires are declared in the ini, not only implied by a child. The list holds the role tags of
		// a notable (section 9.3), and every tag the ini declares under Notable is on it: a seventh role needs its line.
		const TArray<FString> RequiredTags = StringsOf(Schema, TEXT("required_tags"));
		for (const FString& TagName : RequiredTags)
		{
			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*TagName), false);
			const TSharedPtr<FGameplayTagNode> Node = Tag.IsValid() ? UGameplayTagsManager::Get().FindTagNode(Tag) : TSharedPtr<FGameplayTagNode>();
			if (!Node.IsValid() || !Node->IsExplicitTag())
			{
				Problems.Add(FString::Printf(TEXT("schema.required_tags: tag %s is not declared in the ini"), *TagName));
			}
		}
		TArray<FString> RoleTags;
		const FGameplayTag Notable = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable")), false);
		if (Notable.IsValid())
		{
			// Held in a local: a range-for over the array of a temporary container would read it after it is gone.
			const FGameplayTagContainer Roles = UGameplayTagsManager::Get().RequestGameplayTagChildren(Notable);
			for (const FGameplayTag& Role : Roles.GetGameplayTagArray())
			{
				RoleTags.Add(Role.GetTagName().ToString());
			}
		}
		CompareNames(Problems, TEXT("schema.required_tags (the role tags under Notable)"),
			RequiredTags.FilterByPredicate([](const FString& TagName) { return TagName.StartsWith(TEXT("Notable."), ESearchCase::CaseSensitive); }), RoleTags);

		// The rows the code calls by name (the headers the schema lists) against required_rows, both ways, for the tables
		// the schema has. Tools/cosmic_data_schema.py does the same when the assets are built; an edit of the header alone
		// changes no source and builds no asset, so it is held here too.
		const TArray<FString> RowSources = StringsOf(Schema, TEXT("row_name_sources"));
		TArray<FString> CodeRows;
		bool bHeadersRead = RowSources.Num() > 0;
		if (RowSources.IsEmpty())
		{
			Problems.Add(TEXT("schema.row_name_sources: no header is listed, so required_rows cannot be held to the code"));
		}
		for (const FString& Source : RowSources)
		{
			FString Header;
			if (FFileHelper::LoadFileToString(Header, *(FPaths::ProjectDir() / Source)))
			{
				CodeRows.Append(RowNamesInCode(Header));
			}
			else
			{
				Problems.Add(FString::Printf(TEXT("schema.row_name_sources: %s cannot be read"), *Source));
				bHeadersRead = false;
			}
		}
		for (const auto& TablePair : Tables->Values)
		{
			if (!bHeadersRead)
			{
				break;
			}
			const FString TableName(*TablePair.Key);
			const FJsonObjectPtr Spec = TablePair.Value->AsObject();
			const TArray<FString> Names = StringsOf(*Spec, TEXT("names"));
			const TArray<FString> Prefixes = StringsOf(*Spec, TEXT("prefixes"));
			const TArray<FString> Required = StringsOf(*Spec, TEXT("required_rows"));
			for (const FString& Row : CodeRows)
			{
				const bool bOfThisTable = HasText(Names, Row) || Prefixes.ContainsByPredicate([&Row](const FString& Prefix) { return Row.StartsWith(Prefix, ESearchCase::CaseSensitive); });
				if (bOfThisTable && !HasText(Required, Row))
				{
					Problems.Add(FString::Printf(TEXT("schema.%s: the code calls the row %s by name and required_rows does not list it"), *TableName, *Row));
				}
			}
			for (const FString& Row : Required)
			{
				if (!HasText(CodeRows, Row))
				{
					Problems.Add(FString::Printf(TEXT("schema.%s: required_rows lists %s, which the code does not call by name"), *TableName, *Row));
				}
			}
		}

		// The per-tick list is the list of the code, effect by effect.
		const TArray<FString> PerTick = StringsOf(Schema, TEXT("per_tick_effects"));
		const UEnum* Effects = StaticEnum<EEffectType>();
		for (int32 Index = 0; Index < Effects->NumEnums() - 1; ++Index)
		{
			const FString Name = Effects->GetNameStringByIndex(Index);
			const bool bInCode = CosmicEventImpl::IsPerTickEffect(static_cast<EEffectType>(Effects->GetValueByIndex(Index)));
			const bool bListed = HasText(PerTick, Name);
			if (bListed != bInCode)
			{
				Problems.Add(FString::Printf(TEXT("schema.per_tick_effects: %s is %s in the schema and %s in the code"), *Name,
					bListed ? TEXT("listed") : TEXT("not listed"), bInCode ? TEXT("applied by a per-tick source") : TEXT("refused by a per-tick source")));
			}
		}
		return Problems;
	}

	bool NumberOf(const FProperty* Property, const void* Value, double& OutNumber)
	{
		const FNumericProperty* Numeric = CastField<FNumericProperty>(Property);
		if (!Numeric)
		{
			return false;
		}
		OutNumber = Numeric->IsFloatingPoint() ? Numeric->GetFloatingPointPropertyValue(Value) : static_cast<double>(Numeric->GetSignedIntPropertyValue(Value));
		return true;
	}

	// One number against the min and max of its column. A value that is not finite is a problem by itself. ("integer" is
	// left to the sources: a column of whole numbers is an integer property here.) A float column is held to the bound
	// as a float holds it: the sources compare the number as written (0.85 against 0.85), and the same value read back
	// from a float (0.85f is a little above 0.85) must not fall outside.
	void CheckNumber(TArray<FString>& Problems, const FString& Where, const FProperty* Property, double Number, const FJsonObject& Column)
	{
		const bool bSingle = CastField<FFloatProperty>(Property) != nullptr;
		const auto AsHeld = [bSingle](double Bound) { return bSingle ? static_cast<double>(static_cast<float>(Bound)) : Bound; };
		if (!(Number >= std::numeric_limits<double>::lowest() && Number <= std::numeric_limits<double>::max()))
		{
			Problems.Add(FString::Printf(TEXT("%s: not a finite number"), *Where));
			return;
		}
		double Bound = 0.0;
		if (Column.TryGetNumberField(TEXT("min"), Bound) && Number < AsHeld(Bound))
		{
			Problems.Add(FString::Printf(TEXT("%s: %g is below %g"), *Where, Number, Bound));
		}
		if (Column.TryGetNumberField(TEXT("max"), Bound) && Number > AsHeld(Bound))
		{
			Problems.Add(FString::Printf(TEXT("%s: %g is above %g"), *Where, Number, Bound));
		}
	}

	void CheckTag(TArray<FString>& Problems, const FString& Where, const FGameplayTag& Tag, const FJsonObject& Column, bool bMayBeEmpty)
	{
		if (!Tag.IsValid())
		{
			if (!bMayBeEmpty)
			{
				Problems.Add(FString::Printf(TEXT("%s: a tag is required here"), *Where));
			}
			return;
		}
		const FString Name = Tag.GetTagName().ToString();
		// A parent that only its children imply is not a tag the ini declares. (Outside the editor every node counts as
		// declared, so this half sees nothing there.)
		const TSharedPtr<FGameplayTagNode> Node = UGameplayTagsManager::Get().FindTagNode(Tag);
		if (!Node.IsValid() || !Node->IsExplicitTag())
		{
			Problems.Add(FString::Printf(TEXT("%s: tag %s is not declared in the ini"), *Where, *Name));
		}
		const TArray<FString> Roots = RootsOf(Column);
		if (!Roots.IsEmpty() && !Roots.ContainsByPredicate([&Name](const FString& Root) { return Name.StartsWith(Root, ESearchCase::CaseSensitive); }))
		{
			Problems.Add(FString::Printf(TEXT("%s: tag %s is not under %s"), *Where, *Name, *FString::Join(Roots, TEXT(" or "))));
		}
	}

	using FTableMap = TMap<FString, const UDataTable*>;

	void CheckRowName(TArray<FString>& Problems, const FString& Where, FName RowName, const FTableMap& Tables, const FString& TargetTable, bool bMayBeEmpty)
	{
		if (RowName.IsNone())
		{
			if (!bMayBeEmpty)
			{
				Problems.Add(FString::Printf(TEXT("%s: a row of %s is required here"), *Where, *TargetTable));
			}
			return;
		}
		const UDataTable* const* Target = Tables.Find(TargetTable);
		if (!Target || !*Target || !(*Target)->GetRowMap().Contains(RowName))
		{
			Problems.Add(FString::Printf(TEXT("%s: %s has no row %s"), *Where, *TargetTable, *RowName.ToString()));
		}
	}

	// One cell of a loaded row against its column. Kinds this side does not read are skipped (see the note above).
	void CheckCell(TArray<FString>& Problems, const FString& Where, const FProperty* Property, const void* Value, const FJsonObject& Schema,
		const FJsonObject& Column, const FTableMap& Tables, FName RowName)
	{
		FString Kind;
		Column.TryGetStringField(TEXT("kind"), Kind);
		if (!HasText(KindsFor(Property), Kind))
		{
			// CheckSchemaAgainstCode reports the kind. Reading the cell as that kind would read the wrong memory.
			return;
		}
		bool bOptional = false;
		Column.TryGetBoolField(TEXT("optional"), bOptional);
		FString TargetTable;
		Column.TryGetStringField(TEXT("table"), TargetTable);

		if (Kind == TEXT("row"))
		{
			if (const FNameProperty* NameProperty = CastField<FNameProperty>(Property))
			{
				CheckRowName(Problems, Where, NameProperty->GetPropertyValue(Value), Tables, TargetTable, bOptional);
			}
		}
		else if (Kind == TEXT("tag"))
		{
			CheckTag(Problems, Where, *static_cast<const FGameplayTag*>(Value), Column, bOptional);
		}
		else if (Kind == TEXT("tags"))
		{
			for (const FGameplayTag& Tag : static_cast<const FGameplayTagContainer*>(Value)->GetGameplayTagArray())
			{
				CheckTag(Problems, Where, Tag, Column, false);
			}
		}
		else if (Kind == TEXT("number"))
		{
			double Number = 0.0;
			if (NumberOf(Property, Value, Number))
			{
				CheckNumber(Problems, Where, Property, Number, Column);
			}
		}
		else if (Kind == TEXT("pool"))
		{
			// Every resource of the pool against the range of its column.
			for (TFieldIterator<FProperty> It(CastFieldChecked<FStructProperty>(Property)->Struct); It; ++It)
			{
				double Number = 0.0;
				if (NumberOf(*It, It->ContainerPtrToValuePtr<void>(Value), Number))
				{
					CheckNumber(Problems, FString::Printf(TEXT("%s.%s"), *Where, *It->GetName()), *It, Number, Column);
				}
			}
		}
		else if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			FScriptArrayHelper Helper(ArrayProperty, Value);
			const int32 Count = Helper.Num();
			if (Kind == TEXT("numbers"))
			{
				int32 Length = 0;
				if (Column.TryGetNumberField(TEXT("len"), Length) && Count != Length)
				{
					Problems.Add(FString::Printf(TEXT("%s: expected exactly %d entries, found %d"), *Where, Length, Count));
				}
				bool bAscending = false;
				Column.TryGetBoolField(TEXT("ascending"), bAscending);
				double Previous = 0.0;
				for (int32 Index = 0; Index < Count; ++Index)
				{
					double Number = 0.0;
					NumberOf(ArrayProperty->Inner, Helper.GetRawPtr(Index), Number);
					CheckNumber(Problems, FString::Printf(TEXT("%s[%d]"), *Where, Index), ArrayProperty->Inner, Number, Column);
					if (bAscending && Index > 0 && !(Number > Previous))
					{
						Problems.Add(FString::Printf(TEXT("%s: the entries do not rise"), *Where));
					}
					Previous = Number;
				}
				// A cumulative share ends where every draw is covered. A float array holds the mark as a float does.
				double EndsAt = 0.0;
				if (Column.TryGetNumberField(TEXT("ends_at"), EndsAt))
				{
					const double Held = CastField<FFloatProperty>(ArrayProperty->Inner) ? static_cast<double>(static_cast<float>(EndsAt)) : EndsAt;
					if (Count == 0 || Previous != Held)
					{
						Problems.Add(FString::Printf(TEXT("%s: the last entry has to be %g"), *Where, EndsAt));
					}
				}
			}
			else if (Kind == TEXT("texts"))
			{
				// The counts only: an array emptied in the editor reads as a default at run time. What the texts say is
				// checked on the sources.
				int32 Least = 0;
				bool bHasLeast = Column.TryGetNumberField(TEXT("min"), Least);
				bool bSingleVariant = false;
				Column.TryGetBoolField(TEXT("single_variant"), bSingleVariant);
				if (bSingleVariant && HasText(StringsOf(Schema, TEXT("single_variant_rows")), RowName.ToString()))
				{
					// One variant is asked for whether or not the column states a least count of its own.
					Least = 1;
					bHasLeast = true;
				}
				if (bHasLeast && Count < Least)
				{
					Problems.Add(FString::Printf(TEXT("%s: expected %d entries or more, found %d"), *Where, Least, Count));
				}
				const TArray<TSharedPtr<FJsonValue>>* Lengths = nullptr;
				if (Column.TryGetArrayField(TEXT("len_in"), Lengths) && !Lengths->ContainsByPredicate([Count](const TSharedPtr<FJsonValue>& Each) { return static_cast<int32>(Each->AsNumber()) == Count; }))
				{
					Problems.Add(FString::Printf(TEXT("%s: %d entries is none of the allowed counts"), *Where, Count));
				}
				int32 EmptyOrLeast = 0;
				if (Column.TryGetNumberField(TEXT("empty_or_min"), EmptyOrLeast) && Count > 0 && Count < EmptyOrLeast)
				{
					Problems.Add(FString::Printf(TEXT("%s: expected none or %d entries and more, found %d"), *Where, EmptyOrLeast, Count));
				}
			}
			else if (Kind == TEXT("tag_list"))
			{
				for (int32 Index = 0; Index < Count; ++Index)
				{
					CheckTag(Problems, FString::Printf(TEXT("%s[%d]"), *Where, Index), *reinterpret_cast<const FGameplayTag*>(Helper.GetRawPtr(Index)), Column, false);
				}
			}
			else if (Kind == TEXT("rows"))
			{
				// Every entry a row of one of the named tables, as the loaded tables hold them.
				const TArray<FString> Targets = StringsOf(Column, TEXT("tables"));
				const FNameProperty* Inner = CastFieldChecked<FNameProperty>(ArrayProperty->Inner);
				for (int32 Index = 0; Index < Count; ++Index)
				{
					const FName Entry = Inner->GetPropertyValue(Helper.GetRawPtr(Index));
					const bool bFound = !Entry.IsNone() && Targets.ContainsByPredicate([&Tables, Entry](const FString& Target)
					{
						const UDataTable* const* Table = Tables.Find(Target);
						return Table && *Table && (*Table)->GetRowMap().Contains(Entry);
					});
					if (!bFound)
					{
						Problems.Add(FString::Printf(TEXT("%s[%d]: none of %s has a row %s"), *Where, Index, *FString::Join(Targets, TEXT(", ")), *Entry.ToString()));
					}
				}
			}
			else if (Kind == TEXT("conditions") || Kind == TEXT("effects"))
			{
				// How a condition or an effect uses its Tag, Key and Value is checked on the sources. What the sources cannot
				// vouch for is the ini of today: a tag that left it keeps its name in the asset and never matches again.
				const FJsonObject AnyTag;
				for (int32 Index = 0; Index < Count; ++Index)
				{
					const FGameplayTag& Tag = Kind == TEXT("conditions")
						? reinterpret_cast<const FCosmicCondition*>(Helper.GetRawPtr(Index))->Tag
						: reinterpret_cast<const FCosmicEffect*>(Helper.GetRawPtr(Index))->Tag;
					CheckTag(Problems, FString::Printf(TEXT("%s[%d].Tag"), *Where, Index), Tag, AnyTag, true);
				}
			}
			else if (Kind == TEXT("structs"))
			{
				// The counts, then every entry member by member with the columns of the list, as a row is checked.
				int32 Least = 0;
				if (Column.TryGetNumberField(TEXT("min"), Least) && Count < Least)
				{
					Problems.Add(FString::Printf(TEXT("%s: expected %d entries or more, found %d"), *Where, Least, Count));
				}
				const TArray<TSharedPtr<FJsonValue>>* Lengths = nullptr;
				if (Column.TryGetArrayField(TEXT("len_in"), Lengths) && !Lengths->ContainsByPredicate([Count](const TSharedPtr<FJsonValue>& Each) { return static_cast<int32>(Each->AsNumber()) == Count; }))
				{
					Problems.Add(FString::Printf(TEXT("%s: %d entries is none of the allowed counts"), *Where, Count));
				}
				const FStructProperty* Entry = CastFieldChecked<FStructProperty>(ArrayProperty->Inner);
				const FJsonObjectPtr EntryColumns = ObjectOf(Column, TEXT("columns"));
				for (int32 Index = 0; EntryColumns.IsValid() && Index < Count; ++Index)
				{
					for (TFieldIterator<FProperty> It(Entry->Struct, EFieldIteratorFlags::ExcludeSuper); It; ++It)
					{
						if (const FJsonObjectPtr Member = ObjectOf(*EntryColumns, *It->GetName()))
						{
							CheckCell(Problems, FString::Printf(TEXT("%s[%d].%s"), *Where, Index, *It->GetName()), *It, It->ContainerPtrToValuePtr<void>(Helper.GetRawPtr(Index)),
								Schema, *Member, Tables, RowName);
						}
					}
				}
			}
		}
		else if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
		{
			if (Kind != TEXT("tag_map") && Kind != TEXT("row_map") && Kind != TEXT("word_map"))
			{
				return;
			}
			const FJsonObjectPtr Words = ObjectOf(Schema, TEXT("words"));
			FString WordList;
			Column.TryGetStringField(TEXT("words"), WordList);
			FScriptMapHelper Helper(MapProperty, Value);
			for (int32 Index = 0; Index < Helper.GetMaxIndex(); ++Index)
			{
				if (!Helper.IsValidIndex(Index))
				{
					continue;
				}
				FString Here = Where;
				if (Kind == TEXT("tag_map"))
				{
					const FGameplayTag& Key = *reinterpret_cast<const FGameplayTag*>(Helper.GetKeyPtr(Index));
					Here = FString::Printf(TEXT("%s[%s]"), *Where, *Key.GetTagName().ToString());
					CheckTag(Problems, Here, Key, Column, false);
				}
				else
				{
					const FName Key = CastFieldChecked<FNameProperty>(MapProperty->KeyProp)->GetPropertyValue(Helper.GetKeyPtr(Index));
					Here = FString::Printf(TEXT("%s[%s]"), *Where, *Key.ToString());
					if (Kind == TEXT("row_map"))
					{
						CheckRowName(Problems, Here, Key, Tables, TargetTable, false);
					}
					else if (!Words.IsValid() || !HasText(StringsOf(*Words, *WordList), Key.ToString()))
					{
						Problems.Add(FString::Printf(TEXT("%s: %s is none of the words of %s"), *Here, *Key.ToString(), *WordList));
					}
				}
				double Number = 0.0;
				if (NumberOf(MapProperty->ValueProp, Helper.GetValuePtr(Index), Number))
				{
					CheckNumber(Problems, Here, MapProperty->ValueProp, Number, Column);
				}
			}
		}
	}

	// Loaded tables against the schema: row names (pattern, prefix, the rows the code calls by name, one name in the whole
	// game), then every cell of the kinds this side reads.
	TArray<FString> CheckTables(const FJsonObject& Schema, const FTableMap& Tables)
	{
		TArray<FString> Problems;
		const FJsonObjectPtr Specs = ObjectOf(Schema, TEXT("tables"));
		if (!Specs.IsValid())
		{
			Problems.Add(TEXT("schema: no tables"));
			return Problems;
		}
		FString PatternText(TEXT(".*"));
		Schema.TryGetStringField(TEXT("row_name_pattern"), PatternText);
		const FRegexPattern Pattern(PatternText);
		TMap<FName, FString> OwnerOf;

		for (const auto& TablePair : Specs->Values)
		{
			const FString TableName(*TablePair.Key);
			const FJsonObject& Spec = *TablePair.Value->AsObject();
			const UDataTable* const* Found = Tables.Find(TableName);
			const UDataTable* Table = Found ? *Found : nullptr;
			if (!Table)
			{
				bool bMustExist = false;
				if (Spec.TryGetBoolField(TEXT("must_exist"), bMustExist) && bMustExist)
				{
					Problems.Add(FString::Printf(TEXT("%s: the table is not loaded"), *TableName));
				}
				continue;
			}
			const FJsonObjectPtr Columns = ObjectOf(Spec, TEXT("columns"));
			if (!Columns.IsValid())
			{
				Problems.Add(FString::Printf(TEXT("%s: the schema declares no columns for this table yet"), *TableName));
				continue;
			}
			FString StructName;
			Spec.TryGetStringField(TEXT("struct"), StructName);
			const UScriptStruct* Struct = Table->GetRowStruct();
			if (!Struct || Struct != RowStructOf(StructName))
			{
				Problems.Add(FString::Printf(TEXT("%s: the row struct is %s, the schema says %s"), *TableName, *GetNameSafe(Struct), *StructName));
				continue;
			}

			const TArray<FString> Names = StringsOf(Spec, TEXT("names"));
			const TArray<FString> Prefixes = StringsOf(Spec, TEXT("prefixes"));
			for (const FString& Required : StringsOf(Spec, TEXT("required_rows")))
			{
				if (!Table->GetRowMap().Contains(FName(*Required)))
				{
					Problems.Add(FString::Printf(TEXT("%s: the row %s, which the code calls by name, is missing"), *TableName, *Required));
				}
			}
			for (const TPair<FName, uint8*>& Row : Table->GetRowMap())
			{
				const FString RowName = Row.Key.ToString();
				const FString Where = FString::Printf(TEXT("%s.%s"), *TableName, *RowName);
				FRegexMatcher Matcher(Pattern, RowName);
				if (!Matcher.FindNext() || Matcher.GetMatchBeginning() != 0 || Matcher.GetMatchEnding() != RowName.Len())
				{
					Problems.Add(FString::Printf(TEXT("%s: the row name is not lower snake case"), *Where));
				}
				if (!Names.IsEmpty())
				{
					if (!HasText(Names, RowName))
					{
						Problems.Add(FString::Printf(TEXT("%s: this table holds only the rows %s"), *Where, *FString::Join(Names, TEXT(", "))));
					}
				}
				else if (!Prefixes.ContainsByPredicate([&RowName](const FString& Prefix) { return RowName.StartsWith(Prefix, ESearchCase::CaseSensitive); }))
				{
					Problems.Add(FString::Printf(TEXT("%s: the row name does not start with %s"), *Where, *FString::Join(Prefixes, TEXT(" or "))));
				}
				if (const FString* Other = OwnerOf.Find(Row.Key))
				{
					Problems.Add(FString::Printf(TEXT("%s: the row name is used by %s and %s"), *RowName, **Other, *TableName));
				}
				else
				{
					OwnerOf.Add(Row.Key, TableName);
				}

				for (TFieldIterator<FProperty> It(Struct, EFieldIteratorFlags::ExcludeSuper); It; ++It)
				{
					const FJsonObjectPtr Column = ObjectOf(*Columns, *It->GetName());
					if (Column.IsValid())
					{
						CheckCell(Problems, FString::Printf(TEXT("%s.%s"), *Where, *It->GetName()), *It, It->ContainerPtrToValuePtr<void>(Row.Value), Schema, *Column, Tables, Row.Key);
					}
				}
			}
		}
		return Problems;
	}

	bool AnyCarries(const TArray<FString>& Problems, std::initializer_list<const TCHAR*> Words)
	{
		return Problems.ContainsByPredicate([&Words](const FString& Line)
		{
			for (const TCHAR* Word : Words)
			{
				if (!Line.Contains(Word, ESearchCase::CaseSensitive))
				{
					return false;
				}
			}
			return true;
		});
	}

	// The object under a path of field names, or null when a step is missing.
	FJsonObjectPtr At(FJsonObjectPtr Node, std::initializer_list<const TCHAR*> Path)
	{
		for (const TCHAR* Step : Path)
		{
			Node = Node.IsValid() ? ObjectOf(*Node, Step) : FJsonObjectPtr();
		}
		return Node;
	}

	// The tables the loader holds, under their asset names: the member Rules is the asset DT_Rules. Read from the reflected
	// struct, so that a table added to the loader is checked here without a line of this file. OutNames gets every
	// member, loaded or not.
	FTableMap TablesOfTheGame(const FCosmicDataTables& Game, TArray<FString>& OutNames)
	{
		FTableMap Tables;
		for (TFieldIterator<FObjectPropertyBase> It(FCosmicDataTables::StaticStruct()); It; ++It)
		{
			if (It->PropertyClass && It->PropertyClass->IsChildOf(UDataTable::StaticClass()))
			{
				const FString Name = FString(TEXT("DT_")) + It->GetName();
				OutNames.Add(Name);
				if (const UDataTable* Table = Cast<UDataTable>(It->GetObjectPropertyValue_InContainer(&Game)))
				{
					Tables.Add(Name, Table);
				}
			}
		}
		return Tables;
	}

	// A table of the test with the rows of a table of the game, to break one thing in.
	template <typename RowType>
	UDataTable* CopyOf(const UDataTable* Source)
	{
		UDataTable* Copy = NewObject<UDataTable>(GetTransientPackage());
		Copy->RowStruct = RowType::StaticStruct();
		if (Source)
		{
			for (const FName RowName : Source->GetRowNames())
			{
				if (const RowType* Row = Source->FindRow<RowType>(RowName, TEXT("Data.Integrity"), false))
				{
					Copy->AddRow(RowName, *Row);
				}
			}
		}
		return Copy;
	}

	// B5 [B-5d2]: every KillPlayer of the tables' effect arrays, as "<table>.<row>.<column> <cause>", in the order of the
	// tables and their rows - the hazards' four, the events', the feats' price, the edicts' and the transcendents' per-tick
	// effects, and both outcomes of every card's choice.
	TArray<FString> KillPlayersOf(const FCosmicDataTables& Tables)
	{
		TArray<FString> Found;
		auto Walk = [&Found](const TCHAR* Table, FName Row, const FString& Column, const TArray<FCosmicEffect>& Effects)
		{
			for (const FCosmicEffect& Effect : Effects)
			{
				if (Effect.Type == EEffectType::KillPlayer)
				{
					Found.Add(FString::Printf(TEXT("%s.%s.%s %s"), Table, *Row.ToString(), *Column, *Effect.Tag.ToString()));
				}
			}
		};
		auto RowsOf = [](const UDataTable* Table) { return Table ? Table->GetRowNames() : TArray<FName>(); };
		for (const FName Row : RowsOf(Tables.Hazards.Get()))
		{
			if (const FHazardDef* Hazard = Tables.FindHazard(Row))
			{
				Walk(TEXT("DT_Hazards"), Row, TEXT("OnExpireEffects"), Hazard->OnExpireEffects);
				Walk(TEXT("DT_Hazards"), Row, TEXT("OnExpireBranchEffects"), Hazard->OnExpireBranchEffects);
				Walk(TEXT("DT_Hazards"), Row, TEXT("OnExpireNeighbourEffects"), Hazard->OnExpireNeighbourEffects);
				Walk(TEXT("DT_Hazards"), Row, TEXT("OnResolveEffects"), Hazard->OnResolveEffects);
			}
		}
		for (const FName Row : RowsOf(Tables.Events.Get()))
		{
			if (const FCosmicEventDef* Event = Tables.FindEvent(Row))
			{
				Walk(TEXT("DT_Events"), Row, TEXT("Effects"), Event->Effects);
			}
		}
		for (const FName Row : RowsOf(Tables.Feats.Get()))
		{
			if (const FAscensionFeatDef* Feat = Tables.FindFeat(Row))
			{
				Walk(TEXT("DT_Feats"), Row, TEXT("AscensionCost"), Feat->AscensionCost);
			}
		}
		for (const FName Row : RowsOf(Tables.Edicts.Get()))
		{
			if (const FEdictDef* Edict = Tables.FindEdict(Row))
			{
				Walk(TEXT("DT_Edicts"), Row, TEXT("EffectsPerTick"), Edict->EffectsPerTick);
			}
		}
		for (const FName Row : RowsOf(Tables.Transcendents.Get()))
		{
			if (const FTranscendentDef* Transcendent = Tables.FindTranscendent(Row))
			{
				Walk(TEXT("DT_Transcendents"), Row, TEXT("PressureEffectsPerTick"), Transcendent->PressureEffectsPerTick);
			}
		}
		for (const FName Row : RowsOf(Tables.Encounters.Get()))
		{
			if (const FEncounterDef* Card = Tables.FindEncounter(Row))
			{
				for (int32 At = 0; At < Card->Choices.Num(); ++At)
				{
					Walk(TEXT("DT_Encounters"), Row, FString::Printf(TEXT("Choices[%d].OnSuccess"), At), Card->Choices[At].OnSuccess);
					Walk(TEXT("DT_Encounters"), Row, FString::Printf(TEXT("Choices[%d].OnFailure"), At), Card->Choices[At].OnFailure);
				}
			}
		}
		return Found;
	}
}

// The JSON and CSV files under Tools/data are the source of the DataTable and String Table assets, and the game reads only
// the assets. The script that generates them leaves the hashes of what it read behind; a file edited afterwards without
// running the script again fails here.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicDataSourcesInSyncTest, "Cosmic_Ascension.Data.SourcesInSync", CosmicTests::Flags)
bool FCosmicDataSourcesInSyncTest::RunTest(const FString& Parameters)
{
	const FString SourceDir = FPaths::ProjectDir() / TEXT("Tools/data");
	if (!IFileManager::Get().DirectoryExists(*SourceDir))
	{
		AddInfo(TEXT("No Tools/data folder next to this build: nothing to compare."));
		return true;
	}

	TArray<FString> Lines;
	if (!TestTrue(TEXT("Tools/data/generated.sha1 can be read (run Tools/make_data_tables.py)"), FFileHelper::LoadFileToStringArray(Lines, *(SourceDir / TEXT("generated.sha1")))))
	{
		return false;
	}

	// One line per source the script turned into an asset, one for the schema it checked them against and one for the
	// checker it checked them with: "<file name> <sha1>".
	TMap<FString, FString> Recorded;
	for (const FString& Line : Lines)
	{
		FString FileName;
		FString Hash;
		if (Line.TrimStartAndEnd().Split(TEXT(" "), &FileName, &Hash))
		{
			Recorded.Add(FileName, Hash.TrimStartAndEnd());
		}
		else if (!Line.TrimStartAndEnd().IsEmpty())
		{
			AddError(FString::Printf(TEXT("A line of the stamp is not '<file name> <sha1>': %s"), *Line));
		}
	}
	TestTrue(TEXT("The stamp lists at least one source"), Recorded.Num() > 0);
	TestTrue(TEXT("The stamp lists the checker the sources were checked with (Tools/make_data_tables.py stamps it)"), Recorded.Contains(TEXT("../cosmic_data_schema.py")));

	for (const TPair<FString, FString>& Entry : Recorded)
	{
		// A name may climb out of the folder: "../cosmic_data_schema.py" is the checker, which lives next to it. (The
		// second argument: the project dir itself starts with "..", and without it the call stops there.)
		FString Path = SourceDir / Entry.Key;
		FPaths::CollapseRelativeDirectories(Path, true);
		const FString Current = CosmicDataTests::HashOfSource(Path);
		TestTrue(*FString::Printf(TEXT("%s is unchanged since the assets were generated (run Tools/make_data_tables.py)"), *Entry.Key),
			!Current.IsEmpty() && Current.Equals(Entry.Value, ESearchCase::IgnoreCase));
	}

	// A source the script does not know is a table nobody generates. schema.json is a JSON of the folder too: the stamp
	// has its line, so a schema edited without building the assets again fails here.
	for (const TCHAR* Extension : { TEXT("json"), TEXT("csv") })
	{
		TArray<FString> Found;
		IFileManager::Get().FindFiles(Found, *SourceDir, Extension);
		for (const FString& FoundPath : Found)
		{
			// In 5.8 this overload gives file names only; GetCleanFilename keeps the line right for either form.
			const FString FileName = FPaths::GetCleanFilename(FoundPath);
			TestTrue(*FString::Printf(TEXT("%s is listed in the stamp (add it to Tools/make_data_tables.py)"), *FileName), Recorded.Contains(FileName));
		}
	}
	return true;
}

// Tools/data/schema.json against the code and against the loaded assets (section 7.3, 9.3). The sources are checked in
// full by Tools/cosmic_data_schema.py when the assets are built; what this test reads is in the note above ParseJson.
// It runs in an editor binary (the tests are started with -game from UnrealEditor-Cmd): only there does the tag manager
// know which tags the ini spells out, and only there do names keep the case they are written in.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicDataIntegrityTest, "Cosmic_Ascension.Data.Integrity", CosmicTests::Flags)
bool FCosmicDataIntegrityTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDataTests;
	const FString SchemaPath = FPaths::ProjectDir() / TEXT("Tools/data/schema.json");
	if (!IFileManager::Get().FileExists(*SchemaPath))
	{
		AddInfo(TEXT("No Tools/data/schema.json next to this build: nothing to check."));
		return true;
	}
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	FString SchemaText;
	const bool bRead = FFileHelper::LoadFileToString(SchemaText, *SchemaPath);
	const FJsonObjectPtr Schema = bRead ? ParseJson(SchemaText) : FJsonObjectPtr();
	if (!TestTrue(TEXT("Tools/data/schema.json can be read as JSON"), Schema.IsValid()))
	{
		return false;
	}
	const FJsonObjectPtr SchemaTables = ObjectOf(*Schema, TEXT("tables"));
	if (!TestTrue(TEXT("The schema declares tables"), SchemaTables.IsValid() && SchemaTables->Values.Num() > 0))
	{
		return false;
	}

	// Every problem is an error line of its own, so that one run names all of them.
	auto Report = [this](const TCHAR* What, const TArray<FString>& Problems)
	{
		for (const FString& Problem : Problems)
		{
			AddError(FString::Printf(TEXT("%s: %s"), What, *Problem));
		}
		return Problems.IsEmpty();
	};
	// A check seen failing: some problem line carries all the words.
	auto Expect = [this](const FString& What, const TArray<FString>& Problems, std::initializer_list<const TCHAR*> Words)
	{
		TestTrue(*(FString(TEXT("Reported: ")) + What), AnyCarries(Problems, Words));
	};
	// A fresh copy of the schema with one thing changed in it.
	auto SchemaWith = [&SchemaText](TFunctionRef<void(const FJsonObjectPtr&)> Change)
	{
		const FJsonObjectPtr Copy = ParseJson(SchemaText);
		Change(Copy);
		return Copy;
	};
	auto AgainstCode = [&SchemaWith](TFunctionRef<void(const FJsonObjectPtr&)> Change)
	{
		return CheckSchemaAgainstCode(*SchemaWith(Change));
	};

	// ---- The schema and the code ----
	TestTrue(TEXT("Every column of every declared row struct, every enumerator, the word lists and the per-tick list agree with the code"),
		Report(TEXT("schema against code"), CheckSchemaAgainstCode(*Schema)));

	Expect(TEXT("a schema without tables"), AgainstCode([](const FJsonObjectPtr& Root) { Root->RemoveField(TEXT("tables")); }), { TEXT("schema: no tables") });
	Expect(TEXT("a schema without word lists"), AgainstCode([](const FJsonObjectPtr& Root) { Root->RemoveField(TEXT("words")); }), { TEXT("schema: no words") });
	Expect(TEXT("a column of a row struct that the schema does not declare"), AgainstCode([](const FJsonObjectPtr& Root)
	{
		if (const FJsonObjectPtr Columns = At(Root, { TEXT("tables"), TEXT("DT_Rules"), TEXT("columns") }))
		{
			Columns->RemoveField(TEXT("EventSlotTicks"));
		}
	}), { TEXT("schema.DT_Rules.EventSlotTicks"), TEXT("does not declare it") });
	Expect(TEXT("a column the schema declares and the struct lacks"), AgainstCode([](const FJsonObjectPtr& Root)
	{
		if (const FJsonObjectPtr Columns = At(Root, { TEXT("tables"), TEXT("DT_Rules"), TEXT("columns") }))
		{
			const FJsonObjectPtr Column = MakeShared<FJsonObject>();
			Column->SetStringField(TEXT("kind"), TEXT("number"));
			Columns->SetObjectField(TEXT("NoSuchKnob"), Column);
		}
	}), { TEXT("schema.DT_Rules.NoSuchKnob"), TEXT("does not have") });
	{
		// The same column under a name in another case: the struct has Severity, the schema would say severity.
		const TArray<FString> OtherCase = AgainstCode([](const FJsonObjectPtr& Root)
		{
			if (const FJsonObjectPtr Columns = At(Root, { TEXT("tables"), TEXT("DT_Events"), TEXT("columns") }))
			{
				const FJsonObjectPtr Column = ObjectOf(*Columns, TEXT("Severity"));
				Columns->RemoveField(TEXT("Severity"));
				Columns->SetObjectField(TEXT("severity"), Column);
			}
		});
		Expect(TEXT("a column the schema declares in another case, as a column it lacks"), OtherCase, { TEXT("schema.DT_Events.Severity"), TEXT("does not declare it") });
		Expect(TEXT("a column the schema declares in another case, as a column the struct lacks"), OtherCase, { TEXT("schema.DT_Events.severity"), TEXT("does not have") });
	}
	Expect(TEXT("an enum column under the name of another enum"), AgainstCode([](const FJsonObjectPtr& Root)
	{
		if (const FJsonObjectPtr Column = At(Root, { TEXT("tables"), TEXT("DT_Events"), TEXT("columns"), TEXT("Subject") }))
		{
			Column->SetStringField(TEXT("enum"), TEXT("ECosmicEventTrigger"));
		}
	}), { TEXT("schema.DT_Events.Subject"), TEXT("is of the enum ECosmicEventSubject") });
	{
		auto WithRequired = [&AgainstCode](const TCHAR* Table, TFunctionRef<void(TArray<FString>&)> Change)
		{
			return AgainstCode([Table, &Change](const FJsonObjectPtr& Root)
			{
				if (const FJsonObjectPtr Spec = At(Root, { TEXT("tables"), Table }))
				{
					TArray<FString> Rows = StringsOf(*Spec, TEXT("required_rows"));
					Change(Rows);
					TArray<TSharedPtr<FJsonValue>> Values;
					for (const FString& Row : Rows)
					{
						Values.Add(MakeShared<FJsonValueString>(Row));
					}
					Spec->SetArrayField(TEXT("required_rows"), Values);
				}
			});
		};
		Expect(TEXT("a row the code calls by name that required_rows does not list"), WithRequired(TEXT("DT_Civs"), [](TArray<FString>& Rows) { Rows.Reset(); }),
			{ TEXT("schema.DT_Civs"), TEXT("the code calls the row arch_default by name") });
		Expect(TEXT("a required row the code does not call by name"), WithRequired(TEXT("DT_LogTemplates"), [](TArray<FString>& Rows) { Rows.Add(TEXT("log_nobody")); }),
			{ TEXT("schema.DT_LogTemplates"), TEXT("required_rows lists log_nobody") });
		Expect(TEXT("a schema that lists no header of row names"), AgainstCode([](const FJsonObjectPtr& Root) { Root->RemoveField(TEXT("row_name_sources")); }),
			{ TEXT("schema.row_name_sources"), TEXT("no header is listed") });
		Expect(TEXT("a header of row names that cannot be read"), AgainstCode([](const FJsonObjectPtr& Root)
		{
			Root->SetArrayField(TEXT("row_name_sources"), JsonStrings({ TEXT("Source/Nowhere.h") }));
		}), { TEXT("schema.row_name_sources"), TEXT("Source/Nowhere.h cannot be read") });
		TestTrue(TEXT("A constant that is commented out is not a row the code calls"),
			RowNamesInCode(TEXT("A(TEXT(\"row_a\")); // B(TEXT(\"row_b\"));\n/* C(\n TEXT(\"row_c\")); */ D(TEXT(\"row_d\"));")) == TArray<FString>{ TEXT("row_a"), TEXT("row_d") });
	}
	Expect(TEXT("a column of whole numbers that the schema does not call integer"), AgainstCode([](const FJsonObjectPtr& Root)
	{
		if (const FJsonObjectPtr Column = At(Root, { TEXT("tables"), TEXT("DT_Rules"), TEXT("columns"), TEXT("ThroneLockTicks") }))
		{
			Column->RemoveField(TEXT("integer"));
		}
	}), { TEXT("schema.DT_Rules.ThroneLockTicks"), TEXT("holds whole numbers") });

	// A kind that does not fit the type of its column, one case per type of column the row structs have.
	struct FKindCase
	{
		const TCHAR* Table;
		const TCHAR* Column;
		const TCHAR* WrongKind;
		const TCHAR* Fits;
	};
	const FKindCase KindCases[] = {
		{ TEXT("DT_Rules"), TEXT("PlayerArchetypeRow"), TEXT("plain"), TEXT("(row)") },
		{ TEXT("DT_Civs"), TEXT("DisplayName"), TEXT("plain"), TEXT("(text)") },
		{ TEXT("DT_Events"), TEXT("Trigger"), TEXT("number"), TEXT("(enum)") },
		{ TEXT("DT_Rules"), TEXT("EventSlotTicks"), TEXT("text"), TEXT("(number)") },
		{ TEXT("DT_Rules"), TEXT("EventSlotTicks"), TEXT("Number"), TEXT("(number)") },
		{ TEXT("DT_Civs"), TEXT("AIProfile"), TEXT("row"), TEXT("(tag)") },
		{ TEXT("DT_Civs"), TEXT("Traits"), TEXT("tag"), TEXT("(tags)") },
		{ TEXT("DT_Civs"), TEXT("EthicsPreset"), TEXT("number"), TEXT("(plain)") },
		{ TEXT("DT_Civs"), TEXT("NameBank"), TEXT("numbers"), TEXT("(texts)") },
		{ TEXT("DT_Rules"), TEXT("PressureBySourceTier"), TEXT("number"), TEXT("(numbers)") },
		{ TEXT("DT_Civs"), TEXT("DedicationBanks"), TEXT("tags"), TEXT("(tag_list)") },
		{ TEXT("DT_Events"), TEXT("Conditions"), TEXT("effects"), TEXT("(conditions)") },
		{ TEXT("DT_Events"), TEXT("Effects"), TEXT("conditions"), TEXT("(effects)") },
		{ TEXT("DT_Civs"), TEXT("HazardSpawnMultipliers"), TEXT("row_map"), TEXT("(tag_map)") },
		{ TEXT("DT_Civs"), TEXT("InitialRelations"), TEXT("tag_map"), TEXT("(row_map or word_map)") },
		{ TEXT("DT_Hazards"), TEXT("RulerResolutions"), TEXT("row"), TEXT("(rows)") },
		{ TEXT("DT_Hazards"), TEXT("OutbreakRow"), TEXT("rows"), TEXT("(row)") },
		{ TEXT("DT_Megaprojects"), TEXT("CostPerTick"), TEXT("plain"), TEXT("(pool)") },
		{ TEXT("DT_Encounters"), TEXT("Choices"), TEXT("effects"), TEXT("(structs)") },
		{ TEXT("DT_Encounters"), TEXT("Choices"), TEXT("plain"), TEXT("(structs)") },
	};
	for (const FKindCase& Case : KindCases)
	{
		const TArray<FString> Unfit = AgainstCode([&Case](const FJsonObjectPtr& Root)
		{
			if (const FJsonObjectPtr Column = At(Root, { TEXT("tables"), Case.Table, TEXT("columns"), Case.Column }))
			{
				Column->SetStringField(TEXT("kind"), Case.WrongKind);
			}
		});
		const FString Where = FString::Printf(TEXT("schema.%s.%s"), Case.Table, Case.Column);
		Expect(FString::Printf(TEXT("the kind %s on %s.%s, a column that is %s"), Case.WrongKind, Case.Table, Case.Column, Case.Fits), Unfit, { *Where, TEXT("does not fit"), Case.Fits });
	}
	{
		// A list of row names is of the kind rows (the crisis handling work); a list of other structs is structs and a
		// resource pool is pool (MVP item 2) - not "plain", under which they would go unchecked on both sides.
		const FProperty* RowNames = FHazardDef::StaticStruct()->FindPropertyByName(TEXT("RulerResolutions"));
		const FProperty* Structs = FEncounterDef::StaticStruct()->FindPropertyByName(TEXT("Choices"));
		const FProperty* Pool = FEncounterChoice::StaticStruct()->FindPropertyByName(TEXT("Cost"));
		const FProperty* Plain = FCivArchetypeDef::StaticStruct()->FindPropertyByName(TEXT("EthicsPreset"));
		TestTrue(TEXT("A list of row names is of the kind rows"), RowNames && KindsFor(RowNames) == TArray<FString>{ TEXT("rows") });
		TestTrue(TEXT("A list of structs is of the kind structs"), Structs && KindsFor(Structs) == TArray<FString>{ TEXT("structs") });
		TestTrue(TEXT("A resource pool is of the kind pool"), Pool && KindsFor(Pool) == TArray<FString>{ TEXT("pool") });
		TestTrue(TEXT("Another struct is plain"), Plain && KindsFor(Plain) == TArray<FString>{ TEXT("plain") });
	}
	{
		// The columns of a list of structs are held to the members of its entries, as the columns of a row are.
		auto Choice = [&AgainstCode](TFunctionRef<void(const FJsonObjectPtr&)> Change)
		{
			return AgainstCode([&Change](const FJsonObjectPtr& Root) { Change(At(Root, { TEXT("tables"), TEXT("DT_Encounters"), TEXT("columns"), TEXT("Choices") })); });
		};
		auto Member = [](const FJsonObjectPtr& Choices, const TCHAR* Name) { return Choices.IsValid() ? At(Choices, { TEXT("columns"), Name }) : FJsonObjectPtr(); };
		Expect(TEXT("a member of an entry that the list does not declare"), Choice([](const FJsonObjectPtr& Choices)
		{
			if (const FJsonObjectPtr Columns = Choices.IsValid() ? ObjectOf(*Choices, TEXT("columns")) : FJsonObjectPtr())
			{
				Columns->RemoveField(TEXT("Difficulty"));
			}
		}), { TEXT("schema.DT_Encounters.Choices.Difficulty"), TEXT("does not declare it") });
		Expect(TEXT("a column of a list that its entries do not have"), Choice([](const FJsonObjectPtr& Choices)
		{
			if (const FJsonObjectPtr Columns = Choices.IsValid() ? ObjectOf(*Choices, TEXT("columns")) : FJsonObjectPtr())
			{
				const FJsonObjectPtr Column = MakeShared<FJsonObject>();
				Column->SetStringField(TEXT("kind"), TEXT("number"));
				Columns->SetObjectField(TEXT("Odds"), Column);
			}
		}), { TEXT("schema.DT_Encounters.Choices.Odds"), TEXT("does not have") });
		Expect(TEXT("a member of an entry under a kind that does not fit it"), Choice([&Member](const FJsonObjectPtr& Choices)
		{
			if (const FJsonObjectPtr Column = Member(Choices, TEXT("CheckStat")))
			{
				Column->SetStringField(TEXT("kind"), TEXT("row"));
			}
		}), { TEXT("schema.DT_Encounters.Choices.CheckStat"), TEXT("does not fit"), TEXT("(enum)") });
		Expect(TEXT("a member of whole numbers that the list does not call integer"), Choice([&Member](const FJsonObjectPtr& Choices)
		{
			if (const FJsonObjectPtr Column = Member(Choices, TEXT("Difficulty")))
			{
				Column->RemoveField(TEXT("integer"));
			}
		}), { TEXT("schema.DT_Encounters.Choices.Difficulty"), TEXT("holds whole numbers") });
		Expect(TEXT("an enum member under the name of another enum"), Choice([&Member](const FJsonObjectPtr& Choices)
		{
			if (const FJsonObjectPtr Column = Member(Choices, TEXT("CheckStat")))
			{
				Column->SetStringField(TEXT("enum"), TEXT("ECosmicEventTrigger"));
			}
		}), { TEXT("schema.DT_Encounters.Choices.CheckStat"), TEXT("is of the enum ECosmicCheckStat") });
		Expect(TEXT("a list of structs that does not declare its columns"), Choice([](const FJsonObjectPtr& Choices)
		{
			if (Choices.IsValid())
			{
				Choices->RemoveField(TEXT("columns"));
			}
		}), { TEXT("schema.DT_Encounters.Choices"), TEXT("declares the columns of its entries") });
		TestFalse(TEXT("The columns of the list as the schema declares them are the members of FEncounterChoice"),
			AnyCarries(CheckSchemaAgainstCode(*Schema), { TEXT("schema.DT_Encounters.Choices.") }));
	}

	Expect(TEXT("a condition enumerator the schema does not list"), AgainstCode([](const FJsonObjectPtr& Root)
	{
		if (const FJsonObjectPtr Conditions = At(Root, { TEXT("conditions") }))
		{
			Conditions->RemoveField(TEXT("CivHasNotable"));
		}
	}), { TEXT("schema.conditions"), TEXT("CivHasNotable") });
	Expect(TEXT("a condition enumerator the schema spells in another case"), AgainstCode([](const FJsonObjectPtr& Root)
	{
		if (const FJsonObjectPtr Conditions = At(Root, { TEXT("conditions") }))
		{
			Conditions->RemoveField(TEXT("CivHasNotable"));
			Conditions->SetObjectField(TEXT("civhasnotable"), MakeShared<FJsonObject>());
		}
	}), { TEXT("schema.conditions"), TEXT("civhasnotable") });
	Expect(TEXT("an effect the schema lists and the code does not have"), AgainstCode([](const FJsonObjectPtr& Root)
	{
		if (const FJsonObjectPtr Effects = At(Root, { TEXT("effects") }))
		{
			Effects->SetObjectField(TEXT("Smite"), MakeShared<FJsonObject>());
		}
	}), { TEXT("schema.effects"), TEXT("Smite") });
	Expect(TEXT("an enum list that is not the enum of the code"), AgainstCode([](const FJsonObjectPtr& Root)
	{
		if (const FJsonObjectPtr Enums = At(Root, { TEXT("enums") }))
		{
			Enums->SetArrayField(TEXT("ECosmicEventSubject"), JsonStrings({ TEXT("Civilization") }));
		}
	}), { TEXT("schema.enums.ECosmicEventSubject"), TEXT("SettledSystem") });
	Expect(TEXT("an enum the code does not have"), AgainstCode([](const FJsonObjectPtr& Root)
	{
		if (const FJsonObjectPtr Enums = At(Root, { TEXT("enums") }))
		{
			Enums->SetArrayField(TEXT("ENoSuchEnum"), JsonStrings({ TEXT("One") }));
		}
	}), { TEXT("schema.enums.ENoSuchEnum"), TEXT("no such enum") });
	Expect(TEXT("a row struct the code does not have"), AgainstCode([](const FJsonObjectPtr& Root)
	{
		if (const FJsonObjectPtr Table = At(Root, { TEXT("tables"), TEXT("DT_Civs") }))
		{
			Table->SetStringField(TEXT("struct"), TEXT("NoSuchRowDef"));
		}
	}), { TEXT("schema.DT_Civs"), TEXT("NoSuchRowDef"), TEXT("no row struct") });

	{
		const TArray<FString> OtherPerTick = AgainstCode([](const FJsonObjectPtr& Root)
		{
			Root->SetArrayField(TEXT("per_tick_effects"), JsonStrings({ TEXT("addresource"), TEXT("SetFlag") }));
		});
		Expect(TEXT("a per-tick effect of the schema that a per-tick source refuses"), OtherPerTick, { TEXT("SetFlag is listed in the schema and refused") });
		Expect(TEXT("an effect a per-tick source applies and the schema does not list"), OtherPerTick, { TEXT("MultiplyIncome is not listed in the schema and applied") });
		Expect(TEXT("a per-tick effect the schema spells in another case"), OtherPerTick, { TEXT("AddResource is not listed in the schema and applied") });
	}
	{
		auto WithTags = [&AgainstCode](TFunctionRef<void(TArray<FString>&)> Change)
		{
			return AgainstCode([&Change](const FJsonObjectPtr& Root)
			{
				TArray<FString> Tags = StringsOf(*Root, TEXT("required_tags"));
				Change(Tags);
				TArray<TSharedPtr<FJsonValue>> Values;
				for (const FString& Tag : Tags)
				{
					Values.Add(MakeShared<FJsonValueString>(Tag));
				}
				Root->SetArrayField(TEXT("required_tags"), Values);
			});
		};
		// One change per copy: a tag the ini does not have, and (apart from it) a role tag taken off the list.
		const TArray<FString> OneMore = WithTags([](TArray<FString>& Tags) { Tags.Add(TEXT("Trait.NoSuchTag")); });
		Expect(TEXT("a tag the schema requires and the ini does not declare"), OneMore, { TEXT("schema.required_tags: tag Trait.NoSuchTag") });
		TestFalse(TEXT("A required tag the ini declares is not reported as undeclared"), AnyCarries(OneMore, { TEXT("schema.required_tags: tag Notable.Rebel") }));
		TestFalse(TEXT("A tag outside Notable is no matter of the role tags"), AnyCarries(OneMore, { TEXT("the role tags under Notable") }));
		Expect(TEXT("a role tag the ini declares and the list of required tags lacks"), WithTags([](TArray<FString>& Tags) { Tags.Remove(TEXT("Notable.Hearer")); }),
			{ TEXT("the role tags under Notable"), TEXT("Notable.Hearer") });
		TestFalse(TEXT("The role tags of the schema as it is are the role tags of the ini"), AnyCarries(CheckSchemaAgainstCode(*Schema), { TEXT("the role tags under Notable") }));
	}
#if WITH_EDITORONLY_DATA
	Expect(TEXT("a required tag that only its children imply"), AgainstCode([](const FJsonObjectPtr& Root)
	{
		Root->SetArrayField(TEXT("required_tags"), JsonStrings({ TEXT("Notable") }));
	}), { TEXT("schema.required_tags"), TEXT("tag Notable is not declared") });
#endif

	// The word lists that have a list in the code are that list, both ways and to the letter.
	struct FWordCase
	{
		const TCHAR* List;
		const TCHAR* Extra;
	};
	auto WithWords = [&AgainstCode](const TCHAR* List, TFunctionRef<void(TArray<FString>&)> Change)
	{
		return AgainstCode([List, &Change](const FJsonObjectPtr& Root)
		{
			if (const FJsonObjectPtr Words = At(Root, { TEXT("words") }))
			{
				TArray<FString> Listed = StringsOf(*Words, List);
				Change(Listed);
				TArray<TSharedPtr<FJsonValue>> Values;
				for (const FString& Word : Listed)
				{
					Values.Add(MakeShared<FJsonValueString>(Word));
				}
				Words->SetArrayField(List, Values);
			}
		});
	};
	for (const FWordCase& Case : { FWordCase{ TEXT("resource"), TEXT("Gold") }, FWordCase{ TEXT("role"), TEXT("Regent") }, FWordCase{ TEXT("axis"), TEXT("Militarism") }, FWordCase{ TEXT("policy"), TEXT("Greedy") } })
	{
		const FString Where = FString::Printf(TEXT("schema.words.%s"), Case.List);
		Expect(FString::Printf(TEXT("a word under %s that the code does not have"), Case.List), WithWords(Case.List, [&Case](TArray<FString>& Listed) { Listed.Add(Case.Extra); }), { *Where, Case.Extra });
		Expect(FString::Printf(TEXT("a word of the code that the list %s lacks"), Case.List), WithWords(Case.List, [](TArray<FString>& Listed) { Listed.Reset(); }), { *Where, TEXT("the schema lists []") });
		Expect(FString::Printf(TEXT("a word under %s in another case"), Case.List), WithWords(Case.List, [](TArray<FString>& Listed)
		{
			if (Listed.Num() > 0)
			{
				Listed[0] = Listed[0].ToLower();
			}
		}), { *Where, TEXT("the schema lists [") });
	}

	// The words a condition or an effect carries in its Key are the words the evaluator resolves, list by list, and a
	// word outside the list resolves to nothing.
	{
		Sim->NewGalaxy(42, 5, 3);
		FGalaxyState Bench = Sim->GetState();
		if (!TestTrue(TEXT("The bench galaxy has two civilizations and a system"), Bench.Civs.Num() >= 2 && Bench.Systems.Num() >= 1))
		{
			return false;
		}
		// A vessel on the throne of the first civilization, which it reached from the second; the second has a rebel.
		Bench.Calling.LifeState = ELifeState::Embodied;
		Bench.Player.Role = EPlayerRole::Ruler;
		Bench.Civs[0].Stockpile = FResourcePool();
		Bench.Player.FormerCivID = Bench.Civs[1].CivID;
		FNotableRecord& Rebel = Bench.Civs[1].Notables.AddDefaulted_GetRef();
		Rebel.Role = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable.Rebel")));
		Rebel.FirstTick = 1;
		Rebel.LastTick = 0;
		FGalaxyState Wandering = Bench;
		Wandering.Player.Role = EPlayerRole::Wanderer;
		FCosmicStateIndex Index;
		Index.Rebuild(Bench);
		auto Holds = [&](const FGalaxyState& State, EConditionType Type, const FString& Key, float Value, const TCHAR* TagName = nullptr)
		{
			FCosmicCondition Condition;
			Condition.Type = Type;
			Condition.Key = FName(*Key);
			Condition.Value = Value;
			Condition.Tag = TagName ? FGameplayTag::RequestGameplayTag(FName(TagName)) : FGameplayTag();
			FCosmicEvalView View(State, Index, Sim->GetTables(), Sim->GetRules());
			View.CivIndex = 0;
			View.SystemIndex = 0;
			return UCosmicEventSubsystem::Check(Condition, View);
		};
		auto EveryWord = [&](const TCHAR* List, TFunctionRef<bool(const FString&)> Resolves)
		{
			const FJsonObjectPtr Words = ObjectOf(*Schema, TEXT("words"));
			const TArray<FString> Listed = Words.IsValid() ? StringsOf(*Words, List) : TArray<FString>();
			TestTrue(*FString::Printf(TEXT("The schema lists words under %s"), List), Listed.Num() > 0);
			for (const FString& Word : Listed)
			{
				TestTrue(*FString::Printf(TEXT("%s, a word of the list %s, is a word the evaluator resolves"), *Word, List), Resolves(Word));
			}
			TestFalse(*FString::Printf(TEXT("A word outside the list %s resolves to nothing"), List), Resolves(TEXT("NoSuchWord")));
		};
		EveryWord(TEXT("resource"), [&](const FString& Word) { return Holds(Bench, EConditionType::CivResourceAtLeast, Word, 0.f); });
		EveryWord(TEXT("axis"), [&](const FString& Word) { return Holds(Bench, EConditionType::CivEthicsAxisAbove, Word, -2.f); });
		EveryWord(TEXT("stat"), [&](const FString& Word) { return Holds(Bench, EConditionType::PlayerStatAtLeast, Word, 0.f); });
		EveryWord(TEXT("role"), [&](const FString& Word) { return Holds(Bench, EConditionType::PlayerRoleIs, Word, 0.f) != Holds(Wandering, EConditionType::PlayerRoleIs, Word, 0.f); });
		EveryWord(TEXT("civ_selector"), [&](const FString& Word) { return Holds(Bench, EConditionType::CivHasNotable, Word, 0.f, TEXT("Notable.Rebel")); });
		EveryWord(TEXT("research"), [&](const FString& Word) { return Holds(Bench, EConditionType::CivResearchAtLeast, Word, 0.f); });
	}

	// ---- The loaded assets ----
	const FCosmicDataTables& Game = Sim->GetTables();
	TArray<FString> LoaderTables;
	const FTableMap Loaded = TablesOfTheGame(Game, LoaderTables);
	for (const TCHAR* TableName : { TEXT("DT_Rules"), TEXT("DT_Civs"), TEXT("DT_LogTemplates") })
	{
		TestTrue(*FString::Printf(TEXT("%s, a table of the first phase, is a table of the loader and is loaded"), TableName), LoaderTables.Contains(TableName) && Loaded.Contains(TableName));
	}
	for (const FString& TableName : LoaderTables)
	{
		const FJsonObjectPtr Spec = ObjectOf(*SchemaTables, *TableName);
		TestTrue(*FString::Printf(TEXT("%s, a table of the loader, is declared in the schema"), *TableName), DeclaresTable(SchemaTables, TableName));
		if (Loaded.Contains(TableName))
		{
			// A table that has an asset has to exist from then on. Without the flag, the day its asset no longer loads
			// every check of the table is skipped in silence.
			bool bMustExist = false;
			TestTrue(*FString::Printf(TEXT("%s is loaded, so the schema says must_exist for it"), *TableName),
				Spec.IsValid() && Spec->TryGetBoolField(TEXT("must_exist"), bMustExist) && bMustExist);
		}
	}
	{
		// Under a key in another case the table is not declared: the sources are read by the exact name.
		const FJsonObjectPtr OtherCase = SchemaWith([](const FJsonObjectPtr& Root)
		{
			if (const FJsonObjectPtr AllTables = At(Root, { TEXT("tables") }))
			{
				const FJsonObjectPtr Spec = ObjectOf(*AllTables, TEXT("DT_Civs"));
				AllTables->RemoveField(TEXT("DT_Civs"));
				AllTables->SetObjectField(TEXT("dt_civs"), Spec);
			}
		});
		TestFalse(TEXT("A table the schema declares under a key in another case is not declared"), DeclaresTable(ObjectOf(*OtherCase, TEXT("tables")), TEXT("DT_Civs")));
		TestTrue(TEXT("The same table under its own key is"), DeclaresTable(SchemaTables, TEXT("DT_Civs")));
	}
	TestTrue(TEXT("The loaded tables hold the schema: row names, row references, tags, array lengths and the ranges of the numbers"),
		Report(TEXT("loaded assets"), CheckTables(*Schema, Loaded)));

	// Seen failing, one broken copy at a time.
	if (TestTrue(TEXT("The three tables of the game are loaded, with the rows the copies below start from"),
		Game.Rules && Game.Civs && Game.LogTemplates && Game.FindRules() && Game.FindCiv(CosmicRows::ArchDefault) && Game.FindLogTemplate(CosmicRows::LogBreakthrough[0])))
	{
		const FGameplayTag Plague = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Plague")));
		// The problems of the tables of the game with one of them replaced (or, with null, taken away), under the schema
		// of the project or under a changed copy of it.
		auto With = [&](const TCHAR* TableName, const UDataTable* Replacement, const FJsonObjectPtr& Under = FJsonObjectPtr())
		{
			FTableMap Tables = Loaded;
			if (Replacement)
			{
				Tables.Add(TableName, Replacement);
			}
			else
			{
				Tables.Remove(TableName);
			}
			return CheckTables(Under.IsValid() ? *Under : *Schema, Tables);
		};
		auto WithRules = [&](TFunctionRef<void(FCosmicRulesDef&)> Change, FName NewName = NAME_None, const FJsonObjectPtr& Under = FJsonObjectPtr())
		{
			FCosmicRulesDef Row = *Game.FindRules();
			Change(Row);
			UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
			Table->RowStruct = FCosmicRulesDef::StaticStruct();
			Table->AddRow(NewName.IsNone() ? CosmicRows::Rules : NewName, Row);
			return With(TEXT("DT_Rules"), Table, Under);
		};
		auto WithCiv = [&](TFunctionRef<void(FCivArchetypeDef&)> Change, FName NewName = NAME_None, const FJsonObjectPtr& Under = FJsonObjectPtr())
		{
			UDataTable* Table = CopyOf<FCivArchetypeDef>(Game.Civs);
			FCivArchetypeDef Row = *Game.FindCiv(CosmicRows::ArchDefault);
			Change(Row);
			Table->RemoveRow(CosmicRows::ArchDefault);
			Table->AddRow(NewName.IsNone() ? CosmicRows::ArchDefault : NewName, Row);
			return With(TEXT("DT_Civs"), Table, Under);
		};
		auto WithLog = [&](TFunctionRef<void(FLogTemplateDef&)> Change, FName RowName, const FJsonObjectPtr& Under = FJsonObjectPtr())
		{
			UDataTable* Table = CopyOf<FLogTemplateDef>(Game.LogTemplates);
			if (const FLogTemplateDef* Source = Game.FindLogTemplate(RowName))
			{
				FLogTemplateDef Row = *Source;
				Change(Row);
				Table->RemoveRow(RowName);
				Table->AddRow(RowName, Row);
			}
			return With(TEXT("DT_LogTemplates"), Table, Under);
		};
		auto SameRules = [](FCosmicRulesDef&) {};
		auto SameCiv = [](FCivArchetypeDef&) {};

		TestTrue(TEXT("A copy of the rules table passes as the table itself does"), Report(TEXT("copy of DT_Rules"), WithRules(SameRules)));
		TestTrue(TEXT("A copy of the archetype table passes as the table itself does"), Report(TEXT("copy of DT_Civs"), WithCiv(SameCiv)));
		TestTrue(TEXT("A copy of the log table passes as the table itself does"), Report(TEXT("copy of DT_LogTemplates"), WithLog([](FLogTemplateDef&) {}, CosmicRows::LogFirstLife)));
		{
			const FJsonObjectPtr NoTables = ParseJson(SchemaText);
			NoTables->RemoveField(TEXT("tables"));
			Expect(TEXT("loaded tables under a schema without tables"), CheckTables(*NoTables, Loaded), { TEXT("schema: no tables") });
		}

		// Row names.
		Expect(TEXT("a row name without the prefix of its table"), WithCiv(SameCiv, FName(TEXT("civ_default"))), { TEXT("DT_Civs.civ_default"), TEXT("does not start with arch_") });
		Expect(TEXT("a missing row that the code calls by name"), WithCiv(SameCiv, FName(TEXT("arch_other"))), { TEXT("DT_Civs"), TEXT("the row arch_default"), TEXT("is missing") });
		Expect(TEXT("a row name that is not lower snake case"), WithCiv(SameCiv, FName(TEXT("arch_Other"))), { TEXT("DT_Civs.arch_Other"), TEXT("not lower snake case") });
		Expect(TEXT("a row name with a character that is no part of a name"), WithCiv(SameCiv, FName(TEXT("arch_default@1"))), { TEXT("not lower snake case") });
		Expect(TEXT("a row name two tables use"), WithCiv(SameCiv, FName(TEXT("log_first_life"))), { TEXT("log_first_life"), TEXT("is used by") });
		Expect(TEXT("a row of the single-row table under another name"), WithRules(SameRules, FName(TEXT("rules_two"))), { TEXT("DT_Rules.rules_two"), TEXT("holds only the rows rules") });
		Expect(TEXT("a row of the single-row table under its name in another case"), WithRules(SameRules, FName(TEXT("Rules"))), { TEXT("DT_Rules.Rules"), TEXT("holds only the rows rules") });
		Expect(TEXT("a table the schema requires and the game did not load"), With(TEXT("DT_LogTemplates"), nullptr), { TEXT("DT_LogTemplates"), TEXT("not loaded") });
		Expect(TEXT("a table under another row struct"), With(TEXT("DT_Civs"), Game.Rules), { TEXT("DT_Civs"), TEXT("the row struct is") });
		// A table whose columns the schema has not declared yet has no asset yet.
		Expect(TEXT("a loaded table whose columns the schema has not declared"), With(TEXT("DT_Civs"), Game.Civs, SchemaWith([](const FJsonObjectPtr& Root)
		{
			if (const FJsonObjectPtr Table = At(Root, { TEXT("tables"), TEXT("DT_Civs") }))
			{
				Table->SetField(TEXT("columns"), MakeShared<FJsonValueNull>());
			}
		})), { TEXT("DT_Civs"), TEXT("declares no columns") });

		// Row references.
		Expect(TEXT("a row reference that names no row"), WithRules([](FCosmicRulesDef& Row) { Row.PlayerArchetypeRow = FName(TEXT("arch_nobody")); }),
			{ TEXT("DT_Rules.rules.PlayerArchetypeRow"), TEXT("DT_Civs has no row arch_nobody") });
		Expect(TEXT("a required row reference left empty"), WithRules([](FCosmicRulesDef& Row) { Row.PlayerArchetypeRow = NAME_None; }),
			{ TEXT("PlayerArchetypeRow"), TEXT("a row of DT_Civs is required") });
		Expect(TEXT("a row name used as a map key that names no row"), WithCiv([](FCivArchetypeDef& Row) { Row.InitialRelations = { { FName(TEXT("arch_nobody")), 0.5f } }; }),
			{ TEXT("InitialRelations[arch_nobody]"), TEXT("has no row") });

		// Tags.
		Expect(TEXT("a single tag under the wrong root"), WithCiv([Plague](FCivArchetypeDef& Row) { Row.AIProfile = Plague; }), { TEXT("AIProfile"), TEXT("not under AI.Profile.") });
		Expect(TEXT("a tag of a container under the wrong root"), WithCiv([Plague](FCivArchetypeDef& Row) { Row.Traits.AddTag(Plague); }), { TEXT("Traits"), TEXT("not under Trait.") });
		Expect(TEXT("a tag of a list under the wrong root"), WithCiv([Plague](FCivArchetypeDef& Row) { Row.DedicationBanks = { Plague }; }), { TEXT("DedicationBanks[0]"), TEXT("not under Faith.Bank.") });
		Expect(TEXT("a tag used as a map key under the wrong root"),
			WithCiv([](FCivArchetypeDef& Row) { Row.HazardSpawnMultipliers = { { FGameplayTag::RequestGameplayTag(FName(TEXT("Law.Remnant"))), 1.f } }; }),
			{ TEXT("HazardSpawnMultipliers[Law.Remnant]"), TEXT("not under Hazard.") });
		Expect(TEXT("a required tag left empty"), WithCiv([](FCivArchetypeDef& Row) { Row.RevelationMode = FGameplayTag(); }), { TEXT("RevelationMode"), TEXT("a tag is required") });
		Expect(TEXT("an empty tag in a list"), WithCiv([](FCivArchetypeDef& Row) { Row.DedicationBanks = { FGameplayTag() }; }), { TEXT("DedicationBanks[0]"), TEXT("a tag is required") });
		{
			// The same empty tag, under the schema of the project (optional) and under a copy that does not say so.
			auto EmptyProfile = [](FCivArchetypeDef& Row) { Row.AIProfile = FGameplayTag(); Row.HomeStarClass = FGameplayTag(); };
			TestTrue(TEXT("An optional tag left empty is not a problem"), Report(TEXT("empty optional tags"), WithCiv(EmptyProfile)));
			Expect(TEXT("the same empty tag where the schema does not call it optional"), WithCiv(EmptyProfile, NAME_None, SchemaWith([](const FJsonObjectPtr& Root)
			{
				if (const FJsonObjectPtr Column = At(Root, { TEXT("tables"), TEXT("DT_Civs"), TEXT("columns"), TEXT("AIProfile") }))
				{
					Column->RemoveField(TEXT("optional"));
				}
			})), { TEXT("DT_Civs.arch_default.AIProfile"), TEXT("a tag is required") });
		}
		// The tags inside conditions and effects are held to the ini too: the sources were, on the day the assets were
		// built, and the ini is not in the stamp.
		auto WithEvent = [&](const FGameplayTag& ConditionTag, const FGameplayTag& EffectTag)
		{
			FCosmicEventDef Row;
			Row.Category = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Civ.Economy")));
			Row.LogCategory = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Civ")));
			Row.Conditions.AddDefaulted_GetRef().Tag = ConditionTag;
			Row.Effects.AddDefaulted_GetRef().Tag = EffectTag;
			UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
			Table->RowStruct = FCosmicEventDef::StaticStruct();
			Table->AddRow(FName(TEXT("event_test")), Row);
			return With(TEXT("DT_Events"), Table);
		};
		// A list of row names (the rows kind) is held to the tables it names, on the loaded assets too.
		auto WithHazard = [&](TFunctionRef<void(FHazardDef&)> Change)
		{
			UDataTable* Table = CopyOf<FHazardDef>(Game.Hazards);
			const TArray<FName> Names = Game.GetHazardRowNames();
			if (!Names.IsEmpty())
			{
				FHazardDef Row = *Game.FindHazard(Names[0]);
				Change(Row);
				Table->RemoveRow(Names[0]);
				Table->AddRow(Names[0], Row);
			}
			return With(TEXT("DT_Hazards"), Table);
		};
		TestTrue(TEXT("The game has a hazard row to change"), !Game.GetHazardRowNames().IsEmpty());
		Expect(TEXT("a list of row names with a row no named table has"), WithHazard([](FHazardDef& Row) { Row.RulerResolutions = { FName(TEXT("enc_nobody")) }; }),
			{ TEXT("RulerResolutions[0]"), TEXT("none of DT_Megaprojects, DT_Encounters, DT_Edicts has a row enc_nobody") });
		// A project the game's own hazard rows give as an answer, whatever its name - not project_settle, which the code
		// refuses as an answer (CanStart; the schema leaves it out, decision K of the settlement work - Sim.StartMegaproject).
		FName GameAnswer;
		for (const FName HazardName : Game.GetHazardRowNames())
		{
			const FHazardDef* HazardRow = Game.FindHazard(HazardName);
			for (const FName Answer : HazardRow ? HazardRow->RulerResolutions : TArray<FName>())
			{
				if (GameAnswer.IsNone() && Answer != CosmicRows::ProjectSettle && Game.FindMegaproject(Answer))
				{
					GameAnswer = Answer;
				}
			}
		}
		if (TestFalse(TEXT("The game's hazard rows give a project as an answer"), GameAnswer.IsNone()))
		{
			TestFalse(TEXT("A project of DT_Megaprojects answers a hazard"), AnyCarries(WithHazard([GameAnswer](FHazardDef& Row) { Row.RulerResolutions = { GameAnswer }; }), { TEXT("RulerResolutions") }));
		}
		Expect(TEXT("a list of row names with an empty name"), WithHazard([](FHazardDef& Row) { Row.WandererResolutions = { NAME_None }; }),
			{ TEXT("WandererResolutions[0]"), TEXT("none of DT_Encounters has a row None") });
		const FGameplayTag Declared = FGameplayTag::RequestGameplayTag(FName(TEXT("Trait.Arch.Seeker")));
		TestFalse(TEXT("Declared tags and empty tags in conditions and effects are not reported"), AnyCarries(WithEvent(Declared, FGameplayTag()), { TEXT("].Tag") }));
		{
			// A tag whose name the ini does not have at all: what a tag that left the ini looks like in a loaded asset. It
			// keeps its name, it counts as valid, and it never matches again. (Any binary knows that it has no such tag.)
			FGameplayTag Gone;
			if (const FNameProperty* NameOfTag = CastField<FNameProperty>(FGameplayTag::StaticStruct()->FindPropertyByName(TEXT("TagName"))))
			{
				NameOfTag->SetPropertyValue_InContainer(&Gone, FName(TEXT("Trait.NoSuchTag")));
			}
			if (TestTrue(TEXT("The test can build a tag with a name the ini does not have"), Gone.IsValid()))
			{
				Expect(TEXT("a tag of a loaded row whose name the ini does not have"), WithCiv([Gone](FCivArchetypeDef& Row) { Row.AIProfile = Gone; }),
					{ TEXT("DT_Civs.arch_default.AIProfile"), TEXT("Trait.NoSuchTag is not declared") });
				Expect(TEXT("a tag in a condition of a loaded row whose name the ini does not have"), WithEvent(Gone, Declared),
					{ TEXT("DT_Events.event_test.Conditions[0].Tag"), TEXT("Trait.NoSuchTag is not declared") });
				Expect(TEXT("a tag in an effect of a loaded row whose name the ini does not have"), WithEvent(Declared, Gone),
					{ TEXT("DT_Events.event_test.Effects[0].Tag"), TEXT("Trait.NoSuchTag is not declared") });
			}
		}
#if WITH_EDITORONLY_DATA
		// Only the editor binaries know which tags the ini spells out (the tests run in one).
		const FGameplayTag Implied = FGameplayTag::RequestGameplayTag(FName(TEXT("Trait.Arch")));
		Expect(TEXT("a parent tag that only its children imply"), WithCiv([Implied](FCivArchetypeDef& Row) { Row.Traits.AddTag(Implied); }), { TEXT("Traits"), TEXT("Trait.Arch is not declared") });
		Expect(TEXT("a parent tag in a condition of a loaded row"), WithEvent(Implied, Declared), { TEXT("DT_Events.event_test.Conditions[0].Tag"), TEXT("Trait.Arch is not declared") });
		Expect(TEXT("a parent tag in an effect of a loaded row"), WithEvent(Declared, Implied), { TEXT("DT_Events.event_test.Effects[0].Tag"), TEXT("Trait.Arch is not declared") });
#endif

		// Numbers.
		Expect(TEXT("a number above its range"), WithRules([](FCosmicRulesDef& Row) { Row.FallenTranscendentCount = 4; }), { TEXT("FallenTranscendentCount"), TEXT("is above 3") });
		Expect(TEXT("a cumulative share that does not end at its mark"), WithRules([](FCosmicRulesDef& Row) { Row.LivingWorldStarShares.Last() = 0.9f; }),
			{ TEXT("DT_Rules.rules.LivingWorldStarShares"), TEXT("the last entry has to be 1") });
		Expect(TEXT("an empty cumulative share"), WithRules([](FCosmicRulesDef& Row) { Row.LivingWorldStarShares.Reset(); }),
			{ TEXT("DT_Rules.rules.LivingWorldStarShares"), TEXT("the last entry has to be 1") });
		Expect(TEXT("a number below its range"), WithRules([](FCosmicRulesDef& Row) { Row.EventSlotTicks = 0; }), { TEXT("EventSlotTicks"), TEXT("is below 1") });
		Expect(TEXT("a number that is not finite"), WithRules([](FCosmicRulesDef& Row) { Row.PressureAdjacencyFalloff = std::numeric_limits<float>::quiet_NaN(); }),
			{ TEXT("PressureAdjacencyFalloff"), TEXT("not a finite number") });
		Expect(TEXT("a number that is not finite in a column without a range"), WithRules([](FCosmicRulesDef& Row) { Row.ReverenceDecay = std::numeric_limits<float>::infinity(); }),
			{ TEXT("ReverenceDecay"), TEXT("not a finite number") });
		Expect(TEXT("a map value below its range"), WithCiv([Plague](FCivArchetypeDef& Row) { Row.HazardSpawnMultipliers = { { Plague, -1.f } }; }),
			{ TEXT("HazardSpawnMultipliers[Hazard.Plague]"), TEXT("is below 0") });
		Expect(TEXT("a map value above its range"), WithCiv([](FCivArchetypeDef& Row) { Row.InitialRelations = { { CosmicRows::ArchDefault, 2.f } }; }),
			{ TEXT("InitialRelations[arch_default]"), TEXT("is above 1") });
		Expect(TEXT("an array of numbers of the wrong length"), WithRules([](FCosmicRulesDef& Row) { Row.PressureBySourceTier = { 0.f, 0.f, 0.25f, 1.f }; }),
			{ TEXT("PressureBySourceTier"), TEXT("exactly 5 entries, found 4") });
		Expect(TEXT("an entry of an array of numbers out of range"), WithRules([](FCosmicRulesDef& Row) { Row.PressureBySourceTier = { 0.f, 0.f, 0.25f, 1.f, -2.f }; }),
			{ TEXT("PressureBySourceTier[4]"), TEXT("is below 0") });
		Expect(TEXT("an array of numbers that does not rise"), WithRules([](FCosmicRulesDef& Row) { Row.PopulationBandThresholds = { 0.5f, 0.5f, 2.f }; }),
			{ TEXT("PopulationBandThresholds"), TEXT("do not rise") });
		// B3: the lists the code reads by position (a stance, a tier, a defence share).
		Expect(TEXT("a list of stance terms of the wrong length"), WithRules([](FCosmicRulesDef& Row) { Row.RelationStanceTerms = { 0.4f }; }),
			{ TEXT("RelationStanceTerms"), TEXT("exactly 2 entries, found 1") });
		Expect(TEXT("a list of friendly slots of the wrong length"), WithRules([](FCosmicRulesDef& Row) { Row.FriendlySlotsByTier = { 1, 1, 2, 2 }; }),
			{ TEXT("FriendlySlotsByTier"), TEXT("exactly 5 entries, found 4") });
		Expect(TEXT("a list of fleets by tier of the wrong length"), WithRules([](FCosmicRulesDef& Row) { Row.FleetPerSettledByTier = { 0.f, 1.f, 1.f, 1.f }; }),
			{ TEXT("FleetPerSettledByTier"), TEXT("exactly 5 entries, found 4") });
		Expect(TEXT("a list of defence shares of the wrong length"), WithRules([](FCosmicRulesDef& Row) { Row.SiegeDefenceShares = { 1.f, 0.5f }; }),
			{ TEXT("SiegeDefenceShares"), TEXT("exactly 3 entries, found 2") });
		Expect(TEXT("a friendly slot below 0"), WithRules([](FCosmicRulesDef& Row) { Row.FriendlySlotsByTier = { 1, 1, 2, 2, -1 }; }),
			{ TEXT("FriendlySlotsByTier[4]"), TEXT("is below 0") });
		Expect(TEXT("a map key that is no word of its list"), WithRules([](FCosmicRulesDef& Row) { Row.LureAcceptByPolicy = { { FName(TEXT("Greedy")), 0.5f } }; }),
			{ TEXT("LureAcceptByPolicy[Greedy]"), TEXT("none of the words of policy") });
		Expect(TEXT("a map key that is a word of its list in another case"), WithRules([](FCosmicRulesDef& Row) { Row.LureAcceptByPolicy = { { FName(TEXT("cautious")), 0.5f } }; }),
			{ TEXT("LureAcceptByPolicy[cautious]"), TEXT("none of the words of policy") });
		{
			// A float column against a bound that a float cannot hold exactly: the value that sits on the bound passes, as
			// it does on the sources (0.85f is a little above 0.85).
			const FJsonObjectPtr Bounded = SchemaWith([](const FJsonObjectPtr& Root)
			{
				if (const FJsonObjectPtr Column = At(Root, { TEXT("tables"), TEXT("DT_Rules"), TEXT("columns"), TEXT("LureAcceptByPolicy") }))
				{
					Column->SetNumberField(TEXT("max"), 0.85);
					Column->SetNumberField(TEXT("min"), 0.7);
				}
			});
			const FName Tribute(TEXT("Tribute"));
			TestTrue(TEXT("A float value that sits on a decimal bound is inside it, above and below"),
				Report(TEXT("float on its bound"), WithRules([Tribute](FCosmicRulesDef& Row) { Row.LureAcceptByPolicy = { { Tribute, 0.85f }, { FName(TEXT("Cautious")), 0.7f } }; }, NAME_None, Bounded)));
			Expect(TEXT("a float value above a decimal bound"), WithRules([Tribute](FCosmicRulesDef& Row) { Row.LureAcceptByPolicy = { { Tribute, 0.86f } }; }, NAME_None, Bounded),
				{ TEXT("LureAcceptByPolicy[Tribute]"), TEXT("is above 0.85") });
			Expect(TEXT("a float value below a decimal bound"), WithRules([Tribute](FCosmicRulesDef& Row) { Row.LureAcceptByPolicy = { { Tribute, 0.69f } }; }, NAME_None, Bounded),
				{ TEXT("LureAcceptByPolicy[Tribute]"), TEXT("is below 0.7") });
		}

		// Counts of texts.
		const FText Word = FText::AsCultureInvariant(TEXT("word"));
		Expect(TEXT("an array of texts with a count that is none of the allowed"), WithCiv([&Word](FCivArchetypeDef& Row) { Row.AdjectiveBank = { Word, Word }; }),
			{ TEXT("AdjectiveBank"), TEXT("2 entries is none of the allowed counts") });
		Expect(TEXT("a bank of names that is neither empty nor long enough"), WithCiv([&Word](FCivArchetypeDef& Row) { Row.PersonNameBank = { Word, Word, Word }; }),
			{ TEXT("PersonNameBank"), TEXT("none or 10 entries and more, found 3") });
		Expect(TEXT("a log row with fewer variants than three"), WithLog([&Word](FLogTemplateDef& Row) { Row.Templates = { Word, Word }; }, CosmicRows::LogBreakthrough[0]),
			{ TEXT("log_breakthrough_physics.Templates"), TEXT("3 entries or more, found 2") });
		Expect(TEXT("a single-variant log row with no variant at all"), WithLog([](FLogTemplateDef& Row) { Row.Templates.Reset(); }, CosmicRows::LogFirstLife),
			{ TEXT("log_first_life.Templates"), TEXT("1 entries or more, found 0") });
		TestTrue(TEXT("A single-variant log row with one variant is not a problem"),
			Report(TEXT("one variant"), WithLog([&Word](FLogTemplateDef& Row) { Row.Templates = { Word }; }, CosmicRows::LogFirstLife)));
		TestTrue(TEXT("Three variants are enough for a row that needs three"),
			Report(TEXT("three variants"), WithLog([&Word](FLogTemplateDef& Row) { Row.Templates = { Word, Word, Word }; }, CosmicRows::LogBreakthrough[0])));
		// The one variant of a listed row is asked for also where the column states no least count (as the sources read it).
		Expect(TEXT("a single-variant log row with no variant, in a column without a least count"), WithLog([](FLogTemplateDef& Row) { Row.Templates.Reset(); }, CosmicRows::LogFirstLife, SchemaWith([](const FJsonObjectPtr& Root)
		{
			if (const FJsonObjectPtr Column = At(Root, { TEXT("tables"), TEXT("DT_LogTemplates"), TEXT("columns"), TEXT("Templates") }))
			{
				Column->RemoveField(TEXT("min"));
			}
		})), { TEXT("log_first_life.Templates"), TEXT("1 entries or more, found 0") });

		// A resource pool, member by member.
		{
			UDataTable* Projects = CopyOf<FMegaprojectDef>(Game.Megaprojects);
			if (const FMegaprojectDef* Settle = Game.FindMegaproject(CosmicRows::ProjectSettle))
			{
				FMegaprojectDef Row = *Settle;
				Row.CostPerTick.Matter = -1.f;
				Projects->RemoveRow(CosmicRows::ProjectSettle);
				Projects->AddRow(CosmicRows::ProjectSettle, Row);
			}
			Expect(TEXT("a resource of a pool below the least of its column"), With(TEXT("DT_Megaprojects"), Projects),
				{ TEXT("DT_Megaprojects.project_settle.CostPerTick.Matter"), TEXT("is below 0") });
		}
		// A list of structs, entry by entry and member by member: a table of the test stands in for DT_Encounters, with the
		// cards the code calls by name (B3's first contact, B4's pressure cards, the six the epitaph leaves out - B5) copied
		// from the game's.
		{
			auto WithCard = [&](TFunctionRef<void(FEncounterDef&)> Change)
			{
				FEncounterDef Row;
				Row.Title = Word;
				Row.Body = Word;
				Row.Trigger = ECosmicEventTrigger::Standing;
				Row.Choices.SetNum(2);
				for (FEncounterChoice& Choice : Row.Choices)
				{
					Choice.Label = Word;
					Choice.SuccessText = Word;
				}
				Change(Row);
				UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
				Table->RowStruct = FEncounterDef::StaticStruct();
				Table->AddRow(FName(TEXT("petition_test")), Row);
				// The cards the code calls by name, as the game has them.
				for (const FName Named : { CosmicRows::PetitionFirstContact, CosmicRows::PetitionPressureClaim, CosmicRows::PetitionPressureMeasured,
					CosmicRows::PetitionKeepThrone, CosmicRows::PetitionExalt, CosmicRows::PetitionWiseHeart, CosmicRows::PetitionFeast,
					CosmicRows::EncPlagueTend, CosmicRows::EncStarWatch })
				{
					if (const FEncounterDef* Card = Game.FindEncounter(Named))
					{
						Table->AddRow(Named, *Card);
					}
				}
				return With(TEXT("DT_Encounters"), Table);
			};
			TestTrue(TEXT("A table of cards that holds the schema passes"), Report(TEXT("cards"), WithCard([](FEncounterDef&) {})));
			TestTrue(TEXT("A choice whose next card is a row of the table passes"),
				Report(TEXT("chained card"), WithCard([](FEncounterDef& Row) { Row.Choices[0].NextEncounter = FName(TEXT("petition_test")); })));
			Expect(TEXT("a list of structs with a count that is none of the allowed"), WithCard([](FEncounterDef& Row) { Row.Choices.SetNum(1); }),
				{ TEXT("DT_Encounters.petition_test.Choices"), TEXT("1 entries is none of the allowed counts") });
			Expect(TEXT("a resource of a pool inside an entry below its least"), WithCard([](FEncounterDef& Row) { Row.Choices[1].Cost.Influence = -5.f; }),
				{ TEXT("DT_Encounters.petition_test.Choices[1].Cost.Influence"), TEXT("is below 0") });
			Expect(TEXT("a number inside an entry above its range"), WithCard([](FEncounterDef& Row) { Row.Choices[0].Difficulty = 17; }),
				{ TEXT("DT_Encounters.petition_test.Choices[0].Difficulty"), TEXT("is above 16") });
			Expect(TEXT("a row name inside an entry that names no row"), WithCard([](FEncounterDef& Row) { Row.Choices[0].NextEncounter = FName(TEXT("petition_nobody")); }),
				{ TEXT("DT_Encounters.petition_test.Choices[0].NextEncounter"), TEXT("DT_Encounters has no row petition_nobody") });
			FGameplayTag Gone;
			if (const FNameProperty* NameOfTag = CastField<FNameProperty>(FGameplayTag::StaticStruct()->FindPropertyByName(TEXT("TagName"))))
			{
				NameOfTag->SetPropertyValue_InContainer(&Gone, FName(TEXT("Trait.NoSuchTag")));
			}
			Expect(TEXT("a tag in an effect inside an entry whose name the ini does not have"), WithCard([Gone](FEncounterDef& Row)
			{
				FCosmicEffect& Effect = Row.Choices[1].OnFailure.AddDefaulted_GetRef();
				Effect.Type = EEffectType::AddTrait;
				Effect.Tag = Gone;
			}), { TEXT("DT_Encounters.petition_test.Choices[1].OnFailure[0].Tag"), TEXT("Trait.NoSuchTag is not declared") });
		}
	}

	// The immutable laws of the loaded DT_Rules (section 4.5): exactly the five, the sanctuary's Law.Hedge among them, each a
	// tag the ini declares (the schema's contains_all says the same of the source).
	if (const FCosmicRulesDef* GameRules = Game.FindRules())
	{
		TestEqual(TEXT("DT_Rules.ImmutableLaws holds five laws"), GameRules->ImmutableLaws.Num(), 5);
		for (const TCHAR* Law : { TEXT("Law.Judgment"), TEXT("Law.Sabbath"), TEXT("Law.Humility"), TEXT("Law.Remnant"), TEXT("Law.Hedge") })
		{
			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(Law), false);
			TestTrue(*FString::Printf(TEXT("%s is declared and listed in DT_Rules.ImmutableLaws"), Law), Tag.IsValid() && GameRules->ImmutableLaws.HasTagExact(Tag));
		}
	}

	// The rules row and the feat rows together (MVP item 3), on the game's tables and on two broken copies: the first crisis
	// is the tag of a Draw row (a claim or capital row is refused by the crisis stage with an error line, and a tag no row
	// holds places nothing - the schema reads the tag's root only), and a feat never asks for more settled systems than its
	// rung may hold (docs/50_ROADMAP.md, Phase 4: no feat or project asks for more systems than the rung's realm cap - the
	// rung of RequiredTier, read through RealmCapByTier).
	auto MendProblems = [](const FCosmicDataTables& Tables)
	{
		TArray<FString> Problems;
		const FCosmicRulesDef* Rules = Tables.FindRules();
		if (!Rules)
		{
			Problems.Add(TEXT("no DT_Rules row"));
			return Problems;
		}
		if (Rules->FirstCrisisHazard.IsValid())
		{
			const FHazardDef* FirstRow = Tables.FindHazardByTag(Rules->FirstCrisisHazard);
			if (!FirstRow || FirstRow->SpawnRule != ECosmicHazardSpawn::Draw)
			{
				Problems.Add(FString::Printf(TEXT("DT_Rules.FirstCrisisHazard %s is not the tag of a Draw row"), *Rules->FirstCrisisHazard.ToString()));
			}
		}
		for (const FName FeatRow : Tables.GetFeatRowNames())
		{
			const FAscensionFeatDef* Feat = Tables.FindFeat(FeatRow);
			const int32 TierIndex = Feat ? static_cast<int32>(Feat->RequiredTier) : INDEX_NONE;
			for (const FCosmicCondition& Condition : Feat ? Feat->CompletionConditions : TArray<FCosmicCondition>())
			{
				if (Condition.Type == EConditionType::CivSettledSystemsAtLeast
					&& !(Rules->RealmCapByTier.IsValidIndex(TierIndex) && Condition.Value <= static_cast<float>(Rules->RealmCapByTier[TierIndex])))
				{
					Problems.Add(FString::Printf(TEXT("DT_Feats.%s asks for more settled systems than its rung's realm cap"), *FeatRow.ToString()));
				}
			}
		}
		return Problems;
	};
	TestEqual(TEXT("The game's first crisis and feats keep to the rules row"), FString::Join(MendProblems(Game), TEXT("; ")), FString());
	if (const FCosmicRulesDef* GameRules = Game.FindRules())
	{
		FCosmicRulesDef Capital = *GameRules;
		Capital.FirstCrisisHazard = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Rebellion")));
		TestEqual(TEXT("A first crisis of the capital rule's row is caught"), MendProblems(CosmicTests::WithRules(Game, Capital)).Num(), 1);
		const FAscensionFeatDef* Worlds = Game.FindFeat(FName(TEXT("feat_three_worlds")));
		if (TestNotNull(TEXT("(the game has the second T2->T3 feat)"), Worlds))
		{
			FAscensionFeatDef Greedy = *Worlds;
			Greedy.CompletionConditions[0].Value = static_cast<float>(GameRules->RealmCapByTier[static_cast<int32>(Greedy.RequiredTier)] + 1);
			UDataTable* Feats = NewObject<UDataTable>(GetTransientPackage());
			Feats->RowStruct = FAscensionFeatDef::StaticStruct();
			Feats->AddRow(FName(TEXT("feat_test_greedy")), Greedy);
			FCosmicDataTables Broken = Game;
			Broken.Feats = Feats;
			TestEqual(TEXT("A feat asking one system more than its rung's cap is caught"), MendProblems(Broken).Num(), 1);
		}
	}

	// The choices of a card (section 5.9), entry by entry - a rule of the schema holds over every entry of a list, not over
	// each entry's own fields together: a choice with a check says what its failure is, and a choice without one never
	// fails, so it carries no failure effects, no failure line and no difficulty. B1: a wanderer's card costs nothing (no
	// purse fills PersonalPool), and a choice whose success ends the reign is the vessel's own will - no check, the cause
	// Reign.Abdicated (the throne card's last choice; a fall is a failure's effect).
	auto ChoiceProblems = [](const UDataTable* Cards)
	{
		TArray<FString> Problems;
		for (const FName RowName : Cards ? Cards->GetRowNames() : TArray<FName>())
		{
			const FEncounterDef* Card = Cards->FindRow<FEncounterDef>(RowName, TEXT("Data.Integrity"), false);
			for (int32 At = 0; Card && At < Card->Choices.Num(); ++At)
			{
				const FEncounterChoice& Choice = Card->Choices[At];
				const FString Where = FString::Printf(TEXT("DT_Encounters.%s.Choices[%d]"), *RowName.ToString(), At);
				if (Choice.CheckStat != ECosmicCheckStat::NoCheck && Choice.FailureText.IsEmpty())
				{
					Problems.Add(Where + TEXT(": a choice with a check has no failure line"));
				}
				if (Choice.CheckStat == ECosmicCheckStat::NoCheck && (!Choice.OnFailure.IsEmpty() || !Choice.FailureText.IsEmpty() || Choice.Difficulty != 0))
				{
					Problems.Add(Where + TEXT(": a choice without a check never fails, yet it has failure effects, a failure line or a difficulty"));
				}
				const FResourcePool& Cost = Choice.Cost;
				if (Card->Subject == ECosmicEventSubject::VesselSystem
					&& (Cost.Energy != 0.f || Cost.Matter != 0.f || Cost.Insight != 0.f || Cost.Influence != 0.f || Cost.Causality != 0.f))
				{
					Problems.Add(Where + TEXT(": a wanderer's card costs nothing"));
				}
				static const FGameplayTag AbdicatedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Abdicated")));
				for (const FCosmicEffect& Effect : Choice.OnSuccess)
				{
					if (Effect.Type == EEffectType::DeposeRuler && (Choice.CheckStat != ECosmicCheckStat::NoCheck || Effect.Tag != AbdicatedTag))
					{
						Problems.Add(Where + TEXT(": a choice that leaves the throne is the vessel's own will: no check, the cause Reign.Abdicated"));
					}
				}
			}
		}
		return Problems;
	};
	if (TestNotNull(TEXT("The game's cards are loaded"), Game.Encounters.Get()))
	{
		TestTrue(TEXT("Every choice of the game's cards has a failure exactly when it has a check"), Report(TEXT("card choices"), ChoiceProblems(Game.Encounters)));
		auto Broken = [&](TFunctionRef<void(FEncounterChoice& Checked, FEncounterChoice& Free)> Change)
		{
			UDataTable* Copy = CopyOf<FEncounterDef>(Game.Encounters);
			const FName Walls(TEXT("petition_walls"));
			if (const FEncounterDef* Source = Game.FindEncounter(Walls); Source && Source->Choices.Num() == 2)
			{
				FEncounterDef Row = *Source;
				Change(Row.Choices[1], Row.Choices[0]);
				Copy->RemoveRow(Walls);
				Copy->AddRow(Walls, Row);
			}
			return ChoiceProblems(Copy);
		};
		Expect(TEXT("a checked choice without a failure line"), Broken([](FEncounterChoice& Checked, FEncounterChoice&) { Checked.FailureText = FText::GetEmpty(); }),
			{ TEXT("petition_walls.Choices[1]"), TEXT("has no failure line") });
		Expect(TEXT("a free choice with a failure line"), Broken([](FEncounterChoice&, FEncounterChoice& Free) { Free.FailureText = FText::AsCultureInvariant(TEXT("x")); }),
			{ TEXT("petition_walls.Choices[0]"), TEXT("never fails") });
		Expect(TEXT("a free choice with failure effects"), Broken([](FEncounterChoice&, FEncounterChoice& Free) { Free.OnFailure.AddDefaulted(); }),
			{ TEXT("petition_walls.Choices[0]"), TEXT("never fails") });
		Expect(TEXT("a free choice with a difficulty"), Broken([](FEncounterChoice&, FEncounterChoice& Free) { Free.Difficulty = 3; }),
			{ TEXT("petition_walls.Choices[0]"), TEXT("never fails") });
		// B1: the two rules of the wanderer's loop, each broken once on a row of the game.
		auto BrokenRow = [&](const FName Name, TFunctionRef<void(FEncounterDef& Row)> Change)
		{
			UDataTable* Copy = CopyOf<FEncounterDef>(Game.Encounters);
			if (const FEncounterDef* Source = Game.FindEncounter(Name))
			{
				FEncounterDef Row = *Source;
				Change(Row);
				Copy->RemoveRow(Name);
				Copy->AddRow(Name, Row);
			}
			return ChoiceProblems(Copy);
		};
		Expect(TEXT("a wanderer's card with a price"), BrokenRow(FName(TEXT("enc_lane_weave")), [](FEncounterDef& Row) { Row.Choices[0].Cost.Energy = 5.f; }),
			{ TEXT("enc_lane_weave.Choices[0]"), TEXT("costs nothing") });
		Expect(TEXT("leaving the throne on a check"), BrokenRow(CosmicRows::PetitionKeepThrone, [](FEncounterDef& Row)
			{
				FEncounterChoice& Leave = Row.Choices.Last();
				Leave.CheckStat = ECosmicCheckStat::Stability;
				Leave.Difficulty = 4;
				Leave.FailureText = FText::AsCultureInvariant(TEXT("x"));
			}), { TEXT("petition_keep_throne.Choices[2]"), TEXT("the vessel's own will") });
		Expect(TEXT("leaving the throne as a fall"), BrokenRow(CosmicRows::PetitionKeepThrone, [](FEncounterDef& Row)
			{
				Row.Choices.Last().OnSuccess[0].Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Deposed")));
			}), { TEXT("petition_keep_throne.Choices[2]"), TEXT("the vessel's own will") });
	}

	// The edicts (B2 header section 2.7, 3.6), row by row: an edict stands until it is revoked, so no effect of it has a
	// duration; a relation it changes names its other party (the subject of a per-tick source has no other ID); and the
	// inherited edict outruns the most recovery, so that a people left alone still falls (CosmicEdicts::
	// InheritedDrainOutrunsRecovery - a rule across two tables, out of the schema's reach).
	auto EdictProblems = [](const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
	{
		TArray<FString> Problems;
		for (const FName RowName : Tables.GetEdictRowNames())
		{
			const FEdictDef* Edict = Tables.FindEdict(RowName);
			for (int32 At = 0; Edict && At < Edict->EffectsPerTick.Num(); ++At)
			{
				const FCosmicEffect& Effect = Edict->EffectsPerTick[At];
				const FString Where = FString::Printf(TEXT("DT_Edicts.%s.EffectsPerTick[%d]"), *RowName.ToString(), At);
				if (Effect.DurationTicks != 0)
				{
					Problems.Add(Where + TEXT(": an edict stands until it is revoked - no duration"));
				}
				if (Effect.Type == EEffectType::ChangeRelation && Effect.Key.IsNone())
				{
					Problems.Add(Where + TEXT(": a relation an edict changes names its other party"));
				}
			}
		}
		if (!CosmicEdicts::InheritedDrainOutrunsRecovery(Rules, Tables))
		{
			Problems.Add(FString::Printf(TEXT("DT_Rules.StartingEdict %s: the inherited edict has to take more Stability than the most recovery gives"), *Rules.StartingEdict.ToString()));
		}
		return Problems;
	};
	const FCosmicRulesDef* GameRules = Game.FindRules();
	if (TestNotNull(TEXT("The game's edicts are loaded"), Game.Edicts.Get()) && TestNotNull(TEXT("(the game's rules)"), GameRules))
	{
		TestTrue(TEXT("Every edict of the game stands without a duration and the inherited one outruns the recovery"), Report(TEXT("edicts"), EdictProblems(Game, *GameRules)));
		auto BrokenEdict = [&](TFunctionRef<void(FEdictDef& Row)> Change)
		{
			UDataTable* Copy = CopyOf<FEdictDef>(Game.Edicts);
			const FName Name(TEXT("edict_mobilization"));
			if (const FEdictDef* Source = Game.FindEdict(Name))
			{
				FEdictDef Row = *Source;
				Change(Row);
				Copy->RemoveRow(Name);
				Copy->AddRow(Name, Row);
			}
			FCosmicDataTables Broken = Game;
			Broken.Edicts = Copy;
			return Broken;
		};
		Expect(TEXT("an edict's effect with a duration"), EdictProblems(BrokenEdict([](FEdictDef& Row) { Row.EffectsPerTick[0].DurationTicks = 100; }), *GameRules),
			{ TEXT("edict_mobilization.EffectsPerTick[0]"), TEXT("no duration") });
		Expect(TEXT("an edict's relation without its party"), EdictProblems(BrokenEdict([](FEdictDef& Row) { Row.EffectsPerTick[0].Type = EEffectType::ChangeRelation; Row.EffectsPerTick[0].Key = NAME_None; }), *GameRules),
			{ TEXT("edict_mobilization.EffectsPerTick[0]"), TEXT("names its other party") });
		FCosmicRulesDef Inheriting = *GameRules;
		Inheriting.StartingEdict = FName(TEXT("edict_mobilization"));
		const FEdictDef* Mobilization = Game.FindEdict(Inheriting.StartingEdict);
		const float Drain = Mobilization ? CosmicEdicts::StabilityDrainOf(*Mobilization) : 0.f;
		TestTrue(TEXT("(the game's mobilization takes Stability)"), Drain > 0.f);
		Inheriting.StabilityRecoveryPerTick = Drain * 0.5f;
		Inheriting.SocietyRecoveryBonus = 0.5f;
		TestTrue(TEXT("The inherited mobilization outruns a recovery of half its drain x 1.5"), EdictProblems(Game, Inheriting).IsEmpty());
		Inheriting.StabilityRecoveryPerTick = Drain;
		Inheriting.SocietyRecoveryBonus = 0.f;
		Expect(TEXT("a recovery as large as the inherited edict's drain"), EdictProblems(Game, Inheriting), { TEXT("StartingEdict"), TEXT("more Stability") });
		// The net drain: a row that takes 0.0015 and gives back 0.001 drains 0.0005 a tick, below a recovery of 0.001.
		{
			UDataTable* Netted = CopyOf<FEdictDef>(Game.Edicts);
			FEdictDef Row;
			FCosmicEffect Cost;
			Cost.Type = EEffectType::ChangeStability;
			Cost.Value = -0.0015f;
			FCosmicEffect Gain = Cost;
			Gain.Value = 0.001f;
			Row.EffectsPerTick = { Cost, Gain };
			Netted->AddRow(FName(TEXT("edict_test_netted")), Row);
			FCosmicDataTables WithNetted = Game;
			WithNetted.Edicts = Netted;
			FCosmicRulesDef NettedRules = *GameRules;
			NettedRules.StartingEdict = FName(TEXT("edict_test_netted"));
			NettedRules.StabilityRecoveryPerTick = 0.001f;
			NettedRules.SocietyRecoveryBonus = 0.f;
			Expect(TEXT("an inherited edict whose gain nets its cost below the recovery"), EdictProblems(WithNetted, NettedRules), { TEXT("StartingEdict"), TEXT("more Stability") });
		}
		Inheriting.SocietyRecoveryBonus = 0.f;
		Inheriting.StartingEdict = FName(TEXT("edict_free_trade_nobody"));
		Expect(TEXT("an inherited edict the table does not hold"), EdictProblems(Game, Inheriting), { TEXT("StartingEdict"), TEXT("more Stability") });
		Inheriting.StartingEdict = FName(TEXT("edict_machine_rights"));
		Expect(TEXT("an inherited edict with no Stability cost"), EdictProblems(Game, Inheriting), { TEXT("StartingEdict"), TEXT("more Stability") });
	}

	// B4 (docs/60_DECISIONS/headers/B4_transcendent_voice_and_pressure_cards.md section 2.8, 3.3): the theodicy's budget and the
	// name of the edict a card enacts - rules across tables, out of the schema's reach. The budget: for every hazard row, the
	// share a slot of the non-grace Lottery rows of DT_Events that resolve its tag - BaseWeight x the upper end of the sphere
	// factor x EventChancePerSlot, summed - leaves at least a quarter, multiplied over the slots of the longest the hazard can
	// stand: its CountdownTicks, the positive HazardCountdownBonus for the tag of every DT_Edicts row (CountdownBonusOf adds
	// every held one), the guard's FleetDelayCap and, per card row, its largest DelayHazard on the tag; the slots are that
	// many ticks over EventSlotTicks, rounded up. The upper end of the sphere factor of a row's Category is the product, over
	// the transcendent rows that weight that Category, of the most each can give - min(3, modifier x (1 +
	// SphereEventWeightStep x the largest sphere it can hold)) when above 1, the largest sphere being MaxInfluencedSystems or
	// the one core NewGalaxy places for an InitialInfluencedSystems above 0, whichever is more: the factor stops at 3 for one
	// transcendent and not across them, and no roster gives more. A loop of products, no Pow. The per-card DelayHazard counts
	// once because such a card comes once for a claim: DelayShapeProblems below keeps the rows to that shape, and a new reign
	// cannot bring the card again while the claim stands - DT_Rules.ThroneLockTicks (every throne is locked that long after a
	// reign ends, CrownBlockOf) is at least the longest stand of every hazard a card can delay. A non-grace row of another
	// trigger (Standing, Named) that resolves the tag counts as certain - it fires whenever its conditions hold, no share of
	// a draw keeps the quarter (B4 [B-4d2] review).
	auto TheodicyProblems = [](const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
	{
		static const FGameplayTag GraceTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Grace")));
		TArray<FString> Problems;
		const double Step = Rules.SphereEventWeightStep > 0.f ? Rules.SphereEventWeightStep : 0.0;
		auto UpperFactorOf = [&Tables, Step](FGameplayTag Category)
		{
			double Factor = 1.0;
			for (const FName Row : Tables.GetTranscendentRowNames())
			{
				const FTranscendentDef* Def = Tables.FindTranscendent(Row);
				const float* Modifier = Def ? Def->EventWeightModifiers.Find(Category) : nullptr;
				if (Modifier && *Modifier > 0.f)
				{
					const int32 Largest = FMath::Max(Def->MaxInfluencedSystems, Def->InitialInfluencedSystems > 0 ? 1 : 0);
					const double Most = FMath::Min(3.0, *Modifier * (1.0 + Step * FMath::Max(0, Largest)));
					Factor *= Most > 1.0 ? Most : 1.0;
				}
			}
			return Factor;
		};
		const int64 SlotTicks = FMath::Max(1, Rules.EventSlotTicks);
		const double Chance = Rules.EventChancePerSlot > 0.f ? Rules.EventChancePerSlot : 0.0;
		const int64 GuardCap = Rules.FleetDelayCap > 0.f ? static_cast<int64>(FMath::Min(Rules.FleetDelayCap, 100000.f)) : 0;
		for (const FName HazardName : Tables.GetHazardRowNames())
		{
			const FHazardDef* Hazard = Tables.FindHazard(HazardName);
			if (!Hazard || !Hazard->HazardTag.IsValid())
			{
				continue;
			}
			const FGameplayTag Tag = Hazard->HazardTag;
			double Share = 0.0;
			for (const FName EventName : Tables.GetEventRowNames())
			{
				const FCosmicEventDef* Event = Tables.FindEvent(EventName);
				const bool bResolves = Event && !Event->Category.MatchesTag(GraceTag) && Event->Effects.ContainsByPredicate([&Tag](const FCosmicEffect& Effect)
				{
					return Effect.Type == EEffectType::ResolveHazard && (!Effect.Tag.IsValid() || Tag.MatchesTag(Effect.Tag));
				});
				if (bResolves && Event->Trigger != ECosmicEventTrigger::Lottery)
				{
					Share += 1.0;
				}
				else if (bResolves && Event->BaseWeight > 0.f)
				{
					Share += Event->BaseWeight * UpperFactorOf(Event->Category) * Chance;
				}
			}
			if (!(Share > 0.0))
			{
				continue;
			}
			int64 Longest = FMath::Max(1, Hazard->CountdownTicks) + GuardCap;
			for (const FName EdictName : Tables.GetEdictRowNames())
			{
				const FEdictDef* Edict = Tables.FindEdict(EdictName);
				const int32* Bonus = Edict ? Edict->HazardCountdownBonus.Find(Tag) : nullptr;
				Longest += (Bonus && *Bonus > 0) ? *Bonus : 0;
			}
			bool bCardDelays = false;
			for (const FName CardName : Tables.GetEncounterRowNames())
			{
				const FEncounterDef* Card = Tables.FindEncounter(CardName);
				float Most = 0.f;
				for (int32 At = 0; Card && At < Card->Choices.Num(); ++At)
				{
					for (const FCosmicEffect& Effect : Card->Choices[At].OnSuccess)
					{
						Most = (Effect.Type == EEffectType::DelayHazard && Effect.Tag.IsValid() && Tag.MatchesTag(Effect.Tag) && Effect.Value > Most) ? Effect.Value : Most;
					}
				}
				Longest += static_cast<int64>(FMath::Min(Most, 100000.f));
				bCardDelays |= Most > 0.f;
			}
			if (bCardDelays && static_cast<int64>(Rules.ThroneLockTicks) < Longest)
			{
				Problems.Add(FString::Printf(TEXT("DT_Hazards.%s: a card buys %s time, and ThroneLockTicks %d is below its longest stand (%lld ticks) - a new reign could bring the card again on a standing claim"),
					*HazardName.ToString(), *Tag.ToString(), Rules.ThroneLockTicks, Longest));
			}
			const int64 Slots = (Longest + SlotTicks - 1) / SlotTicks;
			double Stays = 1.0;
			for (int64 Slot = 0; Slot < Slots; ++Slot)
			{
				Stays *= Share < 1.0 ? 1.0 - Share : 0.0;
			}
			if (Stays < 0.25)
			{
				Problems.Add(FString::Printf(TEXT("DT_Hazards.%s: the non-grace rows that resolve %s take it away %.4f a slot; over the %lld slots of its longest stand (%lld ticks) %.3f stays - below the quarter the theodicy keeps"),
					*HazardName.ToString(), *Tag.ToString(), Share, Slots, Longest, Stays));
			}
		}
		return Problems;
	};
	// A choice that enacts an edict carries the edict's display name word for word (no {Edict} slot - the name can change); and
	// a choice holds at most one gated effect (SetStance, EnactEdict, DelayHazard), first in OnSuccess - the choice's gate
	// (CosmicPetitions::ChoiceGateOf) reads the state before the answer, so an effect before the gated one could close its
	// gate after the draw.
	auto CardEdictNameProblems = [](const FCosmicDataTables& Tables)
	{
		TArray<FString> Problems;
		for (const FName CardName : Tables.GetEncounterRowNames())
		{
			const FEncounterDef* Card = Tables.FindEncounter(CardName);
			for (int32 At = 0; Card && At < Card->Choices.Num(); ++At)
			{
				const TArray<FCosmicEffect>& OnSuccess = Card->Choices[At].OnSuccess;
				for (int32 EffectAt = 0; EffectAt < OnSuccess.Num(); ++EffectAt)
				{
					const EEffectType Type = OnSuccess[EffectAt].Type;
					if (EffectAt > 0 && (Type == EEffectType::SetStance || Type == EEffectType::EnactEdict || Type == EEffectType::DelayHazard))
					{
						Problems.Add(FString::Printf(TEXT("DT_Encounters.%s.Choices[%d].OnSuccess[%d]: a gated effect stands alone, first in its choice"), *CardName.ToString(), At, EffectAt));
					}
				}
				for (const FCosmicEffect& Effect : Card->Choices[At].OnSuccess)
				{
					const FEdictDef* Edict = Effect.Type == EEffectType::EnactEdict ? Tables.FindEdict(Effect.Key) : nullptr;
					if (Effect.Type == EEffectType::EnactEdict
						&& (!Edict || Edict->DisplayName.IsEmpty() || !Card->Choices[At].Label.ToString().Contains(Edict->DisplayName.ToString(), ESearchCase::CaseSensitive)))
					{
						Problems.Add(FString::Printf(TEXT("DT_Encounters.%s.Choices[%d]: a choice that enacts %s carries that edict's name word for word"),
							*CardName.ToString(), At, *Effect.Key.ToString()));
					}
				}
			}
		}
		return Problems;
	};
	// The shape the theodicy's count assumes of a card that buys a hazard time (one DelayHazard a claim - header section 2.8):
	// the card is no Lottery row, lies on no NextEncounter cycle, no Lottery row leads to it, and among the card and the rows
	// that lead to it (through NextEncounter, any depth) at most one is not Named - one root that comes when its condition
	// comes to hold, the rest only by the chain.
	auto DelayShapeProblems = [](const FCosmicDataTables& Tables)
	{
		TArray<FString> Problems;
		const TArray<FName> Names = Tables.GetEncounterRowNames();
		auto LeadsTo = [&Tables](FName From, FName To)
		{
			const FEncounterDef* Row = Tables.FindEncounter(From);
			return Row && Row->Choices.ContainsByPredicate([To](const FEncounterChoice& Choice) { return Choice.NextEncounter == To; });
		};
		for (const FName CardName : Names)
		{
			const FEncounterDef* Card = Tables.FindEncounter(CardName);
			const bool bDelays = Card && Card->Choices.ContainsByPredicate([](const FEncounterChoice& Choice)
			{
				return Choice.OnSuccess.ContainsByPredicate([](const FCosmicEffect& Effect) { return Effect.Type == EEffectType::DelayHazard; });
			});
			if (!bDelays)
			{
				continue;
			}
			// The rows that lead to the card, any depth (a walk back along NextEncounter).
			TArray<FName> Leading;
			TArray<FName> Open = { CardName };
			while (!Open.IsEmpty())
			{
				const FName At = Open.Pop();
				for (const FName Other : Names)
				{
					if (!Leading.Contains(Other) && LeadsTo(Other, At))
					{
						Leading.Add(Other);
						Open.Add(Other);
					}
				}
			}
			int32 Roots = Card->Trigger == ECosmicEventTrigger::Named ? 0 : 1;
			bool bLottery = Card->Trigger == ECosmicEventTrigger::Lottery;
			for (const FName Other : Leading)
			{
				const FEncounterDef* Row = Tables.FindEncounter(Other);
				Roots += (Row && Row->Trigger != ECosmicEventTrigger::Named && Other != CardName) ? 1 : 0;
				bLottery |= Row && Row->Trigger == ECosmicEventTrigger::Lottery;
			}
			if (bLottery || Leading.Contains(CardName) || Roots > 1)
			{
				Problems.Add(FString::Printf(TEXT("DT_Encounters.%s: a card that buys a hazard time comes once a claim - no Lottery row, no cycle, one root (here %s%s%d roots)"),
					*CardName.ToString(), bLottery ? TEXT("a Lottery row, ") : TEXT(""), Leading.Contains(CardName) ? TEXT("a cycle, ") : TEXT(""), Roots));
			}
		}
		return Problems;
	};
	// B5 [B-5d2] (header 3.2, 3.5, decision 6-ga): the loaded tables hold the one death the schema lets the data carry - the
	// AI awakening's own expiry, by Death.Hazard (the rule is the schema's, held on the sources by check_data.py; this is what
	// the assets the game reads hold) - and CosmicTests::WithoutDeath, the data-off side of a pair (-WithoutDeath=1), takes
	// exactly it away and keeps every row of the tables it copies. One of two pins with Tools/test_data_schema.py's
	// (test_the_lethal_expiry): the schema's rules refuse, and the pins hold the one row allowed.
	if (TestNotNull(TEXT("(the game's hazards, events, feats and cards)"), Game.Hazards.Get()) && Game.Events.Get() && Game.Feats.Get() && Game.Encounters.Get())
	{
		const TArray<FString> Deaths = KillPlayersOf(Game);
		TestTrue(*FString::Printf(TEXT("The game's one death is the AI awakening's own expiry, by Death.Hazard - decision 6-ga (2026-10-30) opened that one alone: another source of death is a new decision (DECISIONS_LOG), and this pin and Tools/test_data_schema.py's change with it (found: %s)"),
			*FString::Join(Deaths, TEXT(", "))),
			Deaths == TArray<FString>{ TEXT("DT_Hazards.crisis_ai_awakening.OnExpireEffects Death.Hazard") });
		const FCosmicDataTables Deathless = CosmicTests::WithoutDeath(Game);
		const TArray<FString> Left = KillPlayersOf(Deathless);
		TestTrue(*FString::Printf(TEXT("CosmicTests::WithoutDeath takes every death away (left: %s)"), *FString::Join(Left, TEXT(", "))), Left.IsEmpty());
		auto RowCount = [](const UDataTable* Table) { return Table ? Table->GetRowMap().Num() : -1; };
		TestTrue(TEXT("and keeps every row of the hazards, events, feats and cards"), RowCount(Deathless.Hazards.Get()) == RowCount(Game.Hazards.Get())
			&& RowCount(Deathless.Events.Get()) == RowCount(Game.Events.Get()) && RowCount(Deathless.Feats.Get()) == RowCount(Game.Feats.Get())
			&& RowCount(Deathless.Encounters.Get()) == RowCount(Game.Encounters.Get()));
		// The walk can tell: a death written anywhere else is found where it stands.
		FCosmicDataTables Moved = Game;
		UDataTable* Hazards = CopyOf<FHazardDef>(Game.Hazards);
		if (FHazardDef* Awakening = Hazards->FindRow<FHazardDef>(FName(TEXT("crisis_ai_awakening")), TEXT("Data.Integrity"), false))
		{
			FCosmicEffect Kill;
			Kill.Type = EEffectType::KillPlayer;
			Kill.Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Death.Hazard")));
			Awakening->OnExpireBranchEffects.Add(Kill);
		}
		Moved.Hazards = Hazards;
		TestTrue(TEXT("(the walk finds a death on the awakening's branch side)"), KillPlayersOf(Moved).Contains(TEXT("DT_Hazards.crisis_ai_awakening.OnExpireBranchEffects Death.Hazard")));
	}
	// B5 [B-5d2] (balance review: the first-crisis edge; DECISIONS_LOG B5 [B-5d2]): the first crisis stands on every capital at
	// FirstCrisisTick and expires its CountdownTicks later; when that row's own expiry kills, the first vessel (born at tick 0)
	// must still be inside the birth protection then - VesselAtRiskOf shields Tick - BornTick < BirthProtectionTicks, so the
	// protection must be above the expiry tick (the design's 'no death within 300 ticks of birth' read with the tick itself -
	// crisis_and_transcendents.md 4.4's forbidden list).
	auto FirstCrisisProblems = [](const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
	{
		TArray<FString> Problems;
		FName Row;
		const FHazardDef* First = Rules.FirstCrisisTick > 0 && Rules.FirstCrisisHazard.IsValid() ? Tables.FindHazardByTag(Rules.FirstCrisisHazard, &Row) : nullptr;
		const bool bKills = First && First->OnExpireEffects.ContainsByPredicate([](const FCosmicEffect& Effect) { return Effect.Type == EEffectType::KillPlayer; });
		const int64 Expiry = First ? static_cast<int64>(Rules.FirstCrisisTick) + FMath::Max(1, First->CountdownTicks) : 0;
		if (bKills && static_cast<int64>(Rules.BirthProtectionTicks) <= Expiry)
		{
			Problems.Add(FString::Printf(TEXT("DT_Hazards.%s: the first crisis kills at its expiry on tick %lld, and BirthProtectionTicks %d does not cover that tick for the first vessel"),
				*Row.ToString(), Expiry, Rules.BirthProtectionTicks));
		}
		return Problems;
	};
	if (TestNotNull(TEXT("(the game's hazards and rules for the first crisis)"), Game.Hazards.Get()) && GameRules)
	{
		TestTrue(TEXT("The first vessel is inside its birth protection when the first crisis expires"), Report(TEXT("first crisis"), FirstCrisisProblems(Game, *GameRules)));
		FCosmicRulesDef Edge = *GameRules;
		const FHazardDef* FirstRow = Game.FindHazardByTag(Edge.FirstCrisisHazard);
		Edge.BirthProtectionTicks = Edge.FirstCrisisTick + (FirstRow ? FMath::Max(1, FirstRow->CountdownTicks) : 1);
		Expect(TEXT("a birth protection ending on the first crisis's expiry tick"), FirstCrisisProblems(Game, Edge), { TEXT("crisis_ai_awakening"), TEXT("first crisis") });
		const FCosmicDataTables Deathless = CosmicTests::WithoutDeath(Game);
		TestTrue(TEXT("A first crisis that kills nobody needs no protection over it"), FirstCrisisProblems(Deathless, Edge).IsEmpty());
	}
	if (TestNotNull(TEXT("(the game's events, hazards, cards and transcendents)"), Game.Events.Get()) && Game.Hazards.Get() && Game.Encounters.Get()
		&& Game.Transcendents.Get() && Game.Edicts.Get() && GameRules)
	{
		TestTrue(TEXT("No hazard of the game is taken away for certain: a quarter of its longest stand stays"), Report(TEXT("theodicy"), TheodicyProblems(Game, *GameRules)));
		TestTrue(TEXT("Every choice of the game's cards that enacts an edict carries the edict's name, and every gated effect stands alone and first"),
			Report(TEXT("card edicts"), CardEdictNameProblems(Game)));
		TestTrue(TEXT("Every card of the game that buys a hazard time comes once a claim"), Report(TEXT("card delays"), DelayShapeProblems(Game)));
		const FGameplayTag ClaimTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Claim")));
		const FName Vigil(TEXT("edict_vigil"));
		const FEdictDef* VigilRow = Game.FindEdict(Vigil);
		// A board of test rows over the game's tables: a claim-easing Lottery row of a weight (0: none), a card whose choice
		// buys a claim time (0: none) or enacts the vigil under a label, a second edict with a claim bonus, a weight the
		// silence row puts on a category.
		struct FTheodicyBoard
		{
			float EaseWeight = 0.f;
			// The easing row's trigger (a Standing one counts as certain).
			ECosmicEventTrigger EaseTrigger = ECosmicEventTrigger::Lottery;
			float CardDelay = 0.f;
			bool bSecondBonus = false;
			FGameplayTag Weighted;
			float WeightOf = 0.f;
			FText VigilLabel;
			// The silence row's sphere cap (-1: the game's) with its one initial core.
			int32 SilenceCap = -1;
			// The resisting card's trigger, whether it chains to itself, how many Standing rows lead to it, and an effect
			// before its gated one.
			ECosmicEventTrigger CardTrigger = ECosmicEventTrigger::Named;
			bool bSelfChain = false;
			int32 LeadingRoots = 0;
			bool bEffectFirst = false;
		};
		// The board starts from the game's tables without B4's pressure data (the cards' DelayHazard and the vigil's row are
		// the cases' own).
		const FCosmicDataTables Unpressed = CosmicTests::WithoutPressure(Game);
		bool bVoiceLeft = false;
		bool bVoiceGone = true;
		for (const FName Row : Game.Transcendents->GetRowNames())
		{
			const FTranscendentDef* With = Game.FindTranscendent(Row);
			const FTranscendentDef* Without = Unpressed.FindTranscendent(Row);
			bVoiceLeft |= With && !With->ClaimTemplates.IsEmpty();
			bVoiceGone &= Without && Without->ClaimTemplates.IsEmpty();
		}
		TestTrue(TEXT("(the game's tables hold B4's pressure data: the two cards, the vigil's row, the seventh edict, a claim's voice)"),
			Game.FindEncounter(CosmicRows::PetitionPressureClaim) && Game.FindEncounter(CosmicRows::PetitionPressureMeasured) && Game.FindEvent(CosmicRows::EventClaimEased)
			&& Game.FindEdict(FName(TEXT("edict_against_silence"))) && bVoiceLeft);
		TestTrue(TEXT("CosmicTests::WithoutPressure takes all of it away and keeps every transcendent row"),
			!Unpressed.FindEncounter(CosmicRows::PetitionPressureClaim) && !Unpressed.FindEncounter(CosmicRows::PetitionPressureMeasured)
			&& !Unpressed.FindEvent(CosmicRows::EventClaimEased) && !Unpressed.FindEdict(FName(TEXT("edict_against_silence"))) && bVoiceGone
			&& Unpressed.Encounters->GetRowMap().Num() == Game.Encounters->GetRowMap().Num() - 2 && Unpressed.Events->GetRowMap().Num() == Game.Events->GetRowMap().Num() - 1
			&& Unpressed.Edicts->GetRowMap().Num() == Game.Edicts->GetRowMap().Num() - 1);
		auto BoardOf = [&](const FTheodicyBoard& Board)
		{
			FCosmicDataTables Tables = Unpressed;
			UDataTable* Events = CopyOf<FCosmicEventDef>(Unpressed.Events);
			if (Board.EaseWeight > 0.f)
			{
				FCosmicEventDef Ease;
				Ease.Category = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Crisis.Claim")));
				Ease.Trigger = Board.EaseTrigger;
				Ease.BaseWeight = Board.EaseWeight;
				FCosmicEffect Resolve;
				Resolve.Type = EEffectType::ResolveHazard;
				Resolve.Tag = ClaimTag;
				Ease.Effects = { Resolve };
				Events->AddRow(FName(TEXT("event_test_claim_eased")), Ease);
			}
			UDataTable* Cards = CopyOf<FEncounterDef>(Unpressed.Encounters);
			if (Board.CardDelay > 0.f || !Board.VigilLabel.IsEmpty())
			{
				FEncounterDef Card;
				FEncounterChoice Act;
				FCosmicEffect Effect;
				Effect.Type = Board.CardDelay > 0.f ? EEffectType::DelayHazard : EEffectType::EnactEdict;
				Effect.Tag = Board.CardDelay > 0.f ? ClaimTag : FGameplayTag();
				Effect.Key = Board.CardDelay > 0.f ? NAME_None : Vigil;
				Effect.Value = Board.CardDelay;
				Act.OnSuccess = { Effect };
				if (Board.bEffectFirst)
				{
					FCosmicEffect Before;
					Before.Type = EEffectType::ChangeStability;
					Before.Value = 0.1f;
					Act.OnSuccess.Insert(Before, 0);
				}
				Act.Label = Board.VigilLabel;
				Act.NextEncounter = Board.bSelfChain ? FName(TEXT("petition_test_resist")) : NAME_None;
				Card.Trigger = Board.CardTrigger;
				Card.Choices = { Act, FEncounterChoice() };
				Cards->AddRow(FName(TEXT("petition_test_resist")), Card);
				for (int32 Root = 0; Root < Board.LeadingRoots; ++Root)
				{
					FEncounterDef Leading;
					Leading.Trigger = ECosmicEventTrigger::Standing;
					FEncounterChoice Discern;
					Discern.NextEncounter = FName(TEXT("petition_test_resist"));
					Leading.Choices = { Discern, FEncounterChoice() };
					Cards->AddRow(FName(*FString::Printf(TEXT("petition_test_lead_%d"), Root)), Leading);
				}
			}
			UDataTable* Edicts = CopyOf<FEdictDef>(Unpressed.Edicts);
			if (Board.bSecondBonus)
			{
				FEdictDef Second;
				Second.HazardCountdownBonus.Add(ClaimTag, 100);
				Edicts->AddRow(FName(TEXT("edict_test_claim_bonus")), Second);
			}
			UDataTable* Transcendents = CopyOf<FTranscendentDef>(Unpressed.Transcendents);
			const FName Silence(TEXT("trans_silence"));
			if (const FTranscendentDef* Source = Board.Weighted.IsValid() ? Unpressed.FindTranscendent(Silence) : nullptr)
			{
				FTranscendentDef Row = *Source;
				Row.EventWeightModifiers.Add(Board.Weighted, Board.WeightOf);
				if (Board.SilenceCap >= 0)
				{
					Row.MaxInfluencedSystems = Board.SilenceCap;
					Row.InitialInfluencedSystems = 1;
				}
				Transcendents->RemoveRow(Silence);
				Transcendents->AddRow(Silence, Row);
			}
			Tables.Events = Events;
			Tables.Encounters = Cards;
			Tables.Edicts = Edicts;
			Tables.Transcendents = Transcendents;
			return Tables;
		};
		FTheodicyBoard Eased;
		Eased.EaseWeight = 1.2f;
		Eased.CardDelay = 50.f;
		TestTrue(TEXT("A claim-easing row of weight 1.2 leaves a quarter of the claim's longest stand (300 + the vigil's 100 + the guard's 100 + a card's 50)"),
			TheodicyProblems(BoardOf(Eased), *GameRules).IsEmpty());
		// A Standing row that eases the claim fires whenever its conditions hold: however light, it takes the claim for certain.
		FTheodicyBoard StandingEase = Eased;
		StandingEase.EaseWeight = 0.1f;
		StandingEase.EaseTrigger = ECosmicEventTrigger::Standing;
		Expect(TEXT("a Standing claim-easing row of weight 0.1"), TheodicyProblems(BoardOf(StandingEase), *GameRules), { TEXT("crisis_claim"), TEXT("Hazard.Claim"), TEXT("quarter") });
		// The card that buys the claim time comes once a reign: the throne lock outlasts the claim's longest stand (550).
		{
			FCosmicRulesDef ShortLock = *GameRules;
			ShortLock.ThroneLockTicks = 549;
			Expect(TEXT("a throne lock of 549 ticks under a card's DelayHazard"), TheodicyProblems(BoardOf(Eased), ShortLock), { TEXT("crisis_claim"), TEXT("ThroneLockTicks 549"), TEXT("550") });
			ShortLock.ThroneLockTicks = 550;
			TestTrue(TEXT("A throne lock of 550 ticks passes"), TheodicyProblems(BoardOf(Eased), ShortLock).IsEmpty());
		}
		FTheodicyBoard Heavier = Eased;
		Heavier.EaseWeight = 1.3f;
		Expect(TEXT("a claim-easing row of weight 1.3"), TheodicyProblems(BoardOf(Heavier), *GameRules), { TEXT("crisis_claim"), TEXT("Hazard.Claim"), TEXT("quarter") });
		FTheodicyBoard LongerCard = Eased;
		LongerCard.CardDelay = 100.f;
		Expect(TEXT("a card's DelayHazard of 100 on the claim"), TheodicyProblems(BoardOf(LongerCard), *GameRules), { TEXT("crisis_claim"), TEXT("quarter") });
		FTheodicyBoard TwoBonuses = Eased;
		TwoBonuses.bSecondBonus = true;
		Expect(TEXT("a second edict's claim bonus"), TheodicyProblems(BoardOf(TwoBonuses), *GameRules), { TEXT("crisis_claim"), TEXT("quarter") });
		FTheodicyBoard WeightedClaim = Eased;
		WeightedClaim.Weighted = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Crisis.Claim")));
		WeightedClaim.WeightOf = 2.f;
		Expect(TEXT("a transcendent row weighting Event.Crisis.Claim"), TheodicyProblems(BoardOf(WeightedClaim), *GameRules), { TEXT("crisis_claim"), TEXT("quarter") });
		FTheodicyBoard WeightedSociety;
		WeightedSociety.Weighted = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Civ.Society")));
		WeightedSociety.WeightOf = 3.f;
		Expect(TEXT("a transcendent row weighting the plague's repentance three times"), TheodicyProblems(BoardOf(WeightedSociety), *GameRules),
			{ TEXT("crisis_plague"), TEXT("Hazard.Plague"), TEXT("quarter") });
		FTheodicyBoard Named;
		Named.VigilLabel = FText::FromString(VigilRow ? TEXT("[") + VigilRow->DisplayName.ToString() + TEXT("]") : FString());
		TestTrue(TEXT("(the game's vigil has a name)"), VigilRow && !VigilRow->DisplayName.IsEmpty());
		TestTrue(TEXT("A choice that enacts the vigil under its name passes"), CardEdictNameProblems(BoardOf(Named)).IsEmpty());
		FTheodicyBoard Unnamed;
		Unnamed.VigilLabel = FText::FromString(TEXT("x"));
		Expect(TEXT("a choice that enacts the vigil under another name"), CardEdictNameProblems(BoardOf(Unnamed)), { TEXT("petition_test_resist.Choices[0]"), TEXT("edict_vigil") });
		FTheodicyBoard Behind = Named;
		Behind.bEffectFirst = true;
		Expect(TEXT("a gated effect after another effect"), CardEdictNameProblems(BoardOf(Behind)), { TEXT("petition_test_resist.Choices[0].OnSuccess[1]"), TEXT("alone, first") });
		// The sphere a fallen row can hold is at least its one core: a cap of 0 with a core weighs as a sphere of one.
		FTheodicyBoard CoreOnly;
		CoreOnly.Weighted = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Civ.Society")));
		CoreOnly.WeightOf = 2.2f;
		CoreOnly.SilenceCap = 0;
		Expect(TEXT("a weight on the plague's repentance over a sphere of one core and a cap of 0"), TheodicyProblems(BoardOf(CoreOnly), *GameRules),
			{ TEXT("crisis_plague"), TEXT("quarter") });
		// The shape of a card that buys a hazard time.
		TestTrue(TEXT("A Named card that buys a claim time, led to by nothing, comes once a claim"), DelayShapeProblems(BoardOf(Eased)).IsEmpty());
		FTheodicyBoard OneRoot = Eased;
		OneRoot.LeadingRoots = 1;
		TestTrue(TEXT("and so does one led to by one Standing card"), DelayShapeProblems(BoardOf(OneRoot)).IsEmpty());
		FTheodicyBoard InLottery = Eased;
		InLottery.CardTrigger = ECosmicEventTrigger::Lottery;
		Expect(TEXT("a Lottery card that buys a hazard time"), DelayShapeProblems(BoardOf(InLottery)), { TEXT("petition_test_resist"), TEXT("a Lottery row") });
		FTheodicyBoard Cycle = Eased;
		Cycle.bSelfChain = true;
		Expect(TEXT("a card that buys a hazard time and chains to itself"), DelayShapeProblems(BoardOf(Cycle)), { TEXT("petition_test_resist"), TEXT("a cycle") });
		FTheodicyBoard TwoRoots = Eased;
		TwoRoots.LeadingRoots = 2;
		Expect(TEXT("a card that buys a hazard time led to by two Standing cards"), DelayShapeProblems(BoardOf(TwoRoots)), { TEXT("petition_test_resist"), TEXT("2 roots") });
	}

	Sim->NewGalaxy(42, 5, 3);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

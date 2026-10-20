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
			{ TEXT("RulerResolutions[0]"), TEXT("none of DT_Megaprojects, DT_Encounters has a row enc_nobody") });
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
		// A list of structs, entry by entry and member by member. DT_Encounters has no asset before its first rows: a table
		// of the test stands in for it.
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

	Sim->NewGalaxy(42, 5, 3);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

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
	// Data.SourcesInSync ties the assets to them.

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

	// The kinds of column (the closed list of schema.json) a property may be declared as.
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

	void CompareNames(TArray<FString>& Problems, const FString& What, TArray<FString> InSchema, TArray<FString> InCode)
	{
		InSchema.Sort();
		InCode.Sort();
		if (InSchema != InCode)
		{
			Problems.Add(FString::Printf(TEXT("%s: the schema lists [%s], the code has [%s]"), *What, *FString::Join(InSchema, TEXT(", ")), *FString::Join(InCode, TEXT(", "))));
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
			for (TFieldIterator<FProperty> It(Struct, EFieldIteratorFlags::ExcludeSuper); It; ++It)
			{
				const FString ColumnName = It->GetName();
				const FJsonObjectPtr Column = ObjectOf(*Columns, *ColumnName);
				if (!Column.IsValid())
				{
					Problems.Add(FString::Printf(TEXT("schema.%s.%s: the struct has this column and the schema does not declare it"), *TableName, *ColumnName));
					continue;
				}
				FString Kind;
				Column->TryGetStringField(TEXT("kind"), Kind);
				const TArray<FString> Allowed = KindsFor(*It);
				if (!Allowed.Contains(Kind))
				{
					Problems.Add(FString::Printf(TEXT("schema.%s.%s: kind '%s' does not fit the type of the column (%s)"), *TableName, *ColumnName, *Kind, *FString::Join(Allowed, TEXT(" or "))));
				}
			}
			for (const FString& ColumnName : KeysOf(Columns))
			{
				if (!Struct->FindPropertyByName(FName(*ColumnName)))
				{
					Problems.Add(FString::Printf(TEXT("schema.%s.%s: the schema declares a column the struct does not have"), *TableName, *ColumnName));
				}
			}
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

		// The tags the code asks for by name are declared, not only implied by a child.
		for (const FString& TagName : StringsOf(Schema, TEXT("required_tags")))
		{
			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*TagName), false);
			const TSharedPtr<FGameplayTagNode> Node = Tag.IsValid() ? UGameplayTagsManager::Get().FindTagNode(Tag) : TSharedPtr<FGameplayTagNode>();
			if (!Node.IsValid() || !Node->IsExplicitTag())
			{
				Problems.Add(FString::Printf(TEXT("schema.required_tags: tag %s is not declared in the ini"), *TagName));
			}
		}

		// The per-tick list is the list of the code, effect by effect.
		const TArray<FString> PerTick = StringsOf(Schema, TEXT("per_tick_effects"));
		const UEnum* Effects = StaticEnum<EEffectType>();
		for (int32 Index = 0; Index < Effects->NumEnums() - 1; ++Index)
		{
			const FString Name = Effects->GetNameStringByIndex(Index);
			const bool bInCode = CosmicEventImpl::IsPerTickEffect(static_cast<EEffectType>(Effects->GetValueByIndex(Index)));
			if (PerTick.Contains(Name) != bInCode)
			{
				Problems.Add(FString::Printf(TEXT("schema.per_tick_effects: %s is %s in the schema and %s in the code"), *Name,
					PerTick.Contains(Name) ? TEXT("listed") : TEXT("not listed"), bInCode ? TEXT("applied by a per-tick source") : TEXT("refused by a per-tick source")));
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
	// left to the sources: a column of whole numbers is an integer property here.)
	void CheckNumber(TArray<FString>& Problems, const FString& Where, double Number, const FJsonObject& Column)
	{
		if (!(Number >= std::numeric_limits<double>::lowest() && Number <= std::numeric_limits<double>::max()))
		{
			Problems.Add(FString::Printf(TEXT("%s: not a finite number"), *Where));
			return;
		}
		double Bound = 0.0;
		if (Column.TryGetNumberField(TEXT("min"), Bound) && Number < Bound)
		{
			Problems.Add(FString::Printf(TEXT("%s: %g is below %g"), *Where, Number, Bound));
		}
		if (Column.TryGetNumberField(TEXT("max"), Bound) && Number > Bound)
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
		if (!KindsFor(Property).Contains(Kind))
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
				CheckNumber(Problems, Where, Number, Column);
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
					CheckNumber(Problems, FString::Printf(TEXT("%s[%d]"), *Where, Index), Number, Column);
					if (bAscending && Index > 0 && !(Number > Previous))
					{
						Problems.Add(FString::Printf(TEXT("%s: the entries do not rise"), *Where));
					}
					Previous = Number;
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
				if (bSingleVariant && StringsOf(Schema, TEXT("single_variant_rows")).Contains(RowName.ToString()))
				{
					Least = 1;
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
					else if (!Words.IsValid() || !StringsOf(*Words, *WordList).Contains(Key.ToString()))
					{
						Problems.Add(FString::Printf(TEXT("%s: %s is none of the words of %s"), *Here, *Key.ToString(), *WordList));
					}
				}
				double Number = 0.0;
				if (NumberOf(MapProperty->ValueProp, Helper.GetValuePtr(Index), Number))
				{
					CheckNumber(Problems, Here, Number, Column);
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
					if (!Names.Contains(RowName))
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
// the assets. The script that generates them leaves the hashes of its sources behind; a source edited afterwards without
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

	// One line per source the script turned into an asset, and one for the schema it checked them against:
	// "<file name> <sha1>".
	TMap<FString, FString> Recorded;
	for (const FString& Line : Lines)
	{
		FString FileName;
		FString Hash;
		if (Line.TrimStartAndEnd().Split(TEXT(" "), &FileName, &Hash))
		{
			Recorded.Add(FileName, Hash.TrimStartAndEnd());
		}
	}
	TestTrue(TEXT("The stamp lists at least one source"), Recorded.Num() > 0);

	for (const TPair<FString, FString>& Entry : Recorded)
	{
		const FString Current = CosmicDataTests::HashOfSource(SourceDir / Entry.Key);
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
	// The problems of a fresh copy of the schema with one thing changed in it, against the code.
	auto AgainstCode = [&SchemaText](TFunctionRef<void(const FJsonObjectPtr&)> Change)
	{
		const FJsonObjectPtr Copy = ParseJson(SchemaText);
		Change(Copy);
		return CheckSchemaAgainstCode(*Copy);
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

	Expect(TEXT("a condition enumerator the schema does not list"), AgainstCode([](const FJsonObjectPtr& Root)
	{
		if (const FJsonObjectPtr Conditions = At(Root, { TEXT("conditions") }))
		{
			Conditions->RemoveField(TEXT("CivHasNotable"));
		}
	}), { TEXT("schema.conditions"), TEXT("CivHasNotable") });
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
			Root->SetArrayField(TEXT("per_tick_effects"), JsonStrings({ TEXT("AddResource"), TEXT("SetFlag") }));
		});
		Expect(TEXT("a per-tick effect of the schema that a per-tick source refuses"), OtherPerTick, { TEXT("SetFlag is listed in the schema and refused") });
		Expect(TEXT("an effect a per-tick source applies and the schema does not list"), OtherPerTick, { TEXT("MultiplyIncome is not listed in the schema and applied") });
	}
	{
		const TArray<FString> OtherTags = AgainstCode([](const FJsonObjectPtr& Root)
		{
			Root->SetArrayField(TEXT("required_tags"), JsonStrings({ TEXT("Notable.Rebel"), TEXT("Notable.Nobody") }));
		});
		Expect(TEXT("a tag the schema requires and the ini does not declare"), OtherTags, { TEXT("schema.required_tags"), TEXT("Notable.Nobody") });
		TestFalse(TEXT("A required tag the ini declares is not reported"), AnyCarries(OtherTags, { TEXT("Notable.Rebel") }));
	}
#if WITH_EDITORONLY_DATA
	Expect(TEXT("a required tag that only its children imply"), AgainstCode([](const FJsonObjectPtr& Root)
	{
		Root->SetArrayField(TEXT("required_tags"), JsonStrings({ TEXT("Notable") }));
	}), { TEXT("schema.required_tags"), TEXT("tag Notable is not declared") });
#endif

	// The word lists that have a list in the code are that list, both ways.
	struct FWordCase
	{
		const TCHAR* List;
		const TCHAR* Extra;
	};
	for (const FWordCase& Case : { FWordCase{ TEXT("resource"), TEXT("Gold") }, FWordCase{ TEXT("role"), TEXT("Regent") }, FWordCase{ TEXT("axis"), TEXT("Militarism") }, FWordCase{ TEXT("policy"), TEXT("Greedy") } })
	{
		const TArray<FString> OneMore = AgainstCode([&Case](const FJsonObjectPtr& Root)
		{
			if (const FJsonObjectPtr Words = At(Root, { TEXT("words") }))
			{
				TArray<FString> Listed = StringsOf(*Words, Case.List);
				Listed.Add(Case.Extra);
				TArray<TSharedPtr<FJsonValue>> Values;
				for (const FString& Word : Listed)
				{
					Values.Add(MakeShared<FJsonValueString>(Word));
				}
				Words->SetArrayField(Case.List, Values);
			}
		});
		const FString Where = FString::Printf(TEXT("schema.words.%s"), Case.List);
		Expect(FString::Printf(TEXT("a word under %s that the code does not have"), Case.List), OneMore, { *Where, Case.Extra });
		const TArray<FString> NoneLeft = AgainstCode([&Case](const FJsonObjectPtr& Root)
		{
			if (const FJsonObjectPtr Words = At(Root, { TEXT("words") }))
			{
				Words->SetArrayField(Case.List, TArray<TSharedPtr<FJsonValue>>());
			}
		});
		Expect(FString::Printf(TEXT("a word of the code that the list %s lacks"), Case.List), NoneLeft, { *Where, TEXT("the schema lists []") });
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
	}

	// ---- The loaded assets ----
	const FCosmicDataTables& Game = Sim->GetTables();
	TArray<FString> LoaderTables;
	const FTableMap Loaded = TablesOfTheGame(Game, LoaderTables);
	TestTrue(TEXT("The loader holds the three tables of the first phase and more"), LoaderTables.Num() >= 3 && Loaded.Contains(TEXT("DT_Rules")) && Loaded.Contains(TEXT("DT_Civs")) && Loaded.Contains(TEXT("DT_LogTemplates")));
	for (const FString& TableName : LoaderTables)
	{
		TestTrue(*FString::Printf(TEXT("%s, a table of the loader, is declared in the schema"), *TableName), ObjectOf(*SchemaTables, *TableName).IsValid());
	}
	TestTrue(TEXT("The loaded tables hold the schema: row names, row references, tags, array lengths and the ranges of the numbers"),
		Report(TEXT("loaded assets"), CheckTables(*Schema, Loaded)));

	// Seen failing, one broken copy at a time.
	if (TestTrue(TEXT("The three tables of the game are loaded, with the rows the copies below start from"),
		Game.Rules && Game.Civs && Game.LogTemplates && Game.FindRules() && Game.FindCiv(CosmicRows::ArchDefault) && Game.FindLogTemplate(CosmicRows::LogBreakthrough[0])))
	{
		const FGameplayTag Plague = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Plague")));
		// The problems of the tables of the game with one of them replaced (or, with null, taken away).
		auto With = [&](const TCHAR* TableName, const UDataTable* Replacement)
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
			return CheckTables(*Schema, Tables);
		};
		auto WithRules = [&](TFunctionRef<void(FCosmicRulesDef&)> Change, FName NewName = NAME_None)
		{
			FCosmicRulesDef Row = *Game.FindRules();
			Change(Row);
			UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
			Table->RowStruct = FCosmicRulesDef::StaticStruct();
			Table->AddRow(NewName.IsNone() ? CosmicRows::Rules : NewName, Row);
			return With(TEXT("DT_Rules"), Table);
		};
		auto WithCiv = [&](TFunctionRef<void(FCivArchetypeDef&)> Change, FName NewName = NAME_None)
		{
			UDataTable* Table = CopyOf<FCivArchetypeDef>(Game.Civs);
			FCivArchetypeDef Row = *Game.FindCiv(CosmicRows::ArchDefault);
			Change(Row);
			Table->RemoveRow(CosmicRows::ArchDefault);
			Table->AddRow(NewName.IsNone() ? CosmicRows::ArchDefault : NewName, Row);
			return With(TEXT("DT_Civs"), Table);
		};
		auto WithLog = [&](TFunctionRef<void(FLogTemplateDef&)> Change, FName RowName)
		{
			UDataTable* Table = CopyOf<FLogTemplateDef>(Game.LogTemplates);
			if (const FLogTemplateDef* Source = Game.FindLogTemplate(RowName))
			{
				FLogTemplateDef Row = *Source;
				Change(Row);
				Table->RemoveRow(RowName);
				Table->AddRow(RowName, Row);
			}
			return With(TEXT("DT_LogTemplates"), Table);
		};
		auto SameRules = [](FCosmicRulesDef&) {};
		auto SameCiv = [](FCivArchetypeDef&) {};

		TestTrue(TEXT("The copies of the tables of the game pass as the tables themselves do"),
			WithRules(SameRules).IsEmpty() && WithCiv(SameCiv).IsEmpty() && WithLog([](FLogTemplateDef&) {}, CosmicRows::LogFirstLife).IsEmpty());
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
		Expect(TEXT("a table the schema requires and the game did not load"), With(TEXT("DT_LogTemplates"), nullptr), { TEXT("DT_LogTemplates"), TEXT("not loaded") });
		Expect(TEXT("a table under another row struct"), With(TEXT("DT_Civs"), Game.Rules), { TEXT("DT_Civs"), TEXT("the row struct is") });
		{
			// A table whose columns the schema has not declared yet has no asset yet.
			UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
			Table->RowStruct = FHazardDef::StaticStruct();
			Expect(TEXT("a loaded table whose columns the schema has not declared"), With(TEXT("DT_Hazards"), Table), { TEXT("DT_Hazards"), TEXT("declares no columns") });
		}

		// Row references.
		Expect(TEXT("a row reference that names no row"), WithRules([](FCosmicRulesDef& Row) { Row.PlayerArchetypeRow = FName(TEXT("arch_nobody")); }),
			{ TEXT("DT_Rules.rules.PlayerArchetypeRow"), TEXT("DT_Civs has no row arch_nobody") });
		Expect(TEXT("a required row reference left empty"), WithRules([](FCosmicRulesDef& Row) { Row.PlayerArchetypeRow = NAME_None; }),
			{ TEXT("PlayerArchetypeRow"), TEXT("a row of DT_Civs is required") });
		Expect(TEXT("a row name used as a map key that names no row"), WithCiv([](FCivArchetypeDef& Row) { Row.InitialRelations.Add(FName(TEXT("arch_nobody")), 0.5f); }),
			{ TEXT("InitialRelations[arch_nobody]"), TEXT("has no row") });

		// Tags.
		Expect(TEXT("a single tag under the wrong root"), WithCiv([Plague](FCivArchetypeDef& Row) { Row.AIProfile = Plague; }), { TEXT("AIProfile"), TEXT("not under AI.Profile.") });
		Expect(TEXT("a tag of a container under the wrong root"), WithCiv([Plague](FCivArchetypeDef& Row) { Row.Traits.AddTag(Plague); }), { TEXT("Traits"), TEXT("not under Trait.") });
		Expect(TEXT("a tag of a list under the wrong root"), WithCiv([Plague](FCivArchetypeDef& Row) { Row.DedicationBanks.Add(Plague); }), { TEXT("DedicationBanks[0]"), TEXT("not under Faith.Bank.") });
		Expect(TEXT("a tag used as a map key under the wrong root"),
			WithCiv([](FCivArchetypeDef& Row) { Row.HazardSpawnMultipliers.Add(FGameplayTag::RequestGameplayTag(FName(TEXT("Law.Remnant"))), 1.f); }),
			{ TEXT("HazardSpawnMultipliers[Law.Remnant]"), TEXT("not under Hazard.") });
		Expect(TEXT("a required tag left empty"), WithCiv([](FCivArchetypeDef& Row) { Row.RevelationMode = FGameplayTag(); }), { TEXT("RevelationMode"), TEXT("a tag is required") });
		Expect(TEXT("an empty tag in a list"), WithCiv([](FCivArchetypeDef& Row) { Row.DedicationBanks.Add(FGameplayTag()); }), { TEXT("DedicationBanks[0]"), TEXT("a tag is required") });
		TestTrue(TEXT("An optional tag left empty is not a problem"), WithCiv([](FCivArchetypeDef& Row) { Row.AIProfile = FGameplayTag(); Row.HomeStarClass = FGameplayTag(); }).IsEmpty());
#if WITH_EDITORONLY_DATA
		Expect(TEXT("a parent tag that only its children imply"),
			WithCiv([](FCivArchetypeDef& Row) { Row.Traits.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Trait.Arch")))); }), { TEXT("Traits"), TEXT("Trait.Arch is not declared") });
#endif

		// Numbers.
		Expect(TEXT("a number above its range"), WithRules([](FCosmicRulesDef& Row) { Row.FallenTranscendentCount = 4; }), { TEXT("FallenTranscendentCount"), TEXT("is above 3") });
		Expect(TEXT("a number below its range"), WithRules([](FCosmicRulesDef& Row) { Row.EventSlotTicks = 0; }), { TEXT("EventSlotTicks"), TEXT("is below 1") });
		Expect(TEXT("a number that is not finite"), WithRules([](FCosmicRulesDef& Row) { Row.PressureAdjacencyFalloff = std::numeric_limits<float>::quiet_NaN(); }),
			{ TEXT("PressureAdjacencyFalloff"), TEXT("not a finite number") });
		Expect(TEXT("a number that is not finite in a column without a range"), WithRules([](FCosmicRulesDef& Row) { Row.ReverenceDecay = std::numeric_limits<float>::infinity(); }),
			{ TEXT("ReverenceDecay"), TEXT("not a finite number") });
		Expect(TEXT("a map value below its range"), WithCiv([Plague](FCivArchetypeDef& Row) { Row.HazardSpawnMultipliers.Add(Plague, -1.f); }),
			{ TEXT("HazardSpawnMultipliers[Hazard.Plague]"), TEXT("is below 0") });
		Expect(TEXT("a map value above its range"), WithCiv([](FCivArchetypeDef& Row) { Row.InitialRelations.Add(CosmicRows::ArchDefault, 2.f); }),
			{ TEXT("InitialRelations[arch_default]"), TEXT("is above 1") });
		Expect(TEXT("an array of numbers of the wrong length"), WithRules([](FCosmicRulesDef& Row) { Row.PressureBySourceTier.Pop(); }),
			{ TEXT("PressureBySourceTier"), TEXT("exactly 5 entries, found 4") });
		Expect(TEXT("an entry of an array of numbers out of range"), WithRules([](FCosmicRulesDef& Row) { Row.PressureBySourceTier[4] = -2.f; }),
			{ TEXT("PressureBySourceTier[4]"), TEXT("is below 0") });
		Expect(TEXT("an array of numbers that does not rise"), WithRules([](FCosmicRulesDef& Row) { Row.PopulationBandThresholds[1] = Row.PopulationBandThresholds[0]; }),
			{ TEXT("PopulationBandThresholds"), TEXT("do not rise") });
		Expect(TEXT("a map key that is no word of its list"), WithRules([](FCosmicRulesDef& Row) { Row.LureAcceptByPolicy.Add(FName(TEXT("Greedy")), 0.5f); }),
			{ TEXT("LureAcceptByPolicy[Greedy]"), TEXT("none of the words of policy") });

		// Counts of texts.
		const FText Word = FText::AsCultureInvariant(TEXT("word"));
		Expect(TEXT("an array of texts with a count that is none of the allowed"), WithCiv([&Word](FCivArchetypeDef& Row) { Row.AdjectiveBank = { Word, Word }; }),
			{ TEXT("AdjectiveBank"), TEXT("2 entries is none of the allowed counts") });
		Expect(TEXT("a bank of names that is neither empty nor long enough"), WithCiv([&Word](FCivArchetypeDef& Row) { Row.PersonNameBank = { Word, Word, Word }; }),
			{ TEXT("PersonNameBank"), TEXT("none or 10 entries and more, found 3") });
		Expect(TEXT("a log row with fewer variants than three"), WithLog([](FLogTemplateDef& Row) { Row.Templates.SetNum(2); }, CosmicRows::LogBreakthrough[0]),
			{ TEXT("log_breakthrough_physics.Templates"), TEXT("3 entries or more, found 2") });
		Expect(TEXT("a single-variant log row with no variant at all"), WithLog([](FLogTemplateDef& Row) { Row.Templates.Reset(); }, CosmicRows::LogFirstLife),
			{ TEXT("log_first_life.Templates"), TEXT("1 entries or more, found 0") });
		TestTrue(TEXT("A single-variant log row with one variant is not a problem, and neither is three for a row that needs three"),
			WithLog([](FLogTemplateDef& Row) { Row.Templates.SetNum(1); }, CosmicRows::LogFirstLife).IsEmpty()
			&& WithLog([](FLogTemplateDef& Row) { Row.Templates.SetNum(3); }, CosmicRows::LogBreakthrough[0]).IsEmpty());
	}

	Sim->NewGalaxy(42, 5, 3);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

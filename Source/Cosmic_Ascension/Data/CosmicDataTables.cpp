// Cosmic Ascension - Data layer.

#include "Data/CosmicDataTables.h"

#include "Cosmic_Ascension.h"
#include "Data/CosmicRowNames.h"
#include "UObject/NameTypes.h"

namespace
{
	UDataTable* LoadTable(const TCHAR* Path, const UScriptStruct* RowStruct)
	{
		UDataTable* Table = LoadObject<UDataTable>(nullptr, Path, {}, LOAD_NoWarn);
		if (!Table)
		{
			UE_LOG(LogCosmic, Warning, TEXT("Data table %s is missing. Run Tools/make_data_tables.py."), Path);
			return nullptr;
		}
		if (Table->GetRowStruct() != RowStruct)
		{
			UE_LOG(LogCosmic, Error, TEXT("Data table %s has row struct %s, expected %s."), Path, *GetNameSafe(Table->GetRowStruct()), *RowStruct->GetName());
			return nullptr;
		}
		return Table;
	}

	template <typename RowType>
	const RowType* FindRowQuiet(const UDataTable* Table, FName RowName)
	{
		return Table ? Table->FindRow<RowType>(RowName, TEXT("FCosmicDataTables"), false) : nullptr;
	}
}

void FCosmicDataTables::LoadFromFixedPaths()
{
	Rules = LoadTable(TEXT("/Game/CA/Data/DT_Rules.DT_Rules"), FCosmicRulesDef::StaticStruct());
	Civs = LoadTable(TEXT("/Game/CA/Data/DT_Civs.DT_Civs"), FCivArchetypeDef::StaticStruct());
	LogTemplates = LoadTable(TEXT("/Game/CA/Data/DT_LogTemplates.DT_LogTemplates"), FLogTemplateDef::StaticStruct());
	Galaxies = LoadTable(TEXT("/Game/CA/Data/DT_Galaxies.DT_Galaxies"), FGalaxyDef::StaticStruct());
	Events = LoadTable(TEXT("/Game/CA/Data/DT_Events.DT_Events"), FCosmicEventDef::StaticStruct());
	Transcendents = LoadTable(TEXT("/Game/CA/Data/DT_Transcendents.DT_Transcendents"), FTranscendentDef::StaticStruct());
	Hazards = LoadTable(TEXT("/Game/CA/Data/DT_Hazards.DT_Hazards"), FHazardDef::StaticStruct());
}

const FCosmicRulesDef* FCosmicDataTables::FindRules() const
{
	return FindRowQuiet<FCosmicRulesDef>(Rules, CosmicRows::Rules);
}

const FCivArchetypeDef* FCosmicDataTables::FindCiv(FName RowName) const
{
	return FindRowQuiet<FCivArchetypeDef>(Civs, RowName);
}

const FLogTemplateDef* FCosmicDataTables::FindLogTemplate(FName RowName) const
{
	return FindRowQuiet<FLogTemplateDef>(LogTemplates, RowName);
}

const FGalaxyDef* FCosmicDataTables::FindGalaxy(FName RowName) const
{
	return FindRowQuiet<FGalaxyDef>(Galaxies, RowName);
}

const FCosmicEventDef* FCosmicDataTables::FindEvent(FName RowName) const
{
	return FindRowQuiet<FCosmicEventDef>(Events, RowName);
}

const FTranscendentDef* FCosmicDataTables::FindTranscendent(FName RowName) const
{
	return FindRowQuiet<FTranscendentDef>(Transcendents, RowName);
}

const FHazardDef* FCosmicDataTables::FindHazard(FName RowName) const
{
	return FindRowQuiet<FHazardDef>(Hazards, RowName);
}

const FHazardDef* FCosmicDataTables::FindHazardByTag(FGameplayTag Tag, FName* OutRowName) const
{
	if (OutRowName)
	{
		*OutRowName = NAME_None;
	}
	if (!Tag.IsValid())
	{
		return nullptr;
	}
	for (const FName RowName : GetHazardRowNames())
	{
		const FHazardDef* Row = FindHazard(RowName);
		if (Row && Row->HazardTag == Tag)
		{
			if (OutRowName)
			{
				*OutRowName = RowName;
			}
			return Row;
		}
	}
	return nullptr;
}

TArray<FName> FCosmicDataTables::GetEventRowNames() const
{
	TArray<FName> Names = Events ? Events->GetRowNames() : TArray<FName>();
	Names.Sort(FNameLexicalLess());
	return Names;
}

TArray<FName> FCosmicDataTables::GetTranscendentRowNames() const
{
	TArray<FName> Names = Transcendents ? Transcendents->GetRowNames() : TArray<FName>();
	Names.Sort(FNameLexicalLess());
	return Names;
}

TArray<FName> FCosmicDataTables::GetHazardRowNames() const
{
	TArray<FName> Names = Hazards ? Hazards->GetRowNames() : TArray<FName>();
	Names.Sort(FNameLexicalLess());
	return Names;
}

TArray<FName> FCosmicDataTables::GetCivRowNames() const
{
	TArray<FName> Names = Civs ? Civs->GetRowNames() : TArray<FName>();
	Names.Sort(FNameLexicalLess());
	return Names;
}

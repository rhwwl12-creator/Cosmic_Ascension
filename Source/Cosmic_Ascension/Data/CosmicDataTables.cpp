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

	// B6b [B-6br] (the G1 time gate): whether Table's keys, in RowMap order, are Keys element by element - case and number
	// compared (FName::IsEqual with ENameCase::CaseSensitive), so that a row renamed in place to another case or number
	// is not taken for the old one. GetRowNames returns exactly this sequence (TMap::GetKeys walks the same pairs).
	bool SameKeySequence(const UDataTable& Table, const TArray<FName>& Keys)
	{
		const TMap<FName, uint8*>& RowMap = Table.GetRowMap();
		if (RowMap.Num() != Keys.Num())
		{
			return false;
		}
		int32 At = 0;
		for (const TPair<FName, uint8*>& Row : RowMap)
		{
			if (!Row.Key.IsEqual(Keys[At], ENameCase::CaseSensitive, true))
			{
				return false;
			}
			++At;
		}
		return true;
	}

	// The row names of Table in lexical order through Memo (FCosmicSortedRowNames). While the keys are the remembered ones,
	// GetRowNames would return the remembered array itself (the same FNames, display and number), and sorting the same
	// array with the same predicate gives the remembered order - the answer of the code before [B-6br]. Otherwise it is
	// sorted again exactly as before (GetRowNames, then Sort with FNameLexicalLess) and remembered. A null table answers
	// an empty array and leaves the memo alone.
	TArray<FName> SortedRowNamesOf(const UDataTable* Table, FCosmicSortedRowNames& Memo)
	{
		if (!Table)
		{
			return TArray<FName>();
		}
		if (!SameKeySequence(*Table, Memo.Keys))
		{
			Memo.Keys = Table->GetRowNames();
			Memo.Sorted = Memo.Keys;
			Memo.Sorted.Sort(FNameLexicalLess());
		}
		return Memo.Sorted;
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
	Megaprojects = LoadTable(TEXT("/Game/CA/Data/DT_Megaprojects.DT_Megaprojects"), FMegaprojectDef::StaticStruct());
	Feats = LoadTable(TEXT("/Game/CA/Data/DT_Feats.DT_Feats"), FAscensionFeatDef::StaticStruct());
	Encounters = LoadTable(TEXT("/Game/CA/Data/DT_Encounters.DT_Encounters"), FEncounterDef::StaticStruct());
	Edicts = LoadTable(TEXT("/Game/CA/Data/DT_Edicts.DT_Edicts"), FEdictDef::StaticStruct());
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

const FMegaprojectDef* FCosmicDataTables::FindMegaproject(FName RowName) const
{
	return FindRowQuiet<FMegaprojectDef>(Megaprojects, RowName);
}

const FAscensionFeatDef* FCosmicDataTables::FindFeat(FName RowName) const
{
	return FindRowQuiet<FAscensionFeatDef>(Feats, RowName);
}

const FEncounterDef* FCosmicDataTables::FindEncounter(FName RowName) const
{
	return FindRowQuiet<FEncounterDef>(Encounters, RowName);
}

const FEdictDef* FCosmicDataTables::FindEdict(FName RowName) const
{
	return FindRowQuiet<FEdictDef>(Edicts, RowName);
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
	return SortedRowNamesOf(Events, EventNamesMemo);
}

TArray<FName> FCosmicDataTables::GetTranscendentRowNames() const
{
	return SortedRowNamesOf(Transcendents, TranscendentNamesMemo);
}

TArray<FName> FCosmicDataTables::GetHazardRowNames() const
{
	return SortedRowNamesOf(Hazards, HazardNamesMemo);
}

TArray<FName> FCosmicDataTables::GetFeatRowNames() const
{
	return SortedRowNamesOf(Feats, FeatNamesMemo);
}

TArray<FName> FCosmicDataTables::GetEncounterRowNames() const
{
	return SortedRowNamesOf(Encounters, EncounterNamesMemo);
}

bool FCosmicDataTables::IsHazardAnswer(FName RowName) const
{
	if (RowName.IsNone() || !Hazards)
	{
		return false;
	}
	// B6b [B-6br]: the keys straight from the row map, without the array GetRowNames builds on every call (a TSet and a
	// copy): the same keys in the same order through the same FindHazard, and whether some row lists the name does not
	// depend on the order anyway. Read through a const pointer: the const GetRowMap, as FindRowInternal reads it.
	const UDataTable* HazardTable = Hazards;
	for (const TPair<FName, uint8*>& Row : HazardTable->GetRowMap())
	{
		const FHazardDef* Def = FindHazard(Row.Key);
		if (Def && Def->RulerResolutions.Contains(RowName))
		{
			return true;
		}
	}
	return false;
}

TArray<FName> FCosmicDataTables::GetEdictRowNames() const
{
	return SortedRowNamesOf(Edicts, EdictNamesMemo);
}

TArray<FName> FCosmicDataTables::GetGalaxyRowNames() const
{
	return SortedRowNamesOf(Galaxies, GalaxyNamesMemo);
}

TArray<FName> FCosmicDataTables::GetCivRowNames() const
{
	return SortedRowNamesOf(Civs, CivNamesMemo);
}

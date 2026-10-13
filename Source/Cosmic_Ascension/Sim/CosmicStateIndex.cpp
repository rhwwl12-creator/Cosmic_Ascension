// Cosmic Ascension - Sim layer.

#include "Sim/CosmicStateIndex.h"

#include "Cosmic_Ascension.h"

void FCosmicStateIndex::Rebuild(const FGalaxyState& State)
{
	CivByID.Reset();
	CivByID.Reserve(State.Civs.Num());
	for (int32 Index = 0; Index < State.Civs.Num(); ++Index)
	{
		CivByID.Add(State.Civs[Index].CivID, Index);
	}

	SystemByID.Reset();
	SystemByID.Reserve(State.Systems.Num());
	for (int32 Index = 0; Index < State.Systems.Num(); ++Index)
	{
		SystemByID.Add(State.Systems[Index].SystemID, Index);
	}
}

int32 FCosmicStateIndex::FindCiv(const FGalaxyState& State, FName CivID) const
{
	const int32* Found = CivID.IsNone() ? nullptr : CivByID.Find(CivID);
	if (!Found)
	{
		return INDEX_NONE;
	}
	if (!State.Civs.IsValidIndex(*Found) || State.Civs[*Found].CivID != CivID)
	{
		UE_LOG(LogCosmic, Error, TEXT("State index is stale for civ %s: rebuild it after changing the number of civs"), *CivID.ToString());
		return INDEX_NONE;
	}
	return *Found;
}

int32 FCosmicStateIndex::FindSystem(const FGalaxyState& State, FName SystemID) const
{
	const int32* Found = SystemID.IsNone() ? nullptr : SystemByID.Find(SystemID);
	if (!Found)
	{
		return INDEX_NONE;
	}
	if (!State.Systems.IsValidIndex(*Found) || State.Systems[*Found].SystemID != SystemID)
	{
		UE_LOG(LogCosmic, Error, TEXT("State index is stale for system %s: rebuild it after changing the number of systems"), *SystemID.ToString());
		return INDEX_NONE;
	}
	return *Found;
}

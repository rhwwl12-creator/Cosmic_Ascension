// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicAutoPause.h"

#include "Core/CosmicTypes.h"
#include "Sim/CosmicStateIndex.h"

bool CosmicAutoPause::StopsFor(const FCosmicLogEntry& Entry)
{
	return !Entry.bUntold && Entry.Severity >= 2;
}

TMap<FName, FGameplayTag> CosmicAutoPause::OwnHazardsOf(const FGalaxyState& State, const FCosmicStateIndex& Index)
{
	TMap<FName, FGameplayTag> Hazards;
	if (State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler || State.Player.RuledCivID.IsNone())
	{
		return Hazards;
	}
	const int32 CivIndex = Index.FindCiv(State, State.Player.RuledCivID);
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return Hazards;
	}
	const FCivilizationState& Civ = State.Civs[CivIndex];
	for (const FName SystemID : Civ.OwnedSystems)
	{
		const int32 SystemIndex = Index.FindSystem(State, SystemID);
		if (State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == Civ.CivID && State.Systems[SystemIndex].ActiveHazard.IsValid())
		{
			Hazards.Add(SystemID, State.Systems[SystemIndex].ActiveHazard);
		}
	}
	return Hazards;
}

bool CosmicAutoPause::HasNewHazard(const TMap<FName, FGameplayTag>& Before, const TMap<FName, FGameplayTag>& Now)
{
	for (const TPair<FName, FGameplayTag>& Standing : Now)
	{
		const FGameplayTag* Was = Before.Find(Standing.Key);
		if (!Was || *Was != Standing.Value)
		{
			return true;
		}
	}
	return false;
}

FName CosmicAutoPause::FirstNewHazardSystem(const TMap<FName, FGameplayTag>& Before, const TMap<FName, FGameplayTag>& Now, const FGalaxyState& State)
{
	for (const FStarSystemState& System : State.Systems)
	{
		const FGameplayTag* Standing = Now.Find(System.SystemID);
		const FGameplayTag* Was = Before.Find(System.SystemID);
		if (Standing && (!Was || *Was != *Standing))
		{
			return System.SystemID;
		}
	}
	return NAME_None;
}

const TCHAR* CosmicAutoPause::WhyKeyOf(ECosmicAutoPause Reason)
{
	switch (Reason)
	{
	case ECosmicAutoPause::MythicLine:
		return TEXT("AutoPause.Why.Mythic");
	case ECosmicAutoPause::OwnHazard:
		return TEXT("AutoPause.Why.Hazard");
	case ECosmicAutoPause::Card:
		return TEXT("AutoPause.Why.Card");
	case ECosmicAutoPause::Encounter:
		return TEXT("AutoPause.Why.Encounter");
	case ECosmicAutoPause::RunEnd:
		return TEXT("Chronicle.Why");
	case ECosmicAutoPause::Continued:
		return TEXT("Save.Why.Continued");
	case ECosmicAutoPause::ContinuedFromBackup:
		return TEXT("Save.Why.Backup");
	case ECosmicAutoPause::SaveUnreadable:
		return TEXT("Save.Why.Unreadable");
	case ECosmicAutoPause::SaveFailed:
		return TEXT("Save.Why.Failed");
	case ECosmicAutoPause::None:
		break;
	}
	return nullptr;
}

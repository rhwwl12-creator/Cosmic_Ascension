// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicAutoPause.h"

#include "Core/CosmicTypes.h"
#include "Data/CosmicDataTables.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicWanderer.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicAutoPauseImpl
{
	// The tick the hazard standing on System expires at: the crisis stage counts down first and expires at 0, so N ticks left
	// expire at State.Tick + N (1 at least).
	int64 ExpiryTickOf(const FGalaxyState& State, const FStarSystemState& System)
	{
		return State.Tick + FMath::Max(1, System.HazardTicksRemaining);
	}
}

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

int32 CosmicAutoPause::LethalPlaceOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
{
	const int32 Place = CosmicWanderer::PlaceIndexOf(State, Index);
	if (!State.Systems.IsValidIndex(Place))
	{
		return INDEX_NONE;
	}
	// Where it stands (a journey with no countdown is none - the move stage only empties it): from now on.
	const int32 Underway = State.Player.TravelTicksRemaining;
	if (!CosmicWanderer::IsTravelling(State) || Underway <= 0)
	{
		return LethalStayOf(State, Tables, Rules, Place, State.Tick) != ECosmicLethalStay::None ? Place : INDEX_NONE;
	}
	if (!CosmicHazards::KillsAVesselOf(Tables, State.Systems[Place].ActiveHazard))
	{
		return INDEX_NONE;
	}
	// Under way the vessel stands where it left until the move stage of the arrival's tick, after that tick's crisis stage
	// ([B-5r]): an expiry later than that finds it elsewhere. Whom the kill reaches is the shared rule (B5 header section 4:
	// the screen does not copy it), at the tick the hazard expires.
	const FStarSystemState& System = State.Systems[Place];
	const int64 ExpiryTick = CosmicAutoPauseImpl::ExpiryTickOf(State, System);
	const bool bReaches = ExpiryTick <= State.Tick + Underway
		&& CosmicHazards::VesselAtRiskOf(State.Player, State.Calling, Rules, ExpiryTick, System.SystemID, NAME_None);
	return bReaches ? Place : INDEX_NONE;
}

ECosmicLethalStay CosmicAutoPause::LethalStayOf(const FGalaxyState& State, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 SystemIndex,
	int64 StandsFromTick)
{
	if (!State.Systems.IsValidIndex(SystemIndex) || !CosmicHazards::KillsAVesselOf(Tables, State.Systems[SystemIndex].ActiveHazard))
	{
		return ECosmicLethalStay::None;
	}
	const FStarSystemState& System = State.Systems[SystemIndex];
	// The crisis stage comes before the move stage and the tenth stage in a tick: an expiry on StandsFromTick or before finds
	// the vessel not yet there.
	const int64 ExpiryTick = CosmicAutoPauseImpl::ExpiryTickOf(State, System);
	if (ExpiryTick <= StandsFromTick)
	{
		return ECosmicLethalStay::None;
	}
	// Whom its kill reaches is the shared rule, asked of a copy of the vessel walking in that system at the expiry's tick (the
	// birth protection among it); the interregnum reaches nobody.
	FPlayerEntityState Placed = State.Player;
	Placed.Role = EPlayerRole::Wanderer;
	Placed.CurrentSystemID = System.SystemID;
	if (!CosmicHazards::VesselAtRiskOf(Placed, State.Calling, Rules, ExpiryTick, System.SystemID, NAME_None))
	{
		return ECosmicLethalStay::None;
	}
	// Set out again at once, the vessel still stands there until the move stage one lane's time later (section 2.8).
	return ExpiryTick <= StandsFromTick + CosmicWanderer::LaneTicksOf(Rules) ? ECosmicLethalStay::NoWayOut : ECosmicLethalStay::CanLeave;
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
	case ECosmicAutoPause::Death:
		return TEXT("AutoPause.Why.Death");
	case ECosmicAutoPause::LethalHazard:
		return TEXT("AutoPause.Why.Lethal");
	case ECosmicAutoPause::Succession:
		return TEXT("Succession.Stopped");
	case ECosmicAutoPause::ReignEnd:
		return TEXT("Reign.Stopped");
	case ECosmicAutoPause::None:
		break;
	}
	return nullptr;
}

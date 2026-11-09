// Cosmic Ascension - Sim layer.

#include "Sim/CosmicWanderer.h"

#include "Core/CosmicIDs.h"
#include "Core/CosmicKnowing.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"

bool CosmicWanderer::IsTravelling(const FGalaxyState& State)
{
	return !State.Player.TravelTargetSystemID.IsNone();
}

bool CosmicWanderer::IsEmbodiedWanderer(const FGalaxyState& State)
{
	return State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Wanderer;
}

int32 CosmicWanderer::PlaceIndexOf(const FGalaxyState& State, const FCosmicStateIndex& Index)
{
	if (!IsEmbodiedWanderer(State))
	{
		return INDEX_NONE;
	}
	const int32 SystemIndex = Index.FindSystem(State, State.Player.CurrentSystemID);
	return State.Systems.IsValidIndex(SystemIndex) ? SystemIndex : INDEX_NONE;
}

int32 CosmicWanderer::StandingIndexOf(const FGalaxyState& State, const FCosmicStateIndex& Index)
{
	return IsTravelling(State) ? INDEX_NONE : PlaceIndexOf(State, Index);
}

int32 CosmicWanderer::LaneTicksOf(const FCosmicRulesDef& Rules)
{
	return Rules.LaneTravelTicks > 1 ? Rules.LaneTravelTicks : 1;
}

CosmicWanderer::ETravelBlock CosmicWanderer::TravelBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, FName TargetSystemID, bool bCardWaiting)
{
	if (!IsEmbodiedWanderer(State))
	{
		return ETravelBlock::NotWanderer;
	}
	if (IsTravelling(State))
	{
		return ETravelBlock::Travelling;
	}
	if (bCardWaiting)
	{
		return ETravelBlock::CardWaiting;
	}
	if (CosmicState::IsSanctuary(TargetSystemID))
	{
		return ETravelBlock::Sanctuary;
	}
	const int32 Here = Index.FindSystem(State, State.Player.CurrentSystemID);
	const int32 There = TargetSystemID.IsNone() ? INDEX_NONE : Index.FindSystem(State, TargetSystemID);
	if (!State.Systems.IsValidIndex(Here) || !State.Systems.IsValidIndex(There) || There == Here
		|| !State.Systems[Here].Lanes.Contains(TargetSystemID)
		|| State.Systems[There].GalaxyRow != State.Systems[Here].GalaxyRow
		|| CosmicHazards::IsDeadNode(State.Systems[There]))
	{
		return ETravelBlock::NotNeighbour;
	}
	return ETravelBlock::None;
}

bool CosmicWanderer::IsReturn(const FGalaxyState& State, FName CivID)
{
	return !CivID.IsNone() && CivID == State.Player.FormerCivID;
}

CosmicWanderer::ECrownBlock CosmicWanderer::CrownBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, FName CivID,
	bool bMet, bool bCardWaiting)
{
	if (!IsEmbodiedWanderer(State))
	{
		return ECrownBlock::NotWanderer;
	}
	if (IsTravelling(State))
	{
		return ECrownBlock::Travelling;
	}
	if (bCardWaiting)
	{
		return ECrownBlock::CardWaiting;
	}
	const int32 Here = Index.FindSystem(State, State.Player.CurrentSystemID);
	const int32 CivIndex = CivID.IsNone() ? INDEX_NONE : Index.FindCiv(State, CivID);
	if (!State.Systems.IsValidIndex(Here) || !State.Civs.IsValidIndex(CivIndex) || State.Systems[Here].OwnerCivID != CivID)
	{
		return ECrownBlock::NotStanding;
	}
	if (!CosmicState::IsSettled(State.Systems[Here]))
	{
		return ECrownBlock::NotSettled;
	}
	if (!bMet)
	{
		return ECrownBlock::Unmet;
	}
	const FCivilizationState& Civ = State.Civs[CivIndex];
	// B6b (decision 41): no throne of a people far above the vessel's own rung - a coronation and a return alike.
	if (CosmicKnowing::IsFarAbove(Civ.Tier, State.Calling.PersonalTier))
	{
		return ECrownBlock::FarAbove;
	}
	if (State.Tick < Civ.ThroneLockedUntilTick)
	{
		return ECrownBlock::ThroneLocked;
	}
	if (!IsReturn(State, CivID) && !(Civ.ReverenceToPlayer >= Rules.CoronationReverence) && !(Civ.Stability < Rules.CoronationStabilityBelow))
	{
		return ECrownBlock::Lacking;
	}
	return ECrownBlock::None;
}

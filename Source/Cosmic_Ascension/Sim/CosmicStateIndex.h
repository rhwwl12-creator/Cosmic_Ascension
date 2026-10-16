// Cosmic Ascension - Sim layer. Design: guideline.md section 5.7, 7.4, 9.4.
// Unsaved lookup from ID to array index for FGalaxyState.Civs and FGalaxyState.Systems, and the settled-system
// predicate. Derived from the state alone: never saved, never hashed, only looked up and never iterated (section 9.2).
// RULE (section 6.1): files under Sim/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicRowNames.h"

struct COSMIC_ASCENSION_API FCosmicStateIndex
{
	// Whoever changes the number of elements of Civs or Systems rebuilds it: NewGalaxy, LoadState, the cleanup and the
	// arrivals of the crossing stage (section 7.4). A test that edits those arrays by hand rebuilds it too.
	void Rebuild(const FGalaxyState& State);

	// Index of the element that carries the ID; INDEX_NONE when the state has none (NAME_None included). The stored
	// index is checked against the state: when the element there carries another ID the index is stale, which is
	// reported with one error line and answered INDEX_NONE, never with a wrong element.
	int32 FindCiv(const FGalaxyState& State, FName CivID) const;
	int32 FindSystem(const FGalaxyState& State, FName SystemID) const;

private:
	TMap<FName, int32> CivByID;
	TMap<FName, int32> SystemByID;
};

namespace CosmicState
{
	// Settled = owned and not being settled (section 5.7). The one predicate for it. The event slots, the economy stage
	// (income and pressure), the hazard stage (the spawn draw, the placement of a hazard), the count of Law.Remnant, the
	// megaprojects and the feats, and settlement (who may settle, from where) use it now. Still to take it up: succession
	// candidates (Phase 4b).
	inline bool IsSettled(const FStarSystemState& System)
	{
		return !System.OwnerCivID.IsNone() && System.ActiveMegaproject != CosmicRows::ProjectSettle;
	}

	/**
	 * Lanes counted from one system (section 5.9): entry i is the fewest lanes from system FromSystemIndex to system i,
	 * 0 for the system itself, and INDEX_NONE for a system that cannot be reached, that lies farther than MaxLanes, or
	 * that lies in another GalaxyRow (a bridge between galaxies is not a lane, and the walk does not follow a lane into
	 * another galaxy even if the state held one). A lane to a SystemID the state does not hold is skipped. An index
	 * out of range gives INDEX_NONE for every system. The array has one entry per element of State.Systems.
	 * The one walk for every rule that counts lanes over more than one step: the place of a starting core now, the
	 * sight of the calling, settlement and succession later. World state only, no draw, one breadth-first pass.
	 */
	COSMIC_ASCENSION_API TArray<int32> LaneDistancesFrom(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 FromSystemIndex, int32 MaxLanes = MAX_int32);

	/**
	 * The AI.Profile tag one number in [0, 1) picks (section 3.1, 5.10): the four profiles in a fixed order, evenly, until
	 * section 16 #36 (1) weighs the draw by ethics. NewGalaxy, the overthrow of a people and the regency after a
	 * deposition use it, each with a number drawn whether or not it is used.
	 */
	COSMIC_ASCENSION_API FGameplayTag PickAIProfile(float Roll);
}

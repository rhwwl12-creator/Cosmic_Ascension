// Cosmic Ascension - Sim layer. Design: guideline.md section 5.7, 7.4, 9.4.
// Unsaved lookup from ID to array index for FGalaxyState.Civs and FGalaxyState.Systems, and the settled-system
// predicate. Derived from the state alone: never saved, never hashed, only looked up and never iterated (section 9.2).
// RULE (section 6.1): files under Sim/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
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
	// Settled = owned and not being settled (section 5.7). The one predicate for it. The event slots use it now. Still to
	// take it up: the count of Law.Remnant and the hazard spawn draw (hazard step), settlement adjacency and the income
	// loop of TickEconomy, which is not filtered yet (settlement step, with project_settle), succession candidates
	// (Phase 4b).
	inline bool IsSettled(const FStarSystemState& System)
	{
		return !System.OwnerCivID.IsNone() && System.ActiveMegaproject != CosmicRows::ProjectSettle;
	}
}

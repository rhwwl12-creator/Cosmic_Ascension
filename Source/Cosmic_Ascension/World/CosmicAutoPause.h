// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 2.1, 8.2 (the automatic pause, MVP item 1).
// When the screens stop a running simulation by themselves: pure functions of a log entry and of the state. The
// player controller holds the viewer's choice and does the stopping (ACosmicPlayerController::RequestAutoPause); nothing
// here is saved or hashed, and nothing writes the state.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Core/GalaxyState.h"
#include "GameplayTagContainer.h"

struct FCosmicStateIndex;

/** Why the screens stopped the simulation by themselves (none: the player set the speed, or nothing stopped it). */
enum class ECosmicAutoPause : uint8
{
	None,
	// A mythic line was told (Severity 2).
	MythicLine,
	// A hazard newly stands on a system of the people the vessel rules.
	OwnHazard,
	// A card arrived (the card widget of MVP item 2 asks for it).
	Card,
};

namespace CosmicAutoPause
{
	/** Whether a told line stops a running game: it is mythic (Severity 2, section 8.2). An entry marked bUntold - never
	 * broadcast - never does; nor does a far line or a meeting (Severity 1). */
	COSMIC_ASCENSION_API bool StopsFor(const FCosmicLogEntry& Entry);

	/** The hazards standing now on the systems of the people an embodied ruling vessel rules, by SystemID (a system
	 * without a hazard is not listed); empty for a wanderer, in the interregnum and for a people the state does not hold. An
	 * entry of OwnedSystems the people does not own counts for nothing. */
	COSMIC_ASCENSION_API TMap<FName, FGameplayTag> OwnHazardsOf(const FGalaxyState& State, const FCosmicStateIndex& Index);

	/** Whether Now holds a hazard Before did not hold on the same system: a new one, or another tag (a hazard that ended
	 * and a system lost stop nothing). */
	COSMIC_ASCENSION_API bool HasNewHazard(const TMap<FName, FGameplayTag>& Before, const TMap<FName, FGameplayTag>& Now);

	/** The ST_CA key of the line a pause shows for Reason ("stopped - ..."); null for None. */
	COSMIC_ASCENSION_API const TCHAR* WhyKeyOf(ECosmicAutoPause Reason);
}

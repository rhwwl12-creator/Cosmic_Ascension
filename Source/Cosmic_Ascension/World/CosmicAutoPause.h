// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 2.1, 8.2 (the automatic pause, MVP item 1).
// When the screens stop a running simulation by themselves: pure functions of a log entry and of the state. The
// player controller holds the viewer's choice and does the stopping (ACosmicPlayerController::RequestAutoPause); nothing
// here is saved or hashed, and nothing writes the state.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Core/GalaxyState.h"
#include "GameplayTagContainer.h"

struct FCosmicDataTables;
struct FCosmicRulesDef;
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
	// The run is over (MVP item 4) - not an automatic pause: set whatever the viewer's choice; every speed asked for afterwards
	// stays 0 (ACosmicPlayerController::ApplySpeed).
	RunEnd,
	// MVP item 5 - not automatic pauses either: set whatever the viewer's choice; cleared by the next speed asked for.
	// The game opened on a continued run.
	Continued,
	// ... on its backup (the last save did not read).
	ContinuedFromBackup,
	// The newest run's save did not read: a new run.
	SaveUnreadable,
	// A write of the game's saving failed (once per game instance).
	SaveFailed,
	// A wanderer's card arrived (B1): the card's pause, in the words of a card that is no petition.
	Encounter,
	// A life closed (B5) - not an automatic pause: set whatever the viewer's choice (the death pages follow, [B-5c]).
	Death,
	// A hazard whose expiry kills a vessel there newly stands where an embodied wanderer counts as standing (B5 header section
	// 2.8, 4 - LethalPlaceOf): the wanderer's own crisis pause (a ruler's systems' is OwnHazard).
	LethalHazard,
	// B5 [B-5c] - not automatic pauses either, set whatever the viewer's choice (ACosmicPlayerController::NextCallingPage):
	// the inauguration closed - the camera went to the new vessel's system and the time waits for the player.
	Succession,
	// A reign ended within one life (an abdication, a deposition): its page opened over the stopped map.
	ReignEnd,
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

	/** The system HasNewHazard counts that comes first in State.Systems (MVP item 6: the crisis the camera goes to - one
	 * system, in an order no TMap decides); None when there is none. */
	COSMIC_ASCENSION_API FName FirstNewHazardSystem(const TMap<FName, FGameplayTag>& Before, const TMap<FName, FGameplayTag>& Now, const FGalaxyState& State);

	/** B5 (header section 2.8, 4): the system an embodied wanderer counts as standing in (CosmicWanderer::PlaceIndexOf - while it
	 * travels, the one it left: a vessel that set out stands there until it arrives) when the hazard standing there kills a
	 * vessel at its expiry (CosmicHazards::KillsAVesselOf) and that kill would reach this vessel then - the rule Apply records
	 * a KillPlayer by (CosmicHazards::VesselAtRiskOf, the birth protection among it), asked at the expiry's tick: State.Tick +
	 * HazardTicksRemaining (1 at least - the crisis stage counts down at the next tick first and expires at 0). INDEX_NONE
	 * otherwise - a ruler, the interregnum, no such hazard, a hazard that expires while the vessel is shielded. The system
	 * panel, the top bar, the travel button's tooltip and the pause (LethalHazard) read it: one answer, the rule not copied. */
	COSMIC_ASCENSION_API int32 LethalPlaceOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules);

	/** The ST_CA key of the line a pause shows for Reason ("stopped - ..."); null for None. */
	COSMIC_ASCENSION_API const TCHAR* WhyKeyOf(ECosmicAutoPause Reason);
}

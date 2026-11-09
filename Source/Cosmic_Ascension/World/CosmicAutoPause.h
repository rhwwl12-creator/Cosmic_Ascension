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

/** B5 [B-5r] (header section 2.8, 4): what a lethal hazard means for a vessel that comes to stand in its system
 * (CosmicAutoPause::LethalStayOf). Not saved. */
enum class ECosmicLethalStay : uint8
{
	// No warning: no hazard there kills a vessel at its expiry, it expires before the vessel stands there, or its kill would
	// not reach the vessel then (the birth protection).
	None,
	// It expires more than one lane's time after the vessel comes to stand there: setting out again at once, the vessel has
	// arrived elsewhere before it.
	CanLeave,
	// It expires within one lane's time after: a vessel that sets out again still stands there at the expiry (section 2.8 -
	// it stands where it left until it arrives), so leaving is no way out.
	NoWayOut,
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
	 * HazardTicksRemaining (1 at least - the crisis stage counts down at the next tick first and expires at 0). Standing, it is
	 * where LethalStayOf from State.Tick is not None. Under way ([B-5r], the [B-5d2] review's play-1) only while that tick
	 * comes at or before the arrival's, State.Tick + Player.TravelTicksRemaining (the crisis stage comes before the move
	 * stage): a hazard that expires after the arrival is left behind. INDEX_NONE otherwise - a ruler, the interregnum, no such
	 * hazard, a hazard that expires while the vessel is shielded or after it arrives elsewhere. The system panel, the top bar,
	 * the travel button's tooltip, the card window and the pause (LethalHazard) read it: one answer, the rule not copied. */
	COSMIC_ASCENSION_API int32 LethalPlaceOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules);

	/**
	 * B5 [B-5r] (header section 2.8, 4; the [B-5d2] review's play-1, risk-1, goals-1): for SystemIndex, where the vessel would
	 * stand from the end of StandsFromTick on - a [Travel]'s arrival (State.Tick + CosmicWanderer::LaneTicksOf; under way
	 * State.Tick + Player.TravelTicksRemaining), the walk of an abdication (State.Tick + 1: the tenth stage of the next
	 * tick), or where it stands now (State.Tick) -, whether the hazard standing there kills a vessel at its expiry
	 * (CosmicHazards::KillsAVesselOf), would still stand when the vessel has come (its expiry after StandsFromTick: the
	 * crisis stage comes before the move stage and the tenth stage in a tick) and would reach the vessel then
	 * (CosmicHazards::VesselAtRiskOf at the expiry's tick, asked of a copy of the vessel walking in that system - the birth
	 * protection among the rule); and if so whether a vessel that sets out again at once would be gone before it (CanLeave:
	 * the expiry more than one lane's time after StandsFromTick) or not (NoWayOut). The expiry's tick is LethalPlaceOf's,
	 * State.Tick + HazardTicksRemaining (1 at least). Time only: whether a living lane leads away is not asked (a vessel
	 * with none - B1 decision 21 - is told so by the top bar's Wander.Bar.Stuck beside the warning). Reads the world only
	 * (no knowing, no draw, no write); None for an index the state does not hold and in the interregnum. The [Travel]
	 * buttons, the top bar under way and [Abdicate] read it.
	 */
	COSMIC_ASCENSION_API ECosmicLethalStay LethalStayOf(const FGalaxyState& State, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 SystemIndex,
		int64 StandsFromTick);

	/** B6b (header section 4, decisions 4-5): whether the expiry of the hazard standing on SystemIndex would end the life of
	 * the ruler of its owner people - CosmicHazards::KillsTheRulerOf its tag, and CosmicHazards::VesselAtRiskOf the systemless
	 * subject of that people at the tick it expires (the birth protection decided there - the rule is not copied). The system
	 * panel's Panel.RulerLethal reads it; no automatic pause does (no new pause). */
	COSMIC_ASCENSION_API bool RulerDiesAtExpiryOf(const FGalaxyState& State, const FCosmicRulesDef& Rules, int32 SystemIndex);

	/** The ST_CA key of the line a pause shows for Reason ("stopped - ..."); null for None. */
	COSMIC_ASCENSION_API const TCHAR* WhyKeyOf(ECosmicAutoPause Reason);
}

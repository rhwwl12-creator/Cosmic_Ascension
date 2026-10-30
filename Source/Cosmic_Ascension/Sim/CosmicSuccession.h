// Cosmic Ascension - Sim layer (Core Logic). Design: docs/60_DECISIONS/headers/B5_death_succession_two_books.md section 2.2,
// 2.4, 2.5 (decisions 3, 8, 23).
// The single home of succession's world rules (rule 13): where a new vessel may be born after a death, how long the leap
// of the interregnum is, and which candidate a number picks. Pure functions of the state, the tables and the rules - no
// draw, no state of their own, no knowing read (rule 2): the tenth stage (UPlayerRoleSubsystem::Die and Succeed) draws the
// numbers and hands them in.
// RULE (section 6.1): files under Sim/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"

struct FCosmicDataTables;
struct FCosmicRulesDef;
struct FCosmicStateIndex;
struct FGalaxyState;

namespace CosmicSuccession
{
	/** The candidates of a succession: indices into State.Systems in array order, and how far the filters were relaxed to
	 * find them (0 none; 1 the hazard filter; 2 the hazard and the sphere filters). */
	struct FCosmicSuccessionCandidates
	{
		TArray<int32> Systems;
		int32 Relaxed = 0;
	};

	/**
	 * Where a new vessel may be born after the death of the vessel at DeathSystemIndex (header section 2.4): the settled
	 * systems (CosmicState::IsSettled) of the death place's GalaxyRow - never the death place itself (to be born again
	 * where one died is the wheel's mark) and never a system whose standing hazard kills a vessel at its expiry
	 * (CosmicHazards::KillsAVesselOf). These two filters never relax. The sanctuary has no owner and falls out by itself.
	 * Two more filters relax in turn while they leave nothing: a hazard with fewer than DT_Rules.SuccessionMinHazardTicks
	 * left goes first, then the sphere of an unsealed fallen transcendent (Realm.Abyssal) holding the system. Empty when
	 * nothing is left after both: no succession (the silence). Every candidate weighs the same (decision 23-ga: the
	 * weighting knobs are reserved and not read). A DeathSystemIndex the state does not hold gives no candidate. Reads the
	 * world only.
	 */
	COSMIC_ASCENSION_API FCosmicSuccessionCandidates CandidatesOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
		const FCosmicRulesDef& Rules, int32 DeathSystemIndex);

	/** The candidate a number in [0, 1) picks among Count of the same weight: min(floor(Number x Count), Count - 1); a number
	 * below 0 or not a number picks the first, one at 1 or above (an infinity too) the last; INDEX_NONE when Count is not
	 * above 0. */
	COSMIC_ASCENSION_API int32 PickOf(float Number, int32 Count);

	/**
	 * The leap a death draws (decision 3), from one number in [0, 1): Lo = max(0, LeapTicksMin), Hi = max(Lo, LeapTicksMax),
	 * L = Lo + min(floor(Number x (Hi - Lo + 1)), Hi - Lo), then at most max(0, LeapTotalCap - LeapTicksTotal) - so a run
	 * whose leaps have reached the cap succeeds on the tick of the death. Whatever the knobs hold, the death draws that one
	 * number (rule 3: FRandomStream::RandRange draws nothing for an empty range). A number below 0 or not a number reads as 0,
	 * one at 1 or above as the longest.
	 */
	COSMIC_ASCENSION_API int64 LeapOf(const FCosmicRulesDef& Rules, int64 LeapTicksTotal, float Number);
}

// Cosmic Ascension - Sim layer (Core Logic). Design: crisis_and_transcendents.md section 4.4, headers/A4_provisional_end.md (MVP item 4).
// The single home of the conditions that end a run: pure functions of the state - no draw, no write, never Knowing. The four
// endings (B5, B6) come here too.

#pragma once

#include "CoreMinimal.h"
#include "Core/CosmicTypes.h"

struct FCosmicRulesDef;
struct FGalaxyState;

/** Why the MVP's provisional end holds. Not a UENUM: never saved, hashed or a data column (the append-only rule does not bind it); it goes with the four endings. */
enum class ECosmicProvisionalEnd : uint8
{
	None,
	Rung,   // no longer produced: the third rung does not end a run (B6a [B-6a3a], decision 2-ga); it goes with the enum in [B-6a3c]
	Clock,  // State.Tick reached DT_Rules.ProvisionalEndTick
};

namespace CosmicEndings
{
	/** None when Calling.Lives is empty; otherwise Clock when Rules.ProvisionalEndTick > 0 and State.Tick >= it; otherwise None. The third rung no longer
	 * ends a run (B6a [B-6a3a], decision 2-ga: a run that climbs goes on to the clock), so Rung is never answered. Reads State.Tick, whether Calling.Lives is
	 * empty and the knob only - never Civs, Knowing, the log or the chronicle. Both inputs only grow - Lives is append-only and the tick only rises - so once
	 * it holds it holds for the run and a load answers alike.
	 * Read by FollowEnd, the commandlet and the save layer's tick-boundary check (UCosmicSaveSubsystem::MarkOf), read only; no stage, command or run reads it. */
	COSMIC_ASCENSION_API ECosmicProvisionalEnd ProvisionalEndOf(const FGalaxyState& State, const FCosmicRulesDef& Rules);

	/** The cause as the commandlet and the CosmicEnd line print it: None or Clock (and Rung, which ProvisionalEndOf no longer answers - [B-6a3c] removes it). */
	COSMIC_ASCENSION_API const TCHAR* NameOf(ECosmicProvisionalEnd End);
}

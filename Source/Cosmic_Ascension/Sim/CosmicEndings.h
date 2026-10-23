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
	Rung,   // a life's PeakTier reached EndRung (it rises only while the vessel rules; a climb in the tick of a deposition counts: feats are judged before the tenth stage)
	Clock,  // State.Tick reached DT_Rules.ProvisionalEndTick
};

namespace CosmicEndings
{
	/** The rung that ends an MVP run (2026-10-18: "T3 or tick 6,000"). A C++ constant, not a knob. */
	inline constexpr EAscensionTier EndRung = EAscensionTier::Tier3_Causal;

	/** None when Calling.Lives is empty; otherwise Rung when any Lives[i].PeakTier >= EndRung (the MVP has one life); otherwise Clock when Rules.ProvisionalEndTick > 0
	 * and State.Tick >= it; otherwise None (Rung wins a tie). Reads State.Tick, Calling.Lives and the knob only - never Civs, Knowing, the log or the chronicle. Both
	 * inputs only grow - Lives is append-only and a PeakTier only rises, through succession too - so once it holds it holds for the run and a load answers alike.
	 * Read by FollowEnd and the commandlet; no stage, command or run reads it. */
	COSMIC_ASCENSION_API ECosmicProvisionalEnd ProvisionalEndOf(const FGalaxyState& State, const FCosmicRulesDef& Rules);

	/** The cause as the commandlet and the CosmicEnd line print it: None, Rung or Clock. */
	COSMIC_ASCENSION_API const TCHAR* NameOf(ECosmicProvisionalEnd End);
}

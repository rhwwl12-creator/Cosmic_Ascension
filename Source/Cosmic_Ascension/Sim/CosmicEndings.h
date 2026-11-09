// Cosmic Ascension - Sim layer (Core Logic). Design: crisis_and_transcendents.md section 4.4, headers/A4_provisional_end.md (MVP item 4),
// headers/B6a_endings_and_the_run_past_the_third_rung.md section 2.2. The single home of the conditions that end a run: pure functions of the
// state - no draw, no write, never Knowing. EndingOf is the ending; the tick writes it once into State.Ending (after the tenth stage) and every
// other reader - the controller's latch, the chronicle page, the save layer, the policies, RunLeap - reads that record.

#pragma once

#include "CoreMinimal.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "GameplayTagContainer.h"

struct FCosmicRulesDef;

namespace CosmicEndings
{
	/**
	 * Whether an end stands now (B6a header section 2.2), whatever State.Ending already holds - an empty Kind when none does. An empty Calling.Lives
	 * answers none; otherwise the first of these that holds (decision 5 - the vessel's own before another people's, any before the clock):
	 *   1 Ending.RuledPeople - some life's PeakTier is the fifth rung (now only the rung the ruled people reached under it: no code raises the personal
	 *     rung yet). CivID: that life's RuledCivIDs read from the end, the first people standing on the fifth rung in Civs; none when no such people stands.
	 *   2 Ending.Vessel - the calling is embodied and its PersonalTier is the fifth rung (unreached in B6a: nothing raises the personal rung).
	 *   3 Ending.OtherPeople - some people stands on the fifth rung; CivID: the first such people in Civs order.
	 *   4 Ending.NoCandidate - the silence (CosmicSuccession::IsSilent).
	 *   5 Ending.Clock - Rules.ProvisionalEndTick > 0 and State.Tick >= it.
	 * Tick is State.Tick. bNoVessel is the interregnum - except for a fifth rung reached in the tick the vessel died (the last life's DiedTick is
	 * State.Tick): the rung was reached at the end of the AI stage, while the vessel stood, before the tenth stage carried the death out.
	 * Reads State.Tick, Calling (Lives, LifeState, PendingLeapTicks, PersonalTier), Civs (CivID, Tier) and the knob only - never Knowing, the log, the
	 * chronicle or the telling stream (rule 2). No draw, no write. Called by the one place that writes the record (UGalaxySimulationSubsystem::SimulateTick,
	 * after the tenth stage, while State.Ending is empty), the tests, the commandlet's end check and its race line (Tests/CosmicRaceMeasure.h's
	 * FifthRungEndOf - B6b [B-6b3b], measurement); the screens, policies and the save layer read the record.
	 */
	COSMIC_ASCENSION_API FCosmicEndingRecord EndingOf(const FGalaxyState& State, const FCosmicRulesDef& Rules);

	/** An ending's kind as the commandlet and the CosmicEnd line print it: the tag's leaf word - RuledPeople, Vessel, OtherPeople, NoCandidate, Clock;
	 * None for the empty tag and for any other tag. */
	COSMIC_ASCENSION_API const TCHAR* NameOf(const FGameplayTag& Kind);
}

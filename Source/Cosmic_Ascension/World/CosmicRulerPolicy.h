// Cosmic Ascension - World layer: the ruler policy a script plays between ticks (MVP item 9).

#pragma once

#include "CoreMinimal.h"

struct FCosmicStateIndex;
struct FEncounterDef;
struct FGalaxyState;
class UGalaxySimulationSubsystem;
class UPlayerRoleSubsystem;

// The ruler policy of the balance commandlet (MVP item 3: -Answer, -Build; MVP item 7: -Focus) as one function the
// commandlet and the hash run (World/CosmicHashRun) share, so the cooked build plays the inputs the editor's baseline
// played (docs/60_DECISIONS/headers/A9_win64_packaging.md section 2.2). Moved out of the commandlet unchanged.
struct FCosmicRulerPolicy
{
	bool bAnswer = false;
	bool bBuild = false;
	bool bFocus = false;
};

// What the policy did so far in a run.
struct FCosmicRulerPolicyCounts
{
	// The ruler's cards seen to come: at the tick boundary, and after each answer (a wanderer's card is not counted - B1).
	int32 CardsCome = 0;
	// Accepted answers, withdrawn cards not counted (ChoiceOf's round reads it).
	int32 Answers = 0;
	int32 Started = 0;
	int32 FocusChanges = 0;
	// A provisional end has been read at a boundary: no focus input from then on (the game takes no input after it).
	bool bEnded = false;
};

namespace CosmicRulerPolicy
{
	// The answer of the -Answer policy: the first choice the stockpile can pay, from choice (answers so far) on, round
	// (so that every choice of a card is taken in turn over a run), among the choices whose success does not record the
	// end of the reign (B1: the script never leaves the throne of its own will - the throne card's last choice). INDEX_NONE
	// when none can be paid.
	int32 ChoiceOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FEncounterDef& Row, int32 AnswersSoFar);

	// One tick boundary, right after RunHeadless(1), before anything else reads the tick: reads the provisional end into
	// bEnded first (as the presentation reads it), counts a card that came, then answers while a card is pending (at
	// most eight), presses the project key until it is refused (at most eight) and, while the run has not ended, puts the
	// research focus on the field of the first research condition of the ways up that is not met (the even focus when
	// none is). CardBefore is the pending card before the tick. Its lines carry this tick.
	void Act(const FCosmicRulerPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, FName CardBefore,
		FCosmicRulerPolicyCounts& Counts);
}

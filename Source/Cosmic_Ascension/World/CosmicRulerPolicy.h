// Cosmic Ascension - World layer: the ruler policy a script plays between ticks (MVP item 9).

#pragma once

#include "CoreMinimal.h"

struct FCosmicStateIndex;
struct FEncounterDef;
struct FGalaxyState;
class UGalaxySimulationSubsystem;
class UPlayerRoleSubsystem;

// The ruler policy of the balance commandlet (MVP item 3: -Answer, -Build; MVP item 7: -Focus; B2: -Edict, -EdictRow) as one
// function the commandlet and the hash run (World/CosmicHashRun) share, so the cooked build plays the inputs the editor's
// baseline played (docs/60_DECISIONS/headers/A9_win64_packaging.md section 2.2). Moved out of the commandlet unchanged.
// Every place that makes one writes all five values (an aggregate initialisation would leave a missing one false).
struct FCosmicRulerPolicy
{
	bool bAnswer = false;
	bool bBuild = false;
	bool bFocus = false;
	// B2 (docs/60_DECISIONS/headers/B2_edicts_and_research.md section 5): the edicts, and the row the script keeps standing
	// when no hazard asks for a slot (None: no such row).
	bool bEdict = false;
	FName EdictRow;
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
	// Accepted edict commands (B2): enactments and revocations.
	int32 EdictsEnacted = 0;
	int32 EdictsRevoked = 0;
	// The vigil step 3 enacted against a coming rebellion; step 4 revokes it once the danger passed (None: none stands).
	FName VigilEnacted;
	// A provisional end has been read at a boundary: no focus or edict input from then on (the game takes no input after
	// it).
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
	// none is), then the edicts (ActEdicts). CardBefore is the pending card before the tick. Its lines carry this tick.
	void Act(const FCosmicRulerPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, FName CardBefore,
		FCosmicRulerPolicyCounts& Counts);

	// The edicts of one boundary (B2 header section 5), for the people of an embodied ruler, through the commands
	// (UPlayerRoleSubsystem::EnactEdict / RevokeEdict - each accepted one counts): (1) below Stability 0.5 it revokes the
	// edicts with a Stability cost; (2) for each hazard standing on a settled system of the people, in Systems order, that
	// no held edict answers, it enacts the first DT_Edicts row of the hazard's RulerResolutions; (3) while the people's
	// LowStabilityTicks is above 0 and no rebellion (a hazard of a Capital row) stands on its capital, it enacts the vigil -
	// the first edict row, lexical, with a countdown bonus for a Capital row's tag; (4) it revokes a held answer edict
	// with no hazard to answer, and the vigil of step 3 once Stability is 0.6 or more with no rebellion standing - never
	// EdictRow; (5) with a free slot it enacts EdictRow - a row with a Stability cost only at Stability 0.6 or more. A
	// full set of slots is made room for in (2) and (3) by revoking first a held edict that answers no standing hazard
	// (EdictRow only when no other one does) - only when the row's one reason is Slots (CosmicEdicts::EnactBlockOf) and
	// the stockpile can pay its EnactCost, so nothing is given up for an enactment that would be refused. One boundary
	// never revokes a row it enacted, nor enacts one it revoked.
	// Nothing once Counts.bEnded is set.
	void ActEdicts(const FCosmicRulerPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, FCosmicRulerPolicyCounts& Counts);

	// The decisions a run took: answers, projects started, focus changes and edict commands (the commandlet's ruler line).
	int32 DecisionsOf(const FCosmicRulerPolicyCounts& Counts);
}

// Cosmic Ascension - World layer: the ruler policy a script plays between ticks (MVP item 9).

#pragma once

#include "CoreMinimal.h"

struct FCosmicRulesDef;
struct FCosmicStateIndex;
struct FEncounterDef;
struct FGalaxyState;
class UGalaxySimulationSubsystem;
class UPlayerRoleSubsystem;

// The ruler policy of the balance commandlet (MVP item 3: -Answer, -Build; MVP item 7: -Focus; B2: -Edict, -EdictRow; B3:
// -Diplomacy) as one function the commandlet and the hash run (World/CosmicHashRun) share, so the cooked build plays the
// inputs the editor's baseline played (docs/60_DECISIONS/headers/A9_win64_packaging.md section 2.2). Moved out of the
// commandlet unchanged. Every place that makes one writes all six values (an aggregate initialisation would leave a
// missing one false or 0).
struct FCosmicRulerPolicy
{
	bool bAnswer = false;
	bool bBuild = false;
	bool bFocus = false;
	// B2 (docs/60_DECISIONS/headers/B2_edicts_and_research.md section 5): the edicts, and the row the script keeps standing
	// when no hazard asks for a slot (None: no such row).
	bool bEdict = false;
	FName EdictRow;
	// B3 (docs/60_DECISIONS/headers/B3_diplomacy_and_conflict.md section 5): 0 no diplomacy, 1 the peace policy, 2 the attack
	// policy (measurement only - no baseline plays it).
	int32 Diplomacy = 0;
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
	// Accepted diplomacy commands (B3): stances set, treaties signed, fleet targets changed (a guard set or withdrawn, a
	// siege laid).
	int32 StanceChanges = 0;
	int32 TreatiesSigned = 0;
	int32 FleetTargetChanges = 0;
	// What the treaty step met (the commandlet's diplomacy line): proposals to bordering peoples the command took, and the
	// (people, treaty) pairs whose first look - from the second rung, the command's own gate - was a refusal; for met
	// peoples that do not border, the first look of each pair, would-take or refused (the policy proposes to them never).
	int32 TreatiesProposedBorder = 0;
	int32 TreatiesRefusedBorder = 0;
	int32 TreatiesProposedOther = 0;
	int32 TreatiesRefusedOther = 0;
	// (the ruled people, the other people, the treaty) - a reign over another people looks again; a return to the same
	// people does not (one look a pair over the run).
	TSet<TTuple<FName, FName, FName>> TreatyLooks;
	// A provisional end has been read at a boundary: no focus or edict input from then on (the game takes no input after
	// it).
	bool bEnded = false;
};

namespace CosmicRulerPolicy
{
	// The answer of the -Answer policy: the first choice the stockpile can pay, from choice (answers so far) on, round
	// (so that every choice of a card is taken in turn over a run), among the choices whose success does not record the
	// end of the reign (B1: the script never leaves the throne of its own will - the throne card's last choice). INDEX_NONE
	// when none can be paid. With Rules, a choice whose stance the gate would refuse is passed by as an unpayable one
	// (CosmicPetitions::StanceBlockOfChoice on the first contact card's people - B3). With a diplomacy policy (Diplomacy
	// not 0), the first contact card (RowName CosmicRows::PetitionFirstContact) takes its first payable choice the gate
	// takes whose success turns the people friendly ('reach out'), whatever the answers so far - the round would mix both
	// choices over the runs otherwise.
	int32 ChoiceOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FEncounterDef& Row, int32 AnswersSoFar, FName RowName = NAME_None,
		int32 Diplomacy = 0, const FCosmicRulesDef* Rules = nullptr);

	// One tick boundary, right after RunHeadless(1), before anything else reads the tick: reads the provisional end into
	// bEnded first (as the presentation reads it), counts a card that came, then answers while a card is pending (at
	// most eight), presses the project key until it is refused (at most eight - the attack policy, Diplomacy 2, stops
	// before a settlement that would fill its last realm slot: it leaves that slot to a conquest, for a people at its
	// cap besieges nothing) and, while the run has not ended, puts the
	// research focus on the field of the first research condition of the ways up that is not met (the even focus when
	// none is), then the edicts (ActEdicts), then the diplomacy (ActDiplomacy). CardBefore is the pending card before the
	// tick. Its lines carry this tick.
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

	// The diplomacy of one boundary (B3 header section 5), for the people of an embodied ruler, through the commands (each
	// accepted one counts), one command a boundary at most, in this order - the guard first (a siege threatens a system
	// now; a stance or a treaty can wait a boundary): (1) the guard - a siege on one of the people's systems (the first in
	// Systems order) is guarded against while the people besieges no one; a guard over a system no longer besieged is
	// withdrawn; (2) friendship - while no first contact card waits (the card decides the first stance, and a friendship
	// set meanwhile would take the slot its [Reach out] needs), the first met people in Civs order that borders the
	// people, toward which it holds no stance, when the command's gate
	// takes friendly and the people's Influence income covers the friendly stances' upkeep with this one and its edicts'
	// upkeep (a friendship the stockpile cannot keep would lapse and be set again every boundary); (3) treaties - to the
	// met bordering peoples in Civs order, non-aggression then trade, the first the gate takes (the first look of every
	// pair is counted - see the counts; the attack policy offers none - a non-aggression would close the siege gate on
	// the neighbours it is there to besiege, and a trade would make each siege open with a betrayal); (4) the attack
	// policy only (2), while its fleet guards no system of its own a siege still stands on: a hostile people's first
	// system the siege gate takes is besieged; else a hostile stance with no such system is turned neutral (the AI's rule: no hostility without
	// a siege); else the first met bordering people with such a system is turned hostile (one boundary each). The peace
	// policy (1) never turns hostile. Nothing for 0, nor once Counts.bEnded is set.
	void ActDiplomacy(const FCosmicRulerPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, FCosmicRulerPolicyCounts& Counts);

	// The decisions a run took: answers, projects started, focus changes, edict commands and diplomacy commands (stances,
	// treaties, fleet targets - the commandlet's ruler line).
	int32 DecisionsOf(const FCosmicRulerPolicyCounts& Counts);
}

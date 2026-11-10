// Cosmic Ascension - World layer: the ruler policy a script plays between ticks (MVP item 9).

#pragma once

#include "CoreMinimal.h"

struct FCosmicCondition;
struct FCosmicDataTables;
struct FCosmicRulesDef;
struct FCosmicStateIndex;
struct FEncounterChoice;
struct FEncounterDef;
struct FGalaxyState;
class UGalaxySimulationSubsystem;
class UPlayerRoleSubsystem;

// How the script answers the pressure cards (B4, docs/60_DECISIONS/headers/B4_transcendent_voice_and_pressure_cards.md
// section 2.9 - measurement only, no baseline plays another than Round): the commandlet's -PlayerPolicy=, in the words of
// docs/40 (Tribute is refused - B4 has no tribute). A pressure card is known by its effects (a choice that enacts an edict
// or buys a hazard time), never by its row name. Not saved.
enum class ECosmicPressurePolicy : uint8
{
	// The round of ChoiceOf, as for every other card.
	Round,
	// Always the watch: the choice with no cost, no check, no effect and no next card.
	Cautious,
	// Discern (the choice that opens the next card), then the fleet's time; else the watch.
	Reckless,
	// The vigil (an EnactEdict of VigilRowOf), else discern; on the next card the other edict; else the watch.
	Faithful,
};

// The ruler policy of the balance commandlet (MVP item 3: -Answer, -Build; MVP item 7: -Focus; B2: -Edict, -EdictRow; B3:
// -Diplomacy; B4: -PlayerPolicy, -Guard, -LeaveNearSphere) as one function the commandlet and the hash run
// (World/CosmicHashRun) share, so the cooked build plays the inputs the editor's baseline played
// (docs/60_DECISIONS/headers/A9_win64_packaging.md section 2.2). Moved out of the commandlet unchanged. Every place that
// makes one writes all ten values (an aggregate initialisation would leave a missing one false or 0); B7x's Crossroads, the
// eleventh, is written by the commandlet alone - every other place leaves it at 0 (the policy of every baseline).
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
	// B4 (header section 2.9 - measurement only): how the pressure cards are answered; the guard (1: the fleet guards the
	// system of ActDiplomacy's step (1) - besieged, else beside a higher sphere, else inside one, else the capital - also
	// with no diplomacy and after the end; 0: none); under a pressure policy (not Round), the awakenings and plagues on the
	// ruled people's settled systems a claimant could reach are left unanswered (projects and answer edicts - a
	// supernova's warning is answered), so that their expiry may bring a claim (header section 5, the first measurement 3).
	ECosmicPressurePolicy Pressure = ECosmicPressurePolicy::Round;
	int32 Guard = 0;
	bool bLeaveNearSphere = false;
	// B5 (header section 5.1 - measurement only): the run's end (the record State.Ending) is not counted - the policy goes on
	// after it.
	bool bPastEnd = false;
	// B7x ([B-7x4m] - docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md section 5, decision 19): how a
	// crossroads card (CosmicRulerPolicy::IsCrossroadsCard) is answered - 0, every baseline: the no-value choice alone; k of 1
	// or more (-Crossroads=<k>, measurement only): the k-th choice the answer takes that moves a brick, else the no-value one.
	int32 Crossroads = 0;
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
	// The record of the run's end (State.Ending - B6a) has been read at a boundary: no focus or edict input from then on (the
	// game takes no input after it).
	bool bEnded = false;
	// B4: the edicts a pressure card's accepted answer enacted (header decision 9). While a claim stands on a settled system
	// of the ruled people, MakeRoomFor never revokes them; after, the usual rules (the vigil as the vigil of step (3)).
	TArray<FName> CardEdicts;
	// B4 (header section 5, the commandlet's pressure line - measurement only, the policy's play unchanged): the pressure
	// cards seen to come (at CardsCome's two places), before the end, the tick of the first (-1: none), as the claim card
	// and the measured card, and by the ruled people's rung (one bin per rung - B6b [B-6b3b]: the third bin was the third and
	// up before; the pressure and race lines read them); the answers taken on them by kind
	// (intercede, discern, watch, edict, resist) and before the end; the choices greyed when such a card came, by reason (the
	// stockpile's, the gate's - its kind and reason); the enactments a card's answer made, by row, and the card edicts the
	// script revoked; the least Insight a discernment was taken at (-1: none).
	int32 PressureCards = 0;
	int32 PressureCardsByEnd = 0;
	int64 FirstPressureCardTick = -1;
	int32 ClaimCards = 0;
	int32 MeasuredCards = 0;
	int32 PressureCardsByTier[5] = { 0, 0, 0, 0, 0 };
	int32 PressureAnswers[5] = { 0, 0, 0, 0, 0 };
	int32 PressureAnswersByEnd = 0;
	TMap<FString, int32> PressureGreyed;
	TMap<FName, int32> CardEnactments;
	int32 CardEdictsRevoked = 0;
	float DiscernInsightMin = -1.f;
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
	// choices over the runs otherwise. With Rules and Tables, every gate of a choice is asked on the card's system
	// (CosmicPetitions::ChoiceGateOf - B4: the edicts and the fleet's time too; the answer refuses what it refuses).
	// With Tables and a pressure policy (B4), a pressure card - one with a choice that enacts an edict or buys a hazard time -
	// is answered by the policy's order of kinds (ECosmicPressurePolicy), the first choice of the first kind the answer
	// takes; the round when none is.
	// B7x ([B-7x4m] - B7x header section 5, decision 19): a crossroads card (IsCrossroadsCard), before everything above, takes
	// the no-value choice (IsNoValueChoice) the answer takes - a choice that moves a brick, and a choice with a value that moves
	// none, are passed by -, or INDEX_NONE when it has none (the card waits, as one the stockpile cannot pay). With Crossroads k
	// of 1 or more, the k-th choice the answer takes that moves a brick (MovesABrickOf) first, the end of the reign let through
	// on it (the kept law's abdication - the throne's guard stays for every other card).
	int32 ChoiceOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FEncounterDef& Row, int32 AnswersSoFar, FName RowName = NAME_None,
		int32 Diplomacy = 0, const FCosmicRulesDef* Rules = nullptr, const FCosmicDataTables* Tables = nullptr,
		ECosmicPressurePolicy Pressure = ECosmicPressurePolicy::Round, int32 Crossroads = 0);

	// B7x ([B-7x4m] - B7x header section 3.3 rules 4 and 5, section 5): the crossroads cards, read from the row and never by its
	// name, so that no row of a data without B7x's cards is one and both policies play as before, bit for bit. A choice moves
	// a brick when either outcome adds or removes a Brick.* tag (AddTrait, RemoveTrait - CosmicTags::IsBrick).
	bool MovesABrickOf(const FEncounterChoice& Choice);

	// The no-value choice: no cost, no check, no effect on either outcome and no next card (the watch of the pressure cards;
	// every card that moves a brick holds one - schema rule 4).
	bool IsNoValueChoice(const FEncounterChoice& Choice);

	// A crossroads card: a condition CivTimeVectorAtLeast (the schema's mark of the crossroads cards - rule 5) or a choice that
	// moves a brick (rule 4).
	bool IsCrossroadsCard(const FEncounterDef& Row);

	// Whether conditions ask a brick or a time vector - CivHasTrait or CivLacksTrait with a Brick.* tag, BorderingCivHasTrait,
	// CivTimeVectorAtLeast: the DT_Events rows of the overlap rules (the commandlet's bricks line, Data.Integrity's pin).
	bool AsksABrickOf(const TArray<FCosmicCondition>& Conditions);

	// The -PlayerPolicy word (docs/40): Round, Cautious, Reckless, Faithful, case-insensitive. False for any other word -
	// Tribute among them (B4 has no tribute): the commandlet runs nothing then.
	bool PressurePolicyOf(const FString& Word, ECosmicPressurePolicy& Out);

	// The word of a pressure policy (the commandlet's pressure line).
	const TCHAR* PressurePolicyName(ECosmicPressurePolicy Pressure);

	// Whether a claim (a hazard of a ClaimRule row) stands on a settled system of the people CivIndex.
	bool ClaimStandsOn(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 CivIndex);

	// The system the guard of -Guard=1 keeps for the people CivIndex when none of its systems is besieged (B4 header section
	// 2.9): among its settled systems in OwnedSystems order, the first one lane from the sphere of an unsealed transcendent
	// whose row stands above the people's Tier (where a claim stands), else the first inside one (where the fleet wears),
	// else the capital; NAME_None when the guard's gate refuses the capital (no fleet - the first rung).
	FName GuardTargetOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex);

	// The projects the script may start for the people CivIndex (CosmicMegaprojects::Choices, in its order) under Policy:
	// all of them for Round; under a pressure policy, without the answer of a claim (the relief - header section 2.8: the
	// theodicy's count is the claims a work did not decide) and, with bLeaveNearSphere, without the answer of an awakening or a
	// plague (a Draw row but the supernova's warning) on a settled system a claimant could reach.
	TArray<TPair<int32, FName>> ProjectChoicesOf(const FCosmicRulerPolicy& Policy, const FGalaxyState& State, const FCosmicStateIndex& Index,
		const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex);

	// One tick boundary, right after RunHeadless(1), before anything else reads the tick: reads the record of the end
	// (State.Ending) into bEnded first (as the presentation reads it), counts a card that came, then answers while a card is pending (at
	// most eight), presses the project key until it is refused (under a pressure policy - B4 - it starts the first of
	// ProjectChoicesOf instead, else a settlement of the first of CosmicMegaprojects::SettleTargets; at most eight - the
	// attack policy, Diplomacy 2, stops before a settlement that would fill its last realm slot: it leaves that slot to a
	// conquest, for a people at its cap besieges nothing) and, while the run has not ended, puts the
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
	// EdictRow; (4') B6b (header section 2.4): when the people stands on the fourth rung or above and the row judged there
	// asks for laws (CosmicEdicts::AsksForLawsOf), it enacts the galaxy laws it lacks in table order, each through the room
	// making for a law - a free slot, or the slot of an edict that is no law (a law is never given up for a law, nor the
	// vigil while step 3 would enact it again or Stability is below 0.5: the law takes the vigil's slot back once the
	// discontent is no longer counted and Stability is 0.5 or more, or takes the free slot step 4 leaves) - and not at all
	// while its slots are fewer than DT_Rules.GalaxyLawsToHold or an answer edict holds a slot; (5) with a free slot
	// it enacts EdictRow - a row with a Stability cost only at Stability 0.6 or more. A full set of slots is made room for
	// in (2) and (3) by revoking first a held edict that answers no standing hazard (EdictRow only when no other one does,
	// a galaxy law only when neither does) - only when the row's one reason is Slots (CosmicEdicts::EnactBlockOf) and the
	// stockpile can pay its EnactCost, so nothing is given up for an enactment that would be refused. Step (1) never
	// revokes a galaxy law. One boundary never revokes a row it enacted, nor enacts one it revoked.
	// B4 (header section 2.9, decision 9): under a pressure policy, step (2) leaves a hazard unanswered when its first edict
	// answer is one ProjectChoicesOf leaves out (a claim's; with bLeaveNearSphere, an awakening's or a plague's on a settled
	// system a claimant could reach). While a claim stands on a settled system of the people (ClaimStandsOn), the
	// room-making revocation and the vigil rule of step (4) pass by Counts.CardEdicts - and, under a pressure policy, the
	// vigil (VigilRowOf), whoever enacted it (the idle-answer rule of step (4) is not held back: no card enacts an answer
	// edict now); a vigil a card enacted goes in step (4), as the vigil of step (3) does, once the danger passed.
	// Nothing once Counts.bEnded is set.
	void ActEdicts(const FCosmicRulerPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, FCosmicRulerPolicyCounts& Counts);

	// The diplomacy of one boundary (B3 header section 5), for the people of an embodied ruler, through the commands (each
	// accepted one counts), one command a boundary at most, in this order - the guard first (a siege threatens a system
	// now; a stance or a treaty can wait a boundary): (1) the guard - a siege on one of the people's systems (the first in
	// Systems order) is guarded against while the people besieges no one; a guard over a system no longer besieged is
	// withdrawn - or, with Guard 1 (B4), the fleet is sent to the system GuardTargetOf keeps instead, guarding or not (a
	// fleet already there stays, and the steps below run on); (2) friendship - while no first contact card waits (the card
	// decides the first stance, and a friendship set meanwhile would take the slot its [Reach out] needs), the first met
	// people in Civs order that borders the people, toward which it holds no stance, when the command's gate takes
	// friendly and the people's Influence income covers the friendly stances' upkeep with this one and its edicts'
	// upkeep (a friendship the stockpile cannot keep would lapse and be set again every boundary); (3) treaties - to the
	// met bordering peoples in Civs order, non-aggression then trade, the first the gate takes (the first look of every
	// pair is counted - see the counts; the attack policy offers none - a non-aggression would close the siege gate on
	// the neighbours it is there to besiege, and a trade would make each siege open with a betrayal); (4) the attack
	// policy only (2), while its fleet guards no system of its own a siege still stands on: a hostile people's first
	// system the siege gate takes is besieged; else a hostile stance with no such system is turned neutral (the AI's rule: no hostility without
	// a siege); else the first met bordering people with such a system is turned hostile (one boundary each). The peace
	// policy (1) never turns hostile. Nothing for 0, nor once Counts.bEnded is set - except step (1) with Guard 1 (B4),
	// which runs with no diplomacy and after the end too; steps (2)-(4) still need a diplomacy policy and a run not ended.
	void ActDiplomacy(const FCosmicRulerPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, FCosmicRulerPolicyCounts& Counts);

	// The decisions a run took: answers, projects started, focus changes, edict commands and diplomacy commands (stances,
	// treaties, fleet targets - the commandlet's ruler line).
	int32 DecisionsOf(const FCosmicRulerPolicyCounts& Counts);

	// The vigil of ActEdicts step (3): the first DT_Edicts row, lexical, that adds to the countdown of a Capital row's
	// hazard (read from the data, not named); NAME_None when no row does. The commandlet's spirit line asks its gate too.
	FName VigilRowOf(const FCosmicDataTables& Tables);
}

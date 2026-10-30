// Cosmic Ascension - Player layer. Design: guideline.md section 3.3, 4.4, 5.9, 5.10, 7.5.
// Player/ is the command API used by UI; it may read state but must not include UUserWidget or AActor headers either.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/CosmicTypes.h"
#include "Events/CosmicPetitions.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicWanderer.h"
#include "PlayerRoleSubsystem.generated.h"

/**
 * Ruler / Wanderer role switching, travel, encounter flow, the ruler command API, and the death, deposition and
 * succession pipeline. Holds no player or life dependent member: all of it lives in FGalaxyState (section 6.1).
 * So far: Install, the deposition pipeline in its Phase 2 form with BecomeWanderer, the ruler's megaproject command
 * (settlement included), the answer to the waiting card (MVP item 2; the wanderer's cards, B1), the research focus (MVP
 * item 7), the wanderer's loop (B1): TravelTo, ScanCurrentSystem, BecomeRuler, Abdicate and the abdication pipeline, and
 * the death and the succession the tenth stage runs (B5: Die, Succeed). The return offer is not B5's (16 #146).
 */
UCLASS()
class COSMIC_ASCENSION_API UPlayerRoleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Starts a life on a new vessel (section 4.4, 7.5). The first life (NewGalaxy, Ruler) and every succession go through
	 * here. Resets the vessel, opens a FLifeRecord and, for a ruler, opens a reign in the three reign arrays.
	 * Writes the birth system into Knowing.KnownSystems and the birth civilization into Knowing.MetCivs, each once and
	 * without a line (section 8.1): a life knows where it was born. It never writes Knowing.FirstContactCivID: the
	 * people one is born into is not a contact. A life that starts as a wanderer (succession, B5 - Succeed) also knows the
	 * lane neighbours of its birth system in its galaxy row (NoteKnown, Lanes order, no line - section 4.4: the new eye).
	 * The card waiting and the first contact card waiting (Knowing.PendingContactCivID) are not the new life's: cleared.
	 * One draw from the sim stream per call, for the name of the vessel, made before the name bank is looked at. A life that is
	 * not the first never takes the previous life's name: the first entry of its bank whose source string (FTextInspector -
	 * no display culture) equals that name's is passed over (B5 header decision 8 - two archetypes may share a bank).
	 * Refused with an error line when called from a handler of a Core Logic delegate (a handler may only read the state),
	 * and for the sanctuary (CosmicState::IsSanctuary - no vessel stands there, section 4.5 (1), 16 #81): a refused call
	 * changes nothing and draws nothing.
	 */
	void Install(FName SystemID, FName CivID, EPlayerRole Role);

	/**
	 * The deposition pipeline (section 5.10), run by the role-effect stage for Effect.DeposeRuler, in its Phase 2 form:
	 * tells CosmicRows::LogFallenMonarch (Severity 2), draws the regent profile (one number from the sim stream,
	 * CosmicState::PickAIProfile), BecomeWanderer with Reign.Deposed, then every people's ReverenceToPlayer times
	 * DT_Rules.ReverenceDecay (no people crowns the fallen at once - a death sets it to 0, B5), for the people it
	 * ruled Stability = min(Stability, PostRebellionStability), and every people's throne locked until Tick +
	 * ThroneLockTicks (never earlier than it was - B1: no people crowns the vessel in that time). The vessel stays where it
	 * stands. The page of a reign's end is the presentation's (B5); the exile encounter is 16 #148. Does nothing unless
	 * the vessel rules; refused with an error line when called from a handler of a Core Logic delegate.
	 */
	void DeposeRuler();

	/**
	 * Ruler to wanderer (section 5.10): Role = Wanderer, FormerCivID = the ruled people, RuledCivID cleared, the people
	 * AI-run with RegentProfile, StatusTags from ReignEndReason (Reign.Abdicated: Status.Abdicated, Reign.Deposed:
	 * Status.FallenMonarch, Reign.Died: none), the open reign closed with ReignEndReason, Knowing.PendingContactCivID
	 * cleared, and the system the vessel stands in and its lane neighbours in its galaxy row known (NoteKnown, Lanes
	 * order, no line -
	 * section 4.4: the new eye of a wanderer; meeting their owners is the next sight stage's), and the journey fields
	 * emptied. With Reign.Abdicated the regent's record (notable hook 1, B1): the NameNotable rule through
	 * UCosmicEventSubsystem::Apply - one number of the sim stream for the name before its guards, Notable.Regent, source row
	 * CosmicRows::LogThroneVacated; Reign.Died takes the same hook (B5 - Die passes the death's source row). Does nothing unless
	 * the vessel rules; refused with an error line when called from a handler of a Core Logic delegate, and when the vessel
	 * stands in the sanctuary (a state no rule makes - Install refuses it). The release of the throne is ReleaseThrone; the new
	 * eye (KnowAround) is this function's own - a death does not look around.
	 */
	void BecomeWanderer(FGameplayTag RegentProfile, FGameplayTag ReignEndReason);

	/**
	 * The death pipeline (B5 header section 2.2 Die), run by the tenth stage for Effect.KillPlayer while the calling is
	 * Embodied: (a) the life closes - DiedTick, CauseOfDeath = PendingCause, CauseSourceRow = PendingSourceRow; (b) the death
	 * line, told before anything else changes (the deposition's form): the row of the cause - CosmicRows::LogDeathHazard for
	 * Death.Hazard, none for a cause with no row of its own (the measurement's Death.Age: no line, no tell number) - Log.Calling,
	 * Severity 2, no focus, {Player} the closed life's name (CosmicChronicle::VesselNameOf), {Hazard} the display name of the
	 * hazard row CauseSourceRow (empty when the table lacks it); one number of the tell stream, none of the sim stream;
	 * (c) a ruler leaves the throne - one number for the regent profile, ReleaseThrone with
	 * Reign.Died (its regent hook draws the name, source row PendingSourceRow), every throne locked (Tick + ThroneLockTicks,
	 * never earlier - B5 decision 4, old 16 #143); no new eye; (d) every people's ReverenceToPlayer = 0 (decision 23-na: the new vessel is a
	 * stranger to every people); (e) the waiting cards, the first contact waiting and the journey are cleared; (f) LifeState =
	 * Interregnum, one number for the leap (CosmicSuccession::LeapOf), LeapTicksTotal += it, PendingLeapTicks = it; (g) a leap
	 * of 0 succeeds at once. Numbers of the sim stream: a wanderer's death 1, a ruler's 3. Does nothing unless the calling is
	 * Embodied; inside a handler of a Core Logic delegate an error line.
	 */
	void Die();

	/**
	 * The succession (B5 header section 2.2 Succeed), run when the leap's countdown reaches 0 (or at once for a leap of 0):
	 * one number for the succession before the candidates are looked at; the candidates of CosmicSuccession::CandidatesOf for
	 * the system the dead vessel stood in; with none, one more number thrown away where the name's would be (two numbers
	 * always - rule 3) and the calling stays in the interregnum with no leap left (the silence - never drawn again). Else the
	 * calling's tier below the third goes back to the first (before Install reads it into PeakTier), its powers keep Way.*
	 * only (the rest is given again from the tier - no row gives one yet), and Install makes the candidate the number picks
	 * (CosmicSuccession::PickOf) the birth place of a wanderer of its owner (the name's number). Then the inauguration line
	 * (CosmicRows::LogInauguration - Log.Calling, Severity 2, {Player} the new life's name (CosmicChronicle::VesselNameOf),
	 * {Civ} and {System} its birth people and system, the line's focus; one number of the tell stream): the line that opens
	 * the called life's chapter (CosmicChronicle::OpensCalledChapter). When this tick's log already holds a fallen's first
	 * person about the birth system (UCosmicLogSubsystem::HasClaimLineThisTick), its second row in its place:
	 * CosmicRows::LogInaugurationUnnamed, {Player} and {Civ}, no {System}, the birth people the focus and no focus system
	 * (docs/70 section 8.3 rule 5) - the same one number. No line in the silence, nor when Install refused.
	 * Does nothing outside the interregnum; inside a handler of a Core Logic delegate an error line.
	 */
	void Succeed();

	/**
	 * The ruler's command (section 5.7, 7.5): starts RowName on SystemID for the people the vessel rules, through
	 * CosmicMegaprojects::Start - the same rules as the AI. Only while Embodied and Ruler, and only for a system of the
	 * ruled people - except project_settle, whose SystemID is the wild system to settle (CosmicMegaprojects::Settle, the
	 * rules of the AI's settlement; the system it starts to settle becomes known at once, through FollowUpKnowing).
	 * Between ticks: the project first progresses in the next tick's crisis stage. False
	 * and nothing changed otherwise; inside a handler of a Core Logic delegate an error line and nothing. No draw.
	 */
	bool StartMegaproject(FName SystemID, FName RowName);

	// The prototype's project key (Phase 3 replaces it with the panel): starts the first of CosmicMegaprojects::Choices
	// for the people the vessel rules (SinceTick = Calling.BornTick), or with none a settlement of the first of
	// CosmicMegaprojects::SettleTargets, under the rules of StartMegaproject. False when there is neither.
	bool StartNextMegaproject();

	/**
	 * The answer to the card that waits (section 5.9, 5.10; Knowing.PendingPetitionRow), between ticks: player input, of
	 * the same standing as a draw of the sim stream (section 9.2). A petition is answered by an embodied ruler, a wanderer's
	 * card (CosmicPetitions::IsWandererCard) by an embodied wanderer - the other role is refused without an error line.
	 * Refused - nothing drawn, nothing changed - while no card waits, and with an error line from a
	 * handler of a Core Logic delegate, for a card whose row DT_Encounters lacks, for a choice out of range, for a
	 * cost the ruled people's stockpile cannot pay in full and, while the card stands (CosmicPetitions::StillStands), for a
	 * choice its gate refuses (CosmicPetitions::ChoiceGateOf - a stance, an edict incl. one held, the fleet's time; B4 header
	 * section 2.4) - the card greys such a choice with the first reason. A wanderer pays no cost (the
	 * choice check of Data.Integrity keeps the costs of VesselSystem rows at 0), and its card's effects run on the system it stands in and that system's
	 * owner (none for a wild system - an effect on a people then does nothing).
	 * An answer taken draws one number 0..5 from the sim stream first, before the choice is read any further. Then: a
	 * card that no longer stands (CosmicPetitions::StillStands) is taken away (Withdrawn - no cost, no effect, no line);
	 * otherwise the cost is paid from the stockpile, the choice is checked (CosmicPetitions::Passes), and its OnSuccess
	 * or OnFailure effects run in place on the ruled people and the card's system (FollowUpKnowing after them), its
	 * SuccessText or FailureText is told (Log.Civ, Severity 1, no draw of the tell stream), the lines that follow are told
	 * (FollowUpNotes), the row is remembered in the life (LastChoiceRows) and the card is taken away. A choice with a
	 * NextEncounter puts that row as the next card on the same subject when its conditions hold (no draw). A DeposeRuler
	 * of an effect is recorded and resolved by the next tick's role-effect stage. The stream is synced at the end.
	 * The first contact card (CosmicRows::PetitionFirstContact, B3 header section 2.9) is about the people of
	 * Knowing.PendingContactCivID: its effects' context OtherID and its texts' {OtherCiv}; that slot is cleared after its
	 * answer, taken or withdrawn. Every other card's context has no OtherID.
	 */
	FCosmicPetitionAnswer AnswerPetition(int32 ChoiceIndex);

	/**
	 * The ruler's research focus (section 5.4, 7.5; MVP item 7), between ticks: Field = a key of the ruled people's
	 * ResearchFocus takes DT_Rules.ResearchFocusShare and the others the rest evenly (CosmicResearch::Focus - the rule of the
	 * FocusResearch effect); NAME_None = every key even (CosmicResearch::Even). The next tick's research stage spends by it.
	 * Only while Embodied and Ruler with a ruled people that is in the state. False and nothing changed: for a field that is
	 * not a key; when the shares would not change (the focus asked is already the people's - not a decision); with one error
	 * line for a field while ResearchFocusShare cannot focus (not above the even share 1/4, above 1, NaN - a data fault the
	 * schema keeps out); with one error line inside a handler of a Core Logic delegate. No draw, no line, no save of its own
	 * (as for StartMegaproject: the next tick boundary that writes, or the exit save, keeps it).
	 */
	bool SetResearchFocus(FName Field);

	/**
	 * The ruler's research shares by hand (B2 header section 2.3, 2.8; 16 #119 (3)), between ticks: Weights holds the ruled
	 * people's fields except the transcendence field, each 0..CosmicResearch::MaxShareWeight with a sum above 0
	 * (CosmicResearch::SharesOf). Only while Embodied and Ruler with a ruled people. False and nothing changed when the
	 * weights do not fit or the shares would not change; with one error line inside a handler of a Core Logic delegate. No
	 * draw, no line, no save of its own (as SetResearchFocus).
	 */
	bool SetResearchShares(const TMap<FName, int32>& Weights);

	/**
	 * The ruler enacts an edict (B2 header section 2.3), between ticks: the first reason of CosmicEdicts::EnactBlockOf for
	 * the ruled people, NotRuler without an embodied ruler, Handler (and an error line) inside a handler of a Core Logic
	 * delegate - nothing changed. On None the row's EnactCost is paid in full and the row appended to ActiveEdicts
	 * (CosmicEdicts::Enact), and log_edict_enacted is told (Severity 1, one draw of the tell stream). No draw of the sim
	 * stream, no save of its own.
	 */
	CosmicEdicts::ECosmicEdictBlock EnactEdict(FName Row);

	/**
	 * The ruler revokes an edict it holds (B2 header section 2.3), between ticks: removed from ActiveEdicts keeping the order
	 * of the rest (CosmicEdicts::Revoke), log_edict_revoked told. No cost - enacting it again pays EnactCost again. False and
	 * nothing changed without an embodied ruler, for an edict not held, and with one error line inside a handler.
	 */
	bool RevokeEdict(FName Row);

	/**
	 * The ruler sets the ruled people's stance toward the people CivID (B3 header section 2.4), between ticks: Stance is
	 * Stance.Friendly, Stance.Hostile or the empty tag (neutral). The first reason of CosmicDiplomacy::StanceBlockOf (whether
	 * the people is met from Knowing.MetCivs - the input definition, rule 2), NotRuler without an embodied ruler, Handler
	 * (and an error line) inside a handler of a Core Logic delegate - nothing changed. On None the stance is written; a
	 * hostile one breaks the pair's trade treaty (CosmicDiplomacy::TurnHostile - the partner's view of the ruled people
	 * falls by DT_Rules.BetrayalRelationPenalty). Tells log_stance_changed, and log_treaty_broken when a treaty broke. No draw
	 * of the sim stream, no save of its own.
	 */
	CosmicDiplomacy::ECosmicStanceBlock SetStance(FName CivID, FGameplayTag Stance);

	/**
	 * The ruler proposes a treaty (Treaty.Trade or Treaty.NonAggression) to the people CivID (B3 header section 2.4), between
	 * ticks: accepted at once when CosmicDiplomacy::TreatyBlockOf says None (no draw) - TreatyCostInfluence is paid and the
	 * treaty signed on both sides (CosmicDiplomacy::Sign), log_treaty_signed told. Refused - nothing changed, no line - with
	 * the first reason; NotRuler and Handler as SetStance.
	 */
	CosmicDiplomacy::ECosmicTreatyBlock ProposeTreaty(FName CivID, FGameplayTag Treaty);

	/**
	 * The ruler breaks a treaty the ruled people holds with the people CivID (B3 header section 2.4), between ticks: removed on
	 * both sides (CosmicDiplomacy::Break), the partner's view of the ruled people falls by BetrayalRelationPenalty at once,
	 * log_treaty_broken told. False and nothing changed without an embodied ruler, for a treaty not held, and with one error
	 * line inside a handler.
	 */
	bool BreakTreaty(FName CivID, FGameplayTag Treaty);

	/**
	 * The ruler's fleet (B3 header section 2.4), between ticks: NAME_None withdraws it (no target - an even guard; always
	 * taken); a settled system of the ruled people is a guard (CosmicConflict::GuardBlockOf, answered with the siege names);
	 * another people's system a siege (CosmicConflict::SiegeBlockOf). On None FleetTargetSystemID and FleetTargetSinceTick =
	 * Tick are written - the same target asked again changes nothing (no siege time is wound back) - and a siege begun tells
	 * log_siege_begun (Severity 1: the player's own act, no auto-pause). Refused - nothing changed - with the first reason;
	 * NotRuler and Handler as SetStance. No draw, no save of its own.
	 */
	CosmicConflict::ECosmicSiegeBlock AllocateFleet(FName SystemID);

	/**
	 * The wanderer's move (B1 header section 2.3), between ticks: one lane - CosmicWanderer::TravelBlockOf must say None
	 * (whether a card waits from Knowing - the input definition). Sets Player.TravelTargetSystemID and
	 * TravelTicksRemaining = CosmicWanderer::LaneTicksOf; the eighth stage moves the vessel. No draw, no line, no save of
	 * its own. Refused - nothing changed - with the first reason; inside a handler of a Core Logic delegate an error line
	 * and Handler.
	 */
	CosmicWanderer::ETravelBlock TravelTo(FName SystemID);

	/**
	 * The wanderer's scan (B1 header section 2.3, section 5.9), between ticks: one look from the system it stands in
	 * (UGalaxySimulationSubsystem::LookFrom) with the reach SightLanesByTier[PersonalTier] (below 1 read as 1) plus
	 * DT_Rules.ScanSightBonusLanes (below 0 read as 0) - knowing and the telling only, no draw of the sim stream. Tells
	 * CosmicRows::LogScanWild (Severity 1) when the vessel stands in a system no people owns and the look learned a system.
	 * Returns how many systems it learned (0 = nothing new); INDEX_NONE when refused: not an embodied wanderer standing
	 * still, or inside a handler (an error line).
	 */
	int32 ScanCurrentSystem();

	/**
	 * The coronation or the return (B1 header section 2.7), between ticks, when CosmicWanderer::CrownBlockOf says None
	 * (whether the people is met from Knowing.MetCivs, whether a card waits - the input definition): the people's
	 * bIsPlayerRuled, Role Ruler and RuledCivID, one more entry in each reign array, the people's open Ruler and Regent
	 * records closed (LastTick = max(Tick, 1)), the journey, the waiting card and Knowing.PendingContactCivID cleared, the
	 * people's owned systems known (NoteKnown in OwnedSystems order, no line), a fleet target on another people's system
	 * lifted and every hostile stance turned neutral (B3 - no siege or hostility the new ruler could not see or change; the
	 * treaties stay), then CosmicRows::LogCrowned - or
	 * LogReturned for the people the vessel left (FormerCivID) - Severity 2. FormerCivID, StatusTags and the life's PeakTier
	 * stay. No draw; the next tick boundary saves it (the Role comparison of the save layer). Refused - nothing changed -
	 * with the first reason; inside a handler an error line and Handler.
	 */
	CosmicWanderer::ECrownBlock BecomeRuler(FName CivID);

	/**
	 * The ruler's abdication (B1 header section 2.8), between ticks: records Effect.DeposeRuler with the cause
	 * Reign.Abdicated and the source row CosmicRows::LogThroneVacated through UCosmicEventSubsystem::Apply (the ruled
	 * people as the context - the first recorded effect wins); the tenth stage of the next tick runs AbdicateRuler. No draw.
	 * False and nothing changed unless an embodied ruler with no role effect recorded; inside a handler an error line.
	 */
	bool Abdicate();

	/**
	 * The abdication pipeline, run by the role-effect stage for Effect.DeposeRuler with the cause Reign.Abdicated (B1 header
	 * section 2.8): tells CosmicRows::LogThroneVacated (Severity 2), draws the regent profile (one number,
	 * CosmicState::PickAIProfile), BecomeWanderer with Reign.Abdicated (its notable hook draws the regent's name - one
	 * number - and opens the Notable.Regent record), then the people it left loses DT_Rules.AbdicationStabilityLoss of
	 * Stability (not below 0; below 0 or not a number read as 0) and every people's throne is locked until Tick +
	 * ThroneLockTicks (never earlier than it was). Reverence stays. Two numbers of the sim stream, always. Does nothing
	 * unless the vessel rules; inside a handler an error line.
	 */
	void AbdicateRuler();

private:
	/**
	 * The half of BecomeWanderer a death shares (B5 header section 2.3): the people is no longer ruled (bIsPlayerRuled false,
	 * RegentProfile when valid), FormerCivID = the ruled people, RuledCivID cleared, Role Wanderer, the status of
	 * ReignEndReason (none for Reign.Died), the open reign closed with it, the waiting cards, the first contact waiting and
	 * the journey cleared, and the regent hook for Reign.Abdicated and Reign.Died (NameNotable through Apply, one number,
	 * source row RegentSourceRow). No new eye. Returns whether the throne was left: false unless an embodied ruler, and with
	 * an error line in the sanctuary. The callers ask about a handler first.
	 */
	bool ReleaseThrone(FGameplayTag RegentProfile, FGameplayTag ReignEndReason, FName RegentSourceRow);
};

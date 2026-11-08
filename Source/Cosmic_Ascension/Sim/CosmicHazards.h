// Cosmic Ascension - Sim layer. Design: guideline.md section 4.1, 4.2, 4.5, 5.10, 7.2, 7.4, 7.5.
// The world side of hazards and of the map: placing a hazard, resolving it, handing a system over, losing a capital,
// destroying a system, and the claim rule's two questions. World state only: no line, no knowing, no delegate, and no
// draw of their own (the effects Resolve applies draw as Apply does: a NameNotable among them draws once).
// UCosmicEventSubsystem::Apply (SpawnHazard, ResolveHazard, TransferSystem) and the hazard stage both come here, so a
// hazard is placed by one path and a system changes hands by one path. What has to be told afterwards is left as a note
// in the context (FCosmicEvalContext::Notes) for UCosmicEventSubsystem::FollowUpNotes.
// The single windows here (CLAUDE.md rule 13): Place (a hazard enters a slot), VesselAtRiskOf (whom a KillPlayer
// reaches - B5), ClaimRuleHoldsOf (the claim rule's deciding three, shared by the crisis stage and the screen - B6a),
// ClaimMayFollowOf (the screen's answer for a standing hazard, after the expiry's order - B6a), the rebellion watch:
// LowStabilityThreshold (the one threshold the economy stage counts with) and RebellionWatchOf (what the capital panel
// shows of it - B6a), and the extinction's two (B6b): LetPeopleGo (a people lets every system go) and KillsTheRulerOf (the
// expiry that kills the ruler). The screen reads these and copies no rule.
// RULE (section 6.1): files under Sim/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Core/GalaxyState.h"
#include "Sim/CosmicEvalContext.h"

struct FCosmicDataTables;
struct FCosmicRulesDef;
struct FCosmicStateIndex;

namespace CosmicHazards
{
	// A destroyed system (section 7.2, 16 #29 (5)): its star class is gone. It has no owner and no lane, lies in no sphere,
	// and every rule over living systems passes it by. It is never removed from State.Systems.
	inline bool IsDeadNode(const FStarSystemState& System)
	{
		return !System.StarClass.IsValid();
	}

	// Empties the hazard slot of System and the response slot that answered its hazard (section 5.7, 16 #23). Resolve,
	// Transfer, Destroy and the expiry come through here, so a response never outlives its hazard.
	COSMIC_ASCENSION_API void EmptyHazardSlot(FStarSystemState& System);

	// Whether SystemIndex is a settled system and the only settled system its owner owns (entries of OwnedSystems whose
	// owner is the people): the system the last-system gate asks about, and the one TransferSystem never takes (section
	// 4.5, 5.7). A system being settled is not a home and does not count: a people that loses its last settled system
	// lets its settlements go with it (LetSettlementsGo).
	COSMIC_ASCENSION_API bool IsLastSettledSystem(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 SystemIndex);

	/**
	 * When the people CivIndex owns no settled system, every system it still owns - each a settlement under way - goes to
	 * no one (section 5.7): owner none, out of its OwnedSystems, both project slots and the hazard slot empty (the
	 * completion records stay). Nothing otherwise. Destroy calls it for the owner of the system it destroys, and the
	 * cleanup of the crossing stage for every people, so that no people holds only settlements it can never pay for.
	 * Returns whether it let any go. No line, no note, no draw, and no role effect: a stage that takes the last settled
	 * system of the people the player rules records the collapse itself (section 7.4 (o)).
	 */
	COSMIC_ASCENSION_API bool LetSettlementsGo(FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex);

	/**
	 * The expiry of an extinction (B6b header section 2.2, decision 5): every system the people CivIndex owns - settled or
	 * being settled, in OwnedSystems order - goes to no one: owner none, out of its OwnedSystems, both project slots and the
	 * hazard slot empty. A hazard of another of its systems ends without result (a rebellion also closes its leader's open
	 * record and starts the count over, as Transfer ends one; an expiry of the same pass finds its slot empty and turns back
	 * at its first question - its two numbers were drawn already); every people's fleet target on the systems is lifted
	 * (CosmicConflict::LiftTargetsOn); the completion records stay (one of no people - a save from before B3 - is stamped with
	 * the people first, as Transfer stamps it, #73). The capital keeps its ID (no system is left to move it to). Only stage
	 * code behind the last-system gate calls it - Transfer refuses a last settled system and LetSettlementsGo moves only a
	 * people with none. Returns whether it let any go. No line, no note, no draw, and no role effect: the expiry records the
	 * ruler's death, the deposition and the record line itself; the crossing stage removes the people.
	 */
	COSMIC_ASCENSION_API bool LetPeopleGo(FCosmicEvalContext& Ctx, int32 CivIndex);

	/**
	 * Places the hazard of the DT_Hazards row HazardRow on SystemIndex: the one path (section 7.4). The spawn draw, the
	 * capital rule, the claim rule and the SpawnHazard effect all come here. Refused (false, nothing changed) for the
	 * sanctuary, whatever else holds (Law.Hedge, section 4.5 (1)), and unless the
	 * row exists, the system is a settled system with a free slot and, for a Capital row, the capital of its owner.
	 * Sets ActiveHazard to the row's HazardTag and HazardTicksRemaining to its CountdownTicks (read as at least 1) plus
	 * the edicts' CosmicEdicts::CountdownBonusOf for its tag on the system (the owner's - B2 - and the galaxy laws of the
	 * other peoples of its galaxy - B6b) plus the delay of a fleet that guards the system
	 * (CosmicConflict::FleetDelayOf, B3) - at placement only: no edict or guard changes a standing hazard's count, a card's
	 * DelayHazard alone adds to it (B4, CosmicConflict::ResistBlockOf) - and leaves a HazardPlaced note in Ctx.
	 */
	COSMIC_ASCENSION_API bool Place(FCosmicEvalContext& Ctx, int32 SystemIndex, FName HazardRow);

	/**
	 * Resolves the hazard of SystemIndex (section 4.1): empties the slot first, leaves a HazardResolved note, then
	 * applies the row's OnResolveEffects with the system and its owner as subject and the row as SourceRow (the subject
	 * of Ctx is restored afterwards). A Hazard.Rebellion also closes the open Notable.Rebel record of the owner
	 * (LastTick = max(Tick, 1)) and starts the owner's LowStabilityTicks over (section 4.1, 7.4 notable hook 2). False
	 * when the slot is empty. A hazard whose tag no row holds is emptied without effects and without a note (a rebellion
	 * still ends its leader and its count). Resolutions nested through OnResolveEffects stop at a fixed depth with an
	 * error line instead of running the stack out: no row of the game nests (Data.Integrity).
	 */
	COSMIC_ASCENSION_API bool Resolve(FCosmicEvalContext& Ctx, int32 SystemIndex);

	/**
	 * Hands SystemIndex to the civilization ToCivIndex, or lets it go to no one when ToCivIndex is INDEX_NONE (section
	 * 16 #29 (6)). Refused (false, nothing changed) for the sanctuary (Law.Hedge, section 4.5 (1)), for a dead node, for a
	 * system that already is the target's (an
	 * unowned one cannot be let go), and for the last settled system of its owner (IsLastSettledSystem): only stage code
	 * behind the last-system gate takes that (section 4.5). What changes with the owner: the system leaves the old owner's OwnedSystems and joins
	 * the end of the new owner's; both project slots end (the completion records stay); the hazard of a Capital
	 * row ends without result, and so does a Hazard.Rebellion whose tag no row holds (the old owner's discontent either
	 * way) and any hazard of a system let go (no people under it, no threat: section 16 #29 (5)) - a rebellion that ends so
	 * also closes its leader's open record and starts the old owner's count over, as a resolution does; when it was the old
	 * owner's capital, the capital moves (MoveCapital). Leaves a SystemReceived note when ToCivIndex is a people, with
	 * the old owner (FollowUpKnowing makes the system known to the calling, and meets the old owner, when that people is
	 * the vessel's); a hazard it ends is not told. Before the owner changes (B3): a completion record of no people (a save
	 * from before B3) is stamped with the old owner (#73), and every people's fleet target on the system is lifted
	 * (CosmicConflict::LiftTargetsOn); a ruling vessel standing on it while the calling is Embodied goes to the capital of
	 * the people it rules when that people loses the system (as Destroy carries it - B3).
	 */
	COSMIC_ASCENSION_API bool Transfer(FCosmicEvalContext& Ctx, int32 SystemIndex, int32 ToCivIndex);

	/**
	 * The capital of CivIndex was lost (section 5.10): HomeSystemID moves to its first settled system in OwnedSystems
	 * order, or with none to its first owned system (an entry whose owner is the people: a listed system another people
	 * owns is never made the capital), Stability goes down by 0.3 and PressureEnduredTicks starts over at 0 (B6b, #156). A
	 * people with no system left keeps the old ID, its Stability and its count.
	 */
	COSMIC_ASCENSION_API void MoveCapital(FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex);

	/**
	 * Destroys SystemIndex (section 7.2, 16 #29 (5)) - never the sanctuary (Law.Hedge, section 4.5 (1): nothing happens):
	 * the owner lets it go - and with it its settlements under way when it
	 * was the owner's last settled system (LetSettlementsGo) - and its capital moves; every lane to and from it
	 * is cut, its star class, hazard and both project slots are cleared (the completion records stay), and it leaves every
	 * sphere. A vessel standing there is carried away while the calling is Embodied and no KillPlayer is pending: a ruler
	 * to the new capital of its people, anyone else (and a ruler whose people owns no capital any more) to the first lane
	 * neighbour of the old Lanes that is neither a dead node nor the sanctuary; with none it stays (section 16 #53 (2), #81). A
	 * journey from there ends with it: both travel fields empty (B1 - the vessel still stands where a journey began). Every
	 * people's fleet target on it is lifted (B3). The stage that destroys
	 * asks the last-system gate first and records the collapse afterwards; it also tells the record line of a settled system
	 * destroyed (CosmicRows::LogRecordDestroyed, after the expiry line - B5 decision 11): no line here.
	 */
	COSMIC_ASCENSION_API void Destroy(FCosmicEvalContext& Ctx, int32 SystemIndex);

	/**
	 * The claimant of SystemIndex now (section 4.2), as an index into State.Transcendents: among the unsealed instances
	 * whose DT_Transcendents row is Realm.Abyssal, whose sphere does not hold the system and lies within
	 * DT_Rules.ClaimAdjacencyLanes lanes of it, the one whose pressure term on the system is largest; equal terms go to
	 * the earlier instance. INDEX_NONE when there is none. Neither the cap of the sphere nor Law.Remnant is asked here
	 * (ClaimRuleHoldsOf asks them).
	 * Tribute (condition (b), Trait.Faith.Tribute) is not here: it needs the trait rows and a tribute paid a tick with it,
	 * and no block owns it yet (B4 header section 0 - open question #137).
	 */
	COSMIC_ASCENSION_API int32 ClaimantOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 SystemIndex);

	/**
	 * Law.Remnant (section 4.2, 4.5 (1)): whether SystemIndex is the last settled system of its owner that lies in the
	 * sphere of no fallen (Realm.Abyssal) instance, sealed or not. Nothing may join it to a sphere.
	 */
	COSMIC_ASCENSION_API bool IsLastUnclaimed(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 SystemIndex);

	/**
	 * The claim rule (section 4.2) - its deciding three, the single window the game and the screen share (CLAUDE.md rule
	 * 13; B6a header section 4, decision 14): a claimant of SystemIndex exists (ClaimantOf) and the tables hold its row,
	 * its sphere holds fewer systems than the row's MaxInfluencedSystems, and the system is not its owner's last unclaimed
	 * settled system (IsLastUnclaimed - Law.Remnant). Not asked here: the system being settled and its slot free, which
	 * the crisis stage's claim rule after an expiry asks before it, and the faith number and the Hazard.Claim row, which it
	 * asks after it (UGalaxySimulationSubsystem - TryPlaceClaim calls this, so the rule is written once); ClaimMayFollowOf
	 * asks for the screen what the expiry decides first. World state only, no draw.
	 */
	COSMIC_ASCENSION_API bool ClaimRuleHoldsOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 SystemIndex);

	/**
	 * The screen's answer for the hazard standing on SystemIndex (B6a header section 4, decision 14): whether a claim may
	 * stand on the system when that hazard expires unanswered - the single window the system panel's warning line and the
	 * hazard's tooltip read (CLAUDE.md rule 13), so the screen copies no part of the rule. It follows the order of the
	 * expiry (UGalaxySimulationSubsystem::ExpireHazard) and asks its gates through the functions the expiry calls:
	 * false with no hazard standing, and for a tag no row holds (the expiry resolves it without a claim rule); false when
	 * the owner holds the edict that answers the hazard (CosmicEdicts::HeldAnswerOf - the expiry resolves it and passes
	 * no claim rule); false for Hazard.PreSupernova (the grace keeps a last system and the claim rule is not reached, or
	 * the system is destroyed and is settled no more), for Hazard.Claim (its expiry joins the system to the claimant's
	 * sphere, and ClaimantOf picks no instance whose sphere holds the system - exact while a run has one fallen instance,
	 * DT_Rules.FallenTranscendentCount 1; when it may have more, the block that raises it looks at this branch again) and
	 * for the extinction (KillsTheRulerOf - B6b: the grace keeps the people and empties its extinction slots only, or
	 * LetPeopleGo leaves the system unsettled - the claim rule is reached on neither).
	 * Otherwise what the claim rule decides on the slot the expiry leaves free (the standing hazard itself is not asked):
	 * a settled system, a Hazard.Claim row in the tables, and ClaimRuleHoldsOf. What the expiry's effects and the other
	 * expiries of its tick change is not foreseen, nor the faith number (it always passes now): a claim "may" follow.
	 * World state only, no draw.
	 */
	COSMIC_ASCENSION_API bool ClaimMayFollowOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 SystemIndex);

	// Stability below this counts toward FCivilizationState.LowStabilityTicks (section 4.1): the economy stage counts with
	// it at its end and RebellionWatchOf shows it - one constant, no copy (B6a header section 4). The CivStabilityBelowFor
	// condition carries 0.3 for the reader only: the code reads its Key (the tick count), never its Value, and the schema
	// holds that Value to 0.3 (Tools/data/schema.json, section 16 #25 (2)).
	inline constexpr float LowStabilityThreshold = 0.3f;

	/** What the rebellion watch of one people shows (RebellionWatchOf). */
	struct COSMIC_ASCENSION_API FCosmicRebellionWatch
	{
		// Whether the capital panel's line stands.
		bool bShown = false;
		// LowStabilityThreshold.
		float Threshold = 0.f;
		// The people's FCivilizationState.LowStabilityTicks: the ticks counted below Threshold so far.
		int32 LowTicks = 0;
		// The tick count of the Hazard.Rebellion row's CivStabilityBelowFor condition: read from the row, never written in
		// code.
		int32 NeededTicks = 0;
		// The first tick of the run the row's GalaxyTickAtLeast conditions hold at: the ceiling of the largest Value (the
		// evaluator asks float(Tick) >= Value), 0 when the row has none or none above zero. Read from the row.
		int64 NotBeforeTick = 0;
		// The count the low run reaches at the first tick both conditions hold, while it runs on - the line's {2}:
		// Max(NeededTicks, LowTicks + Max(0, NotBeforeTick - Tick)), clamped to int32; 0 when NeededTicks is 0.
		int32 WaitTicks = 0;
	};

	/**
	 * The rebellion watch of the people CivIndex (B6a header section 4, decision 14): the single window the capital
	 * panel's line reads (CLAUDE.md rule 13), so the screen writes neither the threshold nor the counts. Threshold is
	 * LowStabilityThreshold; LowTicks the people's LowStabilityTicks; NeededTicks the tick count of the CivStabilityBelowFor
	 * conditions of the DT_Hazards row of Hazard.Rebellion (found by its tag), read as the capital rule's conditions read
	 * it - the largest when the row has several, and 0 when one of the row's conditions read here is never true (a count
	 * that is not digits above zero, a GalaxyTickAtLeast Value that is NaN or past any tick), when the row has no count,
	 * or when no row holds the tag; NotBeforeTick the first tick its GalaxyTickAtLeast conditions hold at (the ceiling of
	 * the largest Value). WaitTicks is what the line names: each tick of the low run adds one to LowTicks and one to the
	 * tick, so the count reached at the first tick both conditions hold is NeededTicks, or LowTicks plus the ticks left to
	 * NotBeforeTick when that is larger (before the row's tick a count past NeededTicks still waits) - the same number from
	 * tick to tick while the run lasts. bShown when the people's Stability is below the threshold, LowTicks is above 0 (a
	 * stability that fell after the economy stage counted - a card's answer between ticks, an expiry of the crisis stage
	 * - has no run yet), NeededTicks is above 0, and no Hazard.Rebellion stands on its capital already (the hazard line
	 * speaks then; the count goes on past NeededTicks while it stands). The capital's slot is not asked (another hazard
	 * holding it keeps the rebellion out while the count runs on): a rebellion "may" stand. For an index out of range: not
	 * shown, Threshold set, the counts and NotBeforeTick 0. World state only, no draw.
	 */
	COSMIC_ASCENSION_API FCosmicRebellionWatch RebellionWatchOf(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 CivIndex);

	/**
	 * Whether a KillPlayer on the subject (SystemID, CivID) at Tick reaches the vessel (B5 header section 5.1): the rule of
	 * the record UCosmicEventSubsystem::Apply keeps, less 'the slot is free'. Only while Calling.LifeState is Embodied. A
	 * subject with a system (SystemID not None) reaches a wanderer standing in it - what happens to a system never kills a
	 * ruler; a subject without a system reaches the ruler of CivID (its extinction - the expiry's code records it so, B6b
	 * decision 5, KillsTheRulerOf); a subject with neither reaches the vessel anywhere. Never within
	 * DT_Rules.BirthProtectionTicks of Calling.BornTick (Tick - BornTick below it). Apply's KillPlayer, the stage clock's note
	 * of an expiry (the extinction's with its systemless subject) and the balance commandlet's death chances read it: the rule
	 * is not copied.
	 */
	COSMIC_ASCENSION_API bool VesselAtRiskOf(const FPlayerEntityState& Player, const FCallingState& Calling, const FCosmicRulesDef& Rules, int64 Tick,
		FName SystemID, FName CivID);

	/**
	 * Whether the hazard Tag kills a vessel standing in its system when it expires (B5 header section 2.4, 4): the DT_Hazards
	 * row of the tag carries a KillPlayer in its expiry effects (OnExpireEffects or OnExpireBranchEffects). False for a tag no
	 * row holds. Succession leaves such a system out of its candidates, and the warnings and the pause of the presentation
	 * read it: one answer.
	 */
	COSMIC_ASCENSION_API bool KillsAVesselOf(const FCosmicDataTables& Tables, const FGameplayTag& Tag);

	/**
	 * Whether the expiry of the hazard Tag kills the ruler of the system's people wherever the vessel stands (B6b header
	 * section 2.2, decision 5): Hazard.Extinction alone - the one death the code records rather than a row (the second source
	 * of a death the design names; DECISIONS_LOG B6b). Its expiry lets the people go (LetPeopleGo) and records KillPlayer
	 * (Death.Hazard, the row as its source) on a subject with no system (VesselAtRiskOf: the ruler of that people; the birth
	 * protection decided there), then DeposeRuler. The expiry's code, the death line's choice (log_death_extinction) and the
	 * screen's answers read it: one answer. KillsAVesselOf stays the data's (a vessel standing there) and is no part of it.
	 */
	COSMIC_ASCENSION_API bool KillsTheRulerOf(const FGameplayTag& Tag);
}

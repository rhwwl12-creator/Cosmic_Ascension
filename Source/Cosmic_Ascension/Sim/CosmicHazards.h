// Cosmic Ascension - Sim layer. Design: guideline.md section 4.1, 4.2, 4.5, 5.10, 7.2, 7.4, 7.5.
// The world side of hazards and of the map: placing a hazard, resolving it, handing a system over, losing a capital,
// destroying a system, and the claim rule's two questions. World state only: no line, no knowing, no delegate, and no
// draw of their own (the effects Resolve applies draw as Apply does: a NameNotable among them draws once).
// UCosmicEventSubsystem::Apply (SpawnHazard, ResolveHazard, TransferSystem) and the hazard stage both come here, so a
// hazard is placed by one path and a system changes hands by one path. What has to be told afterwards is left as a note
// in the context (FCosmicEvalContext::Notes) for UCosmicEventSubsystem::FollowUpNotes.
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
	 * Places the hazard of the DT_Hazards row HazardRow on SystemIndex: the one path (section 7.4). The spawn draw, the
	 * capital rule, the claim rule and the SpawnHazard effect all come here. Refused (false, nothing changed) for the
	 * sanctuary, whatever else holds (Law.Hedge, section 4.5 (1)), and unless the
	 * row exists, the system is a settled system with a free slot and, for a Capital row, the capital of its owner.
	 * Sets ActiveHazard to the row's HazardTag and HazardTicksRemaining to its CountdownTicks (read as at least 1) and
	 * leaves a HazardPlaced note in Ctx.
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
	 * the vessel's); a hazard it ends is not told.
	 */
	COSMIC_ASCENSION_API bool Transfer(FCosmicEvalContext& Ctx, int32 SystemIndex, int32 ToCivIndex);

	/**
	 * The capital of CivIndex was lost (section 5.10): HomeSystemID moves to its first settled system in OwnedSystems
	 * order, or with none to its first owned system (an entry whose owner is the people: a listed system another people
	 * owns is never made the capital), and Stability goes down by 0.3. A people with no system left keeps the old ID and
	 * its Stability.
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
	 * journey from there ends with it: both travel fields empty (B1 - the vessel still stands where a journey began). The stage that destroys
	 * asks the last-system gate first and records the collapse afterwards.
	 */
	COSMIC_ASCENSION_API void Destroy(FCosmicEvalContext& Ctx, int32 SystemIndex);

	/**
	 * The claimant of SystemIndex now (section 4.2), as an index into State.Transcendents: among the unsealed instances
	 * whose DT_Transcendents row is Realm.Abyssal, whose sphere does not hold the system and lies within
	 * DT_Rules.ClaimAdjacencyLanes lanes of it, the one whose pressure term on the system is largest; equal terms go to
	 * the earlier instance. INDEX_NONE when there is none. Neither the cap of the sphere nor Law.Remnant is asked here.
	 * Tribute (condition (b), Trait.Faith.Tribute) joins with the tribute petition of Phase 4.
	 */
	COSMIC_ASCENSION_API int32 ClaimantOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 SystemIndex);

	/**
	 * Law.Remnant (section 4.2, 4.5 (1)): whether SystemIndex is the last settled system of its owner that lies in the
	 * sphere of no fallen (Realm.Abyssal) instance, sealed or not. Nothing may join it to a sphere.
	 */
	COSMIC_ASCENSION_API bool IsLastUnclaimed(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 SystemIndex);
}

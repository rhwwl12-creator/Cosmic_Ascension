// Cosmic Ascension - Sim layer. Design: guideline.md section 4.2, 4.3, 5.7, 5.12, 7.4.
// The world side of megaprojects: starting one and why it cannot start, what a people would start, settlement and why a
// people does not settle, one tick of one project slot of a system (step (a) runs every response slot, then every
// construction slot) and how to read whether a slot paid. World state only: no line, no knowing, no delegate and no draw of their own (a completion that resolves a
// hazard goes through CosmicHazards::Resolve, whose effects draw as Apply does; the AI draws its settlement number
// before it calls Settle). The ruler's command and the AI start through Start and Settle; the crisis stage advances
// through Advance and tells the lines afterwards.
// RULE (section 6.1): files under Sim/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Sim/CosmicEvalContext.h"
#include "Templates/Tuple.h"

struct FMegaprojectDef;

struct FCosmicDataTables;
struct FCosmicRulesDef;
struct FCosmicStateIndex;

namespace CosmicMegaprojects
{
	// The two project slots of a system (section 5.7): the response slot answers the hazard standing there, the
	// construction slot builds. Step (a) of the crisis stage advances every response slot before any construction slot.
	enum class ECosmicProjectSlot : uint8
	{
		Response,
		Construction,
	};

	// One project slot as the state holds it: its row (NAME_None when empty) and its progress. A reading for the
	// presentation and the balance commandlet; never stored.
	struct FCosmicSlotReading
	{
		FName Row;
		float Progress = 0.f;
	};

	// The reading of Slot on System.
	COSMIC_ASCENSION_API FCosmicSlotReading ReadSlot(const FStarSystemState& System, ECosmicProjectSlot Slot);

	/**
	 * Whether a slot read at the end of one tick (Before) and again at the end of the next tick (After) paid nothing in
	 * that tick: it holds the same row, not NAME_None, with exactly the same progress. Exact by Advance and step (a) of the
	 * crisis stage: a tick that cannot be paid in full changes nothing in the slot, and neither does a construction held
	 * back for an unpaid answer of its people (a want of resources too); a paid tick adds the rate (1, or the AI rate,
	 * which is above 0) or completes the row and empties the slot. A settlement whose row the tables do not hold stays and
	 * reads as paying nothing. Not exact for a row that completes in one paid tick and starts again in the same tick (no
	 * game row: RequiredProgress is 100 and up), nor for an AI rate too small to move the float. Reads only.
	 */
	COSMIC_ASCENSION_API bool PaidNothing(const FCosmicSlotReading& Before, const FCosmicSlotReading& After);

	/**
	 * Whether the system holds a completion of RowName (CompletedMegaprojects) whose tick is above 0 and not below
	 * SinceTick: the test of condition MegaprojectComplete (section 4.4, 5.7). An empty RowName is false.
	 */
	COSMIC_ASCENSION_API bool HasCompleted(const FStarSystemState& System, FName RowName, int64 SinceTick);

	/**
	 * The progress Row needs to complete, as Advance reads it: its RequiredProgress, or 1 when that is below 1 or not a
	 * number. The panel shows the same number.
	 */
	COSMIC_ASCENSION_API float RequiredProgressOf(const FMegaprojectDef& Row);

	/**
	 * The cost of one tick of Row as Advance pays it: its CostPerTick with every member below 0, or not a number, or not
	 * finite read as 0.
	 */
	COSMIC_ASCENSION_API FResourcePool CostPerTickOf(const FMegaprojectDef& Row);

	/** Whether Stockpile can pay Cost (a cost read through CostPerTickOf) in full: no member of it is below Cost's. */
	COSMIC_ASCENSION_API bool CanPay(const FResourcePool& Stockpile, const FResourcePool& Cost);

	// Why Start refuses a row on a system, in the order StartBlockOf asks (the system panel shows it - section 5.7, 6.3):
	// first the row (none, the settle row), then the system (none, dead, not settled, no living owner), then - for an
	// answer - whether it answers the hazard standing there (none, another), then the people (the row's Tier), last the
	// slot (this answer runs, another answer runs, this construction runs). A construction slot that holds another row is
	// no reason (Start replaces that row, section 5.7), and neither is the cost (a tick that cannot pay waits).
	enum class ECosmicStartBlock : uint8
	{
		None,
		// The tables hold no such row (NAME_None included).
		NoRow,
		// project_settle: a settlement starts through Settle alone (the panel's settle button).
		SettleRow,
		// SystemIndex is out of range.
		NoSystem,
		// The system is a dead node.
		Dead,
		// The system has no owner, or a settlement is under way there.
		NotSettled,
		// The owner is not a people of Civs.
		NoOwner,
		// An answer (FCosmicDataTables::IsHazardAnswer), and no hazard with a row stands there.
		NoHazard,
		// An answer that the row of the hazard standing there does not list.
		OtherHazard,
		// The owner's Tier is below the row's RequiredTier.
		RowTier,
		// An answer, and the response slot holds this row already.
		Answering,
		// An answer, and the response slot holds another row.
		ResponseBusy,
		// A construction row that runs in the construction slot already.
		Running,
		// The number of the values above (not a reason).
		Count,
	};

	/**
	 * The first reason Start refuses RowName on SystemIndex now, in the order of the enum; None when it takes it. Every
	 * reason is one refusal of the rule of section 5.7 and the rule is their conjunction, so the order names the reason
	 * and decides nothing: no start of the command or of the AI depends on it. The hazard is asked before the rung so that
	 * the panel can leave out an answer whose hazard does not stand there instead of greying it for the rung. Reads only;
	 * draws nothing.
	 */
	COSMIC_ASCENSION_API ECosmicStartBlock StartBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 SystemIndex, FName RowName);

	/**
	 * Whether Start would take RowName on SystemIndex now (section 5.7): StartBlockOf says None - one rule for the ruler's
	 * command, the AI's choices and the panel's project buttons. That is: the row exists and is not project_settle (a
	 * settlement starts through Settle alone), the system is a settled system of a living people whose Tier is at least the
	 * row's RequiredTier, and then - for an answer (FCosmicDataTables::IsHazardAnswer) - the row of the system's hazard
	 * lists it and the response slot is empty, or - for any other row - the construction slot does not hold that row.
	 */
	COSMIC_ASCENSION_API bool CanStart(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 SystemIndex, FName RowName);

	/**
	 * Starts RowName on SystemIndex for its owner (section 5.7): the one path of the ruler's command and of the AI.
	 * Refused (false, nothing changed) when CanStart is false. An answer takes the response slot; any other row takes the
	 * construction slot, over a different row there (that progress is lost). Progress starts at 0. No line, no draw.
	 */
	COSMIC_ASCENSION_API bool Start(FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 SystemIndex, FName RowName);

	/**
	 * What the people CivIndex would start now (section 4.3, 5.7), as (system index, row) in order: for each settled
	 * system it owns, in OwnedSystems order, whose response slot is empty, the first row in the RulerResolutions of its
	 * hazard's row that Start would take; then, while the construction slot of its settled capital is empty, the first
	 * construction row named by a MegaprojectComplete condition of a Ruler feat for its Tier that the judgment looks at
	 * (FAscensionFeatDef::IsJudgedRulerRowAt; feat rows in lexical order, conditions in order) that it has completed on none of the settled systems it owns since SinceTick, that runs in the
	 * construction slot of none of them, and that Start would take. An entry of OwnedSystems whose owner is another people
	 * counts for nothing. Reads only; draws nothing. The AI starts every choice; the prototype's project key starts the
	 * first one for the people the player rules.
	 */
	COSMIC_ASCENSION_API TArray<TPair<int32, FName>> Choices(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 CivIndex, int64 SinceTick);

	// The realm cap of Tier (section 5.12): DT_Rules.RealmCapByTier[Tier], or 0 when the array lacks that index
	// (Data.Integrity keeps exactly five entries).
	COSMIC_ASCENSION_API int32 RealmCapOf(const FCosmicRulesDef& Rules, EAscensionTier Tier);

	// Whether the people CivIndex has a settlement under way: a system it owns whose construction slot holds
	// project_settle (stalled or not - nothing abandons a settlement).
	COSMIC_ASCENSION_API bool IsSettling(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex);

	/**
	 * Whether the people CivIndex may start a settlement now (section 5.7, 5.12), whatever the targets and the tables:
	 * it owns a settled system, it owns fewer systems than RealmCapOf its Tier, and it has no settlement under way.
	 * The AI's settlement draw is made for exactly these peoples (one each, before the targets are looked at).
	 */
	COSMIC_ASCENSION_API bool MaySettle(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, int32 CivIndex);

	// The rung from which a sphere no longer refuses a settlement (section 4.2 modifier 4).
	constexpr EAscensionTier SphereLiftTier = EAscensionTier::Tier3_Causal;

	// Why a settlement of a system is refused, in the order SettleBlockOf asks (the system panel shows it - section 5.7,
	// 5.12): first what the system is (an owner, a dead node, no foothold one lane away), then what the people may do now
	// (the settle row and its Tier, the realm cap, a settlement under way), last the sphere that the third rung lifts.
	enum class ECosmicSettleBlock : uint8
	{
		None,
		// The people or the system does not exist.
		NoPeople,
		// The system has an owner (settled or being settled).
		Owned,
		// The system is a dead node.
		Dead,
		// No settled system of the people lies one lane away in the system's galaxy row.
		NoLane,
		// The tables hold no project_settle row.
		NoRow,
		// The people's Tier is below the project_settle row's RequiredTier.
		RowTier,
		// The people owns as many systems as RealmCapOf its Tier allows.
		RealmCap,
		// The people has a settlement under way.
		Settling,
		// The system lies in the sphere of an unsealed transcendent (one with a DT_Transcendents row, as the pressure counts
		// them) and the people stands below Tier 3 (section 4.2 modifier 4).
		Sphere,
		// The number of the values above (not a reason).
		Count,
	};

	/**
	 * The first reason the people CivIndex cannot settle SystemIndex now, in the order of the enum; None when it can: the
	 * system is wild (no owner, not a dead node) and one lane from a settled system the people owns, in that system's
	 * GalaxyRow; the project_settle row exists and its RequiredTier is at most the people's Tier; the people owns fewer
	 * systems than RealmCapOf its Tier and has no settlement under way (MaySettle); and outside every sphere, or on Tier 3
	 * or higher. Reads only; draws nothing.
	 */
	COSMIC_ASCENSION_API ECosmicSettleBlock SettleBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex, int32 SystemIndex);

	// Whether the people CivIndex can settle SystemIndex now: SettleBlockOf says None (one rule for the command, the AI's
	// targets and the panel's button).
	COSMIC_ASCENSION_API bool CanSettle(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex, int32 SystemIndex);

	// The systems CanSettle holds for the people CivIndex, in Systems order. Reads only. Looks only at the lane neighbours
	// of the people's settled systems (a target lies one lane from one of them).
	COSMIC_ASCENSION_API TArray<int32> SettleTargets(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex);

	/**
	 * Why the people CivIndex starts no settlement now, as one reason (the balance commandlet counts it - section 9.3):
	 * NoPeople for an index out of range; NoLane when it owns no settled system; then the people's reasons of SettleBlockOf
	 * in its order - NoRow, RowTier, RealmCap, Settling; then, when none of those stands, the lane neighbours of its
	 * settled systems (the candidates of SettleTargets), each read by SettleBlockOf itself: None as soon as one of them is
	 * a target, otherwise the strongest of their reasons - Sphere (the third rung lifts it) before Owned before Dead
	 * before NoLane (no neighbour, or none in the galaxy row of a foothold). None exactly when SettleTargets is not empty:
	 * the settlement draw would find a target now. Reads only; draws nothing.
	 */
	COSMIC_ASCENSION_API ECosmicSettleBlock PeopleSettleBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex);

	/**
	 * Starts the settlement of SystemIndex by the people CivIndex (section 5.7): refused (false, nothing changed)
	 * unless CanSettle. The system becomes the people's through CosmicHazards::Transfer (the one path by which an effect
	 * or a settlement hands a system over; only the stage code of Destroy and LetSettlementsGo lets one go otherwise) and
	 * its construction slot takes project_settle from 0. Until the project completes the system is
	 * not settled: no income, no hazard, no foothold for the next settlement, not in the count of Law.Remnant.
	 * No line, no draw (the AI draws before it calls this; the ruler's command draws nothing). Leaves the SystemReceived
	 * note of CosmicHazards::Transfer in Ctx: a caller that may settle for the people the vessel rules follows it
	 * (FollowUpKnowing); the AI stage drops it, since the people it settles for is never the vessel's.
	 */
	COSMIC_ASCENSION_API bool Settle(FCosmicEvalContext& Ctx, int32 CivIndex, int32 SystemIndex);

	/**
	 * One tick of one slot of SystemIndex (step (a) of the crisis stage, section 7.4: every response slot in Systems order,
	 * then every construction slot in Systems order - an answer spends the stockpile before a construction does). A
	 * settlement (project_settle) is paid, progresses and completes as every construction row does; it needs its row like
	 * them, but a settlement whose row the table does not hold stays in its slot (the system does not become settled for
	 * want of a row). Any other slot whose row the table does not hold, and a response the row of the system's hazard no
	 * longer lists, ends without result. Otherwise the slot is paid in full from the owner's stockpile and progresses by 1
	 * (by AIRate for a people the player does not rule), or costs and progresses nothing. A slot whose progress reaches
	 * RequiredProgressOf(row) completes (on the first paid tick on which the float sum reaches it, which for a rate not
	 * exact in binary can be one tick after Required / rate): the record of the row moves to this tick
	 * (CompletedMegaprojects, MegaprojectCompletedTick), the slot empties, the open life of the vessel takes the row when
	 * the people is the one the vessel rules, and the hazard of the system is resolved when its row lists the project
	 * (CosmicHazards::Resolve - a HazardResolved note in Ctx). Returns the completed row, or NAME_None, for the caller's
	 * line.
	 */
	COSMIC_ASCENSION_API FName Advance(FCosmicEvalContext& Ctx, int32 SystemIndex, ECosmicProjectSlot Slot, float AIRate);
}

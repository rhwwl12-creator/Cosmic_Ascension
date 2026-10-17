// Cosmic Ascension - Sim layer. Design: guideline.md section 4.2, 4.3, 5.7, 5.12, 7.4.
// The world side of megaprojects: starting one, what a people would start, settlement, and one tick of one project slot
// of a system (step (a) runs every response slot, then every construction slot). World state only: no line, no knowing, no delegate and no draw of their own (a completion that resolves a
// hazard goes through CosmicHazards::Resolve, whose effects draw as Apply does; the AI draws its settlement number
// before it calls Settle). The ruler's command and the AI start through Start and Settle; the crisis stage advances
// through Advance and tells the lines afterwards.
// RULE (section 6.1): files under Sim/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Events/CosmicEventSubsystem.h"
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

	/**
	 * Whether Start would take RowName on SystemIndex now (section 5.7): the row exists and is not project_settle (a
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
	 * (FAscensionFeatDef::IsOneRungWithDeed; feat rows in lexical order, conditions in order) that it has completed on none of the settled systems it owns since SinceTick, that runs in the
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

	/**
	 * Whether the people CivIndex can settle SystemIndex now: MaySettle; the project_settle row exists and its
	 * RequiredTier is at most the people's Tier; SystemIndex is a wild system (no owner, not a dead node) one lane from
	 * a settled system the people owns and in that system's GalaxyRow; and, inside the sphere of an unsealed
	 * transcendent (one with a DT_Transcendents row, as the pressure counts them), the people stands on Tier 3 or higher
	 * (section 4.2 modifier 4).
	 */
	COSMIC_ASCENSION_API bool CanSettle(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex, int32 SystemIndex);

	// The systems CanSettle holds for the people CivIndex, in Systems order. Reads only. Looks only at the lane neighbours
	// of the people's settled systems (a target lies one lane from one of them).
	COSMIC_ASCENSION_API TArray<int32> SettleTargets(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex);

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

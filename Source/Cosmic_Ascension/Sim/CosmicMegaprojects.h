// Cosmic Ascension - Sim layer. Design: guideline.md section 4.3, 5.7, 7.4, 16 #23.
// The world side of megaprojects: starting one, what a people would start, and one tick of the two slots of a system.
// World state only: no line, no knowing, no delegate and no draw of their own (a completion that resolves a hazard goes
// through CosmicHazards::Resolve, whose effects draw as Apply does). The ruler's command and the AI start through Start;
// the crisis stage advances through Advance and tells the lines afterwards.
// RULE (section 6.1): files under Sim/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Core/GalaxyState.h"
#include "Events/CosmicEventSubsystem.h"
#include "Templates/Tuple.h"

struct FMegaprojectDef;

struct FCosmicDataTables;
struct FCosmicStateIndex;

namespace CosmicMegaprojects
{
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
	 * Whether Start would take RowName on SystemIndex now (section 5.7): the row exists and is not project_settle (its
	 * own rule comes with settlement), the system is a settled system of a living people whose Tier is at least the
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
	 * construction row named by a MegaprojectComplete condition of a Ruler feat for its Tier (feat rows in lexical order,
	 * conditions in order) that it has completed on none of the settled systems it owns since SinceTick, that runs in the
	 * construction slot of none of them, and that Start would take. An entry of OwnedSystems whose owner is another people
	 * counts for nothing. Reads only; draws nothing. The AI starts every choice; the prototype's project key starts the
	 * first one for the people the player rules.
	 */
	COSMIC_ASCENSION_API TArray<TPair<int32, FName>> Choices(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 CivIndex, int64 SinceTick);

	/**
	 * One tick of the two slots of SystemIndex (step (a) of the crisis stage, section 7.4): the construction slot, then the
	 * response slot. A construction slot that holds project_settle is left alone (no pay, no progress, no completion) until
	 * settlement brings its own rule (work item 9). A slot whose row the table does not hold, and a response the row of the
	 * system's hazard no longer lists, ends without result. Otherwise the slot is paid in full from the owner's stockpile
	 * and progresses by 1 (by AIRate for a people the player does not rule), or costs and progresses nothing. A slot whose
	 * progress reaches RequiredProgressOf(row) completes (on the first paid tick on which the float sum reaches it, which
	 * for a rate not exact in binary can be one tick after Required / rate): the record of the row moves to this tick (CompletedMegaprojects,
	 * MegaprojectCompletedTick), the slot empties, the open life of the vessel takes the row when the people is the one the
	 * vessel rules, and the hazard of the system is resolved when its row lists the project (CosmicHazards::Resolve - a
	 * HazardResolved note in Ctx). Returns the completed rows in slot order, for the caller's lines.
	 */
	COSMIC_ASCENSION_API TArray<FName, TInlineAllocator<2>> Advance(FCosmicEvalContext& Ctx, int32 SystemIndex, float AIRate);
}

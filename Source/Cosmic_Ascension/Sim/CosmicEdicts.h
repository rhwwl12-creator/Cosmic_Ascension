// Cosmic Ascension - Sim layer (Core Logic). Design: docs/60_DECISIONS/headers/B2_edicts_and_research.md section 2,
// docs/10_DESIGN/systems.md section 5.8.
// The single home of the edict rules: slots, the gate of an enactment, what a held edict multiplies or adds, the answer a
// people holds against a hazard, the AI's revocations, the inherited edict's invariant and, from B6b, the galaxy laws
// (DT_Edicts rows of Scope HomeGalaxy - docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md section 2.4): the reach
// of their two hazard hooks, the count of LawsHeldTicks and what the AI's law step asks. Pure functions of the state and
// the tables - no draw, no state of their own. The commands (UPlayerRoleSubsystem::EnactEdict / RevokeEdict), the economy,
// crisis and AI stages, the edict window and the commandlet's policy read them.

#pragma once

#include "CoreMinimal.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "GameplayTagContainer.h"

struct FCivilizationState;
struct FCosmicDataTables;
struct FCosmicRulesDef;
struct FCosmicStateIndex;
struct FEdictDef;
struct FGalaxyState;
struct FHazardDef;
struct FResourcePool;

namespace CosmicEdicts
{
	/** Why an edict cannot be enacted, in the order EnactBlockOf asks (the first reason wins). Not a UENUM: never saved,
	 * hashed or a data column. NotRuler and Handler are the command's own (EnactBlockOf never gives them). */
	enum class ECosmicEdictBlock : uint8
	{
		None,
		NotRuler,    // the command: no embodied ruler with a ruled people
		NoRow,       // not a DT_Edicts row
		Held,        // the people already holds it
		Tier,        // the people is below the row's RequiredTier
		Conditions,  // the row's EnactConditions do not hold (the people and its capital as subject)
		Slots,       // every slot is taken (HeldCountOf >= SlotsOf)
		Cost,        // the stockpile cannot pay EnactCost in full
		Handler,     // the command was called from a handler of a Core Logic delegate (an error line)
	};

	/** 1 + the number of DT_Rules.EdictSlotSocietyMarks the people's Society research has reached (Research.FindRef - a
	 * missing key is 0). A mark that is not a number is never reached. */
	COSMIC_ASCENSION_API int32 SlotsOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules);

	/** The held edicts the table knows. A name the table does not hold (a save made with other data) stays in ActiveEdicts and
	 * does nothing: no effect, no upkeep, no slot. */
	COSMIC_ASCENSION_API int32 HeldCountOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables);

	/** The first reason the people CivIndex cannot enact Row, NoRow .. Cost (None = it can). An index out of range is NoRow. */
	COSMIC_ASCENSION_API ECosmicEdictBlock EnactBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
		const FCosmicRulesDef& Rules, int32 CivIndex, FName Row);

	/** The first name in Hazard.RulerResolutions that is a DT_Edicts row the people holds; NAME_None when there is none. A
	 * project name is passed by (a project answers through its response slot). */
	COSMIC_ASCENSION_API FName HeldAnswerOf(const FCivilizationState& Civ, const FHazardDef& Hazard, const FCosmicDataTables& Tables);

	/** Whether Row is listed in the RulerResolutions of a hazard standing now on a settled system the people CivIndex owns. */
	COSMIC_ASCENSION_API bool AnswersStanding(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 CivIndex,
		FName Row);

	/** Whether Row is listed in the RulerResolutions of any DT_Hazards row (an answer edict). */
	COSMIC_ASCENSION_API bool IsAnswerEdict(const FCosmicDataTables& Tables, FName Row);

	/** The edicts whose two hazard hooks reach the system SystemIndex (B6b header section 2.4 - one walk, once a row): the
	 * ActiveEdicts of its owner (both scopes, in that order), then, in Civs order and each in its own ActiveEdicts order, the
	 * HomeGalaxy rows held by the other peoples whose GalaxyRow is the system's, a row name already in the list passed by.
	 * Names the table does not hold are left out. Without a HomeGalaxy row this is the owner's held edicts, as in B2. Empty
	 * for an index out of range; an unowned system has the galaxy's laws alone. */
	COSMIC_ASCENSION_API TArray<const FEdictDef*, TInlineAllocator<8>> HookEdictsOf(const FGalaxyState& State, const FCosmicStateIndex& Index,
		const FCosmicDataTables& Tables, int32 SystemIndex);

	/** Hook 1: the product, in the order of HookEdictsOf, of their HazardSpawnMultipliers for Tag (missing = 1) - on the spawn
	 * share of a Draw row on the system. A factor below 0, or not a finite number, reads as 0 - the comparison guard of the
	 * archetype's factor. */
	COSMIC_ASCENSION_API float SpawnMultiplierOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
		int32 SystemIndex, FGameplayTag Tag);

	/** Hook 1 over a list of hooks the caller holds (B7x [B-7x5r] - the spawn draw asks it for every Draw row of one system,
	 * so it gathers HookEdictsOf once a system): the same product, in the same order, of the same factors. */
	COSMIC_ASCENSION_API float SpawnMultiplierOf(TConstArrayView<const FEdictDef*> Hooks, FGameplayTag Tag);

	/** Hook 2: the sum of the HazardCountdownBonus for Tag of the edicts of HookEdictsOf - added when a hazard is placed on the
	 * system (CosmicHazards::Place): values above 0 only, summed as int64, cut to 0..100000. */
	COSMIC_ASCENSION_API int32 CountdownBonusOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
		int32 SystemIndex, FGameplayTag Tag);

	/** B6b: whether Row is a DT_Edicts row of Scope HomeGalaxy (a galaxy law). */
	COSMIC_ASCENSION_API bool IsGalaxyLaw(const FCosmicDataTables& Tables, FName Row);

	/** B6b: the DT_Edicts rows of Scope HomeGalaxy in lexical order (GetEdictRowNames) - the order the AI's law step and the
	 * measuring policy's law step walk them in. */
	COSMIC_ASCENSION_API TArray<FName> GalaxyLawRowsOf(const FCosmicDataTables& Tables);

	/** B6b: the galaxy laws the people holds that the table knows. */
	COSMIC_ASCENSION_API int32 GalaxyLawCountOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables);

	/** B6b: DT_Rules.GalaxyLawsToHold read for the count - below 1 is 1 (the range is a lint, the guard is here). */
	COSMIC_ASCENSION_API int32 GalaxyLawsToHoldOf(const FCosmicRulesDef& Rules);

	/** B6b (header section 2.4, decision 10): the tick the economy stage counts in LawsHeldTicks - the people stands on the
	 * fourth rung or above and holds GalaxyLawsToHoldOf galaxy laws or more (GalaxyLawCountOf). */
	COSMIC_ASCENSION_API bool HoldsGalaxyLawsOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules);

	/** B6b (header section 2.4, decision 11): whether the row judged at the people's rung asks for laws - the people stands on
	 * the fourth rung or above and a DT_Feats row that IsJudgedRulerRowAt its Tier carries CivLawsHeldTicksAtLeast. The AI's
	 * law step and the measuring policy's law step stand on it; false while no feat row carries the condition. */
	COSMIC_ASCENSION_API bool AsksForLawsOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables);

	/** B4: the product, in ActiveEdicts order, of the held edicts' PressureDrainMultiplier (a factor that is not above 0 and
	 * finite reads as 1 - the schema keeps it in (0, 1]). The economy stage multiplies the pressure's Stability drain by it
	 * once per people; exactly 1 while no held edict carries a factor. */
	COSMIC_ASCENSION_API float PressureDrainMultiplierOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables);

	/** Whether the row's EffectsPerTick hold a ChangeStability below 0 (the AI's revocation, the policy, the first hint). */
	COSMIC_ASCENSION_API bool HasStabilityCost(const FEdictDef& Row);

	/** The row's net Stability drain a tick: minus the sum of its EffectsPerTick ChangeStability values (finite ones only, in
	 * array order), so a cost and a gain net out; at or below 0 when the row takes no Stability on the whole. */
	COSMIC_ASCENSION_API float StabilityDrainOf(const FEdictDef& Row);

	/** A cost of the row read for paying: every member above 0 and finite, the rest 0. */
	COSMIC_ASCENSION_API FResourcePool CostOf(const FResourcePool& Cost);

	/** Pays Cost (read through CostOf) from Stockpile in full and returns true, or changes nothing and returns false. */
	COSMIC_ASCENSION_API bool TryPay(FResourcePool& Stockpile, const FResourcePool& Cost);

	/** Pays the row's EnactCost from the stockpile and appends Row at the end of ActiveEdicts. The gate (EnactBlockOf) is
	 * the caller's, and so is the cost check - this only does what it says. */
	COSMIC_ASCENSION_API void Enact(FCivilizationState& Civ, FName Row, const FEdictDef& Def);

	/** Removes Row from ActiveEdicts keeping the order of the rest (never RemoveSwap: the order is hashed and is the order
	 * the economy stage multiplies in). False when the people does not hold it. */
	COSMIC_ASCENSION_API bool Revoke(FCivilizationState& Civ, FName Row);

	/** The held edicts the AI stage revokes, in ActiveEdicts order (B2 section 2.6): an edict with a Stability cost once
	 * Stability is below DT_Rules.AIEdictRevokeStability (a value that is not a number revokes nothing of that kind), every
	 * other known edict at once. Names the table does not hold are left alone, and so are the galaxy laws (B6b decision 11:
	 * the AI's law step keeps them - a regent's people goes on holding the laws the ruler left). A people that carries
	 * Brick.Form.KeptLaw (B7x, decision 18) keeps the edicts it holds - except an edict that adds to the claim's countdown
	 * (HazardCountdownBonus holds Hazard.Claim) or answers a crisis (FCosmicDataTables::IsHazardAnswer), which it revokes by
	 * the rule above, and except while its row asks for galaxy laws (AsksForLawsOf), when it revokes as any AI does. */
	COSMIC_ASCENSION_API TArray<FName> AIRevokesOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules);

	/** The invariant of the inherited edict (B2 section 2.7): true when DT_Rules.StartingEdict is empty; false when it names
	 * no DT_Edicts row or a row whose net drain (StabilityDrainOf) is not above 0 (no failure end); else whether the most
	 * recovery, StabilityRecoveryPerTick x (1 + SocietyRecoveryBonus), stays below that net drain. The knobs are
	 * read as the economy stage reads them (a bonus that is not above 0 and finite is 0; a recovery outside (0, 1] is 0). */
	COSMIC_ASCENSION_API bool InheritedDrainOutrunsRecovery(const FCosmicRulesDef& Rules, const FCosmicDataTables& Tables);
}

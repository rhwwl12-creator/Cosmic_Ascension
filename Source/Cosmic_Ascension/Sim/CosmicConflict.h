// Cosmic Ascension - Sim layer (Core Logic). Design: docs/60_DECISIONS/headers/B3_diplomacy_and_conflict.md section 2.3,
// docs/10_DESIGN/systems.md section 5.6.
// The single home of the conflict rules: the fleet's cap and growth factor, the effective power across a tier gap, the
// gates of a siege and of a guard, the odds of a siege and its end, and the delay a guarding fleet buys a hazard placed on
// its system. A fleet is no unit: one number of a people (FCivilizationState::FleetPower) and one target system
// (FleetTargetSystemID - its own settled system = a guard, a hostile neighbour's settled system = a siege, none = an even
// guard). Pure functions of the state and the tables - no draw, no state of their own. The conflict stage, the AI stage,
// the command UPlayerRoleSubsystem::AllocateFleet, CosmicHazards::Place, the system panel and the policy read them. The
// attrition pressure is UGalaxySimulationSubsystem::ComputeAttritionPressureIn (beside ComputePressureIn). B4 adds the
// gate of the time a card's fleet buys a standing hazard (ResistBlockOf - docs/60_DECISIONS/headers/
// B4_transcendent_voice_and_pressure_cards.md section 2.4).

#pragma once

#include "CoreMinimal.h"

struct FCivilizationState;
struct FCosmicDataTables;
struct FCosmicRulesDef;
struct FCosmicStateIndex;
struct FGalaxyState;
struct FGameplayTag;
struct FHazardDef;

namespace CosmicConflict
{
	/** Why a siege cannot be laid (or kept), in the order SiegeBlockOf asks: the structure first, the stance last, so the
	 * first reason names the way. Not a UENUM: never saved, hashed or a data column. NotRuler and Handler are the command's
	 * own; AllocateFleet answers a guard's reasons with the same names (GuardBlockOf). */
	enum class ECosmicSiegeBlock : uint8
	{
		None,
		NotRuler,       // the command: no embodied ruler with a ruled people
		Sanctuary,      // the sanctuary (Law.Hedge - the first refusal, rule 13)
		NoSystem,       // no such system, or no such people
		Dead,           // a dead node
		Unowned,        // no owner
		Own,            // the people's own system (that is a guard)
		NotSettled,     // being settled
		Capital,        // the owner's capital - conflict never takes it
		LastSettled,    // the owner's last settled system - nor that (CosmicHazards::IsLastSettledSystem)
		NotAdjacent,    // not one lane from a settled system of the people, in that system's galaxy row
		Besieged,       // another people's siege already stands on it
		RealmCap,       // the people owns as many systems as RealmCapOf its Tier (a conquest counts, #42)
		NoFleet,        // the people's effective power against the owner is 0
		NonAggression,  // the pair holds a non-aggression treaty
		NotHostile,     // the people is not hostile toward the owner
		Handler,        // the command was called from a handler of a Core Logic delegate (an error line)
	};

	/** Why a guard cannot be set, in the order GuardBlockOf asks. Not a UENUM. */
	enum class ECosmicGuardBlock : uint8
	{
		None,
		Sanctuary,
		NoSystem,
		Dead,
		NotOwn,      // not the people's system
		NotSettled,
		NoFleet,     // the fleet's cap is 0 (the first rung has no fleet)
	};

	/** Why a card's DelayHazard cannot buy time on a hazard (B4 section 2.4), in the order ResistBlockOf asks. Not a UENUM:
	 * never saved, hashed or a data column. */
	enum class ECosmicResistBlock : uint8
	{
		None,
		NoHazard,  // no such system, or it holds no hazard of the tag
		NotOwn,    // no such people, or the system is not the people's
		Capital,   // the hazard is a Capital row's (a rebellion: a people's discontent is not put off by force)
		NoFleet,   // the fleet's cap is 0 (the first rung has no fleet - GuardBlockOf's NoFleet)
		Spent,     // a cap but no fleet now (sieges, attrition, an unpaid upkeep, an earlier resistance)
	};

	/** The settled systems the people CivIndex owns (entries of OwnedSystems whose owner is the people). */
	COSMIC_ASCENSION_API int32 SettledCountOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex);

	/** The systems the people CivIndex owns, settled or being settled (the count the realm cap reads). */
	COSMIC_ASCENSION_API int32 OwnedCountOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex);

	/** DT_Rules.FleetPerSettledByTier[Tier] x SettledCountOf (an index the array lacks, or a value that is not above 0 and
	 * finite, reads as 0). */
	COSMIC_ASCENSION_API float FleetCapOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex, const FCosmicRulesDef& Rules);

	/** The product, in ActiveEdicts order, of the held edicts' FleetGrowthMultiplier (a factor that is not above 0 and
	 * finite reads as 1 - the schema keeps it above 0). */
	COSMIC_ASCENSION_API float FleetGrowthMultiplierOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables);

	/** Power x DT_Rules.FleetEffectivenessByTierGap[Clamp(TierGap, 0, 2)] - the gap is how far the other side stands above
	 * (never below 0). A power that is not above 0, or a factor that is not at least 0 and finite, gives 0. */
	COSMIC_ASCENSION_API float EffectivePowerOf(float Power, int32 TierGap, const FCosmicRulesDef& Rules);

	/** How far the owner of SystemIndex stands above the people Attacker (0 when it does not). */
	COSMIC_ASCENSION_API int32 AttackGapOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 Attacker, int32 SystemIndex);

	/** The people whose fleet besieges SystemIndex (its target is that system and it is not the owner), the first in Civs
	 * order; INDEX_NONE when none does. */
	COSMIC_ASCENSION_API int32 BesiegerOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 SystemIndex);

	/** Whether the target of the people CivIndex is a system another people owns (its fleet besieges). */
	COSMIC_ASCENSION_API bool IsBesieging(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex);

	/** The first reason the people Attacker cannot besiege SystemIndex, Sanctuary .. NotHostile. bAssumeHostile skips the
	 * last question (the AI asks whether a system to aim at exists before it turns hostile). */
	COSMIC_ASCENSION_API ECosmicSiegeBlock SiegeBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules,
		int32 Attacker, int32 SystemIndex, bool bAssumeHostile = false);

	/** The first reason the people CivIndex cannot guard SystemIndex, Sanctuary .. NoFleet. */
	COSMIC_ASCENSION_API ECosmicGuardBlock GuardBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules,
		int32 CivIndex, int32 SystemIndex);

	/** A guard's reason in the siege's names, as AllocateFleet answers it (NotOwn is NoSystem: no system of the people). */
	COSMIC_ASCENSION_API ECosmicSiegeBlock SiegeNameOf(ECosmicGuardBlock Block);

	/** The ticks the guard of the people Owner adds to a new hazard against a source Gap rungs above it: DT_Rules.FleetDelayPerPower
	 * x EffectivePowerOf(its FleetPower, Gap), at most FleetDelayCap (and 100000), cut to int32; 0 for no fleet. FleetDelayOf's
	 * arithmetic, and the system panel's delay line (gap 0). */
	COSMIC_ASCENSION_API int32 GuardDelayTicksOf(const FCivilizationState& Owner, int32 Gap, const FCosmicRulesDef& Rules);

	/** The share of its fleet a repulsed attacker keeps: DT_Rules.SiegeRepulsedFleetFactor when in [0, 1], else 0 (the
	 * conflict stage, and [Send the fleet]'s tooltip). */
	COSMIC_ASCENSION_API float RepulsedShareOf(const FCosmicRulesDef& Rules);

	/** The attacker's odds: A = its effective power against the owner, D = the owner's FleetPower x its defence share
	 * (SiegeDefenceShares: [0] its target is this system, [1] it has none, [2] another). 0 when A is not above 0, 1 when D
	 * is not above 0, else A / (A + D). */
	COSMIC_ASCENSION_API float SiegeOddsOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, int32 Attacker, int32 SystemIndex);

	/** Whether the siege takes the system: Number < Odds (a tie stays with the defender). */
	COSMIC_ASCENSION_API bool Takes(float Number, float Odds);

	/** Whether the siege of Civ has reached its end at Tick: Tick >= FleetTargetSinceTick + Max(1, SiegeTicks) (reached or
	 * passed - a save loaded with a shorter knob ends at the next conflict stage). */
	COSMIC_ASCENSION_API bool IsSiegeDue(const FCivilizationState& Civ, int64 Tick, const FCosmicRulesDef& Rules);

	/** The tick the siege of Civ ends at (FleetTargetSinceTick + Max(1, SiegeTicks)). */
	COSMIC_ASCENSION_API int64 SiegeEndTickOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules);

	/** The band of odds read from the ruled people's side: 2 ahead (>= 0.6), 1 even, 0 behind (<= 0.4). Constants. */
	COSMIC_ASCENSION_API int32 OddsBandOf(float OursOdds);

	/**
	 * The countdown a hazard of Row placed on SystemIndex gains from the owner's fleet (B3 section 2.7 - at placement only):
	 * only when the owner's fleet targets that system and the row is not a Capital row (a people's discontent is not put off
	 * by force), FleetDelayPerPower x the owner's effective power across the gap to the hazard's source (a claim: the
	 * claimant's tier, CosmicHazards::ClaimantOf; any other row: 0), within [0, Min(FleetDelayCap, 100000)] by comparisons,
	 * then cut to int32. 0 otherwise.
	 */
	COSMIC_ASCENSION_API int32 FleetDelayOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
		const FCosmicRulesDef& Rules, int32 SystemIndex, const FHazardDef& Row);

	/** Empties every people's fleet target on SystemID, FleetTargetSinceTick = Tick (CosmicHazards::Transfer and Destroy). */
	COSMIC_ASCENSION_API void LiftTargetsOn(FGalaxyState& State, FName SystemID);

	/**
	 * The first reason the people CivIndex cannot buy time with its fleet on the hazard Tag of SystemIndex (B4 section 2.4 -
	 * a card's DelayHazard), NoHazard .. Spent. The one gate: the effect (UCosmicEventSubsystem::Apply) and the card's
	 * choice gate (CosmicPetitions::ChoiceGateOf - the answer, the card window, the policy) read it alone. The hazard
	 * matches Tag as the conditions match one (MatchesTag); an invalid Tag is NoHazard. The tier gap is not asked.
	 */
	COSMIC_ASCENSION_API ECosmicResistBlock ResistBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
		const FCosmicRulesDef& Rules, int32 CivIndex, int32 SystemIndex, const FGameplayTag& Tag);

	/** The share of its fleet a people keeps after a card's DelayHazard: DT_Rules.ResistFleetFactor when in [0, 1], else 1
	 * (comparisons - a value that is not a number is 1). */
	COSMIC_ASCENSION_API float ResistShareOf(const FCosmicRulesDef& Rules);
}

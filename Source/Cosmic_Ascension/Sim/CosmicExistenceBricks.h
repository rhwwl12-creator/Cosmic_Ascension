// Cosmic Ascension - Sim layer (Core Logic). Design: docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md
// section 2 (B7x slice ga - rules R1, R2, R10; walls K1 - K10), docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md.
// The single home of the existence bricks' world rules (rule 13): the count of the six time vectors, the shares a brick
// (a Brick.* tag of FCivilizationState::Traits) takes in the economy, research, diplomacy, crisis and project steps, the
// inward peoples' settle refusal, the AI mutation step and the end shape. Pure functions of the state, the index and the
// rules - no draw, no line, no knowing read (rule 2), no delegate; the few writers are named as such. Every share is read
// through a DT_Rules knob whose C++ initializer is inert (a multiplier 1, a flat 0, a period 0), and every reader asks
// HasTagExact first, so a people without the brick takes the old path bit for bit.
// RULE (section 6.1): files under Sim/ must never include UUserWidget or AActor headers; a Sim header includes no Events header.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

struct FCivilizationState;
struct FCosmicRulesDef;
struct FCosmicStateIndex;
struct FCosmicTimeVectors;
struct FGalaxyState;
struct FResourcePool;

namespace CosmicBricks
{
	// ---- The six time vectors (section 2.2) ----

	/** Which of the six facts hold for one people at the end of this tick's economy stage (section 2.2). Never stored. */
	struct FCosmicTimeFacts
	{
		// A system the people owns has a megaproject in progress other than project_settle (construction or response slot).
		bool bBuilt = false;
		// Its fleet is aimed at a system another people owns, or it holds the edict DT_Rules.StartingEdict.
		bool bConsumed = false;
		// A system the people owns is being settled (project_settle in its construction slot).
		bool bDrifted = false;
		// One of its ties carries a treaty or the friendly stance.
		bool bLinked = false;
		// It has exactly one settled system and no tie with a treaty or a stance that is not neutral.
		bool bIsolated = false;
		// PressureEnduredTicks or LawsHeldTicks is above 0, or a hazard stands on one of its settled systems.
		bool bEndured = false;
	};

	/**
	 * The facts of the people CivIndex now (section 2.2): one walk over its OwnedSystems (an entry whose owner is another
	 * people counts for nothing, as in the economy stage), its ties, its fleet target and its edicts. All false for an index
	 * out of range. Reads only; draws nothing; reads no knowing.
	 */
	COSMIC_ASCENSION_API FCosmicTimeFacts TimeFactsOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, int32 CivIndex);

	/**
	 * The count of the time vectors (section 2.2, rule R2): for every people, in Civs order, each vector whose fact holds
	 * (TimeFactsOf) grows by one (never past MAX_int32); none is ever reset. The sub-step right after the streak counters at the
	 * end of the economy stage (UGalaxySimulationSubsystem::TickEconomy), so Endured reads this tick's PressureEnduredTicks
	 * and LawsHeldTicks. Writes FCivilizationState::TimeVectors only. No draw, no knowing read, no line.
	 */
	COSMIC_ASCENSION_API void CountTimeVectors(FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules);

	/**
	 * The vector a word names - the Key of condition CivTimeVectorAtLeast (schema words.time_vectors: Built, Consumed,
	 * Drifted, Linked, Isolated, Endured): true and its ticks in OutTicks, or false (OutTicks 0) for any other word.
	 */
	COSMIC_ASCENSION_API bool VectorOf(const FCosmicTimeVectors& Vectors, FName Word, int32& OutTicks);

	// ---- The shares (section 2.4) - each 'no change' for a people without its brick or under its inert knob ----

	/**
	 * The bricks' part of the income of the people CivIndex (the economy stage, right before IncomePerTick is cached):
	 * Brick.Diet.StarEater multiplies Income.Energy by BrickStarEaterEnergyMultiplier; then Brick.Diet.DarkMatter adds
	 * BrickDarkMatterEnergyPerTick and Brick.Diet.DistanceTension adds BrickSpanEnergyPerLane x SpanOf to Energy;
	 * Brick.Diet.IdleFeed adds BrickIdleInsightPerTick to Insight on an idle tick (IsIdleOf) and Brick.Diet.CrisisParasite
	 * adds BrickParasiteInsightPerHazard x ParasiteHazardsOf to Insight. Income is not touched at all for a people that
	 * carries none of them, or whose knobs are inert. No draw.
	 */
	COSMIC_ASCENSION_API void AddIncomeShares(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, int32 CivIndex, FResourcePool& Income);

	/** The widest lane-count distance between two settled systems of the people CivIndex (CosmicState::LaneDistancesFrom - a
	 * pair in two galaxy rows, or that no lane joins, counts for nothing); 0 with fewer than two. Reads only. */
	COSMIC_ASCENSION_API int32 SpanOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex);

	/** Whether this is an idle tick for the people CivIndex: none of its settled systems holds a hazard or a project (either
	 * slot), its fleet besieges no system of another people, and no other people's fleet is aimed at one of its settled
	 * systems. False for an index out of range. Reads only. */
	COSMIC_ASCENSION_API bool IsIdleOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex);

	/** Whether a hazard tag is one no brick touches: Hazard.Claim, Hazard.Extinction, Hazard.PreSupernova (as a galaxy law's
	 * hooks pass them by - schema, B6b header 3.3; K9: nothing new delays the claim). */
	COSMIC_ASCENSION_API bool IsSparedHazard(const FGameplayTag& Hazard);

	/** The hazards a crisis parasite CivIndex feeds on now: those standing, not spared (IsSparedHazard), on the settled
	 * systems of every people that borders it (CosmicDiplomacy::BordersOf). Reads only. */
	COSMIC_ASCENSION_API int32 ParasiteHazardsOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex);

	/** The most the crisis parasites add to the countdown of one hazard of tag Hazard (ParasiteCountdownBonusOf - once a
	 * hazard, whatever the number of parasites): 0 for an invalid or spared hazard (IsSparedHazard), else
	 * BrickParasiteCountdownBonusTicks cut to 0..100000. The bound the data test of the longest stay adds (TheodicyProblems).
	 * Reads only. */
	COSMIC_ASCENSION_API int32 ParasiteBonusBoundOf(const FCosmicRulesDef& Rules, const FGameplayTag& Hazard);

	/**
	 * The parasite's countdown bonus for a hazard of tag Hazard placed now on SystemIndex (CosmicHazards::Place, with the
	 * edicts' bonus and the fleet's delay): ParasiteBonusBoundOf when the system's owner borders a people that carries
	 * Brick.Diet.CrisisParasite, once whatever their number; 0 for a spared hazard, an unowned system, an index out of range,
	 * or a knob of 0 (then no border is walked). Reads only.
	 */
	COSMIC_ASCENSION_API int32 ParasiteCountdownBonusOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, int32 SystemIndex, const FGameplayTag& Hazard);

	/** The factor of the people's research progress (the research stage): BrickSwarmResearchMultiplier for
	 * Brick.Instinct.SwarmTech times BrickInwardResearchMultiplier for an inward people (IsInward); exactly 1 for a people
	 * with neither, or under inert knobs. A knob not above 0, or not a number, reads as 1. */
	COSMIC_ASCENSION_API float ResearchMultiplierOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules);

	/** One tick's progress of a settlement of the people (CosmicMegaprojects::Advance, project_settle only): Step x
	 * BrickNomadSettleProgressMultiplier for Brick.Space.Nomad, else Step itself. A knob not above 0 reads as 1. */
	COSMIC_ASCENSION_API float SettleProgressOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules, float Step);

	/** The spawn share of the Draw hazard of tag Hazard on a system of Owner (the crisis stage): Share x
	 * BrickInwardHazardSpawnMultiplier for Brick.Space.FoldedSmall (never the claim, the extinction or the supernova
	 * precursor: IsSparedHazard), then x BrickStellarCrisisSpawnMultiplier when Hazard is Hazard.PreSupernova and Owner
	 * carries Brick.Form.ShutSphere or Brick.Diet.StarEater (once, whichever it carries); else Share itself, bit for bit. A
	 * knob not above 0 reads as 1 (never 0). */
	COSMIC_ASCENSION_API float HazardSpawnShareOf(const FCivilizationState& Owner, const FCosmicRulesDef& Rules, const FGameplayTag& Hazard, float Share);

	/**
	 * The term the bricks of the people Seen add to the drift target of every view of it (CosmicDiplomacy::TargetRelationOf
	 * - the diplomacy stage moves a bordering pair's view toward that target; no other people's state is written here):
	 * BrickStarEaterRelationTarget for Brick.Diet.StarEater plus BrickMergeRelationTarget for Brick.Instinct.HiveMerge, each
	 * read in [-1, 1] (anything else, or not a number: 0). Exactly 0 for a people with neither, or under the inert knobs.
	 */
	COSMIC_ASCENSION_API float RelationTargetTermOf(const FCivilizationState& Seen, const FCosmicRulesDef& Rules);

	// ---- The settle gate (CosmicMegaprojects::SettleBlockOf - reason Inward) ----

	/** Whether the people is folded inward: it carries Brick.Space.ClosedPort or Brick.Space.FoldedSmall. Such a people
	 * starts no further settlement (CosmicMegaprojects::ECosmicSettleBlock::Inward); what it holds stays. Inward here means
	 * the two folded-in peoples (Brick.Space.ClosedPort / FoldedSmall) only; it does not read Brick.Mutation.InwardHunger,
	 * a different fact (S11). */
	COSMIC_ASCENSION_API bool IsInward(const FCivilizationState& Civ);

	// ---- The AI mutation step (section 2.6) ----

	/**
	 * The first mutation brick due for the people, in the fixed order StarTow, DustBand, InwardHunger; the empty
	 * tag when none is, or when BrickMutationThresholdTicks is not above 0. Each is due once (not while the people carries it)
	 * when its prerequisite brick is carried and its vectors reach the threshold: Brick.Mutation.StarTow - Brick.Space.Nomad
	 * and Drifted; Brick.Mutation.DustBand - Brick.Instinct.SwarmTech, Built and Linked; Brick.Mutation.InwardHunger -
	 * Brick.Diet.StarEater and Consumed. Reads only; never a rung (K4).
	 */
	COSMIC_ASCENSION_API FGameplayTag MutationDueOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules);

	/** Whether the AI mutation step runs at Tick: BrickEvolutionPeriodTicks above 0, Tick above 0 and a multiple of it. */
	COSMIC_ASCENSION_API bool MutationStepDueAt(const FCosmicRulesDef& Rules, int64 Tick);

	/**
	 * The AI mutation step (the end of the AI stage, before the feat judgment): every people the player does not rule, in
	 * Civs order, takes the brick MutationDueOf names - added to its Traits as the effect AddTrait adds a tag. The player's
	 * people changes only through cards. Returns how many peoples took one. No draw, no line.
	 */
	COSMIC_ASCENSION_API int32 MutateAIPeoples(FGalaxyState& State, const FCosmicRulesDef& Rules);

	// ---- The seed bricks of the AI peoples (NewGalaxy - draw-free, off while DT_Rules.BrickSeedTags is empty) ----

	/**
	 * The brick NewGalaxy adds to the AI people Civ right after its archetype's Traits are copied: none (the empty tag) when
	 * DT_Rules.BrickSeedTags holds no Brick.* tag or the people already carries a Brick.* tag; else the seed tag at
	 * CosmicSeed::DeriveForName(Seed, Civ.CivID, Purpose::Brick, 0) modulo their number, the tags taken in FNameLexicalLess
	 * order of their names (so the pick does not follow the container's order). A derived integer - the sim stream is not
	 * drawn. Pure.
	 */
	COSMIC_ASCENSION_API FGameplayTag SeedBrickOf(const FCosmicRulesDef& Rules, int32 Seed, const FCivilizationState& Civ);

	// ---- The star eater's dimming (rule R10) - implemented and tested, not wired into a stage yet ----

	/** The next dimmer of the seven Star.Class.* tags (CosmicTags::StarClassAt, hottest first); the dimmest stays itself, and
	 * an empty tag or one outside the seven comes back unchanged (never an eighth class, never a destruction). */
	COSMIC_ASCENSION_API FGameplayTag DimmerStarClassOf(const FGameplayTag& StarClass);

	/** Whether the star eater's capital dims at Tick: the people carries Brick.Diet.StarEater, BrickStarEaterDimPeriodTicks is
	 * above 0, and Tick is above 0 and a multiple of it. */
	COSMIC_ASCENSION_API bool StarDimDueOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules, int64 Tick);

	/** The one world function that dims a star (R10): the capital of the people CivIndex - when it owns it and it is settled,
	 * living and not the sanctuary - takes DimmerStarClassOf its class. True when the class changed. */
	COSMIC_ASCENSION_API bool DimCapitalStar(FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex);

	// ---- The end shape (section 2.7) - display only, never saved, never an ending kind ----

	/**
	 * The shape a people's life took. Four for a people without a shape brick, by its largest vector (ties broken by the
	 * fixed order Built, Consumed, Drifted, Linked, Isolated, Endured), and eight by shape brick. Never saved and never
	 * compared by its numeric value; the presentation maps it to ST_CA Shape.* (CosmicUI::ShapeKey - [B-7x3]).
	 */
	enum class ECosmicShape : uint8
	{
		// Largest vector Built or Linked.
		BuiltOrLinked,
		// Largest vector Consumed or Drifted.
		ConsumedOrDrifted,
		// Largest vector Isolated.
		Isolated,
		// Largest vector Endured.
		Endured,
		// Brick.Form.KeptLaw.
		KeptLaw,
		// Brick.Space.FoldedSmall.
		FoldedSmall,
		// Brick.Space.ClosedPort or Brick.Form.ShutSphere.
		ShutHouse,
		// Brick.Form.LivingNebula or Brick.Diet.DarkMatter.
		SpreadBody,
		// Brick.Space.Nomad.
		StarHerd,
		// Brick.Instinct.SwarmTech.
		WovenSwarm,
		// Brick.Diet.StarEater.
		LastHunger,
		// Brick.Instinct.HiveMerge.
		Entwined,
		// The number of the values above (not a shape).
		Count,
	};

	/**
	 * The end shape of the people (section 2.7): the first shape brick it carries in the priority KeptLaw, FoldedSmall,
	 * ShutHouse, SpreadBody (Brick.Form.LivingNebula or Brick.Diet.DarkMatter), StarHerd, WovenSwarm, LastHunger, Entwined
	 * (HasTagExact); else the shape of its largest vector.
	 * Pure; reads the people alone.
	 */
	COSMIC_ASCENSION_API ECosmicShape ShapeOf(const FCivilizationState& Civ);
}

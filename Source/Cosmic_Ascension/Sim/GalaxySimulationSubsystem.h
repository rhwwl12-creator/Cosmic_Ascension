// Cosmic Ascension - Sim layer. Design: guideline.md section 6.1, 7.4, 9.2.
// RULE (section 6.1): files under Sim/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Math/RandomStream.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Sim/CosmicStateIndex.h"
#include "GalaxySimulationSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSimTick, int64, Tick);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGalaxyGenerated, int32, Seed);

struct FCosmicEvalContext;

/** How the sphere of one transcendent reaches a system (section 4.2). */
enum class ECosmicSphereReach : uint8
{
	// Neither in the sphere nor next to it.
	None,
	// One lane from a system of the sphere.
	Adjacent,
	// In the sphere (FTranscendentState.InfluencedSystems).
	Inside
};

/** Who presses on a system (section 4.2). */
enum class ECosmicPressureSource : uint8
{
	None,
	// An unsealed transcendent whose sphere holds the system or lies one lane from it.
	Transcendent,
	// The civilization that owns the system, whoever rules it.
	Civilization,
	// The vessel as a wanderer, standing in the system.
	Wanderer
};

/**
 * Spiritual pressure on one system (section 4.2): what UGalaxySimulationSubsystem::ComputePressureIn returns.
 * Derived whenever it is asked for: never saved, never hashed, and there is no meter behind it.
 */
struct FCosmicPressure
{
	// P: the sum of the terms of all sources. 0 when nothing of tier 3 or above reaches the system.
	float P = 0.f;

	// The source of the largest single term. Equal terms go to the transcendents in the order of
	// FGalaxyState.Transcendents, then to the civilization, then to the wanderer. None when P is 0.
	ECosmicPressureSource DominantSource = ECosmicPressureSource::None;

	// TranscendentID or CivID of that source. NAME_None when P is 0, and for the wanderer: a vessel has no ID.
	FName DominantSourceID;

	// Tier of that source. Read it only when DominantSource is not None.
	EAscensionTier DominantTier = EAscensionTier::Tier1_Planetary;
};

/**
 * Fixed-step simulation of one run (section 7.4). Owns the whole runtime state (FGalaxyState) and the sim random stream.
 * One tick = one galaxy cycle; at 1x speed one second is one tick (section 2.1).
 * Stage order inside a tick is fixed, ten stages: economy, research, diplomacy, conflict, hazards, civ AI, crossings,
 * player travel, sight, role effects. Events, feats and per-tick pressure have no stage of their own: they are fixed
 * steps inside the economy, research, hazard and civ AI stages (see the stage declarations). Change guideline.md
 * section 7.4 before changing any of this.
 */
UCLASS()
class COSMIC_ASCENSION_API UGalaxySimulationSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// Tickable.h rule for UObjects: construct with Never, enable at the end of Initialize, back to Never in Deinitialize.
	UGalaxySimulationSubsystem() : FTickableGameObject(ETickableTickType::Never) {}

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return bRunning && TimeScale > 0.f; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UGalaxySimulationSubsystem, STATGROUP_Tickables); }

	/**
	 * Replaces the state with a new home galaxy generated from Seed and the definition tables, then installs the first
	 * life as the ruler of the player's people (DT_Rules.PlayerArchetypeRow, home system "sol") through
	 * UPlayerRoleSubsystem::Install (section 5.10, 7.4).
	 *
	 * Numbers. A count of 0 or less is read from the home row (GetHomeGalaxy: LivingWorlds, Peoples), so no caller
	 * keeps a copy of those numbers. Systems: at least 1, and no upper bound here (the actor budget of 150 belongs to
	 * the presentation layer). Peoples: at least 1, at most the number of systems.
	 *
	 * What it makes. Systems "sol" and "sys_<n>", civilizations "civ_<n>" from State.NextCivSerial, no padding; every
	 * system and civilization carries DT_Rules.HomeGalaxyRow in GalaxyRow. sol lies at HomeRadiusShare of the disc
	 * radius, angle 0; every other system at a point drawn in the square and thrown away while it lies outside the
	 * disc, at most 16 tries, the sixteenth halved when it misses too. No trigonometry: only + - x / and the square
	 * root reach a hashed value (section 9.2). Lanes join systems of the one galaxy. One lane neighbour of sol is the
	 * home of the first AI people. Names are final and flattened here: nothing is named later.
	 *
	 * The draws, in this order and no other (section 9.2). Kinds of draw are made in blocks:
	 * 1. Per system, in array order: its place (none for sol; two draws per try), its star class (one draw against
	 *    DT_Rules.LivingWorldStarShares), its richness (one draw). sol draws both too; its richness is always 1, and
	 *    the home of any people whose archetype has a HomeStarClass takes that class instead of the one it drew.
	 * 2. Homes: the first AI people among the lane neighbours of sol (one draw, none when there is one people), then
	 *    one draw per further AI people among the systems still free.
	 * 3. Per AI people, in CivID order: archetype (one draw over the DT_Civs rows of positive Weight in row-name
	 *    order), ethics (three draws: pacifism, spiritualism, openness), AI profile (one draw).
	 * 4. Names. Per people, the player's first: the home system (one draw: HomeSystemNames of the archetype when it
	 *    holds any, else the next name of FGalaxyDef.StarNameBank without replacement) and the people (one draw
	 *    over NameBank of the archetype, {Star} = the home). Then per unowned system in array order: one draw for a
	 *    pair of FGalaxyDef.CatalogPrefixes x CatalogStems, without replacement.
	 * 5. The fallen roster (section 4.2): the instance of CosmicRows::TransSilence when DT_Transcendents holds that row
	 *    as a Realm.Abyssal row, then DT_Rules.FallenTranscendentCount - 1 draws, each picking one more Realm.Abyssal
	 *    row by SpawnWeight among the rows not active yet, in row-name order (a row whose weight is not above zero or
	 *    not finite is no candidate). The knob is read as 0..3, with one error line when it lies outside. Celestial
	 *    and Independent rows get no instance.
	 * 6. Starting cores (section 5.7): one draw per place of the roster. An instance whose row has
	 *    InitialInfluencedSystems above 0 gets one system: an unowned system at least
	 *    DT_Rules.FallenCoreMinLanesFromHome lanes from sol, one lane from the home of an AI people and in no sphere
	 *    yet, picked among the candidates in array order. Without a candidate: the unowned system in no sphere that
	 *    lies farthest from sol in lanes, the lowest SystemID (FNameLexicalLess) among equals. Without an unowned
	 *    system the sphere starts empty.
	 * 7. The first life: the name of the vessel (one draw, in Install).
	 * Every draw is made whether or not it is used: a people whose archetype has fixed ethics, a profile or an empty
	 * bank, a place of the roster without an instance, a row without a core all draw and throw the number away. The
	 * number of draws therefore follows Seed, the two counts and the roster knob, never what the tables hold: adding
	 * a row or filling a bank moves no system and no home.
	 *
	 * Stamps State.TellRngState with (int32)CosmicSeed::Derive(Seed, "", Tell, 0) before the first line is told.
	 * Tick 0 is this generated state; the first simulated tick is 1.
	 * Refused with one error line inside a handler of a Core Logic delegate (IsBroadcasting): the state is not replaced
	 * under a stage or a row that is still running.
	 */
	UFUNCTION(BlueprintCallable)
	void NewGalaxy(int32 Seed, int32 SystemCount, int32 CivCount);

	// The UI offers 0, 1, 2, 5, 10 (section 2.1). 0 pauses, and it is the only pause of the simulation: this subsystem
	// ticks in the engine's null-world pass, so engine pause (SetGamePaused) and time dilation never reach it.
	UFUNCTION(BlueprintCallable)
	void SetTimeScale(float Scale);

	UFUNCTION(BlueprintPure)
	float GetTimeScale() const { return TimeScale; }

	// One tick, with broadcasts as configured. Debug and tests. Refused with one error line inside a handler of a Core
	// Logic delegate (IsBroadcasting): a handler may only read the state (section 9.4).
	UFUNCTION(BlueprintCallable)
	void StepOnce();

	// Runs Ticks ticks with the broadcast gate closed, then restores the gate. Tests, balance runs, interregnum leap (section 4.4).
	// Not yet: the per-tick ending-condition check that stops a leap early arrives with the endings (section 4.4, 7.4).
	// Refused like StepOnce inside a handler of a Core Logic delegate.
	UFUNCTION(BlueprintCallable)
	void RunHeadless(int64 Ticks);

	// Read by C++ (widget bases, other subsystems) through the reference. Not exposed to Blueprint: a graph could read no
	// field of FGalaxyState and every node evaluation would copy the whole state.
	const FGalaxyState& GetState() const { return State; }

	FGalaxyState& GetMutableState() { return State; }

	// The sim stream (section 9.2): every draw that changes saved world state. Never use FMath::Rand*.
	FRandomStream& GetRng() { return Rng; }

	/**
	 * The tell stream (section 9.2): one number in [0, Count) for the choice of a sentence variant, and nothing else.
	 * Opens a local stream on State.TellRngState, draws once and writes the state back, so the saved value is always
	 * current: there is no live stream object and no getter. Never draws from the sim stream, and no rule reads the
	 * result. Count < 1 is a caller error: one error line, no draw, returns 0.
	 */
	uint32 DrawTell(int32 Count);

	// Copies the live stream position into State.RngState. NewGalaxy and every tick end with it. Code that draws from GetRng()
	// outside a tick, and the save subsystem right before saving, call it themselves.
	void SyncRngState();

	// DT_Rules single row. C++ defaults when the table or its row is missing.
	const FCosmicRulesDef& GetRules() const { return Rules; }

	// The DT_Galaxies row of the home galaxy (DT_Rules.HomeGalaxyRow). C++ defaults when the table or the row is
	// missing; a table that lacks the row is reported with one error line, like DT_Rules.
	const FGalaxyDef& GetHomeGalaxy() const { return HomeGalaxy; }

	// The definition tables (section 7.3), loaded from their fixed asset paths in Initialize.
	const FCosmicDataTables& GetTables() const { return Tables; }

	// Replaces the tables and re-reads the rules row and the home galaxy row. Tests inject transient tables here.
	void SetTables(const FCosmicDataTables& InTables);

	// Binds the live state, the sim stream, the state index, the tables and the rules into an evaluation context
	// (section 7.5, Events/CosmicEventSubsystem.h). Stage code that applies effects or fires a row by name builds its
	// context here (the event slots are handed the state and the stream and build their own); a caller outside a tick
	// (encounter resolution, Phase 4) does the same and calls SyncRngState() afterwards. Reports an error when called
	// while IsBroadcasting(): a handler must not evaluate anything.
	FCosmicEvalContext MakeEvalContext();

	// ID -> index for Civs and Systems (section 7.4, 9.4). Rebuilt by NewGalaxy, LoadState and the crossing stage.
	const FCosmicStateIndex& GetStateIndex() const { return StateIndex; }

	// For code that changed the number of elements of Civs or Systems through GetMutableState (tests).
	void RebuildStateIndex() { StateIndex.Rebuild(State); }

	// Counts the Core Logic delegates that are being broadcast right now and that can fire from inside a tick stage:
	// OnLogEntry, OnEventFired, OnTierAscended (section 9.4). Their handlers may read the state and call SetTimeScale
	// and nothing else, so UCosmicEventSubsystem refuses its member functions while this is true, and so do NewGalaxy,
	// LoadState, StepOnce and RunHeadless here, UCosmicSaveSubsystem::LoadGalaxy, UAscensionSubsystem::EvaluateFeats and
	// the role changes and commands of UPlayerRoleSubsystem (Install, DeposeRuler, BecomeWanderer, StartMegaproject,
	// StartNextMegaproject). MakeEvalContext reports an error and still returns the context (it holds references): the
	// static Apply and ApplyAll are kept out of a handler by the rule alone. Every such broadcast site brackets its
	// Broadcast call with BeginBroadcast and EndBroadcast.
	bool IsBroadcasting() const { return BroadcastDepth > 0; }
	void BeginBroadcast() { ++BroadcastDepth; }
	void EndBroadcast() { --BroadcastDepth; }

	/**
	 * Spiritual pressure P on a system and the ID of its dominant source (section 4.2), over the live state. An unknown
	 * SystemID gives 0 and NAME_None. For the presentation layer and for callers that want the ID alone; stage code and
	 * the evaluator call ComputePressureIn, which also says what kind of source it is and of which tier.
	 */
	UFUNCTION(BlueprintPure)
	float ComputeSpiritualPressure(FName SystemID, FName& OutDominantSourceID) const;

	/**
	 * The same over any state: what the member forwards to and what UCosmicEventSubsystem::Check calls with the state
	 * of its view (SystemPressureAtLeast). A pure function of its arguments: world state only, never Knowing, no draw.
	 * P is the sum of the terms below. A term is DT_Rules.PressureBySourceTier[tier of the source] x reach, so a
	 * source below tier 3 adds nothing: the lower rungs do not press on the higher (section 4.2).
	 * - Every unsealed transcendent, in the order of State.Transcendents: the Tier of its DT_Transcendents row; reach 1
	 *   when the system is in its sphere, DT_Rules.PressureAdjacencyFalloff when it is one lane from it, 0 otherwise.
	 *   This term alone is also scaled by (1 + Depth) of the row, Depth read clamped to 0..1: the depth inside the tier
	 *   (section 3.2). The gap inside one tier is therefore at most a factor of two, and a transcendent never presses
	 *   harder than the plain weight of the tier above it. An instance whose row the tables do not hold adds nothing.
	 * - The civilization that owns the system: its Tier, reach 1. A ruling vessel adds no term of its own: the
	 *   civilization it rules presses with its Tier like any other, with or without a living ruler.
	 * - The vessel as a wanderer: Calling.PersonalTier on the system it stands in, reach 1, and only while
	 *   Calling.LifeState is Embodied (during the interregnum FPlayerEntityState still holds the dead vessel).
	 * A SystemIndex out of range gives the zero result. Only + and x reach the value (section 9.2).
	 */
	static FCosmicPressure ComputePressureIn(const FGalaxyState& InState, const FCosmicStateIndex& InIndex, const FCosmicDataTables& InTables, const FCosmicRulesDef& InRules, int32 SystemIndex);

	// How the sphere of Transcendent reaches a system: the pressure function and the weight factor of the event slots
	// ask here. The economy stage builds one table of the same reach for the tick instead (section 9.4): the two agree
	// for every SystemID the state holds, and Sim.PressurePerCivilization holds them together for a people inside a
	// sphere, one lane from it and out of its reach. Reads the Lanes of the system itself, by SystemID (no index is
	// needed); whether the transcendent is sealed is the caller's question. An index out of range is None.
	static ECosmicSphereReach SphereReachIn(const FGalaxyState& InState, const FTranscendentState& Transcendent, int32 SystemIndex);

	// Population band (section 1.3, 8.3). Derived, never saved, never hashed: civ Tier baseline, adjusted by Population against
	// DT_Rules.PopulationBandThresholds. Unknown CivID returns Few.
	UFUNCTION(BlueprintPure)
	EPopulationBand GetPopulationBand(FName CivID) const;

	// Global broadcast gate (section 6.1). Every Core Logic subsystem checks it before broadcasting. Closed during RunHeadless.
	bool IsBroadcastEnabled() const { return bBroadcastEnabled; }
	void SetBroadcastEnabled(bool bEnabled) { bBroadcastEnabled = bEnabled; }

	UPROPERTY(BlueprintAssignable)
	FOnSimTick OnSimTick;

	// Fired after NewGalaxy and after LoadState: the presentation layer rebuilds itself from the state.
	UPROPERTY(BlueprintAssignable)
	FOnGalaxyGenerated OnGalaxyGenerated;

	// Called by the save subsystem. Replaces the state and restores the random stream from State.RngState.
	void LoadState(const FGalaxyState& In);

	/**
	 * Canonical hash of the whole state, knowing and telling included; the world alone is ComputeWorldHash (section 9.2).
	 * Walks FGalaxyState by reflection, so a new field is hashed without touching this function. Skips every FText and
	 * GalaxyID, hashes FName and FGameplayTag as lower-cased strings, TMap entries in FNameLexicalLess key order, tag
	 * containers by their explicit tags, and the live sim stream instead of State.RngState (TellRngState is hashed as
	 * stored: it is always current). Stable across processes of the same build.
	 */
	uint32 ComputeStateHash() const;

	/**
	 * World hash (section 8.1, 9.2): ComputeStateHash without the top-level members Knowing, Log, Chronicle and
	 * TellRngState. Measures that knowing and telling never change the world; builds are compared by this value.
	 */
	uint32 ComputeWorldHash() const;

private:
	// Advances State.Tick first, then runs the ten stages in the fixed order and syncs the random stream state.
	void SimulateTick();

	// Stage 1. Two loops over Civs in array order.
	// Loop one, per civilization: the population regrowth (DT_Rules.PopulationRegrowthPerTick: below 1.0 it rises by the
	// knob up to 1.0; above 1.0, or with a knob not above 0 or not a number, nothing); income of its settled systems into
	// IncomePerTick (a system being settled pays nothing, section 5.7); then the per-tick sources, the only effects that
	// may scale IncomePerTick or act every tick; then IncomePerTick into the Stockpile. The per-tick sources:
	// - The drain of spiritual pressure, once per civilization and never once per system (section 16 #55): Stability
	//   goes down by DT_Rules.SpiritualPressureStabilityDrain x the largest P among its settled systems whose dominant
	//   source has a Tier above the civilization's own.
	// - FTranscendentDef.PressureEffectsPerTick of every unsealed transcendent of a Tier above the civilization's whose
	//   sphere holds one of its settled systems: once per transcendent, in the order of State.Transcendents, through
	//   UCosmicEventSubsystem::ApplyAll with bPerTickSource. Subject = the civilization and the first such system in
	//   OwnedSystems order; OtherID and SourceRow = the TranscendentID. A people that only borders the sphere is drained
	//   and takes none of these effects.
	// - Edict and trait rows, from Phase 4.
	// A per-tick source applies the seven effects that change an amount and no other (UCosmicEventSubsystem::Apply), so
	// this stage makes no draw and records no role effect, whatever the rows hold.
	// The pressures are read from one table of sphere membership and one list of rows built at the start of the stage,
	// not by one call of the public function per system (section 9.4).
	// Loop two: LowStabilityTicks and FallenSphereTicks. Last and separate on purpose: the count reads the Stability every
	// civilization has at the end of this stage. A Stability change made anywhere else in the tick is first counted one
	// tick later. FallenSphereTicks reads the sphere membership the stage built at its start.
	void TickEconomy();

	// Research progress and breakthroughs, civilization by civilization. A Society breakthrough fires the DT_Events row
	// CosmicRows::EventBookFound after its line; the row's conditions pick the people whose book is shut (section 5.11).
	// A people that owns no system (the one the player ruled, kept for the tick of its loss) is passed by: it spends no
	// Insight and crosses no mark (section 7.4 (o)).
	void TickResearch();
	void TickDiplomacy();
	void TickConflict();

	// Stage 5. Four steps in a fixed order (section 7.4). Every hazard is placed through CosmicHazards::Place, and its lines
	// follow (FollowUpNotes) before the next placement.
	// (a) Megaprojects and resolution (AdvanceMegaprojects), before the countdown: a project completed on the last tick of
	//     a hazard still saves the system. Every response slot before any construction slot.
	// (b) Countdown and expiry, in two passes. First every countdown goes down by one and the systems that reach 0 are
	//     collected in Systems order, so a hazard placed later in this stage keeps its whole count this tick. Then two
	//     numbers per collected system are drawn from the sim stream, all of them before the first expiry: the outcome
	//     number (the branch of the row, the new profile of an overthrown people) and the claim number (the weight of the
	//     faith traits, read as 1 until the trait rows of Phase 4). Then ExpireHazard for each, in that order.
	// (c) Spawn. One number per settled system in Systems order, all drawn first, whether its slot is free or not; then,
	//     in the same order, the Draw row the number falls on is placed on a free slot. Then the Capital rows,
	//     civilization by civilization in Civs order.
	// (d) The event slots, UCosmicEventSubsystem::EvaluateAndFire.
	// Each draw point of this stage draws a count that no row, tag, chance or weight decides: one number per settled
	// system, two per hazard that runs out, whatever DT_Hazards holds. How many hazards run out, and when, follows the
	// world, and the world follows the tables (section 9.2). KillPlayer and DeposeRuler are only recorded, through
	// UCosmicEventSubsystem::Apply (first recorded wins).
	void TickHazards();

	/**
	 * One expiry of step (b) (section 7.4), on the hazard tag SystemIndex held when it was collected: a slot that holds
	 * another tag (or none), or a hazard placed after the countdown pass (its count is above 0, whatever its tag), is
	 * passed by and its numbers are thrown away. A tag no row holds leaves its slot as a resolution without a row does: no
	 * effects, no line (a rebellion still ends its leader and its count). In this order: the last-system gate, when the
	 * code of the tag takes the system and it is its owner's last settled system (a kept system ends there: its slot is
	 * emptied without result); the effects (OnExpireEffects, or OnExpireBranchEffects when OutcomeNumber is below
	 * BranchChance), then OnExpireNeighbourEffects; the code of the tag (Hazard.PreSupernova: CosmicHazards::Destroy, then
	 * DeposeRuler through Apply when the people the vessel rules has no system left; Hazard.Rebellion: the throne;
	 * Hazard.Claim: the join); the slot emptied; the claim rule on that slot; the expiry line (a claim that did not join
	 * tells the resolution line); FollowUpNotes. Should the effects have made the system its owner's last settled system
	 * after the gate let it pass (no row of the game can: Data.Integrity keeps TransferSystem out of the expiry effects
	 * and out of OnResolveEffects, which a ResolveHazard on a neighbour runs), the gate is asked again, with an error line,
	 * before the code of the tag takes it; a system kept then has taken the effects already, and the notes they left are
	 * told after the grace line.
	 */
	void ExpireHazard(int32 SystemIndex, FGameplayTag Collected, float OutcomeNumber, float ClaimNumber);

	/**
	 * The last-system gate (section 4.5, 7.4): when SystemIndex is its owner's last settled system
	 * (CosmicHazards::IsLastSettledSystem - a settlement under way is no home and goes with it), fires
	 * CosmicRows::EventGraceRemnant with that system and people as subject. True when the row fired: the loss does not
	 * happen. The row's own conditions keep it to once per run. The subject of Ctx is left as it was.
	 */
	bool GraceKeepsLastSystem(FCosmicEvalContext& Ctx, int32 SystemIndex);

	/**
	 * Step (a) of the crisis stage (section 5.7, 7.4): CosmicMegaprojects::Advance for every response slot that holds a
	 * project, in Systems order, then for every construction slot, in Systems order (an answer spends the stockpile before
	 * a construction does), with DT_Rules.AIAscensionSpeedMultiplier as the rate of the peoples the player does not rule
	 * (not above 0, or not a number: 1, with one error line). After each slot its completion line (the row's
	 * CompleteTemplates, Log.Civ, Severity 1 for the people the player rules and 0 for the others, slots {Civ} {System}
	 * {Project}), then the lines that follow (FollowUpNotes - the resolution a completion made). No draw of its own.
	 */
	void AdvanceMegaprojects();

	// Stage 6. (a) Every people the player does not rule, in Civs order: it starts its CosmicMegaprojects::Choices
	// (SinceTick 0), no draw (section 4.3, 5.7); then, when CosmicMegaprojects::MaySettle holds, one number from the sim
	// stream, drawn before the targets are looked at and thrown away when there is none, picks a target of SettleTargets
	// to Settle (section 9.2). A people that may not settle draws nothing. (b) UAscensionSubsystem::EvaluateFeats,
	// exactly once per tick and nowhere else in the tick.
	void TickCivAI();

	// Stage 7. The only place where the number of elements of State.Civs changes (section 7.4). (o) Cleanup: first every
	// people with no settled system lets its settlements go (CosmicHazards::LetSettlementsGo - a destruction has done so
	// already in play; when that empties the people the player rules with no deposition recorded, an error line says so,
	// for nothing lets go of it then); then every civilization that owns no system (CosmicState::OwnsAnySystem) leaves Civs, in array
	// order, except the one the player rules this tick, which the next tick's cleanup takes once the role-effect stage has
	// let go of it (that rests on the recorded deposition: Phase 4b's death pipeline lets go of a people as well). The
	// Relations key of a removed CivID leaves every other civilization, Knowing.PendingContactCivID is cleared when it
	// names one, and the state index is rebuilt. Every other record of the ID stays. No line: the expiry that took the
	// last system has told it. Arrivals and departures are Phase 6a.
	void TickCrossings();

	void TickPlayerTravel();

	// Stage 9, only while the calling is Embodied. Reads the world and writes only State.Knowing and the telling: no
	// draw from the sim stream. Its two triggers need no state of their own: the period is State.Tick against
	// DT_Rules.SightIntervalTicks, and "the vessel moved this tick" is a local of SimulateTick (Player.CurrentSystemID
	// before and after TickPlayerTravel), never a member of this class and never a saved field (section 4.4: the eye is
	// derived). Whatever places the eye outside stage 8 (Install, a role change) or widens it in place (a scan) writes
	// knowing in its own call. Empty body until Phase 2: the knob and the local arrive with the body.
	void TickSight();

	// Stage 10, outside the stage guard (section 7.4). Effect.DeposeRuler: UPlayerRoleSubsystem::DeposeRuler, then the
	// record is cleared. Effect.KillPlayer stays recorded until the death pipeline of Phase 4b: no row of the game records
	// one before then.
	void ResolvePendingRoleEffect();

	// The walk shared by ComputeStateHash and ComputeWorldHash.
	uint32 HashState(bool bWorldOnly) const;

	const FCivilizationState* FindCiv(FName CivID) const;
	const FStarSystemState* FindSystem(FName SystemID) const;

	UPROPERTY()
	FGalaxyState State;

	UPROPERTY(Transient)
	FCosmicDataTables Tables;

	FRandomStream Rng;
	FCosmicRulesDef Rules;
	FGalaxyDef HomeGalaxy;

	// Derived from State, never saved: see GetStateIndex.
	FCosmicStateIndex StateIndex;

	int32 BroadcastDepth = 0;

	float Accumulator = 0.f;
	float TimeScale = 1.f;
	bool bRunning = false;
	bool bBroadcastEnabled = true;

	// True while stages one to nine run; role effect resolution, the tenth, is outside (the Phase 4b leap runs ticks
	// from there). NewGalaxy, LoadState and SimulateTick ensure against it: the stages hold references into the state.
	// The ensure is a backstop: a delegate fired from inside the stages is a broadcast, and the four entry points that
	// replace the state or run a tick refuse a call made during one (IsBroadcasting).
	bool bInTickStages = false;

	static constexpr float SecondsPerTick = 1.0f;

	// Ticks processed per frame at most. The remainder carries over to the next frame, capped at this many ticks (section 9.4).
	static constexpr int32 MaxTicksPerFrame = 10;
};

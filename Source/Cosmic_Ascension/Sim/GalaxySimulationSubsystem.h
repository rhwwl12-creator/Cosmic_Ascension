// Cosmic Ascension - Sim layer. Design: guideline.md section 6.1, 7.4, 9.2.
// RULE (section 6.1): files under Sim/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
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
 * steps inside the economy, hazard and civ AI stages (see the stage declarations). Change guideline.md section 7.4
 * before changing any of this.
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
	 * Replaces the state with a new galaxy generated from Seed and the definition tables, then installs the first life as
	 * the ruler of the player civ (DT_Rules.PlayerArchetypeRow, home system "sol") through UPlayerRoleSubsystem::Install
	 * (section 5.10). AI civs draw their archetype from the DT_Civs rows with a positive Weight (section 7.4).
	 * The fallen roster (section 4.2): the instance of CosmicRows::TransSilence when DT_Transcendents holds that row as
	 * a Realm.Abyssal row, then DT_Rules.FallenTranscendentCount - 1 draws from the sim stream, each picking one more
	 * Realm.Abyssal row by SpawnWeight among the rows not active yet, in row-name order (a row whose weight is not above
	 * zero or not finite is no candidate). A draw without a candidate is thrown away, so the number of draws follows the
	 * knob alone, never the table. The knob is read as 0..3, with one error line when it lies outside. Celestial and
	 * Independent rows get no instance. Every sphere starts empty: the starting systems (FTranscendentDef.InitialInfluencedSystems) are
	 * placed by the generation change of Phase 2.
	 * Stamps State.TellRngState with (int32)CosmicSeed::Derive(Seed, "", Tell, 0) before the first line is told.
	 * Tick 0 is this generated state; the first simulated tick is 1.
	 * Refused with one error line inside a handler of OnLogEntry or OnEventFired (IsBroadcasting): the state is not
	 * replaced under a stage or a row that is still running.
	 */
	UFUNCTION(BlueprintCallable)
	void NewGalaxy(int32 Seed, int32 SystemCount, int32 CivCount);

	// The UI offers 0, 1, 2, 5, 10 (section 2.1). 0 pauses, and it is the only pause of the simulation: this subsystem
	// ticks in the engine's null-world pass, so engine pause (SetGamePaused) and time dilation never reach it.
	UFUNCTION(BlueprintCallable)
	void SetTimeScale(float Scale);

	UFUNCTION(BlueprintPure)
	float GetTimeScale() const { return TimeScale; }

	// One tick, with broadcasts as configured. Debug and tests. Refused with one error line inside a handler of
	// OnLogEntry or OnEventFired (IsBroadcasting): a handler may only read the state (section 9.4).
	UFUNCTION(BlueprintCallable)
	void StepOnce();

	// Runs Ticks ticks with the broadcast gate closed, then restores the gate. Tests, balance runs, interregnum leap (section 4.4).
	// Not yet: the per-tick ending-condition check that stops a leap early arrives with the endings (section 4.4, 7.4).
	// Refused like StepOnce inside a handler of OnLogEntry or OnEventFired.
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

	// The definition tables (section 7.3), loaded from their fixed asset paths in Initialize.
	const FCosmicDataTables& GetTables() const { return Tables; }

	// Replaces the tables and re-reads the rules row. Tests inject transient tables here.
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
	// LoadState, StepOnce and RunHeadless here and UCosmicSaveSubsystem::LoadGalaxy. Every such broadcast
	// site brackets its Broadcast call with BeginBroadcast and EndBroadcast.
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
	// Loop one, per civilization: income of its systems into IncomePerTick; then the per-tick sources, the only effects
	// that may scale IncomePerTick or act every tick; then IncomePerTick into the Stockpile. The per-tick sources:
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
	// Loop two: LowStabilityTicks. Last and separate on purpose: the count reads the Stability every civilization has at
	// the end of this stage. A Stability change made anywhere else in the tick is first counted one tick later.
	void TickEconomy();

	void TickResearch();
	void TickDiplomacy();
	void TickConflict();

	// Stage 5. Four steps in a fixed order (section 7.4): (a) resolution, (b) countdown and expiry in two passes,
	// (c) spawn, exactly one sim draw per settled system, (d) the event slots, UCosmicEventSubsystem::EvaluateAndFire.
	// (a) to (c) arrive with the crisis handling work of Phase 2; (d) is the last step of this stage from now on.
	// KillPlayer and DeposeRuler are only recorded, through UCosmicEventSubsystem::Apply (first recorded wins).
	void TickHazards();

	// Stage 6. (a) decisions of AI civilizations and regents in Civs array order (a civilization without systems is
	// skipped); (b) UAscensionSubsystem::EvaluateFeats, exactly once per tick and nowhere else in the tick (from the
	// ascension judgment work of Phase 2).
	void TickCivAI();

	// Stage 7. The only place where the number of elements of State.Civs changes. Empty body until Phase 2 (cleanup of
	// civilizations without systems) and Phase 6a (arrivals, departures).
	void TickCrossings();

	void TickPlayerTravel();

	// Stage 9, only while the calling is Embodied. Reads the world and writes only State.Knowing and the telling: no
	// draw from the sim stream. Its two triggers need no state of their own: the period is State.Tick against
	// DT_Rules.SightIntervalTicks, and "the vessel moved this tick" is a local of SimulateTick (Player.CurrentSystemID
	// before and after TickPlayerTravel), never a member of this class and never a saved field (section 4.4: the eye is
	// derived). Whatever places the eye outside stage 8 (Install, a role change) or widens it in place (a scan) writes
	// knowing in its own call. Empty body until Phase 2: the knob and the local arrive with the body.
	void TickSight();

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

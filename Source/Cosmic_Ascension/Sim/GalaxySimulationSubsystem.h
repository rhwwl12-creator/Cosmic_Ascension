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
	 * Stamps State.TellRngState with (int32)CosmicSeed::Derive(Seed, "", Tell, 0) before the first line is told.
	 * Tick 0 is this generated state; the first simulated tick is 1.
	 */
	UFUNCTION(BlueprintCallable)
	void NewGalaxy(int32 Seed, int32 SystemCount, int32 CivCount);

	// The UI offers 0, 1, 2, 5, 10 (section 2.1). 0 pauses, and it is the only pause of the simulation: this subsystem
	// ticks in the engine's null-world pass, so engine pause (SetGamePaused) and time dilation never reach it.
	UFUNCTION(BlueprintCallable)
	void SetTimeScale(float Scale);

	UFUNCTION(BlueprintPure)
	float GetTimeScale() const { return TimeScale; }

	// One tick, with broadcasts as configured. Debug and tests.
	UFUNCTION(BlueprintCallable)
	void StepOnce();

	// Runs Ticks ticks with the broadcast gate closed, then restores the gate. Tests, balance runs, interregnum leap (section 4.4).
	// Not yet: the per-tick ending-condition check that stops a leap early arrives with the endings (section 4.4, 7.4).
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
	// and nothing else, so UCosmicEventSubsystem refuses its member functions while this is true. Every such broadcast
	// site brackets its Broadcast call with BeginBroadcast and EndBroadcast.
	bool IsBroadcasting() const { return BroadcastDepth > 0; }
	void BeginBroadcast() { ++BroadcastDepth; }
	void EndBroadcast() { --BroadcastDepth; }

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
	// that may scale IncomePerTick or act every tick (the Stability drain of spiritual pressure and
	// FTranscendentDef.PressureEffectsPerTick from the pressure step of Phase 2, edict and trait rows from Phase 4),
	// applied through UCosmicEventSubsystem::ApplyAll with bPerTickSource; then IncomePerTick into the Stockpile.
	// Loop two: LowStabilityTicks. Last and separate on purpose: the count reads the Stability every civilization has at
	// the end of this stage. A Stability change made anywhere else in the tick is first counted one tick later.
	void TickEconomy();

	void TickResearch();
	void TickDiplomacy();
	void TickConflict();

	// Stage 5. Four steps in a fixed order (section 7.4): (a) resolution, (b) countdown and expiry in two passes,
	// (c) spawn, exactly one sim draw per settled system, (d) the event slots, UCosmicEventSubsystem::EvaluateAndFire.
	// (a) to (c) arrive with the hazard step of Phase 2; (d) is the last step of this stage from now on.
	// KillPlayer and DeposeRuler are only recorded, through UCosmicEventSubsystem::Apply (first recorded wins).
	void TickHazards();

	// Stage 6. (a) decisions of AI civilizations and regents in Civs array order (a civilization without systems is
	// skipped); (b) UAscensionSubsystem::EvaluateFeats, exactly once per tick and nowhere else in the tick (from the
	// ascension step of Phase 2).
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
	// from there). NewGalaxy, LoadState and SimulateTick ensure against it: the stages hold references into the state,
	// and a delegate fired from inside them (OnLogEntry) must not replace it or run a tick.
	bool bInTickStages = false;

	static constexpr float SecondsPerTick = 1.0f;

	// Ticks processed per frame at most. The remainder carries over to the next frame, capped at this many ticks (section 9.4).
	static constexpr int32 MaxTicksPerFrame = 10;
};

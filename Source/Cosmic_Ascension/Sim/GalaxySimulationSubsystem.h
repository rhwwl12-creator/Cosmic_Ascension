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
#include "GalaxySimulationSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSimTick, int64, Tick);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGalaxyGenerated, int32, Seed);

/**
 * Fixed-step galaxy simulation (section 7.4). Owns the whole runtime state (FGalaxyState) and the single random stream.
 * One tick = one galaxy cycle; at 1x speed one second is one tick (section 2.1).
 * Stage order inside a tick is fixed: economy, research, diplomacy, conflict, hazards, civ AI, player travel, role effects.
 * Change guideline.md section 7.4 before changing that order.
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

	// The only source of randomness for sim code (section 9.2). Never use FMath::Rand*.
	FRandomStream& GetRng() { return Rng; }

	// Copies the live stream position into State.RngState. NewGalaxy and every tick end with it. Code that draws from GetRng()
	// outside a tick, and the save subsystem right before saving, call it themselves.
	void SyncRngState();

	// DT_Rules single row. C++ defaults when the table or its row is missing.
	const FCosmicRulesDef& GetRules() const { return Rules; }

	// The definition tables (section 7.3), loaded from their fixed asset paths in Initialize.
	const FCosmicDataTables& GetTables() const { return Tables; }

	// Replaces the tables and re-reads the rules row. Tests inject transient tables here.
	void SetTables(const FCosmicDataTables& InTables);

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
	 * Canonical hash of the state (section 9.2). Walks FGalaxyState by reflection, so a new field is hashed without touching
	 * this function. Skips every FText and GalaxyID, hashes FName and FGameplayTag as lower-cased strings, TMap entries in
	 * FNameLexicalLess key order, tag containers by their explicit tags, and the live random stream instead of State.RngState.
	 * Stable across processes of the same build.
	 */
	uint32 ComputeStateHash() const;

private:
	// Advances State.Tick first, then runs the eight stages in the fixed order and syncs the random stream state.
	void SimulateTick();

	void TickEconomy();
	void TickResearch();
	void TickDiplomacy();
	void TickConflict();
	void TickHazards();
	void TickCivAI();
	void TickPlayerTravel();
	void ResolvePendingRoleEffect();

	const FCivilizationState* FindCiv(FName CivID) const;
	const FStarSystemState* FindSystem(FName SystemID) const;

	UPROPERTY()
	FGalaxyState State;

	UPROPERTY(Transient)
	FCosmicDataTables Tables;

	FRandomStream Rng;
	FCosmicRulesDef Rules;

	float Accumulator = 0.f;
	float TimeScale = 1.f;
	bool bRunning = false;
	bool bBroadcastEnabled = true;

	// True while stages one to seven run; role effect resolution, the eighth, is outside (the Phase 4b leap runs ticks
	// from there). NewGalaxy, LoadState and SimulateTick ensure against it: the stages hold references into the state,
	// and a delegate fired from inside them (OnLogEntry) must not replace it or run a tick.
	bool bInTickStages = false;

	static constexpr float SecondsPerTick = 1.0f;

	// Ticks processed per frame at most. The remainder carries over to the next frame, capped at this many ticks (section 9.4).
	static constexpr int32 MaxTicksPerFrame = 10;
};

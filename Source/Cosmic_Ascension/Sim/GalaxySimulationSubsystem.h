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

// The time six stages took (B3 header section 5 - the balance commandlet's stage line; B4 added the economy and the card's
// presentation at the end of the knowing stage): off until a caller turns it on. Never saved, hashed or read by a rule; it
// changes no order and draws nothing. Cycles of FPlatformTime::Cycles64 summed per stage - [0] diplomacy, [1] conflict,
// [2] crisis, [3] AI, [4] economy, [5] the card's presentation - over Ticks ticks. While it is on, the conflict stage also notes
// the sieges it ended (the commandlet's conflict line reads them, then empties the list): taken, held (the attacker
// repulsed) or lifted (its gate closed), with the attacker, the owner before the end and the system. B5 ([B-5a]): the crisis
// stage notes every expiry it runs (the commandlet's calling line reads them, then empties the list): its system and tag, the
// side it took (the branch effects, its number under BranchChance), whether the remnant's grace kept the system at its gate
// (no effect ran), and whether a KillPlayer on that system would have reached the vessel then (CosmicHazards::VesselAtRiskOf,
// the slot not asked - the moment Apply would ask). An expiry an answer edict resolves is no expiry and is not noted.
struct FCosmicStageClock
{
	enum class ESiegeEnd : uint8
	{
		Taken,
		Held,
		Lifted,
	};
	struct FSiegeEnd
	{
		ESiegeEnd Kind = ESiegeEnd::Lifted;
		FName Attacker;
		FName Owner;
		FName SystemID;
	};
	struct FExpiry
	{
		FName SystemID;
		FGameplayTag Tag;
		bool bBranch = false;
		bool bGrace = false;
		bool bReached = false;
	};
	bool bOn = false;
	uint64 Cycles[6] = {};
	int64 Ticks = 0;
	TArray<FSiegeEnd> SiegeEnds;
	TArray<FExpiry> Expiries;
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
	 * life as the ruler of the player's people (DT_Rules.PlayerArchetypeRow, home system CosmicIDs::FirstHome "sys_0")
	 * through UPlayerRoleSubsystem::Install (section 5.10, 7.4).
	 *
	 * Numbers. A count of 0 or less is read from the home row (GetHomeGalaxy: LivingWorlds, Peoples), so no caller
	 * keeps a copy of those numbers. Systems: at least 1, and no upper bound here (the actor budget of 150 belongs to
	 * the presentation layer). Peoples: at least 1, at most the number of systems. The sanctuary is not counted: the
	 * state holds SystemCount + 1 systems.
	 *
	 * What it makes. Systems "sys_<n>" (n = the array index, sys_0 the first life's home) and the sanctuary "sol" at the
	 * end of the array, civilizations "civ_<n>" from State.NextCivSerial, no padding; every system and civilization
	 * carries DT_Rules.HomeGalaxyRow in GalaxyRow. sys_0 lies at HomeRadiusShare of the disc radius, angle 0; every other
	 * living system at a point drawn in the square and thrown away while it lies outside the disc, at most 16 tries, the
	 * sixteenth halved when it misses too. No trigonometry: only + - x / and the square root reach a hashed value
	 * (section 9.2). Lanes join systems of the one galaxy. One lane neighbour of sys_0 is the home of the first AI
	 * people. Names are final and flattened here: nothing is named later.
	 *
	 * The draws, in this order and no other (section 9.2). Kinds of draw are made in blocks:
	 * 1. Per living system, in array order: its place (none for sys_0; two draws per try), its star class (one draw
	 *    against DT_Rules.LivingWorldStarShares), its richness (one draw). sys_0 draws both too; its richness is always
	 *    1, and the home of any people whose archetype has a HomeStarClass takes that class instead of the one it drew.
	 * 2. Homes: the first AI people among the lane neighbours of sys_0 (one draw, none when there is one people), then
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
	 *    DT_Rules.FallenCoreMinLanesFromHome lanes from sys_0, one lane from the home of an AI people and in no sphere
	 *    yet, picked among the candidates in array order. Without a candidate: the unowned system in no sphere that
	 *    lies farthest from sys_0 in lanes, the lowest SystemID (FNameLexicalLess) among equals. Without an unowned
	 *    system the sphere starts empty.
	 * 7. The first life: the name of the vessel (one draw, in Install), after the sanctuary has joined the array.
	 * Every people is made holding DT_Rules.StartingEdict when the table holds that row (the inherited edict, B2 - no draw).
	 * The sanctuary (rule 11 of the generation, section 4.5 (1)) draws nothing: after the cores it joins the end of the
	 * array as CosmicIDs::Sanctuary - no lane, at 1.15 x the disc radius and angle 0, star class G, richness 0, the name
	 * of ST_CA System.Sanctuary, no owner, hazard, project, sphere or core.
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

	// The UI offers 0, 1, 2, 5 and 10, and 25 once the calling sees from the third rung (section 2.1 - which speeds are
	// open is the presentation's rule, CosmicUI::IsSpeedOpen; this subsystem keeps nothing of the player). 0 pauses, and
	// it is the only pause of the simulation: this subsystem ticks in the engine's null-world pass, so engine pause
	// (SetGamePaused) and time dilation never reach it. A scale that is not above 0, or not a number, reads as 0 (a
	// compare: FMath::Clamp turns a NaN into its top); one above MaxTimeScale reads as MaxTimeScale. A frame still runs
	// MaxTicksPerFrame ticks at most, so x25 keeps its pace only above 2.5 frames a second (section 9.4).
	UFUNCTION(BlueprintCallable)
	void SetTimeScale(float Scale);

	// The highest time scale: the third rung's x25 (section 2.1). Not the frame budget: MaxTicksPerFrame stays 10.
	static constexpr float MaxTimeScale = 25.f;

	UFUNCTION(BlueprintPure)
	float GetTimeScale() const { return TimeScale; }

	// One tick, with broadcasts as configured. Debug and tests. Refused with one error line inside a handler of a Core
	// Logic delegate (IsBroadcasting): a handler may only read the state (section 9.4) - and, like RunHeadless and RunLeap,
	// inside the tick stages and inside an OnSimTick handler (B5: a handler of the boundary runs no tick).
	UFUNCTION(BlueprintCallable)
	void StepOnce();

	// Runs Ticks ticks with the broadcast gate closed, then restores the gate. Tests and balance runs (section 4.4): no tick
	// inside it sends OnSimTick.
	// The sim records the end and does not stop (B6a header section 2.5): SimulateTick writes State.Ending once, after the
	// tenth stage of the tick whose end it is (CosmicEndings::EndingOf), and StepOnce and RunHeadless go on ticking after it -
	// no stage reads the record, so the world after the end is the world without it and one tick at a time stays the same
	// world as one long leap (the balance commandlet's long runs and the measures past the end, -PastEnd). RunLeap alone stops
	// at the end's tick; the commands after the end are refused by the controller's latch.
	// Refused like StepOnce.
	UFUNCTION(BlueprintCallable)
	void RunHeadless(int64 Ticks);

	// The fast-forward of the interregnum (B5 header section 2.6): up to Ticks ticks with the broadcast gate closed - the same
	// world as those ticks one at a time -, then, while the broadcast gate is open, one OnSimTick for State.Tick from outside
	// the stages: the one boundary at the end of the leap, which the autosave (its cause Ended or Succession), the end's latch
	// and the screens read. It stops at the end (B6a header section 2.6): when State.Ending was empty when it was called, the
	// tick whose SimulateTick wrote the record is the last it runs - an end the presentation cannot know ahead (the silence,
	// another people on the fifth rung) stops the leap in its own tick, and the clock stops it where the controller used to cut
	// it -, then the one boundary as before. When a record already stood it runs all Ticks (a leap past the end - a session
	// with the end switched off). The presentation calls it at its input (ACosmicPlayerController::AdvanceLeap - it sets Ticks,
	// and with the end switched off calls once more past a stop). Refused like StepOnce; Ticks of 0 or below runs nothing and
	// sends no boundary.
	UFUNCTION(BlueprintCallable)
	void RunLeap(int64 Ticks);

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
	 * result. Count < 1 is a caller error: one error line, no draw, returns 0; so is a call from a handler of a Core Logic
	 * delegate (IsBroadcasting).
	 */
	uint32 DrawTell(int32 Count);

	// Copies the live stream position into State.RngState. NewGalaxy and every tick end with it. Code that draws from GetRng()
	// outside a tick, and the save subsystem right before saving, call it themselves.
	void SyncRngState();

	/**
	 * Meets the people CivID (section 7.4, 8.1): the one writer of Knowing.MetCivs besides Install. Nothing when the
	 * people is already met, is not in Civs, or when the calling has no vessel (interregnum). The gap rule comes first: a
	 * people whose Tier is three or more above GetViewerTier is not met. A new meeting is appended with the current tick
	 * and told at once, through the tell stream only (no sim draw): the first meeting of the run - while
	 * Knowing.FirstContactCivID is NAME_None - writes that field and tells CosmicRows::LogFirstContact, any later one
	 * CosmicRows::LogContact, both Severity 1 about that people and about SeenAtSystemID (NAME_None when the caller has
	 * no place). The first meeting also writes Knowing.PendingContactCivID for an embodied ruler while DT_Encounters holds
	 * CosmicRows::PetitionFirstContact (B3: the first contact card - CosmicPetitions::Present puts it). Returns whether the
	 * people was newly met.
	 * Refused with an error line when called from a handler of a Core Logic delegate.
	 */
	bool NoteMet(FName CivID, FName SeenAtSystemID = NAME_None);

	/**
	 * Knows the system SystemID (section 8.1): appended to Knowing.KnownSystems with the current tick when it is not
	 * there yet. No line, no draw. Nothing for a system the state does not hold or without a vessel. Returns whether it
	 * was newly known. Refused like NoteMet inside a handler.
	 */
	bool NoteKnown(FName SystemID);

	/**
	 * One look of an eye (section 7.4, B1 header section 2.1) - the knowing stage's two writes: the walk from every place
	 * at once (CosmicState::SightWalkOf), the systems within Reach lanes known through NoteKnown in the order found (only
	 * when bSees), then every known system's owner met nearer first (NoteMet; lanes from the eye without the reach bound,
	 * ties by CivID). Reads the world, writes only Knowing and the telling; no sim draw. Returns how many systems became
	 * known. TickSight and the wanderer's scan (UPlayerRoleSubsystem::ScanCurrentSystem, B1) call it.
	 */
	int32 LookFrom(TConstArrayView<int32> EyePlaces, int32 Reach, bool bSees);

	/**
	 * The tier the calling sees from (section 8.1): the Tier of the people the vessel rules while it rules one, the
	 * calling's PersonalTier otherwise (a wanderer, and the interregnum). Read by the gap rule of NoteMet and by the
	 * scope of telling; never by the world.
	 */
	EAscensionTier GetViewerTier() const;

	// DT_Rules single row. C++ defaults when the table or its row is missing.
	const FCosmicRulesDef& GetRules() const { return Rules; }
	// The stage clock (B3, measurement only - FCosmicStageClock).
	FCosmicStageClock& GetStageClock() { return StageClock; }

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
	// (a card's answer, the abdication command) does the same and calls SyncRngState() afterwards. Reports an error when called
	// while IsBroadcasting(): a handler must not evaluate anything.
	FCosmicEvalContext MakeEvalContext();

	// ID -> index for Civs and Systems (section 7.4, 9.4). Rebuilt by NewGalaxy, LoadState and the crossing stage.
	const FCosmicStateIndex& GetStateIndex() const { return StateIndex; }

	// For code that changed the number of elements of Civs or Systems through GetMutableState (tests).
	void RebuildStateIndex() { StateIndex.Rebuild(State); }

	// Counts the Core Logic delegates that are being broadcast right now and that can fire from inside a tick stage:
	// OnLogEntry, OnEventFired, OnTierAscended (section 9.4). Their handlers may read the state and call SetTimeScale
	// and nothing else, so UCosmicEventSubsystem refuses its member functions while this is true, and so do NewGalaxy,
	// LoadState, StepOnce, RunHeadless, RunLeap, NoteMet, NoteKnown and DrawTell here, UCosmicLogSubsystem::Push and PushVariants
	// (and so PushRow), UCosmicSaveSubsystem::SaveCurrentGalaxy, LoadGalaxy (LoadRun) and ContinueNewest,
	// UAscensionSubsystem::EvaluateFeats and
	// the role changes and commands of UPlayerRoleSubsystem (Install, DeposeRuler, BecomeWanderer, BecomeRuler, Abdicate,
	// AbdicateRuler, Die, Succeed, StartMegaproject, StartNextMegaproject, SetResearchFocus, SetResearchShares, EnactEdict, RevokeEdict,
	// AnswerPetition, TravelTo, ScanCurrentSystem, SetStance, ProposeTreaty, BreakTreaty, AllocateFleet).
	// MakeEvalContext reports an error and still returns the context (it holds references): the
	// static Apply and ApplyAll are kept out of a handler by the rule alone. Every such broadcast site brackets its
	// Broadcast call with BeginBroadcast and EndBroadcast.
	bool IsBroadcasting() const { return BroadcastDepth > 0; }
	// True while the ten tick stages and the stream's sync at their end run (SimulateTick): the save layer writes and loads
	// at a tick boundary only (16 #33 (1)), so it refuses inside them as it does inside a handler. B5 widened the guard to the
	// tenth stage (ResolvePendingRoleEffect - it runs no tick since the leap is a countdown; header section 2.2): its pipelines
	// (Install, the death's lines) hold the state while they run. StepOnce, RunHeadless and RunLeap refuse inside it.
	bool IsInTickStages() const { return bInTickStages; }
	// True while OnSimTick is being sent (the tick boundary - not a Core Logic broadcast of IsBroadcasting): its handlers may
	// stop the time and replace the state (NewGalaxy, LoadState), but run no tick - StepOnce, RunHeadless and RunLeap refuse.
	bool IsSendingSimTick() const { return SimTickDepth > 0; }
	// True while RunLeap sends its boundary: the ticks behind it told the presentation nothing (the gate was closed), so a
	// screen that keeps what was told rebuilds from the state (the log feed) - whatever the leap's length.
	bool IsSendingLeapBoundary() const { return bSendingLeapBoundary; }
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

	/**
	 * The pressure a fleet of the people CivIndex meets on SystemIndex (B3 section 2.3 - the attrition of the conflict
	 * stage, the AI's choice of a system to aim at, the panel's tooltip): the terms of ComputePressureIn of the unsealed
	 * transcendents whose row stands on a Tier above the people's and whose sphere holds the system (reach Inside) - and no
	 * other term: no owner's, no wanderer's, no lane neighbour's. The same body and the same bits as those terms of
	 * ComputePressureIn. A people or a system out of range gives the zero result; so does the sanctuary.
	 */
	static FCosmicPressure ComputeAttritionPressureIn(const FGalaxyState& InState, const FCosmicStateIndex& InIndex, const FCosmicDataTables& InTables, const FCosmicRulesDef& InRules, int32 CivIndex, int32 SystemIndex);

	/**
	 * The P the economy stage drains the people CivIndex by (section 4.2; B4 header section 2.3): the largest P of
	 * ComputePressureIn among the settled systems it owns (OwnedSystems whose owner is the people) whose dominant source
	 * stands on a Tier above the people's; 0 when there is none. Read at a tick boundary it is the P of the next tick's
	 * economy stage, which builds its own table of the same reach instead (section 9.4) - Sim.PressureDrainEdict holds the
	 * two together. For the measurements and the presentation, so that none keeps a copy of the rule. A people out of
	 * range gives 0.
	 */
	static float ComputeDrainPressureOf(const FGalaxyState& InState, const FCosmicStateIndex& InIndex, const FCosmicDataTables& InTables, const FCosmicRulesDef& InRules, int32 CivIndex);

	/**
	 * The Stability the economy stage drains from Civ a tick at the drain's P Pressure (section 4.2; B4): the knob
	 * DT_Rules.SpiritualPressureStabilityDrain (below 0 or not a number: 0) x Pressure x the held edicts' factor
	 * (CosmicEdicts::PressureDrainMultiplierOf). The stage's own arithmetic - it calls this - before the floor at 0.
	 */
	static float DrainLossOf(const FCosmicRulesDef& InRules, const FCosmicDataTables& InTables, const FCivilizationState& Civ, float Pressure);

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

	// Global broadcast gate (section 6.1). Every Core Logic subsystem checks it before broadcasting. Closed during RunHeadless
	// and during the ticks of RunLeap, which sends one OnSimTick at its end (B5).
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
	 * World hash (section 8.1, 9.2): ComputeStateHash without the top-level members Knowing, Log, Chronicle,
	 * ChronicleEviction and TellRngState. Measures that knowing and telling never change the world. The cooked build is
	 * compared on this value and on ComputeStateHash, in the commandlet's telling mode (section 9.2, MVP item 9).
	 */
	uint32 ComputeWorldHash() const;

private:
	FCosmicStageClock StageClock;

	// Advances State.Tick first, then runs the ten stages in the fixed order, writes the end's record when the first end stands
	// (after the tenth stage, B6a header section 2.3) and syncs the random stream state.
	void SimulateTick();

	// Stage 1. Two loops over Civs in array order.
	// Loop one, per civilization: the population regrowth (DT_Rules.PopulationRegrowthPerTick: below 1.0 it rises by the
	// knob up to 1.0; above 1.0, or with a knob not above 0 or not a number, nothing); the Stability recovery (below
	// DT_Rules.StabilityRestingLevel it rises by StabilityRecoveryPerTick x (1 + SocietyRecoveryBonus x Society) up to the
	// level - MVP item 3, B2; Society research read 0..1, a bonus not above 0 or not finite read as 0); income of its
	// settled systems into IncomePerTick (a system being settled pays nothing, section 5.7; each trade treaty adds
	// DT_Rules.TradeInfluencePerTreaty to the Influence - B3); then the per-tick sources, the
	// only effects that may scale IncomePerTick or act every tick; then IncomePerTick into the Stockpile; then the edicts'
	// upkeep (B2): in ActiveEdicts order each held edict pays UpkeepPerTick in full from the stockpile or lapses
	// (log_edict_lapsed for the people the player rules - the stage's one line, the tell stream only). The per-tick sources:
	// - The held edicts' EffectsPerTick (B2), first, in ActiveEdicts order, subject = the civilization alone (no system, no
	//   OtherID), SourceRow = the edict.
	// - The drain of spiritual pressure, once per civilization and never once per system (section 16 #55): Stability
	//   goes down by DT_Rules.SpiritualPressureStabilityDrain x the largest P among its settled systems whose dominant
	//   source has a Tier above the civilization's own, x the held edicts' PressureDrainMultiplier (B4 - DrainLossOf).
	// - FTranscendentDef.PressureEffectsPerTick of every unsealed transcendent of a Tier above the civilization's whose
	//   sphere holds one of its settled systems: once per transcendent, in the order of State.Transcendents, through
	//   UCosmicEventSubsystem::ApplyAll with bPerTickSource. Subject = the civilization and the first such system in
	//   OwnedSystems order; OtherID and SourceRow = the TranscendentID. A people that only borders the sphere is drained
	//   and takes none of these effects.
	// - Trait rows, from Phase 4.
	// A per-tick source applies the seven effects that change an amount and no other (UCosmicEventSubsystem::Apply), so
	// this stage makes no draw and records no role effect, whatever the rows hold.
	// The pressures are read from one table of sphere membership and one list of rows built at the start of the stage,
	// not by one call of the public function per system (section 9.4).
	// Loop two: LowStabilityTicks, FallenSphereTicks and PressureEnduredTicks (B6b). Last and separate on purpose: the count
	// reads the Stability every civilization has at the end of this stage, recovery and per-tick sources included. A
	// Stability change made anywhere else in the tick is first counted one tick later. FallenSphereTicks reads the sphere
	// membership the stage built at its start; PressureEnduredTicks adds one when this tick's drain took Stability and
	// HomeSystemID lies in no unsealed fallen sphere of that same table, else it starts over (MoveCapital starts it over too).
	void TickEconomy();

	// Research progress and breakthroughs, civilization by civilization. A Society breakthrough fires the DT_Events row
	// CosmicRows::EventBookFound after its line; the row's conditions pick the people whose book is shut (section 5.11).
	// A people that owns no system (the one the player ruled, kept for the tick of its loss) is passed by: it spends no
	// Insight and crosses no mark (section 7.4 (o)).
	void TickResearch();

	// Stage 3 (B3 header section 2.5). No draw; the tell stream only. (1) The friendly stances' upkeep, people by people in
	// Civs order, each people's friendly entries in lexical key order: those beyond its friendly slots (a save made with
	// other knobs) lapse to neutral from the back first, then each pays DT_Rules.FriendlyUpkeepInfluence from the stockpile
	// or lapses. A lapse of the people the player rules is told (CosmicRows::LogStanceLapsed). (2) The drift: for every
	// people X (Civs order) its borders once (CosmicDiplomacy::BordersOf, one array kept across the viewers - the same
	// answer as BorderTableOf, for the drift moves no settled system) and every people Y it borders, in Civs order, X's
	// relation toward Y moves by DT_Rules.RelationDriftRate of the way to
	// CosmicDiplomacy::TargetRelationOf, within [-1, 1] - and an entry is written only when the value changes (a rate of 0
	// writes nothing). A pair that does not border does not move; a transcendent's key never drifts. A silent drift meets
	// no one.
	void TickDiplomacy();

	// Stage 4 (B3 header section 2.6). (1) The fleets, people by people in Civs order: the Energy upkeep
	// (FleetUpkeepEnergyPerPower x FleetPower) paid in full and the fleet grows (FleetGrowthPerTick x the held edicts'
	// CosmicConflict::FleetGrowthMultiplierOf) toward CosmicConflict::FleetCapOf, or nothing is paid and it shrinks by
	// FleetGrowthPerTick; then down to the cap; then, while it has a target, PressureFleetAttrition x
	// ComputeAttritionPressureIn. Never below 0. A people with no cap and no fleet is passed by. (2) The sieges at their end
	// (a target another living people owns, CosmicConflict::IsSiegeDue - stored state and SiegeTicks only, no gate) are
	// collected in Civs order of the attacker and one number is drawn for each, all first. (3) Every target is asked its
	// gate (an own system: GuardBlockOf; another's: SiegeBlockOf); one that fails is lifted (FleetTargetSinceTick = Tick,
	// the rest of an AI) and a collected siege's number is thrown away. (4) The collected sieges left, in order: the gate is
	// asked again (an earlier taking may have closed it - then lifted as in (3)); else CosmicConflict::Takes(number,
	// SiegeOddsOf) - taken: CosmicHazards::Transfer, FollowUpKnowing, every people bordering the conqueror right after (and
	// the loser) lowers its view of it by ConquestRelationPenalty; held: the attacker's fleet x SiegeRepulsedFleetFactor.
	// Either way the attacker turns neutral toward the owner and its target is lifted. A line only when the people the
	// player rules is a party, after it has met the other (NoteMet): taken / lost / held / failed (Severity 2), lifted
	// (Severity 1).
	void TickConflict();

	// The diplomacy of the AI stage for the people CivIndex the player does not rule (B3 header section 2.8 - after its
	// projects, settlement and revocations; no draw, no profile read): it drops its friendly stances and a guard; turns
	// neutral toward every people it is hostile to whose system it does not besiege; and, holding no hostility and not
	// resting (no target and Tick >= FleetTargetSinceTick + SiegeRestTicks), opens one toward the first people in Civs order
	// that passes the gate (bordering, no non-aggression treaty, ViewOf at or below AIHostileRelation - toward the people the
	// player rules AIHostileRelationTowardRuled -, its FleetPower above 0 and at least AIHostilePowerRatio times theirs -
	// AIHostilePowerRatioTowardRuled toward that people -, its Pacifism below AIHostilePacifismMax, no other AI hostile toward
	// them, the ruled people not besieging them, and a system of theirs in Systems order that SiegeBlockOf passes with
	// bAssumeHostile and ComputeAttritionPressureIn leaves at 0 - the gate and the target are CosmicConflict::AIAimOf, asked
	// without its lift, B5) - turning hostile breaks the pair's trade treaty
	// (CosmicDiplomacy::TurnHostile, no penalty) and the siege stands at once on that system. A people the player rules is
	// met first and told (CosmicRows::LogTreatyBroken when a treaty broke, then CosmicRows::LogSiegeOnUs).
	void TickAIDiplomacy(int32 CivIndex);

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
	//     in the same order, the Draw row the number falls on is placed on a free slot (a row's share is its chance times the
	//     owner archetype's and the owner's held edicts' factors - B2 - and, for the plague row on a system with a standing
	//     plague on a lane neighbour as the step begins, DT_Rules.PlagueSpreadMultiplier - B3; the factors change the shares,
	//     never the count of draws). Then the Capital rows, civilization by civilization in Civs order.
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
	 * effects, no line (a rebellion still ends its leader and its count). When the owner holds a DT_Edicts row the hazard row
	 * lists in RulerResolutions (CosmicEdicts::HeldAnswerOf, B2), the hazard is resolved instead (CosmicHazards::Resolve,
	 * then FollowUpKnowing and FollowUpNotes - a project's resolution): its numbers are thrown away and no claim rule runs.
	 * Otherwise, in this order: the last-system gate, when the
	 * code of the tag takes the system and it is its owner's last settled system (a kept system ends there: its slot is
	 * emptied without result); the effects (OnExpireEffects, or OnExpireBranchEffects when OutcomeNumber is below
	 * BranchChance), then OnExpireNeighbourEffects; FollowUpKnowing (a system the effects handed to the vessel's people is
	 * known and its giver met, with the meeting line - the one line that can leave before the code of the tag, while the
	 * slot still holds the expiring tag); the code of the tag (Hazard.PreSupernova: CosmicHazards::Destroy, then
	 * DeposeRuler through Apply when the people the vessel rules has no system left; Hazard.Rebellion: the throne;
	 * Hazard.Claim: the join); the slot emptied; the claim rule on that slot; the expiry line (a claim that did not join
	 * tells the resolution line); the record line of a people's irreversible loss (B5 decision 11 - CosmicRows::
	 * LogRecordDestroyed when a settled system was destroyed, CosmicRows::LogRecordAnnexed when a claim joined; Log.Record,
	 * kept untold, one tell number); the voice of a join (its sign, then the fallen's first-person line); FollowUpNotes; then the awareness line (B6a header
	 * section 2.11 - once per run, for the claimant TryPlaceClaim returned).
	 * Should the effects have made the system its owner's last settled system after the gate let it pass (no row of the game can: Data.Integrity keeps TransferSystem out of the expiry effects
	 * and out of OnResolveEffects, which a ResolveHazard on a neighbour runs), the gate is asked again, with an error line,
	 * before the code of the tag takes it; a system kept then has taken the effects already: what they handed to the
	 * vessel's people was known before the grace line, and the hazard notes they left are told after it.
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
	 * a construction does). An answer that could not pay in full this tick waits for its cost (CostPerTickOf, summed over
	 * the people's unpaid answers): a construction of the same people is held back this tick when, for some resource it
	 * spends and the waiting answers need, what it would leave in the stockpile is below that need - so a cheaper
	 * construction cannot keep taking what the answer waits for, and a construction that spends nothing the answers need
	 * goes on. Whether an answer pays is read before its Advance (CanPay of the stockpile and the cost), not from its
	 * progress. DT_Rules.AIAscensionSpeedMultiplier is the rate of the peoples the player does not rule
	 * (not above 0, or not a number: 1, with one error line). After each slot FollowUpKnowing (a system a resolution
	 * handed to the vessel's people), then its completion line (the row's CompleteTemplates, Log.Civ, Severity 1 for the
	 * people the player rules and 0 for the others, slots {Civ} {System} {Project}), then the lines that follow
	 * (FollowUpNotes - the resolution a completion made). No draw of its own.
	 */
	void AdvanceMegaprojects();

	// Stage 6. (a) Every people the player does not rule, in Civs order: it starts its CosmicMegaprojects::Choices
	// (SinceTick 0), no draw (section 4.3, 5.7); then, when CosmicMegaprojects::MaySettle holds, one number from the sim
	// stream, drawn before the targets are looked at and thrown away when there is none, picks a target of SettleTargets
	// to Settle (section 9.2). A people that may not settle draws nothing. Then it revokes, in ActiveEdicts order, the edicts
	// CosmicEdicts::AIRevokesOf names - no draw, no line: an AI people holds no edict in B2. Then its diplomacy
	// (TickAIDiplomacy, B3). (b)
	// UAscensionSubsystem::EvaluateFeats, exactly once per tick and nowhere else in the tick.
	void TickCivAI();

	// Stage 7. The only place where the number of elements of State.Civs changes (section 7.4). (o) Cleanup: first every
	// people with no settled system lets its settlements go (CosmicHazards::LetSettlementsGo - a destruction has done so
	// already in play; when that empties the people the player rules with no deposition or death recorded, an error line says so,
	// for nothing lets go of it then); then every civilization that owns no system (CosmicState::OwnsAnySystem) leaves Civs, in array
	// order, except the one the player rules this tick, which the next tick's cleanup takes once the role-effect stage has
	// let go of it (that rests on the recorded deposition or death - the death pipeline lets go of the people too, B5). The
	// Relations and Ties keys of a removed CivID leave every other civilization (B3), Knowing.PendingContactCivID is cleared when it
	// names one, and the state index is rebuilt. Every other record of the ID stays. No line: the expiry that took the
	// last system has told it. Arrivals and departures are Phase 6a.
	void TickCrossings();

	// Stage 8, only while the calling is Embodied (section 7.4, B1 header section 2.4): a journey's countdown, and on the
	// tick it reaches 0 the move - when the target is still a living lane neighbour of where the vessel stands, else the
	// journey ends there. Returns whether the vessel arrived in this tick (a local of SimulateTick - the arrival card of
	// the knowing stage reads it; a destruction that carried the vessel is no arrival). Counts the open reign's tick.
	bool TickPlayerTravel();

	// Stage 9, only while the calling is Embodied (section 7.4). Reads the world and writes only State.Knowing and the
	// telling: no draw from the sim stream. Runs when State.Tick is a multiple of DT_Rules.SightIntervalTicks (read as at
	// least 1) or when bVesselMoved: Player.CurrentSystemID differs from its value at the start of the tick (a journey's
	// end, or a destruction that carried the vessel). Both are locals of SimulateTick - no member, no saved field
	// (section 4.4: the eye is derived). Whatever places the eye outside the stages (Install, a role change) or widens it
	// in place (the wanderer's scan, B1) writes knowing in its own call.
	// The eye: a ruler at every system its people owns (OwnedSystems order), with the reach of the people's Tier, and
	// only from DT_Rules.FirstSightTick on (before, the places of the eye still order the meetings but add no system); a
	// wanderer at the system it stands in, with the reach of the calling's PersonalTier. Reach =
	// DT_Rules.SightLanesByTier[tier] lanes (a missing or lower entry reads as 1). The look itself is LookFrom: one
	// breadth-first walk from all places of the eye along Lanes, inside the GalaxyRow of each place, notes every system
	// within reach through NoteKnown, in the order of the walk (a set made for the pass only filters what is known
	// already). Then every known system is looked at again: its owner, when not met yet, is met (NoteMet with that
	// system) - nearer peoples first (lanes from the eye without the reach bound; ties by CivID FNameLexicalLess); the
	// gap rule keeps a people unmet until a later look.
	void TickSight(bool bVesselMoved);

	// Stage 10, inside the stage guard (section 7.4; B5 header section 2.2). In the interregnum no record is made (Apply refuses
	// one) and the stage is the leap's countdown: a remaining leap above 0 goes down by one, and on the tick it reaches 0
	// UPlayerRoleSubsystem::Succeed; with none left (no candidate - the silence) nothing happens. Otherwise
	// Effect.DeposeRuler: UPlayerRoleSubsystem::DeposeRuler - or, with the cause Reign.Abdicated,
	// UPlayerRoleSubsystem::AbdicateRuler (B1); Effect.KillPlayer: UPlayerRoleSubsystem::Die (B5); then the record is cleared.
	// It runs no tick: the fast-forward is RunLeap, from the presentation's input.
	void ResolvePendingRoleEffect();

	// The walk shared by ComputeStateHash and ComputeWorldHash.
	uint32 HashState(bool bWorldOnly) const;

	// Whether a tick may not run now (StepOnce, RunHeadless, RunLeap): inside a handler of a Core Logic delegate, inside the
	// tick stages or inside an OnSimTick handler - one error line naming Caller.
	bool RefusesTicks(const TCHAR* Caller) const;

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

	// True while the ten stages and the stream's sync run (B5 widened it to the tenth stage). NewGalaxy, LoadState and
	// SimulateTick ensure against it: the stages hold references into the state. The ensure is a backstop: a delegate fired
	// from inside the stages is a broadcast, and the entry points that replace the state or run a tick refuse a call made
	// during one (IsBroadcasting); StepOnce, RunHeadless and RunLeap refuse inside the stages too.
	bool bInTickStages = false;

	// How deep OnSimTick is being sent (IsSendingSimTick), and whether it is RunLeap's boundary. Never saved.
	int32 SimTickDepth = 0;
	bool bSendingLeapBoundary = false;

	static constexpr float SecondsPerTick = 1.0f;

	// Ticks processed per frame at most. The remainder carries over to the next frame, capped at this many ticks (section 9.4).
	static constexpr int32 MaxTicksPerFrame = 10;
};

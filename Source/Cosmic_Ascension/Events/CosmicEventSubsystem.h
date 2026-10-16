// Cosmic Ascension - Events layer. Design: guideline.md section 6.1, 7.3, 7.4, 7.5, 9.2.
// RULE (section 6.1): files under Events/ must never include UUserWidget or AActor headers.
//
// Two layers (section 7.5).
// - Static functions (Check, CheckAll, Apply, ApplyAll) have no 'this'. They reach the state, the definition data and the
//   stream only through the view or the context handed in: never GetRng(), never the log, never the knowing writers,
//   never a delegate. They work on any FGalaxyState, the live one or a detached copy.
// - Member functions (EvaluateAndFire, FireRow) work on the live state only and add the two things the static layer
//   cannot do: tell lines and fire OnEventFired.
//
// A row is atomic (section 7.4): every effect in array order, then what the vessel's people received is known and the
// people that gave it met (FollowUpKnowing), then its line, then the lines that follow what the effects did (a hazard
// placed or resolved: FollowUpNotes), then OnEventFired. No line and no delegate leaves between two effects,
// so a handler never reads a half-applied row, and a cause is told before what follows from it. Effects are applied in
// place and nothing is queued: what has to wait is already kept by the state (the pending role effect, resolved by the
// tenth stage; a civilization without systems, removed by the crossing stage).
//
// The three effects that reach into the hazard stage (SpawnHazard, ResolveHazard, TransferSystem) go through the world
// functions of Sim/CosmicHazards.h, the one path for each. SpawnHazard and ResolveHazard leave notes in the context and the
// member layer tells what the notes say. TransferSystem leaves a note of the system received, never told: the member layer
// makes a system the vessel's people received known, and meets the people it came from, before any line
// (FollowUpKnowing). A hazard it ends goes untold.

#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "GameplayTagContainer.h"
#include "Math/RandomStream.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "CosmicEventSubsystem.generated.h"

struct FCosmicDataTables;
struct FCosmicEventDef;
struct FCosmicRulesDef;
struct FCosmicStateIndex;

/**
 * What a fired event row tells the presentation layer (section 7.5). Plain values only: every parameter of a dynamic
 * delegate has to be a reflected type, and a handler is never handed the state or the sim stream. Never saved.
 */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FCosmicEventNotice
{
	GENERATED_BODY()

	// DT_Events row name.
	UPROPERTY(BlueprintReadOnly)
	FName EventRow;

	// Event.* of the row.
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Category;

	// 0 normal, 1 notable, 2 mythic.
	UPROPERTY(BlueprintReadOnly)
	uint8 Severity = 0;

	// Subject system and subject civilization of the fire. NAME_None = none.
	UPROPERTY(BlueprintReadOnly)
	FName SystemID;

	UPROPERTY(BlueprintReadOnly)
	FName CivID;

	UPROPERTY(BlueprintReadOnly)
	int64 Tick = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCosmicEvent, const FCosmicEventNotice&, Notice);

/**
 * Who and where an evaluation is about (section 7.5). Indices, not IDs and not pointers: the caller holds its subject from
 * its own loop, an index costs no lookup, and it stays valid across Apply because no effect changes the number of
 * elements of Civs or Systems (only the crossing stage does, and no subject is kept across it).
 */
struct FCosmicEvalSubject
{
	// Index into FGalaxyState.Civs: the context civilization. INDEX_NONE = none: Civ conditions are false, Civ effects are
	// turned away. CivHasNotable and NameNotable with an empty Key mean this civilization: a caller that means the owner
	// of the current system puts that owner here.
	int32 CivIndex = INDEX_NONE;

	// Index into FGalaxyState.Systems. INDEX_NONE = no system: System conditions are false, System effects are turned away.
	int32 SystemIndex = INDEX_NONE;

	// The other party: a CivID or a TranscendentID. NAME_None = none. Plain data: which Key selectors read it is
	// section 7.3. EvaluateAndFire leaves it empty; FireRow and stage code hand their own value on.
	FName OtherID;

	// DT_Galaxies row of the subject (section 7.2). The caller copies it from the subject civilization or system, which
	// NewGalaxy has stamped (NAME_None only in a state made by hand). Carried from the first header on so that the rules
	// that count lanes never need a second signature; no condition or effect of the vertical slice reads it.
	FName GalaxyRow;

	// The row behind the evaluation: event, hazard, feat, encounter or transcendent row. KillPlayer, DeposeRuler and
	// NameNotable store it. FireRow sets it to the fired row.
	FName SourceRow;

	// Lower bound of the two life-relative conditions (section 4.4): MegaprojectComplete and TranscendentSealed are true
	// only when the completion or sealing tick is above zero and not below this. 0 = any time: events, hazards and the
	// feats of AI civilizations. Only the judgment of the player's feats puts Calling.BornTick here, so nothing else in
	// the world depends on the birth tick of the vessel.
	int64 SinceTick = 0;
};

/** A change of the world that an effect made and the member layer has to follow with lines (section 7.5). */
enum class ECosmicWorldNote : uint8
{
	// A hazard was placed: its outbreak line, then the OutbreakRow of its row.
	HazardPlaced,
	// A hazard was resolved: its resolution line.
	HazardResolved,
	// A system changed hands to a people (CivIndex is the new owner, FromCivIndex the old one). Followed by
	// FollowUpKnowing, never told.
	SystemReceived
};

/** One note: what changed, where, about whom. Plain values, made by the world functions of Sim/CosmicHazards.h. */
struct FCosmicWorldNote
{
	ECosmicWorldNote Kind = ECosmicWorldNote::HazardPlaced;

	// Index into FGalaxyState.Systems.
	int32 SystemIndex = INDEX_NONE;

	// The owner at the moment of the change, index into FGalaxyState.Civs: the people the line is about. For
	// SystemReceived, the new owner.
	int32 CivIndex = INDEX_NONE;

	// DT_Hazards row of the hazard (none for SystemReceived).
	FName HazardRow;

	// SystemReceived only: the owner the system was taken from, index into FGalaxyState.Civs (INDEX_NONE for a wild
	// system). Last, so that the brace lists of the hazard notes stay as they are.
	int32 FromCivIndex = INDEX_NONE;
};

/** Which of a hazard's own lines (FHazardDef: OutbreakTemplates, ExpireTemplates, ResolveTemplates). */
enum class ECosmicHazardLine : uint8
{
	Outbreak,
	Expiry,
	Resolution
};

/**
 * Read-only side of an evaluation: what Check may see. It holds no stream, on purpose: every draw method of
 * FRandomStream is const, so a const reference to a context that held one would not stop a draw. A view without one does.
 */
struct FCosmicEvalView : public FCosmicEvalSubject
{
	FCosmicEvalView(const FGalaxyState& InState, const FCosmicStateIndex& InIndex, const FCosmicDataTables& InTables, const FCosmicRulesDef& InRules)
		: State(InState), Index(InIndex), Tables(InTables), Rules(InRules)
	{
	}

	const FGalaxyState& State;

	// ID -> index, for the few lookups by ID (a Key selector such as Former, the system the vessel stands in). It has
	// to have been rebuilt for State.
	const FCosmicStateIndex& Index;

	const FCosmicDataTables& Tables;
	const FCosmicRulesDef& Rules;
};

/**
 * Mutable side of an evaluation: what Apply may change and the one stream it may draw from (section 7.5, 9.2).
 * Stage code that applies effects or fires a row by name builds it with UGalaxySimulationSubsystem::MakeEvalContext, over
 * the live state and the sim stream; EvaluateAndFire is handed the state and the stream and builds the contexts of its
 * own fires. A test may build one over a detached state and a stream of its own.
 * A plain C++ handle, not a USTRUCT: it refers to the state and to a stream, so it can be neither a UPROPERTY nor a
 * delegate parameter. Make one, use it and drop it inside one tick stage or one input call: State.Civs and State.Systems
 * must not change size while it lives.
 */
struct FCosmicEvalContext : public FCosmicEvalSubject
{
	FCosmicEvalContext(FGalaxyState& InState, FRandomStream& InStream, const FCosmicStateIndex& InIndex, const FCosmicDataTables& InTables, const FCosmicRulesDef& InRules)
		: State(InState), Stream(InStream), Index(InIndex), Tables(InTables), Rules(InRules)
	{
	}

	// Not copyable: a copy would carry bPerTickSource (and whatever a later work item hangs on a context) to a place that did
	// not ask for it. A second context over the same state (FireRow's own) is built from the five references and takes
	// its subject by assignment.
	FCosmicEvalContext(const FCosmicEvalContext&) = delete;
	FCosmicEvalContext(FCosmicEvalContext&&) = default;

	// The same subject over the same state, without the stream.
	FCosmicEvalView View() const
	{
		FCosmicEvalView Result(State, Index, Tables, Rules);
		static_cast<FCosmicEvalSubject&>(Result) = *this;
		return Result;
	}

	FGalaxyState& State;

	// Every draw of an effect comes from here and from nowhere else.
	FRandomStream& Stream;

	const FCosmicStateIndex& Index;
	const FCosmicDataTables& Tables;
	const FCosmicRulesDef& Rules;

	// True only while the economy stage applies a per-tick source (FTranscendentDef.PressureEffectsPerTick; edict and
	// trait rows from Phase 4). MultiplyIncome is applied only then: IncomePerTick is rebuilt every tick, so a multiplier
	// from anywhere else would be gone before it reached the stockpile (section 5.8).
	bool bPerTickSource = false;

	// What the world functions of Sim/CosmicHazards.h changed through this context, in order: the hazard notes wait for
	// UCosmicEventSubsystem::FollowUpNotes, the SystemReceived notes for FollowUpKnowing (the first thing FollowUpNotes
	// does; a settlement leaves only that note: the ruler's command follows it with FollowUpKnowing, the AI stage drops
	// it). FireRow fires on a context of its own, so the notes of a fired row are followed by that fire and never by its
	// caller.
	TArray<FCosmicWorldNote, TInlineAllocator<2>> Notes;
};

/**
 * Condition evaluation, event slots, effect application, cooldowns (section 7.5). One evaluator (Check) and one applier
 * (Apply) serve events, hazards, feats, the per-tick sources and, from Phase 4, encounters and petitions.
 * Holds no state of its own, no cache and no member variable: cooldowns and flags live in FGalaxyState, and the
 * definition rows are read from the tables at every call, so tables injected by a test are seen at once.
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// EvaluateAndFire drops the cooldown entries that are over on ticks that are a multiple of this, so the saved map
	// stays bounded by the rows that fired lately, also when civilizations are removed (section 9.4).
	static constexpr int64 CooldownSweepTicks = 600;

	// ---- Static layer: only what the view or the context hands in ----

	/**
	 * One condition against the subject of View. A pure function of the view: it writes nothing, cannot draw (the view has
	 * no stream) and never reads Knowing, Log, Chronicle or TellRngState (section 8.1).
	 * - A condition whose subject the view lacks (a Civ condition without a civilization, a System condition without a
	 *   system) is false.
	 * - Every Player condition is false unless Calling.LifeState is Embodied: during the interregnum there is no vessel
	 *   (FPlayerEntityState keeps the dead one until Install). Together with the Embodied test of the vessel effects and
	 *   the role effects in Apply, that is the whole exclusion of the player from the leap: the vocabulary has no negation
	 *   that could turn it around (FlagNotSet reads flags only).
	 * - MegaprojectComplete and TranscendentSealed compare with View.SinceTick, never with the life state.
	 *   MegaprojectComplete reads the completion record of its Key's row on the subject system
	 *   (CosmicMegaprojects::HasCompleted); an empty Key is false.
	 * - CivResearchAtLeast resolves its Key only as a key of the civilization's Research (a research field).
	 * - Tier values are tier numbers 1..5 (section 7.3).
	 * - A condition whose Key names something the view cannot resolve is false. The other party of CivRelationAtLeast
	 *   (its Key, or the OtherID of the view when the Key is empty) resolves only when it is the CivID of a living
	 *   civilization, the TranscendentID of a transcendent in State.Transcendents, or the name of a celestial row of
	 *   DT_Transcendents (it never has a state instance and is still a party; a fallen or independent row is a party
	 *   only as an instance of the run). The tick counts of CivStabilityBelowFor and CivFallenSphereTicksReached
	 *   resolve only when their Key is digits alone (nine at most) and above zero.
	 * - SystemPressureAtLeast reads P of the subject system from UGalaxySimulationSubsystem::ComputePressureIn over the
	 *   state of the view. It is the whole sum: the term of the civilization that owns the system and the term of a
	 *   wanderer standing there count like the term of a transcendent (section 16 #62).
	 */
	static bool Check(const FCosmicCondition& Condition, const FCosmicEvalView& View);

	// AND over Conditions in array order, stopping at the first false. An empty array is true.
	static bool CheckAll(TConstArrayView<FCosmicCondition> Conditions, const FCosmicEvalView& View);

	/**
	 * One effect on the subject of Ctx, in place. World state only: it never tells a line, never writes Knowing, never
	 * fires a row or a delegate and never changes the number of elements of Civs or Systems. Returns false when a guard
	 * turned the effect away or its subject is missing: nothing in FGalaxyState was changed (NameNotable has still drawn
	 * its one number from Ctx.Stream, unless a per-tick source asked), and the caller goes on with the next effect.
	 * - A per-tick source (Ctx.bPerTickSource) applies the seven effects that change an amount and no other:
	 *   AddResource, MultiplyIncome, ShiftEthics, ChangeStability, ChangeRelation, ChangeReverence, ChangePopulation.
	 *   Every other effect is turned away before anything is read or drawn, so the economy stage never draws, never
	 *   records a role effect, never touches the vessel and never sets a mark that a row sets once (section 7.5). An
	 *   effect type appended to the enum is refused there until it joins the list.
	 * - KillPlayer and DeposeRuler never run here. They are written to FCallingState (PendingEffect, PendingCause = the
	 *   effect's Tag, PendingSourceRow = Ctx.SourceRow) and the tenth stage resolves them (section 7.4). Both are recorded
	 *   only while Calling.LifeState is Embodied and only while PendingEffect is empty: first recorded wins.
	 *   KillPlayer reaches the vessel in two cases only. A subject with a system: the vessel is a wanderer standing in
	 *   that system (what happens to a system never kills a ruler, section 4.4: a ruler's failure is deposition). A
	 *   subject without a system: the subject civilization is the one the vessel rules (its extinction), or the subject
	 *   names no civilization. Never within DT_Rules.BirthProtectionTicks of Calling.BornTick: a shielded kill leaves the
	 *   slot free for a later record. DeposeRuler is recorded only when the subject civilization is the one the vessel
	 *   rules. Stage code that records a role effect of its own (conquest of a last system, extinction) goes through
	 *   here too: the rules of the record live in one place.
	 * - PlayerStatDelta and GrantPower change the vessel and the calling: like the two role effects they are turned away
	 *   unless Calling.LifeState is Embodied.
	 * - NameNotable holds the four guards and the eviction of section 7.5. It draws exactly one number from Ctx.Stream
	 *   per application, before any guard of its own, and throws it away when no record is made: the number of draws
	 *   of a row depends on its effect list and never on name banks, tiers or caps. It is the only draw in this
	 *   function, and a per-tick source never reaches it.
	 * - MultiplyIncome scales FCivilizationState.IncomePerTick and is applied only while Ctx.bPerTickSource is set.
	 *   One multiplication, income x Value, with a Key (one resource) or without (all five). A Value below zero reads
	 *   as 0: an income never turns negative.
	 * - The eight effects that add to an amount or scale one (AddResource, MultiplyIncome, ShiftEthics, ChangeStability,
	 *   ChangeRelation, ChangeReverence, ChangePopulation, PlayerStatDelta) are turned away when their Value is not a
	 *   finite number: nothing that is not a number is clamped to a bound or written into the state.
	 * - DurationTicks is not read: Apply keeps no record that could count it down (section 16 #27). Data.Integrity keeps
	 *   a value above zero out of every effect array but the per-tick sources.
	 * - SpawnHazard places the DT_Hazards row whose HazardTag is the effect's Tag on the subject system, through
	 *   CosmicHazards::Place (refused there: a system that is not settled or whose slot is taken, a Capital row away from
	 *   its owner's capital, a ClaimRule row always). ResolveHazard resolves the hazard of the subject system through
	 *   CosmicHazards::Resolve; with a Tag, only a hazard that matches it (MatchesTag). TransferSystem hands the subject
	 *   system to the civilization its Key names (a living CivID; an empty Key = to no one) through
	 *   CosmicHazards::Transfer, which never takes or lets go the last settled system of a civilization: only stage code
	 *   behind the last-system gate can (section 4.5, 7.4). SpawnHazard and ResolveHazard leave notes in Ctx.Notes, and
	 *   whoever applied them tells them after its own line (FollowUpNotes); TransferSystem to a people leaves the note of
	 *   the system received, which whoever applied it follows before its own line (FollowUpKnowing). A per-tick source
	 *   turns all three away.
	 * - An effect whose Key names something the context cannot resolve is turned away. ChangeRelation resolves its
	 *   other party as CivRelationAtLeast does and writes no entry for one that does not resolve: the keys of
	 *   Relations are saved, and they are living civilizations, transcendents of the run and celestial rows only.
	 * - NameNotable closes the open record of the same role with LastTick = max(Tick, 1): 0 marks an open record, so a
	 *   record closed before the first tick still reads as closed (section 16 #35).
	 */
	static bool Apply(const FCosmicEffect& Effect, FCosmicEvalContext& Ctx);

	// Apply over Effects in array order. A turned-away effect does not stop the rest. Like Apply it only changes the world:
	// stage code that applies an array of its own (a hazard's expiry and resolution, the cost of a feat; from later work
	// items the outcome of an encounter) follows the systems received first (FollowUpKnowing), then tells its own line -
	// afterwards, never in between - and then follows the other notes (FollowUpNotes). A per-tick source leaves no note
	// and tells no line.
	static void ApplyAll(TConstArrayView<FCosmicEffect> Effects, FCosmicEvalContext& Ctx);

	// Key of FGalaxyState.EventCooldownUntilTick for a slot row and a civilization: "<row>@<CivID>". Keys are saved, so
	// the format is fixed. Rows fired by name have no key.
	static FName CooldownKey(FName EventRow, FName CivID);

	// ---- Member layer: the live state ----

	/**
	 * The event slots of one tick: the last step of the hazard stage (section 7.4). State has to be the state the
	 * simulation subsystem owns and Stream the stream that stage was handed.
	 * A civilization has a slot in this tick when it owns a settled system and State.Tick modulo DT_Rules.EventSlotTicks
	 * equals its phase, CosmicSeed::DeriveForName(State.Seed, CivID, CosmicSeed::Purpose::EventSlot) modulo
	 * EventSlotTicks: one slot per civilization every EventSlotTicks ticks, at a phase that never moves when other
	 * civilizations come or go. (An EventSlotTicks below 1 is read as 1 and an EventChancePerSlot below 0, or one that is
	 * not a number, as 0, each with an error line that names that knob alone: the ranges are a lint, the guard is here.)
	 * 1. Draws: one number in [0, 1) from Stream for every civilization that has a slot, in Civs array order, all of
	 *    them before any row is looked at. These are the only draws of this function itself, and their count follows the
	 *    seed, the tick, the ownership of systems and the CivIDs alone: not DT_Events (no table, no row and forty rows
	 *    draw the same), not cooldowns or weights, not the life state.
	 * 2. Each of those civilizations in Civs array order, against the state as it is at its turn. A row is evaluated on
	 *    the units its Subject names: the civilization once (current system = its capital while that is one of its
	 *    settled systems), or each of its settled systems in OwnedSystems order.
	 *    a. Standing rows: the (row, unit) pairs whose conditions pass and whose cooldown is over are gathered in row-name
	 *       order (FNameLexicalLess; the units of one row keep OwnedSystems order), never in the table's own order, and
	 *       fired one by one. Conditions are looked at first and the cooldown key is built at most once per row, only
	 *       for a row with a unit that passed (section 9.4); Lottery rows are gathered the same way. Right before a pair fires three things are checked again: the unit is still a settled
	 *       system of the civilization, the conditions, the cooldown. The key is per row and civilization, so a Standing
	 *       row with a cooldown fires for at most one unit per slot (the first that passes) and a row with
	 *       CooldownTicks 0 fires for every unit. No draw.
	 *    b. Lottery rows: the (row, unit) pairs whose conditions pass and whose cooldown is over are laid out in the same
	 *       order, each with the share BaseWeight x weight factor x DT_Rules.EventChancePerSlot. The weight factor is
	 *       the product, over the unsealed transcendents in the order of State.Transcendents whose sphere holds the
	 *       unit's system or lies one lane from it, of min(3, EventWeightModifiers[Category of the row] x (1 +
	 *       DT_Rules.SphereEventWeightStep x size of the sphere)); 1 when no transcendent names that Category. The
	 *       unit's system of a Civilization row is the capital. The number of step 1
	 *       picks the pair it falls on, or none: at most one Lottery row per civilization and slot. Shares that add up to
	 *       more than 1 are scaled down together, and then every number picks a pair: the number is multiplied by the
	 *       total instead of each share being divided, so no number is left without a pair by rounding.
	 *    Rows whose Trigger is Named are never looked at.
	 * A fired slot row first writes its cooldown key (State.Tick + CooldownTicks, when above zero), then goes through
	 * the sequence of FireRow. On ticks that are a multiple of CooldownSweepTicks the call first drops the cooldown
	 * entries that are over: an expired entry and a missing one mean the same.
	 * Returns the number of rows fired. Refused with one error line, and nothing done, when State is not the live state
	 * or when called from a handler of a Core Logic delegate.
	 */
	int32 EvaluateAndFire(FGalaxyState& State, FRandomStream& Stream);

	/**
	 * Fires the DT_Events row RowName for the subject of Ctx when every condition of the row passes; false, with nothing
	 * drawn, written or told, when the row is missing or a condition fails. This is how a Named row fires: the row at the
	 * outbreak of a rebellion, the grace rows, the book found at a Society breakthrough (section 4.1, 4.5, 5.11). It neither reads nor writes a cooldown key and makes no
	 * draw of its own. Ctx gives the subject: the fire runs on a context of its own over the same state and stream, with
	 * SourceRow = the row and bPerTickSource off, and leaves the subject of Ctx as it is.
	 * One fire is one fixed sequence, the same for all three triggers:
	 * 1. every effect in array order through Apply, then FollowUpKnowing (a system the vessel's people received is known,
	 *    and the people it came from met, before the line);
	 * 2. the line, through UCosmicLogSubsystem::PushVariants: the row's LogTemplates as its LogCategory and Severity, with
	 *    the row as SourceRow. Subject Civilization tells it about the civilization alone, SettledSystem about the subject
	 *    system and the subject civilization. A row with LogTemplates, a SignRow and a MinTierToReveal of 2 or above
	 *    always tells a second line from that DT_LogTemplates row, in that row's own Category (Log.Sign by
	 *    Data.Integrity). With a MinTierToReveal of 1 or below no viewer tier lies under the first line, and a row
	 *    without LogTemplates tells nothing at all: no second line. The viewer tier windows of the two lines are
	 *    complementary (MinTierToReveal), so the windows add no tell draw: the row's own lines draw 0, 1 or 2 times by
	 *    its data. Knowing adds two more: the far line that stands in for an out-of-scope Severity 2 line, and in step 1
	 *    one meeting line for each giver FollowUpKnowing meets (unmet before, and let through by the gap rule - so that
	 *    one depends on the viewer's tier too, and comes in tell-all mode as well; section 8.1). The line is told whether
	 *    or not an effect was turned away. Slot {Hazard}: the hazard the subject system held before the effects;
	 * 3. FollowUpNotes: the lines of what the effects did (a hazard placed or resolved), in the order it was done;
	 * 4. OnEventFired, while the global broadcast gate is open.
	 * Cooldown keys, sim draw counts and the result never depend on whether a line was told. Outside a tick the caller
	 * syncs the random stream state afterwards. Refused like EvaluateAndFire.
	 */
	bool FireRow(FName RowName, const FCosmicEvalContext& Ctx);

	/**
	 * One of the three lines of the DT_Hazards row HazardRow (section 4.1), about SystemIndex and CivIndex: the caller names
	 * the people, because an expiry may already have taken the system from it. Told through PushVariants with the row's
	 * LogCategory and Severity, SourceRow = the row, focus on the system and the people, slots {Civ} {System} {Hazard}.
	 * A missing row or empty templates tell nothing (no draw). Refused like FireRow.
	 */
	void TellHazardLine(const FCosmicEvalContext& Ctx, ECosmicHazardLine Line, FName HazardRow, int32 SystemIndex, int32 CivIndex);

	/**
	 * Follows the SystemReceived notes of Ctx before any line (section 7.5, 8.1): takes them out of Ctx and, for every
	 * system received by the people the vessel rules (Embodied, Ruler), calls UGalaxySimulationSubsystem::NoteKnown and
	 * then NoteMet for the people it came from, at that system - the other party of the event (section 8.1), under the
	 * gap rule and with its meeting line - so that the lines about the gift are told, unless the gap rule keeps the giver
	 * unmet: a line about the giver then stays out of scope (section 8.1). The other notes stay, in their order, for
	 * FollowUpNotes. Called after the effects and before the own line by FireRow, the hazard expiry (after its neighbour
	 * effects, before its code hooks and the second ask of the last-system gate), the completion of a project and the
	 * judgment of a feat (the giver there is the ascender the line is about); by the ruler's
	 * settlement command; and first of all by FollowUpNotes. No sim draw (a meeting line draws from the tell stream).
	 * Refused like FollowUpNotes: over a detached state or from a handler, an error line and the notes stay.
	 */
	void FollowUpKnowing(FCosmicEvalContext& Ctx);

	/**
	 * Follows Ctx.Notes after the caller's own line (section 7.5). Follows the SystemReceived notes first
	 * (FollowUpKnowing), then takes the other notes out of Ctx and, in their order:
	 * HazardPlaced tells the outbreak line and fires the row's OutbreakRow through FireRow, with that system and people as
	 * subject over the state and the stream of Ctx (a NameNotable of that row draws from it); HazardResolved tells the
	 * resolution line. Every user of the world functions that can leave a hazard note ends with it: FireRow after the
	 * row's line, the hazard stage after each placement and after each expiry line. A settlement leaves only the
	 * SystemReceived note: the ruler's command follows it with FollowUpKnowing alone, and the AI stage drops it (the
	 * people it settles for is never the vessel's). Refused like FireRow, leaving the notes where they are. Outbreak
	 * rows fired from within the follow-up of an outbreak row stop at a fixed depth with an error line instead of running
	 * the stack out: no row of the game nests (Data.Integrity).
	 */
	void FollowUpNotes(FCosmicEvalContext& Ctx);

	// Fired once for every row that fired, after its effects and its lines, while the global broadcast gate is open
	// (section 6.1). Fired synchronously, also from inside a tick stage, and also for rows about peoples the calling has
	// not met: the presentation filters by what is known (section 6.4). Same contract as OnLogEntry (section 9.4): a
	// handler may read the state and call SetTimeScale; it must not call NewGalaxy, LoadGalaxy, SaveCurrentGalaxy,
	// StepOnce or RunHeadless, nor EvaluateAndFire, FireRow or Apply.
	UPROPERTY(BlueprintAssignable)
	FOnCosmicEvent OnEventFired;

private:
	// Steps 1 to 3 of FireRow for a row already found. Conditions and the cooldown key are the caller's business.
	void FireResolved(FName RowName, const FCosmicEventDef& Row, FCosmicEvalContext& Ctx);
};

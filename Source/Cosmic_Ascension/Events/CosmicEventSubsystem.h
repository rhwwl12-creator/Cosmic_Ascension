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
// A row is atomic (section 7.4): every effect in array order, then its line, then OnEventFired. No line and no delegate
// leaves between two effects, so a handler never reads a half-applied row. Effects are applied in place and nothing is
// queued: what has to wait is already kept by the state (the pending role effect, resolved by the tenth stage; a
// civilization without systems, removed by the crossing stage).
//
// Not here yet: the three effects that reach into the hazard stage (SpawnHazard, ResolveHazard, TransferSystem) are
// turned away until the hazard step of Phase 2 gives each its one path. What has to follow such a change of the world
// (a system the vessel's civilization received becomes known, the hazard's own line, the outbreak row) is added to this
// header in that step, with the code that produces it.

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

	// DT_Galaxies row of the subject (section 7.2). The caller copies it from the subject civilization or system;
	// NAME_None until the Phase 2 generation change stamps the state. Carried from the first header on so that the rules
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
 * Inside a tick UGalaxySimulationSubsystem::MakeEvalContext builds it over the live state and the sim stream, and the
 * stages hand it on; a test may build one over a detached state and a stream of its own.
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

	// Not copyable: a copy would carry bPerTickSource (and whatever a later step hangs on a context) to a place that did
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
	 * - Tier values are tier numbers 1..5 (section 7.3).
	 * - A condition whose Key names something the view cannot resolve is false.
	 */
	static bool Check(const FCosmicCondition& Condition, const FCosmicEvalView& View);

	// AND over Conditions in array order, stopping at the first false. An empty array is true.
	static bool CheckAll(TConstArrayView<FCosmicCondition> Conditions, const FCosmicEvalView& View);

	/**
	 * One effect on the subject of Ctx, in place. World state only: it never tells a line, never writes Knowing, never
	 * fires a row or a delegate and never changes the number of elements of Civs or Systems. Returns false when a guard
	 * turned the effect away or its subject is missing: nothing in FGalaxyState was changed (NameNotable has still drawn
	 * its one number from Ctx.Stream), and the caller goes on with the next effect.
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
	 *   per application, before any guard, and throws it away when no record is made: the number of draws of a row
	 *   depends on its effect list and never on name banks, tiers or caps. It is the only draw in this function.
	 * - MultiplyIncome scales FCivilizationState.IncomePerTick and is applied only while Ctx.bPerTickSource is set.
	 * - DurationTicks is not read: Apply keeps no record that could count it down (section 16 #27). Data.Integrity keeps
	 *   a value above zero out of every effect array but the per-tick sources.
	 * - SpawnHazard, ResolveHazard and TransferSystem are turned away with one error line until the hazard step gives
	 *   each its one path. TransferSystem will never take or release the last owned system of a civilization: only
	 *   stage code behind the last-system gate can do that (section 4.5, 7.4).
	 * - An effect whose Key names something the context cannot resolve is turned away.
	 */
	static bool Apply(const FCosmicEffect& Effect, FCosmicEvalContext& Ctx);

	// Apply over Effects in array order. A turned-away effect does not stop the rest. Like Apply it only changes the world:
	// stage code that applies an array of its own (the per-tick sources; from later steps hazard expiry and resolution,
	// the cost of a feat, the outcome of an encounter) tells its own line afterwards, never in between.
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
	 * civilizations come or go. (An EventSlotTicks below 1 is read as 1: the range is a lint, the guard is here.)
	 * 1. Draws: one number in [0, 1) from Stream for every civilization that has a slot, in Civs array order, all of
	 *    them before any row is looked at. These are the only draws of this function itself, and their count follows the
	 *    seed, the tick, the ownership of systems and the CivIDs alone: not DT_Events (no table, no row and forty rows
	 *    draw the same), not cooldowns or weights, not the life state.
	 * 2. Each of those civilizations in Civs array order, against the state as it is at its turn. A row is evaluated on
	 *    the units its Subject names: the civilization once (current system = its capital while that is one of its
	 *    settled systems), or each of its settled systems in OwnedSystems order.
	 *    a. Standing rows: the (row, unit) pairs whose conditions pass and whose cooldown is over are gathered in row-name
	 *       order (FNameLexicalLess; the units of one row keep OwnedSystems order), never in the table's own order, and
	 *       fired one by one. Right before a pair fires three things are checked again: the unit is still a settled
	 *       system of the civilization, the conditions, the cooldown. The key is per row and civilization, so a Standing
	 *       row with a cooldown fires for at most one unit per slot (the first that passes) and a row with
	 *       CooldownTicks 0 fires for every unit. No draw.
	 *    b. Lottery rows: the (row, unit) pairs whose conditions pass and whose cooldown is over are laid out in the same
	 *       order, each with the share BaseWeight x weight factor x DT_Rules.EventChancePerSlot (shares that add up to
	 *       more than 1 are scaled down together). The number of step 1 picks the pair it falls on, or none: at most one
	 *       Lottery row per civilization and slot.
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
	 * outbreak of a rebellion, the grace rows (section 4.1, 4.5). It neither reads nor writes a cooldown key and makes no
	 * draw of its own. Ctx gives the subject: the fire runs on a context of its own over the same state and stream, with
	 * SourceRow = the row and bPerTickSource off, and leaves the subject of Ctx as it is.
	 * One fire is one fixed sequence, the same for all three triggers:
	 * 1. every effect in array order through Apply;
	 * 2. the line, through UCosmicLogSubsystem::PushVariants: the row's LogTemplates as its LogCategory and Severity, with
	 *    the row as SourceRow. Subject Civilization tells it about the civilization alone, SettledSystem about the subject
	 *    system and the subject civilization. A row with a SignRow always tells a second line from that DT_LogTemplates
	 *    row. The viewer tier windows of the two lines are complementary (MinTierToReveal), so the number of tell draws
	 *    of a fire does not depend on the viewer's tier: 0, 1 or 2 by the row's data. The only other tell draw is the far
	 *    line that stands in for an out-of-scope Severity 2 line (section 8.1). The line is told whether or not an effect
	 *    was turned away;
	 * 3. OnEventFired, while the global broadcast gate is open.
	 * Cooldown keys, sim draw counts and the result never depend on whether a line was told. Outside a tick the caller
	 * syncs the random stream state afterwards. Refused like EvaluateAndFire.
	 */
	bool FireRow(FName RowName, const FCosmicEvalContext& Ctx);

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

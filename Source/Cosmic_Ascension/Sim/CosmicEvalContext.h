// Cosmic Ascension - Sim layer. Design: guideline.md section 7.5, 9.2.
// Evaluation context types (moved from Events/CosmicEventSubsystem.h).
// Plain C++ handles for condition evaluation and effect application across Sim and Events.
// RULE (section 6.1): files under Sim/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Math/RandomStream.h"
#include "Core/GalaxyState.h"

struct FCosmicDataTables;
struct FCosmicRulesDef;
struct FCosmicStateIndex;

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

	// True only while the economy stage applies a per-tick source (FEdictDef.EffectsPerTick - B2,
	// FTranscendentDef.PressureEffectsPerTick; trait rows from Phase 4). MultiplyIncome is applied only then: IncomePerTick is rebuilt every tick, so a multiplier
	// from anywhere else would be gone before it reached the stockpile (section 5.8).
	bool bPerTickSource = false;

	// What the world functions of Sim/CosmicHazards.h changed through this context, in order: the hazard notes wait for
	// UCosmicEventSubsystem::FollowUpNotes, the SystemReceived notes for FollowUpKnowing (the first thing FollowUpNotes
	// does; a settlement leaves only that note: the ruler's command follows it with FollowUpKnowing, the AI stage drops
	// it). FireRow fires on a context of its own, so the notes of a fired row are followed by that fire and never by its
	// caller.
	TArray<FCosmicWorldNote, TInlineAllocator<2>> Notes;
};


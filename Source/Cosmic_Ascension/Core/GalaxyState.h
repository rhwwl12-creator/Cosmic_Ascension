// Cosmic Ascension - Core layer. Design: guideline.md section 7.2 (runtime state), 8.1 (log entry), 9.1 (save), 9.2 (determinism).
// All runtime state lives in FGalaxyState as plain USTRUCTs (section 6.2). No UObject graph, no Actor or Widget state.
//
// Save compatibility (section 9.1):
// - Adding a field with a default value is compatible. Removing a field or changing its meaning needs a SaveVersion bump
//   and a migration function.
// - Never change the default member initializer of a saved field once saves exist: treat it as a meaning change.
//   SaveGameToSlot writes a delta against the class default object, so a member outside a TArray element (FGalaxyState,
//   Player, Calling and structs nested directly in them) is not written when it equals its initializer and reloads as the
//   loading build's initializer. TArray element structs are written in full.
// - Identifiers are FName, classification is FGameplayTag. Tag and row names that land in a save are fixed from now on.
// - Nothing here counts souls, salvation or damnation (section 4.2, 8.3). The only size of the soul economy is
//   FTranscendentState.InfluencedSystems.
//
// Determinism (section 9.2):
// - FText is display only: stored where noted, never part of ComputeStateHash. FGalaxyState.GalaxyID is the save slot
//   identity and is not hashed either. Every other field is hashed.
// - ComputeStateHash hashes FName and FGameplayTag as their lower-cased string, never GetTypeHash, a name index or raw
//   struct memory: those differ per process and would break "save, restart, load, same hash".
// - FGameplayTagContainer fields (Traits, StatusTags, Powers, Flags): ComputeStateHash hashes the explicit tags only, never
//   the derived parent tags (transient, rebuilt on load).
// - TMap fields (Research, ResearchFocus, Relations, EventCooldownUntilTick): sim code and ComputeStateHash never depend
//   on their iteration order. Sort the keys with FNameLexicalLess, or iterate Civs / Transcendents and Find. After a
//   Remove, a later Add reuses the freed slot in a running game but appends in a loaded one.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"
#include "GameplayTagContainer.h"
#include "Core/CosmicTypes.h"
#include "GalaxyState.generated.h"

/**
 * One narrative log line (section 8.1). UCosmicLogSubsystem::Push renders the template and stores the flattened result
 * (culture-invariant text): the save holds no format history, and the line keeps the language it was pushed in (section 7.5, 8.5).
 */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FCosmicLogEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int64 Tick = 0;

	// Log.Civ / Crisis / Conflict / Discovery / Ascension / Transcendent / Sign / Calling / Claim / Record.
	// Claim and Record entries always move to FGalaxyState.Chronicle (section 8.4).
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Category;

	// 0 normal, 1 notable, 2 mythic (section 8.2).
	UPROPERTY(BlueprintReadOnly)
	uint8 Severity = 0;

	// Display only, not hashed.
	UPROPERTY(BlueprintReadOnly)
	FText Message;

	// Camera focus target when the entry is clicked. NAME_None = none.
	UPROPERTY(BlueprintReadOnly)
	FName FocusSystemID;

	// Row that produced the entry (event / hazard / encounter / feat / log-template row). Chronicle aggregation and tests read it.
	UPROPERTY(BlueprintReadOnly)
	FName SourceRow;
};

/** One star system (section 7.2). The smallest unit of the map: no planet surface, no per-system population (section 12.2). */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FStarSystemState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName SystemID;

	// Display only, not hashed.
	UPROPERTY(BlueprintReadOnly)
	FText DisplayName;

	// Viewport placement.
	UPROPERTY(BlueprintReadOnly)
	FVector Position = FVector::ZeroVector;

	// Adjacent SystemIDs.
	UPROPERTY(BlueprintReadOnly)
	TArray<FName> Lanes;

	// Star.Class.*
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag StarClass;

	UPROPERTY(BlueprintReadOnly)
	float ResourceRichness = 1.f;

	// NAME_None = unowned.
	UPROPERTY(BlueprintReadOnly)
	FName OwnerCivID;

	// Hazard.* or empty.
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag ActiveHazard;

	UPROPERTY(BlueprintReadOnly)
	int32 HazardTicksRemaining = 0;

	// DT_Megaprojects row name. NAME_None = none.
	UPROPERTY(BlueprintReadOnly)
	FName ActiveMegaproject;

	UPROPERTY(BlueprintReadOnly)
	float MegaprojectProgress = 0.f;

	// 0 = not completed. Condition MegaprojectComplete is true only when this is > 0 and >= Calling.BornTick (section 4.4).
	// BornTick is 0 in the first life, so the > 0 test is not redundant.
	UPROPERTY(BlueprintReadOnly)
	int64 MegaprojectCompletedTick = 0;

	// Phase 6 (append): FName EpithetRow - the second name the chronicle gives a system (section 5.11).
};

/**
 * Notable person record (section 1.3, 7.2). Five fields, fixed: FName, integers and one tag only.
 * A record, not an entity: no stats, no AI, no relations, and no condition or effect that takes a person's ID.
 * NameNotable (adds one record to a civ) and CivHasNotable (reads whether a role is open) are all there is (section 12.2).
 * Created only by rows carrying EEffectType::NameNotable and by three code hooks; closed only by code hooks or civ
 * deletion (section 7.3, 7.4, 7.5). The player vessel is never a notable (FLifeRecord.NameIndex instead).
 * A sixth field has to pass the three questions of section 12.3.
 */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FNotableRecord
{
	GENERATED_BODY()

	// Notable.Ruler / Regent / Rebel / Loyalist / Hearer / Ascendant. A function word, never a title.
	UPROPERTY()
	FGameplayTag Role;

	// Index into the owning civ archetype's PersonNameBank. Rendered as Bank[NameIndex % Bank.Num()]. Never store the FText.
	UPROPERTY()
	int32 NameIndex = 0;

	// Tick the record appeared = tick of the event that made it.
	UPROPERTY()
	int64 FirstTick = 0;

	// Tick the record left. 0 = open. Entry and exit, not birth and death: the game never tells a person's fate (section 8.3).
	UPROPERTY()
	int64 LastTick = 0;

	// Row that made the record: an event / hazard / encounter row, the DT_Feats row (Ascendant hook) or the DT_LogTemplates
	// row log_throne_vacated (abdication Regent). Basis of test Sim.NotablesFromEventsOnly (section 7.5, 9.3).
	UPROPERTY()
	FName SourceRow;
};

/** One civilization (section 7.2). */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FCivilizationState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName CivID;

	// Capital. "The planet" of a Tier 1 civ is this one system. Target system of Hazard.Rebellion (section 5.10).
	UPROPERTY(BlueprintReadOnly)
	FName HomeSystemID;

	// DT_Civs row name (section 5.11). Name, adjective, person-name and dedication banks, revelation mode and the petition
	// speaker branch read it. NAME_None falls back to the arch_default banks (section 9.1).
	UPROPERTY(BlueprintReadOnly)
	FName ArchetypeRow;

	// Display only, not hashed.
	UPROPERTY(BlueprintReadOnly)
	FText CivName;

	UPROPERTY(BlueprintReadOnly)
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(BlueprintReadOnly)
	EAscensionTier Tier = EAscensionTier::Tier1_Planetary;

	UPROPERTY(BlueprintReadOnly)
	FEthicsAxes Ethics;

	// Trait.* (section 5.3).
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer Traits;

	UPROPERTY(BlueprintReadOnly)
	FResourcePool Stockpile;

	// Cache, recomputed every tick.
	UPROPERTY(BlueprintReadOnly)
	FResourcePool IncomePerTick;

	// Abstract ratio, 1.0 = typical for the civ's Tier. Per civilization, never per system (section 12.2). Not a head count and
	// not souls (section 1.3, 8.3). Shown only as a band (GetPopulationBand); the number never reaches UI, log or chronicle.
	UPROPERTY(BlueprintReadOnly)
	float Population = 1.f;

	// Key = research field (Physics / Society / Engineering / Transcendence, section 5.4), value = progress 0..1.
	UPROPERTY(BlueprintReadOnly)
	TMap<FName, float> Research;

	// Same keys, values sum to 1.
	UPROPERTY(BlueprintReadOnly)
	TMap<FName, float> ResearchFocus;

	UPROPERTY(BlueprintReadOnly)
	float FleetPower = 0.f;

	// 0..1. The ruler's single meter. Discontent is the Hazard.Rebellion countdown, not a second meter (section 5.10).
	UPROPERTY(BlueprintReadOnly)
	float Stability = 0.75f;

	// Consecutive ticks with Stability < 0.3. Updated at the end of TickEconomy, read by CivStabilityBelowFor (section 4.1).
	UPROPERTY(BlueprintReadOnly)
	int32 LowStabilityTicks = 0;

	UPROPERTY(BlueprintReadOnly)
	TArray<FName> OwnedSystems;

	// Key = CivID or TranscendentID, value = -1..1 (section 4.2). There is no separate spiritual favor meter.
	UPROPERTY(BlueprintReadOnly)
	TMap<FName, float> Relations;

	// DT_Edicts row names.
	UPROPERTY(BlueprintReadOnly)
	TArray<FName> ActiveEdicts;

	// AI.Profile.* - also the regent profile while the player is away.
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag AIProfile;

	// 0..1. Wanderer -> ruler transition condition (section 5.9). Reverence is respect, not worship.
	UPROPERTY(BlueprintReadOnly)
	float ReverenceToPlayer = 0.f;

	UPROPERTY(BlueprintReadOnly)
	bool bIsPlayerRuled = false;

	// Set to Tick + DT_Rules.ThroneLockTicks on deposition. Enthronement and return require Tick >= this (section 5.10).
	UPROPERTY(BlueprintReadOnly)
	int64 ThroneLockedUntilTick = 0;

	// The few who left a trace (section 1.3). Cap DT_Rules.MaxNotablesPerCiv, at most one open record (LastTick == 0) per role.
	// Never seeded by NewGalaxy: a civ that lived through no notable-making event has none. The player vessel is never in here.
	UPROPERTY(BlueprintReadOnly)
	TArray<FNotableRecord> Notables;
};

/**
 * The vessel (section 4.4 three-way table, column "vessel"). Fully reset on succession.
 * PersonalTier, Powers and CompletedFeats belong to the calling (FCallingState), not to the vessel.
 * The only individual with stats in the whole game (section 1.3).
 */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FPlayerEntityState
{
	GENERATED_BODY()

	// First life is Ruler (section 5.10). Succession.Called always installs a Wanderer; Succession.Returned keeps the role at death.
	UPROPERTY(BlueprintReadOnly)
	EPlayerRole Role = EPlayerRole::Ruler;

	UPROPERTY(BlueprintReadOnly)
	FName CurrentSystemID;

	// NAME_None = not traveling.
	UPROPERTY(BlueprintReadOnly)
	FName TravelTargetSystemID;

	UPROPERTY(BlueprintReadOnly)
	int32 TravelTicksRemaining = 0;

	// Ruler only.
	UPROPERTY(BlueprintReadOnly)
	FName RuledCivID;

	// The civ the player left: recorded by BecomeWanderer on abdication, deposition and death alike. Read by ReturnToFormerCiv
	// and by the Former selector of CivHasNotable / NameNotable (section 5.10, 7.3).
	UPROPERTY(BlueprintReadOnly)
	FName FormerCivID;

	// Personal stats, 0..10. Body / Mind / Causality start from the birth civ's DT_Traits modifiers (section 5.9).
	UPROPERTY(BlueprintReadOnly)
	int32 Body = 1;

	UPROPERTY(BlueprintReadOnly)
	int32 Mind = 1;

	UPROPERTY(BlueprintReadOnly)
	int32 Causality = 0;

	// 0..10. Never trained: accepting worship +1..2, refusing -1, return lure accepted +2 / refused -1, waiting at the
	// Tier 5 threshold -1 (section 5.9, 4.4). Read by the awe/fear branch, the humbling event and the received-way gate.
	UPROPERTY(BlueprintReadOnly)
	int32 Exaltation = 0;

	UPROPERTY(BlueprintReadOnly)
	FResourcePool PersonalPool;

	// DT_Relics row names, at most 3.
	UPROPERTY(BlueprintReadOnly)
	TArray<FName> Relics;

	// Status.FallenMonarch / Status.Abdicated / Status.Returned. Condition PlayerHasStatus (section 5.10).
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer StatusTags;
};

/**
 * One active Transcendent (section 4.2). TranscendentID == DT_Transcendents row name, fixed meaning.
 * Allegiance (Realm.*) is definition data (FTranscendentDef.Allegiance): this struct has no Realm field, on purpose.
 * Realm.Celestial rows never get a state instance.
 */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FTranscendentState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName TranscendentID;

	// Doctrine.Silence / Assimilation / Zeal / Recurrence / Stillness / Release. Saved, so the names are final (section 9.1).
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Doctrine;

	// The sphere. An expired Hazard.Claim adds the system here (section 4.2). Cap FTranscendentDef.MaxInfluencedSystems.
	UPROPERTY(BlueprintReadOnly)
	TArray<FName> InfluencedSystems;

	// Survives succession (section 4.4).
	UPROPERTY(BlueprintReadOnly)
	bool bAwareOfPlayer = false;

	UPROPERTY(BlueprintReadOnly)
	bool bSealed = false;

	// Tick of sealing. Condition TranscendentSealed is true only when bSealed and this is >= Calling.BornTick (section 4.4).
	UPROPERTY(BlueprintReadOnly)
	int64 SealedTick = 0;
};

/** Whether the calling currently rests on a vessel (section 4.4). */
UENUM(BlueprintType)
enum class ELifeState : uint8
{
	Embodied,
	Interregnum
};

/** One finished or current life (section 4.4). FName and numbers only: sentences are rendered from templates at display time. */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FLifeRecord
{
	GENERATED_BODY()

	UPROPERTY()
	int32 LifeIndex = 0;

	UPROPERTY()
	int64 BornTick = 0;

	// TicksLived = DiedTick - BornTick.
	UPROPERTY()
	int64 DiedTick = 0;

	UPROPERTY()
	FName BirthSystemID;

	UPROPERTY()
	FName BirthCivID;

	// DT_Civs row of the birth civ, copied from its ArchetypeRow by Install(). Keeps the vessel's name bank resolvable after
	// Hazard.Extinction deletes the birth civ (section 8.2). Succession.Returned keeps the previous life's value.
	UPROPERTY()
	FName BirthArchetypeRow;

	// The vessel's name: index into the PersonNameBank of BirthArchetypeRow, drawn by Install() with GetRng() (section 4.4, 7.5).
	// Succession.Called draws a new index, Succession.Returned repeats the previous life's index.
	// A nameless birth civ (empty PersonNameBank) uses arch_default's bank instead.
	UPROPERTY()
	int32 NameIndex = 0;

	// The three reign arrays (RuledCivIDs, ReignEndReasons, ReignTicks) always have the same length. Taking a throne
	// (Install as Ruler, BecomeRuler, ReturnToFormerCiv) adds one entry to all three.
	UPROPERTY()
	TArray<FName> RuledCivIDs;

	// Same order as RuledCivIDs. Reign.Abdicated / Reign.Deposed / Reign.Died (section 5.10). Empty tag = the open reign.
	UPROPERTY()
	TArray<FGameplayTag> ReignEndReasons;

	UPROPERTY()
	EAscensionTier PeakTier = EAscensionTier::Tier1_Planetary;

	// Death.Hazard / Death.Encounter / Death.Age / Death.Transcendent.
	UPROPERTY()
	FGameplayTag CauseOfDeath;

	// Hazard / encounter / event row behind the death.
	UPROPERTY()
	FName CauseSourceRow;

	UPROPERTY()
	TArray<FName> Feats;

	UPROPERTY()
	TArray<FName> TranscendentsMet;

	// Last three encounter / petition rows (epitaph).
	UPROPERTY()
	TArray<FName> LastChoiceRows;

	// Era naming (section 5.10).
	UPROPERTY()
	FName EpithetRow;

	UPROPERTY()
	TArray<FName> MegaprojectsCompleted;

	// Same order as RuledCivIDs. Running count: +1 per tick while ruling, at the end of TickPlayerTravel (section 7.4).
	// Era naming (600 ticks) and Trait.Legacy.Enduring (1,800 ticks) read it (section 5.10).
	UPROPERTY()
	TArray<int64> ReignTicks;

	// How this life began: Succession.Installed (first life) / Succession.Called (succession draw) / Succession.Returned (return lure accepted).
	UPROPERTY()
	FGameplayTag SuccessionKind;

	// Whether the return lure was offered at the end of this life. Gate: once per life (section 4.4).
	UPROPERTY()
	bool bReturnOffered = false;
};

/**
 * The calling: the player's identity across vessels (section 4.4 three-way table, column "calling").
 * On succession PersonalTier drops to Tier 1 when it is Tier 2 or lower and is kept from Tier 3 up; Powers are re-granted
 * from the Tier; CompletedFeats stay as a record.
 */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FCallingState
{
	GENERATED_BODY()

	// "The Nth".
	UPROPERTY()
	int32 LifeIndex = 1;

	// Birth tick of the current vessel. Base of the birth protection and of life-relative feat conditions.
	UPROPERTY()
	int64 BornTick = 0;

	UPROPERTY()
	ELifeState LifeState = ELifeState::Embodied;

	// Remaining leap, so that a save taken on entering the interregnum resumes (section 9.1).
	UPROPERTY()
	int64 PendingLeapTicks = 0;

	// Run total. Next leap = clamp(Rng.RandRange(LeapTicksMin, LeapTicksMax), 0, LeapTotalCap - LeapTicksTotal). 0 = succeed at once.
	UPROPERTY()
	int64 LeapTicksTotal = 0;

	UPROPERTY()
	TArray<FLifeRecord> Lives;

	UPROPERTY(BlueprintReadOnly)
	EAscensionTier PersonalTier = EAscensionTier::Tier1_Planetary;

	// Power.* and Way.*.
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer Powers;

	// DT_Feats row names.
	UPROPERTY(BlueprintReadOnly)
	TArray<FName> CompletedFeats;

	// PendingRoleEffect (section 7.4, last tick stage): hazard expiry, event effects and encounter resolution only record
	// here. At most one per tick, first recorded wins. Empty PendingEffect = nothing pending.

	// KillPlayer or DeposeRuler.
	UPROPERTY()
	FGameplayTag PendingEffect;

	// Death.* or Reign.*.
	UPROPERTY()
	FGameplayTag PendingCause;

	UPROPERTY()
	FName PendingSourceRow;
};

/** The whole saved game (section 7.2, 9.1). One of these per galaxy, one save slot per galaxy. */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FGalaxyState
{
	GENERATED_BODY()

	// Bumped when a field is removed or changes meaning (section 9.1). NewGalaxy and the migration stamp it into SaveVersion.
	static constexpr int32 CurrentSaveVersion = 1;

	// Frozen at 1: the version a save loads as when it carries no SaveVersion tag. Saving is a delta against the class
	// default object, so a value equal to this initializer is never written. Never change this literal and never tie it to
	// CurrentSaveVersion.
	UPROPERTY()
	int32 SaveVersion = 1;

	UPROPERTY()
	int32 Seed = 0;

	UPROPERTY()
	int64 Tick = 0;

	// FRandomStream::GetCurrentSeed() of the single sim stream. Restored with FRandomStream::Initialize(int32).
	UPROPERTY()
	int32 RngState = 0;

	// Key of the galaxy's single save slot (section 9.1). Identity, not sim state: not hashed.
	UPROPERTY()
	FGuid GalaxyID;

	UPROPERTY()
	TArray<FStarSystemState> Systems;

	UPROPERTY()
	TArray<FCivilizationState> Civs;

	UPROPERTY()
	TArray<FTranscendentState> Transcendents;

	// The vessel.
	UPROPERTY()
	FPlayerEntityState Player;

	// The calling.
	UPROPERTY()
	FCallingState Calling;

	// Key = DT_Events row name, or the Log.Claim spam-guard key log_claim_<row> (section 4.2). Keys are saved, so their
	// format is fixed.
	UPROPERTY()
	TMap<FName, int64> EventCooldownUntilTick;

	// Capped (section 8.4).
	UPROPERTY()
	TArray<FCosmicLogEntry> Log;

	// Severity 1 and 2 entries moved out of Log when it overflows, plus every Log.Claim / Log.Record. Input of BuildChronicle.
	UPROPERTY()
	TArray<FCosmicLogEntry> Chronicle;

	// Flag.* set by the SetFlag effect (section 4.5).
	UPROPERTY()
	FGameplayTagContainer Flags;
};

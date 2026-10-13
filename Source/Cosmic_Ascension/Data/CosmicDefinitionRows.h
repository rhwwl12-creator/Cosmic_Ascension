// Cosmic Ascension - Data layer. Design: guideline.md section 7.3.
// Definition rows: read-only DataTable data (FTableRowBase). The row name is the ID; rows are never saved in FGalaxyState.
// Player-facing strings are FText columns (Korean lives here and in String Tables, never in source, section 10.3).
// DT_Megaprojects / DT_Edicts / DT_Relics / DT_Traits / DT_Dedications row structs are added with their rows (Phase 2, 4).

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"
#include "Core/CosmicTypes.h"
#include "Data/CosmicConditionTypes.h"
#include "CosmicDefinitionRows.generated.h"

class UNiagaraSystem;
class USoundBase;
class UTexture2D;

/** How a DT_Events row comes to fire (section 7.5). Append only. */
UENUM(BlueprintType)
enum class ECosmicEventTrigger : uint8
{
	// Takes part in the one draw of a civilization's event slot, with BaseWeight. At most one per civilization and slot.
	Lottery,
	// Fires in the event slot whenever its conditions pass and its cooldown is over: a cadence, not a chance. No draw.
	Standing,
	// Never looked at by a slot. Code fires it by row name (UCosmicEventSubsystem::FireRow).
	Named
};

/** What a DT_Events row is about: the unit a slot evaluates it on and the subject of its line (section 7.5, 8.1). Append only. */
UENUM(BlueprintType)
enum class ECosmicEventSubject : uint8
{
	// Once per civilization. Current system = its capital while that is one of its settled systems, else none. The
	// line is about the people: FocusCiv only, so a met people is heard even when its capital is not a known system.
	Civilization,
	// Once per settled system of the civilization, in OwnedSystems order. The line is about the place: FocusSystem
	// and FocusCiv.
	SettledSystem
};

/** DT_Events row. */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FCosmicEventDef : public FTableRowBase
{
	GENERATED_BODY()

	// Event.Civ.Economy / Society, Event.Crisis.Supernova / Transcendent / Rebellion / Claim, Event.Grace.Rescue / Remnant / Jubilee.
	// The weight family of the row: transcendent, trait and edict weight modifiers are keyed by it. It is not the
	// category of the log line; that is LogCategory.
	UPROPERTY(EditAnywhere)
	FGameplayTag Category;

	// Log.*: the Category of the log entry told for this row. Copied, never derived; the legal pairs with Category are
	// in Tools/data/schema.json (Data.Integrity). Saved and hashed with the entry (section 8.1).
	UPROPERTY(EditAnywhere)
	FGameplayTag LogCategory;

	UPROPERTY(EditAnywhere)
	ECosmicEventTrigger Trigger = ECosmicEventTrigger::Lottery;

	UPROPERTY(EditAnywhere)
	ECosmicEventSubject Subject = ECosmicEventSubject::Civilization;

	// AND.
	UPROPERTY(EditAnywhere)
	TArray<FCosmicCondition> Conditions;

	// Lottery rows only: 1 = fires for an eligible unit with chance DT_Rules.EventChancePerSlot per slot.
	// Standing and Named rows carry 0 (Data.Integrity).
	UPROPERTY(EditAnywhere)
	float BaseWeight = 1.f;

	// Per civilization: key "<row>@<CivID>" in FGalaxyState.EventCooldownUntilTick. 0 = no cooldown. Named rows: 0.
	UPROPERTY(EditAnywhere)
	int32 CooldownTicks = 200;

	UPROPERTY(EditAnywhere)
	TArray<FCosmicEffect> Effects;

	// Three or more for Lottery and Standing rows, one or more for Named rows (Data.Integrity). Slots: {Civ}, {System}
	// and {HomeSystem}, nothing else of section 8.2 ({Hazard} joins with the crisis handling work). Empty = the row tells
	// nothing, not its SignRow either.
	UPROPERTY(EditAnywhere)
	TArray<FText> LogTemplates;

	// 0 normal, 1 notable, 2 mythic.
	UPROPERTY(EditAnywhere)
	uint8 Severity = 0;

	// Lowest viewer tier that is told LogTemplates. Tier number 1..5 (3 = Tier 3), not the enum value. 0 = everyone.
	UPROPERTY(EditAnywhere)
	uint8 MinTierToReveal = 0;

	// DT_LogTemplates row of category Log.Sign: the lower speaker's sentence, told to a viewer below MinTierToReveal
	// (section 8.2). NAME_None = nothing is told below it. Needs LogTemplates and a MinTierToReveal of 2..5
	// (Data.Integrity): no viewer tier lies below 1, so with 1 or 0 the sign line is not told.
	UPROPERTY(EditAnywhere)
	FName SignRow;
};

/** DT_Hazards row. */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FHazardDef : public FTableRowBase
{
	GENERATED_BODY()

	// Hazard.*
	UPROPERTY(EditAnywhere)
	FGameplayTag HazardTag;

	// Optional Doctrine.*: the fallen doctrine this hazard belongs to (section 4.2 hierarchy). Empty = no doctrine.
	UPROPERTY(EditAnywhere)
	FGameplayTag SourceDoctrine;

	UPROPERTY(EditAnywhere)
	FText DisplayName;

	UPROPERTY(EditAnywhere)
	int32 CountdownTicks = 300;

	UPROPERTY(EditAnywhere)
	float SpawnChancePerTick = 0.0005f;

	UPROPERTY(EditAnywhere)
	TArray<FCosmicCondition> SpawnConditions;

	// KillPlayer applies only when the player is in that system (section 4.4). crisis_rebellion carries DeposeRuler (section 4.1).
	// Exception: when Hazard.Extinction deletes the civ the player rules, the ruler dies wherever the vessel is (section 4.1, 4.4).
	UPROPERTY(EditAnywhere)
	TArray<FCosmicEffect> OnExpireEffects;

	// Applied once when the hazard is resolved. Ruler, wanderer and AI resolutions all go through the same ResolveHazard path.
	// crisis_visitation: grants Trait.Faith.Visited and NameNotable (Notable.Hearer). Empty by default.
	UPROPERTY(EditAnywhere)
	TArray<FCosmicEffect> OnResolveEffects;

	// DT_Megaprojects / DT_Edicts / DT_Encounters (petition) row names. Any one completed resolves the hazard.
	UPROPERTY(EditAnywhere)
	TArray<FName> RulerResolutions;

	// DT_Encounters row names.
	UPROPERTY(EditAnywhere)
	TArray<FName> WandererResolutions;
};

/** DT_Feats row: a Mythic Feat, the only way up a Tier (section 3.2). */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FAscensionFeatDef : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FText Title;

	// Shown during the ascension sequence.
	UPROPERTY(EditAnywhere)
	FText MythicNarrative;

	UPROPERTY(EditAnywhere)
	EAscensionTier RequiredTier = EAscensionTier::Tier1_Planetary;

	UPROPERTY(EditAnywhere)
	EAscensionTier TargetTier = EAscensionTier::Tier2_Stellar;

	UPROPERTY(EditAnywhere)
	EPlayerRole Path = EPlayerRole::Ruler;

	UPROPERTY(EditAnywhere)
	TArray<FCosmicCondition> CompletionConditions;

	// The price of ascension (section 3.2).
	UPROPERTY(EditAnywhere)
	TArray<FCosmicEffect> AscensionCost;

	// Power.* tags. Tier 4 -> 5 rows include Way.Received or Way.Seized. No feat ever removes Way.Seized (Law.Humility).
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer GrantedPowers;

	// Which kind of dedication name the civ gives on completion (section 8.6).
	UPROPERTY(EditAnywhere)
	FGameplayTag DedicationBank;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UNiagaraSystem> FX;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<USoundBase> Sound;
};

/** One choice of an encounter or petition card. Check: stat + Rng 0..5 >= Difficulty (section 5.9). */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FEncounterChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FText Label;

	// Body / Mind / Causality / Exaltation / None. Petitions also allow Stability / Influence (section 5.10).
	UPROPERTY(EditAnywhere)
	FName CheckStat;

	// >= 16 = an impossible choice (pressure gate, section 4.2).
	UPROPERTY(EditAnywhere)
	int32 Difficulty = 0;

	// Paid from PersonalPool (wanderer) or the civ Stockpile (ruler), section 5.10.
	UPROPERTY(EditAnywhere)
	FResourcePool Cost;

	UPROPERTY(EditAnywhere)
	TArray<FCosmicEffect> OnSuccess;

	// One of the data sources of KillPlayer (section 4.4).
	UPROPERTY(EditAnywhere)
	TArray<FCosmicEffect> OnFailure;

	UPROPERTY(EditAnywhere)
	FText SuccessText;

	UPROPERTY(EditAnywhere)
	FText FailureText;

	// For chains. NAME_None = none.
	UPROPERTY(EditAnywhere)
	FName NextEncounter;
};

/** DT_Encounters row. Wanderer encounters and ruler petitions share it (petition = condition PlayerRoleIs Ruler). */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FEncounterDef : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FText Title;

	UPROPERTY(EditAnywhere)
	FText Body;

	UPROPERTY(EditAnywhere)
	TArray<FCosmicCondition> Conditions;

	UPROPERTY(EditAnywhere)
	float BaseWeight = 1.f;

	// Two or three.
	UPROPERTY(EditAnywhere)
	TArray<FEncounterChoice> Choices;

	// Optional.
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UTexture2D> Illustration;
};

/** DT_Civs row: civilization archetype (section 5.11). A DataTable row, not a DataAsset. */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FCivArchetypeDef : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FText DisplayName;

	// Includes the archetype tag (Trait.Arch.*). Filtered with CivHasTrait.
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Traits;

	UPROPERTY(EditAnywhere)
	FEthicsAxes EthicsPreset;

	// AI.Profile.*
	UPROPERTY(EditAnywhere)
	FGameplayTag AIProfile;

	UPROPERTY(EditAnywhere)
	FLinearColor Color = FLinearColor::White;

	// Civ name patterns with the {Star} slot.
	UPROPERTY(EditAnywhere)
	TArray<FText> NameBank;

	// Three adjectives for the {CivAdj} slot.
	UPROPERTY(EditAnywhere)
	TArray<FText> AdjectiveBank;

	// Names for notables (FNotableRecord.NameIndex) and for the vessel (FLifeRecord.NameIndex). Rendered as
	// Bank[NameIndex % Bank.Num()], so shrinking the bank never breaks an old save. Empty = a nameless civ: NameNotable is
	// ignored (arch_swarm, arch_machine; the same holds for any civ with Trait.Mind.NoSelf). The vessel of a nameless civ
	// takes its name from arch_default's bank (section 4.4). Separate from NameBank. Content rules: test Data.Integrity
	// (section 9.3).
	UPROPERTY(EditAnywhere)
	TArray<FText> PersonNameBank;

	// Hazard.* -> spawn multiplier.
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, float> HazardSpawnMultipliers;

	// Other archetype row -> initial relation.
	UPROPERTY(EditAnywhere)
	TMap<FName, float> InitialRelations;

	// NewGalaxy AI draw weight. 0 = never drawn for AI (arch_terran: referenced only by DT_Rules.PlayerArchetypeRow).
	UPROPERTY(EditAnywhere)
	float Weight = 1.f;

	// Revelation.*. Exactly one per row. Changes text only, never numbers.
	UPROPERTY(EditAnywhere)
	FGameplayTag RevelationMode;

	// Exactly 3 entries (Data.Integrity): [0] sign (Log.Sign) / [1] message (enc_messenger_*, indirect speech) / [2] passing
	// and answer (Hazard.Visitation).
	UPROPERTY(EditAnywhere)
	TArray<FText> RevelationTemplates;

	// Home system display names. Empty = the star-name pools (section 5.11).
	UPROPERTY(EditAnywhere)
	TArray<FText> HomeSystemNames;

	// Fixed star class of the home system. Empty = random.
	UPROPERTY(EditAnywhere)
	FGameplayTag HomeStarClass;

	// Faith.Bank.*: which dedication banks this civ names from (section 8.6). Usually one.
	UPROPERTY(EditAnywhere)
	TArray<FGameplayTag> DedicationBanks;
};

/**
 * DT_Transcendents row (section 4.2). Row name == TranscendentID.
 * Allegiance lives here and only here: FTranscendentState has no Realm field.
 */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FTranscendentDef : public FTableRowBase
{
	GENERATED_BODY()

	// Doctrine.*. Unique across rows.
	UPROPERTY(EditAnywhere)
	FGameplayTag Doctrine;

	// Realm.Abyssal / Realm.Celestial / Realm.Independent.
	UPROPERTY(EditAnywhere)
	FGameplayTag Allegiance;

	// Truth.*: the true thing this doctrine imitates (counterfeit principle). Required for Abyssal rows.
	UPROPERTY(EditAnywhere)
	FGameplayTag CounterfeitOf;

	// A title, never a name. Slot {Transcendent}.
	UPROPERTY(EditAnywhere)
	FText Title;

	// What it offers mortals. Text only.
	UPROPERTY(EditAnywhere)
	FText Lure;

	// Tier 3 or 4. An Abyssal row is never Tier 5.
	UPROPERTY(EditAnywhere)
	EAscensionTier Tier = EAscensionTier::Tier4_Transcendent;

	// Depth inside the tier, 0..1 (section 3.2): the tier says where a being stands, the depth how deep it stands there.
	// Definition data: never saved, never hashed, and there is no depth of a civilization or of the vessel anywhere in
	// the state. It scales the transcendent term of spiritual pressure by (1 + Depth) and is read as 0..1 (a value that
	// is not a number reads as 0): the range is a lint (Data.Integrity), the guard is in
	// UGalaxySimulationSubsystem::ComputePressureIn. A Celestial row never gets an instance and so adds no pressure. An
	// Independent row gets none from the roster of NewGalaxy; an independent instance (Phase 6+) is meant to press with
	// the plain weight of its tier (section 4.2), so the first such row comes with a rule for its Depth.
	UPROPERTY(EditAnywhere)
	float Depth = 0.5f;

	UPROPERTY(EditAnywhere)
	int32 InitialInfluencedSystems = 0;

	// No new Hazard.Claim once InfluencedSystems.Num() reaches this. Celestial rows: 0.
	UPROPERTY(EditAnywhere)
	int32 MaxInfluencedSystems = 12;

	// NewGalaxy fallen-roster draw weight. Celestial and Independent rows: 0. A row whose weight is zero, below zero or
	// not finite is no candidate.
	UPROPERTY(EditAnywhere)
	float SpawnWeight = 1.f;

	// Event.* leaf tag -> weight multiplier of Lottery rows of exactly that Category (no parent match), for units whose
	// system is in the sphere or one lane from it. Factor of one transcendent = min(3, value x (1 +
	// DT_Rules.SphereEventWeightStep x InfluencedSystems.Num())), never below 0; several multiply in Transcendents
	// order. A sealed transcendent has no effect. The unit's system is the capital for a Civilization row and the
	// system itself for a SettledSystem row.
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, float> EventWeightModifiers;

	// Applied once per tick, in the economy stage, to every civilization of a lower Tier that has a settled system in
	// the sphere: once per civilization, not once per system (section 4.2, 16 #55). OtherID of the application is this
	// transcendent. A per-tick source: only the seven effects that change an amount are applied
	// (UCosmicEventSubsystem::Apply turns every other away, and Data.Integrity keeps them out of the row). Empty for
	// Celestial rows.
	UPROPERTY(EditAnywhere)
	TArray<FCosmicEffect> PressureEffectsPerTick;

	// First-person templates. Only Realm.Abyssal speaks in first person (section 8.2).
	UPROPERTY(EditAnywhere)
	TArray<FText> VoiceTemplates;

	// First-person Log.Claim templates used when a Hazard.Claim expires.
	UPROPERTY(EditAnywhere)
	TArray<FText> ClaimTemplates;
};

/**
 * DT_LogTemplates row (section 5.11, 8.2): lines that belong to no event row. Two kinds of row, told apart by Context.
 * Named rows (empty Context) are reached by row name through UCosmicLogSubsystem::PushRow. Selector rows (a
 * Revelation.* Context, at most one per Category and Context) are reached through PickTemplateRow (Phase 4), whose
 * fallback is a named row the caller passes.
 */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FLogTemplateDef : public FTableRowBase
{
	GENERATED_BODY()

	// Log.*
	UPROPERTY(EditAnywhere)
	FGameplayTag Category;

	// Empty = a named row. Revelation.* = the selector row of that revelation mode. Changes text only.
	UPROPERTY(EditAnywhere)
	FGameplayTag Context;

	// Lowest viewer tier that is told this row (FCosmicLine.MinViewerTier). Tier number 1..5, 0 = everyone.
	UPROPERTY(EditAnywhere)
	uint8 MinTierToReveal = 0;

	// Three or more; the rows allowed one are listed by name in Tools/data/schema.json (log_first_life,
	// log_first_contact). Two sentences: fact, then implication (section 8.3).
	UPROPERTY(EditAnywhere)
	TArray<FText> Templates;
};

/** DT_Rules: a single row of tuning knobs and the laws that cannot be rewritten (section 10.1). */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FCosmicRulesDef : public FTableRowBase
{
	GENERATED_BODY()

	// ---- Calling and succession (section 4.4) ----

	UPROPERTY(EditAnywhere)
	int32 LeapTicksMin = 800;

	UPROPERTY(EditAnywhere)
	int32 LeapTicksMax = 1600;

	// Run total cap for interregnum leaps.
	UPROPERTY(EditAnywhere)
	int32 LeapTotalCap = 4500;

	// Lethal effects are ignored for this many ticks after birth.
	UPROPERTY(EditAnywhere)
	int32 BirthProtectionTicks = 300;

	// 0 = aging OFF. Added to DT_Traits.LifespanTicksMod.
	UPROPERTY(EditAnywhere)
	int32 BaseLifespanTicks = 0;

	// Succession draw: per active hazard within two lanes.
	UPROPERTY(EditAnywhere)
	float SuccessionHazardWeight = 2.f;

	// Succession draw: owner civ Tier >= 2.
	UPROPERTY(EditAnywhere)
	float SuccessionCivTierWeight = 1.f;

	// Succession draw: owner civ Reverence above SuccessionReverenceThreshold.
	UPROPERTY(EditAnywhere)
	float SuccessionReverenceWeight = 2.f;

	UPROPERTY(EditAnywhere)
	float SuccessionReverenceThreshold = 0.3f;

	// Succession draw: penalty for the previous life's civ.
	UPROPERTY(EditAnywhere)
	float SuccessionSameCivPenalty = 1.f;

	// Succession draw: systems whose hazard countdown is below this are excluded.
	UPROPERTY(EditAnywhere)
	int32 SuccessionMinHazardTicks = 300;

	// Reverence multiplier on death and deposition.
	UPROPERTY(EditAnywhere)
	float ReverenceDecay = 0.5f;

	// ---- Rivals and the throne (section 4.3, 5.10) ----

	UPROPERTY(EditAnywhere)
	float AIAscensionSpeedMultiplier = 1.f;

	// Stability cap right after deposition.
	UPROPERTY(EditAnywhere)
	float PostRebellionStability = 0.15f;

	UPROPERTY(EditAnywhere)
	int32 ThroneLockTicks = 600;

	// ---- Spiritual pressure and fleets (section 4.2, 5.6) ----

	// Index = static_cast<int32>(EAscensionTier) of the source, i.e. tier number - 1. Exactly 5 entries (Data.Integrity).
	UPROPERTY(EditAnywhere)
	TArray<float> PressureBySourceTier = { 0.f, 0.f, 0.25f, 1.f, 2.f };

	// One lane away.
	UPROPERTY(EditAnywhere)
	float PressureAdjacencyFalloff = 0.5f;

	// Per tick x P.
	UPROPERTY(EditAnywhere)
	float SpiritualPressureStabilityDrain = 0.002f;

	// Per tick x P.
	UPROPERTY(EditAnywhere)
	float PressureFleetAttrition = 0.01f;

	// Index = Tier gap (0, 1, >= 2). Exactly 3 entries (Data.Integrity).
	UPROPERTY(EditAnywhere)
	TArray<float> FleetEffectivenessByTierGap = { 1.f, 0.5f, 0.f };

	// Hazard countdown ticks bought per 1.0 fleet power committed.
	UPROPERTY(EditAnywhere)
	float FleetDelayPerPower = 50.f;

	UPROPERTY(EditAnywhere)
	float FleetDelayCap = 100.f;

	// ---- Laws that cannot be rewritten (section 4.5) ----

	// Law.Judgment, Law.Sabbath, Law.Humility, Law.Remnant.
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer ImmutableLaws;

	// ---- Fallen roster, Earth, claims, return lure (section 4.2, 4.4, 5.11) ----

	// Active (unsealed) fallen Transcendents per run. Silence is fixed; the rest are drawn by SpawnWeight. Read as 0..3
	// (section 4.2: at most three active fallen per run): NewGalaxy reports a value outside with one error line.
	UPROPERTY(EditAnywhere)
	int32 FallenTranscendentCount = 3;

	// DT_Civs row of the player's first-life civ.
	UPROPERTY(EditAnywhere)
	FName PlayerArchetypeRow = FName(TEXT("arch_terran"));

	// Hazard.Claim countdown.
	UPROPERTY(EditAnywhere)
	int32 ClaimCountdown = 300;

	// Claim spawn condition: within this many lanes of a fallen sphere.
	UPROPERTY(EditAnywhere)
	int32 ClaimAdjacencyLanes = 1;

	// Hazard.Claim countdown extension while edict_vigil is active (section 5.8).
	UPROPERTY(EditAnywhere)
	int32 VigilClaimExtension = 100;

	// Doctrine event weight gained per system in the sphere. The multiplier is capped at 3.
	UPROPERTY(EditAnywhere)
	float SphereEventWeightStep = 0.1f;

	// Return lure gate: the death system is within this many lanes of the Recurrence sphere.
	UPROPERTY(EditAnywhere)
	int32 ReturnLureMaxLanes = 2;

	// Succession.Returned lives needed to grant Way.Seized for good.
	UPROPERTY(EditAnywhere)
	int32 ReturnedLivesToSeize = 3;

	// Balance commandlet only: return lure acceptance chance per -PlayerPolicy (section 9.3).
	UPROPERTY(EditAnywhere)
	TMap<FName, float> LureAcceptByPolicy = { { TEXT("Cautious"), 0.10f }, { TEXT("Reckless"), 0.60f }, { TEXT("Faithful"), 0.f }, { TEXT("Tribute"), 0.85f } };

	// ---- Summary unit (section 1.3) ----

	// Notable records per civ. Allowed range 3..5 (Data.Integrity).
	UPROPERTY(EditAnywhere)
	int32 MaxNotablesPerCiv = 4;

	// Population band adjustment, ascending: Population < [1] one band down, < [0] Few, > [2] one band up (cap Countless).
	UPROPERTY(EditAnywhere)
	TArray<float> PopulationBandThresholds = { 0.15f, 0.5f, 2.0f };

	// ---- Event slots (section 7.4, 7.5) ----

	// Every civilization that owns a settled system gets one event slot every this many ticks. Allowed range 1..20
	// (Data.Integrity): above 20 the two murmurs before a rebellion are no longer certain (section 4.1).
	UPROPERTY(EditAnywhere)
	int32 EventSlotTicks = 10;

	// Chance per slot that a Lottery row of BaseWeight 1 fires for a unit it is eligible for. A provisional start value:
	// what happens to one people grows with the number of eligible rows, so it is retuned with the content (section 16 #54).
	UPROPERTY(EditAnywhere)
	float EventChancePerSlot = 0.02f;
};

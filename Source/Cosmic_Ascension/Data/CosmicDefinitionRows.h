// Cosmic Ascension - Data layer. Design: guideline.md section 7.3.
// Definition rows: read-only DataTable data (FTableRowBase). The row name is the ID; rows are never saved in FGalaxyState.
// Player-facing strings are FText columns (Korean lives here and in String Tables, never in source, section 10.3).
// DT_Edicts / DT_Relics / DT_Traits / DT_Dedications row structs are added with their rows (Phase 4).

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
	// line is about the people: FocusCiv only, so a met people is heard even when its capital is not a known system -
	// and so its templates name no system (Data.Integrity: the subject system of a line is the system it names).
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

	// Three or more for Lottery and Standing rows, one or more for Named rows (Data.Integrity). Slots: {Civ}, {System} and
	// {Hazard} (the DisplayName of the hazard the subject system held when the row began to fire), nothing else of section
	// 8.2; a Civilization row calls {Civ} and {Hazard} only (Data.Integrity: the subject system of a line is the system it
	// names, so no capital beside it). Empty = the row tells nothing, not its SignRow either.
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

/** How a DT_Hazards row comes onto a system (section 4.1). Append only. */
UENUM(BlueprintType)
enum class ECosmicHazardSpawn : uint8
{
	// The spawn draw of the hazard stage: one number per settled system and tick, laid over the SpawnChancePerTick of
	// every Draw row whose SpawnConditions pass on that system, in row-name order.
	Draw,
	// No draw: placed on the capital of a people when the capital is a settled system with a free slot and the
	// SpawnConditions pass on it (Hazard.Rebellion, section 4.1).
	Capital,
	// Placed by the claim rule alone, after an expiry (Hazard.Claim, section 4.2). SpawnHazard never places it: the
	// claim rule holds Law.Remnant.
	ClaimRule
};

/**
 * DT_Hazards row (section 4.1). HazardTag is unique across rows (Data.Integrity): a system holds the tag and the code finds
 * the row by it (FCosmicDataTables::FindHazardByTag). What an expiry does that no effect may say is code keyed by the tag
 * (section 16 #29 (5)): Hazard.PreSupernova destroys the system, Hazard.Rebellion changes the throne, Hazard.Claim joins
 * the sphere of its claimant.
 */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FHazardDef : public FTableRowBase
{
	GENERATED_BODY()

	// Hazard.*. Unique across rows.
	UPROPERTY(EditAnywhere)
	FGameplayTag HazardTag;

	// Optional Doctrine.*: the fallen doctrine this hazard belongs to (section 4.2 hierarchy). Empty = no doctrine.
	UPROPERTY(EditAnywhere)
	FGameplayTag SourceDoctrine;

	// The name of the hazard: the UI and the {Hazard} slot.
	UPROPERTY(EditAnywhere)
	FText DisplayName;

	// Ticks from placement to expiry, read as at least 1. Hazard.Claim counts down from its row like every hazard.
	UPROPERTY(EditAnywhere)
	int32 CountdownTicks = 300;

	UPROPERTY(EditAnywhere)
	ECosmicHazardSpawn SpawnRule = ECosmicHazardSpawn::Draw;

	// Draw rows: the share of one spawn draw, times the owner archetype's HazardSpawnMultipliers[HazardTag] (missing = 1).
	// Capital and ClaimRule rows carry 0 (Data.Integrity). Below 0, or not a number, reads as 0.
	UPROPERTY(EditAnywhere)
	float SpawnChancePerTick = 0.0005f;

	// AND. Subject = the system and its owner.
	UPROPERTY(EditAnywhere)
	TArray<FCosmicCondition> SpawnConditions;

	// At expiry: subject = the system and its owner, SourceRow = this row. KillPlayer reaches only a wanderer standing in
	// the system (section 4.4); crisis_rebellion carries DeposeRuler (section 4.1). No row carries KillPlayer before the
	// death pipeline of Phase 4b.
	UPROPERTY(EditAnywhere)
	TArray<FCosmicEffect> OnExpireEffects;

	// The branch of an expiry (the 50/50 of Hazard.AIAwakening, section 4.1): when the expiry's outcome number is below
	// BranchChance, OnExpireBranchEffects are applied instead of OnExpireEffects. 0 = no branch.
	UPROPERTY(EditAnywhere)
	float BranchChance = 0.f;

	UPROPERTY(EditAnywhere)
	TArray<FCosmicEffect> OnExpireBranchEffects;

	// At expiry, after the effects above and before the code of the tag: once for every lane neighbour of the system that
	// is not a dead node, in the order of its Lanes, subject = that neighbour and its owner, SourceRow = this row (the
	// damage of a supernova to its neighbours). The spread of a plague waits for its cure, the quarantine of Phase 4.
	UPROPERTY(EditAnywhere)
	TArray<FCosmicEffect> OnExpireNeighbourEffects;

	// Applied once when the hazard is resolved, after its slot was emptied: subject = the system and its owner, SourceRow
	// = this row. Every resolution goes through CosmicHazards::Resolve. crisis_visitation (Phase 4): Trait.Faith.Visited
	// and NameNotable (Notable.Hearer).
	UPROPERTY(EditAnywhere)
	TArray<FCosmicEffect> OnResolveEffects;

	// The answers of this hazard: DT_Megaprojects / DT_Edicts / DT_Encounters (petition) row names. Any one completed
	// resolves the hazard. A DT_Megaprojects row listed by any hazard row is an answer: it runs in the response slot of
	// the system (FCosmicDataTables::IsHazardAnswer, section 5.7) and ends with the hazard. DT_Edicts and DT_Encounters
	// names are read from Phase 4.
	UPROPERTY(EditAnywhere)
	TArray<FName> RulerResolutions;

	// DT_Encounters row names (Phase 4).
	UPROPERTY(EditAnywhere)
	TArray<FName> WandererResolutions;

	// Log.* and Severity of the three lines below.
	UPROPERTY(EditAnywhere)
	FGameplayTag LogCategory;

	UPROPERTY(EditAnywhere)
	uint8 Severity = 1;

	// The hazard's own lines (UCosmicEventSubsystem::TellHazardLine), about the system and its owner, SourceRow = this
	// row. Slots {Civ} {System} {Hazard} (no capital beside the system: Data.Integrity). Empty = that moment tells nothing
	// of its own.
	UPROPERTY(EditAnywhere)
	TArray<FText> OutbreakTemplates;

	UPROPERTY(EditAnywhere)
	TArray<FText> ExpireTemplates;

	// Also told when a claim ends without joining a sphere.
	UPROPERTY(EditAnywhere)
	TArray<FText> ResolveTemplates;

	// DT_Events Named row fired right after the outbreak line, subject = the system and its owner (crisis_rebellion: the
	// second precursor, which names the rebel). A SettledSystem row (Data.Integrity), so that its line has the hazard's
	// system as its subject, as the hazard's own lines do: the start and the end are told or hidden together. NAME_None =
	// none.
	UPROPERTY(EditAnywhere)
	FName OutbreakRow;
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

	// The ascension line (Log.Ascension, Severity 2, an AI people's too), about the people that ascended and its capital,
	// SourceRow = this row. Slots {Civ} {HomeSystem}.
	UPROPERTY(EditAnywhere)
	TArray<FText> LogTemplates;

	// Whether the judgment looks at this row at all: one rung up (TargetTier = RequiredTier + 1) with a condition (section
	// 3.2). The judgment refuses any other row with an error line, and the AI and the project key build for none.
	bool IsOneRungWithDeed() const
	{
		return static_cast<int32>(TargetTier) == static_cast<int32>(RequiredTier) + 1 && !CompletionConditions.IsEmpty();
	}

	// Whether the feat judgment looks at this row for a people on Tier: a Ruler row of that RequiredTier that
	// IsOneRungWithDeed. The one filter of the judgment (UAscensionSubsystem, which reports a Ruler row of the rung that is
	// not one rung up), the AI's choices (CosmicMegaprojects::Choices) and the top bar's next rung (CosmicNextRung::Of).
	bool IsJudgedRulerRowAt(EAscensionTier Tier) const
	{
		return Path == EPlayerRole::Ruler && RequiredTier == Tier && IsOneRungWithDeed();
	}
};

/**
 * DT_Megaprojects row (section 5.7). A project runs on one system of its owner and is paid on every tick it progresses.
 * Where it runs follows from DT_Hazards: a row that some hazard row lists in RulerResolutions answers that hazard - it
 * starts only while such a hazard stands in the system, runs in the response slot and ends with the hazard. Any other
 * row runs in the construction slot. A completion is recorded on the system and resolves the hazard of the system when
 * that hazard's row lists the project.
 */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FMegaprojectDef : public FTableRowBase
{
	GENERATED_BODY()

	// The name of the project: the UI and the {Project} slot.
	UPROPERTY(EditAnywhere)
	FText DisplayName;

	// The owner's Tier from which the project can be started (section 3.2 table).
	UPROPERTY(EditAnywhere)
	EAscensionTier RequiredTier = EAscensionTier::Tier1_Planetary;

	// Paid in full from the owner's stockpile on every tick the project progresses. A tick the stockpile cannot pay in
	// full costs nothing and progresses nothing; there is no partial pay and no abandonment. A member below 0, or not a
	// number, reads as 0.
	UPROPERTY(EditAnywhere)
	FResourcePool CostPerTick;

	// Progress to completion: 1 per paid tick, DT_Rules.AIAscensionSpeedMultiplier per paid tick for a people the player
	// does not rule. Read as at least 1.
	UPROPERTY(EditAnywhere)
	float RequiredProgress = 100.f;

	// The completion line (Log.Civ; Severity 1 for the people the player rules, 0 for the others), about the system and its
	// owner, SourceRow = this row. Slots {Civ} {System} {Project}. {Dedication} comes with DT_Dedications (Phase 4).
	UPROPERTY(EditAnywhere)
	TArray<FText> CompleteTemplates;
};

/**
 * What a choice of a card is checked on (section 5.9, 5.10). A definition enum: data rows hold it, no save does. Append
 * only. The vessel's stats are 0..10; Stability reads the ruled people's Stability (0..1) as (int32)(Stability * 10), so
 * it takes part in the same check (section 16 #30 (1)). Influence is never a check: a petition spends it as a cost.
 */
UENUM(BlueprintType)
enum class ECosmicCheckStat : uint8
{
	// No check: the choice always succeeds. (Not "None": the data spells an empty name that way.)
	NoCheck,
	Body,
	Mind,
	Causality,
	Exaltation,
	Stability
};

/** One choice of an encounter or petition card. Check: stat + Rng 0..5 >= Difficulty (section 5.9). */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FEncounterChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FText Label;

	UPROPERTY(EditAnywhere)
	ECosmicCheckStat CheckStat = ECosmicCheckStat::NoCheck;

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

/**
 * DT_Encounters row. Wanderer encounters and ruler petitions share it (petition = condition PlayerRoleIs Ruler). Texts:
 * slots {Civ} {System} {Hazard} {Rebel}; a Civilization row names no system (schema).
 */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FEncounterDef : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FText Title;

	// The card's text when it has no voices (VoiceBodies empty); empty when it has them (schema).
	UPROPERTY(EditAnywhere)
	FText Body;

	UPROPERTY(EditAnywhere)
	TArray<FCosmicCondition> Conditions;

	// The pick among the rows of one moment (Lottery, Standing); 0 for a Named row (schema).
	UPROPERTY(EditAnywhere)
	float BaseWeight = 1.f;

	// Two or three. Localization keys are positions: a choice is only ever appended (section 8.5).
	UPROPERTY(EditAnywhere)
	TArray<FEncounterChoice> Choices;

	// Optional.
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UTexture2D> Illustration;

	// When the knowing stage puts the card before a ruler (section 5.10): Standing = in the tick its conditions come to
	// hold (they did not at the head of the tick); Lottery = in the periodic slot (DT_Rules.PetitionPeriodTicks), picked
	// by BaseWeight among the rows whose conditions hold and that are not among the last choices of the life; Named =
	// never by the stage, only by name (the next card of a chain, NextEncounter).
	UPROPERTY(EditAnywhere)
	ECosmicEventTrigger Trigger = ECosmicEventTrigger::Named;

	// What the card is about: the ruled people (Civilization, current system = its capital) or one of its settled
	// systems (SettledSystem, looked at in OwnedSystems order).
	UPROPERTY(EditAnywhere)
	ECosmicEventSubject Subject = ECosmicEventSubject::Civilization;

	// Empty, or the card's text in each of the two voices of a divided people (Trait.Society.Divided): [0] the council,
	// [1] the elders of the congregation. The presentation picks the voice by the ruled people's Spiritualism (above 0 =
	// the elders) when the card arrives; it is not state (section 5.10).
	UPROPERTY(EditAnywhere)
	TArray<FText> VoiceBodies;
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

	// Name patterns of a people. {Star} is the display name of its home system; a row AI peoples are drawn from has
	// it in every pattern, so two peoples of one archetype never share a name (section 5.11). NewGalaxy picks one with
	// one draw per people, made whether or not the bank holds anything (section 9.2).
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

	// NewGalaxy AI draw weight. 0 = never drawn for AI (arch_seeker, the first life's row: referenced only by
	// DT_Rules.PlayerArchetypeRow).
	UPROPERTY(EditAnywhere)
	float Weight = 1.f;

	// Revelation.*. Exactly one per row. Changes text only, never numbers.
	UPROPERTY(EditAnywhere)
	FGameplayTag RevelationMode;

	// Exactly 3 entries (Data.Integrity): [0] sign (Log.Sign) / [1] message (enc_messenger_*, indirect speech) / [2] passing
	// and answer (Hazard.Visitation).
	UPROPERTY(EditAnywhere)
	TArray<FText> RevelationTemplates;

	// Display names for the home system (arch_seeker: one, the name its people call its sun by). Empty = the home takes
	// the next proper name of FGalaxyDef.StarNameBank, without replacement. Either way it is the one home-name draw of the
	// people. A row AI peoples are drawn from leaves this empty (Data.Integrity): a bank read with replacement could name
	// two homes alike.
	UPROPERTY(EditAnywhere)
	TArray<FText> HomeSystemNames;

	// Fixed star class of the home system (arch_seeker: Star.Class.G). Empty = the class the system drew from
	// DT_Rules.LivingWorldStarShares like every other system (section 7.4).
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

	// The starting core of the sphere (section 5.7): 0 = none, 1 = one system, which NewGalaxy places. Allowed range
	// 0..1 (Data.Integrity); the code reads any value above 0 as one system.
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

/**
 * DT_Galaxies row (section 5.11): one galaxy that has a name. The row name is the identity of the galaxy and lands in
 * saves (GalaxyRow, OriginGalaxyRow, FTransitRecord), so it never changes once shipped.
 * One row is the home galaxy (DT_Rules.HomeGalaxyRow), the only galaxy simulated in detail. Of that row the simulation
 * reads LivingWorlds, Peoples, HomeRadiusShare and the three name pools at generation, and AIFourthRungCap from Phase 6a.
 * Of the other rows it reads nothing until the crossings of Phase 6a (BridgeTicks, PeopleStems). Every other column
 * is display only: editing it never moves a hash.
 * The member initializers are what a run falls back on when the table or the home row is missing: the numbers of the
 * vertical slice. A row that is not the home row writes 0 into the four simulation columns.
 * A row that is not the home row and has BridgeTicks 0 is display only (section 5.11): drawn on the universe level, its
 * galaxy panel shown; no rule of the simulation reads it.
 */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FGalaxyDef : public FTableRowBase
{
	GENERATED_BODY()

	// The common name of the galaxy. Display.
	UPROPERTY(EditAnywhere)
	FText DisplayName;

	// Galaxy.Kind.Dwarf / Spiral / GrandSpiral / Elliptical. Definition only, never copied into the state. Display (the
	// panel's word, the sprite's aspect on the universe level).
	UPROPERTY(EditAnywhere)
	FGameplayTag Kind;

	// Living systems: the number of systems NewGalaxy makes when it is asked for 0 (section 3.1). Home row only.
	UPROPERTY(EditAnywhere)
	int32 LivingWorlds = 60;

	// Peoples, the player's included: the number of civilizations NewGalaxy makes when it is asked for 0. Home row only.
	UPROPERTY(EditAnywhere)
	int32 Peoples = 12;

	// The brake on the fourth rung (section 5.12, 7.4 crossing stage). 0 = no brake. Read by the simplified AI ascent
	// of Phase 6a and never by the player's feats. Home row only.
	UPROPERTY(EditAnywhere)
	int32 AIFourthRungCap = 2;

	// Where sol lies: this share of the disc radius, at angle 0 (section 7.4). A place, not an advantage. Read as 0..1;
	// a value outside, or one that is not a number, is reported with one error line and read as the C++ default.
	// Home row only.
	UPROPERTY(EditAnywhere)
	float HomeRadiusShare = 0.53f;

	// Radius in light years: the home row's is the universe level's unit (half of 'about 100,000 across'); another row's is
	// 1/2 x D25 x its central distance (Tools/data/sources/galaxies.md). Only the drawing reads it - the sprite's size on the
	// universe level (CosmicUniverseLook::SpriteShareOf, the square root of its share of 43,500); 0 draws the kind's size.
	// Never printed. Display.
	UPROPERTY(EditAnywhere)
	int32 RadiusLy = 0;

	// The bridge (section 5.9): ticks to cross from the home galaxy. 0 = no bridge; the home row is 0. Not a lane and
	// no state. Display until the crossing stage reads it in Phase 6a.
	UPROPERTY(EditAnywhere)
	int32 BridgeTicks = 0;

	// How many stars, in words that carry their own "about" or "estimated" (section 8.3). May be empty: the panel then
	// leaves the line out. Display.
	UPROPERTY(EditAnywhere)
	FText StarCountText;

	// How far, in words that carry their own "about". The home row tells where the sun lies. Display.
	UPROPERTY(EditAnywhere)
	FText DistanceText;

	// A feature worth one line. May be empty. A figure in it carries its estimate like the two texts above. Display.
	UPROPERTY(EditAnywhere)
	FText FeatureText;

	// Place on the universe level (the same map, zoomed out - section 6.4): home-galaxy radii from the home galaxy's centre,
	// in the map's own X and Y. Entered by hand from the row's sources (Tools/data/sources/galaxies.md): distance
	// 1 + ln(d / 50,000 ly), direction the galactic longitude growing clockwise as the camera sees it (X = -r cos l,
	// Y = -r sin l), nudged where two would stand on one place. The home row is (0,0) and stands at the centre whatever it
	// holds; any other row at (0,0), not a number, inside the home galaxy or farther than 20 is not drawn
	// (CosmicUniverseLook::HasPlace). Display, never hashed.
	UPROPERTY(EditAnywhere)
	FVector2D MapPosition = FVector2D::ZeroVector;

	// Proper names for the home systems of peoples, handed out without replacement (section 5.11). Home row only.
	UPROPERTY(EditAnywhere)
	TArray<FText> StarNameBank;

	// Catalogue names for unowned systems: one prefix (a Greek letter) and one stem (a constellation), joined with a
	// space, every pair handed out at most once. No digits anywhere (section 5.11). Home row only.
	UPROPERTY(EditAnywhere)
	TArray<FText> CatalogPrefixes;

	UPROPERTY(EditAnywhere)
	TArray<FText> CatalogStems;

	// Name stems for a people that crossed over from this galaxy. Read by the arrivals of Phase 6a.
	UPROPERTY(EditAnywhere)
	TArray<FText> PeopleStems;
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

	// DT_Civs row of the player's first-life civ: the first life's row (CosmicRows::ArchSeeker, section 5.10).
	UPROPERTY(EditAnywhere)
	FName PlayerArchetypeRow = FName(TEXT("arch_seeker"));

	// Claim spawn condition: within this many lanes of a fallen sphere. The claim's countdown is the CountdownTicks of its
	// DT_Hazards row (section 4.2).
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

	// Population regrowth (section 4.1, 7.4): every tick, in the economy stage before the income, a people below 1.0
	// gains this much, up to 1.0; above 1.0 nothing changes. Not above 0, or not a number: no regrowth. Start value: the
	// 0.4 a plague takes comes back in 2,000 ticks (Phase 5 tunes it).
	UPROPERTY(EditAnywhere)
	float PopulationRegrowthPerTick = 0.0002f;

	// ---- Event slots (section 7.4, 7.5) ----

	// Every civilization that owns a settled system gets one event slot every this many ticks. Allowed range 1..20
	// (Data.Integrity): above 20 the two murmurs before a rebellion are no longer certain (section 4.1).
	UPROPERTY(EditAnywhere)
	int32 EventSlotTicks = 10;

	// Chance per slot that a Lottery row of BaseWeight 1 fires for a unit it is eligible for. A provisional start value:
	// what happens to one people grows with the number of eligible rows, so it is retuned with the content (section 16 #54).
	UPROPERTY(EditAnywhere)
	float EventChancePerSlot = 0.02f;

	// ---- Galaxy, layers and knowing (section 5.7, 5.11, 5.12, 7.4, 8.1). Start values ----

	// DT_Galaxies row of the home galaxy, the only galaxy simulated in detail. NewGalaxy stamps this name into the
	// GalaxyRow of every system and civilization, whether or not the table holds the row. Never changes once shipped.
	UPROPERTY(EditAnywhere)
	FName HomeGalaxyRow = FName(TEXT("gal_milkyway"));

	// The realm cap (section 5.12): a people may start settling only while it owns fewer systems than this.
	// Index = static_cast<int32>(EAscensionTier). Exactly 5 entries, ascending (Data.Integrity). A knob, not a law, and
	// the same for AI peoples and the people the player rules. Read through CosmicMegaprojects::RealmCapOf.
	UPROPERTY(EditAnywhere)
	TArray<int32> RealmCapByTier = { 1, 3, 4, 7, 10 };

	// How far the calling sees, in lanes (section 7.4 sight stage). Index = static_cast<int32>(EAscensionTier). Exactly
	// 5 entries, every one at least 1 (Data.Integrity). A knob of knowing: the world and the generation never read it.
	// First read by the sight and story-scope work of Phase 2, like the three knobs below it. {1, 2, 4, 6, 10} until MVP 3,
	// which widened the first two rungs for the told lines of the first 600 ticks (docs/60_DECISIONS/headers/A3).
	UPROPERTY(EditAnywhere)
	TArray<int32> SightLanesByTier = { 2, 3, 4, 6, 10 };

	// Period of the sight stage, in ticks.
	UPROPERTY(EditAnywhere)
	int32 SightIntervalTicks = 10;

	// The tick from which a ruler sees from the systems the people owns (5:30 at 1x). The eye of a wanderer is not held back.
	UPROPERTY(EditAnywhere)
	int32 FirstSightTick = 330;

	// Least ticks between two lines of log_far_event. log_far_ascent is told every time (section 8.1).
	UPROPERTY(EditAnywhere)
	int32 FarLineCooldownTicks = 300;

	// Star classes of living systems, cumulative, O to M: one draw per system is compared with it (section 7.4).
	// Exactly 7 entries, ascending, the last 1.0 (Data.Integrity); an array of another length is reported with one error
	// line and the C++ default is used. A design value that leans to G and K, not the real share of the classes
	// (section 3.4). Read by NewGalaxy.
	UPROPERTY(EditAnywhere)
	TArray<float> LivingWorldStarShares = { 0.01f, 0.05f, 0.14f, 0.31f, 0.57f, 0.82f, 1.0f };

	// The starting core of a fallen sphere lies at least this many lanes from the home of the first life (section 5.7):
	// what keeps pressure off the first minutes. Read by NewGalaxy; below 0 reads as 0.
	UPROPERTY(EditAnywhere)
	int32 FallenCoreMinLanesFromHome = 3;

	// ---- The picture (section 3.1 layer 1). Read by the presentation only ----

	// Points of the backdrop of the home galaxy (section 3.1, 6.4): a picture the map draws on one instanced mesh
	// (CosmicMapLook::BackdropPointsOf), never read by the simulation - no state, no hash, no selection. Read as
	// 0..100,000 (CosmicMapLook::BackdropCountOf; the schema keeps the same bounds).
	UPROPERTY(EditAnywhere)
	int32 BackgroundStarCount = 20000;

	// Points of the scenery galaxies (section 5.11): the uncounted galaxies the universe level of the map draws on one
	// instanced mesh (CosmicUniverseLook::SceneryPointsOf), never read by the simulation - no state, no hash, no selection,
	// no label, no panel. Read as 0..50,000 (CosmicUniverseLook::SceneryCountOf; the schema keeps the same bounds).
	UPROPERTY(EditAnywhere)
	int32 SceneryGalaxyCount = 10000;

	// ---- Petitions (section 5.10) ----

	// The periodic slot of the petition cards: in a tick at which Tick - Calling.BornTick is a positive multiple of this,
	// the knowing stage puts a Lottery row before a ruler with no card waiting, when no Standing row came to hold in that
	// tick (CosmicPetitions::Present). Below 1 reads as 1, without an error line (the schema keeps it at 1 or more).
	UPROPERTY(EditAnywhere)
	int32 PetitionPeriodTicks = 300;

	// ---- The first minutes and the mending of the world (MVP 3; section 2.4, 4.1, 4.3, 16 #68 #71 #91) ----

	// Stability comes back toward StabilityRestingLevel (section 4.1, 9.3, 16 #68): in the economy stage, after the
	// population's regrowth and before the income, a people below the level rises by this much and stops at the level;
	// above it nothing changes (what cards and events raised stays). Counted in the same tick's LowStabilityTicks. 0 or
	// below, above 1, or not a number: no recovery. Start value - below the drain beside a fallen sphere (0.002 x P), so
	// that a claimed people still bends (measured in the MVP 3 header: 0.002 brings the claims' settlement rate to 22%).
	UPROPERTY(EditAnywhere)
	float StabilityRecoveryPerTick = 0.001f;

	// The level Stability comes back to. Outside 0..1, or not a number: no recovery.
	UPROPERTY(EditAnywhere)
	float StabilityRestingLevel = 0.6f;

	// The AI profiles whose peoples leave a claim alone (16 #71): in the AI stage a people the player does not rule does
	// not start the answer to a Hazard.Claim on its system when its AIProfile is one of these; every other answer, and
	// every answer of any other profile (an empty profile too), it starts as before. The ruler's project key does not read
	// it. Empty = every people relieves. No draw: the profile was drawn by the generation or the change of throne.
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer ClaimLeaveProfiles;

	// The first crisis (section 2.4 'the first ten minutes', 16 #91): in the tick Tick == FirstCrisisTick, after the draw
	// and the capital rule of the crisis stage, every people's capital (owned, settled), in Civs order, receives the Draw
	// row of FirstCrisisHazard through CosmicHazards::Place - a capital whose slot holds a hazard keeps it (that one is its
	// first). The same rule for every people. 0 or below = none. No draw.
	UPROPERTY(EditAnywhere)
	int32 FirstCrisisTick = 150;

	// The tag of a Draw row of DT_Hazards (Data.Integrity); the tag of another row is refused with an error line, and tables
	// without the tag's row place nothing. Empty (the C++ default: a tag is requested from the tag table, not written
	// here) = none.
	UPROPERTY(EditAnywhere)
	FGameplayTag FirstCrisisHazard;

	// ---- The provisional end (MVP item 4; goes with the four endings of B5, B6) ----

	// The MVP run's clock end (CosmicEndings::ProvisionalEndOf): from the tick boundary where State.Tick reaches this - unless the ruled people reached the third
	// rung first - the presentation keeps the time stopped and opens the chronicle; the run's last tick is this one. 0 or below = no clock end. Nothing in the
	// simulation stops at it (the commandlet reports it). Start value 6000 = ten minutes of running at x10; it lengthens only a run the clock ends.
	UPROPERTY(EditAnywhere)
	int32 ProvisionalEndTick = 6000;

	// ---- The saved chronicle (MVP item 5) ----

	// The cap of FGalaxyState.Chronicle (docs/70 section 8.4, 16 #40 (7)): after a move out of the log, while it holds more
	// entries than this, its oldest Severity 1 entries leave - never Severity 2, Log.Calling, Log.Claim or Log.Record, so the
	// chronicle may hold more. FGalaxyState.ChronicleEviction records it and the chapter says Chronicle.More. 0 or below = no
	// cap. Story only: no world rule reads it and the world hash does not move with it. Not a display cap -
	// CosmicChronicle::MaxLifeLines is.
	UPROPERTY(EditAnywhere)
	int32 ChronicleCap = 1500;

	// ---- The ruler's research focus (MVP item 7) ----

	// The share a ruler's research focus gives its field (docs/10_DESIGN/systems.md section 5.4): the field takes it and the
	// other fields the rest evenly. The schema keeps it 0.3..1; a value not above 1/4 or above 1 (NaN included) cannot focus,
	// and UPlayerRoleSubsystem::SetResearchFocus refuses a field with one error line (the even focus never reads it). Start
	// value 0.55 - the opening petition's choices (petition_wise_heart), so its answer is one of the presets.
	UPROPERTY(EditAnywhere)
	float ResearchFocusShare = 0.55f;

	// ---- The economy and research of every people (MVP item 8, docs/10_DESIGN/systems.md section 5.1, 5.4) ----
	// Moved out of the code's constants (2026-10-18 decision: StarClassEnergy stays in C++ until step 5, the real sky).

	// Income multiplier by rung (Tier1..Tier5) on every resource: an index the array lacks, a value below 0 or not a number
	// reads as 1 (0 stays 0 - a test may lock a rung's income).
	UPROPERTY(EditAnywhere)
	TArray<float> TierMultiplier = { 1.0f, 1.5f, 2.2f, 3.0f, 3.0f };

	// Energy bonus per point of Materialism (Spiritualism below 0). 0 or below, or not a number: no bonus.
	UPROPERTY(EditAnywhere)
	float MaterialistEnergyBonus = 0.2f;

	// Insight per unit of Population before the ethics bonus and the rung. 0 or below, or not a number: no Insight.
	UPROPERTY(EditAnywhere)
	float InsightPerPopulation = 1.0f;

	// Insight bonus per point of Spiritualism (above 0). 0 or below, or not a number: no bonus.
	UPROPERTY(EditAnywhere)
	float SpiritualistInsightBonus = 0.2f;

	// Influence per unit of Population before the ethics bonus and the rung. 0 or below, or not a number: no Influence.
	// 1.0 since MVP item 8 A-8b (0.5 before - the 2026-10-18 decision 10: answers and settlements no longer starve).
	UPROPERTY(EditAnywhere)
	float InfluencePerPopulation = 1.0f;

	// Influence bonus per point of Openness (above 0). 0 or below, or not a number: no bonus.
	UPROPERTY(EditAnywhere)
	float OpenInfluenceBonus = 0.2f;

	// The share of a tick's Insight income the research stage spends (at most the stockpile): read clamped to 0..1, not a
	// number as 0.
	UPROPERTY(EditAnywhere)
	float ResearchInsightShare = 0.5f;

	// Research progress per unit of Insight spent, before the field's share. 0 or below, or not a number: no progress.
	UPROPERTY(EditAnywhere)
	float ResearchPerInsight = 0.003f;
};

// Cosmic Ascension - Data layer. Design: guideline.md section 7.3.
// Condition and effect vocabulary shared by events, hazards, feats, encounters and petitions.
// One evaluator (UCosmicEventSubsystem::Check) and one applier (UCosmicEventSubsystem::Apply) serve all of them (section 7.5).
// Rule (section 9.1): append new enumerators at the end only; never rename or reorder.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CosmicConditionTypes.generated.h"

UENUM(BlueprintType)
enum class EConditionType : uint8
{
	// CivTierAtLeast and PlayerTierAtLeast: Value = tier number 1..5 (EAscensionTier value + 1).
	CivTierAtLeast,
	CivHasTrait,
	CivResourceAtLeast,
	CivStabilityBelow,
	CivEthicsAxisAbove,
	// Value = threshold, Key = tick count (digits only, above zero; anything else is false). Reads
	// FCivilizationState.LowStabilityTicks (section 4.1).
	CivStabilityBelowFor,
	// Key = other ID (CivID or TranscendentID).
	CivRelationAtLeast,
	// ReverenceToPlayer >= Value (section 4.5).
	CivReverenceAtLeast,
	SystemHasHazard,
	SystemOwnedBy,
	// True only when MegaprojectCompletedTick > 0 and >= the SinceTick of the evaluation: 0 for events, hazards and AI
	// feats, Calling.BornTick only in the judgment of the player's feats (section 4.4, 7.5).
	MegaprojectComplete,
	// Spiritual pressure P(System) >= Value (section 4.2). The whole sum of the system: the term of the civilization
	// that owns it and of a wanderer standing there count too (section 16 #62).
	SystemPressureAtLeast,
	// Key = TranscendentID.
	SystemInfluencedBy,
	PlayerTierAtLeast,
	PlayerInSystemWithHazard,
	// Key = Body / Mind / Causality / Exaltation.
	PlayerStatAtLeast,
	PlayerHasRelic,
	// Ruler = petition card (section 5.10).
	PlayerRoleIs,
	// Tag = Status.*
	PlayerHasStatus,
	PlayerLifeIndexAtLeast,
	// Only with aging ON (section 4.4).
	PlayerAgeAtLeast,
	GalaxyTickAtLeast,
	// True only when bSealed and SealedTick >= the SinceTick of the evaluation (as MegaprojectComplete; section 4.4, 7.5).
	TranscendentSealed,
	TranscendentAwareOfPlayer,
	// Tag = Flag.*. Reads FGalaxyState.Flags (section 4.5).
	FlagSet,
	FlagNotSet,
	// Tag = Doctrine.*. Is there an unsealed active Transcendent of that doctrine (section 4.2, 4.4).
	TranscendentDoctrinePresent,
	// Tag = Notable.*. Does the context civ hold an open record (LastTick == 0) of that role (section 7.2).
	// Key = civ selector: empty = the context civilization (FCosmicEvalSubject.CivIndex; a caller that means the owner of
	// the current system puts that owner there), Former = FormerCivID.
	// The only condition that reads notables; it only picks encounter variants.
	CivHasNotable
};

USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FCosmicCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EConditionType Type = EConditionType::GalaxyTickAtLeast;

	// Trait / hazard / doctrine / status / flag ...
	UPROPERTY(EditAnywhere)
	FGameplayTag Tag;

	// Research field, resource name, stat name, other ID ...
	UPROPERTY(EditAnywhere)
	FName Key;

	// Threshold.
	UPROPERTY(EditAnywhere)
	float Value = 0.f;
};

UENUM(BlueprintType)
enum class EEffectType : uint8
{
	AddResource,
	MultiplyIncome,
	ShiftEthics,
	AddTrait,
	RemoveTrait,
	ChangeStability,
	ChangeRelation,
	SpawnHazard,
	ResolveHazard,
	TransferSystem,
	ChangeReverence,
	GrantPower,
	PlayerStatDelta,
	// Writes Tag (Flag.*) into FGalaxyState.Flags (section 4.5).
	SetFlag,
	// KillPlayer and DeposeRuler never run in place: they are only recorded in FCallingState (PendingEffect / PendingCause /
	// PendingSourceRow) and resolved by the last tick stage (section 7.4). Tag = Death.* / Reign.*; the recorded
	// PendingEffect is the tag Effect.KillPlayer / Effect.DeposeRuler.
	KillPlayer,
	DeposeRuler,
	// Value = ratio. Applies to the civ's Population only; there is no per-system population (section 4.1, 4.2).
	ChangePopulation,
	// Tag = Notable.*, Value 0. Adds one notable record to the context civ (Key = selector, as in CivHasNotable).
	// Guards live in UCosmicEventSubsystem::Apply (section 7.5); it draws one number per application, before its guards.
	// The only effect that makes a notable record; its target is the civ, never a person.
	NameNotable
	// Phase 6+ (append): MultiplyResearch, PopulationLossMultiplier, StabilityFloor (section 4.2).
};

USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FCosmicEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EEffectType Type = EEffectType::AddResource;

	UPROPERTY(EditAnywhere)
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere)
	FName Key;

	UPROPERTY(EditAnywhere)
	float Value = 0.f;

	// 0 = instant / permanent.
	UPROPERTY(EditAnywhere)
	int32 DurationTicks = 0;
};

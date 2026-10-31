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
	// Value = threshold 0.3, written for the reader: it is not a parameter. The counter counts the ticks below 0.3, the
	// code does not read the Value and the schema accepts 0.3 only (section 16 #25 (2)). Key = tick count (digits only,
	// above zero; anything else is false). Reads FCivilizationState.LowStabilityTicks (section 4.1).
	CivStabilityBelowFor,
	// Key = other ID (CivID or TranscendentID).
	CivRelationAtLeast,
	// ReverenceToPlayer >= Value (section 4.5).
	CivReverenceAtLeast,
	SystemHasHazard,
	SystemOwnedBy,
	// Key = DT_Megaprojects row (required; empty is false). True only when the subject system holds a record of that row
	// (CompletedMegaprojects) whose Tick is > 0 and >= the SinceTick of the evaluation: 0 for events, hazards and AI feats,
	// Calling.BornTick only in the judgment of the player's feats (section 4.4, 5.7, 7.5) - and made by the context
	// people (#73, B3: the record's CivID; a record of no people, from a save before B3, reads as the system's owner's).
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
	CivHasNotable,
	// Key = tick count (digits only, above zero; anything else is false). True in the one event slot of the civilization
	// in which its stay in a fallen sphere reaches Key ticks: Key <= FallenSphereTicks < Key + DT_Rules.EventSlotTicks
	// (below 1 read as 1). So for a Civilization slot row it is true in at most one slot per stay, and in exactly one when
	// the stay lasts at least Key + EventSlotTicks - 1 ticks: a Standing row whose other conditions hold then fires once, a
	// Lottery row only enters that slot's draw. Named and SettledSystem rows do not carry it (schema). The guarantee
	// assumes EventSlotTicks does not change during a stay: a stay inside its window when the value changes (a save loaded
	// under other rules) may fire twice or not at all. Tag = optional fallen Doctrine.*: one of the civilization's settled
	// systems must then also lie, now, in the sphere of an unsealed transcendent of that doctrine. Value unused (section
	// 5.11 "the book was shut", 16 #25 (1)).
	CivFallenSphereTicksReached,
	// Key = research field (Physics / Society / Engineering / Transcendence: a key of the civilization's Research; anything
	// else is false), Value 0..1: the context civilization's Research[Key] >= Value (section 5.4, 16 #29 (1)).
	CivResearchAtLeast,
	// Tag = Hazard.* (required; empty is false), Value = ticks, a whole number from 1: the subject system holds that hazard
	// with HazardTicksRemaining <= Value. A countdown loses one a tick, so a petition that waits for "fifty ticks left"
	// sees this come to hold in exactly one tick (section 4.1 (3), 5.10).
	SystemHazardTicksAtMost,
	// Value = a whole number from 1: the context civilization's settled systems (owned by it, CosmicState::IsSettled)
	// number at least Value (section 5.12; the second T2->T3 feat, MVP item 3). The subject system is not read.
	CivSettledSystemsAtLeast,
	// B4 (docs/60_DECISIONS/headers/B4_transcendent_voice_and_pressure_cards.md section 2.2): Key = a DT_Edicts row. True when
	// the context people holds it (ActiveEdicts) and the tables hold the row - the count of CosmicEdicts::HeldCountOf. Not
	// on a grace row (the schema: grace is never drawn by an edict). Carried now by event_claim_eased (edict_vigil - B4
	// [B-4d2]).
	CivHasEdict
	// Decided, appended with the first row that uses each (section 16 #29): CivStabilityAtLeast, PlayerHasPower.
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
	// Guards live in UCosmicEventSubsystem::Apply (section 7.5); it draws one number per application, before its guards
	// (a per-tick source never applies it and so never draws).
	// The only effect that makes a notable record; its target is the civ, never a person.
	NameNotable,
	// Key = research field (a key of the context civilization's ResearchFocus), Value = its share, above 0 and at most 1:
	// ResearchFocus[Key] = Value and every other field (1 - Value) / (fields - 1), so the shares still sum to 1 (section
	// 5.4). The ways of the founding petition (section 5.10); the ruler's research focus command applies the same rule
	// through CosmicResearch::Focus (MVP item 7). Never per tick.
	FocusResearch,
	// B3 (docs/60_DECISIONS/headers/B3_diplomacy_and_conflict.md section 2.9): the context people's stance toward the people
	// the Key names (an empty Key = the OtherID of the context) - Tag = Stance.Friendly, or the empty tag for neutral. Through
	// the one gate CosmicDiplomacy::StanceBlockOf (met reads true: the world never reads knowing); a refusal does nothing. A
	// card's answer only (the schema's card_only), never per tick.
	SetStance,
	// B4 (header section 2.2): Key = a DT_Edicts row. The context people enacts it through the one gate
	// CosmicEdicts::EnactBlockOf - only when it gives None, and then CosmicEdicts::Enact pays its EnactCost (the choice holds
	// no cost of its own - the schema). A refusal does nothing (the answer's gate, CosmicPetitions::ChoiceGateOf, refuses
	// first). No line: the card's outcome line tells it. A card's answer only, never per tick. Carried now by the two
	// pressure cards (petition_pressure_claim: edict_vigil, petition_pressure_measured: edict_against_silence - B4 [B-4d2]).
	EnactEdict,
	// B4 (header section 2.5, #127): Tag = Hazard.* (required), Value = ticks (a whole number 1..100). Through the one gate
	// CosmicConflict::ResistBlockOf on the context people and system: the hazard's HazardTicksRemaining gains Value (cut at
	// MAX_int32) and the people's FleetPower is multiplied by DT_Rules.ResistFleetFactor. The tier gap is not read. The one
	// way a standing hazard's countdown grows. A card's answer only, never per tick. Carried now by the second pressure card
	// (petition_pressure_measured: Hazard.Claim, 50 - B4 [B-4d2]).
	DelayHazard
	// Phase 6+ (append): MultiplyResearch, PopulationLossMultiplier, StabilityFloor (section 4.2). They are per-tick
	// effects: each joins the list of UCosmicEventSubsystem::Apply for per-tick sources when it is appended.
	// Decided, appended with the relics of Phase 4 (section 16 #29): GrantRelic, RemoveRelic. A system is let go to no
	// one by TransferSystem with an empty Key, not by an effect of its own.
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

namespace CosmicConditions
{
	// A count of ticks written as a condition's Key (CivStabilityBelowFor and the like): digits only, nine at most. Anything
	// else is 0, which means "no count": a caller tests the count for being above zero before it compares (every counter is at
	// least 0), so a malformed Key makes its condition false and is never read by its numeric prefix. The one parse - the
	// evaluator (Events) and the rebellion watch (Sim/CosmicHazards) both read a Key through it (B6a [B-6a4]).
	inline int32 TickCountOf(FName Key)
	{
		if (Key.IsNone())
		{
			return 0;
		}
		const FString Text = Key.ToString();
		if (Text.Len() > 9)
		{
			return 0;
		}
		int32 Count = 0;
		for (const TCHAR Char : Text)
		{
			if (Char < TEXT('0') || Char > TEXT('9'))
			{
				return 0;
			}
			Count = Count * 10 + static_cast<int32>(Char - TEXT('0'));
		}
		return Count;
	}
}

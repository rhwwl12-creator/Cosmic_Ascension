// Cosmic Ascension - Ascension layer. Design: guideline.md section 3.2, 4.3, 6.1, 7.5.
// RULE (section 6.1): files under Ascension/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Core/CosmicTypes.h"
#include "Math/RandomStream.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AscensionSubsystem.generated.h"

struct FAscensionFeatDef;
struct FCosmicEvalContext;
struct FGalaxyState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTierAscended, EAscensionTier, NewTier, FName, FeatRow, EPlayerRole, Path);

/**
 * The feat judgment: watches the Mythic Feat conditions and raises a Tier (section 3.2, 4.3, 7.5). No member state: what
 * a feat changes lives in FGalaxyState.
 */
UCLASS()
class COSMIC_ASCENSION_API UAscensionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * The feat judgment (section 3.2, 4.3, 7.5): exactly once per tick, at the end of the AI stage, over the live state and
	 * the sim stream it is handed. First the people the vessel rules (only while Embodied and Ruler; SinceTick =
	 * Calling.BornTick), then every other people with a settled system in Civs order (SinceTick 0). For a people, the
	 * DT_Feats rows with Path Ruler and RequiredTier == its Tier are looked at in lexical order (a row whose TargetTier is
	 * not RequiredTier + 1, or that has no condition, is not judged: an error line); the first whose CompletionConditions
	 * all hold with the people and one of the settled systems it owns (OwnedSystems order) as subject is the one, and a
	 * people climbs at most one rung per tick. Climbing, in order: Tier = TargetTier; AscensionCost (ApplyAll, SourceRow =
	 * the feat row); for the player CompletedFeats, the Feats and the PeakTier of the open life and GrantedPowers; for another
	 * people reaching Tier 3 one Notable.Ascendant record (NameNotable through Apply: one draw, before its guards); from
	 * Tier 4 on the people's open notable records close (LastTick = Tick); the ascension line (the row's LogTemplates,
	 * Log.Ascension, Severity 2); the lines that follow (FollowUpNotes); for the player OnTierAscended. Wanderer feats are
	 * judged from Phase 4, with their first row. Inside a handler of a Core Logic delegate, or over a state that is not
	 * the live one: an error line and nothing.
	 */
	void EvaluateFeats(FGalaxyState& State, FRandomStream& Rng);

	// The player's ascensions only (section 7.5): an AI people's ascension is its line alone. Presentation listens; the
	// effect and sound of Phase 3 hang here.
	UPROPERTY(BlueprintAssignable)
	FOnTierAscended OnTierAscended;

private:
	// The judgment of one people. True when it climbed.
	bool JudgePeople(FGalaxyState& State, FRandomStream& Rng, int32 CivIndex, int64 SinceTick, bool bPlayer);

	// One climb by FeatRow, in the order EvaluateFeats gives.
	void Ascend(FCosmicEvalContext& Ctx, FName FeatRow, const FAscensionFeatDef& Feat, bool bPlayer);
};

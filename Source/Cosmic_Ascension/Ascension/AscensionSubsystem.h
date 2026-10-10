// Cosmic Ascension - Ascension layer. Design: guideline.md section 3.2, 6.1, 7.5.
// RULE (section 6.1): files under Ascension/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AscensionSubsystem.generated.h"

/**
 * Watches Mythic Feat conditions, raises Ascension Tier, grants powers, requests FX via delegates.
 * Phase 0: empty shell. Interface (EvaluateFeats / OnTierAscended) is added in Phase 2 (section 7.5).
 */
UCLASS()
class COSMIC_ASCENSION_API UAscensionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};

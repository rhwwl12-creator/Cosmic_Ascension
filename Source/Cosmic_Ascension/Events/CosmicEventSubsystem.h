// Cosmic Ascension - Events layer. Design: guideline.md section 6.1, 7.3, 7.5.
// RULE (section 6.1): files under Events/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CosmicEventSubsystem.generated.h"

/**
 * Condition evaluation, weighted event selection, effect application, cooldowns.
 * Phase 0: empty shell. Interface (EvaluateAndFire / Check / Apply / OnEventFired) is added in Phase 2 (section 7.5).
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};

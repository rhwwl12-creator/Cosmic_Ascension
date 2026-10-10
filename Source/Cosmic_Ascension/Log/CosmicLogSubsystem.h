// Cosmic Ascension - Log layer. Design: guideline.md section 6.1, 7.5, 8.
// RULE (section 6.1): files under Log/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CosmicLogSubsystem.generated.h"

/**
 * Procedural narrative log: template substitution, log queue, chronicle aggregation.
 * Phase 0: empty shell. Interface (Push / OnLogEntry / BuildChronicle) is added in Phase 1 (section 7.5, 8).
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicLogSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};

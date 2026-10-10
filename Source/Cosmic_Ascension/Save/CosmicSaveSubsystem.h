// Cosmic Ascension - Save layer. Design: guideline.md section 6.1, 9.1.
// RULE (section 6.1): files under Save/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CosmicSaveSubsystem.generated.h"

/**
 * FGalaxyState <-> USaveGame serialization and slot management.
 * Phase 0: empty shell. Save/Load round-trip is a Phase 1 exit criterion (section 9.1, 11).
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};

// Cosmic Ascension - Sim layer. Design: guideline.md section 6.1, 7.4.
// RULE (section 6.1): files under Sim/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GalaxySimulationSubsystem.generated.h"

/**
 * Fixed-step galaxy simulation: economy, research, diplomacy, conflict, hazards, civ AI, player travel.
 * Phase 0: empty shell so the module compiles and the editor starts.
 * The full interface (FGalaxyState, FTickableGameObject, NewGalaxy, SetTimeScale, ...) is added in Phase 1 (section 7.4).
 */
UCLASS()
class COSMIC_ASCENSION_API UGalaxySimulationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};

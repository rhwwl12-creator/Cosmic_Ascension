// Cosmic Ascension - Player layer. Design: guideline.md section 3.3, 5.9, 5.10, 7.5.
// Player/ is the command API used by UI; it may read state but must not include UUserWidget or AActor headers either.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerRoleSubsystem.generated.h"

/**
 * Ruler / Wanderer role switching, travel, encounter flow, and the ruler command API.
 * Phase 0: empty shell. Interface (BecomeRuler / BecomeWanderer / TravelTo / ResolveEncounterChoice / ...) is added in Phase 4 (section 7.5).
 */
UCLASS()
class COSMIC_ASCENSION_API UPlayerRoleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};

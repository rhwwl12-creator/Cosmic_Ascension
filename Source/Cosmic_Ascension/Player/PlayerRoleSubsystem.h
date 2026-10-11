// Cosmic Ascension - Player layer. Design: guideline.md section 3.3, 4.4, 5.9, 5.10, 7.5.
// Player/ is the command API used by UI; it may read state but must not include UUserWidget or AActor headers either.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/CosmicTypes.h"
#include "PlayerRoleSubsystem.generated.h"

/**
 * Ruler / Wanderer role switching, travel, encounter flow, the ruler command API, and the death, deposition and
 * succession pipeline. Holds no player or life dependent member: all of it lives in FGalaxyState (section 6.1).
 * So far only Install exists; the rest of the interface in section 7.5 arrives with Phase 4 and 4b.
 */
UCLASS()
class COSMIC_ASCENSION_API UPlayerRoleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Starts a life on a new vessel (section 4.4, 7.5). The first life (NewGalaxy, Ruler) and every succession go through
	 * here. Resets the vessel, opens a FLifeRecord and, for a ruler, opens a reign in the three reign arrays.
	 */
	void Install(FName SystemID, FName CivID, EPlayerRole Role);
};

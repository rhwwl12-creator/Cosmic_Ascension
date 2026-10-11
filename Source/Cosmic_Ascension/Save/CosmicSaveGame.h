// Cosmic Ascension - Save layer. Design: guideline.md section 9.1.
// RULE (section 6.1): files under Save/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Core/GalaxyState.h"
#include "CosmicSaveGame.generated.h"

/**
 * The whole save: one FGalaxyState. Actors and widgets are never saved; the presentation layer redraws from the state.
 * Written with UGameplayStatics::SaveGameToSlot, which stores a delta against the class default object: see the
 * save-compatibility rules at the top of Core/GalaxyState.h before changing any default member initializer.
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGalaxyState State;
};

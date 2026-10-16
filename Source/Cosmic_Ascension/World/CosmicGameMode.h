// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 6.1, 11 Phase 1.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "CosmicGameMode.generated.h"

class UCosmicTextPanelWidget;

/**
 * Game mode of the galaxy map. Starts a galaxy when none is running (the Phase 1 prototype: five systems, seed from
 * -Seed= on the command line or from the clock) and puts the HUD and the player controller in place.
 * Selected for every map by GlobalDefaultGameMode in Config/DefaultEngine.ini.
 */
UCLASS()
class COSMIC_ASCENSION_API ACosmicGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACosmicGameMode();

	virtual void StartPlay() override;
};

/** Shows the Phase 1 text panel. */
UCLASS()
class COSMIC_ASCENSION_API ACosmicHUD : public AHUD
{
	GENERATED_BODY()

public:
	// Redraws the panel from the current state.
	void RefreshPanel();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UCosmicTextPanelWidget> Panel;
};

/**
 * Controls of the prototype: 0 pauses, 1 to 4 set the speed to 1x, 2x, 5x and 10x, Space toggles the pause, P starts the
 * next megaproject of the ruled people (UPlayerRoleSubsystem::StartNextMegaproject; the panels of Phase 3 replace it).
 */
UCLASS()
class COSMIC_ASCENSION_API ACosmicPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACosmicPlayerController();

protected:
	virtual void SetupInputComponent() override;

private:
	void HandleSpeedKey(FKey Key);
	void HandleProjectKey(FKey Key);

	// Speed to return to when the pause is released.
	float SpeedBeforePause = 1.f;
};

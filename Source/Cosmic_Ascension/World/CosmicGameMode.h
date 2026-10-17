// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 6.1, 11 Phase 1.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "CosmicGameMode.generated.h"

class ACosmicCameraPawn;
class ACosmicGalaxyMapActor;
class UCosmicSystemPanelWidget;
class UCosmicTextPanelWidget;
class UCosmicTopBarWidget;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCosmicSelectionChanged, FName /* SystemID, None when cleared */);

/**
 * Game mode of the galaxy map. Starts a galaxy when none is running (the counts of the home galaxy row, seed from -Seed=
 * on the command line or from the clock), puts the HUD and the player controller in place, spawns the map
 * (ACosmicGalaxyMapActor) and a camera pawn (ACosmicCameraPawn) for each player controller, possesses it and hands the
 * map to the controller (UseMap). The pawn is spawned here, not as the default pawn: L_Galaxy has no PlayerStart.
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

/**
 * Puts the panels on the screen: the top bar along the top, the system panel on the right (it follows the controller's
 * selection) and the log tail of the Phase 1 text panel at the bottom left (until the feed of Phase 3 item 4).
 */
UCLASS()
class COSMIC_ASCENSION_API ACosmicHUD : public AHUD
{
	GENERATED_BODY()

public:
	// Redraws every panel from the current state (after a command or a new speed: a paused simulation sends no tick).
	void RefreshPanel();

	UCosmicTopBarWidget* GetTopBar() const { return TopBar; }
	UCosmicSystemPanelWidget* GetSystemPanel() const { return SystemPanel; }
	UCosmicTextPanelWidget* GetLogPanel() const { return Panel; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleSelectionChanged(FName SystemID);

	UPROPERTY(Transient)
	TObjectPtr<UCosmicTextPanelWidget> Panel;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicTopBarWidget> TopBar;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicSystemPanelWidget> SystemPanel;

	FDelegateHandle SelectionHandle;
};

/**
 * Controls: the mouse drives the camera pawn and selects (left click selects, left drag pans, right drag orbits, the
 * wheel zooms, a double click focuses); 0 pauses, 1 to 4 set 1x, 2x, 5x and 10x, Space toggles the pause (the top bar's
 * buttons share SetSpeed). The system panel's commands come through StartProject. Holds the selection (presentation
 * state: not in FGalaxyState, never saved).
 */
UCLASS()
class COSMIC_ASCENSION_API ACosmicPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACosmicPlayerController();

	/** The map this controller selects on (the game mode hands it over after spawning it). */
	void UseMap(ACosmicGalaxyMapActor* InMap);

	/** Selects a system of the state; None clears. An ID the state does not hold is ignored. A change is announced once. */
	void SelectSystem(FName SystemID);
	FName GetSelectedSystem() const { return SelectedSystemID; }
	FOnCosmicSelectionChanged OnSelectionChanged;

	/** Sets the simulation's speed (0 pauses) and remembers the speed to return to after a pause. Redraws the HUD. */
	void SetSpeed(float Speed);

	/** The ruler's command of the system panel (UPlayerRoleSubsystem::StartMegaproject: RowName on SystemID, or
	 * project_settle on the wild SystemID). On success redraws the HUD and asks the map to redraw. */
	bool StartProject(FName SystemID, FName RowName);

	/** Development aid: selects and focuses a system as a double click on it would ("CosmicSelect sol" at the console or
	 * in -ExecCmds). Waits for the map when given before it is built. */
	UFUNCTION(Exec)
	void CosmicSelect(const FString& SystemID);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

private:
	void HandleSpeedKey(FKey Key);
	void HandleLeftPressed(FKey Key);
	void HandleLeftReleased(FKey Key);
	void HandleLeftDoubleClick(FKey Key);
	void HandleRightPressed(FKey Key);
	void HandleRightReleased(FKey Key);
	void HandleWheel(FKey Key);
	void HandleMapRebuilt();

	// The system under the cursor (CosmicMapPick::NearestOnScreen over the projected stars), or None.
	FName PickAtCursor() const;
	// The point of the disc plane under a screen position.
	bool DiscPointAt(const FVector2D& Screen, FVector& OutPoint) const;
	// Selects and focuses SystemID when the map holds it; false otherwise.
	bool SelectAndFocus(FName SystemID);
	ACosmicCameraPawn* GetCameraPawn() const;

	// Speed to return to when the pause is released.
	float SpeedBeforePause = 1.f;

	FName SelectedSystemID;
	// CosmicSelect given before the map was built.
	FName PendingSelect;

	TWeakObjectPtr<ACosmicGalaxyMapActor> Map;
	FDelegateHandle RebuiltHandle;

	// The mouse: a left press becomes a pan once it moves 6 px; each frame of a pan or an orbit moves by the mouse's
	// movement since the last frame.
	bool bLeftDown = false;
	bool bPanning = false;
	FVector2D LeftPressAt = FVector2D::ZeroVector;
	bool bRightDown = false;
	FVector2D LastMouse = FVector2D::ZeroVector;
};

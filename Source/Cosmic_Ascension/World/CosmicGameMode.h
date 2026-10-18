// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 6.1, 11 Phase 1.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Misc/Optional.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicMapLook.h"
#include "CosmicGameMode.generated.h"

class ACosmicCameraPawn;
class ACosmicGalaxyMapActor;
class UCosmicAscensionOverlayWidget;
class UCosmicLogFeedWidget;
class UCosmicSystemPanelWidget;
class UCosmicTopBarWidget;
class UUserWidget;

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
 * selection), the log feed at the bottom left, and over them all the ascension presentation (hidden until an ascension).
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
	UCosmicLogFeedWidget* GetLogFeed() const { return Feed; }
	UCosmicAscensionOverlayWidget* GetAscensionOverlay() const { return Overlay; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleSelectionChanged(FName SystemID);
	// The clock's unit changed: the top bar, the panel's countdown and the feed's stamps follow.
	void HandleTimeUnitChanged();
	// The view changed: the top bar's view button and the panel's spiritual line follow (the controller tells the map
	// itself).
	void HandleViewModeChanged();

	UPROPERTY(Transient)
	TObjectPtr<UCosmicLogFeedWidget> Feed;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicAscensionOverlayWidget> Overlay;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicTopBarWidget> TopBar;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicSystemPanelWidget> SystemPanel;

	FDelegateHandle SelectionHandle;
	FDelegateHandle TimeUnitHandle;
	FDelegateHandle ViewModeHandle;
};

/**
 * Controls: the mouse drives the camera pawn and selects (left click selects, left drag pans, right drag orbits, the
 * wheel zooms, a double click focuses); 0 pauses, 1 to 4 set 1x, 2x, 5x and 10x, Space toggles the pause (the top bar's
 * buttons share SetSpeed). The system panel's commands come through StartProject. Holds the selection, the clock's unit
 * and the view (physical or the third rung's spiritual sight) - presentation state: not in FGalaxyState, never saved
 * with a galaxy (the unit is the viewer's setting, the view lives in memory only).
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

	/** How the UI shows the clock (guideline 2.1, 6.3): the viewer's choice, read from the viewer's settings when the
	 * controller starts (Cycle when none is kept). */
	ECosmicTimeUnit GetTimeUnit() const { return TimeUnit; }
	/** Sets the unit in memory only (the tests fix it this way); a change is announced once. */
	void SetTimeUnit(ECosmicTimeUnit Unit);
	/** The clock's press: the other unit, kept in the viewer's settings for the next session - never while automation
	 * tests run (a test on the developer's machine must not change the developer's choice). */
	void ToggleTimeUnit();
	FSimpleMulticastDelegate OnTimeUnitChanged;

	/** The unit a widget shows: its owning controller's, Cycle without one. */
	static ECosmicTimeUnit TimeUnitFor(const UUserWidget& Widget);

	/** Where the viewer's unit is kept: this section and key of GGameUserSettingsIni, "Year" or "Cycle" (tests read it). */
	static constexpr const TCHAR* TimeUnitSection = TEXT("Cosmic_Ascension.View");
	static constexpr const TCHAR* TimeUnitKey = TEXT("TimeUnit");

	/**
	 * How the map is seen (section 6.4): the view the viewer wants. Presentation state - not in FGalaxyState, never saved,
	 * outside both hashes, not kept across sessions. Spiritual by default, so that the map turns at the tick the viewer
	 * reaches the third rung; a new galaxy or a load returns it to the default, and so does the sight opening again after
	 * it closed (FollowSightOpening). Drawn only while the sight is open.
	 */
	ECosmicViewMode GetWantedViewMode() const { return WantedViewMode; }
	/** Sets the wanted view in memory (tests fix it so); a change is handed to the map and announced once. */
	void SetWantedViewMode(ECosmicViewMode Mode);
	/** The top bar's view button: the other view. Refused - nothing changes, nothing is announced - while the sight is
	 * closed. */
	void ToggleViewMode();
	/** Whether the spiritual sight is open: CosmicMapLook::SpiritualSightOpens of the simulation's viewer tier, or forced
	 * open by CosmicSpiritualSight. */
	bool IsSpiritualSightOpen() const;
	/** The view drawn: CosmicMapLook::ShownViewMode of the wanted view, the force and the viewer tier. */
	ECosmicViewMode GetShownViewMode() const;
	/**
	 * Follows the sight (every frame, from PlayerTick; tests call it in place of a frame): when it went from closed to
	 * open since the last call, the wanted view returns to Spiritual - the reveal of the opening never contradicts the map
	 * (section 16 #96). A first call only notes whether the sight is open.
	 */
	void FollowSightOpening();
	/** Announced when the wanted view or the force changes (the HUD redraws the top bar and the panel). */
	FSimpleMulticastDelegate OnViewModeChanged;

	/** The view a widget shows: its owning controller's view drawn; Physical without one. */
	static ECosmicViewMode ShownViewModeFor(const UUserWidget& Widget);
	/** Whether a widget's owning controller has the sight open; false without one. */
	static bool IsSpiritualSightOpenFor(const UUserWidget& Widget);

	/** The ruler's command of the system panel (UPlayerRoleSubsystem::StartMegaproject: RowName on SystemID, or
	 * project_settle on the wild SystemID). On success redraws the HUD and asks the map to redraw. */
	bool StartProject(FName SystemID, FName RowName);

	/** Selects SystemID and glides the camera to it, as a double click on its star does (the log feed's lines too). False
	 * when the map holds no such star. */
	bool SelectAndFocus(FName SystemID);

	/** Development aid: selects and focuses a system as a double click on it would ("CosmicSelect sol" at the console or
	 * in -ExecCmds). Waits for the map when given before it is built. */
	UFUNCTION(Exec)
	void CosmicSelect(const FString& SystemID);

	/** Development aid: after DelaySeconds of real time shows the ascension presentation to the second rung
	 * ("CosmicPreviewAscension 23" with -ShotAfterSeconds=25 photographs it) - CosmicPreviewRung of the second rung.
	 * Changes no state. */
	UFUNCTION(Exec)
	void CosmicPreviewAscension(float DelaySeconds);

	/** Development aid: after DelaySeconds of real time shows the ascension presentation to Rung (1..5; anything else is
	 * the second) with the first feat row that reaches that rung, or the first feat row ("CosmicPreviewRung 19 3" shows
	 * the third rung's, whose close shows the reveal of the spiritual sight when the view drawn will be spiritual - force
	 * the sight open below the third rung). A command of its own, not a default argument of CosmicPreviewAscension: an
	 * Exec's default comes from editor-only metadata, and a build without it refuses a call that leaves the argument out.
	 * Changes no state. */
	UFUNCTION(Exec)
	void CosmicPreviewRung(float DelaySeconds, int32 Rung);

	/** Development aid: "CosmicSpiritualSight 1" forces the sight open at any rung for the session (the off-screen
	 * screenshot, the frame-time measure) and logs the first system of each fallen sphere for CosmicSelect; 0 gives the
	 * rung its say back. Changes no state, keeps nothing. */
	UFUNCTION(Exec)
	void CosmicSpiritualSight(int32 Open);

protected:
	// Reads the viewer's clock unit before the HUD exists. A development aid: -TimeUnit=Year (or Cycle) sets it for the
	// session without keeping it (screenshots).
	virtual void PostInitializeComponents() override;
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
	ACosmicCameraPawn* GetCameraPawn() const;

	// Speed to return to when the pause is released.
	float SpeedBeforePause = 1.f;

	ECosmicTimeUnit TimeUnit = ECosmicTimeUnit::Cycle;

	ECosmicViewMode WantedViewMode = ECosmicViewMode::Spiritual;
	bool bSightForcedOpen = false;
	// Whether the sight was open at the last FollowSightOpening; unset until the first.
	TOptional<bool> SightWasOpen;

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

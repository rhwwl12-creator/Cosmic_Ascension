// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 6.1, 11 Phase 1.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Misc/Optional.h"
#include "Templates/Function.h"
#include "UI/CosmicUILabels.h"
#include "Events/CosmicPetitions.h"
#include "Sim/CosmicEndings.h"
#include "World/CosmicAutoPause.h"
#include "World/CosmicMapLook.h"
#include "World/CosmicMapPick.h"
#include "World/CosmicZoomLevels.h"
#include "CosmicGameMode.generated.h"

class ACosmicCameraPawn;
class FViewport;
class ACosmicGalaxyMapActor;
class UCosmicAscensionOverlayWidget;
class UCosmicChronicleWidget;
class UCosmicGalaxyPanelWidget;
class UCosmicLogFeedWidget;
class UCosmicPetitionCardWidget;
class UCosmicSystemPanelWidget;
class UCosmicTopBarWidget;
class UGameViewportClient;
class UUserWidget;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCosmicSelectionChanged, FName /* SystemID, None when cleared */);

/** A star a click may take (ACosmicPlayerController::PickCandidatesOf): its system and how the screen shows it. */
struct COSMIC_ASCENSION_API FCosmicPickCandidate
{
	FName SystemID;
	FCosmicScreenTarget Target;
};

/** A named galaxy a click may take at the universe level (ACosmicPlayerController::GalaxyPickCandidatesOf): its row and how
 * the screen shows it. */
struct COSMIC_ASCENSION_API FCosmicGalaxyPickCandidate
{
	FName Row;
	FCosmicScreenTarget Target;
};

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
 * Puts the panels on the screen: the top bar along the top, on the right the system panel or the galaxy panel, as
 * CosmicUniverseLook::PanelOf says of the camera's level and the two selections (section 6.4 - the two never stand
 * together, section 8.3), the galaxy panel with the session's hint (CosmicUI::HintKeyOf), the log feed at the bottom left,
 * the petition card in the middle over the panels (collapsed while no card shows - MVP item 2), over them the ascension
 * presentation (hidden until an ascension), and over everything the chronicle (collapsed until the run ends - MVP item 4).
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
	UCosmicGalaxyPanelWidget* GetGalaxyPanel() const { return GalaxyPanel; }
	UCosmicLogFeedWidget* GetLogFeed() const { return Feed; }
	UCosmicAscensionOverlayWidget* GetAscensionOverlay() const { return Overlay; }
	UCosmicPetitionCardWidget* GetPetitionCard() const { return Card; }
	UCosmicChronicleWidget* GetChronicle() const { return Chronicle; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// The right side again (ShowRightSide).
	void HandleSelectionChanged(FName SystemID);
	// The right side again: CosmicUniverseLook::PanelOf of the controller's level and selections, and the galaxy panel's
	// hint (CosmicUI::HintKeyOf of the level and the controller's hint progress). Bound to the system selection, the galaxy
	// selection and the level; BeginPlay and RefreshPanel call it too.
	void ShowRightSide();
	// The clock's unit changed: the top bar, the panel's countdown and the feed's stamps follow.
	void HandleTimeUnitChanged();
	// The view changed: the top bar's view button and the panel's spiritual line follow (the controller tells the map
	// itself).
	void HandleViewModeChanged();

	UPROPERTY(Transient)
	TObjectPtr<UCosmicGalaxyPanelWidget> GalaxyPanel;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicLogFeedWidget> Feed;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicAscensionOverlayWidget> Overlay;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicPetitionCardWidget> Card;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicChronicleWidget> Chronicle;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicTopBarWidget> TopBar;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicSystemPanelWidget> SystemPanel;

	FDelegateHandle SelectionHandle;
	FDelegateHandle GalaxySelectionHandle;
	FDelegateHandle ZoomLevelHandle;
	FDelegateHandle TimeUnitHandle;
	FDelegateHandle ViewModeHandle;
};

/**
 * Controls: the mouse drives the camera pawn and selects (left click selects, left drag pans, right drag orbits, the
 * wheel zooms, a double click focuses; past the region's far stop the wheel goes on to the universe level - the named
 * galaxies: a click selects one, a double click glides to it, a double click on the home galaxy frames the region again);
 * 0 pauses, 1 to 4 set 1x, 2x, 5x and 10x and 5 sets 25x once the third rung's
 * gate is open (CosmicUI::SpeedSteps, as the top bar's buttons; a key whose speed the gate does not open does nothing),
 * Space toggles the pause (the top bar's buttons share SetSpeed). The system panel's commands come through StartProject.
 * Stops a running simulation by itself when the viewer keeps the automatic pause on (RequestAutoPause). Holds the
 * selection, the galaxy selection, the session's hint progress, the clock's unit, the automatic pause and the view
 * (physical or the third rung's spiritual sight) - presentation state: not in FGalaxyState, never saved with a galaxy
 * (the unit and the automatic pause are the viewer's settings, the view and the hints live in memory only).
 */
UCLASS()
class COSMIC_ASCENSION_API ACosmicPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACosmicPlayerController();

	/** The map this controller selects on (the game mode hands it over after spawning it). */
	void UseMap(ACosmicGalaxyMapActor* InMap);

	/** Selects a system of the state the viewer knows (GetKnownView - a system the viewer does not know is a backdrop
	 * point, and background stars have no selection, section 6.4); None clears. Anything else - an ID the state does not
	 * hold, a system the viewer does not know - is ignored: nothing changes, nothing is announced. A change is announced
	 * once. bByViewer false (the automatic crisis focus, MVP item 6): the selection is not the viewer's and is not noted
	 * in the hint progress. */
	void SelectSystem(FName SystemID, bool bByViewer = true);
	FName GetSelectedSystem() const { return SelectedSystemID; }
	FOnCosmicSelectionChanged OnSelectionChanged;

	/** Selects a named galaxy - a row the universe draws (the map's GetGalaxyMarks); None clears. Any other row is ignored:
	 * nothing changes, nothing is announced. A change is announced once. Presentation state, like the system selection:
	 * never saved, outside both hashes; a rebuild clears it. The system selection stays as it is - the right side follows
	 * the level (CosmicUniverseLook::PanelOf). The first selection of the session is noted in the hint progress. */
	void SelectGalaxy(FName Row);
	FName GetSelectedGalaxy() const { return SelectedGalaxyRow; }
	FOnCosmicSelectionChanged OnGalaxySelectionChanged;

	/** What a double click on a galaxy does: the home galaxy clears the galaxy selection and frames the region again (a
	 * glide); another named galaxy is selected and the camera glides to it (ACosmicCameraPawn::FocusOnUniverse at
	 * CosmicUniverseLook::FocusArmOf). False - nothing changes - for a row the universe does not draw. */
	bool SelectAndFocusGalaxy(FName Row);

	/** The camera framed on the map's levels (ACosmicCameraPawn::FrameAll(Map->GetZoomLevels())) - a glide: the double
	 * click on the home; the third rung's reveal over the universe (UCosmicAscensionOverlayWidget::Close) and a rebuild
	 * snap it. */
	void FrameRegion();

	/** The level of the camera's target arm (ACosmicCameraPawn::GetTargetLevel); Region without a camera pawn. */
	ECosmicZoomLevel GetZoomLevel() const;
	/** Follows the level (every frame from PlayerTick and after a wheel notch; tests call it in place of a frame): a change
	 * is announced once and noted in the hint progress (into the universe, out of it). A first call only notes the level. */
	void FollowZoomLevel();
	FSimpleMulticastDelegate OnZoomLevelChanged;

	/** What the mouse has done once this session (the galaxy panel's hints, CosmicUI::HintKeyOf). */
	const FCosmicHintProgress& GetHintProgress() const { return Hints; }
	/** Sets the progress in memory (tests start from a known one; the HUD redraws with RefreshPanel). */
	void SetHintProgress(const FCosmicHintProgress& Progress) { Hints = Progress; }

	/** Sets the simulation's speed (0 pauses) - a speed the third rung's gate does not open is x10 instead (CosmicUI::
	 * OpenSpeed with IsSpiritualSightOpen; the pause's release and the ascension presentation's give-back come through
	 * here too) - and remembers the speed to return to after a pause. Clears the reason of an automatic pause. Redraws the
	 * HUD. */
	void SetSpeed(float Speed);

	/**
	 * The automatic pause (guideline 2.1, MVP item 1): whether the screens stop a running simulation by themselves - on a
	 * told mythic line (CosmicAutoPause::StopsFor, from OnLogEntry), when a hazard newly stands on a system of the people
	 * the vessel rules (FollowOwnHazards, at the tick boundary) and when a card arrives (the card widget asks). The viewer's
	 * choice, on by default: read from the viewer's settings when the controller starts (AutoPauseKey of TimeUnitSection),
	 * and for the session from -AutoPause=0 or 1 on the command line (a development aid - the frame-time measure at x10).
	 */
	bool IsAutoPauseOn() const { return bAutoPause; }
	/** Sets it in memory only (the tests fix it this way) and redraws the HUD. */
	void SetAutoPause(bool bOn);
	/** The top bar's button: the other choice, kept in the viewer's settings for the next session - never while automation
	 * tests run. */
	void ToggleAutoPause();
	/** Stops a running simulation for Reason when the automatic pause is on, as the pause does (SetSpeed(0) - the speed to
	 * return to is kept), and notes Reason for the top bar's line. Does nothing to a paused simulation, with the automatic
	 * pause off, or for None. Safe inside a handler of a Core Logic delegate (only SetTimeScale reaches the simulation).
	 * True when it stopped the simulation (MVP item 6: only then does the crisis focus move the camera). */
	bool RequestAutoPause(ECosmicAutoPause Reason);
	/** Why the simulation stands still: the reason of the automatic pause that stopped it, None once the speed was set
	 * again (or a new galaxy or a load came). */
	ECosmicAutoPause GetAutoPauseReason() const { return AutoPauseReason; }
	/** Follows the hazards on the systems of the people the vessel rules (every tick from OnSimTick; tests call it in place
	 * of a tick): a hazard newly standing there since the last call asks for the automatic pause (OwnHazard); when that
	 * stops the time, the system (CosmicAutoPause::FirstNewHazardSystem) is selected and the camera glides to it -
	 * SelectAndFocus not by the viewer (MVP item 6; a press in flight does not become a click). The first call after the
	 * controller starts, a new galaxy or a load only notes them. */
	void FollowOwnHazards();
	/** Where the viewer's choice is kept: this key of TimeUnitSection, "On" or "Off" (tests read it). */
	static constexpr const TCHAR* AutoPauseKey = TEXT("AutoPause");

	/**
	 * Follows the petition card that waits (GetKnownView().PendingPetitionRow - MVP item 2): when it is another card than
	 * the one the HUD's card shows, the card shows it (a new voice picked), and a card that arrived at the tick boundary
	 * (bArrived: OnSimTick) asks for the automatic pause (Card); a card that waits after a new galaxy or a load shows
	 * without a pause. The same card is redrawn. An outcome on the card stays until it is closed. Tests call it in place
	 * of a tick.
	 */
	void FollowPetition(bool bArrived);
	/** The card's answer: UPlayerRoleSubsystem::AnswerPetition; an answer taken redraws the HUD and the map (as
	 * StartProject - a paused simulation sends no tick). */
	FCosmicPetitionAnswer AnswerPetition(int32 ChoiceIndex);
	/** The card was closed after an outcome: a chained card shows at once (FollowPetition); otherwise a pause the card
	 * made (reason Card, still standing still) gives the time back at the speed before it. */
	void ClosePetitionCard();

	/** The provisional end as FollowEnd latched it (MVP item 4) - never read afresh elsewhere, so a state a test left behind ends nothing until a tick boundary,
	 * a new galaxy, a load or a BeginPlay says so. None while the run goes on or the end is switched off. Never saved, outside both hashes. */
	ECosmicProvisionalEnd GetRunEnd() const { return RunEnd; }
	bool IsRunEnded() const { return RunEnd != ECosmicProvisionalEnd::None; }
	static bool IsRunEndedFor(const UUserWidget& Widget);   // the widget's owning controller holds an ended run; false without one
	/** Looks at the end (nothing while it is switched off). bArrived (HandleSimTick; tests in its place): a cause seen while none is latched latches it, stops the
	 * time whatever the automatic pause (ApplySpeed(0, RunEnd)), folds the card, logs one CosmicEnd line and starts the wait. Without it (HandleGalaxyGenerated, the
	 * BeginPlay of this controller and of the HUD): the same latch with no wait, and while latched the page opens at once, fully shown, if the HUD has it and it is
	 * closed - whichever BeginPlay runs second opens it. Calls nothing a Core Logic handler may not call (only SetTimeScale reaches the simulation). */
	void FollowEnd(bool bArrived);
	void AdvanceRunEnd(float DeltaSeconds);   // real seconds, counted only while no ascension presentation or reveal shows (PlayerTick; tests); opens after HoldSeconds
	void ClearRunEnd();                       // forgets the latch, the wait and the page; switch and speed stay, the RunEnd reason goes (to None) (HandleGalaxyGenerated first; CosmicTests::AcquireSim)
	void SetProvisionalEndEnabled(bool bOn);  // on; -ProvisionalEnd=0 turns it off for the session, tests in memory; off = ClearRunEnd
	bool IsProvisionalEndEnabled() const { return bProvisionalEnd; }
	void LookAtMap();                         // the page folds over the stopped map
	void ReopenChronicle();                   // the top bar's 'chronicle': the page again, fully shown
	void StartNewRun();                       // NewGalaxy from the clock's seed (-Seed= names the first galaxy only), saves it (MVP item 5), then x1; an input, never from a handler
	// After FollowEnd(false) in BeginPlay: a continued run that is not over stands still with its line (MVP item 5:
	// Continued, ContinuedFromBackup, or SaveUnreadable for the new run after a save that did not read) - unless -Speed= was
	// given. Takes UCosmicSaveSubsystem::TakeStartOutcome; never from a handler of a Core Logic delegate.
	void FollowStart();
	/** Development aid: after DelaySeconds of real time the chronicle over the current state, fully shown, with no cause ("CosmicPreviewChronicle 20" with
	 * -ShotAfterSeconds=22 photographs it). Ends nothing, changes no state. */
	UFUNCTION(Exec)
	void CosmicPreviewChronicle(float DelaySeconds);

	/** What a speed key does: 0 to 5 set the steps of CosmicUI::SpeedSteps through SetSpeed - a step the gate does not
	 * open does nothing (it never becomes x10, guideline 2.1) -, Space toggles the pause. The keys follow the gate alone:
	 * while the ascension presentation shows, key 5 sets x25 although the bar keeps its x25 button folded until the
	 * presentation closes, as keys 1 to 4 already set their speeds under it. The bound handler; tests call it in place of
	 * a key. */
	void PressSpeedKey(FKey Key);

	/** The speed of a number key: 0 to 5 are CosmicUI::SpeedSteps in order; any other key has none. */
	static TOptional<float> SpeedOfKey(const FKey& Key);

	/**
	 * Follows the third rung's gate (IsSpiritualSightOpen), from this controller's handlers of OnSimTick and
	 * OnGalaxyGenerated: when the gate closes - a deposition, a new galaxy, a lower save - a running speed above x10 falls
	 * to x10 (CosmicUI::LowersSpeed). The first look only notes the gate, so a speed above x10 the gate never opened (the
	 * development argument -Speed=25) stays. Tests call it directly too.
	 */
	void FollowSpeedGate();

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
	 * open by CosmicSpiritualSight. The third rung's one gate: x25 opens with it (CosmicUI::IsSpeedOpen). */
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
	 * project_settle on the wild SystemID). On success notes the first project in the hint progress (the system panel's hint
	 * line goes - MVP item 6), redraws the HUD and asks the map to redraw. */
	bool StartProject(FName SystemID, FName RowName);

	/** Selects SystemID and glides the camera to it, as a double click on its star does (the log feed's lines too). False -
	 * and the camera stays - when the map holds no such star or SelectSystem refused it (section 8.1: the camera goes to a
	 * system the viewer is shown only - IsShown: a known system, or the sanctuary by its name). bByViewer false (the
	 * automatic crisis focus, MVP item 6): neither the selection nor the camera's level is noted in the hint progress; the
	 * level is announced as a rebuild announces its framing. */
	bool SelectAndFocus(FName SystemID, bool bByViewer = true);

	/** Development aid: selects and focuses a system as a double click on it would ("CosmicSelect sys_0" at the console or
	 * in -ExecCmds; "CosmicSelect sol" takes the sanctuary, always shown by its name). Waits for a map not built yet; on a
	 * built map a system the viewer is not shown is refused with one log line (CosmicSeeAll 1 draws every system) and
	 * nothing waits. */
	UFUNCTION(Exec)
	void CosmicSelect(const FString& SystemID);

	/** The viewer's knowledge as the screens read it: CosmicMapLook::KnownViewOf of the state, the simulation's viewer
	 * tier and CosmicSeeAll; an empty view without a simulation. */
	FCosmicKnownView GetKnownView() const;
	/** The view a widget reads: its owning controller's; Knowing alone (no see-all) without one; an empty view without a
	 * simulation. */
	static FCosmicKnownView KnownViewFor(const UUserWidget& Widget);

	/**
	 * The stars of Map a click may take, each projected by Project (a world point to the screen; false when it does not
	 * reach it): every star of a system View knows - the star at index i draws system i - and never a backdrop point,
	 * an unknown system's among them (section 6.4). The same answer the selection reads (SelectSystem asks the live view
	 * too). A star's drawn radius is the distance between its projected centre and its centre moved one star radius along
	 * Side (the view's sideways axis). PickAtCursor projects through this controller's view; tests pass their own projector.
	 */
	static TArray<FCosmicPickCandidate> PickCandidatesOf(const ACosmicGalaxyMapActor& Map, const FCosmicKnownView& View, TFunctionRef<bool(const FVector&, FVector2D&)> Project, const FVector& Side);

	/**
	 * The named galaxies of Map a click may take, each projected by Project (a world point to the screen; false when it does
	 * not reach it): every mark of the universe, its drawn radius the distance between its projected centre and its centre
	 * moved CosmicUniverseLook::RingRadiusOf along Side - none while the map draws the region's detail (PickCandidatesOf
	 * takes the stars there), and never a scenery galaxy (section 5.11). PickGalaxyAtCursor projects through this
	 * controller's view; tests pass their own projector.
	 */
	static TArray<FCosmicGalaxyPickCandidate> GalaxyPickCandidatesOf(const ACosmicGalaxyMapActor& Map, TFunctionRef<bool(const FVector&, FVector2D&)> Project, const FVector& Side);

	/** Development aid: turns the wheel by Steps as ACosmicCameraPawn::Zoom does (positive nearer) - "CosmicWheel -100" the
	 * universe's far stop, "CosmicWheel -3" from the region's framing to the universe's near stop, "CosmicWheel -2" to the
	 * region's far stop (off-screen screenshots and the frame-time measure: -ExecCmds cannot scroll). Waits for a map not
	 * built yet. Changes no state. Not named CosmicZoom: a member of that name would hide the namespace CosmicZoom here. */
	UFUNCTION(Exec)
	void CosmicWheel(float Steps);

	/** Development aid: selects and focuses a named galaxy as a double click on it would ("CosmicSelectGalaxy
	 * gal_andromeda"). Waits for a map not built yet; on a built map a row the universe does not draw is refused with one
	 * log line. Changes no state. */
	UFUNCTION(Exec)
	void CosmicSelectGalaxy(const FString& Row);

	/** Development aid: "CosmicSeeAll 1" draws every system as known and every people as shown for the session - the map,
	 * the system panel and the selection (off-screen screenshots of places the viewer has not reached, the frame-time measure
	 * with sixty stars); 0 gives knowledge its say back and lets go of a selected system the viewer does not know. Writes
	 * nothing: Knowing, the state and both hashes stay. */
	UFUNCTION(Exec)
	void CosmicSeeAll(int32 On);
	bool IsSeeingAll() const { return bSeeAll; }

	/** The left mouse button, as bound in SetupInputComponent (tests call them in place of the mouse). A press starts a
	 * click or a pan; a release that ends a press which did not become a pan is a click - the star under the cursor, or
	 * empty space (clears) -, except the release that ends a double click (section 6.4: a double click selects and focuses;
	 * the button-up after it is no second click); a double click selects and focuses the star under the cursor. */
	void HandleLeftPressed(FKey Key);
	void HandleLeftReleased(FKey Key);
	void HandleLeftDoubleClick(FKey Key);

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

	/** Development aid: "CosmicSpiritualSight 1" forces the third rung's gate open at any rung for the session - the
	 * spiritual sight and x25 (the off-screen screenshot, the frame-time measure) - and logs the first system of each
	 * fallen sphere for CosmicSelect (a sphere system the viewer does not know needs CosmicSeeAll 1 before CosmicSelect);
	 * 0 gives the rung its say back. Changes no state, keeps nothing. */
	UFUNCTION(Exec)
	void CosmicSpiritualSight(int32 Open);

	/** Development aid: the system panel's command from the console ("CosmicStartProject <the home's SystemID>
	 * project_dyson" at the console or in -ExecCmds - the home is sys_0) - StartProject,
	 * the path a button takes; it changes the world only as that click would. */
	UFUNCTION(Exec)
	void CosmicStartProject(const FString& SystemID, const FString& RowName);

protected:
	// Reads the viewer's clock unit before the HUD exists. A development aid: -TimeUnit=Year (or Cycle) sets it for the
	// session without keeping it (screenshots).
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

private:
	UFUNCTION()
	void HandleSimTick(int64 Tick);

	UFUNCTION()
	void HandleGalaxyGenerated(int32 Seed);

	// A told line: the automatic pause looks at it.
	UFUNCTION()
	void HandleLogEntry(const FCosmicLogEntry& Entry);

	// SetSpeed with the reason it is set for (None: the player's, or the presentation's give-back).
	void ApplySpeed(float Speed, ECosmicAutoPause Reason);

	void HandleRightPressed(FKey Key);
	void HandleRightReleased(FKey Key);
	void HandleWheel(FKey Key);
	void HandleMapRebuilt();

	// The system under the cursor (CosmicMapPick::NearestOnScreen over PickCandidatesOf through this controller's view and
	// the live GetKnownView), or None.
	FName PickAtCursor() const;
	// The named galaxy under the cursor (CosmicMapPick::NearestOnScreen over GalaxyPickCandidatesOf through this view), or
	// None.
	FName PickGalaxyAtCursor() const;
	// The point of the disc plane under a screen position.
	bool DiscPointAt(const FVector2D& Screen, FVector& OutPoint) const;
	ACosmicCameraPawn* GetCameraPawn() const;
	// The one log line of a refused CosmicSelect: an ID this galaxy does not hold, or a system the viewer does not know.
	void LogRefusedSelect(FName Wanted) const;

	// Speed to return to when the pause is released.
	float SpeedBeforePause = 1.f;

	// The automatic pause: the viewer's choice, why it last stopped the simulation (None once the speed was set again),
	// and the hazards on the ruled people's systems at the last FollowOwnHazards (unset before the first, after a new
	// galaxy or a load).
	bool bAutoPause = true;
	ECosmicAutoPause AutoPauseReason = ECosmicAutoPause::None;
	TOptional<TMap<FName, FGameplayTag>> OwnHazardsSeen;

	// Whether the third rung's gate was open at the last FollowSpeedGate; unset before the first.
	TOptional<bool> ThirdRungWasOpen;

	// The provisional end (MVP item 4): presentation state, never saved, outside both hashes.
	ECosmicProvisionalEnd RunEnd = ECosmicProvisionalEnd::None;
	bool bProvisionalEnd = true;
	// Real seconds left before the chronicle opens; unset while nothing waits.
	TOptional<float> ChronicleWait;
	// The CosmicEnd line's two numbers: UWorld::GetRealTimeSeconds at this controller's BeginPlay or at the last new galaxy or
	// load (HandleGalaxyGenerated), and the real seconds the simulation stood still since then (any reason - a card, a
	// presentation, a pause; PlayerTick counts).
	double RunBeganRealSeconds = 0.0;
	double StoppedRealSeconds = 0.0;
	// The tick at which those seconds began (MVP item 5: a continued run's line says since which tick); -1 before BeginPlay.
	int64 RunBeganTick = -1;
	// A failed write of the game's saving stops the time once (UCosmicSaveSubsystem::TakeFirstFailure, decision 12).
	void FollowSaveFailure();
	// Closing the window (its X, Alt+F4) is the player's quit: the exit save. The close removes the local player and
	// destroys this controller - EndPlay(Destroyed) - a frame before the engine quits (UGameEngine::OnGameWindowClosed ->
	// UGameViewportClient::CloseRequested; UGameEngine::Tick -> CleanupGameViewport -> RequestExit), so EndPlay(Quit)
	// never sees it. Bound in BeginPlay to the game viewport's OnCloseRequested, removed in EndPlay.
	void HandleCloseRequested(FViewport* Viewport);
	TWeakObjectPtr<UGameViewportClient> ClosingViewport;
	FDelegateHandle CloseRequestedHandle;

	ECosmicTimeUnit TimeUnit = ECosmicTimeUnit::Cycle;

	ECosmicViewMode WantedViewMode = ECosmicViewMode::Spiritual;
	bool bSightForcedOpen = false;
	// Whether the sight was open at the last FollowSightOpening; unset until the first.
	TOptional<bool> SightWasOpen;

	FName SelectedSystemID;
	// CosmicSelect given before the map was built.
	FName PendingSelect;
	FName SelectedGalaxyRow;
	// CosmicSelectGalaxy and CosmicWheel given before the map was built.
	FName PendingGalaxy;
	TOptional<float> PendingWheel;
	// The level at the last FollowZoomLevel; unset before the first.
	TOptional<ECosmicZoomLevel> LastLevel;
	FCosmicHintProgress Hints;
	// The development aid CosmicSeeAll (handed to the map; read by GetKnownView).
	bool bSeeAll = false;

	TWeakObjectPtr<ACosmicGalaxyMapActor> Map;
	FDelegateHandle RebuiltHandle;

	// The mouse: a left press becomes a pan once it moves 6 px; each frame of a pan or an orbit moves by the mouse's
	// movement since the last frame.
	bool bLeftDown = false;
	bool bPanning = false;
	// Set by a double click: the release that ends it is no click (it would run the click again through a view that is
	// already gliding, and clear or move the selection the double click just made). The next release consumes it.
	bool bSwallowRelease = false;
	FVector2D LeftPressAt = FVector2D::ZeroVector;
	bool bRightDown = false;
	FVector2D LastMouse = FVector2D::ZeroVector;
};

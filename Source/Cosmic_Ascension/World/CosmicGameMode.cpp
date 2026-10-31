// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicGameMode.h"

#include "Blueprint/UserWidget.h"
#include "Components/InputComponent.h"
#include "CoreGlobals.h"
#include "Core/CosmicTags.h"
#include "Core/GalaxyState.h"
#include "Cosmic_Ascension.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "GeneralProjectSettings.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProperties.h"
#include "Log/CosmicChronicle.h"
#include "Log/CosmicLogSubsystem.h"
#include "Math/RotationMatrix.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Events/CosmicPetitions.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicEndings.h"
#include "Sim/CosmicResearch.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicSuccession.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/Tuple.h"
#include "TimerManager.h"
#include "UI/CosmicAscensionOverlayWidget.h"
#include "UI/CosmicCallingPagesWidget.h"
#include "UI/CosmicChronicleWidget.h"
#include "UI/CosmicEdictWidget.h"
#include "UI/CosmicGalaxyPanelWidget.h"
#include "UI/CosmicLogFeedWidget.h"
#include "UI/CosmicPetitionCardWidget.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "Widgets/Layout/Anchors.h"
#include "World/CosmicCameraPawn.h"
#include "World/CosmicDigest.h"
#include "World/CosmicGalaxyMapActor.h"
#include "World/CosmicHashRun.h"
#include "World/CosmicMapLook.h"
#include "World/CosmicMapPick.h"
#include "World/CosmicStarSystemActor.h"
#include "World/CosmicUniverseLook.h"
#include "World/CosmicZoomLevels.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicControlImpl
{
	// A star is picked within this many pixels of the cursor, or within its own drawn radius when larger.
	constexpr float PickPixels = 24.f;
	// A left press that moves further than this is a pan, not a click.
	constexpr float DragPixels = 6.f;
	constexpr float OrbitDegreesPerPixel = 0.25f;

	// The places of the panels on the screen (slate units).
	constexpr float TopBarHeight = 40.f;
	const FVector2D SystemPanelSize(340.f, 460.f);
	const FVector2D LogPanelSize(620.f, 230.f);
	// The petition card's area, in the middle of the screen: the card sits at its top and the rest lets clicks through.
	const FVector2D PetitionCardSize(560.f, 480.f);
	constexpr float ScreenMargin = 16.f;
	// The edict window's area (B2): the system panel's width, down to just above the log feed (the screen is about 1080 slate
	// units high whatever its pixels - the DPI scale follows the short side); its box is as tall as its page.
	const FVector2D EdictWindowSize(340.f, 760.f);

	// The seed of a galaxy no -Seed= names: the clock's (StartPlay, and every new run of the chronicle's 'new run').
	int32 ClockSeed()
	{
		return static_cast<int32>(FDateTime::UtcNow().GetTicks() & 0x7fffffff);
	}

	// The commit Tools/package.ps1 wrote into the packaged project folder (MVP item 9): the line "commit=<short>" of
	// <ProjectDir>/BuildInfo.txt (<build folder>/Windows/Cosmic_Ascension/ in a package), "-" without the file (an
	// editor run).
	FString BuildCommitOf()
	{
		FString Text;
		if (FFileHelper::LoadFileToString(Text, *(FPaths::ProjectDir() / TEXT("BuildInfo.txt"))))
		{
			TArray<FString> Lines;
			Text.ParseIntoArrayLines(Lines);
			for (const FString& Line : Lines)
			{
				if (Line.StartsWith(TEXT("commit=")))
				{
					return Line.Mid(7).TrimStartAndEnd();
				}
			}
		}
		return TEXT("-");
	}

	// The viewer's clock unit (guideline 6.3) is kept in the viewer's GameUserSettings ini (the controller's
	// TimeUnitSection and TimeUnitKey), not in a galaxy's save.
	const TCHAR* NameOf(ECosmicTimeUnit Unit)
	{
		return Unit == ECosmicTimeUnit::Year ? TEXT("Year") : TEXT("Cycle");
	}

	// Year or Cycle; anything else is no unit.
	bool ParseUnit(const FString& Text, ECosmicTimeUnit& OutUnit)
	{
		if (Text.Equals(TEXT("Year"), ESearchCase::IgnoreCase))
		{
			OutUnit = ECosmicTimeUnit::Year;
			return true;
		}
		if (Text.Equals(TEXT("Cycle"), ESearchCase::IgnoreCase))
		{
			OutUnit = ECosmicTimeUnit::Cycle;
			return true;
		}
		return false;
	}
}

ACosmicGameMode::ACosmicGameMode()
{
	HUDClass = ACosmicHUD::StaticClass();
	PlayerControllerClass = ACosmicPlayerController::StaticClass();
	// The camera pawn is spawned in StartPlay: L_Galaxy has no PlayerStart for a default pawn.
	DefaultPawnClass = nullptr;
}

void ACosmicGameMode::StartPlay()
{
	// Before BeginPlay reaches the HUD, so the panel opens on a galaxy.
	UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	UCosmicSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>() : nullptr;

	// MVP item 9: which build this is - the project version, the commit package.ps1 wrote into the packaged project folder,
	// and whether the data is cooked (the configuration is LogInit's 'Build Configuration' line).
	UE_LOG(LogCosmic, Display, TEXT("CosmicBuild: version %s, commit %s, %s"), *GetDefault<UGeneralProjectSettings>()->ProjectVersion,
		*CosmicControlImpl::BuildCommitOf(), FPlatformProperties::RequiresCookedData() ? TEXT("cooked") : TEXT("editor"));

	// MVP item 9: a hash run instead of a game (World/CosmicHashRun). Before BeginSession, so no continue reads a slot.
	// StartPlay runs inside FEngineLoop::Init (GEngine->Start -> LoadMap), so after the exit request the engine loop never
	// ticks; Super::StartPlay is not called, so no actor begins play (and no EndPlay exit save follows).
	const CosmicHashRun::FSwitch HashSwitch = CosmicHashRun::SwitchOf(FCommandLine::Get());
	if (HashSwitch.Kind != CosmicHashRun::ESwitch::Absent || HashSwitch.EveryKind != CosmicHashRun::ESwitch::Absent || HashSwitch.bPolicy)
	{
		int32 HashSeed = 0;
		UCosmicLogSubsystem* HashLog = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>() : nullptr;
		UPlayerRoleSubsystem* HashRole = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
		if (HashSwitch.Kind == CosmicHashRun::ESwitch::Absent)
		{
			UE_LOG(LogCosmic, Error, TEXT("CosmicHash: refused - no -CosmicHashRun="));
		}
		else if (HashSwitch.Kind == CosmicHashRun::ESwitch::Bad)
		{
			UE_LOG(LogCosmic, Error, TEXT("CosmicHash: refused - bad -CosmicHashRun=%s"), *HashSwitch.Text);
		}
		else if (!HashSwitch.IsGood())
		{
			UE_LOG(LogCosmic, Error, TEXT("CosmicHash: refused - bad -CosmicHashEvery=%s"), *HashSwitch.EveryText);
		}
		else if (!FParse::Value(FCommandLine::Get(), TEXT("Seed="), HashSeed))
		{
			UE_LOG(LogCosmic, Error, TEXT("CosmicHash: refused - no -Seed="));
		}
		else if (!Save || Save->IsGameSavingOn())
		{
			UE_LOG(LogCosmic, Error, TEXT("CosmicHash: refused - the game's saving is on"));
		}
		else if (!Sim || !HashLog || !HashRole)
		{
			UE_LOG(LogCosmic, Error, TEXT("CosmicHash: refused - no simulation"));
		}
		else
		{
			const CosmicHashRun::FResult Result = CosmicHashRun::Run(*Sim, *HashLog, *HashRole, HashSeed, HashSwitch);
			for (const FString& Checkpoint : Result.Checkpoints)
			{
				UE_LOG(LogCosmic, Display, TEXT("%s"), *Checkpoint);
			}
			if (Result.Outcome == CosmicHashRun::EOutcome::Ran)
			{
				UE_LOG(LogCosmic, Display, TEXT("%s"), *CosmicHashRun::LineOf(Result));
			}
			else
			{
				UE_LOG(LogCosmic, Error, TEXT("%s"), *CosmicHashRun::LineOf(Result));
			}
		}
		FPlatformMisc::RequestExitWithStatus(false, 0, TEXT("CosmicHashRun"));
		return;
	}

	// The continue (MVP item 5) before Super::StartPlay: no handler is bound yet, so a continued run that ended stands ended
	// through the two BeginPlay calls (FollowEnd(false) in each - headers/A4 4.4 (5)); the load's OnGalaxyGenerated reaches
	// nobody.
	if (Sim && (!Sim->GetState().GalaxyID.IsValid() || (Save && Save->ConsumeContinueAtStart())))
	{
		const ECosmicContinue Start = Save ? Save->BeginSession() : ECosmicContinue::Off;
		if (Start != ECosmicContinue::Newest && Start != ECosmicContinue::Backup)
		{
			// The seed of a new galaxy is an input to the simulation, not a draw from its stream. The counts come from the
			// home galaxy row (section 3.1).
			int32 Seed = 0;
			if (!FParse::Value(FCommandLine::Get(), TEXT("Seed="), Seed))
			{
				Seed = CosmicControlImpl::ClockSeed();
			}
			Sim->NewGalaxy(Seed, 0, 0);
			if (Save && Start == ECosmicContinue::NoSlot)
			{
				// On the disk from tick 0. While the gate is closed (Off) nothing is written.
				Save->RequestSave(ECosmicSaveCause::NewRun);
			}
			else if (Save && Start == ECosmicContinue::Unreadable)
			{
				// Decision 6: this run is first written at a tick boundary; a quit before that skips its exit save, so the
				// next start reads the run that did not open once more.
				Save->HoldStartRun();
			}
		}
		// A development aid: -Speed=N starts a new galaxy and a continued run at that speed (a continued run then runs
		// instead of standing still), up to x25 whatever the rung (the frame-time measure: item 6 at 10x, item 12 at 25x -
		// the controller lowers only a x25 the gate opened and then closed).
		float Speed = 1.f;
		FParse::Value(FCommandLine::Get(), TEXT("Speed="), Speed);
		Sim->SetTimeScale(Speed);
	}

	Super::StartPlay();

	if (UWorld* World = GetWorld())
	{
		// Development aids (see CLAUDE.md on looking at the screen without a window): -ShotAfterSeconds=N takes one
		// screenshot N seconds in (the first frame of an off-screen run still compiles shaders); -ProfileAfterSeconds=N
		// records 600 frames of the CSV profiler from N seconds in (Saved/Profiling/CSV - the frame time of item 6).
		auto After = [World](const TCHAR* Param, const TCHAR* Command)
		{
			float Seconds = 0.f;
			if (FParse::Value(FCommandLine::Get(), Param, Seconds) && Seconds > 0.f)
			{
				FTimerHandle Handle;
				World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(World, [World, Command]()
				{
					// Typed as at the console: the player's console reaches the viewport that handles "shot".
					if (APlayerController* Controller = World->GetFirstPlayerController())
					{
						Controller->ConsoleCommand(Command);
					}
				}), Seconds, false);
			}
		};
		After(TEXT("ShotAfterSeconds="), TEXT("shot showui"));
		After(TEXT("ProfileAfterSeconds="), TEXT("CsvProfile Frames=600"));
		// Phase 3 item 9: -WheelAfterSeconds=N turns the wheel one notch away N seconds in - from the region's far stop
		// ("CosmicWheel -2" in -ExecCmds) the crossing into the universe falls inside a profiled run.
		After(TEXT("WheelAfterSeconds="), TEXT("CosmicWheel -1"));
		// The profiled run says at what speed it ran: the tick and the time scale when the profile starts and ten seconds
		// later (600 frames at 60 fps), so the log shows that a -Speed=25 run kept x25 (item 12).
		float ProfileSeconds = 0.f;
		if (FParse::Value(FCommandLine::Get(), TEXT("ProfileAfterSeconds="), ProfileSeconds) && ProfileSeconds > 0.f)
		{
			for (const float At : { ProfileSeconds, ProfileSeconds + 10.f })
			{
				FTimerHandle Handle;
				World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(World, [World, At]()
				{
					const UGalaxySimulationSubsystem* Running = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
					if (Running)
					{
						UE_LOG(LogCosmic, Display, TEXT("CosmicProfile: %.1f s in, tick %lld, time scale %.1f"), At, Running->GetState().Tick, Running->GetTimeScale());
					}
				}), At, false);
			}
		}

		// The map reads the state from here on; each player looks at it through a camera pawn.
		ACosmicGalaxyMapActor* Map = World->SpawnActor<ACosmicGalaxyMapActor>();
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* Controller = It->Get();
			if (!Controller)
			{
				continue;
			}
			if (ACosmicCameraPawn* CameraPawn = World->SpawnActor<ACosmicCameraPawn>())
			{
				// Possessing makes the pawn's camera the view.
				Controller->Possess(CameraPawn);
			}
			if (ACosmicPlayerController* Cosmic = Cast<ACosmicPlayerController>(Controller))
			{
				Cosmic->UseMap(Map);
			}
		}
	}
}

void ACosmicHUD::BeginPlay()
{
	using namespace CosmicControlImpl;

	Super::BeginPlay();

	APlayerController* Controller = GetOwningPlayerController();
	if (!Controller)
	{
		return;
	}
	// The log feed at the bottom left.
	Feed = CreateWidget<UCosmicLogFeedWidget>(Controller);
	if (Feed)
	{
		// Position and size first: both reset the anchors to the top left (UGameViewportSubsystem), so the anchors come last.
		Feed->AddToViewport();
		Feed->SetPositionInViewport(FVector2D(ScreenMargin, -ScreenMargin), false);
		Feed->SetDesiredSizeInViewport(LogPanelSize);
		Feed->SetAnchorsInViewport(FAnchors(0.f, 1.f));
		Feed->SetAlignmentInViewport(FVector2D(0.f, 1.f));
	}
	// Along the top, the full width (a horizontal stretch: the size's X is the right inset).
	TopBar = CreateWidget<UCosmicTopBarWidget>(Controller);
	if (TopBar)
	{
		TopBar->AddToViewport(1);
		TopBar->SetPositionInViewport(FVector2D::ZeroVector, false);
		TopBar->SetDesiredSizeInViewport(FVector2D(0.f, TopBarHeight));
		TopBar->SetAnchorsInViewport(FAnchors(0.f, 0.f, 1.f, 0.f));
	}
	// On the right under the bar; it follows the controller's selection.
	SystemPanel = CreateWidget<UCosmicSystemPanelWidget>(Controller);
	if (SystemPanel)
	{
		SystemPanel->AddToViewport(1);
		SystemPanel->SetPositionInViewport(FVector2D(-ScreenMargin, TopBarHeight + ScreenMargin), false);
		SystemPanel->SetDesiredSizeInViewport(SystemPanelSize);
		SystemPanel->SetAnchorsInViewport(FAnchors(1.f, 0.f));
		SystemPanel->SetAlignmentInViewport(FVector2D(1.f, 0.f));
	}
	// In the system panel's place, while no system is selected: the galaxy the viewer is in (section 6.4).
	GalaxyPanel = CreateWidget<UCosmicGalaxyPanelWidget>(Controller);
	if (GalaxyPanel)
	{
		// Position and size first, the anchors last (as above).
		GalaxyPanel->AddToViewport(1);
		GalaxyPanel->SetPositionInViewport(FVector2D(-ScreenMargin, TopBarHeight + ScreenMargin), false);
		GalaxyPanel->SetDesiredSizeInViewport(SystemPanelSize);
		GalaxyPanel->SetAnchorsInViewport(FAnchors(1.f, 0.f));
		GalaxyPanel->SetAlignmentInViewport(FVector2D(1.f, 0.f));
	}
	if (ACosmicPlayerController* Cosmic = Cast<ACosmicPlayerController>(Controller))
	{
		// The right side follows both selections and the camera's level (CosmicUniverseLook::PanelOf).
		SelectionHandle = Cosmic->OnSelectionChanged.AddUObject(this, &ACosmicHUD::HandleSelectionChanged);
		GalaxySelectionHandle = Cosmic->OnGalaxySelectionChanged.AddUObject(this, &ACosmicHUD::HandleSelectionChanged);
		ZoomLevelHandle = Cosmic->OnZoomLevelChanged.AddUObject(this, &ACosmicHUD::ShowRightSide);
		ShowRightSide();
	}
	if (ACosmicPlayerController* Cosmic = Cast<ACosmicPlayerController>(Controller))
	{
		TimeUnitHandle = Cosmic->OnTimeUnitChanged.AddUObject(this, &ACosmicHUD::HandleTimeUnitChanged);
		ViewModeHandle = Cosmic->OnViewModeChanged.AddUObject(this, &ACosmicHUD::HandleViewModeChanged);
	}
	// The edict window on the left under the bar (B2), over the panels (1) and under the card (5); collapsed until it is
	// opened. Position and size first, the anchors last (as above).
	EdictWindow = CreateWidget<UCosmicEdictWidget>(Controller);
	if (EdictWindow)
	{
		EdictWindow->AddToViewport(3);
		EdictWindow->SetPositionInViewport(FVector2D(ScreenMargin, TopBarHeight + ScreenMargin), false);
		EdictWindow->SetDesiredSizeInViewport(EdictWindowSize);
		EdictWindow->SetAnchorsInViewport(FAnchors(0.f, 0.f));
	}
	// The petition card in the middle, over the panels and under the ascension presentation; collapsed while no card
	// shows. Position and size first, the anchors last (as above).
	Card = CreateWidget<UCosmicPetitionCardWidget>(Controller);
	if (Card)
	{
		Card->AddToViewport(5);
		Card->SetPositionInViewport(FVector2D::ZeroVector, false);
		Card->SetDesiredSizeInViewport(PetitionCardSize);
		Card->SetAnchorsInViewport(FAnchors(0.5f, 0.5f));
		Card->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
		if (ACosmicPlayerController* Cosmic = Cast<ACosmicPlayerController>(Controller))
		{
			// A card that waits in the galaxy the game started with.
			Cosmic->FollowPetition(false);
		}
	}
	// Over everything, the whole screen; hidden until an ascension.
	Overlay = CreateWidget<UCosmicAscensionOverlayWidget>(Controller);
	if (Overlay)
	{
		Overlay->AddToViewport(10);
	}
	// Over the card (5) and the presentation (10), under the chronicle (20): the calling's pages, collapsed until a death or the
	// end of a reign (B5 [B-5c]).
	CallingPages = CreateWidget<UCosmicCallingPagesWidget>(Controller);
	if (CallingPages)
	{
		CallingPages->AddToViewport(15);
		CallingPages->SetVisibility(ESlateVisibility::Collapsed);
	}
	// Over the card (5) and the presentation (10): the chronicle, collapsed until the run ends (MVP item 4).
	Chronicle = CreateWidget<UCosmicChronicleWidget>(Controller);
	if (Chronicle)
	{
		Chronicle->AddToViewport(20);
		Chronicle->SetVisibility(ESlateVisibility::Collapsed);
	}
	// A galaxy that started ended (a load before StartPlay - item 5): the second of the two BeginPlay calls opens the page. One
	// that started in the interregnum (B5): the second opens its epitaph.
	if (ACosmicPlayerController* Cosmic = Cast<ACosmicPlayerController>(Controller))
	{
		Cosmic->FollowEnd(false);
		Cosmic->FollowInterregnumOnOpen();
	}
}

void ACosmicHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ACosmicPlayerController* Cosmic = Cast<ACosmicPlayerController>(GetOwningPlayerController()))
	{
		Cosmic->OnSelectionChanged.Remove(SelectionHandle);
		Cosmic->OnGalaxySelectionChanged.Remove(GalaxySelectionHandle);
		Cosmic->OnZoomLevelChanged.Remove(ZoomLevelHandle);
		Cosmic->OnTimeUnitChanged.Remove(TimeUnitHandle);
		Cosmic->OnViewModeChanged.Remove(ViewModeHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void ACosmicHUD::HandleSelectionChanged(FName SystemID)
{
	ShowRightSide();
}

void ACosmicHUD::ShowRightSide()
{
	const ACosmicPlayerController* Cosmic = Cast<ACosmicPlayerController>(GetOwningPlayerController());
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Cosmic || !Sim)
	{
		return;
	}
	// What the player looks at: the level of the camera's target arm and the two selections (never two panels).
	const ECosmicZoomLevel Level = Cosmic->GetZoomLevel();
	const FCosmicPanelChoice Choice = CosmicUniverseLook::PanelOf(Level, Cosmic->GetSelectedSystem(), Cosmic->GetSelectedGalaxy(), Sim->GetRules().HomeGalaxyRow);
	if (SystemPanel)
	{
		SystemPanel->ShowSystem(Choice.bSystemPanel ? Choice.Shown : NAME_None);
	}
	if (GalaxyPanel)
	{
		// The hint, then the galaxy (each redraws the panel; the definitions only).
		GalaxyPanel->ShowHint(CosmicUI::HintKeyOf(Level, Cosmic->GetHintProgress()));
		GalaxyPanel->ShowGalaxy(Choice.bSystemPanel ? NAME_None : Choice.Shown);
	}
}

void ACosmicHUD::HandleTimeUnitChanged()
{
	RefreshPanel();
	// The feed's lines are drawn once when they arrive; their stamps are redrawn here.
	if (Feed)
	{
		Feed->RedrawLines();
	}
}

void ACosmicHUD::HandleViewModeChanged()
{
	// The top bar's view button and the panel's spiritual line; the map was told by the controller.
	RefreshPanel();
}

void ACosmicHUD::RefreshPanel()
{
	// The feed follows the log through its own delegate; a command adds no line of its own.
	if (TopBar)
	{
		TopBar->Refresh();
	}
	// The right side: the system panel (one refresh) or the galaxy panel - it reads only definitions, shown again now (the
	// tables may have changed) - with its hint.
	ShowRightSide();
	// The card's choices: what the stockpile can pay and what their gates allow now (CosmicPetitions::ChoiceGateOf) - an edict
	// revoked in the edict window opens a greyed choice while the card waits.
	if (Card)
	{
		Card->Refresh();
	}
	// The edict window's rows and shares (it redraws nothing while closed).
	if (EdictWindow)
	{
		EdictWindow->Refresh();
	}
}

ACosmicPlayerController::ACosmicPlayerController()
{
	bShowMouseCursor = true;
}

void ACosmicPlayerController::PostInitializeComponents()
{
	using namespace CosmicControlImpl;

	Super::PostInitializeComponents();
	// Before the HUD is spawned, so the panels open in the viewer's unit. A value the code does not know reads as none.
	FString Kept;
	if (GConfig && GConfig->GetString(TimeUnitSection, TimeUnitKey, Kept, GGameUserSettingsIni))
	{
		ParseUnit(Kept, TimeUnit);
	}
	FString Given;
	if (FParse::Value(FCommandLine::Get(), TEXT("TimeUnit="), Given))
	{
		ParseUnit(Given, TimeUnit);
	}
	// The automatic pause: on unless the viewer turned it off; -AutoPause=0 or 1 for the session only.
	FString KeptPause;
	if (GConfig && GConfig->GetString(TimeUnitSection, AutoPauseKey, KeptPause, GGameUserSettingsIni))
	{
		bAutoPause = !KeptPause.Equals(TEXT("Off"), ESearchCase::IgnoreCase);
	}
	int32 GivenPause = 1;
	if (FParse::Value(FCommandLine::Get(), TEXT("AutoPause="), GivenPause))
	{
		bAutoPause = GivenPause != 0;
	}
	// The provisional end (MVP item 4): on; -ProvisionalEnd=0 turns it off for the session (the frame-time measure at x25,
	// a screen past tick 6,000).
	int32 GivenEnd = 1;
	if (FParse::Value(FCommandLine::Get(), TEXT("ProvisionalEnd="), GivenEnd))
	{
		bProvisionalEnd = GivenEnd != 0;
	}
}

void ACosmicPlayerController::SetTimeUnit(ECosmicTimeUnit Unit)
{
	if (Unit == TimeUnit)
	{
		return;
	}
	TimeUnit = Unit;
	OnTimeUnitChanged.Broadcast();
}

void ACosmicPlayerController::ToggleTimeUnit()
{
	using namespace CosmicControlImpl;

	SetTimeUnit(TimeUnit == ECosmicTimeUnit::Year ? ECosmicTimeUnit::Cycle : ECosmicTimeUnit::Year);
	// The viewer's choice outlives the session. A test run on the same machine reads it but never writes it.
	if (GConfig && !GIsAutomationTesting)
	{
		GConfig->SetString(TimeUnitSection, TimeUnitKey, NameOf(TimeUnit), GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);
	}
}

ECosmicTimeUnit ACosmicPlayerController::TimeUnitFor(const UUserWidget& Widget)
{
	const ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(Widget.GetOwningPlayer());
	return Controller ? Controller->GetTimeUnit() : ECosmicTimeUnit::Cycle;
}

void ACosmicPlayerController::SetWantedViewMode(ECosmicViewMode Mode)
{
	if (Mode == WantedViewMode)
	{
		return;
	}
	WantedViewMode = Mode;
	// The map only marks a refresh (it draws at most once a frame); the HUD redraws the bar and the panel.
	if (ACosmicGalaxyMapActor* TheMap = Map.Get())
	{
		TheMap->SetViewWanted(WantedViewMode, bSightForcedOpen);
	}
	OnViewModeChanged.Broadcast();
}

void ACosmicPlayerController::ToggleViewMode()
{
	// The only guard: a closed sight has no other view to turn to.
	if (!IsSpiritualSightOpen())
	{
		return;
	}
	SetWantedViewMode(WantedViewMode == ECosmicViewMode::Spiritual ? ECosmicViewMode::Physical : ECosmicViewMode::Spiritual);
}

bool ACosmicPlayerController::IsSpiritualSightOpen() const
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	return bSightForcedOpen || (Sim && CosmicMapLook::SpiritualSightOpens(Sim->GetViewerTier()));
}

ECosmicViewMode ACosmicPlayerController::GetShownViewMode() const
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	return CosmicMapLook::ShownViewMode(WantedViewMode, bSightForcedOpen, Sim ? Sim->GetViewerTier() : EAscensionTier::Tier1_Planetary);
}

void ACosmicPlayerController::FollowSightOpening()
{
	const bool bOpen = IsSpiritualSightOpen();
	// The sight opened again after it closed (a deposed ruler who climbed back, the force given again): the reveal of the
	// opening never shows over a physical map.
	if (bOpen && SightWasOpen.IsSet() && !SightWasOpen.GetValue())
	{
		SetWantedViewMode(ECosmicViewMode::Spiritual);
	}
	SightWasOpen = bOpen;
}

ECosmicViewMode ACosmicPlayerController::ShownViewModeFor(const UUserWidget& Widget)
{
	const ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(Widget.GetOwningPlayer());
	return Controller ? Controller->GetShownViewMode() : ECosmicViewMode::Physical;
}

bool ACosmicPlayerController::IsSpiritualSightOpenFor(const UUserWidget& Widget)
{
	const ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(Widget.GetOwningPlayer());
	return Controller && Controller->IsSpiritualSightOpen();
}

void ACosmicPlayerController::CosmicSpiritualSight(int32 Open)
{
	const bool bForce = Open != 0;
	if (bForce != bSightForcedOpen)
	{
		bSightForcedOpen = bForce;
		if (ACosmicGalaxyMapActor* TheMap = Map.Get())
		{
			TheMap->SetViewWanted(WantedViewMode, bSightForcedOpen);
		}
		// Forced open from closed is an opening like any other.
		FollowSightOpening();
		OnViewModeChanged.Broadcast();
	}
	if (!bForce)
	{
		return;
	}
	// The first system of each fallen sphere, for CosmicSelect in a screenshot's -ExecCmds.
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return;
	}
	const FGameplayTag Abyssal = FGameplayTag::RequestGameplayTag(FName(TEXT("Realm.Abyssal")), false);
	for (const FTranscendentState& Transcendent : Sim->GetState().Transcendents)
	{
		const FTranscendentDef* Row = Sim->GetTables().FindTranscendent(Transcendent.TranscendentID);
		if (Row && Row->Allegiance == Abyssal && !Transcendent.InfluencedSystems.IsEmpty())
		{
			UE_LOG(LogCosmic, Display, TEXT("CosmicSpiritualSight: %s holds %s"), *Transcendent.TranscendentID.ToString(), *Transcendent.InfluencedSystems[0].ToString());
		}
	}
}

void ACosmicPlayerController::BeginPlay()
{
	Super::BeginPlay();
	// The cursor stays visible while a button is held, and the viewport captures the mouse only during a press.
	if (IsLocalController())
	{
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(Mode);
	}
	// The third rung's gate and the hazards of the ruled people are followed at the tick boundary and on a new galaxy or a
	// load (FollowSpeedGate, FollowOwnHazards); the told lines as they come (the automatic pause).
	if (UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr)
	{
		Sim->OnSimTick.AddUniqueDynamic(this, &ACosmicPlayerController::HandleSimTick);
		Sim->OnGalaxyGenerated.AddUniqueDynamic(this, &ACosmicPlayerController::HandleGalaxyGenerated);
	}
	if (UCosmicLogSubsystem* Log = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>() : nullptr)
	{
		Log->OnLogEntry.AddUniqueDynamic(this, &ACosmicPlayerController::HandleLogEntry);
	}
	// The window's close is the player's quit (HandleCloseRequested).
	if (UGameViewportClient* Viewport = (IsLocalController() && GetWorld()) ? GetWorld()->GetGameViewport() : nullptr)
	{
		ClosingViewport = Viewport;
		CloseRequestedHandle = Viewport->OnCloseRequested().AddUObject(this, &ACosmicPlayerController::HandleCloseRequested);
	}
	RunBeganRealSeconds = GetWorld() ? GetWorld()->GetRealTimeSeconds() : 0.0;
	RunBeganTick = GetGameInstance() && GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>()->GetState().Tick : -1;
	StoppedRealSeconds = 0.0;
	// The game's first galaxy came before the handlers were bound (StartPlay): a galaxy that started ended latches here.
	FollowEnd(false);
	// A continued run that is not over stands still with its line (MVP item 5); one in the interregnum shows its epitaph again
	// (B5 - when the HUD's pages stand already).
	FollowStart();
	FollowInterregnumOnOpen();
	FollowReign(false);
	FollowOwnHazards();
}

void ACosmicPlayerController::FollowStart()
{
	UCosmicSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>() : nullptr;
	const ECosmicContinue Outcome = Save ? Save->TakeStartOutcome() : ECosmicContinue::NotBegun;
	float GivenSpeed = 0.f;
	if (IsRunEnded() || FParse::Value(FCommandLine::Get(), TEXT("Speed="), GivenSpeed))
	{
		return;   // an ended run keeps its own line (Chronicle.Why); a development speed runs
	}
	switch (Outcome)
	{
	case ECosmicContinue::Newest:
		ApplySpeed(0.f, ECosmicAutoPause::Continued);
		break;
	case ECosmicContinue::Backup:
		ApplySpeed(0.f, ECosmicAutoPause::ContinuedFromBackup);
		break;
	case ECosmicContinue::Unreadable:
		ApplySpeed(0.f, ECosmicAutoPause::SaveUnreadable);
		break;
	default:
		break;
	}
}

void ACosmicPlayerController::FollowSaveFailure()
{
	// Once per game instance (decision 12). Not on an ended run: the notice waits for the next run.
	UCosmicSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>() : nullptr;
	if (!IsRunEnded() && Save && Save->TakeFirstFailure())
	{
		ApplySpeed(0.f, ECosmicAutoPause::SaveFailed);
	}
}

void ACosmicPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Quitting the game saves the run - the console's quit and RequestExit end here as Quit; the window's close was saved
	// by HandleCloseRequested and ends here as Destroyed, as a map load does (no save): before the handlers go. The world
	// ends its actors before the game instance shuts down, so the state is still there (GameEngine.cpp).
	if (EndPlayReason == EEndPlayReason::Quit)
	{
		if (UCosmicSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>() : nullptr)
		{
			Save->RequestSave(ECosmicSaveCause::Exit);
		}
	}
	if (UGameViewportClient* Viewport = ClosingViewport.Get())
	{
		Viewport->OnCloseRequested().Remove(CloseRequestedHandle);
	}
	if (ACosmicGalaxyMapActor* TheMap = Map.Get())
	{
		TheMap->OnRebuilt.Remove(RebuiltHandle);
	}
	if (UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr)
	{
		Sim->OnSimTick.RemoveDynamic(this, &ACosmicPlayerController::HandleSimTick);
		Sim->OnGalaxyGenerated.RemoveDynamic(this, &ACosmicPlayerController::HandleGalaxyGenerated);
	}
	if (UCosmicLogSubsystem* Log = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>() : nullptr)
	{
		Log->OnLogEntry.RemoveDynamic(this, &ACosmicPlayerController::HandleLogEntry);
	}
	Super::EndPlay(EndPlayReason);
}

void ACosmicPlayerController::HandleCloseRequested(FViewport* Viewport)
{
	// From the window's message (Slate), outside the world's tick - no tick stage or broadcast runs - with the world and the
	// game instance whole. A quit that reaches EndPlay(Quit) too writes once: the exit skips a slot that holds this state.
	if (UCosmicSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>() : nullptr)
	{
		Save->RequestSave(ECosmicSaveCause::Exit);
	}
}

void ACosmicPlayerController::HandleSimTick(int64 Tick)
{
	FollowSpeedGate();
	// A death before the end: a life that closed on the end's tick is not lost behind the end's early return (B5).
	const bool bDied = FollowDeath(Tick);
	// An abdication or a deposition at this boundary: the reign end's page (a life that closed is the epitaph's - B5 [B-5c]).
	FollowReign(!bDied);
	// The end before the card and the hazards: once the run is over, its own tick asks for nothing.
	FollowEnd(true);
	if (IsRunEnded())
	{
		return;
	}
	FollowOwnHazards();
	FollowPetition(true);
	// A failed boundary write (the save layer's handler may come before or after this one - noticed now or at the next tick).
	FollowSaveFailure();
}

bool ACosmicPlayerController::LifeClosedAt(const FGalaxyState& State, int64 Tick)
{
	const TArray<FLifeRecord>& Lives = State.Calling.Lives;
	const int32 Closed = State.Calling.LifeState == ELifeState::Interregnum ? Lives.Num() - 1 : Lives.Num() - 2;
	return Lives.IsValidIndex(Closed) && Lives[Closed].DiedTick == Tick && Tick > 0;
}

bool ACosmicPlayerController::FollowDeath(int64 Tick)
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || !LifeClosedAt(Sim->GetState(), Tick))
	{
		return false;
	}
	DeathTick = Tick;
	ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
	// The card on the screen folds, an outcome shown on it too: the death emptied the waiting slot (header section 2.10).
	if (UCosmicPetitionCardWidget* Card = CosmicHUD ? CosmicHUD->GetPetitionCard() : nullptr)
	{
		Card->ShowCard(NAME_None, NAME_None);
	}
	// A death page never wears the ascension presentation (header section 4): one showing folds with no give-back and no chime.
	if (UCosmicAscensionOverlayWidget* Overlay = CosmicHUD ? CosmicHUD->GetAscensionOverlay() : nullptr)
	{
		Overlay->FoldSilently();
	}
	// The epitaph ([B-5c], header section 4): 'skip' from a second death on (docs/70). Opened before the stop, so that the redraw
	// greys the speed buttons.
	if (UCosmicCallingPagesWidget* Pages = CosmicHUD ? CosmicHUD->GetCallingPages() : nullptr)
	{
		const int32 LifeAt = CosmicDigest::LastClosedLifeOf(Sim->GetState());
		if (LifeAt != INDEX_NONE)
		{
			Pages->OpenEpitaph(LifeAt, LifeAt >= 1, false);
		}
	}
	ApplySpeed(0.f, ECosmicAutoPause::Death);
	return true;
}

bool ACosmicPlayerController::AdvanceLeap()
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || IsRunEnded() || Sim->GetState().Calling.LifeState != ELifeState::Interregnum)
	{
		return false;
	}
	const FGalaxyState& State = Sim->GetState();
	const int64 Tick = State.Tick;
	// The whole leap, never cut here (B6a header section 2.6): RunLeap stops at the tick that writes the ending record, and with
	// the end switched on that boundary latches it at its tick (FollowEnd).
	const int64 Ticks = State.Calling.PendingLeapTicks;
	if (Ticks <= 0)
	{
		// The silence with no end latched (a session with the end switched off): nothing to run - the time goes back to ordinary
		// play at the speed before the pause (once no calling page shows: ApplySpeed keeps it stopped under one, and the pages
		// give it back as they fold).
		ApplySpeed(SpeedBeforePause > 0.f ? SpeedBeforePause : 1.f, ECosmicAutoPause::None);
		return false;
	}
	const bool bRecordBefore = State.Ending.Kind.IsValid();
	Sim->RunLeap(Ticks);
	// A refused leap (from inside the stages or a boundary handler - one error line) ran nothing.
	const bool bRan = Sim->GetState().Tick != Tick;
	// With the end switched off nothing latched at the record's tick: the rest of the leap runs on, once - a record stops a leap
	// at most once (it is written once), so two calls cover it; never a loop on PendingLeapTicks (a refused leap runs nothing).
	if (bRan && !bProvisionalEnd && !bRecordBefore && Sim->GetState().Ending.Kind.IsValid()
		&& Sim->GetState().Calling.LifeState == ELifeState::Interregnum && Sim->GetState().Calling.PendingLeapTicks > 0)
	{
		Sim->RunLeap(Sim->GetState().Calling.PendingLeapTicks);
	}
	return bRan;
}

// Helpers of the calling's pages (B5 [B-5c]). A named namespace: the module is built as one unity file.
namespace CosmicCallingFlowImpl
{
	// A life after Lives[LifeAt] stands embodied: the succession at the leap's end, or a leap of 0's on the death's tick.
	bool NewLifeStands(const FGalaxyState& State, int32 LifeAt)
	{
		return State.Calling.LifeState == ELifeState::Embodied && State.Calling.Lives.Num() - 1 > LifeAt;
	}

	// The silence: the calling waits in the interregnum with no leap left (no candidate stood at the leap's end). The one
	// predicate in Sim (rule 13 - CosmicSuccession::IsSilent).
	bool IsSilent(const FGalaxyState& State)
	{
		return CosmicSuccession::IsSilent(State.Calling);
	}

	// The leap of the epitaph's 'next' and 'skip' (header section 2.6): AdvanceLeap. A leap that ended in the silence is an end
	// itself while the end is switched on (B6a header section 2.8 - Ending.NoCandidate latched on the succession's tick, so the
	// digest's 'next' opens the chronicle). With the end switched off the second call finds no leap and gives the time back,
	// which waits for the pages (ApplySpeed refuses a speed under a page): the digest's 'next' folds them and gives it back.
	void LeapOfThePages(ACosmicPlayerController& Controller, const FGalaxyState& State)
	{
		if (Controller.AdvanceLeap() && IsSilent(State) && !Controller.IsRunEnded())
		{
			Controller.AdvanceLeap();
		}
	}
}

bool ACosmicPlayerController::IsCallingPageOpen() const
{
	const ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
	const UCosmicCallingPagesWidget* Pages = CosmicHUD ? CosmicHUD->GetCallingPages() : nullptr;
	return Pages && Pages->IsOpen();
}

bool ACosmicPlayerController::IsCallingPageOpenFor(const UUserWidget& Widget)
{
	const ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(Widget.GetOwningPlayer());
	return Controller && Controller->IsCallingPageOpen();
}

bool ACosmicPlayerController::IsVesselShown() const
{
	// Under a calling page the screens wait for its fold (header section 4: the inauguration introduces the new vessel).
	if (IsCallingPageOpen())
	{
		return false;
	}
	// The vessel the end's tick installed: the chronicle's own rule of the lives it shows, read where the run ended.
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	const bool bNeverSeen = Sim && IsRunEnded() && CosmicChronicle::ShownLivesOf(Sim->GetState()) < Sim->GetState().Calling.Lives.Num();
	return !bNeverSeen;
}

bool ACosmicPlayerController::IsVesselShownFor(const UUserWidget& Widget)
{
	const ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(Widget.GetOwningPlayer());
	return !Controller || Controller->IsVesselShown();
}

void ACosmicPlayerController::RefreshVesselScreens()
{
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
	// The map draws at its next frame: a stopped simulation sends it no tick.
	if (ACosmicGalaxyMapActor* TheMap = Map.Get())
	{
		TheMap->RequestRefresh();
	}
}

void ACosmicPlayerController::NextCallingPage()
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
	UCosmicCallingPagesWidget* Pages = CosmicHUD ? CosmicHUD->GetCallingPages() : nullptr;
	// An input only: a handler of a Core Logic delegate or of OnSimTick turns no page (the leap would be refused there anyway).
	if (!Sim || !Pages || !Pages->IsOpen() || Sim->IsBroadcasting() || Sim->IsSendingSimTick())
	{
		return;
	}
	switch (Pages->GetPage())
	{
	case ECosmicCallingPage::Epitaph:
		// A death on the end's tick: no leap, no digest - the chronicle (header section 4).
		if (IsRunEnded())
		{
			FinishCallingPages();
			return;
		}
		// The leap and its one boundary (an end inside it latches at its tick - the silence's too, B6a), then the digest of what
		// the world did meanwhile - a leap of 0 too (its one line). In the silence of a session with the end switched off the
		// time waits for the digest's 'next' (LeapOfThePages).
		CosmicCallingFlowImpl::LeapOfThePages(*this, Sim->GetState());
		Pages->OpenDigest();
		CosmicHUD->RefreshPanel();
		return;
	case ECosmicCallingPage::Digest:
		// An end inside the interregnum: the chronicle, no inauguration (decision 15) - and an end on the succession's tick wins
		// over it (header section 2.6).
		if (IsRunEnded())
		{
			FinishCallingPages();
			return;
		}
		if (CosmicCallingFlowImpl::NewLifeStands(Sim->GetState(), Pages->GetLifeAt()))
		{
			Pages->OpenInauguration(Sim->GetState().Calling.Lives.Num() - 1);
			CosmicHUD->RefreshPanel();
			return;
		}
		// The silence with no end latched (the end switched off): no vessel stands, nothing to inaugurate - the pages fold and
		// the time goes back to ordinary play (AdvanceLeap, header section 2.6).
		FinishCallingPages();
		if (CosmicCallingFlowImpl::IsSilent(Sim->GetState()))
		{
			AdvanceLeap();
		}
		return;
	case ECosmicCallingPage::Inauguration:
		CloseInauguration();
		return;
	case ECosmicCallingPage::ReignEnd:
	case ECosmicCallingPage::None:
		break;
	}
	FinishCallingPages();
}

void ACosmicPlayerController::SkipCallingPages()
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
	UCosmicCallingPagesWidget* Pages = CosmicHUD ? CosmicHUD->GetCallingPages() : nullptr;
	if (!Sim || !Pages || Pages->GetPage() != ECosmicCallingPage::Epitaph || Sim->IsBroadcasting() || Sim->IsSendingSimTick())
	{
		return;
	}
	const int32 LifeAt = Pages->GetLifeAt();
	if (!IsRunEnded())
	{
		CosmicCallingFlowImpl::LeapOfThePages(*this, Sim->GetState());
	}
	if (!IsRunEnded() && CosmicCallingFlowImpl::NewLifeStands(Sim->GetState(), LifeAt))
	{
		CloseInauguration();
		return;
	}
	FinishCallingPages();
	// The silence with no end latched (the end switched off): the time goes back once the pages folded (AdvanceLeap).
	if (!IsRunEnded() && CosmicCallingFlowImpl::IsSilent(Sim->GetState()))
	{
		AdvanceLeap();
	}
}

void ACosmicPlayerController::CloseCallingPages()
{
	ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
	UCosmicCallingPagesWidget* Pages = CosmicHUD ? CosmicHUD->GetCallingPages() : nullptr;
	if (Pages && Pages->IsOpen())
	{
		Pages->Close();
		// The speed buttons come back, and the vessel the pages held back (IsVesselShown): the bar, the panel and the map (a
		// stopped simulation sends no tick to redraw them).
		RefreshVesselScreens();
	}
}

void ACosmicPlayerController::FinishCallingPages()
{
	CloseCallingPages();
	ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
	if (IsRunEnded())
	{
		// The pages stood in the place of the wait before the chronicle: it opens now, with its reveal.
		ChronicleWait.Reset();
		UCosmicChronicleWidget* Page = CosmicHUD ? CosmicHUD->GetChronicle() : nullptr;
		if (Page && !Page->IsOpen())
		{
			Page->Open(LatchedRecordOf(), false);
		}
	}
	if (CosmicHUD)
	{
		CosmicHUD->RefreshPanel();
	}
}

FCosmicEndingRecord ACosmicPlayerController::LatchedRecordOf() const
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	return Sim && IsRunEnded() ? Sim->GetState().Ending : FCosmicEndingRecord();
}

void ACosmicPlayerController::CloseInauguration()
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	const FName Home = Sim ? Sim->GetState().Player.CurrentSystemID : NAME_None;
	CloseCallingPages();
	// The new vessel's system on the map - not the viewer's selection (the crisis focus's form) - and the time stays stopped
	// with its line until the player sets a speed (decision 16: no speed given back before the vessel is found on the map).
	SelectAndFocus(Home, false);
	ApplySpeed(0.f, ECosmicAutoPause::Succession);
}

void ACosmicPlayerController::FollowReign(bool bArrived)
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	const bool bEmbodied = State.Calling.LifeState == ELifeState::Embodied;
	const bool bRuling = bEmbodied && State.Player.Role == EPlayerRole::Ruler;
	const int32 Lives = State.Calling.Lives.Num();
	const bool bWasRuling = bRulingSeen && RulingSeenLives == Lives;
	bRulingSeen = bRuling;
	RulingSeenLives = Lives;
	if (!bArrived || !bWasRuling || bRuling || !bEmbodied || Lives == 0)
	{
		return;
	}
	// Only a reign that closed with its reason: a role set by hand closes none.
	const FLifeRecord& Life = State.Calling.Lives.Last();
	if (Life.ReignEndReasons.IsEmpty() || !Life.ReignEndReasons.Last().IsValid())
	{
		return;
	}
	// The reign end's page never wears the ascension presentation (header section 4): one showing folds with no give-back.
	if (UCosmicAscensionOverlayWidget* Overlay = GetHUD<ACosmicHUD>() ? GetHUD<ACosmicHUD>()->GetAscensionOverlay() : nullptr)
	{
		Overlay->FoldSilently();
	}
	ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
	if (UCosmicCallingPagesWidget* Pages = CosmicHUD ? CosmicHUD->GetCallingPages() : nullptr)
	{
		Pages->OpenReignEnd(Lives - 1);
	}
	ApplySpeed(0.f, ECosmicAutoPause::ReignEnd);
}

void ACosmicPlayerController::FollowInterregnumOnOpen()
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
	UCosmicCallingPagesWidget* Pages = CosmicHUD ? CosmicHUD->GetCallingPages() : nullptr;
	if (!Sim || !Pages || Pages->IsOpen() || IsRunEnded())
	{
		return;
	}
	// A run that opens in the interregnum, or on the tick of a succession no tick has run since - the leap's boundary save, a
	// leap of 0's death, a quit while the digest or the inauguration showed: the player may not have been shown the new vessel.
	// Its 'next' finds no leap and goes on to the digest and the inauguration.
	const FGalaxyState& State = Sim->GetState();
	const TArray<FLifeRecord>& Lives = State.Calling.Lives;
	const bool bWaiting = State.Calling.LifeState == ELifeState::Interregnum;
	const bool bJustCalled = State.Calling.LifeState == ELifeState::Embodied && Lives.Num() > 1 && Lives.Last().BornTick == State.Tick;
	if (!bWaiting && !bJustCalled)
	{
		return;
	}
	const int32 LifeAt = CosmicDigest::LastClosedLifeOf(State);
	if (LifeAt == INDEX_NONE)
	{
		return;
	}
	Pages->OpenEpitaph(LifeAt, LifeAt >= 1, true);
	// A running time stops with the death's line; a stopped one keeps a reason of its own (a continue's) and otherwise takes
	// the death's - the bar never stands still without saying why.
	if (Sim->GetTimeScale() > 0.f || AutoPauseReason == ECosmicAutoPause::None)
	{
		ApplySpeed(0.f, ECosmicAutoPause::Death);
		return;
	}
	CosmicHUD->RefreshPanel();
}

void ACosmicPlayerController::HandleGalaxyGenerated(int32 Seed)
{
	FollowSpeedGate();
	DeathTick = -1;
	// Another game: the end, what stopped the old one and the hazards it had say nothing of this one. An edict hint the old
	// run raised and nobody answered waits for this run's first fall (B2 - one that ended stays ended for the session).
	ClearRunEnd();
	AutoPauseReason = ECosmicAutoPause::None;
	OwnHazardsSeen.Reset();
	LethalSeen.Reset();
	if (!Hints.bEdictHintDone)
	{
		Hints.bEdictHintOn = false;
	}
	RunBeganRealSeconds = GetWorld() ? GetWorld()->GetRealTimeSeconds() : 0.0;
	RunBeganTick = GetGameInstance() && GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>()->GetState().Tick : -1;
	StoppedRealSeconds = 0.0;
	// The role the reign end's page compares with is this run's (B5).
	FollowReign(false);
	FollowEnd(false);
	if (IsRunEnded())
	{
		return;
	}
	FollowOwnHazards();
	FollowPetition(false);
	// A load in the interregnum: its epitaph again (B5 - ClearRunEnd folded the pages of the old run).
	FollowInterregnumOnOpen();
}

void ACosmicPlayerController::FollowEnd(bool bArrived)
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!bProvisionalEnd || !Sim)
	{
		return;
	}
	ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
	if (!IsRunEnded())
	{
		// The record the tick wrote after its tenth stage (B6a header section 2.7) - never the predicate afresh: a state with no
		// record goes on (a save from before the field, decision 2-da, until its next tick writes one).
		const FCosmicEndingRecord Ending = Sim->GetState().Ending;
		if (!Ending.Kind.IsValid())
		{
			return;
		}
		RunEnd = Ending.Kind;
		ApplySpeed(0.f, ECosmicAutoPause::RunEnd);
		// The screens that hold back what the end's tick installed (the feed's inauguration line - [B-5d1]) redraw now.
		OnRunEndChanged.Broadcast();
		// The card folds; the waiting row stays in Knowing.
		if (UCosmicPetitionCardWidget* Card = CosmicHUD ? CosmicHUD->GetPetitionCard() : nullptr)
		{
			Card->ShowCard(NAME_None, NAME_None);
		}
		const double Now = GetWorld() ? GetWorld()->GetRealTimeSeconds() : 0.0;
		// The end's tick is the record's (a state run on past it, a continued save): the clock end still reads 'CosmicEnd: Clock at tick 6000; ...'.
		const int64 EndTick = Ending.Tick;
		if (!bArrived && (RunBeganTick < 0 || RunBeganTick == Sim->GetState().Tick))
		{
			// A run that was over when it was opened (a continue, MVP item 5): its length is in the log of the session it ended in.
			UE_LOG(LogCosmic, Display, TEXT("CosmicEnd: %s at tick %lld; opened ended (continued)"), CosmicEndings::NameOf(Ending.Kind), EndTick);
		}
		else
		{
			UE_LOG(LogCosmic, Display, TEXT("CosmicEnd: %s at tick %lld; %.0f s since tick %lld, %.0f s of them stopped"), CosmicEndings::NameOf(Ending.Kind),
				EndTick, Now - RunBeganRealSeconds, RunBeganTick, StoppedRealSeconds);
		}
		if (bArrived)
		{
			ChronicleWait = CosmicChronicleScreen::HoldSeconds;
			return;
		}
	}
	if (!bArrived)
	{
		ChronicleWait.Reset();
		UCosmicChronicleWidget* Page = CosmicHUD ? CosmicHUD->GetChronicle() : nullptr;
		if (Page && !Page->IsOpen())
		{
			Page->Open(LatchedRecordOf(), true);
		}
		if (CosmicHUD)
		{
			CosmicHUD->RefreshPanel();
		}
	}
}

void ACosmicPlayerController::AdvanceRunEnd(float DeltaSeconds)
{
	if (!ChronicleWait.IsSet())
	{
		return;
	}
	ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
	const UCosmicAscensionOverlayWidget* Overlay = CosmicHUD ? CosmicHUD->GetAscensionOverlay() : nullptr;
	if (Overlay && (Overlay->IsShowing() || Overlay->IsRevealing()))
	{
		return;   // the third rung's presentation and its reveal first
	}
	if (IsCallingPageOpen())
	{
		return;   // the calling's pages first (B5): their last 'next' opens the chronicle (FinishCallingPages)
	}
	ChronicleWait = ChronicleWait.GetValue() - FMath::Max(DeltaSeconds, 0.f);
	if (ChronicleWait.GetValue() <= 0.f)
	{
		ChronicleWait.Reset();
		if (UCosmicChronicleWidget* Page = CosmicHUD ? CosmicHUD->GetChronicle() : nullptr)
		{
			Page->Open(LatchedRecordOf(), false);
		}
		// The bar's 'chronicle' goes while the page shows.
		if (CosmicHUD)
		{
			CosmicHUD->RefreshPanel();
		}
	}
}

void ACosmicPlayerController::ClearRunEnd()
{
	const bool bWasEnded = IsRunEnded();
	RunEnd = FGameplayTag();
	ChronicleWait.Reset();
	if (UCosmicChronicleWidget* Page = GetHUD<ACosmicHUD>() ? GetHUD<ACosmicHUD>()->GetChronicle() : nullptr)
	{
		Page->Close();
	}
	// The calling's pages are the old run's too (B5): a new galaxy, a load, a test's AcquireSim.
	CloseCallingPages();
	// The speed and the switch stay. The RunEnd reason goes: AcquireSim stops the simulation with SetTimeScale, not through
	// ApplySpeed, so the top bar would otherwise keep saying the run is over.
	if (AutoPauseReason == ECosmicAutoPause::RunEnd)
	{
		AutoPauseReason = ECosmicAutoPause::None;
	}
	// What the end held back shows again (the feed's inauguration line of the end's tick - [B-5d1]).
	if (bWasEnded)
	{
		OnRunEndChanged.Broadcast();
	}
}

void ACosmicPlayerController::SetProvisionalEndEnabled(bool bOn)
{
	if (!bOn)
	{
		ClearRunEnd();
	}
	bProvisionalEnd = bOn;
}

bool ACosmicPlayerController::IsRunEndedFor(const UUserWidget& Widget)
{
	const ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(Widget.GetOwningPlayer());
	return Controller && Controller->IsRunEnded();
}

void ACosmicPlayerController::LookAtMap()
{
	if (UCosmicChronicleWidget* Page = GetHUD<ACosmicHUD>() ? GetHUD<ACosmicHUD>()->GetChronicle() : nullptr)
	{
		Page->Close();
	}
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
}

void ACosmicPlayerController::ReopenChronicle()
{
	UCosmicChronicleWidget* Page = GetHUD<ACosmicHUD>() ? GetHUD<ACosmicHUD>()->GetChronicle() : nullptr;
	if (IsRunEnded() && Page && !Page->IsOpen())
	{
		Page->Open(LatchedRecordOf(), true);
	}
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
}

void ACosmicPlayerController::StartNewRun()
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || Sim->IsBroadcasting())
	{
		return;
	}
	// The seed is an input (as in StartPlay); -Seed= named the first galaxy only. HandleGalaxyGenerated clears the end. The new
	// run is on the disk at once (MVP item 5); the old run's slot stays (decision 9).
	Sim->NewGalaxy(CosmicControlImpl::ClockSeed(), 0, 0);
	if (UCosmicSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>() : nullptr)
	{
		Save->RequestSave(ECosmicSaveCause::NewRun);
	}
	SetSpeed(1.f);
	FollowSaveFailure();
}

void ACosmicPlayerController::CosmicPreviewChronicle(float DelaySeconds)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	FTimerHandle Preview;
	World->GetTimerManager().SetTimer(Preview, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (UCosmicChronicleWidget* Page = GetHUD<ACosmicHUD>() ? GetHUD<ACosmicHUD>()->GetChronicle() : nullptr)
		{
			Page->Open(FCosmicEndingRecord(), true);
		}
	}), FMath::Max(DelaySeconds, 0.01f), false);
}

void ACosmicPlayerController::HandleLogEntry(const FCosmicLogEntry& Entry)
{
	// Inside a tick stage: only the time scale reaches the simulation (RequestAutoPause).
	if (CosmicAutoPause::StopsFor(Entry))
	{
		RequestAutoPause(ECosmicAutoPause::MythicLine);
	}
}

void ACosmicPlayerController::FollowOwnHazards()
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return;
	}
	// Another people under the vessel (a coronation, a return, a deposition - B1): its standing hazards are no news.
	const FGalaxyState& State = Sim->GetState();
	const FName Ruling = (State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler) ? State.Player.RuledCivID : NAME_None;
	if (Ruling != OwnHazardsRuler)
	{
		OwnHazardsSeen.Reset();
		OwnHazardsRuler = Ruling;
	}
	TMap<FName, FGameplayTag> Now = CosmicAutoPause::OwnHazardsOf(Sim->GetState(), Sim->GetStateIndex());
	const FName NewOn = OwnHazardsSeen.IsSet() ? CosmicAutoPause::FirstNewHazardSystem(OwnHazardsSeen.GetValue(), Now, Sim->GetState()) : NAME_None;
	OwnHazardsSeen = MoveTemp(Now);
	// The crisis focus (MVP item 6): only when this pause stopped the time - one stop, one move; a game already still (a
	// card, the presentation, another pause) or the automatic pause off keeps the camera where the viewer put it.
	if (!NewOn.IsNone() && RequestAutoPause(ECosmicAutoPause::OwnHazard))
	{
		// A press in flight is no click any more: its release would clear the crisis just selected (as a double click's).
		if (bLeftDown && !bPanning)
		{
			bSwallowRelease = true;
		}
		SelectAndFocus(NewOn, false);
	}
	FollowLethalHazard();
}

void ACosmicPlayerController::FollowLethalHazard()
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	const int32 Place = CosmicAutoPause::LethalPlaceOf(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
	const TPair<FName, FGameplayTag> Now = State.Systems.IsValidIndex(Place) ? TPair<FName, FGameplayTag>(State.Systems[Place].SystemID, State.Systems[Place].ActiveHazard)
		: TPair<FName, FGameplayTag>(NAME_None, FGameplayTag());
	const bool bNew = LethalSeen.IsSet() && !Now.Key.IsNone() && (LethalSeen.GetValue().Key != Now.Key || LethalSeen.GetValue().Value != Now.Value);
	LethalSeen = Now;
	// The wanderer's crisis focus: only when this pause stopped the time (MVP item 6's rule).
	if (bNew && RequestAutoPause(ECosmicAutoPause::LethalHazard))
	{
		if (bLeftDown && !bPanning)
		{
			bSwallowRelease = true;
		}
		SelectAndFocus(Now.Key, false);
	}
}

void ACosmicPlayerController::SetAutoPause(bool bOn)
{
	if (bOn == bAutoPause)
	{
		return;
	}
	bAutoPause = bOn;
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
}

void ACosmicPlayerController::ToggleAutoPause()
{
	SetAutoPause(!bAutoPause);
	// The viewer's choice outlives the session. A test run on the same machine reads it but never writes it.
	if (GConfig && !GIsAutomationTesting)
	{
		GConfig->SetString(TimeUnitSection, AutoPauseKey, bAutoPause ? TEXT("On") : TEXT("Off"), GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);
	}
}

bool ACosmicPlayerController::RequestAutoPause(ECosmicAutoPause Reason)
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!bAutoPause || Reason == ECosmicAutoPause::None || !Sim || !(Sim->GetTimeScale() > 0.f))
	{
		return false;
	}
	ApplySpeed(0.f, Reason);
	return !(Sim->GetTimeScale() > 0.f);
}

void ACosmicPlayerController::FollowSpeedGate()
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return;
	}
	// Only the closing lowers: a first look notes the gate, and a speed the gate never opened (-Speed=25) stays.
	const bool bOpen = IsSpiritualSightOpen();
	if (CosmicUI::LowersSpeed(ThirdRungWasOpen, bOpen, Sim->GetTimeScale()))
	{
		SetSpeed(CosmicUI::CallingTopSpeed);
	}
	ThirdRungWasOpen = bOpen;
}

void ACosmicPlayerController::CosmicStartProject(const FString& SystemID, const FString& RowName)
{
	if (!StartProject(FName(*SystemID), FName(*RowName)))
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicStartProject: %s on %s was refused"), *RowName, *SystemID);
	}
}

void ACosmicPlayerController::CosmicFocus(const FString& Word)
{
	// The even focus by its word, a field by its name (compared as FNames: the case does not matter).
	const FName Field = Word.Equals(TEXT("Even"), ESearchCase::IgnoreCase) ? NAME_None : FName(*Word);
	if (!Field.IsNone() && !CosmicUI::ResearchLabelKey(Field))
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicFocus: %s is no research field (Physics, Society, Engineering, Transcendence or Even)"), *Word);
		return;
	}
	if (!SetResearchFocus(Field))
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicFocus: %s was refused"), *Word);
	}
}

void ACosmicPlayerController::UseMap(ACosmicGalaxyMapActor* InMap)
{
	if (ACosmicGalaxyMapActor* Old = Map.Get())
	{
		Old->OnRebuilt.Remove(RebuiltHandle);
	}
	Map = InMap;
	RebuiltHandle = InMap ? InMap->OnRebuilt.AddUObject(this, &ACosmicPlayerController::HandleMapRebuilt) : FDelegateHandle();
	if (InMap)
	{
		InMap->SetViewWanted(WantedViewMode, bSightForcedOpen);
		InMap->SetSeeAll(bSeeAll);
	}
}

FCosmicKnownView ACosmicPlayerController::GetKnownView() const
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	return Sim ? CosmicMapLook::KnownViewOf(Sim->GetState(), Sim->GetStateIndex(), Sim->GetViewerTier(), bSeeAll) : FCosmicKnownView();
}

FCosmicKnownView ACosmicPlayerController::KnownViewFor(const UUserWidget& Widget)
{
	if (const ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(Widget.GetOwningPlayer()))
	{
		return Controller->GetKnownView();
	}
	const UGameInstance* GameInstance = Widget.GetGameInstance();
	const UGalaxySimulationSubsystem* Sim = GameInstance ? GameInstance->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	return Sim ? CosmicMapLook::KnownViewOf(Sim->GetState(), Sim->GetStateIndex(), Sim->GetViewerTier(), false) : FCosmicKnownView();
}

void ACosmicPlayerController::CosmicSeeAll(int32 On)
{
	const bool bOn = On != 0;
	if (bOn == bSeeAll)
	{
		return;
	}
	bSeeAll = bOn;
	if (ACosmicGalaxyMapActor* TheMap = Map.Get())
	{
		TheMap->SetSeeAll(bSeeAll);
	}
	// Knowledge has its say back: a selection the viewer does not know goes.
	if (!bSeeAll && !SelectedSystemID.IsNone())
	{
		const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
		const int32 Selected = Sim ? Sim->GetStateIndex().FindSystem(Sim->GetState(), SelectedSystemID) : INDEX_NONE;
		if (!GetKnownView().IsShown(Selected))
		{
			SelectSystem(NAME_None);
		}
	}
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
	UE_LOG(LogCosmic, Display, TEXT("CosmicSeeAll: %s"), bSeeAll ? TEXT("every system is drawn as known and every people as shown (nothing is written)") : TEXT("the screens draw what the viewer knows"));
}

void ACosmicPlayerController::SelectSystem(FName SystemID, bool bByViewer)
{
	if (SystemID == SelectedSystemID)
	{
		return;
	}
	if (!SystemID.IsNone())
	{
		// A system of the state that the viewer is shown - known, or the sanctuary by its name: a backdrop point - an
		// unknown system's among them - has no selection (section 6.4).
		const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
		const int32 SystemIndex = Sim ? Sim->GetStateIndex().FindSystem(Sim->GetState(), SystemID) : INDEX_NONE;
		if (SystemIndex == INDEX_NONE || !GetKnownView().IsShown(SystemIndex))
		{
			return;
		}
	}
	SelectedSystemID = SystemID;
	if (ACosmicGalaxyMapActor* TheMap = Map.Get())
	{
		TheMap->SetSelectedSystem(SystemID);
	}
	// Noted before the announcement: the HUD redraws the hint from it. The automatic crisis focus is not the viewer's pick.
	if (!SystemID.IsNone() && bByViewer)
	{
		Hints.bPickedSystem = true;
	}
	OnSelectionChanged.Broadcast(SystemID);
}

void ACosmicPlayerController::SelectGalaxy(FName Row)
{
	if (Row == SelectedGalaxyRow)
	{
		return;
	}
	if (!Row.IsNone())
	{
		// A named galaxy the universe draws: a scenery galaxy has no selection (section 5.11).
		const ACosmicGalaxyMapActor* TheMap = Map.Get();
		if (!TheMap || !TheMap->GetGalaxyMarks().ContainsByPredicate([Row](const FCosmicGalaxyMark& Mark) { return Mark.Row == Row; }))
		{
			return;
		}
	}
	SelectedGalaxyRow = Row;
	if (ACosmicGalaxyMapActor* TheMap = Map.Get())
	{
		TheMap->SetSelectedGalaxy(Row);
	}
	if (!Row.IsNone())
	{
		Hints.bPickedGalaxy = true;
	}
	OnGalaxySelectionChanged.Broadcast(Row);
}

bool ACosmicPlayerController::SelectAndFocusGalaxy(FName Row)
{
	const ACosmicGalaxyMapActor* TheMap = Map.Get();
	const FCosmicGalaxyMark* Mark = (TheMap && !Row.IsNone()) ? TheMap->GetGalaxyMarks().FindByPredicate([Row](const FCosmicGalaxyMark& Each) { return Each.Row == Row; }) : nullptr;
	if (!Mark)
	{
		return false;
	}
	if (Mark->bHome)
	{
		// The way home: the region framed again, no galaxy selected.
		SelectGalaxy(NAME_None);
		FrameRegion();
	}
	else
	{
		SelectGalaxy(Row);
		if (ACosmicCameraPawn* CameraPawn = GetCameraPawn())
		{
			CameraPawn->FocusOnUniverse(Mark->Position, CosmicUniverseLook::FocusArmOf(*Mark, TheMap->GetZoomLevels()));
		}
	}
	FollowZoomLevel();
	return true;
}

void ACosmicPlayerController::FrameRegion()
{
	const ACosmicGalaxyMapActor* TheMap = Map.Get();
	ACosmicCameraPawn* CameraPawn = GetCameraPawn();
	if (TheMap && CameraPawn)
	{
		CameraPawn->FrameAll(TheMap->GetZoomLevels());
	}
}

ECosmicZoomLevel ACosmicPlayerController::GetZoomLevel() const
{
	const ACosmicCameraPawn* CameraPawn = GetCameraPawn();
	return CameraPawn ? CameraPawn->GetTargetLevel() : ECosmicZoomLevel::Region;
}

void ACosmicPlayerController::FollowZoomLevel()
{
	const ECosmicZoomLevel Now = GetZoomLevel();
	if (LastLevel.IsSet() && LastLevel.GetValue() != Now)
	{
		// Noted before the announcement: the HUD redraws the hint from it.
		if (Now == ECosmicZoomLevel::Universe)
		{
			Hints.bReachedUniverse = true;
		}
		else
		{
			Hints.bLeftUniverse = true;
		}
		LastLevel = Now;
		OnZoomLevelChanged.Broadcast();
		return;
	}
	LastLevel = Now;
}

void ACosmicPlayerController::CosmicWheel(float Steps)
{
	const ACosmicGalaxyMapActor* TheMap = Map.Get();
	// A map still to be built frames the camera when it is: wait for it.
	if (!TheMap || TheMap->IsRebuildPending() || TheMap->GetStars().Num() == 0)
	{
		PendingWheel = Steps;
		return;
	}
	if (ACosmicCameraPawn* CameraPawn = GetCameraPawn())
	{
		CameraPawn->Zoom(Steps);
	}
	FollowZoomLevel();
}

void ACosmicPlayerController::CosmicSelectGalaxy(const FString& Row)
{
	const FName Wanted(*Row);
	const ACosmicGalaxyMapActor* TheMap = Map.Get();
	if (!TheMap || TheMap->IsRebuildPending() || TheMap->GetStars().Num() == 0)
	{
		PendingGalaxy = Wanted;
		return;
	}
	if (!SelectAndFocusGalaxy(Wanted))
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicSelectGalaxy: %s is no galaxy the universe draws"), *Wanted.ToString());
	}
}

void ACosmicPlayerController::CosmicSelect(const FString& SystemID)
{
	const FName Wanted(*SystemID);
	const ACosmicGalaxyMapActor* TheMap = Map.Get();
	// A map still to be built (or rebuilt) holds the old galaxy's stars: wait for the build.
	if (!TheMap || TheMap->IsRebuildPending() || TheMap->GetStars().Num() == 0)
	{
		PendingSelect = Wanted;
		return;
	}
	// On a built map nothing waits: a system the viewer does not know is refused at once.
	if (!SelectAndFocus(Wanted))
	{
		LogRefusedSelect(Wanted);
	}
}

void ACosmicPlayerController::LogRefusedSelect(FName Wanted) const
{
	// An ID the state does not hold (a typo) is no matter of knowledge: CosmicSeeAll would not help.
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || Sim->GetStateIndex().FindSystem(Sim->GetState(), Wanted) == INDEX_NONE)
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicSelect: %s is no system of this galaxy"), *Wanted.ToString());
		return;
	}
	UE_LOG(LogCosmic, Display, TEXT("CosmicSelect: %s is not a system the viewer knows (CosmicSeeAll 1 draws every system)"), *Wanted.ToString());
}

void ACosmicPlayerController::CosmicPreviewAscension(float DelaySeconds)
{
	CosmicPreviewRung(DelaySeconds, 2);
}

void ACosmicPlayerController::CosmicPreviewRung(float DelaySeconds, int32 Rung)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	// The rung 1..5; anything else is the second.
	const EAscensionTier Tier = (Rung >= 1 && Rung <= 5) ? static_cast<EAscensionTier>(Rung - 1) : EAscensionTier::Tier2_Stellar;
	FTimerHandle Preview;
	World->GetTimerManager().SetTimer(Preview, FTimerDelegate::CreateWeakLambda(this, [this, Tier]()
	{
		const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
		ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
		const TArray<FName> Feats = Sim ? Sim->GetTables().GetFeatRowNames() : TArray<FName>();
		// The first feat row that reaches the rung, else the first row.
		FName Row = Feats.IsEmpty() ? NAME_None : Feats[0];
		for (const FName Each : Feats)
		{
			const FAscensionFeatDef* Feat = Sim->GetTables().FindFeat(Each);
			if (Feat && Feat->TargetTier == Tier)
			{
				Row = Each;
				break;
			}
		}
		if (CosmicHUD && CosmicHUD->GetAscensionOverlay())
		{
			CosmicHUD->GetAscensionOverlay()->Show(Tier, Row);
		}
	}), FMath::Max(DelaySeconds, 0.01f), false);
}

bool ACosmicPlayerController::SelectAndFocus(FName SystemID, bool bByViewer)
{
	const ACosmicGalaxyMapActor* TheMap = Map.Get();
	const ACosmicStarSystemActor* Star = (TheMap && !SystemID.IsNone()) ? TheMap->FindStar(SystemID) : nullptr;
	if (!Star)
	{
		return false;
	}
	SelectSystem(SystemID, bByViewer);
	// Refused (a system the viewer is not shown): the camera stays (section 8.1 - it goes to a known system only).
	if (SelectedSystemID != SystemID)
	{
		return false;
	}
	if (ACosmicCameraPawn* CameraPawn = GetCameraPawn())
	{
		// From the universe too: a system is looked at in the region.
		CameraPawn->FocusOn(Star->GetActorLocation());
	}
	if (bByViewer)
	{
		FollowZoomLevel();
		return true;
	}
	// Not the viewer's way home (MVP item 6): the level is announced as a rebuild announces its framing, and the next
	// FollowZoomLevel finds it noted - the universe's hints stay.
	const ECosmicZoomLevel Framed = GetZoomLevel();
	if (LastLevel.IsSet() && LastLevel.GetValue() != Framed)
	{
		LastLevel = Framed;
		OnZoomLevelChanged.Broadcast();
	}
	return true;
}

void ACosmicPlayerController::HandleMapRebuilt()
{
	ACosmicGalaxyMapActor* TheMap = Map.Get();
	SelectSystem(NAME_None);
	SelectGalaxy(NAME_None);
	// A new galaxy or a load starts at the default view (the map put its own back before it drew; this announces it).
	SetWantedViewMode(ECosmicViewMode::Spiritual);
	if (ACosmicCameraPawn* CameraPawn = GetCameraPawn(); CameraPawn && TheMap)
	{
		// The region framed on the map's levels, at once; the map's share is the snapped arm's.
		FrameRegion();
		CameraPawn->SnapToTargets();
		TheMap->SetViewDistance(CameraPawn->GetArmLength());
		// That framing is the rebuild's, not the player's way home: the level is announced (the right side follows it)
		// without noting the hint, which stays for the session (Phase 3 item 9's review).
		const ECosmicZoomLevel Framed = GetZoomLevel();
		if (LastLevel.IsSet() && LastLevel.GetValue() != Framed)
		{
			LastLevel = Framed;
			OnZoomLevelChanged.Broadcast();
		}
	}
	if (!PendingSelect.IsNone())
	{
		const FName Wanted = PendingSelect;
		PendingSelect = NAME_None;
		if (!SelectAndFocus(Wanted))
		{
			LogRefusedSelect(Wanted);
		}
	}
	if (PendingWheel.IsSet())
	{
		const float Steps = PendingWheel.GetValue();
		PendingWheel.Reset();
		if (ACosmicCameraPawn* CameraPawn = GetCameraPawn())
		{
			CameraPawn->Zoom(Steps);
		}
	}
	if (!PendingGalaxy.IsNone())
	{
		const FName Wanted = PendingGalaxy;
		PendingGalaxy = NAME_None;
		if (!SelectAndFocusGalaxy(Wanted))
		{
			UE_LOG(LogCosmic, Display, TEXT("CosmicSelectGalaxy: %s is no galaxy the universe draws"), *Wanted.ToString());
		}
	}
	FollowZoomLevel();
}

ACosmicCameraPawn* ACosmicPlayerController::GetCameraPawn() const
{
	return Cast<ACosmicCameraPawn>(GetPawn());
}

void ACosmicPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	for (const FKey& Key : { EKeys::Zero, EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::SpaceBar })
	{
		InputComponent->BindKey(Key, IE_Pressed, this, &ACosmicPlayerController::PressSpeedKey);
	}

	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ACosmicPlayerController::HandleLeftPressed);
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &ACosmicPlayerController::HandleLeftReleased);
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_DoubleClick, this, &ACosmicPlayerController::HandleLeftDoubleClick);
	InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ACosmicPlayerController::HandleRightPressed);
	InputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &ACosmicPlayerController::HandleRightReleased);
	InputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &ACosmicPlayerController::HandleWheel);
	InputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &ACosmicPlayerController::HandleWheel);
}

void ACosmicPlayerController::PlayerTick(float DeltaTime)
{
	using namespace CosmicControlImpl;

	Super::PlayerTick(DeltaTime);

	ACosmicCameraPawn* CameraPawn = GetCameraPawn();
	float MouseX = 0.f;
	float MouseY = 0.f;
	if (CameraPawn && GetMousePosition(MouseX, MouseY))
	{
		const FVector2D Mouse(MouseX, MouseY);
		if (bLeftDown && !bPanning && FVector2D::Distance(Mouse, LeftPressAt) > DragPixels)
		{
			bPanning = true;
		}
		if (bPanning)
		{
			// The disc point that was under the cursor last frame goes under it now (both read through the same view).
			FVector Was;
			FVector Now;
			if (DiscPointAt(LastMouse, Was) && DiscPointAt(Mouse, Now))
			{
				CameraPawn->Pan(Was - Now);
			}
		}
		if (bRightDown)
		{
			const FVector2D Moved = Mouse - LastMouse;
			CameraPawn->Orbit(Moved.X * OrbitDegreesPerPixel, -Moved.Y * OrbitDegreesPerPixel);
		}
		LastMouse = Mouse;
	}
	if (ACosmicGalaxyMapActor* TheMap = Map.Get(); TheMap && CameraPawn)
	{
		TheMap->SetViewDistance(CameraPawn->GetArmLength());
	}
	// The map reads the viewer's tier at each refresh itself; this only brings the wanted view back when the sight opens
	// again after it closed.
	FollowSightOpening();
	FollowZoomLevel();
	// The provisional end: the wait before the chronicle, and the real seconds the simulation stood still before it (the
	// engine never pauses and nothing dilates time, so DeltaTime is real time).
	AdvanceRunEnd(DeltaTime);
	if (const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr; Sim && !IsRunEnded() && !(Sim->GetTimeScale() > 0.f))
	{
		StoppedRealSeconds += DeltaTime;
	}
}

bool ACosmicPlayerController::DiscPointAt(const FVector2D& Screen, FVector& OutPoint) const
{
	FVector Origin;
	FVector Direction;
	return DeprojectScreenPositionToWorld(Screen.X, Screen.Y, Origin, Direction) && CosmicMapPick::OntoDisc(Origin, Direction, OutPoint);
}

TArray<FCosmicPickCandidate> ACosmicPlayerController::PickCandidatesOf(const ACosmicGalaxyMapActor& Map, const FCosmicKnownView& View, TFunctionRef<bool(const FVector&, FVector2D&)> Project, const FVector& Side)
{
	TArray<FCosmicPickCandidate> Candidates;
	// A click takes only what is drawn: at the universe level no star (a named galaxy there).
	if (!Map.IsRegionDetailShown())
	{
		return Candidates;
	}
	const TArray<TObjectPtr<ACosmicStarSystemActor>>& Stars = Map.GetStars();
	for (int32 Index = 0; Index < Stars.Num(); ++Index)
	{
		const ACosmicStarSystemActor* Star = Stars[Index];
		// The star at index i draws system i (the map's own rule): a system the viewer is not shown is a backdrop point,
		// and background stars have no selection.
		if (!Star || !View.IsShown(Index))
		{
			continue;
		}
		// A star's drawn radius: its centre and a point its radius to the side of the view, both projected.
		FVector2D Centre;
		FVector2D Edge;
		if (Project(Star->GetActorLocation(), Centre) && Project(Star->GetActorLocation() + Side * Star->GetStarRadius(), Edge))
		{
			FCosmicPickCandidate& Candidate = Candidates.AddDefaulted_GetRef();
			Candidate.SystemID = Star->GetSystemID();
			Candidate.Target.Centre = Centre;
			Candidate.Target.Radius = static_cast<float>(FVector2D::Distance(Centre, Edge));
		}
	}
	return Candidates;
}

TArray<FCosmicGalaxyPickCandidate> ACosmicPlayerController::GalaxyPickCandidatesOf(const ACosmicGalaxyMapActor& Map, TFunctionRef<bool(const FVector&, FVector2D&)> Project, const FVector& Side)
{
	TArray<FCosmicGalaxyPickCandidate> Candidates;
	// A click takes only what is drawn: in the region no galaxy; never a scenery galaxy (section 5.11).
	if (Map.IsRegionDetailShown())
	{
		return Candidates;
	}
	for (const FCosmicGalaxyMark& Mark : Map.GetGalaxyMarks())
	{
		FVector2D Centre;
		FVector2D Edge;
		if (Project(Mark.Position, Centre) && Project(Mark.Position + Side * CosmicUniverseLook::RingRadiusOf(Mark), Edge))
		{
			FCosmicGalaxyPickCandidate& Candidate = Candidates.AddDefaulted_GetRef();
			Candidate.Row = Mark.Row;
			Candidate.Target.Centre = Centre;
			Candidate.Target.Radius = static_cast<float>(FVector2D::Distance(Centre, Edge));
		}
	}
	return Candidates;
}

FName ACosmicPlayerController::PickGalaxyAtCursor() const
{
	const ACosmicGalaxyMapActor* TheMap = Map.Get();
	float MouseX = 0.f;
	float MouseY = 0.f;
	if (!TheMap || !GetMousePosition(MouseX, MouseY))
	{
		return NAME_None;
	}
	FVector ViewLocation;
	FRotator ViewRotation;
	GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector Side = FRotationMatrix(ViewRotation).GetUnitAxis(EAxis::Y);
	const TArray<FCosmicGalaxyPickCandidate> Candidates = GalaxyPickCandidatesOf(*TheMap, [this](const FVector& World, FVector2D& Screen)
	{
		return ProjectWorldLocationToScreen(World, Screen);
	}, Side);
	TArray<FCosmicScreenTarget> Targets;
	Targets.Reserve(Candidates.Num());
	for (const FCosmicGalaxyPickCandidate& Candidate : Candidates)
	{
		Targets.Add(Candidate.Target);
	}
	const int32 Index = CosmicMapPick::NearestOnScreen(Targets, FVector2D(MouseX, MouseY), CosmicControlImpl::PickPixels);
	return Candidates.IsValidIndex(Index) ? Candidates[Index].Row : NAME_None;
}

FName ACosmicPlayerController::PickAtCursor() const
{
	const ACosmicGalaxyMapActor* TheMap = Map.Get();
	float MouseX = 0.f;
	float MouseY = 0.f;
	if (!TheMap || !GetMousePosition(MouseX, MouseY))
	{
		return NAME_None;
	}
	FVector ViewLocation;
	FRotator ViewRotation;
	GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector Side = FRotationMatrix(ViewRotation).GetUnitAxis(EAxis::Y);
	// The live view, the one the selection asks too: a click and the selection never disagree about a star.
	const TArray<FCosmicPickCandidate> Candidates = PickCandidatesOf(*TheMap, GetKnownView(), [this](const FVector& World, FVector2D& Screen)
	{
		return ProjectWorldLocationToScreen(World, Screen);
	}, Side);
	TArray<FCosmicScreenTarget> Targets;
	Targets.Reserve(Candidates.Num());
	for (const FCosmicPickCandidate& Candidate : Candidates)
	{
		Targets.Add(Candidate.Target);
	}
	const int32 Index = CosmicMapPick::NearestOnScreen(Targets, FVector2D(MouseX, MouseY), CosmicControlImpl::PickPixels);
	return Candidates.IsValidIndex(Index) ? Candidates[Index].SystemID : NAME_None;
}

void ACosmicPlayerController::HandleLeftPressed(FKey Key)
{
	float MouseX = 0.f;
	float MouseY = 0.f;
	GetMousePosition(MouseX, MouseY);
	bLeftDown = true;
	bPanning = false;
	LeftPressAt = FVector2D(MouseX, MouseY);
	LastMouse = LeftPressAt;
	// The swallow of a double click is not cleared here. The engine sends the second press before the double click
	// (UPlayerInput queues IE_Pressed first and sorts the delegates by event index), but the order does not matter:
	// UI.CameraAndSelection holds both orders.
}

void ACosmicPlayerController::HandleLeftReleased(FKey Key)
{
	// The release that ends a double click is no click: the double click already selected and focused, and a second
	// click through the gliding view could clear or move that selection.
	const bool bSwallow = bSwallowRelease;
	bSwallowRelease = false;
	// A press that did not become a pan is a click: what the map draws under the cursor - a star in the region, a named
	// galaxy at the universe level - or empty space (clears the selection of that level).
	if (bLeftDown && !bPanning && !bSwallow)
	{
		const ACosmicGalaxyMapActor* TheMap = Map.Get();
		if (TheMap && !TheMap->IsRegionDetailShown())
		{
			SelectGalaxy(PickGalaxyAtCursor());
		}
		else
		{
			SelectSystem(PickAtCursor());
		}
	}
	bLeftDown = false;
	bPanning = false;
}

void ACosmicPlayerController::HandleLeftDoubleClick(FKey Key)
{
	bSwallowRelease = true;
	const ACosmicGalaxyMapActor* TheMap = Map.Get();
	if (TheMap && !TheMap->IsRegionDetailShown())
	{
		SelectAndFocusGalaxy(PickGalaxyAtCursor());
	}
	else
	{
		SelectAndFocus(PickAtCursor());
	}
}

void ACosmicPlayerController::HandleRightPressed(FKey Key)
{
	float MouseX = 0.f;
	float MouseY = 0.f;
	GetMousePosition(MouseX, MouseY);
	bRightDown = true;
	LastMouse = FVector2D(MouseX, MouseY);
}

void ACosmicPlayerController::HandleRightReleased(FKey Key)
{
	bRightDown = false;
}

void ACosmicPlayerController::HandleWheel(FKey Key)
{
	// The chronicle's page takes the wheel while it shows (its scroll box), never the camera under it.
	if (const UCosmicChronicleWidget* Page = GetHUD<ACosmicHUD>() ? GetHUD<ACosmicHUD>()->GetChronicle() : nullptr; Page && Page->IsOpen())
	{
		return;
	}
	// So do the calling's pages, under the chronicle's veil (B5 header section 4): the map under them does not move.
	if (IsCallingPageOpen())
	{
		return;
	}
	if (ACosmicCameraPawn* CameraPawn = GetCameraPawn())
	{
		CameraPawn->Zoom(Key == EKeys::MouseScrollUp ? 1.f : -1.f);
	}
	FollowZoomLevel();
}

void ACosmicPlayerController::FollowPetition(bool bArrived)
{
	// An ended run shows no card (FollowEnd folded it; the row stays in Knowing).
	if (IsRunEnded())
	{
		return;
	}
	ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
	UCosmicPetitionCardWidget* Card = CosmicHUD ? CosmicHUD->GetPetitionCard() : nullptr;
	if (!Card || Card->IsShowingOutcome())
	{
		return;
	}
	const FCosmicKnownView View = GetKnownView();
	if (View.PendingPetitionRow == Card->GetShownRow() && View.PendingPetitionSystemID == Card->GetShownSystem())
	{
		Card->Refresh();
		return;
	}
	Card->ShowCard(View.PendingPetitionRow, View.PendingPetitionSystemID, View.PendingPetitionRow == CosmicRows::PetitionFirstContact ? View.PendingContactCivID : NAME_None);
	if (bArrived && !View.PendingPetitionRow.IsNone())
	{
		// A wanderer's card is no petition: its own words on the top bar (B1).
		const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
		const FEncounterDef* Row = Sim ? Sim->GetTables().FindEncounter(View.PendingPetitionRow) : nullptr;
		RequestAutoPause((Row && CosmicPetitions::IsWandererCard(*Row)) ? ECosmicAutoPause::Encounter : ECosmicAutoPause::Card);
	}
}

FCosmicPetitionAnswer ACosmicPlayerController::AnswerPetition(int32 ChoiceIndex)
{
	// The run is over: no command reaches the simulation (Refused - nothing drawn, nothing written).
	if (IsRunEnded())
	{
		return FCosmicPetitionAnswer();
	}
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	const FCosmicPetitionAnswer Answer = Roles ? Roles->AnswerPetition(ChoiceIndex) : FCosmicPetitionAnswer();
	if (Answer.Outcome == ECosmicPetitionOutcome::Refused)
	{
		return Answer;
	}
	// The one input with a draw is on the disk at once (decision 11): a forced quit cannot roll the card again.
	if (UCosmicSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>() : nullptr)
	{
		Save->RequestSave(ECosmicSaveCause::Answer);
	}
	FollowSaveFailure();
	// A paused simulation sends no tick: show what the answer changed now, on the panels and on the map.
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
	if (ACosmicGalaxyMapActor* TheMap = Map.Get())
	{
		TheMap->RequestRefresh();
	}
	return Answer;
}

void ACosmicPlayerController::ClosePetitionCard()
{
	FollowPetition(false);
	const ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
	const UCosmicPetitionCardWidget* Card = CosmicHUD ? CosmicHUD->GetPetitionCard() : nullptr;
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	// The card stopped the time and nothing else set a speed since: the time runs again (as the ascension presentation
	// gives its speed back). A chained card keeps it standing.
	if (Card && Card->GetShownRow().IsNone() && Sim && !(Sim->GetTimeScale() > 0.f)
		&& (AutoPauseReason == ECosmicAutoPause::Card || AutoPauseReason == ECosmicAutoPause::Encounter))
	{
		SetSpeed(SpeedBeforePause);
	}
}

bool ACosmicPlayerController::StartProject(FName SystemID, FName RowName)
{
	if (IsRunEnded())
	{
		return false;
	}
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Roles || !Roles->StartMegaproject(SystemID, RowName))
	{
		return false;
	}
	// The system panel's hint line has done its work (MVP item 6): noted before the redraw below.
	Hints.bStartedProject = true;
	// A paused simulation sends no tick: show the new project now, on the panels and on the map.
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
	if (ACosmicGalaxyMapActor* TheMap = Map.Get())
	{
		TheMap->RequestRefresh();
	}
	return true;
}

bool ACosmicPlayerController::SetResearchFocus(FName Field)
{
	if (IsRunEnded())
	{
		return false;
	}
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Roles || !Roles->SetResearchFocus(Field))
	{
		return false;
	}
	// A paused simulation sends no tick: show the new focus now.
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
	return true;
}

bool ACosmicPlayerController::EnactEdict(FName Row)
{
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (IsRunEnded() || !Roles || Roles->EnactEdict(Row) != CosmicEdicts::ECosmicEdictBlock::None)
	{
		return false;
	}
	FollowEdictHint();
	// A paused simulation sends no tick: show the edict now.
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
	return true;
}

bool ACosmicPlayerController::RevokeEdict(FName Row)
{
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (IsRunEnded() || !Roles || !Roles->RevokeEdict(Row))
	{
		return false;
	}
	// A revocation of the inherited edict ends its hint.
	FollowEdictHint();
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
	return true;
}

bool ACosmicPlayerController::SetResearchShares(const TMap<FName, int32>& Weights)
{
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (IsRunEnded() || !Roles || !Roles->SetResearchShares(Weights))
	{
		return false;
	}
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
	return true;
}

bool ACosmicPlayerController::SetStance(FName CivID, FGameplayTag Stance)
{
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (IsRunEnded() || !Roles || Roles->SetStance(CivID, Stance) != CosmicDiplomacy::ECosmicStanceBlock::None)
	{
		return false;
	}
	FollowCommand();
	return true;
}

bool ACosmicPlayerController::ProposeTreaty(FName CivID, FGameplayTag Treaty)
{
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (IsRunEnded() || !Roles || Roles->ProposeTreaty(CivID, Treaty) != CosmicDiplomacy::ECosmicTreatyBlock::None)
	{
		return false;
	}
	FollowCommand();
	return true;
}

bool ACosmicPlayerController::BreakTreaty(FName CivID, FGameplayTag Treaty)
{
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (IsRunEnded() || !Roles || !Roles->BreakTreaty(CivID, Treaty))
	{
		return false;
	}
	FollowCommand();
	return true;
}

bool ACosmicPlayerController::AllocateFleet(FName SystemID)
{
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (IsRunEnded() || !Roles || Roles->AllocateFleet(SystemID) != CosmicConflict::ECosmicSiegeBlock::None)
	{
		return false;
	}
	FollowCommand();
	return true;
}

void ACosmicPlayerController::ToggleEdictWindow()
{
	ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
	if (UCosmicEdictWidget* Window = CosmicHUD ? CosmicHUD->GetEdictWindow() : nullptr)
	{
		Window->Toggle();
	}
}

void ACosmicPlayerController::OpenEdictWindow(ECosmicEdictTab Tab)
{
	ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
	if (UCosmicEdictWidget* Window = CosmicHUD ? CosmicHUD->GetEdictWindow() : nullptr)
	{
		Window->Open(Tab);
	}
}

void ACosmicPlayerController::NoteEdictWindowOpened()
{
	Hints.bEdictHintDone = true;
	// The top bar's mark and the capital panel's line go now.
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
}

void ACosmicPlayerController::FollowEdictHint()
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	const int32 Ruled = State.Player.RuledCivID.IsNone() ? INDEX_NONE : Sim->GetStateIndex().FindCiv(State, State.Player.RuledCivID);
	const bool bRuler = State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler && State.Civs.IsValidIndex(Ruled);
	const FName Inherited = Sim->GetRules().StartingEdict;
	const bool bHolds = bRuler && !Inherited.IsNone() && State.Civs[Ruled].ActiveEdicts.Contains(Inherited);
	CosmicUI::StepEdictHint(Hints, bRuler, bHolds, bRuler ? State.Civs[Ruled].Stability : 1.f);
}

bool ACosmicPlayerController::TravelTo(FName SystemID)
{
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (IsRunEnded() || !Roles || Roles->TravelTo(SystemID) != CosmicWanderer::ETravelBlock::None)
	{
		return false;
	}
	FollowCommand();
	return true;
}

int32 ACosmicPlayerController::ScanHere()
{
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	const int32 Learned = (IsRunEnded() || !Roles) ? INDEX_NONE : Roles->ScanCurrentSystem();
	if (Learned != INDEX_NONE)
	{
		FollowCommand();
	}
	return Learned;
}

bool ACosmicPlayerController::TakeThrone(FName CivID)
{
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (IsRunEnded() || !Roles || Roles->BecomeRuler(CivID) != CosmicWanderer::ECrownBlock::None)
	{
		return false;
	}
	// The throne changed between ticks: the new people's standing crises are noted now - one that rises on the next tick
	// still stops the time -, and the waiting card was cleared with it. The new reign is noted for the reign end's page (B5).
	FollowOwnHazards();
	FollowPetition(false);
	FollowReign(false);
	FollowCommand();
	return true;
}

bool ACosmicPlayerController::Abdicate()
{
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (IsRunEnded() || !Roles || !Roles->Abdicate())
	{
		return false;
	}
	FollowCommand();
	return true;
}

void ACosmicPlayerController::FollowCommand()
{
	// A paused simulation sends no tick: show what the command changed now, on the panels and on the map.
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
	if (ACosmicGalaxyMapActor* TheMap = Map.Get())
	{
		TheMap->RequestRefresh();
	}
}

void ACosmicPlayerController::CosmicTravel(const FString& SystemID)
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (IsRunEnded())
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicTravel: refused (the run is over)"));
		return;
	}
	// No argument: one line with the lanes of the system the vessel stands in and the gate's answer for each.
	if (Sim && SystemID.IsEmpty())
	{
		const FGalaxyState& State = Sim->GetState();
		const int32 Here = Sim->GetStateIndex().FindSystem(State, State.Player.CurrentSystemID);
		TArray<FString> Lanes;
		for (const FName Lane : State.Systems.IsValidIndex(Here) ? State.Systems[Here].Lanes : TArray<FName>())
		{
			Lanes.Add(FString::Printf(TEXT("%s (%d)"), *Lane.ToString(), static_cast<int32>(CosmicWanderer::TravelBlockOf(State, Sim->GetStateIndex(), Lane,
				!GetKnownView().PendingPetitionRow.IsNone()))));
		}
		UE_LOG(LogCosmic, Display, TEXT("CosmicTravel: the lanes from %s (the gate's reason, 0 = taken): %s"), *State.Player.CurrentSystemID.ToString(), *FString::Join(Lanes, TEXT(", ")));
		return;
	}
	const FName Target(*SystemID);
	if (Sim && !TravelTo(Target))
	{
		const bool bCard = !GetKnownView().PendingPetitionRow.IsNone();
		UE_LOG(LogCosmic, Display, TEXT("CosmicTravel: %s was refused (reason %d)"), *SystemID,
			static_cast<int32>(CosmicWanderer::TravelBlockOf(Sim->GetState(), Sim->GetStateIndex(), Target, bCard)));
	}
}

void ACosmicPlayerController::CosmicScan()
{
	if (ScanHere() == INDEX_NONE)
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicScan: refused (a ruler, a vessel under way, or an ended run)"));
	}
}

void ACosmicPlayerController::CosmicCrown()
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return;
	}
	if (IsRunEnded())
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicCrown: refused (the run is over)"));
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	const int32 Here = Sim->GetStateIndex().FindSystem(State, State.Player.CurrentSystemID);
	const FName HereOwner = State.Systems.IsValidIndex(Here) ? State.Systems[Here].OwnerCivID : NAME_None;
	if (!TakeThrone(HereOwner))
	{
		// What the command knows, read as the screens read it (rule 14).
		const FCosmicKnownView View = GetKnownView();
		const int32 OwnerIndex = HereOwner.IsNone() ? INDEX_NONE : Sim->GetStateIndex().FindCiv(State, HereOwner);
		UE_LOG(LogCosmic, Display, TEXT("CosmicCrown: refused over %s (reason %d)"), *HereOwner.ToString(), static_cast<int32>(CosmicWanderer::CrownBlockOf(
			State, Sim->GetStateIndex(), Sim->GetRules(), HereOwner, View.IsMet(OwnerIndex), !View.PendingPetitionRow.IsNone())));
	}
}

void ACosmicPlayerController::CosmicAbdicate()
{
	if (!Abdicate())
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicAbdicate: refused (no embodied ruler, a role effect already recorded, or an ended run)"));
	}
}

void ACosmicPlayerController::CosmicEdict(const FString& Row)
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return;
	}
	if (IsRunEnded())
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicEdict: refused (the run is over)"));
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	const int32 Ruled = (State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler && !State.Player.RuledCivID.IsNone())
		? Sim->GetStateIndex().FindCiv(State, State.Player.RuledCivID) : INDEX_NONE;
	auto Holds = [&](FName Name) { return State.Civs.IsValidIndex(Ruled) && State.Civs[Ruled].ActiveEdicts.Contains(Name); };
	// No embodied ruler: the command's own reason, NotRuler (the gate would read the missing people as NoRow).
	auto ReasonOf = [&](FName Name)
	{
		return static_cast<int32>(State.Civs.IsValidIndex(Ruled) ? CosmicEdicts::EnactBlockOf(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), Ruled, Name)
			: CosmicEdicts::ECosmicEdictBlock::NotRuler);
	};
	// No argument: one line with every row, held or the gate's reason.
	if (Row.IsEmpty())
	{
		TArray<FString> Rows;
		for (const FName Each : Sim->GetTables().GetEdictRowNames())
		{
			const FString Answer = Holds(Each) ? FString(TEXT("held")) : FString::FromInt(ReasonOf(Each));
			Rows.Add(FString::Printf(TEXT("%s (%s)"), *Each.ToString(), *Answer));
		}
		UE_LOG(LogCosmic, Display, TEXT("CosmicEdict: the edicts (held, or the gate's reason - 0 = it can be enacted): %s"), *FString::Join(Rows, TEXT(", ")));
		return;
	}
	const FName Name(*Row);
	if (Holds(Name))
	{
		if (!RevokeEdict(Name))
		{
			UE_LOG(LogCosmic, Display, TEXT("CosmicEdict: the revocation of %s was refused"), *Row);
		}
	}
	else if (!EnactEdict(Name))
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicEdict: %s was refused (reason %d)"), *Row, ReasonOf(Name));
	}
}

void ACosmicPlayerController::CosmicShares(int32 Physics, int32 Society, int32 Engineering)
{
	const TMap<FName, int32> Weights = { { FName(TEXT("Physics")), Physics }, { FName(TEXT("Society")), Society }, { FName(TEXT("Engineering")), Engineering } };
	if (!SetResearchShares(Weights))
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicShares: %d %d %d was refused (no embodied ruler, the same shares, weights outside 0..%d or all 0, or an ended run)"),
			Physics, Society, Engineering, CosmicResearch::MaxShareWeight);
	}
}

void ACosmicPlayerController::CosmicEdictWindow(int32 Page)
{
	ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>();
	UCosmicEdictWidget* Window = CosmicHUD ? CosmicHUD->GetEdictWindow() : nullptr;
	if (!Window)
	{
		return;
	}
	if (Page >= 1 && Page <= 3)
	{
		Window->Open(Page == 1 ? ECosmicEdictTab::Edicts : (Page == 2 ? ECosmicEdictTab::Shares : ECosmicEdictTab::Peoples));
	}
	else
	{
		Window->Close();
	}
}

void ACosmicPlayerController::CosmicStance(const FString& CivID, const FString& Word)
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return;
	}
	if (IsRunEnded())
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicStance: refused (the run is over)"));
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	const int32 Ruled = (State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler && !State.Player.RuledCivID.IsNone())
		? Sim->GetStateIndex().FindCiv(State, State.Player.RuledCivID) : INDEX_NONE;
	const FCosmicKnownView View = GetKnownView();
	auto ReasonOf = [&](FName Other, FGameplayTag Stance)
	{
		return static_cast<int32>(State.Civs.IsValidIndex(Ruled) ? CosmicDiplomacy::StanceBlockOf(State, Sim->GetStateIndex(), Sim->GetRules(), Ruled, Other, Stance,
			View.IsMet(Sim->GetStateIndex().FindCiv(State, Other))) : CosmicDiplomacy::ECosmicStanceBlock::NotRuler);
	};
	// No argument: one line with every people and the gate's answer of each stance (friendly / neutral / hostile).
	if (CivID.IsEmpty())
	{
		TArray<FString> Peoples;
		for (const FCivilizationState& Civ : State.Civs)
		{
			Peoples.Add(FString::Printf(TEXT("%s (%d/%d/%d)"), *Civ.CivID.ToString(), ReasonOf(Civ.CivID, CosmicTags::StanceFriendly()), ReasonOf(Civ.CivID, FGameplayTag()),
				ReasonOf(Civ.CivID, CosmicTags::StanceHostile())));
		}
		UE_LOG(LogCosmic, Display, TEXT("CosmicStance: the peoples (the gate's reason for friendly/neutral/hostile - 0 = taken): %s"), *FString::Join(Peoples, TEXT(", ")));
		return;
	}
	FGameplayTag Stance;
	if (Word.Equals(TEXT("friendly"), ESearchCase::IgnoreCase))
	{
		Stance = CosmicTags::StanceFriendly();
	}
	else if (Word.Equals(TEXT("hostile"), ESearchCase::IgnoreCase))
	{
		Stance = CosmicTags::StanceHostile();
	}
	else if (!Word.Equals(TEXT("neutral"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicStance: '%s' is no stance (friendly, neutral or hostile)"), *Word);
		return;
	}
	const FName Other(*CivID);
	if (!SetStance(Other, Stance))
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicStance: %s %s was refused (reason %d)"), *CivID, *Word, ReasonOf(Other, Stance));
	}
}

void ACosmicPlayerController::CosmicTreaty(const FString& CivID, const FString& Word, const FString& Break)
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return;
	}
	if (IsRunEnded())
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicTreaty: refused (the run is over)"));
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	const int32 Ruled = (State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler && !State.Player.RuledCivID.IsNone())
		? Sim->GetStateIndex().FindCiv(State, State.Player.RuledCivID) : INDEX_NONE;
	const FCosmicKnownView View = GetKnownView();
	auto ReasonOf = [&](FName Other, FGameplayTag Treaty)
	{
		return static_cast<int32>(State.Civs.IsValidIndex(Ruled) ? CosmicDiplomacy::TreatyBlockOf(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), Ruled, Other,
			Treaty, View.IsMet(Sim->GetStateIndex().FindCiv(State, Other))) : CosmicDiplomacy::ECosmicTreatyBlock::NotRuler);
	};
	// No argument: one line with every people and the gate's answer of each treaty (trade / non-aggression).
	if (CivID.IsEmpty())
	{
		TArray<FString> Peoples;
		for (const FCivilizationState& Civ : State.Civs)
		{
			Peoples.Add(FString::Printf(TEXT("%s (%d/%d)"), *Civ.CivID.ToString(), ReasonOf(Civ.CivID, CosmicTags::TreatyTrade()), ReasonOf(Civ.CivID, CosmicTags::TreatyNonAggression())));
		}
		UE_LOG(LogCosmic, Display, TEXT("CosmicTreaty: the peoples (the gate's reason for trade/non-aggression - 0 = taken, 5 = held): %s"), *FString::Join(Peoples, TEXT(", ")));
		return;
	}
	FGameplayTag Treaty;
	if (Word.Equals(TEXT("trade"), ESearchCase::IgnoreCase))
	{
		Treaty = CosmicTags::TreatyTrade();
	}
	else if (Word.Equals(TEXT("nonaggression"), ESearchCase::IgnoreCase))
	{
		Treaty = CosmicTags::TreatyNonAggression();
	}
	else
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicTreaty: '%s' is no treaty (trade or nonaggression)"), *Word);
		return;
	}
	const FName Other(*CivID);
	if (Break.Equals(TEXT("break"), ESearchCase::IgnoreCase))
	{
		if (!BreakTreaty(Other, Treaty))
		{
			UE_LOG(LogCosmic, Display, TEXT("CosmicTreaty: the breaking of %s with %s was refused (not held, no embodied ruler)"), *Word, *CivID);
		}
	}
	else if (!ProposeTreaty(Other, Treaty))
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicTreaty: %s with %s was refused (reason %d)"), *Word, *CivID, ReasonOf(Other, Treaty));
	}
}

void ACosmicPlayerController::CosmicFleet(const FString& SystemID)
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return;
	}
	if (IsRunEnded())
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicFleet: refused (the run is over)"));
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const int32 Ruled = (State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler && !State.Player.RuledCivID.IsNone())
		? Index.FindCiv(State, State.Player.RuledCivID) : INDEX_NONE;
	// The gate the command asks: a guard of an own system, a siege of another's - both in the siege's names, as the command
	// answers them.
	auto ReasonOf = [&](int32 SystemIndex)
	{
		if (!State.Civs.IsValidIndex(Ruled))
		{
			return static_cast<int32>(CosmicConflict::ECosmicSiegeBlock::NotRuler);
		}
		if (State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == State.Civs[Ruled].CivID)
		{
			return static_cast<int32>(CosmicConflict::SiegeNameOf(CosmicConflict::GuardBlockOf(State, Index, Sim->GetRules(), Ruled, SystemIndex)));
		}
		return static_cast<int32>(CosmicConflict::SiegeBlockOf(State, Index, Sim->GetRules(), Ruled, SystemIndex));
	};
	// No argument: one line with the fleet, its target and every settled system of another people with the siege's answer.
	if (SystemID.IsEmpty())
	{
		TArray<FString> Systems;
		for (int32 SystemIndex = 0; State.Civs.IsValidIndex(Ruled) && SystemIndex < State.Systems.Num(); ++SystemIndex)
		{
			const FStarSystemState& System = State.Systems[SystemIndex];
			if (!System.OwnerCivID.IsNone() && System.OwnerCivID != State.Civs[Ruled].CivID && CosmicState::IsSettled(System))
			{
				Systems.Add(FString::Printf(TEXT("%s (%d)"), *System.SystemID.ToString(), ReasonOf(SystemIndex)));
			}
		}
		UE_LOG(LogCosmic, Display, TEXT("CosmicFleet: fleet %.3f, target %s; the systems of other peoples (the siege gate's reason - 0 = taken): %s"),
			State.Civs.IsValidIndex(Ruled) ? State.Civs[Ruled].FleetPower : 0.f, State.Civs.IsValidIndex(Ruled) ? *State.Civs[Ruled].FleetTargetSystemID.ToString() : TEXT("-"),
			*FString::Join(Systems, TEXT(", ")));
		return;
	}
	const FName Target = SystemID.Equals(TEXT("none"), ESearchCase::IgnoreCase) ? FName() : FName(*SystemID);
	if (!AllocateFleet(Target))
	{
		UE_LOG(LogCosmic, Display, TEXT("CosmicFleet: %s was refused (reason %d)"), *SystemID, ReasonOf(Index.FindSystem(State, Target)));
	}
}

void ACosmicPlayerController::SetSpeed(float Speed)
{
	ApplySpeed(Speed, ECosmicAutoPause::None);
}

void ACosmicPlayerController::ApplySpeed(float Speed, ECosmicAutoPause Reason)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return;
	}
	const float Current = Sim->GetTimeScale();
	if (Current > 0.f)
	{
		SpeedBeforePause = Current;
	}
	// The run is over (MVP item 4): every path - the bar, the keys, the presentation's and the card's give-back, the gate's
	// lowering - stops here.
	if (IsRunEnded())
	{
		Speed = 0.f;
		Reason = ECosmicAutoPause::RunEnd;
	}
	// A calling page stops the time until its own buttons turn it (B5 header section 4, decision 16): a give-back - the
	// presentation's, the card's, the silence's - and a speed set under it wait, and the page's reason stays (a plain stop
	// under it - the pause key - keeps it too; only a reason of its own replaces it).
	else if (IsCallingPageOpen() && (Speed > 0.f || Reason == ECosmicAutoPause::None))
	{
		Speed = 0.f;
		Reason = AutoPauseReason;
	}
	// One rule for every path: a speed the third rung's gate does not open runs as x10.
	Sim->SetTimeScale(CosmicUI::OpenSpeed(Speed, IsSpiritualSightOpen()));
	// Set before the redraw: the top bar shows why the simulation stands still.
	AutoPauseReason = Reason;

	// No delegate announces a new time scale, and a paused simulation sends no tick.
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
}

TOptional<float> ACosmicPlayerController::SpeedOfKey(const FKey& Key)
{
	// The number keys in the order of the speed steps, pause first.
	const FKey NumberKeys[] = { EKeys::Zero, EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five };
	static_assert(UE_ARRAY_COUNT(NumberKeys) == UE_ARRAY_COUNT(CosmicUI::SpeedSteps), "One number key per speed step, 0 to 5");
	for (int32 At = 0; At < static_cast<int32>(UE_ARRAY_COUNT(NumberKeys)); ++At)
	{
		if (Key == NumberKeys[At])
		{
			return CosmicUI::SpeedSteps[At];
		}
	}
	return TOptional<float>();
}

void ACosmicPlayerController::PressSpeedKey(FKey Key)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	// A calling page turns with its own buttons (B5): the time under it waits for them.
	if (!Sim || IsCallingPageOpen())
	{
		return;
	}
	if (Key == EKeys::SpaceBar)
	{
		SetSpeed(Sim->GetTimeScale() > 0.f ? 0.f : SpeedBeforePause);
		return;
	}
	// A key whose step the gate does not open does nothing (it never becomes x10). The keys follow the gate alone: under
	// the ascension presentation key 5 sets x25 while the bar keeps that button folded, as keys 1 to 4 set theirs.
	const TOptional<float> Step = SpeedOfKey(Key);
	if (Step.IsSet() && CosmicUI::IsSpeedOpen(Step.GetValue(), IsSpiritualSightOpen()))
	{
		SetSpeed(Step.GetValue());
	}
}

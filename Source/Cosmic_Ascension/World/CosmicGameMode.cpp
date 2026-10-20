// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicGameMode.h"

#include "Blueprint/UserWidget.h"
#include "Components/InputComponent.h"
#include "CoreGlobals.h"
#include "Core/GalaxyState.h"
#include "Cosmic_Ascension.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "Log/CosmicLogSubsystem.h"
#include "Math/RotationMatrix.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/DateTime.h"
#include "Misc/Parse.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "TimerManager.h"
#include "UI/CosmicAscensionOverlayWidget.h"
#include "UI/CosmicGalaxyPanelWidget.h"
#include "UI/CosmicLogFeedWidget.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "Widgets/Layout/Anchors.h"
#include "World/CosmicCameraPawn.h"
#include "World/CosmicGalaxyMapActor.h"
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
	constexpr float ScreenMargin = 16.f;

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
	if (Sim && !Sim->GetState().GalaxyID.IsValid())
	{
		// The seed of a new galaxy is an input to the simulation, not a draw from its stream.
		int32 Seed = 0;
		if (!FParse::Value(FCommandLine::Get(), TEXT("Seed="), Seed))
		{
			Seed = static_cast<int32>(FDateTime::UtcNow().GetTicks() & 0x7fffffff);
		}
		// The counts come from the home galaxy row (section 3.1). A development aid: -Speed=N starts at that speed, up to
		// x25 whatever the rung (the frame-time measure: item 6 at 10x, item 12 at 25x - the controller lowers only a x25
		// the gate opened and then closed).
		Sim->NewGalaxy(Seed, 0, 0);
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
	// Over everything, the whole screen; hidden until an ascension.
	Overlay = CreateWidget<UCosmicAscensionOverlayWidget>(Controller);
	if (Overlay)
	{
		Overlay->AddToViewport(10);
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
	FollowOwnHazards();
}

void ACosmicPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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

void ACosmicPlayerController::HandleSimTick(int64 Tick)
{
	FollowSpeedGate();
	FollowOwnHazards();
}

void ACosmicPlayerController::HandleGalaxyGenerated(int32 Seed)
{
	FollowSpeedGate();
	// Another game: what stopped the old one and the hazards it had say nothing of this one.
	AutoPauseReason = ECosmicAutoPause::None;
	OwnHazardsSeen.Reset();
	FollowOwnHazards();
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
	TMap<FName, FGameplayTag> Now = CosmicAutoPause::OwnHazardsOf(Sim->GetState(), Sim->GetStateIndex());
	const bool bNew = OwnHazardsSeen.IsSet() && CosmicAutoPause::HasNewHazard(OwnHazardsSeen.GetValue(), Now);
	OwnHazardsSeen = MoveTemp(Now);
	if (bNew)
	{
		RequestAutoPause(ECosmicAutoPause::OwnHazard);
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

void ACosmicPlayerController::RequestAutoPause(ECosmicAutoPause Reason)
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!bAutoPause || Reason == ECosmicAutoPause::None || !Sim || !(Sim->GetTimeScale() > 0.f))
	{
		return;
	}
	ApplySpeed(0.f, Reason);
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

void ACosmicPlayerController::SelectSystem(FName SystemID)
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
	// Noted before the announcement: the HUD redraws the hint from it.
	if (!SystemID.IsNone())
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

bool ACosmicPlayerController::SelectAndFocus(FName SystemID)
{
	const ACosmicGalaxyMapActor* TheMap = Map.Get();
	const ACosmicStarSystemActor* Star = (TheMap && !SystemID.IsNone()) ? TheMap->FindStar(SystemID) : nullptr;
	if (!Star)
	{
		return false;
	}
	SelectSystem(SystemID);
	// Refused (a system the viewer does not know): the camera stays (section 8.1 - it goes to a known system only).
	if (SelectedSystemID != SystemID)
	{
		return false;
	}
	if (ACosmicCameraPawn* CameraPawn = GetCameraPawn())
	{
		// From the universe too: a system is looked at in the region.
		CameraPawn->FocusOn(Star->GetActorLocation());
	}
	FollowZoomLevel();
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
	if (ACosmicCameraPawn* CameraPawn = GetCameraPawn())
	{
		CameraPawn->Zoom(Key == EKeys::MouseScrollUp ? 1.f : -1.f);
	}
	FollowZoomLevel();
}

bool ACosmicPlayerController::StartProject(FName SystemID, FName RowName)
{
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Roles || !Roles->StartMegaproject(SystemID, RowName))
	{
		return false;
	}
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
	if (!Sim)
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

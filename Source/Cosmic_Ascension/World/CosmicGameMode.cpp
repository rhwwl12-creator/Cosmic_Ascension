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
#include "Math/RotationMatrix.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/DateTime.h"
#include "Misc/Parse.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "TimerManager.h"
#include "UI/CosmicAscensionOverlayWidget.h"
#include "UI/CosmicLogFeedWidget.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "Widgets/Layout/Anchors.h"
#include "World/CosmicCameraPawn.h"
#include "World/CosmicGalaxyMapActor.h"
#include "World/CosmicMapLook.h"
#include "World/CosmicMapPick.h"
#include "World/CosmicStarSystemActor.h"

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
		if (ACosmicPlayerController* Cosmic = Cast<ACosmicPlayerController>(Controller))
		{
			SelectionHandle = Cosmic->OnSelectionChanged.AddUObject(this, &ACosmicHUD::HandleSelectionChanged);
			SystemPanel->ShowSystem(Cosmic->GetSelectedSystem());
		}
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
		Cosmic->OnTimeUnitChanged.Remove(TimeUnitHandle);
		Cosmic->OnViewModeChanged.Remove(ViewModeHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void ACosmicHUD::HandleSelectionChanged(FName SystemID)
{
	if (SystemPanel)
	{
		SystemPanel->ShowSystem(SystemID);
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
	if (SystemPanel)
	{
		SystemPanel->Refresh();
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
	// The third rung's gate is followed at the tick boundary and on a new galaxy or a load (FollowSpeedGate).
	if (UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr)
	{
		Sim->OnSimTick.AddUniqueDynamic(this, &ACosmicPlayerController::HandleSimTick);
		Sim->OnGalaxyGenerated.AddUniqueDynamic(this, &ACosmicPlayerController::HandleGalaxyGenerated);
	}
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
	Super::EndPlay(EndPlayReason);
}

void ACosmicPlayerController::HandleSimTick(int64 Tick)
{
	FollowSpeedGate();
}

void ACosmicPlayerController::HandleGalaxyGenerated(int32 Seed)
{
	FollowSpeedGate();
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
	}
}

void ACosmicPlayerController::SelectSystem(FName SystemID)
{
	if (SystemID == SelectedSystemID)
	{
		return;
	}
	if (!SystemID.IsNone())
	{
		const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
		const bool bHeld = Sim && Sim->GetState().Systems.ContainsByPredicate([SystemID](const FStarSystemState& System) { return System.SystemID == SystemID; });
		if (!bHeld)
		{
			return;
		}
	}
	SelectedSystemID = SystemID;
	if (ACosmicGalaxyMapActor* TheMap = Map.Get())
	{
		TheMap->SetSelectedSystem(SystemID);
	}
	OnSelectionChanged.Broadcast(SystemID);
}

void ACosmicPlayerController::CosmicSelect(const FString& SystemID)
{
	const FName Wanted(*SystemID);
	const ACosmicGalaxyMapActor* TheMap = Map.Get();
	// A map still to be built (or rebuilt) holds the old galaxy's stars: wait for the build.
	if (!TheMap || TheMap->IsRebuildPending() || TheMap->GetStars().Num() == 0 || !SelectAndFocus(Wanted))
	{
		PendingSelect = Wanted;
	}
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
	if (ACosmicCameraPawn* CameraPawn = GetCameraPawn())
	{
		CameraPawn->FocusOn(Star->GetActorLocation());
	}
	return true;
}

void ACosmicPlayerController::HandleMapRebuilt()
{
	const ACosmicGalaxyMapActor* TheMap = Map.Get();
	SelectSystem(NAME_None);
	// A new galaxy or a load starts at the default view (the map put its own back before it drew; this announces it).
	SetWantedViewMode(ECosmicViewMode::Spiritual);
	if (ACosmicCameraPawn* CameraPawn = GetCameraPawn(); CameraPawn && TheMap)
	{
		CameraPawn->FrameAll(TheMap->GetDiscCentre(), TheMap->GetDiscRadius());
		CameraPawn->SnapToTargets();
	}
	if (!PendingSelect.IsNone())
	{
		const FName Wanted = PendingSelect;
		PendingSelect = NAME_None;
		SelectAndFocus(Wanted);
	}
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
}

bool ACosmicPlayerController::DiscPointAt(const FVector2D& Screen, FVector& OutPoint) const
{
	FVector Origin;
	FVector Direction;
	return DeprojectScreenPositionToWorld(Screen.X, Screen.Y, Origin, Direction) && CosmicMapPick::OntoDisc(Origin, Direction, OutPoint);
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
	// A star's drawn radius: its centre and a point its radius to the side of the view, both projected.
	FVector ViewLocation;
	FRotator ViewRotation;
	GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector Side = FRotationMatrix(ViewRotation).GetUnitAxis(EAxis::Y);

	TArray<FCosmicScreenTarget> Targets;
	TArray<FName> IDs;
	for (const ACosmicStarSystemActor* Star : TheMap->GetStars())
	{
		FVector2D Centre;
		FVector2D Edge;
		if (Star && ProjectWorldLocationToScreen(Star->GetActorLocation(), Centre) && ProjectWorldLocationToScreen(Star->GetActorLocation() + Side * Star->GetStarRadius(), Edge))
		{
			Targets.Add({ Centre, static_cast<float>(FVector2D::Distance(Centre, Edge)) });
			IDs.Add(Star->GetSystemID());
		}
	}
	const int32 Index = CosmicMapPick::NearestOnScreen(Targets, FVector2D(MouseX, MouseY), CosmicControlImpl::PickPixels);
	return IDs.IsValidIndex(Index) ? IDs[Index] : NAME_None;
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
}

void ACosmicPlayerController::HandleLeftReleased(FKey Key)
{
	// A press that did not become a pan is a click: the star under the cursor, or empty space (clears).
	if (bLeftDown && !bPanning)
	{
		SelectSystem(PickAtCursor());
	}
	bLeftDown = false;
	bPanning = false;
}

void ACosmicPlayerController::HandleLeftDoubleClick(FKey Key)
{
	SelectAndFocus(PickAtCursor());
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

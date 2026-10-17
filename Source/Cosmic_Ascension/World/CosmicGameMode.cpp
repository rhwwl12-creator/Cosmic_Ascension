// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicGameMode.h"

#include "Components/InputComponent.h"
#include "Cosmic_Ascension.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Math/RotationMatrix.h"
#include "Misc/CommandLine.h"
#include "Misc/DateTime.h"
#include "Misc/Parse.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "TimerManager.h"
#include "UI/CosmicLogFeedWidget.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "Widgets/Layout/Anchors.h"
#include "World/CosmicCameraPawn.h"
#include "World/CosmicGalaxyMapActor.h"
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
		// The counts come from the home galaxy row (section 3.1).
		Sim->NewGalaxy(Seed, 0, 0);
		Sim->SetTimeScale(1.f);
	}

	Super::StartPlay();

	if (UWorld* World = GetWorld())
	{
		// A development aid: -ShotAfterSeconds=N takes one screenshot N seconds in (the first frame of an off-screen run
		// still compiles shaders). See CLAUDE.md on looking at the screen without a window.
		float ShotAfter = 0.f;
		if (FParse::Value(FCommandLine::Get(), TEXT("ShotAfterSeconds="), ShotAfter) && ShotAfter > 0.f)
		{
			FTimerHandle Shot;
			World->GetTimerManager().SetTimer(Shot, FTimerDelegate::CreateWeakLambda(World, [World]()
			{
				// Typed as at the console: the player's console reaches the viewport that handles "shot".
				if (APlayerController* Controller = World->GetFirstPlayerController())
				{
					Controller->ConsoleCommand(TEXT("shot showui"));
				}
			}), ShotAfter, false);
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
}

void ACosmicHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ACosmicPlayerController* Cosmic = Cast<ACosmicPlayerController>(GetOwningPlayerController()))
	{
		Cosmic->OnSelectionChanged.Remove(SelectionHandle);
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
}

void ACosmicPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ACosmicGalaxyMapActor* TheMap = Map.Get())
	{
		TheMap->OnRebuilt.Remove(RebuiltHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void ACosmicPlayerController::UseMap(ACosmicGalaxyMapActor* InMap)
{
	if (ACosmicGalaxyMapActor* Old = Map.Get())
	{
		Old->OnRebuilt.Remove(RebuiltHandle);
	}
	Map = InMap;
	RebuiltHandle = InMap ? InMap->OnRebuilt.AddUObject(this, &ACosmicPlayerController::HandleMapRebuilt) : FDelegateHandle();
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

	for (const FKey& Key : { EKeys::Zero, EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::SpaceBar })
	{
		InputComponent->BindKey(Key, IE_Pressed, this, &ACosmicPlayerController::HandleSpeedKey);
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
	Sim->SetTimeScale(Speed);

	// No delegate announces a new time scale, and a paused simulation sends no tick.
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
}

void ACosmicPlayerController::HandleSpeedKey(FKey Key)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return;
	}

	const float Current = Sim->GetTimeScale();
	float Wanted = Current;
	if (Key == EKeys::SpaceBar)
	{
		Wanted = (Current > 0.f) ? 0.f : SpeedBeforePause;
	}
	else if (Key == EKeys::Zero)
	{
		Wanted = 0.f;
	}
	else if (Key == EKeys::One)
	{
		Wanted = 1.f;
	}
	else if (Key == EKeys::Two)
	{
		Wanted = 2.f;
	}
	else if (Key == EKeys::Three)
	{
		Wanted = 5.f;
	}
	else if (Key == EKeys::Four)
	{
		Wanted = 10.f;
	}
	SetSpeed(Wanted);
}

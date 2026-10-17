// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicGameMode.h"

#include "Components/InputComponent.h"
#include "Cosmic_Ascension.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/CommandLine.h"
#include "Misc/DateTime.h"
#include "Misc/Parse.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UI/CosmicTextPanelWidget.h"
#include "World/CosmicGalaxyMapActor.h"

ACosmicGameMode::ACosmicGameMode()
{
	HUDClass = ACosmicHUD::StaticClass();
	PlayerControllerClass = ACosmicPlayerController::StaticClass();
	// The map has nothing to fly through yet; the camera pawn arrives with Phase 3.
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

	// The map reads the state from here on; until the camera pawn it is also the view.
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

		ACosmicGalaxyMapActor* Map = World->SpawnActor<ACosmicGalaxyMapActor>();
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (APlayerController* Controller = It->Get(); Controller && Map)
			{
				Controller->SetViewTarget(Map);
			}
		}
	}
}

void ACosmicHUD::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* Controller = GetOwningPlayerController())
	{
		Panel = CreateWidget<UCosmicTextPanelWidget>(Controller);
		if (Panel)
		{
			Panel->AddToViewport();
		}
	}
}

void ACosmicHUD::RefreshPanel()
{
	if (Panel)
	{
		Panel->Refresh();
	}
}

ACosmicPlayerController::ACosmicPlayerController()
{
	bShowMouseCursor = true;
}

void ACosmicPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	for (const FKey& Key : { EKeys::Zero, EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::SpaceBar })
	{
		InputComponent->BindKey(Key, IE_Pressed, this, &ACosmicPlayerController::HandleSpeedKey);
	}
	InputComponent->BindKey(EKeys::P, IE_Pressed, this, &ACosmicPlayerController::HandleProjectKey);
}

void ACosmicPlayerController::HandleProjectKey(FKey Key)
{
	UPlayerRoleSubsystem* Roles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (Roles && Roles->StartNextMegaproject())
	{
		// A paused simulation sends no tick: show the new project now.
		if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
		{
			CosmicHUD->RefreshPanel();
		}
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

	if (Current > 0.f)
	{
		SpeedBeforePause = Current;
	}
	Sim->SetTimeScale(Wanted);

	// No delegate announces a new time scale, and a paused simulation sends no tick.
	if (ACosmicHUD* CosmicHUD = GetHUD<ACosmicHUD>())
	{
		CosmicHUD->RefreshPanel();
	}
}

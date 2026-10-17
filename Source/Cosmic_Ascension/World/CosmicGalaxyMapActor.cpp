// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicGalaxyMapActor.h"

#include "Camera/CameraComponent.h"
#include "Components/LineBatchComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Cosmic_Ascension.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "SceneTypes.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "World/CosmicMapLook.h"
#include "World/CosmicStarSystemActor.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicGalaxyMapImpl
{
	const TCHAR* const NebulaMaterialPath = TEXT("/Game/CA/Materials/M_Nebula.M_Nebula");
	const FLinearColor LaneColor(0.25f, 0.4f, 0.65f, 1.f);
	// Thicknesses are in world units: from the overview one pixel is about 25 units, and a thinner line breaks into dashes.
	constexpr float LaneThickness = 30.f;
	constexpr float RingThickness = 45.f;
	// The owner's ring: this far outside the star, in this many segments (a settlement under way draws every other one).
	constexpr float RingGap = 110.f;
	constexpr int32 RingSegments = 24;
	// The sky sphere: the engine sphere (100 units across) blown up far past the disc of any galaxy size.
	constexpr float SkyScale = 3000.f;
	// The overview camera of item 1: above and behind the disc of the home galaxy (radius about 7,700), looking at its
	// centre, the whole disc in view. The camera pawn of item 2 replaces it.
	const FVector OverviewLocation(-9000.f, 0.f, 15000.f);
}

ACosmicGalaxyMapActor::ACosmicGalaxyMapActor()
{
	using namespace CosmicGalaxyMapImpl;

	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Lanes = CreateDefaultSubobject<ULineBatchComponent>(TEXT("Lanes"));
	Lanes->SetupAttachment(RootComponent);

	Sky = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sky"));
	Sky->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Sphere.Succeeded())
	{
		Sky->SetStaticMesh(Sphere.Object);
	}
	Sky->SetRelativeScale3D(FVector(SkyScale));
	Sky->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Sky->SetCastShadow(false);

	Overview = CreateDefaultSubobject<UCameraComponent>(TEXT("Overview"));
	Overview->SetupAttachment(RootComponent);
	Overview->SetRelativeLocation(OverviewLocation);
	Overview->SetRelativeRotation((-OverviewLocation).Rotation());
	// Emissive stars on a dark sky: a fixed exposure keeps them from flaring and fading as the view moves.
	Overview->PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
	Overview->PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
	Overview->PostProcessSettings.AutoExposureMinBrightness = 1.f;
	Overview->PostProcessSettings.AutoExposureMaxBrightness = 1.f;
}

ACosmicStarSystemActor* ACosmicGalaxyMapActor::FindStar(FName SystemID) const
{
	for (ACosmicStarSystemActor* Star : Stars)
	{
		if (Star && Star->GetSystemID() == SystemID)
		{
			return Star;
		}
	}
	return nullptr;
}

void ACosmicGalaxyMapActor::BeginPlay()
{
	Super::BeginPlay();
	// Loaded here, not by a constructor finder, for the reason ACosmicStarSystemActor::BeginPlay gives.
	if (UMaterialInterface* Nebula = LoadObject<UMaterialInterface>(nullptr, CosmicGalaxyMapImpl::NebulaMaterialPath, nullptr, LOAD_NoWarn))
	{
		Sky->SetMaterial(0, Nebula);
	}
	else
	{
		UE_LOG(LogCosmic, Error, TEXT("%s is missing: run Tools/make_materials.py"), CosmicGalaxyMapImpl::NebulaMaterialPath);
	}
	if (UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr)
	{
		Sim->OnGalaxyGenerated.AddUniqueDynamic(this, &ACosmicGalaxyMapActor::HandleGalaxyGenerated);
		Sim->OnSimTick.AddUniqueDynamic(this, &ACosmicGalaxyMapActor::HandleSimTick);
	}
	bRebuildPending = true;
}

void ACosmicGalaxyMapActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr)
	{
		Sim->OnGalaxyGenerated.RemoveDynamic(this, &ACosmicGalaxyMapActor::HandleGalaxyGenerated);
		Sim->OnSimTick.RemoveDynamic(this, &ACosmicGalaxyMapActor::HandleSimTick);
	}
	Super::EndPlay(EndPlayReason);
}

void ACosmicGalaxyMapActor::HandleGalaxyGenerated(int32 Seed)
{
	bRebuildPending = true;
}

void ACosmicGalaxyMapActor::HandleSimTick(int64 Tick)
{
	bRefreshPending = true;
}

void ACosmicGalaxyMapActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	RefreshNow();
}

void ACosmicGalaxyMapActor::RefreshNow()
{
	if (bRebuildPending)
	{
		Rebuild();
	}
	else if (bRefreshPending)
	{
		Refresh();
	}
}

void ACosmicGalaxyMapActor::Rebuild()
{
	bRebuildPending = false;
	for (ACosmicStarSystemActor* Star : Stars)
	{
		if (Star)
		{
			Star->Destroy();
		}
	}
	Stars.Reset();

	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	UWorld* World = GetWorld();
	if (Sim && World)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		for (const FStarSystemState& System : Sim->GetState().Systems)
		{
			ACosmicStarSystemActor* Star = World->SpawnActor<ACosmicStarSystemActor>(ACosmicStarSystemActor::StaticClass(), System.Position, FRotator::ZeroRotator, Params);
			if (Star)
			{
				Star->Bind(System.SystemID);
			}
			Stars.Add(Star);
		}
	}
	Refresh();
}

void ACosmicGalaxyMapActor::Refresh()
{
	using namespace CosmicGalaxyMapImpl;

	bRefreshPending = false;
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	TArray<FBatchedLine> Lines;
	for (const TPair<int32, int32>& Lane : CosmicMapLook::LanesOf(State))
	{
		Lines.Emplace(State.Systems[Lane.Key].Position, State.Systems[Lane.Value].Position, LaneColor, 0.f, LaneThickness, SDPG_World);
	}
	DrawnLanes = Lines.Num();

	// Systems are never removed (a destroyed one stays as a dead node), so the actors match the systems by index.
	DrawnRings = 0;
	for (int32 Index = 0; Index < Stars.Num() && Index < State.Systems.Num(); ++Index)
	{
		ACosmicStarSystemActor* Star = Stars[Index];
		if (!Star)
		{
			continue;
		}
		const FCosmicSystemLook Look = CosmicMapLook::LookOf(State, Sim->GetTables(), Index);
		Star->ApplyLook(Look);
		if (!Look.bOwned)
		{
			continue;
		}
		// In the plane of the disc, around the star as this look scaled it.
		const FVector Centre = Star->GetActorLocation();
		const float Radius = Star->GetStarRadius() + RingGap;
		for (int32 Segment = 0; Segment < RingSegments; ++Segment)
		{
			if (Look.bSettling && Segment % 2 == 1)
			{
				continue;
			}
			const float From = UE_TWO_PI * Segment / RingSegments;
			const float To = UE_TWO_PI * (Segment + 1) / RingSegments;
			Lines.Emplace(Centre + Radius * FVector(FMath::Cos(From), FMath::Sin(From), 0.f), Centre + Radius * FVector(FMath::Cos(To), FMath::Sin(To), 0.f),
				Look.OwnerColor, 0.f, RingThickness, SDPG_World);
		}
		++DrawnRings;
	}

	Lanes->Flush();
	Lanes->DrawLines(Lines);
}

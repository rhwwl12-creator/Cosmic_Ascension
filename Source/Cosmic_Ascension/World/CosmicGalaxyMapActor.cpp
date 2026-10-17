// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicGalaxyMapActor.h"

#include "Components/LineBatchComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Cosmic_Ascension.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Math/Box2D.h"
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
	// White, one of the Severity colours (guideline 8.2).
	const FLinearColor SelectionColor(0.95f, 0.95f, 1.f, 1.f);
	// Thicknesses are in world units: a lane is the view distance x this, about 1.2 px on screen at any zoom (a thinner
	// line breaks into dashes, a fixed one is 13 px thick from close by). The rings are thicker by RingFactor.
	constexpr float LaneThicknessPerDistance = 0.0018f;
	constexpr float RingFactor = 1.5f;
	// From further than this, labels thin out (CosmicMapLook::ShowsLabel).
	constexpr float FarViewDistance = 9000.f;
	// A new view distance redraws when it moved by more than this share.
	constexpr float ViewDistanceSlack = 0.05f;
	// The owner's ring: this far outside the star, in this many segments (a settlement under way draws every other one).
	constexpr float RingGap = 110.f;
	constexpr int32 RingSegments = 24;
	// The selection ring stands this many ring thicknesses outside the owner's ring.
	constexpr float SelectionGapRings = 4.f;
	constexpr int32 SelectionSegments = 32;
	// A disc smaller than this is taken as this (a board of a few systems).
	constexpr float MinDiscRadius = 1000.f;
	// The sky sphere: the engine sphere (100 units across) blown up far past the disc of any galaxy size.
	constexpr float SkyScale = 3000.f;

	void AddRing(TArray<FBatchedLine>& Lines, const FVector& Centre, float Radius, int32 Segments, bool bDashed, const FLinearColor& Color, float Thickness)
	{
		// In the plane of the disc.
		for (int32 Segment = 0; Segment < Segments; ++Segment)
		{
			if (bDashed && Segment % 2 == 1)
			{
				continue;
			}
			const float From = UE_TWO_PI * Segment / Segments;
			const float To = UE_TWO_PI * (Segment + 1) / Segments;
			Lines.Emplace(Centre + Radius * FVector(FMath::Cos(From), FMath::Sin(From), 0.f), Centre + Radius * FVector(FMath::Cos(To), FMath::Sin(To), 0.f),
				Color, 0.f, Thickness, SDPG_World);
		}
	}
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

void ACosmicGalaxyMapActor::SetSelectedSystem(FName SystemID)
{
	if (SystemID != SelectedSystemID)
	{
		SelectedSystemID = SystemID;
		bRefreshPending = true;
	}
}

void ACosmicGalaxyMapActor::SetViewDistance(float ArmLength)
{
	using namespace CosmicGalaxyMapImpl;

	// A compare, so a distance that is not a number changes nothing.
	if (!(ArmLength > 0.f))
	{
		return;
	}
	const bool bFar = ArmLength > FarViewDistance;
	if (bFar != bFarView || FMath::Abs(ArmLength - ViewDistance) > ViewDistanceSlack * ViewDistance)
	{
		ViewDistance = ArmLength;
		bFarView = bFar;
		bRefreshPending = true;
	}
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
	using namespace CosmicGalaxyMapImpl;

	bRebuildPending = false;
	for (ACosmicStarSystemActor* Star : Stars)
	{
		if (Star)
		{
			Star->Destroy();
		}
	}
	Stars.Reset();
	// A selection belongs to the galaxy it was made in.
	SelectedSystemID = NAME_None;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	UWorld* World = GetWorld();
	FBox2D Box(ForceInit);
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
			Box += FVector2D(System.Position.X, System.Position.Y);
		}
	}

	// The disc: the centre of the systems' box and the farthest system from it.
	DiscCentre = Box.bIsValid ? FVector(Box.GetCenter(), 0.0) : FVector::ZeroVector;
	double Farthest = 0.0;
	if (Sim)
	{
		for (const FStarSystemState& System : Sim->GetState().Systems)
		{
			Farthest = FMath::Max(Farthest, FVector::Dist2D(System.Position, DiscCentre));
		}
	}
	DiscRadius = Farthest > MinDiscRadius ? static_cast<float>(Farthest) : MinDiscRadius;

	Refresh();
	OnRebuilt.Broadcast();
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
	LaneThickness = ViewDistance * LaneThicknessPerDistance;
	const float RingThickness = RingFactor * LaneThickness;

	TArray<FBatchedLine> Lines;
	for (const TPair<int32, int32>& Lane : CosmicMapLook::LanesOf(State))
	{
		Lines.Emplace(State.Systems[Lane.Key].Position, State.Systems[Lane.Value].Position, LaneColor, 0.f, LaneThickness, SDPG_World);
	}
	DrawnLanes = Lines.Num();

	// Systems are never removed (a destroyed one stays as a dead node), so the actors match the systems by index.
	DrawnRings = 0;
	DrawnSelection = NAME_None;
	for (int32 Index = 0; Index < Stars.Num() && Index < State.Systems.Num(); ++Index)
	{
		ACosmicStarSystemActor* Star = Stars[Index];
		if (!Star)
		{
			continue;
		}
		const FCosmicSystemLook Look = CosmicMapLook::LookOf(State, Sim->GetTables(), Index);
		const bool bSelected = !SelectedSystemID.IsNone() && Star->GetSystemID() == SelectedSystemID;
		Star->ApplyLook(Look);
		Star->SetLabelVisible(CosmicMapLook::ShowsLabel(Look, bSelected, bFarView));

		// Around the star as this look scaled it.
		const FVector Centre = Star->GetActorLocation();
		const float OwnerRadius = Star->GetStarRadius() + RingGap;
		if (Look.bOwned)
		{
			AddRing(Lines, Centre, OwnerRadius, RingSegments, Look.bSettling, Look.OwnerColor, RingThickness);
			++DrawnRings;
		}
		if (bSelected)
		{
			AddRing(Lines, Centre, OwnerRadius + SelectionGapRings * RingThickness, SelectionSegments, false, SelectionColor, RingThickness);
			DrawnSelection = SelectedSystemID;
		}
	}

	Lanes->Flush();
	Lanes->DrawLines(Lines);
}

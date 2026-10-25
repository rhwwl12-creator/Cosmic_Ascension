// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicGalaxyMapActor.h"

#include "Blueprint/UserWidget.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/LineBatchComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Cosmic_Ascension.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Math/Box2D.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "SceneTypes.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UI/CosmicPalette.h"
#include "UI/CosmicSystemLabelWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "World/CosmicMapLook.h"
#include "World/CosmicStarSystemActor.h"
#include "World/CosmicUniverseLook.h"
#include "World/CosmicZoomLevels.h"

// The map's own column of the CSV profiler (CosmicMap/GameThread/Refresh): the frame-time measure reads a refresh by
// itself.
CSV_DEFINE_CATEGORY(CosmicMap, true);

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicGalaxyMapImpl
{
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
	// The Sealed ring of the spiritual view stands this many ring thicknesses outside the owner's ring, the selection
	// ring this many.
	constexpr float SealedGapRings = 2.f;
	constexpr float SelectionGapRings = 4.f;
	constexpr int32 SelectionSegments = 32;
	// The vessel's mark (B1): a diamond - a ring of four segments - whose corners stand this many times the selection
	// ring's radius out (its sides then pass outside that ring), or, while it travels, one this wide on its lane; that lane
	// drawn this many lanes thick.
	constexpr float VesselDiamondScale = 1.5f;
	constexpr int32 VesselSegments = 4;
	constexpr float VesselTravelRadius = 160.f;
	constexpr float VesselLaneFactor = 2.5f;
	// The sky sphere before the first rebuild: the engine sphere blown up to the smallest sky (CosmicUniverseLook::
	// MinSkyRadius); every rebuild sizes it to hold the universe (SkyRadiusOf).
	constexpr float SkyScale = CosmicUniverseLook::MinSkyRadius / CosmicUniverseLook::SkySphereRadius;
	// M_Nebula's and M_Backdrop's scalar, looked up once.
	const FName SkyIntensityName(CosmicMapLook::SkyIntensityParam);
	// The engine plane is 100 units across: an instance of a point Size across is scaled by Size / this.
	constexpr float PlaneSize = 100.f;
	// M_Backdrop's LayerFade, looked up once; a share that moved by more than this writes the fades again.
	const FName LayerFadeName(CosmicMapLook::LayerFadeParam);
	constexpr float FadeSlack = 0.005f;
	// A galaxy's label: one line of a name, wider than a system's for the longest names, as tall as the line so that the
	// name stands just above its anchor (a taller box would lift it onto the galaxy above - screenshot 1).
	const FVector2D GalaxyLabelSize(220.f, 20.f);

	// Two marks draw the same instance: the same row, place, size, aspect and turn (the name is not drawn by the mesh).
	bool SameInstance(const FCosmicGalaxyMark& A, const FCosmicGalaxyMark& B)
	{
		return A.Row == B.Row && A.Position.Equals(B.Position, 0.0) && A.Radius == B.Radius && A.Aspect == B.Aspect && A.TurnDegrees == B.TurnDegrees;
	}

	// The transform of a backdrop point: at its place, flat on the disc, Size across.
	FTransform PointTransform(const FCosmicBackdropPoint& Point)
	{
		const double Scale = Point.Size / PlaneSize;
		return FTransform(FQuat::Identity, Point.Position, FVector(Scale, Scale, 1.0));
	}

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

	// The backdrop: one instanced mesh of the engine plane (an engine asset our scripts never refill, so a constructor
	// finder may root it), its points and slots built by Rebuild.
	Backdrop = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Backdrop"));
	Backdrop->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Plane(CosmicMapLook::BackdropMeshPath);
	if (Plane.Succeeded())
	{
		Backdrop->SetStaticMesh(Plane.Object);
	}
	Backdrop->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Backdrop->SetCastShadow(false);
	Backdrop->SetCanEverAffectNavigation(false);
	// Before any instance is added: the setter resets all custom data to 0.
	Backdrop->SetNumCustomDataFloats(CosmicMapLook::BackdropDataFloats);

	// The universe (Phase 3 item 9): a second instanced mesh of the same plane, its own instance of M_Backdrop, built by
	// Rebuild when its key changes; hidden until the camera stands past the region.
	Universe = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Universe"));
	Universe->SetupAttachment(RootComponent);
	if (Plane.Succeeded())
	{
		Universe->SetStaticMesh(Plane.Object);
	}
	Universe->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Universe->SetCastShadow(false);
	Universe->SetCanEverAffectNavigation(false);
	Universe->SetNumCustomDataFloats(CosmicMapLook::BackdropDataFloats);
	Universe->SetVisibility(false);
}

void ACosmicGalaxyMapActor::SetSeeAll(bool bInSeeAll)
{
	// Only marked: the map redraws at most once a frame.
	if (bInSeeAll != bSeeAll)
	{
		bSeeAll = bInSeeAll;
		bRefreshPending = true;
	}
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
	// Where the camera stands between the levels: the fades follow at once (material values, no redraw); a flip of the
	// region's detail is a redraw.
	const float Share = CosmicZoom::UniverseShareOf(ArmLength, ZoomLevels);
	ApplyLayerFades(Share);
	if (CosmicZoom::ShowsRegionDetail(Share) != bRegionDetail)
	{
		bRefreshPending = true;
	}
	UniverseShare = Share;
	const bool bFar = ArmLength > FarViewDistance;
	if (bFar != bFarView || FMath::Abs(ArmLength - ViewDistance) > ViewDistanceSlack * ViewDistance)
	{
		ViewDistance = ArmLength;
		bFarView = bFar;
		bRefreshPending = true;
	}
	// At the universe level a galaxy's label follows the distance itself, not the one last redrawn: a label whose rule
	// turns inside the slack above asks a redraw (else a label near its threshold showed or not by the frame on which the
	// glide last redrew - the far stop's Small Magellanic Cloud did one run and not the next; Phase 3 item 9's review).
	if (!CosmicZoom::ShowsRegionDetail(Share) && ViewDistance != ArmLength)
	{
		for (const FCosmicGalaxyMark& Mark : Marks)
		{
			if (CosmicUniverseLook::ShowsGalaxyLabel(Mark, ViewDistance, false) != CosmicUniverseLook::ShowsGalaxyLabel(Mark, ArmLength, false))
			{
				ViewDistance = ArmLength;
				bRefreshPending = true;
				break;
			}
		}
	}
}

void ACosmicGalaxyMapActor::SetViewWanted(ECosmicViewMode Wanted, bool bForcedOpen)
{
	// Only marked: the map redraws at most once a frame.
	if (Wanted != WantedView || bForcedOpen != bSightForcedOpen)
	{
		WantedView = Wanted;
		bSightForcedOpen = bForcedOpen;
		bRefreshPending = true;
	}
}

void ACosmicGalaxyMapActor::BeginPlay()
{
	Super::BeginPlay();
	// Loaded here, not by a constructor finder, for the reason ACosmicStarSystemActor::BeginPlay gives. The sky draws
	// through its own instance, so that the view can set SpiritualIntensity.
	if (UMaterialInterface* Nebula = LoadObject<UMaterialInterface>(nullptr, CosmicMapLook::NebulaMaterialPath, nullptr, LOAD_NoWarn))
	{
		SkyMaterial = Sky->CreateAndSetMaterialInstanceDynamicFromMaterial(0, Nebula);
	}
	else
	{
		UE_LOG(LogCosmic, Error, TEXT("%s is missing: run Tools/make_materials.py"), CosmicMapLook::NebulaMaterialPath);
	}
	// The backdrop draws only through a material that can draw instanced meshes: without one the plane's own opaque
	// material (or, under -game, the engine's default for a missing usage) would lay a grey disc of quads over the lanes.
	UMaterialInterface* Dust = LoadObject<UMaterialInterface>(nullptr, CosmicMapLook::BackdropMaterialPath, nullptr, LOAD_NoWarn);
	bBackdropUsable = Dust && Dust->GetUsageByFlag(MATUSAGE_InstancedStaticMeshes);
	if (bBackdropUsable)
	{
		BackdropMaterial = Backdrop->CreateAndSetMaterialInstanceDynamicFromMaterial(0, Dust);
	}
	else
	{
		UE_LOG(LogCosmic, Error, TEXT("%s is missing or cannot draw instanced meshes: run Tools/make_materials.py (the backdrop stays hidden)"), CosmicMapLook::BackdropMaterialPath);
	}
	Backdrop->SetVisibility(bBackdropUsable);
	// The universe draws through its own instance of the same material (its LayerFade and SpiritualIntensity apart), and
	// only through a usable one (the same rule as the backdrop's).
	if (bBackdropUsable)
	{
		UniverseMaterial = Universe->CreateAndSetMaterialInstanceDynamicFromMaterial(0, Dust);
	}
	WrittenShare = -1.f;
	ApplyLayerFades(0.f);
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
	// A selection belongs to the galaxy it was made in. A new galaxy or a load starts at the default view - before the
	// first draw, so a third-rung load never shows one physical frame (the controller announces the reset after
	// OnRebuilt). The development force stays for the session.
	SelectedSystemID = NAME_None;
	SelectedGalaxyRow = NAME_None;
	WantedView = ECosmicViewMode::Spiritual;

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
	DiscRadius = Farthest > CosmicZoom::MinDiscRadius ? static_cast<float>(Farthest) : CosmicZoom::MinDiscRadius;

	if (Sim)
	{
		BuildBackdrop(Sim->GetState(), Sim->GetRules());
		BuildUniverse(Sim->GetRules(), Sim->GetTables());
	}
	else
	{
		ZoomLevels = CosmicZoom::LevelsOf(DiscCentre, DiscRadius, 0.f);
	}
	// A new galaxy or a load starts in the region, framed (the controller snaps the camera there after OnRebuilt): the
	// new stars are visible, both fades written for the region and the view distance the framing's.
	UniverseShare = 0.f;
	bRegionDetail = true;
	bStarsHidden = false;
	ViewDistance = CosmicZoom::FrameArmFactor * DiscRadius;
	bFarView = ViewDistance > FarViewDistance;
	WrittenShare = -1.f;
	ApplyLayerFades(0.f);
	Refresh();
	OnRebuilt.Broadcast();
}

void ACosmicGalaxyMapActor::BuildUniverse(const FCosmicRulesDef& Rules, const FCosmicDataTables& Tables)
{
	using namespace CosmicGalaxyMapImpl;

	// The frame, the marks, the reach and the levels at every rebuild (cheap).
	Frame = CosmicUniverseLook::FrameOf(DiscCentre, DiscRadius);
	Marks = CosmicUniverseLook::MarksOf(Tables, Rules.HomeGalaxyRow, Frame);
	const TArray<FName> Unplaced = CosmicUniverseLook::UnplacedRowsOf(Tables, Rules.HomeGalaxyRow);
	if (!Unplaced.IsEmpty())
	{
		FString Names;
		for (const FName Name : Unplaced)
		{
			Names += (Names.IsEmpty() ? TEXT("") : TEXT(", ")) + Name.ToString();
		}
		UE_LOG(LogCosmic, Warning, TEXT("CosmicMap: galaxy rows without a place are not drawn: %s"), *Names);
	}
	Reach = CosmicUniverseLook::ReachOf(Frame, Marks);
	ZoomLevels = CosmicZoom::LevelsOf(DiscCentre, DiscRadius, Reach);

	// One label per mark: the extra ones go, the missing ones are made (a component registered on a begun actor begins at
	// once, so its widget exists right after RegisterComponent).
	while (GalaxyLabels.Num() > Marks.Num())
	{
		if (UWidgetComponent* Extra = GalaxyLabels.Pop())
		{
			Extra->DestroyComponent();
		}
	}
	while (GalaxyLabels.Num() < Marks.Num())
	{
		UWidgetComponent* Label = NewObject<UWidgetComponent>(this);
		Label->SetupAttachment(RootComponent);
		Label->SetWidgetSpace(EWidgetSpace::Screen);
		Label->SetWidgetClass(UCosmicSystemLabelWidget::StaticClass());
		Label->SetDrawSize(GalaxyLabelSize);
		Label->SetPivot(FVector2D(0.5f, 1.f));
		Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Ticked only while its widget shows: a collapsed label (every one in the region) turns its tick off and a change of
		// visibility turns it on again (the default, Enabled, ticks twenty hidden labels every frame).
		Label->SetTickMode(ETickMode::Automatic);
		Label->RegisterComponent();
		GalaxyLabels.Add(Label);
	}
	for (int32 Index = 0; Index < Marks.Num(); ++Index)
	{
		// Bounded by the labels too: a label component the engine refused leaves no hole to index.
		UWidgetComponent* Label = GalaxyLabels.IsValidIndex(Index) ? GalaxyLabels[Index].Get() : nullptr;
		if (!Label)
		{
			continue;
		}
		const FCosmicGalaxyMark& Mark = Marks[Index];
		Label->SetWorldLocation(Mark.Position + FVector(0.0, 0.0, CosmicUniverseLook::LabelLiftShare * Mark.Radius));
		// The name alone: no bar (both shares -1).
		FCosmicSystemLook Look;
		Look.Name = Mark.Name;
		Look.ConstructionShare = -1.f;
		Look.ResponseShare = -1.f;
		if (UCosmicSystemLabelWidget* Widget = Cast<UCosmicSystemLabelWidget>(Label->GetUserWidgetObject()))
		{
			Widget->Show(Look);
			Widget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// The sky holds every camera place and everything drawn (the engine sphere's radius is SkySphereRadius).
	Sky->SetRelativeScale3D(FVector(CosmicUniverseLook::SkyRadiusOf(ZoomLevels, Frame, GetActorLocation()) / CosmicUniverseLook::SkySphereRadius));

	if (!bBackdropUsable)
	{
		return;
	}
	// The instances only when their key changed (ClearInstances rebuilds the render state): the home row, the count, the
	// frame and the marks' instances. Never the run's seed - one sky for every run.
	const int32 Count = CosmicUniverseLook::SceneryCountOf(Rules);
	bool bSameKey = InstanceCount == Count && InstanceRow == Rules.HomeGalaxyRow && InstanceFrame.Centre.Equals(Frame.Centre, 0.0)
		&& InstanceFrame.Unit == Frame.Unit && KeyMarks.Num() == Marks.Num();
	for (int32 Index = 0; bSameKey && Index < Marks.Num(); ++Index)
	{
		bSameKey = SameInstance(KeyMarks[Index], Marks[Index]);
	}
	if (bSameKey)
	{
		return;
	}
	CSV_SCOPED_TIMING_STAT(CosmicMap, Universe);
	const uint64 Start = FPlatformTime::Cycles64();
	TArray<FCosmicSceneryPoint> Points = CosmicUniverseLook::SceneryPointsOf(Rules.HomeGalaxyRow, Count, Frame);
	CosmicUniverseLook::ClearAroundMarks(Points, Marks);
	TArray<FTransform> Transforms;
	Transforms.Reserve(Points.Num() + Marks.Num());
	for (const FCosmicSceneryPoint& Point : Points)
	{
		Transforms.Add(CosmicUniverseLook::InstanceOf(Point));
	}
	for (const FCosmicGalaxyMark& Mark : Marks)
	{
		Transforms.Add(CosmicUniverseLook::InstanceOf(Mark));
	}
	// The ranged custom-data write checks its size fatally: the count of floats is set where it is used.
	if (Universe->NumCustomDataFloats != CosmicMapLook::BackdropDataFloats)
	{
		Universe->SetNumCustomDataFloats(CosmicMapLook::BackdropDataFloats);
	}
	Universe->ClearInstances();
	Universe->AddInstances(Transforms, false, true, false);
	const int32 Instances = Transforms.Num();
	if (Instances > 0)
	{
		TArray<float> Glows;
		Glows.Reserve(Instances * Universe->NumCustomDataFloats);
		for (const FCosmicSceneryPoint& Point : Points)
		{
			Glows.Add(Point.Glow);
		}
		for (const FCosmicGalaxyMark& Mark : Marks)
		{
			Glows.Add(Mark.Glow);
		}
		if (Glows.Num() == Instances * Universe->NumCustomDataFloats)
		{
			Universe->SetCustomData(0, Instances - 1, Glows, false);
		}
	}
	SceneryCount = Points.Num();
	InstanceCount = Count;
	InstanceRow = Rules.HomeGalaxyRow;
	InstanceFrame = Frame;
	KeyMarks = Marks;
	++UniverseBuilds;
	UniverseBuildMs = FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64() - Start);
	UE_LOG(LogCosmic, Log, TEXT("CosmicMap: universe built (%d scenery, %d named) in %.2f ms"), SceneryCount, Marks.Num(), UniverseBuildMs);
}

void ACosmicGalaxyMapActor::ApplyLayerFades(float Share)
{
	using namespace CosmicGalaxyMapImpl;

	const bool bEnd = Share == 0.f || Share == 1.f;
	if (!(FMath::Abs(Share - WrittenShare) > FadeSlack || (bEnd && Share != WrittenShare)))
	{
		return;
	}
	WrittenShare = Share;
	if (BackdropMaterial)
	{
		BackdropMaterial->SetScalarParameterValue(LayerFadeName, 1.f - Share);
	}
	if (UniverseMaterial)
	{
		UniverseMaterial->SetScalarParameterValue(LayerFadeName, Share);
	}
	// A layer that shows nothing is not drawn at all: the universe in the region, the dust at the universe level (a
	// visibility change at each crossing, never a rebuild of the instances).
	Universe->SetVisibility(bBackdropUsable && Share > 0.f);
	Backdrop->SetVisibility(bBackdropUsable && Share < 1.f);
	WriteIntensities();
}

void ACosmicGalaxyMapActor::WriteIntensities()
{
	// At the universe level the sight dims nothing (the fallen do not cross): the sky and the universe take the view's
	// value x (1 - the share). A value equal to the one set before sends no render update.
	const float Share = WrittenShare < 0.f ? UniverseShare : WrittenShare;
	const float Receded = CosmicUniverseLook::RecededIntensityOf(SkyIntensity, Share);
	if (SkyMaterial)
	{
		SkyMaterial->SetScalarParameterValue(CosmicGalaxyMapImpl::SkyIntensityName, Receded);
	}
	if (UniverseMaterial)
	{
		UniverseMaterial->SetScalarParameterValue(CosmicGalaxyMapImpl::SkyIntensityName, Receded);
	}
}

void ACosmicGalaxyMapActor::SetSelectedGalaxy(FName Row)
{
	// Only marked: the map redraws at most once a frame.
	if (Row != SelectedGalaxyRow)
	{
		SelectedGalaxyRow = Row;
		bRefreshPending = true;
	}
}

bool ACosmicGalaxyMapActor::IsGalaxyLabelVisible(int32 MarkIndex) const
{
	const UWidgetComponent* Label = GalaxyLabels.IsValidIndex(MarkIndex) ? GalaxyLabels[MarkIndex].Get() : nullptr;
	const UUserWidget* Widget = Label ? Label->GetUserWidgetObject() : nullptr;
	return Widget && Widget->GetVisibility() != ESlateVisibility::Collapsed;
}

void ACosmicGalaxyMapActor::BuildBackdrop(const FGalaxyState& State, const FCosmicRulesDef& Rules)
{
	using namespace CosmicGalaxyMapImpl;

	if (!bBackdropUsable)
	{
		return;
	}
	const int32 Count = CosmicMapLook::BackdropCountOf(Rules);
	const int32 SystemCount = State.Systems.Num();

	// The slots, at every rebuild: each system's own point at its place (the systems' places come from this state).
	SlotGlows.SetNum(SystemCount);
	TArray<FTransform> SlotTransforms;
	SlotTransforms.Reserve(SystemCount);
	for (int32 Index = 0; Index < SystemCount; ++Index)
	{
		const FStarSystemState& System = State.Systems[Index];
		const FCosmicBackdropPoint Slot = CosmicMapLook::BackdropSlotOf(State.Seed, System.SystemID, System.Position, DiscRadius);
		SlotGlows[Index] = Slot.Glow;
		SlotTransforms.Add(PointTransform(Slot));
	}

	const bool bSameKey = BackdropCount == Count && BackdropSystems == SystemCount && BackdropSeed == State.Seed
		&& BackdropRow == Rules.HomeGalaxyRow && BackdropCentre.Equals(DiscCentre, 0.0) && BackdropRadius == DiscRadius;
	if (bSameKey)
	{
		// The points stand: only the slots move to this state's places (no render state is rebuilt for them).
		if (SystemCount > 0)
		{
			Backdrop->BatchUpdateInstancesTransforms(Count, SlotTransforms, true, false, false);
		}
		return;
	}

	// A new key: the points again. ClearInstances recreates the whole render state, so this runs at a rebuild only, and
	// only when the picture changes - never at a refresh.
	CSV_SCOPED_TIMING_STAT(CosmicMap, Backdrop);
	const uint64 Start = FPlatformTime::Cycles64();
	const TArray<FCosmicBackdropPoint> Points = CosmicMapLook::BackdropPointsOf(State.Seed, Rules.HomeGalaxyRow, Count, DiscCentre, DiscRadius);
	TArray<FTransform> Transforms;
	Transforms.Reserve(Points.Num() + SystemCount);
	TArray<float> Values;
	for (const FCosmicBackdropPoint& Point : Points)
	{
		Transforms.Add(PointTransform(Point));
	}
	Transforms.Append(SlotTransforms);
	// The ranged custom-data write checks its size fatally: the count of floats is set where it is used.
	if (Backdrop->NumCustomDataFloats != CosmicMapLook::BackdropDataFloats)
	{
		Backdrop->SetNumCustomDataFloats(CosmicMapLook::BackdropDataFloats);
	}
	Backdrop->ClearInstances();
	Backdrop->AddInstances(Transforms, false, true, false);
	const int32 Instances = Points.Num() + SystemCount;
	if (Instances > 0)
	{
		// The points' glows, then a dark slot per system (the refresh lights the unknown ones).
		Values.Reserve(Instances * Backdrop->NumCustomDataFloats);
		for (const FCosmicBackdropPoint& Point : Points)
		{
			Values.Add(Point.Glow);
		}
		Values.AddZeroed(SystemCount);
		if (Values.Num() == Instances * Backdrop->NumCustomDataFloats)
		{
			Backdrop->SetCustomData(0, Instances - 1, Values, false);
		}
	}
	SlotValues.Init(0.f, SystemCount);
	BackdropCount = Points.Num();
	BackdropSystems = SystemCount;
	BackdropSeed = State.Seed;
	BackdropRow = Rules.HomeGalaxyRow;
	BackdropCentre = DiscCentre;
	BackdropRadius = DiscRadius;
	++BackdropBuilds;
	BackdropBuildMs = FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64() - Start);
	UE_LOG(LogCosmic, Log, TEXT("CosmicBackdrop: %d points and %d slots built in %.2f ms"), BackdropCount, SystemCount, BackdropBuildMs);
}

void ACosmicGalaxyMapActor::LightSlots(const FGalaxyState& State, const FCosmicKnownView& View)
{
	if (!bBackdropUsable || BackdropCount < 0)
	{
		return;
	}
	// Bounded by what the last build sized: a state the map was not told about never indexes past it.
	const int32 SlotCount = FMath::Min3(State.Systems.Num(), SlotGlows.Num(), SlotValues.Num());
	for (int32 Index = 0; Index < SlotCount; ++Index)
	{
		const float Want = View.IsShown(Index) ? 0.f : SlotGlows[Index];
		if (Want != SlotValues[Index])
		{
			// Marks the instances dirty itself (an incremental update; the render state is not rebuilt).
			Backdrop->SetCustomDataValue(BackdropCount + Index, 0, Want, false);
			SlotValues[Index] = Want;
			++SlotWrites;
		}
	}
}

void ACosmicGalaxyMapActor::Refresh()
{
	using namespace CosmicGalaxyMapImpl;

	CSV_SCOPED_TIMING_STAT(CosmicMap, Refresh);
	bRefreshPending = false;
	++Refreshes;
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	LaneThickness = ViewDistance * LaneThicknessPerDistance;
	const float RingThickness = RingFactor * LaneThickness;

	// The view drawn: the viewer's tier is read here, so the tick of an ascension or a deposition turns it.
	DrawnView = CosmicMapLook::ShownViewMode(WantedView, bSightForcedOpen, Sim->GetViewerTier());
	const bool bSpiritual = DrawnView == ECosmicViewMode::Spiritual;
	bRegionDetail = CosmicZoom::ShowsRegionDetail(UniverseShare);

	if (!bRegionDetail)
	{
		// The universe level: the region's stars, labels, lanes and rings are not drawn - at the far stop the disc is a few
		// dozen pixels across. An actor's hidden flag composes with its sphere's own visibility, so item 8's unknown stars
		// keep theirs; a hidden owner's screen-space label leaves the screen at its next tick.
		if (!bStarsHidden)
		{
			for (ACosmicStarSystemActor* Star : Stars)
			{
				if (Star)
				{
					Star->SetActorHiddenInGame(true);
					Star->SetLabelVisible(false);
				}
			}
			bStarsHidden = true;
		}
		// The cheap redraw: the galaxies' labels and the selected galaxy's ring - no looks, spirits, lanes or slots.
		TArray<FBatchedLine> Lines;
		DrawnGalaxySelection = NAME_None;
		bGalaxyLabelsSet = true;
		for (int32 Index = 0; Index < Marks.Num(); ++Index)
		{
			const FCosmicGalaxyMark& Mark = Marks[Index];
			const bool bSelected = !SelectedGalaxyRow.IsNone() && Mark.Row == SelectedGalaxyRow;
			if (UUserWidget* Widget = GalaxyLabels.IsValidIndex(Index) && GalaxyLabels[Index] ? GalaxyLabels[Index]->GetUserWidgetObject() : nullptr)
			{
				Widget->SetVisibility(CosmicUniverseLook::ShowsGalaxyLabel(Mark, ViewDistance, bSelected) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
			}
			if (bSelected)
			{
				AddRing(Lines, Mark.Position, CosmicUniverseLook::RingRadiusOf(Mark), SelectionSegments, false, CosmicPalette::White, RingThickness);
				DrawnGalaxySelection = Mark.Row;
			}
		}
		DrawnLanes = 0;
		DrawnRings = 0;
		DrawnSealedRings = 0;
		DrawnGreyedHalves = 0;
		DrawnSelection = NAME_None;
		DrawnVesselSystem = NAME_None;
		bDrawnVesselLane = false;
		SkyIntensity = CosmicMapLook::SkyIntensityOf(DrawnView);
		BackdropIntensity = SkyIntensity;
		if (BackdropMaterial)
		{
			BackdropMaterial->SetScalarParameterValue(SkyIntensityName, BackdropIntensity);
		}
		WriteIntensities();
		Lanes->Flush();
		Lanes->DrawLines(Lines);
		return;
	}

	// The region: the stars back when the universe hid them, the galaxies' labels gone.
	if (bStarsHidden)
	{
		for (ACosmicStarSystemActor* Star : Stars)
		{
			if (Star)
			{
				Star->SetActorHiddenInGame(false);
			}
		}
		bStarsHidden = false;
	}
	if (bGalaxyLabelsSet)
	{
		for (UWidgetComponent* Label : GalaxyLabels)
		{
			if (UUserWidget* Widget = Label ? Label->GetUserWidgetObject() : nullptr)
			{
				Widget->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		bGalaxyLabelsSet = false;
	}
	DrawnGalaxySelection = NAME_None;
	// What the viewer knows, read once for this refresh (section 6.4): the one reading of Knowing.
	const FCosmicKnownView View = CosmicMapLook::KnownViewOf(State, Sim->GetStateIndex(), Sim->GetViewerTier(), bSeeAll);

	// The looks first: a half-lane needs the pressure at its end. The world's look, then what the viewer is shown of it.
	// The physical view asks nothing spiritual, and the sight asks nothing of a system the viewer does not know (it keeps
	// the zero spirit look: no pulse, colour, claim marker or sealed ring).
	const int32 SystemCount = State.Systems.Num();
	TArray<FCosmicSystemLook> Looks;
	TArray<FCosmicSpiritLook> Spirits;
	Looks.Reserve(SystemCount);
	Spirits.SetNum(SystemCount);
	for (int32 Index = 0; Index < SystemCount; ++Index)
	{
		Looks.Add(CosmicMapLook::ShownLookOf(CosmicMapLook::LookOf(State, Sim->GetTables(), Index), State, View, Index));
		if (bSpiritual && Looks[Index].bKnown)
		{
			Spirits[Index] = CosmicMapLook::SpiritOf(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), Index);
		}
	}

	// Physical: one line per lane. Spiritual: two halves, each from its end to the midpoint in the colour of its end's
	// pressure (a line batch has one colour per line and no material - there is no M_Lane). Only lanes between two known
	// systems: no stub points at a system that must not be told from the dust.
	TArray<FBatchedLine> Lines;
	const TArray<TPair<int32, int32>> LaneList = CosmicMapLook::KnownLanesOf(CosmicMapLook::LanesOf(State), View);
	DrawnGreyedHalves = 0;
	for (const TPair<int32, int32>& Lane : LaneList)
	{
		const FVector From = State.Systems[Lane.Key].Position;
		const FVector To = State.Systems[Lane.Value].Position;
		if (!bSpiritual)
		{
			Lines.Emplace(From, To, CosmicMapLook::LaneColorOf(0.f, ECosmicViewMode::Physical), 0.f, LaneThickness, SDPG_World);
			continue;
		}
		const FVector Middle = 0.5 * (From + To);
		for (const TPair<int32, FVector>& End : { TPair<int32, FVector>(Lane.Key, From), TPair<int32, FVector>(Lane.Value, To) })
		{
			const float EndPressure = Spirits[End.Key].Pressure;
			Lines.Emplace(End.Value, Middle, CosmicMapLook::LaneColorOf(EndPressure, ECosmicViewMode::Spiritual), 0.f, LaneThickness, SDPG_World);
			DrawnGreyedHalves += EndPressure > 0.f ? 1 : 0;
		}
	}
	DrawnLanes = LaneList.Num();

	// Systems are never removed (a destroyed one stays as a dead node), so the actors match the systems by index.
	DrawnRings = 0;
	DrawnSealedRings = 0;
	DrawnSelection = NAME_None;
	for (int32 Index = 0; Index < Stars.Num() && Index < SystemCount; ++Index)
	{
		ACosmicStarSystemActor* Star = Stars[Index];
		if (!Star)
		{
			continue;
		}
		const FCosmicSystemLook& Look = Looks[Index];
		// A system the viewer does not know takes no ring, even when it is the selected one (a guard: the controller
		// selects only known systems).
		const bool bSelected = !SelectedSystemID.IsNone() && Star->GetSystemID() == SelectedSystemID && Look.bKnown;
		Star->ApplyLook(Look, Spirits[Index], DrawnView);
		Star->SetLabelVisible(CosmicMapLook::ShowsLabel(Look, bSelected, bFarView));

		// Around the star as this look scaled it.
		const FVector Centre = Star->GetActorLocation();
		const float OwnerRadius = Star->GetStarRadius() + RingGap;
		if (Look.bOwned)
		{
			// In its meeting colour (ShownLookOf). OwnerRingColorOf stays as a deliberately idle guard: the palette keeps out
			// of the fallen's band, so in the sight it changes no colour the map draws today.
			AddRing(Lines, Centre, OwnerRadius, RingSegments, Look.bSettling, CosmicMapLook::OwnerRingColorOf(Look.OwnerColor, DrawnView), RingThickness);
			++DrawnRings;
		}
		if (bSpiritual && Spirits[Index].bSealedRing)
		{
			AddRing(Lines, Centre, OwnerRadius + SealedGapRings * RingThickness, RingSegments, false, CosmicPalette::Grey, RingThickness);
			++DrawnSealedRings;
		}
		if (bSelected)
		{
			// White, one of the Severity colours of the palette (guideline 8.2).
			AddRing(Lines, Centre, OwnerRadius + SelectionGapRings * RingThickness, SelectionSegments, false, CosmicPalette::White, RingThickness);
			DrawnSelection = SelectedSystemID;
		}
	}

	// The vessel (B1): a gold diamond - the calling's colour, as its lines in the feed - around the star it stands at; while
	// it travels, a small one along the lane it takes, and that lane lit. A shape of its own - the rings are round, the
	// selection's white. Its system and the lane's ends are always known (a guard all the same: nothing is drawn toward a
	// system the viewer is not shown).
	DrawnVesselSystem = NAME_None;
	bDrawnVesselLane = false;
	const FCosmicVesselLook Vessel = CosmicMapLook::VesselLookOf(State, Sim->GetStateIndex(), Sim->GetRules());
	if (Looks.IsValidIndex(Vessel.FromIndex) && Looks[Vessel.FromIndex].bKnown && Stars.IsValidIndex(Vessel.FromIndex) && Stars[Vessel.FromIndex])
	{
		const ACosmicStarSystemActor* Star = Stars[Vessel.FromIndex];
		if (Looks.IsValidIndex(Vessel.ToIndex) && Looks[Vessel.ToIndex].bKnown)
		{
			const FVector From = State.Systems[Vessel.FromIndex].Position;
			const FVector To = State.Systems[Vessel.ToIndex].Position;
			Lines.Emplace(From, To, CosmicPalette::Gold, 0.f, VesselLaneFactor * LaneThickness, SDPG_World);
			AddRing(Lines, FMath::Lerp(From, To, static_cast<double>(Vessel.Share)), VesselTravelRadius, VesselSegments, false, CosmicPalette::Gold, RingThickness);
			bDrawnVesselLane = true;
		}
		else
		{
			const float SelectionRadius = Star->GetStarRadius() + RingGap + SelectionGapRings * RingThickness;
			AddRing(Lines, Star->GetActorLocation(), VesselDiamondScale * SelectionRadius, VesselSegments, false, CosmicPalette::Gold, RingThickness);
		}
		DrawnVesselSystem = State.Systems[Vessel.FromIndex].SystemID;
	}

	// The backdrop's slots: an unknown system's point lit, a known one's dark (only the slots whose knowledge flipped).
	LightSlots(State, View);

	// The sky goes quiet in the spiritual view, and the dust with it (the sky and the universe recede by the share).
	SkyIntensity = CosmicMapLook::SkyIntensityOf(DrawnView);
	WriteIntensities();
	BackdropIntensity = SkyIntensity;
	if (BackdropMaterial)
	{
		// A value equal to the one set before sends no render update (the instance's setter compares).
		BackdropMaterial->SetScalarParameterValue(SkyIntensityName, BackdropIntensity);
	}

	Lanes->Flush();
	Lanes->DrawLines(Lines);
}

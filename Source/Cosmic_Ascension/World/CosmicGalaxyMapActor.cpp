// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicGalaxyMapActor.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/LineBatchComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Cosmic_Ascension.h"
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
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "World/CosmicMapLook.h"
#include "World/CosmicStarSystemActor.h"

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
	// A disc smaller than this is taken as this (a board of a few systems).
	constexpr float MinDiscRadius = 1000.f;
	// The sky sphere: the engine sphere (100 units across) blown up far past the disc of any galaxy size.
	constexpr float SkyScale = 3000.f;
	// M_Nebula's and M_Backdrop's scalar, looked up once.
	const FName SkyIntensityName(CosmicMapLook::SkyIntensityParam);
	// The engine plane is 100 units across: an instance of a point Size across is scaled by Size / this.
	constexpr float PlaneSize = 100.f;

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
	const bool bFar = ArmLength > FarViewDistance;
	if (bFar != bFarView || FMath::Abs(ArmLength - ViewDistance) > ViewDistanceSlack * ViewDistance)
	{
		ViewDistance = ArmLength;
		bFarView = bFar;
		bRefreshPending = true;
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
	DiscRadius = Farthest > MinDiscRadius ? static_cast<float>(Farthest) : MinDiscRadius;

	if (Sim)
	{
		BuildBackdrop(Sim->GetState(), Sim->GetRules());
	}
	Refresh();
	OnRebuilt.Broadcast();
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
		const float Want = View.IsKnown(Index) ? 0.f : SlotGlows[Index];
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

	// The backdrop's slots: an unknown system's point lit, a known one's dark (only the slots whose knowledge flipped).
	LightSlots(State, View);

	// The sky goes quiet in the spiritual view, and the dust with it.
	SkyIntensity = CosmicMapLook::SkyIntensityOf(DrawnView);
	if (SkyMaterial)
	{
		SkyMaterial->SetScalarParameterValue(SkyIntensityName, SkyIntensity);
	}
	BackdropIntensity = SkyIntensity;
	if (BackdropMaterial)
	{
		// A value equal to the one set before sends no render update (the instance's setter compares).
		BackdropMaterial->SetScalarParameterValue(SkyIntensityName, BackdropIntensity);
	}

	Lanes->Flush();
	Lanes->DrawLines(Lines);
}

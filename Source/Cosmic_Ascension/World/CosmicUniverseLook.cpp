// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicUniverseLook.h"

#include "Core/CosmicSeed.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Math/Quat.h"
#include "UObject/NameTypes.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicUniverseLookImpl
{
	// The engine plane is 100 units across: an instance Size across is scaled by Size / this.
	constexpr float PlaneSize = 100.f;

	struct FKindEntry
	{
		const TCHAR* Tag;
		float Aspect;
		float DefaultShare;
	};
	// An elliptical rounder, a spiral flatter (a disc seen at a slant); the default share of a row without a radius.
	constexpr FKindEntry Kinds[] = {
		{ TEXT("Galaxy.Kind.Dwarf"), 0.7f, 0.45f },
		{ TEXT("Galaxy.Kind.Spiral"), 0.45f, 1.3f },
		{ TEXT("Galaxy.Kind.GrandSpiral"), 0.45f, 1.9f },
		{ TEXT("Galaxy.Kind.Elliptical"), 0.85f, 1.6f },
	};

	float Clamp01(float Value)
	{
		// Compares: a value that is not a number is 0.
		return Value > 0.f ? (Value < 1.f ? Value : 1.f) : 0.f;
	}
}

FCosmicUniverseFrame CosmicUniverseLook::FrameOf(const FVector& DiscCentre, float DiscRadius)
{
	FCosmicUniverseFrame Frame;
	Frame.Centre = FVector(DiscCentre.X, DiscCentre.Y, 0.0);
	Frame.Unit = DiscRadius > CosmicZoom::MinDiscRadius ? DiscRadius : CosmicZoom::MinDiscRadius;
	return Frame;
}

FVector CosmicUniverseLook::PlaceOf(const FCosmicUniverseFrame& Frame, const FVector2D& MapPosition)
{
	return Frame.Centre + FVector(MapPosition.X, MapPosition.Y, 0.0) * Frame.Unit;
}

bool CosmicUniverseLook::HasPlace(const FVector2D& MapPosition, bool bHome)
{
	if (bHome)
	{
		return true;
	}
	const double Length2 = MapPosition.X * MapPosition.X + MapPosition.Y * MapPosition.Y;
	return Length2 >= MinPlaceUnits * MinPlaceUnits && Length2 <= MaxPlaceUnits * MaxPlaceUnits;
}

FCosmicKindLook CosmicUniverseLook::KindLookOf(FGameplayTag Kind)
{
	FCosmicKindLook Look;
	if (!Kind.IsValid())
	{
		return Look;
	}
	for (const CosmicUniverseLookImpl::FKindEntry& Entry : CosmicUniverseLookImpl::Kinds)
	{
		// Requested by name, never compared by a printed name (its case only an editor build keeps).
		if (Kind == FGameplayTag::RequestGameplayTag(FName(Entry.Tag), false))
		{
			Look.bKnown = true;
			Look.Aspect = Entry.Aspect;
			Look.DefaultShare = Entry.DefaultShare;
			return Look;
		}
	}
	return Look;
}

float CosmicUniverseLook::SpriteShareOf(const FGalaxyDef& Row)
{
	// Integers: no value that is not a number reaches the root. Presentation, outside every hash.
	if (Row.RadiusLy > 0)
	{
		return FMath::Clamp(HomeSpriteShare * FMath::Sqrt(static_cast<float>(Row.RadiusLy) / static_cast<float>(HomeLawRadiusLy)), MinSpriteShare, MaxSpriteShare);
	}
	return KindLookOf(Row.Kind).DefaultShare;
}

float CosmicUniverseLook::TurnDegreesOf(FName Row)
{
	const uint32 Bits = CosmicSeed::DeriveForName(UniverseLookSeed, Row, CosmicSeed::Purpose::Scenery, NamedTurnIndex) >> 8;
	return static_cast<float>(Bits) * (360.f / 16777216.f);
}

TArray<FCosmicGalaxyMark> CosmicUniverseLook::MarksOf(const FCosmicDataTables& Tables, FName HomeRow, const FCosmicUniverseFrame& Frame)
{
	TArray<FCosmicGalaxyMark> Marks;
	auto HomeMark = [&Frame, HomeRow](const FGalaxyDef* Def)
	{
		FCosmicGalaxyMark Mark;
		Mark.Row = HomeRow;
		Mark.Name = Def ? Def->DisplayName : FText::GetEmpty();
		Mark.Position = Frame.Centre;
		Mark.Radius = HomeSpriteShare * Frame.Unit;
		Mark.Aspect = 1.f;
		Mark.TurnDegrees = 0.f;
		Mark.Glow = NamedGlow;
		Mark.bHome = true;
		return Mark;
	};
	bool bHomeDrawn = false;
	for (const FName Name : Tables.GetGalaxyRowNames())
	{
		const FGalaxyDef* Def = Tables.FindGalaxy(Name);
		if (!Def)
		{
			continue;
		}
		if (Name == HomeRow)
		{
			Marks.Add(HomeMark(Def));
			bHomeDrawn = true;
			continue;
		}
		if (!HasPlace(Def->MapPosition, false))
		{
			continue;
		}
		FCosmicGalaxyMark& Mark = Marks.AddDefaulted_GetRef();
		Mark.Row = Name;
		Mark.Name = Def->DisplayName;
		Mark.Position = PlaceOf(Frame, Def->MapPosition);
		Mark.Radius = SpriteShareOf(*Def) * Frame.Unit;
		Mark.Aspect = KindLookOf(Def->Kind).Aspect;
		Mark.TurnDegrees = TurnDegreesOf(Name);
		Mark.Glow = NamedGlow;
	}
	if (!bHomeDrawn)
	{
		// Where lexical order puts the home row: the home stands whatever the table holds.
		int32 At = 0;
		while (At < Marks.Num() && FNameLexicalLess()(Marks[At].Row, HomeRow))
		{
			++At;
		}
		Marks.Insert(HomeMark(nullptr), At);
	}
	return Marks;
}

TArray<FName> CosmicUniverseLook::UnplacedRowsOf(const FCosmicDataTables& Tables, FName HomeRow)
{
	TArray<FName> Unplaced;
	for (const FName Name : Tables.GetGalaxyRowNames())
	{
		const FGalaxyDef* Def = Tables.FindGalaxy(Name);
		if (Name != HomeRow && Def && !HasPlace(Def->MapPosition, false))
		{
			Unplaced.Add(Name);
		}
	}
	return Unplaced;
}

int32 CosmicUniverseLook::SceneryCountOf(const FCosmicRulesDef& Rules)
{
	return Rules.SceneryGalaxyCount > 0 ? FMath::Min(Rules.SceneryGalaxyCount, MaxSceneryPoints) : 0;
}

FVector2D CosmicUniverseLook::AnnulusPointOf(FRandomStream& Stream, float InnerUnits, float OuterUnits)
{
	// Even over the unit disc: the generator's rejection in the square, at most 16 tries, the last one halved.
	double X = 0.0;
	double Y = 0.0;
	for (int32 Try = 0; Try < 16; ++Try)
	{
		X = 2.0 * Stream.GetFraction() - 1.0;
		Y = 2.0 * Stream.GetFraction() - 1.0;
		if (X * X + Y * Y <= 1.0)
		{
			break;
		}
		if (Try == 15)
		{
			X *= 0.5;
			Y *= 0.5;
		}
	}
	// Carried into the annulus with even area: r^2 = Inner^2 + rho^2 (Outer^2 - Inner^2) (only + - x / and the root).
	const double Rho2 = X * X + Y * Y;
	const double Inner2 = static_cast<double>(InnerUnits) * InnerUnits;
	const double Outer2 = static_cast<double>(OuterUnits) * OuterUnits;
	const double Radius = FMath::Sqrt(Inner2 + Rho2 * (Outer2 - Inner2));
	if (Rho2 > 0.0)
	{
		const double Scale = Radius / FMath::Sqrt(Rho2);
		return FVector2D(X * Scale, Y * Scale);
	}
	return FVector2D(Radius, 0.0);
}

void CosmicUniverseLook::SceneryLookOf(FRandomStream& Stream, float Unit, FCosmicSceneryPoint& Point)
{
	// Exactly five draws, whatever they give: a draw count that depended on a draw would move every later point.
	const float Kind = Stream.GetFraction();
	const float S = Stream.GetFraction();
	const float G = Stream.GetFraction();
	const float A = Stream.GetFraction();
	const float T = Stream.GetFraction();
	Point.Size = Unit * (SceneryMinSizeUnits + (SceneryMaxSizeUnits - SceneryMinSizeUnits) * S * S);
	Point.Glow = SceneryMinGlow + (SceneryMaxGlow - SceneryMinGlow) * G * G * G;
	Point.Aspect = Kind < SceneryRoundShare
		? SceneryRoundAspectMin + (1.f - SceneryRoundAspectMin) * A
		: SceneryFlatAspectMin + (SceneryFlatAspectMax - SceneryFlatAspectMin) * A;
	Point.TurnDegrees = 360.f * T;
}

float CosmicUniverseLook::SceneryFadeOf(float RadiusUnits)
{
	using CosmicUniverseLookImpl::Clamp01;
	return Clamp01((RadiusUnits - SceneryInnerUnits) / SceneryInnerFadeUnits) * Clamp01((SceneryOuterUnits - RadiusUnits) / SceneryOuterFadeUnits);
}

float CosmicUniverseLook::LocalGroupShareOf(float RadiusUnits, bool bGlow)
{
	return RadiusUnits < LocalGroupUnits ? (bGlow ? LocalGroupGlowShare : LocalGroupSizeShare) : 1.f;
}

TArray<FCosmicSceneryPoint> CosmicUniverseLook::SceneryPointsOf(FName HomeRow, int32 Count, const FCosmicUniverseFrame& Frame)
{
	const int32 Points = Count > 0 ? FMath::Min(Count, MaxSceneryPoints) : 0;
	TArray<FCosmicSceneryPoint> Scenery;
	if (Points == 0 || !(Frame.Unit > 0.f))
	{
		return Scenery;
	}
	// A local stream of the presentation (section 9.2: the third source, Purpose Scenery): never the simulation's, and no
	// run seed - one sky for every run.
	FRandomStream Stream;
	Stream.Initialize(static_cast<int32>(CosmicSeed::DeriveForName(UniverseLookSeed, HomeRow, CosmicSeed::Purpose::Scenery, SceneryIndex)));
	Scenery.SetNum(Points);
	for (FCosmicSceneryPoint& Point : Scenery)
	{
		const FVector2D Place = AnnulusPointOf(Stream, SceneryInnerUnits, SceneryOuterUnits);
		Point.Position = Frame.Centre + FVector(Place.X * Frame.Unit, Place.Y * Frame.Unit, 0.0);
		SceneryLookOf(Stream, Frame.Unit, Point);
		const float Radius = static_cast<float>(Place.Size());
		Point.Glow *= SceneryFadeOf(Radius) * LocalGroupShareOf(Radius, true);
		Point.Size *= LocalGroupShareOf(Radius, false);
	}
	return Scenery;
}

void CosmicUniverseLook::ClearAroundMarks(TArray<FCosmicSceneryPoint>& Points, TConstArrayView<FCosmicGalaxyMark> Marks)
{
	for (FCosmicSceneryPoint& Point : Points)
	{
		for (const FCosmicGalaxyMark& Mark : Marks)
		{
			if (FVector::Dist2D(Point.Position, Mark.Position) < ClearShare * Mark.Radius)
			{
				Point.Glow = 0.f;
				break;
			}
		}
	}
}

float CosmicUniverseLook::ReachOf(const FCosmicUniverseFrame& Frame, TConstArrayView<FCosmicGalaxyMark> Marks)
{
	double Farthest = 0.0;
	for (const FCosmicGalaxyMark& Mark : Marks)
	{
		Farthest = FMath::Max(Farthest, FVector::Dist2D(Mark.Position, Frame.Centre) + Mark.Radius);
	}
	return static_cast<float>(ReachMargin * Farthest);
}

float CosmicUniverseLook::SkyRadiusOf(const FCosmicZoomLevels& Levels, const FCosmicUniverseFrame& Frame, const FVector& SkyCentre)
{
	const double Offset = FVector::Dist2D(Frame.Centre, SkyCentre);
	double Farthest = static_cast<double>(SceneryOuterUnits) * Frame.Unit;
	Farthest = FMath::Max(Farthest, static_cast<double>(Levels.RegionBound) + Levels.RegionFar);
	if (Levels.HasUniverse())
	{
		Farthest = FMath::Max(Farthest, static_cast<double>(Levels.UniverseBound) + Levels.UniverseFar);
	}
	const double Radius = SkyMargin * (Offset + Farthest);
	// A compare: a radius that is not a number is the floor.
	return Radius > MinSkyRadius ? static_cast<float>(Radius) : MinSkyRadius;
}

float CosmicUniverseLook::RecededIntensityOf(float Intensity, float Share)
{
	return Intensity * (1.f - CosmicUniverseLookImpl::Clamp01(Share));
}

bool CosmicUniverseLook::ShowsGalaxyLabel(const FCosmicGalaxyMark& Mark, float ViewDistance, bool bSelected)
{
	return bSelected || Mark.bHome || (ViewDistance > 0.f && Mark.Radius >= LabelRadiusShare * ViewDistance);
}

float CosmicUniverseLook::RingRadiusOf(const FCosmicGalaxyMark& Mark)
{
	return RingShare * Mark.Radius;
}

float CosmicUniverseLook::FocusArmOf(const FCosmicGalaxyMark& Mark, const FCosmicZoomLevels& Levels)
{
	if (!Levels.HasUniverse())
	{
		return 0.f;
	}
	return FMath::Clamp(FMath::Max(Levels.UniverseNear, GalaxyFocusFactor * Mark.Radius), Levels.UniverseNear, Levels.UniverseFar);
}

FTransform CosmicUniverseLook::InstanceOf(const FCosmicGalaxyMark& Mark)
{
	using CosmicUniverseLookImpl::PlaneSize;
	const double Across = 2.0 * Mark.Radius / PlaneSize;
	return FTransform(FQuat(FVector::UpVector, FMath::DegreesToRadians(static_cast<double>(Mark.TurnDegrees))), Mark.Position, FVector(Across, Across * Mark.Aspect, 1.0));
}

FTransform CosmicUniverseLook::InstanceOf(const FCosmicSceneryPoint& Point)
{
	using CosmicUniverseLookImpl::PlaneSize;
	const double Across = Point.Size / PlaneSize;
	return FTransform(FQuat(FVector::UpVector, FMath::DegreesToRadians(static_cast<double>(Point.TurnDegrees))), Point.Position, FVector(Across, Across * Point.Aspect, 1.0));
}

FCosmicPanelChoice CosmicUniverseLook::PanelOf(ECosmicZoomLevel Level, FName SelectedSystem, FName SelectedGalaxy, FName HomeRow)
{
	FCosmicPanelChoice Choice;
	if (Level == ECosmicZoomLevel::Universe)
	{
		Choice.Shown = SelectedGalaxy.IsNone() ? HomeRow : SelectedGalaxy;
		return Choice;
	}
	if (!SelectedSystem.IsNone())
	{
		Choice.bSystemPanel = true;
		Choice.Shown = SelectedSystem;
		return Choice;
	}
	Choice.Shown = HomeRow;
	return Choice;
}

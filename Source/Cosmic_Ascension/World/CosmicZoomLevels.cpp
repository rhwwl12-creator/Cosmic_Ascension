// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicZoomLevels.h"

FCosmicZoomLevels CosmicZoom::LevelsOf(const FVector& DiscCentre, float DiscRadius, float Reach)
{
	// A compare, so a radius that is not a number frames the smallest disc.
	const float Disc = DiscRadius > MinDiscRadius ? DiscRadius : MinDiscRadius;
	FCosmicZoomLevels Levels;
	Levels.Centre = FVector(DiscCentre.X, DiscCentre.Y, 0.0);
	Levels.RegionRadius = Disc;
	Levels.RegionNear = RegionNearArm;
	Levels.RegionFar = RegionFarFactor * Disc;
	Levels.RegionBound = RegionBoundFactor * Disc;
	Levels.RegionEntry = RegionEntryFactor * Disc;
	const float Near = Levels.RegionFar / StepFactor;
	const float Far = UniverseFarFactor * Reach;
	// Compares: a reach that is not a number opens nothing.
	if (Reach > 0.f && Far > Near)
	{
		Levels.UniverseNear = Near;
		Levels.UniverseFar = Far;
		Levels.UniverseBound = Reach;
	}
	return Levels;
}

ECosmicZoomLevel CosmicZoom::LevelOf(float Arm, const FCosmicZoomLevels& Levels)
{
	return Levels.HasUniverse() && Arm > Levels.RegionFar ? ECosmicZoomLevel::Universe : ECosmicZoomLevel::Region;
}

float CosmicZoom::BoundOf(ECosmicZoomLevel Level, const FCosmicZoomLevels& Levels)
{
	return Level == ECosmicZoomLevel::Universe && Levels.HasUniverse() ? Levels.UniverseBound : Levels.RegionBound;
}

bool CosmicZoom::EntersRegion(const FVector& Focus, const FCosmicZoomLevels& Levels)
{
	const double Dx = Focus.X - Levels.Centre.X;
	const double Dy = Focus.Y - Levels.Centre.Y;
	return Dx * Dx + Dy * Dy <= static_cast<double>(Levels.RegionEntry) * Levels.RegionEntry;
}

float CosmicZoom::StepArm(float Arm, int32 Direction, const FCosmicZoomLevels& Levels, const FVector& Focus)
{
	if (!(Arm > 0.f) || Direction == 0)
	{
		return Arm;
	}
	const bool bOpen = Levels.HasUniverse();
	if (LevelOf(Arm, Levels) == ECosmicZoomLevel::Region)
	{
		if (Direction < 0)
		{
			// The region's far stop: one step away is the universe's near stop.
			if (bOpen && Arm >= Levels.RegionFar * (1.f - StopSlack))
			{
				return Levels.UniverseNear;
			}
			return FMath::Clamp(Arm / StepFactor, Levels.RegionNear, Levels.RegionFar);
		}
		return FMath::Clamp(Arm * StepFactor, Levels.RegionNear, Levels.RegionFar);
	}
	if (Direction > 0)
	{
		// The universe's near stop, or an arm between the levels: into the region only over the home galaxy.
		if (Arm <= Levels.UniverseNear * (1.f + StopSlack))
		{
			return EntersRegion(Focus, Levels) ? Levels.RegionFar : Levels.UniverseNear;
		}
		return FMath::Clamp(Arm * StepFactor, Levels.UniverseNear, Levels.UniverseFar);
	}
	return FMath::Clamp(Arm / StepFactor, Levels.UniverseNear, Levels.UniverseFar);
}

float CosmicZoom::ArmAfter(float Arm, float Steps, const FCosmicZoomLevels& Levels, const FVector& Focus)
{
	// A compare, so a number of steps that is not a number takes none.
	if (!(FMath::Abs(Steps) >= 0.5f))
	{
		return Arm;
	}
	const int32 Count = FMath::Min(FMath::RoundToInt(FMath::Abs(Steps)), MaxSteps);
	const int32 Direction = Steps > 0.f ? 1 : -1;
	float Now = Arm;
	for (int32 Step = 0; Step < Count; ++Step)
	{
		const float Next = StepArm(Now, Direction, Levels, Focus);
		if (Next == Now)
		{
			break;
		}
		Now = Next;
	}
	return Now;
}

float CosmicZoom::UniverseShareOf(float Arm, const FCosmicZoomLevels& Levels)
{
	if (!(Levels.HasUniverse() && Arm > Levels.RegionFar))
	{
		return 0.f;
	}
	if (!(Arm < Levels.UniverseNear))
	{
		return 1.f;
	}
	return (Arm - Levels.RegionFar) / (Levels.UniverseNear - Levels.RegionFar);
}

bool CosmicZoom::ShowsRegionDetail(float Share)
{
	return !(Share >= RegionDetailUntil);
}

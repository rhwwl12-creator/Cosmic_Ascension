// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicMapPick.h"

int32 CosmicMapPick::NearestOnScreen(TArrayView<const FCosmicScreenTarget> Targets, const FVector2D& Cursor, float MinRadius)
{
	int32 Best = INDEX_NONE;
	double BestDistance = 0.0;
	for (int32 Index = 0; Index < Targets.Num(); ++Index)
	{
		const FCosmicScreenTarget& Target = Targets[Index];
		const double Reach = FMath::Max(MinRadius, Target.Radius);
		const double Distance = FVector2D::Distance(Target.Centre, Cursor);
		// Strictly nearer: a tie keeps the lower index.
		if (Distance <= Reach && (Best == INDEX_NONE || Distance < BestDistance))
		{
			Best = Index;
			BestDistance = Distance;
		}
	}
	return Best;
}

bool CosmicMapPick::OntoDisc(const FVector& Origin, const FVector& Direction, FVector& OutPoint)
{
	// The ray reaches Z = 0 at Origin + T * Direction with T = -Origin.Z / Direction.Z, ahead of the origin only.
	if (FMath::IsNearlyZero(Direction.Z))
	{
		return false;
	}
	const double T = -Origin.Z / Direction.Z;
	if (!(T >= 0.0))
	{
		return false;
	}
	OutPoint = Origin + T * Direction;
	OutPoint.Z = 0.0;
	return true;
}

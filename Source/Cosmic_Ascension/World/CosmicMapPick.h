// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 6.4, 11 Phase 3.
// Pure helpers of the click and the drag: no UObject, no world.

#pragma once

#include "CoreMinimal.h"

/** A star as the screen shows it: its projected centre and radius in pixels. */
struct COSMIC_ASCENSION_API FCosmicScreenTarget
{
	FVector2D Centre = FVector2D::ZeroVector;
	float Radius = 0.f;
};

namespace CosmicMapPick
{
	/** The index of the target nearest Cursor among those whose centre lies within max(MinRadius, its radius) of it;
	 * INDEX_NONE when none does. A tie goes to the lower index. */
	COSMIC_ASCENSION_API int32 NearestOnScreen(TArrayView<const FCosmicScreenTarget> Targets, const FVector2D& Cursor, float MinRadius);

	/** Where the ray from Origin along Direction meets the disc plane Z = 0; false when it runs parallel to the plane or
	 * away from it. */
	COSMIC_ASCENSION_API bool OntoDisc(const FVector& Origin, const FVector& Direction, FVector& OutPoint);
}

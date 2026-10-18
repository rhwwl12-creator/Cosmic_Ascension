// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 6.4, 11 Phase 3.
// The zoom levels of the one map and the rule of the wheel (section 6.4: "the zoom levels are the system, the region, the
// galaxy and the universe"). Pure: no UObject, no world, no state. Phase 3 item 9 builds two - the region (the map of the
// home galaxy's systems) and the universe (the home galaxy among the named ones and the scenery); v0.8 step 5 puts the
// galaxy level between them and step 6 the system level below, one enumerator and one pair of stops each.

#pragma once

#include "CoreMinimal.h"

/** The level an arm length stands in. Presentation state of the camera: never saved, outside both hashes. */
enum class ECosmicZoomLevel : uint8
{
	// The home galaxy's map: its systems, lanes, rings, labels and backdrop.
	Region,
	// The home galaxy as one galaxy among the named ones and the scenery (section 5.11): the region's detail is not drawn.
	Universe,
	// The number of levels (tests walk them).
	Count,
};

/**
 * The arm lengths and pan bounds of the levels, in world units, about Centre on the disc plane (CosmicZoom::LevelsOf: the
 * map computes them at a rebuild and the camera takes them, ACosmicCameraPawn::FrameAll). The universe begins exactly one
 * wheel step past the region's far stop, so no step comes to rest between the two levels and a picture at rest is always
 * one level's. A universe that is not open (HasUniverse) leaves the region alone: the wheel stops at the region's far stop,
 * as before Phase 3 item 9. The defaults are the camera's before its first framing.
 */
struct COSMIC_ASCENSION_API FCosmicZoomLevels
{
	FVector Centre = FVector::ZeroVector;
	// The disc radius the region frames (at least CosmicZoom::MinDiscRadius).
	float RegionRadius = 10000.f;
	float RegionNear = 1500.f;
	float RegionFar = 30000.f;
	float RegionBound = 10000.f;
	// A step nearer from the universe's near stop enters the region only while the camera's target focus stands this near
	// the centre (within the home galaxy's light); elsewhere it stays at the near stop - the portrait of another galaxy.
	float RegionEntry = 16000.f;
	// 0 while the universe is closed.
	float UniverseNear = 0.f;
	float UniverseFar = 0.f;
	float UniverseBound = 0.f;

	/** The universe is open: its near stop past the region's far stop, its far stop past its near stop and its bound above
	 * 0 (compares, so a value that is not a number closes it). */
	bool HasUniverse() const { return UniverseNear > RegionFar && UniverseFar > UniverseNear && UniverseBound > 0.f; }
};

namespace CosmicZoom
{
	// One wheel step comes nearer by this factor and goes away by its inverse.
	inline constexpr float StepFactor = 0.85f;
	// The region of a disc: the near stop; the far stop at RegionFarFactor x the radius; the pan bound at RegionBoundFactor
	// x the radius. A disc smaller than MinDiscRadius, or not a number, is taken as MinDiscRadius (a board of a few systems)
	// - the map's own floor (ACosmicGalaxyMapActor and CosmicUniverseLook::FrameOf read it).
	inline constexpr float RegionNearArm = 1500.f;
	inline constexpr float RegionFarFactor = 3.f;
	inline constexpr float RegionBoundFactor = 1.1f;
	inline constexpr float MinDiscRadius = 1000.f;
	// The framing of the region (ACosmicCameraPawn::FrameAll, the map's view distance after a rebuild): this x the radius.
	inline constexpr float FrameArmFactor = 2.25f;
	// RegionEntry is this x the radius: the home galaxy's sprite (CosmicUniverseLook::HomeSpriteShare).
	inline constexpr float RegionEntryFactor = 1.6f;
	// The universe's far stop is this x its reach (CosmicUniverseLook::ReachOf) - every named galaxy of the game on a 16:9
	// screen at the start view's pitch (UI.Universe holds it on the game's rows); its pan bound is the reach.
	inline constexpr float UniverseFarFactor = 1.25f;
	// An arm within this share of a stop is at the stop (it absorbs the rounding of x0.85 and its inverse).
	inline constexpr float StopSlack = 0.001f;
	// At most this many steps are taken by one call (a guard on the loop).
	inline constexpr int32 MaxSteps = 1000;
	// From this share of the step between the levels on, the region's detail is not drawn (its stars, their labels, the
	// lanes and the rings of systems) and a click takes named galaxies.
	inline constexpr float RegionDetailUntil = 0.5f;

	/** The levels of the disc of DiscCentre and DiscRadius with the universe opened for Reach (world units): the region as
	 * above; the universe's near stop RegionFar / StepFactor - one wheel step past the region -, its far stop
	 * UniverseFarFactor x Reach, its bound Reach; RegionEntry RegionEntryFactor x the radius. A Reach that is not a positive
	 * number, or whose far stop would not pass the near stop, leaves the universe closed. Centre is put on the disc plane. */
	COSMIC_ASCENSION_API FCosmicZoomLevels LevelsOf(const FVector& DiscCentre, float DiscRadius, float Reach);

	/** The level of an arm length: Universe past the region's far stop while the universe is open, Region otherwise (an
	 * arm that is not a number is the region). */
	COSMIC_ASCENSION_API ECosmicZoomLevel LevelOf(float Arm, const FCosmicZoomLevels& Levels);

	/** The pan bound of a level about Levels.Centre (the region's while the universe is closed). */
	COSMIC_ASCENSION_API float BoundOf(ECosmicZoomLevel Level, const FCosmicZoomLevels& Levels);

	/** Whether Focus stands within RegionEntry of the levels' centre on the disc plane (a compare: a focus that is not a
	 * number does not). */
	COSMIC_ASCENSION_API bool EntersRegion(const FVector& Focus, const FCosmicZoomLevels& Levels);

	/**
	 * One wheel step from Arm with the camera's target focus at Focus: Direction above 0 comes nearer, below 0 goes away, 0
	 * stays. Within a level the arm moves by StepFactor and stays within that level's stops; from the region's far stop a
	 * step away comes to the universe's near stop; from the universe's near stop (or an arm between the levels) a step nearer
	 * comes to the region's far stop when Focus EntersRegion and to the universe's near stop otherwise. An arm that is not a
	 * positive number stays.
	 */
	COSMIC_ASCENSION_API float StepArm(float Arm, int32 Direction, const FCosmicZoomLevels& Levels, const FVector& Focus);

	/** Steps wheel steps from Arm, one StepArm each with Focus: Steps rounded to whole steps, at most MaxSteps, ending early
	 * once a step changes nothing (a stop). Under half a step, and a value that is not a number, take none.
	 * ACosmicCameraPawn::Zoom applies it - one step a notch of the mouse. */
	COSMIC_ASCENSION_API float ArmAfter(float Arm, float Steps, const FCosmicZoomLevels& Levels, const FVector& Focus);

	/** Where an arm stands in the step between the levels, 0..1: 0 at and below the region's far stop, 1 at and past the
	 * universe's near stop, in proportion between; 0 for a closed universe and for an arm that is not a number. What fades
	 * with the distance reads it of the camera's actual (gliding) arm; no arm at rest stands strictly between 0 and 1. */
	COSMIC_ASCENSION_API float UniverseShareOf(float Arm, const FCosmicZoomLevels& Levels);

	/** Whether the region's detail is drawn at Share (UniverseShareOf): below RegionDetailUntil; a share that is not a
	 * number draws it. */
	COSMIC_ASCENSION_API bool ShowsRegionDetail(float Share);
}

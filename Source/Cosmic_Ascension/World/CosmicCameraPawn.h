// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 6.4, 11 Phase 3.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "World/CosmicZoomLevels.h"
#include "CosmicCameraPawn.generated.h"

class UCameraComponent;
class USceneComponent;
class USpringArmComponent;

/**
 * The camera over the galaxy map: the root is the focus point on the disc plane (Z = 0), a spring arm turns about it and
 * the camera sits at its end. Every move sets a target and the pawn glides to it in real time (the simulation's pause
 * does not freeze it); a pan moves at once, so the point the mouse grabbed stays under it. The player controller drives
 * it from the mouse; it reads no state. Two zoom levels of the one map (CosmicZoom): the region of the disc and the
 * universe one wheel step past the region's far stop, as the levels handed to FrameAll say. The pan bound and the pull of
 * the focus follow the level of the target arm.
 */
UCLASS()
class COSMIC_ASCENSION_API ACosmicCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ACosmicCameraPawn();

	/** The region of InLevels in view: focus on its Centre, arm length CosmicZoom::FrameArmFactor x its RegionRadius, pitch
	 * -59, yaw 0 (targets; SnapToTargets jumps). The wheel's stops and the pan bounds become InLevels' - the map's
	 * ACosmicGalaxyMapActor::GetZoomLevels (CosmicZoom::LevelsOf of its disc and its universe's reach). */
	void FrameAll(const FCosmicZoomLevels& InLevels);
	/** Turns about the focus: yaw free, pitch kept within [-85, -20] degrees. */
	void Orbit(float YawDegrees, float PitchDegrees);
	/** Wheel steps (CosmicZoom::ArmAfter with the target focus): x0.85 a step within a level, one step from a level's edge to
	 * the other's - into the region only over the home galaxy. The target focus is then kept within the bound of the level
	 * the new target arm stands in: a step from the universe into the region glides the camera home. */
	void Zoom(float Steps);
	/** Moves the focus on the disc plane at once: the target by Delta, kept within the bound of the level the target arm
	 * stands in, and the actor by the same move - never snapped to the target (a drag during a glide does not jump). */
	void Pan(const FVector& Delta);
	/** Glides to Location and comes to arm length 3,000 unless already nearer - a system is looked at in the region, from
	 * the universe too: the arm is set first, then the focus is kept within the region's bound. */
	void FocusOn(const FVector& Location);
	/** Glides to Location (within the universe's bound) at ArmLength (within the universe's stops; one that is not a positive
	 * number is the near stop) - a named galaxy. False, and nothing moves, while the universe is closed. */
	bool FocusOnUniverse(const FVector& Location, float ArmLength);
	/** Jumps to the targets now. */
	void SnapToTargets();

	FVector GetTargetFocus() const { return TargetFocus; }
	float GetTargetArmLength() const { return TargetArmLength; }
	float GetTargetYaw() const { return TargetYaw; }
	float GetTargetPitch() const { return TargetPitch; }
	/** The arm length now (equal to the target once the glide ends). */
	float GetArmLength() const;
	/** The levels FrameAll set last (the defaults until the first: arm stops 1,500 and 30,000, bound 10,000 about the
	 * origin, the universe closed). */
	const FCosmicZoomLevels& GetZoomLevels() const { return Levels; }
	/** The level of the target arm (CosmicZoom::LevelOf): the pan bound, the pull of the focus and the right side read it. */
	ECosmicZoomLevel GetTargetLevel() const;

protected:
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Focus;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> Arm;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	FVector TargetFocus = FVector::ZeroVector;
	float TargetArmLength = 17500.f;
	float TargetYaw = 0.f;
	float TargetPitch = -59.f;
	// Set by FrameAll.
	FCosmicZoomLevels Levels;
};

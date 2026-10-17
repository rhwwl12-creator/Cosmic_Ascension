// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 6.4, 11 Phase 3.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CosmicCameraPawn.generated.h"

class UCameraComponent;
class USceneComponent;
class USpringArmComponent;

/**
 * The camera over the galaxy map: the root is the focus point on the disc plane (Z = 0), a spring arm turns about it and
 * the camera sits at its end. Every move sets a target and the pawn glides to it in real time (the simulation's pause
 * does not freeze it); a pan moves at once, so the point the mouse grabbed stays under it. The player controller drives
 * it from the mouse; it reads no state.
 */
UCLASS()
class COSMIC_ASCENSION_API ACosmicCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ACosmicCameraPawn();

	/** The whole disc in view: focus on Centre, arm length 2.25 x Radius, pitch -59, yaw 0. Sets the pan bounds (1.1 x
	 * Radius about Centre) and the zoom-out limit (3 x Radius). */
	void FrameAll(const FVector& Centre, float Radius);
	/** Turns about the focus: yaw free, pitch kept within [-85, -20] degrees. */
	void Orbit(float YawDegrees, float PitchDegrees);
	/** Wheel steps: each positive step comes nearer by x0.85, each negative goes away by the inverse; the arm length stays
	 * within [1,500, the zoom-out limit]. */
	void Zoom(float Steps);
	/** Moves the focus on the disc plane; it stays within the pan bounds. */
	void Pan(const FVector& Delta);
	/** Glides to Location (on the disc plane, within the pan bounds) and comes to arm length 3,000 unless already nearer. */
	void FocusOn(const FVector& Location);
	/** Jumps to the targets now. */
	void SnapToTargets();

	FVector GetTargetFocus() const { return TargetFocus; }
	float GetTargetArmLength() const { return TargetArmLength; }
	float GetTargetYaw() const { return TargetYaw; }
	float GetTargetPitch() const { return TargetPitch; }
	/** The arm length now (equal to the target once the glide ends). */
	float GetArmLength() const;

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
	FVector BoundsCentre = FVector::ZeroVector;
	float BoundsRadius = 10000.f;
	float MaxArmLength = 30000.f;
};

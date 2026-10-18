// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicCameraImpl
{
	// The view of item 1: above and behind the disc, looking at its centre, the whole disc in view (the arm is
	// CosmicZoom::FrameArmFactor x the radius; the stops and the bounds are CosmicZoom's).
	constexpr float FramePitch = -59.f;
	constexpr float FocusArmLength = 3000.f;
	constexpr float MinPitch = -85.f;
	constexpr float MaxPitch = -20.f;
	// Per second: the share of the way left that a glide closes is 1 - exp(-GlideRate x seconds).
	constexpr float GlideRate = 10.f;

	// Point on the disc plane within Bound of Centre.
	FVector ClampToBound(const FVector& Point, const FVector& Centre, float Bound)
	{
		const FVector Plane(Centre.X, Centre.Y, 0.0);
		FVector Next(Point.X, Point.Y, 0.0);
		const FVector Offset = Next - Plane;
		if (Offset.Size2D() > Bound)
		{
			Next = Plane + Offset.GetSafeNormal2D() * Bound;
		}
		Next.Z = 0.0;
		return Next;
	}
}

ACosmicCameraPawn::ACosmicCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	// The controller's rotation never turns the pawn: the arm holds the view.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	SetActorEnableCollision(false);

	Focus = CreateDefaultSubobject<USceneComponent>(TEXT("Focus"));
	RootComponent = Focus;

	Arm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Arm"));
	Arm->SetupAttachment(Focus);
	Arm->bDoCollisionTest = false;
	Arm->bEnableCameraLag = false;
	Arm->bEnableCameraRotationLag = false;
	Arm->bUsePawnControlRotation = false;
	Arm->TargetArmLength = TargetArmLength;
	Arm->SetRelativeRotation(FRotator(TargetPitch, TargetYaw, 0.f));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Arm, USpringArmComponent::SocketName);
	// Emissive stars on a dark sky: a fixed exposure keeps them from flaring and fading as the view moves.
	Camera->PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
	Camera->PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
	Camera->PostProcessSettings.AutoExposureMinBrightness = 1.f;
	Camera->PostProcessSettings.AutoExposureMaxBrightness = 1.f;
}

void ACosmicCameraPawn::FrameAll(const FCosmicZoomLevels& InLevels)
{
	using namespace CosmicCameraImpl;

	Levels = InLevels;
	TargetFocus = FVector(Levels.Centre.X, Levels.Centre.Y, 0.0);
	TargetArmLength = CosmicZoom::FrameArmFactor * Levels.RegionRadius;
	TargetYaw = 0.f;
	TargetPitch = FramePitch;
}

ECosmicZoomLevel ACosmicCameraPawn::GetTargetLevel() const
{
	return CosmicZoom::LevelOf(TargetArmLength, Levels);
}

void ACosmicCameraPawn::Orbit(float YawDegrees, float PitchDegrees)
{
	using namespace CosmicCameraImpl;

	TargetYaw = FRotator::NormalizeAxis(TargetYaw + YawDegrees);
	TargetPitch = FMath::Clamp(TargetPitch + PitchDegrees, MinPitch, MaxPitch);
}

void ACosmicCameraPawn::Zoom(float Steps)
{
	using namespace CosmicCameraImpl;

	TargetArmLength = CosmicZoom::ArmAfter(TargetArmLength, Steps, Levels, TargetFocus);
	// Into the region the focus is pulled home: its bound is the level's of the new arm.
	TargetFocus = ClampToBound(TargetFocus, Levels.Centre, CosmicZoom::BoundOf(GetTargetLevel(), Levels));
}

void ACosmicCameraPawn::Pan(const FVector& Delta)
{
	using namespace CosmicCameraImpl;

	// Within the bound of the target arm's level, on the plane.
	const FVector Before = TargetFocus;
	TargetFocus = ClampToBound(TargetFocus + FVector(Delta.X, Delta.Y, 0.0), Levels.Centre, CosmicZoom::BoundOf(GetTargetLevel(), Levels));
	// At once, not by a glide: the controller pans by the mouse's movement each frame. The actor moves by the same move -
	// never snapped to the target, so a drag during a glide does not jump.
	FVector Moved = TargetFocus - Before;
	Moved.Z = 0.0;
	SetActorLocation(GetActorLocation() + Moved);
}

void ACosmicCameraPawn::FocusOn(const FVector& Location)
{
	using namespace CosmicCameraImpl;

	// The arm first: from the universe a system is looked at in the region, so the region's bound holds the focus.
	TargetArmLength = FMath::Min(TargetArmLength, FocusArmLength);
	TargetFocus = ClampToBound(Location, Levels.Centre, Levels.RegionBound);
}

bool ACosmicCameraPawn::FocusOnUniverse(const FVector& Location, float ArmLength)
{
	using namespace CosmicCameraImpl;

	if (!Levels.HasUniverse())
	{
		return false;
	}
	TargetArmLength = ArmLength > 0.f ? FMath::Clamp(ArmLength, Levels.UniverseNear, Levels.UniverseFar) : Levels.UniverseNear;
	TargetFocus = ClampToBound(Location, Levels.Centre, Levels.UniverseBound);
	return true;
}

void ACosmicCameraPawn::SnapToTargets()
{
	SetActorLocation(TargetFocus);
	Arm->TargetArmLength = TargetArmLength;
	Arm->SetRelativeRotation(FRotator(TargetPitch, TargetYaw, 0.f));
}

float ACosmicCameraPawn::GetArmLength() const
{
	return Arm->TargetArmLength;
}

void ACosmicCameraPawn::Tick(float DeltaSeconds)
{
	using namespace CosmicCameraImpl;

	Super::Tick(DeltaSeconds);
	const float Share = 1.f - FMath::Exp(-GlideRate * FMath::Max(DeltaSeconds, 0.f));

	const FVector Location = GetActorLocation();
	if (!Location.Equals(TargetFocus, 1.0))
	{
		SetActorLocation(FMath::Lerp(Location, TargetFocus, static_cast<double>(Share)));
	}
	else if (Location != TargetFocus)
	{
		SetActorLocation(TargetFocus);
	}

	const float Length = Arm->TargetArmLength;
	Arm->TargetArmLength = FMath::IsNearlyEqual(Length, TargetArmLength, 1.f) ? TargetArmLength : FMath::Lerp(Length, TargetArmLength, Share);

	// Yaw the short way round.
	const FRotator Rotation = Arm->GetRelativeRotation();
	const float YawLeft = FMath::FindDeltaAngleDegrees(static_cast<float>(Rotation.Yaw), TargetYaw);
	const float PitchLeft = TargetPitch - static_cast<float>(Rotation.Pitch);
	if (FMath::Abs(YawLeft) > 0.05f || FMath::Abs(PitchLeft) > 0.05f)
	{
		Arm->SetRelativeRotation(FRotator(Rotation.Pitch + PitchLeft * Share, Rotation.Yaw + YawLeft * Share, 0.f));
	}
	else if (YawLeft != 0.f || PitchLeft != 0.f)
	{
		Arm->SetRelativeRotation(FRotator(TargetPitch, TargetYaw, 0.f));
	}
}

// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicCameraImpl
{
	// The view of item 1: above and behind the disc, looking at its centre, the whole disc in view.
	constexpr float FramePitch = -59.f;
	constexpr float FrameArmFactor = 2.25f;
	constexpr float MaxArmFactor = 3.f;
	constexpr float PanBoundsFactor = 1.1f;
	// A disc smaller than this is framed as this (a board of a few systems).
	constexpr float MinDiscRadius = 1000.f;
	// Near enough that a star (300 across) fills a good part of the screen.
	constexpr float MinArmLength = 1500.f;
	constexpr float FocusArmLength = 3000.f;
	constexpr float ZoomStep = 0.85f;
	constexpr float MinPitch = -85.f;
	constexpr float MaxPitch = -20.f;
	// Per second: the share of the way left that a glide closes is 1 - exp(-GlideRate x seconds).
	constexpr float GlideRate = 10.f;
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

void ACosmicCameraPawn::FrameAll(const FVector& Centre, float Radius)
{
	using namespace CosmicCameraImpl;

	// A compare, so a radius that is not a number frames the smallest disc.
	const float Disc = Radius > MinDiscRadius ? Radius : MinDiscRadius;
	BoundsCentre = FVector(Centre.X, Centre.Y, 0.0);
	BoundsRadius = PanBoundsFactor * Disc;
	MaxArmLength = MaxArmFactor * Disc;
	TargetFocus = BoundsCentre;
	TargetArmLength = FrameArmFactor * Disc;
	TargetYaw = 0.f;
	TargetPitch = FramePitch;
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

	TargetArmLength = FMath::Clamp(TargetArmLength * FMath::Pow(ZoomStep, Steps), MinArmLength, MaxArmLength);
}

void ACosmicCameraPawn::Pan(const FVector& Delta)
{
	// Within the bounds, on the plane.
	FVector Next = TargetFocus + FVector(Delta.X, Delta.Y, 0.0);
	const FVector Offset = Next - BoundsCentre;
	if (Offset.Size2D() > BoundsRadius)
	{
		Next = BoundsCentre + Offset.GetSafeNormal2D() * BoundsRadius;
	}
	Next.Z = 0.0;
	// At once, not by a glide: the controller pans by the mouse's movement each frame.
	TargetFocus = Next;
	SetActorLocation(TargetFocus);
}

void ACosmicCameraPawn::FocusOn(const FVector& Location)
{
	using namespace CosmicCameraImpl;

	FVector Next(Location.X, Location.Y, 0.0);
	const FVector Offset = Next - BoundsCentre;
	if (Offset.Size2D() > BoundsRadius)
	{
		Next = BoundsCentre + Offset.GetSafeNormal2D() * BoundsRadius;
	}
	TargetFocus = Next;
	TargetArmLength = FMath::Min(TargetArmLength, FocusArmLength);
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

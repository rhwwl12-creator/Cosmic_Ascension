// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicStarSystemActor.h"

#include "Blueprint/UserWidget.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Cosmic_Ascension.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UI/CosmicSystemLabelWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicStarSystemImpl
{
	const TCHAR* const StarMaterialPath = TEXT("/Game/CA/Materials/M_Star.M_Star");
	// The parameters of M_Star (Tools/make_materials.py writes them).
	const FName StarColorParam(TEXT("StarColor"));
	const FName BrightnessParam(TEXT("Brightness"));
	const FName HazardAmountParam(TEXT("HazardAmount"));

	// The engine sphere is 100 units across; a star of scale 1 is 300 (systems stand about 1,000 apart).
	constexpr float SphereRadius = 50.f;
	constexpr float BaseScale = 3.f;
	// Low enough that the tone mapper keeps the colour of the class instead of washing it to white.
	constexpr float Brightness = 1.3f;
	constexpr float DeadBrightness = 0.4f;
	const FLinearColor DeadColor(0.35f, 0.35f, 0.4f);
}

ACosmicStarSystemActor::ACosmicStarSystemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Star = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Star"));
	RootComponent = Star;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Sphere.Succeeded())
	{
		Star->SetStaticMesh(Sphere.Object);
	}
	Star->SetCastShadow(false);
	// The click picks in screen space (CosmicMapPick), and nothing walks into a star.
	Star->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Label = CreateDefaultSubobject<UWidgetComponent>(TEXT("Label"));
	Label->SetupAttachment(Star);
	Label->SetWidgetSpace(EWidgetSpace::Screen);
	Label->SetWidgetClass(UCosmicSystemLabelWidget::StaticClass());
	Label->SetDrawSize(FVector2D(160.f, 40.f));
	Label->SetPivot(FVector2D(0.5f, 1.f));
	// Above the sphere (radius 50 in its own units) by enough that the camera's slant (pitch -59: a height shows at about
	// half its length) still lifts the label clear of the star.
	Label->SetRelativeLocation(FVector(0.f, 0.f, 110.f));
	Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACosmicStarSystemActor::Bind(FName InSystemID)
{
	SystemID = InSystemID;
}

void ACosmicStarSystemActor::BeginPlay()
{
	Super::BeginPlay();
	// Loaded here, not by a constructor finder: a finder roots the material for the whole process, and the script that
	// refills it in place (Tools/make_materials.py) cannot then rebuild its graph. Cooked by DirectoriesToAlwaysCook.
	if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, CosmicStarSystemImpl::StarMaterialPath, nullptr, LOAD_NoWarn))
	{
		StarMaterial = Star->CreateAndSetMaterialInstanceDynamicFromMaterial(0, Base);
	}
	else
	{
		UE_LOG(LogCosmic, Error, TEXT("%s is missing: run Tools/make_materials.py"), CosmicStarSystemImpl::StarMaterialPath);
	}
	ApplyLook(Applied);
}

void ACosmicStarSystemActor::ApplyLook(const FCosmicSystemLook& Look)
{
	using namespace CosmicStarSystemImpl;

	Applied = Look;
	SetActorScale3D(FVector(BaseScale * (Look.bDead ? 0.6f : Look.StarScale)));
	if (StarMaterial)
	{
		StarMaterial->SetVectorParameterValue(StarColorParam, Look.bDead ? DeadColor : Look.StarColor);
		StarMaterial->SetScalarParameterValue(BrightnessParam, Look.bDead ? DeadBrightness : Brightness);
		StarMaterial->SetScalarParameterValue(HazardAmountParam, Look.bHazard ? 1.f : 0.f);
	}
	if (UCosmicSystemLabelWidget* Widget = Label ? Cast<UCosmicSystemLabelWidget>(Label->GetUserWidgetObject()) : nullptr)
	{
		Widget->Show(Look);
		Widget->SetVisibility(bLabelVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void ACosmicStarSystemActor::SetLabelVisible(bool bVisible)
{
	bLabelVisible = bVisible;
	if (UUserWidget* Widget = Label ? Label->GetUserWidgetObject() : nullptr)
	{
		// On the widget itself, which hides it at once whatever the component's own tick does.
		Widget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

bool ACosmicStarSystemActor::IsLabelVisible() const
{
	const UUserWidget* Widget = Label ? Label->GetUserWidgetObject() : nullptr;
	return Widget ? Widget->GetVisibility() != ESlateVisibility::Collapsed : bLabelVisible;
}

float ACosmicStarSystemActor::GetStarRadius() const
{
	return CosmicStarSystemImpl::SphereRadius * static_cast<float>(GetActorScale3D().X);
}

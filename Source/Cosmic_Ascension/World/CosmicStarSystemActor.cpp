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
	// The parameters of M_Star under the names CosmicMapLook::StarParam keeps (Tools/make_materials.py writes them), each
	// looked up once: a MID setter takes an FName.
	const FName StarColorParam(CosmicMapLook::StarParam::StarColor);
	const FName BrightnessParam(CosmicMapLook::StarParam::Brightness);
	const FName HazardAmountParam(CosmicMapLook::StarParam::HazardAmount);
	const FName WarningColorParam(CosmicMapLook::StarParam::WarningColor);
	const FName ViewModeParam(CosmicMapLook::StarParam::ViewMode);
	const FName PressureParam(CosmicMapLook::StarParam::Pressure);
	const FName DominionColorParam(CosmicMapLook::StarParam::DominionColor);
	const FName ClaimStateParam(CosmicMapLook::StarParam::ClaimState);

	// The engine sphere is 100 units across; a star of scale 1 is 300 (systems stand about 1,000 apart).
	constexpr float SphereRadius = 50.f;
	constexpr float BaseScale = 3.f;
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
	if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, CosmicMapLook::StarMaterialPath, nullptr, LOAD_NoWarn))
	{
		StarMaterial = Star->CreateAndSetMaterialInstanceDynamicFromMaterial(0, Base);
	}
	else
	{
		UE_LOG(LogCosmic, Error, TEXT("%s is missing: run Tools/make_materials.py"), CosmicMapLook::StarMaterialPath);
	}
	ApplyLook(Applied, AppliedSpirit, AppliedView);
}

void ACosmicStarSystemActor::ApplyLook(const FCosmicSystemLook& Look, const FCosmicSpiritLook& Spirit, ECosmicViewMode Shown)
{
	using namespace CosmicStarSystemImpl;

	Applied = Look;
	AppliedSpirit = Spirit;
	AppliedView = Shown;
	AppliedParams = CosmicMapLook::StarParamsOf(Look, Spirit, Shown);
	SetActorScale3D(FVector(BaseScale * (Look.bDead ? 0.6f : Look.StarScale)));
	if (StarMaterial)
	{
		// A value equal to the one set before sends no render update (UMaterialInstance's setters compare).
		StarMaterial->SetVectorParameterValue(StarColorParam, AppliedParams.StarColor);
		StarMaterial->SetScalarParameterValue(BrightnessParam, AppliedParams.Brightness);
		StarMaterial->SetScalarParameterValue(HazardAmountParam, AppliedParams.HazardAmount);
		StarMaterial->SetVectorParameterValue(WarningColorParam, AppliedParams.WarningColor);
		StarMaterial->SetScalarParameterValue(ViewModeParam, AppliedParams.ViewMode);
		StarMaterial->SetScalarParameterValue(PressureParam, AppliedParams.Pressure);
		StarMaterial->SetVectorParameterValue(DominionColorParam, AppliedParams.DominionColor);
		StarMaterial->SetScalarParameterValue(ClaimStateParam, AppliedParams.ClaimState);
	}
	// A system the viewer does not know is a backdrop point: the sphere alone hides (the label, a child, is hidden through
	// its widget by the map's SetLabelVisible), and the map's backdrop draws the system's own point in its place.
	if (Star)
	{
		// The setter does nothing when the flag is already so.
		Star->SetVisibility(Look.bKnown, false);
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

// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 6.4, 11 Phase 3.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/CosmicMapLook.h"
#include "CosmicStarSystemActor.generated.h"

class UMaterialInstanceDynamic;
class UStaticMeshComponent;
class UWidgetComponent;

/**
 * One system on the map: the engine's basic sphere with a dynamic instance of M_Star, and a small screen-space label
 * (UCosmicSystemLabelWidget). It only shows the look it is given; the map actor reads the state and draws the owner's
 * ring around it (a line, readable from afar where a rim on the sphere is not). In the spiritual view (the map's choice)
 * its own colour recedes and it shows the pressure, the claim marker and the colour of the fallen (section 6.4). No
 * collision: the click picks in screen space (CosmicMapPick).
 */
UCLASS()
class COSMIC_ASCENSION_API ACosmicStarSystemActor : public AActor
{
	GENERATED_BODY()

public:
	ACosmicStarSystemActor();

	/** The system this actor draws, set once by the map. */
	void Bind(FName InSystemID);
	FName GetSystemID() const { return SystemID; }

	/** Sets the scale, the material parameters (CosmicMapLook::StarParamsOf of the three, under the names of
	 * CosmicMapLook::StarParam) and the label (the physical look - the sight changes no label). */
	void ApplyLook(const FCosmicSystemLook& Look, const FCosmicSpiritLook& Spirit, ECosmicViewMode Shown);
	/** The world radius of the star as the last look scaled it (the owner's ring stands outside it). */
	float GetStarRadius() const;
	/** Shows or hides the label (the map decides by CosmicMapLook::ShowsLabel). */
	void SetLabelVisible(bool bVisible);
	bool IsLabelVisible() const;
	/** The look applied last (the test reads it). */
	const FCosmicSystemLook& GetAppliedLook() const { return Applied; }
	/** The spiritual look and the material values applied last (tests read them). */
	const FCosmicSpiritLook& GetAppliedSpirit() const { return AppliedSpirit; }
	const FCosmicStarParams& GetAppliedParams() const { return AppliedParams; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Star;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent> Label;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> StarMaterial;

	FName SystemID;
	FCosmicSystemLook Applied;
	// Kept with Applied for BeginPlay, which applies all three again once the material instance exists.
	FCosmicSpiritLook AppliedSpirit;
	ECosmicViewMode AppliedView = ECosmicViewMode::Physical;
	FCosmicStarParams AppliedParams;
	// Kept for a label widget made after the call.
	bool bLabelVisible = true;
};

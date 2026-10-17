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
 * ring around it (a line, readable from afar where a rim on the sphere is not).
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

	/** Sets the scale, the material parameters and the label. */
	void ApplyLook(const FCosmicSystemLook& Look);
	/** The world radius of the star as the last look scaled it (the owner's ring stands outside it). */
	float GetStarRadius() const;
	/** The look applied last (the test reads it). */
	const FCosmicSystemLook& GetAppliedLook() const { return Applied; }

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
};

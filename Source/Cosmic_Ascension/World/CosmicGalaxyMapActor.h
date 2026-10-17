// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 6.4, 11 Phase 3.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CosmicGalaxyMapActor.generated.h"

class ACosmicStarSystemActor;
class ULineBatchComponent;
class UStaticMeshComponent;

/**
 * The galaxy map: one ACosmicStarSystemActor per system at its state Position, the lanes, a ring in the owner's colour
 * around each owned system (dashed while it is being settled), a white ring around the selected system and the nebula
 * sky. Rebuilt when a galaxy is generated or loaded, refreshed when a tick ran - the delegates only mark it, and its Tick
 * redraws at most once a frame. Labels thin out and lines keep about a pixel's width as the camera (ACosmicCameraPawn)
 * moves away. Reads the state; never changes it.
 */
UCLASS()
class COSMIC_ASCENSION_API ACosmicGalaxyMapActor : public AActor
{
	GENERATED_BODY()

public:
	ACosmicGalaxyMapActor();

	/** The actor that draws SystemID, or null. */
	ACosmicStarSystemActor* FindStar(FName SystemID) const;
	const TArray<TObjectPtr<ACosmicStarSystemActor>>& GetStars() const { return Stars; }
	/** The lanes drawn by the last refresh. */
	int32 GetDrawnLaneCount() const { return DrawnLanes; }
	/** The owner's rings drawn by the last refresh. */
	int32 GetDrawnRingCount() const { return DrawnRings; }

	/** Announced after every rebuild (a new galaxy or a load): the controller frames the disc and clears its selection. */
	FSimpleMulticastDelegate OnRebuilt;
	/** A rebuild waits for the next frame (a galaxy was generated or loaded since the last one). */
	bool IsRebuildPending() const { return bRebuildPending; }

	/** The centre and radius of the disc the systems stand on, as of the last rebuild (radius at least 1,000). */
	FVector GetDiscCentre() const { return DiscCentre; }
	float GetDiscRadius() const { return DiscRadius; }

	/** Draws the selection ring around SystemID (None: no ring) from the next frame on. A rebuild clears it. */
	void SetSelectedSystem(FName SystemID);
	/** The system the last refresh drew the selection ring around; None for none. */
	FName GetDrawnSelection() const { return DrawnSelection; }

	/** The camera's distance (its arm length): from afar labels thin out (CosmicMapLook::ShowsLabel) and lines keep about
	 * a pixel's width. Marks a refresh when the far view flips or the distance moved by more than 5%. */
	void SetViewDistance(float ArmLength);
	bool IsFarView() const { return bFarView; }
	/** The lane thickness of the last refresh, in world units. */
	float GetLaneThickness() const { return LaneThickness; }

	/** Applies what is pending now instead of at the next frame (tests). */
	void RefreshNow();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UFUNCTION()
	void HandleGalaxyGenerated(int32 Seed);

	UFUNCTION()
	void HandleSimTick(int64 Tick);

	// One star actor per system (old ones destroyed), the disc, a refresh, then OnRebuilt.
	void Rebuild();
	// Looks, labels, lanes and rings from the state.
	void Refresh();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULineBatchComponent> Lanes;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Sky;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ACosmicStarSystemActor>> Stars;

	bool bRebuildPending = true;
	bool bRefreshPending = false;
	int32 DrawnLanes = 0;
	int32 DrawnRings = 0;

	FName SelectedSystemID;
	FName DrawnSelection;
	float ViewDistance = 17500.f;
	bool bFarView = true;
	float LaneThickness = 30.f;
	FVector DiscCentre = FVector::ZeroVector;
	float DiscRadius = 1000.f;
};

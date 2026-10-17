// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 6.4, 11 Phase 3.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CosmicGalaxyMapActor.generated.h"

class ACosmicStarSystemActor;
class UCameraComponent;
class ULineBatchComponent;
class UStaticMeshComponent;

/**
 * The galaxy map: one ACosmicStarSystemActor per system at its state Position, the lanes, a ring in the owner's colour
 * around each owned system (dashed while it is being settled), the nebula sky and (until the camera pawn) a fixed
 * overview camera. Rebuilt when a galaxy is generated or loaded, refreshed when a tick ran - the
 * delegates only mark it, and its Tick redraws at most once a frame. Reads the state; never changes it.
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

	// One star actor per system (old ones destroyed), then a refresh.
	void Rebuild();
	// Looks, lanes and rings from the state.
	void Refresh();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULineBatchComponent> Lanes;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Sky;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Overview;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ACosmicStarSystemActor>> Stars;

	bool bRebuildPending = true;
	bool bRefreshPending = false;
	int32 DrawnLanes = 0;
	int32 DrawnRings = 0;
};

// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 6.4, 11 Phase 3.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/CosmicMapLook.h"
#include "World/CosmicUniverseLook.h"
#include "World/CosmicZoomLevels.h"
#include "CosmicGalaxyMapActor.generated.h"

class ACosmicStarSystemActor;
class UInstancedStaticMeshComponent;
class ULineBatchComponent;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;
class UWidgetComponent;

/**
 * The galaxy map: one ACosmicStarSystemActor per system at its state Position, the lanes, a ring in the owner's colour
 * around each owned system (dashed while it is being settled), a white ring around the selected system and the nebula
 * sky. Rebuilt when a galaxy is generated or loaded, refreshed when a tick ran - the delegates only mark it, and its Tick
 * redraws at most once a frame. Labels thin out and lines keep about a pixel's width as the camera (ACosmicCameraPawn)
 * moves away. In the spiritual sight (the controller's view, section 6.4) the stars show the fallen's pressure, marks and
 * colour, each lane is two halves greyed by the pressure at their end, a grey ring stands around every system of a sealed
 * sphere and the sky goes quiet (M_Nebula SpiritualIntensity). It draws what the viewer knows (CosmicMapLook::KnownViewOf,
 * section 6.4): a system the viewer does not know is a point of the backdrop - one instanced mesh of
 * DT_Rules.BackgroundStarCount points and one slot per system, lit while the system is unknown -, a lane only between two
 * known systems, an owner's ring only around a system of a people the viewer is shown, in its meeting colour. The
 * development aid CosmicSeeAll draws everything. Two zoom levels (section 6.4): the region - all of the above - and, one
 * wheel step farther, the universe: one instanced mesh of the scenery galaxies and the named galaxies of DT_Galaxies (the
 * home among them, at the disc's centre), their labels and the selected galaxy's ring (CosmicUniverseLook). Where the
 * camera stands between the two levels fades the backdrop into the home galaxy's sprite (material values, no redraw); from
 * the middle of the step on the region's stars, labels, lanes and rings are not drawn. The sky holds the universe. Reads
 * the state; never changes it.
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
	/** The lanes drawn by the last refresh (a lane drawn as two halves in the spiritual view counts once). */
	int32 GetDrawnLaneCount() const { return DrawnLanes; }
	/** The owner's rings drawn by the last refresh (around systems of peoples the viewer is shown). */
	int32 GetDrawnRingCount() const { return DrawnRings; }

	/**
	 * The backdrop (one instanced mesh, section 3.1 layer 1): instance i below GetBackdropPointCount() is point i of
	 * CosmicMapLook::BackdropPointsOf, instance GetBackdropPointCount() + j the slot of system j (CosmicMapLook::
	 * BackdropSlotOf at its place: its glow while the viewer does not know the system, 0 while it does). Tests read its
	 * instances and custom data. Hidden, and never built, when M_Backdrop is missing or cannot draw instanced meshes (an
	 * error line says so): the plane's own opaque material would cover the lanes.
	 */
	const UInstancedStaticMeshComponent* GetBackdrop() const { return Backdrop; }
	/** How many points the backdrop holds before its slots (CosmicMapLook::BackdropCountOf at the last build; -1 before). */
	int32 GetBackdropPointCount() const { return BackdropCount; }
	/** How many times the points were built since the map was spawned: a rebuild builds them again only when the seed, the
	 * home row, the count, the number of systems or the disc changed - a refresh never (tests). */
	int32 GetBackdropBuildCount() const { return BackdropBuilds; }
	/** How long the last build of the points took, in milliseconds (one LogCosmic line per build says it too). */
	double GetBackdropBuildMs() const { return BackdropBuildMs; }
	/** How many slot values the refreshes wrote since the map was spawned: only a slot whose knowledge flipped is written
	 * (tests - a refresh without a change writes none). */
	int32 GetSlotWrites() const { return SlotWrites; }
	/** The SpiritualIntensity the last refresh gave the backdrop (the sky's: the dust recedes with it). */
	float GetBackdropIntensity() const { return BackdropIntensity; }
	/** The sky (tests: the backdrop is a static mesh component too, so a lookup by class may not find the sky). */
	const UStaticMeshComponent* GetSky() const { return Sky; }

	/** The development aid CosmicSeeAll, as the controller hands it over: every system drawn as known and every people as
	 * shown (CosmicMapLook::KnownViewOf's bSeeAll); marks a refresh when it changed. Writes nothing. */
	void SetSeeAll(bool bInSeeAll);
	bool IsSeeingAll() const { return bSeeAll; }

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
	 * a pixel's width. Marks a refresh when the far view flips or the distance moved by more than 5%. It also reads where
	 * the distance stands between the levels (CosmicZoom::UniverseShareOf of GetZoomLevels) at every call: the backdrop's
	 * and the universe's LayerFade follow at once - material values, no redraw - and a flip of the region's detail
	 * (CosmicZoom::ShowsRegionDetail) marks a refresh. */
	void SetViewDistance(float ArmLength);
	/** The view distance the last refresh drew with. */
	float GetViewDistance() const { return ViewDistance; }

	/** The levels of the last rebuild (CosmicZoom::LevelsOf of the disc and GetUniverseReach): the controller frames the
	 * camera with them. */
	const FCosmicZoomLevels& GetZoomLevels() const { return ZoomLevels; }
	/** Where the last view distance stands between the levels (0 in the region, 1 at the universe). A rebuild puts it back
	 * to 0: a new galaxy or a load starts in the region. */
	float GetUniverseShare() const { return UniverseShare; }
	/** Whether the last refresh drew the region's detail: the stars, their labels, the lanes and every ring of a system. A
	 * click takes a star while it does and a named galaxy while it does not. */
	bool IsRegionDetailShown() const { return bRegionDetail; }

	/**
	 * The universe (one instanced mesh, section 5.11, 6.4): instance i below GetSceneryCount() is point i of
	 * CosmicUniverseLook::SceneryPointsOf (dimmed by ClearAroundMarks), instance GetSceneryCount() + j draws mark j of
	 * GetGalaxyMarks(). Tests read its instances and custom data. Visible only while the universe shows (a share above 0)
	 * and M_Backdrop can draw it; the backdrop is hidden at a share of 1.
	 */
	const UInstancedStaticMeshComponent* GetUniverse() const { return Universe; }
	int32 GetSceneryCount() const { return SceneryCount; }
	/** The universe's own instance of M_Backdrop: LayerFade and SpiritualIntensity apart from the backdrop's. */
	const UMaterialInstanceDynamic* GetUniverseMaterial() const { return UniverseMaterial; }
	/** The backdrop's own instance of M_Backdrop. */
	const UMaterialInstanceDynamic* GetBackdropMaterial() const { return BackdropMaterial; }
	/** The sky's own instance of M_Nebula. */
	const UMaterialInstanceDynamic* GetSkyMaterial() const { return SkyMaterial; }
	/** The named galaxies of the last rebuild (CosmicUniverseLook::MarksOf, lexical row order, the home among them), the
	 * frame they stand in and the universe's reach (CosmicUniverseLook::ReachOf). */
	const TArray<FCosmicGalaxyMark>& GetGalaxyMarks() const { return Marks; }
	const FCosmicUniverseFrame& GetUniverseFrame() const { return Frame; }
	float GetUniverseReach() const { return Reach; }
	/** How many times the universe's instances were built since the map was spawned: a rebuild builds them only when the
	 * home row, the scenery count, the frame, or a mark's row, place, size, aspect or turn changed - a refresh never, and
	 * never the run's seed (one sky for every run). */
	int32 GetUniverseBuildCount() const { return UniverseBuilds; }
	/** How long the last build of the universe's instances took, in milliseconds (one LogCosmic line per build says it). */
	double GetUniverseBuildMs() const { return UniverseBuildMs; }
	/** The named galaxies' labels, label j for mark j (screen space, the name alone). */
	const TArray<TObjectPtr<UWidgetComponent>>& GetGalaxyLabels() const { return GalaxyLabels; }
	/** Whether the label of mark MarkIndex shows (its widget's own visibility); false out of range. */
	bool IsGalaxyLabelVisible(int32 MarkIndex) const;
	/** How many refreshes ran since the map was spawned (tests: a fade alone redraws nothing). */
	int32 GetRefreshCount() const { return Refreshes; }

	/** Rings the named galaxy Row (None: no ring) at the universe level, from the next frame on. A rebuild clears it. */
	void SetSelectedGalaxy(FName Row);
	/** The galaxy the last refresh ringed; None for none, and always in the region. */
	FName GetDrawnGalaxySelection() const { return DrawnGalaxySelection; }
	bool IsFarView() const { return bFarView; }
	/** The lane thickness of the last refresh, in world units. */
	float GetLaneThickness() const { return LaneThickness; }

	/**
	 * The view the viewer wants and whether a development aid forces the sight open, as the player controller hands them
	 * over; marks a refresh when either changed. The view drawn is CosmicMapLook::ShownViewMode of these and of the
	 * simulation's viewer tier, read at every refresh: the tick of an ascension or a deposition turns it. A rebuild puts
	 * the wanted view back to Spiritual before it draws (a new galaxy or a load starts at the default).
	 */
	void SetViewWanted(ECosmicViewMode Wanted, bool bForcedOpen);
	/** The view the last refresh drew. */
	ECosmicViewMode GetDrawnViewMode() const { return DrawnView; }
	/** The Sealed rings the last refresh drew (spiritual view only). */
	int32 GetDrawnSealedRingCount() const { return DrawnSealedRings; }
	/** The half-lanes the last refresh drew greyed by pressure (spiritual view only). */
	int32 GetDrawnGreyedHalfCount() const { return DrawnGreyedHalves; }
	/** The SpiritualIntensity the last refresh gave the sky. */
	float GetSkyIntensity() const { return SkyIntensity; }

	/** Applies what is pending now instead of at the next frame (tests). */
	void RefreshNow();
	/** Redraws at the next frame: a command changed the state between ticks (a paused simulation sends no tick). */
	void RequestRefresh() { bRefreshPending = true; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UFUNCTION()
	void HandleGalaxyGenerated(int32 Seed);

	UFUNCTION()
	void HandleSimTick(int64 Tick);

	// One star actor per system (old ones destroyed), the disc, the backdrop, the wanted view back to the default, a
	// refresh, then OnRebuilt.
	void Rebuild();
	// Looks, labels, lanes, rings, the backdrop's slots and the sky from the state, the view and what the viewer knows.
	void Refresh();
	// The backdrop for the galaxy and the disc of this rebuild: the points again only when their key changed; the slots
	// placed again every time (the refresh lights the unknown ones).
	void BuildBackdrop(const FGalaxyState& State, const FCosmicRulesDef& Rules);
	// Lights the slots of the systems View does not know and darkens the others, writing only the slots whose value
	// changed (an incremental update of the instances; nothing when nothing changed).
	void LightSlots(const FGalaxyState& State, const FCosmicKnownView& View);
	// At every rebuild, after the backdrop: the frame, the marks (one Warning line for rows without a place), the reach and
	// the levels, the labels (made or destroyed to one per mark, placed and named), the sky's size; the instances only when
	// their key changed. Never called by a refresh.
	void BuildUniverse(const FCosmicRulesDef& Rules, const FCosmicDataTables& Tables);
	// LayerFade on the backdrop's and the universe's instances for Share (1 - Share and Share), the receded
	// SpiritualIntensity of the sky and the universe, and the two meshes' visibility - written when the share moved by more
	// than a slack, and always on reaching 0 or 1.
	void ApplyLayerFades(float Share);
	// The sky's and the universe's SpiritualIntensity for the view's SkyIntensity and the share written last.
	void WriteIntensities();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULineBatchComponent> Lanes;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Sky;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> Backdrop;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> Universe;

	// The universe's own instance of M_Backdrop (BeginPlay).
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> UniverseMaterial;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UWidgetComponent>> GalaxyLabels;

	// The sky's own instance of M_Nebula (BeginPlay), so that the view can set SpiritualIntensity.
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SkyMaterial;

	// The backdrop's own instance of M_Backdrop (BeginPlay), so that the view can set SpiritualIntensity.
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BackdropMaterial;

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

	ECosmicViewMode WantedView = ECosmicViewMode::Spiritual;
	bool bSightForcedOpen = false;
	ECosmicViewMode DrawnView = ECosmicViewMode::Physical;
	int32 DrawnSealedRings = 0;
	int32 DrawnGreyedHalves = 0;
	float SkyIntensity = 0.f;

	bool bSeeAll = false;
	// False when M_Backdrop is missing or cannot draw instanced meshes: the backdrop stays hidden and empty.
	bool bBackdropUsable = false;
	int32 BackdropBuilds = 0;
	double BackdropBuildMs = 0.0;
	int32 SlotWrites = 0;
	float BackdropIntensity = 0.f;
	// What the points were built for (BackdropCount -1: not built yet).
	int32 BackdropSeed = 0;
	FName BackdropRow;
	int32 BackdropCount = -1;
	int32 BackdropSystems = -1;
	FVector BackdropCentre = FVector::ZeroVector;
	float BackdropRadius = 0.f;
	// By system index: the glow its slot shows while the system is unknown (BackdropSlotOf, kept from the rebuild), and
	// the value written last.
	TArray<float> SlotGlows;
	TArray<float> SlotValues;

	// ---- The universe level (Phase 3 item 9) ----
	FCosmicZoomLevels ZoomLevels;
	FCosmicUniverseFrame Frame;
	TArray<FCosmicGalaxyMark> Marks;
	float Reach = 0.f;
	float UniverseShare = 0.f;
	// The share the fades were written for (-1: none yet).
	float WrittenShare = -1.f;
	bool bRegionDetail = true;
	// Whether the star actors are hidden for the universe (set at the flip, undone at the flip back and by a rebuild).
	bool bStarsHidden = false;
	// Whether a universe refresh set the galaxy labels' visibility since the region last collapsed them (the region's
	// refresh collapses them once, not at every refresh).
	bool bGalaxyLabelsSet = false;
	FName SelectedGalaxyRow;
	FName DrawnGalaxySelection;
	int32 SceneryCount = 0;
	int32 UniverseBuilds = 0;
	double UniverseBuildMs = 0.0;
	int32 Refreshes = 0;
	// What the instances were built for (InstanceCount -1: not built yet). KeyMarks keeps the marks' rows, places, sizes,
	// aspects and turns (their names are not part of the key: the labels are named at every rebuild).
	FName InstanceRow;
	int32 InstanceCount = -1;
	FCosmicUniverseFrame InstanceFrame;
	TArray<FCosmicGalaxyMark> KeyMarks;
};

// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 3.1, 3.4, 5.11, 6.4, 8.3, 9.2, 11 Phase 3.
// What the universe level of the map draws (section 6.4: a zoom level of the one map, not a second map) and what the right
// side of the screen shows there: the home galaxy and the named galaxies of DT_Galaxies at their places, all by one law;
// the scenery galaxies of the Scenery derive; the universe's reach for the camera and the sky; the labels, the ring, the
// reach of a click and the panel. Pure: no UObject, no draw, no write. The scenery draws from a local stream only - never
// the simulation's stream, never the tell stream (section 9.2, the third source).

#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "GameplayTagContainer.h"
#include "Math/RandomStream.h"
#include "World/CosmicZoomLevels.h"

struct FCosmicDataTables;
struct FCosmicRulesDef;
struct FGalaxyDef;

/** Where the universe stands on the map: its centre on the disc plane and its unit - one home-galaxy radius, in world
 * units. Before v0.8 step 5 the map's disc is drawn as the home galaxy itself (sys_0 at HomeRadiusShare 0.53 of it,
 * section 7.4), so the unit is the disc radius; step 5 replaces FrameOf when its galaxy level draws the home galaxy. */
struct COSMIC_ASCENSION_API FCosmicUniverseFrame
{
	FVector Centre = FVector::ZeroVector;
	float Unit = 1000.f;
};

/** One named galaxy as the universe draws it: a DT_Galaxies row - the home row too - by the one law of MarksOf. A picture:
 * no state, never saved, outside both hashes. */
struct COSMIC_ASCENSION_API FCosmicGalaxyMark
{
	FName Row;
	// The row's DisplayName as written: the label shows it alone (section 8.3); empty for a home the table lacks.
	FText Name;
	// On the disc plane.
	FVector Position = FVector::ZeroVector;
	// Half the sprite's length along its major axis, in world units (the engine plane is 100 across).
	float Radius = 0.f;
	// The minor axis over the major one, 0..1 (1: round).
	float Aspect = 1.f;
	// The sprite's turn about +Z, in degrees.
	float TurnDegrees = 0.f;
	// M_Backdrop's per-instance float: the same for every named galaxy (NamedGlow).
	float Glow = 0.f;
	bool bHome = false;
};

/** One scenery galaxy (section 5.11): a picture with no state, selection, label or panel. */
struct COSMIC_ASCENSION_API FCosmicSceneryPoint
{
	// On the disc plane.
	FVector Position = FVector::ZeroVector;
	// Across its major axis, in world units.
	float Size = 0.f;
	float Aspect = 1.f;
	float TurnDegrees = 0.f;
	// M_Backdrop's per-instance float; 0 draws nothing (a point under a named galaxy, ClearAroundMarks).
	float Glow = 0.f;
};

/** What a galaxy kind gives its sprite (CosmicUniverseLook::KindLookOf). */
struct COSMIC_ASCENSION_API FCosmicKindLook
{
	// The tag is one of the Galaxy.Kind tags of the table.
	bool bKnown = false;
	float Aspect = 0.7f;
	// The sprite's share of the unit for a row without a radius (RadiusLy 0).
	float DefaultShare = 0.45f;
};

/** What the right side of the screen shows (CosmicUniverseLook::PanelOf): the system panel of Shown when bSystemPanel, the
 * galaxy panel of Shown otherwise - never both (section 8.3). */
struct COSMIC_ASCENSION_API FCosmicPanelChoice
{
	bool bSystemPanel = false;
	FName Shown;
};

namespace CosmicUniverseLook
{
	// ---- The frame and the places (section 5.11, 7.3 MapPosition) ----

	// A place nearer the centre than MinPlaceUnits stands inside the home galaxy, and one farther than MaxPlaceUnits beyond
	// the universe: neither is a place.
	inline constexpr float MinPlaceUnits = 1.f;
	inline constexpr float MaxPlaceUnits = 20.f;

	/** The universe of the map's disc: its centre on the disc plane and its radius as the unit - a radius not above
	 * CosmicZoom::MinDiscRadius, or not a number, is that floor (the map's own). */
	COSMIC_ASCENSION_API FCosmicUniverseFrame FrameOf(const FVector& DiscCentre, float DiscRadius);

	/** The world place of MapPosition: the frame's centre plus (X, Y, 0) home-galaxy radii, in the map's own X and Y (the
	 * data's rule chose the handedness - Tools/data/sources/galaxies.md). */
	COSMIC_ASCENSION_API FVector PlaceOf(const FCosmicUniverseFrame& Frame, const FVector2D& MapPosition);

	/** Whether a row's MapPosition is a place: always for the home row (it stands at the centre whatever it holds); otherwise
	 * its length lies within [MinPlaceUnits, MaxPlaceUnits] - compares, so a value that is not a number is none, and a row
	 * left at (0,0) has no place yet. */
	COSMIC_ASCENSION_API bool HasPlace(const FVector2D& MapPosition, bool bHome);

	// ---- One law for every galaxy (section 6.4, 12.2: no galaxy is special) ----

	// The home's sprite radius in units: M_Backdrop's round falloff is half bright at 0.29 of a sprite's radius, so the
	// home's light ends near the backdrop's rim (1.15 units) - the dust condenses into it.
	inline constexpr float HomeSpriteShare = 1.6f;
	// The radius the law measures every row against, in light years: the home galaxy's D25 radius (guideline 3.4, about
	// 87,000 across). The home is drawn at HomeSpriteShare by definition, so the law has one definition for every row; the
	// home row's RadiusLy (50,000) stays the frame's unit.
	inline constexpr int32 HomeLawRadiusLy = 43500;
	// Every other sprite: HomeSpriteShare x sqrt(RadiusLy / HomeLawRadiusLy), kept within these.
	inline constexpr float MinSpriteShare = 0.3f;
	inline constexpr float MaxSpriteShare = 2.6f;
	// The one glow of every named galaxy: none shines brighter than another (no throne, section 6.4 'colour').
	inline constexpr float NamedGlow = 0.22f;
	// The universe's draws use no run seed: a named galaxy's turn, and the scenery (one sky for every run - v0.8 decision
	// 5), are the same in every run. The scenery takes index 0 of the home row, a named galaxy's turn index 1 of its own row.
	inline constexpr int32 UniverseLookSeed = 0;
	inline constexpr uint32 SceneryIndex = 0;
	inline constexpr uint32 NamedTurnIndex = 1;

	/** The look of a galaxy kind: a fixed table of the four Galaxy.Kind tags requested by name (Dwarf, Spiral, GrandSpiral,
	 * Elliptical - an elliptical rounder, a spiral flatter); the default look with bKnown false for any other tag. */
	COSMIC_ASCENSION_API FCosmicKindLook KindLookOf(FGameplayTag Kind);

	/** A row's sprite radius in units: HomeSpriteShare x sqrt(Row.RadiusLy / HomeLawRadiusLy) within [MinSpriteShare,
	 * MaxSpriteShare] for a RadiusLy above 0, its kind's DefaultShare otherwise. Never printed. */
	COSMIC_ASCENSION_API float SpriteShareOf(const FGalaxyDef& Row);

	/** A named galaxy's turn, 0..360 degrees: the high 24 bits of CosmicSeed::DeriveForName(UniverseLookSeed, Row,
	 * Purpose::Scenery, NamedTurnIndex) as a share of the circle. */
	COSMIC_ASCENSION_API float TurnDegreesOf(FName Row);

	/**
	 * The named galaxies of the universe (section 5.11): one mark per DT_Galaxies row that HasPlace, in lexical row order
	 * (FCosmicDataTables::GetGalaxyRowNames). The home row - HomeRow, whatever its MapPosition - stands at the frame's
	 * centre at HomeSpriteShare, round and unturned; when the table lacks it, a mark of the same law still stands there
	 * (Name empty), and with no table the home stands alone. Every other row: at PlaceOf its MapPosition, sized by
	 * SpriteShareOf, its kind's aspect, TurnDegreesOf. Every mark glows NamedGlow.
	 */
	COSMIC_ASCENSION_API TArray<FCosmicGalaxyMark> MarksOf(const FCosmicDataTables& Tables, FName HomeRow, const FCosmicUniverseFrame& Frame);

	/** The rows of DT_Galaxies the universe cannot draw (no place - HasPlace), in lexical order: the map names them in one
	 * log line per rebuild. Never the home row. */
	COSMIC_ASCENSION_API TArray<FName> UnplacedRowsOf(const FCosmicDataTables& Tables, FName HomeRow);

	// ---- The scenery (section 5.11, 9.2 Scenery 6) - design values of the picture, never printed ----

	// DT_Rules.SceneryGalaxyCount is read as 0..this (the schema's bound too).
	inline constexpr int32 MaxSceneryPoints = 50000;
	// The scenery in units about the centre: from just past the home's light out past the far stop's screen corners (UI.
	// Universe holds the corners inside Outer - OuterFade); the glow fades in over the inner width and out over the outer
	// one (no rim at rest). Up to LocalGroupUnits it stands for the Local Group's unnamed dwarfs: fainter and smaller by the
	// two shares - a function of the radius alone, no extra draw.
	inline constexpr float SceneryInnerUnits = 2.f;
	inline constexpr float SceneryInnerFadeUnits = 0.5f;
	inline constexpr float LocalGroupUnits = 6.5f;
	inline constexpr float LocalGroupGlowShare = 0.4f;
	inline constexpr float LocalGroupSizeShare = 0.6f;
	inline constexpr float SceneryOuterUnits = 42.f;
	inline constexpr float SceneryOuterFadeUnits = 6.f;
	// The kinds of the picture: a round share with its aspect range, the rest flattened discs.
	inline constexpr float SceneryRoundShare = 0.6f;
	inline constexpr float SceneryRoundAspectMin = 0.75f;
	inline constexpr float SceneryFlatAspectMin = 0.25f;
	inline constexpr float SceneryFlatAspectMax = 0.6f;
	// Across in units, smallest to largest (mostly small); the glow, faintest to brightest (mostly faint).
	inline constexpr float SceneryMinSizeUnits = 0.1f;
	inline constexpr float SceneryMaxSizeUnits = 0.4f;
	inline constexpr float SceneryMinGlow = 0.04f;
	inline constexpr float SceneryMaxGlow = 0.2f;
	// A scenery point within this share of a named galaxy's sprite radius of its centre is dark (ClearAroundMarks).
	inline constexpr float ClearShare = 1.f;

	/** DT_Rules.SceneryGalaxyCount read as 0..MaxSceneryPoints (a compare: below 1 is none). */
	COSMIC_ASCENSION_API int32 SceneryCountOf(const FCosmicRulesDef& Rules);

	/** A point of the annulus [InnerUnits, OuterUnits] about the origin, in units, from Stream: a point of the unit disc by
	 * rejection in the square (at most 16 tries, the last one halved - the generator's rule, no trigonometry), carried out to
	 * the radius sqrt(Inner^2 + rho^2 (Outer^2 - Inner^2)) along its direction, so the annulus is filled evenly by area (rho
	 * 0 goes along +X). Two draws a try. */
	COSMIC_ASCENSION_API FVector2D AnnulusPointOf(FRandomStream& Stream, float InnerUnits, float OuterUnits);

	/** The look of one scenery point from exactly the next five draws of Stream - kind (a fraction against
	 * SceneryRoundShare), size, glow, aspect, turn - whatever they give: Size Unit x (Min + (Max - Min) s^2), Glow Min +
	 * (Max - Min) g^3, the aspect in its kind's range, the turn 360 t. Leaves Position alone. */
	COSMIC_ASCENSION_API void SceneryLookOf(FRandomStream& Stream, float Unit, FCosmicSceneryPoint& Point);

	/** The share of its look a scenery point keeps at RadiusUnits from the centre: the inner fade (0 at the inner edge, 1
	 * past its width) x the outer fade (1 before it, 0 at the outer edge), in [0, 1]; the Local Group band's shares are
	 * LocalGroupShareOf. */
	COSMIC_ASCENSION_API float SceneryFadeOf(float RadiusUnits);
	/** LocalGroupGlowShare (bGlow) or LocalGroupSizeShare inside LocalGroupUnits, 1 beyond. */
	COSMIC_ASCENSION_API float LocalGroupShareOf(float RadiusUnits, bool bGlow);

	/**
	 * The scenery (section 5.11): Count points (read as SceneryCountOf reads a count) from one local stream opened on
	 * CosmicSeed::DeriveForName(UniverseLookSeed, HomeRow, Purpose::Scenery, SceneryIndex) (section 9.2 - one sky for every
	 * run) - per point AnnulusPointOf over [SceneryInnerUnits, SceneryOuterUnits], placed at the frame's centre plus that x
	 * Unit on the disc plane, SceneryLookOf, then the glow x SceneryFadeOf x LocalGroupShareOf(glow) and the size x
	 * LocalGroupShareOf(size) of the point's radius. The same arguments give the same points (a load draws the same picture);
	 * a longer list keeps a shorter one's points; the look's values move no place. Empty for no count or a Unit not above 0.
	 * Reads no row.
	 */
	COSMIC_ASCENSION_API TArray<FCosmicSceneryPoint> SceneryPointsOf(FName HomeRow, int32 Count, const FCosmicUniverseFrame& Frame);

	/** Darkens (Glow 0) every point whose centre lies within ClearShare x a mark's Radius of that mark's centre: the rows
	 * never steer the draws - no point moves, none is drawn again. */
	COSMIC_ASCENSION_API void ClearAroundMarks(TArray<FCosmicSceneryPoint>& Points, TConstArrayView<FCosmicGalaxyMark> Marks);

	// ---- The extent: the camera's universe and the sky ----

	inline constexpr float ReachMargin = 1.1f;
	inline constexpr float SkyMargin = 1.5f;
	// The sky of the region before Phase 3 item 9 (the engine sphere, radius 50, x 3,000): never smaller.
	inline constexpr float MinSkyRadius = 150000.f;
	// The engine sphere's radius (/Engine/BasicShapes/Sphere is 100 across): the sky's scale is its radius over this.
	inline constexpr float SkySphereRadius = 50.f;

	/** How far the named galaxies reach from the frame's centre, in world units: ReachMargin x the farthest mark's far side
	 * (its distance plus its radius). The scenery does not count: it reaches past the far stop's screen. Every length of it
	 * scales with the frame's unit. */
	COSMIC_ASCENSION_API float ReachOf(const FCosmicUniverseFrame& Frame, TConstArrayView<FCosmicGalaxyMark> Marks);

	/** The sky sphere's radius about SkyCentre (the map actor's place): SkyMargin x (the frame centre's distance from it +
	 * the farthest of the scenery's outer edge, the region's bound + far stop and, while open, the universe's bound + far
	 * stop) - every camera place and everything drawn stand inside -, never below MinSkyRadius. */
	COSMIC_ASCENSION_API float SkyRadiusOf(const FCosmicZoomLevels& Levels, const FCosmicUniverseFrame& Frame, const FVector& SkyCentre);

	/** The SpiritualIntensity the sky and the universe take at Share (CosmicZoom::UniverseShareOf): Intensity x (1 - Share),
	 * the share kept within [0, 1] by compares - at the universe level the sight dims nothing (the fallen do not cross,
	 * section 4.2). */
	COSMIC_ASCENSION_API float RecededIntensityOf(float Intensity, float Share);

	// ---- What the screen shows ----

	// A galaxy's label shows at the universe level when its sprite radius is at least this share of the view distance
	// (the dwarfs only nearer than the far stop), when it is selected, or for the home galaxy (never lost).
	inline constexpr float LabelRadiusShare = 0.055f;
	// A label's bottom stands this share of its galaxy's radius above the galaxy's centre (just above its visible core).
	inline constexpr float LabelLiftShare = 0.5f;
	// The selection ring stands at this share of the sprite radius (about where its light fades out); a click reaches as
	// far.
	inline constexpr float RingShare = 0.8f;
	// A double click glides to a galaxy at this many of its radii, never nearer than the universe's near stop.
	inline constexpr float GalaxyFocusFactor = 3.5f;
	// The triangles the universe may draw at most (UI.Universe holds it with the plane's own count).
	inline constexpr int32 UniverseTriangleBudget = 200000;

	/** Whether a named galaxy's label shows at the universe level: selected, the home, or its Radius at least
	 * LabelRadiusShare x ViewDistance (a compare: a distance that is not a number shows only the selected one and the home). */
	COSMIC_ASCENSION_API bool ShowsGalaxyLabel(const FCosmicGalaxyMark& Mark, float ViewDistance, bool bSelected);

	/** The radius of the selection ring around a mark, and of what a click on it reaches: RingShare x its Radius. */
	COSMIC_ASCENSION_API float RingRadiusOf(const FCosmicGalaxyMark& Mark);

	/** The arm a double click on Mark glides to: Max(the universe's near stop, GalaxyFocusFactor x its Radius), within the
	 * universe's stops (0 while the universe is closed). */
	COSMIC_ASCENSION_API float FocusArmOf(const FCosmicGalaxyMark& Mark, const FCosmicZoomLevels& Levels);

	/** The instance of the engine plane that draws a mark (2 x Radius along its turned X, 2 x Radius x Aspect along its Y) or
	 * a scenery point (Size and Size x Aspect), lying in the disc plane at its position. */
	COSMIC_ASCENSION_API FTransform InstanceOf(const FCosmicGalaxyMark& Mark);
	COSMIC_ASCENSION_API FTransform InstanceOf(const FCosmicSceneryPoint& Point);

	/**
	 * What the right side shows (section 8.3 - never two panels; item 8: it says what the player looks at), by the level of
	 * the camera's target arm. Region: the system panel of SelectedSystem when one is selected, else the galaxy panel of
	 * HomeRow (a galaxy selected at the universe level does not show here). Universe: the galaxy panel of SelectedGalaxy
	 * when one is selected, else of HomeRow - a selected system steps aside and comes back in the region.
	 */
	COSMIC_ASCENSION_API FCosmicPanelChoice PanelOf(ECosmicZoomLevel Level, FName SelectedSystem, FName SelectedGalaxy, FName HomeRow);
}

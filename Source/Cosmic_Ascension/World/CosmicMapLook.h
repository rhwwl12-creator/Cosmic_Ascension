// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 3.1, 3.2, 4.2, 5.7, 6.4, 8.1, 8.2, 11 Phase 3.
// What the map draws of the state, computed from the state alone: no UObject, no draw, no write. Knowing is read in one
// place, KnownViewOf: the map, the system panel and the selection ask that view (the galaxy panel reads definitions only -
// a met galaxy, Phase 6a, adds its bit to the view). The universe level's values are CosmicUniverseLook's (Phase 3 item 9).

#pragma once

#include "CoreMinimal.h"
#include "Containers/BitArray.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Math/RandomStream.h"
#include "Templates/Tuple.h"

struct FCosmicDataTables;
struct FCosmicRulesDef;
struct FCosmicStateIndex;
struct FTranscendentDef;

/** How one system is drawn (the physical look): LookOf gives the world's, ShownLookOf what the viewer is shown. */
struct COSMIC_ASCENSION_API FCosmicSystemLook
{
	FText Name;
	// By star class (Star.Class.O .. M; white and 1 for an unknown class).
	FLinearColor StarColor = FLinearColor::White;
	float StarScale = 1.f;
	// The owner, settled or settling, is a people of the state (LookOf) that the viewer is shown (ShownLookOf).
	bool bOwned = false;
	// The colour of the owner's ring: black from LookOf - the world's look has no colour, and nothing on the screen draws
	// FCivilizationState::Color - and, from ShownLookOf, the meeting colour of a shown owner (CosmicPalette::OwnerColorOf).
	FLinearColor OwnerColor = FLinearColor::Black;
	// The construction slot holds project_settle.
	bool bSettling = false;
	// A hazard stands in the slot.
	bool bHazard = false;
	// Progress of the construction and the response slot as a share of RequiredProgressOf(row), 0..1; -1 for an empty
	// slot, a row the tables do not hold, and the works of an owner the viewer is not shown.
	float ConstructionShare = -1.f;
	float ResponseShare = -1.f;
	// Destroyed (CosmicHazards::IsDeadNode): drawn dim, no owner, no hazard, no bars.
	bool bDead = false;
	// Whether the viewer is shown the system (section 6.4): it knows it, or the view shows it by its name alone (the
	// sanctuary, FCosmicKnownView::IsShownByName). False only from ShownLookOf, for a system the viewer is not shown: the
	// default look otherwise - no name, class, owner, hazard, bars or death - and its star hidden; the backdrop's point at
	// its place is all the map shows of it (no label, ring, lane, mark or selection). LookOf and a look made by default say
	// true, so every look made before Phase 3 item 8 keeps its meaning.
	bool bKnown = true;
};

/**
 * The two ways of seeing the one map (section 6.4): the physical view and the spiritual sight of the third rung. There
 * is no second map. The viewer's choice, held by the player controller: never in FGalaxyState, never saved, outside both
 * hashes, not kept across sessions. The value is M_Star's ViewMode (0 or 1).
 */
enum class ECosmicViewMode : uint8
{
	Physical,
	Spiritual,
};

/**
 * The claim marker of a system (section 4.2), derived and never saved; the value is M_Star's ClaimState (the material
 * draws 1 and 2, the map draws the Sealed ring). Only instances whose DT_Transcendents row is Realm.Abyssal mark a
 * system, as CosmicHazards::ClaimantOf reads them. Precedence Claimed, Held, Bound: two of them meet only with two fallen.
 */
enum class ECosmicClaimState : uint8
{
	// No fallen one claims or holds the system.
	None = 0,
	// A Hazard.Claim stands and CosmicHazards::ClaimantOf names its claimant now. A claim nobody can claim now is no mark:
	// its expiry claims nothing.
	Claimed = 1,
	// In the sphere of an unsealed fallen instance.
	Held = 2,
	// In the sphere of a sealed fallen instance and of no unsealed one.
	Bound = 3,
};

/** What the spiritual sight draws of one system (CosmicMapLook::SpiritOf). The physical view draws none of it. */
struct COSMIC_ASCENSION_API FCosmicSpiritLook
{
	// The slow pulse, 0..1 (PressureShareOf the system's P). The pulse shows only where a fallen one leads - its row is
	// Realm.Abyssal and it stands above the owner's rung, or the system has no owner; its strength is the whole P, the
	// owner's own term included, as the drain reads it.
	float Pressure = 0.f;
	// The colour of the fallen one MarkRow names (DominionColorOf its row): black when nothing marks the system.
	FLinearColor DominionColor = FLinearColor::Black;
	ECosmicClaimState ClaimState = ECosmicClaimState::None;
	// In the sphere of a sealed fallen instance, whatever ClaimState says: the map draws the Sealed ring.
	bool bSealedRing = false;
	// The DT_Transcendents row of the fallen one the sight names and colours, in one order: the claimant, else the unsealed
	// holder, else the fallen one that presses, else the sealed holder. None when nothing marks the system.
	FName MarkRow;
	// MarkRow names the fallen one that presses (no claimant and no unsealed holder came before it): the panel says
	// "presses here" rather than the words of ClaimState.
	bool bMarkPresses = false;
	// The system is the sanctuary (CosmicState::IsSanctuary, Law.Hedge - section 4.5 (1)): nothing above is ever set for
	// it, and the panel says that no pressure reaches it (ST_CA Sight.Sanctuary).
	bool bSanctuary = false;
};

/**
 * The values the star actor sets on its dynamic instance of M_Star, under the names of CosmicMapLook::StarParam. The
 * defaults are the base material's own (UI.MaterialParams holds the two together): a star given nothing looks as the
 * material does.
 */
struct COSMIC_ASCENSION_API FCosmicStarParams
{
	FLinearColor StarColor = FLinearColor::White;
	float Brightness = 1.3f;
	float HazardAmount = 0.f;
	// The colour of the hazard's warning pulse: the physical orange, the Severity-0 grey of the spiritual view.
	FLinearColor WarningColor = FLinearColor(1.f, 0.25f, 0.05f);
	float ViewMode = 0.f;
	float Pressure = 0.f;
	FLinearColor DominionColor = FLinearColor::Black;
	float ClaimState = 0.f;
};

/**
 * What the viewer knows, read once from FGalaxyState.Knowing (section 6.4, 8.1 reader 2) for one refresh of the map or
 * of a panel - the presentation's one reading of knowledge (CosmicMapLook::KnownViewOf). Plain values by array index:
 * never stored, outside both hashes; Knowing is never written. The sanctuary, never written into Knowing, is always
 * shown by its name alone (section 4.5 (1)): its bit of ShownByName, beside Known. What the screens draw, pick and
 * select is IsShown - known or shown by name; what is told of an owner stays IsKnown's (OwnerShownOf). The sanctuary
 * has no lane until v0.8 step 5.
 */
struct COSMIC_ASCENSION_API FCosmicKnownView
{
	// By system index (State.Systems): Knowing.KnownSystems holds its ID - every system under bSeeAll.
	TBitArray<> Known;
	// By system index: shown by its name alone, whatever Knowing holds - the sanctuary (CosmicState::IsSanctuary), and
	// under bSeeAll too.
	TBitArray<> ShownByName;
	// By civilization index (State.Civs): the people's meeting order - its place in Knowing.MetCivs, a gone people
	// keeping its place - while the viewer is shown it (CosmicMapLook::IsShownPeople); INDEX_NONE otherwise. Under bSeeAll
	// every people is shown, and one not met follows the met ones in Civs order.
	TArray<int32> ShownOrder;
	// By civilization index: Knowing.MetCivs holds it (B1 - the coronation's gate, CosmicWanderer::CrownBlockOf, asks it
	// here). The same under bSeeAll: the aid shows every people but meets none.
	TBitArray<> Met;
	// The development aid CosmicSeeAll.
	bool bSeeAll = false;
	// The petition card waiting for the player's answer and the system it is about (Knowing.PendingPetitionRow and
	// PendingPetitionSystemID, section 5.10); None when no card waits. The same under bSeeAll.
	FName PendingPetitionRow;
	FName PendingPetitionSystemID;
	// The people the first contact card is about (Knowing.PendingContactCivID, B3); None without one. The same under bSeeAll.
	FName PendingContactCivID;

	bool IsKnown(int32 SystemIndex) const { return Known.IsValidIndex(SystemIndex) && Known[SystemIndex]; }
	bool IsShownByName(int32 SystemIndex) const { return ShownByName.IsValidIndex(SystemIndex) && ShownByName[SystemIndex]; }
	// Whether the screens show the system at all - its star, its label, its lanes, the pick, the selection and the
	// system panel's gate: known, or shown by its name.
	bool IsShown(int32 SystemIndex) const { return IsKnown(SystemIndex) || IsShownByName(SystemIndex); }
	int32 ShownOrderOf(int32 CivIndex) const { return ShownOrder.IsValidIndex(CivIndex) ? ShownOrder[CivIndex] : INDEX_NONE; }
	bool IsMet(int32 CivIndex) const { return Met.IsValidIndex(CivIndex) && Met[CivIndex]; }
};

/** Where the map draws the vessel (B1): the system it stands in - while it travels, the one it left -, the system it
 * travels to and how far along that lane. */
struct COSMIC_ASCENSION_API FCosmicVesselLook
{
	// By system index; INDEX_NONE: no mark (a ruler, a vessel that is not embodied).
	int32 FromIndex = INDEX_NONE;
	// INDEX_NONE while it stands.
	int32 ToIndex = INDEX_NONE;
	// The share of the lane done, 0 to 1 (0 while it stands).
	float Share = 0.f;
};

/** How the owner of a system is shown (section 6.4): the one answer the map's rings and labels and the panel's lines read. */
enum class ECosmicOwnerShown : uint8
{
	// No owner a people of the state carries (a wild system, an owner ID no people carries), a destroyed system, a system
	// the viewer does not know, or an index out of range.
	None,
	// A people the viewer is shown: its colour, its name, its people line, its works.
	Shown,
	// A people the viewer is not shown - not met, or met and ShownGap rungs or more above: no colour, no name, no people
	// line, no works (slot lines, label bars, the settling dash). The system's hazard and the spiritual marks still show.
	Hidden,
};

/** One point of the backdrop (section 3.1 layer 1): a picture without state. */
struct COSMIC_ASCENSION_API FCosmicBackdropPoint
{
	// On the disc plane.
	FVector Position = FVector::ZeroVector;
	// Across, in world units (the engine plane is 100 across: the instance's scale is Size / 100).
	float Size = 0.f;
	// The point's brightness, M_Backdrop's per-instance float; 0 draws nothing.
	float Glow = 0.f;
};

namespace CosmicMapLook
{
	/** The look of State.Systems[SystemIndex] in the world, whatever the viewer knows and with no owner's colour (the map
	 * draws ShownLookOf of it); the default look for an index out of range. */
	COSMIC_ASCENSION_API FCosmicSystemLook LookOf(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 SystemIndex);

	/** The lanes of the world, each once as (lower index, higher index): a lane listed by either end, both ends existing
	 * and not dead. The map draws KnownLanesOf of them. */
	COSMIC_ASCENSION_API TArray<TPair<int32, int32>> LanesOf(const FGalaxyState& State);

	/** Whether a system's label shows: never for a look that is not known; always when near; from afar only for a system
	 * with an owner or a hazard, or the selected one (on a shown look an owner the viewer is not shown lights none). */
	COSMIC_ASCENSION_API bool ShowsLabel(const FCosmicSystemLook& Look, bool bSelected, bool bFarView);

	// The generated materials and their parameter names, as Tools/make_materials.py writes them. The star actor and the
	// map set only these (the actor keeps one FName of each); UI.MaterialParams finds each on the base asset (a dynamic
	// instance takes any name silently).
	inline constexpr const TCHAR* StarMaterialPath = TEXT("/Game/CA/Materials/M_Star.M_Star");
	inline constexpr const TCHAR* NebulaMaterialPath = TEXT("/Game/CA/Materials/M_Nebula.M_Nebula");
	namespace StarParam
	{
		inline constexpr const TCHAR* StarColor = TEXT("StarColor");
		inline constexpr const TCHAR* Brightness = TEXT("Brightness");
		inline constexpr const TCHAR* HazardAmount = TEXT("HazardAmount");
		inline constexpr const TCHAR* WarningColor = TEXT("WarningColor");
		inline constexpr const TCHAR* ViewMode = TEXT("ViewMode");
		inline constexpr const TCHAR* Pressure = TEXT("Pressure");
		inline constexpr const TCHAR* DominionColor = TEXT("DominionColor");
		inline constexpr const TCHAR* ClaimState = TEXT("ClaimState");
	}
	inline constexpr const TCHAR* StarScalarParams[] = { StarParam::Brightness, StarParam::HazardAmount, StarParam::ViewMode, StarParam::Pressure, StarParam::ClaimState };
	inline constexpr const TCHAR* StarVectorParams[] = { StarParam::StarColor, StarParam::WarningColor, StarParam::DominionColor };
	inline constexpr const TCHAR* SkyIntensityParam = TEXT("SpiritualIntensity");

	/** The rung the spiritual sight opens at (section 3.2, 6.4), against UGalaxySimulationSubsystem::GetViewerTier.
	 * Phase 3 item 12's x25 opens at the same rung. */
	inline constexpr EAscensionTier SpiritualSightTier = EAscensionTier::Tier3_Causal;

	/** Whether the spiritual sight is open to a viewer of ViewerTier - the ruled people's Tier while the vessel rules
	 * one, the calling's PersonalTier otherwise: SpiritualSightTier and above. A deposed ruler sees from the first rung
	 * again. */
	COSMIC_ASCENSION_API bool SpiritualSightOpens(EAscensionTier ViewerTier);

	/** The view drawn: Spiritual when it is wanted and the sight is open to ViewerTier or forced open (the development
	 * aid CosmicSpiritualSight); Physical otherwise - a closed sight falls back by itself. */
	COSMIC_ASCENSION_API ECosmicViewMode ShownViewMode(ECosmicViewMode Wanted, bool bForcedOpen, EAscensionTier ViewerTier);

	/**
	 * P (section 4.2) as the 0..1 the material pulses with: P / (2 x DT_Rules.PressureBySourceTier[Tier4]) - 1 is the
	 * term of the deepest fallen of the fourth rung inside its sphere - at most 1. 0 for a P that is not a positive number
	 * and for a fourth-rung weight that is missing or not positive (compares). Never shown as a number.
	 */
	COSMIC_ASCENSION_API float PressureShareOf(float P, const FCosmicRulesDef& Rules);

	/** The colour of a transcendent's row in the sight (section 6.4, 8.2): CosmicPalette::Purple, the one purple of the
	 * fallen, for an Allegiance of Realm.Abyssal; black (no colour) for any other allegiance and for no row. No row name
	 * or doctrine is a key, and the holy order is never a colour (section 4.5). */
	COSMIC_ASCENSION_API FLinearColor DominionColorOf(const FTranscendentDef* Row);

	/**
	 * What the spiritual sight draws of State.Systems[SystemIndex] (section 4.2, 6.4), from the world state and the rows
	 * alone - never Knowing (the map asks it only of systems the viewer knows). ClaimState, bSealedRing and MarkRow as
	 * ECosmicClaimState and FCosmicSpiritLook say; Pressure from one call of UGalaxySimulationSubsystem::ComputePressureIn,
	 * shown where the dominant source has a Realm.Abyssal row (that row is the real guard: a people's or a wanderer's
	 * source has none) and stands above the owner's rung or the system has no owner; DominionColor of MarkRow's row. The
	 * zero look for an index out of range and for a destroyed system. The sanctuary first: the zero look with bSanctuary
	 * - no colour, no pressure, no mark, whatever a sphere lists (Law.Hedge, section 4.5 (1)).
	 */
	COSMIC_ASCENSION_API FCosmicSpiritLook SpiritOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 SystemIndex);

	/** The colour of the half of a lane at one end (the stand-in for M_Lane: a line batch draws each line in one colour,
	 * with no material). Physical: the lane colour whatever the pressure. Spiritual: the lane colour losing its
	 * saturation by the end's Pressure (0 keeps it, 1 is its grey), alpha kept at 1; a value that is not a positive
	 * number keeps the colour. */
	COSMIC_ASCENSION_API FLinearColor LaneColorOf(float EndPressure, ECosmicViewMode Shown);

	/**
	 * The material values of a star - the only place they are chosen. Physical: the class colour and Brightness 1.3 (the
	 * dim grey of a destroyed system), HazardAmount 1 for a hazard with the orange warning, every spiritual value at its
	 * default - item 1's look exactly. Spiritual, for a living system: the class colour desaturated (alpha kept at 1) and
	 * dimmed, the warning in Severity-0 grey (a grey pulse whose peak is about 1 - the fallen's purple is told by its hue,
	 * M_Star tints a marked star instead of lighting it), ViewMode 1, the spirit's Pressure, DominionColor and ClaimState;
	 * a Claimed system pulses in its claimant's colour instead of the warning (HazardAmount 0). A destroyed system draws
	 * nothing spiritual.
	 */
	COSMIC_ASCENSION_API FCosmicStarParams StarParamsOf(const FCosmicSystemLook& Look, const FCosmicSpiritLook& Spirit, ECosmicViewMode Shown);

	/** M_Nebula's SpiritualIntensity: 0 in the physical view, 1 in the spiritual one. */
	COSMIC_ASCENSION_API float SkyIntensityOf(ECosmicViewMode Shown);

	/** The fallen's band of hues around CosmicPalette::Purple (violet, purple, magenta, pink), in the degrees of
	 * FLinearColor::LinearRGBToHSV: no people's ring takes it in the spiritual view. */
	inline constexpr float FallenHueFrom = 250.f;
	inline constexpr float FallenHueTo = 330.f;

	/**
	 * The colour of a people's ring (FCivilizationState::Color) as the map draws it. Physical: the colour itself - item
	 * 1's look. Spiritual: the colour itself, unless its hue lies in the fallen's band (FallenHueFrom..FallenHueTo, a
	 * colour with some saturation): such a ring is drawn in its own grey (its luminance, alpha 1). In the third rung's
	 * sight purple is the fallen's alone (section 6.4) - a people ringed in it would read as marked by them. The state's
	 * Color and both hashes do not move. The palette of CosmicPalette::Owners leaves the band out, so the rule changes no
	 * ring the map draws today: the map keeps calling it as a deliberately idle guard (for a colour that is not the
	 * palette's - EA's wrap, a people's colour of Phase 6a), and UI.KnownFilter holds the palette out of the band.
	 */
	COSMIC_ASCENSION_API FLinearColor OwnerRingColorOf(const FLinearColor& OwnerColor, ECosmicViewMode Shown);

	// ---- Knowing (section 6.4 'the display of knowing', 8.1) ----

	/**
	 * A people this many rungs or more above the viewer is not shown: the gap of the scope of telling's subject rule
	 * (section 3.2, 8.1). The rule lives in three places, held together by their tests until B6 (whose T3 to T5 feats
	 * first make such a people) moves them into one Core predicate: UCosmicLogSubsystem::Tell (Log.TellScope),
	 * UGalaxySimulationSubsystem::NoteMet - seen, not met (Sim.KnowingWriters, Sim.SightStage) - and IsShownPeople here
	 * (UI.KnownFilter's parity with the log).
	 */
	inline constexpr int32 ShownGap = 3;

	/** Whether the viewer is shown Civ (section 3.2, 8.1): it is met (bMet - Knowing.MetCivs holds it, as KnownViewOf
	 * read it) and stands fewer than ShownGap rungs above ViewerTier (UGalaxySimulationSubsystem::GetViewerTier) - the
	 * subject rule of the scope of telling, so the map, the system panel and the feed name the same peoples. */
	COSMIC_ASCENSION_API bool IsShownPeople(bool bMet, const FCivilizationState& Civ, EAscensionTier ViewerTier);

	/** The view of State.Knowing for a viewer on ViewerTier; bSeeAll (the development aid CosmicSeeAll) knows every system
	 * and shows every people. Index resolves the IDs: an ID the state no longer holds (a gone people) sets nothing but
	 * keeps its place in the meeting order. ShownByName holds the sanctuary, found by its ID (CosmicIDs::Sanctuary). The
	 * one reader of Knowing in the presentation; never writes it. */
	COSMIC_ASCENSION_API FCosmicKnownView KnownViewOf(const FGalaxyState& State, const FCosmicStateIndex& Index, EAscensionTier ViewerTier, bool bSeeAll);

	/** How the owner of State.Systems[SystemIndex] is shown: None for an index out of range, a system the view does not
	 * know, a destroyed system, no owner and an owner ID no people of Civs carries; otherwise Shown or Hidden by the view's
	 * ShownOrder. */
	COSMIC_ASCENSION_API ECosmicOwnerShown OwnerShownOf(const FGalaxyState& State, const FCosmicKnownView& View, int32 SystemIndex);

	/**
	 * What the viewer is shown of State.Systems[SystemIndex] (section 6.4), from its world look World (LookOf) - the one
	 * gate the map and its labels read (the pick and the selection ask the view). A system the view does not show
	 * (FCosmicKnownView::IsShown), and an index out of range: the default look with bKnown false - nothing of World (a
	 * destroyed system the viewer never knew stays a point). One shown by its name alone (the sanctuary): its name and its
	 * star only - no owner, hazard, bars, settling dash or death, nothing the viewer could have learned. A known one: World, with
	 * its owner as OwnerShownOf says - Shown: OwnerColor is CosmicPalette::OwnerColorOf its meeting order; Hidden: no owner,
	 * no colour, no settling dash and no bars (both shares -1), its name, class, hazard and death kept; None: World as it is.
	 */
	COSMIC_ASCENSION_API FCosmicSystemLook ShownLookOf(const FCosmicSystemLook& World, const FGalaxyState& State, const FCosmicKnownView& View, int32 SystemIndex);

	/** The lanes of Lanes (LanesOf) whose two ends the view shows (section 6.4, FCosmicKnownView::IsShown), in their order:
	 * no stub toward an unknown end. */
	COSMIC_ASCENSION_API TArray<TPair<int32, int32>> KnownLanesOf(const TArray<TPair<int32, int32>>& Lanes, const FCosmicKnownView& View);

	/** The vessel's mark (B1 header section 4): an embodied wanderer's system and, while it travels, its target and the
	 * share of the lane done - 1 - TravelTicksRemaining / the lane's ticks (CosmicWanderer::LaneTicksOf), within 0..1. No
	 * mark for a ruler or a vessel that is not embodied. Reads no knowing: the system a wanderer stands in and its lane
	 * neighbours are always known. */
	COSMIC_ASCENSION_API FCosmicVesselLook VesselLookOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules);

	// ---- The backdrop (section 3.1 layer 1, 6.4): the points of the galaxy's disc, an unknown system's among them ----

	// The backdrop's mesh (the engine plane: 100 units square in the XY plane, facing +Z, two triangles) and material
	// (Tools/make_materials.py). M_Backdrop's one parameter is the sky's, SkyIntensityParam: the dust recedes with it.
	inline constexpr const TCHAR* BackdropMeshPath = TEXT("/Engine/BasicShapes/Plane.Plane");
	inline constexpr const TCHAR* BackdropMaterialPath = TEXT("/Game/CA/Materials/M_Backdrop.M_Backdrop");
	// The per-instance floats of a point: its Glow (M_Backdrop's PerInstanceCustomData, index 0).
	inline constexpr int32 BackdropDataFloats = 1;
	// M_Backdrop's two parameters of Phase 3 item 9 (Tools/make_materials.py), on both of its instances - the backdrop's and
	// the universe's (ACosmicGalaxyMapActor). LayerFade: the share of its light a layer shows - the backdrop 1 - CosmicZoom::
	// UniverseShareOf, the universe the share itself; 1 by default, item 8's look. Tint: the one colour of the dust and of
	// every galaxy - a default the C++ never sets, held out of the fallen's band of hues and away from white by
	// UI.MaterialParams.
	inline constexpr const TCHAR* LayerFadeParam = TEXT("LayerFade");
	inline constexpr const TCHAR* BackdropTintParam = TEXT("Tint");
	// M_Nebula's two colours (Phase 3 item 9 - section 16 #98 (4), #113): the dark and the bright of its noise, defaults the
	// C++ never sets, held below the fallen's band of hues (a deep blue) by UI.MaterialParams.
	inline constexpr const TCHAR* NebulaLowParam = TEXT("NebulaLow");
	inline constexpr const TCHAR* NebulaHighParam = TEXT("NebulaHigh");
	// DT_Rules.BackgroundStarCount is read as 0..this (the schema's bound too).
	inline constexpr int32 MaxBackdropPoints = 100000;
	// The points stand evenly over this many disc radii; past the disc's edge their glow fades to 0 here (a soft rim - no
	// system stands there).
	inline constexpr float BackdropRimShare = 1.15f;
	// A point's size as a share of the disc radius, smallest to largest, mostly small: 55 to 100 units on the vertical
	// slice's disc (about 7,750), about 1.5 to 3.7 px from the farthest zoom to the overview, on any disc.
	inline constexpr float BackdropMinSizeShare = 0.0071f;
	inline constexpr float BackdropMaxSizeShare = 0.0129f;
	// A point's glow, faintest to brightest, mostly faint (M_Backdrop's tint and gain turn it into light).
	inline constexpr float BackdropMinGlow = 0.1f;
	inline constexpr float BackdropMaxGlow = 0.45f;
	// The triangles the backdrop may draw at most (UI.Backdrop holds it with the mesh's own count).
	inline constexpr int32 BackdropTriangleBudget = 1000000;

	/** DT_Rules.BackgroundStarCount read as 0..MaxBackdropPoints (a compare: below 1 is none). */
	COSMIC_ASCENSION_API int32 BackdropCountOf(const FCosmicRulesDef& Rules);

	/** The look of one point from the next two draws of Stream (section 3.1 layer 1): Size, a share of DiscRadius between
	 * BackdropMinSizeShare and BackdropMaxSizeShare, mostly small; then Glow, between BackdropMinGlow and BackdropMaxGlow,
	 * mostly faint. The one law of every backdrop point and of the point of every unknown system. Leaves Position alone. */
	COSMIC_ASCENSION_API void BackdropLookOf(FRandomStream& Stream, float DiscRadius, FCosmicBackdropPoint& Point);

	/**
	 * The backdrop of a galaxy: Count points (read as BackdropCountOf reads a count) from one local stream opened on
	 * CosmicSeed::DeriveForName(Seed, GalaxyRow, Purpose::Backdrop, 0) (section 9.2), even over BackdropRimShare x the disc
	 * of Centre and Radius in its plane - rejection in the square, at most 16 tries, the last one halved (the generator's
	 * rule; no trigonometry) -, each looked by BackdropLookOf and its Glow fading to 0 from the disc's edge to the rim. The
	 * same arguments give the same points: a load draws the same picture. Empty for no count or a Radius not above 0.
	 */
	COSMIC_ASCENSION_API TArray<FCosmicBackdropPoint> BackdropPointsOf(int32 Seed, FName GalaxyRow, int32 Count, const FVector& Centre, float Radius);

	/** The point of a system the viewer does not know (section 6.4 "the same point"): at Position, looked by BackdropLookOf
	 * from a local stream opened on CosmicSeed::DeriveForName(Seed, SystemID, Purpose::Backdrop, 0) - the law of every
	 * backdrop point, so it can be neither told from them nor counted apart. Alive or destroyed, the same; no rim fade. */
	COSMIC_ASCENSION_API FCosmicBackdropPoint BackdropSlotOf(int32 Seed, FName SystemID, const FVector& Position, float DiscRadius);
}

// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 3.2, 4.2, 6.4, 8.2, 11 Phase 3.
// What the map draws of the state, computed from the state alone: no UObject, no draw, no write.

#pragma once

#include "CoreMinimal.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Templates/Tuple.h"

struct FCosmicDataTables;
struct FCosmicRulesDef;
struct FCosmicStateIndex;
struct FTranscendentDef;

/** How one system is drawn (the physical look). */
struct COSMIC_ASCENSION_API FCosmicSystemLook
{
	FText Name;
	// By star class (Star.Class.O .. M; white and 1 for an unknown class).
	FLinearColor StarColor = FLinearColor::White;
	float StarScale = 1.f;
	// The owner, settled or settling, and its colour (FCivilizationState::Color). No owner: false and black.
	bool bOwned = false;
	FLinearColor OwnerColor = FLinearColor::Black;
	// The construction slot holds project_settle.
	bool bSettling = false;
	// A hazard stands in the slot.
	bool bHazard = false;
	// Progress of the construction and the response slot as a share of RequiredProgressOf(row), 0..1; -1 for an empty
	// slot or a row the tables do not hold.
	float ConstructionShare = -1.f;
	float ResponseShare = -1.f;
	// Destroyed (CosmicHazards::IsDeadNode): drawn dim, no owner, no hazard, no bars.
	bool bDead = false;
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

namespace CosmicMapLook
{
	/** The look of State.Systems[SystemIndex]; the default look for an index out of range. */
	COSMIC_ASCENSION_API FCosmicSystemLook LookOf(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 SystemIndex);

	/** The lanes to draw, each once as (lower index, higher index): a lane listed by either end, both ends existing and not dead. */
	COSMIC_ASCENSION_API TArray<TPair<int32, int32>> LanesOf(const FGalaxyState& State);

	/** Whether a system's label shows: always when near; from afar only for a system with an owner or a hazard, or the
	 * selected one. */
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
	 * alone - never Knowing (item 8 gates the drawing, not this function). ClaimState, bSealedRing and MarkRow as
	 * ECosmicClaimState and FCosmicSpiritLook say; Pressure from one call of UGalaxySimulationSubsystem::ComputePressureIn,
	 * shown where the dominant source has a Realm.Abyssal row (that row is the real guard: a people's or a wanderer's
	 * source has none) and stands above the owner's rung or the system has no owner; DominionColor of MarkRow's row. The
	 * zero look for an index out of range and for a destroyed system. (v0.8 step 4 puts the sanctuary first: no colour,
	 * no pressure.)
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
	 * Color and both hashes do not move; item 8's palette of twelve leaves the band out and makes the rule idle.
	 */
	COSMIC_ASCENSION_API FLinearColor OwnerRingColorOf(const FLinearColor& OwnerColor, ECosmicViewMode Shown);
}

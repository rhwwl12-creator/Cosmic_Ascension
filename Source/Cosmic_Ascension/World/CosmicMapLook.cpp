// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicMapLook.h"

#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Core/CosmicIDs.h"
#include "Core/CosmicKnowing.h"
#include "Core/CosmicSeed.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "GameplayTagContainer.h"
#include "Math/NumericLimits.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UI/CosmicPalette.h"

// The material decodes the claim marker by these values (Tools/make_materials.py: 1 pulses, 2 glows).
static_assert(static_cast<int32>(ECosmicClaimState::Claimed) == 1 && static_cast<int32>(ECosmicClaimState::Held) == 2 && static_cast<int32>(ECosmicClaimState::Bound) == 3,
	"M_Star reads ClaimState as 1 claimed, 2 held, 3 bound");

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicMapLookImpl
{
	struct FStarClassLook
	{
		const TCHAR* Tag;
		FLinearColor Color;
		float Scale;
	};

	// The colour and size of each star class, hot and large to cool and small (presentation only, no rule reads it).
	const FStarClassLook StarClasses[] = {
		{ TEXT("Star.Class.O"), FLinearColor(0.3f, 0.45f, 1.f), 2.f },
		{ TEXT("Star.Class.B"), FLinearColor(0.45f, 0.6f, 1.f), 1.7f },
		{ TEXT("Star.Class.A"), FLinearColor(0.75f, 0.82f, 1.f), 1.4f },
		{ TEXT("Star.Class.F"), FLinearColor(1.f, 0.95f, 0.78f), 1.2f },
		{ TEXT("Star.Class.G"), FLinearColor(1.f, 0.8f, 0.4f), 1.f },
		{ TEXT("Star.Class.K"), FLinearColor(1.f, 0.55f, 0.22f), 0.85f },
		{ TEXT("Star.Class.M"), FLinearColor(1.f, 0.3f, 0.15f), 0.7f },
	};

	// The colour of a lane (the physical view, and an end without pressure in the spiritual one).
	const FLinearColor LaneColor(0.25f, 0.4f, 0.65f, 1.f);
	// Low enough that the tone mapper keeps the colour of the class instead of washing it to white.
	constexpr float Brightness = 1.3f;
	constexpr float DeadBrightness = 0.4f;
	const FLinearColor DeadColor(0.35f, 0.35f, 0.4f);
	// The physical warning of a hazard: the orange of item 1.
	const FLinearColor PhysicalWarning(1.f, 0.25f, 0.05f);
	// In the spiritual view the physical layer recedes (section 6.4): the class colour loses this much of its saturation,
	// the star dims to this brightness, and the warning turns to the Severity-0 grey at this scale - the material
	// multiplies it by 3 at the top of its pulse, so its peak is about 1, a grey pulse. Tuned on the off-screen
	// screenshots (item 7): at 0.8 the dimmed core still washed the pale purple added to it to white; since the review the
	// material tints a marked star toward a deep purple about this dim instead of adding light (the fallen's mark is a
	// hue, never a white glow).
	constexpr float SpiritualDesaturation = 0.7f;
	constexpr float SpiritualBrightness = 0.3f;
	constexpr float SpiritualWarningScale = 0.5f;

	void ApplyStarClass(FGameplayTag StarClass, FCosmicSystemLook& Look)
	{
		for (const FStarClassLook& Each : StarClasses)
		{
			if (StarClass == FGameplayTag::RequestGameplayTag(FName(Each.Tag), false))
			{
				Look.StarColor = Each.Color;
				Look.StarScale = Each.Scale;
				return;
			}
		}
	}

	// The progress of a slot as a share of what its row needs; -1 for an empty slot or a row the tables do not hold. A
	// compare, so a progress that is not a number reads as 0.
	float ShareOf(const FCosmicDataTables& Tables, FName RowName, float Progress)
	{
		const FMegaprojectDef* Row = RowName.IsNone() ? nullptr : Tables.FindMegaproject(RowName);
		if (!Row)
		{
			return -1.f;
		}
		const float Share = Progress / CosmicMegaprojects::RequiredProgressOf(*Row);
		return Share > 0.f ? FMath::Min(Share, 1.f) : 0.f;
	}

	const FGameplayTag& AbyssalTag()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Realm.Abyssal")), false);
		return Tag;
	}

	const FGameplayTag& ClaimTag()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Claim")), false);
		return Tag;
	}

	// Whether a row is a fallen one's: the only rows that mark a system or colour it (the reading of ClaimantOf).
	bool IsFallen(const FTranscendentDef* Row)
	{
		return Row && Row->Allegiance.IsValid() && Row->Allegiance == AbyssalTag();
	}

	// A colour with its alpha put back to 1 (FLinearColor::Desaturate fades the alpha toward 0 as well).
	FLinearColor Opaque(FLinearColor Color)
	{
		Color.A = 1.f;
		return Color;
	}

	// The index in State.Civs of the owner of System, as LookOf finds it: INDEX_NONE for no owner and for an owner ID no
	// people carries.
	int32 OwnerIndexOf(const FGalaxyState& State, const FStarSystemState& System)
	{
		return System.OwnerCivID.IsNone() ? INDEX_NONE
			: State.Civs.IndexOfByPredicate([&System](const FCivilizationState& Civ) { return Civ.CivID == System.OwnerCivID; });
	}

	// A count of backdrop points read as 0..MaxBackdropPoints (a compare: below 1 is none).
	int32 BackdropCountFrom(int32 Count)
	{
		return Count > 0 ? FMath::Min(Count, CosmicMapLook::MaxBackdropPoints) : 0;
	}
}

FCosmicSystemLook CosmicMapLook::LookOf(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 SystemIndex)
{
	using namespace CosmicMapLookImpl;

	FCosmicSystemLook Look;
	if (!State.Systems.IsValidIndex(SystemIndex))
	{
		return Look;
	}
	const FStarSystemState& System = State.Systems[SystemIndex];
	Look.Name = System.DisplayName;
	ApplyStarClass(System.StarClass, Look);
	if (CosmicHazards::IsDeadNode(System))
	{
		Look.bDead = true;
		return Look;
	}
	// The world's look has no owner's colour: the drawn colour is the meeting order's (ShownLookOf), so no path on the
	// screen draws FCivilizationState::Color.
	Look.bOwned = OwnerIndexOf(State, System) != INDEX_NONE;
	Look.bSettling = System.ActiveMegaproject == CosmicRows::ProjectSettle;
	Look.bHazard = System.ActiveHazard.IsValid();
	Look.ConstructionShare = ShareOf(Tables, System.ActiveMegaproject, System.MegaprojectProgress);
	Look.ResponseShare = ShareOf(Tables, System.ResponseMegaproject, System.ResponseProgress);
	return Look;
}

TArray<TPair<int32, int32>> CosmicMapLook::LanesOf(const FGalaxyState& State)
{
	TMap<FName, int32> IndexOf;
	IndexOf.Reserve(State.Systems.Num());
	for (int32 Index = 0; Index < State.Systems.Num(); ++Index)
	{
		IndexOf.Add(State.Systems[Index].SystemID, Index);
	}
	// A lane listed by one end only is drawn too, and a lane listed by both ends once.
	TArray<TPair<int32, int32>> Lanes;
	TSet<uint64> Seen;
	for (int32 Index = 0; Index < State.Systems.Num(); ++Index)
	{
		const FStarSystemState& System = State.Systems[Index];
		if (CosmicHazards::IsDeadNode(System))
		{
			continue;
		}
		for (const FName Lane : System.Lanes)
		{
			const int32* Other = IndexOf.Find(Lane);
			if (!Other || *Other == Index || CosmicHazards::IsDeadNode(State.Systems[*Other]))
			{
				continue;
			}
			const int32 Low = FMath::Min(Index, *Other);
			const int32 High = FMath::Max(Index, *Other);
			bool bAlready = false;
			Seen.Add((static_cast<uint64>(Low) << 32) | static_cast<uint32>(High), &bAlready);
			if (!bAlready)
			{
				Lanes.Emplace(Low, High);
			}
		}
	}
	return Lanes;
}

bool CosmicMapLook::ShowsLabel(const FCosmicSystemLook& Look, bool bSelected, bool bFarView)
{
	// A system the viewer does not know is a backdrop point: no label, near or far, selected or not.
	return Look.bKnown && (!bFarView || bSelected || Look.bOwned || Look.bHazard);
}

bool CosmicMapLook::SpiritualSightOpens(EAscensionTier ViewerTier)
{
	return static_cast<int32>(ViewerTier) >= static_cast<int32>(SpiritualSightTier);
}

ECosmicViewMode CosmicMapLook::ShownViewMode(ECosmicViewMode Wanted, bool bForcedOpen, EAscensionTier ViewerTier)
{
	return Wanted == ECosmicViewMode::Spiritual && (bForcedOpen || SpiritualSightOpens(ViewerTier)) ? ECosmicViewMode::Spiritual : ECosmicViewMode::Physical;
}

float CosmicMapLook::PressureShareOf(float P, const FCosmicRulesDef& Rules)
{
	const int32 Fourth = static_cast<int32>(EAscensionTier::Tier4_Transcendent);
	const float Weight = Rules.PressureBySourceTier.IsValidIndex(Fourth) ? Rules.PressureBySourceTier[Fourth] : 0.f;
	// Compares: a P or a weight that is not a positive number gives no pulse.
	if (!(P > 0.f) || !(Weight > 0.f))
	{
		return 0.f;
	}
	const float Share = P / (2.f * Weight);
	return Share < 1.f ? Share : 1.f;
}

FLinearColor CosmicMapLook::DominionColorOf(const FTranscendentDef* Row)
{
	return CosmicMapLookImpl::IsFallen(Row) ? CosmicPalette::Purple : FLinearColor::Black;
}

FCosmicSpiritLook CosmicMapLook::SpiritOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 SystemIndex)
{
	using namespace CosmicMapLookImpl;

	FCosmicSpiritLook Spirit;
	if (!State.Systems.IsValidIndex(SystemIndex) || CosmicHazards::IsDeadNode(State.Systems[SystemIndex]))
	{
		return Spirit;
	}
	const FStarSystemState& System = State.Systems[SystemIndex];
	// The sanctuary first (Law.Hedge, section 4.5 (1)): no colour, no pressure, no mark.
	if (CosmicState::IsSanctuary(System.SystemID))
	{
		Spirit.bSanctuary = true;
		return Spirit;
	}

	// The holders: the first fallen instance whose sphere holds the system, unsealed and sealed apart (an instance
	// without a row counts for nothing).
	int32 Holder = INDEX_NONE;
	int32 SealedHolder = INDEX_NONE;
	for (int32 At = 0; At < State.Transcendents.Num(); ++At)
	{
		const FTranscendentState& Transcendent = State.Transcendents[At];
		if (!Transcendent.InfluencedSystems.Contains(System.SystemID) || !IsFallen(Tables.FindTranscendent(Transcendent.TranscendentID)))
		{
			continue;
		}
		int32& Slot = Transcendent.bSealed ? SealedHolder : Holder;
		Slot = Slot == INDEX_NONE ? At : Slot;
	}
	Spirit.bSealedRing = SealedHolder != INDEX_NONE;

	// The claimant now: a claim nobody can claim now is no mark.
	const int32 Claimant = System.ActiveHazard.IsValid() && System.ActiveHazard == ClaimTag()
		? CosmicHazards::ClaimantOf(State, Index, Tables, Rules, SystemIndex) : INDEX_NONE;
	Spirit.ClaimState = Claimant != INDEX_NONE ? ECosmicClaimState::Claimed
		: Holder != INDEX_NONE ? ECosmicClaimState::Held
		: SealedHolder != INDEX_NONE ? ECosmicClaimState::Bound : ECosmicClaimState::None;

	// The pulse, where a fallen one leads: the dominant source has a fallen row (a people and a wanderer have none - the
	// kind of the source only says so too) and stands above the owner's rung, or nobody owns the system. Its strength
	// is the whole P, the owner's own term included, as the drain reads it.
	FName Presser;
	const FCosmicPressure Reading = UGalaxySimulationSubsystem::ComputePressureIn(State, Index, Tables, Rules, SystemIndex);
	if (Reading.DominantSource == ECosmicPressureSource::Transcendent && IsFallen(Tables.FindTranscendent(Reading.DominantSourceID)))
	{
		const int32 OwnerIndex = System.OwnerCivID.IsNone() ? INDEX_NONE : Index.FindCiv(State, System.OwnerCivID);
		if (!State.Civs.IsValidIndex(OwnerIndex) || static_cast<int32>(Reading.DominantTier) > static_cast<int32>(State.Civs[OwnerIndex].Tier))
		{
			Spirit.Pressure = PressureShareOf(Reading.P, Rules);
			Presser = Reading.DominantSourceID;
		}
	}

	// One order for the words and the colour: the claimant, the unsealed holder, the presser, the sealed holder - the
	// words always name the one whose purple is drawn.
	if (Claimant != INDEX_NONE)
	{
		Spirit.MarkRow = State.Transcendents[Claimant].TranscendentID;
	}
	else if (Holder != INDEX_NONE)
	{
		Spirit.MarkRow = State.Transcendents[Holder].TranscendentID;
	}
	else if (!Presser.IsNone())
	{
		Spirit.MarkRow = Presser;
		Spirit.bMarkPresses = true;
	}
	else if (SealedHolder != INDEX_NONE)
	{
		Spirit.MarkRow = State.Transcendents[SealedHolder].TranscendentID;
	}
	Spirit.DominionColor = Spirit.MarkRow.IsNone() ? FLinearColor::Black : DominionColorOf(Tables.FindTranscendent(Spirit.MarkRow));
	return Spirit;
}

FLinearColor CosmicMapLook::LaneColorOf(float EndPressure, ECosmicViewMode Shown)
{
	using namespace CosmicMapLookImpl;

	// A compare, so a pressure that is not a number keeps the colour.
	if (Shown != ECosmicViewMode::Spiritual || !(EndPressure > 0.f))
	{
		return LaneColor;
	}
	return Opaque(LaneColor.Desaturate(EndPressure < 1.f ? EndPressure : 1.f));
}

FCosmicStarParams CosmicMapLook::StarParamsOf(const FCosmicSystemLook& Look, const FCosmicSpiritLook& Spirit, ECosmicViewMode Shown)
{
	using namespace CosmicMapLookImpl;

	FCosmicStarParams Params;
	Params.WarningColor = PhysicalWarning;
	if (Look.bDead)
	{
		// A destroyed system is a dim grey in both views and draws nothing spiritual.
		Params.StarColor = DeadColor;
		Params.Brightness = DeadBrightness;
		Params.HazardAmount = 0.f;
		return Params;
	}
	Params.StarColor = Look.StarColor;
	Params.Brightness = Brightness;
	Params.HazardAmount = Look.bHazard ? 1.f : 0.f;
	if (Shown != ECosmicViewMode::Spiritual)
	{
		return Params;
	}
	// The physical layer recedes; the marks the sight draws keep their hue.
	Params.StarColor = Opaque(Look.StarColor.Desaturate(SpiritualDesaturation));
	Params.Brightness = SpiritualBrightness;
	Params.WarningColor = Opaque(CosmicPalette::Grey * SpiritualWarningScale);
	Params.ViewMode = 1.f;
	Params.Pressure = Spirit.Pressure;
	Params.DominionColor = Spirit.DominionColor;
	Params.ClaimState = static_cast<float>(static_cast<int32>(Spirit.ClaimState));
	// A claim pulses in its claimant's colour instead of the warning.
	if (Spirit.ClaimState == ECosmicClaimState::Claimed)
	{
		Params.HazardAmount = 0.f;
	}
	return Params;
}

float CosmicMapLook::SkyIntensityOf(ECosmicViewMode Shown)
{
	return Shown == ECosmicViewMode::Spiritual ? 1.f : 0.f;
}

FLinearColor CosmicMapLook::OwnerRingColorOf(const FLinearColor& OwnerColor, ECosmicViewMode Shown)
{
	if (Shown != ECosmicViewMode::Spiritual)
	{
		return OwnerColor;
	}
	// Presentation only (no hash reads it). Compares: a hue or a saturation that is not a number keeps the colour, and a
	// grey has no hue to keep out.
	const FLinearColor Hsv = OwnerColor.LinearRGBToHSV();
	if (Hsv.G > 0.f && Hsv.R >= FallenHueFrom && Hsv.R <= FallenHueTo)
	{
		const float Grey = OwnerColor.GetLuminance();
		return FLinearColor(Grey, Grey, Grey, 1.f);
	}
	return OwnerColor;
}

bool CosmicMapLook::IsShownPeople(bool bMet, const FCivilizationState& Civ, EAscensionTier ViewerTier)
{
	// The subject rule of the scope of telling (section 8.1): the gap is measured upward only - the one Core predicate.
	return bMet && !CosmicKnowing::IsFarAbove(Civ.Tier, ViewerTier);
}

FCosmicKnownView CosmicMapLook::KnownViewOf(const FGalaxyState& State, const FCosmicStateIndex& Index, EAscensionTier ViewerTier, bool bSeeAll)
{
	// No cut: every mark counts (a tick counter never reaches the largest int64).
	return KnownViewOf(State, Index, ViewerTier, bSeeAll, TNumericLimits<int64>::Max());
}

FCosmicKnownView CosmicMapLook::KnownViewOf(const FGalaxyState& State, const FCosmicStateIndex& Index, EAscensionTier ViewerTier, bool bSeeAll, int64 CutTick)
{
	FCosmicKnownView View;
	View.bSeeAll = bSeeAll;
	View.PendingPetitionRow = State.Knowing.PendingPetitionRow;
	View.PendingPetitionSystemID = State.Knowing.PendingPetitionSystemID;
	View.PendingContactCivID = State.Knowing.PendingContactCivID;
	View.Known.Init(bSeeAll, State.Systems.Num());
	// The sanctuary is shown by its name alone, whatever Knowing holds (section 4.5 (1)): never written there.
	View.ShownByName.Init(false, State.Systems.Num());
	const int32 Sanctuary = Index.FindSystem(State, CosmicIDs::Sanctuary);
	if (View.ShownByName.IsValidIndex(Sanctuary))
	{
		View.ShownByName[Sanctuary] = true;
	}
	if (!bSeeAll)
	{
		for (const FKnownMark& Mark : State.Knowing.KnownSystems)
		{
			const int32 SystemIndex = Mark.Tick < CutTick ? Index.FindSystem(State, Mark.ID) : INDEX_NONE;
			if (View.Known.IsValidIndex(SystemIndex))
			{
				View.Known[SystemIndex] = true;
			}
		}
	}
	// The meeting order: the place in MetCivs, a gone people keeping its place (CivIDs are never reused, so no colour
	// ever passes to another people). A mark at or after the cut is not counted, and the counted ones keep their place.
	View.ShownOrder.Init(INDEX_NONE, State.Civs.Num());
	View.Met.Init(false, State.Civs.Num());
	for (int32 Order = 0; Order < State.Knowing.MetCivs.Num(); ++Order)
	{
		const FKnownMark& Mark = State.Knowing.MetCivs[Order];
		const int32 CivIndex = Mark.Tick < CutTick ? Index.FindCiv(State, Mark.ID) : INDEX_NONE;
		if (View.Met.IsValidIndex(CivIndex))
		{
			View.Met[CivIndex] = true;
		}
		if (State.Civs.IsValidIndex(CivIndex) && View.ShownOrder[CivIndex] == INDEX_NONE
			&& (bSeeAll || IsShownPeople(true, State.Civs[CivIndex], ViewerTier)))
		{
			View.ShownOrder[CivIndex] = Order;
		}
	}
	// See-all shows every people: one not met follows the met ones, in Civs order.
	if (bSeeAll)
	{
		int32 Next = State.Knowing.MetCivs.Num();
		for (int32& Order : View.ShownOrder)
		{
			if (Order == INDEX_NONE)
			{
				Order = Next++;
			}
		}
	}
	return View;
}

ECosmicOwnerShown CosmicMapLook::OwnerShownOf(const FGalaxyState& State, const FCosmicKnownView& View, int32 SystemIndex)
{
	using namespace CosmicMapLookImpl;

	if (!State.Systems.IsValidIndex(SystemIndex) || !View.IsKnown(SystemIndex) || CosmicHazards::IsDeadNode(State.Systems[SystemIndex]))
	{
		return ECosmicOwnerShown::None;
	}
	const int32 OwnerIndex = OwnerIndexOf(State, State.Systems[SystemIndex]);
	if (OwnerIndex == INDEX_NONE)
	{
		return ECosmicOwnerShown::None;
	}
	return View.ShownOrderOf(OwnerIndex) != INDEX_NONE ? ECosmicOwnerShown::Shown : ECosmicOwnerShown::Hidden;
}

FCosmicSystemLook CosmicMapLook::ShownLookOf(const FCosmicSystemLook& World, const FGalaxyState& State, const FCosmicKnownView& View, int32 SystemIndex)
{
	using namespace CosmicMapLookImpl;

	FCosmicSystemLook Unknown;
	Unknown.bKnown = false;
	if (!State.Systems.IsValidIndex(SystemIndex) || !View.IsShown(SystemIndex))
	{
		return Unknown;
	}
	// Shown by its name alone (the sanctuary): its name and its star, nothing the viewer could have learned.
	if (!View.IsKnown(SystemIndex))
	{
		FCosmicSystemLook ByName;
		ByName.Name = World.Name;
		ByName.StarColor = World.StarColor;
		ByName.StarScale = World.StarScale;
		return ByName;
	}
	FCosmicSystemLook Look = World;
	Look.bKnown = true;
	switch (OwnerShownOf(State, View, SystemIndex))
	{
	case ECosmicOwnerShown::Shown:
		Look.OwnerColor = CosmicPalette::OwnerColorOf(View.ShownOrderOf(OwnerIndexOf(State, State.Systems[SystemIndex])));
		break;
	case ECosmicOwnerShown::Hidden:
		// The owner's colour and works are that people's: none of them for a people the viewer is not shown.
		Look.bOwned = false;
		Look.OwnerColor = FLinearColor::Black;
		Look.bSettling = false;
		Look.ConstructionShare = -1.f;
		Look.ResponseShare = -1.f;
		break;
	case ECosmicOwnerShown::None:
		break;
	}
	return Look;
}

TArray<TPair<int32, int32>> CosmicMapLook::KnownLanesOf(const TArray<TPair<int32, int32>>& Lanes, const FCosmicKnownView& View)
{
	TArray<TPair<int32, int32>> Known;
	Known.Reserve(Lanes.Num());
	for (const TPair<int32, int32>& Lane : Lanes)
	{
		if (View.IsShown(Lane.Key) && View.IsShown(Lane.Value))
		{
			Known.Add(Lane);
		}
	}
	return Known;
}

int32 CosmicMapLook::BackdropCountOf(const FCosmicRulesDef& Rules)
{
	return CosmicMapLookImpl::BackdropCountFrom(Rules.BackgroundStarCount);
}

void CosmicMapLook::BackdropLookOf(FRandomStream& Stream, float DiscRadius, FCosmicBackdropPoint& Point)
{
	// Mostly small and mostly faint: the square and the cube of a uniform draw lean toward the low end.
	const float U = Stream.GetFraction();
	Point.Size = DiscRadius * (BackdropMinSizeShare + (BackdropMaxSizeShare - BackdropMinSizeShare) * U * U);
	const float W = Stream.GetFraction();
	Point.Glow = BackdropMinGlow + (BackdropMaxGlow - BackdropMinGlow) * W * W * W;
}

TArray<FCosmicBackdropPoint> CosmicMapLook::BackdropPointsOf(int32 Seed, FName GalaxyRow, int32 Count, const FVector& Centre, float Radius)
{
	const int32 Points = CosmicMapLookImpl::BackdropCountFrom(Count);
	TArray<FCosmicBackdropPoint> Backdrop;
	if (Points == 0 || !(Radius > 0.f))
	{
		return Backdrop;
	}
	// A local stream of the presentation (section 9.2: the third source, Purpose Backdrop): never the simulation's.
	FRandomStream Stream;
	Stream.Initialize(static_cast<int32>(CosmicSeed::DeriveForName(Seed, GalaxyRow, CosmicSeed::Purpose::Backdrop, 0)));
	const double Reach = static_cast<double>(Radius) * BackdropRimShare;
	const double RimWidth = static_cast<double>(Radius) * (BackdropRimShare - 1.f);
	Backdrop.SetNum(Points);
	for (FCosmicBackdropPoint& Point : Backdrop)
	{
		// Even over the disc: the generator's rejection in the square, at most 16 tries, the last one halved (presentation
		// only and outside every hash - no trigonometry is needed either).
		double X = 0.0;
		double Y = 0.0;
		for (int32 Try = 0; Try < 16; ++Try)
		{
			X = 2.0 * Stream.GetFraction() - 1.0;
			Y = 2.0 * Stream.GetFraction() - 1.0;
			if (X * X + Y * Y <= 1.0)
			{
				break;
			}
			if (Try == 15)
			{
				X *= 0.5;
				Y *= 0.5;
			}
		}
		Point.Position = Centre + FVector(X, Y, 0.0) * Reach;
		BackdropLookOf(Stream, Radius, Point);
		// The rim: past the disc's edge the glow fades to 0 at Reach.
		const double Distance = Reach * FMath::Sqrt(X * X + Y * Y);
		if (Distance > Radius && RimWidth > 0.0)
		{
			Point.Glow *= static_cast<float>(FMath::Max(0.0, (Reach - Distance) / RimWidth));
		}
	}
	return Backdrop;
}

FCosmicBackdropPoint CosmicMapLook::BackdropSlotOf(int32 Seed, FName SystemID, const FVector& Position, float DiscRadius)
{
	FCosmicBackdropPoint Point;
	Point.Position = Position;
	FRandomStream Stream;
	Stream.Initialize(static_cast<int32>(CosmicSeed::DeriveForName(Seed, SystemID, CosmicSeed::Purpose::Backdrop, 0)));
	BackdropLookOf(Stream, DiscRadius, Point);
	return Point;
}

FCosmicVesselLook CosmicMapLook::VesselLookOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules)
{
	FCosmicVesselLook Look;
	if (!CosmicWanderer::IsEmbodiedWanderer(State))
	{
		return Look;
	}
	const int32 From = Index.FindSystem(State, State.Player.CurrentSystemID);
	if (!State.Systems.IsValidIndex(From))
	{
		return Look;
	}
	Look.FromIndex = From;
	const int32 To = CosmicWanderer::IsTravelling(State) ? Index.FindSystem(State, State.Player.TravelTargetSystemID) : INDEX_NONE;
	if (State.Systems.IsValidIndex(To))
	{
		Look.ToIndex = To;
		const float Lane = static_cast<float>(CosmicWanderer::LaneTicksOf(Rules));
		Look.Share = FMath::Clamp(1.f - static_cast<float>(State.Player.TravelTicksRemaining) / Lane, 0.f, 1.f);
	}
	return Look;
}

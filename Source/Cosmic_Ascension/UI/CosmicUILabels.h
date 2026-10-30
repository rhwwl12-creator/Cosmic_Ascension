// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 2.1, 5.7, 5.11, 6.3, 8.3, 8.5.
// Small shared helpers of the C++ widgets: the String Table labels, the number forms the UI may show, the speed steps
// and the third rung's speed gate, the joins of texts, the life words and the galaxy kinds.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Containers/ArrayView.h"
#include "Core/CosmicIDs.h"
#include "Core/CosmicSeed.h"
#include "Core/CosmicTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Fonts/SlateFontInfo.h"
#include "GameplayTagContainer.h"
#include "Internationalization/StringTableCoreFwd.h"
#include "Internationalization/Text.h"
#include "Misc/Optional.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicResearch.h"
#include "World/CosmicZoomLevels.h"

/** How the UI shows the clock (guideline 2.1): in cycles, or in years counted from the first record (1 cycle = 1 year,
 * no calendar). The viewer's choice: never in FGalaxyState, never in a save, outside both hashes. The narration and the
 * chronicle always say cycles. */
enum class ECosmicTimeUnit : uint8
{
	Cycle,
	Year,
};

/** What the mouse has done once this session, for the galaxy panel's hints (section 6.4 'the universe screen').
 * Presentation state: never saved, kept across new galaxies in the session (the player already knows). */
struct COSMIC_ASCENSION_API FCosmicHintProgress
{
	// The viewer's first system selection (the crisis focus's does not count, MVP item 6).
	bool bPickedSystem = false;
	// The first visit of the universe level.
	bool bReachedUniverse = false;
	// The first galaxy selection.
	bool bPickedGalaxy = false;
	// The viewer's first move from the universe to the region (a rebuild's framing and the crisis focus do not count).
	bool bLeftUniverse = false;
	// The first project started (MVP item 6): the system panel's hint line goes (CosmicUI::ShowsCommandHint).
	bool bStartedProject = false;
	// The first ship's line was shown (B1): once a session, on the capital's panel (CosmicUI::ShowsShipHint).
	bool bShipHinted = false;
	// The inherited edict's hint (B2): it came - the ruled people held DT_Rules.StartingEdict below
	// CosmicUI::EdictHintStability -, and it ended - the edict window opened, or the people holds that edict no more
	// (CosmicUI::StepEdictHint, ShowsEdictHint). Once a session.
	bool bEdictHintOn = false;
	bool bEdictHintDone = false;
};

namespace CosmicUI
{
	// A label of the String Table of the labels (CosmicLabels::TableId, section 8.5). A blocking load on purpose: the
	// default policy loads the table asynchronously while anything else is loading, and the lines formatted meanwhile
	// would keep the placeholder of a missing entry.
	inline FText Label(const TCHAR* Key)
	{
		return FText::FromStringTable(CosmicLabels::TableId, Key, EStringTableLoadingPolicy::FindOrFullyLoad);
	}

	// The short words after a fleet's greyed command while the people has no fleet yet - [Guard here] of the panel, a card's
	// fleet choice (B4): the rung suffix of the edict window and the projects, "(rung N on)", N the first rung at or above
	// the people's own that gives a fleet (CosmicConflict::FirstFleetRungOf). Empty when none does.
	inline FText FleetSuffix(const FCosmicRulesDef& Rules, EAscensionTier From)
	{
		const int32 At = CosmicConflict::FirstFleetRungOf(Rules, From);
		return At == INDEX_NONE ? FText::GetEmpty() : FText::Format(Label(TEXT("Guard.Suffix.NoFleet")), FText::AsNumber(At + 1));
	}

	// The String Table key of a population band's label (section 8.3), named after its enumerator and written out here (an
	// enumerator's name keeps its case in an editor build only). Null for a value past the last band. (Phase 4: the log's
	// {PopBand} reads the same keys - this table then moves to a Core-side header, Log/ cannot include UI/.)
	inline const TCHAR* PopulationBandKey(EPopulationBand Band)
	{
		switch (Band)
		{
		case EPopulationBand::Few:
			return TEXT("Band.Few");
		case EPopulationBand::Millions:
			return TEXT("Band.Millions");
		case EPopulationBand::Billions:
			return TEXT("Band.Billions");
		case EPopulationBand::HundredBillions:
			return TEXT("Band.HundredBillions");
		case EPopulationBand::Trillions:
			return TEXT("Band.Trillions");
		case EPopulationBand::Countless:
			return TEXT("Band.Countless");
		}
		return nullptr;
	}

	// The label of a band: the only form in which the size of a people reaches the UI - never a number. Empty past the
	// last band.
	inline FText PopulationBandLabel(EPopulationBand Band)
	{
		const TCHAR* Key = PopulationBandKey(Band);
		return Key ? Label(Key) : FText::GetEmpty();
	}

	// The system panel's people line: ST_CA Panel.People with the band in its slot {PopBand}, the band before the word for
	// a people (the word order is the table's, section 16 #45).
	inline FText PeopleLine(EPopulationBand Band)
	{
		return FText::FormatNamed(Label(TEXT("Panel.People")), TEXT("PopBand"), PopulationBandLabel(Band));
	}

	// Resources and progress as whole numbers (section 8.3: resource numbers are for the UI only).
	inline FText WholeNumber(float Value)
	{
		return FText::AsNumber(FMath::FloorToInt32(Value));
	}

	inline FText Percent(float Ratio)
	{
		FNumberFormattingOptions Options;
		Options.MaximumFractionalDigits = 0;
		return FText::AsPercent(Ratio, &Options);
	}

	// The time labels take the unit (guideline 6.3, the clock toggle): the widgets pass their controller's, the tests their own.
	// The cycle forms are the labels of Phase 3; a year is the cycle's number counted from the first record.

	// The clock of the top bar: Panel.Cycle and the number ("cycle 1,204"), or Time.Year ("1,204 years").
	inline FText ClockLabel(int64 Tick, ECosmicTimeUnit Unit)
	{
		return Unit == ECosmicTimeUnit::Year ? FText::Format(Label(TEXT("Time.Year")), FText::AsNumber(Tick))
			: FText::Format(INVTEXT("{0} {1}"), Label(TEXT("Panel.Cycle")), FText::AsNumber(Tick));
	}

	// Both units at once, while the mouse is on the clock (Time.Both: the cycle form, then the year form "from the first record").
	inline FText ClockBothLabel(int64 Tick)
	{
		return FText::Format(Label(TEXT("Time.Both")), ClockLabel(Tick, ECosmicTimeUnit::Cycle), ClockLabel(Tick, ECosmicTimeUnit::Year));
	}

	// A countdown: Panel.CyclesLeft and the number, or Time.YearsLeft ("120 years left").
	inline FText TimeLeftLabel(int64 Ticks, ECosmicTimeUnit Unit)
	{
		return Unit == ECosmicTimeUnit::Year ? FText::Format(Label(TEXT("Time.YearsLeft")), FText::AsNumber(Ticks))
			: FText::Format(INVTEXT("{0} {1}"), Label(TEXT("Panel.CyclesLeft")), FText::AsNumber(Ticks));
	}

	// A length of time (MVP item 6, the stability trend's tooltip): Time.Cycles ("60 cycles") or Time.Year ("60 years").
	inline FText DurationLabel(int64 Ticks, ECosmicTimeUnit Unit)
	{
		return FText::Format(Label(Unit == ECosmicTimeUnit::Year ? TEXT("Time.Year") : TEXT("Time.Cycles")), FText::AsNumber(Ticks));
	}

	// The stamp at the head of a log line: the bare number, or Time.Year.
	inline FText StampLabel(int64 Tick, ECosmicTimeUnit Unit)
	{
		return Unit == ECosmicTimeUnit::Year ? FText::Format(Label(TEXT("Time.Year")), FText::AsNumber(Tick)) : FText::AsNumber(Tick);
	}

	// A speed button: "x2", or Time.YearsPerSecond ("2 years a second"); 0 is the pause (Speed.Pause).
	inline FText SpeedLabel(float Speed, ECosmicTimeUnit Unit)
	{
		if (!(Speed > 0.f))
		{
			return Label(TEXT("Speed.Pause"));
		}
		return Unit == ECosmicTimeUnit::Year ? FText::Format(Label(TEXT("Time.YearsPerSecond")), WholeNumber(Speed))
			: FText::Format(INVTEXT("x{0}"), WholeNumber(Speed));
	}

	// The three stages of time (guideline 2.1): the calling runs up to x10; once the third rung's gate is open, one step
	// more, x25 (the top bar's source asserts it is not above UGalaxySimulationSubsystem::MaxTimeScale).
	inline constexpr float CallingTopSpeed = 10.f;
	inline constexpr float ThirdRungTopSpeed = 25.f;

	// The speed steps of the top bar's buttons and of the number keys 0 to 5, pause first.
	inline constexpr float SpeedSteps[] = { 0.f, 1.f, 2.f, 5.f, CallingTopSpeed, ThirdRungTopSpeed };

	// Whether Speed is offered: up to CallingTopSpeed always, above it while the third rung's gate is open - the gate of
	// the spiritual sight, CosmicMapLook::SpiritualSightOpens of the viewer tier (the player controller's
	// IsSpiritualSightOpen, its development force included). One gate for the sight and for x25 (guideline 6.4).
	inline bool IsSpeedOpen(float Speed, bool bThirdRungOpen)
	{
		return !(Speed > CallingTopSpeed) || bThirdRungOpen;
	}

	// What a speed button or key, the pause's release or the ascension presentation's give-back sets: Speed when it is
	// open, CallingTopSpeed when it is not.
	inline float OpenSpeed(float Speed, bool bThirdRungOpen)
	{
		return IsSpeedOpen(Speed, bThirdRungOpen) ? Speed : CallingTopSpeed;
	}

	// Whether a running Speed falls to CallingTopSpeed now: the gate was open at the last look (WasOpen set and true) and
	// is closed now (a deposition, a new galaxy, a lower save) while Speed is above CallingTopSpeed. A first look (WasOpen
	// unset) lowers nothing, so a speed the gate never opened (the development argument -Speed=25) stays.
	inline bool LowersSpeed(const TOptional<bool>& WasOpen, bool bOpenNow, float Speed)
	{
		return WasOpen.IsSet() && WasOpen.GetValue() && !bOpenNow && Speed > CallingTopSpeed;
	}

	// Texts joined by the String Table's separator (List.Pair, "{0} <middle dot> {1}" - the dot lives in the table, the
	// code stays ASCII), the empty ones left out; empty when all are. Call it with a braced list in the call itself: a view
	// of temporaries kept in a local would dangle.
	inline FText JoinList(TConstArrayView<FText> Texts)
	{
		FText Joined;
		bool bAny = false;
		for (const FText& Text : Texts)
		{
			if (Text.IsEmpty())
			{
				continue;
			}
			Joined = bAny ? FText::Format(Label(TEXT("List.Pair")), Joined, Text) : Text;
			bAny = true;
		}
		return bAny ? Joined : FText::GetEmpty();
	}

	// Texts one per line, the empty ones left out (a tooltip of a reason and its hint): empty when all are.
	inline FText JoinLines(TConstArrayView<FText> Texts)
	{
		TArray<FText> Shown;
		for (const FText& Text : Texts)
		{
			if (!Text.IsEmpty())
			{
				Shown.Add(Text);
			}
		}
		return FText::Join(INVTEXT("\n"), Shown);
	}

	// The four research fields in the top bar's order, with their labels (MVP item 7: the next rung's tooltip, the system
	// panel's focus buttons and the shares of a focus read this one table). Compared as FNames (the case of a name is kept in
	// an editor build only, so never through FName::ToString).
	inline constexpr const TCHAR* ResearchFieldNames[] = { TEXT("Physics"), TEXT("Society"), TEXT("Engineering"), TEXT("Transcendence") };
	inline constexpr const TCHAR* ResearchLabelKeys[] = { TEXT("Research.Physics"), TEXT("Research.Society"), TEXT("Research.Engineering"), TEXT("Research.Transcendence") };
	static_assert(UE_ARRAY_COUNT(ResearchFieldNames) == UE_ARRAY_COUNT(ResearchLabelKeys), "One label per research field");

	// The label key of a research field; null for a key that is not one.
	inline const TCHAR* ResearchLabelKey(FName Field)
	{
		for (int32 At = 0; At < static_cast<int32>(UE_ARRAY_COUNT(ResearchFieldNames)); ++At)
		{
			if (Field == FName(ResearchFieldNames[At]))
			{
				return ResearchLabelKeys[At];
			}
		}
		return nullptr;
	}

	// The shares of a research focus in the top bar's order, each as its label and Percent, joined by JoinList
	// ("Physics 55%, Society 15%, ..." with the List.Pair mark between); a field the map lacks is left out.
	inline FText FocusSharesOf(const TMap<FName, float>& ResearchFocus)
	{
		TArray<FText> Shares;
		for (int32 At = 0; At < static_cast<int32>(UE_ARRAY_COUNT(ResearchFieldNames)); ++At)
		{
			if (const float* Share = ResearchFocus.Find(FName(ResearchFieldNames[At])))
			{
				Shares.Add(FText::Format(INVTEXT("{0} {1}"), Label(ResearchLabelKeys[At]), Percent(*Share)));
			}
		}
		return JoinList(Shares);
	}

	// Whether a press of the focus button of Field (NAME_None: the even one) would leave ResearchFocus as it is: the map
	// CosmicResearch::Focus (with Share, the DT_Rules.ResearchFocusShare the command uses) or Even makes from it, compared
	// key by key (B2 section 2.8 - a preset, not a shape: the slider's 1-5-1 is no focus on society). False when the rule
	// makes nothing of the map.
	inline bool IsPresetNow(const TMap<FName, float>& ResearchFocus, FName Field, float Share)
	{
		TMap<FName, float> Would = ResearchFocus;
		const bool bMade = Field.IsNone() ? CosmicResearch::Even(Would) : CosmicResearch::Focus(Would, Field, Share);
		return bMade && Would.OrderIndependentCompareEqual(ResearchFocus);
	}

	// The word of a research focus: Focus.Even for the even preset, a field's label for that field's preset (IsPresetNow
	// with Share - the four fields in the top bar's order, transcendence too: the console can lean to it), else Focus.Mixed.
	inline FText FocusWordOf(const TMap<FName, float>& ResearchFocus, float Share)
	{
		if (IsPresetNow(ResearchFocus, NAME_None, Share))
		{
			return Label(TEXT("Focus.Even"));
		}
		for (int32 At = 0; At < static_cast<int32>(UE_ARRAY_COUNT(ResearchFieldNames)); ++At)
		{
			const FName Field(ResearchFieldNames[At]);
			if (ResearchFocus.Contains(Field) && IsPresetNow(ResearchFocus, Field, Share))
			{
				return Label(ResearchLabelKeys[At]);
			}
		}
		return Label(TEXT("Focus.Mixed"));
	}

	inline UTextBlock* MakeText(UWidgetTree& Tree, int32 FontSize, bool bWrap = true)
	{
		UTextBlock* Text = Tree.ConstructWidget<UTextBlock>();
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = FontSize;
		Font.TypefaceFontName = FName(TEXT("Regular"));
		Text->SetFont(Font);
		Text->SetAutoWrapText(bWrap);
		return Text;
	}

	// The twelve life words of the system panel (section 5.7, the life word): String Table keys, in the order that maps a
	// system's Life derive to a word - append only (another order or count changes every world's word; display only,
	// nothing is saved or hashed). No number, no creature's proper name, no verdict or harvest word, none of the spirit's
	// four pictures (UI.KnownFilter walks them).
	inline constexpr const TCHAR* LifeWordKeys[] = {
		TEXT("Life.Shoals"), TEXT("Life.Mats"), TEXT("Life.Forests"), TEXT("Life.Flyers"),
		TEXT("Life.Burrowers"), TEXT("Life.Vents"), TEXT("Life.UnderIce"), TEXT("Life.Drifters"),
		TEXT("Life.Vines"), TEXT("Life.Shelled"), TEXT("Life.Grazers"), TEXT("Life.Mycelium"),
	};
	static_assert(UE_ARRAY_COUNT(LifeWordKeys) == 12, "Twelve life words (section 5.7)");

	// The life word of SystemID in a run of Seed: CosmicSeed::DeriveForName(Seed, SystemID, Purpose::Life, 0) modulo twelve
	// (a finalized hash - its low bits are as good as its high ones; the rule against a modulo is FRandomStream's).
	inline int32 LifeWordIndexOf(int32 Seed, FName SystemID)
	{
		return static_cast<int32>(CosmicSeed::DeriveForName(Seed, SystemID, CosmicSeed::Purpose::Life, 0) % static_cast<uint32>(UE_ARRAY_COUNT(LifeWordKeys)));
	}

	// The system panel's life line: Panel.Life and the word of LifeWordIndexOf ("<Panel.Life>  <word>"); empty for the
	// sanctuary (CosmicState::IsSanctuary), whose fixed line "this place is kept" stands instead (section 6.3).
	inline FText LifeLine(int32 Seed, FName SystemID)
	{
		if (CosmicState::IsSanctuary(SystemID))
		{
			return FText::GetEmpty();
		}
		return FText::Format(INVTEXT("{0}  {1}"), Label(TEXT("Panel.Life")), Label(LifeWordKeys[LifeWordIndexOf(Seed, SystemID)]));
	}

	// The galaxy panel's hints, one at a time (section 6.4 'the universe screen'): item 8's (a star), then the wheel past the
	// region, then a galaxy at the universe level, then the way home. String Table keys; HintHomeKey's {Home} is the home
	// galaxy's DisplayName (UCosmicGalaxyPanelWidget::ShowHint fills it).
	inline constexpr const TCHAR* HintPickStarKey = TEXT("Galaxy.Hint");
	inline constexpr const TCHAR* HintZoomOutKey = TEXT("Galaxy.HintZoomOut");
	inline constexpr const TCHAR* HintPickGalaxyKey = TEXT("Galaxy.HintPickGalaxy");
	inline constexpr const TCHAR* HintHomeKey = TEXT("Galaxy.HintHome");

	/** The hint for a level and a progress: in the region item 8's until the first system selection, then HintZoomOutKey
	 * until the first visit of the universe; at the universe HintPickGalaxyKey until the first galaxy selection, then
	 * HintHomeKey until the first move back to the region; null (no line) after them. */
	inline const TCHAR* HintKeyOf(ECosmicZoomLevel Level, const FCosmicHintProgress& Progress)
	{
		if (Level == ECosmicZoomLevel::Universe)
		{
			return !Progress.bPickedGalaxy ? HintPickGalaxyKey : (!Progress.bLeftUniverse ? HintHomeKey : nullptr);
		}
		return !Progress.bPickedSystem ? HintPickStarKey : (!Progress.bReachedUniverse ? HintZoomOutKey : nullptr);
	}

	/** The system panel's hint line (MVP item 6, Hint.Commands): while the shown panel offers at least one enabled project
	 * of the ruler, the run is not over and no project was started this session. It reads what the panel shows, not who
	 * selected the system (the automatic crisis focus shows it too). */
	inline bool ShowsCommandHint(bool bPanelHasEnabledProject, bool bRunEnded, const FCosmicHintProgress& Progress)
	{
		return bPanelHasEnabledProject && !bRunEnded && !Progress.bStartedProject;
	}

	// The first ship's line (B1 header section 4): from 8:30 of x1 (docs/00_VISION.md - the first ship is ready), for ten
	// minutes of x1.
	inline constexpr int64 FirstShipFromTick = 510;
	inline constexpr int64 FirstShipUntilTick = 1110;

	/** The capital's first-ship line (B1, ST_CA Hint.FirstShip): the panel shows the capital of the people an embodied
	 * ruler rules (bRulersCapital), Tick is in [FirstShipFromTick, FirstShipUntilTick), the run is not over, the vessel never
	 * left a throne (bLeftOnce - Player.FormerCivID is set), no rebellion stands on the capital, nor a lethal hazard where
	 * the vessel would walk from (bRebellionStands - B2: a neglected inherited edict puts a rebellion inside this window;
	 * the line then waits until it is answered, so it never reads as advice to flee it; B5 [B-5r]: nor while
	 * CosmicAutoPause::LethalStayOf answers for the walk from where the vessel stands, so it never reads as advice to walk
	 * into it), and the line was not shown this session - or this is the viewing it was shown in
	 * (bThisViewing: the panel has shown that capital since). */
	inline bool ShowsShipHint(bool bRulersCapital, int64 Tick, bool bRunEnded, bool bLeftOnce, const FCosmicHintProgress& Progress, bool bThisViewing,
		bool bRebellionStands)
	{
		return bRulersCapital && Tick >= FirstShipFromTick && Tick < FirstShipUntilTick && !bRunEnded && !bLeftOnce && !bRebellionStands
			&& (!Progress.bShipHinted || bThisViewing);
	}

	// The edict hint (B2 header section 4): from the first look at which the ruled people holds the inherited edict with its
	// Stability below this - the first murmur's line.
	inline constexpr float EdictHintStability = 0.5f;

	/** Steps the edict hint after a look at an embodied ruler's people (bRuler; nothing changes otherwise): holding the
	 * inherited edict (bHoldsInherited) with Stability below EdictHintStability starts it, once a session; a started hint ends
	 * when the people holds that edict no more (a revocation, a lapse). The edict window's open ends it too
	 * (ACosmicPlayerController::NoteEdictWindowOpened), started or not. */
	inline void StepEdictHint(FCosmicHintProgress& Progress, bool bRuler, bool bHoldsInherited, float Stability)
	{
		if (Progress.bEdictHintDone || !bRuler)
		{
			return;
		}
		if (!Progress.bEdictHintOn)
		{
			Progress.bEdictHintOn = bHoldsInherited && Stability < EdictHintStability;
		}
		else if (!bHoldsInherited)
		{
			Progress.bEdictHintDone = true;
		}
	}

	/** Whether the edict hint stands: the top bar's mark and the first line of its edict button's tooltip, the capital
	 * panel's line. */
	inline bool ShowsEdictHint(const FCosmicHintProgress& Progress)
	{
		return Progress.bEdictHintOn && !Progress.bEdictHintDone;
	}

	// The String Table key of a galaxy's kind (section 3.1, 5.11): the four Galaxy.Kind tags, matched as tags requested by
	// name - never by a tag's printed name, whose case only an editor build keeps; the key is the tag's name. Null for any
	// other tag.
	inline const TCHAR* GalaxyKindKey(FGameplayTag Kind)
	{
		static const TCHAR* const Kinds[] = { TEXT("Galaxy.Kind.Dwarf"), TEXT("Galaxy.Kind.Spiral"), TEXT("Galaxy.Kind.GrandSpiral"), TEXT("Galaxy.Kind.Elliptical") };
		for (const TCHAR* Each : Kinds)
		{
			if (Kind.IsValid() && Kind == FGameplayTag::RequestGameplayTag(FName(Each), false))
			{
				return Each;
			}
		}
		return nullptr;
	}
}

// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 2.1, 6.3, 8.3, 8.5.
// Small shared helpers of the C++ widgets: the String Table labels, the number forms the UI may show, the speed steps
// and the third rung's speed gate, and the joins of texts.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Containers/ArrayView.h"
#include "Core/CosmicTypes.h"
#include "Fonts/SlateFontInfo.h"
#include "Internationalization/StringTableCoreFwd.h"
#include "Internationalization/Text.h"
#include "Misc/Optional.h"

/** How the UI shows the clock (guideline 2.1): in cycles, or in years counted from the first record (1 cycle = 1 year,
 * no calendar). The viewer's choice: never in FGalaxyState, never in a save, outside both hashes. The narration and the
 * chronicle always say cycles. */
enum class ECosmicTimeUnit : uint8
{
	Cycle,
	Year,
};

namespace CosmicUI
{
	// The String Table of the UI labels (section 8.5).
	inline const FName LabelTableId(TEXT("/Game/CA/Data/ST_CA.ST_CA"));

	// A blocking load on purpose: the default policy loads the table asynchronously while anything else is loading, and
	// the lines formatted meanwhile would keep the placeholder of a missing entry.
	inline FText Label(const TCHAR* Key)
	{
		return FText::FromStringTable(LabelTableId, Key, EStringTableLoadingPolicy::FindOrFullyLoad);
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
}

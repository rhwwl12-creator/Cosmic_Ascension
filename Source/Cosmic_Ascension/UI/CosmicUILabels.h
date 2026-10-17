// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 2.1, 6.3, 8.3, 8.5.
// Small shared helpers of the C++ widgets: the String Table labels and the number forms the UI may show.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Fonts/SlateFontInfo.h"
#include "Internationalization/StringTableCoreFwd.h"
#include "Internationalization/Text.h"

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

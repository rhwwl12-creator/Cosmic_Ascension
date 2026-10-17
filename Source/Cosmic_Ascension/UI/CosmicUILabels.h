// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 8.3, 8.5.
// Small shared helpers of the C++ widgets: the String Table labels and the number forms the UI may show.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Fonts/SlateFontInfo.h"
#include "Internationalization/StringTableCoreFwd.h"
#include "Internationalization/Text.h"

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

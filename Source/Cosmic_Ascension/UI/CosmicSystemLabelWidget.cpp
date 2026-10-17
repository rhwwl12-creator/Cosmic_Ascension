// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicSystemLabelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Fonts/SlateFontInfo.h"
#include "Layout/Margin.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicSystemLabelImpl
{
	const FLinearColor ConstructionFill(0.35f, 0.65f, 1.f);
	const FLinearColor ResponseFill(1.f, 0.6f, 0.2f);

	UProgressBar* MakeBar(UWidgetTree& Tree, const FLinearColor& Fill)
	{
		UProgressBar* Bar = Tree.ConstructWidget<UProgressBar>();
		Bar->SetFillColorAndOpacity(Fill);
		Bar->SetVisibility(ESlateVisibility::Collapsed);
		return Bar;
	}

	void ShowShare(UProgressBar* Bar, float Share)
	{
		if (!Bar)
		{
			return;
		}
		Bar->SetVisibility(Share >= 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		Bar->SetPercent(Share >= 0.f ? Share : 0.f);
	}
}

TSharedRef<SWidget> UCosmicSystemLabelWidget::RebuildWidget()
{
	using namespace CosmicSystemLabelImpl;

	// RebuildWidget can run again after the Slate widgets are released; the tree is built once.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>();
		Root->SetVisibility(ESlateVisibility::HitTestInvisible);
		WidgetTree->RootWidget = Root;

		NameText = WidgetTree->ConstructWidget<UTextBlock>();
		FSlateFontInfo Font = NameText->GetFont();
		Font.Size = 10;
		Font.TypefaceFontName = FName(TEXT("Regular"));
		NameText->SetFont(Font);
		NameText->SetShadowOffset(FVector2D(1.f, 1.f));
		NameText->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.8f));
		NameText->SetJustification(ETextJustify::Center);
		Root->AddChildToVerticalBox(NameText)->SetHorizontalAlignment(HAlign_Center);

		ConstructionBar = MakeBar(*WidgetTree, ConstructionFill);
		ResponseBar = MakeBar(*WidgetTree, ResponseFill);
		for (UProgressBar* Bar : { ConstructionBar.Get(), ResponseBar.Get() })
		{
			USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>();
			Size->SetWidthOverride(72.f);
			Size->SetHeightOverride(4.f);
			Size->AddChild(Bar);
			UVerticalBoxSlot* BarSlot = Root->AddChildToVerticalBox(Size);
			BarSlot->SetHorizontalAlignment(HAlign_Center);
			BarSlot->SetPadding(FMargin(0.f, 2.f, 0.f, 0.f));
		}
		Show(Shown);
	}
	return Super::RebuildWidget();
}

void UCosmicSystemLabelWidget::Show(const FCosmicSystemLook& Look)
{
	using namespace CosmicSystemLabelImpl;

	Shown = Look;
	if (NameText)
	{
		NameText->SetText(Look.Name);
		NameText->SetColorAndOpacity(FSlateColor(Look.bDead ? FLinearColor(0.5f, 0.5f, 0.5f, 0.6f) : FLinearColor::White));
	}
	ShowShare(ConstructionBar, Look.bDead ? -1.f : Look.ConstructionShare);
	ShowShare(ResponseBar, Look.bDead ? -1.f : Look.ResponseShare);
}

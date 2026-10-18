// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicGalaxyPanelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/GameInstance.h"
#include "Layout/Margin.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Styling/SlateColor.h"
#include "UI/CosmicUILabels.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicGalaxyPanelImpl
{
	// The system panel's box (the two panels share one place and never stand together).
	const FLinearColor BoxColor(0.01f, 0.01f, 0.03f, 0.85f);
	// The hint's grey: a line that invites, quieter than the facts.
	const FLinearColor HintColor(0.6f, 0.6f, 0.65f);
}

TSharedRef<SWidget> UCosmicGalaxyPanelWidget::RebuildWidget()
{
	using namespace CosmicGalaxyPanelImpl;

	// RebuildWidget can run again after the Slate widgets are released; the tree is built once.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		// The box sits at the top of the panel's area; the rest of the area lets clicks through to the map.
		UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>();
		Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		WidgetTree->RootWidget = Root;
		UBorder* Box = WidgetTree->ConstructWidget<UBorder>();
		Box->SetBrushColor(BoxColor);
		Box->SetPadding(FMargin(16.f, 12.f));
		Root->AddChildToVerticalBox(Box);

		UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
		Box->SetContent(Column);
		NameText = CosmicUI::MakeText(*WidgetTree, 18);
		Column->AddChildToVerticalBox(NameText)->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
		LinesText = CosmicUI::MakeText(*WidgetTree, 13);
		Column->AddChildToVerticalBox(LinesText);
		HintText = CosmicUI::MakeText(*WidgetTree, 13);
		HintText->SetColorAndOpacity(FSlateColor(HintColor));
		HintText->SetText(CosmicUI::Label(TEXT("Galaxy.Hint")));
		Column->AddChildToVerticalBox(HintText)->SetPadding(FMargin(0.f, 10.f, 0.f, 0.f));
	}
	TSharedRef<SWidget> Built = Super::RebuildWidget();
	// A row given before the tree existed is drawn now.
	Redraw();
	return Built;
}

void UCosmicGalaxyPanelWidget::ShowGalaxy(FName Row)
{
	ShownRow = Row;
	Redraw();
}

TArray<FText> UCosmicGalaxyPanelWidget::LinesOf(const FGalaxyDef& Galaxy)
{
	TArray<FText> Lines;
	// The kind at tag level (Galaxy.Kind.Spiral covers the barred and the unbarred spiral): the row's own texts say more.
	if (const TCHAR* KindKey = CosmicUI::GalaxyKindKey(Galaxy.Kind))
	{
		Lines.Add(CosmicUI::Label(KindKey));
	}
	// The real figures exactly as the row holds them, each with its own hedge (section 3.4, 8.3); an empty one is no line.
	for (const FText* Text : { &Galaxy.StarCountText, &Galaxy.DistanceText, &Galaxy.FeatureText })
	{
		if (!Text->IsEmpty())
		{
			Lines.Add(*Text);
		}
	}
	// The one line of every galaxy, the home galaxy too: no tag, column, colour or allegiance (section 6.4).
	Lines.Add(CosmicUI::Label(TEXT("Galaxy.Named")));
	return Lines;
}

void UCosmicGalaxyPanelWidget::DropHint()
{
	bHintDropped = true;
	if (HintText)
	{
		HintText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCosmicGalaxyPanelWidget::RestoreHintForTests()
{
	bHintDropped = false;
	Redraw();
}

bool UCosmicGalaxyPanelWidget::IsHintShown() const
{
	return HintText && HintText->GetVisibility() != ESlateVisibility::Collapsed && GetVisibility() != ESlateVisibility::Collapsed;
}

void UCosmicGalaxyPanelWidget::Redraw()
{
	if (!NameText || !LinesText || !HintText)
	{
		return;
	}
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	const FGalaxyDef* Galaxy = (Sim && !ShownRow.IsNone()) ? Sim->GetTables().FindGalaxy(ShownRow) : nullptr;
	if (!Galaxy)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	// The box takes the clicks on it (NativeOnMouseButtonDown); the panel's empty area does not.
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	NameText->SetText(Galaxy->DisplayName);
	// The facts, then an empty line, then the one line of every galaxy.
	TArray<FText> Lines = LinesOf(*Galaxy);
	if (Lines.Num() > 1)
	{
		Lines.Insert(FText::GetEmpty(), Lines.Num() - 1);
	}
	LinesText->SetText(FText::Join(INVTEXT("\n"), Lines));
	HintText->SetVisibility(bHintDropped ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}

FReply UCosmicGalaxyPanelWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Only a hit-testable part of the panel reaches here (the widget and its root are self-hit-test-invisible).
	return FReply::Handled();
}

FReply UCosmicGalaxyPanelWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicTextPanelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/ScrollBox.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "Layout/Margin.h"
#include "Log/CosmicLogSubsystem.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UI/CosmicUILabels.h"

// Constants of this file. A named namespace: the module is built as one unity file.
namespace CosmicTextPanelImpl
{
	// Log lines shown at once. The virtualized feed of Phase 3 item 4 lifts this.
	constexpr int32 MaxShownLogLines = 60;
	const FLinearColor BoxColor(0.01f, 0.01f, 0.03f, 0.7f);
}

TSharedRef<SWidget> UCosmicTextPanelWidget::RebuildWidget()
{
	// RebuildWidget can run again after the Slate widgets are released; the tree is built once.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UBorder* Box = WidgetTree->ConstructWidget<UBorder>();
		Box->SetBrushColor(CosmicTextPanelImpl::BoxColor);
		WidgetTree->RootWidget = Box;
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>();
		Box->SetContent(Root);

		TitleText = CosmicUI::MakeText(*WidgetTree, 14);
		TitleText->SetText(CosmicUI::Label(TEXT("Panel.Log")));
		Root->AddChildToVerticalBox(TitleText)->SetPadding(FMargin(16.f, 8.f, 16.f, 4.f));

		LogText = CosmicUI::MakeText(*WidgetTree, 13);
		LogScroll = WidgetTree->ConstructWidget<UScrollBox>();
		LogScroll->AddChild(LogText);
		UVerticalBoxSlot* LogSlot = Root->AddChildToVerticalBox(LogScroll);
		LogSlot->SetPadding(FMargin(16.f, 0.f, 16.f, 12.f));
		LogSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	return Super::RebuildWidget();
}

void UCosmicTextPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGalaxySimulationSubsystem* Sim = GameInstance->GetSubsystem<UGalaxySimulationSubsystem>())
		{
			Sim->OnGalaxyGenerated.AddUniqueDynamic(this, &UCosmicTextPanelWidget::HandleGalaxyGenerated);
		}
		if (UCosmicLogSubsystem* Log = GameInstance->GetSubsystem<UCosmicLogSubsystem>())
		{
			Log->OnLogEntry.AddUniqueDynamic(this, &UCosmicTextPanelWidget::HandleLogEntry);
		}
	}
	Refresh();
}

void UCosmicTextPanelWidget::Refresh()
{
	RefreshLog();
}

void UCosmicTextPanelWidget::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGalaxySimulationSubsystem* Sim = GameInstance->GetSubsystem<UGalaxySimulationSubsystem>())
		{
			Sim->OnGalaxyGenerated.RemoveDynamic(this, &UCosmicTextPanelWidget::HandleGalaxyGenerated);
		}
		if (UCosmicLogSubsystem* Log = GameInstance->GetSubsystem<UCosmicLogSubsystem>())
		{
			Log->OnLogEntry.RemoveDynamic(this, &UCosmicTextPanelWidget::HandleLogEntry);
		}
	}
	Super::NativeDestruct();
}

void UCosmicTextPanelWidget::HandleGalaxyGenerated(int32 Seed)
{
	Refresh();
}

void UCosmicTextPanelWidget::HandleLogEntry(const FCosmicLogEntry& Entry)
{
	RefreshLog();
}

void UCosmicTextPanelWidget::RefreshLog()
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UGalaxySimulationSubsystem* Sim = GameInstance ? GameInstance->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || !LogText)
	{
		return;
	}
	const TArray<FCosmicLogEntry>& Log = Sim->GetState().Log;

	// The last told entries, oldest first: an entry marked bUntold is kept for the chronicle and never shown (section 8.1).
	TArray<int32> Shown;
	for (int32 Index = Log.Num() - 1; Index >= 0 && Shown.Num() < CosmicTextPanelImpl::MaxShownLogLines; --Index)
	{
		if (!Log[Index].bUntold)
		{
			Shown.Insert(Index, 0);
		}
	}
	TArray<FText> Lines;
	for (const int32 Index : Shown)
	{
		Lines.Add(FText::Format(INVTEXT("[{0}] {1}"), FText::AsNumber(Log[Index].Tick), Log[Index].Message));
	}
	LogText->SetText(FText::Join(INVTEXT("\n"), Lines));

	if (LogScroll)
	{
		LogScroll->ScrollToEnd();
	}
}

FReply UCosmicTextPanelWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Only a hit-testable part of the panel reaches here (the widget and its root are self-hit-test-invisible).
	return FReply::Handled();
}

FReply UCosmicTextPanelWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicLogFeedWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/ListView.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Core/GalaxyState.h"
#include "Cosmic_Ascension.h"
#include "Engine/GameInstance.h"
#include "Framework/Views/ITypedTableView.h"
#include "Layout/Margin.h"
#include "Log/CosmicLogSubsystem.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UI/CosmicPalette.h"
#include "UI/CosmicUILabels.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "World/CosmicGameMode.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicLogFeedImpl
{
	// The entry class: an empty Widget Blueprint whose parent is UCosmicLogEntryWidget (Tools/make_widget_blueprints.py) -
	// an editor build's list view takes only a Blueprint entry class. Cooked by DirectoriesToAlwaysCook.
	const TCHAR* const EntryClassPath = TEXT("/Game/CA/UI/WBP_LogEntry.WBP_LogEntry_C");
	const FLinearColor BoxColor(0.01f, 0.01f, 0.03f, 0.7f);

	// The entry class of a list view and whether it takes the keyboard focus: properties with no public setter in 5.8,
	// set before the list's Slate widget exists.
	void PrepareList(UListView& List, UClass* EntryClass)
	{
		if (FClassProperty* Entry = FindFProperty<FClassProperty>(UListViewBase::StaticClass(), TEXT("EntryWidgetClass")))
		{
			Entry->SetObjectPropertyValue_InContainer(&List, EntryClass);
		}
		if (FBoolProperty* Focusable = FindFProperty<FBoolProperty>(UListView::StaticClass(), TEXT("bIsFocusable")))
		{
			Focusable->SetPropertyValue_InContainer(&List, false);
		}
	}

	UCosmicLogItem* MakeLogItem(UObject* Outer, const FCosmicLogEntry& Entry)
	{
		UCosmicLogItem* Item = NewObject<UCosmicLogItem>(Outer);
		Item->Tick = Entry.Tick;
		Item->Message = Entry.Message;
		Item->Severity = Entry.Severity;
		Item->Category = Entry.Category;
		Item->FocusSystemID = Entry.FocusSystemID;
		Item->FocusCivID = Entry.FocusCivID;
		return Item;
	}
}

FLinearColor CosmicLogFeed::ColorOf(uint8 Severity, FGameplayTag Category)
{
	// The colours of section 8.2, shared with the map's spiritual sight (one purple).
	static const FGameplayTag Transcendent = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Transcendent")), false);
	static const FGameplayTag Claim = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Claim")), false);
	if (Category.IsValid() && (Category == Transcendent || Category == Claim))
	{
		return CosmicPalette::Purple;
	}
	return Severity >= 2 ? CosmicPalette::Gold : (Severity == 1 ? CosmicPalette::White : CosmicPalette::Grey);
}

TSharedRef<SWidget> UCosmicLogEntryWidget::RebuildWidget()
{
	// RebuildWidget can run again after the Slate widgets are released; the tree is built once.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		Text = CosmicUI::MakeText(*WidgetTree, 13);
		WidgetTree->RootWidget = Text;
	}
	return Super::RebuildWidget();
}

FText CosmicLogFeed::LineText(const UCosmicLogItem& Item, ECosmicTimeUnit Unit)
{
	return FText::Format(INVTEXT("[{0}] {1}"), CosmicUI::StampLabel(Item.Tick, Unit), Item.Message);
}

void UCosmicLogEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	ShowItem(Cast<UCosmicLogItem>(ListItemObject));
}

void UCosmicLogEntryWidget::ShowItem(const UCosmicLogItem* Item)
{
	ShownItem = Item;
	Redraw();
}

void UCosmicLogEntryWidget::Redraw()
{
	const UCosmicLogItem* Item = ShownItem.Get();
	if (Text && Item)
	{
		Text->SetText(CosmicLogFeed::LineText(*Item, ACosmicPlayerController::TimeUnitFor(*this)));
		Text->SetColorAndOpacity(FSlateColor(CosmicLogFeed::ColorOf(Item->Severity, Item->Category)));
	}
}

TSharedRef<SWidget> UCosmicLogFeedWidget::RebuildWidget()
{
	using namespace CosmicLogFeedImpl;

	// RebuildWidget can run again after the Slate widgets are released; the tree is built once.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UBorder* Box = WidgetTree->ConstructWidget<UBorder>();
		Box->SetBrushColor(BoxColor);
		WidgetTree->RootWidget = Box;
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
		Box->SetContent(Column);

		UTextBlock* Title = CosmicUI::MakeText(*WidgetTree, 14);
		Title->SetText(CosmicUI::Label(TEXT("Panel.Log")));
		Column->AddChildToVerticalBox(Title)->SetPadding(FMargin(16.f, 8.f, 16.f, 4.f));

		List = WidgetTree->ConstructWidget<UListView>();
		// A focused list would eat Space (the pause).
		UClass* EntryClass = LoadClass<UCosmicLogEntryWidget>(nullptr, EntryClassPath, nullptr, LOAD_NoWarn);
		if (!EntryClass)
		{
			UE_LOG(LogCosmic, Error, TEXT("%s is missing: run Tools/make_widget_blueprints.py"), EntryClassPath);
			EntryClass = UCosmicLogEntryWidget::StaticClass();
		}
		PrepareList(*List, EntryClass);
		List->SetSelectionMode(ESelectionMode::Single);
		List->OnItemClicked().AddUObject(this, &UCosmicLogFeedWidget::HandleItemClicked);
		UVerticalBoxSlot* ListSlot = Column->AddChildToVerticalBox(List);
		ListSlot->SetPadding(FMargin(16.f, 0.f, 16.f, 12.f));
		ListSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	return Super::RebuildWidget();
}

void UCosmicLogFeedWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGalaxySimulationSubsystem* Sim = GameInstance->GetSubsystem<UGalaxySimulationSubsystem>())
		{
			Sim->OnGalaxyGenerated.AddUniqueDynamic(this, &UCosmicLogFeedWidget::HandleGalaxyGenerated);
		}
		if (UCosmicLogSubsystem* Log = GameInstance->GetSubsystem<UCosmicLogSubsystem>())
		{
			Log->OnLogEntry.AddUniqueDynamic(this, &UCosmicLogFeedWidget::HandleLogEntry);
		}
	}
	Refresh();
}

void UCosmicLogFeedWidget::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGalaxySimulationSubsystem* Sim = GameInstance->GetSubsystem<UGalaxySimulationSubsystem>())
		{
			Sim->OnGalaxyGenerated.RemoveDynamic(this, &UCosmicLogFeedWidget::HandleGalaxyGenerated);
		}
		if (UCosmicLogSubsystem* Log = GameInstance->GetSubsystem<UCosmicLogSubsystem>())
		{
			Log->OnLogEntry.RemoveDynamic(this, &UCosmicLogFeedWidget::HandleLogEntry);
		}
	}
	Super::NativeDestruct();
}

void UCosmicLogFeedWidget::HandleGalaxyGenerated(int32 Seed)
{
	Refresh();
}

void UCosmicLogFeedWidget::HandleLogEntry(const FCosmicLogEntry& Entry)
{
	// OnLogEntry never fires for an untold entry (section 8.1); the check keeps the feed's rule in one place.
	if (!Entry.bUntold)
	{
		Append(Entry);
	}
}

void UCosmicLogFeedWidget::Refresh()
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || !List)
	{
		return;
	}
	const TArray<FCosmicLogEntry>& Log = Sim->GetState().Log;
	// The last told entries, oldest first.
	TArray<int32> Shown;
	for (int32 Index = Log.Num() - 1; Index >= 0 && Shown.Num() < CosmicLogFeed::MaxItems; --Index)
	{
		if (!Log[Index].bUntold)
		{
			Shown.Add(Index);
		}
	}
	Items.Reset(Shown.Num());
	for (int32 Back = Shown.Num() - 1; Back >= 0; --Back)
	{
		Items.Add(CosmicLogFeedImpl::MakeLogItem(this, Log[Shown[Back]]));
	}
	List->SetListItems(Items);
	List->ScrollToBottom();
}

void UCosmicLogFeedWidget::Append(const FCosmicLogEntry& Entry)
{
	if (!List)
	{
		return;
	}
	// Follow the new line only when the list showed the end (a reader who scrolled up stays where they are).
	const bool bAtEnd = Items.Num() <= 1 || List->GetScrollOffset() + List->GetDisplayedEntryWidgets().Num() + 1 >= Items.Num();
	UCosmicLogItem* Item = CosmicLogFeedImpl::MakeLogItem(this, Entry);
	Items.Add(Item);
	List->AddItem(Item);
	while (Items.Num() > CosmicLogFeed::MaxItems)
	{
		List->RemoveItem(Items[0]);
		Items.RemoveAt(0);
	}
	if (bAtEnd)
	{
		List->ScrollToBottom();
	}
}

void UCosmicLogFeedWidget::HandleItemClicked(UObject* Item)
{
	OpenItem(Cast<UCosmicLogItem>(Item));
}

void UCosmicLogFeedWidget::RedrawLines()
{
	if (!List)
	{
		return;
	}
	// Only the lines on screen have widgets; a line scrolled into view later is drawn in the new unit when it is set.
	for (UUserWidget* Shown : List->GetDisplayedEntryWidgets())
	{
		if (UCosmicLogEntryWidget* Line = Cast<UCosmicLogEntryWidget>(Shown))
		{
			Line->Redraw();
		}
	}
}

void UCosmicLogFeedWidget::OpenItem(UCosmicLogItem* Item)
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer());
	if (!Item || !Sim || !Controller)
	{
		return;
	}
	// The line's system, or the capital of its people; a line of no place goes nowhere.
	FName Target = Item->FocusSystemID;
	if (Target.IsNone() && !Item->FocusCivID.IsNone())
	{
		const FGalaxyState& State = Sim->GetState();
		const int32 CivIndex = Sim->GetStateIndex().FindCiv(State, Item->FocusCivID);
		Target = State.Civs.IsValidIndex(CivIndex) ? State.Civs[CivIndex].HomeSystemID : NAME_None;
	}
	// SelectAndFocus refuses None, a system the map does not hold, and one the viewer does not know (section 8.1: the
	// camera goes to a known place only).
	Controller->SelectAndFocus(Target);
}

FReply UCosmicLogFeedWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Only a hit-testable part of the feed reaches here (the widget itself is self-hit-test-invisible).
	return FReply::Handled();
}

FReply UCosmicLogFeedWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

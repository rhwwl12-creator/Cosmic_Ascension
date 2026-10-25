// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicChronicleWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/PanelSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Engine/GameInstance.h"
#include "Input/Reply.h"
#include "Layout/Margin.h"
#include "Log/CosmicChronicle.h"
#include "Log/CosmicLogSubsystem.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicLogFeedWidget.h"
#include "UI/CosmicPalette.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicGameMode.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicChronicleWidgetImpl
{
	// Darker than the ascension presentation's 0.6: this screen is read.
	constexpr float VeilAlpha = 0.85f;
	const FLinearColor VeilColor(0.f, 0.f, 0.02f, 1.f);
	constexpr float ColumnWidth = 760.f;
	constexpr float StampWidth = 90.f;
	const FName ToMapAction(TEXT("to_map"));
	const FName NewRunAction(TEXT("new_run"));

	UTextBlock* TextOf(UWidgetTree& Tree, const FText& Words, int32 Size, const FLinearColor& Color)
	{
		UTextBlock* Text = CosmicUI::MakeText(Tree, Size);
		Text->SetText(Words);
		Text->SetColorAndOpacity(FSlateColor(Color));
		return Text;
	}

	// The name of a people the state still holds; empty otherwise.
	FText CivNameOf(const FGalaxyState& State, const FCosmicStateIndex& Index, FName CivID)
	{
		const int32 CivIndex = CivID.IsNone() ? INDEX_NONE : Index.FindCiv(State, CivID);
		return State.Civs.IsValidIndex(CivIndex) ? State.Civs[CivIndex].CivName : FText::GetEmpty();
	}
}

TSharedRef<SWidget> UCosmicChronicleWidget::RebuildWidget()
{
	using namespace CosmicChronicleWidgetImpl;

	// RebuildWidget can run again after the Slate widgets are released; the tree is built once.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		Veil = WidgetTree->ConstructWidget<UBorder>();
		Veil->SetBrushColor(FLinearColor(VeilColor.R, VeilColor.G, VeilColor.B, 0.f));
		Veil->SetHorizontalAlignment(HAlign_Center);
		Veil->SetVerticalAlignment(VAlign_Fill);
		Veil->SetPadding(FMargin(16.f, 56.f, 16.f, 24.f));
		WidgetTree->RootWidget = Veil;

		USizeBox* Width = WidgetTree->ConstructWidget<USizeBox>();
		Width->SetWidthOverride(ColumnWidth);
		Veil->SetContent(Width);
		UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
		Width->SetContent(Column);
		Page = WidgetTree->ConstructWidget<UScrollBox>();
		Column->AddChildToVerticalBox(Page)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		// Below the scroll, apart from the epilogue's first line: the cause, the notice and the buttons (UI words).
		NoteText = TextOf(*WidgetTree, FText::GetEmpty(), 13, CosmicPalette::Grey);
		Column->AddChildToVerticalBox(NoteText)->SetPadding(FMargin(0.f, 12.f, 0.f, 2.f));
		ProvisionalText = TextOf(*WidgetTree, FText::GetEmpty(), 13, CosmicPalette::Grey);
		Column->AddChildToVerticalBox(ProvisionalText)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>();
		ButtonRow = Buttons;
		ToMapButton = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		ToMapButton->Setup(ToMapAction, 0.f, CosmicUI::Label(TEXT("Chronicle.ToMap")));
		ToMapButton->OnAction.AddUObject(this, &UCosmicChronicleWidget::HandleAction);
		Buttons->AddChildToHorizontalBox(ToMapButton)->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));
		NewRunButton = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		NewRunButton->Setup(NewRunAction, 0.f, CosmicUI::Label(TEXT("Chronicle.NewRun")));
		NewRunButton->SetToolTipText(CosmicUI::Label(TEXT("Chronicle.NewRunTip")));
		NewRunButton->OnAction.AddUObject(this, &UCosmicChronicleWidget::HandleAction);
		Buttons->AddChildToHorizontalBox(NewRunButton);
		Column->AddChildToVerticalBox(Buttons);
		SetVisibility(ESlateVisibility::Collapsed);
	}
	return Super::RebuildWidget();
}

FText UCosmicChronicleWidget::NoteOf(ECosmicProvisionalEnd Cause) const
{
	using namespace CosmicChronicleWidgetImpl;
	using CosmicUI::Label;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || Cause == ECosmicProvisionalEnd::None)
	{
		return FText::GetEmpty();
	}
	const FGalaxyState& State = Sim->GetState();
	if (Cause == ECosmicProvisionalEnd::Clock)
	{
		return FText::Format(Label(TEXT("Chronicle.Note.Clock")), FText::AsNumber(State.Tick));
	}
	// The people that climbed: of the first life that reached the rung, its ruled peoples from the last, the first the state
	// still holds on the rung or above.
	for (const FLifeRecord& Life : State.Calling.Lives)
	{
		if (static_cast<int32>(Life.PeakTier) < static_cast<int32>(CosmicEndings::EndRung))
		{
			continue;
		}
		for (int32 At = Life.RuledCivIDs.Num() - 1; At >= 0; --At)
		{
			const int32 CivIndex = Life.RuledCivIDs[At].IsNone() ? INDEX_NONE : Sim->GetStateIndex().FindCiv(State, Life.RuledCivIDs[At]);
			if (State.Civs.IsValidIndex(CivIndex) && static_cast<int32>(State.Civs[CivIndex].Tier) >= static_cast<int32>(CosmicEndings::EndRung))
			{
				FFormatNamedArguments Args;
				Args.Add(TEXT("Civ"), State.Civs[CivIndex].CivName);
				return FText::Format(Label(TEXT("Chronicle.Note.Rung")), Args);
			}
		}
		break;
	}
	return Label(TEXT("Chronicle.Note.RungNoCiv"));
}

void UCosmicChronicleWidget::BuildPage()
{
	using namespace CosmicChronicleWidgetImpl;
	using CosmicUI::Label;

	RowTexts.Reset();
	RowOpacities.Reset();
	Stamps.Reset();
	LineOpacities.Reset();
	LineRows.Reset();
	if (!Page || !WidgetTree)
	{
		return;
	}
	Page->ClearChildren();
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	const UCosmicLogSubsystem* Log = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>() : nullptr;
	if (!Sim || !Log)
	{
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	auto AddRow = [&](UWidget* Row, const FText& Words, float Opacity, const FMargin& Gap)
	{
		Row->SetRenderOpacity(0.f);
		Page->AddChild(Row);
		if (UPanelSlot* Slot = Row->Slot)
		{
			if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(Slot))
			{
				ScrollSlot->SetPadding(Gap);
			}
		}
		LineRows.Add(Row);
		RowTexts.Add(Words);
		RowOpacities.Add(Opacity);
	};
	auto AddText = [&](const FText& Words, int32 Size, const FLinearColor& Color, const FMargin& Gap)
	{
		AddRow(TextOf(*WidgetTree, Words, Size, Color), Words, 1.f, Gap);
	};

	for (const FCosmicChronicleChapter& Chapter : Log->BuildChronicle())
	{
		if (!State.Calling.Lives.IsValidIndex(Chapter.LifeAt))
		{
			continue;
		}
		const FLifeRecord& Life = State.Calling.Lives[Chapter.LifeAt];
		// "The Nth", with the vessel's name when it has one.
		const FText Nth = FText::Format(Label(TEXT("Chronicle.Life")), FText::AsNumber(Life.LifeIndex));
		const FText Name = CosmicChronicle::VesselNameOf(Life, Sim->GetTables());
		AddText(Name.IsEmpty() ? Nth : FText::Format(Label(TEXT("List.Pair")), Nth, Name), 26, CosmicPalette::Gold, FMargin(0.f, 8.f, 0.f, 2.f));
		// The facts of the life - a UI line: the peoples it ruled in order ('A -> B', a people taken again after another
		// once more, one taken again at once not twice), 'wandering' when its last reign ended (B1 - the life walks), the
		// rung, the cycles it lasted.
		FText People;
		FName Last;
		for (const FName CivID : Life.RuledCivIDs)
		{
			const FText PeopleName = CivID == Last ? FText::GetEmpty() : CivNameOf(State, Sim->GetStateIndex(), CivID);
			if (!PeopleName.IsEmpty())
			{
				People = People.IsEmpty() ? PeopleName : FText::Format(Label(TEXT("Chronicle.Then")), People, PeopleName);
				Last = CivID;
			}
		}
		People = People.IsEmpty() ? CivNameOf(State, Sim->GetStateIndex(), Life.BirthCivID) : People;
		if (!Life.ReignEndReasons.IsEmpty() && Life.ReignEndReasons.Last().IsValid())
		{
			People = CosmicUI::JoinList({ People, Label(TEXT("Chronicle.Wandering")) });
		}
		const FText Rung = FText::Format(INVTEXT("{0} {1}"), Label(TEXT("Panel.Tier")), FText::AsNumber(static_cast<int32>(Life.PeakTier) + 1));
		const FText Lasted = FText::Format(Label(TEXT("Chronicle.Stamp")), FText::AsNumber(FMath::Max<int64>(0, Chapter.ToTick - Life.BornTick)));
		AddText(CosmicUI::JoinList({ People, Rung, Lasted }), 14, CosmicPalette::Grey, FMargin(0.f, 0.f, 0.f, 10.f));
		for (const FCosmicChronicleLine& Line : Chapter.Lines)
		{
			UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
			USizeBox* StampBox = WidgetTree->ConstructWidget<USizeBox>();
			StampBox->SetWidthOverride(StampWidth);
			// The stamp is always in cycles: the chronicle reads as the book's own count (UI/CosmicUILabels.h).
			const FText Stamp = FText::Format(Label(TEXT("Chronicle.Stamp")), FText::AsNumber(Line.Entry.Tick));
			UTextBlock* StampText = TextOf(*WidgetTree, Stamp, 13, CosmicPalette::Grey);
			StampText->SetJustification(ETextJustify::Right);
			StampBox->SetContent(StampText);
			Row->AddChildToHorizontalBox(StampBox)->SetPadding(FMargin(0.f, 1.f, 10.f, 0.f));
			UTextBlock* Message = TextOf(*WidgetTree, Line.Entry.Message, 16, CosmicLogFeed::ColorOf(Line.Entry.Severity, Line.Entry.Category));
			Row->AddChildToHorizontalBox(Message)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			const float Opacity = Line.bOwn ? 1.f : CosmicChronicleScreen::WorldLineOpacity;
			AddRow(Row, Line.Entry.Message, Opacity, FMargin(0.f, 0.f, 0.f, 4.f));
			Stamps.Add(Stamp);
			LineOpacities.Add(Opacity);
		}
		if (Chapter.bMore)
		{
			AddText(Label(TEXT("Chronicle.More")), 16, CosmicPalette::Grey, FMargin(0.f, 4.f, 0.f, 0.f));
		}
	}
	// (The two books - B5.) The epilogue's two fixed lines and the horizon line: the same whatever ended the run.
	AddText(Label(TEXT("Chronicle.Opened")), 22, CosmicPalette::White, FMargin(0.f, 24.f, 0.f, 4.f));
	AddText(Label(TEXT("Chronicle.Unopened")), 18, CosmicPalette::Grey, FMargin(0.f, 0.f, 0.f, 18.f));
	AddText(Label(TEXT("Chronicle.Horizon")), 18, CosmicPalette::White, FMargin(0.f, 0.f, 0.f, 8.f));
	Page->ScrollToStart();
}

void UCosmicChronicleWidget::Open(ECosmicProvisionalEnd Cause, bool bAtOnce)
{
	using CosmicUI::Label;

	if (!WidgetTree || !WidgetTree->RootWidget)
	{
		TakeWidget();
	}
	ShownCause = Cause;
	BuildPage();
	const FText Note = NoteOf(Cause);
	if (NoteText)
	{
		NoteText->SetText(Note);
		NoteText->SetVisibility(Note.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	}
	if (ProvisionalText)
	{
		// A preview is no end: no notice.
		const bool bEnd = Cause != ECosmicProvisionalEnd::None;
		ProvisionalText->SetText(bEnd ? Label(TEXT("Chronicle.Provisional")) : FText::GetEmpty());
		ProvisionalText->SetVisibility(bEnd ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (NewRunButton)
	{
		NewRunButton->Setup(TEXT("new_run"), 0.f, Label(TEXT("Chronicle.NewRun")));
		NewRunButton->SetVisibility(Cause == ECosmicProvisionalEnd::None ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	ConfirmLeft.Reset();
	bOpen = true;
	bRevealed = false;
	Elapsed = 0.f;
	SetVisibility(ESlateVisibility::Visible);
	if (bAtOnce)
	{
		PressVeil();
		return;
	}
	ShowProgress();
}

void UCosmicChronicleWidget::Close()
{
	bOpen = false;
	bRevealed = false;
	ConfirmLeft.Reset();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCosmicChronicleWidget::ShowProgress()
{
	using namespace CosmicChronicleWidgetImpl;

	const float VeilShare = bRevealed ? 1.f : FMath::Clamp(Elapsed / CosmicChronicleScreen::VeilSeconds, 0.f, 1.f);
	if (Veil)
	{
		Veil->SetBrushColor(FLinearColor(VeilColor.R, VeilColor.G, VeilColor.B, VeilAlpha * VeilShare));
	}
	const int32 Rows = LineRows.Num();
	const float Interval = Rows > 0 ? FMath::Min(CosmicChronicleScreen::LineSeconds, CosmicChronicleScreen::RevealCapSeconds / static_cast<float>(Rows)) : 0.f;
	const float SinceVeil = Elapsed - CosmicChronicleScreen::VeilSeconds;
	int32 Shown = Rows;
	if (!bRevealed)
	{
		Shown = SinceVeil < 0.f ? 0 : (Interval > 0.f ? FMath::Min(Rows, static_cast<int32>(SinceVeil / Interval) + 1) : Rows);
		bRevealed = Shown >= Rows && SinceVeil >= Interval * static_cast<float>(Rows);
	}
	for (int32 At = 0; At < Rows; ++At)
	{
		if (UWidget* Row = LineRows[At])
		{
			Row->SetRenderOpacity((bRevealed || At < Shown) ? RowOpacities[At] : 0.f);
		}
	}
	// The buttons only once every line shows: a click meant for the page never reaches 'new run'.
	if (ButtonRow)
	{
		ButtonRow->SetVisibility(bRevealed ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
	}
}

void UCosmicChronicleWidget::Advance(float DeltaSeconds)
{
	if (!bOpen)
	{
		return;
	}
	const float Step = FMath::Max(DeltaSeconds, 0.f);
	if (ConfirmLeft.IsSet())
	{
		ConfirmLeft = ConfirmLeft.GetValue() - Step;
		if (ConfirmLeft.GetValue() <= 0.f)
		{
			ConfirmLeft.Reset();
			if (NewRunButton)
			{
				NewRunButton->Setup(TEXT("new_run"), 0.f, CosmicUI::Label(TEXT("Chronicle.NewRun")));
			}
		}
	}
	if (!bRevealed)
	{
		Elapsed += Step;
		ShowProgress();
	}
}

void UCosmicChronicleWidget::PressVeil()
{
	if (!bOpen)
	{
		return;
	}
	bRevealed = true;
	ShowProgress();
}

void UCosmicChronicleWidget::PressToMap()
{
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->LookAtMap();
	}
	else
	{
		Close();
	}
}

void UCosmicChronicleWidget::PressNewRun()
{
	if (!bOpen || !bRevealed || ShownCause == ECosmicProvisionalEnd::None)
	{
		return;
	}
	if (!ConfirmLeft.IsSet())
	{
		// The run's save stays and the continue opens the new run from now on (MVP item 5): a second press says so.
		ConfirmLeft = CosmicChronicleScreen::ConfirmSeconds;
		if (NewRunButton)
		{
			NewRunButton->Setup(TEXT("new_run"), 0.f, CosmicUI::Label(TEXT("Chronicle.NewRunConfirm")));
		}
		return;
	}
	ConfirmLeft.Reset();
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->StartNewRun();
	}
}

void UCosmicChronicleWidget::HandleAction(FName Action, float Value)
{
	if (Action == CosmicChronicleWidgetImpl::ToMapAction)
	{
		PressToMap();
	}
	else if (Action == CosmicChronicleWidgetImpl::NewRunAction)
	{
		PressNewRun();
	}
}

TArray<FText> UCosmicChronicleWidget::GetPageTexts() const
{
	return RowTexts;
}

TArray<FText> UCosmicChronicleWidget::GetLineStamps() const
{
	return Stamps;
}

TArray<float> UCosmicChronicleWidget::GetLineOpacities() const
{
	return LineOpacities;
}

void UCosmicChronicleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Advance(InDeltaTime);
}

FReply UCosmicChronicleWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (IsRevealing())
	{
		PressVeil();
	}
	return FReply::Handled();
}

FReply UCosmicChronicleWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

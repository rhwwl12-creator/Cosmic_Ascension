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
#include "Core/CosmicTags.h"
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
	// A chapter's head: a life chapter's ('the Nth - name'), and the two books' chapter's (B5 - its first part's head).
	constexpr int32 ChapterHeadSize = 26;
	const FMargin ChapterHeadGap(0.f, 8.f, 0.f, 2.f);
	// The two books (B5): part two's head under the chapter's, and the one colour of all their lines - the colour a chapter
	// gives its Severity 1 lines (CosmicLogFeed::ColorOf), so a first-person line is no other colour than the record under
	// which it stands.
	constexpr int32 BookHeadSize = 18;
	const FLinearColor BookLineColor = CosmicPalette::White;

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
	// Rung is no longer produced (the third rung does not end a run - B6a [B-6a3a]): the line that names no people. [B-6a3c]
	// names the people again, from the ending record's CivID.
	return Label(TEXT("Chronicle.Note.RungNoCiv"));
}

void UCosmicChronicleWidget::BuildPage()
{
	using namespace CosmicChronicleWidgetImpl;
	using CosmicUI::Label;

	RowTexts.Reset();
	RowOpacities.Reset();
	RowLooks.Reset();
	Stamps.Reset();
	LineOpacities.Reset();
	LineColors.Reset();
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
	auto AddRow = [&](UWidget* Row, const FText& Words, float Opacity, const FMargin& Gap, int32 Size, const FLinearColor& Color)
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
		RowLooks.Add({ Size, Color, Gap });
	};
	auto AddText = [&](const FText& Words, int32 Size, const FLinearColor& Color, const FMargin& Gap)
	{
		AddRow(TextOf(*WidgetTree, Words, Size, Color), Words, 1.f, Gap, Size, Color);
	};
	// The lines of a body or of a span with no vessel: the stamp, always in cycles (the chronicle reads as the book's own
	// count - UI/CosmicUILabels.h), the message in the feed's colour, and two opacities. The two books give one colour for
	// all their lines (OneColor).
	auto AddLines = [&](const TArray<FCosmicChronicleLine>& Lines, const FLinearColor* OneColor)
	{
		for (const FCosmicChronicleLine& Line : Lines)
		{
			UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
			USizeBox* StampBox = WidgetTree->ConstructWidget<USizeBox>();
			StampBox->SetWidthOverride(StampWidth);
			const FText Stamp = FText::Format(Label(TEXT("Chronicle.Stamp")), FText::AsNumber(Line.Entry.Tick));
			UTextBlock* StampText = TextOf(*WidgetTree, Stamp, 13, CosmicPalette::Grey);
			StampText->SetJustification(ETextJustify::Right);
			StampBox->SetContent(StampText);
			Row->AddChildToHorizontalBox(StampBox)->SetPadding(FMargin(0.f, 1.f, 10.f, 0.f));
			const FLinearColor Color = OneColor ? *OneColor : CosmicLogFeed::ColorOf(Line.Entry.Severity, Line.Entry.Category);
			UTextBlock* Message = TextOf(*WidgetTree, Line.Entry.Message, 16, Color);
			Row->AddChildToHorizontalBox(Message)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			const float Opacity = Line.bOwn ? 1.f : CosmicChronicleScreen::WorldLineOpacity;
			AddRow(Row, Line.Entry.Message, Opacity, FMargin(0.f, 0.f, 0.f, 4.f), 16, Color);
			Stamps.Add(Stamp);
			LineOpacities.Add(Opacity);
			LineColors.Add(Color);
		}
	};
	// A span with no vessel (B5 header section 2.7, 4 - decision 14): its sub-head over its lines, in the chapter's own
	// grey and size, no page of its own; nothing for a span without a line (a leap of 0, a quiet span).
	auto AddSpan = [&](const FCosmicChronicleNoVessel& Span)
	{
		if (Span.Lines.IsEmpty() && !Span.bMore)
		{
			return;
		}
		AddText(Label(TEXT("Chronicle.NoVessel")), 14, CosmicPalette::Grey, FMargin(0.f, 6.f, 0.f, 4.f));
		AddLines(Span.Lines, nullptr);
		if (Span.bMore)
		{
			AddText(Label(TEXT("Chronicle.More")), 16, CosmicPalette::Grey, FMargin(0.f, 4.f, 0.f, 0.f));
		}
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
		AddText(Name.IsEmpty() ? Nth : FText::Format(Label(TEXT("List.Pair")), Nth, Name), ChapterHeadSize, CosmicPalette::Gold, ChapterHeadGap);
		// The facts of the life - a UI line: the peoples it ruled in order ('A -> B', a people taken again after another
		// once more, one taken again at once not twice), 'wandering' when its last reign ended and it walked on (B1), the
		// rung, the cycles it lasted. A life that never ruled (B5 - a called vessel) says where it was born and that it
		// walked: the birth people's name alone would read as a reign.
		FText People;
		if (Life.RuledCivIDs.IsEmpty())
		{
			const FText Birth = CivNameOf(State, Sim->GetStateIndex(), Life.BirthCivID);
			FFormatNamedArguments Args;
			Args.Add(TEXT("Civ"), Birth);
			People = CosmicUI::JoinList({ Birth.IsEmpty() ? FText::GetEmpty() : FText::Format(Label(TEXT("Chronicle.BornAmong")), Args),
				Label(TEXT("Chronicle.Wandering")) });
		}
		else
		{
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
			// Not when the reign ended with the life (Reign.Died - B5): a ruler that died on its throne never walked.
			if (!Life.ReignEndReasons.IsEmpty() && Life.ReignEndReasons.Last().IsValid() && Life.ReignEndReasons.Last() != CosmicTags::ReignDied())
			{
				People = CosmicUI::JoinList({ People, Label(TEXT("Chronicle.Wandering")) });
			}
		}
		const FText Rung = FText::Format(INVTEXT("{0} {1}"), Label(TEXT("Panel.Tier")), FText::AsNumber(static_cast<int32>(Life.PeakTier) + 1));
		const FText Lasted = FText::Format(Label(TEXT("Chronicle.Stamp")), FText::AsNumber(FMath::Max<int64>(0, Chapter.ToTick - Life.BornTick)));
		AddText(CosmicUI::JoinList({ People, Rung, Lasted }), 14, CosmicPalette::Grey, FMargin(0.f, 0.f, 0.f, 10.f));
		// The span with no vessel that led to this life, at the head of its chapter; then the body; then, on the last chapter,
		// the span after its death when no shown life follows.
		AddSpan(Chapter.Before);
		AddLines(Chapter.Lines, nullptr);
		if (Chapter.bMore)
		{
			AddText(Label(TEXT("Chronicle.More")), 16, CosmicPalette::Grey, FMargin(0.f, 4.f, 0.f, 0.f));
		}
		AddSpan(Chapter.After);
	}
	// The two books (B5 header section 4, decisions 10 and 11): after the life chapters, before the fixed lines - one chapter,
	// headed as a life chapter is by part one's head (so it never reads as a part of the last life's chapter, which may end
	// with a death and the span with no vessel), then part two under it as its part (part one holds part two - decision
	// 10-ga), never side by side, no number but the stamps. Their lines are a chapter's lines (the stamp in
	// cycles, the same size, opacity 1) in one colour, the first-person lines too: purple is the feed's alone, and two colours
	// here would read as two camps (B4 header). Part one holds part two (decision 10-ga): it stands whenever the chapter
	// does; part two only over a line; a claimant's head only with its Title. No chapter when both parts are empty, and no
	// closing line of their own ('the other book' is Chronicle.Unopened, once - decision 10-da).
	const FCosmicTwoBooks Books = CosmicChronicle::BuildTwoBooks(State);
	if (!Books.IsEmpty())
	{
		AddText(Label(TEXT("Chronicle.BookRecorded")), ChapterHeadSize, CosmicPalette::Gold, ChapterHeadGap);
		AddLines(Books.Recorded, &BookLineColor);
		if (Books.bRecordedMore)
		{
			AddText(Label(TEXT("Chronicle.More")), 16, CosmicPalette::Grey, FMargin(0.f, 4.f, 0.f, 0.f));
		}
		if (!Books.Claimed.IsEmpty())
		{
			AddText(Label(TEXT("Chronicle.BookClaimed")), BookHeadSize, CosmicPalette::Grey, FMargin(0.f, 14.f, 0.f, 6.f));
			for (const FCosmicClaimedGroup& Group : Books.Claimed)
			{
				const FText Claimant = CosmicChronicle::ClaimantTitleOf(Group.TranscendentID, Sim->GetTables());
				if (!Claimant.IsEmpty())
				{
					FFormatNamedArguments Args;
					Args.Add(TEXT("Transcendent"), Claimant);
					AddText(FText::Format(Label(TEXT("Chronicle.ClaimedBy")), Args), 14, CosmicPalette::Grey, FMargin(0.f, 6.f, 0.f, 4.f));
				}
				AddLines(Group.Lines, &BookLineColor);
			}
			if (Books.bClaimedMore)
			{
				AddText(Label(TEXT("Chronicle.More")), 16, CosmicPalette::Grey, FMargin(0.f, 4.f, 0.f, 0.f));
			}
		}
	}
	// The epilogue's two fixed lines and the horizon line: the same whatever ended the run.
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

TArray<FLinearColor> UCosmicChronicleWidget::GetLineColors() const
{
	return LineColors;
}

TArray<CosmicChronicleScreen::FRowLook> UCosmicChronicleWidget::GetRowLooks() const
{
	return RowLooks;
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

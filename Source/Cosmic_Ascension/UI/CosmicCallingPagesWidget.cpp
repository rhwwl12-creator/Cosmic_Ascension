// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicCallingPagesWidget.h"

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
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "Input/Reply.h"
#include "Internationalization/Text.h"
#include "Layout/Margin.h"
#include "Log/CosmicChronicle.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicChronicleWidget.h"
#include "UI/CosmicPalette.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicDigest.h"
#include "World/CosmicGameMode.h"

#include <initializer_list>

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicCallingPagesImpl
{
	// The chronicle's veil (UI/CosmicChronicleWidget.cpp): these pages are read the same way.
	constexpr float VeilAlpha = 0.85f;
	const FLinearColor VeilColor(0.f, 0.f, 0.02f, 1.f);
	constexpr float ColumnWidth = 760.f;
	const FName NextAction(TEXT("calling_next"));
	const FName SkipAction(TEXT("calling_skip"));

	// The label of a cause of death: a hazard, an encounter, the measure's age (-DeathAt borrows Death.Age - header section 5.1);
	// any other cause (Death.Transcendent is written by no row - header section 3.5) is not named.
	const TCHAR* CauseKeyOf(const FGameplayTag& Cause)
	{
		if (Cause.IsValid() && Cause.MatchesTagExact(CosmicTags::DeathHazard()))
		{
			return TEXT("Epitaph.Cause.Hazard");
		}
		if (Cause.IsValid() && Cause.MatchesTagExact(CosmicTags::DeathEncounter()))
		{
			return TEXT("Epitaph.Cause.Encounter");
		}
		if (Cause.IsValid() && Cause.MatchesTagExact(CosmicTags::DeathAge()))
		{
			return TEXT("Epitaph.Cause.Age");
		}
		return TEXT("Epitaph.Cause.Other");
	}

	const FStarSystemState* SystemOf(const FGalaxyState& State, FName SystemID)
	{
		return SystemID.IsNone() ? nullptr : State.Systems.FindByPredicate([SystemID](const FStarSystemState& Each) { return Each.SystemID == SystemID; });
	}

	const FCivilizationState* CivOf(const FGalaxyState& State, FName CivID)
	{
		return CivID.IsNone() ? nullptr : State.Civs.FindByPredicate([CivID](const FCivilizationState& Each) { return Each.CivID == CivID; });
	}

	// A text with no slot as it stands; one with slots is no plain name (a source or a feat title that would need arguments).
	bool HasNoSlot(const FText& Text)
	{
		TArray<FString> Slots;
		FTextFormat(Text).GetFormatArgumentNames(Slots);
		return Slots.IsEmpty();
	}

	// One stored line of the life's answers, with the place it holds in the two books (the chronicle first, then the log).
	struct FAnswerLine
	{
		int64 Tick = 0;
		int32 Order = 0;
		const FCosmicLogEntry* Entry = nullptr;
	};

	// A card the epitaph never names among the last answers: its title speaks to the reader or exalts the vessel
	// (CosmicRows::EpitaphLeavesOut - decision 12-da: no 'you' across a death; header section 4: nothing rises at a death).
	bool IsLeftOut(FName Row)
	{
		for (const FName& Each : CosmicRows::EpitaphLeavesOut)
		{
			if (Each == Row)
			{
				return true;
			}
		}
		return false;
	}

	// The hazard a card belongs to: the first hazard of the tables one of its conditions names (CosmicCallingPages::
	// IsCardOfHazard - enc_star_watch's Hazard.PreSupernova); null for a card of no hazard.
	const FHazardDef* HazardOfCard(const FEncounterDef& Card, const FCosmicDataTables& Tables)
	{
		for (const FCosmicCondition& Condition : Card.Conditions)
		{
			const FHazardDef* Hazard = Condition.Tag.IsValid() ? Tables.FindHazardByTag(Condition.Tag) : nullptr;
			if (Hazard && CosmicCallingPages::IsCardOfHazard(Card, *Hazard))
			{
				return Hazard;
			}
		}
		return nullptr;
	}
}

bool CosmicCallingPages::IsCardOfHazard(const FEncounterDef& Card, const FHazardDef& Hazard)
{
	if (!Hazard.HazardTag.IsValid())
	{
		return false;
	}
	for (const FCosmicCondition& Condition : Card.Conditions)
	{
		if (Condition.Tag.IsValid() && Condition.Tag.MatchesTagExact(Hazard.HazardTag))
		{
			return true;
		}
	}
	return false;
}

TArray<FCosmicAnswerTitle> CosmicCallingPages::LastAnswersOf(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 LifeAt)
{
	using namespace CosmicCallingPagesImpl;

	TArray<FCosmicAnswerTitle> Titles;
	if (!State.Calling.Lives.IsValidIndex(LifeAt))
	{
		return Titles;
	}
	const FLifeRecord& Life = State.Calling.Lives[LifeAt];
	const TArray<FName>& Rows = Life.LastChoiceRows;
	if (Rows.IsEmpty())
	{
		return Titles;
	}
	// The stored lines of the life whose source is one of its answered cards, oldest first (a line moved out of the log into the
	// chronicle is older than the log's; the order within a tick is the books' own).
	const int64 From = Life.BornTick;
	const int64 To = Life.DiedTick > 0 ? Life.DiedTick : State.Tick;
	TArray<FAnswerLine> Lines;
	int32 Order = 0;
	for (const TArray<FCosmicLogEntry>* Book : { &State.Chronicle, &State.Log })
	{
		for (const FCosmicLogEntry& Entry : *Book)
		{
			++Order;
			if (Entry.Tick >= From && Entry.Tick <= To && Rows.Contains(Entry.SourceRow))
			{
				FAnswerLine& Line = Lines.AddDefaulted_GetRef();
				Line.Tick = Entry.Tick;
				Line.Order = Order;
				Line.Entry = &Entry;
			}
		}
	}
	Lines.Sort([](const FAnswerLine& A, const FAnswerLine& B) { return A.Tick != B.Tick ? A.Tick < B.Tick : A.Order < B.Order; });
	// Each answer with its own line, from the newest back: the newest line of its row not taken by a later answer.
	TArray<const FCosmicLogEntry*> LineOf;
	LineOf.Init(nullptr, Rows.Num());
	int32 Cursor = Lines.Num() - 1;
	for (int32 At = Rows.Num() - 1; At >= 0; --At)
	{
		for (int32 Look = Cursor; Look >= 0; --Look)
		{
			if (Lines[Look].Entry->SourceRow == Rows[At])
			{
				LineOf[At] = Lines[Look].Entry;
				Cursor = Look - 1;
				break;
			}
		}
	}
	// The hazard that closed the life, and where - the dead vessel's place while the calling waits in the interregnum after it.
	const FHazardDef* Killer = Life.CauseOfDeath.IsValid() && Life.CauseOfDeath.MatchesTagExact(CosmicTags::DeathHazard()) ? Tables.FindHazard(Life.CauseSourceRow) : nullptr;
	const bool bPlaceKnown = State.Calling.LifeState == ELifeState::Interregnum && LifeAt == State.Calling.Lives.Num() - 1;
	const FName DeathPlace = bPlaceKnown ? State.Player.CurrentSystemID : NAME_None;
	for (int32 At = 0; At < Rows.Num(); ++At)
	{
		const FEncounterDef* Card = Tables.FindEncounter(Rows[At]);
		if (!Card || Card->Title.IsEmpty() || IsLeftOut(Rows[At]))
		{
			continue;
		}
		const FCosmicLogEntry* Line = LineOf[At];
		TArray<FString> Slots;
		FTextFormat(Card->Title).GetFormatArgumentNames(Slots);
		FFormatNamedArguments Args;
		bool bFilled = true;
		for (const FString& SlotName : Slots)
		{
			if (SlotName.Equals(TEXT("System"), ESearchCase::CaseSensitive))
			{
				const FStarSystemState* System = Line ? SystemOf(State, Line->FocusSystemID) : nullptr;
				bFilled &= System != nullptr;
				if (System)
				{
					Args.Add(TEXT("System"), System->DisplayName);
				}
			}
			else if (SlotName.Equals(TEXT("Civ"), ESearchCase::CaseSensitive))
			{
				const FCivilizationState* Civ = Line ? CivOf(State, Line->FocusCivID) : nullptr;
				bFilled &= Civ != nullptr;
				if (Civ)
				{
					Args.Add(TEXT("Civ"), Civ->CivName);
				}
			}
			else
			{
				// {OtherCiv} (the first contact card's other people), {Hazard}, {Rebel}: the line does not say them.
				bFilled = false;
			}
		}
		if (!bFilled)
		{
			continue;
		}
		FCosmicAnswerTitle& Title = Titles.AddDefaulted_GetRef();
		Title.Row = Rows[At];
		Title.Title = Slots.IsEmpty() ? Card->Title : FText::Format(Card->Title, Args);
		Title.bOfTheDeath = At == Rows.Num() - 1 && Killer && IsCardOfHazard(*Card, *Killer)
			&& (DeathPlace.IsNone() || !Line || Line->FocusSystemID.IsNone() || Line->FocusSystemID == DeathPlace);
	}
	return Titles;
}

TSharedRef<SWidget> UCosmicCallingPagesWidget::RebuildWidget()
{
	using namespace CosmicCallingPagesImpl;

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
		UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>();
		Width->SetContent(Stack);
		Column = WidgetTree->ConstructWidget<UScrollBox>();
		Stack->AddChildToVerticalBox(Column)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		// Below the column: 'next' (or 'to the map') and the epitaph's 'skip'.
		UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>();
		ButtonRow = Buttons;
		NextButton = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		NextButton->Setup(NextAction, 0.f, FText::GetEmpty());
		NextButton->OnAction.AddUObject(this, &UCosmicCallingPagesWidget::HandleAction);
		Buttons->AddChildToHorizontalBox(NextButton)->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));
		SkipButton = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		SkipButton->Setup(SkipAction, 0.f, FText::GetEmpty());
		SkipButton->OnAction.AddUObject(this, &UCosmicCallingPagesWidget::HandleAction);
		SkipButton->SetVisibility(ESlateVisibility::Collapsed);
		Buttons->AddChildToHorizontalBox(SkipButton);
		Stack->AddChildToVerticalBox(Buttons)->SetPadding(FMargin(0.f, 12.f, 0.f, 0.f));
		SetVisibility(ESlateVisibility::Collapsed);
	}
	return Super::RebuildWidget();
}

FText UCosmicCallingPagesWidget::LabelOf(const TCHAR* Key)
{
	Keys.Add(Key);
	return CosmicUI::Label(Key);
}

void UCosmicCallingPagesWidget::AddRow(const FText& Words, int32 Size, const FLinearColor& Color, float GapAbove)
{
	if (!Column || !WidgetTree)
	{
		return;
	}
	UTextBlock* Text = CosmicUI::MakeText(*WidgetTree, Size);
	Text->SetText(Words);
	Text->SetColorAndOpacity(FSlateColor(Color));
	Text->SetRenderOpacity(0.f);
	Column->AddChild(Text);
	if (UScrollBoxSlot* RowSlot = Cast<UScrollBoxSlot>(Text->Slot))
	{
		RowSlot->SetPadding(FMargin(0.f, GapAbove, 0.f, 2.f));
	}
	LineRows.Add(Text);
	RowTexts.Add(Words);
}

void UCosmicCallingPagesWidget::Begin(ECosmicCallingPage Page, int32 InLifeAt)
{
	if (!WidgetTree || !WidgetTree->RootWidget)
	{
		TakeWidget();
	}
	// A page after another keeps the veil up: the map does not show through between the pages.
	const bool bVeilUp = IsOpen();
	Shown = Page;
	LifeAt = InLifeAt;
	RowTexts.Reset();
	Keys.Reset();
	Answers.Reset();
	LineRows.Reset();
	ApartRow = INDEX_NONE;
	CauseRow = INDEX_NONE;
	FactsEndRow = INDEX_NONE;
	bSkipShown = false;
	bRevealed = false;
	Elapsed = bVeilUp ? CosmicChronicleScreen::VeilSeconds : 0.f;
	if (Column)
	{
		Column->ClearChildren();
	}
}

void UCosmicCallingPagesWidget::Finish(const TCHAR* NextKey, bool bSkippable, bool bAtOnce)
{
	using namespace CosmicCallingPagesImpl;

	if (NextButton)
	{
		NextButton->Setup(NextAction, 0.f, LabelOf(NextKey));
	}
	bSkipShown = bSkippable;
	if (SkipButton)
	{
		if (bSkippable)
		{
			SkipButton->Setup(SkipAction, 0.f, LabelOf(TEXT("Epitaph.Skip")));
			SkipButton->SetToolTipText(LabelOf(TEXT("Epitaph.SkipTip")));
		}
		SkipButton->SetVisibility(bSkippable ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (Column)
	{
		Column->ScrollToStart();
	}
	SetVisibility(ESlateVisibility::Visible);
	bRevealed = bAtOnce;
	ShowProgress();
}

void UCosmicCallingPagesWidget::OpenEpitaph(int32 InLifeAt, bool bSkippable, bool bAtOnce)
{
	using namespace CosmicCallingPagesImpl;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || !Sim->GetState().Calling.Lives.IsValidIndex(InLifeAt))
	{
		return;
	}
	Begin(ECosmicCallingPage::Epitaph, InLifeAt);
	const FGalaxyState& State = Sim->GetState();
	const FCosmicDataTables& Tables = Sim->GetTables();
	const FLifeRecord& Life = State.Calling.Lives[InLifeAt];

	// The title: 'the Nth' with the vessel's name - the chronicle's chapter head, the name in the third person (decision 12-da).
	const FText Nth = FText::Format(LabelOf(TEXT("Chronicle.Life")), FText::AsNumber(Life.LifeIndex));
	const FText Name = CosmicChronicle::VesselNameOf(Life, Tables);
	AddRow(Name.IsEmpty() ? Nth : FText::Format(LabelOf(TEXT("List.Pair")), Nth, Name), 24, CosmicPalette::White, 8.f);

	// The facts, a line each: the cause with its source (a hazard's name - a UI label of the tables), the cycles lived, the highest
	// rung the people reached while the vessel ruled (none for the first rung - B6b decision 30: the peak is the people's
	// climb, PeakTier rising only with it), the feats. No epithet: nothing names the era yet (EpithetRow stays empty). A cause whose source is a card (a
	// Death.Encounter) names the hazard the card belongs to, or nothing - never the card's title: the vessel's answer and its
	// death never share a line (header section 3.6 1, 4).
	FText Source;
	if (const FHazardDef* Hazard = Tables.FindHazard(Life.CauseSourceRow))
	{
		Source = Hazard->DisplayName;
	}
	else if (const FEncounterDef* Card = Tables.FindEncounter(Life.CauseSourceRow))
	{
		if (const FHazardDef* CardHazard = HazardOfCard(*Card, Tables))
		{
			Source = CardHazard->DisplayName;
		}
	}
	FFormatNamedArguments CauseArgs;
	CauseArgs.Add(TEXT("Cause"), LabelOf(CauseKeyOf(Life.CauseOfDeath)));
	CauseArgs.Add(TEXT("Source"), Source);
	CauseRow = RowTexts.Num();
	AddRow(FText::Format(LabelOf(Source.IsEmpty() ? TEXT("Epitaph.Cause") : TEXT("Epitaph.CauseFrom")), CauseArgs), 16, CosmicPalette::White, 14.f);
	AddRow(FText::Format(LabelOf(TEXT("Epitaph.Lived")), FText::AsNumber(FMath::Max<int64>(0, Life.DiedTick - Life.BornTick))), 14, CosmicPalette::Grey, 2.f);
	if (Life.PeakTier != EAscensionTier::Tier1_Planetary)
	{
		AddRow(FText::Format(LabelOf(TEXT("Epitaph.Peak")), FText::AsNumber(static_cast<int32>(Life.PeakTier) + 1)), 14, CosmicPalette::Grey, 2.f);
	}
	TArray<FText> FeatTitles;
	for (const FName Feat : Life.Feats)
	{
		const FAscensionFeatDef* Def = Tables.FindFeat(Feat);
		if (Def && !Def->Title.IsEmpty() && HasNoSlot(Def->Title))
		{
			FeatTitles.Add(Def->Title);
		}
	}
	if (!FeatTitles.IsEmpty())
	{
		AddRow(FText::Format(LabelOf(TEXT("Epitaph.Feats")), CosmicUI::JoinList(FeatTitles)), 14, CosmicPalette::Grey, 2.f);
	}
	FactsEndRow = RowTexts.Num() - 1;

	// The last answers, below the facts and never in a sentence with the cause; their titles alone, in time order. The answer on
	// the card of the very hazard that closed the life stands apart below the others (header section 3.6 1).
	Answers = CosmicCallingPages::LastAnswersOf(State, Tables, InLifeAt);
	if (!Answers.IsEmpty())
	{
		AddRow(LabelOf(TEXT("Epitaph.Answers")), 14, CosmicPalette::Grey, 18.f);
		for (const FCosmicAnswerTitle& Answer : Answers)
		{
			if (!Answer.bOfTheDeath)
			{
				AddRow(Answer.Title, 16, CosmicPalette::White, 2.f);
			}
		}
		for (const FCosmicAnswerTitle& Answer : Answers)
		{
			if (Answer.bOfTheDeath)
			{
				ApartRow = RowTexts.Num();
				AddRow(Answer.Title, 16, CosmicPalette::White, 22.f);
			}
		}
	}
	Finish(TEXT("Epitaph.Next"), bSkippable, bAtOnce);
}

void UCosmicCallingPagesWidget::OpenDigest()
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	const int32 Closed = Sim ? CosmicDigest::LastClosedLifeOf(Sim->GetState()) : INDEX_NONE;
	Begin(ECosmicCallingPage::Digest, Closed);
	// The digest's lines as World/CosmicDigest renders them (header section 2.7 - counted when shown; Knowing read through
	// KnownViewOf there, with the controller's see-all): DigestOf then LinesOf, which is LinesOf(Sim, Closed). Its head is the
	// page's title; a leap of 0 has its one line only.
	const ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer());
	if (Sim && Closed != INDEX_NONE)
	{
		const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, Closed, Controller && Controller->IsSeeingAll());
		const TArray<FText> Lines = CosmicDigest::LinesOf(Digest);
		for (int32 At = 0; At < Lines.Num(); ++At)
		{
			const bool bHead = At == 0 && !Digest.bNoLeap && Lines.Num() > 1;
			AddRow(Lines[At], bHead ? 24 : 16, CosmicPalette::White, bHead ? 8.f : 6.f);
		}
	}
	Finish(TEXT("Epitaph.Next"), false, false);
}

void UCosmicCallingPagesWidget::OpenInauguration(int32 InLifeAt)
{
	using namespace CosmicCallingPagesImpl;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || !Sim->GetState().Calling.Lives.IsValidIndex(InLifeAt))
	{
		return;
	}
	Begin(ECosmicCallingPage::Inauguration, InLifeAt);
	const FGalaxyState& State = Sim->GetState();
	const FLifeRecord& Life = State.Calling.Lives[InLifeAt];
	AddRow(FText::Format(LabelOf(TEXT("Chronicle.Life")), FText::AsNumber(Life.LifeIndex)), 24, CosmicPalette::White, 8.f);
	// The new vessel's facts (decision 12-da: its name, in the third person - 'you' comes back with its first act): the people it
	// was born among, its system, its body and mind. The stats are the vessel's while it is the embodied last life.
	const FCivilizationState* Civ = CivOf(State, Life.BirthCivID);
	const FStarSystemState* System = SystemOf(State, Life.BirthSystemID);
	const bool bEmbodied = State.Calling.LifeState == ELifeState::Embodied && InLifeAt == State.Calling.Lives.Num() - 1;
	const FText Body = bEmbodied ? FText::Format(INVTEXT("{0} {1}"), LabelOf(TEXT("Petition.Stat.Body")), FText::AsNumber(State.Player.Body)) : FText::GetEmpty();
	const FText Mind = bEmbodied ? FText::Format(INVTEXT("{0} {1}"), LabelOf(TEXT("Petition.Stat.Mind")), FText::AsNumber(State.Player.Mind)) : FText::GetEmpty();
	// The people it was born among in the chronicle's words for a life (Chronicle.BornAmong).
	FText Born;
	if (Civ)
	{
		FFormatNamedArguments BornArgs;
		BornArgs.Add(TEXT("Civ"), Civ->CivName);
		Born = FText::Format(LabelOf(TEXT("Chronicle.BornAmong")), BornArgs);
	}
	AddRow(CosmicUI::JoinList({ CosmicChronicle::VesselNameOf(Life, Sim->GetTables()), Born, System ? System->DisplayName : FText::GetEmpty(), Body, Mind }),
		14, CosmicPalette::Grey, 2.f);
	// The fixed line of the succession screen (decision 12-na).
	AddRow(LabelOf(TEXT("Succession.Kept")), 18, CosmicPalette::White, 18.f);
	Finish(TEXT("Succession.ToMap"), false, false);
}

void UCosmicCallingPagesWidget::OpenReignEnd(int32 InLifeAt)
{
	using namespace CosmicCallingPagesImpl;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || !Sim->GetState().Calling.Lives.IsValidIndex(InLifeAt))
	{
		return;
	}
	Begin(ECosmicCallingPage::ReignEnd, InLifeAt);
	const FGalaxyState& State = Sim->GetState();
	const FLifeRecord& Life = State.Calling.Lives[InLifeAt];
	// The vessel lives on: no 'the Nth', no cause, no cycles lived (DiedTick is 0) - the reign's people, its cycles and why it
	// ended (header section 4: a living vessel through the form of a death would be the return's picture).
	const int32 Reign = Life.RuledCivIDs.Num() - 1;
	AddRow(LabelOf(TEXT("Reign.Title")), 24, CosmicPalette::White, 8.f);
	const FCivilizationState* Civ = Life.RuledCivIDs.IsValidIndex(Reign) ? CivOf(State, Life.RuledCivIDs[Reign]) : nullptr;
	const int64 Ticks = Life.ReignTicks.IsValidIndex(Reign) ? Life.ReignTicks[Reign] : 0;
	AddRow(CosmicUI::JoinList({ Civ ? Civ->CivName : FText::GetEmpty(), FText::Format(LabelOf(TEXT("Reign.Ticks")), FText::AsNumber(Ticks)) }),
		14, CosmicPalette::Grey, 14.f);
	const FGameplayTag Why = Life.ReignEndReasons.IsValidIndex(Reign) ? Life.ReignEndReasons[Reign] : FGameplayTag();
	const TCHAR* WhyKey = (Why.IsValid() && Why.MatchesTagExact(CosmicTags::ReignAbdicated())) ? TEXT("Reign.End.Abdicated")
		: (Why.IsValid() && Why.MatchesTagExact(CosmicTags::ReignDeposed())) ? TEXT("Reign.End.Deposed") : TEXT("Reign.End.Other");
	AddRow(LabelOf(WhyKey), 14, CosmicPalette::Grey, 2.f);
	Finish(TEXT("Reign.ToMap"), false, false);
}

void UCosmicCallingPagesWidget::Close()
{
	Shown = ECosmicCallingPage::None;
	LifeAt = INDEX_NONE;
	bSkipShown = false;
	bRevealed = false;
	Elapsed = 0.f;
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCosmicCallingPagesWidget::ShowProgress()
{
	using namespace CosmicCallingPagesImpl;

	const float VeilShare = bRevealed ? 1.f : FMath::Clamp(Elapsed / CosmicChronicleScreen::VeilSeconds, 0.f, 1.f);
	if (Veil)
	{
		Veil->SetBrushColor(FLinearColor(VeilColor.R, VeilColor.G, VeilColor.B, VeilAlpha * VeilShare));
	}
	const int32 Rows = LineRows.Num();
	const float Interval = Rows > 0 ? FMath::Min(CosmicChronicleScreen::LineSeconds, CosmicChronicleScreen::RevealCapSeconds / static_cast<float>(Rows)) : 0.f;
	const float SinceVeil = Elapsed - CosmicChronicleScreen::VeilSeconds;
	int32 Showing = Rows;
	if (!bRevealed)
	{
		Showing = SinceVeil < 0.f ? 0 : (Interval > 0.f ? FMath::Min(Rows, static_cast<int32>(SinceVeil / Interval) + 1) : Rows);
		bRevealed = Showing >= Rows && SinceVeil >= Interval * static_cast<float>(Rows);
	}
	for (int32 At = 0; At < Rows; ++At)
	{
		if (UWidget* Row = LineRows[At])
		{
			Row->SetRenderOpacity((bRevealed || At < Showing) ? 1.f : 0.f);
		}
	}
	// The buttons only once every line shows: a click meant for the page never turns it.
	if (ButtonRow)
	{
		ButtonRow->SetVisibility(bRevealed ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
	}
}

void UCosmicCallingPagesWidget::Advance(float DeltaSeconds)
{
	if (!IsOpen() || bRevealed)
	{
		return;
	}
	Elapsed += FMath::Max(DeltaSeconds, 0.f);
	ShowProgress();
}

void UCosmicCallingPagesWidget::PressVeil()
{
	if (!IsOpen())
	{
		return;
	}
	bRevealed = true;
	ShowProgress();
}

void UCosmicCallingPagesWidget::PressNext()
{
	if (!IsOpen() || !bRevealed)
	{
		return;
	}
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->NextCallingPage();
	}
	else
	{
		Close();
	}
}

void UCosmicCallingPagesWidget::PressSkip()
{
	if (!IsOpen() || !bRevealed || !bSkipShown)
	{
		return;
	}
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->SkipCallingPages();
	}
	else
	{
		Close();
	}
}

void UCosmicCallingPagesWidget::HandleAction(FName Action, float Value)
{
	if (Action == CosmicCallingPagesImpl::NextAction)
	{
		PressNext();
	}
	else if (Action == CosmicCallingPagesImpl::SkipAction)
	{
		PressSkip();
	}
}

void UCosmicCallingPagesWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Advance(InDeltaTime);
}

FReply UCosmicCallingPagesWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (IsRevealing())
	{
		PressVeil();
	}
	return FReply::Handled();
}

FReply UCosmicCallingPagesWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

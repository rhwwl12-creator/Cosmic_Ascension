// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicPetitionCardWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
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
#include "Events/CosmicPetitions.h"
#include "Internationalization/Text.h"
#include "Layout/Margin.h"
#include "Math/NumericLimits.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicEdictWidget.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicGameMode.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicPetitionCardImpl
{
	// A little darker and less see-through than the panels: the card is read, not looked through.
	const FLinearColor CardColor(0.01f, 0.01f, 0.03f, 0.94f);
	// The voice's line: the grey of the panels' secondary words.
	const FLinearColor VoiceColor(0.7f, 0.7f, 0.78f);
	const FLinearColor LineColor(0.75f, 0.75f, 0.82f);
	const FName ChoiceAction(TEXT("choice"));
	const FName CloseAction(TEXT("close"));

	bool SameWords(const FText& A, const FText& B)
	{
		return A.ToString().Equals(B.ToString(), ESearchCase::CaseSensitive);
	}

	const TCHAR* StatKeyOf(ECosmicCheckStat Stat)
	{
		switch (Stat)
		{
		case ECosmicCheckStat::Body:
			return TEXT("Petition.Stat.Body");
		case ECosmicCheckStat::Mind:
			return TEXT("Petition.Stat.Mind");
		case ECosmicCheckStat::Causality:
			return TEXT("Petition.Stat.Causality");
		case ECosmicCheckStat::Exaltation:
			return TEXT("Petition.Stat.Exaltation");
		case ECosmicCheckStat::Stability:
			return TEXT("Petition.Stat.Stability");
		case ECosmicCheckStat::NoCheck:
			break;
		}
		return nullptr;
	}

	// "Energy 60, Matter 60" of a choice's cost (its members above 0, as the answer pays them); empty for a free choice.
	FText CostOf(const FEncounterChoice& Choice)
	{
		const FResourcePool Cost = CosmicPetitions::CostOf(Choice);
		const TPair<const TCHAR*, float> Members[] = {
			{ TEXT("Resource.Energy"), Cost.Energy },
			{ TEXT("Resource.Matter"), Cost.Matter },
			{ TEXT("Resource.Insight"), Cost.Insight },
			{ TEXT("Resource.Influence"), Cost.Influence },
			{ TEXT("Resource.Causality"), Cost.Causality },
		};
		FNumberFormattingOptions Options;
		Options.MinimumFractionalDigits = 0;
		Options.MaximumFractionalDigits = 2;
		TArray<FText> Paid;
		for (const TPair<const TCHAR*, float>& Member : Members)
		{
			if (Member.Value > 0.f)
			{
				Paid.Add(FText::Format(INVTEXT("{0} {1}"), CosmicUI::Label(Member.Key), FText::AsNumber(Member.Value, &Options)));
			}
		}
		return CosmicUI::JoinList(Paid);
	}
}

TSharedRef<SWidget> UCosmicPetitionCardWidget::RebuildWidget()
{
	using namespace CosmicPetitionCardImpl;

	// RebuildWidget can run again after the Slate widgets are released; the tree is built once.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		// The card sits at the top of its area; the rest of the area lets clicks through to the map.
		UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>();
		Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		WidgetTree->RootWidget = Root;
		Box = WidgetTree->ConstructWidget<UBorder>();
		Box->SetBrushColor(CardColor);
		Box->SetPadding(FMargin(20.f, 16.f));
		Root->AddChildToVerticalBox(Box);

		UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
		Box->SetContent(Column);
		TitleText = CosmicUI::MakeText(*WidgetTree, 18);
		Column->AddChildToVerticalBox(TitleText)->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));
		VoiceText = CosmicUI::MakeText(*WidgetTree, 13);
		VoiceText->SetColorAndOpacity(FSlateColor(VoiceColor));
		VoiceText->SetVisibility(ESlateVisibility::Collapsed);
		Column->AddChildToVerticalBox(VoiceText)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		BodyText = CosmicUI::MakeText(*WidgetTree, 15);
		Column->AddChildToVerticalBox(BodyText)->SetPadding(FMargin(0.f, 0.f, 0.f, 12.f));
		ChoiceBox = WidgetTree->ConstructWidget<UVerticalBox>();
		Column->AddChildToVerticalBox(ChoiceBox);
		OutcomeText = CosmicUI::MakeText(*WidgetTree, 15);
		OutcomeText->SetVisibility(ESlateVisibility::Collapsed);
		Column->AddChildToVerticalBox(OutcomeText)->SetPadding(FMargin(0.f, 0.f, 0.f, 10.f));
		CloseButton = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		CloseButton->Setup(CloseAction, 0.f, CosmicUI::Label(TEXT("Petition.Close")));
		CloseButton->OnAction.AddUObject(this, &UCosmicPetitionCardWidget::HandleAction);
		CloseButton->SetVisibility(ESlateVisibility::Collapsed);
		Column->AddChildToVerticalBox(CloseButton);
		SetVisibility(ESlateVisibility::Collapsed);
	}
	return Super::RebuildWidget();
}

void UCosmicPetitionCardWidget::ShowCard(FName Row, FName SystemID, FName OtherCivID)
{
	using namespace CosmicPetitionCardImpl;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	const FEncounterDef* Def = (Sim && !Row.IsNone()) ? Sim->GetTables().FindEncounter(Row) : nullptr;
	if (!WidgetTree || !WidgetTree->RootWidget)
	{
		// The tree first: the card may be asked before the viewport built it.
		TakeWidget();
	}
	bShowingOutcome = false;
	if (!Def || !ChoiceBox)
	{
		ShownRow = NAME_None;
		ShownSystem = NAME_None;
		ShownOther = NAME_None;
		Voice = INDEX_NONE;
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	ShownRow = Row;
	ShownSystem = SystemID;
	ShownOther = OtherCivID;
	bShownWanderer = CosmicPetitions::IsWandererCard(*Def);

	// The subject: the ruled people and its system, or - a wanderer's card (B1) - the system the vessel stands in and its
	// people, as the answer reads them, while the vessel stands in the card's own system; otherwise (a destroyed system
	// carried the vessel away while the card waited) the card's system with no people - no people the viewer was not shown
	// is named.
	const FGalaxyState& State = Sim->GetState();
	const FCosmicVesselSubject Vessel = CosmicPetitions::VesselSubjectOf(State, Sim->GetStateIndex());
	const bool bVesselOnCard = State.Systems.IsValidIndex(Vessel.SystemIndex) && State.Systems[Vessel.SystemIndex].SystemID == SystemID;
	const int32 CivIndex = bShownWanderer ? (bVesselOnCard ? Vessel.CivIndex : INDEX_NONE) : CosmicPetitions::RuledCivIndexOf(State, Sim->GetStateIndex());
	const int32 SystemIndex = bShownWanderer ? (bVesselOnCard ? Vessel.SystemIndex : Sim->GetStateIndex().FindSystem(State, SystemID))
		: CosmicPetitions::SubjectOf(State, Sim->GetStateIndex(), CivIndex, Def->Subject, SystemID);
	const FFormatNamedArguments Args = CosmicPetitions::ArgsOf(State, Sim->GetTables(), CivIndex, SystemIndex, OtherCivID, Row == CosmicRows::PetitionFirstContact);

	// The voice that brings it, picked now and kept while it shows (section 5.10): the elders while the people leans to
	// the spirit, the council otherwise. A wanderer's card has none.
	Voice = INDEX_NONE;
	if (!bShownWanderer && Def->VoiceBodies.Num() >= 2)
	{
		Voice = (State.Civs.IsValidIndex(CivIndex) && State.Civs[CivIndex].Ethics.Spiritualism > 0.f) ? 1 : 0;
	}
	TitleText->SetText(FText::Format(Def->Title, Args));
	VoiceText->SetText(Voice == INDEX_NONE ? FText::GetEmpty() : CosmicUI::Label(Voice == 0 ? TEXT("Petition.Voice.Council") : TEXT("Petition.Voice.Elders")));
	VoiceText->SetVisibility(Voice == INDEX_NONE ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	BodyText->SetText(FText::Format(Voice == INDEX_NONE ? Def->Body : Def->VoiceBodies[Voice], Args));

	// The choices, built again for the new card: a button and the line under it.
	ChoiceBox->ClearChildren();
	ChoiceButtons.Reset();
	ChoiceLines.Reset();
	for (int32 At = 0; At < Def->Choices.Num(); ++At)
	{
		UCosmicActionButton* Button = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		Button->Setup(ChoiceAction, static_cast<float>(At), FText::Format(Def->Choices[At].Label, Args));
		Button->OnAction.AddUObject(this, &UCosmicPetitionCardWidget::HandleAction);
		ChoiceBox->AddChildToVerticalBox(Button)->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));
		UTextBlock* Line = CosmicUI::MakeText(*WidgetTree, 12);
		Line->SetColorAndOpacity(FSlateColor(LineColor));
		ChoiceBox->AddChildToVerticalBox(Line)->SetPadding(FMargin(8.f, 1.f, 0.f, 2.f));
		ChoiceButtons.Add(Button);
		ChoiceLines.Add(Line);
	}
	ChoiceBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	OutcomeText->SetVisibility(ESlateVisibility::Collapsed);
	CloseButton->SetVisibility(ESlateVisibility::Collapsed);
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	Refresh();
}

void UCosmicPetitionCardWidget::Refresh()
{
	using namespace CosmicPetitionCardImpl;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	const FEncounterDef* Def = (Sim && !ShownRow.IsNone()) ? Sim->GetTables().FindEncounter(ShownRow) : nullptr;
	if (!Def || bShowingOutcome)
	{
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	const int32 CivIndex = bShownWanderer ? CosmicPetitions::VesselSubjectOf(State, Sim->GetStateIndex()).CivIndex : CosmicPetitions::RuledCivIndexOf(State, Sim->GetStateIndex());
	const bool bStands = CosmicPetitions::StillStands(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
	for (int32 At = 0; At < ChoiceButtons.Num() && At < Def->Choices.Num(); ++At)
	{
		const FEncounterChoice& Choice = Def->Choices[At];
		// A wanderer pays nothing (B1): every choice of its card can be given.
		const bool bPayable = bShownWanderer
			|| (State.Civs.IsValidIndex(CivIndex) && CosmicMegaprojects::CanPay(State.Civs[CivIndex].Stockpile, CosmicPetitions::CostOf(Choice)));
		// A stance the gate would refuse (B3: the first contact card's [Reach out] with the friendly slots taken), with the
		// reason the peoples' tab gives - the answer refuses it too, while the card stands (a card whose people is gone is
		// withdrawn by any answer).
		const CosmicDiplomacy::ECosmicStanceBlock StanceBlock = (bShownWanderer || !bStands || !State.Civs.IsValidIndex(CivIndex)) ? CosmicDiplomacy::ECosmicStanceBlock::None
			: CosmicPetitions::StanceBlockOfChoice(State, Sim->GetStateIndex(), Sim->GetRules(), CivIndex, ShownOther, Choice);
		UCosmicActionButton* Button = ChoiceButtons[At];
		Button->SetIsEnabled(bPayable && StanceBlock == CosmicDiplomacy::ECosmicStanceBlock::None);
		// A tooltip set again is a new tooltip, and an open one would close: set only when the words change.
		const FText Tip = !bPayable ? CosmicUI::Label(TEXT("Petition.Short"))
			: (StanceBlock != CosmicDiplomacy::ECosmicStanceBlock::None ? UCosmicEdictWidget::StanceReasonOf(StanceBlock) : FText::GetEmpty());
		if (!SameWords(Button->GetToolTipText(), Tip))
		{
			Button->SetToolTipText(Tip);
		}
		// The cost and the rolls that pass, as numbers of the screen (section 8.3: numbers stay in the UI).
		const FText Cost = CostOf(Choice);
		const TCHAR* StatKey = StatKeyOf(Choice.CheckStat);
		const FText Check = StatKey
			? FText::Format(CosmicUI::Label(TEXT("Petition.Check")), CosmicUI::Label(StatKey), FText::AsNumber(CosmicPetitions::PassingRollsOf(State, CivIndex, Choice)))
			: FText::GetEmpty();
		const FText CostAndCheck = (!Cost.IsEmpty() && !Check.IsEmpty()) ? FText::Format(CosmicUI::Label(TEXT("Petition.Both")), Cost, Check) : (Cost.IsEmpty() ? Check : Cost);
		// A choice that turns a people friendly (B3): what the friendly stance holds - its upkeep and the slots (numbers of
		// the screen; they move only with a stance or a rung, so the line keeps still).
		FText Friendly;
		if (!bShownWanderer && State.Civs.IsValidIndex(CivIndex) && Choice.OnSuccess.ContainsByPredicate([](const FCosmicEffect& Effect)
			{
				return Effect.Type == EEffectType::SetStance && Effect.Tag == CosmicTags::StanceFriendly();
			}))
		{
			const FCosmicRulesDef& Rules = Sim->GetRules();
			const float Upkeep = (Rules.FriendlyUpkeepInfluence > 0.f && Rules.FriendlyUpkeepInfluence <= TNumericLimits<float>::Max()) ? Rules.FriendlyUpkeepInfluence : 0.f;
			FNumberFormattingOptions Options;
			Options.MinimumFractionalDigits = 0;
			Options.MaximumFractionalDigits = 2;
			Friendly = FText::Format(CosmicUI::Label(TEXT("Petition.FriendlyUpkeep")), FText::AsNumber(Upkeep, &Options),
				CosmicUI::DurationLabel(1, ACosmicPlayerController::TimeUnitFor(*this)), FText::AsNumber(CosmicDiplomacy::FriendlyCountOf(State.Civs[CivIndex])),
				FText::AsNumber(CosmicDiplomacy::FriendlySlotsOf(State.Civs[CivIndex], Rules)));
		}
		const FText Line = CosmicUI::JoinLines({ CostAndCheck, Friendly });
		UTextBlock* LineText = ChoiceLines[At];
		if (!SameWords(LineText->GetText(), Line))
		{
			LineText->SetText(Line);
		}
		LineText->SetVisibility(Line.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
}

void UCosmicPetitionCardWidget::PressChoice(int32 ChoiceIndex)
{
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer());
	if (!Controller || ShownRow.IsNone() || bShowingOutcome)
	{
		return;
	}
	const FCosmicPetitionAnswer Answer = Controller->AnswerPetition(ChoiceIndex);
	if (Answer.Outcome == ECosmicPetitionOutcome::Refused)
	{
		Refresh();
		return;
	}
	// The outcome in the card's place of the choices, until it is closed.
	bShowingOutcome = true;
	// A withdrawn wanderer's card is no petition: its own words (B1).
	const FText Withdrawn = CosmicUI::Label(bShownWanderer ? TEXT("Wander.Withdrawn") : TEXT("Petition.Withdrawn"));
	const FText Outcome = Answer.Outcome == ECosmicPetitionOutcome::Withdrawn ? Withdrawn : Answer.Line;
	OutcomeText->SetText(Outcome);
	OutcomeText->SetVisibility(Outcome.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	ChoiceBox->SetVisibility(ESlateVisibility::Collapsed);
	const bool bNext = !Controller->GetKnownView().PendingPetitionRow.IsNone();
	CloseButton->Setup(CosmicPetitionCardImpl::CloseAction, 0.f, CosmicUI::Label(!bNext ? TEXT("Petition.Close") : (bShownWanderer ? TEXT("Wander.Next") : TEXT("Petition.Next"))));
	CloseButton->SetVisibility(ESlateVisibility::Visible);
}

void UCosmicPetitionCardWidget::PressClose()
{
	if (!bShowingOutcome)
	{
		return;
	}
	bShowingOutcome = false;
	ShownRow = NAME_None;
	ShownSystem = NAME_None;
	ShownOther = NAME_None;
	Voice = INDEX_NONE;
	bShownWanderer = false;
	SetVisibility(ESlateVisibility::Collapsed);
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->ClosePetitionCard();
	}
}

void UCosmicPetitionCardWidget::HandleAction(FName Action, float Value)
{
	if (Action == CosmicPetitionCardImpl::CloseAction)
	{
		PressClose();
	}
	else
	{
		PressChoice(FMath::RoundToInt(Value));
	}
}

FReply UCosmicPetitionCardWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Only the card's box reaches here (the widget and its root are self-hit-test-invisible): the map gets no click of it.
	return FReply::Handled();
}

FReply UCosmicPetitionCardWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

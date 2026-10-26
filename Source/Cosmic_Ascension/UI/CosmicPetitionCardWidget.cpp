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
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEdicts.h"
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

	// The name of the hazard of Tag as the panel says it (its row's DisplayName), or the panel's word for a hazard.
	FText HazardNameOf(const FCosmicDataTables& Tables, const FGameplayTag& Tag)
	{
		const FHazardDef* Row = Tag.IsValid() ? Tables.FindHazardByTag(Tag) : nullptr;
		return (Row && !Row->DisplayName.IsEmpty()) ? Row->DisplayName : CosmicUI::Label(TEXT("Panel.Hazard"));
	}

	// The effect a refused gate names (Gate.EffectAt), or null.
	const FCosmicEffect* RefusedEffectOf(const FCosmicChoiceGate& Gate, const FEncounterChoice& Choice)
	{
		return Choice.OnSuccess.IsValidIndex(Gate.EffectAt) ? &Choice.OnSuccess[Gate.EffectAt] : nullptr;
	}

	// The edict a choice enacts: the refused one when its gate refuses an edict, else the first EnactEdict's Key; None for
	// a choice that enacts none.
	FName EdictRowOf(const FCosmicChoiceGate& Gate, const FEncounterChoice& Choice)
	{
		const FCosmicEffect* Refused = Gate.Kind == ECosmicChoiceGateKind::Edict ? RefusedEffectOf(Gate, Choice) : nullptr;
		const FCosmicEffect* Enact = Refused ? Refused : Choice.OnSuccess.FindByPredicate([](const FCosmicEffect& Effect) { return Effect.Type == EEffectType::EnactEdict; });
		return Enact ? Enact->Key : NAME_None;
	}

	// The long words of a choice's gate (B4 section 4): a stance's as the peoples' tab gives them (B3), an edict's as the
	// edict window gives them (the refused row's rung; the slots in the form for the people's Society research), the fleet's
	// time's (UCosmicPetitionCardWidget::ResistReasonOf with the refused hazard's name); empty for an open gate.
	FText GateReasonOf(const FCosmicChoiceGate& Gate, const FEncounterChoice& Choice, const FCosmicDataTables& Tables, const FCivilizationState& Civ,
		const FCosmicRulesDef& Rules)
	{
		const FCosmicEffect* Refused = RefusedEffectOf(Gate, Choice);
		switch (Gate.Kind)
		{
		case ECosmicChoiceGateKind::Stance:
			return UCosmicEdictWidget::StanceReasonOf(Gate.Stance);
		case ECosmicChoiceGateKind::Edict:
			return UCosmicEdictWidget::ReasonOf(Gate.Edict, (Refused && !Refused->Key.IsNone()) ? Tables.FindEdict(Refused->Key) : nullptr,
				UCosmicEdictWidget::NextSlotMarkOf(Civ, Rules).IsSet());
		case ECosmicChoiceGateKind::Resist:
			return UCosmicPetitionCardWidget::ResistReasonOf(Gate.Resist, HazardNameOf(Tables, Refused ? Refused->Tag : FGameplayTag()));
		case ECosmicChoiceGateKind::None:
			break;
		}
		return FText::GetEmpty();
	}

	// The short words after a greyed choice's label, as the edict window and the panel show them: an edict's
	// (UCosmicEdictWidget::SuffixOf - none for one held), a fleet with no cap yet (CosmicUI::FleetSuffix); none otherwise.
	FText GateSuffixOf(const FCosmicChoiceGate& Gate, const FEncounterChoice& Choice, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, EAscensionTier Tier)
	{
		const FCosmicEffect* Refused = RefusedEffectOf(Gate, Choice);
		if (Gate.Kind == ECosmicChoiceGateKind::Edict)
		{
			return UCosmicEdictWidget::SuffixOf(Gate.Edict, (Refused && !Refused->Key.IsNone()) ? Tables.FindEdict(Refused->Key) : nullptr);
		}
		if (Gate.Kind == ECosmicChoiceGateKind::Resist && Gate.Resist == CosmicConflict::ECosmicResistBlock::NoFleet)
		{
			return CosmicUI::FleetSuffix(Rules, Tier);
		}
		return FText::GetEmpty();
	}

	// A choice that enacts an edict (B4): the edict's cost - the choice costs nothing of its own - and the slots, held and
	// open (as the top bar counts them); empty for a choice that enacts none, or a row the table does not hold.
	FText EdictLineOf(FName Row, const FCosmicDataTables& Tables, const FCivilizationState& Civ, const FCosmicRulesDef& Rules)
	{
		const FEdictDef* Def = Row.IsNone() ? nullptr : Tables.FindEdict(Row);
		if (!Def)
		{
			return FText::GetEmpty();
		}
		const FText Cost = UCosmicEdictWidget::CostWordsOf(Def->EnactCost);
		return FText::Format(CosmicUI::Label(TEXT("Petition.EdictLine")), Cost.IsEmpty() ? CosmicUI::Label(TEXT("Petition.EdictCostNone")) : Cost,
			FText::AsNumber(CosmicEdicts::HeldCountOf(Civ, Tables)), FText::AsNumber(CosmicEdicts::SlotsOf(Civ, Rules)));
	}

	// A choice that buys a standing hazard time with the fleet (B4): the fleet now and after (CosmicConflict::ResistShareOf),
	// the hazard and the time added (CosmicConflict::ResistTicksOf - what the effect adds); empty for a choice that buys none.
	FText ResistLineOf(const FEncounterChoice& Choice, const FCosmicDataTables& Tables, const FCivilizationState& Civ, const FCosmicRulesDef& Rules,
		ECosmicTimeUnit Unit)
	{
		const FCosmicEffect* Delay = Choice.OnSuccess.FindByPredicate([](const FCosmicEffect& Effect) { return Effect.Type == EEffectType::DelayHazard; });
		const int64 Ticks = Delay ? CosmicConflict::ResistTicksOf(Delay->Value) : 0;
		if (Ticks <= 0)
		{
			return FText::GetEmpty();
		}
		const float Power = (Civ.FleetPower > 0.f && Civ.FleetPower <= TNumericLimits<float>::Max()) ? Civ.FleetPower : 0.f;
		return FText::Format(CosmicUI::Label(TEXT("Petition.ResistLine")), UCosmicEdictWidget::AmountOf(Power),
			UCosmicEdictWidget::AmountOf(Power * CosmicConflict::ResistShareOf(Rules)), HazardNameOf(Tables, Delay->Tag),
			CosmicUI::DurationLabel(FMath::Min<int64>(Ticks, MAX_int32), Unit));
	}
}

FText UCosmicPetitionCardWidget::ResistReasonOf(CosmicConflict::ECosmicResistBlock Block, const FText& Hazard)
{
	using CosmicConflict::ECosmicResistBlock;
	using CosmicUI::Label;

	switch (Block)
	{
	case ECosmicResistBlock::NoHazard:
		return FText::FormatNamed(Label(TEXT("Petition.Resist.Block.NoHazard")), TEXT("Hazard"), Hazard);
	case ECosmicResistBlock::Capital:
		return FText::FormatNamed(Label(TEXT("Petition.Resist.Block.Capital")), TEXT("Hazard"), Hazard);
	case ECosmicResistBlock::NoFleet:
		return Label(TEXT("Guard.Block.NoFleet"));
	case ECosmicResistBlock::Spent:
		// The panel's words for the same fleet under the same button words ([Send the fleet] of a siege).
		return Label(TEXT("Siege.Block.NoFleet"));
	case ECosmicResistBlock::None:
	case ECosmicResistBlock::NotOwn:
		// NotOwn: no card shows it - a card whose system is no longer the people's settled one no longer stands.
		break;
	}
	return FText::GetEmpty();
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
	ChoiceLabels.Reset();
	for (int32 At = 0; At < Def->Choices.Num(); ++At)
	{
		UCosmicActionButton* Button = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		ChoiceLabels.Add(FText::Format(Def->Choices[At].Label, Args));
		Button->Setup(ChoiceAction, static_cast<float>(At), ChoiceLabels.Last());
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
	const FCosmicDataTables& Tables = Sim->GetTables();
	const FCosmicRulesDef& Rules = Sim->GetRules();
	const int32 CivIndex = bShownWanderer ? CosmicPetitions::VesselSubjectOf(State, Sim->GetStateIndex()).CivIndex : CosmicPetitions::RuledCivIndexOf(State, Sim->GetStateIndex());
	const bool bStands = CosmicPetitions::StillStands(State, Sim->GetStateIndex(), Tables, Rules);
	// The ruled people whose gates the choices meet, and the card's system as the answer reads it (B4 section 4).
	const bool bGated = !bShownWanderer && bStands && State.Civs.IsValidIndex(CivIndex);
	const int32 SystemIndex = bGated ? CosmicPetitions::SubjectOf(State, Sim->GetStateIndex(), CivIndex, Def->Subject, State.Knowing.PendingPetitionSystemID) : INDEX_NONE;
	const ECosmicTimeUnit Unit = ACosmicPlayerController::TimeUnitFor(*this);
	for (int32 At = 0; At < ChoiceButtons.Num() && At < Def->Choices.Num(); ++At)
	{
		const FEncounterChoice& Choice = Def->Choices[At];
		// A wanderer pays nothing (B1): every choice of its card can be given.
		const bool bPayable = bShownWanderer
			|| (State.Civs.IsValidIndex(CivIndex) && CosmicMegaprojects::CanPay(State.Civs[CivIndex].Stockpile, CosmicPetitions::CostOf(Choice)));
		// A choice its gate would refuse (B3: the first contact card's [Reach out] with the friendly slots taken; B4: an edict
		// it cannot enact, the fleet's time it cannot buy), with the first reason of CosmicPetitions::ChoiceGateOf - the one
		// gate the answer reads, while the card stands (a card that no longer stands is withdrawn by any answer).
		const FCosmicChoiceGate Gate = bGated
			? CosmicPetitions::ChoiceGateOf(State, Sim->GetStateIndex(), Tables, Rules, CivIndex, SystemIndex, ShownOther, Choice) : FCosmicChoiceGate();
		UCosmicActionButton* Button = ChoiceButtons[At];
		Button->SetIsEnabled(bPayable && Gate.IsOpen());
		// The edict a choice enacts (B4): its words in the tooltip, as the edict window gives them (UCosmicEdictWidget::TipOf -
		// what it costs and does while it is held).
		const bool bRuled = !bShownWanderer && State.Civs.IsValidIndex(CivIndex);
		const FName EdictRow = bRuled ? EdictRowOf(Gate, Choice) : NAME_None;
		const FText EdictTip = (!EdictRow.IsNone() && Tables.FindEdict(EdictRow)) ? UCosmicEdictWidget::TipOf(Tables, EdictRow, Unit) : FText::GetEmpty();
		// A tooltip set again is a new tooltip, and an open one would close: set only when the words change. The stockpile's
		// reason comes first (the answer asks it first), then the gate's, then the edict's words.
		const FText Tip = !bPayable ? CosmicUI::Label(TEXT("Petition.Short"))
			: CosmicUI::JoinLines({ Gate.IsOpen() ? FText::GetEmpty() : GateReasonOf(Gate, Choice, Tables, State.Civs[CivIndex], Rules), EdictTip });
		if (!SameWords(Button->GetToolTipText(), Tip))
		{
			Button->SetToolTipText(Tip);
		}
		// The gate's short words after the label of a choice it greys (the edict window's and the panel's form); an edict the
		// people already holds carries the held mark in front instead, as a stance in force does on the peoples' page (B4
		// [B-4d2] - the grey then reads as kept, not as shut).
		const FText Suffix = (bPayable && !Gate.IsOpen()) ? GateSuffixOf(Gate, Choice, Tables, Rules, State.Civs[CivIndex].Tier) : FText::GetEmpty();
		const FText Base = ChoiceLabels.IsValidIndex(At) ? ChoiceLabels[At] : Button->GetLabel();
		const bool bHeld = bPayable && Gate.Kind == ECosmicChoiceGateKind::Edict && Gate.Edict == CosmicEdicts::ECosmicEdictBlock::Held;
		const FText ButtonLabel = bHeld ? FText::Format(INVTEXT("{0} {1}"), CosmicUI::Label(TEXT("Edict.Mark")), Base)
			: (Suffix.IsEmpty() ? Base : FText::Format(INVTEXT("{0}  {1}"), Base, Suffix));
		if (!SameWords(Button->GetLabel(), ButtonLabel))
		{
			Button->Setup(ChoiceAction, static_cast<float>(At), ButtonLabel);
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
			const float Upkeep = (Rules.FriendlyUpkeepInfluence > 0.f && Rules.FriendlyUpkeepInfluence <= TNumericLimits<float>::Max()) ? Rules.FriendlyUpkeepInfluence : 0.f;
			FNumberFormattingOptions Options;
			Options.MinimumFractionalDigits = 0;
			Options.MaximumFractionalDigits = 2;
			Friendly = FText::Format(CosmicUI::Label(TEXT("Petition.FriendlyUpkeep")), FText::AsNumber(Upkeep, &Options),
				CosmicUI::DurationLabel(1, Unit), FText::AsNumber(CosmicDiplomacy::FriendlyCountOf(State.Civs[CivIndex])),
				FText::AsNumber(CosmicDiplomacy::FriendlySlotsOf(State.Civs[CivIndex], Rules)));
		}
		// A choice that enacts an edict or buys the fleet's time (B4 section 4): what it holds, under it in the same place.
		const FText Edict = bRuled ? EdictLineOf(EdictRow, Tables, State.Civs[CivIndex], Rules) : FText::GetEmpty();
		const FText Resist = bRuled ? ResistLineOf(Choice, Tables, State.Civs[CivIndex], Rules, Unit) : FText::GetEmpty();
		const FText Line = CosmicUI::JoinLines({ CostAndCheck, Friendly, Edict, Resist });
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

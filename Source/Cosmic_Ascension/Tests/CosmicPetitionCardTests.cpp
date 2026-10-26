// Cosmic Ascension - automation tests of the petition card (Presentation, MVP item 2). Design: guideline.md section 5.10,
// 6.3; docs/60_DECISIONS/headers/A2_petition_cards.md section 6.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Events/CosmicPetitions.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicAscensionOverlayWidget.h"
#include "UI/CosmicEdictWidget.h"
#include "UI/CosmicPetitionCardWidget.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicAutoPause.h"
#include "World/CosmicGameMode.h"

namespace CosmicPetitionCardTests
{
	FString Words(const TCHAR* Key)
	{
		return CosmicUI::Label(Key).ToString();
	}

	FString Shown(const UTextBlock* Text)
	{
		return (Text && Text->GetVisibility() != ESlateVisibility::Collapsed) ? Text->GetText().ToString() : FString();
	}

	void Put(FGalaxyState& State, FName Row, FName SystemID = NAME_None)
	{
		State.Knowing.PendingPetitionRow = Row;
		State.Knowing.PendingPetitionSystemID = SystemID;
	}

	// A card of the test about the ruled people, with two free choices; the first chains Next when it is given.
	FEncounterDef TestCard(ECosmicEventTrigger Trigger, FName Next = NAME_None)
	{
		FEncounterDef Row;
		Row.Title = FText::AsCultureInvariant(TEXT("card"));
		Row.Body = FText::AsCultureInvariant(TEXT("{Civ} asks."));
		Row.Trigger = Trigger;
		Row.BaseWeight = Trigger == ECosmicEventTrigger::Named ? 0.f : 1.f;
		FCosmicCondition Ruler;
		Ruler.Type = EConditionType::PlayerRoleIs;
		Ruler.Key = FName(TEXT("Ruler"));
		Row.Conditions = { Ruler };
		Row.Choices.SetNum(2);
		for (FEncounterChoice& Choice : Row.Choices)
		{
			Choice.Label = FText::AsCultureInvariant(TEXT("choice"));
			Choice.SuccessText = FText::AsCultureInvariant(TEXT("done."));
		}
		Row.Choices[0].NextEncounter = Next;
		return Row;
	}
}

// The petition card (MVP item 2): what it shows of a waiting card - title, voice, text, the choices with their cost and
// passing rolls, a choice the stockpile cannot pay greyed out -, that a card arriving at the tick boundary stops a
// running game while the automatic pause is on and closing its outcome gives the time back, that a card after a load
// shows without a pause, and the outcome of a withdrawn card.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIPetitionCardTest, "Cosmic_Ascension.UI.PetitionCard", CosmicTests::Flags)
bool FCosmicUIPetitionCardTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPetitionCardTests;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	if (!FSlateApplication::IsInitialized())
	{
		AddError(TEXT("No Slate application: run the automation tests with -game (see CLAUDE.md)."));
		return false;
	}
	UWorld* World = Sim->GetGameInstance()->GetWorld();
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(World ? World->GetFirstPlayerController() : nullptr);
	ACosmicHUD* HUD = Controller ? Controller->GetHUD<ACosmicHUD>() : nullptr;
	UCosmicPetitionCardWidget* Card = HUD ? HUD->GetPetitionCard() : nullptr;
	UCosmicTopBarWidget* Bar = HUD ? HUD->GetTopBar() : nullptr;
	UCosmicAscensionOverlayWidget* Overlay = HUD ? HUD->GetAscensionOverlay() : nullptr;
	if (!TestNotNull(TEXT("The HUD has its petition card"), Card) || !TestNotNull(TEXT("and its top bar"), Bar) || !TestNotNull(TEXT("and its overlay"), Overlay))
	{
		return false;
	}
	const bool bViewerChoice = Controller->IsAutoPauseOn();
	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	ON_SCOPE_EXIT
	{
		Sim->SetTimeScale(0.f);
		Controller->SetAutoPause(bViewerChoice);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Sim->SetBroadcastEnabled(true);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Overlay->Close();
	Sim->SetTimeScale(0.f);
	Controller->SetAutoPause(true);
	FGalaxyState& Live = Sim->GetMutableState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const int32 Ruled = CosmicPetitions::RuledCivIndexOf(Live, Index);
	if (!TestTrue(TEXT("(the vessel rules a people)"), Ruled != INDEX_NONE))
	{
		return false;
	}
	const FName Walls(TEXT("petition_walls"));
	const FName Bread(TEXT("petition_bread"));
	const FEncounterDef* WallsRow = Sim->GetTables().FindEncounter(Walls);
	if (!TestNotNull(TEXT("(the game has the card petition_walls)"), WallsRow))
	{
		return false;
	}
	TestTrue(TEXT("No card shows while none waits"), Card->GetShownRow().IsNone() && Card->GetVisibility() == ESlateVisibility::Collapsed);

	// 1. What the card shows: a voiced card in the council's voice (Spiritualism 0), its choices and their lines.
	Live.Civs[Ruled].Stability = 0.4f;
	Live.Civs[Ruled].Ethics.Spiritualism = 0.f;
	Live.Civs[Ruled].Stockpile = FResourcePool();
	Live.Civs[Ruled].Stockpile.Matter = 100.f;
	Put(Live, Walls);
	Controller->FollowPetition(false);
	FFormatNamedArguments Args = CosmicPetitions::ArgsOf(Live, Sim->GetTables(), Ruled, INDEX_NONE);
	TestEqual(TEXT("The card shows the waiting card"), Card->GetShownRow(), Walls);
	TestTrue(TEXT("and is visible"), Card->GetVisibility() != ESlateVisibility::Collapsed);
	TestEqual(TEXT("its title"), Shown(Card->GetTitleText()), FText::Format(WallsRow->Title, Args).ToString());
	TestEqual(TEXT("A people with Spiritualism 0 hears the council"), Card->GetVoice(), 0);
	TestEqual(TEXT("(the voice's line)"), Shown(Card->GetVoiceText()), Words(TEXT("Petition.Voice.Council")));
	TestEqual(TEXT("and the council's text"), Shown(Card->GetBodyText()), FText::Format(WallsRow->VoiceBodies[0], Args).ToString());
	TestEqual(TEXT("One button per choice"), Card->GetChoiceCount(), WallsRow->Choices.Num());
	const UCosmicActionButton* Build = Card->GetChoiceButton(0);
	if (TestNotNull(TEXT("(the first choice)"), Build))
	{
		TestEqual(TEXT("labelled as the row says"), Build->GetLabel().ToString(), FText::Format(WallsRow->Choices[0].Label, Args).ToString());
		TestTrue(TEXT("A choice the stockpile can pay is enabled"), Build->GetIsEnabled());
	}
	TestTrue(TEXT("Its line says the cost"), Shown(Card->GetChoiceLine(0)).Contains(Words(TEXT("Resource.Matter")) + TEXT(" 80")));
	// Stability 0.4 reads 4: against 5, five of the six rolls pass.
	const FString Rolls = FText::Format(CosmicUI::Label(TEXT("Petition.Check")), CosmicUI::Label(TEXT("Petition.Stat.Stability")), FText::AsNumber(5)).ToString();
	TestEqual(TEXT("A checked choice's line says the stat and the passing rolls"), Shown(Card->GetChoiceLine(1)), Rolls);
	Live.Civs[Ruled].Stockpile.Matter = 10.f;
	Card->Refresh();
	if (Build)
	{
		TestFalse(TEXT("A choice the stockpile cannot pay is greyed out"), Build->GetIsEnabled());
		TestEqual(TEXT("with the reason as its tooltip"), Build->GetToolTipText().ToString(), Words(TEXT("Petition.Short")));
	}
	Live.Civs[Ruled].Stockpile.Matter = 100.f;
	Card->Refresh();

	// The elders bring the card to a people that leans to the spirit; the voice is kept while the card shows.
	Live.Civs[Ruled].Ethics.Spiritualism = 0.3f;
	Controller->FollowPetition(false);
	TestEqual(TEXT("The same card keeps its voice"), Card->GetVoice(), 0);
	Live.Civs[Ruled].Stability = 0.2f;
	Put(Live, Bread);
	Controller->FollowPetition(false);
	TestEqual(TEXT("A new card shows"), Card->GetShownRow(), Bread);
	TestEqual(TEXT("in the elders' voice"), Card->GetVoice(), 1);
	TestEqual(TEXT("(the voice's line)"), Shown(Card->GetVoiceText()), Words(TEXT("Petition.Voice.Elders")));

	// 2. A card that arrives at the tick boundary stops a running game; the outcome's close gives the time back.
	Live.Civs[Ruled].Ethics.Spiritualism = 0.f;
	Live.Civs[Ruled].Stability = 0.4f;
	CosmicPetitions::Clear(Live);
	Controller->FollowPetition(false);
	TestTrue(TEXT("A card taken away collapses the card"), Card->GetShownRow().IsNone() && Card->GetVisibility() == ESlateVisibility::Collapsed);
	Controller->SetSpeed(2.f);
	Put(Live, Walls);
	Controller->FollowPetition(true);
	TestTrue(TEXT("A card that arrives stops a running game"), !(Sim->GetTimeScale() > 0.f));
	TestTrue(TEXT("for the card"), Controller->GetAutoPauseReason() == ECosmicAutoPause::Card);
	const FString Why = Bar->GetAutoPauseText() ? Bar->GetAutoPauseText()->GetText().ToString() : FString();
	TestEqual(TEXT("and the top bar says so"), Why, Words(TEXT("AutoPause.Why.Card")));
	Card->PressChoice(0);
	TestTrue(TEXT("An answer shows its outcome"), Card->IsShowingOutcome());
	TestFalse(TEXT("(the outcome line)"), Shown(Card->GetOutcomeText()).IsEmpty());
	TestTrue(TEXT("the card was answered"), Live.Knowing.PendingPetitionRow.IsNone());
	const UCosmicActionButton* Close = Card->GetCloseButton();
	TestTrue(TEXT("with a close button"), Close && Close->GetVisibility() != ESlateVisibility::Collapsed && Close->GetLabel().ToString() == Words(TEXT("Petition.Close")));
	Controller->FollowPetition(true);
	TestTrue(TEXT("The outcome stays over a tick"), Card->IsShowingOutcome());
	Card->PressClose();
	TestTrue(TEXT("Closing collapses the card"), Card->GetShownRow().IsNone() && Card->GetVisibility() == ESlateVisibility::Collapsed);
	TestEqual(TEXT("and the time runs again at the speed before the card"), Sim->GetTimeScale(), 2.f);
	// A pause the player set after the card's is the player's: closing does not lift it.
	Put(Live, Walls);
	Controller->FollowPetition(true);
	Controller->SetSpeed(0.f);
	Card->PressChoice(1);
	Card->PressClose();
	TestFalse(TEXT("Closing leaves a pause the player set"), Sim->GetTimeScale() > 0.f);
	Sim->SetTimeScale(0.f);

	// The HUD's redraw after a command redraws the card's choices too (a paused game sends no tick).
	Live.Civs[Ruled].Stability = 0.4f;
	Live.Civs[Ruled].Stockpile.Matter = 100.f;
	Put(Live, Walls);
	Controller->FollowPetition(false);
	Live.Civs[Ruled].Stockpile.Matter = 10.f;
	HUD->RefreshPanel();
	const UCosmicActionButton* Paid = Card->GetChoiceButton(0);
	TestTrue(TEXT("The HUD's redraw greys out a choice the stockpile can no longer pay"), Paid && !Paid->GetIsEnabled());
	Live.Civs[Ruled].Stockpile.Matter = 100.f;
	CosmicPetitions::Clear(Live);
	Controller->FollowPetition(false);

	// With the automatic pause off the card shows over a running game; a card that waits after a load shows without one.
	Controller->SetAutoPause(false);
	Controller->SetSpeed(1.f);
	Put(Live, Walls);
	Controller->FollowPetition(true);
	TestEqual(TEXT("With the pause off a card shows"), Card->GetShownRow(), Walls);
	TestEqual(TEXT("and the game runs on"), Sim->GetTimeScale(), 1.f);
	Controller->SetAutoPause(true);
	CosmicPetitions::Clear(Live);
	Controller->FollowPetition(false);
	Put(Live, Walls);
	Controller->FollowPetition(false);
	TestEqual(TEXT("A card that waits after a load shows"), Card->GetShownRow(), Walls);
	TestEqual(TEXT("without a pause"), Sim->GetTimeScale(), 1.f);
	Controller->SetSpeed(0.f);

	// 3. A chained card: the button says the next petition, and closing shows it at once with the time still standing.
	{
		const FCosmicDataTables Original = Sim->GetTables();
		ON_SCOPE_EXIT { Sim->SetTables(Original); };
		const FName First(TEXT("petition_test_first"));
		const FName Next(TEXT("petition_test_next"));
		UDataTable* Cards = CosmicTests::NewTable(FEncounterDef::StaticStruct());
		Cards->AddRow(First, TestCard(ECosmicEventTrigger::Standing, Next));
		Cards->AddRow(Next, TestCard(ECosmicEventTrigger::Named));
		const FName Again(TEXT("petition_test_again"));
		Cards->AddRow(Again, TestCard(ECosmicEventTrigger::Standing, Again));
		FCosmicDataTables Tables = Original;
		Tables.Encounters = Cards;
		Sim->SetTables(Tables);
		Controller->SetSpeed(1.f);
		Put(Live, First);
		Controller->FollowPetition(true);
		Card->PressChoice(0);
		const UCosmicActionButton* Onward = Card->GetCloseButton();
		TestTrue(TEXT("After a chaining answer the button says the next petition"), Onward && Onward->GetLabel().ToString() == Words(TEXT("Petition.Next")));
		Card->PressClose();
		TestEqual(TEXT("Closing shows the next card at once"), Card->GetShownRow(), Next);
		TestFalse(TEXT("with the time still standing"), Sim->GetTimeScale() > 0.f);
		Card->PressChoice(1);
		Card->PressClose();
		TestTrue(TEXT("The last card of the chain closes the card"), Card->GetShownRow().IsNone());
		TestEqual(TEXT("and gives the time back"), Sim->GetTimeScale(), 1.f);
		Sim->SetTimeScale(0.f);
		// A card that chains itself shows again with its choices, not with the outcome of the answer before.
		Put(Live, Again);
		Controller->FollowPetition(false);
		Card->PressChoice(0);
		Card->PressClose();
		TestEqual(TEXT("A card that chains itself shows again"), Card->GetShownRow(), Again);
		TestTrue(TEXT("with its choices and without the outcome"), !Card->IsShowingOutcome() && Shown(Card->GetOutcomeText()).IsEmpty() && Card->GetChoiceCount() == 2);
		CosmicPetitions::Clear(Live);
		Controller->FollowPetition(false);
	}

	// 4. A card that no longer stands is withdrawn: the card says so.
	Put(Live, Walls);
	Controller->FollowPetition(false);
	Live.Civs[Ruled].Stability = 0.6f;
	Card->PressChoice(1);
	TestEqual(TEXT("A withdrawn card's outcome"), Shown(Card->GetOutcomeText()), Words(TEXT("Petition.Withdrawn")));
	Card->PressClose();

	// 5. A card's gates (B4 header section 4): a choice that enacts an edict or buys a standing hazard time with the fleet
	// is greyed by the first reason of CosmicPetitions::ChoiceGateOf - the gate the answer reads -, with the short words
	// after its label, and says under it what it holds; an edict's choice tells the edict's words in its tooltip. The
	// test's edicts and card; the plague stands on the ruled people's capital.
	{
		const FCosmicDataTables Original = Sim->GetTables();
		ON_SCOPE_EXIT { Sim->SetTables(Original); };
		const FName Plain(TEXT("edict_test_plain"));
		const FName High(TEXT("edict_test_high"));
		const FName Fill(TEXT("edict_test_fill"));
		const FName Gate(TEXT("petition_test_gate"));
		UDataTable* Edicts = CosmicTests::NewTable(FEdictDef::StaticStruct());
		FEdictDef PlainRow;
		PlainRow.DisplayName = FText::AsCultureInvariant(TEXT("plain"));
		PlainRow.EnactCost.Influence = 10.f;
		Edicts->AddRow(Plain, PlainRow);
		FEdictDef HighRow;
		HighRow.DisplayName = FText::AsCultureInvariant(TEXT("high"));
		HighRow.RequiredTier = EAscensionTier::Tier2_Stellar;
		Edicts->AddRow(High, HighRow);
		FEdictDef FillRow;
		FillRow.DisplayName = FText::AsCultureInvariant(TEXT("fill"));
		Edicts->AddRow(Fill, FillRow);
		auto Effect = [](EEffectType Type, const TCHAR* TagName, FName Key, float Value)
		{
			FCosmicEffect Out;
			Out.Type = Type;
			Out.Tag = TagName ? FGameplayTag::RequestGameplayTag(FName(TagName)) : FGameplayTag();
			Out.Key = Key;
			Out.Value = Value;
			return Out;
		};
		// Six choices: an edict of the first rung, one of the second, the fleet's time on the plague and on the rebellion,
		// the watch, and the fleet's time on the plague at a cost no stockpile here pays.
		FEncounterDef GateRow = TestCard(ECosmicEventTrigger::Named);
		GateRow.Subject = ECosmicEventSubject::SettledSystem;
		GateRow.Choices.SetNum(6);
		for (FEncounterChoice& Choice : GateRow.Choices)
		{
			Choice.Label = FText::AsCultureInvariant(TEXT("choice"));
			Choice.SuccessText = FText::AsCultureInvariant(TEXT("done."));
			Choice.NextEncounter = NAME_None;
		}
		GateRow.Choices[0].OnSuccess = { Effect(EEffectType::EnactEdict, nullptr, Plain, 0.f) };
		GateRow.Choices[1].OnSuccess = { Effect(EEffectType::EnactEdict, nullptr, High, 0.f) };
		GateRow.Choices[2].OnSuccess = { Effect(EEffectType::DelayHazard, TEXT("Hazard.Plague"), NAME_None, 50.f) };
		GateRow.Choices[3].OnSuccess = { Effect(EEffectType::DelayHazard, TEXT("Hazard.Rebellion"), NAME_None, 50.f) };
		GateRow.Choices[5].OnSuccess = { Effect(EEffectType::DelayHazard, TEXT("Hazard.Plague"), NAME_None, 50.f) };
		GateRow.Choices[5].Cost.Influence = 1000.f;
		UDataTable* Cards = CosmicTests::NewTable(FEncounterDef::StaticStruct());
		Cards->AddRow(Gate, GateRow);
		// A share of the fleet kept that is not its own complement (0.25 - the line and the answer cannot swap kept and spent).
		FCosmicRulesDef Rules = *Original.FindRules();
		Rules.ResistFleetFactor = 0.25f;
		Rules.FleetPerSettledByTier = { 0.f, 1.f, 1.f, 1.f, 1.f };
		Rules.EdictSlotSocietyMarks = { 0.5f };
		auto TablesWith = [&](const FCosmicRulesDef& With)
		{
			FCosmicDataTables Out = CosmicTests::WithRules(Original, With);
			Out.Edicts = Edicts;
			Out.Encounters = Cards;
			return Out;
		};
		const FCosmicDataTables Tables = TablesWith(Rules);
		Sim->SetTables(Tables);

		FCivilizationState& Civ = Live.Civs[Ruled];
		const int32 Capital = Index.FindSystem(Live, Civ.HomeSystemID);
		if (!TestTrue(TEXT("(the ruled people's capital is its settled system)"), Live.Systems.IsValidIndex(Capital)
			&& CosmicPetitions::SubjectOf(Live, Index, Ruled, ECosmicEventSubject::SettledSystem, Live.Systems[Capital].SystemID) == Capital))
		{
			return false;
		}
		const FGameplayTag Plague = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Plague")));
		const FGameplayTag Rebellion = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Rebellion")));
		const FHazardDef* PlagueRow = Tables.FindHazardByTag(Plague);
		const FHazardDef* RebellionRow = Tables.FindHazardByTag(Rebellion);
		if (!TestNotNull(TEXT("(the game has the plague)"), PlagueRow) || !TestNotNull(TEXT("(and the rebellion)"), RebellionRow))
		{
			return false;
		}
		const FText PlagueName = PlagueRow->DisplayName;
		const FText RebellionName = RebellionRow->DisplayName;

		// The fleet's reasons, one by one (pure): the hazard named where the words name one; none for NotOwn, which no card
		// shows, and for None.
		{
			using CosmicConflict::ECosmicResistBlock;
			const FText Name = FText::AsCultureInvariant(TEXT("hazard"));
			TestEqual(TEXT("NoHazard names the hazard"), UCosmicPetitionCardWidget::ResistReasonOf(ECosmicResistBlock::NoHazard, Name).ToString(),
				FText::FormatNamed(CosmicUI::Label(TEXT("Petition.Resist.Block.NoHazard")), TEXT("Hazard"), Name).ToString());
			TestEqual(TEXT("Capital names the hazard"), UCosmicPetitionCardWidget::ResistReasonOf(ECosmicResistBlock::Capital, Name).ToString(),
				FText::FormatNamed(CosmicUI::Label(TEXT("Petition.Resist.Block.Capital")), TEXT("Hazard"), Name).ToString());
			TestEqual(TEXT("NoFleet: the guard's words"), UCosmicPetitionCardWidget::ResistReasonOf(ECosmicResistBlock::NoFleet, Name).ToString(), Words(TEXT("Guard.Block.NoFleet")));
			TestEqual(TEXT("Spent: the panel's words for no fleet to send"), UCosmicPetitionCardWidget::ResistReasonOf(ECosmicResistBlock::Spent, Name).ToString(),
				Words(TEXT("Siege.Block.NoFleet")));
			TestTrue(TEXT("NotOwn: none, which no card shows"), UCosmicPetitionCardWidget::ResistReasonOf(ECosmicResistBlock::NotOwn, Name).IsEmpty());
			TestTrue(TEXT("None: none"), UCosmicPetitionCardWidget::ResistReasonOf(ECosmicResistBlock::None, Name).IsEmpty());
			TestFalse(TEXT("(the named words hold the name)"), UCosmicPetitionCardWidget::ResistReasonOf(ECosmicResistBlock::NoHazard, Name).ToString().Contains(TEXT("{")));
		}

		const FName Society(TEXT("Society"));
		Civ.Tier = EAscensionTier::Tier1_Planetary;
		Civ.ActiveEdicts.Reset();
		Civ.Research.Add(Society, 0.f);
		Civ.Stockpile = FResourcePool();
		Civ.Stockpile.Influence = 100.f;
		Civ.FleetPower = 0.f;
		Live.Systems[Capital].ActiveHazard = Plague;
		Live.Systems[Capital].HazardTicksRemaining = 300;
		Put(Live, Gate, Live.Systems[Capital].SystemID);
		Controller->FollowPetition(false);
		if (!TestEqual(TEXT("The card of the gates shows"), Card->GetShownRow(), Gate) || !TestEqual(TEXT("(with its six choices)"), Card->GetChoiceCount(), 6))
		{
			return false;
		}
		const ECosmicTimeUnit Unit = ACosmicPlayerController::TimeUnitFor(*Card);
		auto GateOf = [&](int32 At)
		{
			return CosmicPetitions::ChoiceGateOf(Live, Index, Sim->GetTables(), Sim->GetRules(), Ruled, Capital, NAME_None, GateRow.Choices[At]);
		};
		auto Enabled = [&](int32 At)
		{
			const UCosmicActionButton* Button = Card->GetChoiceButton(At);
			return Button && Button->GetIsEnabled();
		};
		auto Tip = [&](int32 At)
		{
			const UCosmicActionButton* Button = Card->GetChoiceButton(At);
			return Button ? Button->GetToolTipText().ToString() : FString(TEXT("(no button)"));
		};
		auto LabelOf = [&](int32 At)
		{
			const UCosmicActionButton* Button = Card->GetChoiceButton(At);
			return Button ? Button->GetLabel().ToString() : FString(TEXT("(no button)"));
		};
		auto Suffixed = [](const FText& Suffix)
		{
			return FText::Format(INVTEXT("{0}  {1}"), FText::AsCultureInvariant(TEXT("choice")), Suffix).ToString();
		};
		auto EdictTip = [&](const FText& Reason, FName Row)
		{
			return CosmicUI::JoinLines({ Reason, UCosmicEdictWidget::TipOf(Sim->GetTables(), Row, Unit) }).ToString();
		};
		auto EdictLine = [&](const FText& Cost)
		{
			return FText::Format(CosmicUI::Label(TEXT("Petition.EdictLine")), Cost, FText::AsNumber(CosmicEdicts::HeldCountOf(Civ, Sim->GetTables())),
				FText::AsNumber(CosmicEdicts::SlotsOf(Civ, Sim->GetRules()))).ToString();
		};
		auto ResistLine = [&](const TCHAR* Now, const TCHAR* After)
		{
			return FText::Format(CosmicUI::Label(TEXT("Petition.ResistLine")), FText::AsCultureInvariant(Now), FText::AsCultureInvariant(After), PlagueName,
				CosmicUI::DurationLabel(50, Unit)).ToString();
		};
		const FString PlainCost = EdictLine(UCosmicEdictWidget::CostWordsOf(PlainRow.EnactCost));

		// The first rung: an edict it can enact, one of the second rung, the fleet's time without a fleet, a hazard that does
		// not stand, the watch, and a choice the stockpile cannot pay.
		TestTrue(TEXT("A choice whose edict can be enacted is enabled"), Enabled(0));
		TestTrue(TEXT("(its gate is open)"), GateOf(0).IsOpen());
		TestEqual(TEXT("its label is the row's"), LabelOf(0), FString(TEXT("choice")));
		TestEqual(TEXT("its tooltip tells the edict's words"), Tip(0), EdictTip(FText::GetEmpty(), Plain));
		TestEqual(TEXT("Under it: the edict's cost and the slots"), Shown(Card->GetChoiceLine(0)), PlainCost);
		TestFalse(TEXT("An edict of a higher rung is greyed out"), Enabled(1));
		TestTrue(TEXT("(the gate's first reason is the rung)"), GateOf(1).Kind == ECosmicChoiceGateKind::Edict && GateOf(1).Edict == CosmicEdicts::ECosmicEdictBlock::Tier);
		TestEqual(TEXT("with the edict window's words of the rung, then the edict's"), Tip(1),
			EdictTip(FText::Format(CosmicUI::Label(TEXT("Edict.Block.Tier")), FText::AsNumber(2)), High));
		TestEqual(TEXT("and the rung's short words after its label"), LabelOf(1), Suffixed(UCosmicEdictWidget::SuffixOf(CosmicEdicts::ECosmicEdictBlock::Tier, &HighRow)));
		TestEqual(TEXT("A free edict's line says it costs nothing"), Shown(Card->GetChoiceLine(1)), EdictLine(CosmicUI::Label(TEXT("Petition.EdictCostNone"))));
		TestFalse(TEXT("The fleet's time without a fleet (the first rung) is greyed out"), Enabled(2));
		TestTrue(TEXT("(the gate's first reason is NoFleet)"), GateOf(2).Kind == ECosmicChoiceGateKind::Resist && GateOf(2).Resist == CosmicConflict::ECosmicResistBlock::NoFleet);
		TestEqual(TEXT("with the guard's words: the fleet comes at the second rung"), Tip(2), Words(TEXT("Guard.Block.NoFleet")));
		TestEqual(TEXT("and the guard's short words after its label"), LabelOf(2), Suffixed(CosmicUI::Label(TEXT("Guard.Suffix.NoFleet"))));
		TestEqual(TEXT("Under it: the fleet now and after, the hazard and the time added"), Shown(Card->GetChoiceLine(2)), ResistLine(TEXT("0"), TEXT("0")));
		TestFalse(TEXT("The fleet's time on a hazard that does not stand is greyed out"), Enabled(3));
		TestTrue(TEXT("(the gate's first reason is NoHazard)"), GateOf(3).Resist == CosmicConflict::ECosmicResistBlock::NoHazard);
		TestEqual(TEXT("with its words, naming the hazard it would put off"), Tip(3),
			FText::FormatNamed(CosmicUI::Label(TEXT("Petition.Resist.Block.NoHazard")), TEXT("Hazard"), RebellionName).ToString());
		TestEqual(TEXT("and no short words after its label"), LabelOf(3), FString(TEXT("choice")));
		TestTrue(TEXT("The watch is enabled"), Enabled(4));
		TestEqual(TEXT("with no line under it"), Shown(Card->GetChoiceLine(4)), FString());
		TestFalse(TEXT("A choice the stockpile cannot pay is greyed out"), Enabled(5));
		TestEqual(TEXT("the stockpile's reason before the gate's"), Tip(5), Words(TEXT("Petition.Short")));
		TestEqual(TEXT("and no gate's short words after its label"), LabelOf(5), FString(TEXT("choice")));

		// The second rung: a cap but no fleet, then a fleet.
		Civ.Tier = EAscensionTier::Tier2_Stellar;
		Card->Refresh();
		TestTrue(TEXT("(the second rung has a cap)"), CosmicConflict::FleetCapOf(Live, Index, Ruled, Sim->GetRules()) > 0.f);
		TestTrue(TEXT("An edict of the second rung opens at the second rung"), Enabled(1));
		TestEqual(TEXT("its label loses the short words"), LabelOf(1), FString(TEXT("choice")));
		TestTrue(TEXT("(the gate's first reason is Spent)"), GateOf(2).Resist == CosmicConflict::ECosmicResistBlock::Spent);
		TestEqual(TEXT("A spent fleet greys the fleet's time with the panel's words"), Tip(2), Words(TEXT("Siege.Block.NoFleet")));
		TestEqual(TEXT("and no short words after its label"), LabelOf(2), FString(TEXT("choice")));
		Civ.FleetPower = 4.f;
		Card->Refresh();
		TestTrue(TEXT("A fleet opens the fleet's time"), Enabled(2));
		TestEqual(TEXT("its line says the share it keeps"), Shown(Card->GetChoiceLine(2)), ResistLine(TEXT("4"), TEXT("1")));
		{
			// A share outside [0, 1] keeps the fleet, as ResistShareOf reads it.
			FCosmicRulesDef Above = Rules;
			Above.ResistFleetFactor = 1.5f;
			Sim->SetTables(TablesWith(Above));
			Card->Refresh();
			TestEqual(TEXT("A factor outside [0, 1] keeps the whole fleet in the line"), Shown(Card->GetChoiceLine(2)), ResistLine(TEXT("4"), TEXT("4")));
			Sim->SetTables(Tables);
			Card->Refresh();
		}

		// An edict held, the slots taken (in both forms), a cost the stockpile cannot pay.
		Civ.ActiveEdicts = { Plain };
		Card->Refresh();
		TestFalse(TEXT("An edict held is greyed out"), Enabled(0));
		TestTrue(TEXT("(the gate's first reason is Held)"), GateOf(0).Edict == CosmicEdicts::ECosmicEdictBlock::Held);
		TestEqual(TEXT("with the edict window's words"), Tip(0), EdictTip(CosmicUI::Label(TEXT("Edict.Block.Held")), Plain));
		TestEqual(TEXT("and no short words (the edict window has none for it)"), LabelOf(0), FString(TEXT("choice")));
		TestTrue(TEXT("(one slot at Society 0)"), CosmicEdicts::SlotsOf(Civ, Sim->GetRules()) == 1);
		TestFalse(TEXT("The slots taken grey out another edict"), Enabled(1));
		TestTrue(TEXT("(the gate's first reason is Slots)"), GateOf(1).Edict == CosmicEdicts::ECosmicEdictBlock::Slots);
		TestEqual(TEXT("in the words of slots that research can grow"), Tip(1), EdictTip(CosmicUI::Label(TEXT("Edict.Block.Slots")), High));
		TestEqual(TEXT("with the slots' short words"), LabelOf(1), Suffixed(UCosmicEdictWidget::SuffixOf(CosmicEdicts::ECosmicEdictBlock::Slots, &HighRow)));
		TestEqual(TEXT("and its line counts the slot held"), Shown(Card->GetChoiceLine(1)), EdictLine(CosmicUI::Label(TEXT("Petition.EdictCostNone"))));
		Civ.Research.Add(Society, 1.f);
		Civ.ActiveEdicts = { Plain, Fill };
		Card->Refresh();
		TestTrue(TEXT("(two slots at Society 1, the last mark)"), CosmicEdicts::SlotsOf(Civ, Sim->GetRules()) == 2);
		TestEqual(TEXT("Every slot open and taken: the words of revoking"), Tip(1), EdictTip(CosmicUI::Label(TEXT("Edict.Block.SlotsAll")), High));
		Civ.ActiveEdicts.Reset();
		Civ.Stockpile.Influence = 5.f;
		Card->Refresh();
		TestFalse(TEXT("An edict the stockpile cannot pay is greyed out"), Enabled(0));
		TestTrue(TEXT("(the gate's first reason is Cost)"), GateOf(0).Edict == CosmicEdicts::ECosmicEdictBlock::Cost);
		TestEqual(TEXT("with the edict window's words"), Tip(0), EdictTip(CosmicUI::Label(TEXT("Edict.Block.Cost")), Plain));
		Civ.Stockpile.Influence = 100.f;

		// A rebellion on the capital: its fleet's time is refused (Capital), the plague's (no longer standing) too.
		Live.Systems[Capital].ActiveHazard = Rebellion;
		Card->Refresh();
		TestTrue(TEXT("(the gate's first reason is Capital)"), GateOf(3).Resist == CosmicConflict::ECosmicResistBlock::Capital);
		TestEqual(TEXT("A rebellion is not put off by the fleet"), Tip(3),
			FText::FormatNamed(CosmicUI::Label(TEXT("Petition.Resist.Block.Capital")), TEXT("Hazard"), RebellionName).ToString());
		TestEqual(TEXT("and the plague's choice says the plague does not stand"), Tip(2),
			FText::FormatNamed(CosmicUI::Label(TEXT("Petition.Resist.Block.NoHazard")), TEXT("Hazard"), PlagueName).ToString());

		// The answer agrees: a greyed choice is refused before its draw, an open one buys the time - and the panel's crisis
		// line shows it at once, with no tick.
		AddExpectedError(TEXT("the choice's gate refuses choice 3 of petition_test_gate"), EAutomationExpectedErrorFlags::Contains, 1);
		Card->PressChoice(3);
		TestFalse(TEXT("A greyed choice pressed anyway is refused"), Card->IsShowingOutcome());
		TestEqual(TEXT("and the card still waits"), Live.Knowing.PendingPetitionRow, Gate);
		Live.Systems[Capital].ActiveHazard = Plague;
		Live.Systems[Capital].HazardTicksRemaining = 300;
		Controller->SelectSystem(Live.Systems[Capital].SystemID);
		HUD->RefreshPanel();
		UCosmicSystemPanelWidget* Panel = HUD->GetSystemPanel();
		if (!TestNotNull(TEXT("(the HUD's system panel)"), Panel))
		{
			return false;
		}
		TestTrue(TEXT("(the panel's crisis line shows the time before)"), CosmicTests::ReadTexts(*Panel).Contains(CosmicUI::TimeLeftLabel(300, Unit).ToString()));
		Card->PressChoice(2);
		TestTrue(TEXT("An open fleet's time is answered"), Card->IsShowingOutcome());
		TestEqual(TEXT("and adds the time its line said"), Live.Systems[Capital].HazardTicksRemaining, 350);
		TestEqual(TEXT("for the share of the fleet its line said"), Civ.FleetPower, 1.f);
		TestTrue(TEXT("and the panel's crisis line shows the time at once, with no tick"), CosmicTests::ReadTexts(*Panel).Contains(CosmicUI::TimeLeftLabel(350, Unit).ToString()));
		Card->PressClose();
		Controller->SelectSystem(NAME_None);
		Live.Systems[Capital].ActiveHazard = FGameplayTag();
		Live.Systems[Capital].HazardTicksRemaining = 0;
		CosmicPetitions::Clear(Live);
		Controller->FollowPetition(false);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

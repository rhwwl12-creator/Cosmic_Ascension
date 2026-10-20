// Cosmic Ascension - automation tests of the petition card (Presentation, MVP item 2). Design: guideline.md section 5.10,
// 6.3; docs/60_DECISIONS/headers/A2_petition_cards.md section 6.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Events/CosmicPetitions.h"
#include "Framework/Application/SlateApplication.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicAscensionOverlayWidget.h"
#include "UI/CosmicPetitionCardWidget.h"
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
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

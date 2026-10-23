// Cosmic Ascension - automation tests of the provisional end's presentation (MVP item 4, commit A-4b): the controller's
// latch and its lock, the chronicle's page and the top bar's words. Design: docs/60_DECISIONS/headers/A4_provisional_end.md
// section 5, 7; docs/70_CONTENT_GUIDE.md section 8.4.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Events/CosmicPetitions.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "Internationalization/Text.h"
#include "Log/CosmicChronicle.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicEndings.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicAscensionOverlayWidget.h"
#include "UI/CosmicChronicleWidget.h"
#include "UI/CosmicPetitionCardWidget.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicAutoPause.h"
#include "World/CosmicGalaxyMapActor.h"
#include "World/CosmicGameMode.h"

namespace CosmicChronicleScreenTests
{
	FString Words(const TCHAR* Key)
	{
		return CosmicUI::Label(Key).ToString();
	}

	FString Shown(const UTextBlock* Text)
	{
		return (Text && Text->GetVisibility() != ESlateVisibility::Collapsed) ? Text->GetText().ToString() : FString();
	}

	bool IsShownWidget(const UWidget* Widget)
	{
		const ESlateVisibility Visibility = Widget ? Widget->GetVisibility() : ESlateVisibility::Collapsed;
		return Visibility != ESlateVisibility::Collapsed && Visibility != ESlateVisibility::Hidden;
	}

	// The row of the chronicle's two buttons (the parent of 'look at the map').
	const UWidget* ButtonRowOf(const UCosmicChronicleWidget& Page)
	{
		return Page.GetToMapButton() ? Page.GetToMapButton()->GetParent() : nullptr;
	}

	UCosmicActionButton* ButtonOf(const UCosmicSystemPanelWidget& Panel, FName Action)
	{
		for (UCosmicActionButton* Button : Panel.GetActionButtons())
		{
			if (Button && Button->GetAction() == Action)
			{
				return Button;
			}
		}
		return nullptr;
	}

	struct FScreen
	{
		UGalaxySimulationSubsystem* Sim = nullptr;
		ACosmicPlayerController* Controller = nullptr;
		ACosmicHUD* HUD = nullptr;
		UCosmicChronicleWidget* Page = nullptr;
		UCosmicTopBarWidget* Bar = nullptr;
		UCosmicAscensionOverlayWidget* Overlay = nullptr;
		UCosmicPetitionCardWidget* Card = nullptr;
		UCosmicSystemPanelWidget* Panel = nullptr;
		ACosmicGalaxyMapActor* Map = nullptr;
	};

	// The live controller, HUD and widgets; false (with a test error) when one is missing.
	bool Acquire(FAutomationTestBase& Test, FScreen& Out)
	{
		Out.Sim = CosmicTests::AcquireSim(Test);
		if (!Out.Sim)
		{
			return false;
		}
		if (!FSlateApplication::IsInitialized())
		{
			Test.AddError(TEXT("No Slate application: run the automation tests with -game (see CLAUDE.md)."));
			return false;
		}
		UWorld* World = Out.Sim->GetGameInstance()->GetWorld();
		Out.Controller = Cast<ACosmicPlayerController>(World ? World->GetFirstPlayerController() : nullptr);
		Out.HUD = Out.Controller ? Out.Controller->GetHUD<ACosmicHUD>() : nullptr;
		Out.Page = Out.HUD ? Out.HUD->GetChronicle() : nullptr;
		Out.Bar = Out.HUD ? Out.HUD->GetTopBar() : nullptr;
		Out.Overlay = Out.HUD ? Out.HUD->GetAscensionOverlay() : nullptr;
		Out.Card = Out.HUD ? Out.HUD->GetPetitionCard() : nullptr;
		Out.Panel = Out.HUD ? Out.HUD->GetSystemPanel() : nullptr;
		if (World)
		{
			for (TActorIterator<ACosmicGalaxyMapActor> It(World); It; ++It)
			{
				Out.Map = *It;
			}
		}
		return Test.TestNotNull(TEXT("The HUD has its chronicle"), Out.Page) && Test.TestNotNull(TEXT("and its top bar"), Out.Bar)
			&& Test.TestNotNull(TEXT("and its overlay"), Out.Overlay) && Test.TestNotNull(TEXT("and its petition card"), Out.Card)
			&& Test.TestNotNull(TEXT("and its system panel"), Out.Panel) && Test.TestNotNull(TEXT("The map stands"), Out.Map);
	}

	// No presentation and no reveal stands (one an earlier test raised).
	void Quiet(UCosmicAscensionOverlayWidget& Overlay)
	{
		Overlay.Close();
		Overlay.Advance(CosmicAscensionOverlay::RevealSeconds + 1.f);
	}
}

// The provisional end's latch and lock (MVP item 4): the clock end at exactly tick 6,000, the speed before it, the lock on
// the speeds, the keys and the commands, the card folded, the wait before the page, the third rung's two orders with the
// presentation, an ended load, FollowEnd's contract, the new run, the switch, the reset of AcquireSim and the two hashes.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIRunEndTest, "Cosmic_Ascension.UI.RunEnd", CosmicTests::Flags)
bool FCosmicUIRunEndTest::RunTest(const FString& Parameters)
{
	using namespace CosmicChronicleScreenTests;

	FScreen Screen;
	if (!Acquire(*this, Screen))
	{
		return false;
	}
	UGalaxySimulationSubsystem* Sim = Screen.Sim;
	ACosmicPlayerController* Controller = Screen.Controller;
	UCosmicChronicleWidget* Page = Screen.Page;
	const FCosmicDataTables Original = Sim->GetTables();
	const bool bViewerChoice = Controller->IsAutoPauseOn();
	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	ON_SCOPE_EXIT
	{
		Sim->SetTimeScale(0.f);
		Controller->SetAutoPause(bViewerChoice);
		Controller->SetProvisionalEndEnabled(true);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Controller->SetAutoPause(false);
	Sim->SetBroadcastEnabled(true);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Quiet(*Screen.Overlay);
	Sim->SetTimeScale(0.f);
	FGalaxyState& Live = Sim->GetMutableState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const int32 Ruled = CosmicPetitions::RuledCivIndexOf(Live, Index);
	const int32 Home = Live.Civs.IsValidIndex(Ruled) ? Index.FindSystem(Live, Live.Civs[Ruled].HomeSystemID) : INDEX_NONE;
	if (!TestTrue(TEXT("(the vessel rules a people with a capital)"), Live.Systems.IsValidIndex(Home)))
	{
		return false;
	}
	const int32 EndTick = Sim->GetRules().ProvisionalEndTick;
	TestEqual(TEXT("(the game's clock end is tick 6,000)"), EndTick, 6000);

	// (2) Before the end the speed is the viewer's.
	Live.Tick = EndTick - 2;
	Sim->StepOnce();
	Controller->SetSpeed(2.f);
	TestEqual(TEXT("One tick before the end the speed is set"), Sim->GetTimeScale(), 2.f);
	TestFalse(TEXT("and the run is not over"), Controller->IsRunEnded());

	// A card waits as the end arrives (4).
	// The stockpile pays any choice: only the end refuses the answer (3).
	Live.Civs[Ruled].Stockpile.Energy = 100000.f;
	Live.Civs[Ruled].Stockpile.Matter = 100000.f;
	Live.Civs[Ruled].Stockpile.Insight = 100000.f;
	Live.Civs[Ruled].Stockpile.Influence = 100000.f;
	Live.Civs[Ruled].Stockpile.Causality = 100000.f;
	const FName Walls(TEXT("petition_walls"));
	Live.Knowing.PendingPetitionRow = Walls;
	Live.Knowing.PendingPetitionSystemID = NAME_None;
	Controller->FollowPetition(false);
	TestEqual(TEXT("(the card shows before the end)"), Screen.Card->GetShownRow(), Walls);

	// (1) A frame at x10 from three ticks before: the latch of the tick boundary stops it at exactly the end tick.
	Live.Tick = EndTick - 3;
	Controller->SetSpeed(10.f);
	Sim->Tick(1.f);
	TestEqual(TEXT("The clock end stops the run at exactly its tick"), Live.Tick, static_cast<int64>(EndTick));
	TestEqual(TEXT("with the speed at 0"), Sim->GetTimeScale(), 0.f);
	TestEqual(TEXT("for the clock"), Controller->GetRunEnd(), ECosmicProvisionalEnd::Clock);
	TestTrue(TEXT("and the reason is the end"), Controller->GetAutoPauseReason() == ECosmicAutoPause::RunEnd);
	// (4) The card folds; its row stays waiting.
	TestTrue(TEXT("The end folds the card"), Screen.Card->GetShownRow().IsNone() && Screen.Card->GetVisibility() == ESlateVisibility::Collapsed);
	TestEqual(TEXT("and its row still waits"), Live.Knowing.PendingPetitionRow, Walls);
	Controller->FollowPetition(false);
	TestTrue(TEXT("An ended run shows the waiting card no more"), Screen.Card->GetShownRow().IsNone());

	// (12) From the end on, nothing the screens do moves either hash.
	const uint32 StateHash = Sim->ComputeStateHash();
	const uint32 WorldHash = Sim->ComputeWorldHash();
	const int32 Rng = Live.RngState;

	// (5) The stopped map stays HoldSeconds, then the page opens and reveals itself.
	Controller->AdvanceRunEnd(CosmicChronicleScreen::HoldSeconds - 0.1f);
	TestFalse(TEXT("Before the wait is over the page is folded"), Page->IsOpen());
	Controller->AdvanceRunEnd(0.2f);
	TestTrue(TEXT("After it the page opens"), Page->IsOpen());
	TestTrue(TEXT("and reveals its lines"), Page->IsRevealing());
	TestFalse(TEXT("and the bar's 'chronicle' goes without another redraw"), IsShownWidget(Screen.Bar->GetReopenButton()));

	// (3) The lock: the speeds, the keys, the buttons and the commands.
	Controller->SetSpeed(10.f);
	TestEqual(TEXT("SetSpeed after the end stays 0"), Sim->GetTimeScale(), 0.f);
	Controller->PressSpeedKey(EKeys::Two);
	TestEqual(TEXT("a number key too"), Sim->GetTimeScale(), 0.f);
	Controller->PressSpeedKey(EKeys::SpaceBar);
	TestEqual(TEXT("and Space"), Sim->GetTimeScale(), 0.f);
	TestTrue(TEXT("(the reason stays the end)"), Controller->GetAutoPauseReason() == ECosmicAutoPause::RunEnd);
	Screen.Bar->Refresh();
	for (const UCosmicActionButton* Button : Screen.Bar->GetSpeedButtons())
	{
		if (Button && IsShownWidget(Button))
		{
			TestFalse(*FString::Printf(TEXT("The speed button %g is greyed out"), Button->GetValue()), Button->GetIsEnabled());
		}
	}
	TestEqual(TEXT("The bar says why"), Shown(Screen.Bar->GetAutoPauseText()), Words(TEXT("Chronicle.Why")));
	TestTrue(TEXT("An answer to the waiting card is refused"), Controller->AnswerPetition(0).Outcome == ECosmicPetitionOutcome::Refused);
	TestEqual(TEXT("and the card still waits"), Live.Knowing.PendingPetitionRow, Walls);
	TestEqual(TEXT("with nothing drawn"), Live.RngState, Rng);
	TestFalse(TEXT("A project is not started"), Controller->StartProject(Live.Systems[Home].SystemID, FName(TEXT("project_dyson"))));
	TestTrue(TEXT("(the capital builds nothing)"), Live.Systems[Home].ActiveMegaproject.IsNone());
	// The research focus (MVP item 7): the command is refused, and the capital's four buttons are greyed with the end's
	// reason while the heading still names the focus.
	{
		const TMap<FName, float> FocusBefore = Live.Civs[Ruled].ResearchFocus;
		TestFalse(TEXT("A research focus is not taken"), Controller->SetResearchFocus(FName(TEXT("Physics"))));
		Controller->SelectSystem(Live.Systems[Home].SystemID);
		Screen.Panel->Refresh();
		const TArray<TObjectPtr<UCosmicActionButton>>& Focus = Screen.Panel->GetFocusButtons();
		bool bAllGrey = Screen.Panel->IsFocusShown() && Focus.Num() == 4;
		for (const UCosmicActionButton* Button : Focus)
		{
			bAllGrey &= Button && !Button->GetIsEnabled() && Button->GetToolTipText().ToString() == Words(TEXT("Chronicle.Block"));
		}
		TestTrue(TEXT("The capital's focus buttons are greyed with the end's reason"), bAllGrey);
		TestFalse(TEXT("and the heading still names the focus"), !Screen.Panel->GetFocusHeading() || Screen.Panel->GetFocusHeading()->GetText().IsEmpty());
		if (Focus.Num() == 4 && Focus[1])
		{
			Focus[1]->Press();
		}
		TestTrue(TEXT("A press takes nothing"), Live.Civs[Ruled].ResearchFocus.OrderIndependentCompareEqual(FocusBefore));
		Controller->SelectSystem(NAME_None);
	}
	// A system whose panel shows the settle button: its tooltip and the line under it give the end's reason.
	bool bSettleSeen = false;
	for (const FStarSystemState& System : Live.Systems)
	{
		Controller->SelectSystem(System.SystemID);
		Screen.Panel->Refresh();
		if (const UCosmicActionButton* Settle = ButtonOf(*Screen.Panel, CosmicRows::ProjectSettle))
		{
			bSettleSeen = true;
			TestFalse(TEXT("The settle button is greyed out"), Settle->GetIsEnabled());
			FString FirstLine = Settle->GetToolTipText().ToString();
			FirstLine.Split(TEXT("\n"), &FirstLine, nullptr);
			TestEqual(TEXT("and its tooltip gives the end's reason first"), FirstLine, Words(TEXT("Chronicle.Block")));
			TestTrue(TEXT("and so does the line under it"), CosmicTests::ReadTexts(*Screen.Panel).Contains(Words(TEXT("Chronicle.Block"))));
			break;
		}
	}
	TestTrue(TEXT("(some system's panel shows the settle button)"), bSettleSeen);
	Controller->SelectSystem(NAME_None);

	// (8) FollowEnd(false) over an ended state: the latch and the page at once; a second call changes nothing.
	Controller->ClearRunEnd();
	TestFalse(TEXT("(cleared)"), Controller->IsRunEnded() || Page->IsOpen());
	Controller->FollowEnd(false);
	TestEqual(TEXT("FollowEnd(false) over an ended state latches"), Controller->GetRunEnd(), ECosmicProvisionalEnd::Clock);
	TestTrue(TEXT("and opens the page fully shown"), Page->IsOpen() && !Page->IsRevealing());
	TestFalse(TEXT("with the bar's 'chronicle' gone"), IsShownWidget(Screen.Bar->GetReopenButton()));
	Controller->FollowEnd(false);
	TestTrue(TEXT("A second call changes nothing"), Controller->GetRunEnd() == ECosmicProvisionalEnd::Clock && Page->IsOpen() && !Page->IsRevealing());

	// (12) The page, the map, the reopening and the refused commands wrote nothing.
	Controller->LookAtMap();
	TestFalse(TEXT("'Look at the map' folds the page"), Page->IsOpen());
	Controller->ReopenChronicle();
	TestTrue(TEXT("and the bar's 'chronicle' opens it again"), Page->IsOpen() && !Page->IsRevealing());
	TestEqual(TEXT("From the end to the reopening the state hash stays"), Sim->ComputeStateHash(), StateHash);
	TestEqual(TEXT("and the world hash"), Sim->ComputeWorldHash(), WorldHash);

	// (7) A load of an ended copy: the page at once, no card.
	const FGalaxyState Ended = Sim->GetState();
	Controller->LookAtMap();
	Sim->LoadState(Ended);
	TestEqual(TEXT("A load of an ended state latches"), Controller->GetRunEnd(), ECosmicProvisionalEnd::Clock);
	TestTrue(TEXT("and shows the page fully"), Page->IsOpen() && !Page->IsRevealing());
	TestTrue(TEXT("with no card"), Screen.Card->GetShownRow().IsNone());
	TestEqual(TEXT("and the speed at 0"), Sim->GetTimeScale(), 0.f);

	// (9) A new run: another galaxy at tick 0, the latch and the page gone, x1, and no selection once the map is rebuilt.
	const FGuid OldGalaxy = Sim->GetState().GalaxyID;
	Controller->SelectSystem(Sim->GetState().Systems[0].SystemID);
	Controller->StartNewRun();
	TestNotEqual(TEXT("A new run is another galaxy"), Sim->GetState().GalaxyID, OldGalaxy);
	TestEqual(TEXT("at tick 0"), Sim->GetState().Tick, static_cast<int64>(0));
	TestEqual(TEXT("not over"), Controller->GetRunEnd(), ECosmicProvisionalEnd::None);
	TestFalse(TEXT("with the page folded"), Page->IsOpen());
	TestEqual(TEXT("at x1"), Sim->GetTimeScale(), 1.f);
	Screen.Map->RefreshNow();
	TestTrue(TEXT("and no system selected once the map is rebuilt"), Controller->GetSelectedSystem().IsNone());
	Sim->SetTimeScale(0.f);

	// (10) With the switch off, tick 6,000 is a tick like any other.
	Controller->SetProvisionalEndEnabled(false);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Quiet(*Screen.Overlay);
	Sim->GetMutableState().Tick = EndTick - 3;
	Controller->SetSpeed(10.f);
	Sim->Tick(1.f);
	TestTrue(TEXT("With the end switched off the run passes the end tick"), Sim->GetState().Tick > EndTick);
	TestFalse(TEXT("and is not over"), Controller->IsRunEnded());
	TestEqual(TEXT("and keeps its speed"), Sim->GetTimeScale(), 10.f);
	Sim->SetTimeScale(0.f);
	Controller->SetProvisionalEndEnabled(true);

	// (11) AcquireSim forgets a latch an earlier test left, and its reason.
	Controller->FollowEnd(false);
	TestTrue(TEXT("(latched past the end)"), Controller->IsRunEnded() && Controller->GetAutoPauseReason() == ECosmicAutoPause::RunEnd);
	CosmicTests::AcquireSim(*this);
	TestEqual(TEXT("AcquireSim forgets the latch"), Controller->GetRunEnd(), ECosmicProvisionalEnd::None);
	TestTrue(TEXT("and the end's reason"), Controller->GetAutoPauseReason() == ECosmicAutoPause::None);
	TestFalse(TEXT("and folds the page"), Page->IsOpen());

	// (6) The third rung's end, with the presentation: the clock is switched off (the climb may pass tick 6,000), the climb
	// to the second rung runs headless, then the stabilization of a supernova is stepped with the broadcast on - SetSpeed(10)
	// before each step, as AcquireSim left the speed at 0 (no pause and no presentation would stop anything).
	FCosmicRulesDef NoClock = Sim->GetRules();
	NoClock.ProvisionalEndTick = 0;
	Sim->SetTables(CosmicTests::WithRules(Original, NoClock));
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Quiet(*Screen.Overlay);
	Sim->SetTimeScale(0.f);
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	FGalaxyState& Climb = Sim->GetMutableState();
	const int32 ClimbCiv = CosmicPetitions::RuledCivIndexOf(Climb, Sim->GetStateIndex());
	const int32 ClimbHome = Climb.Civs.IsValidIndex(ClimbCiv) ? Sim->GetStateIndex().FindSystem(Climb, Climb.Civs[ClimbCiv].HomeSystemID) : INDEX_NONE;
	const FHazardDef* NovaRow = Sim->GetTables().FindHazardByTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.PreSupernova"))));
	if (!TestTrue(TEXT("(the climb has a capital and the supernova row)"), Climb.Systems.IsValidIndex(ClimbHome) && NovaRow && Role))
	{
		return false;
	}
	TestTrue(TEXT("(the project key starts the Dyson swarm)"), Role->StartNextMegaproject());
	Sim->SetBroadcastEnabled(false);
	for (int32 Chunk = 0; Chunk < 60 && Climb.Civs[ClimbCiv].Tier == EAscensionTier::Tier1_Planetary; ++Chunk)
	{
		Sim->RunHeadless(100);
	}
	if (!TestEqual(TEXT("(the people climbs to the second rung)"), Climb.Civs[ClimbCiv].Tier, EAscensionTier::Tier2_Stellar))
	{
		return false;
	}
	Climb.Systems[ClimbHome].ActiveHazard = NovaRow->HazardTag;
	Climb.Systems[ClimbHome].HazardTicksRemaining = NovaRow->CountdownTicks;
	Climb.Systems[ClimbHome].ResponseMegaproject = NAME_None;
	Climb.Systems[ClimbHome].ResponseProgress = 0.f;
	TestTrue(TEXT("(the project key answers the supernova)"), Role->StartNextMegaproject());
	const FGalaxyState BeforeThird = Sim->GetState();
	Sim->SetBroadcastEnabled(true);
	for (const bool bAutoPause : { true, false })
	{
		Sim->LoadState(BeforeThird);
		Quiet(*Screen.Overlay);
		Controller->SetAutoPause(bAutoPause);
		const TCHAR* Order = bAutoPause ? TEXT("with the automatic pause on") : TEXT("with the automatic pause off");
		FGalaxyState& Steps = Sim->GetMutableState();
		for (int32 Step = 0; Step < NovaRow->CountdownTicks && Steps.Civs[ClimbCiv].Tier == EAscensionTier::Tier2_Stellar; ++Step)
		{
			Controller->SetSpeed(10.f);
			Sim->StepOnce();
		}
		if (!TestEqual(*FString::Printf(TEXT("(the people climbs to the third rung %s)"), Order), Steps.Civs[ClimbCiv].Tier, EAscensionTier::Tier3_Causal))
		{
			return false;
		}
		TestEqual(*FString::Printf(TEXT("The third rung ends the run %s"), Order), Controller->GetRunEnd(), ECosmicProvisionalEnd::Rung);
		TestTrue(*FString::Printf(TEXT("(the presentation shows %s)"), Order), Screen.Overlay->IsShowing());
		// On: the mythic line's pause came first, so the presentation held 0. Off: the presentation held x10.
		TestEqual(*FString::Printf(TEXT("The presentation holds the speed it found %s"), Order), Screen.Overlay->GetPausedSpeed(), bAutoPause ? 0.f : 10.f);
		TestTrue(*FString::Printf(TEXT("and the reason is the end %s"), Order), Controller->GetAutoPauseReason() == ECosmicAutoPause::RunEnd);
		Controller->AdvanceRunEnd(10.f);
		TestFalse(*FString::Printf(TEXT("While the presentation shows the page waits %s"), Order), Page->IsOpen());
		Screen.Overlay->Close();
		TestEqual(*FString::Printf(TEXT("Closing the presentation gives no speed back %s"), Order), Sim->GetTimeScale(), 0.f);
		if (Screen.Overlay->IsRevealing())
		{
			Controller->AdvanceRunEnd(10.f);
			TestFalse(*FString::Printf(TEXT("and the reveal is waited for %s"), Order), Page->IsOpen());
		}
		Screen.Overlay->Advance(CosmicAscensionOverlay::RevealSeconds + 1.f);
		Controller->AdvanceRunEnd(CosmicChronicleScreen::HoldSeconds + 0.1f);
		TestTrue(*FString::Printf(TEXT("Then the page opens %s"), Order), Page->IsOpen());
	}
	return true;
}

// The chronicle's page and the top bar's words (MVP item 4): the page in the order of the document (the life's title and
// facts, its lines with stamps always in cycles and two opacities, the epilogue's two lines and the horizon last), the
// cause notes, the preview, the reveal, the confirm of 'new run', and the bar's 'chronicle', rule, notice and line after
// a deposition.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIChronicleTest, "Cosmic_Ascension.UI.Chronicle", CosmicTests::Flags)
bool FCosmicUIChronicleTest::RunTest(const FString& Parameters)
{
	using namespace CosmicChronicleScreenTests;
	using CosmicUI::Label;

	FScreen Screen;
	if (!Acquire(*this, Screen))
	{
		return false;
	}
	UGalaxySimulationSubsystem* Sim = Screen.Sim;
	ACosmicPlayerController* Controller = Screen.Controller;
	UCosmicChronicleWidget* Page = Screen.Page;
	UCosmicTopBarWidget* Bar = Screen.Bar;
	const UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	const ECosmicTimeUnit ViewerUnit = Controller->GetTimeUnit();
	ON_SCOPE_EXIT
	{
		Controller->SetTimeUnit(ViewerUnit);
		Sim->SetTimeScale(0.f);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Controller->SetTimeUnit(ECosmicTimeUnit::Cycle);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Quiet(*Screen.Overlay);
	Sim->RunHeadless(1500);
	FGalaxyState& Live = Sim->GetMutableState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const int32 Ruled = CosmicPetitions::RuledCivIndexOf(Live, Index);
	if (!TestNotNull(TEXT("(the log subsystem)"), Log) || !TestTrue(TEXT("(a life rules a people)"), Ruled != INDEX_NONE && Live.Calling.Lives.Num() == 1))
	{
		return false;
	}
	const TArray<FCosmicChronicleChapter> Chapters = Log->BuildChronicle();
	if (!TestEqual(TEXT("(one chapter)"), Chapters.Num(), 1))
	{
		return false;
	}
	const FCosmicChronicleChapter& Chapter = Chapters[0];
	const FLifeRecord& Life = Live.Calling.Lives[0];

	// ---- The page, in the order of the document ----
	Page->Open(ECosmicProvisionalEnd::Clock, true);
	const TArray<FText> Texts = Page->GetPageTexts();
	const int32 Expected = 2 + Chapter.Lines.Num() + (Chapter.bMore ? 1 : 0) + 3;
	if (!TestEqual(TEXT("The page has the title, the facts, the lines, the 'more' line and three closing lines"), Texts.Num(), Expected))
	{
		return false;
	}
	const FText Nth = FText::Format(Label(TEXT("Chronicle.Life")), FText::AsNumber(1));
	const FText Name = CosmicChronicle::VesselNameOf(Life, Sim->GetTables());
	TestEqual(TEXT("The title names the first life"), Texts[0].ToString(), (Name.IsEmpty() ? Nth : FText::Format(Label(TEXT("List.Pair")), Nth, Name)).ToString());
	const FText Rung = FText::Format(INVTEXT("{0} {1}"), Label(TEXT("Panel.Tier")), FText::AsNumber(static_cast<int32>(Life.PeakTier) + 1));
	const FText Lasted = FText::Format(Label(TEXT("Chronicle.Stamp")), FText::AsNumber(Live.Tick - Life.BornTick));
	TestEqual(TEXT("The facts: the people, the rung, the cycles"), Texts[1].ToString(), CosmicUI::JoinList({ Live.Civs[Ruled].CivName, Rung, Lasted }).ToString());
	const TArray<FText> Stamps = Page->GetLineStamps();
	const TArray<float> Opacities = Page->GetLineOpacities();
	bool bOwnSeen = false;
	bool bWorldSeen = false;
	if (TestEqual(TEXT("One stamp and one opacity per line"), Stamps.Num(), Chapter.Lines.Num()) && TestEqual(TEXT("(opacities)"), Opacities.Num(), Chapter.Lines.Num()))
	{
		for (int32 At = 0; At < Chapter.Lines.Num(); ++At)
		{
			const FCosmicChronicleLine& Line = Chapter.Lines[At];
			TestEqual(TEXT("A line's message in order"), Texts[2 + At].ToString(), Line.Entry.Message.ToString());
			TestEqual(TEXT("its stamp in cycles"), Stamps[At].ToString(), FText::Format(Label(TEXT("Chronicle.Stamp")), FText::AsNumber(Line.Entry.Tick)).ToString());
			TestEqual(TEXT("its opacity: 1 for the life's own, 0.7 for the world's"), Opacities[At], Line.bOwn ? 1.f : CosmicChronicleScreen::WorldLineOpacity);
			bOwnSeen |= Line.bOwn;
			bWorldSeen |= !Line.bOwn;
		}
	}
	TestTrue(TEXT("(the page has lines of both kinds)"), bOwnSeen && bWorldSeen);
	if (Chapter.bMore)
	{
		TestEqual(TEXT("The 'more' line closes the chapter"), Texts[2 + Chapter.Lines.Num()].ToString(), Words(TEXT("Chronicle.More")));
	}
	TestEqual(TEXT("The epilogue's first line"), Texts[Expected - 3].ToString(), Words(TEXT("Chronicle.Opened")));
	TestEqual(TEXT("its second"), Texts[Expected - 2].ToString(), Words(TEXT("Chronicle.Unopened")));
	TestEqual(TEXT("and the horizon line last"), Texts[Expected - 1].ToString(), Words(TEXT("Chronicle.Horizon")));
	// The stamps stay in cycles in the viewer's years.
	Controller->SetTimeUnit(ECosmicTimeUnit::Year);
	Page->Open(ECosmicProvisionalEnd::Clock, true);
	TestTrue(TEXT("In years the stamps are still cycles"), Page->GetLineStamps().Num() == Stamps.Num()
		&& (Stamps.IsEmpty() || Page->GetLineStamps()[0].ToString() == Stamps[0].ToString()));
	Controller->SetTimeUnit(ECosmicTimeUnit::Cycle);

	// ---- The cause notes ----
	TestEqual(TEXT("The clock's note"), Shown(Page->GetNoteText()), FText::Format(Label(TEXT("Chronicle.Note.Clock")), FText::AsNumber(Live.Tick)).ToString());
	TestEqual(TEXT("and the provisional notice"), Shown(Page->GetProvisionalText()), Words(TEXT("Chronicle.Provisional")));
	// The rung's note names the people that climbed: of the ruled peoples from the last, one on the rung - not the birth people.
	const int32 Other = Live.Civs.IndexOfByPredicate([&Live, Ruled](const FCivilizationState& Civ) { return Civ.CivID != Live.Civs[Ruled].CivID; });
	if (TestTrue(TEXT("(another people)"), Other != INDEX_NONE))
	{
		FLifeRecord& Mutable = Live.Calling.Lives[0];
		Mutable.PeakTier = EAscensionTier::Tier3_Causal;
		Live.Civs[Ruled].Tier = EAscensionTier::Tier2_Stellar;
		Page->Open(ECosmicProvisionalEnd::Rung, true);
		TestEqual(TEXT("With no ruled people on the rung the note names none"), Shown(Page->GetNoteText()), Words(TEXT("Chronicle.Note.RungNoCiv")));
		Mutable.RuledCivIDs.Add(Live.Civs[Other].CivID);
		Live.Civs[Other].Tier = EAscensionTier::Tier3_Causal;
		Page->Open(ECosmicProvisionalEnd::Rung, true);
		FFormatNamedArguments Args;
		Args.Add(TEXT("Civ"), Live.Civs[Other].CivName);
		TestEqual(TEXT("The rung's note names the ruled people on the rung"), Shown(Page->GetNoteText()), FText::Format(Label(TEXT("Chronicle.Note.Rung")), Args).ToString());
		TestEqual(TEXT("and the provisional notice follows"), Shown(Page->GetProvisionalText()), Words(TEXT("Chronicle.Provisional")));
		Mutable.RuledCivIDs.Pop();
		Mutable.PeakTier = EAscensionTier::Tier1_Planetary;
		Live.Civs[Ruled].Tier = EAscensionTier::Tier1_Planetary;
	}
	// A preview has no cause: no note, no notice, no 'new run'.
	Page->Open(ECosmicProvisionalEnd::None, true);
	TestTrue(TEXT("A preview has no note"), Shown(Page->GetNoteText()).IsEmpty());
	TestTrue(TEXT("no provisional notice"), Shown(Page->GetProvisionalText()).IsEmpty());
	TestFalse(TEXT("and no 'new run'"), IsShownWidget(Page->GetNewRunButton()));

	// ---- The reveal: the buttons only once every line shows ----
	Page->Open(ECosmicProvisionalEnd::Clock, false);
	TestTrue(TEXT("Opened with a reveal the page reveals"), Page->IsRevealing());
	TestFalse(TEXT("and hides its buttons"), IsShownWidget(ButtonRowOf(*Page)));
	Page->Advance(CosmicChronicleScreen::VeilSeconds + 0.01f);
	TestTrue(TEXT("(still revealing after the veil)"), Page->IsRevealing() && !IsShownWidget(ButtonRowOf(*Page)));
	Page->PressNewRun();
	TestEqual(TEXT("'New run' takes no press while the lines show"), Page->GetNewRunButton()->GetLabel().ToString(), Words(TEXT("Chronicle.NewRun")));
	Page->Advance(CosmicChronicleScreen::RevealCapSeconds + 1.f);
	TestFalse(TEXT("Within the cap every line shows"), Page->IsRevealing());
	TestTrue(TEXT("and the buttons with them"), IsShownWidget(ButtonRowOf(*Page)));
	Page->Open(ECosmicProvisionalEnd::Clock, false);
	Page->PressVeil();
	TestTrue(TEXT("A press on the veil shows every line at once"), !Page->IsRevealing() && IsShownWidget(ButtonRowOf(*Page)));

	// ---- 'New run' asks twice ----
	const FGuid Galaxy = Live.GalaxyID;
	Page->PressNewRun();
	TestEqual(TEXT("The first press changes the words"), Page->GetNewRunButton()->GetLabel().ToString(), Words(TEXT("Chronicle.NewRunConfirm")));
	TestEqual(TEXT("and starts nothing"), Sim->GetState().GalaxyID, Galaxy);
	Page->Advance(CosmicChronicleScreen::ConfirmSeconds + 0.1f);
	TestEqual(TEXT("After the confirm window the words come back"), Page->GetNewRunButton()->GetLabel().ToString(), Words(TEXT("Chronicle.NewRun")));
	Page->PressNewRun();
	TestEqual(TEXT("(asked again)"), Sim->GetState().GalaxyID, Galaxy);
	Page->PressNewRun();
	TestNotEqual(TEXT("A second press within the window starts a new run"), Sim->GetState().GalaxyID, Galaxy);
	TestFalse(TEXT("and folds the page"), Page->IsOpen());
	Sim->SetTimeScale(0.f);

	// ---- The top bar ----
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Quiet(*Screen.Overlay);
	Sim->SetTimeScale(0.f);
	FGalaxyState& Bar42 = Sim->GetMutableState();
	const int32 EndTick = Sim->GetRules().ProvisionalEndTick;
	// The clock's tooltip states the rule: in cycles, the rung alone without a clock end, nothing with the end off.
	FString Third = UCosmicTopBarWidget::ClockTipOf(EndTick, true).ToString();
	Third.Split(TEXT("\n"), nullptr, &Third, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
	TestEqual(TEXT("The clock's tooltip ends with the run's rule"), Third, FText::Format(Label(TEXT("Chronicle.Rule")), CosmicUI::ClockLabel(EndTick, ECosmicTimeUnit::Cycle)).ToString());
	TestTrue(TEXT("(which names the end tick in cycles)"), Third.Contains(CosmicUI::ClockLabel(EndTick, ECosmicTimeUnit::Cycle).ToString()));
	FString RungOnly = UCosmicTopBarWidget::ClockTipOf(0, true).ToString();
	RungOnly.Split(TEXT("\n"), nullptr, &RungOnly, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
	TestEqual(TEXT("Without a clock end the rule names the rung alone"), RungOnly, Words(TEXT("Chronicle.RuleRung")));
	TestFalse(TEXT("With the end off the tooltip states no rule"), UCosmicTopBarWidget::ClockTipOf(EndTick, false).ToString().Contains(Words(TEXT("Chronicle.RuleRung")))
		|| UCosmicTopBarWidget::ClockTipOf(EndTick, false).ToString().Contains(CosmicUI::ClockLabel(EndTick, ECosmicTimeUnit::Cycle).ToString()));
	TestEqual(TEXT("The bar's clock carries that tooltip"), Bar->GetClockButton() ? Bar->GetClockButton()->GetToolTipText().ToString() : FString(), UCosmicTopBarWidget::ClockTipOf(EndTick, true).ToString());
	// The notice: none before the last EndSoonTicks, then the time left.
	Bar42.Tick = EndTick - CosmicChronicleScreen::EndSoonTicks - 1;
	Bar->Refresh();
	TestTrue(TEXT("One tick before the notice's window the bar says nothing"), Shown(Bar->GetEndSoonText()).IsEmpty());
	Bar42.Tick = EndTick - CosmicChronicleScreen::EndSoonTicks;
	Bar->Refresh();
	TestEqual(TEXT("In it the bar says the run stops soon"), Shown(Bar->GetEndSoonText()),
		FText::Format(Label(TEXT("Chronicle.Soon")), CosmicUI::TimeLeftLabel(CosmicChronicleScreen::EndSoonTicks, ECosmicTimeUnit::Cycle)).ToString());
	// After a deposition, the clock end is told.
	Bar42.Tick = 3000;
	FLifeRecord& Deposed = Bar42.Calling.Lives.Last();
	Deposed.ReignEndReasons.Reset();
	Deposed.ReignEndReasons.Add(FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Deposed"))));
	Bar->Refresh();
	TestEqual(TEXT("After a deposition the bar tells the end tick"), Shown(Bar->GetEndSoonText()),
		FText::Format(Label(TEXT("Chronicle.Until")), CosmicUI::ClockLabel(EndTick, ECosmicTimeUnit::Cycle)).ToString());
	TestFalse(TEXT("No 'chronicle' before the end"), IsShownWidget(Bar->GetReopenButton()));
	// After the end: no notice; 'chronicle' only while the page is folded, and a press opens it.
	Bar42.Tick = EndTick;
	Controller->FollowEnd(false);
	Bar->Refresh();
	TestTrue(TEXT("After the end the bar says nothing more"), Shown(Bar->GetEndSoonText()).IsEmpty());
	TestFalse(TEXT("No 'chronicle' while the page shows"), IsShownWidget(Bar->GetReopenButton()));
	Controller->LookAtMap();
	Bar->Refresh();
	TestTrue(TEXT("'Chronicle' once the page is folded"), IsShownWidget(Bar->GetReopenButton()));
	if (Bar->GetReopenButton())
	{
		Bar->GetReopenButton()->Press();
	}
	TestTrue(TEXT("and its press opens the page"), Page->IsOpen());
	Controller->ClearRunEnd();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

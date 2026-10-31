// Cosmic Ascension - automation tests of the provisional end's presentation (MVP item 4, commit A-4b): the controller's
// latch and its lock, the chronicle's page and the top bar's words. Design: docs/60_DECISIONS/headers/A4_provisional_end.md
// section 5, 7; docs/70_CONTENT_GUIDE.md section 8.4.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Events/CosmicPetitions.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "Internationalization/StringTableCore.h"
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
#include "UI/CosmicLogFeedWidget.h"
#include "UI/CosmicPalette.h"
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
// presentation (no end since B6a) and the clock's end over the presentation, an ended load, FollowEnd's contract, the new
// run, the switch, the reset of AcquireSim and the two hashes.
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

	// (6) The third rung with the presentation, which ends no run (B6a decision 2-ga), and the end over the presentation:
	// the clock is switched off (the climb may pass tick 6,000), the climb to the second rung runs headless, then the
	// stabilization of a supernova is stepped with the broadcast on - SetSpeed(10) before each step, as AcquireSim left the
	// speed at 0 (no pause and no presentation would stop anything).
	FCosmicRulesDef NoClock = Sim->GetRules();
	NoClock.ProvisionalEndTick = 0;
	Sim->SetTables(CosmicTests::WithRules(Original, NoClock));
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	// The player's first act (B2): the inherited edict revoked - kept, it ends the reign before the climb does.
	CosmicTests::RevokeInheritedEdict(*Sim);
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
	// The tick of the climb, measured once from the same state without the broadcast (a handler writes nothing, so the
	// stepped runs below climb in the same tick); the end over the presentation puts the clock there.
	for (int32 Step = 0; Step < NovaRow->CountdownTicks && Climb.Civs[ClimbCiv].Tier == EAscensionTier::Tier2_Stellar; ++Step)
	{
		Sim->RunHeadless(1);
	}
	const int64 ClimbTick = Climb.Civs[ClimbCiv].Tier == EAscensionTier::Tier3_Causal ? Climb.Tick : int64(-1);
	if (!TestTrue(TEXT("(the climb's tick is measured)"), ClimbTick > BeforeThird.Tick))
	{
		return false;
	}
	// Steps BeforeThird with the broadcast on until the people climbs to the third rung; false when it does not.
	auto StepToThird = [&](bool bAutoPause, const TCHAR* Order)
	{
		Sim->LoadState(BeforeThird);
		Quiet(*Screen.Overlay);
		Controller->SetAutoPause(bAutoPause);
		FGalaxyState& Steps = Sim->GetMutableState();
		for (int32 Step = 0; Step < NovaRow->CountdownTicks && Steps.Civs[ClimbCiv].Tier == EAscensionTier::Tier2_Stellar; ++Step)
		{
			Controller->SetSpeed(10.f);
			Sim->StepOnce();
		}
		return TestEqual(*FString::Printf(TEXT("(the people climbs to the third rung %s)"), Order), Steps.Civs[ClimbCiv].Tier, EAscensionTier::Tier3_Causal)
			&& TestEqual(*FString::Printf(TEXT("(in the measured tick %s)"), Order), Steps.Tick, ClimbTick);
	};
	Sim->SetBroadcastEnabled(true);
	// The third rung ends no run: the presentation and its reveal stand, nothing latches and no page follows them.
	for (const bool bAutoPause : { true, false })
	{
		const TCHAR* Order = bAutoPause ? TEXT("with the automatic pause on") : TEXT("with the automatic pause off");
		if (!StepToThird(bAutoPause, Order))
		{
			return false;
		}
		TestFalse(*FString::Printf(TEXT("The third rung ends no run %s"), Order), Controller->IsRunEnded());
		TestTrue(*FString::Printf(TEXT("the presentation shows %s"), Order), Screen.Overlay->IsShowing());
		// On: the mythic line's pause came first, so the presentation held 0. Off: the presentation held x10.
		TestEqual(*FString::Printf(TEXT("and holds the speed it found %s"), Order), Screen.Overlay->GetPausedSpeed(), bAutoPause ? 0.f : 10.f);
		TestTrue(*FString::Printf(TEXT("for no reason of an end %s"), Order), Controller->GetAutoPauseReason() != ECosmicAutoPause::RunEnd);
		Screen.Overlay->Close();
		Screen.Overlay->Advance(CosmicAscensionOverlay::RevealSeconds + 1.f);
		Controller->AdvanceRunEnd(CosmicChronicleScreen::HoldSeconds + 0.1f);
		TestFalse(*FString::Printf(TEXT("After the presentation and the reveal no page opens %s"), Order), Page->IsOpen() || Controller->IsRunEnded());
		Sim->SetTimeScale(0.f);
	}
	// The end over the presentation: the clock put at the tick of the climb ends the run in that tick.
	FCosmicRulesDef AtClimb = NoClock;
	AtClimb.ProvisionalEndTick = static_cast<int32>(ClimbTick);
	Sim->SetTables(CosmicTests::WithRules(Original, AtClimb));
	for (const bool bAutoPause : { true, false })
	{
		const TCHAR* Order = bAutoPause ? TEXT("with the automatic pause on") : TEXT("with the automatic pause off");
		if (!StepToThird(bAutoPause, Order))
		{
			return false;
		}
		TestEqual(*FString::Printf(TEXT("The clock at the climb's tick ends the run %s"), Order), Controller->GetRunEnd(), ECosmicProvisionalEnd::Clock);
		TestTrue(*FString::Printf(TEXT("(the presentation shows %s)"), Order), Screen.Overlay->IsShowing());
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
// cause notes, the preview, the reveal, the confirm of 'new run', and the bar's 'chronicle', rule, notice and line for a
// wanderer. B5 (header section 2.7, 4): the clock end told to every embodied wanderer and to no ruler, no vessel's line in
// the interregnum, the span with no vessel under its sub-head at the head of the next chapter, the facts of a life that
// never ruled ('born among - wandering') and of a ruler that died on its throne (no 'wandering'), no chapter for a life born
// on the end's tick.
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
	// The player's first act (B2): the inherited edict revoked - kept, it ends the reign before this path does.
	CosmicTests::RevokeInheritedEdict(*Sim);
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
	// The two books' chapter (B5 - CosmicChronicle::BuildTwoBooks) stands between the chapter and the fixed lines when the run
	// lost a settled system; its rows and lines are counted here and read in the B5 part below.
	const FCosmicTwoBooks LiveBooks = CosmicChronicle::BuildTwoBooks(Live);
	int32 BookLines = LiveBooks.Recorded.Num();
	for (const FCosmicClaimedGroup& Group : LiveBooks.Claimed)
	{
		BookLines += Group.Lines.Num();
	}
	const int32 Expected = 2 + Chapter.Lines.Num() + (Chapter.bMore ? 1 : 0) + LiveBooks.NumRows() + 3;
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
	if (TestEqual(TEXT("One stamp and one opacity per line"), Stamps.Num(), Chapter.Lines.Num() + BookLines) && TestEqual(TEXT("(opacities)"), Opacities.Num(), Chapter.Lines.Num() + BookLines))
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
	// The rung's note, which no end gives any more (the third rung ends no run - B6a [B-6a3a]): the line that names no
	// people, whoever is on the rung. Naming the people comes back in [B-6a3c] from the ending record's CivID (B6a header
	// section 2.7).
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
		TestEqual(TEXT("With a ruled people on the rung the note names none either"), Shown(Page->GetNoteText()), Words(TEXT("Chronicle.Note.RungNoCiv")));
		TestEqual(TEXT("and the provisional notice follows"), Shown(Page->GetProvisionalText()), Words(TEXT("Chronicle.Provisional")));
		Mutable.RuledCivIDs.Pop();
		Mutable.PeakTier = EAscensionTier::Tier1_Planetary;
		Live.Civs[Ruled].Tier = EAscensionTier::Tier1_Planetary;

		// The facts of a life that left one throne, took another and left it too (B1): the peoples in order, then 'wandering'.
		const FGameplayTag Left = FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Abdicated")));
		const TArray<FGameplayTag> EndsBefore = Mutable.ReignEndReasons;
		const TArray<int64> TicksBefore = Mutable.ReignTicks;
		Mutable.ReignEndReasons.Last() = Left;
		Mutable.RuledCivIDs.Add(Live.Civs[Other].CivID);
		Mutable.ReignEndReasons.Add(Left);
		Mutable.ReignTicks.Add(10);
		Page->Open(ECosmicProvisionalEnd::Clock, true);
		const FText Peoples = FText::Format(Label(TEXT("Chronicle.Then")), Live.Civs[Ruled].CivName, Live.Civs[Other].CivName);
		TestEqual(TEXT("The facts name the peoples in order and the walking end"), Page->GetPageTexts()[1].ToString(),
			CosmicUI::JoinList({ Peoples, Label(TEXT("Chronicle.Wandering")), Rung, Lasted }).ToString());
		Mutable.RuledCivIDs.Pop();
		// A return to the same people (left it, took it again): that people once.
		Mutable.RuledCivIDs.Add(Live.Civs[Ruled].CivID);
		Page->Open(ECosmicProvisionalEnd::Clock, true);
		TestEqual(TEXT("A return to the same people names it once"), Page->GetPageTexts()[1].ToString(),
			CosmicUI::JoinList({ Live.Civs[Ruled].CivName, Label(TEXT("Chronicle.Wandering")), Rung, Lasted }).ToString());
		Mutable.RuledCivIDs.Pop();
		Mutable.ReignEndReasons = EndsBefore;
		Mutable.ReignTicks = TicksBefore;
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
	// A ruler is told no clock end before the notice's window (its people may end the run at the rung).
	Bar42.Tick = 3000;
	Bar->Refresh();
	TestTrue(TEXT("A ruler's bar tells no end tick"), Shown(Bar->GetEndSoonText()).IsEmpty());
	// Every embodied wanderer is told the clock end (B5 header section 4): after a deposition, and a vessel that never ruled.
	const FString Until = FText::Format(Label(TEXT("Chronicle.Until")), CosmicUI::ClockLabel(EndTick, ECosmicTimeUnit::Cycle)).ToString();
	const FName RuledID = Bar42.Player.RuledCivID;
	FLifeRecord& Deposed = Bar42.Calling.Lives.Last();
	Deposed.ReignEndReasons.Reset();
	Deposed.ReignEndReasons.Add(FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Deposed"))));
	Bar42.Player.Role = EPlayerRole::Wanderer;
	Bar42.Player.RuledCivID = NAME_None;
	Bar->Refresh();
	TestEqual(TEXT("After a deposition the bar tells the end tick"), Shown(Bar->GetEndSoonText()), Until);
	const FLifeRecord Reigned = Deposed;
	Deposed.RuledCivIDs.Reset();
	Deposed.ReignEndReasons.Reset();
	Deposed.ReignTicks.Reset();
	Bar->Refresh();
	TestEqual(TEXT("and so it does for a vessel that never ruled (B5)"), Shown(Bar->GetEndSoonText()), Until);
	// The interregnum: no vessel, so no vessel's line and no end tick told before the notice's window.
	Bar42.Calling.LifeState = ELifeState::Interregnum;
	Bar->Refresh();
	TestTrue(TEXT("In the interregnum the bar tells no end tick"), Shown(Bar->GetEndSoonText()).IsEmpty());
	TestTrue(TEXT("nor any vessel's line"), Shown(Bar->GetWandererText()).IsEmpty() && Shown(Bar->GetLethalText()).IsEmpty() && Shown(Bar->GetRungText()).IsEmpty());
	Bar42.Player.Role = EPlayerRole::Ruler;
	Bar42.Player.RuledCivID = RuledID;
	Bar->Refresh();
	TestTrue(TEXT("A people the dead ruler held is no line of the bar in the interregnum"), Shown(Bar->GetRungText()).IsEmpty() && Shown(Bar->GetStatusText()).IsEmpty());
	Bar42.Calling.LifeState = ELifeState::Embodied;
	Bar42.Player.Role = EPlayerRole::Wanderer;
	Bar42.Player.RuledCivID = NAME_None;
	Deposed = Reigned;
	Bar->Refresh();
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

	// ---- B5 (header section 2.7, 4): two lives - the span with no vessel at the head of the next chapter under its sub-head,
	// the facts of a life that never ruled and of a ruler that died on its throne; a life born on the end's tick has no chapter ----
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Quiet(*Screen.Overlay);
	Sim->SetTimeScale(0.f);
	{
		FGalaxyState& Two = Sim->GetMutableState();
		const int32 Own = CosmicPetitions::RuledCivIndexOf(Two, Sim->GetStateIndex());
		const int32 Later = Own == INDEX_NONE ? INDEX_NONE
			: Two.Civs.IndexOfByPredicate([&Two, Own](const FCivilizationState& Civ) { return Civ.CivID != Two.Civs[Own].CivID; });
		if (!TestTrue(TEXT("(a ruled people and another)"), Own != INDEX_NONE && Later != INDEX_NONE && Two.Calling.Lives.Num() == 1
			&& !Two.Calling.Lives[0].ReignEndReasons.IsEmpty()))
		{
			return false;
		}
		for (const TCHAR* Key : { TEXT("Chronicle.NoVessel"), TEXT("Chronicle.BornAmong"), TEXT("Chronicle.BookRecorded"), TEXT("Chronicle.BookClaimed"),
			TEXT("Chronicle.ClaimedBy") })
		{
			TestFalse(*FString::Printf(TEXT("%s has words in the String Table"), Key), Words(Key).IsEmpty() || Words(Key).Contains(FStringTableEntry::GetPlaceholderSourceString()));
		}
		const FName OwnID = Two.Civs[Own].CivID;
		const FName LaterID = Two.Civs[Later].CivID;
		// The first life died on its throne at 1,000; a called vessel of the other people, never crowned, was born at 1,200.
		Two.Tick = 1500;
		Two.Calling.Lives[0].DiedTick = 1000;
		Two.Calling.Lives[0].ReignEndReasons.Last() = CosmicTags::ReignDied();
		FLifeRecord Called;
		Called.LifeIndex = 2;
		Called.BornTick = 1200;
		Called.BirthSystemID = Two.Civs[Later].HomeSystemID;
		Called.BirthCivID = LaterID;
		Called.BirthArchetypeRow = Two.Civs[Later].ArchetypeRow;
		Two.Calling.Lives.Add(Called);
		Two.Chronicle.Reset();
		Two.Log.Reset();
		auto Told = [](int64 Tick, const TCHAR* Category, uint8 Severity, FName Civ, const TCHAR* Message)
		{
			FCosmicLogEntry Entry;
			Entry.Tick = Tick;
			Entry.Category = FGameplayTag::RequestGameplayTag(FName(Category));
			Entry.Severity = Severity;
			Entry.FocusCivID = Civ;
			Entry.Message = FText::AsCultureInvariant(Message);
			return Entry;
		};
		Two.Log.Add(Told(900, TEXT("Log.Civ"), 2, OwnID, TEXT("b5 first own")));
		Two.Log.Add(Told(1100, TEXT("Log.Civ"), 2, LaterID, TEXT("b5 between")));
		// One loss while no vessel stood: its record (part one) and the claim over it (part two, by trans_silence - B4's
		// SourceRow of a first-person line).
		Two.Log.Add(Told(1150, TEXT("Log.Record"), 1, LaterID, TEXT("b5 record between")));
		Two.Log.Add(Told(1150, TEXT("Log.Claim"), 1, LaterID, TEXT("b5 claim between")));
		Two.Log.Last().SourceRow = CosmicRows::TransSilence;
		Two.Log.Add(Told(1300, TEXT("Log.Civ"), 2, LaterID, TEXT("b5 second own")));

		auto TitleOf = [&](const FLifeRecord& OfLife)
		{
			const FText OfNth = FText::Format(Label(TEXT("Chronicle.Life")), FText::AsNumber(OfLife.LifeIndex));
			const FText OfName = CosmicChronicle::VesselNameOf(OfLife, Sim->GetTables());
			return (OfName.IsEmpty() ? OfNth : FText::Format(Label(TEXT("List.Pair")), OfNth, OfName)).ToString();
		};
		const FText RungOne = FText::Format(INVTEXT("{0} {1}"), Label(TEXT("Panel.Tier")), FText::AsNumber(1));
		auto LastedOf = [&](int64 Ticks) { return FText::Format(Label(TEXT("Chronicle.Stamp")), FText::AsNumber(Ticks)); };
		const FString FirstFacts = CosmicUI::JoinList({ Two.Civs[Own].CivName, RungOne, LastedOf(1000) }).ToString();
		FFormatNamedArguments Born;
		Born.Add(TEXT("Civ"), Two.Civs[Later].CivName);
		const FString CalledFacts = CosmicUI::JoinList({ FText::Format(Label(TEXT("Chronicle.BornAmong")), Born), Label(TEXT("Chronicle.Wandering")), RungOne,
			LastedOf(300) }).ToString();
		auto PageOf = [&]()
		{
			Page->Open(ECosmicProvisionalEnd::Clock, true);
			TArray<FString> Out;
			for (const FText& Row : Page->GetPageTexts())
			{
				Out.Add(Row.ToString());
			}
			return Out;
		};

		// The two books (B5 header section 4, decisions 10 and 11): after the last chapter, before the fixed lines; part one,
		// then part two with its claimant's head.
		const FText SilenceTitle = CosmicChronicle::ClaimantTitleOf(CosmicRows::TransSilence, Sim->GetTables());
		TestFalse(TEXT("(trans_silence has a title)"), SilenceTitle.IsEmpty());
		FFormatNamedArguments ByArgs;
		ByArgs.Add(TEXT("Transcendent"), SilenceTitle);
		const FString BySilence = FText::Format(Label(TEXT("Chronicle.ClaimedBy")), ByArgs).ToString();
		const TArray<FString> Shown2 = PageOf();
		TestEqual(TEXT("Two chapters: the span with no vessel heads the second under its sub-head, before its body; the two books' entries are not there but in their own chapter after the last, before the fixed lines"), Shown2,
			TArray<FString>({ TitleOf(Two.Calling.Lives[0]), FirstFacts, TEXT("b5 first own"),
				TitleOf(Two.Calling.Lives[1]), CalledFacts, Words(TEXT("Chronicle.NoVessel")), TEXT("b5 between"), TEXT("b5 second own"),
				Words(TEXT("Chronicle.BookRecorded")), TEXT("b5 record between"), Words(TEXT("Chronicle.BookClaimed")), BySilence, TEXT("b5 claim between"),
				Words(TEXT("Chronicle.Opened")), Words(TEXT("Chronicle.Unopened")), Words(TEXT("Chronicle.Horizon")) }));
		{
			// One colour for the books' lines - a chapter's Severity 1 colour -, the first-person line too (purple is the feed's
			// alone - B4 header), and the same opacity.
			const TArray<FLinearColor> Colors = Page->GetLineColors();
			const TArray<float> BookOpacities = Page->GetLineOpacities();
			if (TestEqual(TEXT("(one colour per line)"), Colors.Num(), 5) && TestEqual(TEXT("(one opacity per line)"), BookOpacities.Num(), 5))
			{
				TestEqual(TEXT("A record stands in a chapter's Severity 1 colour"), Colors[3], CosmicLogFeed::ColorOf(1, FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Civ")))));
				TestEqual(TEXT("and the first-person line in the same colour"), Colors[4], Colors[3]);
				TestNotEqual(TEXT("not the feed's purple"), Colors[4], CosmicLogFeed::ColorOf(1, CosmicTags::LogClaim()));
				TestTrue(TEXT("both at full opacity"), BookOpacities[3] == 1.f && BookOpacities[4] == 1.f);
			}
		}
		{
			// The two books are one chapter after the lives (reader-2): part one's head is drawn as a life chapter's head - its
			// size, colour and padding -, part two's head under it as its part (grey 18 - part one holds part two, decision
			// 10-ga), a claimant's head under that (grey 14).
			const TArray<CosmicChronicleScreen::FRowLook> Looks = Page->GetRowLooks();
			const int32 RecordedAt = Shown2.IndexOfByKey(Words(TEXT("Chronicle.BookRecorded")));
			const int32 ClaimedAt = Shown2.IndexOfByKey(Words(TEXT("Chronicle.BookClaimed")));
			const int32 ByAt = Shown2.IndexOfByKey(BySilence);
			const int32 LifeHeadAt = Shown2.IndexOfByKey(TitleOf(Two.Calling.Lives[1]));
			if (TestEqual(TEXT("(one look per row)"), Looks.Num(), Shown2.Num()) && TestTrue(TEXT("(the heads stand)"), Looks.IsValidIndex(RecordedAt)
				&& Looks.IsValidIndex(ClaimedAt) && Looks.IsValidIndex(ByAt) && Looks.IsValidIndex(LifeHeadAt) && Looks.IsValidIndex(0)))
			{
				TestTrue(TEXT("Part one's head is a chapter's head: a life chapter's size, colour and padding"), Looks[RecordedAt] == Looks[LifeHeadAt] && Looks[RecordedAt] == Looks[0]);
				TestTrue(TEXT("(the life chapters' heads: gold 26)"), Looks[0].Size == 26 && Looks[0].Color == CosmicPalette::Gold);
				TestTrue(TEXT("Part two's head is a part's head under it: grey 18"), Looks[ClaimedAt].Size == 18 && Looks[ClaimedAt].Color == CosmicPalette::Grey);
				TestTrue(TEXT("and a claimant's head grey 14"), Looks[ByAt].Size == 14 && Looks[ByAt].Color == CosmicPalette::Grey);
				TestTrue(TEXT("(part two's head is smaller than the chapter's and larger than a claimant's)"), Looks[ClaimedAt].Size < Looks[RecordedAt].Size
					&& Looks[ByAt].Size < Looks[ClaimedAt].Size);
			}
		}
		TestFalse(TEXT("A ruler that died on its throne did not walk"), Shown2.IsValidIndex(1) && Shown2[1].Contains(Words(TEXT("Chronicle.Wandering"))));
		TestTrue(TEXT("A life that never ruled says where it was born, not its birth people as if ruled"), Shown2.IsValidIndex(4)
			&& Shown2[4].StartsWith(FText::Format(Label(TEXT("Chronicle.BornAmong")), Born).ToString()));
		TArray<FString> StampWords;
		for (const FText& Stamp : Page->GetLineStamps())
		{
			StampWords.Add(Stamp.ToString());
		}
		TestEqual(TEXT("Every line of the span, of the bodies and of the books has its stamp in cycles"), StampWords,
			TArray<FString>({ LastedOf(900).ToString(), LastedOf(1100).ToString(), LastedOf(1300).ToString(), LastedOf(1150).ToString(), LastedOf(1150).ToString() }));

		// The succession on the end's tick (the end wins): the vessel born there was never shown - no chapter; the span goes to
		// the end of the chapter before.
		Two.Log.Pop();
		Two.Tick = 1200;
		TestEqual(TEXT("A life born on the end's tick gets no chapter; its span closes the chapter before"), PageOf(),
			TArray<FString>({ TitleOf(Two.Calling.Lives[0]), FirstFacts, TEXT("b5 first own"), Words(TEXT("Chronicle.NoVessel")), TEXT("b5 between"),
				Words(TEXT("Chronicle.BookRecorded")), TEXT("b5 record between"), Words(TEXT("Chronicle.BookClaimed")), BySilence, TEXT("b5 claim between"),
				Words(TEXT("Chronicle.Opened")), Words(TEXT("Chronicle.Unopened")), Words(TEXT("Chronicle.Horizon")) }));

		// The two books' chapter as the page shows it: absent when both parts are empty, part one's 'more' over a part two
		// alone, part one alone with no part two's head, and the caps - 12 a part, 28 the chapter - with every claimant's head.
		auto BooksOf = [&](const TArray<FString>& Rows)
		{
			const int32 From = Rows.IndexOfByKey(Words(TEXT("Chronicle.BookRecorded")));
			const int32 To = Rows.IndexOfByKey(Words(TEXT("Chronicle.Opened")));
			TArray<FString> Out;
			for (int32 At = From; From != INDEX_NONE && At < To; ++At)
			{
				Out.Add(Rows[At]);
			}
			return Out;
		};
		auto IsBook = [](const FCosmicLogEntry& Entry) { return Entry.Category == CosmicTags::LogClaim() || Entry.Message.ToString() == TEXT("b5 record between"); };
		auto IsRecord = [](const FCosmicLogEntry& Entry) { return Entry.Message.ToString() == TEXT("b5 record between"); };
		auto IsClaim = [](const FCosmicLogEntry& Entry) { return Entry.Category == CosmicTags::LogClaim(); };
		const TArray<FCosmicLogEntry> Kept = Two.Log;
		Two.Log.RemoveAll(IsBook);
		const TArray<FString> NoBooks = PageOf();
		TestFalse(TEXT("No Log.Record and no Log.Claim: no two books' chapter"), NoBooks.Contains(Words(TEXT("Chronicle.BookRecorded")))
			|| NoBooks.Contains(Words(TEXT("Chronicle.BookClaimed"))) || NoBooks.Contains(BySilence));
		TestEqual(TEXT("and the fixed lines follow the last chapter"), NoBooks.IsValidIndex(NoBooks.Num() - 3) ? NoBooks[NoBooks.Num() - 3] : FString(), Words(TEXT("Chronicle.Opened")));
		Two.Log = Kept;
		Two.Log.RemoveAll(IsRecord);
		TestEqual(TEXT("A part two over an empty part one: part one's head says there was more"), BooksOf(PageOf()),
			TArray<FString>({ Words(TEXT("Chronicle.BookRecorded")), Words(TEXT("Chronicle.More")), Words(TEXT("Chronicle.BookClaimed")), BySilence, TEXT("b5 claim between") }));
		Two.Log = Kept;
		Two.Log.RemoveAll(IsClaim);
		TestEqual(TEXT("A part one with no claim stands alone: no part two's head"), BooksOf(PageOf()),
			TArray<FString>({ Words(TEXT("Chronicle.BookRecorded")), TEXT("b5 record between") }));
		Two.Log = Kept;
		{
			// The caps, with a second claimant (a test row with a title) beside trans_silence.
			const FCosmicDataTables Original = Sim->GetTables();
			ON_SCOPE_EXIT { Sim->SetTables(Original); };
			UDataTable* Transcendents = CosmicTests::CopyWithout<FTranscendentDef>(Original.Transcendents, {});
			if (!TestNotNull(TEXT("(the transcendents' table)"), Transcendents))
			{
				return false;
			}
			const FName OtherClaimant(TEXT("trans_test_other"));
			FTranscendentDef Titled;
			Titled.Title = FText::AsCultureInvariant(TEXT("b5 other claimant"));
			Transcendents->AddRow(OtherClaimant, Titled);
			FCosmicDataTables WithOther = Original;
			WithOther.Transcendents = Transcendents;
			Sim->SetTables(WithOther);
			Two.Tick = 5000;
			for (int32 At = 0; At < 15; ++At)
			{
				Two.Log.Add(Told(2000 + At, TEXT("Log.Record"), 1, LaterID, *FString::Printf(TEXT("b5 record %d"), At)));
			}
			for (int32 At = 0; At < 15; ++At)
			{
				Two.Log.Add(Told(3000 + At, TEXT("Log.Claim"), 1, LaterID, *FString::Printf(TEXT("b5 claim %d"), At)));
				Two.Log.Last().SourceRow = At % 3 == 0 ? OtherClaimant : CosmicRows::TransSilence;
			}
			const FCosmicTwoBooks Books = CosmicChronicle::BuildTwoBooks(Two);
			TArray<FString> ExpectedBooks = { Words(TEXT("Chronicle.BookRecorded")) };
			for (const FCosmicChronicleLine& Line : Books.Recorded)
			{
				ExpectedBooks.Add(Line.Entry.Message.ToString());
			}
			ExpectedBooks.Add(Words(TEXT("Chronicle.More")));
			ExpectedBooks.Add(Words(TEXT("Chronicle.BookClaimed")));
			for (const FCosmicClaimedGroup& Group : Books.Claimed)
			{
				FFormatNamedArguments Of;
				Of.Add(TEXT("Transcendent"), CosmicChronicle::ClaimantTitleOf(Group.TranscendentID, Sim->GetTables()));
				ExpectedBooks.Add(FText::Format(Label(TEXT("Chronicle.ClaimedBy")), Of).ToString());
				for (const FCosmicChronicleLine& Line : Group.Lines)
				{
					ExpectedBooks.Add(Line.Entry.Message.ToString());
				}
			}
			ExpectedBooks.Add(Words(TEXT("Chronicle.More")));
			const TArray<FString> Capped = BooksOf(PageOf());
			TestEqual(TEXT("Over the caps the page shows the books' rows: twelve records and 'more', each claimant under its title, and 'more'"), Capped, ExpectedBooks);
			TestEqual(TEXT("The chapter is MaxBookLines rows at most - here exactly"), Capped.Num(), CosmicChronicle::MaxBookLines);
			TestTrue(TEXT("(both parts over their caps, both claimants standing)"), Books.Recorded.Num() == CosmicChronicle::MaxBookItems && Books.bRecordedMore
				&& Books.bClaimedMore && Books.Claimed.Num() == 2);
			FFormatNamedArguments OtherArgs;
			OtherArgs.Add(TEXT("Transcendent"), Titled.Title);
			TestTrue(TEXT("The other claimant stands under its own title"), Capped.Contains(FText::Format(Label(TEXT("Chronicle.ClaimedBy")), OtherArgs).ToString()));
			Two.Log = Kept;
		}
		{
			// On the game's data every claim line stands under its own claimant's head (critic-6, rule 1 of docs/70: every
			// sentence of the fallen has its speaker's frame): each DT_Transcendents row has a Title, and the page heads its
			// group with it - two claims a row, each over its own record.
			const FCosmicDataTables& Game = Sim->GetTables();
			const TArray<FName> Claimants = Game.GetTranscendentRowNames();
			TestFalse(TEXT("(the game has a transcendent)"), Claimants.IsEmpty());
			Two.Log.RemoveAll(IsBook);
			TMap<FString, FString> HeadOfLine;
			TSet<FString> Heads;
			int64 Tick = 4000;
			// Three claimants at most, so the whole of it fits both caps (6 records; 1 + 3 x 3 rows of part two).
			for (int32 At = 0; At < Claimants.Num() && At < 3; ++At)
			{
				const FText Title = CosmicChronicle::ClaimantTitleOf(Claimants[At], Game);
				TestFalse(*FString::Printf(TEXT("%s has a Title (the head of its claims)"), *Claimants[At].ToString()), Title.IsEmpty());
				FFormatNamedArguments Of;
				Of.Add(TEXT("Transcendent"), Title);
				const FString Head = FText::Format(Label(TEXT("Chronicle.ClaimedBy")), Of).ToString();
				Heads.Add(Head);
				for (int32 Line = 0; Line < 2; ++Line)
				{
					const FString Said = FString::Printf(TEXT("b5 game claim %d %d"), At, Line);
					Two.Log.Add(Told(Tick, TEXT("Log.Record"), 1, LaterID, *FString::Printf(TEXT("b5 game record %d %d"), At, Line)));
					Two.Log.Add(Told(Tick++, TEXT("Log.Claim"), 1, LaterID, *Said));
					Two.Log.Last().SourceRow = Claimants[At];
					HeadOfLine.Add(Said, Head);
				}
			}
			const TArray<FString> GameBooks = BooksOf(PageOf());
			FString Under;
			int32 Seen = 0;
			for (const FString& Row : GameBooks)
			{
				if (Heads.Contains(Row))
				{
					Under = Row;
				}
				else if (const FString* Head = HeadOfLine.Find(Row))
				{
					++Seen;
					TestEqual(*FString::Printf(TEXT("'%s' stands under its claimant's head"), *Row), Under, *Head);
				}
				else if (Row == Words(TEXT("Chronicle.BookClaimed")))
				{
					Under.Reset();
				}
			}
			TestEqual(TEXT("(every claim line is on the page)"), Seen, HeadOfLine.Num());
			Two.Log = Kept;
		}
		Page->Close();
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

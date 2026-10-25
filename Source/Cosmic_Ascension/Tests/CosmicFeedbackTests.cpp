// Cosmic Ascension - automation tests of the feedback of MVP item 6 (Presentation): the stability trend of the top bar and
// the first hints. Design: docs/60_DECISIONS/headers/A6_feedback_and_onboarding.md section 2, 4 and 6.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/UserWidget.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicStabilityTrend.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "UI/CosmicUILabels.h"
#include "Widgets/SWidget.h"
#include "World/CosmicGameMode.h"

namespace CosmicFeedbackTests
{
	void StepBroadcast(UGalaxySimulationSubsystem& Sim)
	{
		const bool bWasBroadcasting = Sim.IsBroadcastEnabled();
		Sim.SetBroadcastEnabled(true);
		Sim.StepOnce();
		Sim.SetBroadcastEnabled(bWasBroadcasting);
	}

	FString Words(const FText& Text)
	{
		return Text.ToString();
	}

	bool HasPlaceholder(const FText& Text)
	{
		return Text.ToString().Contains(FStringTableEntry::GetPlaceholderSourceString());
	}
}

// The stability trend (MVP item 6): the shown percent against the shown percent of the last sixty tick boundaries' mean - a
// mark on the top bar and a tooltip with the difference - as pure functions, and on a live bar: nothing right after a new
// galaxy, the mark of an answer given while paused at once, cleared on another ruled people and on a skipped tick, the
// tooltip kept across ticks and over the stability only.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIStabilityTrendTest, "Cosmic_Ascension.UI.StabilityTrend", CosmicTests::Flags)
bool FCosmicUIStabilityTrendTest::RunTest(const FString& Parameters)
{
	using namespace CosmicFeedbackTests;
	using CosmicUI::StabilityPointsOf;
	using CosmicUI::StabilityTrendOf;
	using CosmicUI::StabilityTrendTicks;

	// ---- Pure ----
	{
		TArray<float> Samples;
		Samples.Init(0.50f, StabilityTrendTicks - 1);
		TestTrue(TEXT("Fewer samples than the window: Unknown"), StabilityTrendOf(Samples, 0.9f) == ECosmicTrend::Unknown);
		TestEqual(TEXT("and no points"), StabilityPointsOf(Samples, 0.9f), 0);
		Samples.Add(0.50f);
		TestTrue(TEXT("A shown percent one above the mean's: Up"), StabilityTrendOf(Samples, 0.51f) == ECosmicTrend::Up);
		TestEqual(TEXT("by one point"), StabilityPointsOf(Samples, 0.51f), 1);
		TestTrue(TEXT("The same shown percent: Flat"), StabilityTrendOf(Samples, 0.504f) == ECosmicTrend::Flat);
		TestTrue(TEXT("One below: Down"), StabilityTrendOf(Samples, 0.49f) == ECosmicTrend::Down);
		TestEqual(TEXT("by minus one point"), StabilityPointsOf(Samples, 0.49f), -1);

		// Only the last window counts: older samples before it change nothing.
		TArray<float> Longer;
		Longer.Init(0.9f, 30);
		Longer.Append(Samples);
		TestTrue(TEXT("Samples older than the window are not in the mean"), StabilityTrendOf(Longer, 0.50f) == ECosmicTrend::Flat);

		// A drop, then the recovery: the mean follows, so the mark turns up while the number climbs - before a whole window
		// has passed (a comparison with the one value a window back would hold Down for the window and flip at its end).
		TArray<float> Ring;
		Ring.Init(0.60f, StabilityTrendTicks);
		float Now = 0.50f;
		bool bDownAtDrop = false;
		int32 FirstUp = INDEX_NONE;
		for (int32 Step = 0; Step < 2 * StabilityTrendTicks; ++Step)
		{
			Ring.Add(Now);
			Ring.RemoveAt(0);
			const ECosmicTrend Trend = StabilityTrendOf(Ring, Now);
			bDownAtDrop |= Step == 0 && Trend == ECosmicTrend::Down;
			if (FirstUp == INDEX_NONE && Trend == ECosmicTrend::Up)
			{
				FirstUp = Step;
			}
			Now += 0.001f;
		}
		TestTrue(TEXT("Right after a drop the mark is Down"), bDownAtDrop);
		TestTrue(TEXT("and the recovery turns it Up before a window has passed"), FirstUp != INDEX_NONE && FirstUp < StabilityTrendTicks);

		const FText Cycles = CosmicUI::DurationLabel(60, ECosmicTimeUnit::Cycle);
		const FText Years = CosmicUI::DurationLabel(60, ECosmicTimeUnit::Year);
		TestTrue(TEXT("A length names its number in either unit"), Words(Cycles).Contains(TEXT("60")) && Words(Years).Contains(TEXT("60")));
		TestTrue(TEXT("in two different forms, from the table"), !Words(Cycles).Equals(Words(Years)) && !HasPlaceholder(Cycles) && !HasPlaceholder(Years));
	}

	// ---- A live bar ----
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
	UCosmicTopBarWidget* Bar = Controller ? CreateWidget<UCosmicTopBarWidget>(Controller) : nullptr;
	if (!TestNotNull(TEXT("(a top bar of the test's own)"), Bar))
	{
		return false;
	}
	const TSharedRef<SWidget> SlateBar = Bar->TakeWidget();
	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	ON_SCOPE_EXIT
	{
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->SetTimeScale(0.f);
	FGalaxyState& Live = Sim->GetMutableState();
	const int32 Ruled = Sim->GetStateIndex().FindCiv(Live, Live.Player.RuledCivID);
	if (!TestTrue(TEXT("(the vessel rules a people, and another people lives)"), Live.Civs.IsValidIndex(Ruled) && Live.Civs.Num() > 1))
	{
		return false;
	}
	const ECosmicTimeUnit Unit = ACosmicPlayerController::TimeUnitFor(*Bar);
	auto Tip = [&]() { return Words(Bar->GetStabilityTip()); };
	auto Mark = [&]() { return Words(Bar->GetTrendText()->GetText()); };

	TestTrue(TEXT("Right after a new galaxy the trend is not known"), Bar->GetStabilityTrend() == ECosmicTrend::Unknown);
	TestTrue(TEXT("no mark stands"), Mark().IsEmpty());
	TestEqual(TEXT("and the tooltip says when one comes"), Tip(), Words(FText::Format(CosmicUI::Label(TEXT("Panel.StabilityWait")), CosmicUI::DurationLabel(StabilityTrendTicks, Unit))));
	TestTrue(TEXT("The stability has a block of its own"), Words(Bar->GetStabilityText()->GetText()).Contains(Words(CosmicUI::Label(TEXT("Panel.Stability")))));
	TestFalse(TEXT("and the resources do not carry it"), Words(Bar->GetStatusText()->GetText()).Contains(Words(CosmicUI::Label(TEXT("Panel.Stability")))));

	// A window of tick boundaries, recorded as the bar saw them.
	TArray<float> Seen;
	for (int32 Step = 0; Step < StabilityTrendTicks; ++Step)
	{
		StepBroadcast(*Sim);
		Seen.Add(Live.Civs[Ruled].Stability);
	}
	const float AtBoundary = Live.Civs[Ruled].Stability;
	TestTrue(TEXT("After a window the trend is the pure function's"), Bar->GetStabilityTrend() == StabilityTrendOf(Seen, AtBoundary));

	// An answer while paused: the number and the mark move at once, before any tick.
	Live.Civs[Ruled].Stability = AtBoundary + 0.15f;
	Bar->Refresh();
	TestTrue(TEXT("A rise while paused shows Up at once"), Bar->GetStabilityTrend() == ECosmicTrend::Up);
	TestEqual(TEXT("with its mark"), Mark(), Words(CosmicUI::Label(TEXT("Panel.TrendUp"))));
	TestEqual(TEXT("and the difference in the tooltip"), Tip(), Words(FText::Format(CosmicUI::Label(TEXT("Panel.StabilityUp")), CosmicUI::DurationLabel(StabilityTrendTicks, Unit),
		FText::AsNumber(StabilityPointsOf(Seen, AtBoundary + 0.15f)))));
	Live.Civs[Ruled].Stability = AtBoundary - 0.15f;
	Bar->Refresh();
	TestTrue(TEXT("A fall shows Down"), Bar->GetStabilityTrend() == ECosmicTrend::Down);
	TestEqual(TEXT("with its mark"), Mark(), Words(CosmicUI::Label(TEXT("Panel.TrendDown"))));
	TestEqual(TEXT("and the tooltip says lower"), Tip(), Words(FText::Format(CosmicUI::Label(TEXT("Panel.StabilityDown")), CosmicUI::DurationLabel(StabilityTrendTicks, Unit),
		FText::AsNumber(-StabilityPointsOf(Seen, AtBoundary - 0.15f)))));
	// The mean itself as the number: flat - no mark.
	double Sum = 0.0;
	for (const float Value : Seen)
	{
		Sum += Value;
	}
	Live.Civs[Ruled].Stability = static_cast<float>(Sum / Seen.Num());
	Bar->Refresh();
	TestTrue(TEXT("The mean as the number is Flat"), Bar->GetStabilityTrend() == ECosmicTrend::Flat);
	TestTrue(TEXT("with no mark"), Mark().IsEmpty());
	TestEqual(TEXT("and the tooltip says about the same"), Tip(), Words(FText::Format(CosmicUI::Label(TEXT("Panel.StabilityFlat")), CosmicUI::DurationLabel(StabilityTrendTicks, Unit))));
	Live.Civs[Ruled].Stability = AtBoundary;

	// The tooltip: a Slate one on the stability, kept across a tick; none on the resources.
	const TSharedPtr<SWidget> SlateStability = Bar->GetStabilityText()->GetCachedWidget();
	if (TestTrue(TEXT("The stability has a Slate tooltip"), SlateStability.IsValid() && SlateStability->GetToolTip().IsValid()))
	{
		const IToolTip* Before = SlateStability->GetToolTip().Get();
		StepBroadcast(*Sim);
		TestTrue(TEXT("a tick keeps the tooltip object"), SlateStability->GetToolTip().Get() == Before);
	}
	const TSharedPtr<SWidget> SlateStatus = Bar->GetStatusText()->GetCachedWidget();
	TestTrue(TEXT("The resources carry no tooltip"), SlateStatus.IsValid() && !SlateStatus->GetToolTip().IsValid());

	// Another ruled people, then a skipped tick: the samples start again.
	const FName RuledID = Live.Player.RuledCivID;
	Live.Player.RuledCivID = Live.Civs[Ruled == 0 ? 1 : 0].CivID;
	StepBroadcast(*Sim);
	TestTrue(TEXT("Another ruled people starts the samples again"), Bar->GetStabilityTrend() == ECosmicTrend::Unknown);
	Live.Player.RuledCivID = RuledID;
	for (int32 Step = 0; Step < StabilityTrendTicks; ++Step)
	{
		StepBroadcast(*Sim);
	}
	TestTrue(TEXT("(a window of the people again)"), Bar->GetStabilityTrend() != ECosmicTrend::Unknown);
	Live.Tick += 5;
	StepBroadcast(*Sim);
	TestTrue(TEXT("A skipped tick starts them again"), Bar->GetStabilityTrend() == ECosmicTrend::Unknown);
	TestEqual(TEXT("and the tooltip counts what is missing"), Tip(), Words(FText::Format(CosmicUI::Label(TEXT("Panel.StabilityWait")), CosmicUI::DurationLabel(StabilityTrendTicks - 1, Unit))));
	TestFalse(TEXT("No placeholder in the tooltip"), HasPlaceholder(Bar->GetStabilityTip()));

	// A load is another game, even at the same tick and the same people's ID: the samples go with the old one.
	for (int32 Step = 0; Step < StabilityTrendTicks; ++Step)
	{
		StepBroadcast(*Sim);
	}
	TestTrue(TEXT("(a window again)"), Bar->GetStabilityTrend() != ECosmicTrend::Unknown);
	{
		const FGalaxyState Copy = Sim->GetState();
		Sim->SetBroadcastEnabled(true);
		Sim->LoadState(Copy);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		TestTrue(TEXT("A load at the same tick clears the samples"), Bar->GetStabilityTrend() == ECosmicTrend::Unknown);
	}
	// A new galaxy clears them at once too (before any tick).
	for (int32 Step = 0; Step < StabilityTrendTicks; ++Step)
	{
		StepBroadcast(*Sim);
	}
	Sim->SetBroadcastEnabled(true);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->SetBroadcastEnabled(bWasBroadcasting);
	TestTrue(TEXT("A new galaxy clears the samples before its first tick"), Bar->GetStabilityTrend() == ECosmicTrend::Unknown && Mark().IsEmpty());
	return true;
}

// The first hints (MVP item 6): every speed button names its number key (the pause Space too) and keeps it; the system
// panel's grey line stands under the buttons of the ruler's settled system until the first project of the session - not
// on another people's system, a settling one, or after the run's end - and the rule as a pure function.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIFirstHintsTest, "Cosmic_Ascension.UI.FirstHints", CosmicTests::Flags)
bool FCosmicUIFirstHintsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicFeedbackTests;

	// ---- Pure ----
	{
		FCosmicHintProgress Fresh;
		FCosmicHintProgress Started;
		Started.bStartedProject = true;
		TestTrue(TEXT("An enabled project, a running game, nothing started: the line"), CosmicUI::ShowsCommandHint(true, false, Fresh));
		TestFalse(TEXT("No enabled project: none"), CosmicUI::ShowsCommandHint(false, false, Fresh));
		TestFalse(TEXT("The run over: none"), CosmicUI::ShowsCommandHint(true, true, Fresh));
		TestFalse(TEXT("A project started: none"), CosmicUI::ShowsCommandHint(true, false, Started));
	}

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
	UCosmicTopBarWidget* Bar = Controller ? CreateWidget<UCosmicTopBarWidget>(Controller) : nullptr;
	UCosmicSystemPanelWidget* Panel = Controller ? CreateWidget<UCosmicSystemPanelWidget>(Controller) : nullptr;
	if (!TestNotNull(TEXT("(a top bar of the test's own)"), Bar) || !TestNotNull(TEXT("(and a system panel)"), Panel))
	{
		return false;
	}
	const TSharedRef<SWidget> SlateBar = Bar->TakeWidget();
	const TSharedRef<SWidget> SlatePanel = Panel->TakeWidget();
	const FCosmicHintProgress Saved = Controller->GetHintProgress();
	ON_SCOPE_EXIT
	{
		Controller->ClearRunEnd();
		Controller->SetHintProgress(Saved);
		Controller->SetSpeed(0.f);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Controller->SetHintProgress(FCosmicHintProgress());
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->SetTimeScale(0.f);

	// ---- The number keys ----
	{
		const TArray<TObjectPtr<UCosmicActionButton>>& Buttons = Bar->GetSpeedButtons();
		bool bAllNamed = Buttons.Num() == static_cast<int32>(UE_ARRAY_COUNT(CosmicUI::SpeedSteps));
		for (int32 Key = 0; bAllNamed && Key < Buttons.Num(); ++Key)
		{
			const FText KeyTip = FText::Format(CosmicUI::Label(Key == 0 ? TEXT("Hint.PauseKey") : TEXT("Hint.SpeedKey")), FText::AsNumber(Key));
			const FText Shown = Buttons[Key]->GetToolTipText();
			bAllNamed = Words(Shown).StartsWith(Words(KeyTip)) && !HasPlaceholder(Shown);
		}
		TestTrue(TEXT("Every speed button names its number key, pause first"), bAllNamed);
		TestTrue(TEXT("The pause names Space"), Words(Buttons[0]->GetToolTipText()).Contains(TEXT("Space")));
		Controller->PressSpeedKey(EKeys::One);
		Bar->Refresh();
		Controller->PressSpeedKey(EKeys::SpaceBar);
		Bar->Refresh();
		TestTrue(TEXT("and keeps it after the keys"), Words(Buttons[1]->GetToolTipText()).StartsWith(Words(FText::Format(CosmicUI::Label(TEXT("Hint.SpeedKey")), FText::AsNumber(1)))));
	}

	// ---- The panel's line ----
	FGalaxyState& Live = Sim->GetMutableState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const int32 Ruled = Index.FindCiv(Live, Live.Player.RuledCivID);
	const int32 Home = Live.Civs.IsValidIndex(Ruled) ? Index.FindSystem(Live, Live.Civs[Ruled].HomeSystemID) : INDEX_NONE;
	const int32 Other = Live.Civs.IsValidIndex(1) ? Index.FindSystem(Live, Live.Civs[Ruled == 1 ? 0 : 1].HomeSystemID) : INDEX_NONE;
	if (!TestTrue(TEXT("(the ruled people has a home, and another people too)"), Live.Systems.IsValidIndex(Home) && Live.Systems.IsValidIndex(Other)))
	{
		return false;
	}
	CosmicTests::KnowAll(Sim->GetMutableState());
	auto LineShown = [&]() { return Panel->GetHintText() && Panel->GetHintText()->GetVisibility() != ESlateVisibility::Collapsed; };
	Panel->ShowSystem(Live.Systems[Home].SystemID);
	Panel->Refresh();
	UCosmicActionButton* Enabled = nullptr;
	for (UCosmicActionButton* Button : Panel->GetActionButtons())
	{
		if (Button && Button->GetIsEnabled())
		{
			Enabled = Button;
			break;
		}
	}
	if (!TestNotNull(TEXT("(the home offers a project to start)"), Enabled))
	{
		return false;
	}
	TestTrue(TEXT("The ruled home's panel shows the hint line"), LineShown());
	TestFalse(TEXT("which has words"), Panel->GetHintText()->GetText().IsEmpty() || HasPlaceholder(Panel->GetHintText()->GetText()));
	Panel->ShowSystem(Live.Systems[Other].SystemID);
	Panel->Refresh();
	TestFalse(TEXT("Another people's system has none"), LineShown());

	// A settling system of the people offers no project: no line.
	Panel->ShowSystem(Live.Systems[Home].SystemID);
	const FName WasActive = Live.Systems[Home].ActiveMegaproject;
	Live.Systems[Home].ActiveMegaproject = CosmicRows::ProjectSettle;
	Panel->Refresh();
	TestFalse(TEXT("A settling system of the people has none"), LineShown());
	Live.Systems[Home].ActiveMegaproject = WasActive;
	Panel->Refresh();
	TestTrue(TEXT("(settled again, it stands)"), LineShown());

	// The run over: the buttons are grey and the line goes.
	const int64 WasTick = Live.Tick;
	Live.Tick = Sim->GetRules().ProvisionalEndTick;
	Controller->FollowEnd(false);
	Panel->Refresh();
	TestFalse(TEXT("After the end the line goes"), LineShown());
	Controller->ClearRunEnd();
	Live.Tick = WasTick;
	Panel->Refresh();

	// The first project: the line goes for the session.
	const bool bStarted = Controller->StartProject(Live.Systems[Home].SystemID, Enabled->GetAction());
	TestTrue(TEXT("(the project starts)"), bStarted);
	TestTrue(TEXT("The first project is noted"), Controller->GetHintProgress().bStartedProject);
	Panel->Refresh();
	TestFalse(TEXT("and the line goes"), LineShown());
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	CosmicTests::KnowAll(Sim->GetMutableState());
	Panel->ShowSystem(Sim->GetState().Systems[Home].SystemID);
	Panel->Refresh();
	TestFalse(TEXT("nor does it come back on a new galaxy of the session"), LineShown());

	// ---- The first ship's line (B1): on the capital's panel once a session, from 8:30 of x1 for ten minutes ----
	{
		FCosmicHintProgress Fresh;
		FCosmicHintProgress Hinted;
		Hinted.bShipHinted = true;
		const int64 From = CosmicUI::FirstShipFromTick;
		TestTrue(TEXT("The capital at 8:30: the ship's line"), CosmicUI::ShowsShipHint(true, From, false, false, Fresh, false));
		TestFalse(TEXT("a tick before: none"), CosmicUI::ShowsShipHint(true, From - 1, false, false, Fresh, false));
		TestFalse(TEXT("from 18:30: none"), CosmicUI::ShowsShipHint(true, CosmicUI::FirstShipUntilTick, false, false, Fresh, false));
		TestFalse(TEXT("another panel: none"), CosmicUI::ShowsShipHint(false, From, false, false, Fresh, false));
		TestFalse(TEXT("an ended run: none"), CosmicUI::ShowsShipHint(true, From, true, false, Fresh, false));
		TestFalse(TEXT("a vessel that once left a throne: none"), CosmicUI::ShowsShipHint(true, From, false, true, Fresh, false));
		TestFalse(TEXT("shown once this session: none"), CosmicUI::ShowsShipHint(true, From, false, false, Hinted, false));
		TestTrue(TEXT("but it stays for the viewing it came in"), CosmicUI::ShowsShipHint(true, From, false, false, Hinted, true));

		FGalaxyState& Run = Sim->GetMutableState();
		auto ShipShown = [&]() { return Panel->GetShipHintText() && Panel->GetShipHintText()->GetVisibility() != ESlateVisibility::Collapsed; };
		auto ShowCapital = [&]()
		{
			Panel->ShowSystem(NAME_None);
			Panel->ShowSystem(Run.Systems[Home].SystemID);
		};
		Controller->SetHintProgress(FCosmicHintProgress());
		const int64 Before = Run.Tick;
		Run.Tick = From;
		ShowCapital();
		TestTrue(TEXT("The capital's panel at 8:30 shows the ship's line"), ShipShown());
		TestFalse(TEXT("which has words"), Panel->GetShipHintText()->GetText().IsEmpty() || HasPlaceholder(Panel->GetShipHintText()->GetText()));
		TestTrue(TEXT("and notes it for the session"), Controller->GetHintProgress().bShipHinted);
		Panel->Refresh();
		TestTrue(TEXT("It stays while the capital shows"), ShipShown());
		Panel->ShowSystem(Run.Systems[Other].SystemID);
		TestFalse(TEXT("Another system's panel has none"), ShipShown());
		Panel->ShowSystem(Run.Systems[Home].SystemID);
		TestFalse(TEXT("and the capital shown again has none: once a session"), ShipShown());
		Controller->SetHintProgress(FCosmicHintProgress());
		Run.Tick = 2000;
		ShowCapital();
		TestFalse(TEXT("A run continued at tick 2,000 shows none"), ShipShown());
		Run.Tick = From;
		Run.Player.FormerCivID = Run.Civs[0].CivID;
		ShowCapital();
		TestFalse(TEXT("A run whose vessel once left a throne shows none"), ShipShown());
		Run.Player.FormerCivID = NAME_None;
		Run.Tick = Before;
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

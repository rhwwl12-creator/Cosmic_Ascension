// Cosmic Ascension - automation tests of the third rung's speed and of the top bar's 'next rung' (Presentation). Design:
// guideline.md section 2.1, 3.2, 6.3, 6.4, 7.5, 8.3, 9.3, 11 Phase 3 (item 12).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Ascension/AscensionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "Internationalization/StringTableCore.h"
#include "Layout/Clipping.h"
#include "Misc/AutomationTest.h"
#include "Misc/Optional.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicAscensionOverlayWidget.h"
#include "UI/CosmicNextRung.h"
#include "UI/CosmicTopBarWidget.h"
#include "UI/CosmicUILabels.h"
#include "UObject/Class.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/IToolTip.h"
#include "Widgets/SWidget.h"
#include "World/CosmicGameMode.h"
#include "World/CosmicMapLook.h"

#include <limits>

namespace CosmicThirdRungTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// Whether a widget shows and lets a click reach it: visible, or self-hit-test-invisible over its hit-testable button.
	bool ShowsForClicks(const UWidget* Widget)
	{
		const ESlateVisibility Visibility = Widget ? Widget->GetVisibility() : ESlateVisibility::Collapsed;
		return Visibility == ESlateVisibility::Visible || Visibility == ESlateVisibility::SelfHitTestInvisible;
	}

	bool IsCollapsed(const UWidget* Widget)
	{
		return Widget && Widget->GetVisibility() == ESlateVisibility::Collapsed;
	}

	// Broadcasts OnTierAscended as the judgment does: with the gate open (the HUD's presentation answers it) and inside a
	// Core Logic broadcast (UAscensionSubsystem::JudgePeople brackets it), so the presentation's pause runs where it runs in
	// the game.
	void AscendNow(UGalaxySimulationSubsystem& Sim, EAscensionTier Tier, FName FeatRow)
	{
		UAscensionSubsystem* Ascension = Sim.GetGameInstance()->GetSubsystem<UAscensionSubsystem>();
		const bool bWasBroadcasting = Sim.IsBroadcastEnabled();
		Sim.SetBroadcastEnabled(true);
		Sim.BeginBroadcast();
		Ascension->OnTierAscended.Broadcast(Tier, FeatRow, EPlayerRole::Ruler);
		Sim.EndBroadcast();
		Sim.SetBroadcastEnabled(bWasBroadcasting);
	}

	// One tick with the broadcast gate open (the controller's and the HUD's handlers answer it).
	void StepBroadcast(UGalaxySimulationSubsystem& Sim)
	{
		const bool bWasBroadcasting = Sim.IsBroadcastEnabled();
		Sim.SetBroadcastEnabled(true);
		Sim.StepOnce();
		Sim.SetBroadcastEnabled(bWasBroadcasting);
	}

	// The digits of a text as one number (a percent the bar shows: "35%" is 35); -1 without a digit.
	int32 DigitsOf(const FString& Text)
	{
		int32 Value = -1;
		for (const TCHAR Char : Text)
		{
			if (Char >= TEXT('0') && Char <= TEXT('9'))
			{
				Value = (Value < 0 ? 0 : Value * 10) + (Char - TEXT('0'));
			}
		}
		return Value;
	}

	bool HasAsciiDigit(const FString& Text)
	{
		return DigitsOf(Text) >= 0;
	}

	FCosmicCondition Cond(EConditionType Type, FName Key = NAME_None, float Value = 0.f)
	{
		FCosmicCondition Condition;
		Condition.Type = Type;
		Condition.Key = Key;
		Condition.Value = Value;
		return Condition;
	}

	FAscensionFeatDef FeatRow(EAscensionTier From, EAscensionTier To, TArray<FCosmicCondition> Conditions, EPlayerRole Path = EPlayerRole::Ruler)
	{
		FAscensionFeatDef Row;
		Row.Title = FText::AsCultureInvariant(TEXT("<feat>"));
		Row.RequiredTier = From;
		Row.TargetTier = To;
		Row.Path = Path;
		Row.CompletionConditions = MoveTemp(Conditions);
		Row.LogTemplates = { FText::AsCultureInvariant(TEXT("{Civ} {HomeSystem} climbed one")), FText::AsCultureInvariant(TEXT("{Civ} {HomeSystem} climbed two")) };
		return Row;
	}

	void Record(FStarSystemState& System, FName Row, int64 Tick)
	{
		FMegaprojectRecord& Added = System.CompletedMegaprojects.AddDefaulted_GetRef();
		Added.Row = Row;
		Added.Tick = Tick;
	}

	const FLinearColor Gold(1.f, 0.82f, 0.4f);
}

// The third rung's speed (Phase 3 item 12, guideline 2.1, 6.3): the table of steps and the one gate (the spiritual
// sight's), the bar's sixth button (collapsed below the third rung and while the ascension presentation shows, its
// tooltip, its press, its year label), a bar without a controller, the controller's keys, SetSpeed, the pause's release,
// the presentation's give-back and the lowering when the gate closes on a tick; neither hash moves.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIThirdRungSpeedTest, "Cosmic_Ascension.UI.ThirdRungSpeed", CosmicTests::Flags)
bool FCosmicUIThirdRungSpeedTest::RunTest(const FString& Parameters)
{
	using namespace CosmicThirdRungTests;
	using CosmicUI::IsSpeedOpen;

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
	UCosmicAscensionOverlayWidget* Overlay = HUD ? HUD->GetAscensionOverlay() : nullptr;
	UCosmicTopBarWidget* HudBar = HUD ? HUD->GetTopBar() : nullptr;
	if (!TestNotNull(TEXT("The first player controller is the game's"), Controller) || !TestNotNull(TEXT("The HUD has its ascension overlay"), Overlay)
		|| !TestNotNull(TEXT("and its top bar"), HudBar))
	{
		return false;
	}
	const ECosmicTimeUnit ViewerUnit = Controller->GetTimeUnit();
	ON_SCOPE_EXIT
	{
		Sim->SetTimeScale(0.f);
		Overlay->Close();
		Overlay->Advance(CosmicAscensionOverlay::RevealSeconds + 1.f);
		Sim->SetTimeScale(0.f);
		Controller->CosmicSpiritualSight(0);
		Controller->SetTimeUnit(ViewerUnit);
		Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Controller->SetTimeUnit(ECosmicTimeUnit::Cycle);
	// A presentation an earlier test raised closes on its own after five seconds of real time: closed here.
	Overlay->Close();
	Sim->SetTimeScale(0.f);
	TestFalse(TEXT("(no presentation shows)"), Overlay->IsShowing());
	FGalaxyState& Live = Sim->GetMutableState();
	const int32 RuledIndex = Sim->GetStateIndex().FindCiv(Live, Live.Player.RuledCivID);
	if (!TestTrue(TEXT("(the player rules a people)"), Live.Civs.IsValidIndex(RuledIndex)))
	{
		return false;
	}
	const TArray<FName> FeatRows = Sim->GetTables().GetFeatRowNames();
	const FName AnyFeat = FeatRows.IsEmpty() ? NAME_None : FeatRows[0];

	// ---- 1. The table and the one gate ----
	{
		const float Expected[] = { 0.f, 1.f, 2.f, 5.f, 10.f, 25.f };
		bool bSteps = UE_ARRAY_COUNT(CosmicUI::SpeedSteps) == UE_ARRAY_COUNT(Expected);
		for (int32 At = 0; bSteps && At < static_cast<int32>(UE_ARRAY_COUNT(Expected)); ++At)
		{
			bSteps = CosmicUI::SpeedSteps[At] == Expected[At];
		}
		TestTrue(TEXT("The speed steps are pause, x1, x2, x5, x10, x25"), bSteps);
		const UEnum* Tiers = StaticEnum<EAscensionTier>();
		for (int32 At = 0; At < Tiers->NumEnums() - 1; ++At)
		{
			const EAscensionTier Tier = static_cast<EAscensionTier>(Tiers->GetValueByIndex(At));
			const int32 Rung = static_cast<int32>(Tier) + 1;
			for (const float Step : CosmicUI::SpeedSteps)
			{
				const bool bOpen = Step <= 10.f || Rung >= 3;
				TestEqual(*FString::Printf(TEXT("Step %g on rung %d is open: %s"), Step, Rung, bOpen ? TEXT("yes") : TEXT("no")),
					IsSpeedOpen(Step, CosmicMapLook::SpiritualSightOpens(Tier)), bOpen);
			}
		}
		TestFalse(TEXT("A second-rung viewer gets no x25"), IsSpeedOpen(25.f, CosmicMapLook::SpiritualSightOpens(EAscensionTier::Tier2_Stellar)));
		TestTrue(TEXT("a third-rung viewer does"), IsSpeedOpen(25.f, CosmicMapLook::SpiritualSightOpens(EAscensionTier::Tier3_Causal)));
		TestEqual(TEXT("x25 behind a closed gate is x10"), CosmicUI::OpenSpeed(25.f, false), 10.f);
		TestEqual(TEXT("x25 through an open gate is x25"), CosmicUI::OpenSpeed(25.f, true), 25.f);
		TestEqual(TEXT("x5 behind a closed gate is x5"), CosmicUI::OpenSpeed(5.f, false), 5.f);
		TestFalse(TEXT("A first look lowers nothing"), CosmicUI::LowersSpeed(TOptional<bool>(), false, 25.f));
		TestTrue(TEXT("A gate that closes lowers a running x25"), CosmicUI::LowersSpeed(TOptional<bool>(true), false, 25.f));
		TestFalse(TEXT("A gate that stays open lowers nothing"), CosmicUI::LowersSpeed(TOptional<bool>(true), true, 25.f));
		TestFalse(TEXT("A gate that stays closed lowers nothing"), CosmicUI::LowersSpeed(TOptional<bool>(false), false, 25.f));
		TestFalse(TEXT("A closing gate leaves x10 alone"), CosmicUI::LowersSpeed(TOptional<bool>(true), false, 10.f));
		const FKey NumberKeys[] = { EKeys::Zero, EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five };
		for (int32 At = 0; At < static_cast<int32>(UE_ARRAY_COUNT(NumberKeys)); ++At)
		{
			const TOptional<float> Step = ACosmicPlayerController::SpeedOfKey(NumberKeys[At]);
			TestTrue(*FString::Printf(TEXT("Key %d sets step %d"), At, At), Step.IsSet() && Step.GetValue() == CosmicUI::SpeedSteps[At]);
		}
		TestFalse(TEXT("Key 6 sets no step"), ACosmicPlayerController::SpeedOfKey(EKeys::Six).IsSet());
		TestFalse(TEXT("Space sets no step"), ACosmicPlayerController::SpeedOfKey(EKeys::SpaceBar).IsSet());
	}

	// ---- 2. The bar ----
	{
		UCosmicTopBarWidget* Bar = CreateWidget<UCosmicTopBarWidget>(Controller);
		if (!TestNotNull(TEXT("The top bar can be created"), Bar))
		{
			return false;
		}
		const TSharedRef<SWidget> SlateBar = Bar->TakeWidget();
		Live.Civs[RuledIndex].Tier = EAscensionTier::Tier1_Planetary;
		Bar->Refresh();
		const TArray<TObjectPtr<UCosmicActionButton>>& Speeds = Bar->GetSpeedButtons();
		bool bOrder = Speeds.Num() == static_cast<int32>(UE_ARRAY_COUNT(CosmicUI::SpeedSteps));
		for (int32 At = 0; bOrder && At < Speeds.Num(); ++At)
		{
			bOrder = Speeds[At] && Speeds[At]->GetValue() == CosmicUI::SpeedSteps[At];
		}
		TestTrue(TEXT("Six speed buttons in the order of the steps"), bOrder);
		UCosmicActionButton* X25 = Bar->FindSpeedButton(25.f);
		UCosmicActionButton* X10 = Bar->FindSpeedButton(10.f);
		if (!TestNotNull(TEXT("(the bar has its x25 button)"), X25) || !TestNotNull(TEXT("(and its x10 button)"), X10))
		{
			return false;
		}
		TestTrue(TEXT("The x25 button is collapsed on the first rung"), IsCollapsed(X25));
		TestTrue(TEXT("and the x10 button shows"), ShowsForClicks(X10));
		Live.Civs[RuledIndex].Tier = EAscensionTier::Tier2_Stellar;
		Bar->Refresh();
		TestTrue(TEXT("The x25 button is collapsed on the second rung"), IsCollapsed(X25));
		Live.Civs[RuledIndex].Tier = EAscensionTier::Tier3_Causal;
		Bar->Refresh();
		TestTrue(TEXT("From the third rung the x25 button shows"), ShowsForClicks(X25));
		TestEqual(TEXT("its tooltip is its number key, then Speed.ThirdRungTip"), X25->GetToolTipText().ToString(),
			CosmicUI::JoinLines({ FText::Format(CosmicUI::Label(TEXT("Hint.SpeedKey")), FText::AsNumber(5)), CosmicUI::Label(TEXT("Speed.ThirdRungTip")) }).ToString());
		TestFalse(TEXT("which has words"), X25->GetToolTipText().IsEmpty());
		TestEqual(TEXT("x10's tooltip is its number key (MVP item 6)"), X10->GetToolTipText().ToString(), FText::Format(CosmicUI::Label(TEXT("Hint.SpeedKey")), FText::AsNumber(4)).ToString());
		X25->Press();
		TestEqual(TEXT("x25 runs the simulation at 25"), Sim->GetTimeScale(), 25.f);
		TestFalse(TEXT("and is greyed out"), X25->GetIsEnabled());
		Controller->SetTimeUnit(ECosmicTimeUnit::Year);
		Bar->Refresh();
		TestEqual(TEXT("In years the x25 button reads its year label"), X25->GetLabel().ToString(), CosmicUI::SpeedLabel(25.f, ECosmicTimeUnit::Year).ToString());
		Controller->SetTimeUnit(ECosmicTimeUnit::Cycle);
		Bar->Refresh();
		TestEqual(TEXT("and in cycles x25"), X25->GetLabel().ToString(), CosmicUI::SpeedLabel(25.f, ECosmicTimeUnit::Cycle).ToString());
		Sim->SetTimeScale(0.f);

		// After the presentation: the third rung's x25 waits for the ascension presentation to close (the HUD's bar, which
		// the overlay's Close redraws - the simulation stays paused, so no speed is given back to redraw it).
		Overlay->Show(EAscensionTier::Tier3_Causal, AnyFeat);
		HudBar->Refresh();
		UCosmicActionButton* HudX25 = HudBar->FindSpeedButton(25.f);
		TestTrue(TEXT("(the presentation shows)"), Overlay->IsShowing());
		TestTrue(TEXT("x25 does not show while the presentation does"), IsCollapsed(HudX25));
		Overlay->Close();
		TestTrue(TEXT("x25 shows when the presentation closes, not before"), ShowsForClicks(HudX25));
		Overlay->Advance(CosmicAscensionOverlay::RevealSeconds + 1.f);
	}

	// ---- 3. A bar without a controller ----
	{
		UWidgetTree* Outer = NewObject<UWidgetTree>(Sim->GetGameInstance());
		UCosmicTopBarWidget* Lone = CreateWidget<UCosmicTopBarWidget>(Outer);
		if (!TestNotNull(TEXT("A bar without a controller can be created"), Lone))
		{
			return false;
		}
		TestNull(TEXT("(no controller)"), Lone->GetOwningPlayer());
		TestNotNull(TEXT("(it reaches the simulation)"), Lone->GetGameInstance());
		const TSharedRef<SWidget> SlateLone = Lone->TakeWidget();
		Live.Civs[RuledIndex].Tier = EAscensionTier::Tier1_Planetary;
		Sim->SetTimeScale(0.f);
		Lone->Refresh();
		if (UCosmicActionButton* LoneX25 = Lone->FindSpeedButton(25.f); TestNotNull(TEXT("(the lone bar has its x25 button)"), LoneX25))
		{
			LoneX25->Press();
			TestEqual(TEXT("A bar without a controller opens no closed speed"), Sim->GetTimeScale(), 10.f);
			Live.Civs[RuledIndex].Tier = EAscensionTier::Tier3_Causal;
			Sim->SetTimeScale(0.f);
			Lone->Refresh();
			LoneX25->Press();
			TestEqual(TEXT("and opens x25 from the third rung"), Sim->GetTimeScale(), 25.f);
		}
		Sim->SetTimeScale(0.f);
	}

	// ---- 4. The controller ----
	{
		// The speeds move neither hash: a block of presses with no tick in it.
		Live.Civs[RuledIndex].Tier = EAscensionTier::Tier3_Causal;
		const uint32 StateHash = Sim->ComputeStateHash();
		const uint32 WorldHash = Sim->ComputeWorldHash();
		Controller->PressSpeedKey(EKeys::Five);
		TestEqual(TEXT("Key 5 runs x25 from the third rung"), Sim->GetTimeScale(), 25.f);
		Controller->PressSpeedKey(EKeys::SpaceBar);
		TestEqual(TEXT("(Space pauses)"), Sim->GetTimeScale(), 0.f);
		Controller->PressSpeedKey(EKeys::SpaceBar);
		TestEqual(TEXT("and gives x25 back on the third rung"), Sim->GetTimeScale(), 25.f);
		Controller->PressSpeedKey(EKeys::Two);
		TestEqual(TEXT("Key 2 runs x2"), Sim->GetTimeScale(), 2.f);
		TestEqual(TEXT("The speeds move no state"), Sim->ComputeStateHash(), StateHash);
		TestEqual(TEXT("and no world"), Sim->ComputeWorldHash(), WorldHash);

		Live.Civs[RuledIndex].Tier = EAscensionTier::Tier1_Planetary;
		Controller->PressSpeedKey(EKeys::Five);
		TestEqual(TEXT("Key 5 below the third rung changes nothing"), Sim->GetTimeScale(), 2.f);
		Controller->SetSpeed(25.f);
		TestEqual(TEXT("SetSpeed(25) below the third rung runs x10"), Sim->GetTimeScale(), 10.f);

		// The pause's release: a x25 remembered while the gate was open comes back as x10 once it closed.
		Live.Civs[RuledIndex].Tier = EAscensionTier::Tier3_Causal;
		Controller->SetSpeed(25.f);
		Controller->PressSpeedKey(EKeys::SpaceBar);
		TestEqual(TEXT("(paused at x25)"), Sim->GetTimeScale(), 0.f);
		Live.Civs[RuledIndex].Tier = EAscensionTier::Tier1_Planetary;
		Controller->PressSpeedKey(EKeys::SpaceBar);
		TestEqual(TEXT("The pause's release gives x10 for a x25 the gate does not open"), Sim->GetTimeScale(), 10.f);

		// The gate closes on a tick: the controller's handler of OnSimTick lowers a running x25.
		Live.Civs[RuledIndex].Tier = EAscensionTier::Tier3_Causal;
		Controller->FollowSpeedGate();
		Controller->SetSpeed(25.f);
		TestEqual(TEXT("(x25 runs)"), Sim->GetTimeScale(), 25.f);
		Live.Civs[RuledIndex].Tier = EAscensionTier::Tier1_Planetary;
		StepBroadcast(*Sim);
		TestEqual(TEXT("When the gate closes on a tick, x25 falls to x10"), Sim->GetTimeScale(), 10.f);

		// A load that closes the gate: the controller's handler of OnGalaxyGenerated lowers a running x25 at once, before
		// any tick (a lower save loaded while x25 runs).
		Live.Civs[RuledIndex].Tier = EAscensionTier::Tier3_Causal;
		Controller->FollowSpeedGate();
		Controller->SetSpeed(25.f);
		TestEqual(TEXT("(x25 runs before the load)"), Sim->GetTimeScale(), 25.f);
		{
			FGalaxyState Lower = Live;
			Lower.Civs[RuledIndex].Tier = EAscensionTier::Tier1_Planetary;
			const int64 TickBefore = Live.Tick;
			const bool bWasLoadBroadcasting = Sim->IsBroadcastEnabled();
			Sim->SetBroadcastEnabled(true);
			Sim->LoadState(Lower);
			Sim->SetBroadcastEnabled(bWasLoadBroadcasting);
			TestEqual(TEXT("A load that closes the gate brings x25 down to x10 before any tick"), Sim->GetTimeScale(), 10.f);
			TestEqual(TEXT("(no tick ran)"), Sim->GetState().Tick, TickBefore);
		}

		// A x25 the gate never opened stays: the development argument -Speed=25 sets the simulation directly.
		const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
		Sim->SetBroadcastEnabled(true);
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Sim->SetTimeScale(25.f);
		StepBroadcast(*Sim);
		TestEqual(TEXT("A x25 the gate never opened stays (the development argument)"), Sim->GetTimeScale(), 25.f);
		Sim->SetTimeScale(0.f);
	}

	// ---- 5. The presentation gives its paused speed back through SetSpeed ----
	{
		FGalaxyState& Fresh = Sim->GetMutableState();
		const int32 Ruled = Sim->GetStateIndex().FindCiv(Fresh, Fresh.Player.RuledCivID);
		if (TestTrue(TEXT("(the player rules a people in the new galaxy)"), Fresh.Civs.IsValidIndex(Ruled)))
		{
			Fresh.Civs[Ruled].Tier = EAscensionTier::Tier3_Causal;
			Controller->SetSpeed(25.f);
			AscendNow(*Sim, EAscensionTier::Tier3_Causal, AnyFeat);
			TestEqual(TEXT("(the presentation paused x25)"), Sim->GetTimeScale(), 0.f);
			Overlay->Close();
			TestEqual(TEXT("The presentation gives x25 back on the third rung"), Sim->GetTimeScale(), 25.f);
			Overlay->Advance(CosmicAscensionOverlay::RevealSeconds + 1.f);

			Fresh.Civs[Ruled].Tier = EAscensionTier::Tier1_Planetary;
			Sim->SetTimeScale(25.f);
			AscendNow(*Sim, EAscensionTier::Tier2_Stellar, AnyFeat);
			TestEqual(TEXT("(the presentation paused it)"), Sim->GetTimeScale(), 0.f);
			Overlay->Close();
			TestEqual(TEXT("and x10 for a x25 the gate does not open"), Sim->GetTimeScale(), 10.f);
			Sim->SetTimeScale(0.f);

			// The development force opens the one gate for x25 as for the sight, and gives it back.
			HudBar->Refresh();
			TestTrue(TEXT("(on the first rung the HUD's x25 is collapsed)"), IsCollapsed(HudBar->FindSpeedButton(25.f)));
			Controller->CosmicSpiritualSight(1);
			HudBar->Refresh();
			TestTrue(TEXT("The development force opens x25 with the sight"), ShowsForClicks(HudBar->FindSpeedButton(25.f)));
			Controller->CosmicSpiritualSight(0);
			HudBar->Refresh();
			TestTrue(TEXT("and closes it with the sight"), IsCollapsed(HudBar->FindSpeedButton(25.f)));
		}
	}
	return true;
}

// The top bar's 'next rung' (Phase 3 item 12, guideline 6.3): what the next feat row asks, read as the judgment reads it
// - the row, its parts and their words on the game's rows, progress, the life-relative completion, the percent forms,
// the judgment itself as the oracle, the second rung, injected rows (a refused row, parts on two systems, other
// conditions), the rungs and roles without a next rung - and the bar's tooltip bound once on the rung and the research.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUINextRungTest, "Cosmic_Ascension.UI.NextRung", CosmicTests::Flags)
bool FCosmicUINextRungTest::RunTest(const FString& Parameters)
{
	using namespace CosmicThirdRungTests;
	using CosmicNextRung::HeldPercent;
	using CosmicNextRung::WholePercent;

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
	UAscensionSubsystem* Ascension = Sim->GetGameInstance()->GetSubsystem<UAscensionSubsystem>();
	if (!TestNotNull(TEXT("The first player controller is the game's"), Controller) || !TestNotNull(TEXT("The ascension subsystem runs"), Ascension))
	{
		return false;
	}
	const FCosmicDataTables GameTables = Sim->GetTables();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(GameTables);
		Sim->SetTimeScale(0.f);
		Sim->NewGalaxy(42, 5, 3);
	};
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	const FString& Placeholder = FStringTableEntry::GetPlaceholderSourceString();
	FGalaxyState& Live = Sim->GetMutableState();
	const int32 Ruled = Sim->GetStateIndex().FindCiv(Live, Live.Player.RuledCivID);
	const int32 Home = Live.Civs.IsValidIndex(Ruled) ? Sim->GetStateIndex().FindSystem(Live, Live.Civs[Ruled].HomeSystemID) : INDEX_NONE;
	if (!TestTrue(TEXT("(the vessel rules a people with a home)"), Live.Systems.IsValidIndex(Home)))
	{
		return false;
	}
	const FName Dyson(TEXT("project_dyson"));
	const FName Stabilize(TEXT("project_stabilize"));
	const FName Physics(TEXT("Physics"));
	const FMegaprojectDef* DysonRow = GameTables.FindMegaproject(Dyson);
	const FHazardDef* NovaRow = GameTables.FindHazardByTag(Tag(TEXT("Hazard.PreSupernova")));
	if (!TestNotNull(TEXT("(the game has the Dyson swarm)"), DysonRow) || !TestNotNull(TEXT("(and the supernova)"), NovaRow))
	{
		return false;
	}
	// The first rung, nothing built, every research share 0 but Physics 0.34.
	auto Reset = [&]()
	{
		Live.Tick = 10;
		Live.Calling.BornTick = 0;
		Live.Calling.LifeState = ELifeState::Embodied;
		Live.Player.Role = EPlayerRole::Ruler;
		Live.Civs[Ruled].Tier = EAscensionTier::Tier1_Planetary;
		for (TPair<FName, float>& Field : Live.Civs[Ruled].Research)
		{
			Field.Value = 0.f;
		}
		Live.Civs[Ruled].Research.Add(Physics, 0.34f);
		for (FStarSystemState& System : Live.Systems)
		{
			CosmicHazards::EmptyHazardSlot(System);
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
			System.CompletedMegaprojects.Reset();
		}
	};
	auto NextOf = [&]()
	{
		return CosmicNextRung::Of(Live, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
	};
	auto TextNow = [&]()
	{
		return CosmicNextRung::TextOf(NextOf(), Sim->GetTables()).ToString();
	};
	// Whether the judgment lifts the people the vessel rules now: one judgment on the live state, then the state back.
	auto JudgmentLifts = [&]()
	{
		Sim->SyncRngState();
		const FGalaxyState Saved = Live;
		const EAscensionTier Before = Live.Civs[Ruled].Tier;
		const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
		Sim->SetBroadcastEnabled(false);
		Ascension->EvaluateFeats(Sim->GetMutableState(), Sim->GetRng());
		const bool bLifted = Sim->GetState().Civs.IsValidIndex(Ruled) && Sim->GetState().Civs[Ruled].Tier != Before;
		Sim->LoadState(Saved);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		return bLifted;
	};
	const FString NextPrefix = FText::Format(CosmicUI::Label(TEXT("Rung.Next")), FText::GetEmpty()).ToString();

	// ---- 1. The row ----
	Reset();
	{
		const FCosmicNextRung Next = NextOf();
		TestEqual(TEXT("The first rung's next rung is the Dyson swarm row"), Next.FeatRow, FName(TEXT("feat_dyson_swarm")));
		if (TestEqual(TEXT("two parts, a project and a research share"), Next.Parts.Num(), 2))
		{
			TestTrue(TEXT("(the first is the swarm)"), Next.Parts[0].Kind == ECosmicRungPartKind::Project && Next.Parts[0].Key == Dyson);
			TestTrue(TEXT("(the second is physics)"), Next.Parts[1].Kind == ECosmicRungPartKind::Research && Next.Parts[1].Key == Physics);
			TestFalse(TEXT("the swarm is not met"), Next.Parts[0].bMet);
			TestFalse(TEXT("physics is not met"), Next.Parts[1].bMet);
		}
		TestFalse(TEXT("Not all met"), Next.AllMet());
	}

	// ---- 2. The words ----
	{
		const FString Text = TextNow();
		TestTrue(TEXT("The words start with Rung.Next"), Text.StartsWith(NextPrefix, ESearchCase::CaseSensitive));
		TestTrue(TEXT("name the swarm"), Text.Contains(DysonRow->DisplayName.ToString()));
		TestTrue(TEXT("say it has not started"), Text.Contains(CosmicUI::Label(TEXT("Rung.NotStarted")).ToString()));
		TestTrue(TEXT("name physics"), Text.Contains(CosmicUI::Label(TEXT("Research.Physics")).ToString()));
		TestTrue(TEXT("hold the share needed"), Text.Contains(CosmicUI::Percent(0.5f).ToString()));
		TestTrue(TEXT("and the share held"), Text.Contains(CosmicUI::Percent(0.34f).ToString()));
		TestFalse(TEXT("leave no slot unfilled"), Text.Contains(TEXT("{")));
	}

	// ---- 3. Progress ----
	Live.Systems[Home].ActiveMegaproject = Dyson;
	Live.Systems[Home].MegaprojectProgress = 120.f;
	TestTrue(TEXT("A swarm under way shows its progress against what it needs"), TextNow().Contains(TEXT("(120/600)")));
	Live.Systems[Home].ActiveMegaproject = NAME_None;
	Live.Systems[Home].MegaprojectProgress = 0.f;

	// ---- 4. Done: the developer's example ----
	Record(Live.Systems[Home], Dyson, 5);
	{
		const FCosmicNextRung Next = NextOf();
		TestTrue(TEXT("A swarm finished since the vessel's birth is met"), Next.Parts.Num() == 2 && Next.Parts[0].bMet);
		const FText Expected = FText::Format(CosmicUI::Label(TEXT("Rung.Next")), CosmicUI::JoinList({
			FText::Format(INVTEXT("{0}({1})"), DysonRow->DisplayName, CosmicUI::Label(TEXT("Rung.Done"))),
			FText::Format(INVTEXT("{0} {1}({2})"), CosmicUI::Label(TEXT("Research.Physics")), CosmicUI::Percent(0.5f), CosmicUI::Percent(0.34f)) }));
		TestEqual(TEXT("The developer's example comes out exactly"), TextNow(), Expected.ToString());
	}

	// ---- 5. Before the birth ----
	Live.Systems[Home].CompletedMegaprojects.Reset();
	Record(Live.Systems[Home], Dyson, 50);
	Live.Calling.BornTick = 100;
	TestTrue(TEXT("A swarm finished before the vessel's birth counts for nothing"), NextOf().Parts.Num() == 2 && !NextOf().Parts[0].bMet);
	Live.Calling.BornTick = 0;

	// ---- 6. The percent forms ----
	{
		TestEqual(TEXT("0.496 against 0.5 holds 49, never 50"), HeldPercent(0.496f, 0.5f, false), 49);
		Reset();
		Live.Civs[Ruled].Research.Add(Physics, 0.496f);
		const FString Shown = TextNow();
		TestTrue(TEXT("and the words say 50%(49%)"), Shown.Contains(CosmicUI::Percent(0.5f).ToString() + TEXT("(") + CosmicUI::Percent(0.49f).ToString() + TEXT(")")));
		TestEqual(TEXT("0.346 reads 35%, as the bar shows it"), HeldPercent(0.346f, 0.5f, false), 35);
		TestEqual(TEXT("A met share reads as the bar rounds it"), HeldPercent(0.523f, 0.5f, true), 52);
		TestEqual(TEXT("Not a number reads 0"), WholePercent(std::numeric_limits<float>::quiet_NaN()), 0);
		TestEqual(TEXT("A share below 0 reads 0"), WholePercent(-0.2f), 0);
		TestEqual(TEXT("An unmet share under a need of 0 holds 0, not below"), HeldPercent(0.f, 0.f, false), 0);
		int32 Disagree = 0;
		for (int32 Thousandth = 0; Thousandth <= 1000; ++Thousandth)
		{
			const float Share = static_cast<float>(Thousandth) / 1000.f;
			Disagree += DigitsOf(CosmicUI::Percent(Share).ToString()) == WholePercent(Share) ? 0 : 1;
		}
		TestEqual(TEXT("WholePercent reads every thousandth as the bar shows it"), Disagree, 0);
		Reset();
	}

	// ---- 7. The judgment is the oracle ----
	{
		Reset();
		TestEqual(TEXT("Nothing met: every part met exactly when the judgment lifts the people"), NextOf().AllMet(), JudgmentLifts());
		Record(Live.Systems[Home], Dyson, 5);
		TestEqual(TEXT("The swarm only: every part met exactly when the judgment lifts the people"), NextOf().AllMet(), JudgmentLifts());
		Live.Systems[Home].CompletedMegaprojects.Reset();
		Record(Live.Systems[Home], Dyson, 50);
		Live.Calling.BornTick = 100;
		Live.Tick = 150;
		Live.Civs[Ruled].Research.Add(Physics, 0.6f);
		TestEqual(TEXT("The swarm before the birth and physics 0.6: every part met exactly when the judgment lifts the people"), NextOf().AllMet(), JudgmentLifts());
		Live.Calling.BornTick = 0;
		const bool bAll = NextOf().AllMet();
		const bool bLifts = JudgmentLifts();
		TestEqual(TEXT("The swarm since the birth and physics 0.6: every part met exactly when the judgment lifts the people"), bAll, bLifts);
		TestTrue(TEXT("(the judgment lifts the people in the last case)"), bLifts);
		Reset();
	}

	// ---- 8. The second rung ----
	{
		Live.Civs[Ruled].Tier = EAscensionTier::Tier2_Stellar;
		const FCosmicNextRung Next = NextOf();
		TestEqual(TEXT("The second rung's next rung is the stabilization"), Next.FeatRow, FName(TEXT("feat_supernova_stabilized")));
		const FString Words = TextNow();
		TestTrue(TEXT("An answer no slot holds names the hazard it answers"), Words.Contains(FText::Format(CosmicUI::Label(TEXT("Rung.WhenHazard")), NovaRow->DisplayName).ToString()));
		Live.Systems[Home].ActiveHazard = Tag(TEXT("Hazard.PreSupernova"));
		Live.Systems[Home].HazardTicksRemaining = 300;
		Live.Systems[Home].ResponseMegaproject = Stabilize;
		Live.Systems[Home].ResponseProgress = 12.f;
		TestTrue(TEXT("A stabilization under way shows its progress"), TextNow().Contains(TEXT("(12/100)")));
		Reset();
	}

	// ---- 9. Injected rows ----
	{
		const FName Build(TEXT("project_test_build"));
		const FName Rebuild(TEXT("project_test_rebuild"));
		UDataTable* Projects = CosmicTests::NewTable(FMegaprojectDef::StaticStruct());
		FMegaprojectDef Plain;
		Plain.DisplayName = FText::AsCultureInvariant(TEXT("<plain>"));
		Plain.RequiredProgress = 3.f;
		Projects->AddRow(Build, Plain);
		Projects->AddRow(Rebuild, Plain);
		UDataTable* Feats = CosmicTests::NewTable(FAscensionFeatDef::StaticStruct());
		// Lexically before the good row: one that skips a rung (the judgment refuses it, with its error line) and a
		// wanderer's row (Path is the judgment's too).
		Feats->AddRow(FName(TEXT("feat_test_aa")), FeatRow(EAscensionTier::Tier1_Planetary, EAscensionTier::Tier3_Causal, { Cond(EConditionType::MegaprojectComplete, Build) }));
		Feats->AddRow(FName(TEXT("feat_test_ab_walk")), FeatRow(EAscensionTier::Tier1_Planetary, EAscensionTier::Tier2_Stellar, { Cond(EConditionType::MegaprojectComplete, Build) }, EPlayerRole::Wanderer));
		Feats->AddRow(FName(TEXT("feat_test_two")), FeatRow(EAscensionTier::Tier1_Planetary, EAscensionTier::Tier2_Stellar,
			{ Cond(EConditionType::MegaprojectComplete, Build), Cond(EConditionType::MegaprojectComplete, Rebuild) }));
		FCosmicDataTables Injected = GameTables;
		Injected.Megaprojects = Projects;
		Injected.Feats = Feats;
		Sim->SetTables(Injected);
		Reset();
		// A second settled system of the people: a wild living system handed over by hand.
		const int32 Second = Live.Systems.IndexOfByPredicate([](const FStarSystemState& System) { return System.OwnerCivID.IsNone() && !CosmicHazards::IsDeadNode(System); });
		if (TestTrue(TEXT("(a wild living system to hand over)"), Live.Systems.IsValidIndex(Second)))
		{
			Live.Systems[Second].OwnerCivID = Live.Civs[Ruled].CivID;
			Live.Civs[Ruled].OwnedSystems.Add(Live.Systems[Second].SystemID);
			TestEqual(TEXT("A row the judgment refuses is never the next rung"), NextOf().FeatRow, FName(TEXT("feat_test_two")));
			AddExpectedError(TEXT("is not one rung up with a condition"), EAutomationExpectedErrorFlags::Contains, 0);
			Record(Live.Systems[Home], Build, 5);
			Record(Live.Systems[Second], Rebuild, 5);
			TestFalse(TEXT("Parts on two systems are not all met"), NextOf().AllMet());
			TestFalse(TEXT("and the judgment does not lift the people"), JudgmentLifts());
			Record(Live.Systems[Home], Rebuild, 5);
			TestTrue(TEXT("Both parts on one system are all met"), NextOf().AllMet());
			TestEqual(TEXT("(read on that system)"), NextOf().SystemIndex, Home);
			TestTrue(TEXT("and the judgment lifts the people"), JudgmentLifts());
			Live.Systems[Second].OwnerCivID = NAME_None;
			Live.Civs[Ruled].OwnedSystems.Remove(Live.Systems[Second].SystemID);
		}

		// Other conditions: no number, folded into one part that reads 'not yet' while one of them is unmet.
		UDataTable* Others = CosmicTests::NewTable(FAscensionFeatDef::StaticStruct());
		Others->AddRow(FName(TEXT("feat_test_other")), FeatRow(EAscensionTier::Tier1_Planetary, EAscensionTier::Tier2_Stellar,
			{ Cond(EConditionType::SystemPressureAtLeast, NAME_None, 50.f), Cond(EConditionType::GalaxyTickAtLeast, NAME_None, 1.f) }));
		Injected.Feats = Others;
		Sim->SetTables(Injected);
		Reset();
		{
			const FCosmicNextRung Next = NextOf();
			TestTrue(TEXT("(two other parts)"), Next.Parts.Num() == 2 && Next.Parts[0].Kind == ECosmicRungPartKind::Other && Next.Parts[1].Kind == ECosmicRungPartKind::Other);
			const FString Words = TextNow();
			const FString OtherWord = CosmicUI::Label(TEXT("Rung.Other")).ToString();
			TestFalse(TEXT("Another condition shows no number"), HasAsciiDigit(Words));
			TestTrue(TEXT("and says Rung.Other"), Words.Contains(OtherWord));
			TestEqual(TEXT("once for all of them"), Words.Len() - Words.Replace(*OtherWord, TEXT("")).Len(), OtherWord.Len());
			TestTrue(TEXT("reading 'not yet' while one is unmet"), Words.Contains(CosmicUI::Label(TEXT("Rung.NotMet")).ToString()));
		}
		Others->RemoveRow(FName(TEXT("feat_test_other")));
		Others->AddRow(FName(TEXT("feat_test_other")), FeatRow(EAscensionTier::Tier1_Planetary, EAscensionTier::Tier2_Stellar,
			{ Cond(EConditionType::SystemPressureAtLeast, NAME_None, 0.f), Cond(EConditionType::GalaxyTickAtLeast, NAME_None, 1.f) }));
		TestTrue(TEXT("and Rung.Met when all of them hold"), TextNow().Contains(CosmicUI::Label(TEXT("Rung.Met")).ToString()));
		Sim->SetTables(GameTables);
		Reset();
	}

	// ---- 10. No next rung ----
	{
		Live.Civs[Ruled].Tier = EAscensionTier::Tier3_Causal;
		TestTrue(TEXT("The third rung has no next rung in the game's rows"), NextOf().FeatRow.IsNone());
		TestTrue(TEXT("and no words"), TextNow().IsEmpty());
		Reset();
		Live.Player.Role = EPlayerRole::Wanderer;
		TestTrue(TEXT("A wanderer has no next rung"), NextOf().FeatRow.IsNone());
		Live.Player.Role = EPlayerRole::Ruler;
		Live.Calling.LifeState = ELifeState::Interregnum;
		TestTrue(TEXT("Nor has the interregnum"), NextOf().FeatRow.IsNone());
		Live.Calling.LifeState = ELifeState::Embodied;
		// Every label this test reads has words: a search for a blank label finds it anywhere, and its length is 0.
		for (const TCHAR* Key : { TEXT("Rung.Next"), TEXT("Rung.Done"), TEXT("Rung.NotStarted"), TEXT("Rung.WhenHazard"), TEXT("Rung.Other"), TEXT("Rung.Met"), TEXT("Rung.NotMet"), TEXT("List.Pair"),
			TEXT("Research.Physics"), TEXT("Panel.Tier"), TEXT("Speed.ThirdRungTip") })
		{
			TestFalse(*FString::Printf(TEXT("%s is no missing String Table entry"), Key), CosmicUI::Label(Key).ToString().Contains(Placeholder));
			TestFalse(*FString::Printf(TEXT("%s has words"), Key), CosmicUI::Label(Key).ToString().IsEmpty());
		}
	}

	// ---- The bar: one bound tooltip on the rung and on the research ----
	{
		Reset();
		UCosmicTopBarWidget* Bar = CreateWidget<UCosmicTopBarWidget>(Controller);
		if (!TestNotNull(TEXT("The top bar can be created"), Bar))
		{
			return false;
		}
		// The tree is built with the Slate widget.
		const TSharedRef<SWidget> SlateBar = Bar->TakeWidget();
		if (!TestNotNull(TEXT("(it has its rung text)"), Bar->GetRungText()) || !TestNotNull(TEXT("(and its research text)"), Bar->GetResearchText()))
		{
			return false;
		}
		Bar->Refresh();
		const FString RungWords = Bar->GetRungText()->GetText().ToString();
		TestTrue(TEXT("The rung text names the people"), RungWords.Contains(Live.Civs[Ruled].CivName.ToString()));
		TestTrue(TEXT("and its rung"), RungWords.Contains(FString::Printf(TEXT("%s 1"), *CosmicUI::Label(TEXT("Panel.Tier")).ToString())));
		TestTrue(TEXT("The research text names physics"), Bar->GetResearchText()->GetText().ToString().Contains(CosmicUI::Label(TEXT("Research.Physics")).ToString()));
		// The block that yields when the band runs short (the third rung in years with the mouse on the clock): clipped to
		// its slot and ended with an ellipsis, never drawn over the clock.
		TestTrue(TEXT("The research text is clipped to its slot"), Bar->GetResearchText()->GetClipping() == EWidgetClipping::ClipToBounds);
		TestTrue(TEXT("and ends with an ellipsis when it runs short"), Bar->GetResearchText()->GetTextOverflowPolicy() == ETextOverflowPolicy::Ellipsis);
		const TSharedPtr<SWidget> SlateRung = Bar->GetRungText()->GetCachedWidget();
		const TSharedPtr<SWidget> SlateResearch = Bar->GetResearchText()->GetCachedWidget();
		const bool bRungTip = TestTrue(TEXT("The rung text has a Slate tooltip"), SlateRung.IsValid() && SlateRung->GetToolTip().IsValid());
		const bool bResearchTip = TestTrue(TEXT("The research text has one too"), SlateResearch.IsValid() && SlateResearch->GetToolTip().IsValid());
		if (bRungTip && bResearchTip)
		{
			TestFalse(TEXT("it is not empty on the first rung"), SlateRung->GetToolTip()->IsEmpty());
			TestFalse(TEXT("nor the research one"), SlateResearch->GetToolTip()->IsEmpty());
			TestEqual(TEXT("The tooltip's words are the next rung's"), Bar->GetNextRungTip().ToString(), TextNow());
			// The research block's own (MVP item 7): the focus first, then the same next rung.
			const FString ResearchTip = Bar->GetResearchTip().ToString();
			TestTrue(TEXT("The research tooltip names the focus first"), ResearchTip.StartsWith(FText::Format(CosmicUI::Label(TEXT("Focus.BarLine")), CosmicUI::FocusWordOf(Live.Civs[Ruled].ResearchFocus, Sim->GetRules().ResearchFocusShare), CosmicUI::FocusSharesOf(Live.Civs[Ruled].ResearchFocus)).ToString()));
			TestTrue(TEXT("and ends with the next rung"), ResearchTip.EndsWith(TextNow()));
			TestEqual(TEXT("The research block's tooltip is bound to its own words"), Bar->GetResearchText()->ToolTipTextDelegate.GetFunctionName(), GET_FUNCTION_NAME_CHECKED(UCosmicTopBarWidget, GetResearchTip));
			const IToolTip* TipBefore = SlateRung->GetToolTip().Get();
			const IToolTip* ResearchTipBefore = SlateResearch->GetToolTip().Get();
			StepBroadcast(*Sim);
			TestTrue(TEXT("a tick keeps the tooltip object"), SlateRung->GetToolTip().Get() == TipBefore);
			TestTrue(TEXT("and the research one's"), SlateResearch->GetToolTip().Get() == ResearchTipBefore);
			Live.Civs[Ruled].Tier = EAscensionTier::Tier3_Causal;
			Bar->Refresh();
			// Read once and guarded: a text set in place of the binding takes the tooltip away when it is empty, and an
			// assertion does not stop the test before the pointer is used.
			const TSharedPtr<IToolTip> TipOnThird = SlateRung->GetToolTip();
			TestTrue(TEXT("(on the third rung the rung keeps its tooltip object)"), TipOnThird.Get() == TipBefore);
			TestTrue(TEXT("On the third rung the same tooltip is empty"), TipOnThird.IsValid() && TipOnThird->IsEmpty());
			Live.Civs[Ruled].Tier = EAscensionTier::Tier1_Planetary;
		}
		Bar->SetRungHovered(true);
		TestTrue(TEXT("The mouse on the rung lights it as the clock"), Bar->GetRungText()->GetColorAndOpacity().GetSpecifiedColor().Equals(Gold, 1e-4f));
		Bar->SetRungHovered(false);
		TestTrue(TEXT("and off it the rung is white again"), Bar->GetRungText()->GetColorAndOpacity().GetSpecifiedColor().Equals(FLinearColor::White, 1e-4f));
		// The gold says a tooltip waits there: with no next rung to tell (the third rung of the game's rows) it stays white.
		Live.Civs[Ruled].Tier = EAscensionTier::Tier3_Causal;
		Bar->Refresh();
		TestTrue(TEXT("(the third rung has no next rung's words)"), Bar->GetNextRungTip().IsEmpty());
		Bar->SetRungHovered(true);
		TestTrue(TEXT("With no next rung the mouse leaves the rung white"), Bar->GetRungText()->GetColorAndOpacity().GetSpecifiedColor().Equals(FLinearColor::White, 1e-4f));
		Bar->SetRungHovered(false);
		Live.Civs[Ruled].Tier = EAscensionTier::Tier1_Planetary;
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

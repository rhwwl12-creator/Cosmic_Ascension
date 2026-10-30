// Cosmic Ascension - automation tests of when the screens draw the vessel (Presentation, B5 [B-5c]): the controller's one answer
// (ACosmicPlayerController::IsVesselShown) that the top bar's wanderer line and lethal warning, the map's mark and lane and the
// system panel's wanderer block read - none in the interregnum, none while a calling page shows, the new vessel after the
// inauguration's close, and never the vessel an ended run's last tick installed.
// Design: docs/60_DECISIONS/headers/B5_death_succession_two_books.md section 2.6, 2.10, 4.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/UserWidget.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Core/CosmicTags.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicCallingPagesWidget.h"
#include "UI/CosmicChronicleWidget.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "UI/CosmicUILabels.h"
#include "UObject/Package.h"
#include "Widgets/SWidget.h"
#include "World/CosmicAutoPause.h"
#include "World/CosmicGalaxyMapActor.h"
#include "World/CosmicGameMode.h"
#include "World/CosmicMapLook.h"

namespace CosmicVesselShownTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// What a text block shows; empty while it is collapsed.
	FString Shown(const UTextBlock* Text)
	{
		return (Text && Text->GetVisibility() != ESlateVisibility::Collapsed) ? Text->GetText().ToString() : FString();
	}

	// The lines of a system panel.
	FString PanelLines(const UCosmicSystemPanelWidget& Panel)
	{
		return Panel.GetDetailText() ? Panel.GetDetailText()->GetText().ToString() : FString();
	}

	const UCosmicActionButton* ButtonOf(const UCosmicSystemPanelWidget& Panel, FName Action)
	{
		for (const UCosmicActionButton* Button : Panel.GetWandererButtons())
		{
			if (Button && Button->GetAction() == Action)
			{
				return Button;
			}
		}
		return nullptr;
	}

	// The game's rules with a leap of exactly Ticks (one number either way - decision 3).
	FCosmicRulesDef LeapOf(const FCosmicRulesDef& Base, int32 Ticks)
	{
		FCosmicRulesDef Rules = Base;
		Rules.LeapTicksMin = Ticks;
		Rules.LeapTicksMax = Ticks;
		return Rules;
	}

	// The tables with a KillPlayer in the supernova precursor's expiry: the game's own data kills nobody until [B-5d2], so the
	// lethal warning is seen with a test row of this file's own (UI.LethalWarning's form).
	FCosmicDataTables WithLethalPrecursor(const FCosmicDataTables& Base)
	{
		FCosmicDataTables Tables = Base;
		if (UDataTable* Hazards = CosmicTests::CopyWithout<FHazardDef>(Base.Hazards, {}))
		{
			for (const TPair<FName, uint8*>& Row : Hazards->GetRowMap())
			{
				FHazardDef& Hazard = *reinterpret_cast<FHazardDef*>(Row.Value);
				if (Hazard.HazardTag == Tag(TEXT("Hazard.PreSupernova")))
				{
					FCosmicEffect Kill;
					Kill.Type = EEffectType::KillPlayer;
					Kill.Tag = Tag(TEXT("Death.Hazard"));
					Hazard.OnExpireEffects.Add(Kill);
				}
			}
			Tables.Hazards = Hazards;
		}
		return Tables;
	}

	// A death recorded on the live state as Apply records one: the tenth stage of the next tick carries it out.
	void RecordDeath(FGalaxyState& State)
	{
		State.Calling.PendingEffect = CosmicTags::EffectKillPlayer();
		State.Calling.PendingCause = Tag(TEXT("Death.Hazard"));
		State.Calling.PendingSourceRow = FName(TEXT("crisis_pre_supernova"));
	}

	// A new galaxy of the home row (seed 42; its vessel rules), paused, at tick At - 1: the next StepOnce is the tick At.
	FGalaxyState& LayAt(UGalaxySimulationSubsystem& Sim, int64 At)
	{
		Sim.NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		Sim.SetTimeScale(0.f);
		FGalaxyState& State = Sim.GetMutableState();
		State.Tick = At - 1;
		Sim.SyncRngState();
		return State;
	}

	struct FScreen
	{
		UGalaxySimulationSubsystem* Sim = nullptr;
		ACosmicPlayerController* Controller = nullptr;
		UCosmicCallingPagesWidget* Pages = nullptr;
		UCosmicChronicleWidget* Chronicle = nullptr;
		UCosmicTopBarWidget* Bar = nullptr;
		ACosmicGalaxyMapActor* Map = nullptr;
	};

	// The live controller, HUD widgets and map (UI.Epitaph's form); false (with a test error) when one is missing.
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
		const ACosmicHUD* HUD = Out.Controller ? Out.Controller->GetHUD<ACosmicHUD>() : nullptr;
		Out.Pages = HUD ? HUD->GetCallingPages() : nullptr;
		Out.Chronicle = HUD ? HUD->GetChronicle() : nullptr;
		Out.Bar = HUD ? HUD->GetTopBar() : nullptr;
		if (World)
		{
			for (TActorIterator<ACosmicGalaxyMapActor> It(World); It; ++It)
			{
				Out.Map = *It;
			}
		}
		return Test.TestNotNull(TEXT("The HUD has its calling pages"), Out.Pages) && Test.TestNotNull(TEXT("and its chronicle"), Out.Chronicle)
			&& Test.TestNotNull(TEXT("and its top bar"), Out.Bar) && Test.TestNotNull(TEXT("The map stands"), Out.Map);
	}
}

// When the screens draw the vessel (header section 2.10, 4; critic-2 and SCREEN-2 of the [B-5c] review): one answer of the
// controller (IsVesselShown), true for a widget without a controller. In the interregnum the map draws no vessel - no mark and no
// lane, under the epitaph and with the pages folded (a dead ruler walks as a wanderer: only the life state hides it). After the
// leap the digest and the inauguration hold the new vessel back - no wanderer's line and no lethal warning on the bar, no mark on
// the map, no wanderer's command or warning on its system's panel - and the inauguration's close shows them all, the map redrawn
// by the fold alone. The reign end's page holds the walking vessel back the same way until it folds. A succession on the end's
// tick: its vessel is never drawn after the chronicle's 'look at the map' - and a vessel born before the end's tick still is.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIVesselShownTest, "Cosmic_Ascension.UI.VesselShown", CosmicTests::Flags)
bool FCosmicUIVesselShownTest::RunTest(const FString& Parameters)
{
	using namespace CosmicVesselShownTests;
	using CosmicUI::Label;

	FScreen Screen;
	if (!Acquire(*this, Screen))
	{
		return false;
	}
	UGalaxySimulationSubsystem* Sim = Screen.Sim;
	ACosmicPlayerController* Controller = Screen.Controller;
	UCosmicCallingPagesWidget* Pages = Screen.Pages;
	UCosmicTopBarWidget* Bar = Screen.Bar;
	ACosmicGalaxyMapActor* Map = Screen.Map;
	UCosmicSystemPanelWidget* Panel = CreateWidget<UCosmicSystemPanelWidget>(Controller);
	if (!TestNotNull(TEXT("(a system panel of the test's own)"), Panel))
	{
		return false;
	}
	const TSharedRef<SWidget> SlatePanel = Panel->TakeWidget();
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef GameRules = Sim->GetRules();
	const bool bViewerChoice = Controller->IsAutoPauseOn();
	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	ON_SCOPE_EXIT
	{
		Controller->SelectSystem(NAME_None);
		Controller->SetAutoPause(bViewerChoice);
		Controller->SetProvisionalEndEnabled(true);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Controller->SetAutoPause(false);
	Controller->SetProvisionalEndEnabled(true);
	Sim->SetBroadcastEnabled(true);
	Sim->SetTables(WithLethalPrecursor(CosmicTests::WithRules(Original, LeapOf(GameRules, 40))));
	const FCosmicDataTables& Tables = Sim->GetTables();
	const FCosmicRulesDef& Rules = Sim->GetRules();
	const FGameplayTag Precursor = Tag(TEXT("Hazard.PreSupernova"));
	const FHazardDef* PrecursorRow = Tables.FindHazardByTag(Precursor);
	if (!TestTrue(TEXT("(the test's tables: a precursor that kills at its expiry, a leap of 40)"), PrecursorRow && CosmicHazards::KillsAVesselOf(Tables, Precursor)
		&& Rules.LeapTicksMin == 40 && Rules.LeapTicksMax == 40))
	{
		return false;
	}
	const FString PanelWarning = Label(TEXT("Wander.Lethal")).ToString();
	FFormatNamedArguments HazardArgs;
	HazardArgs.Add(TEXT("Hazard"), PrecursorRow->DisplayName);
	const FString BarStanding = FText::Format(Label(TEXT("Wander.Bar.Lethal")), HazardArgs).ToString();
	// The mark a refresh draws now (whatever was pending).
	auto MarkNow = [&]()
	{
		Map->RequestRefresh();
		Map->RefreshNow();
		return Map->GetDrawnVesselSystem();
	};
	// A lethal hazard where the vessel stands whose expiry falls after the birth protection: the shared rule reaches the vessel
	// there (CosmicAutoPause::LethalPlaceOf), so only the predicate keeps its warnings away.
	auto PlaceLethalAt = [&](FGalaxyState& Live, FName SystemID)
	{
		const int32 At = Sim->GetStateIndex().FindSystem(Live, SystemID);
		if (Live.Systems.IsValidIndex(At))
		{
			Live.Systems[At].ActiveHazard = Precursor;
			Live.Systems[At].HazardTicksRemaining = Rules.BirthProtectionTicks + 100;
		}
		return At != INDEX_NONE && CosmicAutoPause::LethalPlaceOf(Live, Sim->GetStateIndex(), Tables, Rules) == At;
	};

	// ---- 0. Without a controller the vessel is shown (a widget of a test's own keeps its lines) ----
	{
		// An object of the class with no player context (never initialized, never shown).
		const UCosmicTopBarWidget* Lone = NewObject<UCosmicTopBarWidget>(GetTransientPackage());
		TestTrue(TEXT("A widget without an owning controller shows the vessel"), Lone && !Lone->GetOwningPlayer() && ACosmicPlayerController::IsVesselShownFor(*Lone));
	}

	// ---- 1. The interregnum: the map draws no vessel (header section 2.10) ----
	{
		FGalaxyState& Live = LayAt(*Sim, 400);
		RecordDeath(Live);
		Controller->SetSpeed(1.f);
		Sim->StepOnce();
		Map->RefreshNow();
		if (!TestTrue(TEXT("(the ruler died at 400: the interregnum under its epitaph)"), Live.Calling.LifeState == ELifeState::Interregnum
			&& Pages->GetPage() == ECosmicCallingPage::Epitaph))
		{
			return false;
		}
		TestTrue(TEXT("(a dead ruler walks as a wanderer where it stood - only the life state hides it)"), Live.Player.Role == EPlayerRole::Wanderer
			&& !Live.Player.CurrentSystemID.IsNone());
		TestFalse(TEXT("Under a calling page the controller does not show the vessel"), Controller->IsVesselShown());
		TestTrue(TEXT("The map draws no vessel under the epitaph"), MarkNow().IsNone() && !Map->IsVesselLaneDrawn());
		// The pages folded by hand: nothing but the interregnum hides it now.
		Pages->Close();
		TestTrue(TEXT("(folded, the controller would show a vessel)"), Controller->IsVesselShown());
		TestEqual(TEXT("The dead vessel has no mark (CosmicMapLook::VesselLookOf)"), CosmicMapLook::VesselLookOf(Live, Sim->GetStateIndex(), Rules).FromIndex,
			static_cast<int32>(INDEX_NONE));
		TestTrue(TEXT("In the interregnum the map draws no vessel"), MarkNow().IsNone());
		TestFalse(TEXT("and lights no lane"), Map->IsVesselLaneDrawn());
		Bar->Refresh();
		TestTrue(TEXT("nor does the bar show a wanderer's line"), Shown(Bar->GetWandererText()).IsEmpty());
	}

	// ---- 2. A death and its leap: the digest and the inauguration hold the new vessel back until the inauguration's close ----
	{
		FGalaxyState& Live = LayAt(*Sim, 400);
		RecordDeath(Live);
		Controller->SetSpeed(1.f);
		Sim->StepOnce();
		Map->RefreshNow();
		Pages->PressVeil();
		Pages->PressNext();
		if (!TestTrue(TEXT("(the epitaph's 'next' ran the leap: a new vessel stands at 440, the digest over it)"), Pages->GetPage() == ECosmicCallingPage::Digest
			&& Live.Tick == 440 && Live.Calling.Lives.Num() == 2 && CosmicWanderer::IsEmbodiedWanderer(Live)))
		{
			return false;
		}
		const FCosmicStateIndex& Index = Sim->GetStateIndex();
		const FName Home = Live.Player.CurrentSystemID;
		TestTrue(TEXT("(a lethal hazard where it stands reaches it at its expiry)"), PlaceLethalAt(Live, Home));
		const FString Line = UCosmicTopBarWidget::WandererLineOf(Live, Index, Controller->GetTimeUnit()).ToString();
		TestFalse(TEXT("(the state has a wanderer's line)"), Line.IsEmpty());
		TestFalse(TEXT("Under the digest the vessel is not shown"), Controller->IsVesselShown());
		Bar->Refresh();
		TestTrue(TEXT("The bar shows no wanderer's line under the digest"), Shown(Bar->GetWandererText()).IsEmpty());
		TestTrue(TEXT("nor the lethal warning"), Shown(Bar->GetLethalText()).IsEmpty());
		TestTrue(TEXT("The map draws no mark under the digest"), MarkNow().IsNone() && !Map->IsVesselLaneDrawn());
		Panel->ShowSystem(Home);
		TestEqual(TEXT("The panel of its system has no wanderer's command"), Panel->GetWandererButtons().Num(), 0);
		TestFalse(TEXT("nor the lethal warning"), PanelLines(*Panel).Contains(PanelWarning));
		// The inauguration introduces it: still held back.
		Pages->PressVeil();
		Pages->PressNext();
		TestTrue(TEXT("(the inauguration shows)"), Pages->GetPage() == ECosmicCallingPage::Inauguration);
		Bar->Refresh();
		TestTrue(TEXT("Under the inauguration the bar shows no wanderer's line"), Shown(Bar->GetWandererText()).IsEmpty());
		TestTrue(TEXT("nor the map a mark"), MarkNow().IsNone());
		// Its system selected already: the close's own selection asks the map for nothing, so the mark can come from the fold alone.
		Controller->SelectSystem(Home);
		Map->RefreshNow();
		TestTrue(TEXT("(selected, still no mark)"), Controller->GetSelectedSystem() == Home && Map->GetDrawnVesselSystem().IsNone());
		Pages->PressVeil();
		Pages->PressNext();
		TestFalse(TEXT("(the inauguration's close folds the pages)"), Pages->IsOpen());
		TestTrue(TEXT("Folded, the vessel is shown"), Controller->IsVesselShown());
		Map->RefreshNow();
		TestEqual(TEXT("The fold redraws the map: the new vessel's mark on its system"), Map->GetDrawnVesselSystem(), Home);
		TestEqual(TEXT("and the bar its wanderer's line"), Shown(Bar->GetWandererText()), Line);
		TestEqual(TEXT("and its lethal warning"), Shown(Bar->GetLethalText()), BarStanding);
		Panel->Refresh();
		TestNotNull(TEXT("The panel of its system has its [Scan]"), ButtonOf(*Panel, UCosmicSystemPanelWidget::ScanAction));
		TestTrue(TEXT("and its lethal warning"), PanelLines(*Panel).Contains(PanelWarning));
	}

	// ---- 3. An abdication: the reign end's page holds the walking vessel back until it folds ----
	{
		FGalaxyState& Ruling = LayAt(*Sim, 400);
		Map->RefreshNow();
		if (!TestTrue(TEXT("(the abdication is recorded)"), Controller->Abdicate()))
		{
			return false;
		}
		Controller->SetSpeed(1.f);
		Sim->StepOnce();
		if (!TestTrue(TEXT("(the reign end's page shows over a wanderer of the same life)"), Pages->GetPage() == ECosmicCallingPage::ReignEnd
			&& CosmicWanderer::IsEmbodiedWanderer(Ruling) && Ruling.Calling.Lives.Num() == 1))
		{
			return false;
		}
		const FName Here = Ruling.Player.CurrentSystemID;
		TestTrue(TEXT("(a lethal hazard where it stands reaches it at its expiry)"), PlaceLethalAt(Ruling, Here));
		TestFalse(TEXT("Under the reign end the vessel is not shown"), Controller->IsVesselShown());
		Bar->Refresh();
		TestTrue(TEXT("The bar shows no wanderer's line under the reign end"), Shown(Bar->GetWandererText()).IsEmpty());
		TestTrue(TEXT("nor the lethal warning"), Shown(Bar->GetLethalText()).IsEmpty());
		TestTrue(TEXT("nor the map a mark"), MarkNow().IsNone());
		Panel->ShowSystem(Here);
		TestEqual(TEXT("nor the panel a wanderer's command"), Panel->GetWandererButtons().Num(), 0);
		TestFalse(TEXT("or the lethal warning"), PanelLines(*Panel).Contains(PanelWarning));
		Pages->PressVeil();
		Pages->PressNext();
		TestFalse(TEXT("(the reign end folded)"), Pages->IsOpen());
		TestFalse(TEXT("The fold redraws the bar: the wanderer's line"), Shown(Bar->GetWandererText()).IsEmpty());
		TestEqual(TEXT("and the lethal warning"), Shown(Bar->GetLethalText()), BarStanding);
		Map->RefreshNow();
		TestEqual(TEXT("and the map: the mark where the vessel stands"), Map->GetDrawnVesselSystem(), Here);
		Panel->Refresh();
		TestNotNull(TEXT("The panel has its [Scan] again"), ButtonOf(*Panel, UCosmicSystemPanelWidget::ScanAction));
		TestTrue(TEXT("and its lethal warning"), PanelLines(*Panel).Contains(PanelWarning));
	}

	// ---- 4. A succession on the end's tick (the end wins - header section 2.6, 4): the vessel the player never saw ----
	const int64 EndTick = Rules.ProvisionalEndTick;
	if (TestTrue(TEXT("(the run has a clock end beyond a leap)"), EndTick > 41))
	{
		FGalaxyState& Last = LayAt(*Sim, EndTick - 40);
		RecordDeath(Last);
		Controller->SetSpeed(1.f);
		Sim->StepOnce();
		Map->RefreshNow();
		Pages->PressVeil();
		Pages->PressNext();
		if (!TestTrue(TEXT("(the leap ends on the end's tick: a new vessel stands there and the end latches)"), Last.Tick == EndTick && Controller->IsRunEnded()
			&& Last.Calling.Lives.Num() == 2 && Last.Calling.Lives.Last().BornTick == EndTick && CosmicWanderer::IsEmbodiedWanderer(Last)
			&& Pages->GetPage() == ECosmicCallingPage::Digest))
		{
			return false;
		}
		Pages->PressVeil();
		Pages->PressNext();
		TestTrue(TEXT("(no inauguration: the chronicle)"), !Pages->IsOpen() && Screen.Chronicle->IsOpen());
		Controller->LookAtMap();
		TestFalse(TEXT("(the chronicle folded over the map)"), Screen.Chronicle->IsOpen());
		const FName Home = Last.Player.CurrentSystemID;
		TestTrue(TEXT("(a lethal hazard where it stands would reach it at its expiry)"), PlaceLethalAt(Last, Home));
		TestFalse(TEXT("The vessel the end's tick installed is not shown"), Controller->IsVesselShown());
		TestTrue(TEXT("After 'look at the map' the map never draws it"), MarkNow().IsNone() && !Map->IsVesselLaneDrawn());
		Bar->Refresh();
		TestTrue(TEXT("nor the bar its wanderer's line"), Shown(Bar->GetWandererText()).IsEmpty());
		TestTrue(TEXT("or its lethal warning"), Shown(Bar->GetLethalText()).IsEmpty());
		Panel->ShowSystem(Home);
		TestEqual(TEXT("nor the panel of its system a command"), Panel->GetWandererButtons().Num(), 0);
		TestFalse(TEXT("or a warning"), PanelLines(*Panel).Contains(PanelWarning));
		// A vessel the run showed before its end is drawn: only the end's own tick's is held back (CosmicChronicle::ShownLivesOf).
		FLifeRecord& Called = Last.Calling.Lives.Last();
		Called.BornTick = EndTick - 1;
		TestTrue(TEXT("An ended run's vessel born before the end's tick is shown"), Controller->IsVesselShown());
		TestEqual(TEXT("(its mark)"), MarkNow(), Home);
		Called.BornTick = EndTick;
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

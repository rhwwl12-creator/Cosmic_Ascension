// Cosmic Ascension - automation tests of the top bar and the system panel (Presentation). Design: guideline.md section
// 5.7, 5.12, 8.3, 9.3, 11 Phase 3.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "CoreGlobals.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "Internationalization/StringTableCore.h"
#include "Layout/Geometry.h"
#include "Misc/AutomationTest.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/ScopeExit.h"
#include "Rendering/SlateLayoutTransform.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicLogFeedWidget.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "UI/CosmicUILabels.h"
#include "UObject/NameTypes.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/IToolTip.h"
#include "Widgets/SWidget.h"
#include "World/CosmicGalaxyMapActor.h"
#include "World/CosmicGameMode.h"

namespace CosmicPanelTests
{
	// Everything the text blocks of a widget's own tree show, one block per line.
	FString ReadTexts(const UUserWidget& Widget)
	{
		FString Shown;
		if (Widget.WidgetTree)
		{
			Widget.WidgetTree->ForEachWidget([&Shown](UWidget* Child)
			{
				if (const UTextBlock* Block = Cast<UTextBlock>(Child))
				{
					Shown += Block->GetText().ToString();
					Shown += TEXT("\n");
				}
			});
		}
		return Shown;
	}

	TArray<FName> ActionsOf(const UCosmicSystemPanelWidget& Panel)
	{
		TArray<FName> Actions;
		for (const UCosmicActionButton* Button : Panel.GetActionButtons())
		{
			Actions.Add(Button ? Button->GetAction() : NAME_None);
		}
		return Actions;
	}

	FString LabelText(const TCHAR* Key)
	{
		return CosmicUI::Label(Key).ToString();
	}

	// A wild system that no system of the people touches by a lane; INDEX_NONE when there is none.
	int32 FindUntouched(const UGalaxySimulationSubsystem& Sim, FName CivID)
	{
		const FGalaxyState& State = Sim.GetState();
		for (int32 Index = 0; Index < State.Systems.Num(); ++Index)
		{
			const FStarSystemState& System = State.Systems[Index];
			const bool bTouches = System.Lanes.ContainsByPredicate([&](FName Lane)
			{
				const int32 Other = Sim.GetStateIndex().FindSystem(State, Lane);
				return State.Systems.IsValidIndex(Other) && State.Systems[Other].OwnerCivID == CivID;
			});
			if (System.OwnerCivID.IsNone() && !bTouches)
			{
				return Index;
			}
		}
		return INDEX_NONE;
	}

	UDataTable* RulesTable(const FCosmicRulesDef& Rules)
	{
		UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
		Table->RowStruct = FCosmicRulesDef::StaticStruct();
		Table->AddRow(CosmicRows::Rules, Rules);
		return Table;
	}

	// The list view in a widget's own tree, or null.
	UListView* FindList(const UUserWidget& Widget)
	{
		UListView* Found = nullptr;
		if (Widget.WidgetTree)
		{
			Widget.WidgetTree->ForEachWidget([&Found](UWidget* Child)
			{
				if (!Found)
				{
					Found = Cast<UListView>(Child);
				}
			});
		}
		return Found;
	}

	// Lets a list view make its line widgets now. A list makes them on Slate's tick with its size on screen, and a
	// -nullrhi run gives it no size - the list is ticked once here with a size of its own.
	void DrawListNow(UListView& List)
	{
		const TSharedPtr<SWidget> Slate = List.GetCachedWidget();
		if (Slate.IsValid())
		{
			Slate->SlatePrepass(1.f);
			Slate->Tick(FGeometry::MakeRoot(FVector2f(600.f, 400.f), FSlateLayoutTransform()), FSlateApplication::Get().GetCurrentTime(), 0.f);
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUITopBarAndPanelTest, "Cosmic_Ascension.UI.TopBarAndPanel", CosmicTests::Flags)
bool FCosmicUITopBarAndPanelTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPanelTests;

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
	if (!TestNotNull(TEXT("The first player controller is the game's"), Controller))
	{
		return false;
	}
	const FCosmicDataTables GameTables = Sim->GetTables();
	const ECosmicTimeUnit ViewerUnit = Controller->GetTimeUnit();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(GameTables);
		Sim->SetTimeScale(0.f);
		Controller->SelectSystem(NAME_None);
		Controller->SetTimeUnit(ViewerUnit);
		Sim->NewGalaxy(42, 5, 3);
	};
	Sim->NewGalaxy(42, 0, 0);
	// The labels below are the cycle forms: the unit is fixed in memory (the developer's own choice may be years, and
	// a test never writes it - guideline 9.3 UI.TimeUnitToggle).
	Controller->SetTimeUnit(ECosmicTimeUnit::Cycle);
	const FString& Placeholder = FStringTableEntry::GetPlaceholderSourceString();
	FGalaxyState& Live = Sim->GetMutableState();
	const int32 RuledIndex = Sim->GetStateIndex().FindCiv(Live, Live.Player.RuledCivID);
	if (!TestTrue(TEXT("(the player rules a people)"), Live.Civs.IsValidIndex(RuledIndex)))
	{
		return false;
	}
	const FName RuledID = Live.Civs[RuledIndex].CivID;
	const FName Home = Live.Civs[RuledIndex].HomeSystemID;
	const int32 HomeIndex = Sim->GetStateIndex().FindSystem(Live, Home);
	if (!TestTrue(TEXT("(the ruled people has its home system)"), Live.Systems.IsValidIndex(HomeIndex)))
	{
		return false;
	}

	// ---- The top bar: the ruled people, the cycle, the speeds ----
	{
		// A cycle the bar cannot show by chance (the bar reads the state; the scope's new galaxy puts it back).
		Live.Tick = 77;
		UCosmicTopBarWidget* Bar = CreateWidget<UCosmicTopBarWidget>(Controller);
		if (!TestNotNull(TEXT("The top bar can be created"), Bar))
		{
			return false;
		}
		const TSharedRef<SWidget> SlateBar = Bar->TakeWidget();
		Bar->Refresh();
		const FString Shown = ReadTexts(*Bar);
		TestFalse(TEXT("No label of the top bar is a missing String Table entry"), Shown.Contains(Placeholder));
		TestTrue(TEXT("The top bar names the ruled people"), Shown.Contains(Live.Civs[RuledIndex].CivName.ToString()));
		TestTrue(TEXT("and its rung, 1 to 5"), Shown.Contains(FString::Printf(TEXT("%s %d"), *LabelText(TEXT("Panel.Tier")), static_cast<int32>(Live.Civs[RuledIndex].Tier) + 1)));
		TestTrue(TEXT("and the cycle"), Shown.Contains(CosmicUI::ClockLabel(Live.Tick, ECosmicTimeUnit::Cycle).ToString()));

		const TArray<TObjectPtr<UCosmicActionButton>>& Speeds = Bar->GetSpeedButtons();
		const float Expected[] = { 0.f, 1.f, 2.f, 5.f, 10.f };
		if (TestEqual(TEXT("Five speed buttons"), Speeds.Num(), 5))
		{
			for (int32 Index = 0; Index < 5; ++Index)
			{
				TestEqual(*FString::Printf(TEXT("Speed button %d sets %g"), Index, Expected[Index]), Speeds[Index]->GetValue(), Expected[Index]);
			}
			TestFalse(TEXT("Paused, the pause button is greyed out"), Speeds[0]->GetIsEnabled());
			TestTrue(TEXT("and x5 is not"), Speeds[3]->GetIsEnabled());
			Speeds[3]->Press();
			TestEqual(TEXT("x5 sets the speed"), Sim->GetTimeScale(), 5.f);
			TestFalse(TEXT("and is greyed out"), Speeds[3]->GetIsEnabled());
			TestTrue(TEXT("and the pause button is not"), Speeds[0]->GetIsEnabled());
			Speeds[0]->Press();
			TestEqual(TEXT("The pause button pauses"), Sim->GetTimeScale(), 0.f);
		}
	}

	UCosmicSystemPanelWidget* Panel = CreateWidget<UCosmicSystemPanelWidget>(Controller);
	if (!TestNotNull(TEXT("The system panel can be created"), Panel))
	{
		return false;
	}
	const TSharedRef<SWidget> SlatePanel = Panel->TakeWidget();

	// ---- No system: no panel ----
	Panel->ShowSystem(NAME_None);
	TestTrue(TEXT("With no system the panel is hidden"), Panel->GetVisibility() == ESlateVisibility::Collapsed);

	// ---- The ruled home: what it is, and one button per project it can start ----
	{
		Panel->ShowSystem(Home);
		TestTrue(TEXT("A shown system shows the panel"), Panel->GetVisibility() != ESlateVisibility::Collapsed);
		const FString Shown = ReadTexts(*Panel);
		TestFalse(TEXT("No label of the panel is a missing String Table entry"), Shown.Contains(Placeholder));
		TestTrue(TEXT("The panel names the system"), Shown.Contains(Live.Systems[HomeIndex].DisplayName.ToString()));
		TestTrue(TEXT("and its owner"), Shown.Contains(FString::Printf(TEXT("%s  %s"), *LabelText(TEXT("Panel.Owner")), *Live.Civs[RuledIndex].CivName.ToString())));
		const FString ClassName = Live.Systems[HomeIndex].StarClass.GetTagName().ToString();
		TestTrue(TEXT("and its star class by letter"), Shown.Contains(FString::Printf(TEXT("%s  %s"), *LabelText(TEXT("Panel.StarClass")), *ClassName.RightChop(ClassName.Find(TEXT("."), ESearchCase::CaseSensitive, ESearchDir::FromEnd) + 1))));

		TArray<FName> Startable = GameTables.Megaprojects ? GameTables.Megaprojects->GetRowNames() : TArray<FName>();
		Startable.Sort(FNameLexicalLess());
		Startable.RemoveAll([&](FName Row) { return !CosmicMegaprojects::CanStart(Live, Sim->GetStateIndex(), GameTables, HomeIndex, Row); });
		TestTrue(TEXT("(the home can start a project)"), !Startable.IsEmpty());
		TestTrue(TEXT("One button per project the home can start, in row order"), ActionsOf(*Panel) == Startable);
		TestFalse(TEXT("No reason line on a settled system"), Panel->IsReasonShown());
		if (!Panel->GetActionButtons().IsEmpty() && Panel->GetActionButtons()[0])
		{
			const FName Row = Panel->GetActionButtons()[0]->GetAction();
			Panel->GetActionButtons()[0]->Press();
			const FStarSystemState& After = Live.Systems[HomeIndex];
			TestTrue(TEXT("Pressing it starts the project"), After.ActiveMegaproject == Row || After.ResponseMegaproject == Row);
			TestFalse(TEXT("and its button goes"), ActionsOf(*Panel).Contains(Row));
			const FMegaprojectDef* Def = GameTables.FindMegaproject(Row);
			if (TestNotNull(TEXT("(the row is the game's)"), Def))
			{
				TestTrue(TEXT("The panel shows the project with its progress and what it needs"), ReadTexts(*Panel).Contains(FString::Printf(TEXT("%s 0/%s"),
					*Def->DisplayName.ToString(), *FText::AsNumber(FMath::FloorToInt32(CosmicMegaprojects::RequiredProgressOf(*Def))).ToString())));
			}
		}
	}

	// ---- Every reason a settle button can give has its words ----
	{
		using CosmicMegaprojects::ECosmicSettleBlock;
		// The reasons a button shows, by hand (the enum's count below catches one added without words here).
		const ECosmicSettleBlock Shown[] = { ECosmicSettleBlock::Owned, ECosmicSettleBlock::NoLane, ECosmicSettleBlock::NoRow, ECosmicSettleBlock::RowTier,
			ECosmicSettleBlock::RealmCap, ECosmicSettleBlock::Settling, ECosmicSettleBlock::Sphere };
		int32 Showable = 0;
		for (int32 At = 0; At < static_cast<int32>(ECosmicSettleBlock::Count); ++At)
		{
			const ECosmicSettleBlock Block = static_cast<ECosmicSettleBlock>(At);
			const FString Words = UCosmicSystemPanelWidget::ReasonOf(Block, GameTables).ToString();
			const bool bShows = Block != ECosmicSettleBlock::None && Block != ECosmicSettleBlock::NoPeople && Block != ECosmicSettleBlock::Dead;
			Showable += bShows ? 1 : 0;
			if (bShows)
			{
				TestFalse(*FString::Printf(TEXT("Reason %d has words"), At), Words.IsEmpty());
				TestFalse(*FString::Printf(TEXT("Reason %d is no missing String Table entry"), At), Words.Contains(Placeholder));
				TestFalse(*FString::Printf(TEXT("Reason %d leaves no argument unfilled"), At), Words.Contains(TEXT("{")));
			}
			else
			{
				TestTrue(*FString::Printf(TEXT("Reason %d, which no button shows, has no words"), At), Words.IsEmpty());
			}
		}
		TestEqual(TEXT("Every reason a button shows is named here"), Showable, static_cast<int32>(UE_ARRAY_COUNT(Shown)));
		// The rungs the words name, numbered as the top bar numbers rungs: the game's settle row asks for the second, and the
		// third lifts a sphere. Below the third rung the sphere's words name neither transcendent nor sphere (section 3.2).
		TestTrue(TEXT("The rung reason names the settle row's rung"), UCosmicSystemPanelWidget::ReasonOf(ECosmicSettleBlock::RowTier, GameTables).ToString().Contains(TEXT("2")));
		const FString SphereWords = UCosmicSystemPanelWidget::ReasonOf(ECosmicSettleBlock::Sphere, GameTables).ToString();
		TestTrue(TEXT("The sphere reason names the rung that lifts it"), SphereWords.Contains(TEXT("3")));
		// "Transcendent" and "sphere" in the String Table's language, escaped: the source files are ASCII (section 10.3).
		TestFalse(TEXT("and does not say transcendent"), SphereWords.Contains(TEXT("\xCD08\xC6D4\xC790")));
		TestFalse(TEXT("nor sphere"), SphereWords.Contains(TEXT("\xC601\xD5A5\xAD8C")));
		TestEqual(TEXT("A reason's words come from its String Table key"), UCosmicSystemPanelWidget::ReasonOf(ECosmicSettleBlock::Owned, GameTables).ToString(), LabelText(TEXT("Settle.Block.Owned")));
	}

	// ---- A wild neighbour: the settle button, then the greyed button and its reason ----
	{
		// On the third rung the settlement's tier and a transcendent's sphere do not stand in the way.
		Live.Civs[RuledIndex].Tier = EAscensionTier::Tier3_Causal;
		const TArray<int32> Targets = CosmicMegaprojects::SettleTargets(Live, Sim->GetStateIndex(), GameTables, Sim->GetRules(), RuledIndex);
		if (TestTrue(TEXT("(the ruled people has two wild systems to settle)"), Targets.Num() >= 2))
		{
			const FName First = Live.Systems[Targets[0]].SystemID;
			const FName Second = Live.Systems[Targets[1]].SystemID;
			Panel->ShowSystem(First);
			TestTrue(TEXT("A wild system the people can settle shows one settle button"), ActionsOf(*Panel) == TArray<FName>({ CosmicRows::ProjectSettle }));
			TestTrue(TEXT("and says it has no owner"), ReadTexts(*Panel).Contains(LabelText(TEXT("Panel.NoOwner"))));
			TestTrue(TEXT("The settle button of a target is enabled"), !Panel->GetActionButtons().IsEmpty() && Panel->GetActionButtons()[0]->GetIsEnabled());
			TestFalse(TEXT("and no reason line shows"), Panel->IsReasonShown());
			TestTrue(TEXT("(no reason is held)"), Panel->GetShownSettleBlock() == CosmicMegaprojects::ECosmicSettleBlock::None);

			// In a transcendent's sphere below the third rung: greyed out, the sphere's words as the line and the tooltip, and
			// a press does nothing.
			Live.Civs[RuledIndex].Tier = EAscensionTier::Tier2_Stellar;
			const int32 Unsealed = Live.Transcendents.IndexOfByPredicate([&](const FTranscendentState& Each) { return !Each.bSealed && GameTables.FindTranscendent(Each.TranscendentID) != nullptr; });
			if (TestTrue(TEXT("(the game's galaxy has an unsealed transcendent)"), Unsealed != INDEX_NONE))
			{
				Live.Transcendents[Unsealed].InfluencedSystems.Add(First);
				Panel->Refresh();
				const FString SphereWords = UCosmicSystemPanelWidget::ReasonOf(CosmicMegaprojects::ECosmicSettleBlock::Sphere, GameTables).ToString();
				TestTrue(TEXT("A neighbour in a transcendent's sphere gives the sphere as its reason"), Panel->GetShownSettleBlock() == CosmicMegaprojects::ECosmicSettleBlock::Sphere);
				TestTrue(TEXT("on the line"), ReadTexts(*Panel).Contains(SphereWords));
				TestTrue(TEXT("and as the tooltip"), !Panel->GetActionButtons().IsEmpty() && Panel->GetActionButtons()[0]->GetToolTipText().ToString() == SphereWords);
				// A press on the greyed-out button announces nothing (a click never reaches it; a press from code is refused
				// the same way) - counted on the button's own delegate, as the command would refuse the settlement anyway.
				int32 Announced = 0;
				if (UCosmicActionButton* Greyed = Panel->GetActionButtons().IsEmpty() ? nullptr : Panel->GetActionButtons()[0].Get())
				{
					const FDelegateHandle Counter = Greyed->OnAction.AddLambda([&Announced](FName, float) { ++Announced; });
					Greyed->Press();
					Greyed->OnAction.Remove(Counter);
				}
				TestEqual(TEXT("A press on the greyed-out button announces nothing"), Announced, 0);
				TestTrue(TEXT("and settles nothing"), Live.Systems[Targets[0]].OwnerCivID.IsNone());
				Live.Transcendents[Unsealed].InfluencedSystems.Remove(First);
			}
			Live.Civs[RuledIndex].Tier = EAscensionTier::Tier3_Causal;
			Panel->Refresh();
			int32 Announced = 0;
			if (UCosmicActionButton* Settle = Panel->GetActionButtons().IsEmpty() ? nullptr : Panel->GetActionButtons()[0].Get())
			{
				const FDelegateHandle Counter = Settle->OnAction.AddLambda([&Announced](FName, float) { ++Announced; });
				Settle->Press();
				Settle->OnAction.Remove(Counter);
			}
			TestEqual(TEXT("A press on the enabled button announces its action once"), Announced, 1);
			TestEqual(TEXT("Pressing it settles: the system is the people's"), Live.Systems[Targets[0]].OwnerCivID, RuledID);
			TestEqual(TEXT("with the settlement in its construction slot"), Live.Systems[Targets[0]].ActiveMegaproject, CosmicRows::ProjectSettle);

			// Every realm cap 2: the people owns two systems now, exactly its cap, so the other neighbour's settle button is
			// greyed out with the cap as its reason (line and tooltip), before the settlement under way - and a wild system the
			// people does not touch gives the lane as its reason (the system's own reasons come first).
			FCosmicRulesDef Capped = Sim->GetRules();
			Capped.RealmCapByTier = { 2, 2, 2, 2, 2 };
			FCosmicDataTables CappedTables = GameTables;
			CappedTables.Rules = RulesTable(Capped);
			Sim->SetTables(CappedTables);
			Panel->ShowSystem(Second);
			TestEqual(TEXT("At its realm cap a wild neighbour still shows the settle button"), Panel->GetActionButtons().Num(), 1);
			TestFalse(TEXT("greyed out"), !Panel->GetActionButtons().IsEmpty() && Panel->GetActionButtons()[0]->GetIsEnabled());
			TestTrue(TEXT("with the realm cap as its reason, before the settlement under way"), Panel->GetShownSettleBlock() == CosmicMegaprojects::ECosmicSettleBlock::RealmCap);
			TestTrue(TEXT("on a line"), Panel->IsReasonShown());
			TestTrue(TEXT("and as its tooltip"), !Panel->GetActionButtons().IsEmpty() && Panel->GetActionButtons()[0]->GetToolTipText().ToString() == LabelText(TEXT("Settle.Block.RealmCap")));
			TestTrue(TEXT("The line says so"), ReadTexts(*Panel).Contains(LabelText(TEXT("Settle.Block.RealmCap"))));
			const int32 Untouched = FindUntouched(*Sim, RuledID);
			if (TestTrue(TEXT("(a wild system away from the people)"), Untouched != INDEX_NONE))
			{
				Panel->ShowSystem(Live.Systems[Untouched].SystemID);
				TestTrue(TEXT("At the cap a wild system away from the people gives the lane as its reason"), Panel->GetShownSettleBlock() == CosmicMegaprojects::ECosmicSettleBlock::NoLane);
			}
			Sim->SetTables(GameTables);
			Panel->ShowSystem(Second);
			TestTrue(TEXT("Below the cap, the settlement under way is the reason"), Panel->GetShownSettleBlock() == CosmicMegaprojects::ECosmicSettleBlock::Settling);
		}
		// A wild system no settled system of the people touches: a greyed settle button that gives the lane as its reason.
		const int32 Far = FindUntouched(*Sim, RuledID);
		if (TestTrue(TEXT("(a wild system away from the people)"), Far != INDEX_NONE))
		{
			Panel->ShowSystem(Live.Systems[Far].SystemID);
			TestEqual(TEXT("A wild system away from the people shows the settle button"), Panel->GetActionButtons().Num(), 1);
			TestTrue(TEXT("with the lane as its reason"), Panel->GetShownSettleBlock() == CosmicMegaprojects::ECosmicSettleBlock::NoLane);

			// Destroyed: said so, no buttons.
			const FGameplayTag StarClass = Live.Systems[Far].StarClass;
			Live.Systems[Far].StarClass = FGameplayTag();
			Panel->Refresh();
			TestTrue(TEXT("A destroyed system says so"), ReadTexts(*Panel).Contains(LabelText(TEXT("Panel.Destroyed"))));
			TestEqual(TEXT("and has no button"), Panel->GetActionButtons().Num(), 0);
			Live.Systems[Far].StarClass = StarClass;
		}
		Live.Civs[RuledIndex].Tier = EAscensionTier::Tier1_Planetary;
	}

	// ---- Another people's system: a greyed settle button that names no people; the buttons outlive a tick ----
	{
		const int32 Other = Live.Systems.IndexOfByPredicate([&](const FStarSystemState& System) { return !System.OwnerCivID.IsNone() && System.OwnerCivID != RuledID; });
		if (TestTrue(TEXT("(another people owns a system)"), Other != INDEX_NONE))
		{
			Panel->ShowSystem(Live.Systems[Other].SystemID);
			TestTrue(TEXT("Another people's system shows the settle button"), ActionsOf(*Panel) == TArray<FName>({ CosmicRows::ProjectSettle }));
			TestFalse(TEXT("greyed out"), !Panel->GetActionButtons().IsEmpty() && Panel->GetActionButtons()[0]->GetIsEnabled());
			TestTrue(TEXT("because another people lives there"), Panel->GetShownSettleBlock() == CosmicMegaprojects::ECosmicSettleBlock::Owned);
			TestTrue(TEXT("The line says so"), ReadTexts(*Panel).Contains(LabelText(TEXT("Settle.Block.Owned"))));

			// The same content keeps the same button: a refresh, then a tick through the delegate (a button built again
			// between the press and the release of a click drops the click).
			const UCosmicActionButton* Before = Panel->GetActionButtons().IsEmpty() ? nullptr : Panel->GetActionButtons()[0].Get();
			Panel->Refresh();
			TestTrue(TEXT("A refresh with the same buttons keeps the button object"), !Panel->GetActionButtons().IsEmpty() && Panel->GetActionButtons()[0].Get() == Before);
			Controller->SelectSystem(Live.Systems[Other].SystemID);
			ACosmicHUD* HUD = Controller->GetHUD<ACosmicHUD>();
			UCosmicSystemPanelWidget* HudPanel = HUD ? HUD->GetSystemPanel() : nullptr;
			if (TestNotNull(TEXT("(the HUD has its system panel)"), HudPanel))
			{
				const UCosmicActionButton* HudBefore = HudPanel->GetActionButtons().IsEmpty() ? nullptr : HudPanel->GetActionButtons()[0].Get();
				const FString TooltipBefore = HudBefore ? HudBefore->GetToolTipText().ToString() : FString();
				// Every SetToolTipText makes a new Slate tooltip (an open one would close): the same words keep the object.
				const TSharedPtr<SWidget> SlateBefore = HudBefore ? HudBefore->GetCachedWidget() : TSharedPtr<SWidget>();
				const IToolTip* TipBefore = SlateBefore ? SlateBefore->GetToolTip().Get() : nullptr;
				const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
				Sim->SetBroadcastEnabled(true);
				Sim->StepOnce();
				Sim->SetBroadcastEnabled(bWasBroadcasting);
				TestTrue(TEXT("(the system is still the other people's)"), Live.Systems[Other].OwnerCivID != RuledID && !Live.Systems[Other].OwnerCivID.IsNone());
				TestTrue(TEXT("A tick keeps the button object"), HudBefore && !HudPanel->GetActionButtons().IsEmpty() && HudPanel->GetActionButtons()[0].Get() == HudBefore);
				TestEqual(TEXT("and its tooltip"), HudBefore ? HudBefore->GetToolTipText().ToString() : FString(), TooltipBefore);
				TestTrue(TEXT("(the greyed-out button has a tooltip)"), TipBefore != nullptr);
				TestTrue(TEXT("and the tick keeps the tooltip object"), SlateBefore && SlateBefore->GetToolTip().Get() == TipBefore);
			}
			Controller->SelectSystem(NAME_None);
		}
		// A wanderer has no commands.
		Live.Systems[HomeIndex].ActiveMegaproject = NAME_None;
		Live.Systems[HomeIndex].ResponseMegaproject = NAME_None;
		Panel->ShowSystem(Home);
		TestTrue(TEXT("(the ruler has buttons at home)"), Panel->GetActionButtons().Num() > 0);
		Live.Player.Role = EPlayerRole::Wanderer;
		Panel->Refresh();
		TestEqual(TEXT("A wanderer's panel has no button"), Panel->GetActionButtons().Num(), 0);
		Live.Player.Role = EPlayerRole::Ruler;
	}

	// ---- A hazard with the cycles it has left; a row that needs less than 1 shows that it needs 1 ----
	{
		const FGameplayTag Awakening = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.AIAwakening")));
		const FHazardDef* HazardRow = GameTables.FindHazardByTag(Awakening);
		Live.Systems[HomeIndex].ActiveHazard = Awakening;
		Live.Systems[HomeIndex].HazardTicksRemaining = 123;
		Panel->ShowSystem(Home);
		if (TestNotNull(TEXT("(the game has the AI awakening row)"), HazardRow))
		{
			TestTrue(TEXT("The panel names the hazard"), ReadTexts(*Panel).Contains(HazardRow->DisplayName.ToString()));
		}
		TestTrue(TEXT("and the cycles it has left"), ReadTexts(*Panel).Contains(CosmicUI::TimeLeftLabel(123, ECosmicTimeUnit::Cycle).ToString()));

		// Two rows added out of lexical order: the buttons come in lexical order.
		{
			UDataTable* TwoRows = NewObject<UDataTable>(GetTransientPackage());
			TwoRows->RowStruct = FMegaprojectDef::StaticStruct();
			FMegaprojectDef Plain;
			Plain.DisplayName = FText::AsCultureInvariant(TEXT("<plain>"));
			TwoRows->AddRow(FName(TEXT("project_test_b")), Plain);
			TwoRows->AddRow(FName(TEXT("project_test_a")), Plain);
			FCosmicDataTables WithTwo = GameTables;
			WithTwo.Megaprojects = TwoRows;
			Sim->SetTables(WithTwo);
			Panel->ShowSystem(Home);
			TestTrue(TEXT("The project buttons come in lexical row order"), ActionsOf(*Panel) == TArray<FName>({ FName(TEXT("project_test_a")), FName(TEXT("project_test_b")) }));
			Sim->SetTables(GameTables);
		}

		UDataTable* Projects = NewObject<UDataTable>(GetTransientPackage());
		Projects->RowStruct = FMegaprojectDef::StaticStruct();
		FMegaprojectDef Half;
		Half.DisplayName = FText::AsCultureInvariant(TEXT("<half>"));
		Half.RequiredProgress = 0.5f;
		Projects->AddRow(FName(TEXT("project_test_half")), Half);
		FCosmicDataTables WithHalf = GameTables;
		WithHalf.Megaprojects = Projects;
		Sim->SetTables(WithHalf);
		Live.Systems[HomeIndex].ActiveMegaproject = FName(TEXT("project_test_half"));
		Live.Systems[HomeIndex].MegaprojectProgress = 0.f;
		Panel->Refresh();
		TestTrue(TEXT("A row that needs less than 1 shows that it needs 1"), ReadTexts(*Panel).Contains(TEXT("<half> 0/1")));
		Sim->SetTables(GameTables);
		Live.Systems[HomeIndex].ActiveMegaproject = NAME_None;
	}

	// ---- The HUD's panel follows the selection, and a tick redraws it ----
	{
		ACosmicHUD* HUD = Controller->GetHUD<ACosmicHUD>();
		UCosmicSystemPanelWidget* HudPanel = HUD ? HUD->GetSystemPanel() : nullptr;
		if (TestNotNull(TEXT("The HUD has its system panel"), HudPanel))
		{
			Controller->SelectSystem(Home);
			TestEqual(TEXT("Selecting a system shows it on the HUD's panel"), HudPanel->GetShownSystem(), Home);
			Live.Systems[HomeIndex].HazardTicksRemaining = 500;
			HudPanel->Refresh();
			const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
			Sim->SetBroadcastEnabled(true);
			Sim->StepOnce();
			Sim->SetBroadcastEnabled(bWasBroadcasting);
			TestTrue(TEXT("A tick redraws the panel through the delegate"), ReadTexts(*HudPanel).Contains(CosmicUI::TimeLeftLabel(Live.Systems[HomeIndex].HazardTicksRemaining, ECosmicTimeUnit::Cycle).ToString()));
			TestEqual(TEXT("(the hazard counted down)"), Live.Systems[HomeIndex].HazardTicksRemaining, 499);
			Controller->SelectSystem(NAME_None);
			TestTrue(TEXT("Clearing the selection hides it"), HudPanel->GetVisibility() == ESlateVisibility::Collapsed);
		}
	}
	return true;
}

// Phase 3's exit condition, as far as a test can see it: the first life climbs to the third rung by the buttons of the HUD
// alone - the top bar's speed and the system panel's commands - with no key and no prototype command.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIMouseOnlyAscentTest, "Cosmic_Ascension.UI.MouseOnlyAscent", CosmicTests::Flags)
bool FCosmicUIMouseOnlyAscentTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UWorld* World = Sim->GetGameInstance()->GetWorld();
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(World ? World->GetFirstPlayerController() : nullptr);
	ACosmicHUD* HUD = Controller ? Controller->GetHUD<ACosmicHUD>() : nullptr;
	UCosmicSystemPanelWidget* Panel = HUD ? HUD->GetSystemPanel() : nullptr;
	UCosmicTopBarWidget* Bar = HUD ? HUD->GetTopBar() : nullptr;
	ACosmicGalaxyMapActor* Map = nullptr;
	for (TActorIterator<ACosmicGalaxyMapActor> It(World); World && It; ++It)
	{
		Map = *It;
	}
	if (!TestNotNull(TEXT("The HUD has its system panel"), Panel) || !TestNotNull(TEXT("and its top bar"), Bar) || !TestNotNull(TEXT("The map stands"), Map))
	{
		return false;
	}
	ON_SCOPE_EXIT
	{
		Controller->SelectSystem(NAME_None);
		Sim->SetTimeScale(0.f);
		Sim->NewGalaxy(42, 5, 3);
	};
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Map->RefreshNow();
	FGalaxyState& Live = Sim->GetMutableState();
	const int32 CivIndex = Sim->GetStateIndex().FindCiv(Live, Live.Player.RuledCivID);
	const int32 Home = Live.Civs.IsValidIndex(CivIndex) ? Sim->GetStateIndex().FindSystem(Live, Live.Civs[CivIndex].HomeSystemID) : INDEX_NONE;
	if (!TestTrue(TEXT("(the vessel rules a people with a capital)"), Live.Systems.IsValidIndex(Home)))
	{
		return false;
	}
	// A click on the panel's button of Row; false when the panel shows no such button or shows it greyed out.
	auto Click = [Panel](FName Row)
	{
		for (UCosmicActionButton* Button : Panel->GetActionButtons())
		{
			if (Button && Button->GetAction() == Row && Button->GetIsEnabled())
			{
				Button->Press();
				return true;
			}
		}
		return false;
	};

	// The first rung: select the capital (a click on its star) and press the Dyson swarm; the bar's x10 sets the pace.
	Controller->SelectSystem(Live.Systems[Home].SystemID);
	TestTrue(TEXT("The capital's panel offers the Dyson swarm"), Click(FName(TEXT("project_dyson"))));
	TestEqual(TEXT("and the button starts it"), Live.Systems[Home].ActiveMegaproject, FName(TEXT("project_dyson")));
	if (Bar->GetSpeedButtons().Num() == 5)
	{
		Bar->GetSpeedButtons()[4]->Press();
	}
	TestEqual(TEXT("The bar's x10 sets the pace"), Sim->GetTimeScale(), 10.f);
	Sim->SetTimeScale(0.f);
	for (int32 Chunk = 0; Chunk < 60 && Live.Civs[CivIndex].Tier == EAscensionTier::Tier1_Planetary; ++Chunk)
	{
		Sim->RunHeadless(100);
	}
	if (!TestEqual(TEXT("The people climbs to the second rung"), Live.Civs[CivIndex].Tier, EAscensionTier::Tier2_Stellar))
	{
		return false;
	}

	// The second rung widens the realm: a wild neighbour's panel offers the settlement.
	const TArray<int32> Targets = CosmicMegaprojects::SettleTargets(Live, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), CivIndex);
	if (TestTrue(TEXT("(the people has a wild neighbour to settle)"), !Targets.IsEmpty()))
	{
		Controller->SelectSystem(Live.Systems[Targets[0]].SystemID);
		TestTrue(TEXT("A wild neighbour's panel offers the settlement"), Click(CosmicRows::ProjectSettle));
		TestEqual(TEXT("and the button starts it"), Live.Systems[Targets[0]].OwnerCivID, Live.Civs[CivIndex].CivID);
	}

	// The third rung: a supernova threatens the capital with the game's countdown; its panel offers the stabilization.
	const FHazardDef* NovaRow = Sim->GetTables().FindHazardByTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.PreSupernova"))));
	if (!TestNotNull(TEXT("(the game has the row of the supernova)"), NovaRow))
	{
		return false;
	}
	Live.Systems[Home].ActiveHazard = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.PreSupernova")));
	Live.Systems[Home].HazardTicksRemaining = NovaRow->CountdownTicks;
	Live.Systems[Home].ResponseMegaproject = NAME_None;
	Live.Systems[Home].ResponseProgress = 0.f;
	Controller->SelectSystem(Live.Systems[Home].SystemID);
	Panel->Refresh();
	TestTrue(TEXT("The threatened capital's panel offers the stabilization"), Click(FName(TEXT("project_stabilize"))));
	for (int32 Step = 0; Step < NovaRow->CountdownTicks && Live.Civs[CivIndex].Tier == EAscensionTier::Tier2_Stellar; ++Step)
	{
		Sim->RunHeadless(1);
	}
	TestEqual(TEXT("The people climbs to the third rung by the buttons alone"), Live.Civs[CivIndex].Tier, EAscensionTier::Tier3_Causal);
	return true;
}

// The clock's unit (guideline 2.1, 6.3 - v0.8 step 2): the label functions take the unit, a press on the top bar's clock
// switches cycles and years for the clock, the speeds, the panel's countdown and the feed's stamps, the mouse on the
// clock shows both, and the switch moves neither hash nor the viewer's settings file (a test on the developer's machine
// fixes the unit in memory and never writes the developer's choice).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUITimeUnitToggleTest, "Cosmic_Ascension.UI.TimeUnitToggle", CosmicTests::Flags)
bool FCosmicUITimeUnitToggleTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPanelTests;

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
	if (!TestNotNull(TEXT("The first player controller is the game's"), Controller))
	{
		return false;
	}
	const ECosmicTimeUnit ViewerUnit = Controller->GetTimeUnit();
	ON_SCOPE_EXIT
	{
		Sim->SetTimeScale(0.f);
		Controller->SelectSystem(NAME_None);
		Controller->SetTimeUnit(ViewerUnit);
		Sim->NewGalaxy(42, 5, 3);
	};
	Sim->NewGalaxy(42, 0, 0);
	Controller->SetTimeUnit(ECosmicTimeUnit::Cycle);
	const FString& Placeholder = FStringTableEntry::GetPlaceholderSourceString();
	// The year word of the String Table (U+B144) and a number past a thousand (the culture groups its digits).
	const FString YearWord(TEXT("\xB144"));
	constexpr int64 Tick = 1204;
	const FString Number = FText::AsNumber(Tick).ToString();

	// ---- The label functions take the unit ----
	const FString YearStamp = CosmicUI::StampLabel(Tick, ECosmicTimeUnit::Year).ToString();
	UCosmicLogItem* Item = NewObject<UCosmicLogItem>(GetTransientPackage());
	Item->Tick = Tick;
	Item->Message = FText::AsCultureInvariant(TEXT("line"));
	{
		const FString CycleClock = CosmicUI::ClockLabel(Tick, ECosmicTimeUnit::Cycle).ToString();
		const FString YearClock = CosmicUI::ClockLabel(Tick, ECosmicTimeUnit::Year).ToString();
		TestEqual(TEXT("The cycle clock keeps the Phase 3 form"), CycleClock, FString::Printf(TEXT("%s %s"), *LabelText(TEXT("Panel.Cycle")), *Number));
		TestTrue(TEXT("The year clock holds the number"), YearClock.Contains(Number));
		TestTrue(TEXT("The year clock holds the year word"), YearClock.Contains(YearWord));
		TestFalse(TEXT("The year clock does not say cycle"), YearClock.Contains(LabelText(TEXT("Panel.Cycle"))));
		const FString Both = CosmicUI::ClockBothLabel(Tick).ToString();
		TestTrue(TEXT("Both units hold the cycle form"), Both.Contains(CycleClock));
		TestTrue(TEXT("Both units hold the year form"), Both.Contains(YearClock));

		const FString CycleLeft = CosmicUI::TimeLeftLabel(120, ECosmicTimeUnit::Cycle).ToString();
		const FString YearLeft = CosmicUI::TimeLeftLabel(120, ECosmicTimeUnit::Year).ToString();
		TestEqual(TEXT("The cycle countdown keeps the Phase 3 form"), CycleLeft, FString::Printf(TEXT("%s 120"), *LabelText(TEXT("Panel.CyclesLeft"))));
		TestTrue(TEXT("The year countdown holds the number"), YearLeft.Contains(TEXT("120")));
		TestTrue(TEXT("The year countdown holds the year word"), YearLeft.Contains(YearWord));
		TestFalse(TEXT("The year countdown does not say cycles left"), YearLeft.Contains(LabelText(TEXT("Panel.CyclesLeft"))));

		TestEqual(TEXT("The cycle stamp is the bare number"), CosmicUI::StampLabel(Tick, ECosmicTimeUnit::Cycle).ToString(), Number);
		TestTrue(TEXT("The year stamp holds the number"), YearStamp.Contains(Number));
		TestTrue(TEXT("The year stamp holds the year word"), YearStamp.Contains(YearWord));

		TestEqual(TEXT("A speed in cycles is x2"), CosmicUI::SpeedLabel(2.f, ECosmicTimeUnit::Cycle).ToString(), FString(TEXT("x2")));
		const FString YearSpeed = CosmicUI::SpeedLabel(2.f, ECosmicTimeUnit::Year).ToString();
		TestTrue(TEXT("A speed in years holds the number"), YearSpeed.Contains(TEXT("2")));
		TestTrue(TEXT("A speed in years holds the year word"), YearSpeed.Contains(YearWord));
		TestEqual(TEXT("The pause is the pause in cycles"), CosmicUI::SpeedLabel(0.f, ECosmicTimeUnit::Cycle).ToString(), LabelText(TEXT("Speed.Pause")));
		TestEqual(TEXT("and in years"), CosmicUI::SpeedLabel(0.f, ECosmicTimeUnit::Year).ToString(), LabelText(TEXT("Speed.Pause")));

		TestEqual(TEXT("A line in cycles is [number] message"), CosmicLogFeed::LineText(*Item, ECosmicTimeUnit::Cycle).ToString(), FString::Printf(TEXT("[%s] line"), *Number));
		TestEqual(TEXT("A line in years carries the year stamp"), CosmicLogFeed::LineText(*Item, ECosmicTimeUnit::Year).ToString(), FString::Printf(TEXT("[%s] line"), *YearStamp));

		const FString Shown[] = { CycleClock, YearClock, Both, CycleLeft, YearLeft, YearStamp, YearSpeed, LabelText(TEXT("Time.Tip")), LabelText(TEXT("Time.TipPress")) };
		for (const FString& Text : Shown)
		{
			TestFalse(*FString::Printf(TEXT("No missing String Table entry in '%s'"), *Text), Text.Contains(Placeholder));
		}
	}

	// ---- A change is announced once ----
	{
		int32 Announced = 0;
		const FDelegateHandle Probe = Controller->OnTimeUnitChanged.AddLambda([&Announced]() { ++Announced; });
		Controller->SetTimeUnit(ECosmicTimeUnit::Year);
		Controller->SetTimeUnit(ECosmicTimeUnit::Year);
		Controller->OnTimeUnitChanged.Remove(Probe);
		TestEqual(TEXT("Setting the same unit twice announces one change"), Announced, 1);
		Controller->SetTimeUnit(ECosmicTimeUnit::Cycle);
	}

	// ---- The scene: a clock past a thousand, a hazard counting down at home, home selected on the HUD ----
	FGalaxyState& Live = Sim->GetMutableState();
	const int32 RuledIndex = Sim->GetStateIndex().FindCiv(Live, Live.Player.RuledCivID);
	if (!TestTrue(TEXT("(the player rules a people)"), Live.Civs.IsValidIndex(RuledIndex)))
	{
		return false;
	}
	const FName Home = Live.Civs[RuledIndex].HomeSystemID;
	const int32 HomeIndex = Sim->GetStateIndex().FindSystem(Live, Home);
	if (!TestTrue(TEXT("(the ruled people has its home system)"), Live.Systems.IsValidIndex(HomeIndex)))
	{
		return false;
	}
	Live.Tick = Tick;
	Live.Systems[HomeIndex].ActiveHazard = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.AIAwakening")));
	Live.Systems[HomeIndex].HazardTicksRemaining = 120;
	Controller->SelectSystem(Home);
	ACosmicHUD* HUD = Controller->GetHUD<ACosmicHUD>();
	UCosmicTopBarWidget* HudBar = HUD ? HUD->GetTopBar() : nullptr;
	UCosmicSystemPanelWidget* HudPanel = HUD ? HUD->GetSystemPanel() : nullptr;
	UCosmicLogFeedWidget* HudFeed = HUD ? HUD->GetLogFeed() : nullptr;
	if (!TestNotNull(TEXT("The HUD has its top bar"), HudBar) || !TestNotNull(TEXT("The HUD has its system panel"), HudPanel)
		|| !TestNotNull(TEXT("The HUD has its log feed"), HudFeed) || !TestNotNull(TEXT("(the HUD's top bar has its clock)"), HudBar->GetClockText()))
	{
		return false;
	}
	HUD->RefreshPanel();
	// The HUD feed's lines on screen (the new galaxy's first lines), drawn now.
	UListView* FeedList = FindList(*HudFeed);
	if (!TestNotNull(TEXT("(the HUD feed has its list)"), FeedList))
	{
		return false;
	}
	DrawListNow(*FeedList);
	auto FeedTexts = [FeedList]()
	{
		TArray<FString> Texts;
		for (const UUserWidget* Entry : FeedList->GetDisplayedEntryWidgets())
		{
			const UCosmicLogEntryWidget* FeedLine = Cast<UCosmicLogEntryWidget>(Entry);
			Texts.Add(FeedLine && FeedLine->GetText() ? FeedLine->GetText()->GetText().ToString() : FString());
		}
		return Texts;
	};
	const TArray<FString> CycleLines = FeedTexts();
	if (!TestTrue(TEXT("(the HUD feed draws the galaxy's lines)"), CycleLines.Num() > 0))
	{
		return false;
	}
	for (const FString& Text : CycleLines)
	{
		TestTrue(*FString::Printf(TEXT("A feed line starts with a stamp: %s"), *Text), Text.StartsWith(TEXT("[")));
		TestFalse(*FString::Printf(TEXT("and the stamp is in cycles: %s"), *Text), Text.Contains(YearWord + TEXT("]")));
	}

	// ---- The top bar's clock ----
	UCosmicTopBarWidget* Bar = CreateWidget<UCosmicTopBarWidget>(Controller);
	if (!TestNotNull(TEXT("The top bar can be created"), Bar))
	{
		return false;
	}
	const TSharedRef<SWidget> SlateBar = Bar->TakeWidget();
	Bar->Refresh();
	if (!TestNotNull(TEXT("The bar has its clock"), Bar->GetClockText()) || !TestNotNull(TEXT("The bar has its clock's button"), Bar->GetClockButton())
		|| !TestEqual(TEXT("(five speed buttons)"), Bar->GetSpeedButtons().Num(), 5))
	{
		return false;
	}
	auto ClockShown = [Bar]() { return Bar->GetClockText()->GetText().ToString(); };
	auto SpeedShown = [Bar]() { return Bar->GetSpeedButtons()[2]->GetLabel().ToString(); };
	TestEqual(TEXT("The clock starts in cycles"), ClockShown(), CosmicUI::ClockLabel(Tick, ECosmicTimeUnit::Cycle).ToString());
	TestEqual(TEXT("The speeds start in cycles"), SpeedShown(), CosmicUI::SpeedLabel(2.f, ECosmicTimeUnit::Cycle).ToString());
	const FString ClockTip = Bar->GetClockButton()->GetToolTipText().ToString();
	TestTrue(TEXT("The clock's tooltip says what a year is"), ClockTip.Contains(LabelText(TEXT("Time.Tip"))));
	TestTrue(TEXT("and that a press switches the unit"), ClockTip.Contains(LabelText(TEXT("Time.TipPress"))));

	// A line drawn by a widget of the test's own.
	UCosmicLogEntryWidget* Line = CreateWidget<UCosmicLogEntryWidget>(Controller);
	if (!TestNotNull(TEXT("A feed line can be created"), Line))
	{
		return false;
	}
	const TSharedRef<SWidget> SlateLine = Line->TakeWidget();
	Line->ShowItem(Item);
	if (!TestNotNull(TEXT("(the line has its text)"), Line->GetText()))
	{
		return false;
	}
	TestEqual(TEXT("A line starts with the cycle stamp"), Line->GetText()->GetText().ToString(), CosmicLogFeed::LineText(*Item, ECosmicTimeUnit::Cycle).ToString());

	const uint32 StateHash = Sim->ComputeStateHash();
	const uint32 WorldHash = Sim->ComputeWorldHash();
	FString KeptBefore;
	const bool bKeptBefore = GConfig && GConfig->GetString(ACosmicPlayerController::TimeUnitSection, ACosmicPlayerController::TimeUnitKey, KeptBefore, GGameUserSettingsIni);
	auto NothingWritten = [&](const TCHAR* When)
	{
		FString KeptNow;
		const bool bKeptNow = GConfig && GConfig->GetString(ACosmicPlayerController::TimeUnitSection, ACosmicPlayerController::TimeUnitKey, KeptNow, GGameUserSettingsIni);
		TestEqual(*FString::Printf(TEXT("%s, the viewer's settings hold the same entry or none"), When), bKeptNow, bKeptBefore);
		TestEqual(*FString::Printf(TEXT("%s, the viewer's settings keep their value"), When), KeptNow, KeptBefore);
		TestEqual(*FString::Printf(TEXT("%s, the state hash is the same"), When), Sim->ComputeStateHash(), StateHash);
		TestEqual(*FString::Printf(TEXT("%s, the world hash is the same"), When), Sim->ComputeWorldHash(), WorldHash);
	};

	Bar->PressClock();
	TestTrue(TEXT("A press on the clock turns it to years"), Controller->GetTimeUnit() == ECosmicTimeUnit::Year);
	TestEqual(TEXT("The clock shows years"), ClockShown(), CosmicUI::ClockLabel(Tick, ECosmicTimeUnit::Year).ToString());
	TestEqual(TEXT("The speeds follow the clock"), SpeedShown(), CosmicUI::SpeedLabel(2.f, ECosmicTimeUnit::Year).ToString());
	TestEqual(TEXT("The pause stays the pause"), Bar->GetSpeedButtons()[0]->GetLabel().ToString(), LabelText(TEXT("Speed.Pause")));
	TestEqual(TEXT("The HUD's clock follows"), HudBar->GetClockText()->GetText().ToString(), CosmicUI::ClockLabel(Tick, ECosmicTimeUnit::Year).ToString());
	TestTrue(TEXT("The HUD panel's countdown follows"), ReadTexts(*HudPanel).Contains(CosmicUI::TimeLeftLabel(120, ECosmicTimeUnit::Year).ToString()));
	const TArray<FString> YearLines = FeedTexts();
	TestEqual(TEXT("The HUD feed keeps its lines"), YearLines.Num(), CycleLines.Num());
	for (const FString& Text : YearLines)
	{
		TestTrue(*FString::Printf(TEXT("A feed line on screen takes the year stamp: %s"), *Text), Text.Contains(YearWord + TEXT("]")));
	}
	Line->Redraw();
	TestEqual(TEXT("A redrawn line takes the year stamp"), Line->GetText()->GetText().ToString(), CosmicLogFeed::LineText(*Item, ECosmicTimeUnit::Year).ToString());
	NothingWritten(TEXT("After the first press"));

	// The mouse on the clock shows both units; off it, the chosen one.
	Bar->SetClockHovered(true);
	TestEqual(TEXT("The mouse on the clock shows both units"), ClockShown(), CosmicUI::ClockBothLabel(Tick).ToString());
	Bar->SetClockHovered(false);
	TestEqual(TEXT("Off the clock it shows the chosen unit"), ClockShown(), CosmicUI::ClockLabel(Tick, ECosmicTimeUnit::Year).ToString());

	Bar->PressClock();
	TestTrue(TEXT("A second press turns it back to cycles"), Controller->GetTimeUnit() == ECosmicTimeUnit::Cycle);
	TestEqual(TEXT("The clock shows cycles again"), ClockShown(), CosmicUI::ClockLabel(Tick, ECosmicTimeUnit::Cycle).ToString());
	TestEqual(TEXT("and the speeds"), SpeedShown(), CosmicUI::SpeedLabel(2.f, ECosmicTimeUnit::Cycle).ToString());
	TestTrue(TEXT("and the HUD panel's countdown"), ReadTexts(*HudPanel).Contains(CosmicUI::TimeLeftLabel(120, ECosmicTimeUnit::Cycle).ToString()));
	const TArray<FString> CycleAgain = FeedTexts();
	for (const FString& Text : CycleAgain)
	{
		TestFalse(*FString::Printf(TEXT("A feed line on screen takes the cycle stamp again: %s"), *Text), Text.Contains(YearWord + TEXT("]")));
	}
	Line->Redraw();
	TestEqual(TEXT("A redrawn line takes the cycle stamp again"), Line->GetText()->GetText().ToString(), CosmicLogFeed::LineText(*Item, ECosmicTimeUnit::Cycle).ToString());
	NothingWritten(TEXT("After the second press"));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

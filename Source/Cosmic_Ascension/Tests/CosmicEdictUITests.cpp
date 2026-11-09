// Cosmic Ascension - automation tests of the edicts on the screen (B2): the edict window, the top bar's edict button and the
// edict hint, the system panel's edict line, [Share by hand] and the focus buttons judged by preset.
// Design: docs/60_DECISIONS/headers/B2_edicts_and_research.md section 2.8, 4 and 6.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicResearch.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicEdictWidget.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicGameMode.h"

#include <limits>

namespace CosmicEdictUITests
{
	const FName Physics(TEXT("Physics"));
	const FName Society(TEXT("Society"));
	const FName Engineering(TEXT("Engineering"));
	const FName Mobilization(TEXT("edict_mobilization"));
	const FName MachineRights(TEXT("edict_machine_rights"));
	const FName OpenThought(TEXT("edict_open_thought"));
	const FName FreeTrade(TEXT("edict_free_trade"));
	const FName Vigil(TEXT("edict_vigil"));
	// Rows this test adds to the game's five: one of the second rung, one of the first whose condition no people meets.
	const FName High(TEXT("edict_test_high"));
	const FName Bound(TEXT("edict_test_bound"));
	// Machine rights' cost, pinned by the test (the game's row values are provisional until the balance commit).
	constexpr float RightsCost = 40.f;

	FString Words(const FText& Text)
	{
		return Text.ToString();
	}

	FString Words(const TCHAR* Key)
	{
		return CosmicUI::Label(Key).ToString();
	}

	bool HasPlaceholder(const FText& Text)
	{
		return Text.ToString().Contains(FStringTableEntry::GetPlaceholderSourceString()) || Text.ToString().Contains(TEXT("{"));
	}

	bool Shown(const UWidget* Widget)
	{
		return Widget && Widget->GetVisibility() != ESlateVisibility::Collapsed;
	}

	void StepBroadcast(UGalaxySimulationSubsystem& Sim)
	{
		const bool bWasBroadcasting = Sim.IsBroadcastEnabled();
		Sim.SetBroadcastEnabled(true);
		Sim.StepOnce();
		Sim.SetBroadcastEnabled(bWasBroadcasting);
	}

	FCosmicEffect Effect(EEffectType Type, const TCHAR* Key, float Value)
	{
		FCosmicEffect Out;
		Out.Type = Type;
		Out.Key = Key ? FName(Key) : FName();
		Out.Value = Value;
		return Out;
	}

	// The game's edicts with the values this test reads pinned (mobilization's effects and upkeep, machine rights' cost - the
	// game's are provisional until the balance commit), one row of the second rung and one whose condition no people meets.
	UDataTable* EdictsOfTest(const UDataTable& Game)
	{
		UDataTable* Table = CosmicTests::NewTable(FEdictDef::StaticStruct());
		for (const FName Row : Game.GetRowNames())
		{
			if (const FEdictDef* Def = Game.FindRow<FEdictDef>(Row, TEXT("test"), false))
			{
				FEdictDef Copy = *Def;
				if (Row == Mobilization)
				{
					Copy.EffectsPerTick = { Effect(EEffectType::MultiplyIncome, TEXT("Energy"), 1.2f), Effect(EEffectType::MultiplyIncome, TEXT("Matter"), 1.3f),
						Effect(EEffectType::ChangeStability, nullptr, -0.0015f) };
					Copy.UpkeepPerTick = FResourcePool();
				}
				if (Row == MachineRights)
				{
					Copy.EnactCost = FResourcePool();
					Copy.EnactCost.Influence = RightsCost;
				}
				Table->AddRow(Row, Copy);
			}
		}
		FEdictDef Higher;
		Higher.DisplayName = FText::AsCultureInvariant(TEXT("high"));
		Higher.Summary = FText::AsCultureInvariant(TEXT("a test edict."));
		Higher.RequiredTier = EAscensionTier::Tier2_Stellar;
		Table->AddRow(High, Higher);
		FEdictDef Unmet;
		Unmet.DisplayName = FText::AsCultureInvariant(TEXT("bound"));
		Unmet.Summary = FText::AsCultureInvariant(TEXT("a test edict."));
		FCosmicCondition Book;
		Book.Type = EConditionType::CivHasTrait;
		Book.Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Trait.Faith.Book")));
		Unmet.EnactConditions = { Book };
		Table->AddRow(Bound, Unmet);
		return Table;
	}

	// The game's hazards, the machine's awakening answered by machine rights too (the balance commit's data, set here).
	UDataTable* HazardsAnsweredByRights(const UDataTable& Game)
	{
		UDataTable* Table = CosmicTests::NewTable(FHazardDef::StaticStruct());
		for (const FName Row : Game.GetRowNames())
		{
			if (const FHazardDef* Def = Game.FindRow<FHazardDef>(Row, TEXT("test"), false))
			{
				FHazardDef Copy = *Def;
				if (Copy.HazardTag == FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.AIAwakening"))))
				{
					Copy.RulerResolutions.Add(MachineRights);
				}
				Table->AddRow(Row, Copy);
			}
		}
		return Table;
	}
}

// The pure parts: the edict hint's steps, the preset judgment and the word of a focus, the next slot's mark.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIEdictRulesTest, "Cosmic_Ascension.UI.EdictRules", CosmicTests::Flags)
bool FCosmicUIEdictRulesTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEdictUITests;

	// The hint: starts below 0.5 while the inherited edict is held, ends when it is held no more, once a session.
	{
		FCosmicHintProgress Progress;
		CosmicUI::StepEdictHint(Progress, true, true, 0.5f);
		TestFalse(TEXT("At 0.5 the hint does not start"), CosmicUI::ShowsEdictHint(Progress));
		CosmicUI::StepEdictHint(Progress, true, false, 0.2f);
		TestFalse(TEXT("nor without the inherited edict"), CosmicUI::ShowsEdictHint(Progress));
		CosmicUI::StepEdictHint(Progress, false, true, 0.2f);
		TestFalse(TEXT("nor for a vessel that rules no people"), CosmicUI::ShowsEdictHint(Progress));
		CosmicUI::StepEdictHint(Progress, true, true, 0.49f);
		TestTrue(TEXT("Below 0.5 holding it, it starts"), CosmicUI::ShowsEdictHint(Progress));
		CosmicUI::StepEdictHint(Progress, true, true, 0.9f);
		TestTrue(TEXT("and stays when Stability rises again"), CosmicUI::ShowsEdictHint(Progress));
		CosmicUI::StepEdictHint(Progress, false, false, 1.f);
		TestTrue(TEXT("and while the vessel rules no people"), CosmicUI::ShowsEdictHint(Progress));
		CosmicUI::StepEdictHint(Progress, true, false, 0.2f);
		TestFalse(TEXT("The people holds it no more: it ends"), CosmicUI::ShowsEdictHint(Progress));
		CosmicUI::StepEdictHint(Progress, true, true, 0.2f);
		TestFalse(TEXT("and never comes back in the session"), CosmicUI::ShowsEdictHint(Progress));
	}

	// The preset judgment: the map a press would make, compared key by key.
	{
		TMap<FName, float> Focus = { { Physics, 0.25f }, { Society, 0.25f }, { Engineering, 0.25f }, { CosmicResearch::TranscendenceField(), 0.25f } };
		TestTrue(TEXT("The generation's map is the even preset"), CosmicUI::IsPresetNow(Focus, NAME_None, 0.55f));
		TestFalse(TEXT("and no field's"), CosmicUI::IsPresetNow(Focus, Society, 0.55f));
		TestEqual(TEXT("Its word is even"), Words(CosmicUI::FocusWordOf(Focus, 0.55f)), Words(TEXT("Focus.Even")));
		CosmicResearch::Focus(Focus, Society, 0.55f);
		TestTrue(TEXT("Society's preset is society's"), CosmicUI::IsPresetNow(Focus, Society, 0.55f));
		TestFalse(TEXT("not at another share"), CosmicUI::IsPresetNow(Focus, Society, 0.7f));
		TestEqual(TEXT("Its word is society"), Words(CosmicUI::FocusWordOf(Focus, 0.55f)), Words(TEXT("Research.Society")));
		// Shares by hand that lean to society: no preset, so every button stays pressable and the word is mixed.
		const TMap<FName, int32> Weights = { { Physics, 1 }, { Society, 5 }, { Engineering, 1 } };
		TestTrue(TEXT("(the weights 1-5-1 are taken)"), CosmicResearch::SharesOf(Focus, Weights));
		TestFalse(TEXT("Weights 1-5-1 are not society's preset"), CosmicUI::IsPresetNow(Focus, Society, 0.55f));
		TestEqual(TEXT("and their word is mixed"), Words(CosmicUI::FocusWordOf(Focus, 0.55f)), Words(TEXT("Focus.Mixed")));
		// A lean to society of another share (an answer's value, a knob changed since): society's shape, not its preset.
		CosmicResearch::Focus(Focus, Society, 0.7f);
		TestEqual(TEXT("(a lean of 0.7 has society's shape)"), CosmicResearch::ShapeOf(Focus), ECosmicFocusShape::Focused);
		TestFalse(TEXT("but is not society's preset at 0.55"), CosmicUI::IsPresetNow(Focus, Society, 0.55f));
		TestEqual(TEXT("and its word is mixed"), Words(CosmicUI::FocusWordOf(Focus, 0.55f)), Words(TEXT("Focus.Mixed")));
		CosmicResearch::Focus(Focus, CosmicResearch::TranscendenceField(), 0.55f);
		TestEqual(TEXT("A lean to transcendence is named"), Words(CosmicUI::FocusWordOf(Focus, 0.55f)), Words(TEXT("Research.Transcendence")));
	}

	// The next slot: the smallest mark not reached, as SlotsOf reads the marks.
	{
		FCosmicRulesDef Rules;
		Rules.EdictSlotSocietyMarks = { 1.f, 0.5f };
		FCivilizationState Civ;
		TestTrue(TEXT("At Society 0 the next slot opens at 0.5"), UCosmicEdictWidget::NextSlotMarkOf(Civ, Rules) == TOptional<float>(0.5f));
		Civ.Research.Add(Society, 0.5f);
		TestTrue(TEXT("At 0.5 the next is 1.0 (0.5 is reached)"), UCosmicEdictWidget::NextSlotMarkOf(Civ, Rules) == TOptional<float>(1.f));
		Civ.Research.Add(Society, 1.f);
		TestFalse(TEXT("At 1.0 every slot is open"), UCosmicEdictWidget::NextSlotMarkOf(Civ, Rules).IsSet());
		Rules.EdictSlotSocietyMarks = { std::numeric_limits<float>::quiet_NaN() };
		TestFalse(TEXT("A mark that is not a number is no slot"), UCosmicEdictWidget::NextSlotMarkOf(Civ, Rules).IsSet());
	}

	// The pressure's drain in an edict's tooltip (B4 header section 4): one line for a factor that changes it, read as the
	// economy stage reads it (CosmicEdicts::PressureDrainMultiplierOf - 0, below 0 and not a number count as 1).
	{
		UDataTable* Edicts = CosmicTests::NewTable(FEdictDef::StaticStruct());
		auto Add = [Edicts](const TCHAR* Name, float Factor)
		{
			FEdictDef Row;
			Row.DisplayName = FText::AsCultureInvariant(Name);
			Row.PressureDrainMultiplier = Factor;
			Edicts->AddRow(FName(Name), Row);
		};
		Add(TEXT("edict_test_drain_less"), 0.45f);
		Add(TEXT("edict_test_drain_more"), 1.5f);
		Add(TEXT("edict_test_drain_one"), 1.f);
		Add(TEXT("edict_test_drain_zero"), 0.f);
		Add(TEXT("edict_test_drain_below"), -0.5f);
		Add(TEXT("edict_test_drain_nan"), std::numeric_limits<float>::quiet_NaN());
		Add(TEXT("edict_test_drain_inf"), std::numeric_limits<float>::infinity());
		FCosmicDataTables Tables;
		Tables.Edicts = Edicts;
		auto TipOfRow = [&Tables](const TCHAR* Name)
		{
			return Words(UCosmicEdictWidget::TipOf(Tables, FName(Name), ECosmicTimeUnit::Cycle));
		};
		const FString Drain = Words(TEXT("Edict.Effect.PressureDrain"));
		int32 Brace = INDEX_NONE;
		const FString Lead = Drain.FindChar(TEXT('{'), Brace) ? Drain.Left(Brace) : Drain;
		TestFalse(TEXT("(the drain's words lead with words)"), Lead.IsEmpty());
		TestTrue(TEXT("An edict that eases the drain says its factor"),
			TipOfRow(TEXT("edict_test_drain_less")).Contains(Words(FText::Format(CosmicUI::Label(TEXT("Edict.Effect.PressureDrain")), UCosmicEdictWidget::AmountOf(0.45f)))));
		TestTrue(TEXT("an edict that deepens it says so too (the stage multiplies it)"),
			TipOfRow(TEXT("edict_test_drain_more")).Contains(Words(FText::Format(CosmicUI::Label(TEXT("Edict.Effect.PressureDrain")), UCosmicEdictWidget::AmountOf(1.5f)))));
		TestFalse(TEXT("An edict of factor 1 says nothing of the drain"), TipOfRow(TEXT("edict_test_drain_one")).Contains(Lead));
		TestFalse(TEXT("nor one of factor 0, which the stage reads as 1"), TipOfRow(TEXT("edict_test_drain_zero")).Contains(Lead));
		TestFalse(TEXT("nor one below 0"), TipOfRow(TEXT("edict_test_drain_below")).Contains(Lead));
		TestFalse(TEXT("nor one that is not a number"), TipOfRow(TEXT("edict_test_drain_nan")).Contains(Lead));
		TestFalse(TEXT("nor an infinite one"), TipOfRow(TEXT("edict_test_drain_inf")).Contains(Lead));
	}
	return true;
}

// The window on the HUD: the top bar's button, the rows and their gate, the inherited mark, the hint, the tabs, the shares,
// the panel's edict line, the end of the run, a wanderer, a new galaxy.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIEdictWindowTest, "Cosmic_Ascension.UI.EdictWindow", CosmicTests::Flags)
bool FCosmicUIEdictWindowTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEdictUITests;
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
	UCosmicSystemPanelWidget* Panel = HUD ? HUD->GetSystemPanel() : nullptr;
	UCosmicTopBarWidget* Bar = HUD ? HUD->GetTopBar() : nullptr;
	UCosmicEdictWidget* Window = HUD ? HUD->GetEdictWindow() : nullptr;
	if (!TestNotNull(TEXT("(the HUD's system panel)"), Panel) || !TestNotNull(TEXT("(its top bar)"), Bar) || !TestNotNull(TEXT("(its edict window)"), Window))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicHintProgress HintsBefore = Controller->GetHintProgress();
	ON_SCOPE_EXIT
	{
		Window->Close();
		Controller->SetHintProgress(HintsBefore);
		Controller->ClearRunEnd();
		Controller->SelectSystem(NAME_None);
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	if (!TestTrue(TEXT("(the game's edicts and hazards are loaded)"), Original.Edicts && Original.Hazards && Original.FindRules()))
	{
		return false;
	}
	// The balance commit's data, set here: every people inherits mobilization; machine rights answers the awakening.
	FCosmicRulesDef Rules = *Original.FindRules();
	Rules.StartingEdict = Mobilization;
	Rules.SocietyRecoveryBonus = 0.f;
	FCosmicDataTables Tables = CosmicTests::WithRules(Original, Rules);
	Tables.Edicts = EdictsOfTest(*Original.Edicts);
	Tables.Hazards = HazardsAnsweredByRights(*Original.Hazards);
	Sim->SetTables(Tables);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->SetTimeScale(0.f);
	Controller->SetHintProgress(FCosmicHintProgress());
	FGalaxyState& Live = Sim->GetMutableState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const int32 Ruled = Index.FindCiv(Live, Live.Player.RuledCivID);
	const int32 Home = Live.Civs.IsValidIndex(Ruled) ? Index.FindSystem(Live, Live.Civs[Ruled].HomeSystemID) : INDEX_NONE;
	const int32 OtherCiv = Ruled == 0 ? 1 : 0;
	const int32 Other = Live.Civs.IsValidIndex(OtherCiv) ? Index.FindSystem(Live, Live.Civs[OtherCiv].HomeSystemID) : INDEX_NONE;
	if (!TestTrue(TEXT("(the ruled people has a home, and another people too)"), Live.Systems.IsValidIndex(Home) && Live.Systems.IsValidIndex(Other)))
	{
		return false;
	}
	FCivilizationState* Civ = &Live.Civs[Ruled];
	TestTrue(TEXT("(the people inherits mobilization)"), Civ->ActiveEdicts == TArray<FName>({ Mobilization }));
	CosmicTests::KnowAll(Live);
	const FName HomeID = Live.Systems[Home].SystemID;
	Controller->SelectSystem(HomeID);
	Civ->Stability = 0.6f;
	HUD->RefreshPanel();
	const ECosmicTimeUnit Unit = ACosmicPlayerController::TimeUnitFor(*Window);
	auto Count = [](int32 Held, int32 Slots) { return Words(FText::Format(CosmicUI::Label(TEXT("Edict.Count")), FText::AsNumber(Held), FText::AsNumber(Slots))); };

	// The top bar's button: a ruler's, 'edicts held/slots'; the mark keeps its place while hidden.
	UCosmicActionButton* EdictButton = Bar->GetEdictButton();
	if (!TestNotNull(TEXT("(the edict button)"), EdictButton))
	{
		return false;
	}
	TestTrue(TEXT("A ruler's bar shows the edict button"), Shown(EdictButton) && Shown(EdictButton->GetParent()));
	TestEqual(TEXT("saying one held of one slot"), Words(EdictButton->GetLabel()), Count(1, 1));
	{
		const FString Tip = Words(EdictButton->GetToolTipText());
		TestTrue(TEXT("Its tooltip says the next slot opens at Society 50%"), Tip.Contains(Words(FText::Format(CosmicUI::Label(TEXT("Edict.Bar.NextSlot")), CosmicUI::Percent(0.5f)))));
		TestTrue(TEXT("and the upkeep held (mobilization has none)"), Tip.Contains(Words(FText::Format(CosmicUI::Label(TEXT("Edict.Bar.Upkeep")), CosmicUI::Label(TEXT("Edict.NoUpkeep"))))));
		TestFalse(TEXT("with every slot filled"), HasPlaceholder(EdictButton->GetToolTipText()));
	}
	TestEqual(TEXT("No mark above 0.5: hidden, keeping its width"), Bar->GetEdictMark()->GetVisibility(), ESlateVisibility::Hidden);
	TestFalse(TEXT("nor a hint line on the capital's panel"), Shown(Panel->GetEdictHintText()));

	// The hint: Stability below 0.5 with the inherited edict held - the bar's mark, the tooltip's first line, the capital's line.
	Civ->Stability = 0.45f;
	HUD->RefreshPanel();
	const FString HintWords = Words(UCosmicEdictWidget::HintOf(Tables, Rules));
	TestEqual(TEXT("Below 0.5 the bar's mark stands"), Bar->GetEdictMark()->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestTrue(TEXT("and the tooltip says the hint first"), Words(EdictButton->GetToolTipText()).StartsWith(HintWords) && !HintWords.IsEmpty());
	TestTrue(TEXT("The capital's panel shows the hint"), Shown(Panel->GetEdictHintText()) && Words(Panel->GetEdictHintText()->GetText()) == HintWords);
	TestFalse(TEXT("which names the inherited edict, slots filled"), HasPlaceholder(Panel->GetEdictHintText()->GetText()));
	{
		const UVerticalBox* Column = Panel->GetColumn();
		const int32 AbdicateAt = Column ? Column->GetChildIndex(Panel->GetAbdicateButton()) : INDEX_NONE;
		const int32 HintAt = Column ? Column->GetChildIndex(Panel->GetEdictHintText()) : INDEX_NONE;
		const int32 ShipAt = Column ? Column->GetChildIndex(Panel->GetShipHintText()) : INDEX_NONE;
		TestTrue(TEXT("under [Abdicate], above the first ship's line"), AbdicateAt != INDEX_NONE && HintAt == AbdicateAt + 1 && ShipAt == HintAt + 1);
	}
	Civ->Stability = 0.6f;
	HUD->RefreshPanel();
	TestTrue(TEXT("The hint stays when Stability rises again"), Bar->GetEdictMark()->GetVisibility() == ESlateVisibility::HitTestInvisible && Shown(Panel->GetEdictHintText()));
	Controller->SelectSystem(Live.Systems[Other].SystemID);
	HUD->RefreshPanel();
	TestFalse(TEXT("Another system's panel has no hint line"), Shown(Panel->GetEdictHintText()));
	Controller->SelectSystem(HomeID);

	// The bar's button opens the window: both hints end. A second press closes it.
	TestFalse(TEXT("(the window starts closed)"), Window->IsOpen());
	EdictButton->Press();
	TestTrue(TEXT("A press of the bar's button opens the window"), Window->IsOpen() && Shown(Window));
	TestEqual(TEXT("on its edicts' page"), Window->GetTab(), ECosmicEdictTab::Edicts);
	TestEqual(TEXT("The open ends the bar's mark"), Bar->GetEdictMark()->GetVisibility(), ESlateVisibility::Hidden);
	TestFalse(TEXT("and the capital's line"), Shown(Panel->GetEdictHintText()));
	TestFalse(TEXT("and the tooltip's first line"), Words(EdictButton->GetToolTipText()).StartsWith(HintWords));
	EdictButton->Press();
	TestFalse(TEXT("A second press closes it"), Window->IsOpen() || Shown(Window));
	// A window opened before Stability falls: the hint never comes in that session.
	Controller->SetHintProgress(FCosmicHintProgress());
	EdictButton->Press();
	EdictButton->Press();
	Civ->Stability = 0.45f;
	HUD->RefreshPanel();
	TestFalse(TEXT("Opened before the fall, the hint never comes"), Bar->GetEdictMark()->GetVisibility() == ESlateVisibility::HitTestInvisible || Shown(Panel->GetEdictHintText()));
	Civ->Stability = 0.6f;
	EdictButton->Press();

	// The tabs and [Close].
	Window->GetSharesTabButton()->Press();
	TestTrue(TEXT("The shares' tab turns the page and greys itself"), Window->GetTab() == ECosmicEdictTab::Shares && !Window->GetSharesTabButton()->GetIsEnabled()
		&& Window->GetEdictsTabButton()->GetIsEnabled());
	Window->GetEdictsTabButton()->Press();
	TestTrue(TEXT("The edicts' tab turns it back"), Window->GetTab() == ECosmicEdictTab::Edicts && !Window->GetEdictsTabButton()->GetIsEnabled());
	TestEqual(TEXT("and counts the held edicts and the slots"), Words(Window->GetEdictsTabButton()->GetLabel()), Count(1, 1));
	// The third tab (B3): the peoples' page, shown alone; the other two tabs then pressable.
	TestTrue(TEXT("The peoples' tab can be pressed from the edicts' page"), Window->GetPeoplesTabButton()->GetIsEnabled());
	Window->GetPeoplesTabButton()->Press();
	TestTrue(TEXT("and turns to the peoples' page"), Window->GetTab() == ECosmicEdictTab::Peoples && !Window->GetPeoplesTabButton()->GetIsEnabled()
		&& Window->GetEdictsTabButton()->GetIsEnabled() && Window->GetSharesTabButton()->GetIsEnabled());
	TestTrue(TEXT("shown alone"), Shown(Window->GetPeoplesPage()) && !Shown(Window->GetEdictsPage()) && !Shown(Window->GetSharesPage()));
	Window->GetEdictsTabButton()->Press();
	TestTrue(TEXT("(back on the edicts' page)"), Window->GetTab() == ECosmicEdictTab::Edicts && Shown(Window->GetEdictsPage()) && !Shown(Window->GetPeoplesPage()));
	Window->GetCloseButton()->Press();
	TestFalse(TEXT("[Close] closes the window"), Window->IsOpen() || Shown(Window));
	EdictButton->Press();

	// The rows: the first rung's (no row of a higher rung stands - the test's and the game's seventh, B4), lexical; the
	// command's own gate on each.
	TArray<FName> Expected = Tables.GetEdictRowNames();
	Expected.RemoveAll([&Tables](FName Row)
	{
		const FEdictDef* Def = Tables.FindEdict(Row);
		return !Def || Def->RequiredTier != EAscensionTier::Tier1_Planetary;
	});
	TestTrue(TEXT("(the first rung's rows: the game's six and the test's bound one)"), Expected.Num() == 7 && !Expected.Contains(High));
	TestTrue(TEXT("The window lists the first rung's edicts in lexical order"), Window->GetShownRows() == Expected);
	TestTrue(TEXT("Mobilization's row says it was inherited"), Shown(Window->GetRowMark(Mobilization)));
	TestEqual(TEXT("and offers its revocation"), Words(Window->GetRowButton(Mobilization)->GetLabel()), Words(TEXT("Edict.Revoke")));
	TestTrue(TEXT("enabled"), Window->GetRowButton(Mobilization)->GetIsEnabled());
	bool bSameGate = true;
	for (const FName Row : Window->GetShownRows())
	{
		if (Row != Mobilization)
		{
			const TOptional<CosmicEdicts::ECosmicEdictBlock> Block = Window->GetShownBlock(Row);
			bSameGate &= Block.IsSet() && Block.GetValue() == CosmicEdicts::EnactBlockOf(Live, Index, Tables, Rules, Ruled, Row);
			bSameGate &= !Window->GetRowButton(Row)->GetIsEnabled() && !Shown(Window->GetRowMark(Row));
		}
		TestFalse(*FString::Printf(TEXT("%s's tooltip has every slot filled"), *Row.ToString()), HasPlaceholder(Window->GetRowButton(Row)->GetToolTipText()));
	}
	TestTrue(TEXT("Every other row shows the command's own gate, greyed"), bSameGate);
	TestTrue(TEXT("A row whose conditions fail says so before the slots"), Window->GetShownBlock(Bound) == TOptional<CosmicEdicts::ECosmicEdictBlock>(CosmicEdicts::ECosmicEdictBlock::Conditions)
		&& Words(Window->GetRowButton(Bound)->GetToolTipText()).StartsWith(Words(TEXT("Edict.Block.Conditions")))
		&& Words(Window->GetRowButton(Bound)->GetLabel()).Contains(Words(TEXT("Edict.Suffix.Conditions"))));
	TestTrue(TEXT("Machine rights waits for a slot"), Window->GetShownBlock(MachineRights) == TOptional<CosmicEdicts::ECosmicEdictBlock>(CosmicEdicts::ECosmicEdictBlock::Slots)
		&& Words(Window->GetRowButton(MachineRights)->GetToolTipText()).StartsWith(Words(TEXT("Edict.Block.Slots")))
		&& Words(Window->GetRowButton(MachineRights)->GetLabel()).Contains(Words(TEXT("Edict.Suffix.Slots"))));
	{
		const FString Tip = Words(UCosmicEdictWidget::TipOf(Tables, Mobilization, Unit));
		TestTrue(TEXT("Mobilization's tooltip: Energy +20%"), Tip.Contains(Words(FText::Format(CosmicUI::Label(TEXT("Edict.Effect.Income")), CosmicUI::Label(TEXT("Resource.Energy")), FText::AsCultureInvariant(TEXT("+20%"))))));
		TestTrue(TEXT("and Stability -15 points over a hundred cycles"), Tip.Contains(Words(FText::Format(CosmicUI::Label(TEXT("Edict.Effect.Stability")), FText::AsNumber(-15), CosmicUI::DurationLabel(100, Unit)))));
		TestTrue(TEXT("The vigil's says a standing hazard's time is not lengthened"), Words(UCosmicEdictWidget::TipOf(Tables, Vigil, Unit)).Contains(Words(TEXT("Edict.TipNewOnly"))));
		const FHazardDef* Awakening = Tables.FindHazardByTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.AIAwakening"))));
		TestTrue(TEXT("Machine rights' says it answers the awakening"), Awakening && Words(UCosmicEdictWidget::TipOf(Tables, MachineRights, Unit)).Contains(
			Words(FText::FormatNamed(CosmicUI::Label(TEXT("Edict.TipAnswers")), TEXT("Hazard"), Awakening->DisplayName))));
	}

	// B6b: the galaxy laws (Scope HomeGalaxy) - the row head's mark on every law and on no other row, and under a law's
	// hazard lines what reaches the whole home galaxy (none without hazard lines, none for a people's own edict).
	{
		const FName LawSpawn(TEXT("edict_test_law_spawn"));
		const FName LawPlain(TEXT("edict_test_law_plain"));
		const FName LawCountdown(TEXT("edict_test_law_countdown"));
		const FName PeopleSpawn(TEXT("edict_test_people_spawn"));
		const FGameplayTag Plague = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Plague")));
		auto RowOf = [&Plague](ECosmicEdictScope Scope, bool bSpawn, bool bCountdown = false)
		{
			FEdictDef Row;
			Row.DisplayName = FText::AsCultureInvariant(TEXT("<law>"));
			Row.RequiredTier = EAscensionTier::Tier4_Transcendent;
			Row.Scope = Scope;
			if (bSpawn)
			{
				Row.HazardSpawnMultipliers.Add(Plague, 0.5f);
			}
			if (bCountdown)
			{
				Row.HazardCountdownBonus.Add(Plague, 30);
			}
			return Row;
		};
		FCosmicDataTables WideTables = Tables;
		UDataTable* Edicts = CosmicTests::CopyWithout<FEdictDef>(Tables.Edicts, {});
		Edicts->AddRow(LawSpawn, RowOf(ECosmicEdictScope::HomeGalaxy, true));
		Edicts->AddRow(LawPlain, RowOf(ECosmicEdictScope::HomeGalaxy, false));
		Edicts->AddRow(LawCountdown, RowOf(ECosmicEdictScope::HomeGalaxy, false, true));
		Edicts->AddRow(PeopleSpawn, RowOf(ECosmicEdictScope::People, true));
		WideTables.Edicts = Edicts;
		const FString Wide = Words(TEXT("Edict.TipWide"));
		TestFalse(TEXT("(Edict.TipWide and Edict.Wide have words)"), Wide.IsEmpty() || HasPlaceholder(CosmicUI::Label(TEXT("Edict.TipWide")))
			|| Words(TEXT("Edict.Wide")).IsEmpty() || HasPlaceholder(CosmicUI::Label(TEXT("Edict.Wide"))));
		const FString LawTip = Words(UCosmicEdictWidget::TipOf(WideTables, LawSpawn, Unit));
		TestTrue(TEXT("A galaxy law's tooltip says what reaches the home galaxy, under its hazard lines"), LawTip.EndsWith(TEXT("\n") + Wide));
		TestFalse(TEXT("(not first)"), LawTip.StartsWith(Wide));
		const FHazardDef* PlagueRow = WideTables.FindHazardByTag(Plague);
		if (TestNotNull(TEXT("(the game has the plague row)"), PlagueRow))
		{
			FFormatNamedArguments SpawnArgs;
			SpawnArgs.Add(TEXT("Hazard"), PlagueRow->DisplayName);
			SpawnArgs.Add(TEXT("Factor"), UCosmicEdictWidget::AmountOf(0.5f));
			TestTrue(TEXT("right after the spawn line"), LawTip.Contains(Words(FText::Format(CosmicUI::Label(TEXT("Edict.Effect.Spawn")), SpawnArgs)) + TEXT("\n") + Wide));
		}
		const FString CountdownTip = Words(UCosmicEdictWidget::TipOf(WideTables, LawCountdown, Unit));
		TestTrue(TEXT("A galaxy law with only a countdown line says it too"), CountdownTip.EndsWith(TEXT("\n") + Wide));
		TestTrue(TEXT("right after what the countdown reaches"), CountdownTip.Contains(Words(TEXT("Edict.TipNewOnly")) + TEXT("\n") + Wide));
		TestFalse(TEXT("A galaxy law without hazard lines does not"), Words(UCosmicEdictWidget::TipOf(WideTables, LawPlain, Unit)).Contains(Wide));
		TestFalse(TEXT("nor a people's own edict with them"), Words(UCosmicEdictWidget::TipOf(WideTables, PeopleSpawn, Unit)).Contains(Wide));
		Sim->SetTables(WideTables);
		const EAscensionTier WasTier = Civ->Tier;
		Civ->Tier = EAscensionTier::Tier4_Transcendent;
		Window->Refresh();
		TestTrue(TEXT("(on the fourth rung the laws' rows stand)"), Window->GetShownRows().Contains(LawSpawn) && Window->GetShownRows().Contains(PeopleSpawn));
		bool bMarks = true;
		for (const FName Row : Window->GetShownRows())
		{
			bMarks &= Shown(Window->GetRowWideMark(Row)) == CosmicEdicts::IsGalaxyLaw(WideTables, Row);
		}
		TestTrue(TEXT("A galaxy law's row carries the mark"), Shown(Window->GetRowWideMark(LawSpawn)) && Shown(Window->GetRowWideMark(LawPlain)));
		TestTrue(TEXT("and no other row does"), bMarks && !Shown(Window->GetRowWideMark(PeopleSpawn)));
		TestEqual(TEXT("which says the edict reaches the home galaxy"), Words(Window->GetRowWideMark(LawSpawn)->GetText()), Words(TEXT("Edict.Wide")));
		Civ->Tier = WasTier;
		Sim->SetTables(Tables);
		Window->Refresh();
	}

	// Every slot open and filled: the slots' reason no longer sends the player to Society research.
	{
		const float WasSociety = Civ->Research.FindRef(Society);
		Civ->Research.Add(Society, 1.f);
		Civ->Stockpile.Influence = 1000.f;
		Window->Refresh();
		Window->GetRowButton(MachineRights)->Press();
		Window->GetRowButton(OpenThought)->Press();
		TestEqual(TEXT("(three edicts in three slots)"), Civ->ActiveEdicts.Num(), 3);
		TestTrue(TEXT("Every slot taken and open: the reason says to revoke"), Window->GetShownBlock(FreeTrade) == TOptional<CosmicEdicts::ECosmicEdictBlock>(CosmicEdicts::ECosmicEdictBlock::Slots)
			&& Words(Window->GetRowButton(FreeTrade)->GetToolTipText()).StartsWith(Words(TEXT("Edict.Block.SlotsAll"))));
		Window->GetRowButton(MachineRights)->Press();
		Window->GetRowButton(OpenThought)->Press();
		Civ->Research.Add(Society, WasSociety);
		Window->Refresh();
	}

	// A press revokes; another enacts; the inherited mark comes back with the edict enacted again.
	Window->GetRowButton(Mobilization)->Press();
	TestTrue(TEXT("A press revokes mobilization"), Civ->ActiveEdicts.IsEmpty());
	TestFalse(TEXT("its mark goes"), Shown(Window->GetRowMark(Mobilization)));
	TestEqual(TEXT("the bar says none of one"), Words(EdictButton->GetLabel()), Count(0, 1));
	Civ->Stockpile.Influence = 0.f;
	Window->Refresh();
	TestTrue(TEXT("An empty stockpile greys machine rights by its cost"), Window->GetShownBlock(MachineRights) == TOptional<CosmicEdicts::ECosmicEdictBlock>(CosmicEdicts::ECosmicEdictBlock::Cost)
		&& Words(Window->GetRowButton(MachineRights)->GetToolTipText()).StartsWith(Words(TEXT("Edict.Block.Cost")))
		&& Words(Window->GetRowButton(MachineRights)->GetLabel()).Contains(Words(TEXT("Edict.Suffix.Cost"))));
	Window->GetRowButton(MachineRights)->Press();
	TestTrue(TEXT("and a press of the greyed button does nothing"), Civ->ActiveEdicts.IsEmpty());
	Civ->Stockpile.Influence = 100.f;
	Window->Refresh();
	TestTrue(TEXT("With 100 Influence it can be enacted"), Window->GetRowButton(MachineRights)->GetIsEnabled());
	Window->GetRowButton(MachineRights)->Press();
	TestTrue(TEXT("A press enacts machine rights"), Civ->ActiveEdicts == TArray<FName>({ MachineRights }));
	TestEqual(TEXT("paying its cost"), Civ->Stockpile.Influence, 100.f - RightsCost);
	Window->GetRowButton(MachineRights)->Press();
	Window->GetRowButton(Mobilization)->Press();
	TestTrue(TEXT("Mobilization enacted again"), Civ->ActiveEdicts == TArray<FName>({ Mobilization }));
	TestTrue(TEXT("carries the inherited mark again"), Shown(Window->GetRowMark(Mobilization)));

	// A rung up shows the second rung's row.
	Civ->Tier = EAscensionTier::Tier2_Stellar;
	Window->Refresh();
	TestTrue(TEXT("On the second rung its row stands"), Window->GetShownRows().Contains(High));
	// The game's seventh edict (B4 - its rung is the second) stands at the top: its row name comes first.
	TestTrue(TEXT("and the seventh edict heads the list"), !Window->GetShownRows().IsEmpty() && Window->GetShownRows()[0] == FName(TEXT("edict_against_silence")));
	Civ->Tier = EAscensionTier::Tier1_Planetary;
	Window->Refresh();
	TestFalse(TEXT("back on the first, it goes"), Window->GetShownRows().Contains(High));

	// The panel's edict line: on the ruled people's settled system only.
	const FGameplayTag AwakeningTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.AIAwakening")));
	const FEdictDef* Rights = Tables.FindEdict(MachineRights);
	const FString Can = Words(FText::FormatNamed(CosmicUI::Label(TEXT("Edict.Panel.Can")), TEXT("Edict"), Rights->DisplayName));
	const FString Held = Words(FText::FormatNamed(CosmicUI::Label(TEXT("Edict.Panel.Held")), TEXT("Edict"), Rights->DisplayName));
	{
		Live.Systems[Home].ActiveHazard = AwakeningTag;
		Live.Systems[Home].HazardTicksRemaining = 50;
		Live.Systems[Other].ActiveHazard = AwakeningTag;
		Live.Systems[Other].HazardTicksRemaining = 50;
		Panel->Refresh();
		TestTrue(TEXT("Under the awakening on the home: machine rights can answer it"), Words(Panel->GetDetailText()->GetText()).Contains(Can));
		Controller->SelectSystem(Live.Systems[Other].SystemID);
		HUD->RefreshPanel();
		const FString OtherWords = Words(Panel->GetDetailText()->GetText());
		TestFalse(TEXT("Another people's system says nothing of edicts"), OtherWords.Contains(Can) || OtherWords.Contains(Held));
		Controller->SelectSystem(HomeID);
		HUD->RefreshPanel();
	}

	// The shares: [Share by hand] opens the second page; [Share it so] only after a person's move, one command per press.
	if (!TestNotNull(TEXT("(the panel's [Share by hand])"), Panel->GetDirectButton()))
	{
		return false;
	}
	TestTrue(TEXT("The focus block shows [Share by hand]"), Shown(Panel->GetDirectButton()) && Panel->GetDirectButton()->GetIsEnabled());
	{
		const FString SocietyTip = Words(Panel->GetFocusButtons()[2]->GetToolTipText());
		TestTrue(TEXT("[Society]'s tooltip says what Society opens"), SocietyTip.Contains(Words(FText::Format(CosmicUI::Label(TEXT("Focus.TipOpensSlot")), CosmicUI::Percent(0.5f)))));
		TestFalse(TEXT("and no faster recovery while the knob gives none"), SocietyTip.Contains(Words(TEXT("Focus.TipOpensRecovery"))));
	}
	Panel->GetDirectButton()->Press();
	TestTrue(TEXT("[Share by hand] turns the open window to the shares' page"), Window->IsOpen() && Window->GetTab() == ECosmicEdictTab::Shares);
	TestEqual(TEXT("The sliders stand at the even shares' weights"), Window->GetSliderWeight(Society), 7);
	TestFalse(TEXT("Without a move [Share it so] is greyed"), Window->GetApplyButton()->GetIsEnabled());
	TestEqual(TEXT("and its tooltip says what stays"), Words(Window->GetApplyButton()->GetToolTipText()), Words(TEXT("Edict.Shares.Tip")));
	TestEqual(TEXT("Before a move the 'would' line is hidden"), Window->GetWouldText()->GetVisibility(), ESlateVisibility::Hidden);
	TestEqual(TEXT("and the now line says the shares now"), Words(Window->GetNowText()->GetText()),
		Words(FText::Format(CosmicUI::Label(TEXT("Edict.Shares.Now")), CosmicUI::FocusSharesOf(Civ->ResearchFocus))));
	TestTrue(TEXT("(the focus changes by a command)"), Controller->SetResearchFocus(Physics));
	TestEqual(TEXT("The sliders follow the shares while nobody moved them (0.55 of 0.85 is 13 of 20)"), Window->GetSliderWeight(Physics), 13);
	TestFalse(TEXT("and the window's own setting of them enables nothing"), Window->GetApplyButton()->GetIsEnabled());
	Window->MoveSlider(Society, 9);
	Panel->GetDirectButton()->Press();
	TestTrue(TEXT("[Share by hand] on the open window keeps a person's move"), Window->GetSliderWeight(Society) == 9 && Window->GetApplyButton()->GetIsEnabled());
	const TMap<FName, float> Before = Civ->ResearchFocus;
	Window->MoveSlider(Physics, 1);
	Window->MoveSlider(Society, 5);
	Window->MoveSlider(Engineering, 1);
	TMap<FName, float> Would = Civ->ResearchFocus;
	CosmicResearch::SharesOf(Would, { { Physics, 1 }, { Society, 5 }, { Engineering, 1 } });
	TestTrue(TEXT("A person's moves enable [Share it so]"), Window->GetApplyButton()->GetIsEnabled());
	TestTrue(TEXT("and command nothing yet"), Civ->ResearchFocus.OrderIndependentCompareEqual(Before));
	TestTrue(TEXT("The 'would' line shows the shares of 1-5-1"), Window->GetWouldText()->GetVisibility() == ESlateVisibility::HitTestInvisible
		&& Words(Window->GetWouldText()->GetText()) == Words(FText::Format(CosmicUI::Label(TEXT("Edict.Shares.Would")), CosmicUI::FocusSharesOf(Would))));
	HUD->RefreshPanel();
	StepBroadcast(*Sim);
	TestEqual(TEXT("A refresh and a tick leave the moved slider where the person put it"), Window->GetSliderWeight(Society), 5);
	Would = Civ->ResearchFocus;
	CosmicResearch::SharesOf(Would, { { Physics, 1 }, { Society, 5 }, { Engineering, 1 } });
	Window->GetApplyButton()->Press();
	TestTrue(TEXT("One press shares the research out by the weights"), Civ->ResearchFocus.OrderIndependentCompareEqual(Would));
	TestFalse(TEXT("and greys the button again"), Window->GetApplyButton()->GetIsEnabled());
	TestTrue(TEXT("The sliders stay where the person put them"), Window->GetSliderWeight(Physics) == 1 && Window->GetSliderWeight(Society) == 5 && Window->GetSliderWeight(Engineering) == 1);
	TestEqual(TEXT("and the now line says the shares given"), Words(Window->GetNowText()->GetText()),
		Words(FText::Format(CosmicUI::Label(TEXT("Edict.Shares.Now")), CosmicUI::FocusSharesOf(Would))));
	Window->MoveSlider(Physics, 0);
	Window->MoveSlider(Society, 0);
	Window->MoveSlider(Engineering, 0);
	TestFalse(TEXT("Three sliders at 0 grey [Share it so]"), Window->GetApplyButton()->GetIsEnabled());
	TestEqual(TEXT("and say why in the 'would' line"), Words(Window->GetWouldText()->GetText()), Words(TEXT("Edict.Shares.Zero")));
	Panel->Refresh();
	bool bAllPressable = true;
	for (const UCosmicActionButton* Button : Panel->GetFocusButtons())
	{
		bAllPressable &= Button->GetIsEnabled();
	}
	TestTrue(TEXT("Shares 1-5-1 leave the four focus buttons pressable"), bAllPressable);
	TestEqual(TEXT("and the heading says mixed"), Words(Panel->GetFocusHeading()->GetText()), Words(FText::Format(CosmicUI::Label(TEXT("Focus.Heading")), CosmicUI::Label(TEXT("Focus.Mixed")))));
	// A lean to society of another share has society's shape but is not the preset [Society] gives: it stays pressable.
	CosmicResearch::Focus(Civ->ResearchFocus, Society, 0.7f);
	Panel->Refresh();
	TestTrue(TEXT("A lean of 0.7 to society leaves [Society] pressable"), Panel->GetFocusButtons()[2]->GetIsEnabled());
	Panel->GetFocusButtons()[2]->Press();
	TestFalse(TEXT("[Society] gives society's preset back and greys"), Panel->GetFocusButtons()[2]->GetIsEnabled());
	// The faster recovery, while the knob gives one; every slot open.
	{
		FCosmicRulesDef Bonus = Rules;
		Bonus.SocietyRecoveryBonus = 0.25f;
		Sim->SetTables(CosmicTests::WithRules(Tables, Bonus));
		Panel->Refresh();
		TestTrue(TEXT("With a recovery bonus [Society] says it speeds the recovery"), Words(Panel->GetFocusButtons()[2]->GetToolTipText()).Contains(Words(TEXT("Focus.TipOpensRecovery"))));
		const float WasSociety = Civ->Research.FindRef(Society);
		Civ->Research.Add(Society, 1.f);
		Panel->Refresh();
		TestTrue(TEXT("At Society 1.0 it says every slot is open"), Words(Panel->GetFocusButtons()[2]->GetToolTipText()).Contains(Words(TEXT("Focus.TipOpensAll"))));
		Civ->Research.Add(Society, WasSociety);
		Sim->SetTables(Tables);
		HUD->RefreshPanel();
	}

	// The end of the run: no edict button, no window, [Share by hand] greyed, every command refused; the panel keeps only the
	// held form of its edict line.
	{
		Window->GetRowButton(Mobilization)->Press();
		Civ->Stockpile.Influence = 100.f;
		Window->Refresh();
		Window->GetRowButton(MachineRights)->Press();
		TestTrue(TEXT("(machine rights held before the end)"), Civ->ActiveEdicts == TArray<FName>({ MachineRights }));
		const int64 WasTick = Live.Tick;
		// The end laid by hand with the record its tick would have written (B6a [B-6a3c]: the latch reads the record).
		CosmicTests::EndRunAt(Live, CosmicTags::EndingClock(), Sim->GetRules().ProvisionalEndTick);
		Controller->FollowEnd(false);
		TestTrue(TEXT("(the end latched)"), Controller->IsRunEnded());
		HUD->RefreshPanel();
		TestFalse(TEXT("After the end the bar has no edict button"), Shown(EdictButton->GetParent()));
		TestFalse(TEXT("and the window closes"), Window->IsOpen() || Shown(Window));
		Window->Open(ECosmicEdictTab::Edicts);
		TestFalse(TEXT("and will not open"), Window->IsOpen());
		TestTrue(TEXT("[Share by hand] is greyed with the end's words"), !Panel->GetDirectButton()->GetIsEnabled()
			&& Words(Panel->GetDirectButton()->GetToolTipText()) == Words(TEXT("Chronicle.Block")));
		const TMap<FName, float> Focus = Civ->ResearchFocus;
		TestFalse(TEXT("The controller refuses an enactment"), Controller->EnactEdict(Vigil));
		TestFalse(TEXT("a revocation"), Controller->RevokeEdict(MachineRights));
		TestFalse(TEXT("and shares"), Controller->SetResearchShares({ { Physics, 1 }, { Society, 5 }, { Engineering, 1 } }));
		Controller->CosmicShares(1, 5, 1);
		TestTrue(TEXT("(the console's shares too)"), Civ->ActiveEdicts == TArray<FName>({ MachineRights }) && Civ->ResearchFocus.OrderIndependentCompareEqual(Focus));
		TestTrue(TEXT("The held edict's line stays after the end"), Words(Panel->GetDetailText()->GetText()).Contains(Held));
		Controller->ClearRunEnd();
		Live.Tick = WasTick;
		Live.Ending = FCosmicEndingRecord();
		HUD->RefreshPanel();
		// The other form: machine rights not held.
		EdictButton->Press();
		Window->GetRowButton(MachineRights)->Press();
		TestTrue(TEXT("(machine rights revoked)"), Civ->ActiveEdicts.IsEmpty());
		CosmicTests::EndRunAt(Live, CosmicTags::EndingClock(), Sim->GetRules().ProvisionalEndTick);
		Controller->FollowEnd(false);
		TestTrue(TEXT("(the end latched again)"), Controller->IsRunEnded());
		HUD->RefreshPanel();
		TestFalse(TEXT("An edict that only could answer is not offered after the end"), Words(Panel->GetDetailText()->GetText()).Contains(Can));
		Controller->ClearRunEnd();
		Live.Tick = WasTick;
		Live.Ending = FCosmicEndingRecord();
		Live.Systems[Home].ActiveHazard = FGameplayTag();
		Live.Systems[Other].ActiveHazard = FGameplayTag();
		HUD->RefreshPanel();
	}

	// No window and no button but for an embodied ruler.
	EdictButton->Press();
	TestTrue(TEXT("(the window is open again)"), Window->IsOpen());
	Live.Player.Role = EPlayerRole::Wanderer;
	HUD->RefreshPanel();
	TestFalse(TEXT("A wanderer's bar has no edict button"), Shown(EdictButton->GetParent()));
	TestFalse(TEXT("and the window closes"), Window->IsOpen() || Shown(Window));
	Live.Player.Role = EPlayerRole::Ruler;
	HUD->RefreshPanel();

	// A new galaxy: the open window closes, and a hint raised and left unanswered waits for the new run's first fall.
	{
		EdictButton->Press();
		FCosmicHintProgress Started;
		Started.bEdictHintOn = true;
		Controller->SetHintProgress(Started);
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		TestFalse(TEXT("A new galaxy closes the window"), Window->IsOpen() || Shown(Window));
		TestFalse(TEXT("and an unanswered hint waits again"), Controller->GetHintProgress().bEdictHintOn || Controller->GetHintProgress().bEdictHintDone);
		FCosmicHintProgress Ended;
		Ended.bEdictHintOn = true;
		Ended.bEdictHintDone = true;
		Controller->SetHintProgress(Ended);
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		TestTrue(TEXT("A hint that ended stays ended"), Controller->GetHintProgress().bEdictHintDone);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

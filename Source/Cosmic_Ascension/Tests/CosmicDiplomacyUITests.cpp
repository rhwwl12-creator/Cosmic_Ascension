// Cosmic Ascension - automation tests of diplomacy and conflict on the screen (B3): the edict window's peoples' page, the
// system panel's fleet lines and buttons, the edicts' two new tooltip lines and the top bar's edict button.
// Design: docs/60_DECISIONS/headers/B3_diplomacy_and_conflict.md section 4 and 6.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/WidgetTree.h"
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
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "GenericPlatform/GenericApplication.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "Internationalization/StringTableCore.h"
#include "Layout/Geometry.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicEdictWidget.h"
#include "UI/CosmicPetitionCardWidget.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "UI/CosmicUILabels.h"
#include "Widgets/Input/SButton.h"
#include "World/CosmicGameMode.h"

#include <limits>

namespace CosmicDiplomacyUITests
{
	using CosmicConflict::ECosmicSiegeBlock;
	using CosmicDiplomacy::ECosmicStanceBlock;
	using CosmicDiplomacy::ECosmicTreatyBlock;

	// The window's inner width (340 less 14 a side, B3 header section 4): no line of the page may ask for more.
	constexpr float InnerWidth = 312.f;
	// The rows sit in the page's scroll box, whose bar (and its gap) takes about this much beside them once it scrolls.
	constexpr float RowWidth = InnerWidth - 16.f;
	// The board's edict that eases the treaties and quickens the fleet (B3 section 3.2).
	const FName AccordRow(TEXT("edict_test_accord"));

	FString Words(const FText& Text)
	{
		return Text.ToString();
	}

	FString Words(const TCHAR* Key)
	{
		return CosmicUI::Label(Key).ToString();
	}

	// The words a key says before its first argument (to look for a line whatever its number).
	FString Lead(const TCHAR* Key)
	{
		const FString All = Words(Key);
		int32 At = INDEX_NONE;
		return All.FindChar(TEXT('{'), At) ? All.Left(At) : All;
	}

	bool HasPlaceholder(const FText& Text)
	{
		return Text.ToString().Contains(FStringTableEntry::GetPlaceholderSourceString()) || Text.ToString().Contains(TEXT("{"));
	}

	bool Shown(const UWidget* Widget)
	{
		return Widget && Widget->GetVisibility() != ESlateVisibility::Collapsed && Widget->GetVisibility() != ESlateVisibility::Hidden;
	}

	float WidthOf(const UWidget* Widget)
	{
		UWidget* Mutable = const_cast<UWidget*>(Widget);
		if (!Mutable)
		{
			return 0.f;
		}
		Mutable->ForceLayoutPrepass();
		return static_cast<float>(Mutable->GetDesiredSize().X);
	}

	FName IDOf(const FGalaxyState& State, int32 SystemIndex)
	{
		return State.Systems[SystemIndex].SystemID;
	}

	UCosmicActionButton* Find(const TArray<TObjectPtr<UCosmicActionButton>>& Buttons, FName Action)
	{
		for (const TObjectPtr<UCosmicActionButton>& Button : Buttons)
		{
			if (Button && Button->GetAction() == Action)
			{
				return Button.Get();
			}
		}
		return nullptr;
	}

	// The panel's fleet button of Action ([Guard here] or [Send the fleet]).
	UCosmicActionButton* FleetButton(const UCosmicSystemPanelWidget& Panel, FName Action)
	{
		return Find(Panel.GetFleetButtons(), Action);
	}

	// A label with its short reason after it, as the panel joins them.
	FString WithSuffix(const TCHAR* Name, const TCHAR* Suffix)
	{
		return Words(FText::Format(INVTEXT("{0}  {1}"), CosmicUI::Label(Name), CosmicUI::Label(Suffix)));
	}

	/**
	 * The board of the B3 sim tests on the game's tables: twelve living systems wired as CosmicTests::WireChain and four
	 * peoples owning all of them (0 {0, 1}, 1 {3, 2, 4}, 2 {6, 5, 7}, 3 {9, 8, 10, 11}; capitals 0, 3, 6, 9), fleets from the
	 * second rung (one a settled system), no drift, no transcendent, every system known; people 0 the ruled one on the
	 * first rung, the others on the second. Met: 2 then 1 (in that order) - people 3 not. The diplomacy and siege knobs the
	 * page reads are set here (the expectations below hold whatever [B-3d] tunes), and the only edict is AccordRow
	 * (TreatyAcceptBonus 0.15 - a bar that is no tenth, FleetGrowthMultiplier 1.5). The run is the game's otherwise.
	 */
	FGalaxyState* LayBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Original)
	{
		const FCosmicRulesDef* Game = Original.FindRules();
		if (!Game)
		{
			return nullptr;
		}
		FCosmicRulesDef Rules = CosmicTests::WithoutStartingEdict(*Game);
		Rules.EventChancePerSlot = 0.f;
		Rules.FirstCrisisTick = 0;
		Rules.RelationDriftRate = 0.f;
		Rules.FleetPerSettledByTier = { 0.f, 1.f, 1.f, 1.f, 1.f };
		Rules.RealmCapByTier = { 4, 4, 5, 5, 5 };
		Rules.FirstSightTick = 1000000;
		Rules.SightIntervalTicks = 1000000;
		Rules.RelationEthicsWeight = 0.6f;
		Rules.RelationStanceTerms = { 0.4f, -0.4f };
		Rules.RelationTreatyTerm = 0.2f;
		Rules.BetrayalRelationPenalty = 0.3f;
		Rules.ConquestRelationPenalty = 0.2f;
		Rules.FriendlySlotsByTier = { 1, 1, 2, 2, 3 };
		Rules.FriendlyUpkeepInfluence = 0.2f;
		Rules.TreatyAcceptRelation = -0.1f;
		Rules.TreatyCostInfluence = 50.f;
		Rules.TradeInfluencePerTreaty = 0.2f;
		Rules.SiegeTicks = 200;
		Rules.SiegeDefenceShares = { 1.f, 0.5f, 0.25f };
		Rules.SiegeRepulsedFleetFactor = 0.5f;
		Rules.FleetEffectivenessByTierGap = { 1.f, 0.5f, 0.f };
		Rules.FleetDelayPerPower = 50.f;
		Rules.FleetUpkeepEnergyPerPower = 0.5f;
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));
		UDataTable* Edicts = CosmicTests::NewTable(FEdictDef::StaticStruct());
		FEdictDef Accord;
		Accord.DisplayName = FText::AsCultureInvariant(TEXT("accord"));
		Accord.TreatyAcceptBonus = 0.15f;
		Accord.FleetGrowthMultiplier = 1.5f;
		Edicts->AddRow(AccordRow, Accord);
		Tables.Edicts = Edicts;
		Sim.SetTables(Tables);
		Sim.NewGalaxy(42, 12, 4);
		Sim.SetTimeScale(0.f);
		FGalaxyState& State = Sim.GetMutableState();
		if (State.Civs.Num() != 4 || !CosmicTests::WireChain(State))
		{
			return nullptr;
		}
		State.Transcendents.Reset();
		CosmicTests::ClearOwners(State);
		for (FStarSystemState& System : State.Systems)
		{
			System.ActiveHazard = FGameplayTag();
			System.ActiveMegaproject = NAME_None;
			System.ResponseMegaproject = NAME_None;
		}
		CosmicTests::Give(State, 0, { 0, 1 });
		CosmicTests::Give(State, 1, { 3, 2, 4 });
		CosmicTests::Give(State, 2, { 6, 5, 7 });
		CosmicTests::Give(State, 3, { 9, 8, 10, 11 });
		const int32 Capitals[] = { 0, 3, 6, 9 };
		for (int32 CivIndex = 0; CivIndex < 4; ++CivIndex)
		{
			FCivilizationState& Civ = State.Civs[CivIndex];
			Civ.HomeSystemID = State.Systems[Capitals[CivIndex]].SystemID;
			Civ.Tier = CivIndex == 0 ? EAscensionTier::Tier1_Planetary : EAscensionTier::Tier2_Stellar;
			Civ.Ethics = FEthicsAxes();
			Civ.FleetPower = CivIndex == 0 ? 0.f : 1.f;
			Civ.FleetTargetSystemID = NAME_None;
			Civ.Ties.Reset();
			Civ.Relations.Reset();
			Civ.ActiveEdicts.Reset();
			Civ.Stockpile.Influence = 100.f;
		}
		State.Player.CurrentSystemID = State.Systems[0].SystemID;
		CosmicTests::KnowAll(State);
		CosmicTests::SetKnowing(State, {}, { 0, 2, 1 });
		for (const FStarSystemState& System : State.Systems)
		{
			FKnownMark& Mark = State.Knowing.KnownSystems.AddDefaulted_GetRef();
			Mark.ID = System.SystemID;
		}
		Sim.RebuildStateIndex();
		return &State;
	}

	// Meets 2 then 1 again, every system known (after a test cleared the marks).
	void MeetAgain(FGalaxyState& State)
	{
		CosmicTests::SetKnowing(State, {}, { 0, 2, 1 });
		for (const FStarSystemState& System : State.Systems)
		{
			FKnownMark& Mark = State.Knowing.KnownSystems.AddDefaulted_GetRef();
			Mark.ID = System.SystemID;
		}
	}
}

// The peoples' page and the panel's fleet (B3 header section 4): the tabs, the rows of the peoples met in their order, each
// line within the window's width at its widest, the band and its tooltip's terms, their hostility, the buttons' gates the
// commands' own - each press through the button one command -, the tooltips' costs and consequences; the fleet's head;
// the panel's [Guard here], [Send the fleet] and siege lines - our side's band, a besieger not shown nameless, a siege
// between two other peoples untold, an owner not shown offered nothing; the edicts' new tooltip lines and the top bar's.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIPeoplesTabTest, "Cosmic_Ascension.UI.PeoplesTab", CosmicTests::Flags)
bool FCosmicUIPeoplesTabTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDiplomacyUITests;
	using CosmicUI::Label;
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
	ON_SCOPE_EXIT
	{
		Window->Close();
		Controller->CosmicSeeAll(0);
		Controller->ClearRunEnd();
		Controller->SelectSystem(NAME_None);
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	FGalaxyState* State = LayBoard(*Sim, Original);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicRulesDef& Rules = Sim->GetRules();
	TArray<FCivilizationState>& Civs = State->Civs;
	const FName Zero = Civs[0].CivID;
	const FName One = Civs[1].CivID;
	const FName Two = Civs[2].CivID;
	const ECosmicTimeUnit Unit = ACosmicPlayerController::TimeUnitFor(*Window);
	TestTrue(TEXT("(the player rules people 0)"), Civs[0].bIsPlayerRuled && State->Player.RuledCivID == Zero);

	// The page: the tab, the pages, the peoples met in their meeting order, no other entry.
	Window->Open(ECosmicEdictTab::Peoples);
	TestTrue(TEXT("The window opens on the peoples' page"), Window->IsOpen() && Window->GetTab() == ECosmicEdictTab::Peoples);
	TestFalse(TEXT("whose tab is greyed (the page shown)"), Window->GetPeoplesTabButton()->GetIsEnabled());
	TestTrue(TEXT("and shows that page alone"), Shown(Window->GetPeoplesPage()) && !Shown(Window->GetEdictsPage()) && !Shown(Window->GetSharesPage()));
	TestEqual(TEXT("The tab says the peoples"), Words(Window->GetPeoplesTabButton()->GetLabel()), Words(TEXT("Diplomacy.Tab")));
	Window->GetEdictsTabButton()->Press();
	TestTrue(TEXT("Another tab turns away from it, and the peoples' tab can be pressed again"), Window->GetTab() == ECosmicEdictTab::Edicts
		&& !Shown(Window->GetPeoplesPage()) && Window->GetPeoplesTabButton()->GetIsEnabled());
	Window->GetPeoplesTabButton()->Press();
	TestTrue(TEXT("Its press turns back to the page"), Window->GetTab() == ECosmicEdictTab::Peoples && Shown(Window->GetPeoplesPage()) && !Shown(Window->GetEdictsPage()));
	TestTrue(TEXT("The rows are the peoples met, in the meeting order (2, then 1 - 3 is not met)"), Window->GetShownPeoples() == TArray<FName>({ Two, One }));
	TestFalse(TEXT("(nobody-met line hidden)"), Shown(Window->GetNoPeoplesText()));
	TestEqual(TEXT("With no fleet and no cap the head says there is no fleet yet"), Words(Window->GetFleetText()->GetText()), Words(TEXT("Guard.NoFleet")));
	TestFalse(TEXT("(no target line)"), Shown(Window->GetTargetText()));
	TestFalse(TEXT("(no [Withdraw the fleet])"), Shown(Window->GetWithdrawButton()));
	TestTrue(TEXT("The tab line fits the window"), WidthOf(Window->GetTabLine()) > 0.f && WidthOf(Window->GetTabLine()) <= InnerWidth);
	TestTrue(TEXT("and so does each row beside the scroll bar"), WidthOf(Window->GetPeopleRow(One)) > 0.f && WidthOf(Window->GetPeopleRow(One)) <= RowWidth
		&& WidthOf(Window->GetPeopleRow(Two)) > 0.f && WidthOf(Window->GetPeopleRow(Two)) <= RowWidth);
	// A key of Relations and a met mark that are no people (a transcendent's), and the whole sky shown: no other row.
	{
		const FName Stranger(TEXT("tr_test_stranger"));
		Civs[0].Relations.Add(Stranger, 0.5f);
		FKnownMark& Mark = State->Knowing.MetCivs.AddDefaulted_GetRef();
		Mark.ID = Stranger;
		Window->Refresh();
		TestTrue(TEXT("A met mark or a relation that is no people's gives no row"), Window->GetShownPeoples() == TArray<FName>({ Two, One }));
		Civs[0].Relations.Remove(Stranger);
		State->Knowing.MetCivs.RemoveAll([Stranger](const FKnownMark& Each) { return Each.ID == Stranger; });
		Controller->CosmicSeeAll(1);
		Window->Refresh();
		TestTrue(TEXT("Seeing all, a people not met still has no row"), Window->GetShownPeoples() == TArray<FName>({ Two, One }));
		Controller->CosmicSeeAll(0);
		Window->Refresh();
	}
	// The head's tooltips: how the fleet grows and its upkeep a size; the rung that opens the next friendly slot.
	{
		TestEqual(TEXT("The fleet's tooltip says how it grows and its upkeep"), Words(Window->GetFleetText()->GetToolTipText()), Words(CosmicUI::JoinLines({
			Label(TEXT("Siege.FleetTip")), FText::Format(Label(TEXT("Siege.FleetUpkeep")), UCosmicEdictWidget::AmountOf(0.5f), CosmicUI::DurationLabel(1, Unit)) })));
		TestEqual(TEXT("The slots' tooltip names the rung of the next slot (the third: 1, 1, 2)"), Words(Window->GetSlotsText()->GetToolTipText()),
			Words(CosmicUI::JoinLines({ Label(TEXT("Diplomacy.SlotsTip")), FText::Format(Label(TEXT("Diplomacy.SlotsNext")), FText::AsNumber(3)) })));
		FCivilizationState Probe;
		Probe.Tier = EAscensionTier::Tier3_Causal;
		TestTrue(TEXT("From the third rung the next slot is the fifth's"), UCosmicEdictWidget::NextFriendlySlotRungOf(Probe, Rules) == TOptional<int32>(4));
		Probe.Tier = EAscensionTier::Tier5_EternalOrder;
		TestFalse(TEXT("and on the fifth every slot is open"), UCosmicEdictWidget::NextFriendlySlotRungOf(Probe, Rules).IsSet());
		Civs[0].Tier = EAscensionTier::Tier5_EternalOrder;
		Window->Refresh();
		TestEqual(TEXT("which the slots' tooltip says"), Words(Window->GetSlotsText()->GetToolTipText()),
			Words(CosmicUI::JoinLines({ Label(TEXT("Diplomacy.SlotsTip")), Label(TEXT("Diplomacy.SlotsAll")) })));
		Civs[0].Tier = EAscensionTier::Tier1_Planetary;
		Window->Refresh();
		// The words of the bands and of the odds (CosmicDiplomacy::ViewBandOf's edges, CosmicConflict::OddsBandOf's).
		TestTrue(TEXT("The view's words by band"), Words(UCosmicEdictWidget::ViewWordOf(-0.5f)) == Words(TEXT("Diplomacy.View.0"))
			&& Words(UCosmicEdictWidget::ViewWordOf(-0.2f)) == Words(TEXT("Diplomacy.View.1")) && Words(UCosmicEdictWidget::ViewWordOf(0.f)) == Words(TEXT("Diplomacy.View.2"))
			&& Words(UCosmicEdictWidget::ViewWordOf(0.3f)) == Words(TEXT("Diplomacy.View.3")) && Words(UCosmicEdictWidget::ViewWordOf(0.6f)) == Words(TEXT("Diplomacy.View.4")));
		TestTrue(TEXT("The odds' words by band"), Words(UCosmicEdictWidget::OddsWordOf(0.6f)) == Words(TEXT("Siege.Band.Ahead"))
			&& Words(UCosmicEdictWidget::OddsWordOf(0.5f)) == Words(TEXT("Siege.Band.Even")) && Words(UCosmicEdictWidget::OddsWordOf(0.4f)) == Words(TEXT("Siege.Band.Behind")));
		TestTrue(TEXT("A number nearer 0 than 0.005 is a plain 0 (no '-0', no '+0')"), Words(UCosmicEdictWidget::AmountOf(-0.005f)) == Words(UCosmicEdictWidget::AmountOf(0.f))
			&& Words(UCosmicEdictWidget::AmountOf(-0.f)) == Words(UCosmicEdictWidget::AmountOf(0.f)) && Words(UCosmicEdictWidget::AmountOf(0.f)) == TEXT("0"));
	}

	// The band of the view they hold of us, and its terms.
	{
		Civs[1].Relations.Add(Zero, -0.6f);
		Window->Refresh();
		TestEqual(TEXT("People 1 (bordering) shows the band of its stored view"), Words(Window->GetPeopleBand(One)->GetText()), Words(TEXT("Diplomacy.View.0")));
		auto Line = [](const TCHAR* Key, const TCHAR* Amount)
		{
			return FText::Format(CosmicUI::Label(Key), FText::AsCultureInvariant(Amount));
		};
		TestEqual(TEXT("whose tooltip tells the terms, not the view that moves - a term of 0 a plain 0"), Words(Window->GetPeopleBand(One)->GetToolTipText()),
			Words(CosmicUI::JoinLines({ Line(TEXT("Diplomacy.TipTarget"), TEXT("0")), Line(TEXT("Diplomacy.TipEthics"), TEXT("0")),
				Line(TEXT("Diplomacy.TipStance"), TEXT("0")), Line(TEXT("Diplomacy.TipTreaties"), TEXT("0")) })));
		// Each term where the rule reads it: their ethics (-0.6 x 1/3), our friendly stance (+0.4 - theirs, hostile, is no term
		// of their view), one treaty (+0.2); the target is their sum.
		Civs[1].Ethics.Pacifism = -1.f;
		CosmicDiplomacy::SetStanceOf(Civs[0], One, CosmicTags::StanceFriendly());
		CosmicDiplomacy::SetStanceOf(Civs[1], Zero, CosmicTags::StanceHostile());
		CosmicDiplomacy::Sign(*State, 0, 1, CosmicTags::TreatyTrade());
		Window->Refresh();
		TestEqual(TEXT("The terms of a bordering pair, signed"), Words(Window->GetPeopleBand(One)->GetToolTipText()),
			Words(CosmicUI::JoinLines({ Line(TEXT("Diplomacy.TipTarget"), TEXT("+0.4")), Line(TEXT("Diplomacy.TipEthics"), TEXT("-0.2")),
				Line(TEXT("Diplomacy.TipStance"), TEXT("+0.4")), Line(TEXT("Diplomacy.TipTreaties"), TEXT("+0.2")) })));
		CosmicDiplomacy::Break(*State, 0, 1, CosmicTags::TreatyTrade());
		CosmicDiplomacy::SetStanceOf(Civs[0], One, FGameplayTag());
		CosmicDiplomacy::SetStanceOf(Civs[1], Zero, FGameplayTag());
		Civs[1].Ethics = FEthicsAxes();
		// People 2 does not border us: the band of the target without the stance - our friendly stance would lift the target
		// to 0 (indifferent), the view stays at the ethics' -0.4 (cold).
		Civs[2].Ethics.Pacifism = -1.f;
		Civs[2].Ethics.Spiritualism = -1.f;
		CosmicDiplomacy::SetStanceOf(Civs[0], Two, CosmicTags::StanceFriendly());
		Window->Refresh();
		const float FarView = CosmicDiplomacy::ViewOf(*State, Index, Rules, 2, 0);
		const float FarTarget = CosmicDiplomacy::TargetRelationOf(*State, Rules, 2, 0);
		TestEqual(TEXT("People 2 (not bordering) shows the band of the view without the stance"), Words(Window->GetPeopleBand(Two)->GetText()), Words(TEXT("Diplomacy.View.1")));
		TestTrue(TEXT("(the view and the target are in other bands)"), Words(UCosmicEdictWidget::ViewWordOf(FarView)) != Words(UCosmicEdictWidget::ViewWordOf(FarTarget)));
		TestEqual(TEXT("whose tooltip tells its two terms and that the border is far - no stance, no target"), Words(Window->GetPeopleBand(Two)->GetToolTipText()),
			Words(CosmicUI::JoinLines({ Line(TEXT("Diplomacy.TipEthics"), TEXT("-0.4")), Line(TEXT("Diplomacy.TipTreaties"), TEXT("0")), Label(TEXT("Diplomacy.TipFar")) })));
		CosmicDiplomacy::SetStanceOf(Civs[0], Two, FGameplayTag());
		Civs[2].Ethics = FEthicsAxes();
		TestFalse(TEXT("No hostility shown yet"), Shown(Window->GetPeopleHostility(One)));
		CosmicDiplomacy::SetStanceOf(Civs[1], Zero, CosmicTags::StanceHostile());
		Window->Refresh();
		TestTrue(TEXT("Their hostility toward us shows"), Shown(Window->GetPeopleHostility(One)));
		TestTrue(TEXT("on a line of its own in the row"), Window->GetPeopleHostility(One)->GetParent() == Window->GetPeopleRow(One));
		// The widest row: a long name, their hostility, both treaties held (the breaking labels), the second rung.
		const FText Name = Civs[1].CivName;
		Civs[1].CivName = FText::AsCultureInvariant(TEXT("\uBBF8\uC544\uD50C\uB77C\uD0A4\uB450\uC2A4 \uACF5\uD654\uAD6D"));
		Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		CosmicDiplomacy::Sign(*State, 0, 1, CosmicTags::TreatyTrade());
		CosmicDiplomacy::Sign(*State, 0, 1, CosmicTags::TreatyNonAggression());
		Window->Refresh();
		const float Widest = WidthOf(Window->GetPeopleRow(One));
		TArray<FString> LineWidths;
		if (const UPanelWidget* RowBox = Cast<UPanelWidget>(Window->GetPeopleRow(One)))
		{
			for (int32 Child = 0; Child < RowBox->GetChildrenCount(); ++Child)
			{
				LineWidths.Add(FString::Printf(TEXT("%.1f"), WidthOf(RowBox->GetChildAt(Child))));
			}
		}
		AddInfo(FString::Printf(TEXT("The tab line asks for %.1f, the widest row for %.1f (its lines %s)"), WidthOf(Window->GetTabLine()), Widest,
			*FString::Join(LineWidths, TEXT(", "))));
		TestTrue(TEXT("The widest row fits beside the scroll bar"), Widest > 0.f && Widest <= RowWidth);
		CosmicDiplomacy::Break(*State, 0, 1, CosmicTags::TreatyTrade());
		CosmicDiplomacy::Break(*State, 0, 1, CosmicTags::TreatyNonAggression());
		Civs[0].Tier = EAscensionTier::Tier1_Planetary;
		Civs[1].CivName = Name;
		CosmicDiplomacy::SetStanceOf(Civs[1], Zero, FGameplayTag());
		Civs[1].Relations.Reset();
		Window->Refresh();
	}

	// The stance buttons: the commands' gate, the one held greyed and marked; a press through the button is one command.
	{
		const bool bMet = true;
		for (const FName Other : { One, Two })
		{
			for (int32 At = 0; At < 3; ++At)
			{
				const ECosmicStanceBlock Gate = CosmicDiplomacy::StanceBlockOf(*State, Index, Rules, 0, Other, UCosmicEdictWidget::StanceAt(At), bMet);
				TestTrue(*FString::Printf(TEXT("Stance %d of %s greys by the command's gate"), At, *Other.ToString()),
					Window->GetShownStanceBlock(Other, At) == TOptional<ECosmicStanceBlock>(Gate) && Window->GetStanceButton(Other, At)->GetIsEnabled() == (Gate == ECosmicStanceBlock::None));
				TestFalse(TEXT("(its tooltip filled)"), HasPlaceholder(Window->GetStanceButton(Other, At)->GetToolTipText()));
			}
		}
		TestTrue(TEXT("Neutral, held, is greyed as the stance now"), Window->GetShownStanceBlock(One, 1) == TOptional<ECosmicStanceBlock>(ECosmicStanceBlock::Same)
			&& Words(Window->GetStanceButton(One, 1)->GetToolTipText()) == Words(TEXT("Diplomacy.TipNow")));
		TestEqual(TEXT("and marked apart from a refused one"), Words(Window->GetStanceButton(One, 1)->GetLabel()),
			Words(FText::Format(INVTEXT("{0} {1}"), Label(TEXT("Edict.Mark")), CosmicDiplomacy::StanceNameOf(FGameplayTag()))));
		TestEqual(TEXT("(a refused one keeps its plain name)"), Words(Window->GetStanceButton(One, 2)->GetLabel()), Words(CosmicDiplomacy::StanceNameOf(CosmicTags::StanceHostile())));
		TestTrue(TEXT("Hostility on the first rung is greyed with its reason"), Window->GetShownStanceBlock(One, 2) == TOptional<ECosmicStanceBlock>(ECosmicStanceBlock::Tier)
			&& Words(Window->GetStanceButton(One, 2)->GetToolTipText()).StartsWith(Words(TEXT("Diplomacy.Block.Tier"))));
		const int32 Lines = State->Log.Num();
		// A greyed button takes no press, though its command would be taken.
		UCosmicActionButton* Friendly = Window->GetStanceButton(One, 0);
		Friendly->SetIsEnabled(false);
		Friendly->Press();
		TestTrue(TEXT("A press on a greyed button does nothing"), CosmicDiplomacy::StanceOf(Civs[0], One) != CosmicTags::StanceFriendly() && State->Log.Num() == Lines);
		Friendly->SetIsEnabled(true);
		Friendly->Press();
		TestTrue(TEXT("A press on [Friendly] turns the people friendly"), CosmicDiplomacy::StanceOf(Civs[0], One) == CosmicTags::StanceFriendly());
		TestEqual(TEXT("with one line"), State->Log.Num(), Lines + 1);
		TestTrue(TEXT("and the row follows: friendly is now the stance held"), Window->GetShownStanceBlock(One, 0) == TOptional<ECosmicStanceBlock>(ECosmicStanceBlock::Same));
		const FString HeldTip = Words(Window->GetStanceButton(One, 0)->GetToolTipText());
		TestTrue(TEXT("whose tooltip keeps its upkeep in sight"), HeldTip.StartsWith(Words(TEXT("Diplomacy.TipNow")) + TEXT("\n")) && HeldTip.Contains(Lead(TEXT("Diplomacy.TipFriendly"))));
		TestTrue(TEXT("and the slot is taken for people 2"), Window->GetShownStanceBlock(Two, 0) == TOptional<ECosmicStanceBlock>(ECosmicStanceBlock::FriendlySlots));
		TestEqual(TEXT("The head counts the slot"), Words(Window->GetSlotsText()->GetText()),
			Words(FText::Format(Label(TEXT("Diplomacy.Slots")), FText::AsNumber(1), FText::AsNumber(1))));
		const int32 Before = State->Log.Num();
		Window->GetStanceButton(Two, 0)->Press();
		TestEqual(TEXT("A press on a button the gate greys does nothing"), State->Log.Num(), Before);
	}

	// The treaty buttons: the first rung refuses; from the second, the gate's reasons, the cost and the bar, a press signs
	// and the button then breaks.
	{
		TestTrue(TEXT("A treaty on the first rung is greyed (Tier)"), Window->GetShownTreatyBlock(One, 0) == TOptional<ECosmicTreatyBlock>(ECosmicTreatyBlock::Tier)
			&& !Window->GetTreatyButton(One, 0)->GetIsEnabled());
		Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		Window->Refresh();
		for (const FName Other : { One, Two })
		{
			for (int32 At = 0; At < 2; ++At)
			{
				const ECosmicTreatyBlock Gate = CosmicDiplomacy::TreatyBlockOf(*State, Index, Sim->GetTables(), Rules, 0, Other, UCosmicEdictWidget::TreatyAt(At), true);
				TestTrue(*FString::Printf(TEXT("Treaty %d of %s greys by the command's gate (none here)"), At, *Other.ToString()),
					Window->GetShownTreatyBlock(Other, At) == TOptional<ECosmicTreatyBlock>(Gate) && Gate == ECosmicTreatyBlock::None);
				TestFalse(TEXT("(its tooltip filled)"), HasPlaceholder(Window->GetTreatyButton(Other, At)->GetToolTipText()));
			}
		}
		const FString TradeTip = Words(Window->GetTreatyButton(One, 0)->GetToolTipText());
		TestTrue(TEXT("The treaty's tooltip says its cost"), TradeTip.Contains(Words(FText::Format(Label(TEXT("Diplomacy.TipCost")), UCosmicEdictWidget::AmountOf(50.f)))));
		TestTrue(TEXT("and their view against the bar"), TradeTip.Contains(Words(FText::Format(Label(TEXT("Diplomacy.TipBar")), UCosmicEdictWidget::AmountOf(0.f),
			UCosmicEdictWidget::AmountOf(-0.1f)))));
		Civs[0].ActiveEdicts = { AccordRow };
		Window->Refresh();
		TestTrue(TEXT("An edict that eases the treaties lowers the bar it tells (-0.1 - 0.15)"), Words(Window->GetTreatyButton(One, 0)->GetToolTipText()).Contains(
			Words(FText::Format(Label(TEXT("Diplomacy.TipBar")), UCosmicEdictWidget::AmountOf(0.f), UCosmicEdictWidget::AmountOf(-0.25f)))));
		// A bar that is no tenth: a view that reaches it never reads below it (-0.22 cut down would read -0.3).
		Civs[1].Relations.Add(Zero, -0.22f);
		Window->Refresh();
		TestTrue(TEXT("A view at or over a bar that is no tenth reads no lower than the bar"), Window->GetTreatyButton(One, 0)->GetIsEnabled()
			&& Words(Window->GetTreatyButton(One, 0)->GetToolTipText()).Contains(Words(FText::Format(Label(TEXT("Diplomacy.TipBar")),
				UCosmicEdictWidget::AmountOf(-0.25f), UCosmicEdictWidget::AmountOf(-0.25f)))));
		Civs[1].Relations.Reset();
		Civs[0].ActiveEdicts.Reset();
		// Below the bar: refused, the view cut down to a tenth (-0.14 reads -0.2, under the bar -0.1).
		Civs[1].Relations.Add(Zero, -0.14f);
		Window->Refresh();
		const FString Refused = Words(Window->GetTreatyButton(One, 0)->GetToolTipText());
		TestTrue(TEXT("A view below the bar refuses, its reason first"), Window->GetShownTreatyBlock(One, 0) == TOptional<ECosmicTreatyBlock>(ECosmicTreatyBlock::Refused)
			&& !Window->GetTreatyButton(One, 0)->GetIsEnabled() && Refused.StartsWith(Words(TEXT("Diplomacy.Treaty.Block.Refused"))));
		TestTrue(TEXT("and the view beside the bar is cut down, never at the bar"), Refused.Contains(Words(FText::Format(Label(TEXT("Diplomacy.TipBar")),
			UCosmicEdictWidget::AmountOf(-0.2f), UCosmicEdictWidget::AmountOf(-0.1f)))));
		Civs[1].Relations.Reset();
		CosmicDiplomacy::SetStanceOf(Civs[1], Zero, CosmicTags::StanceHostile());
		Window->Refresh();
		TestTrue(TEXT("Their hostility refuses (Hostile)"), Window->GetShownTreatyBlock(One, 1) == TOptional<ECosmicTreatyBlock>(ECosmicTreatyBlock::Hostile)
			&& Words(Window->GetTreatyButton(One, 1)->GetToolTipText()).StartsWith(Words(TEXT("Diplomacy.Treaty.Block.Hostile"))));
		CosmicDiplomacy::SetStanceOf(Civs[1], Zero, FGameplayTag());
		Civs[0].Stockpile.Influence = 10.f;
		Window->Refresh();
		TestTrue(TEXT("Too little influence refuses (Cost)"), Window->GetShownTreatyBlock(One, 0) == TOptional<ECosmicTreatyBlock>(ECosmicTreatyBlock::Cost)
			&& Words(Window->GetTreatyButton(One, 0)->GetToolTipText()).StartsWith(Words(TEXT("Diplomacy.Treaty.Block.Cost"))));
		Civs[0].Stockpile.Influence = 100.f;
		Window->Refresh();
		const int32 Lines = State->Log.Num();
		Window->GetTreatyButton(One, 1)->Press();
		TestTrue(TEXT("A press on [Non-aggression] signs it"), CosmicDiplomacy::HasTreaty(Civs[0], One, CosmicTags::TreatyNonAggression()) && State->Log.Num() == Lines + 1);
		TestFalse(TEXT("and the button now breaks it (no gate shown)"), Window->GetShownTreatyBlock(One, 1).IsSet());
		TestEqual(TEXT("its label says so"), Words(Window->GetTreatyButton(One, 1)->GetLabel()), Words(TEXT("Diplomacy.Break.NonAggression")));
		// The second press of a double-click on the button that turned in place: Slate's double-click is handled, no press.
		{
			const UButton* Inner = Cast<UButton>(Window->GetTreatyButton(One, 1)->WidgetTree->RootWidget);
			TSharedPtr<SButton> Slate = Inner ? StaticCastSharedPtr<SButton>(Inner->GetCachedWidget()) : TSharedPtr<SButton>();
			if (TestTrue(TEXT("(the button's Slate button is built)"), Slate.IsValid()))
			{
				const FPointerEvent Double(0, FVector2D::ZeroVector, FVector2D::ZeroVector, TSet<FKey>({ EKeys::LeftMouseButton }), EKeys::LeftMouseButton, 0.f,
					FModifierKeysState());
				const FReply Reply = Slate->OnMouseButtonDoubleClick(FGeometry(), Double);
				TestTrue(TEXT("A double-click's second press is handled and presses nothing"), Reply.IsEventHandled() && !Slate->IsPressed());
				TestTrue(TEXT("(the treaty still stands)"), CosmicDiplomacy::HasTreaty(Civs[0], One, CosmicTags::TreatyNonAggression()));
			}
		}
		TestTrue(TEXT("its tooltip the fall a bordering people's view takes"), Words(Window->GetTreatyButton(One, 1)->GetToolTipText()).Contains(
			Words(FText::Format(Label(TEXT("Diplomacy.TipBreak")), UCosmicEdictWidget::AmountOf(0.3f)))));
		TestTrue(TEXT("Hostility under it is greyed (NonAggression)"), Window->GetShownStanceBlock(One, 2) == TOptional<ECosmicStanceBlock>(ECosmicStanceBlock::NonAggression));
		Window->GetTreatyButton(One, 1)->Press();
		TestFalse(TEXT("A second press breaks it"), CosmicDiplomacy::HasTreaty(Civs[0], One, CosmicTags::TreatyNonAggression()));
		Window->GetTreatyButton(Two, 0)->Press();
		TestTrue(TEXT("A far people takes [Trade] too"), CosmicDiplomacy::HasTreaty(Civs[0], Two, CosmicTags::TreatyTrade()));
		TestTrue(TEXT("and its [Break trade] says the treaty's own share leaves their view (no stored view far away)"),
			Words(Window->GetTreatyButton(Two, 0)->GetToolTipText()).Contains(Words(FText::Format(Label(TEXT("Diplomacy.TipBreakFar")), UCosmicEdictWidget::AmountOf(0.2f)))));
		TestTrue(TEXT("and its [Hostile] says the betrayal takes that share"), Words(Window->GetStanceButton(Two, 2)->GetToolTipText()).Contains(
			Words(FText::Format(Label(TEXT("Diplomacy.TipBetrayFar")), UCosmicEdictWidget::AmountOf(0.2f)))));
		Window->GetTreatyButton(Two, 0)->Press();
		TestFalse(TEXT("and breaks it"), CosmicDiplomacy::HasTreaty(Civs[0], Two, CosmicTags::TreatyTrade()));
		// Hostility with a trade treaty held: the betrayal's fall in its tooltip.
		CosmicDiplomacy::Sign(*State, 0, 1, CosmicTags::TreatyTrade());
		Window->Refresh();
		TestTrue(TEXT("[Hostile] over a trade treaty says the betrayal's fall"), Words(Window->GetStanceButton(One, 2)->GetToolTipText()).Contains(
			Words(FText::Format(Label(TEXT("Diplomacy.TipBetray")), UCosmicEdictWidget::AmountOf(0.3f)))));
		CosmicDiplomacy::Break(*State, 0, 1, CosmicTags::TreatyTrade());
		Civs[0].Stockpile.Influence = 100.f;
		Window->Refresh();
	}

	// The fleet's head: power and cap, the target, [Withdraw the fleet]; a siege between us in the row.
	{
		Civs[0].FleetPower = 1.5f;
		Window->Refresh();
		TestEqual(TEXT("The head says the fleet and its cap"), Words(Window->GetFleetText()->GetText()),
			Words(FText::Format(Label(TEXT("Siege.Fleet")), FText::AsNumber(1.5f), FText::AsNumber(2))));
		TestEqual(TEXT("with no target, an even guard"), Words(Window->GetTargetText()->GetText()), Words(TEXT("Guard.Even")));
		TestFalse(TEXT("and no [Withdraw the fleet]"), Shown(Window->GetWithdrawButton()));
		Civs[0].FleetTargetSystemID = IDOf(*State, 1);
		Window->Refresh();
		TestEqual(TEXT("A guard names its system"), Words(Window->GetTargetText()->GetText()),
			Words(FText::FormatNamed(Label(TEXT("Guard.Line")), TEXT("System"), State->Systems[1].DisplayName)));
		TestTrue(TEXT("and [Withdraw the fleet] shows, saying a guard ends"), Shown(Window->GetWithdrawButton())
			&& Words(Window->GetWithdrawButton()->GetToolTipText()) == Words(TEXT("Guard.WithdrawTip")));
		Window->GetWithdrawButton()->Press();
		TestTrue(TEXT("A press withdraws it"), Civs[0].FleetTargetSystemID.IsNone());
		// Sieges begun 50 and 100 ticks ago (of 200): 150 and 100 left.
		const int64 TickBefore = State->Tick;
		State->Tick = 1000;
		CosmicDiplomacy::SetStanceOf(Civs[0], One, CosmicTags::StanceHostile());
		Civs[0].FleetTargetSystemID = IDOf(*State, 2);
		Civs[0].FleetTargetSinceTick = 950;
		Window->Refresh();
		auto SiegeText = [&](const TCHAR* Key, int32 Attacker, int32 SystemIndex, bool bOurs, int64 Left)
		{
			const float Odds = CosmicConflict::SiegeOddsOf(*State, Index, Rules, Attacker, SystemIndex);
			FFormatNamedArguments Args;
			Args.Add(TEXT("System"), State->Systems[SystemIndex].DisplayName);
			Args.Add(TEXT("Time"), CosmicUI::DurationLabel(Left, Unit));
			Args.Add(TEXT("Band"), UCosmicEdictWidget::OddsWordOf(bOurs ? Odds : 1.f - Odds));
			return FText::Format(Label(Key), Args);
		};
		{
			FCivilizationState Late;
			Late.FleetTargetSinceTick = 700;
			TestEqual(TEXT("A siege past its end has 0 left (not below)"), UCosmicEdictWidget::SiegeTicksLeftOf(Late, 1000, Rules), static_cast<int64>(0));
		}
		TestEqual(TEXT("(our odds on their system 2: 1.5 against 1 x 0.5 - ahead)"), Words(UCosmicEdictWidget::OddsWordOf(CosmicConflict::SiegeOddsOf(*State, Index, Rules, 0, 2))),
			Words(TEXT("Siege.Band.Ahead")));
		TestEqual(TEXT("Our siege of their system is a line of their row, with its length left and our band"), Words(Window->GetPeopleSiege(One)->GetText()),
			Words(SiegeText(TEXT("Siege.Row.Ours"), 0, 2, true, 150)));
		TestTrue(TEXT("(shown)"), Shown(Window->GetPeopleSiege(One)));
		TestFalse(TEXT("not of another people's row"), Shown(Window->GetPeopleSiege(Two)));
		TestEqual(TEXT("and the head says the siege"), Words(Window->GetTargetText()->GetText()), Words(SiegeText(TEXT("Siege.Line"), 0, 2, true, 150)));
		TestFalse(TEXT("(the time left said once)"), Words(Window->GetTargetText()->GetText()).Contains(Words(TEXT("Panel.CyclesLeft"))));
		TestEqual(TEXT("[Withdraw the fleet] says a siege ends with its count"), Words(Window->GetWithdrawButton()->GetToolTipText()), Words(TEXT("Siege.WithdrawTip")));
		const FString LiftsWords = Words(FText::FormatNamed(Label(TEXT("Siege.Lifts")), TEXT("System"), State->Systems[2].DisplayName));
		TestTrue(TEXT("[Neutral] and [Friendly] say our siege of them would lift"), Words(Window->GetStanceButton(One, 1)->GetToolTipText()).Contains(LiftsWords)
			&& Words(Window->GetStanceButton(One, 0)->GetToolTipText()).Contains(LiftsWords));
		TestFalse(TEXT("(not another people's)"), Words(Window->GetStanceButton(Two, 1)->GetToolTipText()).Contains(LiftsWords));
		// Theirs on our system 1 too: our side's band flipped (1 x 1 against 1.5 x 0.25: we are behind).
		Civs[1].FleetTargetSystemID = IDOf(*State, 1);
		Civs[1].FleetTargetSinceTick = 900;
		Window->Refresh();
		TestEqual(TEXT("(our side of theirs: behind)"), Words(UCosmicEdictWidget::OddsWordOf(1.f - CosmicConflict::SiegeOddsOf(*State, Index, Rules, 1, 1))), Words(TEXT("Siege.Band.Behind")));
		TestEqual(TEXT("Their siege of ours is the row's second line, our side's band flipped"), Words(Window->GetPeopleSiege(One)->GetText()),
			Words(CosmicUI::JoinLines({ SiegeText(TEXT("Siege.Row.Ours"), 0, 2, true, 150), SiegeText(TEXT("Siege.Row.Theirs"), 1, 1, false, 100) })));
		Civs[1].FleetTargetSystemID = NAME_None;
		Civs[0].FleetTargetSystemID = NAME_None;
		CosmicDiplomacy::SetStanceOf(Civs[0], One, FGameplayTag());
		State->Tick = TickBefore;
		Window->Refresh();
	}

	// The panel on our settled system: [Guard here] by the guard's gate; a guard's lines; a siege on us.
	{
		Civs[0].FleetPower = 3.f;
		const int64 TickBefore = State->Tick;
		State->Tick = 1000;
		Controller->SelectSystem(IDOf(*State, 1));
		HUD->RefreshPanel();
		UCosmicActionButton* Guard = FleetButton(*Panel, UCosmicSystemPanelWidget::GuardAction);
		TestTrue(TEXT("Our settled system offers [Guard here]"), Guard && Guard->GetIsEnabled() && Words(Guard->GetLabel()) == Words(TEXT("Guard.Here")));
		TestTrue(TEXT("by the command's gate"), Panel->GetShownFleetBlock() == TOptional<ECosmicSiegeBlock>(ECosmicSiegeBlock::None));
		TestTrue(TEXT("in a box of its own (the project buttons stay the projects')"), !Find(Panel->GetActionButtons(), UCosmicSystemPanelWidget::GuardAction));
		// A siege elsewhere: the guard's tooltip says it would lift.
		CosmicDiplomacy::SetStanceOf(Civs[0], One, CosmicTags::StanceHostile());
		Civs[0].FleetTargetSystemID = IDOf(*State, 2);
		HUD->RefreshPanel();
		Guard = FleetButton(*Panel, UCosmicSystemPanelWidget::GuardAction);
		TestTrue(TEXT("[Guard here] while besieging says the siege would lift"), Guard && Words(Guard->GetToolTipText()).Contains(
			Words(FText::FormatNamed(Label(TEXT("Siege.Lifts")), TEXT("System"), State->Systems[2].DisplayName))));
		Civs[0].FleetTargetSystemID = NAME_None;
		CosmicDiplomacy::SetStanceOf(Civs[0], One, FGameplayTag());
		HUD->RefreshPanel();
		Guard = FleetButton(*Panel, UCosmicSystemPanelWidget::GuardAction);
		if (Guard)
		{
			Guard->Press();
		}
		TestTrue(TEXT("A press guards it"), Civs[0].FleetTargetSystemID == IDOf(*State, 1));
		HUD->RefreshPanel();
		const FString FleetWords = Words(Panel->GetFleetText()->GetText());
		TestTrue(TEXT("The panel says the fleet guards it"), FleetWords.StartsWith(Words(TEXT("Guard.Holding"))));
		TestTrue(TEXT("and that a new hazard there gains time (50 x 3 is a whole tick)"), FleetWords.Contains(Words(TEXT("Guard.Delay")))
			&& Words(Panel->GetFleetText()->GetToolTipText()).Contains(Words(TEXT("Guard.DelayTip"))));
		TestFalse(TEXT("and offers no [Guard here]"), FleetButton(*Panel, UCosmicSystemPanelWidget::GuardAction) != nullptr);
		// People 1 besieges it: named while shown, our side's band read as the defender (1 against 3 x 1: we are ahead).
		Civs[1].FleetTargetSystemID = IDOf(*State, 1);
		Civs[1].FleetTargetSinceTick = 900;
		HUD->RefreshPanel();
		auto OnUsText = [&](const TCHAR* Key)
		{
			FFormatNamedArguments Args;
			Args.Add(TEXT("OtherCiv"), Civs[1].CivName);
			Args.Add(TEXT("Time"), CosmicUI::DurationLabel(100, Unit));
			Args.Add(TEXT("Band"), UCosmicEdictWidget::OddsWordOf(1.f - CosmicConflict::SiegeOddsOf(*State, Index, Rules, 1, 1)));
			return Words(FText::Format(Label(Key), Args));
		};
		TestEqual(TEXT("(our side: ahead)"), Words(UCosmicEdictWidget::OddsWordOf(1.f - CosmicConflict::SiegeOddsOf(*State, Index, Rules, 1, 1))), Words(TEXT("Siege.Band.Ahead")));
		TestTrue(TEXT("A siege on us names the besieger, the length left and our side's band"), Words(Panel->GetFleetText()->GetText()).EndsWith(TEXT("\n") + OnUsText(TEXT("Siege.OnUs"))));
		State->Knowing.MetCivs.RemoveAll([One](const FKnownMark& Mark) { return Mark.ID == One; });
		HUD->RefreshPanel();
		TestFalse(TEXT("A besieger not met has no name"), Words(Panel->GetFleetText()->GetText()).Contains(Words(Civs[1].CivName)));
		TestTrue(TEXT("(the nameless line)"), Words(Panel->GetFleetText()->GetText()).EndsWith(TEXT("\n") + OnUsText(TEXT("Siege.OnUsUnknown"))));
		MeetAgain(*State);
		// Met, but three rungs above the viewer: no row, and nameless.
		Civs[1].Tier = EAscensionTier::Tier5_EternalOrder;
		HUD->RefreshPanel();
		Window->Refresh();
		TestTrue(TEXT("A people met but three rungs above is no row"), Window->GetShownPeoples() == TArray<FName>({ Two }));
		TestTrue(TEXT("and its siege on us has no name"), Words(Panel->GetFleetText()->GetText()).EndsWith(TEXT("\n") + OnUsText(TEXT("Siege.OnUsUnknown"))));
		Controller->SelectSystem(IDOf(*State, 2));
		HUD->RefreshPanel();
		TestTrue(TEXT("and its system offers no [Send the fleet] (a met owner not shown)"), Panel->GetFleetButtons().IsEmpty() && !Panel->GetShownFleetBlock().IsSet()
			&& !Shown(Panel->GetFleetText()));
		Controller->SelectSystem(IDOf(*State, 1));
		Civs[1].Tier = EAscensionTier::Tier2_Stellar;
		Civs[1].FleetTargetSystemID = NAME_None;
		// A small fleet: the guard stands, but a new hazard gains no whole tick - the line follows the guard's own delay.
		Civs[0].FleetPower = 0.01f;
		HUD->RefreshPanel();
		TestTrue(TEXT("A fleet of 0.01 still guards"), Words(Panel->GetFleetText()->GetText()) == Words(TEXT("Guard.Holding")));
		TestFalse(TEXT("but says no delay (50 x 0.01 is no whole tick)"), Words(Panel->GetFleetText()->GetToolTipText()).Contains(Words(TEXT("Guard.DelayTip"))));
		for (const float Power : { 0.019f, 0.02f })
		{
			Civs[0].FleetPower = Power;
			HUD->RefreshPanel();
			const bool bDelay = Words(Panel->GetFleetText()->GetText()).Contains(Words(TEXT("Guard.Delay")));
			TestEqual(*FString::Printf(TEXT("At %.3f the delay line stands exactly while the guard's delay is a whole tick"), Power), bDelay,
				CosmicConflict::GuardDelayTicksOf(Civs[0], 0, Rules) >= 1);
			TestEqual(*FString::Printf(TEXT("(at %.3f: %s)"), Power, Power > 0.0195f ? TEXT("50 x 0.02 is one tick") : TEXT("50 x 0.019 is none")), bDelay, Power > 0.0195f);
		}
		Civs[0].FleetPower = 3.f;
		Civs[0].FleetTargetSystemID = NAME_None;
		// The first rung has no fleet: [Guard here] greyed with its reason, short and long.
		Civs[0].Tier = EAscensionTier::Tier1_Planetary;
		HUD->RefreshPanel();
		UCosmicActionButton* Greyed = FleetButton(*Panel, UCosmicSystemPanelWidget::GuardAction);
		TestTrue(TEXT("On the first rung [Guard here] is greyed"), Greyed && !Greyed->GetIsEnabled() && Panel->GetShownFleetBlock() == TOptional<ECosmicSiegeBlock>(ECosmicSiegeBlock::NoFleet));
		TestTrue(TEXT("its short reason after the name"), Greyed
			&& Words(Greyed->GetLabel()) == Words(FText::Format(INVTEXT("{0}  {1}"), CosmicUI::Label(TEXT("Guard.Here")), CosmicUI::FleetSuffix(Sim->GetRules(), Civs[0].Tier))));
		// The short words name the fleet's first rung as the edict window names an edict's (B4): the rules' first rung above 0.
		{
			const FString RungTwo = FText::Format(CosmicUI::Label(TEXT("Guard.Suffix.NoFleet")), FText::AsNumber(2)).ToString();
			TestEqual(TEXT("(the board's fleet comes at the second rung)"), CosmicUI::FleetSuffix(Sim->GetRules(), Civs[0].Tier).ToString(), RungTwo);
			TestTrue(TEXT("(and the words carry the rung as a number)"), RungTwo.Contains(TEXT("2")) && !RungTwo.Contains(TEXT("{0}")));
			FCosmicRulesDef Later = Sim->GetRules();
			Later.FleetPerSettledByTier = { 0.f, 0.f, -1.f, 0.5f };
			TestEqual(TEXT("A fleet from the fourth rung: its suffix says the fourth (a negative value is no fleet)"), CosmicUI::FleetSuffix(Later, EAscensionTier::Tier1_Planetary).ToString(),
				FText::Format(CosmicUI::Label(TEXT("Guard.Suffix.NoFleet")), FText::AsNumber(4)).ToString());
			Later.FleetPerSettledByTier = { 0.f, std::numeric_limits<float>::quiet_NaN() };
			TestTrue(TEXT("No rung with a fleet: no suffix"), CosmicUI::FleetSuffix(Later, EAscensionTier::Tier1_Planetary).IsEmpty());
			Later.FleetPerSettledByTier = { 0.f, 1.f, 0.f, 1.f, 1.f };
			TestEqual(TEXT("A fleet that skips the third rung: from the third the suffix says the fourth"), CosmicUI::FleetSuffix(Later, EAscensionTier::Tier3_Causal).ToString(),
				FText::Format(CosmicUI::Label(TEXT("Guard.Suffix.NoFleet")), FText::AsNumber(4)).ToString());
			TestEqual(TEXT("and from the first it says the second"), CosmicUI::FleetSuffix(Later, EAscensionTier::Tier1_Planetary).ToString(), RungTwo);
		}
		TestTrue(TEXT("saying there is no fleet yet"), Greyed && Words(Greyed->GetToolTipText()).StartsWith(Words(TEXT("Guard.Block.NoFleet"))));
		Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		State->Tick = TickBefore;
	}

	// The panel on another people's system: [Send the fleet] by the siege's gate; our siege's line; a third people's siege
	// untold; an owner not shown offered nothing.
	{
		Controller->SelectSystem(IDOf(*State, 2));
		HUD->RefreshPanel();
		UCosmicActionButton* Send = FleetButton(*Panel, UCosmicSystemPanelWidget::SiegeAction);
		const ECosmicSiegeBlock Gate = CosmicConflict::SiegeBlockOf(*State, Index, Rules, 0, 2);
		TestTrue(TEXT("Their system offers [Send the fleet] greyed by the command's gate (not hostile)"), Send && !Send->GetIsEnabled()
			&& Gate == ECosmicSiegeBlock::NotHostile && Panel->GetShownFleetBlock() == TOptional<ECosmicSiegeBlock>(Gate));
		TestTrue(TEXT("its short reason after the name"), Send && Words(Send->GetLabel()) == WithSuffix(TEXT("Siege.Send"), TEXT("Siege.Suffix.NotHostile")));
		const FString SendWhat = Words(FText::Format(Label(TEXT("Siege.SendTip")), CosmicUI::DurationLabel(200, Unit), CosmicUI::Percent(0.5f), UCosmicEdictWidget::AmountOf(0.2f)));
		TestTrue(TEXT("its reason first in the tooltip, then what a siege costs"), Send && Words(Send->GetToolTipText()).StartsWith(Words(TEXT("Siege.Block.NotHostile")))
			&& Words(Send->GetToolTipText()).Contains(SendWhat));
		TestFalse(TEXT("but no band on a greyed button"), Send && Words(Send->GetToolTipText()).Contains(Lead(TEXT("Siege.SendOdds"))));
		CosmicDiplomacy::SetStanceOf(Civs[0], One, CosmicTags::StanceHostile());
		Civs[0].FleetTargetSystemID = IDOf(*State, 1);
		HUD->RefreshPanel();
		Send = FleetButton(*Panel, UCosmicSystemPanelWidget::SiegeAction);
		TestTrue(TEXT("Hostile, it is pressable"), Send && Send->GetIsEnabled() && Words(Send->GetLabel()) == Words(TEXT("Siege.Send")));
		TestTrue(TEXT("its tooltip with our side's band"), Send && Words(Send->GetToolTipText()).Contains(Words(FText::Format(Label(TEXT("Siege.SendOdds")),
			UCosmicEdictWidget::OddsWordOf(CosmicConflict::SiegeOddsOf(*State, Index, Rules, 0, 2))))));
		TestTrue(TEXT("and, while the fleet guards elsewhere, that it would leave"), Send && Words(Send->GetToolTipText()).Contains(
			Words(FText::FormatNamed(Label(TEXT("Guard.Moves")), TEXT("System"), State->Systems[1].DisplayName))));
		if (Send)
		{
			Send->Press();
		}
		TestTrue(TEXT("A press lays the siege"), Civs[0].FleetTargetSystemID == IDOf(*State, 2) && Civs[0].FleetTargetSinceTick == State->Tick);
		Civs[0].FleetTargetSinceTick = State->Tick - 50;
		HUD->RefreshPanel();
		FFormatNamedArguments Args;
		Args.Add(TEXT("OtherCiv"), Civs[0].CivName);
		Args.Add(TEXT("Time"), CosmicUI::DurationLabel(150, Unit));
		Args.Add(TEXT("Band"), UCosmicEdictWidget::OddsWordOf(CosmicConflict::SiegeOddsOf(*State, Index, Rules, 0, 2)));
		TestEqual(TEXT("Our siege's line with its length left and our side's band"), Words(Panel->GetFleetText()->GetText()), Words(FText::Format(Label(TEXT("Siege.Ours")), Args)));
		TestTrue(TEXT("(shown)"), Shown(Panel->GetFleetText()));
		TestFalse(TEXT("and no second [Send the fleet]"), FleetButton(*Panel, UCosmicSystemPanelWidget::SiegeAction) != nullptr);
		Civs[0].FleetTargetSystemID = NAME_None;
		// The owner two rungs above: a fleet that cannot reach them, said as such.
		Civs[1].Tier = EAscensionTier::Tier4_Transcendent;
		HUD->RefreshPanel();
		Send = FleetButton(*Panel, UCosmicSystemPanelWidget::SiegeAction);
		TestTrue(TEXT("An owner two rungs above greys it (NoFleet, though the fleet is there)"), Send && !Send->GetIsEnabled()
			&& Panel->GetShownFleetBlock() == TOptional<ECosmicSiegeBlock>(ECosmicSiegeBlock::NoFleet));
		TestTrue(TEXT("saying the fleet does not reach them"), Send && Words(Send->GetToolTipText()).StartsWith(Words(TEXT("Siege.Block.Above")))
			&& Words(Send->GetLabel()) == WithSuffix(TEXT("Siege.Send"), TEXT("Siege.Suffix.Above")));
		Civs[1].Tier = EAscensionTier::Tier2_Stellar;
		// People 2 besieges that system: our [Send the fleet] says another siege stands; no line of theirs.
		Civs[2].FleetTargetSystemID = IDOf(*State, 2);
		HUD->RefreshPanel();
		Send = FleetButton(*Panel, UCosmicSystemPanelWidget::SiegeAction);
		TestTrue(TEXT("Another people's siege greys it (Besieged)"), Send && Panel->GetShownFleetBlock() == TOptional<ECosmicSiegeBlock>(ECosmicSiegeBlock::Besieged)
			&& Words(Send->GetToolTipText()).StartsWith(Words(TEXT("Siege.Block.Besieged"))));
		TestFalse(TEXT("A siege between two other peoples has no line"), Shown(Panel->GetFleetText()));
		Controller->SelectSystem(IDOf(*State, 4));
		Civs[2].FleetTargetSystemID = IDOf(*State, 4);
		HUD->RefreshPanel();
		TestFalse(TEXT("(nor on a system out of our reach)"), Shown(Panel->GetFleetText()));
		Civs[2].FleetTargetSystemID = NAME_None;
		// People 3 is not met: its system offers no fleet button and no line.
		Controller->SelectSystem(IDOf(*State, 8));
		HUD->RefreshPanel();
		TestTrue(TEXT("An owner the viewer is not shown is offered no [Send the fleet]"), Panel->GetFleetButtons().IsEmpty() && !Panel->GetShownFleetBlock().IsSet()
			&& !Shown(Panel->GetFleetText()));
		CosmicDiplomacy::SetStanceOf(Civs[0], One, FGameplayTag());
	}

	// The end of the run: the four commands refused (each would be taken before it), no window, the panel's fleet buttons
	// greyed with the end's words.
	{
		CosmicDiplomacy::Sign(*State, 0, 2, CosmicTags::TreatyNonAggression());
		CosmicDiplomacy::SetStanceOf(Civs[0], One, CosmicTags::StanceHostile());
		Civs[0].FleetTargetSystemID = NAME_None;
		Civs[0].Stockpile.Influence = 100.f;
		const int64 WasTick = State->Tick;
		// The end laid by hand with the record its tick would have written (B6a [B-6a3c]: the latch reads the record).
		CosmicTests::EndRunAt(*State, CosmicTags::EndingClock(), Sim->GetRules().ProvisionalEndTick);
		Controller->FollowEnd(false);
		TestTrue(TEXT("(the end latched)"), Controller->IsRunEnded());
		Controller->SelectSystem(IDOf(*State, 1));
		HUD->RefreshPanel();
		const int32 Lines = State->Log.Num();
		TestFalse(TEXT("After the end the controller refuses a stance"), Controller->SetStance(Two, CosmicTags::StanceFriendly()));
		TestFalse(TEXT("a treaty"), Controller->ProposeTreaty(Two, CosmicTags::TreatyTrade()));
		TestFalse(TEXT("a break"), Controller->BreakTreaty(Two, CosmicTags::TreatyNonAggression()));
		TestFalse(TEXT("and the fleet"), Controller->AllocateFleet(IDOf(*State, 2)));
		TestTrue(TEXT("(nothing changed, no line)"), State->Log.Num() == Lines && CosmicDiplomacy::StanceOf(Civs[0], Two) != CosmicTags::StanceFriendly()
			&& !CosmicDiplomacy::HasTreaty(Civs[0], Two, CosmicTags::TreatyTrade()) && CosmicDiplomacy::HasTreaty(Civs[0], Two, CosmicTags::TreatyNonAggression())
			&& Civs[0].FleetTargetSystemID.IsNone());
		Window->Open(ECosmicEdictTab::Peoples);
		TestFalse(TEXT("The window will not open"), Window->IsOpen());
		UCosmicActionButton* Guard = FleetButton(*Panel, UCosmicSystemPanelWidget::GuardAction);
		TestTrue(TEXT("[Guard here] is greyed with the end's words"), Guard && !Guard->GetIsEnabled() && Words(Guard->GetLabel()) == WithSuffix(TEXT("Guard.Here"), TEXT("Chronicle.Suffix"))
			&& Words(Guard->GetToolTipText()) == Words(TEXT("Chronicle.Block")));
		Controller->SelectSystem(IDOf(*State, 2));
		HUD->RefreshPanel();
		UCosmicActionButton* Send = FleetButton(*Panel, UCosmicSystemPanelWidget::SiegeAction);
		TestTrue(TEXT("and so is [Send the fleet]"), Send && !Send->GetIsEnabled() && Words(Send->GetLabel()) == WithSuffix(TEXT("Siege.Send"), TEXT("Chronicle.Suffix"))
			&& Words(Send->GetToolTipText()) == Words(TEXT("Chronicle.Block")));
		Controller->ClearRunEnd();
		State->Tick = WasTick;
		State->Ending = FCosmicEndingRecord();
		CosmicDiplomacy::Break(*State, 0, 2, CosmicTags::TreatyNonAggression());
		CosmicDiplomacy::SetStanceOf(Civs[0], One, FGameplayTag());
		HUD->RefreshPanel();
		Window->Open(ECosmicEdictTab::Peoples);
	}

	// The first contact card (B3 header section 4): its words name the people met, and the friendly choice says what the
	// friendly stance holds.
	{
		UCosmicPetitionCardWidget* Card = HUD->GetPetitionCard();
		State->Knowing.PendingContactCivID = Two;
		State->Knowing.PendingPetitionRow = CosmicRows::PetitionFirstContact;
		State->Knowing.PendingPetitionSystemID = NAME_None;
		Controller->FollowPetition(false);
		TestTrue(TEXT("The first contact card shows, naming the people met"), Card && Card->GetShownRow() == CosmicRows::PetitionFirstContact
			&& Words(Card->GetTitleText()->GetText()).Contains(Words(Civs[2].CivName)) && !HasPlaceholder(Card->GetBodyText()->GetText()));
		if (Card && Card->GetChoiceLine(0) && Card->GetChoiceLine(1))
		{
			TestEqual(TEXT("Its friendly choice says what the friendly stance holds"), Words(Card->GetChoiceLine(0)->GetText()),
				Words(FText::Format(Label(TEXT("Petition.FriendlyUpkeep")), UCosmicEdictWidget::AmountOf(0.2f), CosmicUI::DurationLabel(1, Unit),
					FText::AsNumber(CosmicDiplomacy::FriendlyCountOf(Civs[0])), FText::AsNumber(CosmicDiplomacy::FriendlySlotsOf(Civs[0], Rules)))));
			TestFalse(TEXT("and the other choice nothing"), Shown(Card->GetChoiceLine(1)));
			// The line reads the rule, and an upkeep that is none reads as none.
			const FCosmicDataTables Laid = Sim->GetTables();
			FCosmicRulesDef Dearer = Rules;
			for (const float Upkeep : { 0.35f, -1.f })
			{
				Dearer.FriendlyUpkeepInfluence = Upkeep;
				Sim->SetTables(CosmicTests::WithRules(Laid, Dearer));
				Card->Refresh();
				TestEqual(FString::Printf(TEXT("The friendly line reads the rule's upkeep (%.2f)"), Upkeep), Words(Card->GetChoiceLine(0)->GetText()),
					Words(FText::Format(Label(TEXT("Petition.FriendlyUpkeep")), UCosmicEdictWidget::AmountOf(FMath::Max(Upkeep, 0.f)), CosmicUI::DurationLabel(1, Unit),
						FText::AsNumber(CosmicDiplomacy::FriendlyCountOf(Civs[0])), FText::AsNumber(CosmicDiplomacy::FriendlySlotsOf(Civs[0], Sim->GetRules())))));
			}
			Sim->SetTables(Laid);
			// With the only friendly slot taken, [Reach out] is greyed with the peoples' tab's reason.
			CosmicDiplomacy::SetStanceOf(Civs[0], One, CosmicTags::StanceFriendly());
			Card->Refresh();
			const UCosmicActionButton* Reach = Card->GetChoiceButton(0);
			TestTrue(TEXT("With the friendly slot taken, reaching out is greyed with the gate's reason"), Reach && !Reach->GetIsEnabled()
				&& Words(Reach->GetToolTipText()) == Words(UCosmicEdictWidget::StanceReasonOf(CosmicDiplomacy::ECosmicStanceBlock::FriendlySlots)));
			TestTrue(TEXT("and watching is not"), Card->GetChoiceButton(1) && Card->GetChoiceButton(1)->GetIsEnabled());
			// Its people removed while it shows (the crossing stage's cleanup clears the slot): every answer withdraws it, so
			// nothing is greyed; shown again (after a load) it names the people by its own words.
			State->Knowing.PendingContactCivID = NAME_None;
			Card->Refresh();
			TestTrue(TEXT("A card whose people is gone greys nothing (any answer withdraws it)"), Card->GetChoiceButton(0) && Card->GetChoiceButton(0)->GetIsEnabled());
			Card->ShowCard(CosmicRows::PetitionFirstContact, NAME_None, NAME_None);
			TestTrue(TEXT("and shown again it names the people by the card's own words"), Words(Card->GetTitleText()->GetText()).Contains(Words(TEXT("Petition.OtherGone")))
				&& !HasPlaceholder(Card->GetTitleText()->GetText()));
			State->Knowing.PendingContactCivID = Two;
			CosmicDiplomacy::SetStanceOf(Civs[0], One, FGameplayTag());
		}
		State->Knowing.PendingPetitionRow = NAME_None;
		State->Knowing.PendingContactCivID = NAME_None;
		Controller->FollowPetition(false);
	}

	// The edicts' two new lines (only where they change something) and the top bar's.
	{
		UDataTable* Edicts = CosmicTests::NewTable(FEdictDef::StaticStruct());
		FEdictDef Plain;
		Plain.DisplayName = FText::AsCultureInvariant(TEXT("plain"));
		Edicts->AddRow(FName(TEXT("edict_test_plain")), Plain);
		FCosmicDataTables Tables = Sim->GetTables();
		const FString Tip = Words(UCosmicEdictWidget::TipOf(Tables, AccordRow, ECosmicTimeUnit::Cycle));
		TestTrue(TEXT("An edict's tooltip says its fleet growth"), Tip.Contains(Words(FText::Format(Label(TEXT("Edict.Effect.FleetGrowth")), FText::AsNumber(1.5f)))));
		TestTrue(TEXT("and its treaty acceptance"), Tip.Contains(Words(FText::Format(Label(TEXT("Edict.Effect.TreatyAccept")), FText::AsNumber(0.15f)))));
		Tables.Edicts = Edicts;
		const FString PlainTip = Words(UCosmicEdictWidget::TipOf(Tables, FName(TEXT("edict_test_plain")), ECosmicTimeUnit::Cycle));
		TestFalse(TEXT("An edict of factor 1 and no bonus says neither"), PlainTip.Contains(Lead(TEXT("Edict.Effect.FleetGrowth"))) || PlainTip.Contains(Lead(TEXT("Edict.Effect.TreatyAccept"))));
		HUD->RefreshPanel();
		TestTrue(TEXT("The top bar's edict button says the peoples are there too"), Bar->GetEdictButton() && Words(Bar->GetEdictButton()->GetToolTipText()).Contains(Words(TEXT("Edict.Bar.Peoples"))));
	}

	// Nobody met: the page says so.
	{
		CosmicTests::SetKnowing(*State, {}, { 0 });
		Window->Refresh();
		TestTrue(TEXT("With nobody met the page says so"), Window->GetShownPeoples().IsEmpty() && Shown(Window->GetNoPeoplesText()));
	}
	return true;
}

#endif

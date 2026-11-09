// Cosmic Ascension - automation tests of the system panel's command reasons and of the paused mark (Presentation).
// Design: guideline.md section 5.7, 6.3, 8.3, 9.3, 11 Phase 3 (item 12).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
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
#include "Internationalization/StringTableCore.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicSeal.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicUILabels.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/IToolTip.h"
#include "Widgets/SWidget.h"
#include "World/CosmicGameMode.h"

namespace CosmicCommandReasonTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// Everything the text blocks of a widget's own tree show (shared - Tests/CosmicTestHelpers.h).
	using CosmicTests::ReadTexts;

	TArray<FName> ActionsOf(const UCosmicSystemPanelWidget& Panel)
	{
		TArray<FName> Actions;
		for (const UCosmicActionButton* Button : Panel.GetActionButtons())
		{
			Actions.Add(Button ? Button->GetAction() : NAME_None);
		}
		return Actions;
	}

	// The panel's button of Action; null when it shows none.
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

	FString TipOf(const UCosmicSystemPanelWidget& Panel, FName Action)
	{
		const UCosmicActionButton* Button = ButtonOf(Panel, Action);
		return Button ? Button->GetToolTipText().ToString() : FString();
	}

	FString Words(const TCHAR* Key)
	{
		return CosmicUI::Label(Key).ToString();
	}

	const FName Build(TEXT("project_test_build"));
	const FName Rebuild(TEXT("project_test_rebuild"));
	const FName High(TEXT("project_test_high"));
	const FName Free(TEXT("project_test_free"));
	const FName Half(TEXT("project_test_half"));
	const FName Answer(TEXT("project_test_answer"));
	const FName Second(TEXT("project_test_second"));
	const FName HighAnswer(TEXT("project_test_high_answer"));
	const FName Pricey(TEXT("project_test_pricey"));
	const FName PriceyAnswer(TEXT("project_test_pricey_answer"));
	const FName Storm(TEXT("crisis_test_storm"));
	const FName Hold(TEXT("crisis_test_hold"));

	FMegaprojectDef Project(EAscensionTier Tier, float Energy, float Matter, float Required, const TCHAR* Stem)
	{
		FMegaprojectDef Row;
		Row.DisplayName = FText::AsCultureInvariant(FString::Printf(TEXT("<%s>"), Stem));
		Row.RequiredTier = Tier;
		Row.CostPerTick.Energy = Energy;
		Row.CostPerTick.Matter = Matter;
		Row.RequiredProgress = Required;
		return Row;
	}

	// The game's hazards with a test row of the test's own: every game row but those on the test row's tag (FindHazardByTag
	// takes the first row of a tag in name order - a game row on it, the cult row of B7 for one, would stand in front
	// of the test's), then Row on Name, never drawn.
	UDataTable* HazardsWith(const FCosmicDataTables& Game, FName Name, const FHazardDef& Row)
	{
		UDataTable* Hazards = CosmicTests::NewTable(FHazardDef::StaticStruct());
		if (Game.Hazards)
		{
			for (const FName RowName : Game.GetHazardRowNames())
			{
				const FHazardDef* GameRow = Game.FindHazard(RowName);
				if (GameRow && GameRow->HazardTag != Row.HazardTag)
				{
					Hazards->AddRow(RowName, *GameRow);
				}
			}
		}
		Hazards->AddRow(Name, Row);
		return Hazards;
	}

	// A hazard row of the tests on Hazard.Cult (a tag the game's rows do not use yet; HazardsWith leaves out any that do),
	// answered by Answers, with a long countdown and no draw.
	FHazardDef TestHazard(const TCHAR* Stem, TArray<FName> Answers)
	{
		FHazardDef Row;
		Row.HazardTag = Tag(TEXT("Hazard.Cult"));
		Row.DisplayName = FText::AsCultureInvariant(FString::Printf(TEXT("<%s>"), Stem));
		Row.CountdownTicks = 500;
		Row.SpawnRule = ECosmicHazardSpawn::Draw;
		Row.SpawnChancePerTick = 0.f;
		Row.LogCategory = Tag(TEXT("Log.Crisis"));
		Row.Severity = 1;
		Row.RulerResolutions = MoveTemp(Answers);
		return Row;
	}

	// The game's tables with the test's projects - two constructions of the first rung that cost Energy 10, one of the
	// third rung (a rung other than the settle row's second), one that costs nothing, one with a cost in decimals, two
	// answers of the storm and a third of the third rung, and a copy of the game's project_settle row - and the game's
	// hazards plus the storm (Hazard.Cult), answered by the three answers and never drawn. The game's AI awakening lists no
	// answer: under it the answers have no button.
	FCosmicDataTables ReasonTables(const FCosmicDataTables& Game)
	{
		UDataTable* Projects = CosmicTests::NewTable(FMegaprojectDef::StaticStruct());
		Projects->AddRow(Build, Project(EAscensionTier::Tier1_Planetary, 10.f, 0.f, 1.0e6f, TEXT("build")));
		Projects->AddRow(Rebuild, Project(EAscensionTier::Tier1_Planetary, 10.f, 0.f, 1.0e6f, TEXT("rebuild")));
		Projects->AddRow(High, Project(EAscensionTier::Tier3_Causal, 10.f, 0.f, 1.0e6f, TEXT("high")));
		Projects->AddRow(Free, Project(EAscensionTier::Tier1_Planetary, 0.f, 0.f, 1.0e6f, TEXT("free")));
		Projects->AddRow(Half, Project(EAscensionTier::Tier1_Planetary, 1.5f, 1.f, 1.0e6f, TEXT("half")));
		Projects->AddRow(Answer, Project(EAscensionTier::Tier1_Planetary, 0.f, 5.f, 1.0e6f, TEXT("answer")));
		Projects->AddRow(Second, Project(EAscensionTier::Tier1_Planetary, 0.f, 5.f, 1.0e6f, TEXT("second")));
		Projects->AddRow(HighAnswer, Project(EAscensionTier::Tier3_Causal, 0.f, 5.f, 1.0e6f, TEXT("high_answer")));
		if (const FMegaprojectDef* Settle = Game.FindMegaproject(CosmicRows::ProjectSettle))
		{
			Projects->AddRow(CosmicRows::ProjectSettle, *Settle);
		}
		FCosmicDataTables Tables = Game;
		Tables.Megaprojects = Projects;
		Tables.Hazards = HazardsWith(Game, Storm, TestHazard(TEXT("storm"), { Answer, Second, HighAnswer }));
		return Tables;
	}

	// One tick with the broadcast gate open (the HUD's panel answers it).
	void StepBroadcast(UGalaxySimulationSubsystem& Sim)
	{
		const bool bWasBroadcasting = Sim.IsBroadcastEnabled();
		Sim.SetBroadcastEnabled(true);
		Sim.StepOnce();
		Sim.SetBroadcastEnabled(bWasBroadcasting);
	}

	// A watch stepped along readings of one slot at consecutive ticks from 1: whether it shows the mark after each.
	TArray<bool> Marks(const TArray<CosmicMegaprojects::FCosmicSlotReading>& Readings)
	{
		TArray<bool> Shown;
		FCosmicSlotWatch Watch;
		for (int32 At = 0; At < Readings.Num(); ++At)
		{
			Watch = CosmicSlotWatch::Step(Watch, Readings[At], At + 1);
			Shown.Add(Watch.bStalled);
		}
		return Shown;
	}

	CosmicMegaprojects::FCosmicSlotReading Reading(FName Row, float Progress)
	{
		CosmicMegaprojects::FCosmicSlotReading Read;
		Read.Row = Row;
		Read.Progress = Progress;
		return Read;
	}
}

// Why a project button is greyed (Phase 3 item 12, guideline 5.7, 6.3): the words of every reason, which rows belong on a
// system and in which order, the cost line of every command button (the settle button too), a pressed construction that
// stays greyed with its reason, the replacement warning, the answers under their hazard and hidden under another, the
// year unit, a tooltip that a tick does not rebuild, the game's stabilization under a supernova, the binding's hint (B6b
// decision 28: the fallen one's title in the spiritual view, none in the physical view or with none left), and the labels.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIStartReasonsTest, "Cosmic_Ascension.UI.StartReasons", CosmicTests::Flags)
bool FCosmicUIStartReasonsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicCommandReasonTests;
	using CosmicMegaprojects::ECosmicStartBlock;

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
	UCosmicSystemPanelWidget* HudPanel = HUD ? HUD->GetSystemPanel() : nullptr;
	if (!TestNotNull(TEXT("The first player controller is the game's"), Controller) || !TestNotNull(TEXT("The HUD has its system panel"), HudPanel))
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
	const FCosmicDataTables Tables = ReasonTables(GameTables);
	Sim->SetTables(Tables);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	// The whole board known (Phase 3 item 8): the wild neighbour of section 10 is unknown at tick 0, and an unknown system
	// shows no panel.
	CosmicTests::KnowAll(Sim->GetMutableState());
	Controller->SetTimeUnit(ECosmicTimeUnit::Cycle);
	const FString& Placeholder = FStringTableEntry::GetPlaceholderSourceString();
	FGalaxyState& Live = Sim->GetMutableState();
	const int32 Ruled = Sim->GetStateIndex().FindCiv(Live, Live.Player.RuledCivID);
	const int32 HomeIndex = Live.Civs.IsValidIndex(Ruled) ? Sim->GetStateIndex().FindSystem(Live, Live.Civs[Ruled].HomeSystemID) : INDEX_NONE;
	if (!TestTrue(TEXT("(the vessel rules a people with a home)"), Live.Systems.IsValidIndex(HomeIndex)))
	{
		return false;
	}
	const FName Home = Live.Systems[HomeIndex].SystemID;
	UCosmicSystemPanelWidget* Panel = CreateWidget<UCosmicSystemPanelWidget>(Controller);
	if (!TestNotNull(TEXT("The system panel can be created"), Panel))
	{
		return false;
	}
	const TSharedRef<SWidget> SlatePanel = Panel->TakeWidget();

	// ---- 1. The words of every reason ----
	{
		const ECosmicStartBlock Shown[] = { ECosmicStartBlock::RowTier, ECosmicStartBlock::NoFallen, ECosmicStartBlock::Answering, ECosmicStartBlock::ResponseBusy, ECosmicStartBlock::Running };
		int32 Showable = 0;
		for (int32 At = 0; At < static_cast<int32>(ECosmicStartBlock::Count); ++At)
		{
			const ECosmicStartBlock Block = static_cast<ECosmicStartBlock>(At);
			const FString Reason = UCosmicSystemPanelWidget::StartReasonOf(Block, Tables, High).ToString();
			bool bShows = false;
			for (const ECosmicStartBlock Each : Shown)
			{
				bShows |= Each == Block;
			}
			Showable += bShows ? 1 : 0;
			if (bShows)
			{
				TestFalse(*FString::Printf(TEXT("Reason %d has words"), At), Reason.IsEmpty());
				TestFalse(*FString::Printf(TEXT("Reason %d is no missing String Table entry"), At), Reason.Contains(Placeholder));
				TestFalse(*FString::Printf(TEXT("Reason %d leaves no argument unfilled"), At), Reason.Contains(TEXT("{")));
			}
			else
			{
				TestTrue(*FString::Printf(TEXT("Reason %d, which no button shows, has no words"), At), Reason.IsEmpty());
			}
		}
		TestEqual(TEXT("Every reason a button shows is named here"), Showable, static_cast<int32>(UE_ARRAY_COUNT(Shown)));
		TestTrue(TEXT("The rung reason names the row's rung"), UCosmicSystemPanelWidget::StartReasonOf(ECosmicStartBlock::RowTier, Tables, High).ToString().Contains(TEXT("3")));
	}

	// ---- 2. The quiet home: its constructions in lexical order, no answer ----
	// B6b [B-6b2c] (header 4): a construction row shows from the rung before its RequiredTier on - the first-rung home lists
	// no button of the third rung's High (mutation: the filter dropped); on the second rung it stands, greyed for the rung.
	Panel->ShowSystem(Home);
	{
		const TArray<FName> Expected = { Build, Free, Half, Rebuild };
		TestTrue(TEXT("The quiet first-rung home lists its constructions in lexical order and no answer"), ActionsOf(*Panel) == Expected);
		TestTrue(TEXT("A construction two rungs up has no button on a first-rung home"), Panel->GetShownStartBlock(High) == ECosmicStartBlock::Count && !ButtonOf(*Panel, High));
		TestTrue(TEXT("An answer with no hazard here has no button"), Panel->GetShownStartBlock(Answer) == ECosmicStartBlock::Count);
		TestTrue(TEXT("nor has the settle row"), Panel->GetShownStartBlock(CosmicRows::ProjectSettle) == ECosmicStartBlock::Count);
		Live.Civs[Ruled].Tier = EAscensionTier::Tier2_Stellar;
		Panel->Refresh();
		const TArray<FName> OnSecond = { Build, Free, Half, High, Rebuild };
		TestTrue(TEXT("The second-rung home lists the third rung's construction among them"), ActionsOf(*Panel) == OnSecond);
		const UCosmicActionButton* BuildButton = ButtonOf(*Panel, Build);
		const UCosmicActionButton* HighButton = ButtonOf(*Panel, High);
		if (TestNotNull(TEXT("(the home shows the build button)"), BuildButton) && TestNotNull(TEXT("(and the high one)"), HighButton))
		{
			TestTrue(TEXT("A construction it can start is enabled"), BuildButton->GetIsEnabled());
			TestFalse(TEXT("One above its rung is greyed"), HighButton->GetIsEnabled());
			TestTrue(TEXT("with the rung as its reason"), Panel->GetShownStartBlock(High) == ECosmicStartBlock::RowTier);
			TestTrue(TEXT("its tooltip starts with the rung reason"), TipOf(*Panel, High).StartsWith(UCosmicSystemPanelWidget::StartReasonOf(ECosmicStartBlock::RowTier, Tables, High).ToString(), ESearchCase::CaseSensitive));
			TestTrue(TEXT("its name is followed by the short reason"), HighButton->GetLabel().ToString().EndsWith(FText::Format(CosmicUI::Label(TEXT("Start.Suffix.RowTier")), FText::AsNumber(3)).ToString(), ESearchCase::CaseSensitive));
			// The whole label, not a bracket: a short reason without brackets would pass a search for one.
			const FMegaprojectDef* BuildDef = Tables.FindMegaproject(Build);
			const FString Unsuffixed = BuildDef ? FText::Format(INVTEXT("{0}  {1}"), CosmicUI::Label(TEXT("Panel.Start")), BuildDef->DisplayName).ToString() : FString(TEXT("<no build row>"));
			TestTrue(TEXT("An enabled button carries no short reason"), BuildButton->GetLabel().ToString().Equals(Unsuffixed, ESearchCase::CaseSensitive));
			// The short reason is read: Slate draws a greyed-out widget at 45% opacity, so a greyed label is light on a dark
			// face, and an enabled one stays dark on the light face.
			const UButton* Face = nullptr;
			const UTextBlock* Caption = nullptr;
			if (HighButton->WidgetTree)
			{
				HighButton->WidgetTree->ForEachWidget([&Face, &Caption](UWidget* Child)
				{
					Face = Face ? Face : Cast<UButton>(Child);
					Caption = Caption ? Caption : Cast<UTextBlock>(Child);
				});
			}
			if (TestNotNull(TEXT("(the button's face)"), Face) && TestNotNull(TEXT("(and its label)"), Caption))
			{
				const FButtonStyle& Style = Face->GetStyle();
				TestTrue(TEXT("The label takes the face's foreground"), Caption->GetColorAndOpacity() == FSlateColor::UseForeground());
				TestTrue(TEXT("A greyed label is light"), Style.DisabledForeground.GetSpecifiedColor().GetLuminance() > 0.6f);
				TestTrue(TEXT("on a dark face"), Style.Disabled.TintColor.GetSpecifiedColor().GetLuminance() < 0.2f);
				TestTrue(TEXT("An enabled label stays dark"), Style.NormalForeground.GetSpecifiedColor().GetLuminance() < 0.2f);
				TestTrue(TEXT("on the light face"), Style.Normal.TintColor.GetSpecifiedColor().GetLuminance() > 0.3f);
			}
		}
		Live.Civs[Ruled].Tier = EAscensionTier::Tier1_Planetary;
		Panel->Refresh();
	}

	// ---- 3. The cost line ----
	{
		const FStarSystemState& System = Live.Systems[HomeIndex];
		const FString BuildHint = UCosmicSystemPanelWidget::StartHintOf(Tables, Build, System, ECosmicTimeUnit::Cycle).ToString();
		TestEqual(TEXT("The enabled button's tooltip is its cost line"), TipOf(*Panel, Build), BuildHint);
		TestTrue(TEXT("it names the Energy"), BuildHint.Contains(Words(TEXT("Resource.Energy"))));
		TestTrue(TEXT("and the amount"), BuildHint.Contains(FText::AsNumber(10).ToString()));
		TestTrue(TEXT("A construction that costs something warns that answers pay first"), BuildHint.Contains(Words(TEXT("Start.AnswersFirst"))));
		TestFalse(TEXT("A first line names no member it does not pay"), BuildHint.Contains(Words(TEXT("Resource.Matter"))));
		const FString FreeHint = TipOf(*Panel, Free);
		TestEqual(TEXT("A row that costs nothing says so"), FreeHint, Words(TEXT("Start.Free")));
		FNumberFormattingOptions Decimals;
		Decimals.MinimumFractionalDigits = 0;
		Decimals.MaximumFractionalDigits = 2;
		const FString HalfHint = TipOf(*Panel, Half);
		TestTrue(TEXT("A cost in decimals shows its decimals"), HalfHint.Contains(FString::Printf(TEXT("%s %s"), *Words(TEXT("Resource.Energy")), *FText::AsNumber(1.5f, &Decimals).ToString())));
		TestTrue(TEXT("and a whole member of it as a whole number"), HalfHint.Contains(FString::Printf(TEXT("%s %s"), *Words(TEXT("Resource.Matter")), *FText::AsNumber(1).ToString())));
		TestTrue(TEXT("The members are joined by the table's separator"), HalfHint.Contains(FText::Format(CosmicUI::Label(TEXT("List.Pair")),
			FText::Format(INVTEXT("{0} {1}"), CosmicUI::Label(TEXT("Resource.Energy")), FText::AsNumber(1.5f, &Decimals)),
			FText::Format(INVTEXT("{0} {1}"), CosmicUI::Label(TEXT("Resource.Matter")), FText::AsNumber(1))).ToString()));
		// The join leaves an empty text out (a part with no words never leaves a separator behind).
		const FText One = FText::AsCultureInvariant(TEXT("<one>"));
		const FText Two = FText::AsCultureInvariant(TEXT("<two>"));
		TestEqual(TEXT("A join leaves an empty text out"), CosmicUI::JoinList({ One, FText::GetEmpty(), Two }).ToString(), FText::Format(CosmicUI::Label(TEXT("List.Pair")), One, Two).ToString());
		TestTrue(TEXT("and of empty texts alone is empty"), CosmicUI::JoinList({ FText::GetEmpty(), FText::GetEmpty() }).IsEmpty());
		TestTrue(TEXT("An answer's cost line has no warning about answers"), !UCosmicSystemPanelWidget::StartHintOf(Tables, Answer, System, ECosmicTimeUnit::Cycle).ToString().Contains(Words(TEXT("Start.AnswersFirst"))));
		TestTrue(TEXT("A row the tables do not hold has no hint"), UCosmicSystemPanelWidget::StartHintOf(Tables, FName(TEXT("project_test_gone")), System, ECosmicTimeUnit::Cycle).IsEmpty());
	}

	// ---- 4. Pressing a construction: its button stays, greyed with the running reason ----
	{
		UCosmicActionButton* BuildButton = ButtonOf(*Panel, Build);
		if (TestNotNull(TEXT("(the build button)"), BuildButton))
		{
			const int32 Place = Panel->GetActionButtons().IndexOfByKey(BuildButton);
			BuildButton->Press();
			TestEqual(TEXT("Pressing starts it"), Live.Systems[HomeIndex].ActiveMegaproject, Build);
			TestTrue(TEXT("its button stays"), Panel->GetActionButtons().IsValidIndex(Place) && Panel->GetActionButtons()[Place].Get() == BuildButton);
			TestFalse(TEXT("greyed out"), BuildButton->GetIsEnabled());
			TestTrue(TEXT("with the running reason"), Panel->GetShownStartBlock(Build) == ECosmicStartBlock::Running);
			TestTrue(TEXT("its tooltip starts with the running words"), TipOf(*Panel, Build).StartsWith(Words(TEXT("Start.Block.Running")), ESearchCase::CaseSensitive));
			TestTrue(TEXT("its name is followed by the building suffix"), BuildButton->GetLabel().ToString().EndsWith(Words(TEXT("Start.Suffix.Running")), ESearchCase::CaseSensitive));
			int32 Announced = 0;
			const FDelegateHandle Counter = BuildButton->OnAction.AddLambda([&Announced](FName, float) { ++Announced; });
			BuildButton->Press();
			BuildButton->OnAction.Remove(Counter);
			TestEqual(TEXT("A greyed project button announces nothing"), Announced, 0);
		}
	}

	// ---- 5. Another construction warns that the running one's progress is lost ----
	{
		const FMegaprojectDef* BuildDef = Tables.FindMegaproject(Build);
		const FString RebuildTip = TipOf(*Panel, Rebuild);
		const FString Warning = BuildDef ? FText::Format(CosmicUI::Label(TEXT("Start.Replaces")), BuildDef->DisplayName).ToString() : FString(TEXT("<no build row>"));
		TestTrue(TEXT("Another construction warns that the running one's progress is lost"), RebuildTip.Contains(Warning));
		TestTrue(TEXT("first, before the cost: the one loss a start cannot undo"), RebuildTip.StartsWith(Warning, ESearchCase::CaseSensitive));
		TestTrue(TEXT("and stays enabled"), ButtonOf(*Panel, Rebuild) && ButtonOf(*Panel, Rebuild)->GetIsEnabled());
		TestFalse(TEXT("The running construction's own tooltip does not warn"), TipOf(*Panel, Build).Contains(Warning));
	}

	// ---- 6. The answers under their hazard, and hidden under another ----
	{
		Live.Systems[HomeIndex].ActiveHazard = Tag(TEXT("Hazard.Cult"));
		Live.Systems[HomeIndex].HazardTicksRemaining = 300;
		Panel->Refresh();
		TestTrue(TEXT("Under its hazard the first answer shows"), ActionsOf(*Panel).Contains(Answer));
		TestTrue(TEXT("and so does the second"), ActionsOf(*Panel).Contains(Second));
		TestTrue(TEXT("the first enabled"), ButtonOf(*Panel, Answer) && ButtonOf(*Panel, Answer)->GetIsEnabled());
		TestTrue(TEXT("and the second"), ButtonOf(*Panel, Second) && ButtonOf(*Panel, Second)->GetIsEnabled());
		// B6b [B-6b2c] (header 4): the rung's filter is the build buttons' - an answer shows under its hazard at any rung
		// (mutation: the filter on the answers too - the third rung's answer gone from a first-rung home).
		TestTrue(TEXT("Under its hazard an answer two rungs up shows on a first-rung home"), ButtonOf(*Panel, HighAnswer) != nullptr);
		TestFalse(TEXT("greyed"), ButtonOf(*Panel, HighAnswer) && ButtonOf(*Panel, HighAnswer)->GetIsEnabled());
		TestTrue(TEXT("for the rung"), Panel->GetShownStartBlock(HighAnswer) == ECosmicStartBlock::RowTier);
		if (UCosmicActionButton* AnswerButton = ButtonOf(*Panel, Answer))
		{
			AnswerButton->Press();
		}
		TestEqual(TEXT("(the answer runs)"), Live.Systems[HomeIndex].ResponseMegaproject, Answer);
		TestTrue(TEXT("the running answer reads Answering"), Panel->GetShownStartBlock(Answer) == ECosmicStartBlock::Answering);
		TestTrue(TEXT("the other reads ResponseBusy"), Panel->GetShownStartBlock(Second) == ECosmicStartBlock::ResponseBusy);
		TestTrue(TEXT("each with its short reason"), ButtonOf(*Panel, Second) && ButtonOf(*Panel, Second)->GetLabel().ToString().EndsWith(Words(TEXT("Start.Suffix.ResponseBusy")), ESearchCase::CaseSensitive));
		TestTrue(TEXT("(and the running one's)"), ButtonOf(*Panel, Answer) && ButtonOf(*Panel, Answer)->GetLabel().ToString().EndsWith(Words(TEXT("Start.Suffix.Answering")), ESearchCase::CaseSensitive));
		// The words follow the reason, not its place in the enum: each tooltip starts with its own words.
		TestTrue(TEXT("The running answer's tooltip starts with the Answering words"), TipOf(*Panel, Answer).StartsWith(Words(TEXT("Start.Block.Answering")), ESearchCase::CaseSensitive));
		TestTrue(TEXT("the other's with the ResponseBusy words"), TipOf(*Panel, Second).StartsWith(Words(TEXT("Start.Block.ResponseBusy")), ESearchCase::CaseSensitive));
		TestTrue(TEXT("(the AI awakening has a row)"), Tables.FindHazardByTag(Tag(TEXT("Hazard.AIAwakening"))) != nullptr);
		Live.Systems[HomeIndex].ActiveHazard = Tag(TEXT("Hazard.AIAwakening"));
		Panel->Refresh();
		TestTrue(TEXT("A hazard that lists neither answer hides the first"), Panel->GetShownStartBlock(Answer) == ECosmicStartBlock::Count);
		TestTrue(TEXT("and the second"), Panel->GetShownStartBlock(Second) == ECosmicStartBlock::Count);
		// The control of the two above, in the same refresh: an empty panel would hide them too.
		TestTrue(TEXT("while the same refresh still lists the home's constructions"), ActionsOf(*Panel).Contains(Build) && ActionsOf(*Panel).Contains(Rebuild));
		CosmicHazards::EmptyHazardSlot(Live.Systems[HomeIndex]);
	}

	// ---- 7. The year unit ----
	{
		Controller->SetTimeUnit(ECosmicTimeUnit::Year);
		Panel->Refresh();
		const FString YearHint = UCosmicSystemPanelWidget::StartHintOf(Tables, Rebuild, Live.Systems[HomeIndex], ECosmicTimeUnit::Year).ToString();
		const FString YearLine = FText::Format(CosmicUI::Label(TEXT("Start.CostPerYear")), FText::Format(INVTEXT("{0} {1}"), CosmicUI::Label(TEXT("Resource.Energy")), FText::AsNumber(10))).ToString();
		TestTrue(TEXT("In years the cost reads a year"), TipOf(*Panel, Rebuild).Contains(YearLine));
		TestTrue(TEXT("(the tooltip is the year's hint)"), TipOf(*Panel, Rebuild).Contains(YearHint));
		TestFalse(TEXT("(which is not the cycle's line)"), YearHint.Equals(UCosmicSystemPanelWidget::StartHintOf(Tables, Rebuild, Live.Systems[HomeIndex], ECosmicTimeUnit::Cycle).ToString(), ESearchCase::CaseSensitive));
		Controller->SetTimeUnit(ECosmicTimeUnit::Cycle);
		Panel->Refresh();
	}

	// ---- 8. A tick that lets the people pay keeps the tooltip object (the words never read the stockpile) ----
	{
		Live.Systems[HomeIndex].ActiveMegaproject = NAME_None;
		Live.Systems[HomeIndex].MegaprojectProgress = 0.f;
		Live.Civs[Ruled].Stockpile.Energy = 9.5f;
		Controller->SelectSystem(Home);
		HudPanel->Refresh();
		const UCosmicActionButton* HudBuild = ButtonOf(*HudPanel, Build);
		const TSharedPtr<SWidget> SlateBuild = HudBuild ? HudBuild->GetCachedWidget() : TSharedPtr<SWidget>();
		const IToolTip* TipBefore = SlateBuild.IsValid() ? SlateBuild->GetToolTip().Get() : nullptr;
		TestTrue(TEXT("(the HUD's build button has a tooltip)"), TipBefore != nullptr);
		TestTrue(TEXT("(the people cannot pay it yet)"), Live.Civs[Ruled].Stockpile.Energy < 10.f);
		StepBroadcast(*Sim);
		TestTrue(TEXT("(the tick let the people pay it)"), Live.Civs[Ruled].Stockpile.Energy >= 10.f);
		TestTrue(TEXT("(the same button)"), ButtonOf(*HudPanel, Build) == HudBuild);
		TestTrue(TEXT("A tick that lets the people pay keeps the enabled button's tooltip object"), SlateBuild.IsValid() && SlateBuild->GetToolTip().Get() == TipBefore);
		Controller->SelectSystem(NAME_None);
	}

	// ---- 9. The game's tables: the stabilization of the second rung ----
	{
		Sim->SetTables(GameTables);
		const FName Stabilize(TEXT("project_stabilize"));
		CosmicHazards::EmptyHazardSlot(Live.Systems[HomeIndex]);
		Live.Systems[HomeIndex].ActiveMegaproject = NAME_None;
		Live.Civs[Ruled].Tier = EAscensionTier::Tier1_Planetary;
		Panel->ShowSystem(Home);
		TestTrue(TEXT("A quiet first-rung home lists no stabilization"), Panel->GetShownStartBlock(Stabilize) == ECosmicStartBlock::Count);
		// The control, in the same refresh: the game's construction is listed.
		TestTrue(TEXT("while it lists the Dyson swarm"), Panel->GetShownStartBlock(FName(TEXT("project_dyson"))) != ECosmicStartBlock::Count);
		Live.Systems[HomeIndex].ActiveHazard = Tag(TEXT("Hazard.PreSupernova"));
		Live.Systems[HomeIndex].HazardTicksRemaining = 300;
		Panel->Refresh();
		TestTrue(TEXT("Under a supernova a first-rung people sees the stabilization greyed"), ButtonOf(*Panel, Stabilize) && !ButtonOf(*Panel, Stabilize)->GetIsEnabled());
		TestTrue(TEXT("for the rung"), Panel->GetShownStartBlock(Stabilize) == ECosmicStartBlock::RowTier);
		const FString RungWords = UCosmicSystemPanelWidget::StartReasonOf(ECosmicStartBlock::RowTier, GameTables, Stabilize).ToString();
		TestTrue(TEXT("its tooltip starts with the rung reason"), TipOf(*Panel, Stabilize).StartsWith(RungWords, ESearchCase::CaseSensitive));
		TestTrue(TEXT("which names the rung 2"), RungWords.Contains(TEXT("2")));
		CosmicHazards::EmptyHazardSlot(Live.Systems[HomeIndex]);
	}

	// ---- 10. The settle button ----
	{
		Live.Civs[Ruled].Tier = EAscensionTier::Tier3_Causal;
		const TArray<int32> Targets = CosmicMegaprojects::SettleTargets(Live, Sim->GetStateIndex(), GameTables, Sim->GetRules(), Ruled);
		if (TestTrue(TEXT("(the people has a wild neighbour to settle)"), !Targets.IsEmpty()))
		{
			const FStarSystemState& Target = Live.Systems[Targets[0]];
			Panel->ShowSystem(Target.SystemID);
			const FString SettleHint = UCosmicSystemPanelWidget::StartHintOf(GameTables, CosmicRows::ProjectSettle, Target, ECosmicTimeUnit::Cycle).ToString();
			TestEqual(TEXT("The settle button's tooltip is the settle row's cost line"), TipOf(*Panel, CosmicRows::ProjectSettle), SettleHint);
			TestTrue(TEXT("(which names its cost)"), SettleHint.Contains(Words(TEXT("Resource.Matter"))));
			Live.Civs[Ruled].Tier = EAscensionTier::Tier2_Stellar;
			const int32 Unsealed = Live.Transcendents.IndexOfByPredicate([&](const FTranscendentState& Each) { return !Each.bSealed && GameTables.FindTranscendent(Each.TranscendentID) != nullptr; });
			if (TestTrue(TEXT("(the game's galaxy has an unsealed transcendent)"), Unsealed != INDEX_NONE))
			{
				Live.Transcendents[Unsealed].InfluencedSystems.Add(Target.SystemID);
				Panel->Refresh();
				const FString Tip = TipOf(*Panel, CosmicRows::ProjectSettle);
				TestTrue(TEXT("In a sphere the settle tooltip starts with its reason"), Tip.StartsWith(UCosmicSystemPanelWidget::ReasonOf(CosmicMegaprojects::ECosmicSettleBlock::Sphere, GameTables).ToString(), ESearchCase::CaseSensitive));
				TestTrue(TEXT("and ends with the cost line"), Tip.EndsWith(SettleHint, ESearchCase::CaseSensitive));
				Live.Transcendents[Unsealed].InfluencedSystems.Remove(Target.SystemID);
			}
		}
		const int32 Other = Live.Systems.IndexOfByPredicate([&](const FStarSystemState& System) { return !System.OwnerCivID.IsNone() && System.OwnerCivID != Live.Civs[Ruled].CivID && !CosmicHazards::IsDeadNode(System); });
		if (TestTrue(TEXT("(another people owns a system)"), Other != INDEX_NONE))
		{
			Panel->ShowSystem(Live.Systems[Other].SystemID);
			TestEqual(TEXT("Where another people lives the settle tooltip is the reason alone"), TipOf(*Panel, CosmicRows::ProjectSettle),
				UCosmicSystemPanelWidget::ReasonOf(CosmicMegaprojects::ECosmicSettleBlock::Owned, GameTables).ToString());
		}
		Live.Civs[Ruled].Tier = EAscensionTier::Tier1_Planetary;
	}

	// ---- 10b. The binding's hint (B6b decision 28): which fallen one it would hold, in the spiritual view only ----
	{
		FCosmicDataTables BindTables = Tables;
		UDataTable* Projects = CosmicTests::CopyWithout<FMegaprojectDef>(Tables.Megaprojects, {});
		Projects->AddRow(CosmicRows::ProjectBinding, Project(EAscensionTier::Tier4_Transcendent, 10.f, 0.f, 1.0e6f, TEXT("binding")));
		BindTables.Megaprojects = Projects;
		Sim->SetTables(BindTables);
		const TArray<FTranscendentState> SavedTranscendents = Live.Transcendents;
		const int32 Target = CosmicSeal::BindTargetOf(Live, BindTables);
		const FTranscendentDef* Fallen = Live.Transcendents.IsValidIndex(Target) ? BindTables.FindTranscendent(Live.Transcendents[Target].TranscendentID) : nullptr;
		if (TestNotNull(TEXT("(the galaxy has a fallen one to bind, with its row)"), Fallen))
		{
			const FString Hint = FText::FormatNamed(CosmicUI::Label(TEXT("Start.BindHint")), TEXT("Transcendent"), Fallen->Title).ToString();
			// The words after the title: a piece of the hint that names no one.
			const FString HintTail = Hint.Right(Hint.Len() / 2);
			Live.Civs[Ruled].Tier = EAscensionTier::Tier3_Causal;
			Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
			Panel->ShowSystem(Home);
			TestTrue(TEXT("(the view drawn is spiritual)"), ACosmicPlayerController::ShownViewModeFor(*Panel) == ECosmicViewMode::Spiritual);
			const UCosmicActionButton* BindButton = ButtonOf(*Panel, CosmicRows::ProjectBinding);
			TestTrue(TEXT("On the third rung the binding stands greyed for the rung"), BindButton && !BindButton->GetIsEnabled()
				&& Panel->GetShownStartBlock(CosmicRows::ProjectBinding) == ECosmicStartBlock::RowTier);
			TestTrue(TEXT("its tooltip names the fallen one it would hold"), TipOf(*Panel, CosmicRows::ProjectBinding).Contains(Hint));
			TestFalse(TEXT("another construction's does not"), TipOf(*Panel, Build).Contains(HintTail));
			Controller->SetWantedViewMode(ECosmicViewMode::Physical);
			Panel->Refresh();
			TestTrue(TEXT("(the view drawn is physical)"), ACosmicPlayerController::ShownViewModeFor(*Panel) == ECosmicViewMode::Physical);
			TestTrue(TEXT("(the binding still stands)"), ButtonOf(*Panel, CosmicRows::ProjectBinding) != nullptr);
			TestFalse(TEXT("In the physical view its tooltip names no one"), TipOf(*Panel, CosmicRows::ProjectBinding).Contains(HintTail));
			Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
			// Every fallen one bound: NoFallen, and no hint.
			for (FTranscendentState& Transcendent : Live.Transcendents)
			{
				Transcendent.bSealed = true;
			}
			Live.Civs[Ruled].Tier = EAscensionTier::Tier4_Transcendent;
			Panel->Refresh();
			TestTrue(TEXT("With every fallen one bound the binding is NoFallen"), Panel->GetShownStartBlock(CosmicRows::ProjectBinding) == ECosmicStartBlock::NoFallen);
			const UCosmicActionButton* Closed = ButtonOf(*Panel, CosmicRows::ProjectBinding);
			TestTrue(TEXT("its label says so"), Closed && Closed->GetLabel().ToString().Contains(Words(TEXT("Start.Suffix.NoFallen"))));
			const FString ClosedTip = TipOf(*Panel, CosmicRows::ProjectBinding);
			TestTrue(TEXT("its tooltip starts with the reason"), ClosedTip.StartsWith(Words(TEXT("Start.Block.NoFallen")), ESearchCase::CaseSensitive));
			TestFalse(TEXT("and names no one"), ClosedTip.Contains(HintTail));
		}
		Live.Transcendents = SavedTranscendents;
		Live.Civs[Ruled].Tier = EAscensionTier::Tier1_Planetary;
		Sim->SetTables(Tables);
	}

	// ---- 11. The labels ----
	{
		Panel->ShowSystem(Home);
		TestFalse(TEXT("No label of the panel is a missing String Table entry"), ReadTexts(*Panel).Contains(Placeholder));
		// Every label this test reads has words: a search for a blank label finds it anywhere, and two blank texts are equal.
		for (const TCHAR* Key : { TEXT("Start.Block.RowTier"), TEXT("Start.Block.NoFallen"), TEXT("Start.Block.Answering"), TEXT("Start.Block.ResponseBusy"), TEXT("Start.Block.Running"),
			TEXT("Start.Suffix.RowTier"), TEXT("Start.Suffix.NoFallen"), TEXT("Start.Suffix.Answering"), TEXT("Start.Suffix.ResponseBusy"), TEXT("Start.Suffix.Running"),
			TEXT("Start.CostPerCycle"), TEXT("Start.CostPerYear"), TEXT("Start.Free"), TEXT("Start.AnswersFirst"), TEXT("Start.Replaces"),
			TEXT("Panel.Stalled"), TEXT("Speed.ThirdRungTip"), TEXT("Panel.Start"), TEXT("Resource.Energy"), TEXT("Resource.Matter"),
			TEXT("List.Pair"), TEXT("Settle.Block.Owned"), TEXT("Settle.Block.Sphere"), TEXT("Start.BindHint") })
		{
			TestFalse(*FString::Printf(TEXT("%s is no missing String Table entry"), Key), Words(Key).Contains(Placeholder));
			TestFalse(*FString::Printf(TEXT("%s has words"), Key), Words(Key).IsEmpty());
		}
	}
	return true;
}

// The paused mark (ST_CA Panel.Stalled, Phase 3 item 12, guideline 6.3): the watch step by step (two unpaid ticks in a row turn
// it on, two paid ones turn it off; a redraw, a command, a gap, another row, an empty slot and a lower progress), then the
// HUD's panel over broadcast ticks - the slot line, the paid ticks, another people's slot, a slot watched while another
// system was shown, a tick the panel did not see, a load, a refresh inside a broadcast - and neither hash moves.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIStalledSlotTest, "Cosmic_Ascension.UI.StalledSlot", CosmicTests::Flags)
bool FCosmicUIStalledSlotTest::RunTest(const FString& Parameters)
{
	using namespace CosmicCommandReasonTests;
	using CosmicMegaprojects::ECosmicProjectSlot;
	using CosmicMegaprojects::FCosmicSlotReading;

	// ---- The watch by itself ----
	{
		const FName Row(TEXT("project_test_x"));
		const TArray<bool> Never = Marks({ Reading(Row, 0.f), Reading(Row, 0.f), Reading(Row, 0.f) });
		TestFalse(TEXT("A slot that never pays shows no mark after one tick"), Never.IsValidIndex(1) && Never[1]);
		TestTrue(TEXT("and the mark after two"), Never.IsValidIndex(2) && Never[2]);
		TArray<FCosmicSlotReading> Always;
		TArray<FCosmicSlotReading> EveryOther;
		TArray<FCosmicSlotReading> OneInThree;
		float OtherProgress = 0.f;
		float ThirdProgress = 0.f;
		for (int32 Tick = 1; Tick <= 40; ++Tick)
		{
			Always.Add(Reading(Row, static_cast<float>(Tick)));
			OtherProgress += Tick % 2 == 0 ? 1.f : 0.f;
			EveryOther.Add(Reading(Row, OtherProgress));
			ThirdProgress += Tick % 3 == 0 ? 1.f : 0.f;
			OneInThree.Add(Reading(Row, ThirdProgress));
		}
		TestFalse(TEXT("A slot that always pays never shows it"), Marks(Always).Contains(true));
		TestFalse(TEXT("A slot that pays every other cycle never shows it"), Marks(EveryOther).Contains(true));
		const TArray<bool> Third = Marks(OneInThree);
		bool bSteady = Third.Num() == 40;
		// It pays on ticks 3, 6, 9, ...: unpaid on 4 and 5 turns the mark on, and one paid tick between two unpaid pairs
		// never clears it.
		for (int32 At = 4; bSteady && At < Third.Num(); ++At)
		{
			bSteady = Third[At];
		}
		TestTrue(TEXT("A slot that pays one cycle in three shows it steadily from its fifth tick"), bSteady);
		const TArray<bool> Back = Marks({ Reading(Row, 0.f), Reading(Row, 0.f), Reading(Row, 0.f), Reading(Row, 1.f), Reading(Row, 2.f) });
		TestTrue(TEXT("One paid tick keeps the mark"), Back.IsValidIndex(3) && Back[3]);
		TestFalse(TEXT("two clear it"), Back.IsValidIndex(4) && Back[4]);

		FCosmicSlotWatch Stalled;
		for (int32 Tick = 1; Tick <= 3; ++Tick)
		{
			Stalled = CosmicSlotWatch::Step(Stalled, Reading(Row, 5.f), Tick);
		}
		TestTrue(TEXT("(a stalled watch)"), Stalled.bStalled);
		const FCosmicSlotWatch Again = CosmicSlotWatch::Step(Stalled, Reading(Row, 5.f), 3);
		TestTrue(TEXT("The same tick seen again changes nothing"), Again.bStalled && Again.UnpaidRun == Stalled.UnpaidRun && Again.Tick == Stalled.Tick);
		TestFalse(TEXT("A new reading at the same tick starts again"), CosmicSlotWatch::Step(Stalled, Reading(FName(TEXT("project_test_y")), 0.f), 3).bStalled);
		TestFalse(TEXT("A tick that is not the next starts again"), CosmicSlotWatch::Step(Stalled, Reading(Row, 5.f), 5).bStalled);
		// Another row further on than the old one: read as the same row it would count as a paid tick and keep the mark.
		TestFalse(TEXT("Another row starts again"), CosmicSlotWatch::Step(Stalled, Reading(FName(TEXT("project_test_y")), 6.f), 4).bStalled);
		TestFalse(TEXT("An empty slot never shows it"), CosmicSlotWatch::Step(Stalled, Reading(NAME_None, 0.f), 4).bStalled);
		TestFalse(TEXT("A lower progress starts again"), CosmicSlotWatch::Step(Stalled, Reading(Row, 3.f), 4).bStalled);
	}

	// ---- The HUD's panel ----
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UWorld* World = Sim->GetGameInstance()->GetWorld();
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(World ? World->GetFirstPlayerController() : nullptr);
	ACosmicHUD* HUD = Controller ? Controller->GetHUD<ACosmicHUD>() : nullptr;
	UCosmicSystemPanelWidget* Panel = HUD ? HUD->GetSystemPanel() : nullptr;
	if (!TestNotNull(TEXT("The first player controller is the game's"), Controller) || !TestNotNull(TEXT("The HUD has its system panel"), Panel))
	{
		return false;
	}
	const FCosmicDataTables GameTables = Sim->GetTables();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(GameTables);
		Sim->SetTimeScale(0.f);
		Controller->SelectSystem(NAME_None);
		Sim->NewGalaxy(42, 5, 3);
	};
	// Rows of the test - a construction no stockpile pays and an answer no stockpile pays (of the hold, a hazard of the
	// test) - and a copy of the game's settle row.
	UDataTable* Projects = CosmicTests::NewTable(FMegaprojectDef::StaticStruct());
	Projects->AddRow(Pricey, Project(EAscensionTier::Tier1_Planetary, 1.0e9f, 0.f, 1.0e9f, TEXT("pricey")));
	Projects->AddRow(PriceyAnswer, Project(EAscensionTier::Tier1_Planetary, 1.0e9f, 0.f, 1.0e9f, TEXT("pricey answer")));
	if (const FMegaprojectDef* Settle = GameTables.FindMegaproject(CosmicRows::ProjectSettle))
	{
		Projects->AddRow(CosmicRows::ProjectSettle, *Settle);
	}
	FCosmicDataTables Tables = GameTables;
	Tables.Megaprojects = Projects;
	Tables.Hazards = HazardsWith(GameTables, Hold, TestHazard(TEXT("hold"), { PriceyAnswer }));
	Sim->SetTables(Tables);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	FGalaxyState& Live = Sim->GetMutableState();
	// The whole board known (Phase 3 item 8): another people's system must be selectable, or "another people's unpaid
	// slot shows no mark" would pass on a panel that stayed on the home.
	CosmicTests::KnowAll(Live);
	const int32 Ruled = Sim->GetStateIndex().FindCiv(Live, Live.Player.RuledCivID);
	const int32 HomeIndex = Live.Civs.IsValidIndex(Ruled) ? Sim->GetStateIndex().FindSystem(Live, Live.Civs[Ruled].HomeSystemID) : INDEX_NONE;
	const int32 OtherIndex = Live.Systems.IndexOfByPredicate([&](const FStarSystemState& System)
	{
		return Live.Civs.IsValidIndex(Ruled) && !System.OwnerCivID.IsNone() && System.OwnerCivID != Live.Civs[Ruled].CivID && CosmicState::IsSettled(System);
	});
	if (!TestTrue(TEXT("(the vessel rules a people with a home)"), Live.Systems.IsValidIndex(HomeIndex)) || !TestTrue(TEXT("(another people has a settled system)"), Live.Systems.IsValidIndex(OtherIndex)))
	{
		return false;
	}
	const FName Home = Live.Systems[HomeIndex].SystemID;
	const FName Other = Live.Systems[OtherIndex].SystemID;
	const FString Mark = Words(TEXT("Panel.Stalled"));
	auto Detail = [Panel]() { return Panel->GetDetailText() ? Panel->GetDetailText()->GetText().ToString() : FString(); };
	auto HomeStalled = [&]() { return Panel->IsSlotStalled(Home, ECosmicProjectSlot::Construction); };
	FMegaprojectDef* PriceyRow = Projects->FindRow<FMegaprojectDef>(Pricey, TEXT("StalledSlot"));
	if (!TestNotNull(TEXT("(the pricey row is held)"), PriceyRow))
	{
		return false;
	}

	// The home's construction slot holds a row no stockpile pays.
	Live.Systems[HomeIndex].ActiveMegaproject = Pricey;
	Live.Systems[HomeIndex].MegaprojectProgress = 0.f;
	Controller->SelectSystem(Home);
	StepBroadcast(*Sim);
	TestFalse(TEXT("One unpaid tick shows no mark yet"), HomeStalled());
	StepBroadcast(*Sim);
	TestTrue(TEXT("Two unpaid ticks show the mark"), HomeStalled());
	TestTrue(TEXT("and the slot line says so"), Detail().Contains(Mark));
	TestFalse(TEXT("(the response slot shows none)"), Panel->IsSlotStalled(Home, ECosmicProjectSlot::Response));

	// Free now: the slot pays.
	PriceyRow->CostPerTick = FResourcePool();
	StepBroadcast(*Sim);
	TestTrue(TEXT("(it paid)"), Live.Systems[HomeIndex].MegaprojectProgress > 0.f);
	TestTrue(TEXT("One paid tick keeps it"), HomeStalled());
	StepBroadcast(*Sim);
	TestFalse(TEXT("Two paid ticks clear it"), HomeStalled());
	TestFalse(TEXT("and the line no longer says so"), Detail().Contains(Mark));
	PriceyRow->CostPerTick.Energy = 1.0e9f;

	// Another people's slot that does not pay: no mark (it would tell that people's stockpile). The home meanwhile does not
	// pay either, and is watched while it is not shown.
	Live.Systems[OtherIndex].ActiveMegaproject = Pricey;
	Live.Systems[OtherIndex].MegaprojectProgress = 0.f;
	Controller->SelectSystem(Other);
	for (int32 Step = 0; Step < 3; ++Step)
	{
		StepBroadcast(*Sim);
	}
	TestTrue(TEXT("(the other people's slot paid nothing)"), Live.Systems[OtherIndex].ActiveMegaproject == Pricey && Live.Systems[OtherIndex].MegaprojectProgress == 0.f);
	TestFalse(TEXT("Another people's unpaid slot shows no mark"), Panel->IsSlotStalled(Other, ECosmicProjectSlot::Construction));
	TestFalse(TEXT("and its line says nothing of it"), Detail().Contains(Mark));
	Controller->SelectSystem(Home);
	TestTrue(TEXT("A slot watched while another system was shown shows its mark"), HomeStalled());
	TestTrue(TEXT("(and its line says so)"), Detail().Contains(Mark));

	// A tick the panel did not see (a headless tick broadcasts nothing): the watch starts again.
	Sim->RunHeadless(1);
	StepBroadcast(*Sim);
	TestFalse(TEXT("A tick the panel did not see starts the watch again"), HomeStalled());

	// A load is another game: a stalled home loaded one tick later starts its watch anew.
	StepBroadcast(*Sim);
	StepBroadcast(*Sim);
	TestTrue(TEXT("(stalled again)"), HomeStalled());
	{
		FGalaxyState Later = Live;
		Later.Tick += 1;
		const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
		Sim->SetBroadcastEnabled(true);
		Sim->LoadState(Later);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Panel->Refresh();
		TestFalse(TEXT("A loaded galaxy starts its watches anew"), HomeStalled());
	}

	// A refresh inside a Core Logic broadcast judges nothing (mid-tick a slot may not have been paid yet); the next one
	// outside does.
	Controller->SelectSystem(Home);
	StepBroadcast(*Sim);
	TestFalse(TEXT("(one unpaid tick)"), HomeStalled());
	Sim->BeginBroadcast();
	Live.Tick += 1;
	Panel->Refresh();
	const bool bJudgedInside = HomeStalled();
	Sim->EndBroadcast();
	TestFalse(TEXT("A refresh inside a Core Logic broadcast judges nothing"), bJudgedInside);
	Panel->Refresh();
	TestTrue(TEXT("and the next refresh outside does"), HomeStalled());

	// The watch is the panel's memory: it moves neither hash.
	const uint32 StateHash = Sim->ComputeStateHash();
	const uint32 WorldHash = Sim->ComputeWorldHash();
	Panel->Refresh();
	Controller->SelectSystem(Other);
	Controller->SelectSystem(Home);
	Panel->Refresh();
	TestEqual(TEXT("The watch moves no state"), Sim->ComputeStateHash(), StateHash);
	TestEqual(TEXT("and no world"), Sim->ComputeWorldHash(), WorldHash);

	// The mark belongs to the people the vessel rules (it would tell another people's stockpile): when the home stops being
	// theirs, or the vessel stops ruling, both its watches go - and inside a Core Logic broadcast, where the watches are
	// not stepped, the line itself asks the owner. Both slots of the home stall first: the hold stands there, and its
	// answer no stockpile pays waits in the response slot.
	Live.Systems[HomeIndex].ActiveHazard = Tag(TEXT("Hazard.Cult"));
	Live.Systems[HomeIndex].HazardTicksRemaining = 400;
	Live.Systems[HomeIndex].ResponseMegaproject = PriceyAnswer;
	Live.Systems[HomeIndex].ResponseProgress = 0.f;
	Controller->SelectSystem(Home);
	StepBroadcast(*Sim);
	StepBroadcast(*Sim);
	StepBroadcast(*Sim);
	auto HomeAnswerStalled = [&]() { return Panel->IsSlotStalled(Home, ECosmicProjectSlot::Response); };
	if (!TestTrue(TEXT("(both slots of the home are stalled)"), HomeStalled() && HomeAnswerStalled() && Live.Systems[HomeIndex].ResponseMegaproject == PriceyAnswer))
	{
		return false;
	}
	const FName RuledID = Live.Civs[Ruled].CivID;
	const FName OtherOwner = Live.Systems[OtherIndex].OwnerCivID;
	const int32 OtherCiv = Sim->GetStateIndex().FindCiv(Live, OtherOwner);
	if (!TestTrue(TEXT("(the other system's owner is a people)"), Live.Civs.IsValidIndex(OtherCiv)))
	{
		return false;
	}
	// The home handed to the other people inside a broadcast (a tick stage's handler redraws the HUD there).
	Sim->BeginBroadcast();
	Live.Systems[HomeIndex].OwnerCivID = OtherOwner;
	Live.Civs[Ruled].OwnedSystems.Remove(Home);
	Live.Civs[OtherCiv].OwnedSystems.Add(Home);
	Panel->Refresh();
	const bool bMarkedInside = Detail().Contains(Mark);
	Sim->EndBroadcast();
	TestFalse(TEXT("Inside a broadcast a home handed to another people shows no mark"), bMarkedInside);
	Panel->Refresh();
	TestFalse(TEXT("A home handed to another people drops its construction watch"), HomeStalled());
	TestFalse(TEXT("and its response watch"), HomeAnswerStalled());
	TestFalse(TEXT("(and its line says nothing of it)"), Detail().Contains(Mark));
	// Back to the people: watched anew, stalled again two unpaid ticks later.
	Live.Systems[HomeIndex].OwnerCivID = RuledID;
	Live.Civs[OtherCiv].OwnedSystems.Remove(Home);
	Live.Civs[Ruled].OwnedSystems.Add(Home);
	Panel->Refresh();
	StepBroadcast(*Sim);
	StepBroadcast(*Sim);
	if (!TestTrue(TEXT("(stalled once more)"), HomeStalled() && HomeAnswerStalled()))
	{
		return false;
	}
	// A deposition (BecomeWanderer): the vessel walks, and its old people's slots are no longer its to mark.
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	if (!TestNotNull(TEXT("(the role subsystem runs)"), Role))
	{
		return false;
	}
	Role->BecomeWanderer(FGameplayTag(), Tag(TEXT("Reign.Deposed")));
	if (!TestTrue(TEXT("(the vessel walks)"), Live.Player.Role == EPlayerRole::Wanderer))
	{
		return false;
	}
	Panel->Refresh();
	TestFalse(TEXT("A vessel that no longer rules keeps no construction watch"), HomeStalled());
	TestFalse(TEXT("nor a response watch"), HomeAnswerStalled());
	TestFalse(TEXT("and the home's line says nothing of it"), Detail().Contains(Mark));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

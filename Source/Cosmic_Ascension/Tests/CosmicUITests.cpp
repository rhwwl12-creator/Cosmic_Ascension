// Cosmic Ascension - automation tests of the UI layer. Design: guideline.md section 8.5, 9.3.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Engine/DataTable.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTableCoreFwd.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/SharedPointer.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicTextPanelWidget.h"
#include "Widgets/SWidget.h"

namespace CosmicUITests
{
	// Everything the text blocks of the panel show, one block per line. Returns the number of text blocks.
	int32 ReadPanel(const UCosmicTextPanelWidget& Panel, FString& OutShown)
	{
		int32 TextBlocks = 0;
		OutShown.Reset();
		if (Panel.WidgetTree)
		{
			Panel.WidgetTree->ForEachWidget([&TextBlocks, &OutShown](UWidget* Widget)
			{
				if (const UTextBlock* Block = Cast<UTextBlock>(Widget))
				{
					++TextBlocks;
					OutShown += Block->GetText().ToString();
					OutShown += TEXT("\n");
				}
			});
		}
		return TextBlocks;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIPanelLabelsTest, "Cosmic_Ascension.UI.PanelLabels", CosmicTests::Flags)
bool FCosmicUIPanelLabelsTest::RunTest(const FString& Parameters)
{
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

	Sim->NewGalaxy(42, 5, 3);
	const FGalaxyState& State = Sim->GetState();

	// A panel of its own, so that the test does not lean on the HUD of the running map. Taking the Slate widget builds
	// the tree and constructs the panel; holding it keeps the panel constructed until the checks are done.
	UCosmicTextPanelWidget* Panel = CreateWidget<UCosmicTextPanelWidget>(Sim->GetGameInstance());
	if (!TestNotNull(TEXT("The panel can be created"), Panel))
	{
		return false;
	}
	const TSharedRef<SWidget> SlatePanel = Panel->TakeWidget();
	Panel->Refresh();

	// The simulation is paused here, so the status shows the pause label and with it every key the panel reads.
	const FString& Placeholder = FStringTableEntry::GetPlaceholderSourceString();
	FString Shown;
	TestEqual(TEXT("The panel has its two text blocks"), CosmicUITests::ReadPanel(*Panel, Shown), 2);
	TestTrue(TEXT("No label is a missing String Table entry (run Tools/make_data_tables.py)"), !Shown.Contains(Placeholder));

	const FName RuledCivID = State.Player.RuledCivID;
	const FCivilizationState* Civ = State.Civs.FindByPredicate([RuledCivID](const FCivilizationState& Candidate) { return Candidate.CivID == RuledCivID; });
	if (TestNotNull(TEXT("The player rules a civ"), Civ))
	{
		TestTrue(TEXT("The panel names the ruled civ"), !Civ->CivName.IsEmpty() && Shown.Contains(Civ->CivName.ToString()));
		// The rung as the data counts it, 1 to 5 (section 7.3), after the label.
		const FString TierLabel = FText::FromStringTable(FName(TEXT("/Game/CA/Data/ST_CA.ST_CA")), TEXT("Panel.Tier"), EStringTableLoadingPolicy::FindOrFullyLoad).ToString();
		TestTrue(TEXT("The panel shows the rung of the ruled civ"), Shown.Contains(FString::Printf(TEXT("%s %d"), *TierLabel, static_cast<int32>(Civ->Tier) + 1)));
	}
	if (TestTrue(TEXT("The log has its first line"), !State.Log.IsEmpty()))
	{
		TestTrue(TEXT("The panel shows the last log line"), Shown.Contains(State.Log.Last().Message.ToString()));
	}

	// An entry marked bUntold is kept, never shown (section 8.1): the panel passes it by and still shows the line before it.
	{
		const FString LastTold = State.Log.IsEmpty() ? FString() : State.Log.Last().Message.ToString();
		FCosmicLogEntry Untold;
		Untold.Tick = State.Tick;
		Untold.Severity = 2;
		Untold.Message = FText::AsCultureInvariant(TEXT("<untold line of the test>"));
		Untold.bUntold = true;
		Sim->GetMutableState().Log.Add(Untold);
		Panel->Refresh();
		FString WithUntold;
		CosmicUITests::ReadPanel(*Panel, WithUntold);
		TestFalse(TEXT("The panel does not show an untold entry"), WithUntold.Contains(TEXT("<untold line of the test>")));
		TestTrue(TEXT("and still shows the last told line"), !LastTold.IsEmpty() && WithUntold.Contains(LastTold));
		Sim->GetMutableState().Log.Pop();
		Panel->Refresh();
	}

	// A project the ruler starts shows with its system and its name (the project key of the prototype).
	{
		UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
		const FName Capital = Civ ? Civ->HomeSystemID : FName();
		const FStarSystemState* Home = State.Systems.FindByPredicate([Capital](const FStarSystemState& System) { return System.SystemID == Capital; });
		if (TestTrue(TEXT("The project key starts a project"), Role->StartNextMegaproject()) && TestNotNull(TEXT("The ruled civ has its capital"), Home))
		{
			const FMegaprojectDef* Row = Sim->GetTables().FindMegaproject(Home->ActiveMegaproject);
			Panel->Refresh();
			FString WithProject;
			CosmicUITests::ReadPanel(*Panel, WithProject);
			if (TestNotNull(TEXT("The started project is a row of the game"), Row))
			{
				TestTrue(TEXT("The panel shows the running project"), WithProject.Contains(Home->DisplayName.ToString() + TEXT(" ") + Row->DisplayName.ToString()));
				// The progress against what the simulation needs, as whole numbers.
				TestTrue(TEXT("with its progress and the progress it needs"), WithProject.Contains(FString::Printf(TEXT(" %s %s/%s"), *Row->DisplayName.ToString(),
					*FText::AsNumber(FMath::FloorToInt32(Home->MegaprojectProgress)).ToString(), *FText::AsNumber(FMath::FloorToInt32(CosmicMegaprojects::RequiredProgressOf(*Row))).ToString())));
			}
			TestTrue(TEXT("No label of the project line is missing"), !WithProject.Contains(Placeholder));
			Shown = WithProject;
		}
	}

	// The progress a project needs is the simulation's reading of its row: a row below 1 (only a hand-made table holds one -
	// the schema asks for 1 or more) needs 1.
	{
		const FCosmicDataTables GameTables = Sim->GetTables();
		const FName Capital = Civ ? Civ->HomeSystemID : FName();
		FStarSystemState* Home = Sim->GetMutableState().Systems.FindByPredicate([Capital](const FStarSystemState& System) { return System.SystemID == Capital; });
		if (TestNotNull(TEXT("The ruled civ has its capital for the half row"), Home))
		{
			const FName SlotBefore = Home->ActiveMegaproject;
			const float ProgressBefore = Home->MegaprojectProgress;
			UDataTable* Projects = NewObject<UDataTable>(GetTransientPackage());
			Projects->RowStruct = FMegaprojectDef::StaticStruct();
			FMegaprojectDef Half;
			Half.DisplayName = FText::AsCultureInvariant(TEXT("<half>"));
			Half.RequiredProgress = 0.5f;
			Projects->AddRow(FName(TEXT("project_test_half")), Half);
			FCosmicDataTables WithHalf = GameTables;
			WithHalf.Megaprojects = Projects;
			Sim->SetTables(WithHalf);
			Home->ActiveMegaproject = FName(TEXT("project_test_half"));
			Home->MegaprojectProgress = 0.f;
			Panel->Refresh();
			FString WithHalfShown;
			CosmicUITests::ReadPanel(*Panel, WithHalfShown);
			TestTrue(TEXT("A row that needs less than 1 shows that it needs 1"), WithHalfShown.Contains(TEXT("<half> 0/1")));
			Sim->SetTables(GameTables);
			Home->ActiveMegaproject = SlotBefore;
			Home->MegaprojectProgress = ProgressBefore;
			Panel->Refresh();
		}
	}

	// A tick redraws the status through the delegate alone; nothing here asks the panel to refresh.
	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	Sim->SetBroadcastEnabled(true);
	Sim->StepOnce();
	Sim->SetBroadcastEnabled(bWasBroadcasting);
	FString ShownAfterTick;
	CosmicUITests::ReadPanel(*Panel, ShownAfterTick);
	TestTrue(TEXT("A tick redraws the panel"), ShownAfterTick != Shown);
	TestTrue(TEXT("No label is missing after the tick"), !ShownAfterTick.Contains(Placeholder));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

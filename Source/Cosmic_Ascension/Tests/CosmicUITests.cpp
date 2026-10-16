// Cosmic Ascension - automation tests of the UI layer. Design: guideline.md section 8.5, 9.3.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "Internationalization/StringTableCore.h"
#include "Misc/AutomationTest.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/SharedPointer.h"
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
	}
	if (TestTrue(TEXT("The log has its first line"), !State.Log.IsEmpty()))
	{
		TestTrue(TEXT("The panel shows the last log line"), Shown.Contains(State.Log.Last().Message.ToString()));
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
			}
			TestTrue(TEXT("No label of the project line is missing"), !WithProject.Contains(Placeholder));
			Shown = WithProject;
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

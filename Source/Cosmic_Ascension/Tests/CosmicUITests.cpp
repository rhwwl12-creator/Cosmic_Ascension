// Cosmic Ascension - automation tests of the UI layer: the log tail of the Phase 1 text panel (the top bar and the system
// panel are Tests/CosmicPanelTests.cpp). Design: guideline.md section 8.1, 8.5, 9.3.

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
#include "GameplayTagContainer.h"
#include "Log/CosmicLogSubsystem.h"
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

	// Every key the panel reads (its title) shows at once.
	const FString& Placeholder = FStringTableEntry::GetPlaceholderSourceString();
	FString Shown;
	TestEqual(TEXT("The panel has its two text blocks"), CosmicUITests::ReadPanel(*Panel, Shown), 2);
	TestTrue(TEXT("No label is a missing String Table entry (run Tools/make_data_tables.py)"), !Shown.Contains(Placeholder));

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

	// A full panel counts told lines, not entries: 70 told lines, with an untold entry after the 40th and another after
	// them all, show the last 60 told (the panel's MaxShownLogLines). Two untold entries inside the last 61 entries tell a
	// count of told lines from a window of entries one longer.
	{
		constexpr int32 PanelLines = 60;
		TArray<FCosmicLogEntry>& Entries = Sim->GetMutableState().Log;
		const int32 Before = Entries.Num();
		for (int32 Index = 1; Index <= 70; ++Index)
		{
			FCosmicLogEntry Told;
			Told.Tick = State.Tick;
			Told.Message = FText::AsCultureInvariant(FString::Printf(TEXT("<told line %d>"), Index));
			Entries.Add(Told);
			if (Index == 40 || Index == 70)
			{
				FCosmicLogEntry Untold;
				Untold.Tick = State.Tick;
				Untold.Severity = 2;
				Untold.Message = FText::AsCultureInvariant(TEXT("<untold line of the test>"));
				Untold.bUntold = true;
				Entries.Add(Untold);
			}
		}
		Panel->Refresh();
		FString Full;
		CosmicUITests::ReadPanel(*Panel, Full);
		TestTrue(TEXT("A full panel shows the oldest of its told lines"), Full.Contains(FString::Printf(TEXT("<told line %d>"), 70 - PanelLines + 1)));
		TestFalse(TEXT("and not one older"), Full.Contains(FString::Printf(TEXT("<told line %d>"), 70 - PanelLines)));
		Entries.SetNum(Before);
		Panel->Refresh();
	}

	// A new line redraws the log through the delegate alone; nothing here asks the panel to refresh.
	{
		const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
		Sim->SetBroadcastEnabled(true);
		Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>()->Push(FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Civ"))), 1,
			FText::AsCultureInvariant(TEXT("<a new line of the test>")), FFormatNamedArguments(), NAME_None, NAME_None);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		FString AfterLine;
		CosmicUITests::ReadPanel(*Panel, AfterLine);
		TestTrue(TEXT("A new log line redraws the panel"), AfterLine.Contains(TEXT("<a new line of the test>")));
		TestTrue(TEXT("No label is missing after the line"), !AfterLine.Contains(Placeholder));
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

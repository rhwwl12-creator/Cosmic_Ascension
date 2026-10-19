// Cosmic Ascension - automation tests of the log feed (Presentation). Design: guideline.md section 6.3, 8.1, 8.2, 9.3,
// 9.4, 11 Phase 3.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Core/GalaxyState.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "Internationalization/StringTableCore.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicLogFeedWidget.h"
#include "World/CosmicCameraPawn.h"
#include "World/CosmicGalaxyMapActor.h"
#include "World/CosmicGameMode.h"

namespace CosmicLogFeedTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	ACosmicGalaxyMapActor* FindMap(UWorld* World)
	{
		// The iterator checks its world as it is made, before any loop condition.
		if (!World)
		{
			return nullptr;
		}
		for (TActorIterator<ACosmicGalaxyMapActor> It(World); It; ++It)
		{
			return *It;
		}
		return nullptr;
	}

	// The messages of the told entries of the log, oldest first.
	TArray<FString> ToldMessages(const FGalaxyState& State)
	{
		TArray<FString> Messages;
		for (const FCosmicLogEntry& Entry : State.Log)
		{
			if (!Entry.bUntold)
			{
				Messages.Add(Entry.Message.ToString());
			}
		}
		return Messages;
	}

	TArray<FString> ItemMessages(const UCosmicLogFeedWidget& Feed)
	{
		TArray<FString> Messages;
		for (const UCosmicLogItem* Item : Feed.GetItems())
		{
			Messages.Add(Item ? Item->Message.ToString() : FString());
		}
		return Messages;
	}

	FCosmicLogEntry Line(const FGalaxyState& State, const FString& Message, bool bUntold = false)
	{
		FCosmicLogEntry Entry;
		Entry.Tick = State.Tick;
		Entry.Severity = bUntold ? 2 : 1;
		Entry.Message = FText::AsCultureInvariant(Message);
		Entry.bUntold = bUntold;
		return Entry;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUILogFeedTest, "Cosmic_Ascension.UI.LogFeed", CosmicTests::Flags)
bool FCosmicUILogFeedTest::RunTest(const FString& Parameters)
{
	using namespace CosmicLogFeedTests;

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
	ACosmicGalaxyMapActor* Map = FindMap(World);
	if (!TestNotNull(TEXT("The first player controller is the game's"), Controller) || !TestNotNull(TEXT("The game mode spawned the map"), Map))
	{
		return false;
	}
	ON_SCOPE_EXIT
	{
		Controller->SelectSystem(NAME_None);
		Sim->NewGalaxy(42, 5, 3);
	};
	Sim->NewGalaxy(42, 0, 0);
	Map->RefreshNow();

	// A feed of its own, so that the test does not lean on the HUD; holding the Slate widget keeps it constructed.
	UCosmicLogFeedWidget* Feed = CreateWidget<UCosmicLogFeedWidget>(Controller);
	if (!TestNotNull(TEXT("The feed can be created"), Feed))
	{
		return false;
	}
	const TSharedRef<SWidget> SlateFeed = Feed->TakeWidget();
	FGalaxyState& Live = Sim->GetMutableState();

	// ---- The items are the told entries, in order ----
	{
		FString Title;
		Feed->WidgetTree->ForEachWidget([&Title](UWidget* Child)
		{
			if (const UTextBlock* Block = Cast<UTextBlock>(Child))
			{
				Title += Block->GetText().ToString();
			}
		});
		TestFalse(TEXT("No label of the feed is a missing String Table entry"), Title.Contains(FStringTableEntry::GetPlaceholderSourceString()));
		TestTrue(TEXT("(the log has its first lines)"), !Live.Log.IsEmpty());
		TestTrue(TEXT("The feed holds the told entries, oldest first"), ItemMessages(*Feed) == ToldMessages(Live));
	}

	// ---- An untold entry is never an item ----
	{
		const int32 Before = Feed->GetItems().Num();
		Live.Log.Add(Line(Live, TEXT("<untold line of the test>"), true));
		Feed->Refresh();
		TestEqual(TEXT("An untold entry adds no item"), Feed->GetItems().Num(), Before);
		TestFalse(TEXT("and does not show"), ItemMessages(*Feed).Contains(TEXT("<untold line of the test>")));
		Live.Log.Pop();
		Feed->Refresh();
	}

	// ---- 600 told lines (an untold one among them) leave the last 500 told; one more through the delegate drops the oldest ----
	{
		const int32 Before = Live.Log.Num();
		for (int32 Index = 1; Index <= 600; ++Index)
		{
			Live.Log.Add(Line(Live, FString::Printf(TEXT("<told line %d>"), Index)));
			if (Index == 300)
			{
				Live.Log.Add(Line(Live, TEXT("<untold line of the test>"), true));
			}
		}
		Feed->Refresh();
		TestEqual(TEXT("A long log leaves 500 items"), Feed->GetItems().Num(), CosmicLogFeed::MaxItems);
		const TArray<FString> Shown = ItemMessages(*Feed);
		TestEqual(TEXT("the last 500 told lines, from the 101st"), Shown.IsEmpty() ? FString() : Shown[0], FString(TEXT("<told line 101>")));
		TestEqual(TEXT("to the last"), Shown.IsEmpty() ? FString() : Shown.Last(), FString(TEXT("<told line 600>")));

		const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
		Sim->SetBroadcastEnabled(true);
		Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>()->Push(Tag(TEXT("Log.Civ")), 1, FText::AsCultureInvariant(TEXT("<a new line of the test>")),
			FFormatNamedArguments(), NAME_None, NAME_None);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		const TArray<FString> After = ItemMessages(*Feed);
		TestEqual(TEXT("A new line arrives through the delegate alone"), After.IsEmpty() ? FString() : After.Last(), FString(TEXT("<a new line of the test>")));
		TestEqual(TEXT("still 500 items"), After.Num(), CosmicLogFeed::MaxItems);
		TestEqual(TEXT("the oldest dropped"), After.IsEmpty() ? FString() : After[0], FString(TEXT("<told line 102>")));
		Live.Log.SetNum(Before);
		Feed->Refresh();
	}

	// ---- A new galaxy rebuilds the feed through its delegate ----
	{
		const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
		Sim->SetBroadcastEnabled(true);
		Live.Log.Add(Line(Live, TEXT("<a line of the old galaxy>")));
		Feed->Refresh();
		Sim->NewGalaxy(42, 0, 0);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Map->RefreshNow();
		TestFalse(TEXT("A new galaxy clears the old galaxy's lines"), ItemMessages(*Feed).Contains(TEXT("<a line of the old galaxy>")));
		TestTrue(TEXT("and shows its own"), ItemMessages(*Feed) == ToldMessages(Sim->GetState()));
	}

	// ---- Colours (section 8.2) ----
	{
		const FLinearColor Plain = CosmicLogFeed::ColorOf(0, Tag(TEXT("Log.Civ")));
		const FLinearColor Notable = CosmicLogFeed::ColorOf(1, Tag(TEXT("Log.Civ")));
		const FLinearColor Mythic = CosmicLogFeed::ColorOf(2, Tag(TEXT("Log.Civ")));
		const FLinearColor Spirit = CosmicLogFeed::ColorOf(0, Tag(TEXT("Log.Transcendent")));
		TestFalse(TEXT("Severity 0 and 1 differ"), Plain.Equals(Notable));
		TestFalse(TEXT("Severity 1 and 2 differ"), Notable.Equals(Mythic));
		TestFalse(TEXT("Severity 0 and 2 differ"), Plain.Equals(Mythic));
		TestFalse(TEXT("The spirit's voice is not the grey"), Spirit.Equals(Plain));
		TestFalse(TEXT("nor the white"), Spirit.Equals(Notable));
		TestFalse(TEXT("nor the gold"), Spirit.Equals(Mythic));
		TestTrue(TEXT("whatever its severity"), CosmicLogFeed::ColorOf(2, Tag(TEXT("Log.Transcendent"))).Equals(Spirit));
		TestTrue(TEXT("and a claim speaks in it too"), CosmicLogFeed::ColorOf(1, Tag(TEXT("Log.Claim"))).Equals(Spirit));
	}

	// ---- A click on a line goes to its system ----
	{
		// The whole board known (Phase 3 item 8: the click goes to a known place only - UI.KnownFilter holds the gate).
		CosmicTests::KnowAll(Sim->GetMutableState());
		const FGalaxyState& State = Sim->GetState();
		const int32 RuledIndex = Sim->GetStateIndex().FindCiv(State, State.Player.RuledCivID);
		ACosmicCameraPawn* CameraPawn = Cast<ACosmicCameraPawn>(Controller->GetPawn());
		if (TestTrue(TEXT("(the board has a ruled people, a second system and the camera)"), State.Civs.IsValidIndex(RuledIndex) && State.Systems.IsValidIndex(1) && CameraPawn != nullptr))
		{
			const FName Second = State.Systems[1].SystemID;
			UCosmicLogItem* AtSystem = NewObject<UCosmicLogItem>(Feed);
			AtSystem->FocusSystemID = Second;
			AtSystem->FocusCivID = State.Player.RuledCivID;
			Feed->OpenItem(AtSystem);
			TestEqual(TEXT("A line of a system selects that system"), Controller->GetSelectedSystem(), Second);
			TestTrue(TEXT("and the camera goes there"), CameraPawn->GetTargetFocus().Equals(FVector(State.Systems[1].Position.X, State.Systems[1].Position.Y, 0.0), 0.01));

			UCosmicLogItem* OfPeople = NewObject<UCosmicLogItem>(Feed);
			OfPeople->FocusCivID = State.Player.RuledCivID;
			Feed->OpenItem(OfPeople);
			TestEqual(TEXT("A line of a people alone goes to its capital"), Controller->GetSelectedSystem(), State.Civs[RuledIndex].HomeSystemID);

			UCosmicLogItem* Nowhere = NewObject<UCosmicLogItem>(Feed);
			Controller->SelectSystem(Second);
			Feed->OpenItem(Nowhere);
			TestEqual(TEXT("A line of no place changes nothing"), Controller->GetSelectedSystem(), Second);
		}
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

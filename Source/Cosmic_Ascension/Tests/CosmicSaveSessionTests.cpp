// Cosmic Ascension - automation tests of the game's own saving (MVP item 5, commit A-5c): the tick-boundary autosave, the
// saves of input and exit, the gate, and the continue at the start of the game - a real map load. Design:
// docs/60_DECISIONS/headers/A5_save_minimum.md section 4, 5, 8; 16 #33 (1).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Events/CosmicPetitions.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayTagContainer.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformTime.h"
#include "InputCoreTypes.h"
#include "Internationalization/Text.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Save/CosmicSaveSlot.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/CosmicEndings.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "Tests/CosmicTestProbe.h"
#include "UI/CosmicChronicleWidget.h"
#include "UI/CosmicPetitionCardWidget.h"
#include "World/CosmicAutoPause.h"
#include "World/CosmicGameMode.h"

namespace CosmicSaveSessionTests
{
	UCosmicSaveSubsystem* SaveOf(UGalaxySimulationSubsystem& Sim)
	{
		return Sim.GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>();
	}

	int64 HeaderTickOf(const FString& Path)
	{
		TArray<uint8> Bytes;
		FFileHelper::LoadFileToArray(Bytes, *Path, FILEREAD_Silent);
		CosmicSaveSlot::FHeader Header;
		CosmicSaveSlot::ReadHeader(Bytes, Bytes.Num(), nullptr, Header);
		return Header.Tick;
	}

	void SetReadOnly(const FString& Path, bool bReadOnly)
	{
		FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*Path, bReadOnly);
	}

	// The continue tests' own slot folder, emptied (read-only leftovers of a killed run too): the newest run there is the one
	// the test wrote, whatever other tests and processes left under Automation/. The next AcquireSim sets Automation/ again.
	void UseOwnFolder(UCosmicSaveSubsystem& Save)
	{
		const FString Folder = TEXT("Automation/Continue/");
		Save.SetFolderForTest(Folder);
		const FString Directory = FPaths::ProjectSavedDir() / TEXT("SaveGames") / Folder;
		TArray<FString> Files;
		IFileManager::Get().FindFiles(Files, *Directory, TEXT(".sav"));
		for (const FString& File : Files)
		{
			IFileManager::Get().Delete(*(Directory / File), false, true, true);
		}
	}

	// The one world of the game, or null when the process has another set of worlds (the map load of the continue tests needs
	// exactly one game world context - the engine's own FLoadGameMapCommand checks the same).
	UWorld* TheGameWorld()
	{
		if (!GEngine || GEngine->GetWorldContexts().Num() != 1 || GEngine->GetWorldContexts()[0].WorldType != EWorldType::Game)
		{
			return nullptr;
		}
		return GEngine->GetWorldContexts()[0].World();
	}

	// Opens the game's map again (the engine travels on its next tick), then waits for the new world: another world, its
	// actors initialized, the match started, the first controller ours and its HUD with its chronicle - or TimeoutSeconds,
	// with a test error. FWaitForMapToLoadCommand is not used: it can see the old world and end in the same frame.
	void ReloadMapThen(FAutomationTestBase& Test, TFunction<void()> Then)
	{
		UWorld* Old = TheGameWorld();
		GEngine->Exec(Old, TEXT("Open /Game/CA/Maps/L_Galaxy"));
		const TWeakObjectPtr<UWorld> OldWorld(Old);
		const double Deadline = FPlatformTime::Seconds() + 15.0;
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([OldWorld, Deadline, &Test, Then]()
		{
			UWorld* World = TheGameWorld();
			const ACosmicPlayerController* Controller = World ? Cast<ACosmicPlayerController>(World->GetFirstPlayerController()) : nullptr;
			const ACosmicHUD* HUD = Controller ? Controller->GetHUD<ACosmicHUD>() : nullptr;
			const bool bReady = World && World != OldWorld.Get() && World->AreActorsInitialized() && World->GetGameState()
				&& World->GetGameState()->HasMatchStarted() && HUD && HUD->GetChronicle();
			if (!bReady && FPlatformTime::Seconds() < Deadline)
			{
				return false;
			}
			if (!bReady)
			{
				Test.AddError(TEXT("The map did not load again within 15 seconds."));
			}
			Then();
			return true;
		}));
	}
}

// The boundary rule of the save layer (16 #33 (1)): what each tick boundary writes and why, the saves asked at input time,
// a save asked from a handler waiting for the boundary, a failure kept and written again after the wait (noticed only while
// no write has put it right; the end of the run is written through the wait), the exit skipped only for the run of
// decision 6, and the gate - as pure functions, one case per reason; B5 (header section 2.6, 6): the causes Death and
// Succession and their place in the order (a leap of 0 is a succession at the death's boundary).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveTriggersTest, "Cosmic_Ascension.Sim.SaveTriggers", CosmicTests::Flags)
bool FCosmicSimSaveTriggersTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSaveSessionTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicSaveSubsystem* Save = SaveOf(*Sim);
	UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	AddExpectedError(TEXT("SaveCurrentGalaxy called from a handler of a Core Logic delegate"), EAutomationExpectedErrorFlags::Contains, 1);
	AddExpectedMessage(TEXT("CosmicSave: .* was not written"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 4);

	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	TArray<FGuid> Written;
	ON_SCOPE_EXIT
	{
		Save->PrepareForTests();
		for (const FGuid& ID : Written)
		{
			SetReadOnly(Save->SlotPathOf(ID), false);
			Save->DeleteSave(ID);
		}
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Save->SetGameSavingForTest(true);
	Save->SetTimingForTest(1e9, 0.0);
	Sim->SetBroadcastEnabled(true);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->SetTimeScale(0.f);
	FGalaxyState& State = Sim->GetMutableState();
	Written.Add(State.GalaxyID);
	const FString MainPath = Save->SlotPathOf(State.GalaxyID);
	int32 Count = Save->GetSavesWritten();
	auto Wrote = [&]()
	{
		const int32 Now = Save->GetSavesWritten();
		const int32 Delta = Now - Count;
		Count = Now;
		return Delta;
	};
	auto Last = [&]() { return Save->GetLastCause().Get(ECosmicSaveCause::Asked); };

	// ---- At the tick boundary ----
	Save->RequestSave(ECosmicSaveCause::NewRun);
	TestTrue(TEXT("A new run is written at once"), Wrote() == 1 && Last() == ECosmicSaveCause::NewRun);
	for (int32 Step = 0; Step < 5; ++Step)
	{
		Sim->StepOnce();
	}
	TestEqual(TEXT("Boundaries with nothing new write nothing"), Wrote(), 0);
	State.Calling.CompletedFeats.Add(FName(TEXT("feat_test")));
	Sim->StepOnce();
	TestTrue(TEXT("A feat is written at the boundary"), Wrote() == 1 && Last() == ECosmicSaveCause::Ascended);
	State.Player.Role = EPlayerRole::Wanderer;
	Sim->StepOnce();
	TestTrue(TEXT("A change of role is written at the boundary"), Wrote() == 1 && Last() == ECosmicSaveCause::Role);
	State.Player.Role = EPlayerRole::Ruler;
	State.Calling.CompletedFeats.Add(FName(TEXT("feat_test_two")));
	Sim->StepOnce();
	TestTrue(TEXT("A feat and a role in one boundary are one write"), Wrote() == 1 && Last() == ECosmicSaveCause::Ascended);
	Save->SetTimingForTest(0.0, 0.0);
	Sim->StepOnce();
	TestTrue(TEXT("Past the interval a moved tick is written"), Wrote() == 1 && Last() == ECosmicSaveCause::Interval);
	Save->SetTimingForTest(1e9, 0.0);

	// ---- At input time ----
	Save->RequestSave(ECosmicSaveCause::Exit);
	TestEqual(TEXT("An exit over a slot that holds this state writes nothing"), Wrote(), 0);
	Sim->StepOnce();
	Save->RequestSave(ECosmicSaveCause::Exit);
	TestTrue(TEXT("An exit after a change writes"), Wrote() == 1 && Last() == ECosmicSaveCause::Exit);

	// ---- From a handler: refused, kept for the boundary ----
	{
		CosmicTests::FScopedProbe Scoped(*Sim);
		bool bRefused = false;
		Scoped.Probe->OnLog = [&](const FCosmicLogEntry&)
		{
			bRefused = !Save->SaveCurrentGalaxy();
			Save->RequestSave(ECosmicSaveCause::Answer);
		};
		Log->Push(FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Civ"))), 1, FText::AsCultureInvariant(TEXT("told")), FFormatNamedArguments(), NAME_None, FName(TEXT("log_test")));
		Scoped.Probe->OnLog = nullptr;
		TestTrue(TEXT("A save from a handler is refused"), bRefused);
		TestEqual(TEXT("and a save asked there waits"), Wrote(), 0);
	}
	Sim->StepOnce();
	TestTrue(TEXT("The next boundary writes what a handler asked"), Wrote() == 1 && Last() == ECosmicSaveCause::Answer);

	// ---- The end ----
	State.Tick = Sim->GetRules().ProvisionalEndTick - 1;
	Sim->StepOnce();
	TestTrue(TEXT("The tick the run ends is written"), Wrote() == 1 && Last() == ECosmicSaveCause::Ended);

	// ---- A failure: noticed once, waited for, written again (the flag through input, no boundary - header section 4.5) ----
	Save->SetTimingForTest(1e9, 1e9);
	SetReadOnly(MainPath, true);
	Sim->StepOnce();   // a change for the exit's hash
	Save->RequestSave(ECosmicSaveCause::Answer);
	TestEqual(TEXT("A failed write writes nothing"), Wrote(), 0);
	SetReadOnly(MainPath, false);
	Save->RequestSave(ECosmicSaveCause::Answer);
	TestTrue(TEXT("An input after a failure writes at once"), Wrote() == 1 && Last() == ECosmicSaveCause::Answer);
	TestFalse(TEXT("A failure written over before it was noticed is not told"), Save->TakeFirstFailure());
	SetReadOnly(MainPath, true);
	Sim->StepOnce();
	Save->RequestSave(ECosmicSaveCause::Answer);
	TestEqual(TEXT("Another failed write writes nothing"), Wrote(), 0);
	TestTrue(TEXT("and is noticed once"), Save->TakeFirstFailure());
	TestFalse(TEXT("only once"), Save->TakeFirstFailure());
	SetReadOnly(MainPath, false);
	Sim->StepOnce();
	TestEqual(TEXT("Within the wait a boundary writes nothing"), Wrote(), 0);
	Save->SetTimingForTest(1e9, 0.0);
	Sim->StepOnce();
	TestTrue(TEXT("After the wait the boundary writes the failed answer"), Wrote() == 1 && Last() == ECosmicSaveCause::Answer);
	Save->SetTimingForTest(0.0, 0.0);
	SetReadOnly(MainPath, true);
	Sim->StepOnce();
	TestEqual(TEXT("A failed boundary write writes nothing"), Wrote(), 0);
	Save->SetTimingForTest(1e9, 0.0);
	SetReadOnly(MainPath, false);
	Sim->StepOnce();
	TestTrue(TEXT("and its cause is kept for the next boundary"), Wrote() == 1 && Last() == ECosmicSaveCause::Interval);

	// ---- Decision 6: the exit skipped for the held run alone ----
	Sim->NewGalaxy(7, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Written.Add(Sim->GetState().GalaxyID);
	Save->HoldStartRun();
	Save->RequestSave(ECosmicSaveCause::Exit);
	TestEqual(TEXT("The exit of the held new run at tick 0 writes nothing"), Wrote(), 0);
	Sim->NewGalaxy(8, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Written.Add(Sim->GetState().GalaxyID);
	Save->RequestSave(ECosmicSaveCause::Exit);
	TestTrue(TEXT("The exit of another new run writes"), Wrote() == 1 && Last() == ECosmicSaveCause::Exit);

	// ---- The end of the run is written through the wait: the latch stops the time in that tick ----
	{
		const FString OtherPath = Save->SlotPathOf(Sim->GetState().GalaxyID);
		Save->SetTimingForTest(1e9, 1e9);
		SetReadOnly(OtherPath, true);
		Sim->StepOnce();
		Save->RequestSave(ECosmicSaveCause::Answer);
		SetReadOnly(OtherPath, false);
		Sim->StepOnce();
		TestEqual(TEXT("(within the wait a boundary writes nothing)"), Wrote(), 0);
		Sim->GetMutableState().Tick = Sim->GetRules().ProvisionalEndTick - 1;
		Sim->StepOnce();
		TestTrue(TEXT("The tick the run ends is written within the wait"), Wrote() == 1 && Last() == ECosmicSaveCause::Ended);
		Save->SetTimingForTest(1e9, 0.0);
	}

	// ---- The gate ----
	TestFalse(TEXT("-NoSave closes it"), UCosmicSaveSubsystem::CommandLineAllows(TEXT("-NoSave")));
	TestFalse(TEXT("-unattended closes it"), UCosmicSaveSubsystem::CommandLineAllows(TEXT("-unattended")));
	TestFalse(TEXT("-Seed= closes it"), UCosmicSaveSubsystem::CommandLineAllows(TEXT("-Seed=42")));
	TestTrue(TEXT("a plain game opens it"), UCosmicSaveSubsystem::CommandLineAllows(TEXT("-game -windowed")));
	auto Closed = [](bool bAllows, bool bBlocked, bool bAutomation, bool bEditor, bool bOverride)
	{
		const TCHAR* By = UCosmicSaveSubsystem::GateClosedBy(bAllows, bBlocked, bAutomation, bEditor, bOverride);
		return By ? FString(By) : FString();
	};
	TestEqual(TEXT("The command line is a reason"), Closed(false, false, false, false, false), FString(TEXT("command line")));
	TestEqual(TEXT("the tests"), Closed(true, true, false, false, false), FString(TEXT("tests")));
	TestEqual(TEXT("an automation test running"), Closed(true, false, true, false, false), FString(TEXT("automation")));
	TestEqual(TEXT("the editor process"), Closed(true, false, false, true, false), FString(TEXT("editor")));
	TestEqual(TEXT("None of them: open"), Closed(true, false, false, false, false), FString());
	TestEqual(TEXT("A test's override opens it whatever the rest"), Closed(false, true, true, true, true), FString());
	Save->SetGameSavingForTest(false);
	State.Calling.CompletedFeats.Add(FName(TEXT("feat_test_three")));
	Sim->StepOnce();
	Save->RequestSave(ECosmicSaveCause::Answer);
	TestEqual(TEXT("With the gate closed nothing of the game's saving writes"), Wrote(), 0);

	// ---- The boundary rule, pure ----
	{
		FCosmicSavedMark Saved;
		Saved.GalaxyID = FGuid(1, 2, 3, 4);
		Saved.Tick = 10;
		Saved.RealSeconds = 100.0;
		FCosmicSavedMark Now = Saved;
		auto Due = [&](double At, double Interval) { return UCosmicSaveSubsystem::DueAt(Saved, Now, At, Interval); };
		TestFalse(TEXT("Nothing new: nothing due"), Due(1000.0, 600.0).IsSet());
		Now.Tick = 11;
		TestTrue(TEXT("A moved tick past the interval: Interval"), Due(700.0, 600.0) == ECosmicSaveCause::Interval);
		TestFalse(TEXT("within it: nothing"), Due(699.0, 600.0).IsSet());
		Now.RuledCivID = FName(TEXT("civ_9"));
		TestTrue(TEXT("Another ruled people: Role"), Due(0.0, 600.0) == ECosmicSaveCause::Role);
		Now.Feats = 1;
		TestTrue(TEXT("A feat before a role"), Due(0.0, 600.0) == ECosmicSaveCause::Ascended);
		Now.bEnded = true;
		TestTrue(TEXT("The end before a feat"), Due(0.0, 600.0) == ECosmicSaveCause::Ended);
		Now.GalaxyID = FGuid(5, 6, 7, 8);
		TestTrue(TEXT("Another galaxy before all"), Due(0.0, 600.0) == ECosmicSaveCause::NewRun);
	}

	// ---- The death and the succession, pure (B5 header section 2.6): NewRun > Ended > Ascended > Death > Succession > Role ----
	{
		// The disk copy of an embodied ruler of one life.
		FCosmicSavedMark Saved;
		Saved.GalaxyID = FGuid(1, 2, 3, 4);
		Saved.Tick = 10;
		Saved.RealSeconds = 100.0;
		Saved.Role = EPlayerRole::Ruler;
		Saved.RuledCivID = FName(TEXT("civ_0"));
		Saved.Lives = 1;
		auto Due = [&Saved](const FCosmicSavedMark& Now) { return UCosmicSaveSubsystem::DueAt(Saved, Now, 0.0, 600.0); };

		// A death: the calling entered the interregnum.
		FCosmicSavedMark Died = Saved;
		Died.Tick = 11;
		Died.bInterregnum = true;
		TestTrue(TEXT("The calling entered the interregnum: Death"), Due(Died) == ECosmicSaveCause::Death);
		Died.Role = EPlayerRole::Wanderer;
		Died.RuledCivID = NAME_None;
		TestTrue(TEXT("A ruler's death changes the role too: Death before Role"), Due(Died) == ECosmicSaveCause::Death);
		Died.Lives = 2;
		TestTrue(TEXT("Death before Succession"), Due(Died) == ECosmicSaveCause::Death);
		Died.Lives = 1;
		Died.Feats = 1;
		TestTrue(TEXT("A feat before a death"), Due(Died) == ECosmicSaveCause::Ascended);
		Died.Feats = 0;
		Died.bEnded = true;
		TestTrue(TEXT("The end before a death (a death on the end's tick)"), Due(Died) == ECosmicSaveCause::Ended);

		// A leap of 0: at the death's boundary the new life already stands - embodied, with one more life.
		FCosmicSavedMark Zero = Saved;
		Zero.Tick = 11;
		Zero.Lives = 2;
		TestTrue(TEXT("A leap of 0 (embodied with one more life): Succession"), Due(Zero) == ECosmicSaveCause::Succession);
		Zero.Role = EPlayerRole::Wanderer;
		Zero.RuledCivID = NAME_None;
		TestTrue(TEXT("The ruler died and the new vessel walks: Succession before Role"), Due(Zero) == ECosmicSaveCause::Succession);
		Zero.Lives = 1;
		TestTrue(TEXT("(the same marks without the life: Role)"), Due(Zero) == ECosmicSaveCause::Role);
		Zero.Lives = 2;
		Zero.Feats = 1;
		TestTrue(TEXT("A feat before a succession"), Due(Zero) == ECosmicSaveCause::Ascended);

		// The interregnum goes on, then a leap ends in a succession.
		FCosmicSavedMark InLeap = Saved;
		InLeap.bInterregnum = true;
		InLeap.Role = EPlayerRole::Wanderer;
		InLeap.RuledCivID = NAME_None;
		FCosmicSavedMark Later = InLeap;
		Later.Tick = 50;
		TestFalse(TEXT("The interregnum going on is nothing new"), UCosmicSaveSubsystem::DueAt(InLeap, Later, 0.0, 600.0).IsSet());
		Later.bInterregnum = false;
		Later.Lives = 2;
		TestTrue(TEXT("Out of the interregnum with one more life: Succession"), UCosmicSaveSubsystem::DueAt(InLeap, Later, 0.0, 600.0) == ECosmicSaveCause::Succession);
		Later.bEnded = true;
		TestTrue(TEXT("The end before a succession (a succession on the end's tick)"), UCosmicSaveSubsystem::DueAt(InLeap, Later, 0.0, 600.0) == ECosmicSaveCause::Ended);
		Later.bEnded = false;
		Later.bInterregnum = true;
		Later.Lives = 1;
		TestFalse(TEXT("The silence (no candidate: still in the interregnum, no life added) is nothing new"), UCosmicSaveSubsystem::DueAt(InLeap, Later, 0.0, 600.0).IsSet());
	}
	return true;
}

// The continue of an ended run (headers/A4 4.4 (5)): the game opened again on a run that was over stands over - the latch,
// the page fully shown, no card, the time still - through the two BeginPlay calls of a real map load.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIContinueEndedTest, "Cosmic_Ascension.UI.ContinueEnded", CosmicTests::Flags)
bool FCosmicUIContinueEndedTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSaveSessionTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	if (!TheGameWorld())
	{
		AddError(TEXT("The map load needs exactly one game world context."));
		return false;
	}
	UCosmicSaveSubsystem* Save = SaveOf(*Sim);
	const FGuid Ended(0x5A5E0010, 1, 1, 1);
	UseOwnFolder(*Save);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	FGalaxyState& State = Sim->GetMutableState();
	State.GalaxyID = Ended;
	State.Tick = Sim->GetRules().ProvisionalEndTick;
	State.Knowing.PendingPetitionRow = FName(TEXT("petition_walls"));
	if (!TestTrue(TEXT("(the ended run is saved)"), Save->SaveCurrentGalaxy()))
	{
		return false;
	}
	const uint32 Hash = Sim->ComputeStateHash();
	Sim->NewGalaxy(7, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Save->SetGameSavingForTest(true);
	Save->ContinueAtNextStartForTest();
	UCosmicSaveSubsystem* SaveLater = Save;
	ReloadMapThen(*this, [this, Sim, SaveLater, Ended, Hash]()
	{
		UWorld* World = TheGameWorld();
		ACosmicPlayerController* Controller = World ? Cast<ACosmicPlayerController>(World->GetFirstPlayerController()) : nullptr;
		ACosmicHUD* HUD = Controller ? Controller->GetHUD<ACosmicHUD>() : nullptr;
		if (TestNotNull(TEXT("(the new controller and HUD)"), HUD))
		{
			TestTrue(TEXT("The game opens the ended run"), Sim->GetState().GalaxyID == Ended);
			TestEqual(TEXT("as it was saved"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Hash));
			TestTrue(TEXT("from its slot"), SaveLater->GetLastContinue() == ECosmicContinue::Newest);
			TestEqual(TEXT("over: the latch"), Controller->GetRunEnd(), ECosmicProvisionalEnd::Clock);
			TestTrue(TEXT("the page open and fully shown"), HUD->GetChronicle()->IsOpen() && !HUD->GetChronicle()->IsRevealing());
			TestTrue(TEXT("no card"), HUD->GetPetitionCard() && HUD->GetPetitionCard()->GetShownRow().IsNone());
			TestEqual(TEXT("(its row still waits)"), Sim->GetState().Knowing.PendingPetitionRow, FName(TEXT("petition_walls")));
			TestEqual(TEXT("the time still"), Sim->GetTimeScale(), 0.f);
			TestTrue(TEXT("for the end"), Controller->GetAutoPauseReason() == ECosmicAutoPause::RunEnd);
		}
		SaveLater->SetGameSavingForTest(false);
		SaveLater->DeleteSave(Ended);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	});
	return true;
}

// The continue of a running run (decision 1): the game opened again on a run that was going stands still with its line and
// its card; an answer is saved at once, and a write that fails stops the time once and is written again at the boundary
// after the wait; Space runs it again; closing the window writes the run.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIContinueRunningTest, "Cosmic_Ascension.UI.ContinueRunning", CosmicTests::Flags)
bool FCosmicUIContinueRunningTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSaveSessionTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	if (!TheGameWorld())
	{
		AddError(TEXT("The map load needs exactly one game world context."));
		return false;
	}
	AddExpectedMessage(TEXT("CosmicSave: .* was not written"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);
	UCosmicSaveSubsystem* Save = SaveOf(*Sim);
	const FGuid Running(0x5A5E0011, 1, 1, 1);
	const FName Card = CosmicRows::PetitionWiseHeart;
	UseOwnFolder(*Save);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(40);
	FGalaxyState& State = Sim->GetMutableState();
	State.GalaxyID = Running;
	State.Knowing.PendingPetitionRow = Card;
	State.Knowing.PendingPetitionSystemID = NAME_None;
	// The stockpile pays any choice: the answer is taken.
	const int32 Ruled = CosmicPetitions::RuledCivIndexOf(State, Sim->GetStateIndex());
	if (State.Civs.IsValidIndex(Ruled))
	{
		FResourcePool& Stock = State.Civs[Ruled].Stockpile;
		Stock.Energy = Stock.Matter = Stock.Insight = Stock.Influence = Stock.Causality = 100000.f;
	}
	if (!TestTrue(TEXT("(the running run is saved)"), Save->SaveCurrentGalaxy()))
	{
		return false;
	}
	const uint32 Hash = Sim->ComputeStateHash();
	Sim->NewGalaxy(7, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Save->SetGameSavingForTest(true);
	Save->ContinueAtNextStartForTest();
	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	UCosmicSaveSubsystem* SaveLater = Save;
	ReloadMapThen(*this, [this, Sim, SaveLater, Running, Card, Hash, bWasBroadcasting]()
	{
		UWorld* World = TheGameWorld();
		ACosmicPlayerController* Controller = World ? Cast<ACosmicPlayerController>(World->GetFirstPlayerController()) : nullptr;
		ACosmicHUD* HUD = Controller ? Controller->GetHUD<ACosmicHUD>() : nullptr;
		const FString MainPath = SaveLater->SlotPathOf(Running);
		if (TestNotNull(TEXT("(the new controller and HUD)"), HUD))
		{
			TestTrue(TEXT("The game opens the running run"), Sim->GetState().GalaxyID == Running);
			TestEqual(TEXT("as it was saved"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Hash));
			TestEqual(TEXT("standing still"), Sim->GetTimeScale(), 0.f);
			TestTrue(TEXT("with its line"), Controller->GetAutoPauseReason() == ECosmicAutoPause::Continued);
			TestTrue(TEXT("and its card"), HUD->GetPetitionCard() && HUD->GetPetitionCard()->GetShownRow() == Card);

			// An answer is saved at once; a failed write stops the running time once and waits for the boundary after the wait.
			const int32 Before = SaveLater->GetSavesWritten();
			Sim->SetTimeScale(1.f);
			SetReadOnly(MainPath, true);
			const FCosmicPetitionAnswer Answer = Controller->AnswerPetition(1);
			SetReadOnly(MainPath, false);
			TestTrue(TEXT("(the answer is taken)"), Answer.Outcome != ECosmicPetitionOutcome::Refused);
			TestEqual(TEXT("A failed save of the answer writes nothing"), SaveLater->GetSavesWritten(), Before);
			TestEqual(TEXT("and stops the time"), Sim->GetTimeScale(), 0.f);
			TestTrue(TEXT("saying so"), Controller->GetAutoPauseReason() == ECosmicAutoPause::SaveFailed);
			SaveLater->SetTimingForTest(1e9, 0.0);
			Sim->SetBroadcastEnabled(true);
			Sim->StepOnce();
			Sim->SetBroadcastEnabled(bWasBroadcasting);
			TestTrue(TEXT("The next boundary writes the answer"), SaveLater->GetSavesWritten() == Before + 1 && SaveLater->GetLastCause() == ECosmicSaveCause::Answer);
			TestEqual(TEXT("(the slot holds this tick)"), HeaderTickOf(MainPath), Sim->GetState().Tick);
			Controller->PressSpeedKey(EKeys::SpaceBar);
			TestEqual(TEXT("Space runs it again"), Sim->GetTimeScale(), 1.f);
			TestTrue(TEXT("and the line goes"), Controller->GetAutoPauseReason() == ECosmicAutoPause::None);

			// Closing the window is the exit save (the controller ends as Destroyed, not Quit): a changed run is written, once.
			UGameViewportClient* Viewport = World->GetGameViewport();
			if (TestNotNull(TEXT("(the game viewport)"), Viewport))
			{
				Sim->StepOnce();
				const int32 AtClose = SaveLater->GetSavesWritten();
				Viewport->OnCloseRequested().Broadcast(Viewport->Viewport);
				TestTrue(TEXT("Closing the window writes the run"), SaveLater->GetSavesWritten() == AtClose + 1 && SaveLater->GetLastCause() == ECosmicSaveCause::Exit);
				Viewport->OnCloseRequested().Broadcast(Viewport->Viewport);
				TestEqual(TEXT("and only once"), SaveLater->GetSavesWritten(), AtClose + 1);
			}
		}
		SetReadOnly(MainPath, false);
		SaveLater->SetGameSavingForTest(false);
		SaveLater->DeleteSave(Running);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	});
	return true;
}

// The continue of a run in the interregnum (B5 header section 2.6; the form of UI.ContinueRunning): the game opened again on
// a run whose vessel died with its leap ahead opens it as it was saved and stands still with the continue's line - the time
// does not run the leap on its own - through the start path of a real map load (StartPlay's BeginSession and ContinueNewest,
// the controller's FollowStart taking TakeStartOutcome).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveLeapContinueTest, "Cosmic_Ascension.Sim.SaveLeapContinue", CosmicTests::Flags)
bool FCosmicSimSaveLeapContinueTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSaveSessionTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	if (!TheGameWorld())
	{
		AddError(TEXT("The map load needs exactly one game world context."));
		return false;
	}
	UCosmicSaveSubsystem* Save = SaveOf(*Sim);
	const FGuid Waiting(0x5A5E0012, 1, 1, 1);
	UseOwnFolder(*Save);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	FGalaxyState& State = Sim->GetMutableState();
	State.GalaxyID = Waiting;
	State.Tick = 399;
	Sim->SyncRngState();
	// A death recorded as Apply records one (a hazard's expiry): the tenth stage of the next tick carries it out.
	State.Calling.PendingEffect = CosmicTags::EffectKillPlayer();
	State.Calling.PendingCause = FGameplayTag::RequestGameplayTag(FName(TEXT("Death.Hazard")));
	State.Calling.PendingSourceRow = FName(TEXT("crisis_pre_supernova"));
	Sim->StepOnce();
	if (!TestTrue(TEXT("(the vessel died at tick 400 with its leap ahead)"), State.Tick == 400 && State.Calling.LifeState == ELifeState::Interregnum
		&& State.Calling.PendingLeapTicks > 0 && State.Calling.Lives.Num() == 1))
	{
		return false;
	}
	const int64 Leap = State.Calling.PendingLeapTicks;
	if (!TestTrue(TEXT("(the run in the interregnum is saved)"), Save->SaveCurrentGalaxy()))
	{
		return false;
	}
	const uint32 Hash = Sim->ComputeStateHash();
	Sim->NewGalaxy(7, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Save->SetGameSavingForTest(true);
	Save->ContinueAtNextStartForTest();
	UCosmicSaveSubsystem* SaveLater = Save;
	ReloadMapThen(*this, [this, Sim, SaveLater, Waiting, Hash, Leap]()
	{
		UWorld* World = TheGameWorld();
		ACosmicPlayerController* Controller = World ? Cast<ACosmicPlayerController>(World->GetFirstPlayerController()) : nullptr;
		if (TestNotNull(TEXT("(the new controller)"), Controller))
		{
			const FCallingState& Calling = Sim->GetState().Calling;
			TestTrue(TEXT("The game opens the run in the interregnum"), Sim->GetState().GalaxyID == Waiting);
			TestEqual(TEXT("as it was saved"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Hash));
			TestTrue(TEXT("from its slot"), SaveLater->GetLastContinue() == ECosmicContinue::Newest);
			TestTrue(TEXT("The calling waits in the interregnum with its whole leap ahead"), Calling.LifeState == ELifeState::Interregnum && Calling.PendingLeapTicks == Leap
				&& Calling.Lives.Num() == 1);
			TestFalse(TEXT("(the run is not over)"), Controller->IsRunEnded());
			TestEqual(TEXT("A continued run in the interregnum stands still"), Sim->GetTimeScale(), 0.f);
			TestTrue(TEXT("with the continue's line"), Controller->GetAutoPauseReason() == ECosmicAutoPause::Continued);
		}
		SaveLater->SetGameSavingForTest(false);
		SaveLater->DeleteSave(Waiting);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	});
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

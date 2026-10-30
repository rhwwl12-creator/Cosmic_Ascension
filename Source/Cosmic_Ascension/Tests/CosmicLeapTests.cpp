// Cosmic Ascension - automation tests of the interregnum's leap (B5, commit [B-5b]): the tenth stage inside the stage
// guard, the tick boundary that runs no tick, the fast-forward (UGalaxySimulationSubsystem::RunLeap through
// ACosmicPlayerController::AdvanceLeap) against the leap one tick at a time, the saves of a death and of the middle of an
// interregnum, and the one boundary at the end of a leap (the end's latch, the slot's cause, the feed, the time stop).
// Design: docs/60_DECISIONS/headers/B5_death_succession_two_books.md section 2.2, 2.6, 6.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Core/CosmicIDs.h"
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "Misc/ScopeExit.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/CosmicEndings.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/SharedPointer.h"
#include "Tests/CosmicTestHelpers.h"
#include "Tests/CosmicTestProbe.h"
#include "UI/CosmicLogFeedWidget.h"
#include "Widgets/SWidget.h"
#include "World/CosmicAutoPause.h"
#include "World/CosmicGameMode.h"

namespace CosmicLeapTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// The source row a test's death names: a hazard row of the game, recorded by hand (the precursor kills nobody in the game's
	// data - its one death is the AI awakening's own expiry, [B-5d2]).
	const FName DeathRow(TEXT("crisis_pre_supernova"));

	// The game's rules with the interregnum's leap drawn between Min and Max (one number either way - decision 3).
	FCosmicRulesDef LeapRules(const FCosmicRulesDef& Base, int32 Min, int32 Max)
	{
		FCosmicRulesDef Rules = Base;
		Rules.LeapTicksMin = Min;
		Rules.LeapTicksMax = Max;
		return Rules;
	}

	// A death recorded on the live state as Apply records one (Effect.KillPlayer with its cause and source row): the tenth
	// stage of the next tick carries it out (UPlayerRoleSubsystem::Die).
	void RecordDeath(FGalaxyState& State)
	{
		State.Calling.PendingEffect = CosmicTags::EffectKillPlayer();
		State.Calling.PendingCause = Tag(TEXT("Death.Hazard"));
		State.Calling.PendingSourceRow = DeathRow;
	}

	// The game's controller - no widget of it is read.
	ACosmicPlayerController* ControllerOf(UGalaxySimulationSubsystem& Sim)
	{
		UWorld* World = Sim.GetGameInstance() ? Sim.GetGameInstance()->GetWorld() : nullptr;
		return Cast<ACosmicPlayerController>(World ? World->GetFirstPlayerController() : nullptr);
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

	bool IsWaiting(const FGalaxyState& State)
	{
		return State.Calling.LifeState == ELifeState::Interregnum;
	}

	// One tick at a time while the calling waits in the interregnum with a leap left; how many ticks that took.
	int64 StepThroughLeap(UGalaxySimulationSubsystem& Sim)
	{
		int64 Steps = 0;
		while (IsWaiting(Sim.GetState()) && Sim.GetState().Calling.PendingLeapTicks > 0 && Steps < 100000)
		{
			Sim.StepOnce();
			++Steps;
		}
		return Steps;
	}

	// Handlers of OnSimTick a test can bind without a UFUNCTION of its own: the simulation's own RunLeap and RunHeadless,
	// UFUNCTIONs of one int64 like the delegate. At a boundary each is refused with one error line ("... called from a
	// handler of OnSimTick") - so the error lines also count the boundaries a call sends. Unbound when the scope ends.
	struct FBoundaryRunners
	{
		FBoundaryRunners(UGalaxySimulationSubsystem& InSim, bool bLeap, bool bHeadless)
			: Sim(InSim)
		{
			if (bLeap)
			{
				Sim.OnSimTick.AddUniqueDynamic(&Sim, &UGalaxySimulationSubsystem::RunLeap);
			}
			if (bHeadless)
			{
				Sim.OnSimTick.AddUniqueDynamic(&Sim, &UGalaxySimulationSubsystem::RunHeadless);
			}
		}

		~FBoundaryRunners()
		{
			Sim.OnSimTick.RemoveDynamic(&Sim, &UGalaxySimulationSubsystem::RunLeap);
			Sim.OnSimTick.RemoveDynamic(&Sim, &UGalaxySimulationSubsystem::RunHeadless);
		}

		UGalaxySimulationSubsystem& Sim;
	};

	// The writes of the save layer since the last look (Sim.SaveTriggers' count).
	struct FWrites
	{
		explicit FWrites(UCosmicSaveSubsystem& InSave)
			: Save(InSave)
			, Count(InSave.GetSavesWritten())
		{
		}

		int32 Take()
		{
			const int32 Now = Save.GetSavesWritten();
			const int32 Delta = Now - Count;
			Count = Now;
			return Delta;
		}

		// Exactly one write since the last look, for Cause.
		bool One(ECosmicSaveCause Cause)
		{
			return Take() == 1 && Save.GetLastCause().Get(ECosmicSaveCause::Asked) == Cause;
		}

		UCosmicSaveSubsystem& Save;
		int32 Count;
	};

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

	// The newest told entry of the log, null for none.
	const FCosmicLogEntry* NewestTold(const FGalaxyState& State)
	{
		for (int32 Index = State.Log.Num() - 1; Index >= 0; --Index)
		{
			if (!State.Log[Index].bUntold)
			{
				return &State.Log[Index];
			}
		}
		return nullptr;
	}

	// The first settled system of a people the vessel does not rule (where a plague's expiry is told about another people),
	// INDEX_NONE for none.
	int32 OtherPeoplesSettled(const FGalaxyState& State)
	{
		for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num(); ++SystemIndex)
		{
			const FStarSystemState& System = State.Systems[SystemIndex];
			if (CosmicState::IsSettled(System) && System.OwnerCivID != State.Player.RuledCivID)
			{
				return SystemIndex;
			}
		}
		return INDEX_NONE;
	}
}

// The leap's determinism and the widened guard (header section 2.2, 2.6; section 6 Calling.LeapDeterminism): the tenth
// stage runs inside the stage guard (a handler of the deposition's line, and of the death line - [B-5d1] -, which that stage
// tells, reads IsInTickStages); a
// handler of OnSimTick that calls RunLeap or RunHeadless is refused and moves no tick - at a tick's boundary and at RunLeap's
// own; the same seed and the same death tick give the same state one tick at a time through the leap, through the
// controller's AdvanceLeap and through RunLeap of the remaining leap.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLeapDeterminismTest, "Cosmic_Ascension.Calling.LeapDeterminism", CosmicTests::Flags)
bool FCosmicLeapDeterminismTest::RunTest(const FString& Parameters)
{
	using namespace CosmicLeapTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	ACosmicPlayerController* Controller = ControllerOf(*Sim);
	if (!TestNotNull(TEXT("The first player controller is the game's"), Controller))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	ON_SCOPE_EXIT
	{
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	// One RunLeap refused at a tick's boundary; RunHeadless there and at RunLeap's boundary.
	AddExpectedError(TEXT("RunLeap called from a handler of OnSimTick"), EAutomationExpectedErrorFlags::Contains, 1);
	AddExpectedError(TEXT("RunHeadless called from a handler of OnSimTick"), EAutomationExpectedErrorFlags::Contains, 2);
	Sim->SetTables(CosmicTests::WithRules(Original, LeapRules(Sim->GetRules(), 30, 60)));
	Sim->SetBroadcastEnabled(true);

	// ---- The tenth stage runs inside the stage guard; the boundary runs no tick ----
	{
		FGalaxyState& Live = LayAt(*Sim, 400);
		Live.Calling.PendingEffect = Tag(TEXT("Effect.DeposeRuler"));
		Live.Calling.PendingCause = Tag(TEXT("Reign.Deposed"));
		bool bHeard = false;
		bool bInStages = false;
		bool bSending = true;
		{
			CosmicTests::FScopedProbe Scoped(*Sim);
			Scoped.Probe->OnLog = [&bHeard, &bInStages, &bSending, Sim](const FCosmicLogEntry& Entry)
			{
				if (!bHeard && Entry.SourceRow == CosmicRows::LogFallenMonarch)
				{
					bHeard = true;
					bInStages = Sim->IsInTickStages();
					bSending = Sim->IsSendingSimTick();
				}
			};
			Sim->StepOnce();
			Scoped.Probe->OnLog = nullptr;
		}
		TestTrue(TEXT("(the deposition is carried out at the tenth stage, its line told)"), bHeard && Live.Player.Role == EPlayerRole::Wanderer);
		TestTrue(TEXT("The tenth stage runs inside the stage guard: a handler of its line reads IsInTickStages true"), bInStages);
		TestFalse(TEXT("and no OnSimTick is being sent while it runs"), bSending);
		TestFalse(TEXT("Both are down once the tick has returned"), Sim->IsInTickStages() || Sim->IsSendingSimTick());

		const int64 Before = Live.Tick;
		{
			FBoundaryRunners Runners(*Sim, true, true);
			Sim->StepOnce();
		}
		TestEqual(TEXT("A handler of OnSimTick that calls RunLeap and RunHeadless is refused: one StepOnce, one tick"), Live.Tick, Before + 1);
		{
			FBoundaryRunners Runners(*Sim, false, true);
			Sim->RunLeap(3);
		}
		TestEqual(TEXT("RunLeap's one boundary refuses them too: three ticks and no more"), Live.Tick, Before + 4);
	}

	// ---- [B-5d1]: the death line is told inside the stage guard too ----
	{
		FGalaxyState& Live = LayAt(*Sim, 400);
		RecordDeath(Live);
		bool bHeard = false;
		bool bInStages = false;
		{
			CosmicTests::FScopedProbe Scoped(*Sim);
			Scoped.Probe->OnLog = [&bHeard, &bInStages, Sim](const FCosmicLogEntry& Entry)
			{
				if (!bHeard && Entry.SourceRow == CosmicRows::LogDeathHazard)
				{
					bHeard = true;
					bInStages = Sim->IsInTickStages();
				}
			};
			Sim->StepOnce();
			Scoped.Probe->OnLog = nullptr;
		}
		TestTrue(TEXT("(the death is carried out at the tenth stage, its line told)"), bHeard && IsWaiting(Live));
		TestTrue(TEXT("A handler of the death line reads IsInTickStages true"), bInStages);
	}

	// ---- One tick at a time = AdvanceLeap = RunLeap of the remaining leap ----
	// A ruler's death at tick 400 on a new galaxy of the same seed (its regent, its lock, its leap).
	auto DieAt400 = [Sim]() -> FGalaxyState&
	{
		FGalaxyState& State = LayAt(*Sim, 400);
		RecordDeath(State);
		Sim->StepOnce();
		return State;
	};
	FGalaxyState& Live = DieAt400();
	if (!TestTrue(TEXT("(the vessel died at tick 400 with a leap ahead)"), Live.Tick == 400 && IsWaiting(Live) && Live.Calling.PendingLeapTicks > 0))
	{
		return false;
	}
	const int64 Leap = Live.Calling.PendingLeapTicks;
	TestTrue(TEXT("(the leap is drawn in the test's range)"), Leap >= 30 && Leap <= 60);
	TestEqual(TEXT("One tick at a time the leap lasts its length"), StepThroughLeap(*Sim), Leap);
	if (!TestTrue(TEXT("and ends in a succession on its last tick"), !IsWaiting(Live) && Live.Calling.Lives.Num() == 2 && Live.Calling.Lives.Last().BornTick == 400 + Leap))
	{
		return false;
	}
	const int64 LeapEnd = Live.Tick;
	const uint32 OneByOne = Sim->ComputeStateHash();
	const uint32 OneByOneWorld = Sim->ComputeWorldHash();

	DieAt400();
	TestEqual(TEXT("The same seed and death tick draw the same leap"), Live.Calling.PendingLeapTicks, Leap);
	TestTrue(TEXT("The controller's AdvanceLeap runs the leap"), Controller->AdvanceLeap());
	TestEqual(TEXT("to the same tick"), Live.Tick, LeapEnd);
	TestEqual(TEXT("The fast-forward is the same state as the leap one tick at a time"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(OneByOne));
	TestEqual(TEXT("and the same world"), static_cast<int64>(Sim->ComputeWorldHash()), static_cast<int64>(OneByOneWorld));
	TestFalse(TEXT("Outside the interregnum AdvanceLeap runs nothing"), Controller->AdvanceLeap());
	TestEqual(TEXT("(the tick stays)"), Live.Tick, LeapEnd);

	DieAt400();
	Sim->RunLeap(Live.Calling.PendingLeapTicks);
	TestEqual(TEXT("RunLeap of the remaining leap is the same state too"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(OneByOne));
	return true;
}

// The saves of an interregnum (header section 2.6; section 6 Sim.SaveLeapResume, docs/40's Calling.LeapResumeRoundTrip): the
// boundary of a death writes the slot for the death; a save right after the death and a save in the middle of the
// interregnum each load and, the leap continued (one tick at a time, then AdvanceLeap), give the state of the uninterrupted
// run; the disk copy holds the lives and the interregnum (MarkOf at the write and the load) - the loaded interregnum's
// boundaries write nothing, the succession's boundary writes Succession.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSaveLeapResumeTest, "Cosmic_Ascension.Sim.SaveLeapResume", CosmicTests::Flags)
bool FCosmicSaveLeapResumeTest::RunTest(const FString& Parameters)
{
	using namespace CosmicLeapTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	ACosmicPlayerController* Controller = ControllerOf(*Sim);
	UCosmicSaveSubsystem* Save = Sim->GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>();
	if (!TestNotNull(TEXT("The first player controller is the game's"), Controller) || !TestNotNull(TEXT("The save subsystem exists"), Save))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	FGuid Galaxy;
	ON_SCOPE_EXIT
	{
		Save->PrepareForTests();
		if (Galaxy.IsValid())
		{
			Save->DeleteSave(Galaxy);
		}
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Sim->SetTables(CosmicTests::WithRules(Original, LeapRules(Sim->GetRules(), 40, 40)));
	Sim->SetBroadcastEnabled(true);
	Save->SetGameSavingForTest(true);
	Save->SetTimingForTest(1e9, 0.0);
	FGalaxyState& Live = LayAt(*Sim, 400);
	Galaxy = Live.GalaxyID;
	FWrites Writes(*Save);
	Save->RequestSave(ECosmicSaveCause::NewRun);
	TestTrue(TEXT("(the new run is written)"), Writes.One(ECosmicSaveCause::NewRun));

	// The death, and the game's own save of it at the boundary.
	RecordDeath(Live);
	Sim->StepOnce();
	if (!TestTrue(TEXT("(the vessel died at tick 400 with its leap of 40 ahead)"), Live.Tick == 400 && IsWaiting(Live) && Live.Calling.PendingLeapTicks == 40))
	{
		return false;
	}
	TestTrue(TEXT("The death's boundary writes the slot once, for the death"), Writes.One(ECosmicSaveCause::Death));
	const uint32 AtDeath = Sim->ComputeStateHash();

	// The uninterrupted run, with the game's saving closed so that the slot keeps the death.
	Save->SetGameSavingForTest(false);
	TestTrue(TEXT("AdvanceLeap runs the uninterrupted leap"), Controller->AdvanceLeap());
	if (!TestTrue(TEXT("(to the succession at tick 440)"), Live.Tick == 440 && !IsWaiting(Live) && Live.Calling.Lives.Num() == 2))
	{
		return false;
	}
	const uint32 Straight = Sim->ComputeStateHash();
	Save->SetGameSavingForTest(true);

	// (1) The save right after the death: loaded, then the leap continued - a part one tick at a time, the rest at once.
	TestTrue(TEXT("The slot of the death loads"), Save->LoadGalaxy(Galaxy));
	TestEqual(TEXT("with the state of the death"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(AtDeath));
	{
		const FCosmicSavedMark Mark = UCosmicSaveSubsystem::MarkOf(Sim->GetState(), Sim->GetRules());
		TestTrue(TEXT("Its disk copy holds the interregnum"), Mark.bInterregnum);
		TestEqual(TEXT("and one life"), Mark.Lives, 1);
	}
	for (int32 Step = 0; Step < 15; ++Step)
	{
		Sim->StepOnce();
	}
	TestEqual(TEXT("The loaded interregnum's boundaries write nothing (the load's disk copy holds the interregnum)"), Writes.Take(), 0);
	TestEqual(TEXT("(the leap counts down)"), Live.Calling.PendingLeapTicks, static_cast<int64>(25));

	// (2) A save in the middle of the interregnum.
	TestTrue(TEXT("A save in the middle of the interregnum is written"), Save->SaveCurrentGalaxy());
	Writes.Take();
	const uint32 Middle = Sim->ComputeStateHash();
	// The death's save continued to its end with the game's saving closed, so that the slot keeps the middle (the
	// succession's write would replace it); the succession's write is counted on the middle's continuation below.
	Save->SetGameSavingForTest(false);
	TestTrue(TEXT("AdvanceLeap continues the loaded leap"), Controller->AdvanceLeap());
	TestEqual(TEXT("to the tick of the uninterrupted run"), Live.Tick, static_cast<int64>(440));
	TestEqual(TEXT("The death's save, loaded and continued, is the uninterrupted run's state"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Straight));
	Save->SetGameSavingForTest(true);

	TestTrue(TEXT("The slot of the middle loads"), Save->LoadGalaxy(Galaxy));
	TestEqual(TEXT("with the state of the middle"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Middle));
	{
		const FCosmicSavedMark Mark = UCosmicSaveSubsystem::MarkOf(Sim->GetState(), Sim->GetRules());
		TestTrue(TEXT("Its disk copy holds the interregnum"), Mark.bInterregnum);
		TestEqual(TEXT("and one life"), Mark.Lives, 1);
		TestEqual(TEXT("(with 25 ticks of the leap left)"), Live.Calling.PendingLeapTicks, static_cast<int64>(25));
	}
	TestTrue(TEXT("AdvanceLeap continues the loaded leap"), Controller->AdvanceLeap());
	TestEqual(TEXT("to the tick of the uninterrupted run"), Live.Tick, static_cast<int64>(440));
	TestEqual(TEXT("The middle's save, loaded and continued, is the uninterrupted run's state"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Straight));
	TestTrue(TEXT("Its succession's boundary writes the slot once, for the succession"), Writes.One(ECosmicSaveCause::Succession));
	return true;
}

// The one boundary at the end of a leap, through the controller with no widget of its own read (header section 2.6;
// section 6 UI.LeapBoundary, the form of UI.RunEnd): LifeClosedAt as a pure function; a death's boundary stops the time
// whatever the automatic pause and writes Death; a leap the end cuts stops at the end's tick, latches the end (Clock) and
// writes Ended once; a leap that succeeds before the end sends one boundary, writes Succession once and rebuilds the log
// feed, whose last item is then the inauguration line ([B-5d1]) - a leap of one tick too, whose boundary follows the feed's last tick by one; with the end switched off the leap runs past the end and nothing latches; the silence runs to the end with the end
// ahead and nothing with the switch off; a leap of 0 and a death on the end's tick stop the time at their boundary too; a
// succession on the end's tick - through a leap that ends there and through a leap of 0 - leaves the feed's last item the
// line before the inauguration line of the vessel the end installed (CosmicChronicle::IsUnseenLifeLine, [B-5d1]), and the
// end forgotten shows it again.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLeapBoundaryTest, "Cosmic_Ascension.UI.LeapBoundary", CosmicTests::Flags)
bool FCosmicLeapBoundaryTest::RunTest(const FString& Parameters)
{
	using namespace CosmicLeapTests;
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
	ACosmicPlayerController* Controller = ControllerOf(*Sim);
	UCosmicSaveSubsystem* Save = Sim->GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>();
	if (!TestNotNull(TEXT("The first player controller is the game's"), Controller) || !TestNotNull(TEXT("The save subsystem exists"), Save))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef GameRules = Sim->GetRules();
	const bool bViewerChoice = Controller->IsAutoPauseOn();
	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	TArray<FGuid> Written;
	ON_SCOPE_EXIT
	{
		Save->PrepareForTests();
		for (const FGuid& ID : Written)
		{
			Save->DeleteSave(ID);
		}
		Controller->SetAutoPause(bViewerChoice);
		Controller->SetProvisionalEndEnabled(true);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	// The one boundary of case (3), counted by a refused handler.
	AddExpectedError(TEXT("RunHeadless called from a handler of OnSimTick"), EAutomationExpectedErrorFlags::Contains, 1);

	// ---- LifeClosedAt, pure: the last closed life's DiedTick ----
	{
		FGalaxyState Bare;
		TestFalse(TEXT("No life: none closed"), ACosmicPlayerController::LifeClosedAt(Bare, 100));
		Bare.Calling.Lives.AddDefaulted();
		TestFalse(TEXT("One embodied life: none closed"), ACosmicPlayerController::LifeClosedAt(Bare, 100));
		Bare.Calling.Lives[0].DiedTick = 100;
		Bare.Calling.LifeState = ELifeState::Interregnum;
		TestTrue(TEXT("In the interregnum the last life closed at its DiedTick"), ACosmicPlayerController::LifeClosedAt(Bare, 100));
		TestFalse(TEXT("and at no other tick"), ACosmicPlayerController::LifeClosedAt(Bare, 101));
		FLifeRecord& Next = Bare.Calling.Lives.AddDefaulted_GetRef();
		Next.BornTick = 100;
		Bare.Calling.LifeState = ELifeState::Embodied;
		TestTrue(TEXT("A leap of 0: the life before the embodied one closed at the tick the next began"), ACosmicPlayerController::LifeClosedAt(Bare, 100));
		TestFalse(TEXT("and at no other tick"), ACosmicPlayerController::LifeClosedAt(Bare, 101));
		Next.DiedTick = 200;
		Bare.Calling.LifeState = ELifeState::Interregnum;
		TestTrue(TEXT("A second death: the last life is read in the interregnum"), ACosmicPlayerController::LifeClosedAt(Bare, 200));
		TestFalse(TEXT("not the one before it"), ACosmicPlayerController::LifeClosedAt(Bare, 100));
	}

	Controller->SetAutoPause(false);
	Sim->SetBroadcastEnabled(true);
	Save->SetGameSavingForTest(true);
	Save->SetTimingForTest(1e9, 0.0);
	const FCosmicDataTables Fixed = CosmicTests::WithRules(Original, LeapRules(GameRules, 40, 40));
	Sim->SetTables(Fixed);
	const int64 EndTick = Sim->GetRules().ProvisionalEndTick;
	if (!TestTrue(TEXT("(the clock end lies past the cases' deaths)"), EndTick > 500))
	{
		return false;
	}
	FWrites Writes(*Save);
	// A galaxy laid for a death at the tick At, its new run written: the boundaries below compare with that disk copy.
	auto Lay = [&](int64 At) -> FGalaxyState&
	{
		FGalaxyState& State = LayAt(*Sim, At);
		Written.Add(State.GalaxyID);
		Save->RequestSave(ECosmicSaveCause::NewRun);
		Writes.Take();
		return State;
	};
	// The death at the next tick's boundary, with the time running at x1.
	auto Kill = [&](FGalaxyState& State)
	{
		RecordDeath(State);
		Controller->SetSpeed(1.f);
		Sim->StepOnce();
	};

	// ---- (1) A death's boundary, (2) a leap the end cuts ----
	{
		const int64 Died = EndTick - 20;
		FGalaxyState& Live = Lay(Died);
		Kill(Live);
		if (!TestTrue(TEXT("(the vessel died with its leap of 40 ahead)"), Live.Tick == Died && IsWaiting(Live) && Live.Calling.PendingLeapTicks == 40))
		{
			return false;
		}
		TestTrue(TEXT("(a life closed at the death's boundary)"), ACosmicPlayerController::LifeClosedAt(Live, Died));
		TestEqual(TEXT("The death's boundary stops the time"), Sim->GetTimeScale(), 0.f);
		TestTrue(TEXT("for the death, with the automatic pause off"), Controller->GetAutoPauseReason() == ECosmicAutoPause::Death);
		TestEqual(TEXT("and the controller keeps the death's tick"), Controller->GetDeathTick(), Died);
		TestTrue(TEXT("The death's boundary writes the slot once, for the death"), Writes.One(ECosmicSaveCause::Death));
		TestTrue(TEXT("AdvanceLeap runs"), Controller->AdvanceLeap());
		TestEqual(TEXT("up to the end's tick, not past it"), Live.Tick, EndTick);
		TestTrue(TEXT("with the rest of the leap kept"), IsWaiting(Live) && Live.Calling.PendingLeapTicks == 20);
		TestTrue(TEXT("Its boundary latches the end"), Controller->IsRunEnded());
		TestEqual(TEXT("of the clock"), Controller->GetRunEnd(), ECosmicProvisionalEnd::Clock);
		TestTrue(TEXT("and writes the slot once, for the end"), Writes.One(ECosmicSaveCause::Ended));
		TestFalse(TEXT("An ended run's AdvanceLeap runs nothing"), Controller->AdvanceLeap());
		TestEqual(TEXT("(the tick stays)"), Live.Tick, EndTick);
	}

	// ---- (3) A leap that succeeds before the end: one boundary, one write, the feed rebuilt ----
	{
		const int64 Died = 400;
		FGalaxyState& Live = Lay(Died);
		// A plague about to end on a settled system of another people: its expiry is told inside the leap, where no line
		// reaches the feed through OnLogEntry.
		const FGameplayTag Plague = Tag(TEXT("Hazard.Plague"));
		const int32 Plagued = OtherPeoplesSettled(Live);
		if (!TestTrue(TEXT("(another people holds a settled system, and the game has the plague)"), Plagued != INDEX_NONE && Sim->GetTables().FindHazardByTag(Plague)))
		{
			return false;
		}
		Live.Systems[Plagued].ActiveHazard = Plague;
		Live.Systems[Plagued].HazardTicksRemaining = 5;
		// A feed of its own (the form of UI.LogFeed); holding the Slate widget keeps it constructed.
		UCosmicLogFeedWidget* Feed = CreateWidget<UCosmicLogFeedWidget>(Controller);
		if (!TestNotNull(TEXT("The feed can be created"), Feed))
		{
			return false;
		}
		const TSharedRef<SWidget> SlateFeed = Feed->TakeWidget();
		Kill(Live);
		Writes.Take();
		if (!TestTrue(TEXT("(the vessel died with its leap of 40 ahead)"), Live.Tick == Died && IsWaiting(Live) && Live.Calling.PendingLeapTicks == 40))
		{
			return false;
		}
		const int32 ToldAtDeath = ToldMessages(Live).Num();
		TestTrue(TEXT("(the feed holds the told lines at the death)"), ItemMessages(*Feed) == ToldMessages(Live));
		{
			FBoundaryRunners Count(*Sim, false, true);
			TestTrue(TEXT("AdvanceLeap runs"), Controller->AdvanceLeap());
		}
		TestEqual(TEXT("the whole leap"), Live.Tick, Died + 40);
		TestTrue(TEXT("to the succession"), !IsWaiting(Live) && Live.Calling.Lives.Num() == 2 && Live.Calling.Lives.Last().BornTick == Died + 40);
		TestFalse(TEXT("Nothing latches before the end"), Controller->IsRunEnded());
		TestTrue(TEXT("The leap's boundary writes the slot once, for the succession"), Writes.One(ECosmicSaveCause::Succession));
		const TArray<FString> Told = ToldMessages(Live);
		const TArray<FString> Shown = ItemMessages(*Feed);
		if (TestTrue(TEXT("(the leap told lines the feed did not hear)"), Told.Num() > ToldAtDeath))
		{
			TestEqual(TEXT("The feed rebuilt at the leap's boundary: its newest item is the newest told line"), Shown.IsEmpty() ? FString() : Shown.Last(), Told.Last());
			TestEqual(TEXT("and it holds the told lines"), Shown.Num(), FMath::Min(Told.Num(), CosmicLogFeed::MaxItems));
		}
		// [B-5d1]: the last line told inside the leap is the inauguration line of the life the succession stood up - the feed's
		// last item after the leap.
		const FCosmicLogEntry* Newest = NewestTold(Live);
		if (TestTrue(TEXT("The newest told line is the inauguration line of the leap's last tick"), Newest && Newest->SourceRow == CosmicRows::LogInauguration
			&& Newest->Tick == Died + 40))
		{
			TestEqual(TEXT("The feed's last item after the leap is the inauguration line"), Shown.IsEmpty() ? FString() : Shown.Last(), Newest->Message.ToString());
		}
	}

	// ---- (3b) A leap of one tick (the cap leaves 1): its boundary follows the death's by one tick, and still rebuilds the feed ----
	{
		FCosmicRulesDef OneTick = LeapRules(GameRules, 40, 40);
		OneTick.LeapTotalCap = 1;
		Sim->SetTables(CosmicTests::WithRules(Original, OneTick));
		const int64 Died = 400;
		FGalaxyState& Live = Lay(Died);
		const FGameplayTag Plague = Tag(TEXT("Hazard.Plague"));
		const int32 Plagued = OtherPeoplesSettled(Live);
		if (!TestTrue(TEXT("(another people holds a settled system, and the game has the plague)"), Plagued != INDEX_NONE && Sim->GetTables().FindHazardByTag(Plague)))
		{
			return false;
		}
		UCosmicLogFeedWidget* Feed = CreateWidget<UCosmicLogFeedWidget>(Controller);
		if (!TestNotNull(TEXT("The feed can be created"), Feed))
		{
			return false;
		}
		const TSharedRef<SWidget> SlateFeed = Feed->TakeWidget();
		Kill(Live);
		Writes.Take();
		if (!TestTrue(TEXT("(the vessel died with a leap of one tick ahead)"), Live.Tick == Died && IsWaiting(Live) && Live.Calling.PendingLeapTicks == 1))
		{
			return false;
		}
		// A plague that ends on the leap's one tick: its expiry is told inside the leap, where no line reaches the feed through
		// OnLogEntry, and the boundary's tick is the one after the feed's last.
		Live.Systems[Plagued].ActiveHazard = Plague;
		Live.Systems[Plagued].HazardTicksRemaining = 1;
		const int32 ToldAtDeath = ToldMessages(Live).Num();
		TestTrue(TEXT("(the feed holds the told lines at the death)"), ItemMessages(*Feed) == ToldMessages(Live));
		TestTrue(TEXT("AdvanceLeap runs"), Controller->AdvanceLeap());
		TestTrue(TEXT("one tick, to the succession"), Live.Tick == Died + 1 && !IsWaiting(Live) && Live.Calling.Lives.Num() == 2 && Live.Calling.Lives.Last().BornTick == Died + 1);
		TestTrue(TEXT("Its boundary writes the slot once, for the succession"), Writes.One(ECosmicSaveCause::Succession));
		const FCosmicLogEntry* Newest = NewestTold(Live);
		const TArray<FString> Told = ToldMessages(Live);
		const TArray<FString> Shown = ItemMessages(*Feed);
		if (TestTrue(TEXT("(the leap's one tick told a line the feed did not hear)"), Told.Num() > ToldAtDeath && Newest && Newest->Tick == Died + 1))
		{
			TestEqual(TEXT("The feed rebuilt at the boundary of a one-tick leap: its newest item is the line told inside the leap"), Shown.IsEmpty() ? FString() : Shown.Last(),
				Newest->Message.ToString());
			TestEqual(TEXT("and it holds the told lines"), Shown.Num(), FMath::Min(Told.Num(), CosmicLogFeed::MaxItems));
		}
		Sim->SetTables(Fixed);
	}

	// ---- (4) With the end switched off the leap runs past the end's tick and nothing latches ----
	{
		Controller->SetProvisionalEndEnabled(false);
		const int64 Died = EndTick - 20;
		FGalaxyState& Live = Lay(Died);
		Kill(Live);
		TestTrue(TEXT("AdvanceLeap runs"), Controller->AdvanceLeap());
		TestEqual(TEXT("With the end switched off the leap runs past the end's tick"), Live.Tick, Died + 40);
		TestTrue(TEXT("to the succession"), !IsWaiting(Live) && Live.Calling.Lives.Num() == 2);
		TestFalse(TEXT("and nothing latches"), Controller->IsRunEnded());
		Writes.Take();
		Controller->SetProvisionalEndEnabled(true);
	}

	// ---- (5) The silence: no candidate - every other system moved out of the death place's galaxy row ----
	{
		const int64 Died = EndTick - 100;
		FGalaxyState& Live = Lay(Died);
		const int32 Death = Sim->GetStateIndex().FindSystem(Live, Live.Player.CurrentSystemID);
		if (!TestTrue(TEXT("(the vessel stands in a system)"), Live.Systems.IsValidIndex(Death)))
		{
			return false;
		}
		for (int32 SystemIndex = 0; SystemIndex < Live.Systems.Num(); ++SystemIndex)
		{
			if (SystemIndex != Death && !CosmicState::IsSanctuary(Live.Systems[SystemIndex].SystemID))
			{
				Live.Systems[SystemIndex].GalaxyRow = FName(TEXT("galaxy_test_apart"));
			}
		}
		Kill(Live);
		Writes.Take();
		TestTrue(TEXT("AdvanceLeap runs the leap"), Controller->AdvanceLeap());
		const int64 Silent = Died + 40;
		TestTrue(TEXT("to a succession with no candidate: the calling stays in the interregnum with no leap left (the silence)"),
			Live.Tick == Silent && IsWaiting(Live) && Live.Calling.PendingLeapTicks == 0 && Live.Calling.Lives.Num() == 1);
		TestEqual(TEXT("The silence's boundary writes nothing"), Writes.Take(), 0);
		Controller->SetProvisionalEndEnabled(false);
		TestFalse(TEXT("With the end switched off the silence's AdvanceLeap runs nothing"), Controller->AdvanceLeap());
		TestEqual(TEXT("(the tick stays)"), Live.Tick, Silent);
		Controller->SetProvisionalEndEnabled(true);
		TestTrue(TEXT("With the end ahead it runs"), Controller->AdvanceLeap());
		TestEqual(TEXT("to the end's tick"), Live.Tick, EndTick);
		TestTrue(TEXT("where the end latches"), Controller->IsRunEnded() && Controller->GetRunEnd() == ECosmicProvisionalEnd::Clock);
		TestTrue(TEXT("(still the silence)"), IsWaiting(Live) && Live.Calling.PendingLeapTicks == 0 && Live.Calling.Lives.Num() == 1);
		TestTrue(TEXT("and its boundary writes the slot once, for the end"), Writes.One(ECosmicSaveCause::Ended));
	}

	// ---- (6) A leap of 0 (LeapTotalCap 0): the succession on the death's tick, and the boundary still stops the time ----
	{
		FCosmicRulesDef Capped = LeapRules(GameRules, 40, 40);
		Capped.LeapTotalCap = 0;
		Sim->SetTables(CosmicTests::WithRules(Original, Capped));
		const int64 Died = 400;
		FGalaxyState& Live = Lay(Died);
		Kill(Live);
		if (TestTrue(TEXT("(a leap of 0 succeeds on the death's tick)"), !IsWaiting(Live) && Live.Calling.Lives.Num() == 2 && Live.Calling.Lives[0].DiedTick == Died
			&& Live.Calling.Lives[1].BornTick == Died))
		{
			TestTrue(TEXT("A life closed at that boundary, though the calling is embodied"), ACosmicPlayerController::LifeClosedAt(Live, Died));
			TestEqual(TEXT("The boundary of a leap of 0 stops the time"), Sim->GetTimeScale(), 0.f);
			TestTrue(TEXT("for the death"), Controller->GetAutoPauseReason() == ECosmicAutoPause::Death);
			TestEqual(TEXT("and the controller keeps the death's tick"), Controller->GetDeathTick(), Died);
			TestTrue(TEXT("Its write is the succession, once (the new life stands at the boundary)"), Writes.One(ECosmicSaveCause::Succession));
			TestFalse(TEXT("An embodied calling's AdvanceLeap runs nothing"), Controller->AdvanceLeap());
			TestEqual(TEXT("(the tick stays)"), Live.Tick, Died);
		}
		Sim->SetTables(Fixed);
	}

	// ---- (7) A death on the end's tick: its boundary sees the death before the end's latch ----
	{
		FGalaxyState& Live = Lay(EndTick);
		Kill(Live);
		if (TestTrue(TEXT("(the vessel died on the end's tick)"), Live.Tick == EndTick && IsWaiting(Live)))
		{
			TestEqual(TEXT("The death on the end's tick is seen at its boundary"), Controller->GetDeathTick(), EndTick);
			TestTrue(TEXT("and the end latches there too"), Controller->IsRunEnded() && Controller->GetRunEnd() == ECosmicProvisionalEnd::Clock);
			TestEqual(TEXT("with the time stopped"), Sim->GetTimeScale(), 0.f);
			TestTrue(TEXT("Its write is the end's, once (the end before the death)"), Writes.One(ECosmicSaveCause::Ended));
			TestFalse(TEXT("The ended run's AdvanceLeap runs nothing"), Controller->AdvanceLeap());
			TestTrue(TEXT("(the leap waits)"), Live.Tick == EndTick && Live.Calling.PendingLeapTicks == 40);
		}
	}

	// ---- (8) [B-5d1] A succession on the end's tick: the feed holds back the inauguration line of the vessel the end installed ----
	// (CosmicChronicle::IsUnseenLifeLine - the player never saw that vessel, ACosmicPlayerController::IsVesselShown): its last item
	// is the line told before it - through a leap that ends on the end's tick (the boundary's rebuild) and through a leap of 0
	// on it (the line came through OnLogEntry before the end latched: the latch's OnRunEndChanged rebuilds the feed); with the
	// end forgotten the line shows again.
	{
		// The told line before Entry (null for none) and the feed's items.
		auto ToldBefore = [](const FGalaxyState& State, const FCosmicLogEntry* Entry) -> const FCosmicLogEntry*
		{
			const int32 At = Entry ? static_cast<int32>(Entry - State.Log.GetData()) : INDEX_NONE;
			for (int32 Index = At - 1; At != INDEX_NONE && Index >= 0; --Index)
			{
				if (!State.Log[Index].bUntold)
				{
					return &State.Log[Index];
				}
			}
			return nullptr;
		};
		auto IsInauguration = [](const FCosmicLogEntry* Entry)
		{
			return Entry && (Entry->SourceRow == CosmicRows::LogInauguration || Entry->SourceRow == CosmicRows::LogInaugurationUnnamed);
		};
		// (8a) The leap that ends on the end's tick.
		{
			FGalaxyState& Live = Lay(EndTick - 40);
			UCosmicLogFeedWidget* Feed = CreateWidget<UCosmicLogFeedWidget>(Controller);
			if (!TestNotNull(TEXT("The feed can be created (the end's succession)"), Feed))
			{
				return false;
			}
			const TSharedRef<SWidget> SlateFeed = Feed->TakeWidget();
			Kill(Live);
			if (!TestTrue(TEXT("(the vessel died with its leap of 40 to the end's tick ahead)"), IsWaiting(Live) && Live.Calling.PendingLeapTicks == 40
				&& Live.Tick + 40 == EndTick))
			{
				return false;
			}
			TestTrue(TEXT("AdvanceLeap runs (the end's succession)"), Controller->AdvanceLeap());
			if (TestTrue(TEXT("(the succession stood a vessel up on the end's tick, and the end latched there)"), Live.Tick == EndTick && !IsWaiting(Live)
				&& Live.Calling.Lives.Num() == 2 && Live.Calling.Lives.Last().BornTick == EndTick && Controller->IsRunEnded()))
			{
				const FCosmicLogEntry* Newest = NewestTold(Live);
				const FCosmicLogEntry* Before = ToldBefore(Live, Newest);
				const TArray<FString> Shown = ItemMessages(*Feed);
				if (TestTrue(TEXT("(the newest told line is the inauguration line of the end's tick, with a told line before it)"), IsInauguration(Newest)
					&& Newest->Tick == EndTick && Before != nullptr))
				{
					TestEqual(TEXT("The feed's last item is the line told before the inauguration line of the vessel the end installed"),
						Shown.IsEmpty() ? FString() : Shown.Last(), Before->Message.ToString());
					TestFalse(TEXT("and the inauguration line is not among its items"), Shown.Contains(Newest->Message.ToString()));
					TestEqual(TEXT("It holds every other told line"), Shown.Num(), FMath::Min(ToldMessages(Live).Num() - 1, CosmicLogFeed::MaxItems));
					// The end forgotten (the switch off): the vessel is the run's again, and so is its line.
					Controller->SetProvisionalEndEnabled(false);
					const TArray<FString> Again = ItemMessages(*Feed);
					TestEqual(TEXT("With the end forgotten the feed shows the inauguration line again, last"), Again.IsEmpty() ? FString() : Again.Last(),
						Newest->Message.ToString());
					Controller->SetProvisionalEndEnabled(true);
				}
			}
		}
		// (8b) A leap of 0 on the end's tick: the death and the succession on one tick, their lines heard as they were told.
		{
			FCosmicRulesDef Capped = LeapRules(GameRules, 40, 40);
			Capped.LeapTotalCap = 0;
			Sim->SetTables(CosmicTests::WithRules(Original, Capped));
			FGalaxyState& Live = Lay(EndTick);
			UCosmicLogFeedWidget* Feed = CreateWidget<UCosmicLogFeedWidget>(Controller);
			if (!TestNotNull(TEXT("The feed can be created (a leap of 0 on the end's tick)"), Feed))
			{
				return false;
			}
			const TSharedRef<SWidget> SlateFeed = Feed->TakeWidget();
			Kill(Live);
			if (TestTrue(TEXT("(a leap of 0 stood a vessel up on the end's tick, and the end latched at its boundary)"), Live.Tick == EndTick && !IsWaiting(Live)
				&& Live.Calling.Lives.Num() == 2 && Live.Calling.Lives.Last().BornTick == EndTick && Controller->IsRunEnded()))
			{
				const FCosmicLogEntry* Newest = NewestTold(Live);
				const FCosmicLogEntry* Before = ToldBefore(Live, Newest);
				const TArray<FString> Shown = ItemMessages(*Feed);
				if (TestTrue(TEXT("(the tick ends with the death line, then the inauguration line)"), IsInauguration(Newest) && Before
					&& Before->SourceRow == CosmicRows::LogDeathHazard))
				{
					TestEqual(TEXT("The feed's last item after the end's latch is the death line, not the inauguration line"), Shown.IsEmpty() ? FString() : Shown.Last(),
						Before->Message.ToString());
					TestFalse(TEXT("which it heard and let go when the end latched"), Shown.Contains(Newest->Message.ToString()));
					// A line of that life heard after the end latched (nothing in the game tells one; the test tells it by hand): the
					// feed's rule holds for a new line as for the rebuild.
					UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
					FFormatNamedArguments Args;
					if (TestTrue(TEXT("(a second inauguration line of the end's tick is told after the latch)"), Log
						&& Log->PushRow(CosmicRows::LogInauguration, 2, Args, NAME_None, NAME_None) && IsInauguration(NewestTold(Live))))
					{
						TestEqual(TEXT("The feed does not take it either"), ItemMessages(*Feed), Shown);
					}
				}
			}
			Sim->SetTables(Fixed);
		}
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

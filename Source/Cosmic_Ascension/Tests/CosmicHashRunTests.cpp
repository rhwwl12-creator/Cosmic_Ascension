// Cosmic Ascension - automation tests of the hash run of a game process (MVP item 9): the switch, the idle and the policy
// run, the refusal on missing data, the checkpoints and the lines. Design: docs/60_DECISIONS/headers/A9_win64_packaging.md
// section 2 and 5. The run in a game process itself (ACosmicGameMode::StartPlay) is seen by run_checks and package.ps1.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "Tests/CosmicTestProbe.h"
#include "World/CosmicHashRun.h"
#include "World/CosmicRulerPolicy.h"

namespace CosmicHashRunTests
{
	struct FHashes
	{
		uint32 State = 0;
		uint32 World = 0;
		int64 Tick = -1;
	};

	FHashes HashesOf(const UGalaxySimulationSubsystem& Sim)
	{
		return { Sim.ComputeStateHash(), Sim.ComputeWorldHash(), Sim.GetState().Tick };
	}

	// The idle run written out by hand: the commandlet's -TellAll as given, NewGalaxy of the home row, one RunHeadless.
	FHashes IdleByHand(UGalaxySimulationSubsystem& Sim, UCosmicLogSubsystem& Log, bool bTellAll, int64 Ticks)
	{
		Log.SetTellAll(bTellAll);
		Sim.NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		Sim.RunHeadless(Ticks);
		return HashesOf(Sim);
	}

	CosmicHashRun::FSwitch SwitchOf(int64 Ticks, int64 Every = 0, bool bPolicy = false)
	{
		CosmicHashRun::FSwitch Switch;
		Switch.Kind = CosmicHashRun::ESwitch::Ticks;
		Switch.Ticks = Ticks;
		Switch.EveryKind = Every > 0 ? CosmicHashRun::ESwitch::Ticks : CosmicHashRun::ESwitch::Absent;
		Switch.Every = Every;
		Switch.bPolicy = bPolicy;
		return Switch;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimHashRunTest, "Cosmic_Ascension.Sim.HashRun", CosmicTests::Flags)
bool FCosmicSimHashRunTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHashRunTests;
	using CosmicHashRun::ESwitch;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UGameInstance* GameInstance = Sim->GetGameInstance();
	UCosmicLogSubsystem* Log = GameInstance->GetSubsystem<UCosmicLogSubsystem>();
	UPlayerRoleSubsystem* Role = GameInstance->GetSubsystem<UPlayerRoleSubsystem>();
	if (!TestNotNull(TEXT("(the log subsystem)"), Log) || !TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT
	{
		Log->SetTellAll(true);
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
	};

	// (1) The switch: a count is 1 to 6 ASCII digits within MaxTicks, read as text; anything else on the key is Bad.
	{
		const auto Kind = [](const TCHAR* Line) { return CosmicHashRun::SwitchOf(Line).Kind; };
		const auto Ticks = [](const TCHAR* Line) { return CosmicHashRun::SwitchOf(Line).Ticks; };
		TestEqual(TEXT("-CosmicHashRun=3000 asks 3,000 ticks"), Ticks(TEXT("-Seed=42 -CosmicHashRun=3000 -nullrhi")), static_cast<int64>(3000));
		TestTrue(TEXT("(and is a count)"), Kind(TEXT("-Seed=42 -CosmicHashRun=3000 -nullrhi")) == ESwitch::Ticks);
		TestEqual(TEXT("the key is read in any case"), Ticks(TEXT("-cosmichashrun=5")), static_cast<int64>(5));
		TestTrue(TEXT("=0 is the generated galaxy alone"), Kind(TEXT("-CosmicHashRun=0")) == ESwitch::Ticks && Ticks(TEXT("-CosmicHashRun=0")) == 0);
		TestEqual(TEXT("=100000 is the most"), Ticks(TEXT("-CosmicHashRun=100000")), CosmicHashRun::MaxTicks);
		TestTrue(TEXT("No key is no hash run"), Kind(TEXT("-Seed=42 -nullrhi")) == ESwitch::Absent);
		for (const TCHAR* Bad : { TEXT("-CosmicHashRun=abc"), TEXT("-CosmicHashRun="), TEXT("-CosmicHashRun=+3000"), TEXT("-CosmicHashRun=3000x"),
			TEXT("-CosmicHashRun=-5"), TEXT("-CosmicHashRun=100001"), TEXT("-CosmicHashRun=0000001"), TEXT("-CosmicHashRun=3,000"),
			TEXT("-CosmicHashRun=30)") })
		{
			TestTrue(FString::Printf(TEXT("%s is Bad"), Bad), Kind(Bad) == ESwitch::Bad);
		}
		TestEqual(TEXT("A Bad value keeps its text for the refusal line"), CosmicHashRun::SwitchOf(TEXT("-CosmicHashRun=3000x")).Text, FString(TEXT("3000x")));
		const CosmicHashRun::FSwitch Every = CosmicHashRun::SwitchOf(TEXT("-CosmicHashRun=300 -CosmicHashEvery=100 -CosmicHashPolicy"));
		TestTrue(TEXT("-CosmicHashEvery is a count of its own"), Every.EveryKind == ESwitch::Ticks && Every.Every == 100);
		TestTrue(TEXT("-CosmicHashPolicy asks the ruler policy"), Every.bPolicy);
		TestFalse(TEXT("(without it the run is idle)"), CosmicHashRun::SwitchOf(TEXT("-CosmicHashRun=300")).bPolicy);
		TestTrue(TEXT("A good switch may run"), Every.IsGood());
		TestFalse(TEXT("A Bad -CosmicHashEvery may not"), CosmicHashRun::SwitchOf(TEXT("-CosmicHashRun=300 -CosmicHashEvery=x")).IsGood());
		TestFalse(TEXT("(nor one cut at a comma)"), CosmicHashRun::SwitchOf(TEXT("-CosmicHashRun=300 -CosmicHashEvery=1,000")).IsGood());
		TestFalse(TEXT("-CosmicHashPolicy alone may not"), CosmicHashRun::SwitchOf(TEXT("-CosmicHashPolicy")).IsGood());
		TestFalse(TEXT("-CosmicHashEvery alone may not"), CosmicHashRun::SwitchOf(TEXT("-CosmicHashEvery=100")).IsGood());
		TestFalse(TEXT("A Bad -CosmicHashRun may not"), CosmicHashRun::SwitchOf(TEXT("-CosmicHashRun=x")).IsGood());
	}

	// (2) The idle run is the commandlet's idle run: tell-all, NewGalaxy of the home row, RunHeadless.
	const FHashes Whole = IdleByHand(*Sim, *Log, true, 300);
	const FHashes Scoped = IdleByHand(*Sim, *Log, false, 300);
	const FHashes Generated = IdleByHand(*Sim, *Log, true, 0);
	Log->SetTellAll(true);
	{
		const CosmicHashRun::FResult Result = CosmicHashRun::Run(*Sim, *Log, *Role, 42, SwitchOf(300));
		TestTrue(TEXT("The idle run ran"), Result.Outcome == CosmicHashRun::EOutcome::Ran);
		TestEqual(TEXT("its state hash is the hand-written run's"), Result.StateHash, Whole.State);
		TestEqual(TEXT("and so is its world hash"), Result.WorldHash, Whole.World);
		TestEqual(TEXT("after 300 ticks"), Sim->GetState().Tick, static_cast<int64>(300));
		const CosmicHashRun::FResult Zero = CosmicHashRun::Run(*Sim, *Log, *Role, 42, SwitchOf(0));
		TestTrue(TEXT("0 ticks hashes the generated galaxy"), Zero.StateHash == Generated.State && Zero.WorldHash == Generated.World);
		TestEqual(TEXT("(at tick 0)"), Sim->GetState().Tick, static_cast<int64>(0));
		// The seed is the run's: another seed is another galaxy, and the line names the seed that made it.
		Log->SetTellAll(true);
		Sim->NewGalaxy(7, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		const FHashes Seven = HashesOf(*Sim);
		const CosmicHashRun::FResult Other = CosmicHashRun::Run(*Sim, *Log, *Role, 7, SwitchOf(0));
		TestTrue(TEXT("The run takes its seed"), Other.StateHash == Seven.State && Other.WorldHash == Seven.World && Other.Seed == 7);
		TestNotEqual(TEXT("(seed 7 is not seed 42 - the case can tell)"), Seven.World, Generated.World);
	}

	// (3) The idle run tells all whatever the log's mode was, switched on before NewGalaxy, and gives the mode back.
	{
		TestNotEqual(TEXT("(the scope changes the whole hash - the case can tell)"), Scoped.State, Whole.State);
		TestEqual(TEXT("(and leaves the world alone)"), Scoped.World, Whole.World);
		Log->SetTellAll(false);
		CosmicTests::FScopedProbe Probe(*Sim);
		TArray<bool> TellingAll;
		Probe.Probe->OnLog = [Log, &TellingAll](const FCosmicLogEntry&) { TellingAll.Add(Log->IsTellAll()); };
		const CosmicHashRun::FResult Result = CosmicHashRun::Run(*Sim, *Log, *Role, 42, SwitchOf(300));
		Probe.Probe->OnLog = nullptr;
		TestEqual(TEXT("A run from the scope gives the tell-all state hash"), Result.StateHash, Whole.State);
		TestFalse(TEXT("and the log is back in the scope"), Log->IsTellAll());
		TestTrue(TEXT("(a line was told inside NewGalaxy)"), TellingAll.Num() > 0);
		TestTrue(TEXT("and the log told all by then (switched before NewGalaxy)"), TellingAll.Num() > 0 && TellingAll[0]);
		Log->SetTellAll(true);
	}

	// (4) Missing data: every table, the rules row and the home galaxy row - refused before NewGalaxy.
	{
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		Sim->RunHeadless(5);
		const FGuid GalaxyBefore = Sim->GetState().GalaxyID;
		const int64 TickBefore = Sim->GetState().Tick;
		const auto Refused = [this, Sim, Log, Role, GalaxyBefore, TickBefore](const FCosmicDataTables& Tables, const TCHAR* Expected)
		{
			Sim->SetTables(Tables);
			const CosmicHashRun::FResult Result = CosmicHashRun::Run(*Sim, *Log, *Role, 42, SwitchOf(10));
			TestTrue(FString::Printf(TEXT("Missing %s refuses the run"), Expected), Result.Outcome == CosmicHashRun::EOutcome::MissingData);
			TestEqual(FString::Printf(TEXT("and names %s"), Expected), Result.Missing, FString(Expected));
			TestTrue(FString::Printf(TEXT("and never calls NewGalaxy (%s)"), Expected), Sim->GetState().GalaxyID == GalaxyBefore && Sim->GetState().Tick == TickBefore);
		};
		const TPair<const TCHAR*, TObjectPtr<UDataTable> FCosmicDataTables::*> Members[] = {
			{ TEXT("DT_Rules"), &FCosmicDataTables::Rules }, { TEXT("DT_Civs"), &FCosmicDataTables::Civs },
			{ TEXT("DT_LogTemplates"), &FCosmicDataTables::LogTemplates }, { TEXT("DT_Galaxies"), &FCosmicDataTables::Galaxies },
			{ TEXT("DT_Events"), &FCosmicDataTables::Events }, { TEXT("DT_Transcendents"), &FCosmicDataTables::Transcendents },
			{ TEXT("DT_Hazards"), &FCosmicDataTables::Hazards }, { TEXT("DT_Megaprojects"), &FCosmicDataTables::Megaprojects },
			{ TEXT("DT_Feats"), &FCosmicDataTables::Feats }, { TEXT("DT_Encounters"), &FCosmicDataTables::Encounters },
			{ TEXT("DT_Edicts"), &FCosmicDataTables::Edicts },
		};
		for (const TPair<const TCHAR*, TObjectPtr<UDataTable> FCosmicDataTables::*>& Member : Members)
		{
			FCosmicDataTables Without = Original;
			Without.*(Member.Value) = nullptr;
			Refused(Without, Member.Key);
		}
		AddExpectedError(TEXT("DT_Rules has no row"), EAutomationExpectedErrorFlags::Contains, 1);
		FCosmicDataTables NoRulesRow = Original;
		NoRulesRow.Rules = CosmicTests::NewTable(FCosmicRulesDef::StaticStruct());
		Refused(NoRulesRow, TEXT("DT_Rules:rules"));
		AddExpectedError(TEXT("DT_Galaxies has no row"), EAutomationExpectedErrorFlags::Contains, 1);
		Refused(CosmicTests::WithGalaxies(Original, CosmicTests::NewTable(FGalaxyDef::StaticStruct())), TEXT("DT_Galaxies:home"));
		Sim->SetTables(Original);
	}

	// (5) The policy run is the commandlet's loop: the scope of a game, one tick at a time, the ruler's input between ticks.
	FHashes PolicyByHand;
	{
		Log->SetTellAll(false);
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		const FCosmicRulerPolicy Policy{ true, true, true, true, NAME_None, 1, ECosmicPressurePolicy::Round, 0, false };
		FCosmicRulerPolicyCounts Counts;
		for (int64 Done = 0; Done < 300; ++Done)
		{
			const FName CardBefore = Sim->GetState().Knowing.PendingPetitionRow;
			Sim->RunHeadless(1);
			CosmicRulerPolicy::Act(Policy, *Sim, *Role, CardBefore, Counts);
		}
		PolicyByHand = HashesOf(*Sim);
		TestTrue(TEXT("(the policy answered, started, focused and turned edicts - the case can tell)"), Counts.Answers > 0 && Counts.Started > 0 && Counts.FocusChanges > 0
			&& Counts.EdictsEnacted + Counts.EdictsRevoked > 0);
		TestNotEqual(TEXT("(and its world is not the idle one)"), PolicyByHand.World, Whole.World);
		Log->SetTellAll(true);
		const CosmicHashRun::FResult Result = CosmicHashRun::Run(*Sim, *Log, *Role, 42, SwitchOf(300, 0, true));
		TestTrue(TEXT("The policy run ran"), Result.Outcome == CosmicHashRun::EOutcome::Ran && Result.bPolicy);
		TestEqual(TEXT("its state hash is the hand-written loop's"), Result.StateHash, PolicyByHand.State);
		TestEqual(TEXT("and so is its world hash"), Result.WorldHash, PolicyByHand.World);
		TestTrue(TEXT("and the log is back in its mode"), Log->IsTellAll());
	}

	// (5b) The diplomacy of the policy (B3): past the ruled people's second rung the hash run's policy signs treaties, and a
	// policy without diplomacy is another world - so the hash run plays the commandlet's -Diplomacy=1.
	{
		constexpr int64 PastSecondRung = 1500;
		auto ByHand = [&](int32 Diplomacy, FCosmicRulerPolicyCounts& Counts)
		{
			Log->SetTellAll(false);
			Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
			const FCosmicRulerPolicy Policy{ true, true, true, true, NAME_None, Diplomacy, ECosmicPressurePolicy::Round, 0, false };
			for (int64 Done = 0; Done < PastSecondRung; ++Done)
			{
				const FName CardBefore = Sim->GetState().Knowing.PendingPetitionRow;
				Sim->RunHeadless(1);
				CosmicRulerPolicy::Act(Policy, *Sim, *Role, CardBefore, Counts);
			}
			Log->SetTellAll(true);
			return HashesOf(*Sim);
		};
		FCosmicRulerPolicyCounts With;
		const FHashes Diplomatic = ByHand(1, With);
		FCosmicRulerPolicyCounts Without;
		const FHashes Quiet = ByHand(0, Without);
		TestTrue(TEXT("(the policy's diplomacy acted by tick 1,500 - the case can tell)"), With.StanceChanges + With.TreatiesSigned + With.FleetTargetChanges > 0);
		TestNotEqual(TEXT("A policy without diplomacy is another world"), Quiet.World, Diplomatic.World);
		const CosmicHashRun::FResult Result = CosmicHashRun::Run(*Sim, *Log, *Role, 42, SwitchOf(PastSecondRung, 0, true));
		TestTrue(TEXT("The hash run's policy is the diplomatic one"), Result.StateHash == Diplomatic.State && Result.WorldHash == Diplomatic.World);

		// With fleets (set here whatever the data holds - [B-3d2] turned the game's on) the attack policy is a third world - it
		// offers no treaty and leaves its last realm slot to a conquest: the hash run plays the peace policy, not the attack one.
		FCosmicRulesDef Armed = Sim->GetRules();
		Armed.FleetPerSettledByTier = { 0.f, 1.f, 1.f, 1.f, 1.f };
		Sim->SetTables(CosmicTests::WithRules(Original, Armed));
		FCosmicRulerPolicyCounts Peace;
		const FHashes Peaceful = ByHand(1, Peace);
		FCosmicRulerPolicyCounts Attack;
		const FHashes Hostile = ByHand(2, Attack);
		TestNotEqual(TEXT("The attack policy is another world (the case can tell)"), Hostile.World, Peaceful.World);
		const CosmicHashRun::FResult ArmedRun = CosmicHashRun::Run(*Sim, *Log, *Role, 42, SwitchOf(PastSecondRung, 0, true));
		TestTrue(TEXT("and the hash run's policy is the peace one"), ArmedRun.StateHash == Peaceful.State && ArmedRun.WorldHash == Peaceful.World);
		Sim->SetTables(Original);
	}

	// (6) Checkpoints: tick 0 and every Every ticks; the run in steps is the run at once.
	{
		const CosmicHashRun::FResult Stepped = CosmicHashRun::Run(*Sim, *Log, *Role, 42, SwitchOf(300, 100));
		TestEqual(TEXT("Four checkpoints for 300 ticks every 100"), Stepped.Checkpoints.Num(), 4);
		TestTrue(TEXT("the first at tick 0"), Stepped.Checkpoints.Num() == 4 && Stepped.Checkpoints[0].StartsWith(TEXT("CosmicHashAt: tick 0, ")));
		TestEqual(TEXT("the last is the end"), Stepped.Checkpoints.Num() == 4 ? Stepped.Checkpoints[3] : FString(),
			FString::Printf(TEXT("CosmicHashAt: tick 300, hash %u, world %u"), Whole.State, Whole.World));
		TestTrue(TEXT("RunHeadless in steps is RunHeadless at once"), Stepped.StateHash == Whole.State && Stepped.WorldHash == Whole.World);
		const CosmicHashRun::FResult Odd = CosmicHashRun::Run(*Sim, *Log, *Role, 42, SwitchOf(300, 70));
		TestEqual(TEXT("Every 70 of 300: ticks 0, 70, 140, 210, 280"), Odd.Checkpoints.Num(), 5);
		TestTrue(TEXT("and the end is still the run at once"), Odd.StateHash == Whole.State && Odd.WorldHash == Whole.World);
		const CosmicHashRun::FResult PolicySteps = CosmicHashRun::Run(*Sim, *Log, *Role, 42, SwitchOf(300, 100, true));
		TestEqual(TEXT("The policy run has the same checkpoints"), PolicySteps.Checkpoints.Num(), 4);
		TestTrue(TEXT("on the same ticks"), PolicySteps.Checkpoints.Num() == 4 && PolicySteps.Checkpoints[1].StartsWith(TEXT("CosmicHashAt: tick 100, ")));
		TestEqual(TEXT("the last is the policy loop's end"), PolicySteps.Checkpoints.Num() == 4 ? PolicySteps.Checkpoints[3] : FString(),
			FString::Printf(TEXT("CosmicHashAt: tick 300, hash %u, world %u"), PolicyByHand.State, PolicyByHand.World));
		TestTrue(TEXT("and the policy run in steps is the policy run"), PolicySteps.StateHash == PolicyByHand.State && PolicySteps.WorldHash == PolicyByHand.World);
	}

	// (7) The lines.
	{
		CosmicHashRun::FResult Line;
		Line.Outcome = CosmicHashRun::EOutcome::Ran;
		Line.Seed = 42;
		Line.Ticks = 3000;
		Line.StateHash = 4061194924u;
		Line.WorldHash = 681643174u;
		TestEqual(TEXT("The idle line is the commandlet's key"), CosmicHashRun::LineOf(Line), FString(TEXT("CosmicHash: seed 42, 3000 ticks, hash 4061194924, world 681643174")));
		Line.bPolicy = true;
		TestEqual(TEXT("the policy line says so"), CosmicHashRun::LineOf(Line), FString(TEXT("CosmicHash: seed 42, 3000 ticks, hash 4061194924, world 681643174, policy")));
		Line.Outcome = CosmicHashRun::EOutcome::MissingData;
		Line.Missing = TEXT("DT_Rules, DT_Galaxies:home");
		TestEqual(TEXT("and a refusal names what is missing"), CosmicHashRun::LineOf(Line), FString(TEXT("CosmicHash: refused - missing DT_Rules, DT_Galaxies:home")));
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

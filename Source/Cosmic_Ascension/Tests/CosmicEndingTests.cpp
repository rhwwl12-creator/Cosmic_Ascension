// Cosmic Ascension - automation tests of the ending record (B6a, commit B-6a2: the field alone - nothing writes it yet).
// Design: docs/60_DECISIONS/headers/B6a_endings_and_the_run_past_the_third_rung.md section 2.1, 6; docs/20 section 9.1, 9.2.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Containers/StringConv.h"
#include "Core/CosmicTags.h"
#include "Core/GalaxyState.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "Misc/ScopeExit.h"
#include "Save/CosmicSaveGame.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/Function.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/Package.h"

namespace CosmicEndingTests
{
	// Whether the engine's save bytes of the state hold the member name "Ending": the delta save writes a member outside a
	// TArray element only when it is off its initializer (Core/GalaxyState.h). Case-sensitive, so PendingEffect and the
	// other Pending* members never match.
	bool WritesEnding(const FGalaxyState& State)
	{
		UCosmicSaveGame* Carrier = NewObject<UCosmicSaveGame>(GetTransientPackage());
		Carrier->State = State;
		TArray<uint8> Raw;
		UGameplayStatics::SaveGameToMemory(Carrier, Raw);
		const auto Name = StringCast<ANSICHAR>(TEXT("Ending"));
		for (int32 At = 0; At + Name.Length() <= Raw.Num(); ++At)
		{
			if (FMemory::Memcmp(Raw.GetData() + At, Name.Get(), Name.Length()) == 0)
			{
				return true;
			}
		}
		return false;
	}

	bool SameRecord(const FCosmicEndingRecord& A, const FCosmicEndingRecord& B)
	{
		return A.Kind == B.Kind && A.Tick == B.Tick && A.CivID == B.CivID && A.bNoVessel == B.bNoVessel;
	}

	// The five kinds with the saved name each must keep (CLAUDE.md rule 7: tag names land in saves).
	struct FKind
	{
		FGameplayTag Tag;
		const TCHAR* SavedName;
	};

	TArray<FKind> Kinds()
	{
		return {
			{ CosmicTags::EndingRuledPeople(), TEXT("Ending.RuledPeople") },
			{ CosmicTags::EndingVessel(), TEXT("Ending.Vessel") },
			{ CosmicTags::EndingOtherPeople(), TEXT("Ending.OtherPeople") },
			{ CosmicTags::EndingNoCandidate(), TEXT("Ending.NoCandidate") },
			{ CosmicTags::EndingClock(), TEXT("Ending.Clock") },
		};
	}
}

// The ending record through the save (header section 2.1, 6): its four initializers are the frozen ones; a record left at
// them is not written, so a save made before the field (which never wrote it) loads as a run that goes on (decision 2-da);
// a standing record of every kind round trips field by field through the game's slot with the hash it was saved with; an
// empty record round trips as an empty record.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveEndingRecordTest, "Cosmic_Ascension.Sim.SaveEndingRecord", CosmicTests::Flags)
bool FCosmicSimSaveEndingRecordTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEndingTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicSaveSubsystem* Save = Sim->GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>();
	if (!TestNotNull(TEXT("The save subsystem exists"), Save))
	{
		return false;
	}
	FGuid Galaxy;
	ON_SCOPE_EXIT
	{
		if (Galaxy.IsValid())
		{
			Save->DeleteSave(Galaxy);
		}
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};

	// (1) The frozen initializers (rule 7): an empty kind, tick 0, no people, a vessel.
	const FCosmicEndingRecord Fresh;
	TestFalse(TEXT("The frozen kind is the empty tag (the run goes on)"), Fresh.Kind.IsValid());
	TestEqual(TEXT("The frozen tick is 0"), Fresh.Tick, static_cast<int64>(0));
	TestEqual(TEXT("The frozen people is none"), Fresh.CivID, FName(NAME_None));
	TestFalse(TEXT("The frozen bNoVessel is false"), Fresh.bNoVessel);
	TestTrue(TEXT("A new state's record is the frozen one"), SameRecord(FGalaxyState().Ending, Fresh));

	// (2) The five kinds are declared under their saved names (an undeclared name would request the empty tag).
	for (const FKind& Kind : Kinds())
	{
		TestTrue(*FString::Printf(TEXT("%s is declared"), Kind.SavedName), Kind.Tag.IsValid());
		TestEqual(*FString::Printf(TEXT("%s keeps its saved name"), Kind.SavedName), Kind.Tag.GetTagName(), FName(Kind.SavedName));
	}

	// (3) The delta save: a record at its initializers is not written; a record with any one field off them is.
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(200);
	FGalaxyState& Live = Sim->GetMutableState();
	Galaxy = Live.GalaxyID;
	TestTrue(TEXT("(a plain run leaves the record empty - nothing writes it before B-6a3b)"), SameRecord(Live.Ending, Fresh));
	TestFalse(TEXT("An empty record is not written - the bytes a build before the field wrote"), WritesEnding(Live));
	{
		FGalaxyState Probe = Live;
		Probe.Ending.Kind = CosmicTags::EndingClock();
		TestTrue(TEXT("A record with a kind is written"), WritesEnding(Probe));
		Probe.Ending = FCosmicEndingRecord();
		Probe.Ending.Tick = 6000;
		TestTrue(TEXT("A record with a tick is written"), WritesEnding(Probe));
		Probe.Ending = FCosmicEndingRecord();
		Probe.Ending.CivID = Live.Civs[0].CivID;
		TestTrue(TEXT("A record with a people is written"), WritesEnding(Probe));
		Probe.Ending = FCosmicEndingRecord();
		Probe.Ending.bNoVessel = true;
		TestTrue(TEXT("A record with bNoVessel is written"), WritesEnding(Probe));
	}

	// (4) An empty record round trips as an empty record, with its hash.
	{
		const uint32 Hash = Sim->ComputeStateHash();
		const uint32 World = Sim->ComputeWorldHash();
		if (TestTrue(TEXT("The run with an empty record saves"), Save->SaveCurrentGalaxy()))
		{
			Live.Ending.Kind = CosmicTags::EndingClock();
			Live.Ending.Tick = 1;
			if (TestTrue(TEXT("and loads"), Save->LoadGalaxy(Galaxy)))
			{
				TestTrue(TEXT("The loaded record is empty - the run goes on"), SameRecord(Sim->GetState().Ending, Fresh));
				TestEqual(TEXT("with the hash it was saved with"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Hash));
				TestEqual(TEXT("and the world hash"), static_cast<int64>(Sim->ComputeWorldHash()), static_cast<int64>(World));
			}
		}
	}

	// (5) A standing record of each kind round trips field by field through the game's slot.
	int32 Index = 0;
	for (const FKind& Kind : Kinds())
	{
		FGalaxyState& State = Sim->GetMutableState();
		FCosmicEndingRecord Standing;
		Standing.Kind = Kind.Tag;
		Standing.Tick = State.Tick - Index;
		Standing.CivID = State.Civs[Index % State.Civs.Num()].CivID;
		Standing.bNoVessel = Index % 2 == 0;
		State.Ending = Standing;
		const uint32 Hash = Sim->ComputeStateHash();
		if (!TestTrue(*FString::Printf(TEXT("The run with a %s record saves"), Kind.SavedName), Save->SaveCurrentGalaxy()))
		{
			continue;
		}
		State.Ending = FCosmicEndingRecord();
		if (TestTrue(TEXT("and loads"), Save->LoadGalaxy(Galaxy)))
		{
			const FCosmicEndingRecord& Loaded = Sim->GetState().Ending;
			TestEqual(*FString::Printf(TEXT("%s: the kind survives the round trip"), Kind.SavedName), Loaded.Kind, Standing.Kind);
			TestEqual(*FString::Printf(TEXT("%s: the tick"), Kind.SavedName), Loaded.Tick, Standing.Tick);
			TestEqual(*FString::Printf(TEXT("%s: the people"), Kind.SavedName), Loaded.CivID, Standing.CivID);
			TestEqual(*FString::Printf(TEXT("%s: bNoVessel"), Kind.SavedName), Loaded.bNoVessel, Standing.bNoVessel);
			TestEqual(*FString::Printf(TEXT("%s: the hash it was saved with"), Kind.SavedName), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Hash));
		}
		++Index;
	}
	return true;
}

// The ending record is world state (header section 2.1, decision 3): any one of its fields off its initializer moves both
// hashes, two kinds hash apart, and restoring the record restores both hashes. The exclusion lists are untouched (CLAUDE.md
// rule 6) - Sim.TellStreamSeparate classifies every top-level member, this one included.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimHashCoversEndingTest, "Cosmic_Ascension.Sim.HashCoversEnding", CosmicTests::Flags)
bool FCosmicSimHashCoversEndingTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	ON_SCOPE_EXIT
	{
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(200);
	FGalaxyState& State = Sim->GetMutableState();
	const uint32 World = Sim->ComputeWorldHash();
	const uint32 Full = Sim->ComputeStateHash();

	auto Moves = [&](const TCHAR* What, TFunctionRef<void(FCosmicEndingRecord&)> Set)
	{
		Set(State.Ending);
		TestTrue(*FString::Printf(TEXT("%s moves the world hash"), What), Sim->ComputeWorldHash() != World);
		TestTrue(*FString::Printf(TEXT("%s moves the full hash"), What), Sim->ComputeStateHash() != Full);
		State.Ending = FCosmicEndingRecord();
		TestTrue(*FString::Printf(TEXT("(both hashes are back after %s is cleared)"), What), Sim->ComputeWorldHash() == World && Sim->ComputeStateHash() == Full);
	};
	Moves(TEXT("A kind"), [](FCosmicEndingRecord& Record) { Record.Kind = CosmicTags::EndingClock(); });
	Moves(TEXT("A tick"), [](FCosmicEndingRecord& Record) { Record.Tick = 6000; });
	Moves(TEXT("A people"), [&State](FCosmicEndingRecord& Record) { Record.CivID = State.Civs[0].CivID; });
	Moves(TEXT("bNoVessel"), [](FCosmicEndingRecord& Record) { Record.bNoVessel = true; });

	// The kind is hashed by its name: two kinds at the same tick hash apart in both hashes.
	State.Ending.Kind = CosmicTags::EndingClock();
	State.Ending.Tick = 6000;
	const uint32 ClockWorld = Sim->ComputeWorldHash();
	const uint32 ClockFull = Sim->ComputeStateHash();
	State.Ending.Kind = CosmicTags::EndingNoCandidate();
	TestTrue(TEXT("Two kinds hash apart in the world hash"), Sim->ComputeWorldHash() != ClockWorld);
	TestTrue(TEXT("and in the full hash"), Sim->ComputeStateHash() != ClockFull);
	State.Ending = FCosmicEndingRecord();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

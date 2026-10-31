// Cosmic Ascension - automation tests of the ending record (B6a): the field through the save and the hashes (commit B-6a2), and
// from B-6a3b the predicate CosmicEndings::EndingOf, the one write after the tenth stage, a world the record leaves alone, and
// the silence as an ending.
// Design: docs/60_DECISIONS/headers/B6a_endings_and_the_run_past_the_third_rung.md section 2.1-2.5, 2.8, 2.9, 6; docs/20
// section 9.1, 9.2.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Containers/StringConv.h"
#include "Core/CosmicIDs.h"
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "Misc/ScopeExit.h"
#include "Save/CosmicSaveGame.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/CosmicEndings.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicSuccession.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/Function.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/Package.h"

#include <initializer_list>

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

	// ---- Hand-built states for the predicate (B-6a3b) ----

	const FName PeopleA(TEXT("civ_ending_a"));
	const FName PeopleB(TEXT("civ_ending_b"));
	const FName PeopleC(TEXT("civ_ending_c"));

	struct FPeopleAt
	{
		FName CivID;
		EAscensionTier Tier;
	};

	// One life with its peak and the peoples it ruled, in order; DiedTick 0 (an open life).
	FLifeRecord EndingLifeOf(EAscensionTier Peak, std::initializer_list<FName> Ruled)
	{
		FLifeRecord Life;
		Life.LifeIndex = 1;
		Life.PeakTier = Peak;
		for (const FName& CivID : Ruled)
		{
			Life.RuledCivIDs.Add(CivID);
		}
		Life.BirthCivID = Life.RuledCivIDs.IsEmpty() ? NAME_None : Life.RuledCivIDs[0];
		return Life;
	}

	// A state at Tick with one life (peak Peak, ruling Ruled) - embodied, personal tier the first - and the peoples in Civs order.
	FGalaxyState EndingStateOf(int64 Tick, EAscensionTier Peak, std::initializer_list<FName> Ruled, std::initializer_list<FPeopleAt> Peoples)
	{
		FGalaxyState State;
		State.Tick = Tick;
		State.Calling.Lives.Add(EndingLifeOf(Peak, Ruled));
		for (const FPeopleAt& People : Peoples)
		{
			FCivilizationState& Civ = State.Civs.AddDefaulted_GetRef();
			Civ.CivID = People.CivID;
			Civ.Tier = People.Tier;
		}
		return State;
	}

	// The calling put in the interregnum: the last life died at DiedTick, LeapLeft ticks of the leap remain (0 = the silence).
	void PutInInterregnum(FGalaxyState& State, int64 DiedTick, int64 LeapLeft)
	{
		State.Calling.LifeState = ELifeState::Interregnum;
		State.Calling.PendingLeapTicks = LeapLeft;
		if (!State.Calling.Lives.IsEmpty())
		{
			State.Calling.Lives.Last().DiedTick = DiedTick;
		}
	}

	// ---- Live boards (B-6a3b) ----

	// The game's rules with the clock at ClockTick (0 = no clock).
	FCosmicRulesDef EndingClockAt(const FCosmicRulesDef& Base, int32 ClockTick)
	{
		FCosmicRulesDef Rules = Base;
		Rules.ProvisionalEndTick = ClockTick;
		return Rules;
	}

	// The game's rules with the interregnum's leap exactly Ticks long (one number either way - B5 decision 3).
	FCosmicRulesDef EndingLeapOf(const FCosmicRulesDef& Base, int32 Ticks)
	{
		FCosmicRulesDef Rules = Base;
		Rules.LeapTicksMin = Ticks;
		Rules.LeapTicksMax = Ticks;
		return Rules;
	}

	// The game's tables without a KillPlayer (no death but the ones a test records) and with Rules as DT_Rules.
	FCosmicDataTables EndingTables(const FCosmicDataTables& Original, const FCosmicRulesDef& Rules)
	{
		return CosmicTests::WithRules(CosmicTests::WithoutDeath(Original), Rules);
	}

	// A new galaxy (seed 42) of the given size, paused, at tick At - 1: the next RunHeadless(1) is the tick At.
	FGalaxyState& LayEndingBoardAt(UGalaxySimulationSubsystem& Sim, int32 Systems, int32 Civs, int64 At)
	{
		Sim.NewGalaxy(42, Systems, Civs);
		Sim.SetTimeScale(0.f);
		FGalaxyState& State = Sim.GetMutableState();
		State.Tick = At - 1;
		Sim.SyncRngState();
		return State;
	}

	// A death recorded on the live state as Apply records one: the tenth stage of the next tick carries it out.
	void RecordEndingDeath(FGalaxyState& State)
	{
		State.Calling.PendingEffect = CosmicTags::EffectKillPlayer();
		State.Calling.PendingCause = CosmicTags::DeathHazard();
		State.Calling.PendingSourceRow = FName(TEXT("crisis_ai_awakening"));
	}

	// The silence ahead (UI.Epitaph's LeaveNoCandidate - Tests/CosmicCallingPagesTests.cpp): every system but the vessel's
	// and the sanctuary leaves the vessel's galaxy row, so that no candidate stands at the leap's end.
	void LeaveNoEndingCandidate(FGalaxyState& State, const FCosmicStateIndex& Index)
	{
		const int32 Here = Index.FindSystem(State, State.Player.CurrentSystemID);
		for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num(); ++SystemIndex)
		{
			if (SystemIndex != Here && !CosmicState::IsSanctuary(State.Systems[SystemIndex].SystemID))
			{
				State.Systems[SystemIndex].GalaxyRow = FName(TEXT("galaxy_test_apart"));
			}
		}
	}

	// The index in Civs of a people that is not the ruled one (INDEX_NONE when there is none).
	int32 UnruledCivIndexOf(const FGalaxyState& State)
	{
		for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
		{
			if (State.Civs[CivIndex].CivID != State.Player.RuledCivID)
			{
				return CivIndex;
			}
		}
		return INDEX_NONE;
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

// The ending predicate (header section 2.2, 2.4; decision 5) on hand-built states: the five kinds and none (no life is none,
// even past the clock); the order RuledPeople > Vessel > OtherPeople > NoCandidate > Clock (mutation: OtherPeople put before
// RuledPeople answers the first fifth-rung people of Civs); the people (RuledPeople: the life's ruled people at the fifth rung,
// scanned from the end of its RuledCivIDs; OtherPeople: the first fifth-rung people in Civs order; none for the other kinds);
// bNoVessel (the interregnum at the tick, but not for a fifth rung on the tick of the last death - mutation: dropping the
// exception, or widening it to every kind, breaks it); a third or fourth peak is no end; knowing, the log, the chronicle and the
// tell stream change no answer (mutation: an OtherPeople that reads Knowing.MetCivs answers differently over an empty, a partial
// and a full MetCivs); a knob at 0 or below has no clock; NameOf(tag) is the leaf word.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimEndingOfTest, "Cosmic_Ascension.Sim.EndingOf", CosmicTests::Flags)
bool FCosmicSimEndingOfTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEndingTests;
	using CosmicEndings::EndingOf;
	const EAscensionTier T1 = EAscensionTier::Tier1_Planetary;
	const EAscensionTier T2 = EAscensionTier::Tier2_Stellar;
	const EAscensionTier T3 = EAscensionTier::Tier3_Causal;
	const EAscensionTier T4 = EAscensionTier::Tier4_Transcendent;
	const EAscensionTier T5 = EAscensionTier::Tier5_EternalOrder;
	const FGameplayTag NoEnd;
	const FGameplayTag Ruled = CosmicTags::EndingRuledPeople();
	const FGameplayTag Vessel = CosmicTags::EndingVessel();
	const FGameplayTag Other = CosmicTags::EndingOtherPeople();
	const FGameplayTag Silence = CosmicTags::EndingNoCandidate();
	const FGameplayTag Clock = CosmicTags::EndingClock();

	FCosmicRulesDef Rules;
	Rules.ProvisionalEndTick = 6000;

	// The answer for State under RulesUsed: its kind, and for a standing kind its people, bNoVessel and tick (= State.Tick).
	auto ExpectUnder = [&](const TCHAR* What, const FGalaxyState& State, const FCosmicRulesDef& RulesUsed, const FGameplayTag& Kind, FName CivID, bool bNoVessel)
	{
		const FCosmicEndingRecord Got = EndingOf(State, RulesUsed);
		TestTrue(*FString::Printf(TEXT("%s: %s (answered %s)"), What, CosmicEndings::NameOf(Kind), CosmicEndings::NameOf(Got.Kind)), Got.Kind == Kind);
		if (Kind.IsValid())
		{
			TestEqual(*FString::Printf(TEXT("%s: the people"), What), Got.CivID, CivID);
			TestEqual(*FString::Printf(TEXT("%s: bNoVessel"), What), Got.bNoVessel, bNoVessel);
			TestEqual(*FString::Printf(TEXT("%s: the tick is the state's"), What), Got.Tick, State.Tick);
		}
	};
	auto Expect = [&](const TCHAR* What, const FGalaxyState& State, const FGameplayTag& Kind, FName CivID, bool bNoVessel)
	{
		ExpectUnder(What, State, Rules, Kind, CivID, bNoVessel);
	};

	// ---- None ----
	{
		FGalaxyState State;
		State.Tick = 9999;
		FCivilizationState& Civ = State.Civs.AddDefaulted_GetRef();
		Civ.CivID = PeopleA;
		Civ.Tier = T5;
		State.Calling.LifeState = ELifeState::Interregnum;
		State.Calling.PendingLeapTicks = 0;
		Expect(TEXT("No life: none - past the clock, in the silence, with a people at the fifth rung"), State, NoEnd, NAME_None, false);
	}
	Expect(TEXT("The second rung at 5,999: none"), EndingStateOf(5999, T2, { PeopleA }, { { PeopleA, T2 } }), NoEnd, NAME_None, false);

	// ---- A third or fourth rung is no end (B6a decision 2-ga) ----
	Expect(TEXT("A third peak, its people at the third rung: none"), EndingStateOf(100, T3, { PeopleA }, { { PeopleA, T3 } }), NoEnd, NAME_None, false);
	Expect(TEXT("A fourth peak, its people at the fourth rung: none"), EndingStateOf(100, T4, { PeopleA }, { { PeopleA, T4 } }), NoEnd, NAME_None, false);
	Expect(TEXT("Another people at the fourth rung: none"), EndingStateOf(100, T2, { PeopleA }, { { PeopleA, T2 }, { PeopleB, T4 } }), NoEnd, NAME_None, false);

	// ---- Ending.RuledPeople: a life's peak at the fifth rung ----
	Expect(TEXT("A fifth peak"), EndingStateOf(100, T5, { PeopleA }, { { PeopleA, T5 } }), Ruled, PeopleA, false);
	Expect(TEXT("The people is scanned from the end of RuledCivIDs"), EndingStateOf(100, T5, { PeopleA, PeopleB }, { { PeopleA, T5 }, { PeopleB, T5 } }), Ruled, PeopleB, false);
	Expect(TEXT("past a later ruled people below the fifth rung"), EndingStateOf(100, T5, { PeopleA, PeopleB }, { { PeopleA, T5 }, { PeopleB, T3 } }), Ruled, PeopleA, false);
	Expect(TEXT("No ruled people at the fifth rung: no people - not another people's"), EndingStateOf(100, T5, { PeopleA }, { { PeopleA, T4 }, { PeopleC, T5 } }), Ruled, NAME_None, false);
	{
		// A life before the last counts, and the people is that life's (the last life rules B, below the fifth rung).
		FGalaxyState State = EndingStateOf(100, T5, { PeopleA }, { { PeopleA, T5 }, { PeopleB, T2 } });
		State.Calling.Lives[0].DiedTick = 50;
		FLifeRecord& Next = State.Calling.Lives.Add_GetRef(EndingLifeOf(T1, { PeopleB }));
		Next.LifeIndex = 2;
		Next.BornTick = 60;
		Expect(TEXT("A life before the last at the fifth peak: that life's people"), State, Ruled, PeopleA, false);
	}

	// ---- Ending.Vessel: the embodied vessel's personal tier at the fifth rung ----
	{
		FGalaxyState State = EndingStateOf(100, T1, { PeopleA }, { { PeopleA, T2 } });
		State.Calling.PersonalTier = T5;
		Expect(TEXT("The vessel's personal tier at the fifth rung"), State, Vessel, NAME_None, false);
		PutInInterregnum(State, 90, 10);
		Expect(TEXT("but not in the interregnum"), State, NoEnd, NAME_None, false);
	}

	// ---- Ending.OtherPeople: a people at the fifth rung the life's peak does not count ----
	Expect(TEXT("Another people at the fifth rung: the first in Civs order"), EndingStateOf(100, T3, { PeopleA }, { { PeopleA, T3 }, { PeopleB, T5 }, { PeopleC, T5 } }), Other, PeopleB, false);
	Expect(TEXT("The ruled people raised by hand, the peak below: OtherPeople"), EndingStateOf(100, T2, { PeopleA }, { { PeopleA, T5 } }), Other, PeopleA, false);

	// ---- Ending.NoCandidate: the interregnum with no leap left ----
	{
		FGalaxyState State = EndingStateOf(100, T1, { PeopleA }, { { PeopleA, T2 } });
		PutInInterregnum(State, 80, 0);
		Expect(TEXT("The silence"), State, Silence, NAME_None, true);
		State.Calling.PendingLeapTicks = 1;
		Expect(TEXT("A leap still running: none"), State, NoEnd, NAME_None, false);
		PutInInterregnum(State, 100, 0);
		Expect(TEXT("The silence on the death's tick (a leap of 0) has no vessel - the exception is the fifth rung's only"), State, Silence, NAME_None, true);
	}

	// ---- Ending.Clock ----
	Expect(TEXT("The clock at 6,000"), EndingStateOf(6000, T2, { PeopleA }, { { PeopleA, T2 } }), Clock, NAME_None, false);
	Expect(TEXT("The clock at 6,001"), EndingStateOf(6001, T2, { PeopleA }, { { PeopleA, T2 } }), Clock, NAME_None, false);
	{
		FGalaxyState State = EndingStateOf(6000, T2, { PeopleA }, { { PeopleA, T2 } });
		PutInInterregnum(State, 5000, 5);
		Expect(TEXT("The clock in the interregnum: no vessel"), State, Clock, NAME_None, true);
		PutInInterregnum(State, 6000, 5);
		Expect(TEXT("The clock on the death's tick: no vessel - the exception is the fifth rung's only"), State, Clock, NAME_None, true);
	}

	// ---- The order (decision 5) ----
	Expect(TEXT("RuledPeople over OtherPeople - though the other people comes first in Civs"), EndingStateOf(100, T5, { PeopleB }, { { PeopleA, T5 }, { PeopleB, T5 } }), Ruled, PeopleB, false);
	{
		FGalaxyState State = EndingStateOf(100, T5, { PeopleA }, { { PeopleA, T5 } });
		State.Calling.PersonalTier = T5;
		Expect(TEXT("RuledPeople over Vessel"), State, Ruled, PeopleA, false);
	}
	{
		FGalaxyState State = EndingStateOf(100, T1, { PeopleA }, { { PeopleA, T2 }, { PeopleB, T5 } });
		State.Calling.PersonalTier = T5;
		Expect(TEXT("Vessel over OtherPeople"), State, Vessel, NAME_None, false);
	}
	{
		FGalaxyState State = EndingStateOf(100, T1, { PeopleA }, { { PeopleA, T2 }, { PeopleB, T5 } });
		PutInInterregnum(State, 60, 0);
		Expect(TEXT("OtherPeople over NoCandidate (a death before: no vessel)"), State, Other, PeopleB, true);
	}
	{
		FGalaxyState State = EndingStateOf(6000, T1, { PeopleA }, { { PeopleA, T2 } });
		PutInInterregnum(State, 5500, 0);
		Expect(TEXT("NoCandidate over Clock in the silence at the clock's tick"), State, Silence, NAME_None, true);
	}
	Expect(TEXT("RuledPeople over Clock"), EndingStateOf(6000, T5, { PeopleA }, { { PeopleA, T5 } }), Ruled, PeopleA, false);

	// ---- bNoVessel and the fifth rung on the tick of the last death (header section 2.2, 2.9) ----
	{
		FGalaxyState State = EndingStateOf(100, T5, { PeopleA }, { { PeopleA, T5 } });
		PutInInterregnum(State, 100, 900);
		Expect(TEXT("RuledPeople on the tick the vessel died: the climb had a vessel"), State, Ruled, PeopleA, false);
		PutInInterregnum(State, 60, 900);
		Expect(TEXT("RuledPeople in an interregnum whose death was earlier: no vessel"), State, Ruled, PeopleA, true);
	}
	{
		FGalaxyState State = EndingStateOf(100, T2, { PeopleA }, { { PeopleA, T2 }, { PeopleB, T5 } });
		PutInInterregnum(State, 100, 900);
		Expect(TEXT("OtherPeople on the tick the vessel died: the climb had a vessel"), State, Other, PeopleB, false);
		PutInInterregnum(State, 60, 900);
		Expect(TEXT("OtherPeople in an interregnum whose death was earlier: no vessel"), State, Other, PeopleB, true);
	}

	// ---- Knowing, the log, the chronicle and the tell stream change no answer (rule 2) ----
	{
		FGalaxyState Base = EndingStateOf(100, T2, { PeopleA }, { { PeopleA, T2 }, { PeopleB, T5 }, { PeopleC, T5 } });
		auto Meet = [](FGalaxyState& State, std::initializer_list<FName> Met)
		{
			State.Knowing.MetCivs.Reset();
			for (const FName& CivID : Met)
			{
				FKnownMark& Mark = State.Knowing.MetCivs.AddDefaulted_GetRef();
				Mark.ID = CivID;
				Mark.Tick = State.Tick;
			}
		};
		Expect(TEXT("(nothing known: the first fifth-rung people in Civs)"), Base, Other, PeopleB, false);
		FGalaxyState Some = Base;
		Meet(Some, { PeopleC });
		Expect(TEXT("Only the later fifth-rung people met: the same answer"), Some, Other, PeopleB, false);
		FGalaxyState All = Base;
		Meet(All, { PeopleC, PeopleA, PeopleB });
		All.Knowing.KnownSystems.AddDefaulted();
		All.Knowing.FirstContactCivID = PeopleC;
		All.Knowing.PendingContactCivID = PeopleC;
		All.Log.AddDefaulted_GetRef().Message = FText::AsCultureInvariant(TEXT("line"));
		All.Chronicle.AddDefaulted_GetRef().Message = FText::AsCultureInvariant(TEXT("old"));
		All.TellRngState = 77;
		Expect(TEXT("Every people met, a log, a chronicle and a tell stream: the same answer"), All, Other, PeopleB, false);
		FGalaxyState Quiet = All;
		Quiet.Civs[1].Tier = T2;
		Quiet.Civs[2].Tier = T2;
		PutInInterregnum(Quiet, 60, 0);
		FGalaxyState QuietUnknown = Quiet;
		Meet(QuietUnknown, {});
		Expect(TEXT("The silence with every people met"), Quiet, Silence, NAME_None, true);
		Expect(TEXT("and with none met"), QuietUnknown, Silence, NAME_None, true);
	}

	// ---- A knob at 0 or below has no clock; the silence is no clock ----
	{
		FCosmicRulesDef NoClock = Rules;
		NoClock.ProvisionalEndTick = 0;
		ExpectUnder(TEXT("A knob of 0: no clock"), EndingStateOf(99999, T2, { PeopleA }, { { PeopleA, T2 } }), NoClock, NoEnd, NAME_None, false);
		FGalaxyState Quiet = EndingStateOf(99999, T2, { PeopleA }, { { PeopleA, T2 } });
		PutInInterregnum(Quiet, 99000, 0);
		ExpectUnder(TEXT("A knob of 0 keeps the silence"), Quiet, NoClock, Silence, NAME_None, true);
		NoClock.ProvisionalEndTick = -5;
		ExpectUnder(TEXT("A knob below 0: no clock"), EndingStateOf(99999, T2, { PeopleA }, { { PeopleA, T2 } }), NoClock, NoEnd, NAME_None, false);
	}

	// ---- The names the commandlet and the CosmicEnd line print ----
	TestEqual(TEXT("NameOf of the five kinds and the empty tag"), FString(CosmicEndings::NameOf(Ruled)) + TEXT(",") + CosmicEndings::NameOf(Vessel) + TEXT(",")
		+ CosmicEndings::NameOf(Other) + TEXT(",") + CosmicEndings::NameOf(Silence) + TEXT(",") + CosmicEndings::NameOf(Clock) + TEXT(",")
		+ CosmicEndings::NameOf(NoEnd), FString(TEXT("RuledPeople,Vessel,OtherPeople,NoCandidate,Clock,None")));
	return true;
}

// The one write (header section 2.3, 2.9, 2.12; decision 4) on a live board without deaths but the recorded ones: (1) the clock
// is written in its tick and not before - Ending.Clock with that tick and a vessel - and it is EndingOf's answer at that boundary;
// (2) a death carried out in the clock's tick gives bNoVessel true (mutation: writing before the tenth stage reads the vessel
// still standing); (3) a fifth rung that stands in the tick the vessel dies - the life's peak (RuledPeople) or another people
// (OtherPeople) - gives bNoVessel false (mutation: dropping the exception); (4) a record that stands is never written again:
// another people at the fifth rung and a later death leave it as it is (mutation: writing every tick, or whenever EndingOf
// answers); (5) the sim stream is untouched: the same seed with the clock at 6,000 and with no clock reaches the same stream
// past 6,000 (mutation: a draw in the write).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimEndingRecordOnceTest, "Cosmic_Ascension.Sim.EndingRecordOnce", CosmicTests::Flags)
bool FCosmicSimEndingRecordOnceTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEndingTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef GameRules = Sim->GetRules();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	const int32 ClockTick = 300;
	const FCosmicEndingRecord Fresh;
	Sim->SetTables(EndingTables(Original, EndingClockAt(GameRules, ClockTick)));

	// ---- (1) The clock, in its tick ----
	{
		Sim->NewGalaxy(42, 5, 3);
		FGalaxyState& State = Sim->GetMutableState();
		Sim->RunHeadless(ClockTick - 1);
		TestEqual(TEXT("(the run stands one tick before the clock)"), State.Tick, static_cast<int64>(ClockTick - 1));
		TestTrue(TEXT("One tick before the clock the record is empty"), SameRecord(State.Ending, Fresh));
		Sim->RunHeadless(1);
		TestTrue(TEXT("The clock's tick writes Ending.Clock"), State.Ending.Kind == CosmicTags::EndingClock());
		TestEqual(TEXT("with that tick"), State.Ending.Tick, static_cast<int64>(ClockTick));
		TestEqual(TEXT("no people"), State.Ending.CivID, FName(NAME_None));
		TestFalse(TEXT("and a vessel (no death on this board)"), State.Ending.bNoVessel);
		TestTrue(TEXT("The record is EndingOf's answer at the boundary"), SameRecord(State.Ending, CosmicEndings::EndingOf(State, Sim->GetRules())));

		// ---- (4) Written once: another predicate later, and a later death, change nothing ----
		const FCosmicEndingRecord Written = State.Ending;
		const int32 Unruled = UnruledCivIndexOf(State);
		if (TestTrue(TEXT("(the board has a people the vessel does not rule)"), State.Civs.IsValidIndex(Unruled)))
		{
			State.Civs[Unruled].Tier = EAscensionTier::Tier5_EternalOrder;
			TestTrue(TEXT("(another people at the fifth rung: EndingOf now answers OtherPeople)"), CosmicEndings::EndingOf(State, Sim->GetRules()).Kind == CosmicTags::EndingOtherPeople());
			Sim->RunHeadless(5);
			TestTrue(TEXT("A later tick where OtherPeople holds does not overwrite the record"), SameRecord(State.Ending, Written));
		}
		RecordEndingDeath(State);
		Sim->RunHeadless(1);
		TestTrue(TEXT("(a death after the record: the interregnum)"), State.Calling.LifeState == ELifeState::Interregnum);
		TestTrue(TEXT("A death after the record leaves it as it is - bNoVessel too"), SameRecord(State.Ending, Written));
	}

	// ---- (2) A death carried out in the clock's tick: no vessel ----
	{
		FGalaxyState& State = LayEndingBoardAt(*Sim, 5, 3, ClockTick);
		RecordEndingDeath(State);
		Sim->RunHeadless(1);
		if (TestTrue(TEXT("(the vessel died in the clock's tick, a leap ahead)"), State.Tick == ClockTick && State.Calling.LifeState == ELifeState::Interregnum
			&& State.Calling.PendingLeapTicks > 0 && State.Calling.Lives.Last().DiedTick == ClockTick))
		{
			TestTrue(TEXT("The clock is written"), State.Ending.Kind == CosmicTags::EndingClock() && State.Ending.Tick == ClockTick);
			TestTrue(TEXT("after the tenth stage: bNoVessel"), State.Ending.bNoVessel);
		}
	}

	// ---- (3) A fifth rung standing in the tick the vessel dies: the climb had a vessel ----
	{
		FGalaxyState& State = LayEndingBoardAt(*Sim, 5, 3, ClockTick);
		const int32 RuledIndex = Sim->GetStateIndex().FindCiv(State, State.Player.RuledCivID);
		if (TestTrue(TEXT("(the vessel rules a people of the board)"), State.Player.Role == EPlayerRole::Ruler && State.Civs.IsValidIndex(RuledIndex)
			&& !State.Calling.Lives.IsEmpty() && State.Calling.Lives.Last().RuledCivIDs.Contains(State.Player.RuledCivID)))
		{
			const FName RuledCiv = State.Player.RuledCivID;
			State.Civs[RuledIndex].Tier = EAscensionTier::Tier5_EternalOrder;
			State.Calling.Lives.Last().PeakTier = EAscensionTier::Tier5_EternalOrder;
			RecordEndingDeath(State);
			Sim->RunHeadless(1);
			if (TestTrue(TEXT("(the vessel died in the tick)"), State.Calling.LifeState == ELifeState::Interregnum && State.Calling.Lives.Last().DiedTick == ClockTick))
			{
				TestTrue(TEXT("The life's fifth peak in the death's tick: RuledPeople - over the clock of the same tick"), State.Ending.Kind == CosmicTags::EndingRuledPeople());
				TestEqual(TEXT("its people the ruled one"), State.Ending.CivID, RuledCiv);
				TestEqual(TEXT("its tick"), State.Ending.Tick, static_cast<int64>(ClockTick));
				TestFalse(TEXT("and a vessel: the climb came before the tenth stage's death"), State.Ending.bNoVessel);
			}
		}
	}
	{
		FGalaxyState& State = LayEndingBoardAt(*Sim, 5, 3, ClockTick);
		const int32 Unruled = UnruledCivIndexOf(State);
		if (TestTrue(TEXT("(the board has a people the vessel does not rule)"), State.Civs.IsValidIndex(Unruled)))
		{
			const FName OtherCiv = State.Civs[Unruled].CivID;
			State.Civs[Unruled].Tier = EAscensionTier::Tier5_EternalOrder;
			RecordEndingDeath(State);
			Sim->RunHeadless(1);
			if (TestTrue(TEXT("(the vessel died in the tick)"), State.Calling.LifeState == ELifeState::Interregnum && State.Calling.Lives.Last().DiedTick == ClockTick))
			{
				TestTrue(TEXT("Another people's fifth rung in the death's tick: OtherPeople"), State.Ending.Kind == CosmicTags::EndingOtherPeople());
				TestEqual(TEXT("its people that one"), State.Ending.CivID, OtherCiv);
				TestFalse(TEXT("and a vessel"), State.Ending.bNoVessel);
			}
		}
	}

	// ---- (5) The write draws nothing: the clock at 6,000 and no clock reach the same stream past 6,000 ----
	{
		auto StreamWith = [&](int32 Knob, FCosmicEndingRecord& OutRecord) -> TPair<int32, int32>
		{
			Sim->SetTables(EndingTables(Original, EndingClockAt(GameRules, Knob)));
			Sim->NewGalaxy(42, 5, 3);
			Sim->RunHeadless(6005);
			OutRecord = Sim->GetState().Ending;
			return TPair<int32, int32>(Sim->GetRng().GetCurrentSeed(), Sim->GetState().RngState);
		};
		FCosmicEndingRecord Clocked;
		FCosmicEndingRecord Unclocked;
		const TPair<int32, int32> WithClock = StreamWith(6000, Clocked);
		const TPair<int32, int32> WithoutClock = StreamWith(0, Unclocked);
		TestTrue(TEXT("(the clock run wrote Ending.Clock at 6,000)"), Clocked.Kind == CosmicTags::EndingClock() && Clocked.Tick == 6000);
		TestTrue(TEXT("(the run with no clock wrote nothing)"), SameRecord(Unclocked, Fresh));
		TestEqual(TEXT("The sim stream is the same with the write and without"), WithClock.Key, WithoutClock.Key);
		TestEqual(TEXT("and so is the saved RngState"), WithClock.Value, WithoutClock.Value);
	}
	return true;
}

// Nothing reads the record (header section 2.5): the same seed with the clock at 6,000 and with no clock, each run to 7,000 -
// a thousand ticks past the end - differ in the record alone: the record of the first put on the second gives the same world
// hash, the same state hash and the same stream (mutation: a stage that reads the record and skips something after the end; a
// line told at the write moves the state hash).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimEndingInertTest, "Cosmic_Ascension.Sim.EndingInert", CosmicTests::Flags)
bool FCosmicSimEndingInertTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEndingTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef GameRules = Sim->GetRules();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	const int64 RunTo = 7000;

	Sim->SetTables(EndingTables(Original, EndingClockAt(GameRules, 6000)));
	Sim->NewGalaxy(42, 5, 3);
	Sim->RunHeadless(RunTo);
	const FCosmicEndingRecord Ended = Sim->GetState().Ending;
	const uint32 EndedWorld = Sim->ComputeWorldHash();
	const uint32 EndedFull = Sim->ComputeStateHash();
	const int32 EndedStream = Sim->GetState().RngState;
	TestTrue(TEXT("(the run with the clock ended at 6,000, a vessel standing)"), Ended.Kind == CosmicTags::EndingClock() && Ended.Tick == 6000 && !Ended.bNoVessel);

	Sim->SetTables(EndingTables(Original, EndingClockAt(GameRules, 0)));
	Sim->NewGalaxy(42, 5, 3);
	Sim->RunHeadless(RunTo);
	FGalaxyState& State = Sim->GetMutableState();
	TestEqual(TEXT("(both runs reach the same tick)"), State.Tick, RunTo);
	TestFalse(TEXT("The two records differ"), SameRecord(State.Ending, Ended));
	TestTrue(TEXT("(the run with no clock has none)"), !State.Ending.Kind.IsValid());
	TestTrue(TEXT("(so the world hashes differ by the record)"), Sim->ComputeWorldHash() != EndedWorld);
	State.Ending = Ended;
	TestEqual(TEXT("With the record put back the world is the same a thousand ticks past the end"), static_cast<int64>(Sim->ComputeWorldHash()), static_cast<int64>(EndedWorld));
	TestEqual(TEXT("and the whole state"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(EndedFull));
	TestEqual(TEXT("and the sim stream"), State.RngState, EndedStream);
	return true;
}

// The silence is an ending (header section 2.8, decision 8): on a board where no candidate stands in the vessel's galaxy row, a
// death with a leap of 40 writes nothing in the leap and Ending.NoCandidate with no vessel on the succession's tick (the leap's
// end), where CosmicSuccession::IsSilent holds; the calling stays in the interregnum, never succeeds again and the record stays
// over the next hundred ticks. A leap of 0 (the cap reached) writes it on the death's tick, still with no vessel. IsSilent's
// table: embodied, or a leap left, is not the silence (mutation: IsSilent without the leap test writes NoCandidate in the leap).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSilenceEndsTest, "Cosmic_Ascension.Sim.SilenceEnds", CosmicTests::Flags)
bool FCosmicSimSilenceEndsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEndingTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef GameRules = Sim->GetRules();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};

	// ---- IsSilent, from the calling alone ----
	{
		FCallingState Calling;
		TestFalse(TEXT("IsSilent: an embodied calling is not the silence"), CosmicSuccession::IsSilent(Calling));
		Calling.LifeState = ELifeState::Interregnum;
		Calling.PendingLeapTicks = 3;
		TestFalse(TEXT("IsSilent: nor an interregnum with a leap left"), CosmicSuccession::IsSilent(Calling));
		Calling.PendingLeapTicks = 0;
		TestTrue(TEXT("IsSilent: the interregnum with no leap left is"), CosmicSuccession::IsSilent(Calling));
	}

	const int64 DeathTick = 400;
	const int32 Leap = 40;
	const FCosmicEndingRecord Fresh;

	// ---- A leap of 40 into the silence ----
	{
		Sim->SetTables(EndingTables(Original, EndingLeapOf(GameRules, Leap)));
		FGalaxyState& State = LayEndingBoardAt(*Sim, CosmicTests::SliceSystems, CosmicTests::SliceCivs, DeathTick);
		LeaveNoEndingCandidate(State, Sim->GetStateIndex());
		RecordEndingDeath(State);
		Sim->RunHeadless(1);
		if (!TestTrue(TEXT("(the vessel died with a leap of 40 ahead)"), State.Tick == DeathTick && State.Calling.LifeState == ELifeState::Interregnum
			&& State.Calling.PendingLeapTicks == Leap && State.Calling.Lives.Num() == 1 && State.Calling.Lives.Last().DiedTick == DeathTick))
		{
			return false;
		}
		TestTrue(TEXT("The death's tick writes nothing: a leap runs"), SameRecord(State.Ending, Fresh));
		Sim->RunHeadless(Leap - 1);
		TestEqual(TEXT("(one tick of the leap is left)"), State.Calling.PendingLeapTicks, static_cast<int64>(1));
		TestTrue(TEXT("Nothing is written in the leap"), SameRecord(State.Ending, Fresh));
		Sim->RunHeadless(1);
		const int64 SuccessionTick = DeathTick + Leap;
		TestTrue(TEXT("(the succession's tick found no candidate: the silence)"), State.Tick == SuccessionTick && CosmicSuccession::IsSilent(State.Calling)
			&& State.Calling.Lives.Num() == 1);
		TestTrue(TEXT("The succession's tick writes Ending.NoCandidate"), State.Ending.Kind == CosmicTags::EndingNoCandidate());
		TestEqual(TEXT("with that tick"), State.Ending.Tick, SuccessionTick);
		TestEqual(TEXT("no people"), State.Ending.CivID, FName(NAME_None));
		TestTrue(TEXT("and no vessel"), State.Ending.bNoVessel);
		const FCosmicEndingRecord Written = State.Ending;
		Sim->RunHeadless(100);
		TestTrue(TEXT("A hundred ticks on, the calling waits in the interregnum with no leap"), State.Calling.LifeState == ELifeState::Interregnum
			&& State.Calling.PendingLeapTicks == 0);
		TestEqual(TEXT("and never succeeds again: one life"), State.Calling.Lives.Num(), 1);
		TestTrue(TEXT("The record stays as it was written"), SameRecord(State.Ending, Written));
	}

	// ---- A leap of 0: the silence on the death's tick ----
	{
		FCosmicRulesDef Capped = EndingLeapOf(GameRules, Leap);
		Capped.LeapTotalCap = 0;
		Sim->SetTables(EndingTables(Original, Capped));
		FGalaxyState& State = LayEndingBoardAt(*Sim, CosmicTests::SliceSystems, CosmicTests::SliceCivs, DeathTick);
		LeaveNoEndingCandidate(State, Sim->GetStateIndex());
		RecordEndingDeath(State);
		Sim->RunHeadless(1);
		if (TestTrue(TEXT("(a leap of 0: the silence on the death's tick)"), State.Tick == DeathTick && CosmicSuccession::IsSilent(State.Calling)
			&& State.Calling.Lives.Num() == 1 && State.Calling.Lives.Last().DiedTick == DeathTick))
		{
			TestTrue(TEXT("The death's tick writes Ending.NoCandidate"), State.Ending.Kind == CosmicTags::EndingNoCandidate() && State.Ending.Tick == DeathTick);
			TestTrue(TEXT("with no vessel - the death's-tick exception is the fifth rung's only"), State.Ending.bNoVessel);
		}
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

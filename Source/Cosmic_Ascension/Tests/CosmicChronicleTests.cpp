// Cosmic Ascension - automation tests of the provisional end and the chronicle's minimal form (MVP item 4, commit A-4a).
// Design: docs/60_DECISIONS/headers/A4_provisional_end.md section 3, 4, 7; crisis_and_transcendents.md section 4.4; docs/70 section 8.4.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "Log/CosmicChronicle.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicEndings.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"

#include <initializer_list>

namespace CosmicChronicleTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	FLifeRecord LifeOf(int64 BornTick, FName BirthCiv, EAscensionTier Peak = EAscensionTier::Tier1_Planetary)
	{
		FLifeRecord Life;
		Life.LifeIndex = 1;
		Life.BornTick = BornTick;
		Life.BirthCivID = BirthCiv;
		Life.RuledCivIDs.Add(BirthCiv);
		Life.PeakTier = Peak;
		return Life;
	}

	FCosmicLogEntry EntryOf(int64 Tick, const TCHAR* Category, uint8 Severity, FName FocusCiv, const FString& Message, FName SourceRow = NAME_None, bool bUntold = false)
	{
		FCosmicLogEntry Entry;
		Entry.Tick = Tick;
		Entry.Category = Tag(Category);
		Entry.Severity = Severity;
		Entry.FocusCivID = FocusCiv;
		Entry.Message = FText::AsCultureInvariant(Message);
		Entry.SourceRow = SourceRow;
		Entry.bUntold = bUntold;
		return Entry;
	}

	const FName CivA(TEXT("civ_a"));
	const FName CivB(TEXT("civ_b"));
	const FName CivX(TEXT("civ_x"));

	TArray<FString> MessagesOf(const FCosmicChronicleChapter& Chapter)
	{
		TArray<FString> Out;
		for (const FCosmicChronicleLine& Line : Chapter.Lines)
		{
			Out.Add(Line.Entry.Message.ToString());
		}
		return Out;
	}
}

// The provisional end's predicate (header section 3.1): its table, its purity, and that nothing in the simulation stops at it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimProvisionalEndTest, "Cosmic_Ascension.Sim.ProvisionalEnd", CosmicTests::Flags)
bool FCosmicSimProvisionalEndTest::RunTest(const FString& Parameters)
{
	using namespace CosmicChronicleTests;
	using CosmicEndings::ProvisionalEndOf;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	TestEqual(TEXT("The game's clock end is tick 6,000"), Sim->GetRules().ProvisionalEndTick, 6000);
	TestEqual(TEXT("The end's rung is the third"), CosmicEndings::EndRung, EAscensionTier::Tier3_Causal);

	FCosmicRulesDef Rules;
	Rules.ProvisionalEndTick = 6000;
	auto With = [](int64 Tick, std::initializer_list<EAscensionTier> Peaks)
	{
		FGalaxyState State;
		State.Tick = Tick;
		for (const EAscensionTier Peak : Peaks)
		{
			State.Calling.Lives.Add(LifeOf(0, CivA, Peak));
		}
		return State;
	};
	const EAscensionTier T1 = EAscensionTier::Tier1_Planetary;
	const EAscensionTier T2 = EAscensionTier::Tier2_Stellar;
	const EAscensionTier T3 = EAscensionTier::Tier3_Causal;

	TestEqual(TEXT("No life: no end, not even past the clock"), ProvisionalEndOf(With(9999, {}), Rules), ECosmicProvisionalEnd::None);
	TestEqual(TEXT("The second rung at 5,999: none"), ProvisionalEndOf(With(5999, { T2 }), Rules), ECosmicProvisionalEnd::None);
	TestEqual(TEXT("at 6,000: the clock"), ProvisionalEndOf(With(6000, { T2 }), Rules), ECosmicProvisionalEnd::Clock);
	TestEqual(TEXT("at 6,001: the clock"), ProvisionalEndOf(With(6001, { T2 }), Rules), ECosmicProvisionalEnd::Clock);
	TestEqual(TEXT("The third rung at 100: the rung"), ProvisionalEndOf(With(100, { T3 }), Rules), ECosmicProvisionalEnd::Rung);
	TestEqual(TEXT("The rung wins a tie with the clock"), ProvisionalEndOf(With(6000, { T3 }), Rules), ECosmicProvisionalEnd::Rung);
	TestEqual(TEXT("A fourth rung is past the third: the rung"), ProvisionalEndOf(With(100, { EAscensionTier::Tier4_Transcendent }), Rules), ECosmicProvisionalEnd::Rung);
	TestEqual(TEXT("A life before the last that reached the third rung: the rung"), ProvisionalEndOf(With(100, { T3, T1 }), Rules), ECosmicProvisionalEnd::Rung);
	{
		FCosmicRulesDef NoClock = Rules;
		NoClock.ProvisionalEndTick = 0;
		TestEqual(TEXT("A knob of 0 has no clock end"), ProvisionalEndOf(With(99999, { T2 }), NoClock), ECosmicProvisionalEnd::None);
		TestEqual(TEXT("and the rung still ends"), ProvisionalEndOf(With(10, { T3 }), NoClock), ECosmicProvisionalEnd::Rung);
		NoClock.ProvisionalEndTick = -5;
		TestEqual(TEXT("A knob below 0 has no clock end"), ProvisionalEndOf(With(99999, { T2 }), NoClock), ECosmicProvisionalEnd::None);
	}
	{
		// A people's Tier alone is not the life's rung: the predicate reads Calling.Lives, never Civs.
		FGalaxyState State = With(100, { T2 });
		FCivilizationState& Civ = State.Civs.AddDefaulted_GetRef();
		Civ.CivID = CivA;
		Civ.Tier = T3;
		Civ.bIsPlayerRuled = true;
		TestEqual(TEXT("A people raised by hand to the third rung, its life's peak the second: none"), ProvisionalEndOf(State, Rules), ECosmicProvisionalEnd::None);
		// Knowing, the log, the chronicle and the tell stream change nothing.
		FGalaxyState Told = With(6000, { T2 });
		Told.Knowing.KnownSystems.AddDefaulted();
		Told.Log.Add(EntryOf(5, TEXT("Log.Calling"), 2, CivA, TEXT("line")));
		Told.Chronicle.Add(EntryOf(4, TEXT("Log.Calling"), 2, CivA, TEXT("old")));
		Told.TellRngState = 77;
		TestEqual(TEXT("Knowing, the log, the chronicle and the tell stream change nothing"), ProvisionalEndOf(Told, Rules), ECosmicProvisionalEnd::Clock);
	}
	TestEqual(TEXT("The names of the causes"), FString(CosmicEndings::NameOf(ECosmicProvisionalEnd::Rung)) + CosmicEndings::NameOf(ECosmicProvisionalEnd::Clock)
		+ CosmicEndings::NameOf(ECosmicProvisionalEnd::None), FString(TEXT("RungClockNone")));

	// Pure, and nothing stops at it: a run at its rung end goes on, and the knob changes no world.
	Sim->NewGalaxy(42, 5, 3);
	{
		FGalaxyState& Live = Sim->GetMutableState();
		if (TestFalse(TEXT("(the board has a life)"), Live.Calling.Lives.IsEmpty()))
		{
			Live.Calling.Lives.Last().PeakTier = T3;
		}
		Sim->SyncRngState();
		const uint32 Whole = Sim->ComputeStateHash();
		const uint32 World = Sim->ComputeWorldHash();
		const int32 Stream = Live.RngState;
		for (int32 Call = 0; Call < 1000; ++Call)
		{
			ProvisionalEndOf(Sim->GetState(), Sim->GetRules());
		}
		TestEqual(TEXT("Reading the end changes no state"), Sim->ComputeStateHash(), Whole);
		TestEqual(TEXT("nor the world"), Sim->ComputeWorldHash(), World);
		TestEqual(TEXT("nor the stream"), Sim->GetState().RngState, Stream);
		TestEqual(TEXT("(the run is at its rung end)"), ProvisionalEndOf(Sim->GetState(), Sim->GetRules()), ECosmicProvisionalEnd::Rung);
		const int64 Before = Live.Tick;
		Sim->RunHeadless(5);
		TestEqual(TEXT("RunHeadless goes on past the end"), Live.Tick, Before + 5);
		Sim->StepOnce();
		TestEqual(TEXT("and so does StepOnce"), Live.Tick, Before + 6);
	}
	auto WorldWith = [&](int32 Knob)
	{
		FCosmicRulesDef Knobbed = *Original.FindRules();
		Knobbed.ProvisionalEndTick = Knob;
		Sim->SetTables(CosmicTests::WithRules(Original, Knobbed));
		Sim->NewGalaxy(42, 5, 3);
		Sim->RunHeadless(300);
		return TPair<uint32, uint32>(Sim->ComputeStateHash(), Sim->ComputeWorldHash());
	};
	const TPair<uint32, uint32> Early = WorldWith(200);
	const TPair<uint32, uint32> Never = WorldWith(0);
	TestEqual(TEXT("A clock end at 200 is the same run as none (state)"), Early.Key, Never.Key);
	TestEqual(TEXT("and the same world"), Early.Value, Never.Value);
	return true;
}

// The chronicle's minimal form (header section 4): which entries it admits, the chapters, the cap and its ranks, the repeat
// rule, the ranges, that it reads no knowing and writes nothing, and the vessel's name.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLogChronicleBuildTest, "Cosmic_Ascension.Log.ChronicleBuild", CosmicTests::Flags)
bool FCosmicLogChronicleBuildTest::RunTest(const FString& Parameters)
{
	using namespace CosmicChronicleTests;
	using CosmicChronicle::Admits;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	ON_SCOPE_EXIT { Sim->NewGalaxy(42, 5, 3); };

	// ---- Admits, from the eight fields alone ----
	{
		const TArray<FName> Peoples = { CivA, CivB };
		struct FCase { FCosmicLogEntry Entry; bool bAdmitted; const TCHAR* What; };
		const FCase Cases[] = {
			{ EntryOf(1, TEXT("Log.Civ"), 0, CivA, TEXT("s0")), false, TEXT("Severity 0 of the own people") },
			{ EntryOf(1, TEXT("Log.Civ"), 1, CivA, TEXT("s1")), true, TEXT("Severity 1 of the birth people") },
			{ EntryOf(1, TEXT("Log.Civ"), 1, CivB, TEXT("s1")), true, TEXT("Severity 1 of a people ruled later") },
			{ EntryOf(1, TEXT("Log.Civ"), 1, CivX, TEXT("s1")), false, TEXT("Severity 1 of another people") },
			{ EntryOf(1, TEXT("Log.Discovery"), 1, CivX, TEXT("met"), CosmicRows::LogFirstContact), true, TEXT("the first contact") },
			{ EntryOf(1, TEXT("Log.Discovery"), 1, CivX, TEXT("met again"), FName(TEXT("log_contact"))), false, TEXT("a later contact") },
			{ EntryOf(1, TEXT("Log.Civ"), 1, NAME_None, TEXT("far")), false, TEXT("Severity 1 with no people") },
			{ EntryOf(1, TEXT("Log.Calling"), 0, NAME_None, TEXT("calling")), true, TEXT("Log.Calling of Severity 0") },
			{ EntryOf(1, TEXT("Log.Claim"), 1, CivX, TEXT("claim"), NAME_None, true), true, TEXT("Log.Claim of another people, untold") },
			{ EntryOf(1, TEXT("Log.Record"), 0, CivX, TEXT("record")), true, TEXT("Log.Record of Severity 0") },
			{ EntryOf(1, TEXT("Log.Civ"), 2, CivX, TEXT("s2"), NAME_None, true), true, TEXT("Severity 2 of another people, untold") },
		};
		for (const FCase& Case : Cases)
		{
			TestEqual(*FString::Printf(TEXT("Admits: %s"), Case.What), Admits(Case.Entry, Peoples), Case.bAdmitted);
			FCosmicLogEntry Flipped = Case.Entry;
			Flipped.bUntold = !Flipped.bUntold;
			TestEqual(*FString::Printf(TEXT("and bUntold does not change it: %s"), Case.What), Admits(Flipped, Peoples), Case.bAdmitted);
		}
		const FLifeRecord Life = LifeOf(0, CivA);
		FLifeRecord Twice = Life;
		Twice.RuledCivIDs = { CivA, NAME_None, CivB, CivB };
		TestEqual(TEXT("The peoples of a life: birth first, ruled after, no None, no repeat"), CosmicChronicle::PeoplesOf(Twice), TArray<FName>({ CivA, CivB }));
		FLifeRecord Moved = Life;
		Moved.RuledCivIDs = { CivB };
		TestEqual(TEXT("The birth people counts though the life never ruled it"), CosmicChronicle::PeoplesOf(Moved), TArray<FName>({ CivA, CivB }));
	}

	// ---- One chapter: the chronicle before the log, the two books' entries out, the first repeat rule ----
	{
		FGalaxyState State;
		State.Tick = 50;
		State.Calling.Lives.Add(LifeOf(0, CivA));
		State.Chronicle.Add(EntryOf(5, TEXT("Log.Civ"), 2, CivA, TEXT("old")));
		State.Log.Add(EntryOf(10, TEXT("Log.Civ"), 1, CivA, TEXT("new")));
		State.Log.Add(EntryOf(11, TEXT("Log.Claim"), 1, CivA, TEXT("claim")));
		State.Log.Add(EntryOf(12, TEXT("Log.Record"), 1, CivA, TEXT("record")));
		State.Log.Add(EntryOf(13, TEXT("Log.Civ"), 1, CivA, TEXT("twice")));
		State.Log.Add(EntryOf(14, TEXT("Log.Civ"), 1, CivA, TEXT("twice")));
		State.Log.Add(EntryOf(15, TEXT("Log.Civ"), 2, CivX, TEXT("other")));
		const TArray<FCosmicChronicleChapter> Chapters = CosmicChronicle::Build(State);
		if (TestEqual(TEXT("One life, one chapter"), Chapters.Num(), 1))
		{
			TestEqual(TEXT("The chronicle's entry before the log's, the books' entries out, a repeat dropped"), MessagesOf(Chapters[0]),
				TArray<FString>({ TEXT("old"), TEXT("new"), TEXT("twice"), TEXT("other") }));
			TestEqual(TEXT("The candidates are counted after the repeat"), Chapters[0].Candidates, 4);
			TestFalse(TEXT("Under the cap nothing more"), Chapters[0].bMore);
			TestTrue(TEXT("The own people's line is the life's own"), Chapters[0].Lines.Num() == 4 && Chapters[0].Lines[0].bOwn && Chapters[0].Lines[1].bOwn);
			TestFalse(TEXT("another people's Severity 2 is not (dimmed)"), Chapters[0].Lines.Num() == 4 && Chapters[0].Lines[3].bOwn);
			TestEqual(TEXT("The chapter runs to the current tick"), Chapters[0].ToTick, static_cast<int64>(50));
		}
		TestEqual(TEXT("No life, no chapter"), CosmicChronicle::Build(FGalaxyState()).Num(), 0);
	}

	// ---- The cap ----
	const int32 Cap = CosmicChronicle::MaxLifeLines;
	auto Fill = [](FGalaxyState& State, int32 Count, const TCHAR* Category, uint8 Severity, FName Civ, const TCHAR* Stem, int64& Tick)
	{
		for (int32 At = 0; At < Count; ++At)
		{
			State.Log.Add(EntryOf(Tick++, Category, Severity, Civ, FString::Printf(TEXT("%s %d"), Stem, At)));
		}
	};
	{
		for (const int32 Count : { Cap, Cap + 1 })
		{
			FGalaxyState State;
			State.Tick = 10000;
			State.Calling.Lives.Add(LifeOf(0, CivA));
			int64 Tick = 1;
			Fill(State, Count, TEXT("Log.Civ"), 1, CivA, TEXT("s1"), Tick);
			const FCosmicChronicleChapter Chapter = CosmicChronicle::Build(State)[0];
			TestEqual(*FString::Printf(TEXT("%d candidates: more is %s"), Count, Count > Cap ? TEXT("set") : TEXT("not set")), Chapter.bMore, Count > Cap);
			TestEqual(*FString::Printf(TEXT("and the lines are at most the cap (%d)"), Count), Chapter.Lines.Num(), FMath::Min(Count, Cap));
		}
	}
	{
		// Log.Calling whole, then another people's Severity 2 thinned by (i * n) / k.
		FGalaxyState State;
		State.Tick = 10000;
		State.Calling.Lives.Add(LifeOf(0, CivA));
		int64 Tick = 1;
		Fill(State, 3, TEXT("Log.Calling"), 1, NAME_None, TEXT("calling"), Tick);
		Fill(State, 45, TEXT("Log.Civ"), 2, CivX, TEXT("far"), Tick);
		const FCosmicChronicleChapter Chapter = CosmicChronicle::BuildCapped(State, 40)[0];
		TArray<FString> Expected = { TEXT("calling 0"), TEXT("calling 1"), TEXT("calling 2") };
		for (int32 I = 0; I < 37; ++I)
		{
			Expected.Add(FString::Printf(TEXT("far %d"), (I * 45) / 37));
		}
		TestEqual(TEXT("Log.Calling is kept whole and the next rank thinned evenly"), MessagesOf(Chapter), Expected);
		TestTrue(TEXT("and more is set"), Chapter.bMore);
		TestEqual(TEXT("The candidates count all 48"), Chapter.Candidates, 48);
	}
	{
		// Two callings, five own Severity 2, fifty Severity 1 (thinned to the first 17 and the last 16) and other peoples' lines left out.
		FGalaxyState State;
		State.Tick = 10000;
		State.Calling.Lives.Add(LifeOf(0, CivA));
		int64 Tick = 1;
		Fill(State, 2, TEXT("Log.Calling"), 0, NAME_None, TEXT("calling"), Tick);
		Fill(State, 5, TEXT("Log.Civ"), 2, CivA, TEXT("own"), Tick);
		Fill(State, 50, TEXT("Log.Civ"), 1, CivA, TEXT("choice"), Tick);
		Fill(State, 4, TEXT("Log.Civ"), 2, CivX, TEXT("far"), Tick);
		const FCosmicChronicleChapter Chapter = CosmicChronicle::BuildCapped(State, 40)[0];
		TArray<FString> Expected = { TEXT("calling 0"), TEXT("calling 1") };
		for (int32 I = 0; I < 5; ++I)
		{
			Expected.Add(FString::Printf(TEXT("own %d"), I));
		}
		for (int32 I = 0; I < 17; ++I)
		{
			Expected.Add(FString::Printf(TEXT("choice %d"), I));
		}
		for (int32 I = 34; I < 50; ++I)
		{
			Expected.Add(FString::Printf(TEXT("choice %d"), I));
		}
		TestEqual(TEXT("Severity 1 keeps its first and last lines, and the rank after it is left out"), MessagesOf(Chapter), Expected);
		TestTrue(TEXT("more is set"), Chapter.bMore);
	}
	{
		// Too many callings for the cap: they are kept whole, nothing else.
		FGalaxyState State;
		State.Tick = 10000;
		State.Calling.Lives.Add(LifeOf(0, CivA));
		int64 Tick = 1;
		Fill(State, 3, TEXT("Log.Calling"), 0, NAME_None, TEXT("calling"), Tick);
		Fill(State, 2, TEXT("Log.Civ"), 2, CivA, TEXT("own"), Tick);
		const FCosmicChronicleChapter Chapter = CosmicChronicle::BuildCapped(State, 2)[0];
		TestEqual(TEXT("Log.Calling is never cut, even over the cap"), MessagesOf(Chapter), TArray<FString>({ TEXT("calling 0"), TEXT("calling 1"), TEXT("calling 2") }));
		TestTrue(TEXT("and more is set"), Chapter.bMore);
		TestEqual(TEXT("A cap below 0 reads as 0: the callings alone"), CosmicChronicle::BuildCapped(State, -3)[0].Lines.Num(), 3);
	}
	{
		// The second repeat rule: the thinning brings two equal lines together, one of them is dropped.
		FGalaxyState State;
		State.Tick = 100;
		State.Calling.Lives.Add(LifeOf(0, CivA));
		State.Log.Add(EntryOf(1, TEXT("Log.Civ"), 1, CivA, TEXT("same")));
		State.Log.Add(EntryOf(2, TEXT("Log.Civ"), 1, CivA, TEXT("between")));
		State.Log.Add(EntryOf(3, TEXT("Log.Civ"), 1, CivA, TEXT("same")));
		const FCosmicChronicleChapter Apart = CosmicChronicle::BuildCapped(State, 3)[0];
		TestEqual(TEXT("Equal lines apart are both kept"), MessagesOf(Apart), TArray<FString>({ TEXT("same"), TEXT("between"), TEXT("same") }));
		const FCosmicChronicleChapter Thinned = CosmicChronicle::BuildCapped(State, 2)[0];
		TestEqual(TEXT("When the thinning makes them neighbours, one is left"), MessagesOf(Thinned), TArray<FString>({ TEXT("same") }));
		TestEqual(TEXT("and the candidates were three"), Thinned.Candidates, 3);
	}

	// ---- The ranges of two lives ----
	{
		FGalaxyState State;
		State.Tick = 300;
		State.Calling.Lives.Add(LifeOf(0, CivA));
		State.Calling.Lives.Add(LifeOf(100, CivB));
		State.Log.Add(EntryOf(-5, TEXT("Log.Calling"), 2, NAME_None, TEXT("before birth")));
		State.Log.Add(EntryOf(99, TEXT("Log.Civ"), 2, CivA, TEXT("first life end")));
		State.Log.Add(EntryOf(100, TEXT("Log.Civ"), 2, CivB, TEXT("second life start")));
		State.Log.Add(EntryOf(150, TEXT("Log.Civ"), 1, CivA, TEXT("old people later")));
		State.Log.Add(EntryOf(160, TEXT("Log.Civ"), 1, CivB, TEXT("new people")));
		TArray<FCosmicChronicleChapter> Chapters = CosmicChronicle::Build(State);
		if (TestEqual(TEXT("Two lives, two chapters"), Chapters.Num(), 2))
		{
			TestEqual(TEXT("The first chapter holds every earlier tick and ends before the next birth"), MessagesOf(Chapters[0]),
				TArray<FString>({ TEXT("before birth"), TEXT("first life end") }));
			TestEqual(TEXT("The second starts at its birth; the old people's Severity 1 is not its own"), MessagesOf(Chapters[1]),
				TArray<FString>({ TEXT("second life start"), TEXT("new people") }));
			TestEqual(TEXT("The first chapter ends at the next birth - 1"), Chapters[0].ToTick, static_cast<int64>(99));
			TestEqual(TEXT("The last chapter ends at the current tick"), Chapters[1].ToTick, static_cast<int64>(300));
			TestEqual(TEXT("The second chapter starts at its birth"), Chapters[1].FromTick, static_cast<int64>(100));
		}
		State.Calling.Lives[0].DiedTick = 80;
		State.Log.Add(EntryOf(90, TEXT("Log.Calling"), 2, NAME_None, TEXT("between lives")));
		State.Log.Sort([](const FCosmicLogEntry& A, const FCosmicLogEntry& B) { return A.Tick < B.Tick; });
		Chapters = CosmicChronicle::Build(State);
		TestEqual(TEXT("With a death at 80 the first chapter ends there"), Chapters[0].ToTick, static_cast<int64>(80));
		TestFalse(TEXT("and a line between the death and the next birth is in no chapter"),
			MessagesOf(Chapters[0]).Contains(TEXT("between lives")) || MessagesOf(Chapters[1]).Contains(TEXT("between lives")));
	}

	// ---- No knowing read, nothing written, the subsystem's entry point ----
	{
		FGalaxyState State;
		State.Tick = 50;
		State.Calling.Lives.Add(LifeOf(0, CivA));
		State.Log.Add(EntryOf(10, TEXT("Log.Civ"), 1, CivA, TEXT("line")));
		const TArray<FString> Plain = MessagesOf(CosmicChronicle::Build(State)[0]);
		State.Knowing.KnownSystems.AddDefaulted();
		State.Knowing.MetCivs.AddDefaulted();
		TestEqual(TEXT("Knowing changes no chapter"), MessagesOf(CosmicChronicle::Build(State)[0]), Plain);
	}
	Sim->NewGalaxy(42, 5, 3);
	Sim->RunHeadless(200);
	{
		Sim->SyncRngState();
		const uint32 Whole = Sim->ComputeStateHash();
		const uint32 World = Sim->ComputeWorldHash();
		const TArray<FCosmicChronicleChapter> Direct = CosmicChronicle::Build(Sim->GetState());
		UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
		const TArray<FCosmicChronicleChapter> Wrapped = Log ? Log->BuildChronicle() : TArray<FCosmicChronicleChapter>();
		TestEqual(TEXT("Building the chronicle changes no state"), Sim->ComputeStateHash(), Whole);
		TestEqual(TEXT("nor the world"), Sim->ComputeWorldHash(), World);
		if (TestEqual(TEXT("BuildChronicle is Build over the live state"), Wrapped.Num(), Direct.Num()) && !Direct.IsEmpty())
		{
			TestEqual(TEXT("line for line"), MessagesOf(Wrapped[0]), MessagesOf(Direct[0]));
			TestTrue(TEXT("(the run told something of its own)"), !Direct[0].Lines.IsEmpty());
		}
	}

	// ---- The vessel's name ----
	{
		UDataTable* Civs = CosmicTests::NewTable(FCivArchetypeDef::StaticStruct());
		FCivArchetypeDef Named;
		Named.PersonNameBank = { FText::AsCultureInvariant(TEXT("first")), FText::AsCultureInvariant(TEXT("second")) };
		Civs->AddRow(FName(TEXT("arch_test_named")), Named);
		Civs->AddRow(FName(TEXT("arch_test_empty")), FCivArchetypeDef());
		FCivArchetypeDef Fallback;
		Fallback.PersonNameBank = { FText::AsCultureInvariant(TEXT("default")) };
		Civs->AddRow(CosmicRows::ArchDefault, Fallback);
		FCosmicDataTables Tables;
		Tables.Civs = Civs;
		FLifeRecord Life = LifeOf(0, CivA);
		Life.BirthArchetypeRow = FName(TEXT("arch_test_named"));
		Life.NameIndex = 3;
		TestEqual(TEXT("The name is the bank's entry at NameIndex modulo its size"), CosmicChronicle::VesselNameOf(Life, Tables).ToString(), FString(TEXT("second")));
		Life.BirthArchetypeRow = FName(TEXT("arch_test_empty"));
		TestEqual(TEXT("An empty bank falls back to arch_default's"), CosmicChronicle::VesselNameOf(Life, Tables).ToString(), FString(TEXT("default")));
		FCosmicDataTables NoDefault;
		UDataTable* Bare = CosmicTests::NewTable(FCivArchetypeDef::StaticStruct());
		Bare->AddRow(FName(TEXT("arch_test_empty")), FCivArchetypeDef());
		NoDefault.Civs = Bare;
		TestTrue(TEXT("and with neither the name is empty"), CosmicChronicle::VesselNameOf(Life, NoDefault).IsEmpty());
	}

	// ---- The saved chronicle's cap (MVP item 5): a chapter whose range reaches what it let go closes with 'more' ----
	{
		FGalaxyState State;
		State.Tick = 900;
		State.Calling.Lives.Add(LifeOf(0, CivA));
		State.Calling.Lives.Add(LifeOf(500, CivB));
		auto MoreOf = [&State](int32 Count, int64 Through)
		{
			State.ChronicleEviction.Count = Count;
			State.ChronicleEviction.ThroughTick = Through;
			const TArray<FCosmicChronicleChapter> Chapters = CosmicChronicle::BuildCapped(State, 40);
			return Chapters.Num() == 2 ? TArray<bool>({ Chapters[0].bMore, Chapters[1].bMore }) : TArray<bool>();
		};
		TestEqual(TEXT("Lines let go through tick 300: the first chapter says 'more'"), MoreOf(1, 300), TArray<bool>({ true, false }));
		TestEqual(TEXT("through tick 500, the second's first tick: both do"), MoreOf(1, 500), TArray<bool>({ true, true }));
		TestEqual(TEXT("Nothing let go: neither does"), MoreOf(0, 900), TArray<bool>({ false, false }));
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

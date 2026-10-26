// Cosmic Ascension - automation tests of the log subsystem. Design: guideline.md section 7.5, 8.4, 9.3.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Events/CosmicEventSubsystem.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Log/CosmicChronicle.h"
#include "Log/CosmicLogSubsystem.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace CosmicLogTests
{
	// True when no two neighbouring entries carry the same sentence.
	bool HasNoConsecutiveRepeat(const TArray<FCosmicLogEntry>& Log)
	{
		for (int32 Index = 1; Index < Log.Num(); ++Index)
		{
			if (Log[Index].Message.ToString() == Log[Index - 1].Message.ToString())
			{
				return false;
			}
		}
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLogNoConsecutiveRepeatTest, "Cosmic_Ascension.Log.NoConsecutiveRepeat", CosmicTests::Flags)
bool FCosmicLogNoConsecutiveRepeatTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();

	Sim->NewGalaxy(42, 5, 3);
	const FGalaxyState& State = Sim->GetState();

	const FName RowName = CosmicRows::LogBreakthrough[0];
	const FLogTemplateDef* Row = Sim->GetTables().FindLogTemplate(RowName);
	if (!TestNotNull(TEXT("DT_LogTemplates holds the breakthrough row (run Tools/make_data_tables.py)"), Row)
		|| !TestTrue(TEXT("The row has more than one template"), Row->Templates.Num() > 1))
	{
		return false;
	}

	// Every row the code calls by name is in its table: a missing one is silent at run time.
	for (const FName Name : CosmicRows::LogBreakthrough)
	{
		const FLogTemplateDef* Each = Sim->GetTables().FindLogTemplate(Name);
		TestTrue(*FString::Printf(TEXT("DT_LogTemplates row %s has three templates or more"), *Name.ToString()), Each && Each->Templates.Num() >= 3);
	}
	TestNotNull(TEXT("DT_Rules holds the row rules"), Sim->GetTables().FindRules());
	TestNotNull(TEXT("DT_Civs holds arch_default"), Sim->GetTables().FindCiv(CosmicRows::ArchDefault));

	FFormatNamedArguments Args;
	Args.Add(TEXT("Civ"), FText::AsCultureInvariant(TEXT("X")));

	// The same row over and over: one entry and exactly one draw from the tell stream per call, the sim stream left
	// alone, and never the same sentence twice in a row (section 9.2).
	const int32 SimSeed = Sim->GetRng().GetCurrentSeed();
	bool bOneEntryPerCall = true;
	bool bOneDrawPerCall = true;
	bool bSimUntouched = true;
	TSet<FString> Sentences;
	for (int32 Call = 0; Call < 300; ++Call)
	{
		FRandomStream Probe(State.TellRngState);
		const int32 EntriesBefore = State.Log.Num();

		const bool bPushed = Log->PushRow(RowName, 0, Args, NAME_None);

		Probe.RandHelper(Row->Templates.Num());
		bOneEntryPerCall &= bPushed && State.Log.Num() == EntriesBefore + 1;
		bOneDrawPerCall &= Probe.GetCurrentSeed() == State.TellRngState;
		bSimUntouched &= Sim->GetRng().GetCurrentSeed() == SimSeed && State.RngState == SimSeed;
		Sentences.Add(State.Log.Last().Message.ToString());
	}
	TestTrue(TEXT("PushRow appends exactly one entry per call"), bOneEntryPerCall);
	TestTrue(TEXT("PushRow draws exactly once per call from the tell stream"), bOneDrawPerCall);
	TestTrue(TEXT("PushRow leaves the sim stream alone"), bSimUntouched);
	TestTrue(TEXT("No sentence twice in a row after 300 pushes of one row"), CosmicLogTests::HasNoConsecutiveRepeat(State.Log));
	// A draw that always returned the same number would alternate between two variants and pass the checks above.
	TestTrue(TEXT("300 pushes of one row tell three variants or more"), Sentences.Num() >= 3);
	TestEqual(TEXT("Entries carry the row as their source"), State.Log.Last().SourceRow, RowName);
	TestTrue(TEXT("Entries carry the row's category"), State.Log.Last().Category == Row->Category);
	TestTrue(TEXT("Without a subject civ the entry carries none"), State.Log.Last().FocusCivID.IsNone());

	// The subject of a line is stored with it (section 8.1).
	const FName SubjectSystem = State.Systems[1].SystemID;
	const FName SubjectCiv = State.Civs[1].CivID;
	TestTrue(TEXT("PushRow stores the subject system, the subject civ and the row"),
		Log->PushRow(RowName, 0, Args, SubjectSystem, SubjectCiv)
		&& State.Log.Last().FocusSystemID == SubjectSystem && State.Log.Last().FocusCivID == SubjectCiv && State.Log.Last().SourceRow == RowName);

	// A missing row: nothing is pushed and nothing is drawn, from either stream.
	const int32 TellBefore = State.TellRngState;
	const int32 EntriesBefore = State.Log.Num();
	TestFalse(TEXT("A missing row is not pushed"), Log->PushRow(FName(TEXT("log_no_such_row")), 0, Args, NAME_None));
	TestTrue(TEXT("A missing row draws nothing and adds nothing"),
		State.TellRngState == TellBefore && Sim->GetRng().GetCurrentSeed() == SimSeed && State.Log.Num() == EntriesBefore);

	// A row of one template still draws exactly once: the number of draws never depends on the content of a row.
	const FLogTemplateDef* FirstRow = Sim->GetTables().FindLogTemplate(CosmicRows::LogFirstLife);
	if (TestTrue(TEXT("log_first_life has exactly one template"), FirstRow && FirstRow->Templates.Num() == 1))
	{
		FRandomStream Probe(State.TellRngState);
		FFormatNamedArguments HomeArgs;
		HomeArgs.Add(TEXT("HomeSystem"), FText::AsCultureInvariant(TEXT("X")));
		TestTrue(TEXT("A one-template row is pushed"), Log->PushRow(CosmicRows::LogFirstLife, 0, HomeArgs, NAME_None));
		Probe.RandHelper(1);
		TestTrue(TEXT("A one-template row still draws exactly once from the tell stream"), Probe.GetCurrentSeed() == State.TellRngState);
		TestTrue(TEXT("A one-template row leaves the sim stream alone"), Sim->GetRng().GetCurrentSeed() == SimSeed && State.RngState == SimSeed);
	}

	// A whole run: the log the simulation writes by itself. In Phase 1 such a run never pushes the same row twice in a
	// row, so the check below gains force only with the tick logs of Phase 2.
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(20000);
	TestTrue(TEXT("The run wrote more than the first line"), State.Log.Num() + State.Chronicle.Num() > 1);
	TestTrue(TEXT("No sentence twice in a row after 20,000 ticks"), CosmicLogTests::HasNoConsecutiveRepeat(State.Log));
	return true;
}

// The move out of the log (section 8.4, MVP item 5 - 16 #40 (7)): over 2,000 the oldest entries leave; what the chronicle can
// show stays - Severity 2, Log.Calling, Log.Claim, Log.Record, and a Severity 1 entry about a people of a life or the first
// contact - and the rest is dropped. The expected rule is written here, not read from CosmicChronicle::Admits. The page reads
// as if nothing had left.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLogCapAndChronicleTest, "Cosmic_Ascension.Log.CapAndChronicle", CosmicTests::Flags)
bool FCosmicLogCapAndChronicleTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	ON_SCOPE_EXIT { Sim->NewGalaxy(42, 5, 3); };

	Sim->NewGalaxy(42, 5, 3);
	const FGalaxyState& State = Sim->GetState();

	const FGameplayTag CivTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Civ")));
	const FGameplayTag CallingTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Calling")));
	const FGameplayTag ClaimTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Claim")));
	const FGameplayTag RecordTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Record")));
	const FText Template = FText::AsCultureInvariant(TEXT("entry {Number}"));
	const FName TestRow(TEXT("log_test"));
	if (!TestTrue(TEXT("(a life rules a people and another people is there)"), State.Calling.Lives.Num() == 1 && State.Civs.Num() >= 2))
	{
		return false;
	}
	const FName Own = State.Calling.Lives[0].BirthCivID;
	const FName Other = State.Civs[0].CivID == Own ? State.Civs[1].CivID : State.Civs[0].CivID;

	// What the chronicle keeps of an entry that leaves, for the peoples of the lives.
	auto Kept = [&](const FCosmicLogEntry& Entry, const TArray<FName>& Peoples)
	{
		return Entry.Severity >= 2 || Entry.Category == CallingTag || Entry.Category == ClaimTag || Entry.Category == RecordTag
			|| (Entry.Severity == 1 && ((!Entry.FocusCivID.IsNone() && Peoples.Contains(Entry.FocusCivID)) || Entry.SourceRow == CosmicRows::LogFirstContact));
	};

	const TArray<FCosmicLogEntry> Initial = State.Log;
	TArray<FCosmicLogEntry> Pushed;
	const int32 Pushes = UCosmicLogSubsystem::MaxLogEntries + 100;
	for (int32 Index = 0; Index < Pushes; ++Index)
	{
		// Mostly ordinary lines, some notable and mythic ones, now and then an ordinary claim or record; the Severity 1 lines
		// about the ruled people, another people, nobody, and the first contact (about the other people) in turn.
		const uint8 Severity = static_cast<uint8>(Index % 3);
		const FGameplayTag Category = (Index % 50 == 0) ? ClaimTag : (Index % 50 == 25) ? RecordTag : CivTag;
		const int32 Focus = (Index / 3) % 4;
		const FName FocusCiv = Focus == 0 ? Own : Focus == 2 ? NAME_None : Other;
		const FName Row = Focus == 3 ? CosmicRows::LogFirstContact : TestRow;
		FFormatNamedArguments Args;
		Args.Add(TEXT("Number"), FText::AsCultureInvariant(FString::FromInt(Index)));
		// A stamp of its own for every entry (the first line of the galaxy carries tick 0), for the order check below.
		Sim->GetMutableState().Tick = Index + 1;
		Log->Push(Category, Severity, Template, Args, NAME_None, Row, FocusCiv);
		Pushed.Add(State.Log.Last());
	}

	TArray<FCosmicLogEntry> All = Initial;
	All.Append(Pushed);
	const int32 Departed = All.Num() - UCosmicLogSubsystem::MaxLogEntries;
	const TArray<FName> Peoples = { Own };
	int32 ExpectedChronicle = 0;
	for (int32 Index = 0; Index < Departed; ++Index)
	{
		ExpectedChronicle += Kept(All[Index], Peoples) ? 1 : 0;
	}

	TestEqual(TEXT("The log keeps the most recent entries only"), State.Log.Num(), UCosmicLogSubsystem::MaxLogEntries);
	TestEqual(TEXT("The chronicle holds what it can show of what left"), State.Chronicle.Num(), ExpectedChronicle);
	TestTrue(TEXT("Something was dropped and something was kept"), ExpectedChronicle > 0 && ExpectedChronicle < Departed);
	TestEqual(TEXT("The newest entry is the last one pushed"), State.Log.Last().Message.ToString(), FString::Printf(TEXT("entry %d"), Pushes - 1));
	TestEqual(TEXT("The oldest entry left is the first one inside the cap"), State.Log[0].Message.ToString(), FString::Printf(TEXT("entry %d"), Departed - Initial.Num()));
	TestEqual(TEXT("Pushed entries carry their source row"), State.Log.Last().SourceRow, Pushed.Last().SourceRow);

	bool bChronicleInOrder = true;
	bool bChronicleRuleHolds = true;
	bool bOtherSeverity1 = false;
	bool bOwnSeverity1 = false;
	bool bFirstContact = false;
	for (int32 Index = 0; Index < State.Chronicle.Num(); ++Index)
	{
		const FCosmicLogEntry& Entry = State.Chronicle[Index];
		bChronicleInOrder &= Index == 0 || State.Chronicle[Index - 1].Tick < Entry.Tick;
		bChronicleRuleHolds &= Kept(Entry, Peoples);
		const bool bSeverity1Civ = Entry.Severity == 1 && Entry.Category == CivTag;
		bOtherSeverity1 |= bSeverity1Civ && Entry.FocusCivID == Other && Entry.SourceRow != CosmicRows::LogFirstContact;
		bOwnSeverity1 |= bSeverity1Civ && Entry.FocusCivID == Own;
		bFirstContact |= bSeverity1Civ && Entry.SourceRow == CosmicRows::LogFirstContact;
	}
	TestTrue(TEXT("The chronicle keeps the order of the log"), bChronicleInOrder);
	TestTrue(TEXT("Nothing it cannot show reached the chronicle"), bChronicleRuleHolds);
	TestFalse(TEXT("Another people's Severity 1 is not in the chronicle"), bOtherSeverity1);
	TestTrue(TEXT("The ruled people's Severity 1 is"), bOwnSeverity1);
	TestTrue(TEXT("and so is the first contact"), bFirstContact);

	// The page reads as if nothing had left: the same chapter as over a log that kept every line.
	{
		FGalaxyState Reference = State;
		Reference.Log = All;
		Reference.Chronicle.Reset();
		const TArray<FCosmicChronicleChapter> Kept40 = CosmicChronicle::BuildCapped(State, 40);
		const TArray<FCosmicChronicleChapter> Whole40 = CosmicChronicle::BuildCapped(Reference, 40);
		bool bSame = Kept40.Num() == Whole40.Num();
		for (int32 Chapter = 0; bSame && Chapter < Kept40.Num(); ++Chapter)
		{
			bSame &= Kept40[Chapter].Candidates == Whole40[Chapter].Candidates && Kept40[Chapter].Lines.Num() == Whole40[Chapter].Lines.Num();
			for (int32 Line = 0; bSame && Line < Kept40[Chapter].Lines.Num(); ++Line)
			{
				bSame &= Kept40[Chapter].Lines[Line].Entry.Message.ToString() == Whole40[Chapter].Lines[Line].Entry.Message.ToString();
			}
		}
		TestTrue(TEXT("The chronicle's page is the one of a log that kept every line"), bSame);
	}

	// The subject of a line is stored with it (section 8.1). Pushed last: the counts above are already checked.
	const FName SubjectSystem = State.Systems[1].SystemID;
	const FName SubjectCiv = State.Civs[1].CivID;
	FFormatNamedArguments SubjectArgs;
	SubjectArgs.Add(TEXT("Number"), FText::AsCultureInvariant(TEXT("subject")));
	Log->Push(CivTag, 0, Template, SubjectArgs, SubjectSystem, TestRow, SubjectCiv);
	TestTrue(TEXT("Push stores the subject system, the subject civ and the source row"),
		State.Log.Last().FocusSystemID == SubjectSystem && State.Log.Last().FocusCivID == SubjectCiv && State.Log.Last().SourceRow == TestRow);

	// Two lives: the peoples of every life, not the last one's alone.
	Sim->NewGalaxy(42, 5, 3);
	{
		FLifeRecord Second;
		Second.LifeIndex = 2;
		Second.BornTick = 1;
		Second.BirthCivID = Other;
		Second.RuledCivIDs.Add(Other);
		Sim->GetMutableState().Calling.Lives.Add(Second);
	}
	for (int32 Index = 0; Index < Pushes; ++Index)
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Number"), FText::AsCultureInvariant(FString::FromInt(Index)));
		Sim->GetMutableState().Tick = Index + 1;
		Log->Push(CivTag, 1, Template, Args, NAME_None, TestRow, Index % 2 == 0 ? Own : Other);
	}
	bool bFirstLife = false;
	bool bSecondLife = false;
	for (const FCosmicLogEntry& Entry : State.Chronicle)
	{
		bFirstLife |= Entry.Severity == 1 && Entry.FocusCivID == Own;
		bSecondLife |= Entry.Severity == 1 && Entry.FocusCivID == Other;
	}
	TestTrue(TEXT("With two lives the first life's people's Severity 1 stays"), bFirstLife);
	TestTrue(TEXT("and the second's"), bSecondLife);
	return true;
}

// The chronicle's cap (MVP item 5, DT_Rules.ChronicleCap; 16 #40 (7)): over it the oldest Severity 1 entries leave, never
// Severity 2, Log.Calling, Log.Claim or Log.Record; the record counts them and keeps the newest tick; 0 = no cap; and it is
// story only - the world hash does not move with it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLogChronicleCapTest, "Cosmic_Ascension.Log.ChronicleCap", CosmicTests::Flags)
bool FCosmicLogChronicleCapTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef Rules = Sim->GetRules();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
	};
	const FText Template = FText::AsCultureInvariant(TEXT("line {Number}"));
	const FName TestRow(TEXT("log_test"));

	// Thirty lines pushed first (ticks 1-30): every fifth one cannot leave - Severity 2, Log.Calling, Log.Claim, Log.Record,
	// Severity 2, Log.Calling - the 24 others are Severity 1 about the ruled people. Then 2,000 ordinary fillers: the thirty
	// leave the log, in order, and nothing else does (an ordinary line never reaches the chronicle).
	TArray<int64> Severity1Ticks;
	auto Run = [&](int32 Cap)
	{
		FCosmicRulesDef Capped = Rules;
		Capped.ChronicleCap = Cap;
		Sim->SetTables(CosmicTests::WithRules(Original, Capped));
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		FGalaxyState& State = Sim->GetMutableState();
		State.Log.Reset();
		State.Chronicle.Reset();
		const FName Own = State.Calling.Lives.IsEmpty() ? NAME_None : State.Calling.Lives[0].BirthCivID;
		static const TCHAR* const Kinds[] = { TEXT("Log.Civ"), TEXT("Log.Calling"), TEXT("Log.Claim"), TEXT("Log.Record") };
		static const uint8 KindSeverity[] = { 2, 1, 1, 0 };
		Severity1Ticks.Reset();
		for (int32 Index = 0; Index < 30 + UCosmicLogSubsystem::MaxLogEntries; ++Index)
		{
			State.Tick = Index + 1;
			FFormatNamedArguments Args;
			Args.Add(TEXT("Number"), FText::AsCultureInvariant(FString::FromInt(Index)));
			if (Index >= 30)
			{
				Log->Push(FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Civ"))), 0, Template, Args, NAME_None, TestRow);
			}
			else if (Index % 5 == 0)
			{
				const int32 Kind = (Index / 5) % 4;
				Log->Push(FGameplayTag::RequestGameplayTag(FName(Kinds[Kind])), KindSeverity[Kind], Template, Args, NAME_None, TestRow, Kind == 0 ? Own : NAME_None);
			}
			else
			{
				Log->Push(FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Civ"))), 1, Template, Args, NAME_None, TestRow, Own);
				Severity1Ticks.Add(State.Tick);
			}
		}
		return Sim->GetState();
	};
	auto Unremovable = [](const FGalaxyState& State)
	{
		int32 Count = 0;
		for (const FCosmicLogEntry& Entry : State.Chronicle)
		{
			Count += CosmicChronicle::MayLetGo(Entry) ? 0 : 1;
		}
		return Count;
	};

	const FGalaxyState Ten = Run(10);
	const uint32 TenWorld = Sim->ComputeWorldHash();
	const uint32 TenFull = Sim->ComputeStateHash();
	TestEqual(TEXT("(24 Severity 1 lines pushed)"), Severity1Ticks.Num(), 24);
	TestEqual(TEXT("Over the cap the chronicle comes down to it"), Ten.Chronicle.Num(), 10);
	TestEqual(TEXT("keeping every line that cannot leave"), Unremovable(Ten), 6);
	TestEqual(TEXT("The record counts what left"), Ten.ChronicleEviction.Count, 20);
	TestEqual(TEXT("and keeps the newest tick of it - the oldest Severity 1 lines went"), Ten.ChronicleEviction.ThroughTick, Severity1Ticks.IsValidIndex(19) ? Severity1Ticks[19] : -1);
	bool bNewestKept = true;
	bool bInOrder = true;
	for (int32 Index = 0; Index < Ten.Chronicle.Num(); ++Index)
	{
		const FCosmicLogEntry& Entry = Ten.Chronicle[Index];
		bNewestKept &= !CosmicChronicle::MayLetGo(Entry) || (Severity1Ticks.IsValidIndex(19) && Entry.Tick > Severity1Ticks[19]);
		bInOrder &= Index == 0 || Ten.Chronicle[Index - 1].Tick < Entry.Tick;
	}
	TestTrue(TEXT("The Severity 1 lines left are the newest"), bNewestKept);
	TestTrue(TEXT("in the order of the log"), bInOrder);

	const FGalaxyState Three = Run(3);
	TestEqual(TEXT("A cap under the lines that cannot leave lets every Severity 1 go"), Three.ChronicleEviction.Count, 24);
	TestEqual(TEXT("and the chronicle stays above it"), Three.Chronicle.Num(), 6);

	const FGalaxyState None = Run(0);
	TestEqual(TEXT("A cap of 0 lets nothing go"), None.ChronicleEviction.Count, 0);
	TestEqual(TEXT("and keeps every line"), None.Chronicle.Num(), 30);
	TestEqual(TEXT("The cap is story only: the world hash with a cap of 10 is that of no cap"), static_cast<int64>(TenWorld), static_cast<int64>(Sim->ComputeWorldHash()));
	TestTrue(TEXT("while the full hash differs"), TenFull != Sim->ComputeStateHash());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLogVariantsLedgerTest, "Cosmic_Ascension.Log.VariantsLedger", CosmicTests::Flags)
bool FCosmicLogVariantsLedgerTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	UCosmicEventSubsystem* Events = Sim->GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	Sim->NewGalaxy(42, 5, 3);
	const FGalaxyState& State = Sim->GetState();
	const FGameplayTag CivTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Civ")));
	const FGameplayTag SignTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Sign")));
	const TArray<FText> Three = { FText::AsCultureInvariant(TEXT("ledger one")), FText::AsCultureInvariant(TEXT("ledger two")), FText::AsCultureInvariant(TEXT("ledger three")) };
	const TArray<FText> One = { FText::AsCultureInvariant(TEXT("ledger alone")) };
	const int32 SimSeed = Sim->GetRng().GetCurrentSeed();

	FCosmicLine Line;
	Line.Category = CivTag;
	Line.Severity = 1;
	Line.FocusSystem = State.Systems[1].SystemID;
	Line.FocusCiv = State.Civs[1].CivID;
	Line.SourceRow = FName(TEXT("evt_test"));

	// One entry and exactly one tell draw per call, also for a single variant; the entry carries the line.
	for (const TArray<FText>* Variants : { &Three, &One })
	{
		FRandomStream Probe(State.TellRngState);
		const int32 EntriesBefore = State.Log.Num();
		const bool bPushed = Log->PushVariants(Line, *Variants, FFormatNamedArguments());
		Probe.RandHelper(Variants->Num());
		TestTrue(TEXT("PushVariants appends one entry and draws once from the tell stream"),
			bPushed && State.Log.Num() == EntriesBefore + 1 && State.TellRngState == Probe.GetCurrentSeed());
		const FCosmicLogEntry& Entry = State.Log.Last();
		TestTrue(TEXT("The entry carries the category, the severity, both subjects and the row of the line"),
			Entry.Category == CivTag && Entry.Severity == 1 && Entry.FocusSystemID == Line.FocusSystem && Entry.FocusCivID == Line.FocusCiv
			&& Entry.SourceRow == Line.SourceRow && Entry.Tick == State.Tick);
	}

	// An empty array: false, no entry, no draw.
	{
		const int32 TellBefore = State.TellRngState;
		const int32 EntriesBefore = State.Log.Num();
		TestFalse(TEXT("PushVariants of no variant is false"), Log->PushVariants(Line, TArray<FText>(), FFormatNamedArguments()));
		TestTrue(TEXT("No variant: no entry and no draw"), State.Log.Num() == EntriesBefore && State.TellRngState == TellBefore);
	}
	TestTrue(TEXT("PushVariants leaves the sim stream alone"), Sim->GetRng().GetCurrentSeed() == SimSeed && State.RngState == SimSeed);

	// PushRow is PushVariants over the row: from the same state both leave the same tell stream and the same sentence.
	{
		const FName RowName = CosmicRows::LogBreakthrough[0];
		const FLogTemplateDef* Row = Sim->GetTables().FindLogTemplate(RowName);
		if (TestNotNull(TEXT("DT_LogTemplates holds the breakthrough row (run Tools/make_data_tables.py)"), Row))
		{
			FFormatNamedArguments Args;
			Args.Add(TEXT("Civ"), FText::AsCultureInvariant(TEXT("X")));
			const FGalaxyState Start = Sim->GetState();
			Log->PushRow(RowName, 0, Args, NAME_None);
			const int32 TellAfterRow = State.TellRngState;
			const FString SentenceOfRow = State.Log.Last().Message.ToString();

			Sim->LoadState(Start);
			FCosmicLine RowLine;
			RowLine.Category = Row->Category;
			RowLine.SourceRow = RowName;
			Log->PushVariants(RowLine, Row->Templates, Args);
			TestTrue(TEXT("PushRow and PushVariants over the same row leave the same tell stream and the same sentence"),
				State.TellRngState == TellAfterRow && State.Log.Last().Message.ToString() == SentenceOfRow
				&& State.Log.Last().SourceRow == RowName && State.Log.Last().Category == Row->Category);
		}
	}

	// The tell draws of a fired event row follow the row: two with a sign row, one without, none without templates.
	{
		const FName SignRow(TEXT("log_test_sign"));
		UDataTable* LogTable = NewObject<UDataTable>(GetTransientPackage());
		LogTable->RowStruct = FLogTemplateDef::StaticStruct();
		FLogTemplateDef SignDef;
		SignDef.Category = SignTag;
		SignDef.Templates = Three;
		LogTable->AddRow(SignRow, SignDef);

		const FName WithSign(TEXT("evt_test_sign"));
		const FName Plain(TEXT("evt_test_plain"));
		const FName Mute(TEXT("evt_test_mute"));
		UDataTable* EventTable = NewObject<UDataTable>(GetTransientPackage());
		EventTable->RowStruct = FCosmicEventDef::StaticStruct();
		FCosmicEventDef Def;
		Def.Category = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Civ.Society")));
		Def.LogCategory = CivTag;
		Def.Trigger = ECosmicEventTrigger::Named;
		Def.BaseWeight = 0.f;
		Def.CooldownTicks = 0;
		Def.Severity = 2;
		EventTable->AddRow(Mute, Def);
		Def.LogTemplates = Three;
		EventTable->AddRow(Plain, Def);
		Def.MinTierToReveal = 3;
		Def.SignRow = SignRow;
		EventTable->AddRow(WithSign, Def);
		// A sign row needs somebody below the window of the row: tier 2 is the lowest MinTierToReveal that has one.
		const FName SignAtTwo(TEXT("evt_test_sign_two"));
		const FName SignAtOne(TEXT("evt_test_sign_one"));
		const FName SignAtZero(TEXT("evt_test_sign_zero"));
		const FName SignButMute(TEXT("evt_test_sign_mute"));
		Def.MinTierToReveal = 2;
		EventTable->AddRow(SignAtTwo, Def);
		Def.MinTierToReveal = 1;
		EventTable->AddRow(SignAtOne, Def);
		Def.MinTierToReveal = 0;
		EventTable->AddRow(SignAtZero, Def);
		Def.MinTierToReveal = 3;
		Def.LogTemplates.Reset();
		EventTable->AddRow(SignButMute, Def);

		FCosmicDataTables Tables = Original;
		Tables.LogTemplates = LogTable;
		Tables.Events = EventTable;
		Sim->SetTables(Tables);

		auto TellStepsOf = [&](FName RowName, int32 ExpectedLines)
		{
			FRandomStream Probe(State.TellRngState);
			const int32 EntriesBefore = State.Log.Num();
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			Ctx.CivIndex = 1;
			const bool bFired = Events->FireRow(RowName, Ctx);
			for (int32 Step = 0; Step < ExpectedLines; ++Step)
			{
				Probe.RandHelper(Three.Num());
			}
			return bFired && State.TellRngState == Probe.GetCurrentSeed() && State.Log.Num() == EntriesBefore + ExpectedLines;
		};
		TestTrue(TEXT("A row without templates tells nothing and draws nothing from the tell stream"), TellStepsOf(Mute, 0));
		TestTrue(TEXT("A row with templates tells one line with one tell draw"), TellStepsOf(Plain, 1));
		TestTrue(TEXT("A row with a sign row tells two lines with two tell draws"), TellStepsOf(WithSign, 2));
		// The sign first, the row's own line after it (B4 - UCosmicLogSubsystem::PushWithSign).
		const FCosmicLogEntry& Sign = State.Log[State.Log.Num() - 2];
		const FCosmicLogEntry& Main = State.Log.Last();
		TestTrue(TEXT("The main line carries the row's log category and severity"), Main.Category == CivTag && Main.Severity == 2 && Main.SourceRow == WithSign);
		TestTrue(TEXT("The sign line is a Log.Sign entry of severity 1 at most, with the event row as its source"),
			Sign.Category == SignTag && Sign.Severity == 1 && Sign.SourceRow == WithSign && Sign.FocusCivID == State.Civs[1].CivID);

		TestTrue(TEXT("A sign row under a row revealed from tier 2 is told: two lines"), TellStepsOf(SignAtTwo, 2));
		TestTrue(TEXT("A sign row under a row revealed from tier 1 has nobody below it: one line"), TellStepsOf(SignAtOne, 1));
		TestTrue(TEXT("A sign row under a row revealed to everyone has nobody below it: one line"), TellStepsOf(SignAtZero, 1));
		TestTrue(TEXT("A row without templates tells nothing, its sign row included"), TellStepsOf(SignButMute, 0));
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

// The sign helper (B4 header section 2.6): the lower speaker's sentence first, in the window below the line's, then the line;
// no variants, no draw at all; a sign row the table lacks is an error line and the line alone. Judged by the scope of
// telling (tell-all off), with the ruled people's rung as the viewer's.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLogSignHelperTest, "Cosmic_Ascension.Log.SignHelper", CosmicTests::Flags)
bool FCosmicLogSignHelperTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	const FCosmicDataTables Original = Sim->GetTables();
	const bool bWasTellAll = Log->IsTellAll();
	ON_SCOPE_EXIT { Log->SetTellAll(bWasTellAll); Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	const FGameplayTag CivTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Civ")));
	const FGameplayTag SignTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Sign")));
	const TArray<FText> Three = { FText::AsCultureInvariant(TEXT("helper one")), FText::AsCultureInvariant(TEXT("helper two")), FText::AsCultureInvariant(TEXT("helper three")) };
	const FName SignRow(TEXT("log_test_sign"));
	UDataTable* LogTable = NewObject<UDataTable>(GetTransientPackage());
	LogTable->RowStruct = FLogTemplateDef::StaticStruct();
	FLogTemplateDef SignDef;
	SignDef.Category = SignTag;
	SignDef.Templates = { FText::AsCultureInvariant(TEXT("sign one")), FText::AsCultureInvariant(TEXT("sign two")), FText::AsCultureInvariant(TEXT("sign three")) };
	LogTable->AddRow(SignRow, SignDef);
	FCosmicDataTables Tables = Original;
	Tables.LogTemplates = LogTable;
	Sim->SetTables(Tables);
	Sim->NewGalaxy(42, 5, 3);
	Log->SetTellAll(false);
	FGalaxyState& State = Sim->GetMutableState();
	TestTrue(TEXT("(the vessel rules civ 0)"), State.Player.Role == EPlayerRole::Ruler && State.Civs[0].bIsPlayerRuled);

	// A Severity 2 line about the ruled people, told from the third rung up.
	FCosmicLine Line;
	Line.Category = CivTag;
	Line.Severity = 2;
	Line.FocusCiv = State.Civs[0].CivID;
	Line.SourceRow = FName(TEXT("tr_test_voice"));
	Line.MinViewerTier = 3;
	// Pushes with the sign row of the case and gives the number of tell draws and of entries it made.
	auto Push = [&](FName Sign, TConstArrayView<FText> Variants, int32 Draws, int32 Entries)
	{
		FRandomStream Probe(State.TellRngState);
		const int32 Before = State.Log.Num();
		Log->PushWithSign(Line, Variants, Sign, FFormatNamedArguments());
		for (int32 Draw = 0; Draw < Draws; ++Draw)
		{
			Probe.RandHelper(3);
		}
		return State.TellRngState == Probe.GetCurrentSeed() && State.Log.Num() == Before + Entries;
	};

	// The first rung: the sign is told (its window [0, 2]), then the line, kept untold (out of its window, Severity 2).
	TestTrue(TEXT("Two tell draws and two entries at the first rung"), Push(SignRow, Three, 2, 2));
	{
		const FCosmicLogEntry& Sign = State.Log[State.Log.Num() - 2];
		const FCosmicLogEntry& Main = State.Log.Last();
		TestTrue(TEXT("The sign comes first: Log.Sign, Severity 1, the line's focus and row, told"),
			Sign.Category == SignTag && Sign.Severity == 1 && Sign.FocusCivID == Line.FocusCiv && Sign.SourceRow == Line.SourceRow && !Sign.bUntold);
		TestTrue(TEXT("the line after it, kept untold below its window"), Main.Category == CivTag && Main.Severity == 2 && Main.bUntold);
	}
	// The second rung, the top of the sign's window: the sign told, the line kept untold.
	State.Civs[0].Tier = EAscensionTier::Tier2_Stellar;
	TestTrue(TEXT("Two tell draws and two entries at the second rung"), Push(SignRow, Three, 2, 2));
	TestTrue(TEXT("the sign told, the line untold"), !State.Log[State.Log.Num() - 2].bUntold && State.Log[State.Log.Num() - 2].Category == SignTag && State.Log.Last().bUntold);
	// The third rung: the line is told; the sign, out of its window and of Severity 1, is not stored - its draw stays.
	State.Civs[0].Tier = EAscensionTier::Tier3_Causal;
	TestTrue(TEXT("Two tell draws and one entry at the third rung"), Push(SignRow, Three, 2, 1));
	TestTrue(TEXT("and that entry is the line, told"), State.Log.Last().Category == CivTag && !State.Log.Last().bUntold);
	// No variants: nothing told, no draw - the sign neither.
	TestFalse(TEXT("No variants is false"), Log->PushWithSign(Line, TArray<FText>(), SignRow, FFormatNamedArguments()));
	TestTrue(TEXT("and tells nothing, the sign included"), Push(SignRow, TArray<FText>(), 0, 0));
	// No sign row: the line alone; a sign row the table lacks: an error line and the line alone.
	TestTrue(TEXT("Without a sign row, the line alone"), Push(NAME_None, Three, 1, 1));
	AddExpectedError(TEXT("names the sign row log_test_nobody"), EAutomationExpectedErrorFlags::Contains, 1);
	TestTrue(TEXT("A sign row the table lacks: the line alone"), Push(FName(TEXT("log_test_nobody")), Three, 1, 1));
	// A window that starts at the first rung has nobody below it: no sign.
	Line.MinViewerTier = 1;
	TestTrue(TEXT("A line told from the first rung up has no sign"), Push(SignRow, Three, 1, 1));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLogCapAndChronicleTest, "Cosmic_Ascension.Log.CapAndChronicle", CosmicTests::Flags)
bool FCosmicLogCapAndChronicleTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();

	Sim->NewGalaxy(42, 5, 3);
	const FGalaxyState& State = Sim->GetState();

	const FGameplayTag CivTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Civ")));
	const FGameplayTag ClaimTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Claim")));
	const FGameplayTag RecordTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Record")));
	const FText Template = FText::AsCultureInvariant(TEXT("entry {Number}"));
	const FName TestRow(TEXT("log_test"));

	// What every entry is expected to do when it leaves the log: true = kept in the chronicle.
	const int32 InitialCount = State.Log.Num();
	TArray<bool> bKept;
	for (const FCosmicLogEntry& Entry : State.Log)
	{
		bKept.Add(Entry.Severity >= 1 || Entry.Category == ClaimTag || Entry.Category == RecordTag);
	}

	const int32 Pushes = UCosmicLogSubsystem::MaxLogEntries + 100;
	for (int32 Index = 0; Index < Pushes; ++Index)
	{
		// Mostly ordinary lines, some notable and mythic ones, and now and then an ordinary claim or record.
		const uint8 Severity = static_cast<uint8>(Index % 3);
		const FGameplayTag Category = (Index % 50 == 0) ? ClaimTag : (Index % 50 == 25) ? RecordTag : CivTag;
		FFormatNamedArguments Args;
		Args.Add(TEXT("Number"), FText::AsCultureInvariant(FString::FromInt(Index)));
		// A stamp of its own for every entry (the first line of the galaxy carries tick 0), for the order check below.
		Sim->GetMutableState().Tick = Index + 1;
		Log->Push(Category, Severity, Template, Args, NAME_None, TestRow);
		bKept.Add(Severity >= 1 || Category != CivTag);
	}

	const int32 Total = bKept.Num();
	const int32 Departed = Total - UCosmicLogSubsystem::MaxLogEntries;
	int32 ExpectedChronicle = 0;
	for (int32 Index = 0; Index < Departed; ++Index)
	{
		ExpectedChronicle += bKept[Index] ? 1 : 0;
	}

	TestEqual(TEXT("The log keeps the most recent entries only"), State.Log.Num(), UCosmicLogSubsystem::MaxLogEntries);
	TestEqual(TEXT("The chronicle holds what had to be kept"), State.Chronicle.Num(), ExpectedChronicle);
	TestTrue(TEXT("Something was dropped and something was kept"), ExpectedChronicle > 0 && ExpectedChronicle < Departed);
	TestEqual(TEXT("The newest entry is the last one pushed"), State.Log.Last().Message.ToString(), FString::Printf(TEXT("entry %d"), Pushes - 1));
	TestEqual(TEXT("The oldest entry left is the first one inside the cap"), State.Log[0].Message.ToString(), FString::Printf(TEXT("entry %d"), Departed - InitialCount));
	TestEqual(TEXT("Pushed entries carry their source row"), State.Log.Last().SourceRow, TestRow);

	bool bChronicleInOrder = true;
	for (int32 Index = 1; Index < State.Chronicle.Num(); ++Index)
	{
		bChronicleInOrder &= State.Chronicle[Index - 1].Tick < State.Chronicle[Index].Tick;
	}
	TestTrue(TEXT("The chronicle keeps the order of the log"), bChronicleInOrder);

	bool bChronicleRuleHolds = true;
	for (const FCosmicLogEntry& Entry : State.Chronicle)
	{
		bChronicleRuleHolds &= Entry.Severity >= 1 || Entry.Category == ClaimTag || Entry.Category == RecordTag;
	}
	TestTrue(TEXT("Nothing ordinary reached the chronicle"), bChronicleRuleHolds);

	// The subject of a line is stored with it (section 8.1). Pushed last: the counts above are already checked.
	const FName SubjectSystem = State.Systems[1].SystemID;
	const FName SubjectCiv = State.Civs[1].CivID;
	FFormatNamedArguments SubjectArgs;
	SubjectArgs.Add(TEXT("Number"), FText::AsCultureInvariant(TEXT("subject")));
	Log->Push(CivTag, 0, Template, SubjectArgs, SubjectSystem, TestRow, SubjectCiv);
	TestTrue(TEXT("Push stores the subject system, the subject civ and the source row"),
		State.Log.Last().FocusSystemID == SubjectSystem && State.Log.Last().FocusCivID == SubjectCiv && State.Log.Last().SourceRow == TestRow);
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
		const FCosmicLogEntry& Sign = State.Log.Last();
		const FCosmicLogEntry& Main = State.Log[State.Log.Num() - 2];
		TestTrue(TEXT("The main line carries the row's log category and severity"), Main.Category == CivTag && Main.Severity == 2 && Main.SourceRow == WithSign);
		TestTrue(TEXT("The sign line is a Log.Sign entry of severity 1 at most, with the event row as its source"),
			Sign.Category == SignTag && Sign.Severity == 1 && Sign.SourceRow == WithSign && Sign.FocusCivID == State.Civs[1].CivID);
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

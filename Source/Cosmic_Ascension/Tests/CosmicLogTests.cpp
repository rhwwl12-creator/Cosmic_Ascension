// Cosmic Ascension - automation tests of the log subsystem. Design: guideline.md section 7.5, 8.4, 9.3.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicRowNames.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Log/CosmicLogSubsystem.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"

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

	FFormatNamedArguments Args;
	Args.Add(TEXT("Civ"), FText::AsCultureInvariant(TEXT("X")));

	// The same row over and over: one entry and exactly one draw per call, and never the same sentence twice in a row.
	bool bOneEntryPerCall = true;
	bool bOneDrawPerCall = true;
	bool bStateSeedSynced = true;
	for (int32 Call = 0; Call < 300; ++Call)
	{
		FRandomStream Probe(Sim->GetRng().GetCurrentSeed());
		const int32 EntriesBefore = State.Log.Num();

		const bool bPushed = Log->PushRow(RowName, 0, Args, NAME_None);

		Probe.RandRange(0, Row->Templates.Num() - 1);
		bOneEntryPerCall &= bPushed && State.Log.Num() == EntriesBefore + 1;
		bOneDrawPerCall &= Probe.GetCurrentSeed() == Sim->GetRng().GetCurrentSeed();
		bStateSeedSynced &= State.RngState == Sim->GetRng().GetCurrentSeed();
	}
	TestTrue(TEXT("PushRow appends exactly one entry per call"), bOneEntryPerCall);
	TestTrue(TEXT("PushRow draws exactly once per call"), bOneDrawPerCall);
	TestTrue(TEXT("PushRow keeps State.RngState in sync"), bStateSeedSynced);
	TestTrue(TEXT("No sentence twice in a row after 300 pushes of one row"), CosmicLogTests::HasNoConsecutiveRepeat(State.Log));
	TestEqual(TEXT("Entries carry the row as their source"), State.Log.Last().SourceRow, RowName);
	TestTrue(TEXT("Entries carry the row's category"), State.Log.Last().Category == Row->Category);

	// A missing row: nothing is pushed and nothing is drawn.
	const int32 SeedBefore = Sim->GetRng().GetCurrentSeed();
	const int32 EntriesBefore = State.Log.Num();
	TestFalse(TEXT("A missing row is not pushed"), Log->PushRow(FName(TEXT("log_no_such_row")), 0, Args, NAME_None));
	TestTrue(TEXT("A missing row draws nothing and adds nothing"), Sim->GetRng().GetCurrentSeed() == SeedBefore && State.Log.Num() == EntriesBefore);

	// A whole run: the log the simulation writes by itself.
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
		bChronicleInOrder &= State.Chronicle[Index - 1].Tick <= State.Chronicle[Index].Tick;
	}
	TestTrue(TEXT("The chronicle keeps the order of the log"), bChronicleInOrder);

	bool bChronicleRuleHolds = true;
	for (const FCosmicLogEntry& Entry : State.Chronicle)
	{
		bChronicleRuleHolds &= Entry.Severity >= 1 || Entry.Category == ClaimTag || Entry.Category == RecordTag;
	}
	TestTrue(TEXT("Nothing ordinary reached the chronicle"), bChronicleRuleHolds);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

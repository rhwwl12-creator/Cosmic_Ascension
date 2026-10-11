// Cosmic Ascension - automation tests of the simulation core. Design: guideline.md section 9.2, 9.3.
// Run with the automation command in CLAUDE.md, twice in a row after a build (see Sim.SaveRestart).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/GalaxyState.h"
#include "Data/CosmicRowNames.h"
#include "GameplayTagContainer.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/UnrealType.h"

#include <limits>

namespace CosmicSimTests
{
	uint32 HashAfter(UGalaxySimulationSubsystem& Sim, int32 Seed, int64 Ticks)
	{
		Sim.NewGalaxy(Seed, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		Sim.RunHeadless(Ticks);
		return Sim.ComputeStateHash();
	}

	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// Counts non-finite floating point values anywhere under a struct, by reflection.
	int32 CountNonFinite(const FProperty* Property, const void* Value);

	int32 CountNonFiniteInStruct(const UStruct* Struct, const void* Data)
	{
		int32 Count = 0;
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			for (int32 Index = 0; Index < It->ArrayDim; ++Index)
			{
				Count += CountNonFinite(*It, It->ContainerPtrToValuePtr<void>(Data, Index));
			}
		}
		return Count;
	}

	int32 CountNonFinite(const FProperty* Property, const void* Value)
	{
		if (const FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
		{
			return (NumericProperty->IsFloatingPoint() && !FMath::IsFinite(NumericProperty->GetFloatingPointPropertyValue(Value))) ? 1 : 0;
		}
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			return CountNonFiniteInStruct(StructProperty->Struct, Value);
		}
		if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			int32 Count = 0;
			FScriptArrayHelper Helper(ArrayProperty, Value);
			for (int32 Index = 0; Index < Helper.Num(); ++Index)
			{
				Count += CountNonFinite(ArrayProperty->Inner, Helper.GetRawPtr(Index));
			}
			return Count;
		}
		if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
		{
			int32 Count = 0;
			FScriptMapHelper Helper(MapProperty, Value);
			for (int32 Index = 0; Index < Helper.GetMaxIndex(); ++Index)
			{
				if (Helper.IsValidIndex(Index))
				{
					Count += CountNonFinite(MapProperty->ValueProp, Helper.GetValuePtr(Index));
				}
			}
			return Count;
		}
		return 0;
	}

	// Binary that holds the simulation code. Its timestamp tells a rebuilt game from a restarted one.
	FString SimBinaryPath()
	{
#if IS_MONOLITHIC
		return FPlatformProcess::ExecutablePath();
#else
		return FModuleManager::Get().GetModuleFilename(FName(TEXT("Cosmic_Ascension")));
#endif
	}

	// Puts one value into every container of the state that a plain run leaves empty, so that the save and the hash see
	// tag containers, maps, notables, transcendents and log entries.
	void FillEmptyContainers(FGalaxyState& State)
	{
		const FName TestRow(TEXT("evt_test"));

		State.Flags.AddTag(Tag(TEXT("Flag.Grace.Spent")));
		State.Civs[0].Traits.AddTag(Tag(TEXT("Trait.Faith.Seeking")));
		State.Player.StatusTags.AddTag(Tag(TEXT("Status.Abdicated")));
		State.Calling.Powers.AddTag(Tag(TEXT("Way.Received")));
		State.EventCooldownUntilTick.Add(TestRow, 5);
		State.Civs[0].Relations.Add(State.Civs[1].CivID, 0.5f);

		FNotableRecord& Notable = State.Civs[0].Notables.AddDefaulted_GetRef();
		Notable.Role = Tag(TEXT("Notable.Rebel"));
		Notable.NameIndex = 3;
		Notable.FirstTick = 10;
		Notable.SourceRow = TestRow;

		FTranscendentState& Transcendent = State.Transcendents.AddDefaulted_GetRef();
		Transcendent.TranscendentID = FName(TEXT("tr_test"));
		Transcendent.Doctrine = Tag(TEXT("Doctrine.Silence"));
		Transcendent.InfluencedSystems.Add(State.Systems.Last().SystemID);

		FCosmicLogEntry Entry;
		Entry.Tick = 10;
		Entry.Category = Tag(TEXT("Log.Civ"));
		Entry.Message = FText::AsCultureInvariant(TEXT("x"));
		Entry.SourceRow = TestRow;
		State.Log.Add(Entry);
		State.Chronicle.Add(Entry);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimDeterminismTest, "Cosmic_Ascension.Sim.Determinism", CosmicTests::Flags)
bool FCosmicSimDeterminismTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	const uint32 First = CosmicSimTests::HashAfter(*Sim, 42, 3000);
	const uint32 Second = CosmicSimTests::HashAfter(*Sim, 42, 3000);
	TestEqual(TEXT("Seed 42, 3,000 ticks, twice: same hash"), static_cast<int64>(Second), static_cast<int64>(First));
	TestEqual(TEXT("Tick counter after 3,000 ticks"), Sim->GetState().Tick, static_cast<int64>(3000));

	// The hash must react to each kind of field; GalaxyID and FText must stay out of it.
	FGalaxyState& State = Sim->GetMutableState();
	const uint32 Base = Sim->ComputeStateHash();

	float& Matter = State.Civs.Last().Stockpile.Matter;
	Matter += 1.f;
	TestTrue(TEXT("Hash sees a civ stockpile"), Sim->ComputeStateHash() != Base);
	Matter -= 1.f;

	float& Society = State.Civs[0].Research.FindChecked(FName(TEXT("Society")));
	const float OldSociety = Society;
	Society = 0.123f;
	TestTrue(TEXT("Hash sees a map value"), Sim->ComputeStateHash() != Base);
	Society = OldSociety;

	++State.Calling.Lives[0].ReignTicks[0];
	TestTrue(TEXT("Hash sees a life record array"), Sim->ComputeStateHash() != Base);
	--State.Calling.Lives[0].ReignTicks[0];

	FName& Owner = State.Systems.Last().OwnerCivID;
	const FName OldOwner = Owner;
	Owner = FName(TEXT("civ_xx"));
	TestTrue(TEXT("Hash sees a system name field"), Sim->ComputeStateHash() != Base);
	Owner = OldOwner;

	TestEqual(TEXT("Hash is back after undoing the changes"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Base));

	State.GalaxyID = FGuid::NewGuid();
	State.Civs[0].CivName = FText::AsCultureInvariant(TEXT("x"));
	TestEqual(TEXT("GalaxyID and FText stay out of the hash"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Base));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimNoNaNTest, "Cosmic_Ascension.Sim.NoNaN", CosmicTests::Flags)
bool FCosmicSimNoNaNTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	Sim->NewGalaxy(7, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(10000);
	TestEqual(TEXT("Non-finite floats in the state after 10,000 ticks"), CosmicSimTests::CountNonFiniteInStruct(FGalaxyState::StaticStruct(), &Sim->GetState()), 0);

	// The walker itself: it has to find a NaN planted deep in the state.
	FGalaxyState Poisoned = Sim->GetState();
	Poisoned.Civs.Last().Stockpile.Energy = std::numeric_limits<float>::quiet_NaN();
	TestEqual(TEXT("The walker sees a planted NaN"), CosmicSimTests::CountNonFiniteInStruct(FGalaxyState::StaticStruct(), &Poisoned), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimFirstLifeTest, "Cosmic_Ascension.Sim.FirstLife", CosmicTests::Flags)
bool FCosmicSimFirstLifeTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	// The Phase 1 prototype size: five systems.
	Sim->NewGalaxy(42, 5, 3);
	const FGalaxyState& State = Sim->GetState();
	const FName Sol(TEXT("sol"));

	const FCivArchetypeDef* Archetype = Sim->GetTables().FindCiv(Sim->GetRules().PlayerArchetypeRow);
	if (!TestNotNull(TEXT("DT_Civs holds the player archetype row (run Tools/make_data_tables.py)"), Archetype))
	{
		return false;
	}
	if (!TestEqual(TEXT("Systems"), State.Systems.Num(), 5) || !TestEqual(TEXT("Civs"), State.Civs.Num(), 3) || !TestEqual(TEXT("Lives"), State.Calling.Lives.Num(), 1))
	{
		return false;
	}

	const FStarSystemState& Home = State.Systems[0];
	const FCivilizationState& Civ = State.Civs[0];
	TestEqual(TEXT("Home system ID"), Home.SystemID, Sol);
	TestEqual(TEXT("Home system owner"), Home.OwnerCivID, Civ.CivID);
	TestEqual(TEXT("Home richness is ordinary"), Home.ResourceRichness, 1.f);
	TestTrue(TEXT("Home star class comes from the archetype"), Home.StarClass == Archetype->HomeStarClass);
	TestFalse(TEXT("Home system has a display name"), Home.DisplayName.IsEmpty());

	TestEqual(TEXT("Player civ archetype"), Civ.ArchetypeRow, Sim->GetRules().PlayerArchetypeRow);
	TestEqual(TEXT("Player civ home"), Civ.HomeSystemID, Sol);
	TestEqual(TEXT("Player civ owns one system"), Civ.OwnedSystems.Num(), 1);
	TestTrue(TEXT("Player civ is player ruled"), Civ.bIsPlayerRuled);
	TestFalse(TEXT("Player civ has a name"), Civ.CivName.IsEmpty());
	TestTrue(TEXT("Player civ traits come from the archetype"), Civ.Traits == Archetype->Traits);
	TestEqual(TEXT("Player civ openness comes from the archetype"), Civ.Ethics.Openness, Archetype->EthicsPreset.Openness);
	for (int32 Index = 1; Index < State.Civs.Num(); ++Index)
	{
		TestFalse(TEXT("Only the player civ is player ruled"), State.Civs[Index].bIsPlayerRuled);
		TestTrue(TEXT("AI civ has an AI profile"), State.Civs[Index].AIProfile.IsValid());
	}

	TestTrue(TEXT("Vessel is a ruler"), State.Player.Role == EPlayerRole::Ruler);
	TestEqual(TEXT("Vessel rules the player civ"), State.Player.RuledCivID, Civ.CivID);
	TestEqual(TEXT("Vessel is at home"), State.Player.CurrentSystemID, Sol);
	TestEqual(TEXT("First life index"), State.Calling.LifeIndex, 1);
	TestTrue(TEXT("Calling is embodied"), State.Calling.LifeState == ELifeState::Embodied);

	const FLifeRecord& Life = State.Calling.Lives[0];
	TestTrue(TEXT("Succession kind"), Life.SuccessionKind == CosmicSimTests::Tag(TEXT("Succession.Installed")));
	TestEqual(TEXT("Birth system"), Life.BirthSystemID, Sol);
	TestEqual(TEXT("Birth civ"), Life.BirthCivID, Civ.CivID);
	TestEqual(TEXT("Birth archetype row"), Life.BirthArchetypeRow, Civ.ArchetypeRow);
	TestTrue(TEXT("Reign arrays hold one open reign"),
		Life.RuledCivIDs.Num() == 1 && Life.ReignEndReasons.Num() == 1 && Life.ReignTicks.Num() == 1
		&& Life.RuledCivIDs[0] == Civ.CivID && !Life.ReignEndReasons[0].IsValid() && Life.ReignTicks[0] == 0);

	// The first line of the run calls the home system by name.
	if (TestTrue(TEXT("The log holds the first line"), State.Log.Num() >= 1))
	{
		const FCosmicLogEntry& First = State.Log[0];
		const FString Message = First.Message.ToString();
		TestEqual(TEXT("First line row"), First.SourceRow, CosmicRows::LogFirstLife);
		TestTrue(TEXT("First line category"), First.Category == CosmicSimTests::Tag(TEXT("Log.Calling")));
		TestEqual(TEXT("First line severity"), static_cast<int32>(First.Severity), 2);
		TestEqual(TEXT("First line tick"), First.Tick, static_cast<int64>(0));
		TestEqual(TEXT("First line focus"), First.FocusSystemID, Sol);
		TestTrue(TEXT("First line starts with the home system name"), Message.StartsWith(Home.DisplayName.ToString(), ESearchCase::CaseSensitive));
		TestFalse(TEXT("First line has no unfilled slot"), Message.Contains(TEXT("{")));
	}

	Sim->RunHeadless(100);
	TestEqual(TEXT("The open reign counts every tick"), Sim->GetState().Calling.Lives[0].ReignTicks[0], static_cast<int64>(100));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimPopulationBandTest, "Cosmic_Ascension.Sim.PopulationBand", CosmicTests::Flags)
bool FCosmicSimPopulationBandTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	Sim->NewGalaxy(42, 5, 3);
	const TArray<float>& Thresholds = Sim->GetRules().PopulationBandThresholds;
	if (!TestEqual(TEXT("PopulationBandThresholds has three entries"), Thresholds.Num(), 3))
	{
		return false;
	}

	FCivilizationState& Civ = Sim->GetMutableState().Civs[0];
	auto Band = [Sim, &Civ](EAscensionTier Tier, float Population)
	{
		Civ.Tier = Tier;
		Civ.Population = Population;
		return static_cast<int32>(Sim->GetPopulationBand(Civ.CivID));
	};

	TestEqual(TEXT("Tier 1 baseline"), Band(EAscensionTier::Tier1_Planetary, 1.f), static_cast<int32>(EPopulationBand::Billions));
	TestEqual(TEXT("Tier 2 baseline"), Band(EAscensionTier::Tier2_Stellar, 1.f), static_cast<int32>(EPopulationBand::HundredBillions));
	TestEqual(TEXT("Tier 3 baseline"), Band(EAscensionTier::Tier3_Causal, 1.f), static_cast<int32>(EPopulationBand::Trillions));
	TestEqual(TEXT("Tier 5 baseline"), Band(EAscensionTier::Tier5_EternalOrder, 1.f), static_cast<int32>(EPopulationBand::Countless));
	TestEqual(TEXT("Below the middle threshold: one band down"), Band(EAscensionTier::Tier1_Planetary, (Thresholds[0] + Thresholds[1]) * 0.5f), static_cast<int32>(EPopulationBand::Millions));
	TestEqual(TEXT("Below the lowest threshold: few"), Band(EAscensionTier::Tier3_Causal, Thresholds[0] * 0.5f), static_cast<int32>(EPopulationBand::Few));
	TestEqual(TEXT("Above the highest threshold: one band up"), Band(EAscensionTier::Tier1_Planetary, Thresholds[2] + 1.f), static_cast<int32>(EPopulationBand::HundredBillions));
	TestEqual(TEXT("One band up is capped at countless"), Band(EAscensionTier::Tier4_Transcendent, Thresholds[2] + 1.f), static_cast<int32>(EPopulationBand::Countless));
	TestEqual(TEXT("Unknown civ"), static_cast<int32>(Sim->GetPopulationBand(FName(TEXT("civ_none")))), static_cast<int32>(EPopulationBand::Few));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimFrameStepTest, "Cosmic_Ascension.Sim.FrameStep", CosmicTests::Flags)
bool FCosmicSimFrameStepTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	Sim->NewGalaxy(42, 5, 3);
	const FGalaxyState& State = Sim->GetState();

	Sim->Tick(5.f);
	TestEqual(TEXT("Paused: no tick"), State.Tick, static_cast<int64>(0));

	Sim->SetTimeScale(10.f);
	Sim->Tick(0.35f);
	TestEqual(TEXT("10x for 0.35 s: three ticks"), State.Tick, static_cast<int64>(3));

	Sim->SetTimeScale(50.f);
	TestEqual(TEXT("Time scale is capped at 10"), Sim->GetTimeScale(), 10.f);
	Sim->Tick(1.f);
	TestEqual(TEXT("At most ten ticks per frame"), State.Tick, static_cast<int64>(13));

	Sim->Tick(100.f);
	TestEqual(TEXT("A long frame still runs ten ticks"), State.Tick, static_cast<int64>(23));
	Sim->Tick(0.f);
	TestEqual(TEXT("The backlog carried over is ten ticks at most"), State.Tick, static_cast<int64>(33));
	Sim->Tick(0.f);
	TestEqual(TEXT("Nothing is left after the carried ticks"), State.Tick, static_cast<int64>(33));

	Sim->SetTimeScale(0.f);
	Sim->Tick(5.f);
	TestEqual(TEXT("Paused again: no tick"), State.Tick, static_cast<int64>(33));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveRoundTripTest, "Cosmic_Ascension.Sim.SaveRoundTrip", CosmicTests::Flags)
bool FCosmicSimSaveRoundTripTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicSaveSubsystem* Save = Sim->GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>();

	// The refusals below each log one LogCosmic warning.
	AddExpectedMessage(TEXT("LoadGalaxy: slot .* is (truncated or holds another galaxy|missing or unreadable)"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 2);
	AddExpectedMessage(TEXT("LoadGalaxy: slot .* has save version"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);
	// The engine reports the slot that this test cuts in half: one or more errors, and a warning depending on where the cut falls.
	AddExpectedError(TEXT("Failed loading tagged"), EAutomationExpectedErrorFlags::Contains, 0);
	AddExpectedMessage(TEXT("SerializeFromMismatchedTag failed"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

	// The budget size, so that names with a number part (sys_100, civ_10) go through the save and the hash.
	Sim->NewGalaxy(42, CosmicTests::BudgetSystems, CosmicTests::BudgetCivs);
	Sim->RunHeadless(1000);
	CosmicSimTests::FillEmptyContainers(Sim->GetMutableState());
	// TickResearch re-adds the field: into the freed slot of the running map, at the end of the loaded one (section 9.2).
	Sim->GetMutableState().Civs[0].Research.Remove(FName(TEXT("Physics")));
	// A draw outside a tick: the save itself has to capture the live stream position.
	Sim->GetRng().GetUnsignedInt();

	const FGuid GalaxyID = Sim->GetState().GalaxyID;
	const uint32 HashAtSave = Sim->ComputeStateHash();
	if (!TestTrue(TEXT("Save succeeds"), Save->SaveCurrentGalaxy()))
	{
		return false;
	}

	// The uninterrupted run: 500 more ticks, then four draws from the stream.
	Sim->RunHeadless(500);
	const uint32 HashContinued = Sim->ComputeStateHash();
	uint32 DrawsContinued[4];
	for (uint32& Draw : DrawsContinued)
	{
		Draw = Sim->GetRng().GetUnsignedInt();
	}

	// The same from the save.
	if (TestTrue(TEXT("Load succeeds"), Save->LoadGalaxy(GalaxyID)))
	{
		TestEqual(TEXT("Hash right after load equals hash at save"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(HashAtSave));
		TestEqual(TEXT("Tick after load"), Sim->GetState().Tick, static_cast<int64>(1000));
		TestTrue(TEXT("GalaxyID survives the round trip"), Sim->GetState().GalaxyID == GalaxyID);

		Sim->RunHeadless(500);
		TestEqual(TEXT("Hash after load + 500 ticks equals the uninterrupted run"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(HashContinued));
		for (const uint32 Expected : DrawsContinued)
		{
			TestEqual(TEXT("Random stream continues identically after load"), static_cast<int64>(Sim->GetRng().GetUnsignedInt()), static_cast<int64>(Expected));
		}
	}

	// Refused loads leave the running galaxy untouched.
	const uint32 HashBeforeRefusals = Sim->ComputeStateHash();
	const FString SlotName = UCosmicSaveSubsystem::MakeSlotName(GalaxyID);

	// A slot cut in half, as a write interrupted by a crash would leave it.
	TArray<uint8> Bytes;
	if (TestTrue(TEXT("Slot bytes can be read"), UGameplayStatics::LoadDataFromSlot(Bytes, SlotName, 0)))
	{
		Bytes.SetNum(Bytes.Num() / 2);
		TestTrue(TEXT("Truncated slot written"), UGameplayStatics::SaveDataToSlot(Bytes, SlotName, 0));
		TestFalse(TEXT("A truncated slot is refused"), Save->LoadGalaxy(GalaxyID));
	}
	TestTrue(TEXT("Test slot removed"), Save->DeleteSave(GalaxyID));
	TestFalse(TEXT("A missing slot is refused"), Save->LoadGalaxy(GalaxyID));
	TestEqual(TEXT("Refused loads leave the galaxy untouched"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(HashBeforeRefusals));

	// A save newer than this build.
	Sim->NewGalaxy(43, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	const FGuid NewerID = Sim->GetState().GalaxyID;
	Sim->GetMutableState().SaveVersion = FGalaxyState::CurrentSaveVersion + 1;
	TestTrue(TEXT("Save of a newer-version state succeeds"), Save->SaveCurrentGalaxy());
	Sim->NewGalaxy(44, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	const uint32 HashBeforeNewer = Sim->ComputeStateHash();
	TestFalse(TEXT("A newer save is refused"), Save->LoadGalaxy(NewerID));
	TestEqual(TEXT("A refused newer save leaves the galaxy untouched"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(HashBeforeNewer));
	TestTrue(TEXT("Newer-version slot removed"), Save->DeleteSave(NewerID));
	return true;
}

// The Phase 1 exit criterion needs a restart in the middle, so this test spans two processes: the first run saves and
// records the hash, the next run (a new process of the same build) loads and compares, and also simulates the same
// seed afresh. A rebuilt binary starts over. After a failed verification the save and the record stay, so every later
// process of that build fails the same way.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveRestartTest, "Cosmic_Ascension.Sim.SaveRestart", CosmicTests::Flags)
bool FCosmicSimSaveRestartTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicSaveSubsystem* Save = Sim->GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>();

	const FString RecordPath = FPaths::ProjectSavedDir() / TEXT("Automation/SaveRestart.txt");
	const FString BinaryStamp = IFileManager::Get().GetTimeStamp(*CosmicSimTests::SimBinaryPath()).ToIso8601();
	const uint32 ProcessId = FPlatformProcess::GetCurrentProcessId();

	FString Record;
	FString RecordedStamp;
	FString RecordedGalaxy;
	FString RecordedHash;
	uint32 RecordedProcessId = 0;
	FGuid RecordedGalaxyID;
	const bool bHasRecord = FFileHelper::LoadFileToString(Record, *RecordPath)
		&& FParse::Value(*Record, TEXT("Binary="), RecordedStamp)
		&& FParse::Value(*Record, TEXT("Galaxy="), RecordedGalaxy)
		&& FParse::Value(*Record, TEXT("Hash="), RecordedHash)
		&& FParse::Value(*Record, TEXT("Process="), RecordedProcessId)
		&& FGuid::Parse(RecordedGalaxy, RecordedGalaxyID);

	if (bHasRecord && RecordedStamp == BinaryStamp && RecordedProcessId != ProcessId)
	{
		// Second half: a save written by an earlier process of this same build.
		bool bVerified = false;
		if (TestTrue(TEXT("Load of the save written by the previous process succeeds"), Save->LoadGalaxy(RecordedGalaxyID)))
		{
			const bool bHashOk = TestEqual(TEXT("Hash after restart and load equals the hash recorded at save"),
				FString::Printf(TEXT("%u"), Sim->ComputeStateHash()), RecordedHash);
			const bool bTickOk = TestEqual(TEXT("Tick after restart and load"), Sim->GetState().Tick, static_cast<int64>(1000));
			// Simulation determinism across processes, not only the hash of a loaded state.
			const bool bFreshOk = TestEqual(TEXT("A fresh run of seed 42 in this process equals the hash recorded by the previous process"),
				FString::Printf(TEXT("%u"), CosmicSimTests::HashAfter(*Sim, 42, 1000)), RecordedHash);
			bVerified = bHashOk && bTickOk && bFreshOk;
		}
		// A slot that is gone cannot be verified again: start over.
		if (bVerified || !Save->DoesSaveExist(RecordedGalaxyID))
		{
			Save->DeleteSave(RecordedGalaxyID);
			IFileManager::Get().Delete(*RecordPath);
		}
		if (bVerified)
		{
			AddInfo(TEXT("Restart half verified against the save of an earlier process."));
		}
		return true;
	}

	// First half (or a stale record from another build): leave a save and its hash for the next process.
	if (bHasRecord)
	{
		Save->DeleteSave(RecordedGalaxyID);
	}
	const uint32 Hash = CosmicSimTests::HashAfter(*Sim, 42, 1000);
	if (!TestTrue(TEXT("Save succeeds"), Save->SaveCurrentGalaxy()))
	{
		return false;
	}
	Record = FString::Printf(TEXT("Binary=%s Galaxy=%s Hash=%u Process=%u"),
		*BinaryStamp, *Sim->GetState().GalaxyID.ToString(EGuidFormats::Digits), Hash, ProcessId);
	TestTrue(TEXT("Record written"), FFileHelper::SaveStringToFile(Record, *RecordPath));
	AddInfo(TEXT("Saved and recorded the hash. Run the tests once more in a new process to verify the restart half."));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

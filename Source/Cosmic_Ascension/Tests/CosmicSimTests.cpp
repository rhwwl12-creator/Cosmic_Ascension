// Cosmic Ascension - automation tests of the simulation core. Design: guideline.md section 9.2, 9.3.
// Run with the automation command in CLAUDE.md (-game: the tests use the running GameInstance and replace its galaxy).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/GalaxyState.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UObject/UnrealType.h"

namespace
{
	constexpr EAutomationTestFlags SimTestFlags = EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter;

	// Vertical slice size (section 12.1).
	constexpr int32 TestSystems = 20;
	constexpr int32 TestCivs = 6;

	UGameInstance* FindGameInstance()
	{
		if (GEngine)
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.OwningGameInstance)
				{
					return Context.OwningGameInstance;
				}
			}
		}
		return nullptr;
	}

	// The simulation of the running game, paused so that frame ticks cannot interleave with a test. Null without a GameInstance.
	UGalaxySimulationSubsystem* AcquireSim(FAutomationTestBase& Test)
	{
		UGameInstance* GameInstance = FindGameInstance();
		UGalaxySimulationSubsystem* Sim = GameInstance ? GameInstance->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
		if (!Sim)
		{
			Test.AddError(TEXT("No GameInstance: run the automation tests with -game (see CLAUDE.md)."));
			return nullptr;
		}
		Sim->SetTimeScale(0.f);
		return Sim;
	}

	uint32 HashAfter(UGalaxySimulationSubsystem& Sim, int32 Seed, int64 Ticks)
	{
		Sim.NewGalaxy(Seed, TestSystems, TestCivs);
		Sim.RunHeadless(Ticks);
		return Sim.ComputeStateHash();
	}

	// Counts non-finite floating point values anywhere under a struct, by reflection.
	int32 CountNonFinite(const FProperty* Property, const void* Value);

	int32 CountNonFiniteInStruct(const UStruct* Struct, const void* Data)
	{
		int32 Count = 0;
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			Count += CountNonFinite(*It, It->ContainerPtrToValuePtr<void>(Data));
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
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimDeterminismTest, "Cosmic_Ascension.Sim.Determinism", SimTestFlags)
bool FCosmicSimDeterminismTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	const uint32 First = HashAfter(*Sim, 42, 3000);
	const uint32 Second = HashAfter(*Sim, 42, 3000);
	TestEqual(TEXT("Seed 42, 3,000 ticks, twice: same hash"), static_cast<int64>(Second), static_cast<int64>(First));

	const uint32 OtherSeed = HashAfter(*Sim, 43, 3000);
	TestTrue(TEXT("A different seed gives a different hash"), OtherSeed != First);

	const uint32 Shorter = HashAfter(*Sim, 42, 2999);
	TestTrue(TEXT("One tick less gives a different hash"), Shorter != First);

	TestEqual(TEXT("Tick counter after 2,999 ticks"), Sim->GetState().Tick, static_cast<int64>(2999));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimNoNaNTest, "Cosmic_Ascension.Sim.NoNaN", SimTestFlags)
bool FCosmicSimNoNaNTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	Sim->NewGalaxy(7, TestSystems, TestCivs);
	Sim->RunHeadless(10000);
	TestEqual(TEXT("Non-finite floats in the state after 10,000 ticks"), CountNonFiniteInStruct(FGalaxyState::StaticStruct(), &Sim->GetState()), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveRoundTripTest, "Cosmic_Ascension.Sim.SaveRoundTrip", SimTestFlags)
bool FCosmicSimSaveRoundTripTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicSaveSubsystem* Save = Sim->GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>();

	Sim->NewGalaxy(42, TestSystems, TestCivs);
	Sim->RunHeadless(1000);
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
	const bool bLoaded = TestTrue(TEXT("Load succeeds"), Save->LoadGalaxy(GalaxyID));
	if (bLoaded)
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

	TestTrue(TEXT("Test slot removed"), Save->DeleteSave(GalaxyID));
	return true;
}

// The Phase 1 exit criterion needs a restart in the middle, so this test spans two processes: the first run saves and
// records the hash, the next run (a new process of the same build) loads and compares. A rebuilt binary starts over.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveRestartTest, "Cosmic_Ascension.Sim.SaveRestart", SimTestFlags)
bool FCosmicSimSaveRestartTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicSaveSubsystem* Save = Sim->GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>();

	const FString RecordPath = FPaths::ProjectSavedDir() / TEXT("Automation/SaveRestart.txt");
	const FString BinaryStamp = IFileManager::Get().GetTimeStamp(*SimBinaryPath()).ToIso8601();
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
		if (TestTrue(TEXT("Load of the save written by the previous process succeeds"), Save->LoadGalaxy(RecordedGalaxyID)))
		{
			TestEqual(TEXT("Hash after restart and load equals the hash recorded at save"),
				FString::Printf(TEXT("%u"), Sim->ComputeStateHash()), RecordedHash);
			TestEqual(TEXT("Tick after restart and load"), Sim->GetState().Tick, static_cast<int64>(1000));
		}
		Save->DeleteSave(RecordedGalaxyID);
		IFileManager::Get().Delete(*RecordPath);
		return true;
	}

	// First half (or a stale record from another build): leave a save and its hash for the next process.
	if (bHasRecord)
	{
		Save->DeleteSave(RecordedGalaxyID);
	}
	Sim->NewGalaxy(42, TestSystems, TestCivs);
	Sim->RunHeadless(1000);
	if (!TestTrue(TEXT("Save succeeds"), Save->SaveCurrentGalaxy()))
	{
		return false;
	}
	Record = FString::Printf(TEXT("Binary=%s Galaxy=%s Hash=%u Process=%u"),
		*BinaryStamp, *Sim->GetState().GalaxyID.ToString(EGuidFormats::Digits), Sim->ComputeStateHash(), ProcessId);
	TestTrue(TEXT("Record written"), FFileHelper::SaveStringToFile(Record, *RecordPath));
	AddInfo(TEXT("Saved and recorded the hash. Run the tests once more in a new process to verify the restart half."));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

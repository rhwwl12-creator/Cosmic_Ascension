// Cosmic Ascension - automation tests of the fourth and fifth rungs (B6b): from B-6b1 the two saved counters of a people
// (LawsHeldTicks, PressureEnduredTicks) through the save and the hashes. The count of the endured pressure is
// Sim.PressureEndured (Tests/CosmicBookTests.cpp, beside the fallen-sphere counter it is counted with).
// Design: docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md section 2.1, 6; docs/20 section 9.1.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Containers/StringConv.h"
#include "Core/GalaxyState.h"
#include "Engine/GameInstance.h"
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

namespace CosmicUpperRungTests
{
	// The engine's save bytes of a state (the carrier the game's slot wraps - Save/CosmicSaveSubsystem.cpp).
	TArray<uint8> SaveBytesOf(const FGalaxyState& State)
	{
		UCosmicSaveGame* Carrier = NewObject<UCosmicSaveGame>(GetTransientPackage());
		Carrier->State = State;
		TArray<uint8> Raw;
		UGameplayStatics::SaveGameToMemory(Carrier, Raw);
		return Raw;
	}

	// Renames every saved member called Name to a name no build knows (its last letter changed, so the bytes keep their
	// length): what a save made before the member was added reads like. Returns how many it renamed.
	int32 Forget(TArray<uint8>& Raw, const TCHAR* Name)
	{
		const auto Ansi = StringCast<ANSICHAR>(Name);
		int32 Renamed = 0;
		for (int32 At = 0; At + Ansi.Length() <= Raw.Num(); ++At)
		{
			if (FMemory::Memcmp(Raw.GetData() + At, Ansi.Get(), Ansi.Length()) == 0)
			{
				Raw[At + Ansi.Length() - 1] = static_cast<uint8>('Q');
				++Renamed;
			}
		}
		return Renamed;
	}
}

// The two counters through the save (header section 2.1, 6): their initializers are the frozen 0 (CLAUDE.md rule 7 -
// mutation: an initializer of 1 breaks it); off 0 they round trip civilization by civilization through the game's slot with
// the hash they were saved with; and a save without them - written before B-6b1 - loads them as 0 while the rest of each
// civilization comes back (an element of Civs is written whole, so the old bytes simply lack the two members).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveLawsHeldTest, "Cosmic_Ascension.Sim.SaveLawsHeld", CosmicTests::Flags)
bool FCosmicSimSaveLawsHeldTest::RunTest(const FString& Parameters)
{
	using namespace CosmicUpperRungTests;
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

	// (1) The frozen initializers.
	const FCivilizationState Fresh;
	TestEqual(TEXT("The frozen LawsHeldTicks is 0"), Fresh.LawsHeldTicks, 0);
	TestEqual(TEXT("The frozen PressureEnduredTicks is 0"), Fresh.PressureEnduredTicks, 0);

	// (2) Off 0, both round trip through the game's slot, each on its own people, with the hash.
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(200);
	FGalaxyState& Live = Sim->GetMutableState();
	Galaxy = Live.GalaxyID;
	if (!TestTrue(TEXT("The slice has three peoples"), Live.Civs.Num() >= 3))
	{
		return false;
	}
	Live.Civs[1].LawsHeldTicks = 321;
	Live.Civs[2].PressureEnduredTicks = 654;
	Live.Civs[2].FallenSphereTicks = 77;
	const uint32 Hash = Sim->ComputeStateHash();
	const uint32 World = Sim->ComputeWorldHash();
	if (TestTrue(TEXT("The run with both counters off 0 saves"), Save->SaveCurrentGalaxy()))
	{
		Live.Civs[1].LawsHeldTicks = 0;
		Live.Civs[2].PressureEnduredTicks = 0;
		if (TestTrue(TEXT("and loads"), Save->LoadGalaxy(Galaxy)))
		{
			const FGalaxyState& Loaded = Sim->GetState();
			TestEqual(TEXT("LawsHeldTicks survives the round trip"), Loaded.Civs[1].LawsHeldTicks, 321);
			TestEqual(TEXT("on its own people only"), Loaded.Civs[2].LawsHeldTicks, 0);
			TestEqual(TEXT("PressureEnduredTicks survives the round trip"), Loaded.Civs[2].PressureEnduredTicks, 654);
			TestEqual(TEXT("on its own people only"), Loaded.Civs[1].PressureEnduredTicks, 0);
			TestEqual(TEXT("with the hash it was saved with"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Hash));
			TestEqual(TEXT("and the world hash"), static_cast<int64>(Sim->ComputeWorldHash()), static_cast<int64>(World));
		}
	}

	// (3) A save without the two members (an old save) loads them as 0, and the rest of the element as it was.
	TArray<uint8> Raw = SaveBytesOf(Sim->GetState());
	TestEqual(TEXT("Every people writes LawsHeldTicks"), Forget(Raw, TEXT("LawsHeldTicks")), Sim->GetState().Civs.Num());
	TestEqual(TEXT("Every people writes PressureEnduredTicks"), Forget(Raw, TEXT("PressureEnduredTicks")), Sim->GetState().Civs.Num());
	const UCosmicSaveGame* Old = Cast<UCosmicSaveGame>(UGameplayStatics::LoadGameFromMemory(Raw));
	if (TestNotNull(TEXT("The bytes without the two members load"), Old) && TestEqual(TEXT("with every people"), Old->State.Civs.Num(), Sim->GetState().Civs.Num()))
	{
		TestEqual(TEXT("An old save reads LawsHeldTicks as 0"), Old->State.Civs[1].LawsHeldTicks, 0);
		TestEqual(TEXT("An old save reads PressureEnduredTicks as 0"), Old->State.Civs[2].PressureEnduredTicks, 0);
		TestEqual(TEXT("and the member beside them as it was saved"), Old->State.Civs[2].FallenSphereTicks, 77);
		TestEqual(TEXT("and the people's name"), Old->State.Civs[2].CivID, Sim->GetState().Civs[2].CivID);
	}
	return true;
}

// The two counters are world state (header section 2.1): either of them off its value moves both hashes, and putting it back
// restores both. The exclusion lists are untouched (CLAUDE.md rule 6); they name top-level members of FGalaxyState only, so a
// people's counter cannot be left out there - the mutation is a hasher that skips either name, which leaves both hashes where
// they were (Tools/mutations/B6b/b6b1_hasher_skips_counters.patch).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimHashCoversNewCountersTest, "Cosmic_Ascension.Sim.HashCoversNewCounters", CosmicTests::Flags)
bool FCosmicSimHashCoversNewCountersTest::RunTest(const FString& Parameters)
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
	if (!TestTrue(TEXT("The slice has two peoples"), State.Civs.Num() >= 2))
	{
		return false;
	}
	const uint32 World = Sim->ComputeWorldHash();
	const uint32 Full = Sim->ComputeStateHash();

	auto Moves = [&](const TCHAR* What, TFunctionRef<int32&(FCivilizationState&)> Counter)
	{
		int32& Value = Counter(State.Civs[1]);
		const int32 Was = Value;
		Value = Was + 600;
		TestTrue(*FString::Printf(TEXT("%s moves the world hash"), What), Sim->ComputeWorldHash() != World);
		TestTrue(*FString::Printf(TEXT("%s moves the full hash"), What), Sim->ComputeStateHash() != Full);
		Value = Was;
		TestTrue(*FString::Printf(TEXT("(both hashes are back after %s is put back)"), What), Sim->ComputeWorldHash() == World && Sim->ComputeStateHash() == Full);
	};
	Moves(TEXT("LawsHeldTicks"), [](FCivilizationState& Civ) -> int32& { return Civ.LawsHeldTicks; });
	Moves(TEXT("PressureEnduredTicks"), [](FCivilizationState& Civ) -> int32& { return Civ.PressureEnduredTicks; });
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

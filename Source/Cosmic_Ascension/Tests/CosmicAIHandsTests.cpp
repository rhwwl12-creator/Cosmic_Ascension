// Cosmic Ascension - automation tests of the AI peoples' hands (B7): from [B-7a1] FoundingCivID, WarTicks,
// TreatyBrokenTick, TradeShockUntilTick, PendingAskCivID and PendingExileCivID through saving, both hashes, the four
// runtime write sites (NewGalaxy, CosmicMegaprojects::Advance on project_settle, TickConflict end, CosmicDiplomacy::
// Break and TurnHostile), and the cleanup hooks (TickCrossings, Install, ReleaseThrone, Die, BecomeRuler).
// Design: docs/60_DECISIONS/headers/B7_ai_peoples_hands.md sections 2.7, 2.8, 2.10, 2.15, 6; docs/20 section 9.1.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Containers/StringConv.h"
#include "Core/CosmicSeed.h"
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Kismet/GameplayStatics.h"
#include "Math/NumericLimits.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Save/CosmicSaveGame.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"

#include <initializer_list>

namespace CosmicAIHandsTests
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

	// Renames every occurrence of Name in the save bytes by changing its last byte, simulating a save written before
	// that property was added. Returns the number of occurrences rewritten.
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

	// Builds a quiet 12-system, 4-people board wired as CosmicTests::WireChain (0..9, 4-10, 10-11), with inert bricks,
	// no random events, no drift, no sight draws, and no unprompted AI hostilities (AIHostilePacifismMax = -2).
	FGalaxyState* LayQuietBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Original, TFunctionRef<void(FCosmicRulesDef&)> Edit)
	{
		const FCosmicRulesDef* Game = Original.FindRules();
		if (!Game)
		{
			return nullptr;
		}
		FCosmicRulesDef Rules = CosmicTests::WithInertBricks(CosmicTests::WithoutStartingEdict(*Game));
		Rules.EventChancePerSlot = 0.f;
		Rules.SpiritualPressureStabilityDrain = 0.f;
		Rules.FirstCrisisTick = 0;
		Rules.SocietyRecoveryBonus = 0.f;
		Rules.RealmCapByTier = { 6, 6, 6, 6, 6 };
		Rules.RelationDriftRate = 0.f;
		Rules.FleetPerSettledByTier = { 0.f, 2.f, 2.f, 2.f, 2.f };
		Rules.FleetUpkeepEnergyPerPower = 0.f;
		Rules.PlagueSpreadMultiplier = 1.f;
		Rules.AIHostilePacifismMax = -2.f;
		Rules.FirstSightTick = 1000000;
		Rules.SightIntervalTicks = 1000000;
		Edit(Rules);

		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));
		Tables.Hazards = CosmicTests::NewTable(FHazardDef::StaticStruct());
		Tables.Events = nullptr;
		Sim.SetTables(Tables);
		Sim.NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim.GetMutableState();
		CosmicTests::StripBricks(State);
		if (State.Civs.Num() != 4 || !CosmicTests::WireChain(State))
		{
			return nullptr;
		}
		State.Transcendents.Reset();
		CosmicTests::ClearOwners(State);
		for (FStarSystemState& System : State.Systems)
		{
			System.ActiveHazard = FGameplayTag();
			System.HazardTicksRemaining = 0;
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
			System.ResponseMegaproject = NAME_None;
			System.ResponseProgress = 0.f;
			System.CompletedMegaprojects.Reset();
			System.FoundingCivID = NAME_None;
		}
		CosmicTests::Give(State, 0, { 0, 1 });
		CosmicTests::Give(State, 1, { 3, 2, 4 });
		CosmicTests::Give(State, 2, { 6, 5, 7 });
		CosmicTests::Give(State, 3, { 9, 8, 10, 11 });
		const int32 Capitals[] = { 0, 3, 6, 9 };
		for (int32 CivIndex = 0; CivIndex < 4; ++CivIndex)
		{
			FCivilizationState& Civ = State.Civs[CivIndex];
			Civ.HomeSystemID = State.Systems[Capitals[CivIndex]].SystemID;
			State.Systems[Capitals[CivIndex]].FoundingCivID = Civ.CivID;
			Civ.Tier = EAscensionTier::Tier2_Stellar;
			Civ.Ethics = FEthicsAxes();
			Civ.FleetPower = 0.f;
			Civ.FleetTargetSystemID = NAME_None;
			Civ.FleetTargetSinceTick = 0;
			Civ.Ties.Reset();
			Civ.Relations.Reset();
			Civ.ActiveEdicts.Reset();
			Civ.Stability = 0.75f;
			Civ.Stockpile = FResourcePool();
			Civ.WarTicks = 0;
			Civ.TreatyBrokenTick = 0;
			Civ.TradeShockUntilTick = 0;
		}
		State.Player.CurrentSystemID = State.Systems[0].SystemID;
		State.Knowing = FKnowingState();
		CosmicTests::SetKnowing(State, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }, { 0, 1, 2, 3 });
		State.Knowing.FirstContactCivID = State.Civs[1].CivID;
		Sim.RebuildStateIndex();
		Sim.SyncRngState();
		return &State;
	}

	int32 IndexOfSystemNamed(const FGalaxyState& State, FName SystemID)
	{
		for (int32 Index = 0; Index < State.Systems.Num(); ++Index)
		{
			if (State.Systems[Index].SystemID == SystemID)
			{
				return Index;
			}
		}
		return INDEX_NONE;
	}
}

// Frozen defaults, save slot round-trip on distinct elements, and delta load of saves written before B7 [B-7a1]
// (header sections 2.7, 2.8, 2.10, 2.15, 6; rule 7). Also pins CosmicSeed::Purpose::TreatyAsk = 14.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveB7FieldsTest, "Cosmic_Ascension.Sim.SaveB7Fields", CosmicTests::Flags)
bool FCosmicSimSaveB7FieldsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicAIHandsTests;
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

	// (1) Frozen default initializers and fixed derived seed purpose number.
	const FStarSystemState FreshSystem;
	TestTrue(TEXT("The frozen FoundingCivID is NAME_None"), FreshSystem.FoundingCivID.IsNone());
	const FCivilizationState FreshCiv;
	TestEqual(TEXT("The frozen WarTicks is 0"), FreshCiv.WarTicks, 0);
	TestEqual(TEXT("The frozen TreatyBrokenTick is 0"), FreshCiv.TreatyBrokenTick, int64(0));
	TestEqual(TEXT("The frozen TradeShockUntilTick is 0"), FreshCiv.TradeShockUntilTick, int64(0));
	const FKnowingState FreshKnowing;
	TestTrue(TEXT("The frozen PendingAskCivID is NAME_None"), FreshKnowing.PendingAskCivID.IsNone());
	TestTrue(TEXT("The frozen PendingExileCivID is NAME_None"), FreshKnowing.PendingExileCivID.IsNone());
	TestEqual(TEXT("CosmicSeed::Purpose::TreatyAsk is fixed at 14"), CosmicSeed::Purpose::TreatyAsk, uint32(14));

	// (2) Round-trip through the game's save slot, element by element, with both hashes intact.
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(20);
	FGalaxyState& Live = Sim->GetMutableState();
	Galaxy = Live.GalaxyID;
	int32 WildIndex = INDEX_NONE;
	for (int32 Index = 0; Index < Live.Systems.Num(); ++Index)
	{
		if (Live.Systems[Index].OwnerCivID.IsNone() && !CosmicState::IsSanctuary(Live.Systems[Index].SystemID))
		{
			WildIndex = Index;
			break;
		}
	}
	if (!TestTrue(TEXT("The slice has 3+ peoples and a wild system"), Live.Civs.Num() >= 3 && WildIndex != INDEX_NONE))
	{
		return false;
	}

	int32 OtherSettled = INDEX_NONE;
	for (int32 Index = 1; Index < Live.Systems.Num(); ++Index)
	{
		if (Index != WildIndex && !CosmicState::IsSanctuary(Live.Systems[Index].SystemID))
		{
			OtherSettled = Index;
			break;
		}
	}
	if (!TestTrue(TEXT("The slice has a distinct second system"), OtherSettled != INDEX_NONE && OtherSettled != 0 && OtherSettled != WildIndex))
	{
		return false;
	}
	const FName FounderZero = Live.Civs[0].CivID;
	const FName FounderTwo = Live.Civs[2].CivID;
	Live.Systems[0].FoundingCivID = FounderZero;
	Live.Systems[OtherSettled].FoundingCivID = FounderTwo;
	Live.Systems[WildIndex].FoundingCivID = NAME_None;

	Live.Civs[0].WarTicks = 0;
	Live.Civs[0].TreatyBrokenTick = 0;
	Live.Civs[0].TradeShockUntilTick = 0;
	Live.Civs[1].FleetTargetSinceTick = 33;
	Live.Civs[1].WarTicks = 7;
	Live.Civs[1].TreatyBrokenTick = 21;
	Live.Civs[1].TradeShockUntilTick = 141;
	Live.Civs[2].WarTicks = 19;
	Live.Civs[2].TreatyBrokenTick = 55;
	Live.Civs[2].TradeShockUntilTick = 255;

	const FName AskID = Live.Civs[2].CivID;
	const FName ExileID = Live.Civs[1].CivID;
	Live.Knowing.PendingContactCivID = Live.Civs[1].CivID;
	Live.Knowing.PendingAskCivID = AskID;
	Live.Knowing.PendingExileCivID = ExileID;

	const uint32 SavedFullHash = Sim->ComputeStateHash();
	const uint32 SavedWorldHash = Sim->ComputeWorldHash();
	if (TestTrue(TEXT("The run with non-default B7 fields saves"), Save->SaveCurrentGalaxy()))
	{
		Live.Systems[0].FoundingCivID = NAME_None;
		Live.Systems[OtherSettled].FoundingCivID = NAME_None;
		Live.Civs[1].WarTicks = 0;
		Live.Civs[1].TreatyBrokenTick = 0;
		Live.Civs[1].TradeShockUntilTick = 0;
		Live.Civs[2].WarTicks = 0;
		Live.Civs[2].TreatyBrokenTick = 0;
		Live.Civs[2].TradeShockUntilTick = 0;
		Live.Knowing.PendingAskCivID = NAME_None;
		Live.Knowing.PendingExileCivID = NAME_None;
		if (TestTrue(TEXT("and loads"), Save->LoadGalaxy(Galaxy)))
		{
			const FGalaxyState& Loaded = Sim->GetState();
			TestEqual(TEXT("FoundingCivID survives on system 0"), Loaded.Systems[0].FoundingCivID, FounderZero);
			TestEqual(TEXT("and independently on another system"), Loaded.Systems[OtherSettled].FoundingCivID, FounderTwo);
			TestTrue(TEXT("while the wild system stays un-founded"), Loaded.Systems[WildIndex].FoundingCivID.IsNone());
			TestEqual(TEXT("Civ 0 stays 0 for WarTicks"), Loaded.Civs[0].WarTicks, 0);
			TestEqual(TEXT("Civ 1 keeps WarTicks"), Loaded.Civs[1].WarTicks, 7);
			TestEqual(TEXT("Civ 1 keeps TreatyBrokenTick"), Loaded.Civs[1].TreatyBrokenTick, int64(21));
			TestEqual(TEXT("Civ 1 keeps TradeShockUntilTick"), Loaded.Civs[1].TradeShockUntilTick, int64(141));
			TestEqual(TEXT("Civ 2 keeps its own WarTicks"), Loaded.Civs[2].WarTicks, 19);
			TestEqual(TEXT("Civ 2 keeps its own TreatyBrokenTick"), Loaded.Civs[2].TreatyBrokenTick, int64(55));
			TestEqual(TEXT("Civ 2 keeps its own TradeShockUntilTick"), Loaded.Civs[2].TradeShockUntilTick, int64(255));
			TestEqual(TEXT("PendingAskCivID survives"), Loaded.Knowing.PendingAskCivID, AskID);
			TestEqual(TEXT("PendingExileCivID survives"), Loaded.Knowing.PendingExileCivID, ExileID);
			TestEqual(TEXT("State hash matches saved hash"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(SavedFullHash));
			TestEqual(TEXT("World hash matches saved world hash"), static_cast<int64>(Sim->ComputeWorldHash()), static_cast<int64>(SavedWorldHash));
		}
	}

	// (3) A save lacking the six B7 fields loads them at frozen defaults while keeping every existing element field.
	TArray<uint8> Raw = SaveBytesOf(Sim->GetState());
	const int32 NumSystems = Sim->GetState().Systems.Num();
	const int32 NumCivs = Sim->GetState().Civs.Num();
	TestEqual(TEXT("Every system writes FoundingCivID"), Forget(Raw, TEXT("FoundingCivID")), NumSystems);
	TestEqual(TEXT("Every civ writes WarTicks"), Forget(Raw, TEXT("WarTicks")), NumCivs);
	TestEqual(TEXT("Every civ writes TreatyBrokenTick"), Forget(Raw, TEXT("TreatyBrokenTick")), NumCivs);
	TestEqual(TEXT("Every civ writes TradeShockUntilTick"), Forget(Raw, TEXT("TradeShockUntilTick")), NumCivs);
	TestEqual(TEXT("Knowing writes PendingAskCivID"), Forget(Raw, TEXT("PendingAskCivID")), 1);
	TestEqual(TEXT("Knowing writes PendingExileCivID"), Forget(Raw, TEXT("PendingExileCivID")), 1);

	const UCosmicSaveGame* Old = Cast<UCosmicSaveGame>(UGameplayStatics::LoadGameFromMemory(Raw));
	if (TestNotNull(TEXT("Old save bytes without the B7 fields load"), Old)
		&& TestEqual(TEXT("with every system"), Old->State.Systems.Num(), NumSystems)
		&& TestEqual(TEXT("and every civ"), Old->State.Civs.Num(), NumCivs))
	{
		TestTrue(TEXT("Old save reads FoundingCivID as NAME_None"), Old->State.Systems[OtherSettled].FoundingCivID.IsNone());
		TestEqual(TEXT("Old save reads WarTicks as 0"), Old->State.Civs[1].WarTicks, 0);
		TestEqual(TEXT("Old save reads TreatyBrokenTick as 0"), Old->State.Civs[1].TreatyBrokenTick, int64(0));
		TestEqual(TEXT("Old save reads TradeShockUntilTick as 0"), Old->State.Civs[1].TradeShockUntilTick, int64(0));
		TestTrue(TEXT("Old save reads PendingAskCivID as NAME_None"), Old->State.Knowing.PendingAskCivID.IsNone());
		TestTrue(TEXT("Old save reads PendingExileCivID as NAME_None"), Old->State.Knowing.PendingExileCivID.IsNone());
		TestEqual(TEXT("Peer FleetTargetSinceTick survives intact"), Old->State.Civs[1].FleetTargetSinceTick, int64(33));
		TestEqual(TEXT("Peer PendingContactCivID survives intact"), Old->State.Knowing.PendingContactCivID, Sim->GetState().Civs[1].CivID);
		TestEqual(TEXT("Peer system OwnerCivID survives intact"), Old->State.Systems[0].OwnerCivID, Sim->GetState().Systems[0].OwnerCivID);
	}
	return true;
}

// Hash coverage of all six [B-7a1] fields (world vs knowing separation), all four runtime write sites
// (NewGalaxy, project_settle completion and non-overwrite guarantees, TickConflict end, CosmicDiplomacy::Break and
// TurnHostile), and the cleanup hooks in TickCrossings, Install, ReleaseThrone/BecomeWanderer, Die and BecomeRuler.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimHashCoversB7FieldsTest, "Cosmic_Ascension.Sim.HashCoversB7Fields", CosmicTests::Flags)
bool FCosmicSimHashCoversB7FieldsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicAIHandsTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	if (!TestNotNull(TEXT("The player role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables OriginalTables = Sim->GetTables();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(OriginalTables);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};

	// (A) Direct field coverage across both hashes vs state-hash-only for Knowing slots.
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(20);
	{
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The slice has 3+ civs and 2+ systems"), State.Civs.Num() >= 3 && State.Systems.Num() >= 2))
		{
			return false;
		}
		const uint32 BaseWorld = Sim->ComputeWorldHash();
		const uint32 BaseFull = Sim->ComputeStateHash();

		auto CheckWorldName = [&](const TCHAR* What, FName& Field, FName TestValue)
		{
			const FName Was = Field;
			Field = TestValue;
			TestTrue(*FString::Printf(TEXT("%s moves the world hash"), What), Sim->ComputeWorldHash() != BaseWorld);
			TestTrue(*FString::Printf(TEXT("%s moves the state hash"), What), Sim->ComputeStateHash() != BaseFull);
			Field = Was;
			TestTrue(*FString::Printf(TEXT("(%s restores both hashes)"), What), Sim->ComputeWorldHash() == BaseWorld && Sim->ComputeStateHash() == BaseFull);
		};
		auto CheckWorldInt32 = [&](const TCHAR* What, int32& Field)
		{
			const int32 Was = Field;
			Field = Was + 9;
			TestTrue(*FString::Printf(TEXT("%s moves the world hash"), What), Sim->ComputeWorldHash() != BaseWorld);
			TestTrue(*FString::Printf(TEXT("%s moves the state hash"), What), Sim->ComputeStateHash() != BaseFull);
			Field = Was;
			TestTrue(*FString::Printf(TEXT("(%s restores both hashes)"), What), Sim->ComputeWorldHash() == BaseWorld && Sim->ComputeStateHash() == BaseFull);
		};
		auto CheckWorldInt64 = [&](const TCHAR* What, int64& Field)
		{
			const int64 Was = Field;
			Field = Was + 9;
			TestTrue(*FString::Printf(TEXT("%s moves the world hash"), What), Sim->ComputeWorldHash() != BaseWorld);
			TestTrue(*FString::Printf(TEXT("%s moves the state hash"), What), Sim->ComputeStateHash() != BaseFull);
			Field = Was;
			TestTrue(*FString::Printf(TEXT("(%s restores both hashes)"), What), Sim->ComputeWorldHash() == BaseWorld && Sim->ComputeStateHash() == BaseFull);
		};

		CheckWorldName(TEXT("FoundingCivID"), State.Systems[1].FoundingCivID, State.Civs[2].CivID);
		CheckWorldInt32(TEXT("WarTicks"), State.Civs[1].WarTicks);
		CheckWorldInt64(TEXT("TreatyBrokenTick"), State.Civs[1].TreatyBrokenTick);
		CheckWorldInt64(TEXT("TradeShockUntilTick"), State.Civs[1].TradeShockUntilTick);

		// Swapping TreatyBrokenTick and TradeShockUntilTick changes hashes (field-order sensitive, not commutative).
		{
			FCivilizationState& Civ = State.Civs[1];
			const int64 WasBroken = Civ.TreatyBrokenTick;
			const int64 WasShock = Civ.TradeShockUntilTick;
			Civ.TreatyBrokenTick = 11;
			Civ.TradeShockUntilTick = 29;
			const uint32 FirstOrderWorld = Sim->ComputeWorldHash();
			Civ.TreatyBrokenTick = 29;
			Civ.TradeShockUntilTick = 11;
			TestTrue(TEXT("Swapping TreatyBrokenTick and TradeShockUntilTick moves world hash"), Sim->ComputeWorldHash() != FirstOrderWorld);
			Civ.TreatyBrokenTick = WasBroken;
			Civ.TradeShockUntilTick = WasShock;
		}

		auto CheckKnowingOnly = [&](const TCHAR* What, FName& Field, FName TestValue)
		{
			const FName Was = Field;
			Field = TestValue;
			TestEqual(*FString::Printf(TEXT("%s leaves world hash identical"), What), static_cast<int64>(Sim->ComputeWorldHash()), static_cast<int64>(BaseWorld));
			TestTrue(*FString::Printf(TEXT("%s moves state hash"), What), Sim->ComputeStateHash() != BaseFull);
			Field = Was;
			TestEqual(*FString::Printf(TEXT("(%s restores state hash)"), What), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(BaseFull));
		};
		CheckKnowingOnly(TEXT("PendingAskCivID"), State.Knowing.PendingAskCivID, State.Civs[1].CivID);
		CheckKnowingOnly(TEXT("PendingExileCivID"), State.Knowing.PendingExileCivID, State.Civs[2].CivID);

		// (B) Write site 1: NewGalaxy stamps FoundingCivID on every starting home system and leaves wild systems NAME_None.
		for (const FCivilizationState& Civ : State.Civs)
		{
			const int32 HomeIdx = IndexOfSystemNamed(State, Civ.HomeSystemID);
			TestTrue(*FString::Printf(TEXT("NewGalaxy gave %s a valid home"), *Civ.CivID.ToString()), State.Systems.IsValidIndex(HomeIdx));
			if (State.Systems.IsValidIndex(HomeIdx))
			{
				TestEqual(*FString::Printf(TEXT("Home %s FoundingCivID == OwnerCivID"), *Civ.HomeSystemID.ToString()),
					State.Systems[HomeIdx].FoundingCivID, Civ.CivID);
			}
		}
		for (const FStarSystemState& System : State.Systems)
		{
			if (System.OwnerCivID.IsNone())
			{
				TestTrue(*FString::Printf(TEXT("Unowned system %s has FoundingCivID NAME_None"), *System.SystemID.ToString()),
					System.FoundingCivID.IsNone());
			}
		}
		if (State.Civs.Num() > 0)
		{
			const int32 HomeZeroIdx = IndexOfSystemNamed(State, State.Civs[0].HomeSystemID);
			if (State.Systems.IsValidIndex(HomeZeroIdx))
			{
				const uint32 WithStamp = Sim->ComputeWorldHash();
				const uint32 WithStampFull = Sim->ComputeStateHash();
				const FName SavedFounder = State.Systems[HomeZeroIdx].FoundingCivID;
				State.Systems[HomeZeroIdx].FoundingCivID = NAME_None;
				TestTrue(TEXT("NewGalaxy FoundingCivID stamp moves hashes"), Sim->ComputeWorldHash() != WithStamp && Sim->ComputeStateHash() != WithStampFull);
				State.Systems[HomeZeroIdx].FoundingCivID = SavedFounder;
			}
		}
	}

	// (C) Write site 2: CosmicMegaprojects::Advance sets FoundingCivID on project_settle completion if NAME_None,
	// ignores non-settle projects, and never overwrites across Transfer, LetPeopleGo + re-settle, Destroy, or TickCrossings.
	{
		FGalaxyState* Board = LayQuietBoard(*Sim, OriginalTables, [](FCosmicRulesDef&) {});
		if (!TestNotNull(TEXT("Board laid for settlement checks"), Board))
		{
			return false;
		}
		FMegaprojectDef FreeRow;
		FreeRow.DisplayName = FText::AsCultureInvariant(TEXT("free"));
		FreeRow.RequiredProgress = 1.f;
		UDataTable* Projects = CosmicTests::NewTable(FMegaprojectDef::StaticStruct());
		Projects->AddRow(CosmicRows::ProjectSettle, FreeRow);
		Projects->AddRow(CosmicTests::StandingProject(), FreeRow);
		FCosmicDataTables Payable = Sim->GetTables();
		Payable.Megaprojects = Projects;
		const FCosmicRulesDef BoardRules = Sim->GetRules();

		// Make systems 10 and 11 unowned wild systems (Civ 3 retains systems 8 and 9).
		Board->Civs[3].OwnedSystems.Remove(Board->Systems[10].SystemID);
		Board->Civs[3].OwnedSystems.Remove(Board->Systems[11].SystemID);
		Board->Systems[10].OwnerCivID = NAME_None;
		Board->Systems[11].OwnerCivID = NAME_None;
		Board->Systems[10].FoundingCivID = NAME_None;
		Board->Systems[11].FoundingCivID = NAME_None;
		Sim->RebuildStateIndex();

		// 1. Completing a non-settle construction project on a system without FoundingCivID leaves FoundingCivID NAME_None.
		Board->Systems[2].FoundingCivID = NAME_None;
		Board->Systems[2].ActiveMegaproject = CosmicTests::StandingProject();
		Board->Systems[2].MegaprojectProgress = 0.f;
		{
			FRandomStream Stream(5);
			FCosmicEvalContext Ctx(*Board, Stream, Sim->GetStateIndex(), Payable, BoardRules);
			CosmicMegaprojects::Advance(Ctx, 2, CosmicMegaprojects::ECosmicProjectSlot::Construction, 1.f);
		}
		TestTrue(TEXT("Non-settle project completion does not write FoundingCivID"), Board->Systems[2].FoundingCivID.IsNone());

		// 2. Completing project_settle writes FoundingCivID = CompleterID and moves both hashes vs NAME_None.
		{
			FRandomStream Stream(5);
			FCosmicEvalContext Ctx(*Board, Stream, Sim->GetStateIndex(), Payable, BoardRules);
			TestTrue(TEXT("Civ 1 can start project_settle on system 10"), CosmicMegaprojects::Settle(Ctx, 1, 10));
			Board->Systems[10].MegaprojectProgress = 0.f;
			CosmicMegaprojects::Advance(Ctx, 10, CosmicMegaprojects::ECosmicProjectSlot::Construction, 1.f);
		}
		TestEqual(TEXT("Settle completion writes FoundingCivID = Civ 1"), Board->Systems[10].FoundingCivID, Board->Civs[1].CivID);
		{
			const uint32 HashWithFounder = Sim->ComputeStateHash();
			const uint32 WorldWithFounder = Sim->ComputeWorldHash();
			const FName Was = Board->Systems[10].FoundingCivID;
			Board->Systems[10].FoundingCivID = NAME_None;
			TestTrue(TEXT("Settle FoundingCivID moves both hashes"), Sim->ComputeStateHash() != HashWithFounder && Sim->ComputeWorldHash() != WorldWithFounder);
			Board->Systems[10].FoundingCivID = Was;
		}

		// 3. Transfer, Destroy, LetPeopleGo + re-settle completion never overwrite FoundingCivID.
		{
			FRandomStream Stream(5);
			FCosmicEvalContext Ctx(*Board, Stream, Sim->GetStateIndex(), Payable, BoardRules);
			TestTrue(TEXT("Transfer hands system 10 to Civ 2"), CosmicHazards::Transfer(Ctx, 10, 2));
			TestEqual(TEXT("Transfer preserves original FoundingCivID"), Board->Systems[10].FoundingCivID, Board->Civs[1].CivID);

			Board->Systems[11].FoundingCivID = Board->Civs[0].CivID;
			CosmicHazards::Destroy(Ctx, 11);
			TestEqual(TEXT("Destroy preserves original FoundingCivID"), Board->Systems[11].FoundingCivID, Board->Civs[0].CivID);

			// Prepare system 4 (already owned & settled by Civ 1, founded by Civ 2) for LetPeopleGo -> re-settle by Civ 3.
			Board->Systems[4].FoundingCivID = Board->Civs[2].CivID;
			TestTrue(TEXT("LetPeopleGo releases Civ 1 systems"), CosmicHazards::LetPeopleGo(Ctx, 1));
			TestEqual(TEXT("LetPeopleGo preserves system 4 FoundingCivID"), Board->Systems[4].FoundingCivID, Board->Civs[2].CivID);
			Board->Systems[4].ActiveMegaproject = CosmicRows::ProjectSettle;
			Board->Systems[4].MegaprojectProgress = 0.f;
			Board->Systems[4].OwnerCivID = Board->Civs[3].CivID;
			Board->Civs[3].OwnedSystems.Add(Board->Systems[4].SystemID);
			Sim->RebuildStateIndex();
			CosmicMegaprojects::Advance(Ctx, 4, CosmicMegaprojects::ECosmicProjectSlot::Construction, 1.f);
			TestTrue(TEXT("Re-settle completed by Civ 3"), Board->Systems[4].ActiveMegaproject.IsNone());
			TestEqual(TEXT("Re-settle does NOT overwrite existing FoundingCivID"), Board->Systems[4].FoundingCivID, Board->Civs[2].CivID);
		}

		// 4. Crossing cleanup deletes Civ 1 (now owning 0 systems), yet system 10 retains Civ 1's FoundingCivID.
		const FName VanishedID = Board->Civs[1].CivID;
		Sim->RunHeadless(1);
		TestEqual(TEXT("TickCrossings removed empty Civ 1"), Sim->GetStateIndex().FindCiv(Sim->GetState(), VanishedID), INDEX_NONE);
		TestEqual(TEXT("Vanished founder CivID remains recorded on system 10"), Sim->GetState().Systems[10].FoundingCivID, VanishedID);
	}

	// (D) Write site 3: TickConflict counts WarTicks at stage end (after siege resolution) for both sides, skips guards,
	// clamps at MAX_int32, and resets to 0 on the very tick a siege ends.
	{
		FGalaxyState* Board = LayQuietBoard(*Sim, OriginalTables, [](FCosmicRulesDef& Rules)
		{
			Rules.SiegeTicks = 4;
		});
		if (!TestNotNull(TEXT("Board laid for WarTicks checks"), Board))
		{
			return false;
		}
		// Ruled Civ 0 guards its own system 0 (not an active siege -> WarTicks stays 0).
		Board->Civs[0].FleetPower = 2.f;
		Board->Civs[0].FleetTargetSystemID = Board->Systems[0].SystemID;
		Board->Civs[0].FleetTargetSinceTick = Board->Tick;

		// AI Civ 1 besieges AI Civ 2's settled non-capital system 5 (adjacent across 4-5; Civ 2 holds {6, 5, 7}).
		Board->Civs[1].FleetPower = 2.f;
		Board->Civs[2].FleetPower = 0.5f;
		CosmicDiplomacy::SetStanceOf(Board->Civs[1], Board->Civs[2].CivID, CosmicTags::StanceHostile());
		Board->Civs[1].FleetTargetSystemID = Board->Systems[5].SystemID;
		Board->Civs[1].FleetTargetSinceTick = Board->Tick;

		// Tick 1 (Tick == 1 < 0 + 4): ongoing siege stands -> WarTicks == 1 on attacker and defender, 0 on guarding Civ 0 & idle Civ 3.
		Sim->RunHeadless(1);
		TestEqual(TEXT("Guarding Civ 0 has WarTicks == 0"), Sim->GetState().Civs[0].WarTicks, 0);
		TestEqual(TEXT("Besieger Civ 1 counts WarTicks == 1"), Sim->GetState().Civs[1].WarTicks, 1);
		TestEqual(TEXT("Besieged Civ 2 counts WarTicks == 1"), Sim->GetState().Civs[2].WarTicks, 1);
		TestEqual(TEXT("Idle Civ 3 stays WarTicks == 0"), Sim->GetState().Civs[3].WarTicks, 0);

		// Tick 2: WarTicks == 2 on both sides.
		Sim->RunHeadless(1);
		TestEqual(TEXT("Besieger Civ 1 counts WarTicks == 2"), Sim->GetState().Civs[1].WarTicks, 2);
		TestEqual(TEXT("Besieged Civ 2 counts WarTicks == 2"), Sim->GetState().Civs[2].WarTicks, 2);

		// Verify MAX_int32 saturation on tick 3.
		Sim->GetMutableState().Civs[2].WarTicks = MAX_int32;
		Sim->RunHeadless(1);
		TestEqual(TEXT("Besieger Civ 1 counts WarTicks == 3"), Sim->GetState().Civs[1].WarTicks, 3);
		TestEqual(TEXT("MAX_int32 WarTicks saturates without overflow"), Sim->GetState().Civs[2].WarTicks, MAX_int32);

		// Tick 4 (Tick == 4 >= 0 + 4): siege ends in step (4) -> TickConflict end resets WarTicks to 0 on BOTH sides.
		Sim->RunHeadless(1);
		TestTrue(TEXT("Siege ended on tick 4"), Sim->GetState().Civs[1].FleetTargetSystemID.IsNone());
		TestEqual(TEXT("Besieger WarTicks resets to 0 on siege completion tick"), Sim->GetState().Civs[1].WarTicks, 0);
		TestEqual(TEXT("Besieged WarTicks resets to 0 on siege completion tick"), Sim->GetState().Civs[2].WarTicks, 0);
	}

	// (E) Write site 4: CosmicDiplomacy::Break and TurnHostile stamp TreatyBrokenTick and TradeShockUntilTick.
	{
		FGalaxyState* Board = LayQuietBoard(*Sim, OriginalTables, [](FCosmicRulesDef&) {});
		if (!TestNotNull(TEXT("Board laid for diplomacy timestamp checks"), Board))
		{
			return false;
		}
		Board->Tick = 50;

		TestFalse(TEXT("Unheld Break returns false"), CosmicDiplomacy::Break(*Board, 0, 1, CosmicTags::TreatyTrade(), 120));
		TestEqual(TEXT("Unheld Break keeps TreatyBrokenTick at 0"), Board->Civs[0].TreatyBrokenTick, int64(0));
		TestEqual(TEXT("Unheld Break keeps A's TradeShockUntilTick at 0"), Board->Civs[0].TradeShockUntilTick, int64(0));
		TestEqual(TEXT("Unheld Break keeps B's TradeShockUntilTick at 0"), Board->Civs[1].TradeShockUntilTick, int64(0));

		// Non-aggression break: sets breaker's TreatyBrokenTick only, never TradeShockUntilTick.
		CosmicDiplomacy::Sign(*Board, 0, 1, CosmicTags::TreatyNonAggression());
		TestTrue(TEXT("Held NonAggression Break returns true"), CosmicDiplomacy::Break(*Board, 0, 1, CosmicTags::TreatyNonAggression(), 120));
		TestEqual(TEXT("Breaker gets TreatyBrokenTick = Tick"), Board->Civs[0].TreatyBrokenTick, int64(50));
		TestEqual(TEXT("Partner keeps TreatyBrokenTick = 0"), Board->Civs[1].TreatyBrokenTick, int64(0));
		TestEqual(TEXT("NonAggression break leaves A's TradeShockUntilTick 0"), Board->Civs[0].TradeShockUntilTick, int64(0));
		TestEqual(TEXT("NonAggression break leaves B's TradeShockUntilTick 0"), Board->Civs[1].TradeShockUntilTick, int64(0));

		// Trade break with negative shock clamps shock window to State.Tick (max(0, TradeShockTicks)).
		CosmicDiplomacy::Sign(*Board, 0, 1, CosmicTags::TreatyTrade());
		TestTrue(TEXT("Held Trade Break returns true"), CosmicDiplomacy::Break(*Board, 0, 1, CosmicTags::TreatyTrade(), -5));
		TestEqual(TEXT("Negative TradeShockTicks clamps to State.Tick on A"), Board->Civs[0].TradeShockUntilTick, int64(50));
		TestEqual(TEXT("Negative TradeShockTicks clamps to State.Tick on B"), Board->Civs[1].TradeShockUntilTick, int64(50));

		// Trade break with positive shock stamps TreatyBrokenTick on A and TradeShockUntilTick on BOTH A and B.
		Board->Tick = 55;
		CosmicDiplomacy::Sign(*Board, 0, 1, CosmicTags::TreatyTrade());
		TestTrue(TEXT("Held Trade Break with positive shock returns true"), CosmicDiplomacy::Break(*Board, 0, 1, CosmicTags::TreatyTrade(), 120));
		TestEqual(TEXT("Trade Break updates A's TreatyBrokenTick"), Board->Civs[0].TreatyBrokenTick, int64(55));
		TestEqual(TEXT("Trade Break leaves B's TreatyBrokenTick untouched"), Board->Civs[1].TreatyBrokenTick, int64(0));
		TestEqual(TEXT("Trade Break stamps A's TradeShockUntilTick = Tick + 120"), Board->Civs[0].TradeShockUntilTick, int64(175));
		TestEqual(TEXT("Trade Break stamps B's TradeShockUntilTick = Tick + 120"), Board->Civs[1].TradeShockUntilTick, int64(175));

		// TurnHostile forwards TradeShockTicks when breaking a trade treaty, and keeps prior stamps when no trade treaty held.
		Board->Tick = 60;
		CosmicDiplomacy::Sign(*Board, 1, 2, CosmicTags::TreatyTrade());
		TestTrue(TEXT("TurnHostile with trade returns true"), CosmicDiplomacy::TurnHostile(*Board, 1, 2, 0.25f, 80));
		TestEqual(TEXT("TurnHostile stamps Civ 1 TreatyBrokenTick"), Board->Civs[1].TreatyBrokenTick, int64(60));
		TestEqual(TEXT("TurnHostile leaves Civ 2 TreatyBrokenTick untouched"), Board->Civs[2].TreatyBrokenTick, int64(0));
		TestEqual(TEXT("TurnHostile stamps Civ 1 TradeShockUntilTick = 140"), Board->Civs[1].TradeShockUntilTick, int64(140));
		TestEqual(TEXT("TurnHostile stamps Civ 2 TradeShockUntilTick = 140"), Board->Civs[2].TradeShockUntilTick, int64(140));

		Board->Tick = 70;
		TestFalse(TEXT("TurnHostile without trade returns false"), CosmicDiplomacy::TurnHostile(*Board, 0, 2, 0.25f, 80));
		TestEqual(TEXT("Unheld TurnHostile leaves Civ 0 TreatyBrokenTick at 55"), Board->Civs[0].TreatyBrokenTick, int64(55));
		TestEqual(TEXT("Unheld TurnHostile leaves Civ 0 TradeShockUntilTick at 175"), Board->Civs[0].TradeShockUntilTick, int64(175));
		TestEqual(TEXT("Unheld TurnHostile leaves Civ 2 TradeShockUntilTick at 140"), Board->Civs[2].TradeShockUntilTick, int64(140));
	}

	// (F) Knowing cleanup hooks: TickCrossings, BecomeWanderer (ReleaseThrone keeps PendingExileCivID), BecomeRuler,
	// Install, and Die step (e).
	{
		FGalaxyState* Board = LayQuietBoard(*Sim, OriginalTables, [](FCosmicRulesDef&) {});
		if (!TestNotNull(TEXT("Board laid for Knowing cleanup checks"), Board))
		{
			return false;
		}
		const FName SurvivingID = Board->Civs[1].CivID;
		const FName EmptyID = Board->Civs[3].CivID;

		// 1. TickCrossings clears removed CivID from PendingAskCivID and PendingExileCivID, keeps surviving CivIDs.
		Board->Knowing.PendingAskCivID = SurvivingID;
		Board->Knowing.PendingExileCivID = EmptyID;
		for (FStarSystemState& System : Board->Systems)
		{
			if (System.OwnerCivID == EmptyID)
			{
				System.OwnerCivID = NAME_None;
			}
		}
		Board->Civs[3].OwnedSystems.Reset();
		Sim->RebuildStateIndex();
		Sim->RunHeadless(1);
		TestEqual(TEXT("TickCrossings removed empty Civ 3"), Sim->GetStateIndex().FindCiv(Sim->GetState(), EmptyID), INDEX_NONE);
		TestEqual(TEXT("TickCrossings keeps PendingAskCivID of surviving civ"), Sim->GetState().Knowing.PendingAskCivID, SurvivingID);
		TestTrue(TEXT("TickCrossings clears PendingExileCivID matching removed civ"), Sim->GetState().Knowing.PendingExileCivID.IsNone());

		FGalaxyState& Clean = Sim->GetMutableState();
		Clean.Knowing.PendingAskCivID = SurvivingID;
		Clean.Knowing.PendingExileCivID = SurvivingID;
		Clean.Civs[2].OwnedSystems.Reset();
		for (FStarSystemState& System : Clean.Systems)
		{
			if (System.OwnerCivID == Clean.Civs[2].CivID)
			{
				System.OwnerCivID = NAME_None;
			}
		}
		const FName Empty2ID = Clean.Civs[2].CivID;
		Clean.Knowing.PendingAskCivID = Empty2ID;
		Sim->RebuildStateIndex();
		Sim->RunHeadless(1);
		TestTrue(TEXT("TickCrossings clears PendingAskCivID matching removed civ"), Sim->GetState().Knowing.PendingAskCivID.IsNone());
		TestEqual(TEXT("TickCrossings keeps PendingExileCivID of surviving civ"), Sim->GetState().Knowing.PendingExileCivID, SurvivingID);

		// 2. BecomeWanderer (ReleaseThrone): clears PendingContactCivID and PendingAskCivID, leaves PendingExileCivID intact.
		FGalaxyState& Live = Sim->GetMutableState();
		Live.Knowing.PendingContactCivID = SurvivingID;
		Live.Knowing.PendingAskCivID = SurvivingID;
		Live.Knowing.PendingExileCivID = SurvivingID;
		const FGameplayTag Conservative = FGameplayTag::RequestGameplayTag(FName(TEXT("AI.Profile.Conservative")));
		Role->BecomeWanderer(Conservative, CosmicTags::ReignAbdicated());
		TestEqual(TEXT("BecomeWanderer moved vessel to Wanderer"), Sim->GetState().Player.Role, EPlayerRole::Wanderer);
		TestTrue(TEXT("ReleaseThrone clears PendingContactCivID"), Sim->GetState().Knowing.PendingContactCivID.IsNone());
		TestTrue(TEXT("ReleaseThrone clears PendingAskCivID"), Sim->GetState().Knowing.PendingAskCivID.IsNone());
		TestEqual(TEXT("ReleaseThrone preserves PendingExileCivID for [B-7d2] deposed BecomeWanderer"),
			Sim->GetState().Knowing.PendingExileCivID, SurvivingID);

		// 3. BecomeRuler: clears all three waiting slots.
		FGalaxyState& Wanderer = Sim->GetMutableState();
		const int32 TargetCivIdx = Sim->GetStateIndex().FindCiv(Wanderer, SurvivingID);
		if (!TestTrue(TEXT("Surviving civ exists for BecomeRuler"), Wanderer.Civs.IsValidIndex(TargetCivIdx)))
		{
			return false;
		}
		Wanderer.Player.CurrentSystemID = Wanderer.Systems[3].SystemID;
		Wanderer.Player.TravelTargetSystemID = NAME_None;
		Wanderer.Player.TravelTicksRemaining = 0;
		Wanderer.Civs[TargetCivIdx].ThroneLockedUntilTick = 0;
		Wanderer.Civs[TargetCivIdx].ReverenceToPlayer = Sim->GetRules().CoronationReverence + 10.f;
		Wanderer.Knowing.PendingPetitionRow = NAME_None;
		Wanderer.Knowing.PendingPetitionSystemID = NAME_None;
		Wanderer.Knowing.PendingContactCivID = SurvivingID;
		Wanderer.Knowing.PendingAskCivID = SurvivingID;
		Wanderer.Knowing.PendingExileCivID = SurvivingID;
		const CosmicWanderer::ECrownBlock CrownResult = Role->BecomeRuler(SurvivingID);
		TestEqual(TEXT("BecomeRuler succeeded"), static_cast<int32>(CrownResult), static_cast<int32>(CosmicWanderer::ECrownBlock::None));
		TestEqual(TEXT("BecomeRuler made vessel Ruler of SurvivingID"), Sim->GetState().Player.RuledCivID, SurvivingID);
		TestTrue(TEXT("BecomeRuler clears PendingContactCivID"), Sim->GetState().Knowing.PendingContactCivID.IsNone());
		TestTrue(TEXT("BecomeRuler clears PendingAskCivID"), Sim->GetState().Knowing.PendingAskCivID.IsNone());
		TestTrue(TEXT("BecomeRuler clears PendingExileCivID"), Sim->GetState().Knowing.PendingExileCivID.IsNone());

		// 4. Die (step e): clears all three waiting slots (even though ReleaseThrone leaves PendingExileCivID).
		FGalaxyState& Dying = Sim->GetMutableState();
		Dying.Knowing.PendingContactCivID = SurvivingID;
		Dying.Knowing.PendingAskCivID = SurvivingID;
		Dying.Knowing.PendingExileCivID = SurvivingID;
		Dying.Calling.PendingEffect = CosmicTags::EffectKillPlayer();
		Dying.Calling.PendingCause = CosmicTags::DeathAge();
		Dying.Calling.PendingSourceRow = NAME_None;
		Role->Die();
		TestTrue(TEXT("Die clears PendingContactCivID"), Sim->GetState().Knowing.PendingContactCivID.IsNone());
		TestTrue(TEXT("Die clears PendingAskCivID"), Sim->GetState().Knowing.PendingAskCivID.IsNone());
		TestTrue(TEXT("Die step (e) clears PendingExileCivID"), Sim->GetState().Knowing.PendingExileCivID.IsNone());

		// 5. Install: clears all three waiting slots.
		FGalaxyState& Installing = Sim->GetMutableState();
		Installing.Knowing.PendingContactCivID = SurvivingID;
		Installing.Knowing.PendingAskCivID = SurvivingID;
		Installing.Knowing.PendingExileCivID = SurvivingID;
		Role->Install(Installing.Systems[0].SystemID, Installing.Civs[0].CivID, EPlayerRole::Ruler);
		TestTrue(TEXT("Install clears PendingContactCivID"), Sim->GetState().Knowing.PendingContactCivID.IsNone());
		TestTrue(TEXT("Install clears PendingAskCivID"), Sim->GetState().Knowing.PendingAskCivID.IsNone());
		TestTrue(TEXT("Install clears PendingExileCivID"), Sim->GetState().Knowing.PendingExileCivID.IsNone());
	}

	return true;
}

#endif

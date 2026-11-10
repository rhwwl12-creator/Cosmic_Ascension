// Cosmic Ascension - automation tests of the existence bricks (B7x): from B-7x1 the six time vectors of a people
// (FCivilizationState::TimeVectors) and the host of a system (FStarSystemState::HostCivID) through the save and the hashes;
// from B-7x2 the module CosmicBricks - the count of the vectors, the bricks' shares, the settle gate's reason Inward, the AI
// mutation step and the kept law's edicts, the end shape, the seed bricks of NewGalaxy - and the three conditions.
// Design: docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md section 2.1, 6 (rules R2, R3); docs/20 section 9.1.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Containers/StringConv.h"
#include "Core/CosmicSeed.h"
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Events/CosmicEventSubsystem.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "Misc/ScopeExit.h"
#include "Save/CosmicSaveGame.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicExistenceBricks.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/Function.h"
#include "Tests/CosmicB6bRowNames.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/Field.h"
#include "UObject/NameTypes.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

#include <initializer_list>

namespace CosmicExistenceBricksTests
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

	// Renames every occurrence of Name in the save bytes to a name no build knows (its last letter changed, so the bytes keep
	// their length): what a save made before the member was added reads like. Returns how many it renamed. Only an OUTER name
	// is ever forgotten here - an inner name such as Endured is a substring of PressureEndured(Ticks) and would corrupt it.
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

	// The six vectors as one list, in the fixed order (Built, Consumed, Drifted, Linked, Isolated, Endured).
	TArray<int32> VectorsOf(const FCosmicTimeVectors& Vectors)
	{
		return { Vectors.Built, Vectors.Consumed, Vectors.Drifted, Vectors.Linked, Vectors.Isolated, Vectors.Endured };
	}

	// Sets the six vectors from a first value counting up by one.
	void SetVectors(FCosmicTimeVectors& Vectors, int32 First)
	{
		Vectors.Built = First;
		Vectors.Consumed = First + 1;
		Vectors.Drifted = First + 2;
		Vectors.Linked = First + 3;
		Vectors.Isolated = First + 4;
		Vectors.Endured = First + 5;
	}
}

// The time vectors and the host through the save (header section 2.1, 6; rules R2, R3): their initializers are the frozen 0
// and NAME_None (CLAUDE.md rule 7 - mutation: an initializer of 1 breaks it); off them they round trip people by people and
// system by system through the game's slot with the hash they were saved with; and a save without them - written before
// B-7x1 - loads them at their initializers while the rest of each element comes back (an element of Civs or Systems is
// written whole, so the old bytes simply lack the member). The old save forgets the OUTER name TimeVectors - never an inner
// one: Endured is a substring of PressureEnduredTicks.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveTimeVectorsTest, "Cosmic_Ascension.Sim.SaveTimeVectors", CosmicTests::Flags)
bool FCosmicSimSaveTimeVectorsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicExistenceBricksTests;
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
	TestTrue(TEXT("The frozen time vectors are all 0"), VectorsOf(Fresh.TimeVectors) == TArray<int32>({ 0, 0, 0, 0, 0, 0 }));
	const FStarSystemState FreshSystem;
	TestTrue(TEXT("The frozen HostCivID is NAME_None"), FreshSystem.HostCivID.IsNone());

	// (2) Off their initializers, both round trip through the game's slot, each on its own element, with both hashes.
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(200);
	FGalaxyState& Live = Sim->GetMutableState();
	Galaxy = Live.GalaxyID;
	if (!TestTrue(TEXT("The slice has three peoples and two systems"), Live.Civs.Num() >= 3 && Live.Systems.Num() >= 2))
	{
		return false;
	}
	SetVectors(Live.Civs[1].TimeVectors, 301);
	SetVectors(Live.Civs[2].TimeVectors, 401);
	Live.Civs[2].PressureEnduredTicks = 77;
	Live.Systems[1].HostCivID = Live.Civs[2].CivID;
	const TArray<int32> One = VectorsOf(Live.Civs[1].TimeVectors);
	const TArray<int32> Two = VectorsOf(Live.Civs[2].TimeVectors);
	const TArray<int32> Zero = VectorsOf(Live.Civs[0].TimeVectors);
	const FName Host = Live.Civs[2].CivID;
	const uint32 Hash = Sim->ComputeStateHash();
	const uint32 World = Sim->ComputeWorldHash();
	if (TestTrue(TEXT("The run with the vectors and a host off their initializers saves"), Save->SaveCurrentGalaxy()))
	{
		SetVectors(Live.Civs[1].TimeVectors, 0);
		Live.Systems[1].HostCivID = NAME_None;
		if (TestTrue(TEXT("and loads"), Save->LoadGalaxy(Galaxy)))
		{
			const FGalaxyState& Loaded = Sim->GetState();
			TestTrue(TEXT("The six vectors survive the round trip"), VectorsOf(Loaded.Civs[1].TimeVectors) == One);
			TestTrue(TEXT("on their own people only"), VectorsOf(Loaded.Civs[2].TimeVectors) == Two && VectorsOf(Loaded.Civs[0].TimeVectors) == Zero);
			TestEqual(TEXT("HostCivID survives the round trip"), Loaded.Systems[1].HostCivID, Host);
			TestTrue(TEXT("on its own system only"), Loaded.Systems[0].HostCivID.IsNone());
			TestEqual(TEXT("with the hash it was saved with"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Hash));
			TestEqual(TEXT("and the world hash"), static_cast<int64>(Sim->ComputeWorldHash()), static_cast<int64>(World));
		}
	}

	// (3) A save without the two members (an old save) loads them at their initializers, and the rest of the element as it
	// was. The struct's own type name holds the outer name too (CosmicTimeVectors), so each people may carry it twice.
	TArray<uint8> Raw = SaveBytesOf(Sim->GetState());
	const int32 Civs = Sim->GetState().Civs.Num();
	const int32 VectorNames = Forget(Raw, TEXT("TimeVectors"));
	TestTrue(TEXT("Every people writes TimeVectors"), VectorNames >= Civs);
	TestEqual(TEXT("Every system writes HostCivID"), Forget(Raw, TEXT("HostCivID")), Sim->GetState().Systems.Num());
	const UCosmicSaveGame* Old = Cast<UCosmicSaveGame>(UGameplayStatics::LoadGameFromMemory(Raw));
	if (TestNotNull(TEXT("The bytes without the two members load"), Old) && TestEqual(TEXT("with every people"), Old->State.Civs.Num(), Civs)
		&& TestTrue(TEXT("and every system"), Old->State.Systems.Num() == Sim->GetState().Systems.Num()))
	{
		TestTrue(TEXT("An old save reads the time vectors as 0"), VectorsOf(Old->State.Civs[2].TimeVectors) == TArray<int32>({ 0, 0, 0, 0, 0, 0 }));
		TestTrue(TEXT("An old save reads HostCivID as NAME_None"), Old->State.Systems[1].HostCivID.IsNone());
		TestEqual(TEXT("and the counter whose name holds Endured as it was saved"), Old->State.Civs[2].PressureEnduredTicks, 77);
		TestEqual(TEXT("and the people's name"), Old->State.Civs[2].CivID, Sim->GetState().Civs[2].CivID);
		TestEqual(TEXT("and the system's owner"), Old->State.Systems[1].OwnerCivID, Sim->GetState().Systems[1].OwnerCivID);
	}
	return true;
}

// The time vectors and the host are world state (header section 2.1; rules R2, R3): any of the six vectors off its value, or a
// host named on a system, moves both hashes, and putting it back restores both. The exclusion lists are untouched (CLAUDE.md
// rule 6): they name top-level members of FGalaxyState only - the mutation is a hasher that skips the names.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimHashCoversTimeVectorsTest, "Cosmic_Ascension.Sim.HashCoversTimeVectors", CosmicTests::Flags)
bool FCosmicSimHashCoversTimeVectorsTest::RunTest(const FString& Parameters)
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
	if (!TestTrue(TEXT("The slice has three peoples and two systems"), State.Civs.Num() >= 3 && State.Systems.Num() >= 2))
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
	Moves(TEXT("Built"), [](FCivilizationState& Civ) -> int32& { return Civ.TimeVectors.Built; });
	Moves(TEXT("Consumed"), [](FCivilizationState& Civ) -> int32& { return Civ.TimeVectors.Consumed; });
	Moves(TEXT("Drifted"), [](FCivilizationState& Civ) -> int32& { return Civ.TimeVectors.Drifted; });
	Moves(TEXT("Linked"), [](FCivilizationState& Civ) -> int32& { return Civ.TimeVectors.Linked; });
	Moves(TEXT("Isolated"), [](FCivilizationState& Civ) -> int32& { return Civ.TimeVectors.Isolated; });
	Moves(TEXT("Endured"), [](FCivilizationState& Civ) -> int32& { return Civ.TimeVectors.Endured; });

	// Two vectors swapped are another state: the struct's members are hashed in their order, not as a sum.
	{
		FCosmicTimeVectors& Vectors = State.Civs[1].TimeVectors;
		const FCosmicTimeVectors Was = Vectors;
		Vectors.Built = Was.Built + 5;
		Vectors.Consumed = Was.Consumed + 7;
		const uint32 First = Sim->ComputeWorldHash();
		Vectors.Built = Was.Built + 7;
		Vectors.Consumed = Was.Consumed + 5;
		TestTrue(TEXT("Two vectors that trade their values move the world hash apart"), Sim->ComputeWorldHash() != First);
		Vectors = Was;
	}

	// The host of a system.
	FStarSystemState& System = State.Systems[1];
	const FName WasHost = System.HostCivID;
	System.HostCivID = State.Civs[2].CivID;
	TestTrue(TEXT("A host named on a system moves the world hash"), Sim->ComputeWorldHash() != World);
	TestTrue(TEXT("and the full hash"), Sim->ComputeStateHash() != Full);
	System.HostCivID = WasHost;
	TestTrue(TEXT("(both hashes are back after the host is taken off)"), Sim->ComputeWorldHash() == World && Sim->ComputeStateHash() == Full);
	return true;
}

// ---- B-7x2: the module CosmicBricks ----

namespace CosmicExistenceBricksTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// The edicts and the hazard of the boards below (the test builds them: no case leans on the game's rows). A people's edict
	// with no cost of any kind (the AI revokes it at once - CosmicEdicts::AIRevokesOf), and a Draw plague of countdown 200.
	const FName Kept(TEXT("edict_test_kept"));
	const FName Sick(TEXT("crisis_test_sick"));

	UDataTable* Edicts()
	{
		UDataTable* Table = CosmicTests::NewTable(FEdictDef::StaticStruct());
		FEdictDef Row;
		Row.DisplayName = FText::AsCultureInvariant(TEXT("kept"));
		Row.Summary = FText::AsCultureInvariant(TEXT("a test edict."));
		Row.Scope = ECosmicEdictScope::People;
		Row.RequiredTier = EAscensionTier::Tier1_Planetary;
		Table->AddRow(Kept, Row);
		return Table;
	}

	UDataTable* Hazards()
	{
		UDataTable* Table = CosmicTests::NewTable(FHazardDef::StaticStruct());
		FHazardDef Plague;
		Plague.HazardTag = Tag(TEXT("Hazard.Plague"));
		Plague.DisplayName = FText::AsCultureInvariant(TEXT("sick"));
		Plague.CountdownTicks = 200;
		Plague.SpawnRule = ECosmicHazardSpawn::Draw;
		Plague.SpawnChancePerTick = 0.f;
		Table->AddRow(Sick, Plague);
		return Table;
	}

	void NoEdit(FCosmicRulesDef&) {}

	/**
	 * The board of Tests/CosmicUpperRungTests.cpp: twelve systems wired as CosmicTests::WireChain, four peoples owning all of
	 * them, all settled - 0 (the player's) {0, 1}, 1 {3, 2, 4}, 2 {6, 5, 7}, 3 {9, 8, 10, 11}; capitals 0, 3, 6, 9. 0 borders
	 * 1, 1 borders 2 and 3, 2 borders 3. Quiet rules (no event, no drain, no first crisis, no inherited edict, no drift, no
	 * fleet, no sight stage), unpaid projects (CosmicTests::WithoutProjects), this file's edict and plague, no transcendent.
	 * The bricks inert whatever DT_Rules holds (CosmicTests::WithInertBricks - a case sets the knob it reads in Edit) and no
	 * people carrying one (CosmicTests::StripBricks - no seed brick, no archetype brick): a case lays the bricks it reads.
	 * Every people on the first rung, no tie, no edict, no fleet target, Stability 0.75, 1000 of each resource; every time
	 * vector 0 (a fresh galaxy). So no fact of the six holds for any people on the board as laid.
	 */
	FGalaxyState* LayBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Original, TFunctionRef<void(FCosmicRulesDef&)> Edit)
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
		Rules.RealmCapByTier = { 4, 4, 5, 5, 5 };
		Rules.RelationDriftRate = 0.f;
		Rules.FleetPerSettledByTier = { 0.f, 0.f, 0.f, 0.f, 0.f };
		Rules.FirstSightTick = 1000000;
		Rules.SightIntervalTicks = 1000000;
		Edit(Rules);
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));
		Tables.Edicts = Edicts();
		Tables.Hazards = Hazards();
		Tables.Events = nullptr;
		Tables.Encounters = nullptr;
		Tables.Transcendents = nullptr;
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
			CosmicHazards::EmptyHazardSlot(System);
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
			System.ResponseMegaproject = NAME_None;
			System.ResponseProgress = 0.f;
			System.CompletedMegaprojects.Reset();
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
			Civ.Tier = EAscensionTier::Tier1_Planetary;
			Civ.Ethics = FEthicsAxes();
			Civ.FleetPower = 0.f;
			Civ.FleetTargetSystemID = NAME_None;
			Civ.FleetTargetSinceTick = 0;
			Civ.Ties.Reset();
			Civ.Relations.Reset();
			Civ.ActiveEdicts.Reset();
			Civ.Stability = 0.75f;
			Civ.Research.Add(FName(TEXT("Society")), 1.f);
			Civ.Stockpile.Energy = 1000.f;
			Civ.Stockpile.Matter = 1000.f;
			Civ.Stockpile.Insight = 1000.f;
			Civ.Stockpile.Influence = 1000.f;
			Civ.Stockpile.Causality = 0.f;
			Civ.TimeVectors = FCosmicTimeVectors();
		}
		Sim.RebuildStateIndex();
		Sim.SyncRngState();
		return &State;
	}

	FCosmicCondition Cond(EConditionType Type, const TCHAR* TagName, const TCHAR* Key = nullptr, float Value = 0.f)
	{
		FCosmicCondition Condition;
		Condition.Type = Type;
		Condition.Tag = TagName ? Tag(TagName) : FGameplayTag();
		Condition.Key = Key ? FName(Key) : FName();
		Condition.Value = Value;
		return Condition;
	}
}

// The count of the six time vectors (header section 2.2): on a board where no fact holds, each fact laid on people 1 makes
// exactly its vector grow (people 2, the control, stays at 0); the vectors are cumulative - a second count adds one more, and
// a fact that stops leaves the count where it was; the economy stage counts them (one StepOnce); and the count takes no draw.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimTimeVectorsCountTest, "Cosmic_Ascension.Sim.TimeVectorsCount", CosmicTests::Flags)
bool FCosmicSimTimeVectorsCountTest::RunTest(const FString& Parameters)
{
	using namespace CosmicExistenceBricksTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const TArray<int32> None = { 0, 0, 0, 0, 0, 0 };
	// VectorsOf order: Built, Consumed, Drifted, Linked, Isolated, Endured.
	auto Case = [&](const TCHAR* What, TFunctionRef<void(FGalaxyState&)> Lay, const TArray<int32>& Expected, TFunctionRef<void(FCosmicRulesDef&)> Edit)
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, Edit);
		if (!TestNotNull(*FString::Printf(TEXT("The board is laid (%s)"), What), Board))
		{
			return;
		}
		Lay(*Board);
		const int32 SeedBefore = Sim->GetRng().GetCurrentSeed();
		CosmicBricks::CountTimeVectors(*Board, Sim->GetStateIndex(), Sim->GetRules());
		TestTrue(*FString::Printf(TEXT("%s: people 1 counts exactly its vector"), What), VectorsOf(Board->Civs[1].TimeVectors) == Expected);
		TestTrue(*FString::Printf(TEXT("%s: the control people 2 counts nothing"), What), VectorsOf(Board->Civs[2].TimeVectors) == None);
		TestEqual(*FString::Printf(TEXT("%s: the count takes no draw"), What), Sim->GetRng().GetCurrentSeed(), SeedBefore);
	};

	Case(TEXT("Nothing laid"), [](FGalaxyState&) {}, None, NoEdit);
	Case(TEXT("A project in a construction slot"), [](FGalaxyState& State) { State.Systems[2].ActiveMegaproject = CosmicTests::StandingProject(); },
		{ 1, 0, 0, 0, 0, 0 }, NoEdit);
	Case(TEXT("A project in a response slot"), [](FGalaxyState& State) { State.Systems[4].ResponseMegaproject = CosmicTests::StandingProject(); },
		{ 1, 0, 0, 0, 0, 0 }, NoEdit);
	Case(TEXT("A settlement (not built: drifted)"), [](FGalaxyState& State) { State.Systems[2].ActiveMegaproject = CosmicRows::ProjectSettle; },
		{ 0, 0, 1, 0, 0, 0 }, NoEdit);
	Case(TEXT("A fleet aimed at another people's system"), [](FGalaxyState& State) { State.Civs[1].FleetTargetSystemID = State.Systems[6].SystemID; },
		{ 0, 1, 0, 0, 0, 0 }, NoEdit);
	Case(TEXT("A fleet guarding its own system"), [](FGalaxyState& State) { State.Civs[1].FleetTargetSystemID = State.Systems[3].SystemID; },
		None, NoEdit);
	Case(TEXT("The starting edict held"), [](FGalaxyState& State) { State.Civs[1].ActiveEdicts = { Kept }; },
		{ 0, 1, 0, 0, 0, 0 }, [](FCosmicRulesDef& Rules) { Rules.StartingEdict = Kept; });
	Case(TEXT("Another edict held"), [](FGalaxyState& State) { State.Civs[1].ActiveEdicts = { Kept }; }, None, NoEdit);
	Case(TEXT("A friendly stance"), [](FGalaxyState& State) { State.Civs[1].Ties.Add(State.Civs[2].CivID).Stance = CosmicTags::StanceFriendly(); },
		{ 0, 0, 0, 1, 0, 0 }, NoEdit);
	Case(TEXT("A treaty"), [](FGalaxyState& State) { State.Civs[1].Ties.Add(State.Civs[3].CivID).Treaties.AddTag(CosmicTags::TreatyTrade()); },
		{ 0, 0, 0, 1, 0, 0 }, NoEdit);
	Case(TEXT("A hostile stance alone"), [](FGalaxyState& State) { State.Civs[1].Ties.Add(State.Civs[2].CivID).Stance = CosmicTags::StanceHostile(); },
		None, NoEdit);
	// One settled system left (the other two being settled - drifted too), no tie: isolated.
	Case(TEXT("One settled system and no tie"), [](FGalaxyState& State)
	{
		State.Systems[2].ActiveMegaproject = CosmicRows::ProjectSettle;
		State.Systems[4].ActiveMegaproject = CosmicRows::ProjectSettle;
	}, { 0, 0, 1, 0, 1, 0 }, NoEdit);
	Case(TEXT("One settled system and a hostile stance"), [](FGalaxyState& State)
	{
		State.Systems[2].ActiveMegaproject = CosmicRows::ProjectSettle;
		State.Systems[4].ActiveMegaproject = CosmicRows::ProjectSettle;
		State.Civs[1].Ties.Add(State.Civs[2].CivID).Stance = CosmicTags::StanceHostile();
	}, { 0, 0, 1, 0, 0, 0 }, NoEdit);
	Case(TEXT("An endured pressure"), [](FGalaxyState& State) { State.Civs[1].PressureEnduredTicks = 1; }, { 0, 0, 0, 0, 0, 1 }, NoEdit);
	Case(TEXT("Laws held"), [](FGalaxyState& State) { State.Civs[1].LawsHeldTicks = 1; }, { 0, 0, 0, 0, 0, 1 }, NoEdit);
	Case(TEXT("A hazard on a settled system"), [](FGalaxyState& State) { State.Systems[3].ActiveHazard = Tag(TEXT("Hazard.Plague")); },
		{ 0, 0, 0, 0, 0, 1 }, NoEdit);
	// An entry of OwnedSystems another people owns counts for nothing.
	Case(TEXT("A project on a system the people lists but does not own"), [](FGalaxyState& State)
	{
		State.Civs[1].OwnedSystems.Add(State.Systems[8].SystemID);
		State.Systems[8].ActiveMegaproject = CosmicTests::StandingProject();
	}, None, NoEdit);

	// Cumulative: never reset.
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		if (!TestNotNull(TEXT("The board is laid (cumulative)"), Board))
		{
			return false;
		}
		Board->Civs[1].Ties.Add(Board->Civs[2].CivID).Stance = CosmicTags::StanceFriendly();
		CosmicBricks::CountTimeVectors(*Board, Sim->GetStateIndex(), Sim->GetRules());
		CosmicBricks::CountTimeVectors(*Board, Sim->GetStateIndex(), Sim->GetRules());
		TestEqual(TEXT("Two ticks of a fact count two"), Board->Civs[1].TimeVectors.Linked, 2);
		Board->Civs[1].Ties.Reset();
		CosmicBricks::CountTimeVectors(*Board, Sim->GetStateIndex(), Sim->GetRules());
		TestEqual(TEXT("A fact that stops leaves its vector where it was (cumulative, never reset)"), Board->Civs[1].TimeVectors.Linked, 2);
		Board->Civs[1].TimeVectors.Endured = MAX_int32;
		Board->Civs[1].PressureEnduredTicks = 1;
		CosmicBricks::CountTimeVectors(*Board, Sim->GetStateIndex(), Sim->GetRules());
		TestEqual(TEXT("A vector never passes MAX_int32"), Board->Civs[1].TimeVectors.Endured, MAX_int32);
	}

	// The economy stage counts them, and the count draws nothing: a run whose people builds ends on the seed of a run whose
	// people does not, with the vectors of every people laid far off 0 in the second.
	{
		int32 Seeds[2] = { 0, 0 };
		for (int32 Pass = 0; Pass < 2; ++Pass)
		{
			FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
			if (!TestNotNull(TEXT("The board is laid (stage)"), Board))
			{
				return false;
			}
			if (Pass == 1)
			{
				Board->Systems[2].ActiveMegaproject = CosmicTests::StandingProject();
				for (FCivilizationState& Civ : Board->Civs)
				{
					SetVectors(Civ.TimeVectors, 1000);
				}
			}
			Sim->StepOnce();
			Seeds[Pass] = Sim->GetRng().GetCurrentSeed();
			if (Pass == 1)
			{
				TestEqual(TEXT("The economy stage counts a tick of building"), Board->Civs[1].TimeVectors.Built, 1001);
				TestEqual(TEXT("and not a fact that does not hold"), Board->Civs[1].TimeVectors.Linked, 1003);
			}
		}
		TestEqual(TEXT("The count draws nothing"), Seeds[1], Seeds[0]);
	}
	return true;
}

// The bricks' shares (header section 2.4): each wired share changes the tagged people alone, and under the inert knobs (the C++
// initializers - LayBoard's CosmicTests::WithInertBricks, whatever DT_Rules holds) nothing changes, whatever the people
// carries. The very small's damping spares the extinction and the supernova precursor (code 2 - mutation
// b7x2_inward_damps_extinction); the parasite's countdown bonus is pinned as the bound TheodicyProblems adds (code 3, hand-off
// test 3): once a hazard whatever the number of parasites, cut to 0..100000, 0 for a spared hazard (mutations
// b7x2_parasite_bonus_per_parasite and b7x2_parasite_bound_unclamped); the term of the drift target is added unclamped (code 4
// - mutation: the clamp back); the capital's star dims only on a settled capital the people owns (decision 8 - mutation: the
// owner and settle guards out). The functions first, then the stage: the energy and the research, the inert knobs (drift on and
// a plague standing, so the views and the countdowns could move - mutations b7x2_inert_brick_term_floor and
// b7x2_inert_parasite_bound_floor), the nomad's settlement through Advance (b7x2_settle_step_bare) and the very small's
// spawn share through the crisis stage (b7x2_spawn_share_bare).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimBrickSharesTest, "Cosmic_Ascension.Sim.BrickShares", CosmicTests::Flags)
bool FCosmicSimBrickSharesTest::RunTest(const FString& Parameters)
{
	using namespace CosmicExistenceBricksTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
	if (!TestNotNull(TEXT("The board is laid"), Board))
	{
		return false;
	}
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicRulesDef Inert = FCosmicRulesDef();
	FCosmicRulesDef On = Sim->GetRules();
	On.BrickDarkMatterEnergyPerTick = 2.f;
	On.BrickStarEaterEnergyMultiplier = 1.5f;
	On.BrickStarEaterRelationTarget = -0.4f;
	On.BrickIdleInsightPerTick = 3.f;
	On.BrickSpanEnergyPerLane = 0.5f;
	On.BrickParasiteCountdownBonusTicks = 40;
	On.BrickParasiteInsightPerHazard = 0.25f;
	On.BrickNomadSettleProgressMultiplier = 3.f;
	On.BrickInwardResearchMultiplier = 1.5f;
	On.BrickInwardHazardSpawnMultiplier = 0.5f;
	On.BrickSwarmResearchMultiplier = 2.f;
	On.BrickMergeRelationTarget = 0.3f;
	auto Income = [&](const FCosmicRulesDef& Rules, int32 CivIndex)
	{
		FResourcePool Pool;
		Pool.Energy = 10.f;
		Pool.Insight = 5.f;
		CosmicBricks::AddIncomeShares(*Board, Index, Rules, CivIndex, Pool);
		return Pool;
	};
	FCivilizationState& One = Board->Civs[1];

	// (1) The income. Without a brick: untouched under any knob.
	TestEqual(TEXT("Without a brick the income is untouched (Energy)"), Income(On, 1).Energy, 10.f);
	TestEqual(TEXT("Without a brick the income is untouched (Insight)"), Income(On, 1).Insight, 5.f);
	One.Traits.AddTag(CosmicTags::BrickDietDarkMatter());
	TestEqual(TEXT("Dark matter adds its flat Energy"), Income(On, 1).Energy, 12.f);
	TestEqual(TEXT("under the inert knob, nothing"), Income(Inert, 1).Energy, 10.f);
	TestEqual(TEXT("and only the tagged people takes it"), Income(On, 2).Energy, 10.f);
	One.Traits.AddTag(CosmicTags::BrickDietStarEater());
	TestEqual(TEXT("The star eater multiplies the Energy before the flat shares"), Income(On, 1).Energy, 17.f);
	One.Traits.Reset();
	// The span: people 1 holds 3, 2, 4 on the chain - two lanes from 2 to 4; people 3 holds 9, 8, 10, 11 - seven from 9 to 11.
	TestEqual(TEXT("The span of people 1 is two lanes"), CosmicBricks::SpanOf(*Board, Index, 1), 2);
	TestEqual(TEXT("The span of people 3 is seven lanes"), CosmicBricks::SpanOf(*Board, Index, 3), 7);
	One.Traits.AddTag(CosmicTags::BrickDietDistanceTension());
	TestEqual(TEXT("The distance tension adds the knob for each lane of its span"), Income(On, 1).Energy, 11.f);
	TestEqual(TEXT("under the inert knob, nothing"), Income(Inert, 1).Energy, 10.f);
	One.Traits.Reset();
	// The idle tick.
	One.Traits.AddTag(CosmicTags::BrickDietIdleFeed());
	TestTrue(TEXT("The board is idle for people 1"), CosmicBricks::IsIdleOf(*Board, Index, 1));
	TestEqual(TEXT("An idle tick feeds the idle feeder"), Income(On, 1).Insight, 8.f);
	TestEqual(TEXT("under the inert knob, nothing"), Income(Inert, 1).Insight, 5.f);
	Board->Systems[3].ActiveHazard = Tag(TEXT("Hazard.Plague"));
	TestFalse(TEXT("A hazard on its settled system ends the idle tick"), CosmicBricks::IsIdleOf(*Board, Index, 1));
	CosmicHazards::EmptyHazardSlot(Board->Systems[3]);
	Board->Systems[4].ActiveMegaproject = CosmicTests::StandingProject();
	TestFalse(TEXT("so does a project"), CosmicBricks::IsIdleOf(*Board, Index, 1));
	Board->Systems[4].ActiveMegaproject = NAME_None;
	Board->Civs[2].FleetTargetSystemID = Board->Systems[2].SystemID;
	TestFalse(TEXT("so does another people's fleet aimed at it"), CosmicBricks::IsIdleOf(*Board, Index, 1));
	Board->Civs[2].FleetTargetSystemID = NAME_None;
	One.FleetTargetSystemID = Board->Systems[6].SystemID;
	TestFalse(TEXT("so does its own siege"), CosmicBricks::IsIdleOf(*Board, Index, 1));
	One.FleetTargetSystemID = NAME_None;
	TestTrue(TEXT("(idle again)"), CosmicBricks::IsIdleOf(*Board, Index, 1));
	One.Traits.Reset();
	// The crisis parasite's Insight: a plague on a system of people 2 (it borders people 1) feeds it; the claim does not.
	One.Traits.AddTag(CosmicTags::BrickDietCrisisParasite());
	Board->Systems[6].ActiveHazard = Tag(TEXT("Hazard.Plague"));
	Board->Systems[8].ActiveHazard = CosmicTags::HazardClaim();
	TestEqual(TEXT("The parasite counts the standing hazards of its neighbours, the claim left out"), CosmicBricks::ParasiteHazardsOf(*Board, Index, 1), 1);
	TestEqual(TEXT("and takes the knob's Insight for each"), Income(On, 1).Insight, 5.25f);
	TestEqual(TEXT("under the inert knob, nothing"), Income(Inert, 1).Insight, 5.f);
	CosmicHazards::EmptyHazardSlot(Board->Systems[6]);
	CosmicHazards::EmptyHazardSlot(Board->Systems[8]);
	One.Traits.Reset();

	// (2) The parasite's countdown bonus: people 0 is the parasite; it borders people 1 alone.
	Board->Civs[0].Traits.AddTag(CosmicTags::BrickDietCrisisParasite());
	const FGameplayTag Plague = Tag(TEXT("Hazard.Plague"));
	TestEqual(TEXT("A new hazard next to the parasite takes the bonus"), CosmicBricks::ParasiteCountdownBonusOf(*Board, Index, On, 3, Plague), 40);
	TestEqual(TEXT("not on a people that does not border it"), CosmicBricks::ParasiteCountdownBonusOf(*Board, Index, On, 6, Plague), 0);
	TestEqual(TEXT("never for the claim"), CosmicBricks::ParasiteCountdownBonusOf(*Board, Index, On, 3, CosmicTags::HazardClaim()), 0);
	TestEqual(TEXT("nor the extinction"), CosmicBricks::ParasiteCountdownBonusOf(*Board, Index, On, 3, CosmicTags::HazardExtinction()), 0);
	TestEqual(TEXT("nor the supernova precursor"), CosmicBricks::ParasiteCountdownBonusOf(*Board, Index, On, 3, CosmicTags::HazardPreSupernova()), 0);
	TestEqual(TEXT("and under the inert knob, nothing"), CosmicBricks::ParasiteCountdownBonusOf(*Board, Index, Inert, 3, Plague), 0);
	// The bound (code 3 - what TheodicyProblems adds, hand-off test 3): once a hazard, whatever the number of parasites, cut to
	// 0..100000 and 0 for a spared hazard. Two parasites next door - people 0 and people 2 both border people 1, the owner of
	// system 3 - give the bonus once (mutation b7x2_parasite_bonus_per_parasite); the literal 40, not the bound, so a mutation
	// of both functions together still fails. People 2 keeps the brick through the placement below, so its 240 is once too.
	Board->Civs[2].Traits.AddTag(CosmicTags::BrickDietCrisisParasite());
	TestEqual(TEXT("Two parasites on its border: the bonus once"), CosmicBricks::ParasiteCountdownBonusOf(*Board, Index, On, 3, Plague), 40);
	TestEqual(TEXT("The bound is the knob"), CosmicBricks::ParasiteBonusBoundOf(On, Plague), 40);
	for (const FGameplayTag NoBound : { CosmicTags::HazardClaim(), CosmicTags::HazardExtinction(), CosmicTags::HazardPreSupernova(), FGameplayTag() })
	{
		TestEqual(*FString::Printf(TEXT("No bound for %s"), *NoBound.ToString()), CosmicBricks::ParasiteBonusBoundOf(On, NoBound), 0);
	}
	{
		// Mutation b7x2_parasite_bound_unclamped: the knob read without its upper cut.
		FCosmicRulesDef Wide = On;
		Wide.BrickParasiteCountdownBonusTicks = 200000;
		TestEqual(TEXT("The bound is cut to 100000"), CosmicBricks::ParasiteBonusBoundOf(Wide, Plague), 100000);
		Wide.BrickParasiteCountdownBonusTicks = -5;
		TestEqual(TEXT("and never below 0"), CosmicBricks::ParasiteBonusBoundOf(Wide, Plague), 0);
	}
	{
		// Through the one placement window.
		FRandomStream Stream(7);
		FCosmicEvalContext Ctx(*Board, Stream, Index, Sim->GetTables(), On);
		TestTrue(TEXT("The plague is placed next to the parasite"), CosmicHazards::Place(Ctx, 3, Sick));
		TestEqual(TEXT("with its countdown and the bonus"), Board->Systems[3].HazardTicksRemaining, 240);
		FCosmicEvalContext Plain(*Board, Stream, Index, Sim->GetTables(), Inert);
		TestTrue(TEXT("(and one away from it)"), CosmicHazards::Place(Plain, 6, Sick));
		TestEqual(TEXT("with its countdown alone"), Board->Systems[6].HazardTicksRemaining, 200);
		CosmicHazards::EmptyHazardSlot(Board->Systems[3]);
		CosmicHazards::EmptyHazardSlot(Board->Systems[6]);
	}
	Board->Civs[2].Traits.RemoveTag(CosmicTags::BrickDietCrisisParasite());
	Board->Civs[0].Traits.Reset();

	// (3) The factors of research, settlement and spawn: exactly the old value without the brick or under the inert knob.
	TestEqual(TEXT("No brick: research factor 1"), CosmicBricks::ResearchMultiplierOf(One, On), 1.f);
	One.Traits.AddTag(CosmicTags::BrickInstinctSwarmTech());
	TestEqual(TEXT("The swarm's research factor"), CosmicBricks::ResearchMultiplierOf(One, On), 2.f);
	One.Traits.AddTag(CosmicTags::BrickSpaceClosedPort());
	TestEqual(TEXT("with an inward brick, both factors"), CosmicBricks::ResearchMultiplierOf(One, On), 3.f);
	TestEqual(TEXT("under the inert knobs, 1"), CosmicBricks::ResearchMultiplierOf(One, Inert), 1.f);
	FCosmicRulesDef Zero = On;
	Zero.BrickSwarmResearchMultiplier = 0.f;
	Zero.BrickInwardResearchMultiplier = -1.f;
	TestEqual(TEXT("a knob not above 0 reads as 1"), CosmicBricks::ResearchMultiplierOf(One, Zero), 1.f);
	One.Traits.Reset();
	TestEqual(TEXT("No brick: the settlement's step itself"), CosmicBricks::SettleProgressOf(One, On, 0.75f), 0.75f);
	One.Traits.AddTag(CosmicTags::BrickSpaceNomad());
	TestEqual(TEXT("The nomad's settlement step"), CosmicBricks::SettleProgressOf(One, On, 0.75f), 2.25f);
	TestEqual(TEXT("under the inert knob, the step itself"), CosmicBricks::SettleProgressOf(One, Inert, 0.75f), 0.75f);
	One.Traits.Reset();
	TestEqual(TEXT("No brick: the spawn share itself"), CosmicBricks::HazardSpawnShareOf(One, On, Plague, 0.02f), 0.02f);
	One.Traits.AddTag(CosmicTags::BrickSpaceFoldedSmall());
	TestEqual(TEXT("The very small halves the spawn share"), CosmicBricks::HazardSpawnShareOf(One, On, Plague, 0.02f), 0.01f);
	TestEqual(TEXT("under the inert knob, the share itself"), CosmicBricks::HazardSpawnShareOf(One, Inert, Plague, 0.02f), 0.02f);
	// Never the hazards no brick touches (code 2, CosmicBricks::IsSparedHazard): the extinction and the supernova precursor of a
	// very small owner keep the share an owner without the brick has.
	{
		const FCivilizationState& Unbricked = Board->Civs[2];
		for (const FGameplayTag Spared : { CosmicTags::HazardExtinction(), CosmicTags::HazardPreSupernova() })
		{
			TestEqual(*FString::Printf(TEXT("The very small leaves the share of %s as an owner without the brick has it"), *Spared.ToString()),
				CosmicBricks::HazardSpawnShareOf(One, On, Spared, 0.02f), CosmicBricks::HazardSpawnShareOf(Unbricked, On, Spared, 0.02f));
		}
		TestEqual(TEXT("(the owner without the brick has the share itself)"), CosmicBricks::HazardSpawnShareOf(Unbricked, On, CosmicTags::HazardExtinction(), 0.02f), 0.02f);
	}
	Zero.BrickInwardHazardSpawnMultiplier = 0.f;
	TestEqual(TEXT("never 0: a knob of 0 reads as 1"), CosmicBricks::HazardSpawnShareOf(One, Zero, Plague, 0.02f), 0.02f);
	One.Traits.Reset();
	// The stellar crisis on the system of a people that lives off its star: the supernova precursor's share alone, once.
	{
		FCosmicRulesDef Stellar = On;
		Stellar.BrickStellarCrisisSpawnMultiplier = 3.f;
		const FGameplayTag Precursor = CosmicTags::HazardPreSupernova();
		TestEqual(TEXT("No brick: the precursor's share itself"), CosmicBricks::HazardSpawnShareOf(One, Stellar, Precursor, 0.02f), 0.02f);
		One.Traits.AddTag(CosmicTags::BrickFormShutSphere());
		TestEqual(TEXT("The shut sphere round its star triples the precursor's share"), CosmicBricks::HazardSpawnShareOf(One, Stellar, Precursor, 0.02f), 0.06f);
		TestEqual(TEXT("and no other hazard's"), CosmicBricks::HazardSpawnShareOf(One, Stellar, Plague, 0.02f), 0.02f);
		One.Traits.AddTag(CosmicTags::BrickDietStarEater());
		TestEqual(TEXT("with the star eater too, once"), CosmicBricks::HazardSpawnShareOf(One, Stellar, Precursor, 0.02f), 0.06f);
		One.Traits.RemoveTag(CosmicTags::BrickFormShutSphere());
		TestEqual(TEXT("the star eater alone, too"), CosmicBricks::HazardSpawnShareOf(One, Stellar, Precursor, 0.02f), 0.06f);
		TestEqual(TEXT("under the inert knob, the share itself"), CosmicBricks::HazardSpawnShareOf(One, Inert, Precursor, 0.02f), 0.02f);
		Stellar.BrickStellarCrisisSpawnMultiplier = 0.f;
		TestEqual(TEXT("never 0: a knob of 0 reads as 1"), CosmicBricks::HazardSpawnShareOf(One, Stellar, Precursor, 0.02f), 0.02f);
		One.Traits.Reset();
	}

	// (4) The term of the drift target: how the others see a star eater, a merging people (CosmicDiplomacy::TargetRelationOf).
	// The term is added as the stance term is - the drift stage clamps the relation, not the target (code 4).
	const float Before = CosmicDiplomacy::TargetRelationOf(*Board, On, 2, 1);
	const float InertBefore = CosmicDiplomacy::TargetRelationOf(*Board, Inert, 2, 1);
	const float Third = CosmicDiplomacy::TargetRelationOf(*Board, On, 2, 3);
	TestEqual(TEXT("No brick: no term"), CosmicBricks::RelationTargetTermOf(One, On), 0.f);
	One.Traits.AddTag(CosmicTags::BrickDietStarEater());
	TestEqual(TEXT("A star eater is seen worse"), CosmicBricks::RelationTargetTermOf(One, On), -0.4f);
	TestEqual(TEXT("in the drift target of a people that borders it, the term added"), CosmicDiplomacy::TargetRelationOf(*Board, On, 2, 1), Before - 0.4f, 1.e-6f);
	TestEqual(TEXT("under the inert knob, the target as it was"), CosmicDiplomacy::TargetRelationOf(*Board, Inert, 2, 1), InertBefore);
	TestEqual(TEXT("(the term is 0 under the inert knob)"), CosmicBricks::RelationTargetTermOf(One, Inert), 0.f);
	One.Traits.AddTag(CosmicTags::BrickInstinctHiveMerge());
	TestEqual(TEXT("with the merging brick too, the two terms add"), CosmicBricks::RelationTargetTermOf(One, On), -0.1f, 1.e-6f);
	FCosmicRulesDef Far = On;
	Far.BrickStarEaterRelationTarget = -5.f;
	TestEqual(TEXT("a term outside [-1, 1] reads as 0"), CosmicBricks::RelationTargetTermOf(One, Far), 0.3f, 1.e-6f);
	Far.BrickStarEaterRelationTarget = -1.f;
	Far.BrickMergeRelationTarget = 0.f;
	TestEqual(TEXT("a term of -1, the edge of its range, is added whole"), CosmicDiplomacy::TargetRelationOf(*Board, Far, 2, 1), Before - 1.f, 1.e-6f);
	TestEqual(TEXT("The view of another people is not touched"), CosmicDiplomacy::TargetRelationOf(*Board, On, 2, 3), Third);
	One.Traits.Reset();
	// No clamp on the target: a target already below -1 without the brick - an ethics distance of 2 and a hostile stance toward
	// the viewer - is lowered by exactly the star eater's term (mutation: the target clamped to [-1, 1] again).
	{
		FCivilizationState& Viewer = Board->Civs[2];
		const FEthicsAxes WasOne = One.Ethics;
		const FEthicsAxes WasViewer = Viewer.Ethics;
		One.Ethics.Pacifism = 1.f;
		One.Ethics.Spiritualism = 1.f;
		One.Ethics.Openness = 1.f;
		Viewer.Ethics.Pacifism = -1.f;
		Viewer.Ethics.Spiritualism = -1.f;
		Viewer.Ethics.Openness = -1.f;
		One.Ties.Add(Viewer.CivID).Stance = CosmicTags::StanceHostile();
		FCosmicRulesDef Deep = On;
		Deep.RelationEthicsWeight = 0.6f;
		Deep.RelationStanceTerms = { 0.4f, -0.4f };
		Deep.RelationTreatyTerm = 0.2f;
		Deep.BrickStarEaterRelationTarget = -0.4f;
		Deep.BrickMergeRelationTarget = 0.f;
		const float DeepTarget = CosmicDiplomacy::TargetRelationOf(*Board, Deep, 2, 1);
		TestTrue(TEXT("(without a brick the target is below -1: an ethics distance of 2 and a hostile stance)"), DeepTarget < -1.f);
		One.Traits.AddTag(CosmicTags::BrickDietStarEater());
		TestEqual(TEXT("A star eater lowers a target below -1 by exactly its term"), CosmicDiplomacy::TargetRelationOf(*Board, Deep, 2, 1), DeepTarget - 0.4f, 1.e-6f);
		One.Traits.Reset();
		One.Ties.Reset();
		One.Ethics = WasOne;
		Viewer.Ethics = WasViewer;
	}

	// (5) The star eater's dimming (R10 - implemented, not wired into a stage): along the seven classes, never past the dimmest.
	for (int32 At = 0; At + 1 < CosmicTags::StarClassCount; ++At)
	{
		TestTrue(*FString::Printf(TEXT("Class %d dims to class %d"), At, At + 1), CosmicBricks::DimmerStarClassOf(CosmicTags::StarClassAt(At)) == CosmicTags::StarClassAt(At + 1));
	}
	const FGameplayTag Dimmest = CosmicTags::StarClassAt(CosmicTags::StarClassCount - 1);
	TestTrue(TEXT("The dimmest stays the dimmest"), CosmicBricks::DimmerStarClassOf(Dimmest) == Dimmest);
	TestFalse(TEXT("A dead node's empty class stays empty"), CosmicBricks::DimmerStarClassOf(FGameplayTag()).IsValid());
	TestTrue(TEXT("A tag outside the seven comes back unchanged"), CosmicBricks::DimmerStarClassOf(Plague) == Plague);
	FCosmicRulesDef Dim = On;
	Dim.BrickStarEaterDimPeriodTicks = 100;
	TestFalse(TEXT("No star eater, no dimming"), CosmicBricks::StarDimDueOf(One, Dim, 200));
	One.Traits.AddTag(CosmicTags::BrickDietStarEater());
	TestTrue(TEXT("A star eater dims on its period"), CosmicBricks::StarDimDueOf(One, Dim, 200));
	TestFalse(TEXT("and not between"), CosmicBricks::StarDimDueOf(One, Dim, 250));
	TestFalse(TEXT("nor under the inert period 0"), CosmicBricks::StarDimDueOf(One, On, 200));
	Board->Systems[3].StarClass = CosmicTags::StarClassAt(4);
	TestTrue(TEXT("The capital's star dims"), CosmicBricks::DimCapitalStar(*Board, Index, 1));
	TestTrue(TEXT("one class"), Board->Systems[3].StarClass == CosmicTags::StarClassAt(5));
	// The three refusals of the one world function (decision 8: built, wired to no stage yet), each on a class that could dim,
	// after the class above is read (the capital goes back to the fifth class first).
	Board->Systems[3].StarClass = CosmicTags::StarClassAt(4);
	Board->Systems[3].ActiveMegaproject = CosmicRows::ProjectSettle;
	TestFalse(TEXT("not while the capital is being settled"), CosmicBricks::DimCapitalStar(*Board, Index, 1));
	Board->Systems[3].ActiveMegaproject = NAME_None;
	const FName OwnCapital = One.HomeSystemID;
	const FGameplayTag RivalClass = Board->Systems[6].StarClass;
	Board->Systems[6].StarClass = CosmicTags::StarClassAt(4);
	One.HomeSystemID = Board->Systems[6].SystemID;
	TestFalse(TEXT("not a capital another people owns"), CosmicBricks::DimCapitalStar(*Board, Index, 1));
	TestTrue(TEXT("(that people's star kept its class)"), Board->Systems[6].StarClass == CosmicTags::StarClassAt(4));
	Board->Systems[6].StarClass = RivalClass;
	One.HomeSystemID = OwnCapital;
	TestFalse(TEXT("not without a people"), CosmicBricks::DimCapitalStar(*Board, Index, INDEX_NONE));
	TestTrue(TEXT("(the class stayed)"), Board->Systems[3].StarClass == CosmicTags::StarClassAt(4));
	Board->Systems[3].StarClass = Dimmest;
	TestFalse(TEXT("and never past the dimmest"), CosmicBricks::DimCapitalStar(*Board, Index, 1));
	One.Traits.Reset();

	// (6) The stage: dark matter reaches IncomePerTick and the swarm's factor the research progress of the tagged people
	// alone; with every share brick on people 1 under the inert knobs, no number of the world moves.
	{
		float Energy[2] = { 0.f, 0.f };
		float OtherEnergy[2] = { 0.f, 0.f };
		float Physics[2] = { 0.f, 0.f };
		float OtherPhysics[2] = { 0.f, 0.f };
		for (int32 Pass = 0; Pass < 2; ++Pass)
		{
			FGalaxyState* Run = LayBoard(*Sim, Original, [](FCosmicRulesDef& Rules)
			{
				Rules.BrickDarkMatterEnergyPerTick = 2.f;
				Rules.BrickSwarmResearchMultiplier = 2.f;
			});
			if (!TestNotNull(TEXT("The board is laid (stage)"), Run))
			{
				return false;
			}
			if (Pass == 1)
			{
				Run->Civs[1].Traits.AddTag(CosmicTags::BrickDietDarkMatter());
				Run->Civs[1].Traits.AddTag(CosmicTags::BrickInstinctSwarmTech());
			}
			Sim->StepOnce();
			Energy[Pass] = Run->Civs[1].IncomePerTick.Energy;
			OtherEnergy[Pass] = Run->Civs[2].IncomePerTick.Energy;
			Physics[Pass] = Run->Civs[1].Research.FindRef(FName(TEXT("Physics")));
			OtherPhysics[Pass] = Run->Civs[2].Research.FindRef(FName(TEXT("Physics")));
		}
		TestEqual(TEXT("The economy stage adds dark matter's Energy to the tagged people"), Energy[1] - Energy[0], 2.f, 1.e-3f);
		TestEqual(TEXT("and to no other"), OtherEnergy[1], OtherEnergy[0]);
		TestTrue(TEXT("(the research stage spends Insight on the board)"), Physics[0] > 0.f);
		TestEqual(TEXT("The research stage doubles the swarm's progress"), Physics[1], 2.f * Physics[0], 1.e-6f);
		TestEqual(TEXT("and no other people's"), OtherPhysics[1], OtherPhysics[0]);
	}
	{
		TArray<FResourcePool> Stockpiles[2];
		TArray<float> Stabilities[2];
		TArray<float> Views[2];
		TArray<int32> Countdowns[2];
		int32 Seeds[2] = { 0, 0 };
		for (int32 Pass = 0; Pass < 2; ++Pass)
		{
			// The drift on (LayBoard's Edit comes after it zeroes the rate), so the views run through the drift target and its
			// brick term.
			FGalaxyState* Run = LayBoard(*Sim, Original, [](FCosmicRulesDef& Edit) { Edit.RelationDriftRate = 0.05f; });
			if (!TestNotNull(TEXT("The board is laid (inert)"), Run))
			{
				return false;
			}
			if (Pass == 1)
			{
				// Every brick whose reader is a share (the inward bricks refuse a settlement and the kept law keeps its edicts
				// whatever the knobs say: they are Sim.BrickSettleGate's and Sim.MutationStep's).
				for (const FGameplayTag Brick : { CosmicTags::BrickDietDarkMatter(), CosmicTags::BrickDietStarEater(), CosmicTags::BrickDietIdleFeed(),
						CosmicTags::BrickDietDistanceTension(), CosmicTags::BrickDietCrisisParasite(), CosmicTags::BrickSpaceNomad(),
						CosmicTags::BrickInstinctSwarmTech(), CosmicTags::BrickInstinctHiveMerge(), CosmicTags::BrickFormSociety(),
						CosmicTags::BrickFormLivingNebula(), CosmicTags::BrickFormShutSphere() })
				{
					Run->Civs[1].Traits.AddTag(Brick);
				}
			}
			// A plague on system 0 (people 0's - it borders people 1, the parasite of pass 1), placed after the bricks are laid
			// (the parasite's bonus is read at placement) through the board's own inert rules and a local stream (the sim
			// stream is not touched): the countdowns then hold a number the bricks could move.
			{
				FRandomStream Stream(5);
				FCosmicEvalContext Ctx(*Run, Stream, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
				TestTrue(TEXT("(the plague stands next to the bricks)"), CosmicHazards::Place(Ctx, 0, Sick));
			}
			for (int32 Tick = 0; Tick < 20; ++Tick)
			{
				Sim->StepOnce();
			}
			for (const FCivilizationState& Civ : Run->Civs)
			{
				Stockpiles[Pass].Add(Civ.Stockpile);
				Stabilities[Pass].Add(Civ.Stability);
				for (const FCivilizationState& Other : Run->Civs)
				{
					Views[Pass].Add(Civ.Relations.FindRef(Other.CivID));
				}
			}
			for (const FStarSystemState& System : Run->Systems)
			{
				Countdowns[Pass].Add(System.HazardTicksRemaining);
			}
			Seeds[Pass] = Sim->GetRng().GetCurrentSeed();
		}
		bool bSameStockpiles = Stockpiles[0].Num() == Stockpiles[1].Num();
		for (int32 At = 0; bSameStockpiles && At < Stockpiles[0].Num(); ++At)
		{
			const FResourcePool& A = Stockpiles[0][At];
			const FResourcePool& B = Stockpiles[1][At];
			bSameStockpiles = A.Energy == B.Energy && A.Matter == B.Matter && A.Insight == B.Insight && A.Influence == B.Influence && A.Causality == B.Causality;
		}
		TestTrue(TEXT("Under the inert knobs the bricks move no stockpile, bit for bit"), bSameStockpiles);
		TestTrue(TEXT("no Stability"), Stabilities[0] == Stabilities[1]);
		// The ethics are equal and no tie stands, so the drift target without a brick is 0 and the views of pass 0 stay 0; a brick
		// term that is not 0 would move pass 1 alone (the views of people 1 from 0, 2 and 3).
		TestTrue(TEXT("no view"), Views[0] == Views[1]);
		TestTrue(TEXT("no countdown"), Countdowns[0] == Countdowns[1]);
		// 200 less the 20 ticks: no expiry draw, and the crisis stage draws one number per settled system whether its slot is
		// full or not.
		TestTrue(TEXT("(a countdown stands to compare)"), Countdowns[0].IsValidIndex(0) && Countdowns[0][0] > 0);
		TestEqual(TEXT("and no draw"), Seeds[1], Seeds[0]);
	}

	// (7) The nomad's settlement through the construction slot (CosmicMegaprojects::Advance - mutation: the settle step back
	// to the bare rate). A fresh board (the blocks above left bricks on it) and a payable table: CostPerTick 0, so every tick is
	// paid, and a RequiredProgress no step reaches. People 1 is AI-run (the rate is the AI rate, 1) and the row is of the first
	// rung (no delay above the third).
	{
		using CosmicMegaprojects::ECosmicProjectSlot;
		FGalaxyState* Settling = LayBoard(*Sim, Original, NoEdit);
		if (!TestNotNull(TEXT("The board is laid (settlement)"), Settling))
		{
			return false;
		}
		FMegaprojectDef Free;
		Free.DisplayName = FText::AsCultureInvariant(TEXT("free"));
		Free.RequiredProgress = 1.0e9f;
		UDataTable* Projects = CosmicTests::NewTable(FMegaprojectDef::StaticStruct());
		Projects->AddRow(CosmicRows::ProjectSettle, Free);
		Projects->AddRow(CosmicTests::StandingProject(), Free);
		FCosmicDataTables Payable = Sim->GetTables();
		Payable.Megaprojects = Projects;
		FCivilizationState& Nomad = Settling->Civs[1];
		FStarSystemState& Camp = Settling->Systems[2];
		auto ProgressOf = [&](FName Row, const FCosmicRulesDef& Rules)
		{
			Camp.ActiveMegaproject = Row;
			Camp.MegaprojectProgress = 0.f;
			FRandomStream Stream(5);
			FCosmicEvalContext Ctx(*Settling, Stream, Sim->GetStateIndex(), Payable, Rules);
			CosmicMegaprojects::Advance(Ctx, 2, ECosmicProjectSlot::Construction, 1.f);
			return Camp.MegaprojectProgress;
		};
		TestFalse(TEXT("(people 1 is AI-run)"), Nomad.bIsPlayerRuled);
		TestEqual(TEXT("Advance: no brick, the settlement step itself"), ProgressOf(CosmicRows::ProjectSettle, On), 1.f);
		Nomad.Traits.AddTag(CosmicTags::BrickSpaceNomad());
		TestEqual(TEXT("Advance: the nomad's settlement takes its factor"), ProgressOf(CosmicRows::ProjectSettle, On), 3.f);
		TestEqual(TEXT("Advance: under the inert knob, the step itself"), ProgressOf(CosmicRows::ProjectSettle, Inert), 1.f);
		TestEqual(TEXT("Advance: another row of the nomad, the step itself"), ProgressOf(CosmicTests::StandingProject(), On), 1.f);
		Nomad.Traits.Reset();
		Camp.ActiveMegaproject = NAME_None;
		Camp.MegaprojectProgress = 0.f;
	}

	// (8) The very small's spawn share through the crisis stage (TickHazards - mutation: the share back to the bare product).
	// A Draw plague of share 1: every settled system without a hazard takes it in the first step. Pass 1 folds people 1 into
	// the very small at a knob of 1e-6 (both passes lay the knob, pass 0 no brick): its three systems stay clear, people 2's
	// take the plague in both passes, and the stage draws the same numbers (one per settled system, rule 3).
	{
		bool bOwnHeld[2] = { false, false };
		bool bOtherHeld[2] = { false, false };
		int32 SpawnSeeds[2] = { 0, 0 };
		for (int32 Pass = 0; Pass < 2; ++Pass)
		{
			FGalaxyState* Run = LayBoard(*Sim, Original, [](FCosmicRulesDef& Small) { Small.BrickInwardHazardSpawnMultiplier = 1.e-6f; });
			if (!TestNotNull(TEXT("The board is laid (spawn)"), Run))
			{
				return false;
			}
			UDataTable* Sure = CosmicTests::NewTable(FHazardDef::StaticStruct());
			FHazardDef SurePlague;
			SurePlague.HazardTag = Tag(TEXT("Hazard.Plague"));
			SurePlague.DisplayName = FText::AsCultureInvariant(TEXT("sick"));
			SurePlague.CountdownTicks = 200;
			SurePlague.SpawnRule = ECosmicHazardSpawn::Draw;
			SurePlague.SpawnChancePerTick = 1.f;
			Sure->AddRow(Sick, SurePlague);
			FCosmicDataTables WithSure = Sim->GetTables();
			WithSure.Hazards = Sure;
			Sim->SetTables(WithSure);
			if (Pass == 1)
			{
				Run->Civs[1].Traits.AddTag(CosmicTags::BrickSpaceFoldedSmall());
			}
			Sim->StepOnce();
			bOwnHeld[Pass] = Run->Systems[2].ActiveHazard.IsValid() || Run->Systems[3].ActiveHazard.IsValid() || Run->Systems[4].ActiveHazard.IsValid();
			bOtherHeld[Pass] = Run->Systems[5].ActiveHazard.IsValid() && Run->Systems[6].ActiveHazard.IsValid() && Run->Systems[7].ActiveHazard.IsValid();
			SpawnSeeds[Pass] = Sim->GetRng().GetCurrentSeed();
		}
		TestTrue(TEXT("Share 1: people 1's settled systems take the plague"), bOwnHeld[0]);
		TestFalse(TEXT("folded into the very small, its share is the knob's and they stay clear"), bOwnHeld[1]);
		TestTrue(TEXT("people 2's take it in both passes"), bOtherHeld[0] && bOtherHeld[1]);
		TestEqual(TEXT("and the stage draws the same numbers"), SpawnSeeds[1], SpawnSeeds[0]);
	}
	return true;
}

// The settle gate (header section 2.5): a people folded inward - Brick.Space.ClosedPort or Brick.Space.FoldedSmall - starts
// no further settlement. One reason appended at the end of ECosmicSettleBlock (Inward), answered by the one gate SettleBlockOf
// and by PeopleSettleBlockOf after Settling; SettleTargets is empty, the command refuses, the AI still draws its number.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimBrickSettleGateTest, "Cosmic_Ascension.Sim.BrickSettleGate", CosmicTests::Flags)
bool FCosmicSimBrickSettleGateTest::RunTest(const FString& Parameters)
{
	using namespace CosmicExistenceBricksTests;
	using CosmicMegaprojects::ECosmicSettleBlock;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	TestEqual(TEXT("Inward is the last reason, appended at the end"), static_cast<int32>(ECosmicSettleBlock::Inward), static_cast<int32>(ECosmicSettleBlock::Count) - 1);
	TestEqual(TEXT("twelve values before Count"), static_cast<int32>(ECosmicSettleBlock::Count), 12);

	// System 4 left wild: people 1 (3, 2) and people 2 (5) both have a foothold one lane away.
	auto Lay = [&]()
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		if (Board)
		{
			Board->Systems[4].OwnerCivID = NAME_None;
			Board->Civs[1].OwnedSystems.Remove(Board->Systems[4].SystemID);
		}
		return Board;
	};
	FGalaxyState* Board = Lay();
	if (!TestNotNull(TEXT("The board is laid"), Board))
	{
		return false;
	}
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicDataTables& Tables = Sim->GetTables();
	const FCosmicRulesDef& Rules = Sim->GetRules();
	TestTrue(TEXT("Without a brick people 1 may settle the wild system"), CosmicMegaprojects::SettleBlockOf(*Board, Index, Tables, Rules, 1, 4) == ECosmicSettleBlock::None);
	for (const FGameplayTag Inward : { CosmicTags::BrickSpaceClosedPort(), CosmicTags::BrickSpaceFoldedSmall() })
	{
		Board->Civs[1].Traits.Reset();
		Board->Civs[1].Traits.AddTag(Inward);
		const FString Name = Inward.ToString();
		TestTrue(*FString::Printf(TEXT("%s: the gate gives Inward"), *Name), CosmicMegaprojects::SettleBlockOf(*Board, Index, Tables, Rules, 1, 4) == ECosmicSettleBlock::Inward);
		TestTrue(*FString::Printf(TEXT("%s: so does the people's reason"), *Name), CosmicMegaprojects::PeopleSettleBlockOf(*Board, Index, Tables, Rules, 1) == ECosmicSettleBlock::Inward);
		TestTrue(*FString::Printf(TEXT("%s: no target"), *Name), CosmicMegaprojects::SettleTargets(*Board, Index, Tables, Rules, 1).IsEmpty());
		TestTrue(*FString::Printf(TEXT("%s: MaySettle as before (the AI's draw is made)"), *Name), CosmicMegaprojects::MaySettle(*Board, Index, Rules, 1));
		TestTrue(*FString::Printf(TEXT("%s: another people may still settle there"), *Name), CosmicMegaprojects::SettleBlockOf(*Board, Index, Tables, Rules, 2, 4) == ECosmicSettleBlock::None);
		TestTrue(*FString::Printf(TEXT("%s: an owned system gives Owned first"), *Name), CosmicMegaprojects::SettleBlockOf(*Board, Index, Tables, Rules, 1, 6) == ECosmicSettleBlock::Owned);
		{
			FRandomStream Stream(3);
			FCosmicEvalContext Ctx(*Board, Stream, Index, Tables, Rules);
			TestFalse(*FString::Printf(TEXT("%s: the command refuses"), *Name), CosmicMegaprojects::Settle(Ctx, 1, 4));
			TestTrue(*FString::Printf(TEXT("%s: and the system stays wild"), *Name), Board->Systems[4].OwnerCivID.IsNone());
		}
	}
	// A settlement under way is named before the brick.
	Board->Systems[2].ActiveMegaproject = CosmicRows::ProjectSettle;
	TestTrue(TEXT("A settlement under way comes before Inward"), CosmicMegaprojects::SettleBlockOf(*Board, Index, Tables, Rules, 1, 4) == ECosmicSettleBlock::Settling);
	TestTrue(TEXT("(for the people's reason too)"), CosmicMegaprojects::PeopleSettleBlockOf(*Board, Index, Tables, Rules, 1) == ECosmicSettleBlock::Settling);

	// The AI's settlement draw is made whether the people is folded inward or not: the two runs end on one seed, and only the
	// people without the brick settles.
	int32 Seeds[2] = { 0, 0 };
	bool bSettled[2] = { false, false };
	for (int32 Pass = 0; Pass < 2; ++Pass)
	{
		FGalaxyState* Run = Lay();
		if (!TestNotNull(TEXT("The board is laid (draw)"), Run))
		{
			return false;
		}
		// People 2 may not settle the wild system this tick: a settlement of its own is under way (MaySettle - it draws
		// nothing). People 3 (10 lies next to 4) is at its realm cap. People 1 is the only one that could.
		Run->Systems[7].ActiveMegaproject = CosmicRows::ProjectSettle;
		if (Pass == 1)
		{
			Run->Civs[1].Traits.AddTag(CosmicTags::BrickSpaceFoldedSmall());
		}
		Sim->StepOnce();
		Seeds[Pass] = Sim->GetRng().GetCurrentSeed();
		bSettled[Pass] = !Run->Systems[4].OwnerCivID.IsNone();
	}
	TestTrue(TEXT("Without the brick the AI settles the wild system"), bSettled[0]);
	TestFalse(TEXT("folded inward it does not"), bSettled[1]);
	TestEqual(TEXT("and the draw is the same"), Seeds[1], Seeds[0]);
	return true;
}

// The AI mutation step (header section 2.6) and the kept law's edicts: MutationDueOf names the first due mutation in the
// fixed order StarTow, DustBand, InwardHunger from the prerequisite brick and the vectors (threshold 0 = never);
// MutateAIPeoples adds it to the peoples the player does not rule; the stage runs it on its period (0 = off) without a draw.
// A people that carries Brick.Form.KeptLaw keeps an edict that neither adds to the claim's countdown nor answers a crisis
// (CosmicEdicts::AIRevokesOf - decision 18's two exceptions are Sim.KeptLawEdicts').
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimMutationStepTest, "Cosmic_Ascension.Sim.MutationStep", CosmicTests::Flags)
bool FCosmicSimMutationStepTest::RunTest(const FString& Parameters)
{
	using namespace CosmicExistenceBricksTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	FCosmicRulesDef Rules;
	Rules.BrickMutationThresholdTicks = 100;
	FCivilizationState Civ;
	TestFalse(TEXT("No prerequisite brick: no mutation"), CosmicBricks::MutationDueOf(Civ, Rules).IsValid());
	Civ.Traits.AddTag(CosmicTags::BrickSpaceNomad());
	Civ.Traits.AddTag(CosmicTags::BrickInstinctSwarmTech());
	Civ.Traits.AddTag(CosmicTags::BrickDietStarEater());
	TestFalse(TEXT("Vectors below the threshold: none"), CosmicBricks::MutationDueOf(Civ, Rules).IsValid());
	SetVectors(Civ.TimeVectors, 100);
	TestTrue(TEXT("All due: the star tow first"), CosmicBricks::MutationDueOf(Civ, Rules) == CosmicTags::BrickMutationStarTow());
	Civ.Traits.AddTag(CosmicTags::BrickMutationStarTow());
	TestTrue(TEXT("then the dust band"), CosmicBricks::MutationDueOf(Civ, Rules) == CosmicTags::BrickMutationDustBand());
	Civ.Traits.AddTag(CosmicTags::BrickMutationDustBand());
	TestTrue(TEXT("then the inward hunger"), CosmicBricks::MutationDueOf(Civ, Rules) == CosmicTags::BrickMutationInwardHunger());
	Civ.Traits.AddTag(CosmicTags::BrickMutationInwardHunger());
	TestFalse(TEXT("then none: each is due once"), CosmicBricks::MutationDueOf(Civ, Rules).IsValid());
	{
		FCivilizationState Swarm;
		Swarm.Traits.AddTag(CosmicTags::BrickInstinctSwarmTech());
		Swarm.TimeVectors.Built = 100;
		TestFalse(TEXT("The dust band needs Built and Linked both"), CosmicBricks::MutationDueOf(Swarm, Rules).IsValid());
		Swarm.TimeVectors.Linked = 100;
		TestTrue(TEXT("(with both it is due)"), CosmicBricks::MutationDueOf(Swarm, Rules) == CosmicTags::BrickMutationDustBand());
		TestFalse(TEXT("Under the inert threshold 0 no mutation is ever due"), CosmicBricks::MutationDueOf(Swarm, FCosmicRulesDef()).IsValid());
	}
	FCosmicRulesDef Period;
	Period.BrickEvolutionPeriodTicks = 50;
	TestTrue(TEXT("The step runs on its period"), CosmicBricks::MutationStepDueAt(Period, 100));
	TestFalse(TEXT("not between"), CosmicBricks::MutationStepDueAt(Period, 120));
	TestFalse(TEXT("not at tick 0"), CosmicBricks::MutationStepDueAt(Period, 0));
	TestFalse(TEXT("never under the inert period 0"), CosmicBricks::MutationStepDueAt(FCosmicRulesDef(), 100));

	// The step on the board: people 0 is the player's and keeps its bricks as they are.
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		if (!TestNotNull(TEXT("The board is laid"), Board))
		{
			return false;
		}
		TestTrue(TEXT("(people 0 is the player's)"), Board->Civs[0].bIsPlayerRuled);
		for (const int32 CivIndex : { 0, 1 })
		{
			Board->Civs[CivIndex].Traits.AddTag(CosmicTags::BrickSpaceNomad());
			Board->Civs[CivIndex].TimeVectors.Drifted = 100;
		}
		TestEqual(TEXT("One AI people takes a mutation"), CosmicBricks::MutateAIPeoples(*Board, Rules), 1);
		TestTrue(TEXT("people 1 carries the star tow"), Board->Civs[1].Traits.HasTagExact(CosmicTags::BrickMutationStarTow()));
		TestFalse(TEXT("the player's people changes only through cards"), Board->Civs[0].Traits.HasTagExact(CosmicTags::BrickMutationStarTow()));
	}
	// The stage: on its period, before the feats; no draw.
	{
		int32 Seeds[2] = { 0, 0 };
		for (int32 Pass = 0; Pass < 2; ++Pass)
		{
			FGalaxyState* Run = LayBoard(*Sim, Original, [Pass](FCosmicRulesDef& Edit)
			{
				Edit.BrickMutationThresholdTicks = 100;
				Edit.BrickEvolutionPeriodTicks = Pass == 1 ? 1 : 0;
			});
			if (!TestNotNull(TEXT("The board is laid (stage)"), Run))
			{
				return false;
			}
			Run->Civs[2].Traits.AddTag(CosmicTags::BrickDietStarEater());
			Run->Civs[2].TimeVectors.Consumed = 100;
			Sim->StepOnce();
			Seeds[Pass] = Sim->GetRng().GetCurrentSeed();
			TestTrue(*FString::Printf(TEXT("Pass %d: the inward hunger comes exactly when the step runs"), Pass),
				Run->Civs[2].Traits.HasTagExact(CosmicTags::BrickMutationInwardHunger()) == (Pass == 1));
		}
		TestEqual(TEXT("The step draws nothing"), Seeds[1], Seeds[0]);
	}

	// The kept law keeps an edict of this file's board (no claim hook, no crisis's answer, a row that asks for no laws) when the
	// AI runs the people.
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		if (!TestNotNull(TEXT("The board is laid (law)"), Board))
		{
			return false;
		}
		const FCosmicDataTables& Tables = Sim->GetTables();
		Board->Civs[1].ActiveEdicts = { Kept };
		TestTrue(TEXT("Without the brick the AI revokes a people's edict it holds"), CosmicEdicts::AIRevokesOf(Board->Civs[1], Tables, Sim->GetRules()) == TArray<FName>({ Kept }));
		Board->Civs[1].Traits.AddTag(CosmicTags::BrickFormKeptLaw());
		TestTrue(TEXT("A kept-law people revokes no edict that neither touches the claim nor answers a crisis"), CosmicEdicts::AIRevokesOf(Board->Civs[1], Tables, Sim->GetRules()).IsEmpty());
		Board->Civs[2].ActiveEdicts = { Kept };
		Sim->StepOnce();
		TestTrue(TEXT("and keeps its edict through the AI stage"), Board->Civs[1].ActiveEdicts.Contains(Kept));
		TestFalse(TEXT("while a people without it loses its own"), Board->Civs[2].ActiveEdicts.Contains(Kept));
	}
	return true;
}

// The kept law's two exceptions (decision 18, header section 2.4 - CosmicEdicts::AIRevokesOf), on the game's edict and hazard
// rows: (a) a people that carries Brick.Form.KeptLaw still revokes, as any AI does, an edict that adds to the claim's countdown
// (edict_vigil) or answers a crisis (edict_machine_rights - the awakening's answer), and keeps one that does neither
// (edict_mobilization) - mutation b7x2_kept_law_keeps_claim_edicts; (b) while the row its rung is judged by asks for galaxy
// laws (CosmicEdicts::AsksForLawsOf) it revokes its people's edicts as any AI does, so the law step finds the three slots and
// the people holds the three laws (mutation b7x2_kept_law_keeps_slots: mobilization stays in a slot and the step stops at two,
// as the control below shows for a people that keeps it by the rule every AI uses).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimKeptLawEdictsTest, "Cosmic_Ascension.Sim.KeptLawEdicts", CosmicTests::Flags)
bool FCosmicSimKeptLawEdictsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicExistenceBricksTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	// The game's rows (no code names them by a constant - the names of Tests/CosmicEdictUITests.cpp) and the facts each branch
	// stands on, read from the game's tables: the vigil adds to the claim's countdown, machine rights answers a crisis,
	// mobilization does neither and costs Stability; all three of the people's scope.
	const FName Vigil(TEXT("edict_vigil"));
	const FName Rights(TEXT("edict_machine_rights"));
	const FName Mobilization(TEXT("edict_mobilization"));
	const FEdictDef* VigilRow = Original.FindEdict(Vigil);
	const FEdictDef* RightsRow = Original.FindEdict(Rights);
	const FEdictDef* MobilizationRow = Original.FindEdict(Mobilization);
	if (!TestTrue(TEXT("(the game's three edicts are loaded)"), VigilRow && RightsRow && MobilizationRow))
	{
		return false;
	}
	const FGameplayTag Claim = CosmicTags::HazardClaim();
	const bool bFacts = VigilRow->HazardCountdownBonus.Contains(Claim) && !Original.IsHazardAnswer(Vigil)
		&& !RightsRow->HazardCountdownBonus.Contains(Claim) && Original.IsHazardAnswer(Rights)
		&& !MobilizationRow->HazardCountdownBonus.Contains(Claim) && !Original.IsHazardAnswer(Mobilization) && CosmicEdicts::HasStabilityCost(*MobilizationRow)
		&& VigilRow->Scope == ECosmicEdictScope::People && RightsRow->Scope == ECosmicEdictScope::People && MobilizationRow->Scope == ECosmicEdictScope::People;
	if (!TestTrue(TEXT("(the vigil adds to the claim's countdown, machine rights answers a crisis, mobilization does neither and costs Stability)"), bFacts))
	{
		return false;
	}

	// (a) Below AIEdictRevokeStability every AI revokes all three; a kept-law people the two the narrowing hands back.
	{
		FCosmicRulesDef Revoking;
		Revoking.AIEdictRevokeStability = 1.f;
		FCivilizationState Holder;
		Holder.Tier = EAscensionTier::Tier1_Planetary;
		Holder.Stability = 0.5f;
		Holder.ActiveEdicts = { Vigil, Rights, Mobilization };
		TestTrue(TEXT("Without the brick the AI revokes all three (mobilization below AIEdictRevokeStability)"),
			CosmicEdicts::AIRevokesOf(Holder, Original, Revoking) == TArray<FName>({ Vigil, Rights, Mobilization }));
		Holder.Traits.AddTag(CosmicTags::BrickFormKeptLaw());
		const TArray<FName> Revoked = CosmicEdicts::AIRevokesOf(Holder, Original, Revoking);
		TestTrue(TEXT("A kept-law people revokes the vigil: it adds to the claim's countdown"), Revoked.Contains(Vigil));
		TestTrue(TEXT("and machine rights: it answers a crisis"), Revoked.Contains(Rights));
		TestFalse(TEXT("and keeps mobilization, which does neither"), Revoked.Contains(Mobilization));
		TestTrue(TEXT("(in the order it holds them)"), Revoked == TArray<FName>({ Vigil, Rights }));
	}

	// (b) LayBoard's board (three slots - Society 1, the marks set here; 1000 Influence) with the game's edicts and one feat row
	// of the fourth rung that reads the held laws, people 1 on the fourth rung at Stability 0.75 holding mobilization.
	UDataTable* Feats = CosmicTests::NewTable(FAscensionFeatDef::StaticStruct());
	{
		FAscensionFeatDef LawRow;
		LawRow.Title = FText::AsCultureInvariant(TEXT("laws"));
		LawRow.RequiredTier = EAscensionTier::Tier4_Transcendent;
		LawRow.TargetTier = EAscensionTier::Tier5_EternalOrder;
		LawRow.Path = EPlayerRole::Ruler;
		LawRow.CompletionConditions = { Cond(EConditionType::CivLawsHeldTicksAtLeast, nullptr, TEXT("600")) };
		Feats->AddRow(FName(TEXT("feat_test_laws")), LawRow);
	}
	auto LayLaws = [&](bool bKeptLaw, float RevokeBelow) -> FGalaxyState*
	{
		FGalaxyState* Laid = LayBoard(*Sim, Original, [RevokeBelow](FCosmicRulesDef& Edit)
		{
			Edit.EdictSlotSocietyMarks = { 0.5f, 1.f };
			Edit.GalaxyLawsToHold = 3;
			Edit.AIEdictRevokeStability = RevokeBelow;
		});
		if (!Laid)
		{
			return nullptr;
		}
		FCosmicDataTables WithLaws = Sim->GetTables();
		WithLaws.Edicts = Original.Edicts;
		WithLaws.Feats = Feats;
		Sim->SetTables(WithLaws);
		FCivilizationState& Asker = Laid->Civs[1];
		Asker.Tier = EAscensionTier::Tier4_Transcendent;
		Asker.ActiveEdicts = { Mobilization };
		if (bKeptLaw)
		{
			Asker.Traits.AddTag(CosmicTags::BrickFormKeptLaw());
		}
		return Laid;
	};
	const TArray<FName> GameLaws = CosmicEdicts::GalaxyLawRowsOf(Original);
	if (!TestTrue(TEXT("(the game holds the three galaxy laws)"), GameLaws.Num() == 3 && GameLaws.Contains(CosmicB6bRows::EdictWideMachineAccord)
		&& GameLaws.Contains(CosmicB6bRows::EdictWideQuarantine) && GameLaws.Contains(CosmicB6bRows::EdictWideWarning)))
	{
		return false;
	}
	// The facts branch (b) stands on, read from the game's rows: each law asks no condition and no rung above the fourth, and
	// LayBoard's stockpile (1000 Energy, Matter, Insight and Influence, 0 Causality) pays all three EnactCosts and then each
	// law's UpkeepPerTick for the two ticks the case steps. A data change that breaks one fails here, by its own message, and
	// not at the law step below.
	bool bLawFacts = true;
	FResourcePool LaidStock;
	LaidStock.Energy = 1000.f;
	LaidStock.Matter = 1000.f;
	LaidStock.Insight = 1000.f;
	LaidStock.Influence = 1000.f;
	LaidStock.Causality = 0.f;
	for (const FName Law : GameLaws)
	{
		const FEdictDef* LawRow = Original.FindEdict(Law);
		bLawFacts = bLawFacts && LawRow && LawRow->EnactConditions.IsEmpty()
			&& static_cast<uint8>(LawRow->RequiredTier) <= static_cast<uint8>(EAscensionTier::Tier4_Transcendent)
			&& CosmicEdicts::TryPay(LaidStock, LawRow->EnactCost);
	}
	for (const FName Law : GameLaws)
	{
		const FEdictDef* LawRow = Original.FindEdict(Law);
		bLawFacts = bLawFacts && LawRow && CosmicEdicts::TryPay(LaidStock, LawRow->UpkeepPerTick) && CosmicEdicts::TryPay(LaidStock, LawRow->UpkeepPerTick);
	}
	if (!TestTrue(TEXT("(each galaxy law asks no condition and no rung above the fourth, and the laid stockpile pays its three costs and two ticks of upkeep)"), bLawFacts))
	{
		return false;
	}

	// The control: a people that keeps mobilization in a slot by the rule every AI uses (AIEdictRevokeStability 0 - Stability
	// is not below it) takes two laws and holds none for the count: the slot the narrowing frees.
	{
		FGalaxyState* Control = LayLaws(false, 0.f);
		if (!TestNotNull(TEXT("The board is laid (control)"), Control))
		{
			return false;
		}
		TestTrue(TEXT("(control: the row people 1 is judged by asks for laws)"), CosmicEdicts::AsksForLawsOf(Control->Civs[1], Sim->GetTables()));
		TestEqual(TEXT("(control: three slots)"), CosmicEdicts::SlotsOf(Control->Civs[1], Sim->GetRules()), 3);
		Sim->StepOnce();
		TestTrue(TEXT("Control: a people that keeps mobilization"), Control->Civs[1].ActiveEdicts.Contains(Mobilization));
		TestEqual(TEXT("Control: the law step stops at two laws"), CosmicEdicts::GalaxyLawCountOf(Control->Civs[1], Sim->GetTables()), 2);
		TestFalse(TEXT("Control: and does not hold the laws"), CosmicEdicts::HoldsGalaxyLawsOf(Control->Civs[1], Sim->GetTables(), Sim->GetRules()));
		Sim->StepOnce();
		TestEqual(TEXT("Control: LawsHeldTicks stays 0"), Control->Civs[1].LawsHeldTicks, 0);
	}

	// The kept law under the row that asks: mobilization goes, the three laws come.
	{
		FGalaxyState* KeptBoard = LayLaws(true, 1.f);
		if (!TestNotNull(TEXT("The board is laid (kept law)"), KeptBoard))
		{
			return false;
		}
		FCivilizationState& Asker = KeptBoard->Civs[1];
		TestTrue(TEXT("(the row people 1 is judged by asks for laws)"), CosmicEdicts::AsksForLawsOf(Asker, Sim->GetTables()));
		TestTrue(TEXT("While its row asks for laws a kept-law people revokes its people's edict as any AI does"),
			CosmicEdicts::AIRevokesOf(Asker, Sim->GetTables(), Sim->GetRules()) == TArray<FName>({ Mobilization }));
		Asker.Tier = EAscensionTier::Tier3_Causal;
		TestTrue(TEXT("(on the third rung no row asks: it keeps mobilization)"), CosmicEdicts::AIRevokesOf(Asker, Sim->GetTables(), Sim->GetRules()).IsEmpty());
		Asker.Tier = EAscensionTier::Tier4_Transcendent;
		Sim->StepOnce();
		TestFalse(TEXT("Through the AI stage it gives up mobilization"), Asker.ActiveEdicts.Contains(Mobilization));
		TestEqual(TEXT("and the law step takes the three galaxy laws"), CosmicEdicts::GalaxyLawCountOf(Asker, Sim->GetTables()), 3);
		TestTrue(TEXT("(the game's three)"), Asker.ActiveEdicts.Contains(CosmicB6bRows::EdictWideMachineAccord) && Asker.ActiveEdicts.Contains(CosmicB6bRows::EdictWideQuarantine)
			&& Asker.ActiveEdicts.Contains(CosmicB6bRows::EdictWideWarning));
		TestTrue(TEXT("so it holds the laws"), CosmicEdicts::HoldsGalaxyLawsOf(Asker, Sim->GetTables(), Sim->GetRules()));
		Sim->StepOnce();
		TestEqual(TEXT("and the economy stage counts them the next tick"), Asker.LawsHeldTicks, 1);
		TestEqual(TEXT("(the laws stay: a kept-law people revokes no galaxy law)"), CosmicEdicts::GalaxyLawCountOf(Asker, Sim->GetTables()), 3);
	}
	return true;
}

// The end shape (header section 2.7): the eight shape bricks in their priority, then the largest vector with the ties broken by
// the fixed order Built, Consumed, Drifted, Linked, Isolated, Endured - all twelve answers.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimShapeOfTest, "Cosmic_Ascension.Sim.ShapeOf", CosmicTests::Flags)
bool FCosmicSimShapeOfTest::RunTest(const FString& Parameters)
{
	using namespace CosmicExistenceBricksTests;
	using CosmicBricks::ECosmicShape;
	TestEqual(TEXT("Twelve shapes"), static_cast<int32>(ECosmicShape::Count), 12);
	auto ShapeWith = [](std::initializer_list<FGameplayTag> Tags, TFunctionRef<void(FCosmicTimeVectors&)> Vectors)
	{
		FCivilizationState Civ;
		for (const FGameplayTag& Each : Tags)
		{
			Civ.Traits.AddTag(Each);
		}
		Vectors(Civ.TimeVectors);
		return CosmicBricks::ShapeOf(Civ);
	};
	auto Zero = [](FCosmicTimeVectors&) {};
	TestTrue(TEXT("All vectors 0: Built wins the tie"), ShapeWith({}, Zero) == ECosmicShape::BuiltOrLinked);
	TestTrue(TEXT("Largest Consumed"), ShapeWith({}, [](FCosmicTimeVectors& V) { V.Consumed = 5; }) == ECosmicShape::ConsumedOrDrifted);
	TestTrue(TEXT("Largest Drifted"), ShapeWith({}, [](FCosmicTimeVectors& V) { V.Drifted = 5; }) == ECosmicShape::ConsumedOrDrifted);
	TestTrue(TEXT("Largest Linked"), ShapeWith({}, [](FCosmicTimeVectors& V) { V.Linked = 5; }) == ECosmicShape::BuiltOrLinked);
	TestTrue(TEXT("Largest Isolated"), ShapeWith({}, [](FCosmicTimeVectors& V) { V.Isolated = 5; }) == ECosmicShape::Isolated);
	TestTrue(TEXT("Largest Endured"), ShapeWith({}, [](FCosmicTimeVectors& V) { V.Endured = 5; }) == ECosmicShape::Endured);
	TestTrue(TEXT("A tie of Isolated and Endured goes to Isolated, the earlier"), ShapeWith({}, [](FCosmicTimeVectors& V) { V.Isolated = 5; V.Endured = 5; }) == ECosmicShape::Isolated);
	TestTrue(TEXT("A tie of Drifted and Isolated goes to Drifted"), ShapeWith({}, [](FCosmicTimeVectors& V) { V.Drifted = 9; V.Isolated = 9; }) == ECosmicShape::ConsumedOrDrifted);
	TestTrue(TEXT("A tie of Built and Endured goes to Built"), ShapeWith({}, [](FCosmicTimeVectors& V) { V.Built = 3; V.Endured = 3; }) == ECosmicShape::BuiltOrLinked);
	// The shape bricks, each alone.
	TestTrue(TEXT("The kept law"), ShapeWith({ CosmicTags::BrickFormKeptLaw() }, Zero) == ECosmicShape::KeptLaw);
	TestTrue(TEXT("The very small"), ShapeWith({ CosmicTags::BrickSpaceFoldedSmall() }, Zero) == ECosmicShape::FoldedSmall);
	TestTrue(TEXT("The closed port"), ShapeWith({ CosmicTags::BrickSpaceClosedPort() }, Zero) == ECosmicShape::ShutHouse);
	TestTrue(TEXT("The shut sphere round its star"), ShapeWith({ CosmicTags::BrickFormShutSphere() }, Zero) == ECosmicShape::ShutHouse);
	TestTrue(TEXT("The living nebula"), ShapeWith({ CosmicTags::BrickFormLivingNebula() }, Zero) == ECosmicShape::SpreadBody);
	TestTrue(TEXT("The dark matter feeder spreads its body too"), ShapeWith({ CosmicTags::BrickDietDarkMatter() }, Zero) == ECosmicShape::SpreadBody);
	TestTrue(TEXT("(with the living nebula, the same answer)"), ShapeWith({ CosmicTags::BrickDietDarkMatter(), CosmicTags::BrickFormLivingNebula() }, Zero) == ECosmicShape::SpreadBody);
	TestTrue(TEXT("The nomad"), ShapeWith({ CosmicTags::BrickSpaceNomad() }, Zero) == ECosmicShape::StarHerd);
	TestTrue(TEXT("The swarm"), ShapeWith({ CosmicTags::BrickInstinctSwarmTech() }, Zero) == ECosmicShape::WovenSwarm);
	TestTrue(TEXT("The star eater"), ShapeWith({ CosmicTags::BrickDietStarEater() }, Zero) == ECosmicShape::LastHunger);
	TestTrue(TEXT("The merging people"), ShapeWith({ CosmicTags::BrickInstinctHiveMerge() }, Zero) == ECosmicShape::Entwined);
	// The priority, pairwise along it; a brick that names no shape and a large vector change nothing.
	auto Endless = [](FCosmicTimeVectors& V) { V.Endured = 100000; };
	TestTrue(TEXT("KeptLaw before FoldedSmall"), ShapeWith({ CosmicTags::BrickSpaceFoldedSmall(), CosmicTags::BrickFormKeptLaw() }, Endless) == ECosmicShape::KeptLaw);
	TestTrue(TEXT("FoldedSmall before ShutHouse"), ShapeWith({ CosmicTags::BrickSpaceClosedPort(), CosmicTags::BrickSpaceFoldedSmall() }, Endless) == ECosmicShape::FoldedSmall);
	TestTrue(TEXT("ShutHouse before SpreadBody"), ShapeWith({ CosmicTags::BrickFormLivingNebula(), CosmicTags::BrickFormShutSphere() }, Endless) == ECosmicShape::ShutHouse);
	TestTrue(TEXT("SpreadBody before StarHerd"), ShapeWith({ CosmicTags::BrickSpaceNomad(), CosmicTags::BrickFormLivingNebula() }, Endless) == ECosmicShape::SpreadBody);
	TestTrue(TEXT("ShutHouse before SpreadBody by dark matter"), ShapeWith({ CosmicTags::BrickDietDarkMatter(), CosmicTags::BrickSpaceClosedPort() }, Endless) == ECosmicShape::ShutHouse);
	TestTrue(TEXT("SpreadBody by dark matter before StarHerd"), ShapeWith({ CosmicTags::BrickSpaceNomad(), CosmicTags::BrickDietDarkMatter() }, Endless) == ECosmicShape::SpreadBody);
	TestTrue(TEXT("StarHerd before WovenSwarm"), ShapeWith({ CosmicTags::BrickInstinctSwarmTech(), CosmicTags::BrickSpaceNomad() }, Endless) == ECosmicShape::StarHerd);
	TestTrue(TEXT("WovenSwarm before LastHunger"), ShapeWith({ CosmicTags::BrickDietStarEater(), CosmicTags::BrickInstinctSwarmTech() }, Endless) == ECosmicShape::WovenSwarm);
	TestTrue(TEXT("LastHunger before Entwined"), ShapeWith({ CosmicTags::BrickInstinctHiveMerge(), CosmicTags::BrickDietStarEater() }, Endless) == ECosmicShape::LastHunger);
	TestTrue(TEXT("A brick that names no shape leaves the vectors to decide"),
		ShapeWith({ CosmicTags::BrickDietDistanceTension(), CosmicTags::BrickMutationInwardHunger(), CosmicTags::BrickFormSociety() }, Endless) == ECosmicShape::Endured);
	return true;
}

// The seed bricks (draw-free, off by default): SeedBrickOf picks one tag of DT_Rules.BrickSeedTags for a people that carries
// no brick - by CosmicSeed Purpose::Brick (13) over the tags in lexical order - and none otherwise; NewGalaxy gives one to every
// AI people and none to the player's, without a draw: the stream after NewGalaxy and the generated galaxy are the same with and
// without the knob.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSeedBricksTest, "Cosmic_Ascension.Sim.SeedBricks", CosmicTests::Flags)
bool FCosmicSimSeedBricksTest::RunTest(const FString& Parameters)
{
	using namespace CosmicExistenceBricksTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	TestEqual(TEXT("The brick's purpose is 13, appended after Digest"), CosmicSeed::Purpose::Brick, 13u);

	FCosmicRulesDef Rules;
	TArray<FGameplayTag> Listed = { CosmicTags::BrickSpaceNomad(), CosmicTags::BrickDietDarkMatter(), CosmicTags::BrickInstinctSwarmTech() };
	for (const FGameplayTag& Each : Listed)
	{
		Rules.BrickSeedTags.AddTag(Each);
	}
	Listed.Sort([](const FGameplayTag& A, const FGameplayTag& B) { return FNameLexicalLess()(A.GetTagName(), B.GetTagName()); });
	FCivilizationState Civ;
	Civ.CivID = FName(TEXT("civ_7"));
	Civ.Traits.AddTag(Tag(TEXT("Trait.Arch.Default")));
	const FGameplayTag Picked = CosmicBricks::SeedBrickOf(Rules, 42, Civ);
	TestTrue(TEXT("A people without a brick takes one of the seed tags"), Listed.Contains(Picked));
	TestTrue(TEXT("the one the derived integer names, the tags in lexical order"),
		Picked == Listed[static_cast<int32>(CosmicSeed::DeriveForName(42, Civ.CivID, CosmicSeed::Purpose::Brick, 0) % 3u)]);
	TestTrue(TEXT("The same seed gives the same brick"), CosmicBricks::SeedBrickOf(Rules, 42, Civ) == Picked);
	{
		// The container's own order does not decide the pick.
		FCosmicRulesDef Reversed;
		for (int32 At = Listed.Num() - 1; At >= 0; --At)
		{
			Reversed.BrickSeedTags.AddTag(Listed[At]);
		}
		TestTrue(TEXT("Whatever order the tags were listed in"), CosmicBricks::SeedBrickOf(Reversed, 42, Civ) == Picked);
	}
	TestFalse(TEXT("An empty knob gives none"), CosmicBricks::SeedBrickOf(FCosmicRulesDef(), 42, Civ).IsValid());
	FCivilizationState Carrier = Civ;
	Carrier.Traits.AddTag(CosmicTags::BrickFormSociety());
	TestFalse(TEXT("A people that carries a brick gets none"), CosmicBricks::SeedBrickOf(Rules, 42, Carrier).IsValid());
	{
		FCosmicRulesDef TraitsOnly;
		TraitsOnly.BrickSeedTags.AddTag(Tag(TEXT("Trait.Faith.Book")));
		TestFalse(TEXT("A tag that is no brick is never seeded"), CosmicBricks::SeedBrickOf(TraitsOnly, 42, Civ).IsValid());
	}

	// NewGalaxy: without the knob (the game's rules with the bricks inert - CosmicTests::WithInertBricks, whatever DT_Rules
	// holds) and with this test's three seed tags on that copy. A people counts the bricks of its archetype row's Traits (the
	// first life's arch_seeker may carry one) and, in the second pass, an AI people that carries none of its own takes one seed.
	const FCosmicRulesDef* Game = Original.FindRules();
	if (!TestNotNull(TEXT("The game's rules"), Game))
	{
		return false;
	}
	const FCosmicRulesDef Unseeded = CosmicTests::WithInertBricks(*Game);
	FCosmicRulesDef Seeding = Unseeded;
	Seeding.BrickSeedTags = Rules.BrickSeedTags;
	int32 Seeds[2] = { 0, 0 };
	TArray<FName> Owners[2];
	TArray<float> Ethics[2];
	for (int32 Pass = 0; Pass < 2; ++Pass)
	{
		Sim->SetTables(CosmicTests::WithRules(Original, Pass == 0 ? Unseeded : Seeding));
		Sim->NewGalaxy(42, 12, 4);
		const FGalaxyState& State = Sim->GetState();
		Seeds[Pass] = Sim->GetRng().GetCurrentSeed();
		for (const FStarSystemState& System : State.Systems)
		{
			Owners[Pass].Add(System.OwnerCivID);
		}
		for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
		{
			const FCivilizationState& Each = State.Civs[CivIndex];
			Ethics[Pass].Add(Each.Ethics.Pacifism);
			int32 Bricks = 0;
			for (const FGameplayTag& Held : Each.Traits)
			{
				Bricks += CosmicTags::IsBrick(Held) ? 1 : 0;
			}
			int32 ArchetypeBricks = 0;
			if (const FCivArchetypeDef* Archetype = Original.FindCiv(Each.ArchetypeRow))
			{
				for (const FGameplayTag& ArchetypeTag : Archetype->Traits)
				{
					ArchetypeBricks += CosmicTags::IsBrick(ArchetypeTag) ? 1 : 0;
				}
			}
			const bool bSeeded = Pass == 1 && CivIndex > 0 && ArchetypeBricks == 0;
			const int32 Expected = ArchetypeBricks + (bSeeded ? 1 : 0);
			TestEqual(*FString::Printf(TEXT("Pass %d, people %d: the bricks of its archetype row and its seed brick"), Pass, CivIndex), Bricks, Expected);
			if (bSeeded)
			{
				FCivilizationState Bare = Each;
				Bare.Traits.Reset();
				TestTrue(*FString::Printf(TEXT("people %d: the pick of SeedBrickOf"), CivIndex), Each.Traits.HasTagExact(CosmicBricks::SeedBrickOf(Seeding, 42, Bare)));
			}
		}
	}
	TestEqual(TEXT("NewGalaxy takes no draw for the seed bricks"), Seeds[1], Seeds[0]);
	TestTrue(TEXT("and generates the same galaxy otherwise (owners)"), Owners[0] == Owners[1]);
	TestTrue(TEXT("(ethics)"), Ethics[0] == Ethics[1]);
	return true;
}

// The two helpers of the shared boards (notes section 5.4, test hand-off 1 - Tests/CosmicTestHelpers.h). WithInertBricks puts
// every Brick field of FCosmicRulesDef back to its initializer - the fields found by reflection and pinned at seventeen, so a
// knob added to the struct and not to the helper breaks the count, and a line left out of the helper breaks its field
// (mutation: one assignment of the helper left out) - and keeps every other field as given. StripBricks takes every Brick.* tag
// off every people and leaves the archetype's Trait.* tags (mutation: one root of the bricks only).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimBrickBoardHelpersTest, "Cosmic_Ascension.Sim.BrickBoardHelpers", CosmicTests::Flags)
bool FCosmicSimBrickBoardHelpersTest::RunTest(const FString& Parameters)
{
	using namespace CosmicExistenceBricksTests;
	// Every Brick field off its initializer: a number by reflection, the seed tags by hand; two fields that are no brick knob.
	const FCosmicRulesDef Initial;
	FCosmicRulesDef Loud;
	TArray<const FProperty*> Knobs;
	for (TFieldIterator<FProperty> It(FCosmicRulesDef::StaticStruct()); It; ++It)
	{
		const FProperty* Knob = *It;
		if (!Knob->GetName().StartsWith(TEXT("Brick"), ESearchCase::CaseSensitive))
		{
			continue;
		}
		Knobs.Add(Knob);
		if (const FNumericProperty* Number = CastField<FNumericProperty>(Knob))
		{
			void* Value = Number->ContainerPtrToValuePtr<void>(static_cast<void*>(&Loud));
			if (Number->IsFloatingPoint())
			{
				Number->SetFloatingPointPropertyValue(Value, 12345.5);
			}
			else
			{
				Number->SetIntPropertyValue(Value, static_cast<int64>(12345));
			}
		}
	}
	Loud.BrickSeedTags.AddTag(CosmicTags::BrickSpaceNomad());
	Loud.StartingEdict = Kept;
	Loud.RelationDriftRate = 0.375f;
	TestEqual(TEXT("FCosmicRulesDef holds seventeen Brick fields (sixteen numbers and the seed tags)"), Knobs.Num(), 17);
	const FCosmicRulesDef Inert = CosmicTests::WithInertBricks(Loud);
	for (const FProperty* Knob : Knobs)
	{
		TestFalse(*FString::Printf(TEXT("(%s is laid off its initializer)"), *Knob->GetName()), Knob->Identical_InContainer(&Loud, &Initial));
		TestTrue(*FString::Printf(TEXT("WithInertBricks puts %s back to its initializer"), *Knob->GetName()), Knob->Identical_InContainer(&Inert, &Initial));
	}
	TestEqual(TEXT("and keeps a field that is no brick knob (StartingEdict)"), Inert.StartingEdict, Kept);
	TestEqual(TEXT("(RelationDriftRate)"), Inert.RelationDriftRate, 0.375f);

	// StripBricks: an archetype's brick, a seed brick and a mutation go; the archetype tags stay.
	FGalaxyState Laid;
	Laid.Civs.SetNum(2);
	Laid.Civs[0].Traits.AddTag(Tag(TEXT("Trait.Arch.Seeker")));
	Laid.Civs[0].Traits.AddTag(CosmicTags::BrickFormSociety());
	Laid.Civs[1].Traits.AddTag(Tag(TEXT("Trait.Arch.Default")));
	Laid.Civs[1].Traits.AddTag(CosmicTags::BrickDietStarEater());
	Laid.Civs[1].Traits.AddTag(CosmicTags::BrickMutationStarTow());
	CosmicTests::StripBricks(Laid);
	auto BricksOf = [](const FCivilizationState& Carrier)
	{
		int32 Count = 0;
		for (const FGameplayTag& Held : Carrier.Traits)
		{
			Count += CosmicTags::IsBrick(Held) ? 1 : 0;
		}
		return Count;
	};
	TestEqual(TEXT("StripBricks takes the archetype's brick off the first people"), BricksOf(Laid.Civs[0]), 0);
	TestEqual(TEXT("and every brick off the next (a seed brick and a mutation)"), BricksOf(Laid.Civs[1]), 0);
	TestTrue(TEXT("and leaves the archetype tags"), Laid.Civs[0].Traits.HasTagExact(Tag(TEXT("Trait.Arch.Seeker"))) && Laid.Civs[1].Traits.HasTagExact(Tag(TEXT("Trait.Arch.Default"))));
	TestEqual(TEXT("(one tag each is left)"), Laid.Civs[0].Traits.Num() + Laid.Civs[1].Traits.Num(), 2);
	return true;
}

// The three conditions on a laid board (header section 2.3 - the detached cases are in Events.CheckIsPure): CivTimeVectorAtLeast
// reads the context people's counted vector; CivLacksTrait is the negation of CivHasTrait for a valid tag and an existing
// people; BorderingCivHasTrait looks at the peoples that border the context people, and only at them.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicEventsBrickConditionsTest, "Cosmic_Ascension.Events.BrickConditions", CosmicTests::Flags)
bool FCosmicEventsBrickConditionsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicExistenceBricksTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
	if (!TestNotNull(TEXT("The board is laid"), Board))
	{
		return false;
	}
	auto Holds = [&](const FCosmicCondition& Condition, int32 CivIndex)
	{
		FCosmicEvalView View(*Board, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
		View.CivIndex = CivIndex;
		View.SystemIndex = INDEX_NONE;
		return UCosmicEventSubsystem::Check(Condition, View);
	};
	// The vector, once counted.
	Board->Civs[1].Ties.Add(Board->Civs[2].CivID).Stance = CosmicTags::StanceFriendly();
	for (int32 Tick = 0; Tick < 3; ++Tick)
	{
		CosmicBricks::CountTimeVectors(*Board, Sim->GetStateIndex(), Sim->GetRules());
	}
	TestTrue(TEXT("Three linked ticks reach 3"), Holds(Cond(EConditionType::CivTimeVectorAtLeast, nullptr, TEXT("Linked"), 3.f), 1));
	TestFalse(TEXT("and not 4"), Holds(Cond(EConditionType::CivTimeVectorAtLeast, nullptr, TEXT("Linked"), 4.f), 1));
	TestFalse(TEXT("A Value below 1 is false"), Holds(Cond(EConditionType::CivTimeVectorAtLeast, nullptr, TEXT("Linked"), 0.f), 1));
	TestFalse(TEXT("Another people's vector is not read"), Holds(Cond(EConditionType::CivTimeVectorAtLeast, nullptr, TEXT("Linked"), 1.f), 2));
	// The lacking trait and the bordering trait: people 0 is a star eater; it borders people 1 alone.
	Board->Civs[0].Traits.AddTag(CosmicTags::BrickDietStarEater());
	TestFalse(TEXT("The star eater does not lack its brick"), Holds(Cond(EConditionType::CivLacksTrait, TEXT("Brick.Diet.StarEater")), 0));
	TestTrue(TEXT("its neighbour does"), Holds(Cond(EConditionType::CivLacksTrait, TEXT("Brick.Diet.StarEater")), 1));
	TestFalse(TEXT("Without a people it is false"), Holds(Cond(EConditionType::CivLacksTrait, TEXT("Brick.Diet.StarEater")), INDEX_NONE));
	TestFalse(TEXT("and without a tag"), Holds(Cond(EConditionType::CivLacksTrait, nullptr), 1));
	TestTrue(TEXT("People 1 borders the star eater"), Holds(Cond(EConditionType::BorderingCivHasTrait, TEXT("Brick.Diet.StarEater")), 1));
	TestFalse(TEXT("people 2 does not"), Holds(Cond(EConditionType::BorderingCivHasTrait, TEXT("Brick.Diet.StarEater")), 2));
	TestFalse(TEXT("nor is a people its own neighbour"), Holds(Cond(EConditionType::BorderingCivHasTrait, TEXT("Brick.Diet.StarEater")), 0));
	TestFalse(TEXT("A brick nobody carries is not found"), Holds(Cond(EConditionType::BorderingCivHasTrait, TEXT("Brick.Instinct.HiveMerge")), 1));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

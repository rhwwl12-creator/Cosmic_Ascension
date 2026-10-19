// Cosmic Ascension - automation tests of the sanctuary (Law.Hedge). Design: guideline.md section 4.5 (1), 7.4 (rule 11
// of the generation), 9.1, 16 #80 #81.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicIDs.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "Internationalization/StringTableCoreFwd.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"

namespace CosmicSanctuaryTests
{
	// The index of the sanctuary in State.Systems; INDEX_NONE when the state holds none.
	int32 SanctuaryOf(const FGalaxyState& State)
	{
		return State.Systems.IndexOfByPredicate([](const FStarSystemState& System) { return CosmicState::IsSanctuary(System.SystemID); });
	}

	// What Law.Hedge keeps true of the sanctuary in a running world (section 4.5 (1)), each part ANDed over every reading.
	struct FKept
	{
		// The sanctuary is the last system of the array, and the only one with its ID.
		bool bAtEnd = true;
		// No owner, and no people lists it among its systems.
		bool bNoOwner = true;
		bool bNoHazard = true;
		// Both project slots empty.
		bool bNoProject = true;
		// ComputeSpiritualPressure gives 0 and no dominant source.
		bool bNoPressure = true;
		// No lane of its own, and no system lists a lane to it.
		bool bNoLane = true;
		// In the sphere of no transcendent.
		bool bNoSphere = true;
		// Never written into Knowing (the view shows it by its name alone).
		bool bNotKnown = true;
		// The vessel never stands there, and no life was born there.
		bool bNoVessel = true;
		// SettleBlockOf says Sanctuary for every people.
		bool bRefusesSettlement = true;

		void Read(const UGalaxySimulationSubsystem& Sim)
		{
			const FGalaxyState& State = Sim.GetState();
			const FCosmicStateIndex& Index = Sim.GetStateIndex();
			const int32 At = SanctuaryOf(State);
			bAtEnd &= At != INDEX_NONE && At == State.Systems.Num() - 1
				&& State.Systems.FilterByPredicate([](const FStarSystemState& System) { return CosmicState::IsSanctuary(System.SystemID); }).Num() == 1;
			if (At == INDEX_NONE)
			{
				return;
			}
			const FStarSystemState& Sanctuary = State.Systems[At];
			bNoOwner &= Sanctuary.OwnerCivID.IsNone() && !State.Civs.ContainsByPredicate([](const FCivilizationState& Civ) { return Civ.OwnedSystems.Contains(CosmicIDs::Sanctuary); });
			bNoHazard &= !Sanctuary.ActiveHazard.IsValid() && Sanctuary.HazardTicksRemaining == 0;
			bNoProject &= Sanctuary.ActiveMegaproject.IsNone() && Sanctuary.ResponseMegaproject.IsNone();
			FName Dominant;
			bNoPressure &= Sim.ComputeSpiritualPressure(CosmicIDs::Sanctuary, Dominant) == 0.f && Dominant.IsNone();
			bNoLane &= Sanctuary.Lanes.IsEmpty() && !State.Systems.ContainsByPredicate([](const FStarSystemState& System) { return System.Lanes.Contains(CosmicIDs::Sanctuary); });
			bNoSphere &= !State.Transcendents.ContainsByPredicate([](const FTranscendentState& Transcendent) { return Transcendent.InfluencedSystems.Contains(CosmicIDs::Sanctuary); });
			bNotKnown &= !State.Knowing.KnownSystems.ContainsByPredicate([](const FKnownMark& Mark) { return Mark.ID == CosmicIDs::Sanctuary; });
			bNoVessel &= !CosmicState::IsSanctuary(State.Player.CurrentSystemID)
				&& !State.Calling.Lives.ContainsByPredicate([](const FLifeRecord& Life) { return CosmicState::IsSanctuary(Life.BirthSystemID); });
			for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
			{
				bRefusesSettlement &= CosmicMegaprojects::SettleBlockOf(State, Index, Sim.GetTables(), Sim.GetRules(), CivIndex, At) == CosmicMegaprojects::ECosmicSettleBlock::Sanctuary;
			}
		}

		void Report(FAutomationTestBase& Test, const FString& When) const
		{
			Test.TestTrue(*(When + TEXT(": the sanctuary stays the last system")), bAtEnd);
			Test.TestTrue(*(When + TEXT(": no people owns it")), bNoOwner);
			Test.TestTrue(*(When + TEXT(": no hazard stands there")), bNoHazard);
			Test.TestTrue(*(When + TEXT(": no project runs there")), bNoProject);
			Test.TestTrue(*(When + TEXT(": no pressure reaches it")), bNoPressure);
			Test.TestTrue(*(When + TEXT(": no lane reaches it")), bNoLane);
			Test.TestTrue(*(When + TEXT(": no sphere holds it")), bNoSphere);
			Test.TestTrue(*(When + TEXT(": Knowing never holds it")), bNotKnown);
			Test.TestTrue(*(When + TEXT(": no vessel stands or is born there")), bNoVessel);
			Test.TestTrue(*(When + TEXT(": every people is refused its settlement for the sanctuary's reason")), bRefusesSettlement);
		}
	};
}

// The sanctuary of the generation and of a long run (section 4.5 (1), rule 11 of 7.4): the home galaxy's living worlds and
// then sol at the end of the array - no lane, a fixed place outside the disc, star class G, richness 0, the name of ST_CA
// System.Sanctuary -, and over 20,000 ticks of two seeds in the tell-all mode no rule of the world gives it an owner, a
// hazard, a project, a lane, a sphere, pressure or a vessel, Knowing never holds it, and every people's settlement of it is
// refused for the sanctuary's reason. The stages pass it by through the settled and wild predicates: no branch of theirs
// names it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSanctuaryKeptTest, "Cosmic_Ascension.Sim.SanctuaryKept", CosmicTests::Flags)
bool FCosmicSimSanctuaryKeptTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSanctuaryTests;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FString Name = FText::FromStringTable(CosmicLabels::TableId, TEXT("System.Sanctuary"), EStringTableLoadingPolicy::FindOrFullyLoad).ToString();
	for (const int32 Seed : { 42, 7 })
	{
		Sim->NewGalaxy(Seed, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		const FGalaxyState& State = Sim->GetState();
		const int32 Living = Sim->GetHomeGalaxy().LivingWorlds;
		const FString Run = FString::Printf(TEXT("Seed %d"), Seed);

		// ---- The generation ----
		TestEqual(*(Run + TEXT(": the living worlds of the home row and the sanctuary")), State.Systems.Num(), Living + 1);
		bool bIDs = State.Systems.Num() == Living + 1;
		for (int32 At = 0; bIDs && At < Living; ++At)
		{
			bIDs = State.Systems[At].SystemID == FName(*FString::Printf(TEXT("sys_%d"), At));
		}
		TestTrue(*(Run + TEXT(": the living systems are sys_<n> in array order, sys_0 first")), bIDs);
		TestTrue(*(Run + TEXT(": sys_0 is the first life's home")), CosmicState::IsFirstHome(State.Systems[0].SystemID) && State.Player.CurrentSystemID == CosmicIDs::FirstHome);
		const int32 At = SanctuaryOf(State);
		if (!TestEqual(*(Run + TEXT(": the sanctuary is the last system")), At, State.Systems.Num() - 1))
		{
			return false;
		}
		const FStarSystemState& Sanctuary = State.Systems[At];
		const float Radius = 1000.f * FMath::Sqrt(static_cast<float>(Living));
		TestTrue(*(Run + TEXT(": its place is fixed at 1.15 disc radii, angle 0")), Sanctuary.Position.Equals(FVector(1.15f * Radius, 0.0, 0.0), 0.01));
		TestTrue(*(Run + TEXT(": outside the disc of the living worlds")), State.Systems.ContainsByPredicate([&](const FStarSystemState& System)
		{
			return !CosmicState::IsSanctuary(System.SystemID) && FVector::Dist2D(System.Position, FVector::ZeroVector) > Radius;
		}) == false);
		TestTrue(*(Run + TEXT(": star class G")), Sanctuary.StarClass == FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.G"))));
		TestEqual(*(Run + TEXT(": richness 0")), Sanctuary.ResourceRichness, 0.f);
		TestEqual(*(Run + TEXT(": the home galaxy's row")), Sanctuary.GalaxyRow, Sim->GetRules().HomeGalaxyRow);
		TestTrue(*(Run + TEXT(": the name of ST_CA System.Sanctuary, flattened")), !Name.IsEmpty() && Sanctuary.DisplayName.ToString() == Name && Sanctuary.DisplayName.IsCultureInvariant());
		TestFalse(*(Run + TEXT(": no other system bears its name")), State.Systems.ContainsByPredicate([&](const FStarSystemState& System)
		{
			return !CosmicState::IsSanctuary(System.SystemID) && System.DisplayName.ToString() == Name;
		}));
		TestTrue(*(Run + TEXT(": no starting core")), !State.Transcendents.ContainsByPredicate([](const FTranscendentState& Transcendent) { return Transcendent.InfluencedSystems.Contains(CosmicIDs::Sanctuary); }));
		FKept AtStart;
		AtStart.Read(*Sim);
		AtStart.Report(*this, Run + TEXT(" at tick 0"));

		// ---- 20,000 ticks, read after each ----
		FKept Running;
		for (int32 Tick = 0; Tick < 20000; ++Tick)
		{
			Sim->RunHeadless(1);
			Running.Read(*Sim);
		}
		Running.Report(*this, Run + TEXT(" over 20,000 ticks"));
		TestEqual(*(Run + TEXT(": (the run reached tick 20,000)")), State.Tick, static_cast<int64>(20000));
	}
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

// Every path of the world refuses the sanctuary (Law.Hedge, section 4.5 (1)), each against a control that the same call
// takes elsewhere: a settlement (SettleBlockOf first, SettleTargets and PeopleSettleBlockOf with a lane to it), a hazard
// (Place - even on a sanctuary the test makes settled), a hand-over (Transfer), a destruction (Destroy), pressure (the
// sanctuary in a fallen sphere still gives 0), a vessel born there (Install) and a wanderer walking from there
// (BecomeWanderer). A refused call changes nothing and draws nothing.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSanctuaryRefusalsTest, "Cosmic_Ascension.Sim.SanctuaryRefusals", CosmicTests::Flags)
bool FCosmicSimSanctuaryRefusalsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSanctuaryTests;
	using CosmicMegaprojects::ECosmicSettleBlock;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	AddExpectedError(TEXT("Install: no vessel is born in the sanctuary"), EAutomationExpectedErrorFlags::Contains, 1);
	AddExpectedError(TEXT("BecomeWanderer: the vessel stands in the sanctuary"), EAutomationExpectedErrorFlags::Contains, 1);

	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	FGalaxyState& State = Sim->GetMutableState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicDataTables& Tables = Sim->GetTables();
	const FCosmicRulesDef& Rules = Sim->GetRules();
	const int32 At = SanctuaryOf(State);
	const int32 Ruled = Index.FindCiv(State, State.Player.RuledCivID);
	if (!TestTrue(TEXT("(the galaxy holds the sanctuary and a ruled people)"), State.Systems.IsValidIndex(At) && State.Civs.IsValidIndex(Ruled)))
	{
		return false;
	}
	const int32 Home = Index.FindSystem(State, State.Civs[Ruled].HomeSystemID);

	// ---- Settlement ----
	TestTrue(TEXT("SettleBlockOf gives the sanctuary's reason first"), CosmicMegaprojects::SettleBlockOf(State, Index, Tables, Rules, Ruled, At) == ECosmicSettleBlock::Sanctuary);
	TestTrue(TEXT("whoever asks: a people out of range too"), CosmicMegaprojects::SettleBlockOf(State, Index, Tables, Rules, 99, At) == ECosmicSettleBlock::Sanctuary);
	TestTrue(TEXT("(a system out of range is NoPeople)"), CosmicMegaprojects::SettleBlockOf(State, Index, Tables, Rules, Ruled, 999) == ECosmicSettleBlock::NoPeople);
	{
		// A people that could settle a wild neighbour - the third rung lifts the rung, the cap and the sphere - with a lane
		// from its home to the sanctuary (v0.8 step 5 may give the sanctuary lanes).
		const FGalaxyState Before = State;
		State.Civs[Ruled].Tier = EAscensionTier::Tier3_Causal;
		State.Systems[Home].Lanes.Add(CosmicIDs::Sanctuary);
		State.Systems[At].Lanes.Add(State.Systems[Home].SystemID);
		TestTrue(TEXT("(with a lane from home the people may settle now)"), CosmicMegaprojects::MaySettle(State, Index, Rules, Ruled));
		TestTrue(TEXT("Across a lane the sanctuary is still refused for its reason"), CosmicMegaprojects::SettleBlockOf(State, Index, Tables, Rules, Ruled, At) == ECosmicSettleBlock::Sanctuary);
		TestFalse(TEXT("and it is no target of the people"), CosmicMegaprojects::SettleTargets(State, Index, Tables, Rules, Ruled).Contains(At));
		TestTrue(TEXT("nor a reason of the people as a whole"), CosmicMegaprojects::PeopleSettleBlockOf(State, Index, Tables, Rules, Ruled) != ECosmicSettleBlock::Sanctuary);
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		TestFalse(TEXT("Settle refuses it"), CosmicMegaprojects::Settle(Ctx, Ruled, At));
		TestTrue(TEXT("and the sanctuary stays unowned"), State.Systems[At].OwnerCivID.IsNone());
		// With the sanctuary as the only neighbour of the people's one foothold, the people has no lane to settle by.
		State.Systems[Home].Lanes = { CosmicIDs::Sanctuary };
		TestTrue(TEXT("A people whose only neighbour is the sanctuary has no lane: it is no reason of the people"),
			CosmicMegaprojects::PeopleSettleBlockOf(State, Index, Tables, Rules, Ruled) == ECosmicSettleBlock::NoLane);
		TestTrue(TEXT("(and no target)"), CosmicMegaprojects::SettleTargets(State, Index, Tables, Rules, Ruled).IsEmpty());
		State = Before;
	}

	// ---- Hazards ----
	{
		FName DrawRow;
		for (const FName Row : Tables.GetHazardRowNames())
		{
			const FHazardDef* Def = Tables.FindHazard(Row);
			if (DrawRow.IsNone() && Def && Def->SpawnRule == ECosmicHazardSpawn::Draw)
			{
				DrawRow = Row;
			}
		}
		if (TestFalse(TEXT("(the game has a hazard row of the spawn draw)"), DrawRow.IsNone()))
		{
			const FGalaxyState Before = State;
			// The sanctuary made a settled system of the ruled people, as no rule of the game makes it: the guard still refuses.
			State.Systems[At].OwnerCivID = State.Civs[Ruled].CivID;
			State.Civs[Ruled].OwnedSystems.Add(CosmicIDs::Sanctuary);
			State.Systems[Home].ActiveHazard = FGameplayTag();
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			TestFalse(TEXT("Place refuses the sanctuary even when it is settled"), CosmicHazards::Place(Ctx, At, DrawRow));
			TestFalse(TEXT("and no hazard stands there"), State.Systems[At].ActiveHazard.IsValid());
			TestTrue(TEXT("The same row takes the people's home (the control)"), CosmicHazards::Place(Ctx, Home, DrawRow));
			State = Before;
		}
	}

	// ---- Hand-over and destruction ----
	{
		const FGalaxyState Before = State;
		const int32 Wild = State.Systems.IndexOfByPredicate([](const FStarSystemState& System)
		{
			return System.OwnerCivID.IsNone() && !CosmicState::IsSanctuary(System.SystemID) && !CosmicHazards::IsDeadNode(System);
		});
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		TestFalse(TEXT("Transfer refuses to hand the sanctuary to a people"), CosmicHazards::Transfer(Ctx, At, Ruled));
		TestTrue(TEXT("and it stays unowned"), State.Systems[At].OwnerCivID.IsNone() && !State.Civs[Ruled].OwnedSystems.Contains(CosmicIDs::Sanctuary));
		if (TestTrue(TEXT("(the galaxy has a wild living system)"), State.Systems.IsValidIndex(Wild)))
		{
			TestTrue(TEXT("Transfer hands a wild system over (the control)"), CosmicHazards::Transfer(Ctx, Wild, Ruled));
			CosmicHazards::Destroy(Ctx, Wild);
			TestTrue(TEXT("Destroy destroys a living system (the control)"), CosmicHazards::IsDeadNode(State.Systems[Wild]));
		}
		CosmicHazards::Destroy(Ctx, At);
		TestFalse(TEXT("Destroy leaves the sanctuary alive"), CosmicHazards::IsDeadNode(State.Systems[At]));
		State = Before;
	}

	// ---- The vessel is never carried into the sanctuary ----
	{
		// A wanderer on a wild system whose first lane leads to the sanctuary and whose second to a living system: when
		// that system is destroyed, the vessel goes to the living one (lanes to the sanctuary come with v0.8 step 5).
		const FGalaxyState Before = State;
		TArray<int32> Wild;
		for (int32 Each = 0; Each < State.Systems.Num() && Wild.Num() < 2; ++Each)
		{
			if (State.Systems[Each].OwnerCivID.IsNone() && !CosmicState::IsSanctuary(State.Systems[Each].SystemID))
			{
				Wild.Add(Each);
			}
		}
		if (TestEqual(TEXT("(the galaxy has two wild living systems)"), Wild.Num(), 2))
		{
			const FName Doomed = State.Systems[Wild[0]].SystemID;
			const FName Refuge = State.Systems[Wild[1]].SystemID;
			State.Systems[Wild[0]].Lanes = { CosmicIDs::Sanctuary, Refuge };
			State.Player.Role = EPlayerRole::Wanderer;
			State.Player.RuledCivID = NAME_None;
			State.Player.CurrentSystemID = Doomed;
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			CosmicHazards::Destroy(Ctx, Wild[0]);
			TestTrue(TEXT("(the wanderer's system is destroyed)"), CosmicHazards::IsDeadNode(State.Systems[Wild[0]]));
			TestEqual(TEXT("The vessel is carried past the sanctuary to the next living neighbour"), State.Player.CurrentSystemID, Refuge);
		}
		State = Before;
	}

	// ---- Pressure ----
	{
		const FGalaxyState Before = State;
		const int32 Holder = State.Transcendents.IndexOfByPredicate([&](const FTranscendentState& Transcendent)
		{
			return !Transcendent.bSealed && !Transcendent.InfluencedSystems.IsEmpty() && Tables.FindTranscendent(Transcendent.TranscendentID);
		});
		if (TestTrue(TEXT("(the game's roster has an unsealed sphere with a row)"), State.Transcendents.IsValidIndex(Holder)))
		{
			const int32 Core = Index.FindSystem(State, State.Transcendents[Holder].InfluencedSystems[0]);
			State.Transcendents[Holder].InfluencedSystems.Add(CosmicIDs::Sanctuary);
			TestTrue(TEXT("(the sphere presses on its core: the control)"), UGalaxySimulationSubsystem::ComputePressureIn(State, Index, Tables, Rules, Core).P > 0.f);
			const FCosmicPressure Reading = UGalaxySimulationSubsystem::ComputePressureIn(State, Index, Tables, Rules, At);
			TestEqual(TEXT("No pressure reaches the sanctuary, even inside a sphere"), Reading.P, 0.f);
			TestTrue(TEXT("and nothing dominates there"), Reading.DominantSourceID.IsNone() && Reading.DominantSource == ECosmicPressureSource::None);
			FName Dominant;
			TestEqual(TEXT("The member reading says the same"), Sim->ComputeSpiritualPressure(CosmicIDs::Sanctuary, Dominant), 0.f);
		}
		State = Before;
	}

	// ---- The vessel ----
	{
		const uint32 Hash = Sim->ComputeStateHash();
		const int32 Stream = Sim->GetRng().GetCurrentSeed();
		Role->Install(CosmicIDs::Sanctuary, State.Civs[Ruled].CivID, EPlayerRole::Wanderer);
		TestEqual(TEXT("Install refuses the sanctuary and changes nothing"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Hash));
		TestEqual(TEXT("and draws nothing"), Sim->GetRng().GetCurrentSeed(), Stream);
		TestTrue(TEXT("The vessel still rules where it was born"), State.Player.Role == EPlayerRole::Ruler && State.Player.CurrentSystemID == CosmicIDs::FirstHome);

		const FGalaxyState Before = State;
		State.Player.CurrentSystemID = CosmicIDs::Sanctuary;
		Role->BecomeWanderer(FGameplayTag(), FGameplayTag());
		TestTrue(TEXT("BecomeWanderer refuses a vessel that stands in the sanctuary"), State.Player.Role == EPlayerRole::Ruler && !State.Player.RuledCivID.IsNone());
		TestFalse(TEXT("and writes nothing into Knowing"), State.Knowing.KnownSystems.ContainsByPredicate([](const FKnownMark& Mark) { return Mark.ID == CosmicIDs::Sanctuary; }));
		State = Before;
	}

	Sim->NewGalaxy(42, 5, 3);
	return true;
}

// The load refuses a state that does not end with a kept sanctuary (section 4.5 (1), 9.1): a save from before v0.8 step 4
// (the sanctuary missing - sol was the first life's home then), and one whose sanctuary has an owner, a hazard or a
// project. A refused load leaves the running galaxy as it was; a kept one loads.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveSanctuaryTest, "Cosmic_Ascension.Sim.SaveSanctuary", CosmicTests::Flags)
bool FCosmicSimSaveSanctuaryTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSanctuaryTests;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicSaveSubsystem* Save = Sim->GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>();
	AddExpectedError(TEXT("LoadGalaxy: slot .* does not end with a kept sanctuary"), EAutomationExpectedErrorFlags::Contains, 5);

	// A kept sanctuary loads.
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(100);
	const FGuid Kept = Sim->GetState().GalaxyID;
	const uint32 KeptHash = Sim->ComputeStateHash();
	if (TestTrue(TEXT("(the kept galaxy is saved)"), Save->SaveCurrentGalaxy()))
	{
		Sim->NewGalaxy(43, 5, 3);
		TestTrue(TEXT("A galaxy that ends with a kept sanctuary loads"), Save->LoadGalaxy(Kept));
		TestEqual(TEXT("as it was saved"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(KeptHash));
		Save->DeleteSave(Kept);
	}

	// Each broken state saved under its own galaxy, then refused while another galaxy runs.
	struct FCase
	{
		const TCHAR* What;
		TFunction<void(FGalaxyState&)> Break;
	};
	const FCase Cases[] = {
		{ TEXT("a state without the sanctuary (a save from before v0.8 step 4)"), [](FGalaxyState& State) { State.Systems.Pop(); } },
		{ TEXT("a state whose sanctuary is not the last system"), [](FGalaxyState& State) { State.Systems.Swap(0, State.Systems.Num() - 1); } },
		{ TEXT("a state whose sanctuary has an owner"), [](FGalaxyState& State) { State.Systems.Last().OwnerCivID = State.Civs[0].CivID; } },
		{ TEXT("a state whose sanctuary has a hazard"), [](FGalaxyState& State) { State.Systems.Last().ActiveHazard = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Plague"))); } },
		{ TEXT("a state whose sanctuary runs a project"), [](FGalaxyState& State) { State.Systems.Last().ResponseMegaproject = FName(TEXT("project_test")); } },
	};
	for (const FCase& Case : Cases)
	{
		Sim->NewGalaxy(44, 5, 3);
		Case.Break(Sim->GetMutableState());
		Sim->RebuildStateIndex();
		const FGuid Broken = Sim->GetState().GalaxyID;
		if (!TestTrue(*FString::Printf(TEXT("(%s is saved)"), Case.What), Save->SaveCurrentGalaxy()))
		{
			continue;
		}
		Sim->NewGalaxy(45, 5, 3);
		const uint32 Running = Sim->ComputeStateHash();
		TestFalse(*FString::Printf(TEXT("The load refuses %s"), Case.What), Save->LoadGalaxy(Broken));
		TestEqual(*FString::Printf(TEXT("and leaves the running galaxy as it was (%s)"), Case.What), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Running));
		Save->DeleteSave(Broken);
	}
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

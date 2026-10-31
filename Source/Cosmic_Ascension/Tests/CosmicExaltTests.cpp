// Cosmic Ascension - automation test of petition_exalt's arrival (B6a [B-6a6]). Design:
// docs/60_DECISIONS/headers/B6a_endings_and_the_run_past_the_third_rung.md section 3.1, section 6 (Sim.ExaltArrives),
// decision 9; the edge rule of the Standing cards: Events/CosmicPetitions.h (StandingHeld, Present, IsFirstTickOfReign).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "Events/CosmicPetitions.h"
#include "GameplayTagContainer.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"

namespace CosmicExaltTests
{
	// What one pass over the two boards saw (RunExaltBoards).
	struct FExaltSeen
	{
		bool bLaid = false;
		// Board 1 - the ruler's people climbs to the second rung in the tick.
		bool bHeldBeforeClimb = true;
		EAscensionTier ClimbTier = EAscensionTier::Tier1_Planetary;
		FName ClimbCard;
		FName KeptHoldingCard;
		// Board 2 - the people climbs while the vessel walks, then takes the vessel back.
		bool bWandering = false;
		EAscensionTier WalkClimbTier = EAscensionTier::Tier1_Planetary;
		FName WandererCard;
		CosmicWanderer::ECrownBlock Crown = CosmicWanderer::ECrownBlock::NotWanderer;
		bool bFirstTickOfReign = false;
		bool bHeldAtCrowning = false;
		FName ReignCard;
	};

	// The people the vessel rules and its home, on a fresh slice board of seed 42 run to tick 10 on Tables (below the
	// GalaxyTickAtLeast 30 of petition_wise_heart, the first crisis at 150 and the periodic slot at 300: no other card can
	// come to hold with the climb but by a hazard drawn in that very tick). False when the first life rules no people with
	// a home.
	bool LayExaltBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Tables, int32& OutCiv, int32& OutHome)
	{
		Sim.SetTables(Tables);
		Sim.NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		Sim.RunHeadless(10);
		const FGalaxyState& State = Sim.GetState();
		OutCiv = CosmicPetitions::RuledCivIndexOf(State, Sim.GetStateIndex());
		OutHome = State.Civs.IsValidIndex(OutCiv) ? Sim.GetStateIndex().FindSystem(State, State.Civs[OutCiv].HomeSystemID) : INDEX_NONE;
		return State.Systems.IsValidIndex(OutHome);
	}

	// A quiet board: no hazard anywhere (no crisis card stands or comes to hold by a hazard already placed), the people's
	// Stability 0.8 (no low-stability card) and no card waiting (StandingHeld is empty while one waits - the head of the
	// next tick would hold nothing).
	void QuietExaltBoard(FGalaxyState& State, int32 CivIndex)
	{
		for (FStarSystemState& System : State.Systems)
		{
			CosmicHazards::EmptyHazardSlot(System);
		}
		State.Civs[CivIndex].Stability = 0.8f;
		CosmicPetitions::Clear(State);
	}

	// The second rung's feat (feat_dyson_swarm: the Dyson swarm and physics 0.5) met on the people's home, so that the
	// judgment at the end of the AI stage of the next tick lifts the people from the first rung: the head of that tick
	// still reads the first rung.
	void ReadyExaltClimb(FGalaxyState& State, int32 CivIndex, int32 Home)
	{
		FCivilizationState& Civ = State.Civs[CivIndex];
		Civ.Tier = EAscensionTier::Tier1_Planetary;
		Civ.Research.Add(FName(TEXT("Physics")), 0.6f);
		FStarSystemState& System = State.Systems[Home];
		System.CompletedMegaprojects.RemoveAll([](const FMegaprojectRecord& Kept) { return Kept.Row == FName(TEXT("project_dyson")); });
		FMegaprojectRecord& Done = System.CompletedMegaprojects.AddDefaulted_GetRef();
		Done.Row = FName(TEXT("project_dyson"));
		Done.Tick = State.Tick;
		Done.CivID = Civ.CivID;
	}

	bool HoldsExalt(const UGalaxySimulationSubsystem& Sim)
	{
		return CosmicPetitions::StandingHeld(Sim.GetState(), Sim.GetStateIndex(), Sim.GetTables(), Sim.GetRules())
			.ContainsByPredicate([](const FCosmicPetitionUnit& Unit) { return Unit.Row == CosmicRows::PetitionExalt; });
	}

	// Both boards on Tables, ticked through the simulation (StepOnce - the head of the tick reads StandingHeld itself).
	FExaltSeen RunExaltBoards(UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, const FCosmicDataTables& Tables)
	{
		FExaltSeen Seen;
		int32 Civ = INDEX_NONE;
		int32 Home = INDEX_NONE;

		// Board 1: the ruled people climbs in tick 11.
		if (!LayExaltBoard(Sim, Tables, Civ, Home))
		{
			return Seen;
		}
		{
			FGalaxyState& Live = Sim.GetMutableState();
			QuietExaltBoard(Live, Civ);
			ReadyExaltClimb(Live, Civ, Home);
			Seen.bHeldBeforeClimb = HoldsExalt(Sim);
			Sim.StepOnce();
			Seen.ClimbTier = Sim.GetState().Civs[Civ].Tier;
			Seen.ClimbCard = Sim.GetState().Knowing.PendingPetitionRow;
		}
		{
			// The condition keeps holding: the next tick's head holds it, so it does not come again.
			CosmicPetitions::Clear(Sim.GetMutableState());
			Sim.StepOnce();
			Seen.KeptHoldingCard = Sim.GetState().Knowing.PendingPetitionRow;
		}

		// Board 2: the vessel leaves the throne in tick 10, its people climbs in tick 11 while it walks, and it is taken back
		// before tick 12 - the first tick of that reign compares with an empty head (B1).
		if (!LayExaltBoard(Sim, Tables, Civ, Home))
		{
			return Seen;
		}
		{
			FGalaxyState& Live = Sim.GetMutableState();
			const FName HomeID = Live.Systems[Home].SystemID;
			const FName CivID = Live.Civs[Civ].CivID;
			CosmicPetitions::Clear(Live);
			Live.Player.CurrentSystemID = HomeID;
			Role.BecomeWanderer(FGameplayTag::RequestGameplayTag(FName(TEXT("AI.Profile.Conservative"))), FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Deposed"))));
			FGalaxyState& Walking = Sim.GetMutableState();
			Seen.bWandering = Walking.Player.Role == EPlayerRole::Wanderer && CosmicPetitions::RuledCivIndexOf(Walking, Sim.GetStateIndex()) == INDEX_NONE;
			Walking.Player.CurrentSystemID = HomeID;
			QuietExaltBoard(Walking, Civ);
			ReadyExaltClimb(Walking, Civ, Home);
			Sim.StepOnce();
			Seen.WalkClimbTier = Sim.GetState().Civs[Civ].Tier;
			Seen.WandererCard = Sim.GetState().Knowing.PendingPetitionRow;

			FGalaxyState& Back = Sim.GetMutableState();
			QuietExaltBoard(Back, Civ);
			Back.Civs[Civ].ThroneLockedUntilTick = 0;
			Back.Player.CurrentSystemID = HomeID;
			Sim.NoteMet(CivID, HomeID);
			Seen.Crown = Role.BecomeRuler(CivID);
			Seen.bFirstTickOfReign = CosmicPetitions::IsFirstTickOfReign(Sim.GetState());
			Seen.bHeldAtCrowning = HoldsExalt(Sim);
			Sim.StepOnce();
			Seen.ReignCard = Sim.GetState().Knowing.PendingPetitionRow;
		}
		Seen.bLaid = true;
		return Seen;
	}
}

// petition_exalt on the game's data (B6a header section 3.1, decision 9): the condition reads the rung of the people the
// vessel rules, so the card reaches the waiting slot in the tick that people climbs to the second rung, and on the first
// tick of a reign over a people already at the second rung (the empty head of B1); not again while it keeps holding, not
// for a wanderer, and never on the tables with the row as before (CosmicTests::WithExaltAsBefore - the personal rung).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimExaltArrivesTest, "Cosmic_Ascension.Sim.ExaltArrives", CosmicTests::Flags)
bool FCosmicSimExaltArrivesTest::RunTest(const FString& Parameters)
{
	using namespace CosmicExaltTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	if (!TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const FCosmicDataTables GameTables = Sim->GetTables();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(GameTables);
		Sim->NewGalaxy(42, 5, 3);
	};
	if (!TestTrue(TEXT("(the game's tables hold petition_exalt)"), GameTables.FindEncounter(CosmicRows::PetitionExalt) != nullptr))
	{
		return false;
	}

	// The helper: the row as before - the personal rung, no rung of a people.
	{
		const FCosmicDataTables Before = CosmicTests::WithExaltAsBefore(GameTables);
		const FEncounterDef* Row = Before.FindEncounter(CosmicRows::PetitionExalt);
		const bool bPersonal = Row && Row->Conditions.ContainsByPredicate([](const FCosmicCondition& Condition)
		{
			return Condition.Type == EConditionType::PlayerTierAtLeast && Condition.Value == 2.f;
		});
		const bool bPeople = Row && Row->Conditions.ContainsByPredicate([](const FCosmicCondition& Condition) { return Condition.Type == EConditionType::CivTierAtLeast; });
		TestTrue(TEXT("WithExaltAsBefore gives the row the old PlayerTierAtLeast 2 and no CivTierAtLeast"), bPersonal && !bPeople);
		TestTrue(TEXT("and leaves the game's own table as it is"), GameTables.Encounters != Before.Encounters);
	}

	// The game's data.
	const FExaltSeen Game = RunExaltBoards(*Sim, *Role, GameTables);
	if (!TestTrue(TEXT("(both boards are laid: the first life rules a people with a home)"), Game.bLaid))
	{
		return false;
	}
	TestFalse(TEXT("At the head of the climb tick the people's first rung does not hold the card"), Game.bHeldBeforeClimb);
	TestEqual(TEXT("The ruled people climbs to the second rung in that tick"), Game.ClimbTier, EAscensionTier::Tier2_Stellar);
	TestEqual(TEXT("and petition_exalt reaches the waiting slot in it"), Game.ClimbCard, CosmicRows::PetitionExalt);
	TestNotEqual(TEXT("It does not come again while the condition keeps holding"), Game.KeptHoldingCard, CosmicRows::PetitionExalt);
	TestTrue(TEXT("(board 2: the vessel walks)"), Game.bWandering);
	TestEqual(TEXT("The people climbs while the vessel walks"), Game.WalkClimbTier, EAscensionTier::Tier2_Stellar);
	TestNotEqual(TEXT("A wanderer standing in its home gets no exalt card"), Game.WandererCard, CosmicRows::PetitionExalt);
	TestTrue(TEXT("The people takes the vessel back"), Game.Crown == CosmicWanderer::ECrownBlock::None);
	TestTrue(TEXT("(the next tick is the first of that reign)"), Game.bFirstTickOfReign);
	TestTrue(TEXT("(the card's condition already holds when it is crowned)"), Game.bHeldAtCrowning);
	TestEqual(TEXT("The first tick of a reign over a people at the second rung puts petition_exalt"), Game.ReignCard, CosmicRows::PetitionExalt);

	// The row as before: the same climbs and crowning, and the card never comes.
	const FExaltSeen Old = RunExaltBoards(*Sim, *Role, CosmicTests::WithExaltAsBefore(GameTables));
	if (TestTrue(TEXT("(the boards as before are laid)"), Old.bLaid))
	{
		TestEqual(TEXT("As before the people still climbs"), Old.ClimbTier, EAscensionTier::Tier2_Stellar);
		TestNotEqual(TEXT("but petition_exalt does not come in the climb tick"), Old.ClimbCard, CosmicRows::PetitionExalt);
		TestTrue(TEXT("(as before the vessel is taken back)"), Old.Crown == CosmicWanderer::ECrownBlock::None);
		TestNotEqual(TEXT("nor on the first tick of the reign"), Old.ReignCard, CosmicRows::PetitionExalt);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

// Cosmic Ascension - automation tests of B7x's measurement ([B-7x4m]): the ruler's and the wanderer's skip on the crossroads
// cards (B7x header section 5, decision 19) and -Crossroads=<k>, on bench cards the test builds and, from [B-7x5c], once more on
// the game's cards (header section 6), and the balance commandlet's -SaveAt list. [B-7x5c] (header section 6): the game's
// crossroads cards on the periodic slot (Sim.CrossroadsPresent) and the people their bricks reach (Sim.CrossroadsAnswer).
// Design: docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md section 5, 6; notes section 5.4 code 8.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Events/CosmicPetitions.h"
#include "GameplayTagContainer.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicBalanceCommandlet.h"
#include "Tests/CosmicTestHelpers.h"
#include "World/CosmicRulerPolicy.h"
#include "World/CosmicWandererPolicy.h"

namespace CosmicCrossroadsPolicyTests
{
	// The bench card's row in the test's DT_Encounters (Act answers a card the tables hold).
	const FName BenchRow(TEXT("petition_test_crossroads"));
	// The same card as a wanderer's (Subject VesselSystem - the enc_ rows).
	const FName WandererBenchRow(TEXT("enc_test_crossroads"));

	FCosmicEffect EffectOf(EEffectType Type, const FGameplayTag& Tag, float Value = 0.f)
	{
		FCosmicEffect Effect;
		Effect.Type = Type;
		Effect.Tag = Tag;
		Effect.Value = Value;
		return Effect;
	}

	// The condition of the crossroads cards (schema rule 5): a time vector at least Value.
	FCosmicCondition VectorAtLeast(const TCHAR* Word, float Value)
	{
		FCosmicCondition Condition;
		Condition.Type = EConditionType::CivTimeVectorAtLeast;
		Condition.Key = FName(Word);
		Condition.Value = Value;
		return Condition;
	}

	// The first crossroads' shape: [0] a brick given and Society taken, for Matter, no check (the shape's choice); [1] Society
	// taken with a value back, for Insight (the half step); [2] the no-value choice.
	FEncounterDef FirstBench()
	{
		FEncounterDef Card;
		Card.Trigger = ECosmicEventTrigger::Lottery;
		Card.Subject = ECosmicEventSubject::Civilization;
		Card.Conditions.Add(VectorAtLeast(TEXT("Built"), 1.f));
		FEncounterChoice& Shape = Card.Choices.AddDefaulted_GetRef();
		Shape.Cost.Matter = 100.f;
		Shape.OnSuccess.Add(EffectOf(EEffectType::AddTrait, CosmicTags::BrickSpaceNomad()));
		Shape.OnSuccess.Add(EffectOf(EEffectType::RemoveTrait, CosmicTags::BrickFormSociety()));
		FEncounterChoice& Half = Card.Choices.AddDefaulted_GetRef();
		Half.Cost.Insight = 10.f;
		Half.OnSuccess.Add(EffectOf(EEffectType::RemoveTrait, CosmicTags::BrickFormSociety()));
		FCosmicEffect Back = EffectOf(EEffectType::AddResource, FGameplayTag(), 50.f);
		Back.Key = FName(TEXT("Matter"));
		Half.OnSuccess.Add(Back);
		Card.Choices.AddDefaulted();
		return Card;
	}

	// The second crossroads' shape: [0] a brick given, for Matter; [1] a value that moves no brick (Stability, for Influence);
	// [2] the no-value choice.
	FEncounterDef SecondBench()
	{
		FEncounterDef Card;
		Card.Trigger = ECosmicEventTrigger::Lottery;
		Card.Subject = ECosmicEventSubject::Civilization;
		Card.Conditions.Add(VectorAtLeast(TEXT("Linked"), 1.f));
		FEncounterChoice& Brick = Card.Choices.AddDefaulted_GetRef();
		Brick.Cost.Matter = 100.f;
		Brick.OnSuccess.Add(EffectOf(EEffectType::AddTrait, CosmicTags::BrickMutationDustBand()));
		FEncounterChoice& Value = Card.Choices.AddDefaulted_GetRef();
		Value.Cost.Influence = 10.f;
		Value.OnSuccess.Add(EffectOf(EEffectType::ChangeStability, FGameplayTag(), 0.05f));
		Card.Choices.AddDefaulted();
		return Card;
	}

	// [B-7x5c]: a row of the game's DT_Encounters (nullptr when the table or the row is missing).
	const FEncounterDef* GameCard(const FCosmicDataTables& Tables, FName Name)
	{
		const UDataTable* Cards = Tables.Encounters.Get();
		return Cards ? Cards->FindRow<FEncounterDef>(Name, TEXT("crossroads tests"), false) : nullptr;
	}

	// A DT_Encounters that holds the one row: the periodic slot puts it or nothing (the pick among other cards is
	// Sim.PetitionPresent's).
	UDataTable* OneCard(FName Name, const FEncounterDef& Row)
	{
		UDataTable* Table = CosmicTests::NewTable(FEncounterDef::StaticStruct());
		Table->AddRow(Name, Row);
		return Table;
	}

	// The threshold of a crossroads card: the value of its first CivTimeVectorAtLeast (each game card holds one; no schema rule
	// caps it); -1 without one.
	int32 ThresholdOf(const FEncounterDef& Row)
	{
		for (const FCosmicCondition& Condition : Row.Conditions)
		{
			if (Condition.Type == EConditionType::CivTimeVectorAtLeast)
			{
				return static_cast<int32>(Condition.Value);
			}
		}
		return -1;
	}

	// The game's tables with DT_Encounters holding the one card, a new galaxy of seed 42 (the first life rules arch_seeker's
	// people), and the state to set up.
	FGalaxyState& BoardWith(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Original, FName Name, const FEncounterDef& Row)
	{
		FCosmicDataTables Tables = Original;
		Tables.Encounters = OneCard(Name, Row);
		Sim.SetTables(Tables);
		Sim.NewGalaxy(42, 5, 3);
		return Sim.GetMutableState();
	}

	// The periodic slot of the life (CosmicPetitions::Present - section 5.10): no card waiting, the life born at 0 and the
	// tick one period on, no Standing unit at the head of the tick. True when a card was put.
	bool Slot(UGalaxySimulationSubsystem& Sim, FGalaxyState& State)
	{
		CosmicPetitions::Clear(State);
		State.Calling.BornTick = 0;
		State.Tick = FMath::Max(1, Sim.GetRules().PetitionPeriodTicks);
		return CosmicPetitions::Present(State, Sim.GetStateIndex(), Sim.GetTables(), Sim.GetRules(), TArray<FCosmicPetitionUnit>());
	}

	// The kept law's shape (C10): [0] the brick and the end of the reign; [1] a value; [2] the no-value choice.
	FEncounterDef LawBench()
	{
		FEncounterDef Card = SecondBench();
		Card.Choices[0].Cost = FResourcePool();
		Card.Choices[0].OnSuccess.Reset();
		Card.Choices[0].OnSuccess.Add(EffectOf(EEffectType::AddTrait, CosmicTags::BrickFormKeptLaw()));
		Card.Choices[0].OnSuccess.Add(EffectOf(EEffectType::DeposeRuler, FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Abdicated")))));
		return Card;
	}
}

// The measurement policies on a crossroads card (B7x header section 5, decision 19): the card is known by its row (a time vector
// asked, or a choice that moves a brick), never by its name; the ruler's ChoiceOf and the wanderer's PickOf take the no-value
// choice alone - a choice that moves a brick, and a value that moves none, are passed by, under every pressure policy (mutation:
// the skip removed - the round's choice); -Crossroads=<k> takes the k-th choice the answer takes that moves a brick first (the
// payable ones - mutation: k counted from 0), the end of the reign let through on it and on no other card (mutation: the
// throne's guard dropped); a crossroads card without the no-value choice is left waiting; the ruler's Act hands its policy's
// Crossroads to ChoiceOf, the wanderer's Act to PickOf (mutation: 0 passed). A card that is no crossroads card is answered as before.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimCrossroadsPolicyTest, "Cosmic_Ascension.Sim.CrossroadsPolicy", CosmicTests::Flags)
bool FCosmicSimCrossroadsPolicyTest::RunTest(const FString& Parameters)
{
	using namespace CosmicCrossroadsPolicyTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	const FEncounterDef First = FirstBench();
	const FEncounterDef Second = SecondBench();
	const FEncounterDef Law = LawBench();
	// No crossroads card: a value and the end of the reign (the throne card's shape).
	FEncounterDef Throne;
	Throne.Choices.AddDefaulted_GetRef().OnSuccess.Add(EffectOf(EEffectType::ChangeStability, FGameplayTag(), 0.05f));
	Throne.Choices.AddDefaulted_GetRef().OnSuccess.Add(EffectOf(EEffectType::DeposeRuler, FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Abdicated")))));
	// A crossroads card with no no-value choice (the schema refuses it - rule 4 - so the policy has nothing safe to take).
	FEncounterDef NoWatch = First;
	NoWatch.Choices.RemoveAt(2);

	// ---- What a crossroads card is ----
	{
		FEncounterDef ByCondition = First;
		for (FEncounterChoice& Choice : ByCondition.Choices)
		{
			Choice.OnSuccess.Reset();
		}
		FEncounterDef ByEffect = First;
		ByEffect.Conditions.Reset();
		TestTrue(TEXT("A card that asks a time vector and moves a brick is a crossroads card"), CosmicRulerPolicy::IsCrossroadsCard(First));
		TestTrue(TEXT("so is one that only asks a time vector (schema rule 5)"), CosmicRulerPolicy::IsCrossroadsCard(ByCondition));
		TestTrue(TEXT("and one that only moves a brick (schema rule 4)"), CosmicRulerPolicy::IsCrossroadsCard(ByEffect));
		TestFalse(TEXT("A card with neither is none (the throne card's shape)"), CosmicRulerPolicy::IsCrossroadsCard(Throne));
		FEncounterChoice OnFailure;
		OnFailure.OnFailure.Add(EffectOf(EEffectType::RemoveTrait, CosmicTags::BrickSpaceNomad()));
		TestTrue(TEXT("A choice moves a brick by giving it, by taking it, or on its failure"), CosmicRulerPolicy::MovesABrickOf(First.Choices[0])
			&& CosmicRulerPolicy::MovesABrickOf(First.Choices[1]) && CosmicRulerPolicy::MovesABrickOf(OnFailure));
		TestFalse(TEXT("but not by a value, nor by nothing"), CosmicRulerPolicy::MovesABrickOf(Second.Choices[1]) || CosmicRulerPolicy::MovesABrickOf(First.Choices[2]));
		FEncounterChoice Costed;
		Costed.Cost.Influence = 1.f;
		FEncounterChoice Checked;
		Checked.CheckStat = ECosmicCheckStat::Stability;
		FEncounterChoice Chained;
		Chained.NextEncounter = FName(TEXT("petition_test_next"));
		TestTrue(TEXT("The no-value choice: no cost, no check, no effect, no next card"), CosmicRulerPolicy::IsNoValueChoice(First.Choices[2]));
		TestFalse(TEXT("a cost, a check, an effect or a next card gives it a value"), CosmicRulerPolicy::IsNoValueChoice(Costed) || CosmicRulerPolicy::IsNoValueChoice(Checked)
			|| CosmicRulerPolicy::IsNoValueChoice(Chained) || CosmicRulerPolicy::IsNoValueChoice(Second.Choices[1]));
		auto Asks = [](EConditionType Type, const FGameplayTag& Tag)
		{
			FCosmicCondition Condition;
			Condition.Type = Type;
			Condition.Tag = Tag;
			return CosmicRulerPolicy::AsksABrickOf(TArray<FCosmicCondition>{ Condition });
		};
		TestTrue(TEXT("Conditions ask a brick: a brick held or lacked, a bordering brick, a time vector"), Asks(EConditionType::CivHasTrait, CosmicTags::BrickSpaceNomad())
			&& Asks(EConditionType::CivLacksTrait, CosmicTags::BrickSpaceNomad()) && Asks(EConditionType::BorderingCivHasTrait, FGameplayTag())
			&& Asks(EConditionType::CivTimeVectorAtLeast, FGameplayTag()));
		TestFalse(TEXT("not a trait that is no brick, nor another condition"), Asks(EConditionType::CivHasTrait, FGameplayTag())
			|| Asks(EConditionType::PlayerRoleIs, FGameplayTag()));
	}

	// ---- The ruler's ChoiceOf on a bench board ----
	Sim->NewGalaxy(42, 5, 3);
	FGalaxyState& State = Sim->GetMutableState();
	const int32 Ruled = State.Civs.IndexOfByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; });
	if (!TestTrue(TEXT("(the first life rules a people)"), State.Civs.IsValidIndex(Ruled)))
	{
		return false;
	}
	State.Civs[Ruled].Stockpile.Matter = 1000.f;
	State.Civs[Ruled].Stockpile.Insight = 1000.f;
	State.Civs[Ruled].Stockpile.Influence = 1000.f;
	auto Choose = [&](const FEncounterDef& Card, int32 Answers, int32 Crossroads, ECosmicPressurePolicy Pressure = ECosmicPressurePolicy::Round)
	{
		return CosmicRulerPolicy::ChoiceOf(State, Sim->GetStateIndex(), Card, Answers, BenchRow, 1, &Sim->GetRules(), &Sim->GetTables(), Pressure, Crossroads);
	};
	bool bWatches = true;
	bool bRoundsElsewhere = true;
	for (int32 Answers = 0; Answers < 3; ++Answers)
	{
		bWatches &= Choose(First, Answers, 0) == 2 && Choose(Second, Answers, 0) == 2;
		bRoundsElsewhere &= Choose(Throne, Answers, 0) == 0;
	}
	TestTrue(TEXT("On a crossroads card the policy takes the no-value choice, whatever the round's turn"), bWatches);
	TestTrue(TEXT("under every pressure policy too"), Choose(First, 0, 0, ECosmicPressurePolicy::Cautious) == 2 && Choose(First, 0, 0, ECosmicPressurePolicy::Reckless) == 2
		&& Choose(First, 0, 0, ECosmicPressurePolicy::Faithful) == 2);
	TestTrue(TEXT("(another card keeps the throne's guard: its value, never the end of the reign)"), bRoundsElsewhere);

	// -Crossroads=<k>.
	TestEqual(TEXT("-Crossroads=1 takes the first choice that moves a brick"), Choose(First, 0, 1), 0);
	TestEqual(TEXT("-Crossroads=2 the second"), Choose(First, 0, 2), 1);
	TestEqual(TEXT("-Crossroads=3, past the last, the no-value choice"), Choose(First, 0, 3), 2);
	TestEqual(TEXT("A value that moves no brick is not counted: the second crossroads' -Crossroads=2 is the no-value choice"), Choose(Second, 0, 2), 2);
	State.Civs[Ruled].Stockpile.Matter = 0.f;
	TestEqual(TEXT("The k-th the answer takes: with no Matter, -Crossroads=1 takes the half step"), Choose(First, 0, 1), 1);
	TestEqual(TEXT("and -Crossroads=2 the no-value choice"), Choose(First, 0, 2), 2);
	State.Civs[Ruled].Stockpile.Matter = 1000.f;
	TestEqual(TEXT("The kept law's card: the no-value choice"), Choose(Law, 0, 0), 2);
	TestEqual(TEXT("-Crossroads=1 lets its end of the reign through"), Choose(Law, 0, 1), 0);
	TestEqual(TEXT("(and on no other card: the throne card's value)"), Choose(Throne, 1, 1), 0);
	TestEqual(TEXT("A crossroads card without the no-value choice is left waiting"), Choose(NoWatch, 0, 0), static_cast<int32>(INDEX_NONE));

	// ---- [B-7x5c] (header section 6): the skip once more on the game's cards - the eleven petitions and the wanderer's card
	// (mutations b7x4m_ruler_skip_removed, b7x4m_wanderer_skip_removed) ----
	{
		const TArray<FName> Petitions = { CosmicTests::B7xRows::PetitionCrossBuilt, CosmicTests::B7xRows::PetitionCrossConsumed,
			CosmicTests::B7xRows::PetitionCrossDrifted, CosmicTests::B7xRows::PetitionCrossIsolated, CosmicTests::B7xRows::PetitionCrossDustBand,
			CosmicTests::B7xRows::PetitionCrossStarLine, CosmicTests::B7xRows::PetitionCrossBetweenStars, CosmicTests::B7xRows::PetitionCrossIntoDust,
			CosmicTests::B7xRows::PetitionCrossWrappedStar, CosmicTests::B7xRows::PetitionCrossLawStays, CosmicTests::B7xRows::PetitionCrossSeam };
		State.Civs[Ruled].Stockpile.Energy = 1000.f;
		int32 Found = 0;
		bool bKnown = true;
		bool bGameWatches = true;
		bool bFirstBrick = true;
		for (const FName& Name : Petitions)
		{
			const FEncounterDef* Row = GameCard(Original, Name);
			if (!Row)
			{
				continue;
			}
			++Found;
			bKnown &= CosmicRulerPolicy::IsCrossroadsCard(*Row);
			for (int32 Answers = 0; Answers < 3; ++Answers)
			{
				bGameWatches &= CosmicRulerPolicy::ChoiceOf(State, Sim->GetStateIndex(), *Row, Answers, Name, 1, &Sim->GetRules(), &Sim->GetTables(),
					ECosmicPressurePolicy::Round, 0) == 2;
			}
			bFirstBrick &= CosmicRulerPolicy::ChoiceOf(State, Sim->GetStateIndex(), *Row, 0, Name, 1, &Sim->GetRules(), &Sim->GetTables(),
				ECosmicPressurePolicy::Round, 1) == 0;
		}
		TestEqual(TEXT("(the game holds the eleven crossroads petitions)"), Found, Petitions.Num());
		TestTrue(TEXT("Each of the game's crossroads petitions is known as one, from its row"), bKnown);
		TestTrue(TEXT("on each the policy takes the no-value choice, whatever the round's turn"), bGameWatches);
		TestTrue(TEXT("and -Crossroads=1 its first choice, the one that moves a brick (the kept law's abdication let through)"), bFirstBrick);
		const FEncounterDef* Fleet = GameCard(Original, CosmicTests::B7xRows::EncCrossDriftFleet);
		if (TestNotNull(TEXT("(the game's wanderer crossroads card)"), Fleet))
		{
			TestTrue(TEXT("The wanderer's crossroads card of the game is one"), CosmicRulerPolicy::IsCrossroadsCard(*Fleet));
			TestTrue(TEXT("the wanderer takes its no-value choice, whatever its turn"), CosmicWandererPolicy::PickOf(*Fleet, 0) == 2
				&& CosmicWandererPolicy::PickOf(*Fleet, 1) == 2 && CosmicWandererPolicy::PickOf(*Fleet, 2) == 2);
			TestEqual(TEXT("and -Crossroads=1 the choice that gives Brick.Space.Nomad"), CosmicWandererPolicy::PickOf(*Fleet, 0, 1), 0);
		}
	}

	// ---- The wanderer's PickOf ----
	bool bPicksWatch = true;
	for (int32 Answers = 0; Answers < 3; ++Answers)
	{
		bPicksWatch &= CosmicWandererPolicy::PickOf(First, Answers) == 2 && CosmicWandererPolicy::PickOf(Second, Answers) == 2;
	}
	TestTrue(TEXT("The wanderer takes the no-value choice of a crossroads card, whatever its turn"), bPicksWatch);
	TestTrue(TEXT("-Crossroads=1, 2 and 3: the first, the second, then the no-value choice"), CosmicWandererPolicy::PickOf(First, 0, 1) == 0
		&& CosmicWandererPolicy::PickOf(First, 0, 2) == 1 && CosmicWandererPolicy::PickOf(First, 0, 3) == 2);
	TestEqual(TEXT("Another card's choices in turn, as before (the count of answers mod the choices)"), CosmicWandererPolicy::PickOf(Throne, 5), 1);
	TestEqual(TEXT("A crossroads card without the no-value choice is left waiting"), CosmicWandererPolicy::PickOf(NoWatch, 0), static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("and a card without choices"), CosmicWandererPolicy::PickOf(FEncounterDef(), 0), static_cast<int32>(INDEX_NONE));

	// ---- Act hands the policy's Crossroads to ChoiceOf (the bench card in the tables, its condition held) ----
	{
		UDataTable* Cards = CosmicTests::CopyWithout<FEncounterDef>(Original.Encounters, {});
		if (!TestNotNull(TEXT("(the game's DT_Encounters)"), Cards))
		{
			return false;
		}
		Cards->AddRow(BenchRow, First);
		FCosmicDataTables Bench = Original;
		Bench.Encounters = Cards;
		Sim->SetTables(Bench);
		Sim->NewGalaxy(42, 5, 3);
		FGalaxyState& Board = Sim->GetMutableState();
		const int32 BoardRuled = Board.Civs.IndexOfByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; });
		if (!TestTrue(TEXT("(the first life rules a people on the bench tables)"), Board.Civs.IsValidIndex(BoardRuled)))
		{
			return false;
		}
		Board.Civs[BoardRuled].TimeVectors.Built = 10;
		Board.Civs[BoardRuled].Stockpile.Matter = 1000.f;
		Board.Civs[BoardRuled].Stockpile.Insight = 1000.f;
		Board.Civs[BoardRuled].Traits.RemoveTag(CosmicTags::BrickSpaceNomad());
		auto Answered = [&](int32 Crossroads)
		{
			Board.Knowing.PendingPetitionRow = BenchRow;
			Board.Knowing.PendingPetitionSystemID = NAME_None;
			FCosmicRulerPolicyCounts Counts;
			const FCosmicRulerPolicy Answering{ true, false, false, false, NAME_None, 0, ECosmicPressurePolicy::Round, 0, false, false, Crossroads };
			CosmicRulerPolicy::Act(Answering, *Sim, *Role, BenchRow, Counts);
			return Counts.Answers == 1 && Board.Knowing.PendingPetitionRow.IsNone();
		};
		const float MatterBefore = Board.Civs[BoardRuled].Stockpile.Matter;
		TestTrue(TEXT("The policy answers the waiting crossroads card"), Answered(0));
		TestTrue(TEXT("with the no-value choice: no brick, nothing paid"), !Board.Civs[BoardRuled].Traits.HasTagExact(CosmicTags::BrickSpaceNomad())
			&& Board.Civs[BoardRuled].Stockpile.Matter == MatterBefore);
		TestTrue(TEXT("With Crossroads 1 it answers again"), Answered(1));
		TestTrue(TEXT("with the choice that gives the brick, paid for"), Board.Civs[BoardRuled].Traits.HasTagExact(CosmicTags::BrickSpaceNomad())
			&& Board.Civs[BoardRuled].Stockpile.Matter == MatterBefore - 100.f);
	}

	// ---- The wanderer's Act hands its policy's Crossroads to PickOf (the bench card as a wanderer's card in the tables, about
	// the system the vessel stands in; its people's condition held; a wanderer pays nothing) ----
	{
		FEncounterDef Wandering = First;
		Wandering.Subject = ECosmicEventSubject::VesselSystem;
		UDataTable* Cards = CosmicTests::CopyWithout<FEncounterDef>(Original.Encounters, {});
		if (!TestNotNull(TEXT("(the game's DT_Encounters, for the wanderer)"), Cards))
		{
			return false;
		}
		Cards->AddRow(WandererBenchRow, Wandering);
		FCosmicDataTables Bench = Original;
		Bench.Encounters = Cards;
		Sim->SetTables(Bench);
		Sim->NewGalaxy(42, 5, 3);
		FGalaxyState& Board = Sim->GetMutableState();
		const int32 Home = Board.Civs.IndexOfByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; });
		const FName HomeID = Board.Civs.IsValidIndex(Home) ? Board.Civs[Home].CivID : NAME_None;
		const int32 HomeSystem = HomeID.IsNone() ? INDEX_NONE : Board.Systems.IndexOfByPredicate([HomeID](const FStarSystemState& System) { return System.OwnerCivID == HomeID; });
		if (!TestTrue(TEXT("(the first life rules a people that holds a system)"), Board.Systems.IsValidIndex(HomeSystem)))
		{
			return false;
		}
		Board.Player.CurrentSystemID = Board.Systems[HomeSystem].SystemID;
		Role->BecomeWanderer(FGameplayTag::RequestGameplayTag(FName(TEXT("AI.Profile.Conservative"))), FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Deposed"))));
		if (!TestTrue(TEXT("(the vessel walks, standing in a system of the people it ruled)"), CosmicWanderer::IsEmbodiedWanderer(Board)
			&& CosmicPetitions::VesselSubjectOf(Board, Sim->GetStateIndex()).CivIndex == Home))
		{
			return false;
		}
		Board.Civs[Home].TimeVectors.Built = 10;
		Board.Civs[Home].Traits.RemoveTag(CosmicTags::BrickSpaceNomad());
		// No throne taken again at the same boundary (the return path) - the card is the test's, not the crown.
		Board.Civs[Home].ThroneLockedUntilTick = Board.Tick + 100000;
		auto Walked = [&](int32 Crossroads)
		{
			// Standing again where its card is (the policy's last step may have set it on its way).
			Board.Player.TravelTargetSystemID = NAME_None;
			Board.Player.TravelTicksRemaining = 0;
			Board.Knowing.PendingPetitionRow = WandererBenchRow;
			Board.Knowing.PendingPetitionSystemID = Board.Systems[HomeSystem].SystemID;
			FCosmicWandererPolicyCounts Counts;
			const FCosmicWandererPolicy Walking{ 0, true, false, Crossroads };
			CosmicWandererPolicy::Act(Walking, *Sim, *Role, WandererBenchRow, Counts);
			return Counts.Answers == 1 && Board.Knowing.PendingPetitionRow.IsNone();
		};
		TestTrue(TEXT("The wanderer's Act answers the waiting crossroads card"), Walked(0));
		TestFalse(TEXT("with the no-value choice: no brick on the people it stands among"), Board.Civs[Home].Traits.HasTagExact(CosmicTags::BrickSpaceNomad()));
		TestTrue(TEXT("With Crossroads 1 the wanderer answers again"), Walked(1));
		TestTrue(TEXT("with the first choice that moves a brick: the brick on that people"), Board.Civs[Home].Traits.HasTagExact(CosmicTags::BrickSpaceNomad()));
	}
	return true;
}

// The game's crossroads cards on the periodic slot ([B-7x5c] - B7x header section 3.1, 6; design section 4.3): a first
// crossroads comes when its time vector reaches the card's threshold and not one count before (mutations: the condition's
// 'at least' read as 'above', or as one count below); it comes once - its answer takes Brick.Form.Society, the mark the four
// first cards ask (mutation: a brick taken off the next people) - and a second crossroads closes on the tag its first choice
// gives; another card waiting keeps it away; and the second choice of a second crossroads that moves no brick is a value
// that does not pile up (decision 11 (na)): its effects are a Stability change, a resource and the Spiritualism lean alone,
// and answered twice it leaves the vessel's stats and the people's bricks as they were (mutation
// b7x5c_data_law_stays_exaltation_cpp). The game's rows, each alone in DT_Encounters.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimCrossroadsPresentTest, "Cosmic_Ascension.Sim.CrossroadsPresent", CosmicTests::Flags)
bool FCosmicSimCrossroadsPresentTest::RunTest(const FString& Parameters)
{
	using namespace CosmicCrossroadsPolicyTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FName BuiltName = CosmicTests::B7xRows::PetitionCrossBuilt;
	const FName StarLineName = CosmicTests::B7xRows::PetitionCrossStarLine;
	const FName LawName = CosmicTests::B7xRows::PetitionCrossLawStays;
	const FEncounterDef* Built = GameCard(Original, BuiltName);
	const FEncounterDef* StarLine = GameCard(Original, StarLineName);
	const FEncounterDef* Law = GameCard(Original, LawName);
	if (!TestNotNull(TEXT("(the game's petition_cross_built)"), Built) || !TestNotNull(TEXT("(the game's petition_cross_star_line)"), StarLine)
		|| !TestNotNull(TEXT("(the game's petition_cross_law_stays)"), Law))
	{
		return false;
	}
	const FGameplayTag Society = CosmicTags::BrickFormSociety();

	// ---- A first crossroads: the threshold, the waiting slot, once ----
	{
		FGalaxyState& State = BoardWith(*Sim, Original, BuiltName, *Built);
		const int32 Ruled = CosmicPetitions::RuledCivIndexOf(State, Sim->GetStateIndex());
		if (!TestTrue(TEXT("(the first life rules a people)"), State.Civs.IsValidIndex(Ruled)))
		{
			return false;
		}
		FCivilizationState& Civ = State.Civs[Ruled];
		TestTrue(TEXT("The first life's people carries Brick.Form.Society (arch_seeker - the first crossroads' mark)"), Civ.Traits.HasTagExact(Society));
		const int32 Threshold = ThresholdOf(*Built);
		if (!TestTrue(TEXT("(petition_cross_built asks Built at least a positive count)"), Threshold > 0))
		{
			return false;
		}
		Civ.TimeVectors.Built = Threshold - 1;
		TestFalse(TEXT("One count of Built below the card's threshold, the slot puts no card"), Slot(*Sim, State));
		Civ.TimeVectors.Built = Threshold;
		TestTrue(TEXT("At the threshold the slot puts it"), Slot(*Sim, State) && State.Knowing.PendingPetitionRow == BuiltName);
		const FName Waiting(TEXT("petition_test_waiting"));
		State.Knowing.PendingPetitionRow = Waiting;
		TestFalse(TEXT("Another card waiting keeps it away"), CosmicPetitions::Present(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(),
			TArray<FCosmicPetitionUnit>()));
		TestEqual(TEXT("(and stays)"), State.Knowing.PendingPetitionRow, Waiting);

		// The four first cards hold on the people while it carries Society, every count past every threshold.
		const TArray<FName> Firsts = { BuiltName, CosmicTests::B7xRows::PetitionCrossConsumed, CosmicTests::B7xRows::PetitionCrossDrifted,
			CosmicTests::B7xRows::PetitionCrossIsolated };
		Civ.TimeVectors.Built = Civ.TimeVectors.Consumed = Civ.TimeVectors.Drifted = Civ.TimeVectors.Isolated = 100000;
		const int32 Capital = CosmicPetitions::SubjectOf(State, Sim->GetStateIndex(), Ruled, ECosmicEventSubject::Civilization, NAME_None);
		auto FirstsHolding = [&]()
		{
			int32 Holding = 0;
			for (const FName& Name : Firsts)
			{
				const FEncounterDef* Row = GameCard(Original, Name);
				Holding += Row && CosmicPetitions::Holds(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), Name, *Row, Ruled, Capital) ? 1 : 0;
			}
			return Holding;
		};
		TestEqual(TEXT("(the four first crossroads hold on the people that carries Society)"), FirstsHolding(), 4);
		// The half step (no check): it takes Society.
		Civ.Stockpile.Insight = 1000.f;
		TestTrue(TEXT("(put again)"), Slot(*Sim, State) && State.Knowing.PendingPetitionRow == BuiltName);
		TestTrue(TEXT("The half step is taken"), Role->AnswerPetition(1).Outcome == ECosmicPetitionOutcome::Succeeded);
		TestFalse(TEXT("and takes Brick.Form.Society"), Civ.Traits.HasTagExact(Society));
		TestEqual(TEXT("so none of the four first crossroads holds any more, every count past its threshold"), FirstsHolding(), 0);
		State.Calling.Lives.Last().LastChoiceRows.Reset();
		TestFalse(TEXT("and the slot does not put the card again (the life's last choices forgotten)"), Slot(*Sim, State));
	}

	// ---- A second crossroads closes on the tag its first choice gives ----
	{
		FGalaxyState& State = BoardWith(*Sim, Original, StarLineName, *StarLine);
		const int32 Ruled = CosmicPetitions::RuledCivIndexOf(State, Sim->GetStateIndex());
		if (!TestTrue(TEXT("(the first life rules a people - the second crossroads)"), State.Civs.IsValidIndex(Ruled)))
		{
			return false;
		}
		FCivilizationState& Civ = State.Civs[Ruled];
		Civ.Tier = EAscensionTier::Tier3_Causal;
		Civ.Traits.AddTag(CosmicTags::BrickSpaceNomad());
		Civ.TimeVectors.Drifted = ThresholdOf(*StarLine);
		Civ.Stability = 1.f;
		Civ.Stockpile.Energy = 1000.f;
		Civ.Stockpile.Matter = 1000.f;
		TestTrue(TEXT("A second crossroads: the third rung, the first's brick and its count put it"), Slot(*Sim, State)
			&& State.Knowing.PendingPetitionRow == StarLineName);
		TestTrue(TEXT("Its first choice is taken (a check the people's Stability passes on every roll)"),
			Role->AnswerPetition(0).Outcome == ECosmicPetitionOutcome::Succeeded);
		TestTrue(TEXT("and gives Brick.Mutation.StarTow"), Civ.Traits.HasTagExact(CosmicTags::BrickMutationStarTow()));
		State.Calling.Lives.Last().LastChoiceRows.Reset();
		TestFalse(TEXT("The card closes on that tag: the slot does not put it again"), Slot(*Sim, State));
	}

	// ---- The value of a second crossroads does not pile up ----
	{
		const TArray<FName> Values = { CosmicTests::B7xRows::PetitionCrossDustBand, StarLineName, CosmicTests::B7xRows::PetitionCrossBetweenStars,
			CosmicTests::B7xRows::PetitionCrossIntoDust, LawName, CosmicTests::B7xRows::PetitionCrossSeam };
		const FName Spiritualism(TEXT("Spiritualism"));
		int32 Found = 0;
		bool bValueOnly = true;
		for (const FName& Name : Values)
		{
			const FEncounterDef* Row = GameCard(Original, Name);
			if (!Row || !Row->Choices.IsValidIndex(1))
			{
				continue;
			}
			++Found;
			const FEncounterChoice& Value = Row->Choices[1];
			bValueOnly &= !CosmicRulerPolicy::MovesABrickOf(Value);
			auto ValueEffects = [&](const TArray<FCosmicEffect>& Effects)
			{
				for (const FCosmicEffect& Effect : Effects)
				{
					bValueOnly &= Effect.Type == EEffectType::ChangeStability || Effect.Type == EEffectType::AddResource
						|| (Effect.Type == EEffectType::ShiftEthics && Effect.Key == Spiritualism);
				}
			};
			ValueEffects(Value.OnSuccess);
			ValueEffects(Value.OnFailure);
		}
		TestEqual(TEXT("(the game holds the six second crossroads whose second choice moves no brick)"), Found, Values.Num());
		TestTrue(TEXT("Their second choice is a Stability change, a resource and the Spiritualism lean alone"), bValueOnly);

		FGalaxyState& State = BoardWith(*Sim, Original, LawName, *Law);
		const int32 Ruled = CosmicPetitions::RuledCivIndexOf(State, Sim->GetStateIndex());
		if (!TestTrue(TEXT("(the first life rules a people - the kept law)"), State.Civs.IsValidIndex(Ruled)))
		{
			return false;
		}
		FCivilizationState& Civ = State.Civs[Ruled];
		Civ.Tier = EAscensionTier::Tier3_Causal;
		Civ.TimeVectors.Endured = ThresholdOf(*Law);
		Civ.Stockpile.Influence = 1000.f;
		const FPlayerEntityState PlayerBefore = State.Player;
		const FGameplayTagContainer TraitsBefore = Civ.Traits;
		bool bAgain = true;
		for (int32 Time = 0; Time < 2; ++Time)
		{
			State.Calling.Lives.Last().LastChoiceRows.Reset();
			bAgain &= Slot(*Sim, State) && State.Knowing.PendingPetitionRow == LawName;
			bAgain &= Role->AnswerPetition(1).Outcome == ECosmicPetitionOutcome::Succeeded;
		}
		TestTrue(TEXT("The kept law's card comes again after its second choice, taken each time"), bAgain);
		TestTrue(TEXT("and the vessel's stats are as they were"), State.Player.Exaltation == PlayerBefore.Exaltation && State.Player.Body == PlayerBefore.Body
			&& State.Player.Mind == PlayerBefore.Mind && State.Player.Causality == PlayerBefore.Causality);
		TestTrue(TEXT("and no brick came"), Civ.Traits == TraitsBefore);
	}
	return true;
}

// The people a crossroads answer reaches ([B-7x5c] - B7x header section 6): a ruler's card puts its brick on the people the
// vessel rules and takes Society from it, a wanderer's card on the owner of the system the vessel stands in - here another
// people's system, so that owner is told apart from the people the vessel ruled (Player.FormerCivID); no other people's
// traits move (mutations: a brick given to the next people, a brick taken off the next people).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimCrossroadsAnswerTest, "Cosmic_Ascension.Sim.CrossroadsAnswer", CosmicTests::Flags)
bool FCosmicSimCrossroadsAnswerTest::RunTest(const FString& Parameters)
{
	using namespace CosmicCrossroadsPolicyTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	auto TraitsOf = [](const FGalaxyState& State)
	{
		TArray<FGameplayTagContainer> Out;
		for (const FCivilizationState& Civ : State.Civs)
		{
			Out.Add(Civ.Traits);
		}
		return Out;
	};
	auto OthersAsBefore = [](const FGalaxyState& State, const TArray<FGameplayTagContainer>& Before, int32 Skip)
	{
		if (State.Civs.Num() != Before.Num())
		{
			return false;
		}
		for (int32 At = 0; At < Before.Num(); ++At)
		{
			if (At != Skip && !(State.Civs[At].Traits == Before[At]))
			{
				return false;
			}
		}
		return true;
	};

	// ---- A ruler's card: petition_cross_built's first choice ----
	{
		const FName Name = CosmicTests::B7xRows::PetitionCrossBuilt;
		const FEncounterDef* Built = GameCard(Original, Name);
		if (!TestNotNull(TEXT("(the game's petition_cross_built)"), Built))
		{
			return false;
		}
		FGalaxyState& State = BoardWith(*Sim, Original, Name, *Built);
		const int32 Ruled = CosmicPetitions::RuledCivIndexOf(State, Sim->GetStateIndex());
		if (!TestTrue(TEXT("(the first life rules a people)"), State.Civs.IsValidIndex(Ruled)))
		{
			return false;
		}
		FCivilizationState& Civ = State.Civs[Ruled];
		Civ.TimeVectors.Built = ThresholdOf(*Built);
		Civ.Stability = 1.f;
		Civ.Stockpile.Matter = 1000.f;
		Civ.Stockpile.Insight = 1000.f;
		const TArray<FGameplayTagContainer> Before = TraitsOf(State);
		TestTrue(TEXT("(the card is put)"), Slot(*Sim, State) && State.Knowing.PendingPetitionRow == Name);
		TestTrue(TEXT("Its first choice is taken (a check the people's Stability passes on every roll)"),
			Role->AnswerPetition(0).Outcome == ECosmicPetitionOutcome::Succeeded);
		TestTrue(TEXT("Brick.Instinct.SwarmTech comes to the people the vessel rules"), Civ.Traits.HasTagExact(CosmicTags::BrickInstinctSwarmTech()));
		TestFalse(TEXT("and Brick.Form.Society leaves it"), Civ.Traits.HasTagExact(CosmicTags::BrickFormSociety()));
		TestTrue(TEXT("No other people's traits moved"), OthersAsBefore(State, Before, Ruled));
	}

	// ---- A wanderer's card: enc_cross_drift_fleet's first choice, on the owner of the system the vessel stands in - another
	// people's settled system, not one of the people the vessel ruled (Player.FormerCivID after BecomeWanderer), so the two are
	// told apart. The game's row without its settled-systems condition: a people holds one system at the start (the condition
	// holds for the slot and the answer alike - StillStands re-checks it - so the copy drops it; the answer's people is what
	// this test reads) ----
	{
		const FName Name = CosmicTests::B7xRows::EncCrossDriftFleet;
		const FEncounterDef* Game = GameCard(Original, Name);
		if (!TestNotNull(TEXT("(the game's enc_cross_drift_fleet)"), Game))
		{
			return false;
		}
		FEncounterDef Fleet = *Game;
		const int32 Dropped = Fleet.Conditions.RemoveAll([](const FCosmicCondition& Condition) { return Condition.Type == EConditionType::CivSettledSystemsAtLeast; });
		TestEqual(TEXT("(the one settled-systems condition taken off the copy)"), Dropped, 1);
		FGalaxyState& Board = BoardWith(*Sim, Original, Name, Fleet);
		const FCosmicStateIndex& StateIndex = Sim->GetStateIndex();
		const int32 Home = Board.Civs.IndexOfByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; });
		const FName HomeID = Board.Civs.IsValidIndex(Home) ? Board.Civs[Home].CivID : NAME_None;
		const int32 HomeSystem = HomeID.IsNone() ? INDEX_NONE : Board.Systems.IndexOfByPredicate([HomeID](const FStarSystemState& System) { return System.OwnerCivID == HomeID; });
		if (!TestTrue(TEXT("(the first life rules a people that holds a system)"), Board.Systems.IsValidIndex(HomeSystem)))
		{
			return false;
		}
		Board.Player.CurrentSystemID = Board.Systems[HomeSystem].SystemID;
		Role->BecomeWanderer(FGameplayTag::RequestGameplayTag(FName(TEXT("AI.Profile.Conservative"))), FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Deposed"))));
		if (!TestTrue(TEXT("(the vessel walks; the people it ruled is its former people)"), CosmicWanderer::IsEmbodiedWanderer(Board)
			&& Board.Player.FormerCivID == HomeID))
		{
			return false;
		}
		// Another people's settled system the card can come on: its people within the wanderer's gap.
		const int32 OtherSystem = Board.Systems.IndexOfByPredicate([&Board, &StateIndex, HomeID](const FStarSystemState& System)
		{
			const int32 Owner = (CosmicState::IsSettled(System) && System.OwnerCivID != HomeID) ? StateIndex.FindCiv(Board, System.OwnerCivID) : INDEX_NONE;
			return Board.Civs.IsValidIndex(Owner) && CosmicPetitions::WithinWandererGap(Board, Owner);
		});
		const int32 Other = Board.Systems.IsValidIndex(OtherSystem) ? StateIndex.FindCiv(Board, Board.Systems[OtherSystem].OwnerCivID) : INDEX_NONE;
		if (!TestTrue(TEXT("(another people holds a settled system within the wanderer's gap)"), Board.Civs.IsValidIndex(Other) && Other != Home))
		{
			return false;
		}
		// The vessel stands there, not travelling.
		Board.Player.CurrentSystemID = Board.Systems[OtherSystem].SystemID;
		Board.Player.TravelTargetSystemID = NAME_None;
		Board.Player.TravelTicksRemaining = 0;
		const FCosmicVesselSubject Vessel = CosmicPetitions::VesselSubjectOf(Board, StateIndex);
		if (!TestTrue(TEXT("(the vessel stands in the other people's system)"), Vessel.SystemIndex == OtherSystem && Vessel.CivIndex == Other))
		{
			return false;
		}
		// The card's people: Drifted at the card's threshold, none of the four bricks the card asks it to lack.
		FCivilizationState& Stood = Board.Civs[Other];
		Stood.TimeVectors.Drifted = ThresholdOf(Fleet);
		Stood.Traits.RemoveTag(CosmicTags::BrickSpaceNomad());
		Stood.Traits.RemoveTag(CosmicTags::BrickSpaceClosedPort());
		Stood.Traits.RemoveTag(CosmicTags::BrickSpaceFoldedSmall());
		Stood.Traits.RemoveTag(CosmicTags::BrickFormSociety());
		// The people the vessel ruled without the brick too, so a brick sent to it shows.
		Board.Civs[Home].Traits.RemoveTag(CosmicTags::BrickSpaceNomad());
		Board.Player.Body = 10;
		const TArray<FGameplayTagContainer> Before = TraitsOf(Board);
		Board.Knowing.PendingPetitionRow = Name;
		Board.Knowing.PendingPetitionSystemID = Board.Systems[OtherSystem].SystemID;
		TestTrue(TEXT("(the card stands on the other people's system)"), CosmicPetitions::StillStands(Board, StateIndex, Sim->GetTables(), Sim->GetRules()));
		TestTrue(TEXT("The wanderer's first choice is taken (a Body check the vessel passes on every roll)"),
			Role->AnswerPetition(0).Outcome == ECosmicPetitionOutcome::Succeeded);
		TestTrue(TEXT("Brick.Space.Nomad comes to the owner of the system the vessel stands in"), Board.Civs[Other].Traits.HasTagExact(CosmicTags::BrickSpaceNomad()));
		TestFalse(TEXT("not to the people the vessel ruled (Player.FormerCivID)"), Board.Civs[Home].Traits.HasTagExact(CosmicTags::BrickSpaceNomad()));
		TestTrue(TEXT("No other people's traits moved (wanderer)"), OthersAsBefore(Board, Before, Other));
	}
	return true;
}

// The commandlet's -SaveAt gate (B7x header section 5: -WithoutB7xData joins the refusal list; -Crossroads, an input's form, does
// not): CosmicBalanceArgs::IsSaveShapeOf writes a slot only beside a run of the game's shape (mutation: -WithoutB7xData left out).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimBalanceArgsTest, "Cosmic_Ascension.Sim.BalanceArgs", CosmicTests::Flags)
bool FCosmicSimBalanceArgsTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("-TellAll=0 with the policy's arguments is the game's shape: -SaveAt writes"),
		CosmicBalanceArgs::IsSaveShapeOf(TEXT("-Seed=7 -Ticks=3000 -TellAll=0 -Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -SaveAt=1200")));
	TestFalse(TEXT("Without -TellAll=0 it does not (every line stored by default)"), CosmicBalanceArgs::IsSaveShapeOf(TEXT("-Seed=7 -SaveAt=1200")));
	TestFalse(TEXT("-WithoutB7xData=1 writes no save"), CosmicBalanceArgs::IsSaveShapeOf(TEXT("-TellAll=0 -WithoutB7xData=1 -SaveAt=1200")));
	TestFalse(TEXT("nor does -WithoutB7xData=2"), CosmicBalanceArgs::IsSaveShapeOf(TEXT("-TellAll=0 -WithoutB7xData=2 -SaveAt=1200")));
	TestTrue(TEXT("-Crossroads is an input's form: -SaveAt still writes"), CosmicBalanceArgs::IsSaveShapeOf(TEXT("-TellAll=0 -Answer=1 -Crossroads=1 -SaveAt=1200")));
	TestFalse(TEXT("(-WithoutB6bData, -FourthRungCap - even 0 - and -Rules write none, as before)"),
		CosmicBalanceArgs::IsSaveShapeOf(TEXT("-TellAll=0 -WithoutB6bData=1")) || CosmicBalanceArgs::IsSaveShapeOf(TEXT("-TellAll=0 -FourthRungCap=0"))
		|| CosmicBalanceArgs::IsSaveShapeOf(TEXT("-TellAll=0 -Rules=ProvisionalEndTick:0")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

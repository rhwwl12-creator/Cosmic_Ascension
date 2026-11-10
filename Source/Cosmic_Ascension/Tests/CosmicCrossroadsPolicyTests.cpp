// Cosmic Ascension - automation tests of B7x's measurement ([B-7x4m]): the ruler's and the wanderer's skip on the crossroads
// cards (B7x header section 5, decision 19) and -Crossroads=<k>, on bench cards the test builds - the game's cards arrive with
// [B-7x5c], whose tests run the skip once more on them (header section 6) -, and the balance commandlet's -SaveAt list.
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

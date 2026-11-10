// Cosmic Ascension - automation tests of B6b's two side rows in the game's data ([B-6b8]): the era of the endured pressure
// (DT_Events event_era_endured - header section 2.7, #156, decision 22) and the wanderer's witness on a claim (DT_Encounters
// enc_witness - header section 2.8, #141, decisions 20 and 21), each read from the game's own row on a board the test lays,
// and the wanderer policy's count of the answers without a check (the race line's witness split, notes section 2.1).
// The vocabulary the era row is written in is Sim.EraSelector (Tests/CosmicUpperRungTests.cpp), its count Sim.PressureEndured
// (Tests/CosmicBookTests.cpp); the wanderer's card rules are Sim.EncounterPresent (Tests/CosmicWandererTests.cpp).
// Design: docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md section 2.7, 2.8; notes section 1.5 (C6, C7).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Events/CosmicPetitions.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicB6bRowNames.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/NameTypes.h"
#include "World/CosmicWandererPolicy.h"

#include <initializer_list>

namespace CosmicSideRowTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	const FName Fallen(TEXT("tr_test_side_fallen"));

	// The game's tables with these event, encounter and hazard rows, one fallen transcendent row of the fourth rung (weight 0:
	// NewGalaxy makes no instance of it), no project or feat, and the knobs of the boards: no inherited edict, a slot every ten
	// ticks with no Lottery chance, the drain as given and no recovery, no drift, no fleet, no settlement, no first crisis,
	// claims one lane out, no arrival card and no periodic petition; the existence bricks inert whatever DT_Rules holds (B7x -
	// CosmicTests::WithInertBricks; LayBoard strips the bricks off the peoples - StripBricks).
	FCosmicDataTables TablesOf(const FCosmicDataTables& Original, const FCosmicRulesDef& BaseRules, float Drain, UDataTable* Events, UDataTable* Encounters,
		UDataTable* Hazards)
	{
		FCosmicRulesDef Rules = CosmicTests::WithInertBricks(CosmicTests::WithoutStartingEdict(BaseRules));
		Rules.EventSlotTicks = 10;
		Rules.EventChancePerSlot = 0.f;
		Rules.SpiritualPressureStabilityDrain = Drain;
		Rules.StabilityRecoveryPerTick = 0.f;
		Rules.RelationDriftRate = 0.f;
		Rules.FleetPerSettledByTier = { 0.f, 0.f, 0.f, 0.f, 0.f };
		Rules.RealmCapByTier = { 1, 1, 1, 1, 1 };
		Rules.FirstCrisisTick = 0;
		Rules.ClaimAdjacencyLanes = 1;
		Rules.EncounterChancePerArrival = 0.f;
		Rules.PetitionPeriodTicks = 100000;
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));
		Tables.Events = Events;
		Tables.Encounters = Encounters;
		Tables.Hazards = Hazards;
		UDataTable* Transcendents = CosmicTests::NewTable(FTranscendentDef::StaticStruct());
		FTranscendentDef Row;
		Row.Doctrine = Tag(TEXT("Doctrine.Silence"));
		Row.Allegiance = Tag(TEXT("Realm.Abyssal"));
		Row.Tier = EAscensionTier::Tier4_Transcendent;
		Row.MaxInfluencedSystems = 12;
		Row.SpawnWeight = 0.f;
		Transcendents->AddRow(Fallen, Row);
		Tables.Transcendents = Transcendents;
		return Tables;
	}

	// The board: the chain of CosmicTests::WireChain (0-1-...-9, 4-10, 10-11). The vessel rules people 0 (4 and 5, capital 4);
	// people 1 holds 3 and 2 (capital 3), people 2 holds 10, people 3 holds 7 and 8 (capital 7) - people 0 borders 1 and 2, not 3
	// (6 is wild). Every people on the first rung at Stability 0.75 with no reverence, no edict, no research and no existence
	// brick (B7x - CosmicTests::StripBricks right after NewGalaxy); no hazard, no project; the one transcendent is the test's
	// fallen, its sphere Sphere (indices into Systems). Null when the galaxy does not give that.
	FGalaxyState* LayBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Tables, std::initializer_list<int32> Sphere)
	{
		Sim.SetTables(Tables);
		Sim.NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim.GetMutableState();
		CosmicTests::StripBricks(State);
		if (State.Civs.Num() < 4 || !CosmicTests::WireChain(State) || State.Player.RuledCivID != State.Civs[0].CivID)
		{
			return nullptr;
		}
		CosmicTests::ClearOwners(State);
		for (FStarSystemState& System : State.Systems)
		{
			CosmicHazards::EmptyHazardSlot(System);
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
		}
		CosmicTests::Give(State, 0, { 4, 5 });
		CosmicTests::Give(State, 1, { 3, 2 });
		CosmicTests::Give(State, 2, { 10 });
		CosmicTests::Give(State, 3, { 7, 8 });
		const int32 Capitals[] = { 4, 3, 10, 7 };
		for (int32 CivIndex = 0; CivIndex < 4; ++CivIndex)
		{
			FCivilizationState& Civ = State.Civs[CivIndex];
			Civ.HomeSystemID = State.Systems[Capitals[CivIndex]].SystemID;
			Civ.Tier = EAscensionTier::Tier1_Planetary;
			Civ.Stability = 0.75f;
			Civ.ReverenceToPlayer = 0.f;
			Civ.ActiveEdicts.Reset();
			for (TPair<FName, float>& Focus : Civ.ResearchFocus)
			{
				Focus.Value = 0.f;
			}
		}
		State.Transcendents.Reset();
		TArray<FName> Members;
		for (const int32 SystemIndex : Sphere)
		{
			Members.Add(State.Systems[SystemIndex].SystemID);
		}
		CosmicTests::AddInstance(State, Fallen, Members).Doctrine = Tag(TEXT("Doctrine.Silence"));
		Sim.RebuildStateIndex();
		Sim.SyncRngState();
		return &State;
	}

	int32 LinesOf(const FGalaxyState& State, FName Row, FName CivID)
	{
		return State.Log.FilterByPredicate([Row, CivID](const FCosmicLogEntry& Entry) { return Entry.SourceRow == Row && Entry.FocusCivID == CivID; }).Num();
	}

	// The seed of the sim stream after one more number.
	int32 SeedAfterOne(const FRandomStream& Stream)
	{
		FRandomStream Copy = Stream;
		Copy.GetFraction();
		return Copy.GetCurrentSeed();
	}
}

// The era of the endured pressure in the game's data (header section 2.7, #156 - decision 22, the answer A): the game's
// event_era_endured alone in DT_Events, on a board whose fallen sphere drains the ruled people and an AI people through a system
// that is not their capital (Sim.PressureEndured's drain, 0.001). The ruled people's count put at 590: at 599 no line; through
// the window of the condition (600-609 under slots of ten) exactly one line about it; each people bordering it gains the
// row's 0.2 from 0 and nobody else gains (mutation b6b8_data_era_borders_0_3 - the row's 0.3); the AI people drained the same
// way, its count through the same window, gets no line (Ruled; mutation b6b8_data_era_no_ruled_past_checker - the shape rule's
// Ruled taken out with the row's); the stretch does not fire again after the window; and the sim
// stream is the one of the same board without the row (the line draws only the tell stream).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimEraEnduredTest, "Cosmic_Ascension.Sim.EraEndured", CosmicTests::Flags)
bool FCosmicSimEraEnduredTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSideRowTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FCosmicEventDef* GameRow = Original.FindEvent(CosmicB6bRows::EventEraEndured);
	if (!TestNotNull(TEXT("The game has the era's row event_era_endured"), GameRow))
	{
		return false;
	}
	UDataTable* Era = CosmicTests::NewTable(FCosmicEventDef::StaticStruct());
	Era->AddRow(CosmicB6bRows::EventEraEndured, *GameRow);
	UDataTable* NoEncounter = CosmicTests::NewTable(FEncounterDef::StaticStruct());

	// One board with the row or without it, the two counts at 590; then 9, 11 and 30 ticks.
	auto Lay = [&](UDataTable* Events) -> FGalaxyState*
	{
		FGalaxyState* State = LayBoard(*Sim, TablesOf(Original, BaseRules, 0.001f, Events, NoEncounter, CosmicTests::NewTable(FHazardDef::StaticStruct())), { 5, 8 });
		if (State)
		{
			State->Civs[0].PressureEnduredTicks = 590;
			State->Civs[3].PressureEnduredTicks = 590;
		}
		return State;
	};
	FGalaxyState* State = Lay(Era);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const FName Ruled = State->Civs[0].CivID;
	const FName AI = State->Civs[3].CivID;
	Sim->RunHeadless(9);
	if (!TestEqual(TEXT("(the ruled people's count runs one a tick under the drain)"), State->Civs[0].PressureEnduredTicks, 599))
	{
		return false;
	}
	TestEqual(TEXT("At 599 there is no era line"), LinesOf(*State, CosmicB6bRows::EventEraEndured, Ruled), 0);
	Sim->RunHeadless(11);
	TestEqual(TEXT("(the counts passed the window)"), State->Civs[0].PressureEnduredTicks, 610);
	TestEqual(TEXT("(the AI people's too)"), State->Civs[3].PressureEnduredTicks, 610);
	TestEqual(TEXT("Through the window the ruled people gets exactly one era line"), LinesOf(*State, CosmicB6bRows::EventEraEndured, Ruled), 1);
	TestEqual(TEXT("An AI people under the same drain gets none"), LinesOf(*State, CosmicB6bRows::EventEraEndured, AI), 0);
	TestEqual(TEXT("A people bordering the ruled people gains the row's 0.2"), State->Civs[1].ReverenceToPlayer, 0.2f);
	TestEqual(TEXT("every people bordering it"), State->Civs[2].ReverenceToPlayer, 0.2f);
	TestEqual(TEXT("the ruled people itself does not"), State->Civs[0].ReverenceToPlayer, 0.f);
	TestEqual(TEXT("nor a people beyond its border"), State->Civs[3].ReverenceToPlayer, 0.f);
	Sim->RunHeadless(30);
	TestEqual(TEXT("After the window the same stretch does not fire again"), LinesOf(*State, CosmicB6bRows::EventEraEndured, Ruled), 1);
	const int32 SeedWith = Sim->GetRng().GetCurrentSeed();
	const int32 CountWith = State->Civs[0].PressureEnduredTicks;

	FGalaxyState* Without = Lay(CosmicTests::NewTable(FCosmicEventDef::StaticStruct()));
	if (TestNotNull(TEXT("The board without the row is laid"), Without))
	{
		Sim->RunHeadless(50);
		TestEqual(TEXT("(the same stretch)"), Without->Civs[0].PressureEnduredTicks, CountWith);
		TestEqual(TEXT("The era's line takes no number of the sim stream"), Sim->GetRng().GetCurrentSeed(), SeedWith);
	}
	return true;
}

// The wanderer's witness in the game's data (header section 2.8, #141 - decisions 20 and 21): the game's enc_witness among the
// game's encounters, on a board whose fallen sphere (system 1) claims the system 2 of people 1. A wanderer of the first rung is
// shown it when a claim lands on the system it stands in (the expiry of a spark there - no arrival, the unit not held at the
// tick's head) and when it arrives on a claimed system of a people one rung above it; not on the land of a people two rungs
// above; not again at once after its answer (a step away and back). The check choice with a Mind that passes on every face
// raises the owner's reverence by the row's 0.1 (mutation b6b8_data_witness_reverence_0_3 - the row's 0.3) and leaves the claim
// and its countdown as they were; the step back raises nothing; each accepted answer draws one number of the sim stream. The
// wanderer policy, answering two witness cards in turn, counts per boundary the answers without a check by the row - none after
// the check, one after the step back - and two answers in all (mutation b6b8_wanderer_nocheck_not_by_row - every answer counted
// as one without a check; b6b8_wanderer_nocheck_reads_next_choice - the choice read after the count moved on;
// b6b8_wanderer_nocheck_inverted - the check counted instead of the step back).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimWitnessCardTest, "Cosmic_Ascension.Sim.WitnessCard", CosmicTests::Flags)
bool FCosmicSimWitnessCardTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSideRowTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FEncounterDef* Witness = Original.FindEncounter(CosmicB6bRows::EncWitness);
	if (!TestNotNull(TEXT("The game has the witness card enc_witness"), Witness) || !TestEqual(TEXT("(two choices)"), Witness->Choices.Num(), 2))
	{
		return false;
	}
	const FGameplayTag Claim = Tag(TEXT("Hazard.Claim"));
	// The hazards of the board: a spark that runs out a tick after it is put (its expiry lets the claim rule place a claim)
	// and the claim the rule places, standing 300 ticks; neither is drawn.
	UDataTable* Hazards = CosmicTests::NewTable(FHazardDef::StaticStruct());
	const TCHAR* const HazardTags[] = { TEXT("Hazard.Plague"), TEXT("Hazard.Claim") };
	for (const TCHAR* const Name : HazardTags)
	{
		FHazardDef Row;
		Row.HazardTag = Tag(Name);
		Row.DisplayName = FText::AsCultureInvariant(FString(Name));
		Row.CountdownTicks = Row.HazardTag == Claim ? 300 : 1;
		Row.SpawnRule = Row.HazardTag == Claim ? ECosmicHazardSpawn::ClaimRule : ECosmicHazardSpawn::Draw;
		Row.SpawnChancePerTick = 0.f;
		Row.LogCategory = Tag(TEXT("Log.Crisis"));
		Row.Severity = 1;
		Row.OutbreakTemplates = { FText::AsCultureInvariant(FString(Name) + TEXT(" outbreak")) };
		Row.ExpireTemplates = { FText::AsCultureInvariant(FString(Name) + TEXT(" expiry")) };
		Row.ResolveTemplates = { FText::AsCultureInvariant(FString(Name) + TEXT(" resolution")) };
		Hazards->AddRow(FName(*FString::Printf(TEXT("crisis_test_%s"), Row.HazardTag == Claim ? TEXT("claim") : TEXT("spark"))), Row);
	}
	const FCosmicDataTables Tables = TablesOf(Original, BaseRules, 0.f, CosmicTests::NewTable(FCosmicEventDef::StaticStruct()), Original.Encounters.Get(), Hazards);
	const int32 Lane = CosmicWanderer::LaneTicksOf(Tables.FindRules() ? *Tables.FindRules() : BaseRules);
	// The board with people 1 on Rung, the vessel deposed and walking from Start with a Mind of 3 (the check's Difficulty);
	// with bClaimed, the claim stands on 2 already.
	auto Lay = [&](EAscensionTier Rung, int32 Start, bool bClaimed) -> FGalaxyState*
	{
		FGalaxyState* State = LayBoard(*Sim, Tables, { 1 });
		if (!State)
		{
			return nullptr;
		}
		State->Civs[1].Tier = Rung;
		State->Calling.PersonalTier = EAscensionTier::Tier1_Planetary;
		State->Player.CurrentSystemID = State->Systems[Start].SystemID;
		Role->BecomeWanderer(Tag(TEXT("AI.Profile.Conservative")), Tag(TEXT("Reign.Deposed")));
		State->Player.Mind = 3;
		if (bClaimed)
		{
			State->Systems[2].ActiveHazard = Claim;
			State->Systems[2].HazardTicksRemaining = 300;
		}
		return CosmicWanderer::IsEmbodiedWanderer(*State) ? State : nullptr;
	};
	auto Shown = [](const FGalaxyState& State) { return State.Knowing.PendingPetitionRow == CosmicB6bRows::EncWitness && State.Knowing.PendingPetitionSystemID == State.Systems[2].SystemID; };
	auto Go = [&](FGalaxyState& State, int32 To)
	{
		Role->TravelTo(State.Systems[To].SystemID);
		Sim->RunHeadless(Lane);
		return State.Player.CurrentSystemID == State.Systems[To].SystemID && !CosmicWanderer::IsTravelling(State);
	};

	// 1. A claim lands where the vessel stands (people 1 on its rung): the card at the tick of the landing. Then the check.
	{
		FGalaxyState* State = Lay(EAscensionTier::Tier1_Planetary, 2, false);
		if (!TestNotNull(TEXT("The board of the landing is laid"), State))
		{
			return false;
		}
		State->Systems[2].ActiveHazard = Tag(TEXT("Hazard.Plague"));
		State->Systems[2].HazardTicksRemaining = 1;
		Sim->StepOnce();
		if (!TestTrue(TEXT("(the spark's expiry placed the claim on the system the vessel stands in)"), State->Systems[2].ActiveHazard == Claim))
		{
			return false;
		}
		TestTrue(TEXT("A claim that lands where the wanderer stands brings the witness"), Shown(*State));
		TestEqual(TEXT("(the Mind passes the check on every face)"), CosmicPetitions::PassingRollsOf(*State, 1, Witness->Choices[0]), 6);
		const float Reverence = State->Civs[1].ReverenceToPlayer;
		const int32 Countdown = State->Systems[2].HazardTicksRemaining;
		const int32 Seed = SeedAfterOne(Sim->GetRng());
		const FCosmicPetitionAnswer Taken = Role->AnswerPetition(0);
		TestTrue(TEXT("The check choice passes"), Taken.Outcome == ECosmicPetitionOutcome::Succeeded);
		TestEqual(TEXT("and raises the owner's reverence by the row's 0.1"), State->Civs[1].ReverenceToPlayer, Reverence + 0.1f);
		TestTrue(TEXT("The claim still stands"), State->Systems[2].ActiveHazard == Claim);
		TestEqual(TEXT("with its countdown unchanged"), State->Systems[2].HazardTicksRemaining, Countdown);
		TestEqual(TEXT("The answer draws one number of the sim stream"), Sim->GetRng().GetCurrentSeed(), Seed);
		// A step away and back: the answered card does not come again at once.
		if (TestTrue(TEXT("(the vessel steps to 1)"), Go(*State, 1)) && TestTrue(TEXT("(and back to 2)"), Go(*State, 2)))
		{
			TestTrue(TEXT("Right after its answer the witness is not shown again"), State->Knowing.PendingPetitionRow.IsNone());
		}
	}

	// 2. A people two rungs above: the arrival on its claimed system brings no card.
	{
		FGalaxyState* State = Lay(EAscensionTier::Tier3_Causal, 3, true);
		if (TestNotNull(TEXT("The board of the third rung is laid"), State) && TestTrue(TEXT("(the vessel arrives on 2)"), Go(*State, 2)))
		{
			TestTrue(TEXT("On the land of a people two rungs above the wanderer no witness comes"), State->Knowing.PendingPetitionRow.IsNone());
		}
	}

	// 3. A people one rung above: the arrival brings the card. Then the step back, and the policy.
	FGalaxyState* State = Lay(EAscensionTier::Tier2_Stellar, 3, true);
	if (!TestNotNull(TEXT("The board of the second rung is laid"), State))
	{
		return false;
	}
	Role->TravelTo(State->Systems[2].SystemID);
	Sim->RunHeadless(Lane - 1);
	TestTrue(TEXT("(no card on the way)"), State->Knowing.PendingPetitionRow.IsNone());
	Sim->StepOnce();
	TestTrue(TEXT("The arrival on a claimed system of a people one rung above brings the witness"), Shown(*State));
	{
		const float Reverence = State->Civs[1].ReverenceToPlayer;
		const int32 Seed = SeedAfterOne(Sim->GetRng());
		const FCosmicPetitionAnswer Taken = Role->AnswerPetition(1);
		TestTrue(TEXT("The step back is taken"), Taken.Outcome == ECosmicPetitionOutcome::Succeeded);
		TestEqual(TEXT("and raises no reverence"), State->Civs[1].ReverenceToPlayer, Reverence);
		TestEqual(TEXT("(one number of the sim stream)"), Sim->GetRng().GetCurrentSeed(), Seed);
		TestTrue(TEXT("(the claim stands)"), State->Systems[2].ActiveHazard == Claim);
	}
	// The policy answers a witness card at two boundaries (the card put by hand where the vessel stands, the journey the first
	// boundary began taken back): the first answer takes the check, the second the step back - the count is read per boundary.
	const FCosmicWandererPolicy Walks{ 0, true };
	FCosmicWandererPolicyCounts Counts;
	for (int32 Boundary = 0; Boundary < 2; ++Boundary)
	{
		State->Player.CurrentSystemID = State->Systems[2].SystemID;
		State->Player.TravelTargetSystemID = NAME_None;
		State->Player.TravelTicksRemaining = 0;
		State->Knowing.PendingPetitionRow = CosmicB6bRows::EncWitness;
		State->Knowing.PendingPetitionSystemID = State->Systems[2].SystemID;
		CosmicWandererPolicy::Act(Walks, *Sim, *Role, NAME_None, Counts);
		TestEqual(*FString::Printf(TEXT("(after boundary %d: the answers without a check by the row - the check first, then the step back)"), Boundary),
			Counts.NoCheckByRow.FindRef(CosmicB6bRows::EncWitness), Boundary);
	}
	const FName Row = CosmicB6bRows::EncWitness;
	TestTrue(TEXT("The policy counts both witness cards and both answers by the row"), Counts.CardsByRow.FindRef(Row) == 2 && Counts.AnswersByRow.FindRef(Row) == 2);
	TestEqual(TEXT("and one answer without a check - the step back"), Counts.NoCheckByRow.FindRef(Row), 1);
	TestTrue(TEXT("(the check passed; nothing failed or went before its answer)"), Counts.FailedByRow.FindRef(Row) == 0 && Counts.WithdrawnByRow.IsEmpty());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

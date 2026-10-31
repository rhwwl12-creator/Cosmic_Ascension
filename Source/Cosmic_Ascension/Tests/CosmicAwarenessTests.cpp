// Cosmic Ascension - automation tests of the awareness (milestone B item B6a, commit [B-6a3a]): the claim rule of the crisis
// stage makes the fallen that claims a settled system of the people the embodied ruler holds on the third rung or above
// aware of the player - once per transcendent, from the world alone, without a draw.
// Design: docs/60_DECISIONS/headers/B6a_endings_and_the_run_past_the_third_rung.md section 2.11, 6 and decision 7.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicHazards.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/NameTypes.h"

#include <initializer_list>

namespace CosmicAwarenessTests
{
	FGameplayTag AwareTag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// The rows of these tests.
	const FName SparkRow(TEXT("crisis_test_aware_spark"));
	const FName ClaimRow(TEXT("crisis_test_aware_claim"));
	const FName FirstFallen(TEXT("tr_test_aware_first"));
	const FName SecondFallen(TEXT("tr_test_aware_second"));

	// The systems of the board (LayAwareBoard), by index into State.Systems.
	constexpr int32 VesselSystem = 0;   // people 0, settled - the ruler stands here
	constexpr int32 ClaimedSystem = 2;  // people 0, settled, one lane from the sphere - the first expiry runs out here
	constexpr int32 SphereSystem = 3;   // nobody's - every fallen of the board holds it in its sphere
	constexpr int32 LaterSystem = 4;    // people 0, settled, one lane from the sphere - a later expiry runs out here

	// The leap of the tests' rules: a death is followed by a succession this many ticks later.
	constexpr int32 Leap = 5;

	// A hazard row of the tests: no spawn chance, three lines for each moment.
	FHazardDef HazardRowOf(const TCHAR* TagName, ECosmicHazardSpawn Rule, int32 Countdown)
	{
		auto Lines = [TagName](const TCHAR* Moment)
		{
			const FString Stem = FString::Printf(TEXT("%s %s"), TagName, Moment);
			return TArray<FText>{ FText::AsCultureInvariant(Stem + TEXT(" one")), FText::AsCultureInvariant(Stem + TEXT(" two")), FText::AsCultureInvariant(Stem + TEXT(" three")) };
		};
		FHazardDef Row;
		Row.HazardTag = AwareTag(TagName);
		Row.DisplayName = FText::AsCultureInvariant(FString::Printf(TEXT("<%s>"), TagName));
		Row.CountdownTicks = Countdown;
		Row.SpawnRule = Rule;
		Row.SpawnChancePerTick = 0.f;
		Row.LogCategory = AwareTag(TEXT("Log.Crisis"));
		Row.Severity = 1;
		Row.OutbreakTemplates = Lines(TEXT("outbreak"));
		Row.ExpireTemplates = Lines(TEXT("expiry"));
		Row.ResolveTemplates = Lines(TEXT("resolution"));
		return Row;
	}

	// A fallen row of tier 4 with a sphere of twelve; its weight 0, so the roster of NewGalaxy makes no instance of it.
	FTranscendentDef FallenRowOf(const TCHAR* Doctrine, float Depth)
	{
		FTranscendentDef Row;
		Row.Doctrine = AwareTag(Doctrine);
		Row.Allegiance = AwareTag(TEXT("Realm.Abyssal"));
		Row.Tier = EAscensionTier::Tier4_Transcendent;
		Row.Depth = Depth;
		Row.MaxInfluencedSystems = 12;
		Row.SpawnWeight = 0.f;
		return Row;
	}

	// The game's tables made quiet for the board: no inherited edict, no event slot, no first crisis, no periodic petition,
	// no drift and no fleet, no pressure drain, a realm cap of one (no people settles), claims one lane out, a leap of exactly
	// Leap ticks, no project or feat (CosmicTests::WithoutProjects), no event or card row. The hazards: a plague of the
	// tests (one tick; with bDeposing its expiry deposes the ruler of its owner - DeposeRuler, Reign.Deposed) and the claim
	// row (300 ticks: no claim of a case runs out). The transcendents: two fallen rows, the second deeper (it claims over the
	// first where both reach).
	FCosmicDataTables TablesOf(const FCosmicDataTables& Original, const FCosmicRulesDef& GameRules, bool bDeposing)
	{
		FCosmicRulesDef Rules = CosmicTests::WithoutStartingEdict(GameRules);
		Rules.EventChancePerSlot = 0.f;
		Rules.FirstCrisisTick = 0;
		Rules.PetitionPeriodTicks = 100000;
		Rules.RealmCapByTier = { 1, 1, 1, 1, 1 };
		Rules.RelationDriftRate = 0.f;
		Rules.FleetPerSettledByTier = { 0.f, 0.f, 0.f, 0.f, 0.f };
		Rules.SpiritualPressureStabilityDrain = 0.f;
		Rules.ClaimAdjacencyLanes = 1;
		Rules.LeapTicksMin = Leap;
		Rules.LeapTicksMax = Leap;
		Rules.LeapTotalCap = 1000000;
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));

		UDataTable* Hazards = CosmicTests::NewTable(FHazardDef::StaticStruct());
		FHazardDef Spark = HazardRowOf(TEXT("Hazard.Plague"), ECosmicHazardSpawn::Draw, 1);
		if (bDeposing)
		{
			FCosmicEffect Depose;
			Depose.Type = EEffectType::DeposeRuler;
			Depose.Tag = AwareTag(TEXT("Reign.Deposed"));
			Spark.OnExpireEffects = { Depose };
		}
		Hazards->AddRow(SparkRow, Spark);
		Hazards->AddRow(ClaimRow, HazardRowOf(TEXT("Hazard.Claim"), ECosmicHazardSpawn::ClaimRule, 300));
		Tables.Hazards = Hazards;

		UDataTable* Transcendents = CosmicTests::NewTable(FTranscendentDef::StaticStruct());
		Transcendents->AddRow(FirstFallen, FallenRowOf(TEXT("Doctrine.Silence"), 0.f));
		Transcendents->AddRow(SecondFallen, FallenRowOf(TEXT("Doctrine.Zeal"), 1.f));
		Tables.Transcendents = Transcendents;

		Tables.Events = CosmicTests::NewTable(FCosmicEventDef::StaticStruct());
		Tables.Encounters = CosmicTests::NewTable(FEncounterDef::StaticStruct());
		return Tables;
	}

	// The plague of the tests on System, running out at the next tick: the claim rule follows its expiry.
	void ExpireOn(FGalaxyState& State, int32 System)
	{
		State.Systems[System].ActiveHazard = AwareTag(TEXT("Hazard.Plague"));
		State.Systems[System].HazardTicksRemaining = 1;
	}

	// A death recorded on the live state as Apply records one: the tenth stage of the next tick carries it out.
	void RecordDeathByHand(FGalaxyState& State)
	{
		State.Calling.PendingEffect = CosmicTags::EffectKillPlayer();
		State.Calling.PendingCause = AwareTag(TEXT("Death.Hazard"));
		State.Calling.PendingSourceRow = SparkRow;
	}

	// The board on the live state (the simulation's tables are the tests'): a galaxy of seed 42 with twelve living systems
	// wired as CosmicTests::WireChain (0-1-...-9, 4-10, 10-11); people 0 - the vessel's - holds 0, 1, 2 and 4 with its capital
	// in 0 and stands on Rung; people 1 holds 6, people 2 holds 7, people 3 holds 11; every Stability 0.75; no hazard, project
	// or notable; the instances of Fallen (in that order, unaware, unsealed) hold SphereSystem; the vessel rules people 0
	// standing in VesselSystem; tick 400 (past the birth protection). The plague of the tests runs out on ClaimedSystem at
	// the next tick. Null when the galaxy does not give that.
	FGalaxyState* LayAwareBoard(UGalaxySimulationSubsystem& Sim, EAscensionTier Rung, std::initializer_list<FName> Fallen)
	{
		Sim.NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim.GetMutableState();
		if (CosmicTests::LivingSystems(State) < 12 || State.Civs.Num() < 4 || State.Player.RuledCivID != State.Civs[0].CivID
			|| State.Calling.LifeState != ELifeState::Embodied || !CosmicTests::WireChain(State))
		{
			return nullptr;
		}
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { VesselSystem, 1, ClaimedSystem, LaterSystem });
		CosmicTests::Give(State, 1, { 6 });
		CosmicTests::Give(State, 2, { 7 });
		CosmicTests::Give(State, 3, { 11 });
		for (FStarSystemState& System : State.Systems)
		{
			CosmicHazards::EmptyHazardSlot(System);
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
		}
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.Stability = 0.75f;
			Civ.LowStabilityTicks = 0;
			Civ.Notables.Reset();
		}
		State.Civs[0].Tier = Rung;
		State.Transcendents.Reset();
		for (const FName ID : Fallen)
		{
			const FTranscendentDef* Def = Sim.GetTables().FindTranscendent(ID);
			if (!Def)
			{
				return nullptr;
			}
			FTranscendentState& Instance = CosmicTests::AddInstance(State, ID, { State.Systems[SphereSystem].SystemID });
			Instance.Doctrine = Def->Doctrine;
		}
		State.Player.CurrentSystemID = State.Systems[VesselSystem].SystemID;
		State.Tick = 400;
		ExpireOn(State, ClaimedSystem);
		return &State;
	}

	// How many transcendents of the state are aware of the player.
	int32 AwareCount(const FGalaxyState& State)
	{
		int32 Count = 0;
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			Count += Transcendent.bAwareOfPlayer ? 1 : 0;
		}
		return Count;
	}

	// Whether any system holds a claim.
	bool AnyClaim(const FGalaxyState& State)
	{
		const FGameplayTag Claim = AwareTag(TEXT("Hazard.Claim"));
		return State.Systems.ContainsByPredicate([&Claim](const FStarSystemState& System) { return System.ActiveHazard == Claim; });
	}
}

// Pins the awareness (B6a header section 2.11, section 6 Sim.AwarenessOnce, decision 7-ga/7-na): in the tick a claim is placed
// on a settled system of the people the embodied ruler holds on the third rung or above, the claimant - and only the claimant
// - becomes aware of the player. Not on the second rung (mutation: dropping the rung gate breaks it), not for a wanderer, not
// by the rung alone; not when this tick's expiry recorded a deposition before the claim, nor when a death waits (mutation:
// dropping the PendingEffect gate breaks both); with two fallen rows the second transcendent's first claim sets its own flag;
// the flag survives a succession; the knowing does not change the answer; the write draws nothing and changes nothing else
// (the twin whose flag was set before the tick ends in the same sim stream and the same world).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicAwarenessOnceTest, "Cosmic_Ascension.Sim.AwarenessOnce", CosmicTests::Flags)
bool FCosmicAwarenessOnceTest::RunTest(const FString& Parameters)
{
	using namespace CosmicAwarenessTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef GameRules = Sim->GetRules();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	const FCosmicDataTables Plain = TablesOf(Original, GameRules, false);
	const FCosmicDataTables Deposing = TablesOf(Original, GameRules, true);
	const FGameplayTag Claim = AwareTag(TEXT("Hazard.Claim"));
	const FGalaxyState& State = Sim->GetState();
	Sim->SetTables(Plain);

	// (a) The third rung: the claimant is aware in the tick of the claim. Then (e) the flag through a death and a succession.
	int32 SeedAfter = 0;
	uint32 WorldAfter = 0;
	{
		FGalaxyState* Laid = LayAwareBoard(*Sim, EAscensionTier::Tier3_Causal, { FirstFallen });
		if (!TestNotNull(TEXT("(the board of the third rung is laid)"), Laid))
		{
			return false;
		}
		TestEqual(TEXT("(nobody is aware before the claim)"), AwareCount(State), 0);
		Sim->RunHeadless(1);
		if (!TestTrue(TEXT("(the expiry next to the sphere placed a claim on the ruled people's system)"), State.Systems[ClaimedSystem].ActiveHazard == Claim))
		{
			return false;
		}
		TestTrue(TEXT("A claim on a settled system of the ruled people on the third rung makes its claimant aware in that tick"), State.Transcendents[0].bAwareOfPlayer);
		SeedAfter = Sim->GetRng().GetCurrentSeed();
		WorldAfter = Sim->ComputeWorldHash();

		RecordDeathByHand(*Laid);
		Sim->RunHeadless(1);
		if (TestTrue(TEXT("(the ruler died: the calling is in the interregnum)"), State.Calling.LifeState == ELifeState::Interregnum && State.Calling.PendingLeapTicks == Leap))
		{
			TestTrue(TEXT("The flag stays through the death"), State.Transcendents[0].bAwareOfPlayer);
			Sim->RunHeadless(Leap);
			if (TestTrue(TEXT("(a new vessel was called at the leap's end)"), State.Calling.LifeState == ELifeState::Embodied && State.Calling.Lives.Num() == 2))
			{
				TestTrue(TEXT("The flag survives the succession"), State.Transcendents[0].bAwareOfPlayer);
				Sim->RunHeadless(1);
				TestTrue(TEXT("and nothing takes it back after it"), State.Transcendents[0].bAwareOfPlayer);
			}
		}
	}

	// (g) The twin whose fallen was aware before the tick: the claim writes nothing, and the run ends in the same sim stream
	// and the same world as the run that wrote the flag (mutation: a draw in the write breaks the stream).
	{
		FGalaxyState* Laid = LayAwareBoard(*Sim, EAscensionTier::Tier3_Causal, { FirstFallen });
		if (!TestNotNull(TEXT("(the board of the aware twin is laid)"), Laid))
		{
			return false;
		}
		Laid->Transcendents[0].bAwareOfPlayer = true;
		Sim->RunHeadless(1);
		TestTrue(TEXT("(the twin's claim is placed)"), State.Systems[ClaimedSystem].ActiveHazard == Claim);
		TestTrue(TEXT("A fallen aware already stays aware"), State.Transcendents[0].bAwareOfPlayer);
		TestEqual(TEXT("The write draws nothing: the sim stream is where the twin's is"), Sim->GetRng().GetCurrentSeed(), SeedAfter);
		TestTrue(TEXT("and the flag is all it writes: the world is the twin's"), Sim->ComputeWorldHash() == WorldAfter);
	}

	// (f) The knowing is not read: with nothing known and nobody met, the same answer, the same stream, the same world.
	{
		FGalaxyState* Laid = LayAwareBoard(*Sim, EAscensionTier::Tier3_Causal, { FirstFallen });
		if (!TestNotNull(TEXT("(the board without knowing is laid)"), Laid))
		{
			return false;
		}
		Laid->Knowing.KnownSystems.Reset();
		Laid->Knowing.MetCivs.Reset();
		Sim->RunHeadless(1);
		TestTrue(TEXT("(the claim is placed without knowing)"), State.Systems[ClaimedSystem].ActiveHazard == Claim);
		TestTrue(TEXT("Knowing nothing, the claimant is aware all the same"), State.Transcendents[0].bAwareOfPlayer);
		TestEqual(TEXT("with the same sim stream"), Sim->GetRng().GetCurrentSeed(), SeedAfter);
		TestTrue(TEXT("and the same world"), Sim->ComputeWorldHash() == WorldAfter);
	}

	// (b) The second rung: no awareness, even with the whole board known (mutation: dropping the rung gate breaks it).
	{
		FGalaxyState* Laid = LayAwareBoard(*Sim, EAscensionTier::Tier2_Stellar, { FirstFallen });
		if (!TestNotNull(TEXT("(the board of the second rung is laid)"), Laid))
		{
			return false;
		}
		CosmicTests::KnowAll(*Laid);
		Sim->RunHeadless(1);
		TestTrue(TEXT("(the claim on the second rung is placed)"), State.Systems[ClaimedSystem].ActiveHazard == Claim);
		TestEqual(TEXT("A claim on the ruled people's system on the second rung makes nobody aware"), AwareCount(State), 0);
	}

	// (a) Above the third rung: the fourth rung is past the gate too (mutation: the gate written as 'exactly the third rung'
	// breaks it).
	{
		FGalaxyState* Laid = LayAwareBoard(*Sim, EAscensionTier::Tier4_Transcendent, { FirstFallen });
		if (!TestNotNull(TEXT("(the board of the fourth rung is laid)"), Laid))
		{
			return false;
		}
		Sim->RunHeadless(1);
		TestTrue(TEXT("(the claim on the fourth rung is placed)"), State.Systems[ClaimedSystem].ActiveHazard == Claim);
		TestTrue(TEXT("A claim on the ruled people's system on the fourth rung makes its claimant aware"), State.Transcendents[0].bAwareOfPlayer);
	}

	// (b) Another people's system: a claim on a settled system of a people on the third rung that the vessel does not rule
	// makes nobody aware, while the ruled people stands on the second (mutation: dropping the owner gate breaks it - the rung
	// gate would then read the claimed system's owner).
	{
		FGalaxyState* Laid = LayAwareBoard(*Sim, EAscensionTier::Tier2_Stellar, { FirstFallen });
		if (!TestNotNull(TEXT("(the board of another people's claim is laid)"), Laid))
		{
			return false;
		}
		CosmicHazards::EmptyHazardSlot(Laid->Systems[ClaimedSystem]);
		Laid->Civs[0].OwnedSystems.Remove(Laid->Systems[LaterSystem].SystemID);
		CosmicTests::Give(*Laid, 1, { LaterSystem });
		Laid->Civs[1].Tier = EAscensionTier::Tier3_Causal;
		ExpireOn(*Laid, LaterSystem);
		Sim->RunHeadless(1);
		TestTrue(TEXT("(the claim on the other people's system is placed)"), State.Systems[LaterSystem].ActiveHazard == Claim);
		TestEqual(TEXT("A claim on a system of a people the vessel does not rule makes nobody aware"), AwareCount(State), 0);
	}

	// (b) The rung alone: a people that comes to the third rung with no claim placed makes nobody aware.
	{
		FGalaxyState* Laid = LayAwareBoard(*Sim, EAscensionTier::Tier2_Stellar, { FirstFallen });
		if (!TestNotNull(TEXT("(the board of the rung alone is laid)"), Laid))
		{
			return false;
		}
		CosmicHazards::EmptyHazardSlot(Laid->Systems[ClaimedSystem]);
		Sim->RunHeadless(1);
		Laid->Civs[0].Tier = EAscensionTier::Tier3_Causal;
		Sim->RunHeadless(20);
		TestFalse(TEXT("(no claim is placed without an expiry)"), AnyClaim(State));
		TestEqual(TEXT("The third rung alone makes nobody aware"), AwareCount(State), 0);
	}

	// (c) A deposition this tick's expiry recorded before its claim: nobody aware (mutation: dropping the PendingEffect gate
	// breaks it). Then (b) the wanderer the deposition made: a claim on its old people on the third rung makes nobody aware.
	Sim->SetTables(Deposing);
	{
		FGalaxyState* Laid = LayAwareBoard(*Sim, EAscensionTier::Tier3_Causal, { FirstFallen });
		if (!TestNotNull(TEXT("(the board of the deposition is laid)"), Laid))
		{
			return false;
		}
		Sim->RunHeadless(1);
		TestTrue(TEXT("(the deposing expiry placed its claim)"), State.Systems[ClaimedSystem].ActiveHazard == Claim);
		TestEqual(TEXT("A claim placed after the expiry recorded a deposition makes nobody aware"), AwareCount(State), 0);
		if (TestTrue(TEXT("(the deposition was carried out at the tenth stage: the vessel wanders)"), State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Wanderer))
		{
			ExpireOn(*Laid, LaterSystem);
			Sim->RunHeadless(1);
			TestTrue(TEXT("(the later expiry placed a claim on another system of the old people)"), State.Systems[LaterSystem].ActiveHazard == Claim);
			TestEqual(TEXT("A wanderer is not noticed through the people it ruled"), AwareCount(State), 0);
		}
	}
	Sim->SetTables(Plain);

	// (c) A death recorded before the tick: the claim of that tick makes nobody aware (mutation: dropping the PendingEffect
	// gate breaks it).
	{
		FGalaxyState* Laid = LayAwareBoard(*Sim, EAscensionTier::Tier3_Causal, { FirstFallen });
		if (!TestNotNull(TEXT("(the board of the death is laid)"), Laid))
		{
			return false;
		}
		RecordDeathByHand(*Laid);
		Sim->RunHeadless(1);
		TestTrue(TEXT("(the claim of the death's tick is placed)"), State.Systems[ClaimedSystem].ActiveHazard == Claim);
		TestEqual(TEXT("A claim placed while a death waits makes nobody aware"), AwareCount(State), 0);
		TestTrue(TEXT("(the death was carried out at the tenth stage)"), State.Calling.LifeState == ELifeState::Interregnum);
	}

	// (d) Two fallen rows: the claimant is the one made aware, and the second transcendent's first claim sets its own flag
	// though the first is aware already.
	{
		FGalaxyState* Laid = LayAwareBoard(*Sim, EAscensionTier::Tier3_Causal, { FirstFallen, SecondFallen });
		if (!TestNotNull(TEXT("(the board of two fallen is laid)"), Laid))
		{
			return false;
		}
		Sim->RunHeadless(1);
		TestTrue(TEXT("(the claim of two fallen is placed)"), State.Systems[ClaimedSystem].ActiveHazard == Claim);
		TestTrue(TEXT("Of two fallen, the claimant - the deeper, second instance - is made aware"), State.Transcendents[1].bAwareOfPlayer);
		TestFalse(TEXT("and not the other"), State.Transcendents[0].bAwareOfPlayer);
	}
	{
		FGalaxyState* Laid = LayAwareBoard(*Sim, EAscensionTier::Tier3_Causal, { FirstFallen, SecondFallen });
		if (!TestNotNull(TEXT("(the board of two fallen, one aware, is laid)"), Laid))
		{
			return false;
		}
		Laid->Transcendents[0].bAwareOfPlayer = true;
		Sim->RunHeadless(1);
		TestTrue(TEXT("(the second fallen's claim is placed)"), State.Systems[ClaimedSystem].ActiveHazard == Claim);
		TestTrue(TEXT("The second transcendent's first claim sets its own flag"), State.Transcendents[1].bAwareOfPlayer);
		TestTrue(TEXT("and the first stays aware"), State.Transcendents[0].bAwareOfPlayer);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

// Cosmic Ascension - automation tests of succession (milestone B item B5, commit [B-5b]): the single window of where a new
// vessel may be born after a death (CosmicSuccession::CandidatesOf, PickOf), the guarantee that a death in an input-less run
// finds a place whenever the world holds one, the silence when it holds none, and the references a succession leaves behind.
// Design: docs/60_DECISIONS/headers/B5_death_succession_two_books.md section 2.1, 2.2, 2.4, 2.5, 2.6, 2.11 and 6.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicIDs.h"
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicSuccession.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/Function.h"
#include "Tests/CosmicTestHelpers.h"

#include <initializer_list>
#include <limits>

namespace CosmicSuccessionTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// "When: what" - the text of an assertion made at one moment of a run.
	FString Says(const FString& When, const TCHAR* What)
	{
		return When + TEXT(": ") + What;
	}

	// The rows of these tests.
	const FName LethalRow(TEXT("crisis_test_lethal"));
	const FName BranchLethalRow(TEXT("crisis_test_lethal_branch"));
	const FName MildRow(TEXT("crisis_test_mild"));
	const FName FallenRow(TEXT("tr_test_fallen"));
	const FName SealedRow(TEXT("tr_test_sealed"));
	const FName ApartRow(TEXT("tr_test_apart"));
	// A galaxy row no system of NewGalaxy has: CandidatesOf compares the names only.
	const FName FarGalaxyRow(TEXT("galaxy_test_far"));

	// The short-hazard bound of the tests (DT_Rules.SuccessionMinHazardTicks), set in their own rules.
	constexpr int32 MinTicks = 300;

	// The systems of the candidates' board (LayCandidates), by index into State.Systems; the sanctuary is the last element.
	constexpr int32 BoardDeath = 0;           // the death place: people 0, settled, clean
	constexpr int32 BoardSameCiv = 1;         // people 0, settled, clean
	constexpr int32 BoardNextToSphere = 2;    // people 1, settled, one lane from the unsealed fallen sphere
	constexpr int32 BoardInSphere = 3;        // people 1, settled, inside the unsealed fallen sphere
	constexpr int32 BoardInSealed = 4;        // people 2, settled, inside a sealed fallen sphere
	constexpr int32 BoardInApart = 5;         // people 2, settled, inside the sphere of a transcendent that is not fallen
	constexpr int32 BoardBeingSettled = 6;    // people 3, being settled (project_settle) - not settled
	constexpr int32 BoardWild = 7;            // nobody's
	constexpr int32 BoardFar = 8;             // people 3, settled, in another galaxy row
	constexpr int32 BoardLethal = 9;          // people 3, settled, a hazard that kills at its expiry, long to go
	constexpr int32 BoardBranchLethal = 10;   // people 3, settled, a hazard that kills on its branch, long to go
	constexpr int32 BoardAtMinimum = 11;      // people 3, settled, a hazard that kills nobody with exactly MinTicks left

	// The quiet boards (LayQuietBoard): the leap of their rules and the second settled system of the twin of the silence.
	constexpr int32 QuietLeap = 6;
	constexpr int32 QuietOther = 6;

	FCosmicEffect Effect(EEffectType Type, const TCHAR* TagName = nullptr, float Value = 0.f)
	{
		FCosmicEffect Out;
		Out.Type = Type;
		Out.Tag = TagName ? Tag(TagName) : FGameplayTag();
		Out.Value = Value;
		return Out;
	}

	// The hazard rows of the candidates' board: one that kills a vessel at its expiry, one that kills only on its branch at
	// the expiry, and one whose expiry kills nobody.
	UDataTable* HazardRows()
	{
		UDataTable* Table = CosmicTests::NewTable(FHazardDef::StaticStruct());
		FHazardDef Lethal;
		Lethal.HazardTag = Tag(TEXT("Hazard.PreSupernova"));
		Lethal.OnExpireEffects = { Effect(EEffectType::KillPlayer, TEXT("Death.Hazard")) };
		Table->AddRow(LethalRow, Lethal);
		FHazardDef Branch;
		Branch.HazardTag = Tag(TEXT("Hazard.AIAwakening"));
		Branch.BranchChance = 0.5f;
		Branch.OnExpireEffects = { Effect(EEffectType::ChangePopulation, nullptr, -0.1f) };
		Branch.OnExpireBranchEffects = { Effect(EEffectType::KillPlayer, TEXT("Death.Hazard")) };
		Table->AddRow(BranchLethalRow, Branch);
		FHazardDef Mild;
		Mild.HazardTag = Tag(TEXT("Hazard.Plague"));
		Mild.OnExpireEffects = { Effect(EEffectType::ChangePopulation, nullptr, -0.4f) };
		Table->AddRow(MildRow, Mild);
		return Table;
	}

	FTranscendentDef TranscendentRow(const TCHAR* Doctrine, const TCHAR* Allegiance)
	{
		FTranscendentDef Row;
		Row.Doctrine = Tag(Doctrine);
		Row.Allegiance = Tag(Allegiance);
		Row.SpawnWeight = 0.f;
		return Row;
	}

	// Two fallen rows (one for the sealed instance) and one that is not fallen.
	UDataTable* TranscendentRows()
	{
		UDataTable* Table = CosmicTests::NewTable(FTranscendentDef::StaticStruct());
		Table->AddRow(FallenRow, TranscendentRow(TEXT("Doctrine.Silence"), TEXT("Realm.Abyssal")));
		Table->AddRow(SealedRow, TranscendentRow(TEXT("Doctrine.Zeal"), TEXT("Realm.Abyssal")));
		Table->AddRow(ApartRow, TranscendentRow(TEXT("Doctrine.Stillness"), TEXT("Realm.Independent")));
		return Table;
	}

	void Place(FGalaxyState& State, int32 SystemIndex, const TCHAR* HazardTag, int32 Ticks)
	{
		State.Systems[SystemIndex].ActiveHazard = Tag(HazardTag);
		State.Systems[SystemIndex].HazardTicksRemaining = Ticks;
	}

	// The system leaves its owner's array and its owner.
	void Unown(FGalaxyState& State, int32 SystemIndex)
	{
		FStarSystemState& System = State.Systems[SystemIndex];
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.OwnedSystems.Remove(System.SystemID);
		}
		System.OwnerCivID = NAME_None;
	}

	// The system passes to the people CivID (a conquest a test lays by hand; no array changes its number of elements).
	void Hand(FGalaxyState& State, int32 SystemIndex, FName CivID)
	{
		FStarSystemState& System = State.Systems[SystemIndex];
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.OwnedSystems.Remove(System.SystemID);
			if (Civ.CivID == CivID)
			{
				Civ.OwnedSystems.Add(System.SystemID);
			}
		}
		System.OwnerCivID = CivID;
	}

	void JoinSphere(FGalaxyState& State, FName TranscendentID, int32 SystemIndex)
	{
		for (FTranscendentState& Transcendent : State.Transcendents)
		{
			if (Transcendent.TranscendentID == TranscendentID)
			{
				Transcendent.InfluencedSystems.AddUnique(State.Systems[SystemIndex].SystemID);
			}
		}
	}

	// The candidates' board on a galaxy of seed 42 with twelve living systems (the Board* indices), on the chain of
	// CosmicTests::WireChain (0-1-...-9, 4-10, 10-11), the sanctuary last and nobody's. Three transcendent instances: an
	// unsealed fallen one holding BoardInSphere, a sealed fallen one holding BoardInSealed and one that is not fallen holding
	// BoardInApart. What the reserved weights would read is there too: people 1 on the second rung, people 2 revering the
	// calling, a hazard within two lanes of BoardInSealed and BoardInApart, and the first life born into people 0. The vessel
	// stands in BoardDeath. False when the galaxy does not give that.
	bool LayCandidates(FGalaxyState& State)
	{
		if (CosmicTests::LivingSystems(State) < 12 || !CosmicState::IsSanctuary(State.Systems.Last().SystemID) || State.Civs.Num() < 4
			|| !CosmicTests::WireChain(State))
		{
			return false;
		}
		CosmicTests::ClearOwners(State);
		State.Transcendents.Reset();
		for (FStarSystemState& System : State.Systems)
		{
			CosmicHazards::EmptyHazardSlot(System);
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
		}
		CosmicTests::Give(State, 0, { BoardDeath, BoardSameCiv });
		CosmicTests::Give(State, 1, { BoardNextToSphere, BoardInSphere });
		CosmicTests::Give(State, 2, { BoardInSealed, BoardInApart });
		CosmicTests::Give(State, 3, { BoardBeingSettled, BoardFar, BoardLethal, BoardBranchLethal, BoardAtMinimum });
		State.Systems[BoardBeingSettled].ActiveMegaproject = CosmicRows::ProjectSettle;
		State.Systems[BoardFar].GalaxyRow = FarGalaxyRow;
		Place(State, BoardLethal, TEXT("Hazard.PreSupernova"), MinTicks + 200);
		Place(State, BoardBranchLethal, TEXT("Hazard.AIAwakening"), MinTicks + 200);
		Place(State, BoardAtMinimum, TEXT("Hazard.Plague"), MinTicks);
		CosmicTests::AddInstance(State, FallenRow, { State.Systems[BoardInSphere].SystemID });
		FTranscendentState& Sealed = CosmicTests::AddInstance(State, SealedRow, { State.Systems[BoardInSealed].SystemID });
		Sealed.bSealed = true;
		Sealed.SealedTick = State.Tick;
		CosmicTests::AddInstance(State, ApartRow, { State.Systems[BoardInApart].SystemID });
		State.Civs[1].Tier = EAscensionTier::Tier2_Stellar;
		State.Civs[2].ReverenceToPlayer = 0.9f;
		State.Player.CurrentSystemID = State.Systems[BoardDeath].SystemID;
		return true;
	}

	// The candidates as one line: the system indices in their order and how far the filters were let go of.
	FString Show(const CosmicSuccession::FCosmicSuccessionCandidates& Found)
	{
		FString Out = TEXT("[");
		for (int32 At = 0; At < Found.Systems.Num(); ++At)
		{
			if (At > 0)
			{
				Out += TEXT(" ");
			}
			Out += FString::FromInt(Found.Systems[At]);
		}
		return Out + FString::Printf(TEXT("] relaxed %d"), Found.Relaxed);
	}

	FString Expect(std::initializer_list<int32> Systems, int32 Relaxed)
	{
		CosmicSuccession::FCosmicSuccessionCandidates Found;
		Found.Systems = Systems;
		Found.Relaxed = Relaxed;
		return Show(Found);
	}

	// Whether a new vessel may be born in Candidate after a death in DeathIndex by the two filters that never relax (B5 header
	// section 2.4): a settled system of the death place's galaxy row, not the death place, not the sanctuary, under no hazard
	// that kills a vessel at its expiry.
	bool IsPlaceFor(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 DeathIndex, int32 Candidate)
	{
		if (!State.Systems.IsValidIndex(DeathIndex) || !State.Systems.IsValidIndex(Candidate) || Candidate == DeathIndex)
		{
			return false;
		}
		const FStarSystemState& System = State.Systems[Candidate];
		return System.GalaxyRow == State.Systems[DeathIndex].GalaxyRow && CosmicState::IsSettled(System) && !CosmicState::IsSanctuary(System.SystemID)
			&& !(System.ActiveHazard.IsValid() && CosmicHazards::KillsAVesselOf(Tables, System.ActiveHazard));
	}

	bool HasPlaceFor(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 DeathIndex)
	{
		for (int32 Candidate = 0; Candidate < State.Systems.Num(); ++Candidate)
		{
			if (IsPlaceFor(State, Tables, DeathIndex, Candidate))
			{
				return true;
			}
		}
		return false;
	}

	// The guarantee of section 6 (Sim.SuccessionPossible): after every filter that relaxes is let go of, a candidate exists
	// exactly when the death place's row holds a settled system other than the death place and the lethal hazards' systems;
	// and every candidate is one, in array order.
	void CheckGuarantee(FAutomationTestBase& Test, const FString& When, const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
		const FCosmicRulesDef& Rules, int32 DeathIndex)
	{
		const CosmicSuccession::FCosmicSuccessionCandidates Found = CosmicSuccession::CandidatesOf(State, Index, Tables, Rules, DeathIndex);
		Test.TestTrue(*Says(When, TEXT("a candidate is found exactly when the death place's galaxy row holds a settled system other than the death place and the lethal hazards' systems")),
			Found.Systems.IsEmpty() != HasPlaceFor(State, Tables, DeathIndex));
		bool bEveryFits = true;
		bool bOrdered = true;
		for (int32 At = 0; At < Found.Systems.Num(); ++At)
		{
			bEveryFits = bEveryFits && IsPlaceFor(State, Tables, DeathIndex, Found.Systems[At]);
			bOrdered = bOrdered && (At == 0 || Found.Systems[At - 1] < Found.Systems[At]);
		}
		Test.TestTrue(*Says(When, TEXT("every candidate is such a system")), bEveryFits);
		Test.TestTrue(*Says(When, TEXT("the candidates are in array order, each once")), bOrdered);
	}

	// How many numbers a stream drew between two of its states. INDEX_NONE beyond a bound no tick of a test reaches.
	int32 DrawsBetween(int32 FromSeed, int32 ToSeed)
	{
		FRandomStream Stream(FromSeed);
		for (int32 Count = 0; Count <= 100000; ++Count)
		{
			if (Stream.GetCurrentSeed() == ToSeed)
			{
				return Count;
			}
			Stream.GetFraction();
		}
		return INDEX_NONE;
	}

	// A death recorded on the live state as Apply records one (section 2.2): the tenth stage of the next tick carries it out.
	void RecordDeath(FGalaxyState& State)
	{
		State.Calling.PendingEffect = CosmicTags::EffectKillPlayer();
		State.Calling.PendingCause = Tag(TEXT("Death.Hazard"));
		State.Calling.PendingSourceRow = LethalRow;
	}

	// The game's tables made quiet for a hand-laid board that ticks: no inherited edict, no event slot, no first crisis, no
	// periodic petition, no drift and no fleet, a realm cap of one (no people settles), no hazard, event or card row, no
	// project or feat (CosmicTests::WithoutProjects), and a leap of exactly Leap ticks. What a board's owners hold stays
	// theirs through a leap.
	FCosmicDataTables QuietTables(const FCosmicDataTables& Original, const FCosmicRulesDef& GameRules, int32 Leap)
	{
		FCosmicRulesDef Rules = CosmicTests::WithoutStartingEdict(GameRules);
		Rules.EventChancePerSlot = 0.f;
		Rules.FirstCrisisTick = 0;
		Rules.PetitionPeriodTicks = 100000;
		Rules.RealmCapByTier = { 1, 1, 1, 1, 1 };
		Rules.RelationDriftRate = 0.f;
		Rules.FleetPerSettledByTier = { 0.f, 0.f, 0.f, 0.f, 0.f };
		Rules.LeapTicksMin = Leap;
		Rules.LeapTicksMax = Leap;
		Rules.LeapTotalCap = 1000000;
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));
		Tables.Hazards = CosmicTests::NewTable(FHazardDef::StaticStruct());
		Tables.Events = CosmicTests::NewTable(FCosmicEventDef::StaticStruct());
		Tables.Encounters = CosmicTests::NewTable(FEncounterDef::StaticStruct());
		return Tables;
	}

	// A quiet board on the live state (the simulation's tables are the quiet ones): a galaxy of seed 42 with twelve living
	// systems, nobody owning anything but what Owners hands out, no hazard, project or transcendent, the vessel ruling people
	// 0 from system 0, at tick 400 (past the birth protection of the first life). Null when the galaxy does not give that.
	FGalaxyState* LayQuietBoard(UGalaxySimulationSubsystem& Sim, TFunctionRef<void(FGalaxyState&)> Owners)
	{
		Sim.NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim.GetMutableState();
		if (CosmicTests::LivingSystems(State) < 12 || State.Civs.Num() < 4 || State.Player.RuledCivID != State.Civs[0].CivID)
		{
			return nullptr;
		}
		CosmicTests::ClearOwners(State);
		State.Transcendents.Reset();
		for (FStarSystemState& System : State.Systems)
		{
			CosmicHazards::EmptyHazardSlot(System);
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
		}
		Owners(State);
		State.Player.CurrentSystemID = State.Systems[0].SystemID;
		State.Tick = 400;
		return &State;
	}

	bool HoldsSystem(const FGalaxyState& State, FName SystemID)
	{
		return !SystemID.IsNone() && State.Systems.ContainsByPredicate([SystemID](const FStarSystemState& System) { return System.SystemID == SystemID; });
	}

	const FCivilizationState* CivOf(const FGalaxyState& State, FName CivID)
	{
		return CivID.IsNone() ? nullptr : State.Civs.FindByPredicate([CivID](const FCivilizationState& Civ) { return Civ.CivID == CivID; });
	}

	// The references of the present after a succession (docs/40 Sim.NoDanglingRefsAfterLeap, B5 header section 6): the
	// vessel's and the new life's names all name what the state holds. The historical IDs of the lives before, Knowing's IDs
	// and Transits are exempt.
	void CheckPresentRefs(FAutomationTestBase& Test, const FString& When, const UGalaxySimulationSubsystem& Sim)
	{
		const FGalaxyState& State = Sim.GetState();
		const FPlayerEntityState& Player = State.Player;
		const FCallingState& Calling = State.Calling;
		Test.TestTrue(*Says(When, TEXT("the vessel stands in a system the state holds")),
			HoldsSystem(State, Player.CurrentSystemID) && Sim.GetStateIndex().FindSystem(State, Player.CurrentSystemID) != INDEX_NONE);
		Test.TestTrue(*Says(When, TEXT("it rules no people")), Player.RuledCivID.IsNone());
		Test.TestTrue(*Says(When, TEXT("and left none (a new vessel starts from nothing)")), Player.FormerCivID.IsNone());
		Test.TestTrue(*Says(When, TEXT("no journey names a system the state lacks")), Player.TravelTargetSystemID.IsNone() || HoldsSystem(State, Player.TravelTargetSystemID));
		Test.TestEqual(*Says(When, TEXT("it carries no relic")), Player.Relics.Num(), 0);
		Test.TestFalse(*Says(When, TEXT("no role effect waits")), Calling.PendingEffect.IsValid() || !Calling.PendingSourceRow.IsNone());
		Test.TestTrue(*Says(When, TEXT("no first contact card waits on a people the state lacks")),
			State.Knowing.PendingContactCivID.IsNone() || CivOf(State, State.Knowing.PendingContactCivID) != nullptr);
		if (!Test.TestFalse(*Says(When, TEXT("(the calling has a life)")), Calling.Lives.IsEmpty()))
		{
			return;
		}
		const FLifeRecord& Life = Calling.Lives.Last();
		Test.TestTrue(*Says(When, TEXT("the new life's birth system is one the state holds")), HoldsSystem(State, Life.BirthSystemID));
		Test.TestEqual(*Says(When, TEXT("and the vessel stands there")), Player.CurrentSystemID, Life.BirthSystemID);
		Test.TestEqual(*Says(When, TEXT("the new life rules no people")), Life.RuledCivIDs.Num(), 0);
		const FCivilizationState* Birth = CivOf(State, Life.BirthCivID);
		if (Test.TestNotNull(*Says(When, TEXT("the new life's birth people is one the state holds")), Birth))
		{
			const int32 BirthSystem = State.Systems.IndexOfByPredicate([&Life](const FStarSystemState& System) { return System.SystemID == Life.BirthSystemID; });
			Test.TestTrue(*Says(When, TEXT("and it holds the birth system")), State.Systems.IsValidIndex(BirthSystem) && State.Systems[BirthSystem].OwnerCivID == Life.BirthCivID);
			Test.TestEqual(*Says(When, TEXT("the vessel's name is read from the birth people's archetype")), Life.BirthArchetypeRow, Birth->ArchetypeRow);
		}
		// The vessel's name ({Player}) resolves through BirthArchetypeRow, or arch_default when that bank is empty (section 8.2).
		const FCivArchetypeDef* Bank = Sim.GetTables().FindCiv(Life.BirthArchetypeRow);
		if (!Bank || Bank->PersonNameBank.IsEmpty())
		{
			Bank = Sim.GetTables().FindCiv(CosmicRows::ArchDefault);
		}
		if (Bank && !Bank->PersonNameBank.IsEmpty())
		{
			Test.TestTrue(*Says(When, TEXT("the vessel's name is an entry of its bank")), Bank->PersonNameBank.IsValidIndex(Life.NameIndex));
		}
	}
}

// Pins the single window of succession (B5 header section 2.4, decision 23-ga; section 6 Sim.SuccessionCandidates):
// CandidatesOf keeps the settled systems of the death place's galaxy row and never the death place, the sanctuary or a
// system under a hazard that kills a vessel at its expiry (CosmicHazards::KillsAVesselOf - the two filters that never relax);
// it leaves out a system in the sphere of an unsealed fallen transcendent and one whose hazard has fewer than
// SuccessionMinHazardTicks left, and lets go of them in that order - the hazard first, then the sphere - while they leave
// nothing; every candidate weighs the same (the five weight knobs are reserved and not read) and knowing is not read.
// PickOf maps one number evenly onto the candidates, the ends and a NaN included.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSuccessionCandidatesTest, "Cosmic_Ascension.Sim.SuccessionCandidates", CosmicTests::Flags)
bool FCosmicSuccessionCandidatesTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSuccessionTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FCosmicRulesDef Rules = Sim->GetRules();
	Rules.SuccessionMinHazardTicks = MinTicks;
	// CandidatesOf reads the hazard rows (KillsAVesselOf) and the transcendent rows (the allegiance) of the tables handed in.
	FCosmicDataTables Tables = Original;
	Tables.Hazards = HazardRows();
	Tables.Transcendents = TranscendentRows();

	// The lethal hazards (section 2.4): a KillPlayer in the expiry's effects or in its branch's.
	TestTrue(TEXT("A hazard whose expiry kills a vessel kills a vessel"), CosmicHazards::KillsAVesselOf(Tables, Tag(TEXT("Hazard.PreSupernova"))));
	TestTrue(TEXT("and so does one whose branch at the expiry kills"), CosmicHazards::KillsAVesselOf(Tables, Tag(TEXT("Hazard.AIAwakening"))));
	TestFalse(TEXT("A hazard whose expiry kills nobody does not"), CosmicHazards::KillsAVesselOf(Tables, Tag(TEXT("Hazard.Plague"))));
	TestFalse(TEXT("nor does a tag no row holds"), CosmicHazards::KillsAVesselOf(Tables, Tag(TEXT("Hazard.SubspaceErosion"))));
	TestFalse(TEXT("nor no tag"), CosmicHazards::KillsAVesselOf(Tables, FGameplayTag()));

	Sim->NewGalaxy(42, 12, 4);
	FGalaxyState Base = Sim->GetState();
	if (!TestTrue(TEXT("The candidates' board is laid"), LayCandidates(Base)))
	{
		return false;
	}
	const int32 Sanctuary = Base.Systems.Num() - 1;
	auto Of = [&Tables](const FGalaxyState& State, const FCosmicRulesDef& InRules, int32 DeathIndex)
	{
		FCosmicStateIndex Index;
		Index.Rebuild(State);
		return CosmicSuccession::CandidatesOf(State, Index, Tables, InRules, DeathIndex);
	};

	// Nothing to let go of: the filters as they stand.
	const CosmicSuccession::FCosmicSuccessionCandidates Found = Of(Base, Rules, BoardDeath);
	TestEqual(TEXT("The candidates are the settled systems of the death place's row that pass every filter, in array order, with nothing let go of"), Show(Found),
		Expect({ BoardSameCiv, BoardNextToSphere, BoardInSealed, BoardInApart, BoardAtMinimum }, 0));
	TestFalse(TEXT("Never the death place (to be born again where one died is the wheel's mark)"), Found.Systems.Contains(BoardDeath));
	TestFalse(TEXT("Never a system of another galaxy row"), Found.Systems.Contains(BoardFar));
	TestFalse(TEXT("Never a system being settled (it is not settled)"), Found.Systems.Contains(BoardBeingSettled));
	TestFalse(TEXT("Never a system nobody owns"), Found.Systems.Contains(BoardWild));
	TestFalse(TEXT("Never the sanctuary"), Found.Systems.Contains(Sanctuary));
	TestFalse(TEXT("Never a system whose hazard kills a vessel at its expiry, however long it has to go"), Found.Systems.Contains(BoardLethal));
	TestFalse(TEXT("nor one whose hazard kills on its branch"), Found.Systems.Contains(BoardBranchLethal));
	TestFalse(TEXT("While others are left, not a system in the sphere of an unsealed fallen transcendent"), Found.Systems.Contains(BoardInSphere));
	TestTrue(TEXT("A system one lane from that sphere is not in it"), Found.Systems.Contains(BoardNextToSphere));
	TestTrue(TEXT("The sphere of a sealed transcendent leaves nothing out"), Found.Systems.Contains(BoardInSealed));
	TestTrue(TEXT("nor does the sphere of one that is not fallen"), Found.Systems.Contains(BoardInApart));
	TestTrue(TEXT("A hazard with exactly SuccessionMinHazardTicks left is not short"), Found.Systems.Contains(BoardAtMinimum));
	TestTrue(TEXT("A system of the dead vessel's own people is a candidate like any other"), Found.Systems.Contains(BoardSameCiv));

	// The short-hazard filter: below the bound, by the state's countdown and by the rules' bound alike.
	FGalaxyState Short = Base;
	Short.Systems[BoardAtMinimum].HazardTicksRemaining = MinTicks - 1;
	TestEqual(TEXT("A hazard with fewer than SuccessionMinHazardTicks left leaves its system out while others are left"), Show(Of(Short, Rules, BoardDeath)),
		Expect({ BoardSameCiv, BoardNextToSphere, BoardInSealed, BoardInApart }, 0));
	FCosmicRulesDef Longer = Rules;
	Longer.SuccessionMinHazardTicks = MinTicks + 1;
	TestEqual(TEXT("The bound is the rules' SuccessionMinHazardTicks"), Show(Of(Base, Longer, BoardDeath)),
		Expect({ BoardSameCiv, BoardNextToSphere, BoardInSealed, BoardInApart }, 0));

	// The death place is the one handed in, not where the vessel stands or a people's capital.
	TestEqual(TEXT("The death place left out is the one handed in"), Show(Of(Base, Rules, BoardSameCiv)),
		Expect({ BoardDeath, BoardNextToSphere, BoardInSealed, BoardInApart, BoardAtMinimum }, 0));
	// The galaxy row is the death place's.
	FGalaxyState Far = Base;
	Far.Systems[BoardWild].GalaxyRow = FarGalaxyRow;
	CosmicTests::Give(Far, 3, { BoardWild });
	TestEqual(TEXT("A death in another galaxy row finds the settled systems of that row only"), Show(Of(Far, Rules, BoardFar)), Expect({ BoardWild }, 0));
	TestEqual(TEXT("and a death in the home row none of them"), Show(Of(Far, Rules, BoardDeath)),
		Expect({ BoardSameCiv, BoardNextToSphere, BoardInSealed, BoardInApart, BoardAtMinimum }, 0));
	TestTrue(TEXT("A death place the state does not hold gives no candidate (INDEX_NONE)"), Of(Base, Rules, INDEX_NONE).Systems.IsEmpty());
	TestTrue(TEXT("nor does one past the end"), Of(Base, Rules, Base.Systems.Num()).Systems.IsEmpty());

	// Letting go, the hazard first: a short hazard outside the sphere, a system inside it, a short hazard inside it, and a
	// lethal hazard that is short too.
	FGalaxyState HazardFirst = Base;
	Unown(HazardFirst, BoardNextToSphere);
	Unown(HazardFirst, BoardInSealed);
	Unown(HazardFirst, BoardInApart);
	Place(HazardFirst, BoardSameCiv, TEXT("Hazard.Plague"), MinTicks - 1);
	Place(HazardFirst, BoardAtMinimum, TEXT("Hazard.Plague"), MinTicks - 1);
	JoinSphere(HazardFirst, FallenRow, BoardAtMinimum);
	Place(HazardFirst, BoardBranchLethal, TEXT("Hazard.PreSupernova"), MinTicks - 1);
	TestEqual(TEXT("When the filters leave nothing the short-hazard filter is let go of first: the system with the short hazard outside the sphere, not the one in the sphere"),
		Show(Of(HazardFirst, Rules, BoardDeath)), Expect({ BoardSameCiv }, 1));
	// Then the sphere: every system that is left is inside it - the death place and a lethal one too.
	FGalaxyState Both = HazardFirst;
	JoinSphere(Both, FallenRow, BoardSameCiv);
	JoinSphere(Both, FallenRow, BoardDeath);
	JoinSphere(Both, FallenRow, BoardLethal);
	const CosmicSuccession::FCosmicSuccessionCandidates LetGo = Of(Both, Rules, BoardDeath);
	TestEqual(TEXT("When that leaves nothing too the sphere filter is let go of: every system the two fixed filters leave"), Show(LetGo),
		Expect({ BoardSameCiv, BoardInSphere, BoardAtMinimum }, 2));
	TestFalse(TEXT("The death place stays out after both filters are let go of"), LetGo.Systems.Contains(BoardDeath));
	TestFalse(TEXT("and so do the lethal hazards' systems, long or short to go"), LetGo.Systems.Contains(BoardLethal) || LetGo.Systems.Contains(BoardBranchLethal));
	TestFalse(TEXT("and the sanctuary"), LetGo.Systems.Contains(Sanctuary));
	// Nothing the fixed filters leave: no candidate (the silence).
	FGalaxyState Nothing = Base;
	for (const int32 SystemIndex : { BoardSameCiv, BoardNextToSphere, BoardInSphere, BoardInSealed, BoardInApart, BoardAtMinimum })
	{
		Unown(Nothing, SystemIndex);
	}
	TestTrue(TEXT("With only the death place, the lethal hazards' systems and no settled system else in the row, no candidate is left"),
		Of(Nothing, Rules, BoardDeath).Systems.IsEmpty());

	// The same weight (decision 23-ga): the five weight knobs are not read, whatever they hold.
	FCosmicRulesDef Flat = Rules;
	Flat.SuccessionHazardWeight = 0.f;
	Flat.SuccessionCivTierWeight = 0.f;
	Flat.SuccessionReverenceWeight = 0.f;
	Flat.SuccessionReverenceThreshold = 0.f;
	Flat.SuccessionSameCivPenalty = 0.f;
	FCosmicRulesDef Steep = Rules;
	Steep.SuccessionHazardWeight = 100.f;
	Steep.SuccessionCivTierWeight = -5.f;
	Steep.SuccessionReverenceWeight = 1000.f;
	Steep.SuccessionReverenceThreshold = 0.95f;
	Steep.SuccessionSameCivPenalty = 100.f;
	const FGalaxyState* Boards[] = { &Base, &HazardFirst, &Both };
	for (const FGalaxyState* Board : Boards)
	{
		const FString Even = Show(Of(*Board, Rules, BoardDeath));
		TestEqual(TEXT("The weight knobs at 0 change no candidate"), Show(Of(*Board, Flat, BoardDeath)), Even);
		TestEqual(TEXT("nor do steep or negative weight knobs"), Show(Of(*Board, Steep, BoardDeath)), Even);
	}

	// Knowing is not read (rule 2 - Sim.KnowingInert's check for the succession).
	FGalaxyState Unknowing = Base;
	Unknowing.Knowing = FKnowingState();
	FGalaxyState AllKnown = Base;
	CosmicTests::KnowAll(AllKnown);
	FGalaxyState DeathOnly = Base;
	CosmicTests::SetKnowing(DeathOnly, { BoardDeath }, { 0 });
	TestTrue(TEXT("(the knowing of the three boards differs)"), Unknowing.Knowing.KnownSystems.IsEmpty() && AllKnown.Knowing.KnownSystems.Num() == AllKnown.Systems.Num()
		&& DeathOnly.Knowing.KnownSystems.Num() == 1);
	TestEqual(TEXT("Knowing nothing changes no candidate"), Show(Of(Unknowing, Rules, BoardDeath)), Show(Found));
	TestEqual(TEXT("nor does knowing everything"), Show(Of(AllKnown, Rules, BoardDeath)), Show(Found));
	TestEqual(TEXT("nor knowing the death place alone"), Show(Of(DeathOnly, Rules, BoardDeath)), Show(Found));
	FGalaxyState BothKnown = Both;
	CosmicTests::KnowAll(BothKnown);
	TestEqual(TEXT("nor does knowing change what is let go of"), Show(Of(BothKnown, Rules, BoardDeath)), Show(LetGo));

	// PickOf: min(floor(Number x Count), Count - 1); below 0 or not a number the first; no count, no pick.
	TestEqual(TEXT("PickOf: 0 picks the first"), CosmicSuccession::PickOf(0.f, 4), 0);
	TestEqual(TEXT("PickOf: a number just below a quarter still picks the first of four"), CosmicSuccession::PickOf(0.2499f, 4), 0);
	TestEqual(TEXT("PickOf: a quarter picks the second"), CosmicSuccession::PickOf(0.25f, 4), 1);
	TestEqual(TEXT("PickOf: three quarters pick the fourth"), CosmicSuccession::PickOf(0.75f, 4), 3);
	TestEqual(TEXT("PickOf: a number just below 1 picks the last"), CosmicSuccession::PickOf(0.9999f, 4), 3);
	TestEqual(TEXT("PickOf: 1 picks the last (cut, not wrapped)"), CosmicSuccession::PickOf(1.f, 4), 3);
	TestEqual(TEXT("PickOf: a number above 1 picks the last"), CosmicSuccession::PickOf(1.5f, 4), 3);
	// At 1 or above - an infinity too - the last, read before the cast (no undefined conversion), whatever the count.
	TestEqual(TEXT("PickOf: 1e30 picks the last"), CosmicSuccession::PickOf(1e30f, 4), 3);
	TestEqual(TEXT("PickOf: infinity picks the last"), CosmicSuccession::PickOf(std::numeric_limits<float>::infinity(), 4), 3);
	TestEqual(TEXT("PickOf: 1 picks the last of a thousand"), CosmicSuccession::PickOf(1.f, 1000), 999);
	TestEqual(TEXT("PickOf: 1e30 picks the last of a thousand"), CosmicSuccession::PickOf(1e30f, 1000), 999);
	TestEqual(TEXT("PickOf: infinity picks the last of a thousand"), CosmicSuccession::PickOf(std::numeric_limits<float>::infinity(), 1000), 999);
	TestEqual(TEXT("PickOf: infinity picks the one candidate"), CosmicSuccession::PickOf(std::numeric_limits<float>::infinity(), 1), 0);
	TestEqual(TEXT("PickOf: a number below 0 picks the first"), CosmicSuccession::PickOf(-0.5f, 4), 0);
	TestEqual(TEXT("PickOf: minus infinity picks the first"), CosmicSuccession::PickOf(-std::numeric_limits<float>::infinity(), 4), 0);
	TestEqual(TEXT("PickOf: a number that is not a number picks the first"), CosmicSuccession::PickOf(std::numeric_limits<float>::quiet_NaN(), 4), 0);
	TestEqual(TEXT("PickOf: one candidate is picked by every number"), CosmicSuccession::PickOf(0.99f, 1), 0);
	TestEqual(TEXT("PickOf: no candidate is no pick"), CosmicSuccession::PickOf(0.5f, 0), static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("PickOf: a count below 0 is no pick"), CosmicSuccession::PickOf(0.5f, -2), static_cast<int32>(INDEX_NONE));
	// The same weight: numbers spread evenly over [0, 1) give every candidate the same share.
	int32 Shares[4] = { 0, 0, 0, 0 };
	for (int32 Step = 0; Step < 256; ++Step)
	{
		const int32 Pick = CosmicSuccession::PickOf(static_cast<float>(Step) / 256.f, 4);
		if (Pick >= 0 && Pick < 4)
		{
			++Shares[Pick];
		}
	}
	for (int32 Pick = 0; Pick < 4; ++Pick)
	{
		TestEqual(*FString::Printf(TEXT("PickOf: candidate %d of four takes a quarter of the numbers"), Pick), Shares[Pick], 64);
	}
	return true;
}

// Pins that a succession finds a place whenever the world holds one and that the silence is a state, not a retry (B5 header
// section 2.2 Succeed, 2.4, 2.5, 2.11; section 6 Sim.SuccessionPossible). Over a 20,000-tick input-less run of the game's
// data (no kill in it - only the test's deaths happen) a death forced at several ticks has a candidate, before it and at the
// succession, exactly when the death place's galaxy row holds a settled system other than the death place and the lethal
// hazards' systems; the dead vessel's place is kept through the interregnum; the new vessel is born where the succession's
// number (the second last of its tick - the name's is the last) picks; and the succession's tick draws exactly two numbers
// more than the same tick without it. On a board where only the death place is settled the succession finds none: the
// calling stays in the interregnum with no leap left, that tick drew its two numbers all the same, and the ticks after it
// draw what ticks that only count a leap down draw - the succession is never drawn again. Its twin with one more settled
// system is called there, with the same two numbers.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSuccessionPossibleTest, "Cosmic_Ascension.Sim.SuccessionPossible", CosmicTests::Flags)
bool FCosmicSuccessionPossibleTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSuccessionTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef GameRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FGalaxyState& State = Sim->GetState();

	// The input-less run: the game's data without a kill anywhere, the leap knobs (read at a death only) the test's own.
	constexpr int64 RunLeapTicks = 20;
	FCosmicRulesDef RunRules = GameRules;
	RunRules.LeapTicksMin = static_cast<int32>(RunLeapTicks);
	RunRules.LeapTicksMax = static_cast<int32>(RunLeapTicks);
	RunRules.LeapTotalCap = 1000000;
	Sim->SetTables(CosmicTests::WithoutDeath(CosmicTests::WithRules(Original, RunRules)));
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	const int64 Samples[] = { 600, 3000, 7000, 11000, 15000, 20000 };
	int32 WithPlace = 0;
	int32 Called = 0;
	for (const int64 At : Samples)
	{
		Sim->RunHeadless(At - State.Tick);
		const FGalaxyState Sample = State;
		const FString When = FString::Printf(TEXT("A death at tick %lld"), At);
		if (!TestTrue(*Says(When, TEXT("(a vessel stands - the run's data kills nobody)")), Sample.Calling.LifeState == ELifeState::Embodied))
		{
			continue;
		}
		const int32 DeathIndex = Sim->GetStateIndex().FindSystem(Sample, Sample.Player.CurrentSystemID);
		CheckGuarantee(*this, When + TEXT(", before it"), Sample, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), DeathIndex);
		CheckGuarantee(*this, When + TEXT(", before it, on the game's hazard rows"), Sample, Sim->GetStateIndex(), Original, Sim->GetRules(), DeathIndex);
		WithPlace += HasPlaceFor(Sample, Sim->GetTables(), DeathIndex) ? 1 : 0;

		RecordDeath(Sim->GetMutableState());
		Sim->RunHeadless(1);
		if (!TestTrue(*Says(When, TEXT("(the calling is in the interregnum with the whole leap left)")),
			State.Calling.LifeState == ELifeState::Interregnum && State.Calling.PendingLeapTicks == RunLeapTicks))
		{
			Sim->LoadState(Sample);
			continue;
		}
		Sim->RunHeadless(State.Calling.PendingLeapTicks - 1);
		TestEqual(*Says(When, TEXT("(one tick of the leap is left)")), State.Calling.PendingLeapTicks, int64{ 1 });
		TestEqual(*Says(When, TEXT("the dead vessel's place is kept through the interregnum")), State.Player.CurrentSystemID, Sample.Player.CurrentSystemID);
		const FGalaxyState Before = State;
		const int32 FromSeed = Sim->GetRng().GetCurrentSeed();
		Sim->RunHeadless(1);
		const int32 Draws = DrawsBetween(FromSeed, Sim->GetRng().GetCurrentSeed());
		// Install changes no field CandidatesOf reads: the state after the tick answers as the tenth stage was answered.
		const CosmicSuccession::FCosmicSuccessionCandidates Found = CosmicSuccession::CandidatesOf(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), DeathIndex);
		CheckGuarantee(*this, When + TEXT(", at the succession"), State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), DeathIndex);
		if (Found.Systems.IsEmpty())
		{
			TestTrue(*Says(When, TEXT("with no candidate the calling stays in the interregnum with no leap left")),
				State.Calling.LifeState == ELifeState::Interregnum && State.Calling.PendingLeapTicks == 0);
		}
		else if (TestTrue(*Says(When, TEXT("a candidate calls a new vessel")),
			State.Calling.LifeState == ELifeState::Embodied && State.Calling.Lives.Num() == Sample.Calling.Lives.Num() + 1)
			&& TestTrue(*Says(When, TEXT("(the tick drew at least the succession's two numbers)")), Draws >= 2))
		{
			++Called;
			FRandomStream Replay(FromSeed);
			for (int32 Draw = 0; Draw < Draws - 2; ++Draw)
			{
				Replay.GetFraction();
			}
			const int32 Pick = CosmicSuccession::PickOf(Replay.GetFraction(), Found.Systems.Num());
			const FLifeRecord& Life = State.Calling.Lives.Last();
			if (TestTrue(*Says(When, TEXT("(the succession's number picks a candidate)")), Found.Systems.IsValidIndex(Pick)))
			{
				const FStarSystemState& Birth = State.Systems[Found.Systems[Pick]];
				TestEqual(*Says(When, TEXT("the new vessel is born where the succession's number picks among the candidates")), Life.BirthSystemID, Birth.SystemID);
				TestEqual(*Says(When, TEXT("into the people that holds it")), Life.BirthCivID, Birth.OwnerCivID);
			}
		}
		// The same tick with nothing left to count down (the silence's tenth stage does nothing): two numbers fewer.
		FGalaxyState Quiet = Before;
		Quiet.Calling.PendingLeapTicks = 0;
		Sim->LoadState(Quiet);
		Sim->RunHeadless(1);
		TestEqual(*Says(When, TEXT("the succession's tick draws exactly two numbers more than the same tick without it")),
			Draws - DrawsBetween(FromSeed, Sim->GetRng().GetCurrentSeed()), 2);
		Sim->LoadState(Sample);
	}
	TestTrue(TEXT("(the guarantee had a place to keep at some death of the run)"), WithPlace > 0);
	TestTrue(TEXT("(and some death of the run called a new vessel)"), Called > 0);

	// The silence (section 2.11), and its twin with a second settled system: a ruler dies in system 0.
	Sim->SetTables(QuietTables(Original, GameRules, QuietLeap));
	for (const bool bOther : { false, true })
	{
		const FString Case = bOther ? TEXT("With a second settled system") : TEXT("The silence");
		FGalaxyState* Laid = LayQuietBoard(*Sim, [bOther](FGalaxyState& Board)
		{
			CosmicTests::Give(Board, 0, { 0 });
			if (bOther)
			{
				CosmicTests::Give(Board, 1, { QuietOther });
			}
		});
		if (!TestNotNull(*Says(Case, TEXT("(the board is laid)")), Laid))
		{
			continue;
		}
		const FName DeathID = State.Systems[0].SystemID;
		const FName OtherID = State.Systems[QuietOther].SystemID;
		const FName OtherCiv = State.Civs[1].CivID;
		RecordDeath(*Laid);
		Sim->RunHeadless(1);
		if (!TestTrue(*Says(Case, TEXT("(the ruler died: the calling is in the interregnum with the whole leap left)")),
			State.Calling.LifeState == ELifeState::Interregnum && State.Calling.PendingLeapTicks == QuietLeap))
		{
			continue;
		}
		Sim->RunHeadless(QuietLeap - 1);
		const FGalaxyState Before = State;
		const int32 FromSeed = Sim->GetRng().GetCurrentSeed();
		Sim->RunHeadless(1);
		const int32 Draws = DrawsBetween(FromSeed, Sim->GetRng().GetCurrentSeed());
		const FGalaxyState After = State;
		CheckGuarantee(*this, Case + TEXT(", at the succession"), State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0);
		if (!bOther)
		{
			TestTrue(TEXT("The silence: only the death place is settled, so no candidate is left"),
				CosmicSuccession::CandidatesOf(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0).Systems.IsEmpty());
			TestTrue(TEXT("The silence: the calling stays in the interregnum"), State.Calling.LifeState == ELifeState::Interregnum);
			TestEqual(TEXT("The silence: with no leap left"), State.Calling.PendingLeapTicks, int64{ 0 });
			TestEqual(TEXT("The silence: no life begins"), State.Calling.Lives.Num(), 1);
			TestEqual(TEXT("The silence: the dead vessel's place is kept"), State.Player.CurrentSystemID, DeathID);
		}
		else if (TestTrue(TEXT("With a second settled system: a new vessel is called"), State.Calling.LifeState == ELifeState::Embodied && State.Calling.Lives.Num() == 2))
		{
			TestEqual(TEXT("With a second settled system: it is born there"), State.Calling.Lives.Last().BirthSystemID, OtherID);
			TestEqual(TEXT("With a second settled system: into the people that holds it"), State.Calling.Lives.Last().BirthCivID, OtherCiv);
		}
		FGalaxyState Quiet = Before;
		Quiet.Calling.PendingLeapTicks = 0;
		Sim->LoadState(Quiet);
		Sim->RunHeadless(1);
		TestEqual(*Says(Case, TEXT("the succession's tick draws exactly two numbers more than the same tick without it, a candidate or none")),
			Draws - DrawsBetween(FromSeed, Sim->GetRng().GetCurrentSeed()), 2);
		if (!bOther)
		{
			// The ticks after the silence, against the same ticks counting a long leap down (the tenth stage draws nothing).
			constexpr int64 Later = 20;
			Sim->LoadState(After);
			Sim->RunHeadless(Later);
			const int32 Silent = Sim->GetRng().GetCurrentSeed();
			TestTrue(TEXT("The silence holds: still the interregnum, no leap left, no new life"),
				State.Calling.LifeState == ELifeState::Interregnum && State.Calling.PendingLeapTicks == 0 && State.Calling.Lives.Num() == 1);
			FGalaxyState Waiting = After;
			Waiting.Calling.PendingLeapTicks = 1000;
			Sim->LoadState(Waiting);
			Sim->RunHeadless(Later);
			TestEqual(TEXT("(the control counted its leap down)"), State.Calling.PendingLeapTicks, int64{ 1000 } - Later);
			TestEqual(TEXT("The ticks after the silence draw what ticks that only count a leap down draw: the succession is never drawn again"),
				Silent, Sim->GetRng().GetCurrentSeed());
		}
	}
	return true;
}

// Pins that a succession leaves no reference to what the state no longer holds (B5 header section 2.1, 2.2 Succeed, 2.6;
// section 6 Sim.NoDanglingRefsAfterLeap, docs/40 - the exemptions as written there). A ruler dies, the people it ruled
// loses its systems during the leap and leaves the state, and the fast-forward (RunLeap) runs the leap to the succession: the
// vessel stands in a system the state holds, rules and left no people (the dead ruler's FormerCivID named the people that
// is gone), carries nothing, no role effect or contact card waits, and the new life's birth system and birth people are
// held - the people holding that system, its archetype the name's bank; a tick later still so. The first life's people is
// gone: the historical IDs of old lives are exempt, and this one dangles.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicNoDanglingRefsAfterLeapTest, "Cosmic_Ascension.Sim.NoDanglingRefsAfterLeap", CosmicTests::Flags)
bool FCosmicNoDanglingRefsAfterLeapTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSuccessionTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef GameRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	constexpr int32 Leap = 30;
	Sim->SetTables(QuietTables(Original, GameRules, Leap));
	FGalaxyState* Laid = LayQuietBoard(*Sim, [](FGalaxyState& Board)
	{
		CosmicTests::Give(Board, 0, { 0, 1 });
		CosmicTests::Give(Board, 1, { 6 });
		CosmicTests::Give(Board, 2, { 8 });
	});
	if (!TestNotNull(TEXT("The board is laid"), Laid))
	{
		return false;
	}
	const FGalaxyState& State = Sim->GetState();
	const FName FirstPeople = State.Civs[0].CivID;
	const FName Taker = State.Civs[2].CivID;
	RecordDeath(*Laid);
	Sim->RunHeadless(1);
	if (!TestTrue(TEXT("(the ruler died: the calling is in the interregnum)"), State.Calling.LifeState == ELifeState::Interregnum
		&& State.Calling.PendingLeapTicks == Leap))
	{
		return false;
	}
	TestEqual(TEXT("(the dead ruler remembers the people it left - the reference a succession must not carry)"), State.Player.FormerCivID, FirstPeople);
	// During the leap another people takes both systems of the first life's people: that people leaves the state at the
	// next crossing stage.
	Hand(*Laid, 0, Taker);
	Hand(*Laid, 1, Taker);
	const int64 From = State.Tick;
	Sim->RunLeap(State.Calling.PendingLeapTicks);
	TestEqual(TEXT("(the fast-forward ran the leap's ticks)"), State.Tick, From + Leap);
	if (!TestTrue(TEXT("(a new vessel was called at the leap's end)"), State.Calling.LifeState == ELifeState::Embodied && State.Calling.Lives.Num() == 2))
	{
		return false;
	}
	TestTrue(TEXT("(the first life's people is gone: its historical IDs dangle, as the exemption allows)"), CivOf(State, State.Calling.Lives[0].BirthCivID) == nullptr);
	CheckPresentRefs(*this, TEXT("After the leap"), *Sim);
	Sim->RunHeadless(1);
	CheckPresentRefs(*this, TEXT("A tick after the leap"), *Sim);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

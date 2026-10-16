// Cosmic Ascension - automation tests of knowing and the scope of telling: the sight stage, the writers of knowing, the
// scope of the log, and the proof that the world never reads any of it. Design: guideline.md section 4.4, 7.4, 8.1, 9.3.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

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
#include "Log/CosmicLogSubsystem.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "Tests/CosmicTestProbe.h"
#include "UObject/NameTypes.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#include <initializer_list>

namespace CosmicKnowingTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	UDataTable* NewTable(UScriptStruct* RowStruct)
	{
		UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
		Table->RowStruct = RowStruct;
		return Table;
	}

	// The knobs of the tests, not the game's: sight every ten ticks from tick 30, reach 1/2/3/4/5 lanes, a far event
	// line every 300 ticks at most, no slot chance, no drain, no regrowth, nobody settles.
	FCosmicRulesDef Knobs(const FCosmicRulesDef& Base)
	{
		FCosmicRulesDef Rules = Base;
		Rules.SightLanesByTier = { 1, 2, 3, 4, 5 };
		Rules.SightIntervalTicks = 10;
		Rules.FirstSightTick = 30;
		Rules.FarLineCooldownTicks = 300;
		Rules.EventSlotTicks = 10;
		Rules.EventChancePerSlot = 0.f;
		Rules.SpiritualPressureStabilityDrain = 0.f;
		Rules.PopulationRegrowthPerTick = 0.f;
		Rules.RealmCapByTier = { 1, 1, 1, 1, 1 };
		return Rules;
	}

	// A supernova that never spawns by itself and expires the tick after it is placed by hand.
	UDataTable* NovaRows()
	{
		UDataTable* Table = NewTable(FHazardDef::StaticStruct());
		FHazardDef Nova;
		Nova.HazardTag = Tag(TEXT("Hazard.PreSupernova"));
		Nova.DisplayName = FText::AsCultureInvariant(TEXT("<nova>"));
		Nova.CountdownTicks = 1;
		Nova.SpawnRule = ECosmicHazardSpawn::Draw;
		Nova.SpawnChancePerTick = 0.f;
		Nova.LogCategory = Tag(TEXT("Log.Crisis"));
		Nova.Severity = 1;
		Table->AddRow(FName(TEXT("crisis_test_nova")), Nova);
		return Table;
	}

	// The game's tables (its log rows among them) with Rules, without the game's projects and feats
	// (CosmicTests::WithoutProjects), with Hazards (none when null) and Events (none when null), and without
	// transcendent rows: on a board nothing tells a line but what a test asks for.
	FCosmicDataTables TablesWith(const FCosmicDataTables& Original, const FCosmicRulesDef& Rules, UDataTable* Hazards = nullptr, UDataTable* Events = nullptr)
	{
		UDataTable* RulesTable = NewTable(FCosmicRulesDef::StaticStruct());
		RulesTable->AddRow(CosmicRows::Rules, Rules);
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(Original);
		Tables.Rules = RulesTable;
		Tables.Hazards = Hazards ? Hazards : NewTable(FHazardDef::StaticStruct());
		Tables.Events = Events ? Events : NewTable(FCosmicEventDef::StaticStruct());
		Tables.Transcendents = NewTable(FTranscendentDef::StaticStruct());
		return Tables;
	}

	// The game's tables with the rules row replaced.
	FCosmicDataTables WithRules(const FCosmicDataTables& Original, const FCosmicRulesDef& Rules)
	{
		UDataTable* RulesTable = NewTable(FCosmicRulesDef::StaticStruct());
		RulesTable->AddRow(CosmicRows::Rules, Rules);
		FCosmicDataTables Tables = Original;
		Tables.Rules = RulesTable;
		return Tables;
	}

	void Mark(TArray<FKnownMark>& Marks, FName ID)
	{
		FKnownMark& Added = Marks.AddDefaulted_GetRef();
		Added.ID = ID;
	}

	// The board: the chain of CosmicTests::WireChain (0-1-...-9, 4-10, 10-11). The vessel rules people 0 from system 0;
	// people 1 holds 2, people 2 holds 8, people 3 holds 11; every people on Tier 1, researching nothing; no hazard,
	// project, transcendent, flag or log line. The calling knows system 0 and has met people 0 and nobody else: no first
	// contact yet. False below twelve systems or four peoples.
	bool Board(FGalaxyState& State)
	{
		if (!CosmicTests::WireChain(State) || State.Civs.Num() < 4)
		{
			return false;
		}
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { 0 });
		CosmicTests::Give(State, 1, { 2 });
		CosmicTests::Give(State, 2, { 8 });
		CosmicTests::Give(State, 3, { 11 });
		State.Transcendents.Reset();
		State.Flags.Reset();
		State.Log.Reset();
		for (FStarSystemState& System : State.Systems)
		{
			CosmicHazards::EmptyHazardSlot(System);
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
		}
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.Tier = EAscensionTier::Tier1_Planetary;
			for (TPair<FName, float>& Focus : Civ.ResearchFocus)
			{
				Focus.Value = 0.f;
			}
		}
		State.Calling.PersonalTier = EAscensionTier::Tier1_Planetary;
		State.Player.CurrentSystemID = State.Systems[0].SystemID;
		State.Knowing = FKnowingState();
		Mark(State.Knowing.KnownSystems, State.Systems[0].SystemID);
		Mark(State.Knowing.MetCivs, State.Civs[0].CivID);
		return true;
	}

	FGalaxyState* LayBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Tables)
	{
		Sim.SetTables(Tables);
		Sim.NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim.GetMutableState();
		return Board(State) ? &State : nullptr;
	}

	// The vessel walks: a wanderer standing in SystemIndex, its people no longer ruled.
	void MakeWanderer(FGalaxyState& State, int32 SystemIndex, EAscensionTier PersonalTier)
	{
		State.Civs[0].bIsPlayerRuled = false;
		State.Player.Role = EPlayerRole::Wanderer;
		State.Player.RuledCivID = NAME_None;
		State.Player.CurrentSystemID = State.Systems[SystemIndex].SystemID;
		State.Calling.PersonalTier = PersonalTier;
	}

	TArray<FName> IDs(const TArray<FKnownMark>& Marks)
	{
		TArray<FName> Result;
		for (const FKnownMark& Each : Marks)
		{
			Result.Add(Each.ID);
		}
		return Result;
	}

	TArray<FName> SystemIDs(const FGalaxyState& State, std::initializer_list<int32> Indices)
	{
		TArray<FName> Result;
		for (const int32 Index : Indices)
		{
			Result.Add(State.Systems[Index].SystemID);
		}
		return Result;
	}

	bool Knows(const FGalaxyState& State, int32 SystemIndex)
	{
		const FName SystemID = State.Systems[SystemIndex].SystemID;
		return State.Knowing.KnownSystems.ContainsByPredicate([SystemID](const FKnownMark& Mark) { return Mark.ID == SystemID; });
	}

	bool HasMet(const FGalaxyState& State, FName CivID)
	{
		return State.Knowing.MetCivs.ContainsByPredicate([CivID](const FKnownMark& Mark) { return Mark.ID == CivID; });
	}

	// The index of the entry of Row told at Tick, or INDEX_NONE.
	int32 FindLine(const FGalaxyState& State, FName Row, int64 Tick)
	{
		return State.Log.IndexOfByPredicate([Row, Tick](const FCosmicLogEntry& Entry) { return Entry.SourceRow == Row && Entry.Tick == Tick; });
	}

	// How many numbers the tell stream moved from Before to After, or -1 beyond four.
	int32 DrawsBetween(int32 Before, int32 After)
	{
		FRandomStream Walk(Before);
		for (int32 Count = 0; Count <= 4; ++Count)
		{
			if (Walk.GetCurrentSeed() == After)
			{
				return Count;
			}
			Walk.GetFraction();
		}
		return -1;
	}

	// A probe on OnLogEntry for the lifetime of the object.
	struct FScopedLogProbe
	{
		explicit FScopedLogProbe(UCosmicLogSubsystem& InLog)
			: Log(&InLog)
			, Probe(NewObject<UCosmicTestProbe>())
		{
			Probe->AddToRoot();
			Log->OnLogEntry.AddDynamic(Probe, &UCosmicTestProbe::HandleLogEntry);
		}

		~FScopedLogProbe()
		{
			Log->OnLogEntry.RemoveDynamic(Probe, &UCosmicTestProbe::HandleLogEntry);
			Probe->RemoveFromRoot();
		}

		UCosmicLogSubsystem* Log;
		UCosmicTestProbe* Probe;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSightStageTest, "Cosmic_Ascension.Sim.SightStage", CosmicTests::Flags)
bool FCosmicSightStageTest::RunTest(const FString& Parameters)
{
	using namespace CosmicKnowingTests;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FCosmicRulesDef Rules = Knobs(Sim->GetRules());
	const FCosmicDataTables Tables = TablesWith(Original, Rules);

	// ---- A ruler's eye opens at FirstSightTick; the first meeting is the first contact ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the first contact is laid"), State))
		{
			return false;
		}
		// People 1 also holds 1, one lane from the vessel's system.
		CosmicTests::Give(*State, 1, { 1 });
		const FName One = State->Civs[1].CivID;
		Sim->RunHeadless(Rules.FirstSightTick - 1);
		TestTrue(TEXT("Before FirstSightTick a ruler knows only its birth system"), IDs(State->Knowing.KnownSystems) == SystemIDs(*State, { 0 }));
		TestTrue(TEXT("and has made no first contact"), State->Knowing.FirstContactCivID.IsNone());
		Sim->StepOnce();
		const int64 Opened = State->Tick;
		TestEqual(TEXT("The eye opens at FirstSightTick"), Opened, static_cast<int64>(Rules.FirstSightTick));
		TestTrue(TEXT("and sees one lane on Tier 1"), IDs(State->Knowing.KnownSystems) == SystemIDs(*State, { 0, 1 }));
		TestEqual(TEXT("The owner of the system seen is the first contact"), State->Knowing.FirstContactCivID, One);
		const FKnownMark* Met = State->Knowing.MetCivs.FindByPredicate([One](const FKnownMark& Mark) { return Mark.ID == One; });
		if (TestNotNull(TEXT("The meeting is on the record"), Met))
		{
			TestEqual(TEXT("at its tick"), Met->Tick, Opened);
		}
		const int32 Line = FindLine(*State, CosmicRows::LogFirstContact, Opened);
		if (TestTrue(TEXT("The first contact is told"), Line != INDEX_NONE))
		{
			TestEqual(TEXT("as a line of Severity 1"), static_cast<int32>(State->Log[Line].Severity), 1);
			TestEqual(TEXT("about the people met"), State->Log[Line].FocusCivID, One);
			TestEqual(TEXT("and the system it was seen at"), State->Log[Line].FocusSystemID, State->Systems[1].SystemID);
			TestTrue(TEXT("which names the people"), State->Log[Line].Message.ToString().Contains(State->Civs[1].CivName.ToString()));
		}
		TestEqual(TEXT("No other meeting line is told"), FindLine(*State, CosmicRows::LogContact, Opened), static_cast<int32>(INDEX_NONE));
	}

	// ---- Every system the people owns, the reach of its Tier, the order of the walk; the nearer people first ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the two places is laid"), State))
		{
			return false;
		}
		const FName One = State->Civs[1].CivID;
		const FName Two = State->Civs[2].CivID;
		if (!TestTrue(TEXT("The board's peoples 1 and 2 are in that order by ID"), FNameLexicalLess()(One, Two)))
		{
			return false;
		}
		// People 0 holds 0 and 9 on Tier 2 (reach 2). People 2 holds 8, one lane from 9; people 1 holds 2, two lanes from 0.
		CosmicTests::Give(*State, 0, { 9 });
		State->Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		Sim->RunHeadless(Rules.FirstSightTick);
		TestTrue(TEXT("A ruler sees from every system its people owns, as far as its Tier reaches, in the order of the walk"),
			IDs(State->Knowing.KnownSystems) == SystemIDs(*State, { 0, 9, 1, 8, 2, 7 }));
		TestEqual(TEXT("The nearer people is met first, whatever its ID"), State->Knowing.FirstContactCivID, Two);
		TestTrue(TEXT("then the farther one"), IDs(State->Knowing.MetCivs) == TArray<FName>({ State->Civs[0].CivID, Two, One }));
		const int32 Contact = FindLine(*State, CosmicRows::LogContact, State->Tick);
		if (TestTrue(TEXT("A later meeting is told as a meeting"), Contact != INDEX_NONE))
		{
			TestEqual(TEXT("about that people"), State->Log[Contact].FocusCivID, One);
			TestEqual(TEXT("and where it was seen"), State->Log[Contact].FocusSystemID, State->Systems[2].SystemID);
		}
	}

	// ---- Equal lanes: the lower ID first, not the one the walk found first ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the tie is laid"), State))
		{
			return false;
		}
		CosmicTests::ClearOwners(*State);
		CosmicTests::Give(*State, 0, { 0, 9 });
		// People 2 holds 2 (found by the walk before 7), people 1 holds 7: both two lanes from the eye.
		CosmicTests::Give(*State, 2, { 2 });
		CosmicTests::Give(*State, 1, { 7 });
		CosmicTests::Give(*State, 3, { 11 });
		State->Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		Sim->RunHeadless(Rules.FirstSightTick);
		TestEqual(TEXT("At equal lanes the people of the lower ID is met first"), State->Knowing.FirstContactCivID, State->Civs[1].CivID);
	}

	// ---- Another galaxy row is not walked into ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the other row is laid"), State))
		{
			return false;
		}
		CosmicTests::Give(*State, 1, { 1 });
		State->Systems[1].GalaxyRow = FName(TEXT("gal_test_other"));
		Sim->RunHeadless(Rules.FirstSightTick);
		TestFalse(TEXT("A lane into another galaxy row is not walked"), Knows(*State, 1));
		TestTrue(TEXT("so its owner is not met"), State->Knowing.FirstContactCivID.IsNone());
	}

	// ---- The gap rule: known by its system, met when the viewer climbs within two rungs ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the gap is laid"), State))
		{
			return false;
		}
		CosmicTests::Give(*State, 1, { 1 });
		State->Civs[1].Tier = EAscensionTier::Tier4_Transcendent;
		const FName One = State->Civs[1].CivID;
		Sim->RunHeadless(Rules.FirstSightTick);
		TestTrue(TEXT("The system of a people three rungs above is known"), Knows(*State, 1));
		TestFalse(TEXT("but the people is not met"), HasMet(*State, One));
		TestEqual(TEXT("and no line is told of it"), FindLine(*State, CosmicRows::LogFirstContact, State->Tick), static_cast<int32>(INDEX_NONE));
		State->Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		Sim->RunHeadless(Rules.SightIntervalTicks);
		TestTrue(TEXT("Once the viewer stands two rungs below, the next look meets it"), HasMet(*State, One));
	}

	// ---- A wanderer sees from where it stands, with its own Tier, from the first look ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the wanderer is laid"), State))
		{
			return false;
		}
		MakeWanderer(*State, 4, EAscensionTier::Tier3_Causal);
		Sim->RunHeadless(Rules.SightIntervalTicks);
		TestTrue(TEXT("A wanderer sees from the system it stands in, as far as its own Tier reaches, before FirstSightTick"),
			IDs(State->Knowing.KnownSystems) == SystemIDs(*State, { 0, 4, 3, 5, 10, 2, 6, 11, 1, 7 }));
		// Peoples 1 and 3 both stand two lanes off: the lower ID first.
		const bool bOneFirst = FNameLexicalLess()(State->Civs[1].CivID, State->Civs[3].CivID);
		const TArray<FName> Expected = { State->Civs[0].CivID, State->Civs[bOneFirst ? 1 : 3].CivID, State->Civs[bOneFirst ? 3 : 1].CivID };
		TestTrue(TEXT("and meets the owners of what it sees"), IDs(State->Knowing.MetCivs) == Expected);
		TestFalse(TEXT("but not the owner of a system out of reach"), HasMet(*State, State->Civs[2].CivID));
	}

	// ---- The vessel moved: the stage looks again off its period (a journey, and a destruction that carried it) ----
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Rules, NovaRows()));
		if (!TestNotNull(TEXT("The board of the move is laid"), State))
		{
			return false;
		}
		// People 2 holds 7 and 8; the wanderer (reach 1) stands in 4.
		CosmicTests::Give(*State, 2, { 7 });
		MakeWanderer(*State, 4, EAscensionTier::Tier1_Planetary);
		Sim->RunHeadless(Rules.SightIntervalTicks + 2);
		const FGalaxyState Before = *State;
		TestTrue(TEXT("The wanderer knows its neighbourhood"), IDs(State->Knowing.KnownSystems) == SystemIDs(*State, { 0, 4, 3, 5, 10 }));

		State->Player.TravelTargetSystemID = State->Systems[9].SystemID;
		State->Player.TravelTicksRemaining = 1;
		Sim->StepOnce();
		TestTrue(TEXT("A journey's end off the period: the stage looks from where the vessel arrived"), Knows(*State, 9) && Knows(*State, 8));
		TestTrue(TEXT("and meets who it sees there"), HasMet(*State, State->Civs[2].CivID));

		Sim->LoadState(Before);
		Sim->StepOnce();
		TestFalse(TEXT("The same tick without the journey looks at nothing"), Knows(*State, 9));

		// The wanderer stands in 8, which a supernova takes: the destruction carries it to 7, and the stage looks again.
		FGalaxyState Carried = Before;
		Carried.Player.CurrentSystemID = Carried.Systems[8].SystemID;
		Carried.Systems[8].ActiveHazard = Tag(TEXT("Hazard.PreSupernova"));
		Carried.Systems[8].HazardTicksRemaining = 1;
		Sim->LoadState(Carried);
		Sim->StepOnce();
		TestTrue(TEXT("The supernova took the system the vessel stood in"), CosmicHazards::IsDeadNode(State->Systems[8]));
		TestEqual(TEXT("and carried the vessel to a lane neighbour"), State->Player.CurrentSystemID, State->Systems[7].SystemID);
		TestTrue(TEXT("A carried vessel looks again off the period: it knows the neighbour of where it came to"), Knows(*State, 6));
	}

	// ---- No vessel, no eye ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the interregnum is laid"), State))
		{
			return false;
		}
		MakeWanderer(*State, 4, EAscensionTier::Tier3_Causal);
		State->Calling.LifeState = ELifeState::Interregnum;
		Sim->RunHeadless(Rules.SightIntervalTicks);
		TestTrue(TEXT("In the interregnum the stage does not look"), IDs(State->Knowing.KnownSystems) == SystemIDs(*State, { 0 }));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicKnowingWritersTest, "Cosmic_Ascension.Sim.KnowingWriters", CosmicTests::Flags)
bool FCosmicKnowingWritersTest::RunTest(const FString& Parameters)
{
	using namespace CosmicKnowingTests;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UGameInstance* GameInstance = Sim->GetGameInstance();
	UCosmicLogSubsystem* Log = GameInstance->GetSubsystem<UCosmicLogSubsystem>();
	UCosmicEventSubsystem* Events = GameInstance->GetSubsystem<UCosmicEventSubsystem>();
	UPlayerRoleSubsystem* PlayerRole = GameInstance->GetSubsystem<UPlayerRoleSubsystem>();
	if (!TestTrue(TEXT("The subsystems exist"), Log && Events && PlayerRole))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Log->SetTellAll(true); Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FCosmicRulesDef Rules = Knobs(Sim->GetRules());
	const FCosmicDataTables Tables = TablesWith(Original, Rules);

	// ---- NoteMet and NoteKnown ----
	{
		Sim->SetTables(Tables);
		Sim->NewGalaxy(42, 12, 4);
		TestTrue(TEXT("A first life as a ruler knows its birth system alone: the eye of a ruler waits for the sight stage"),
			IDs(Sim->GetState().Knowing.KnownSystems) == SystemIDs(Sim->GetState(), { 0 }));
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the writers is laid"), State))
		{
			return false;
		}
		const FName One = State->Civs[1].CivID;
		const FName Two = State->Civs[2].CivID;
		const FName Three = State->Civs[3].CivID;
		const int32 SimBefore = Sim->GetRng().GetCurrentSeed();
		TestTrue(TEXT("NoteMet meets a people not met yet"), Sim->NoteMet(One, State->Systems[2].SystemID));
		TestFalse(TEXT("but not twice"), Sim->NoteMet(One));
		TestEqual(TEXT("The first meeting of the run is the first contact"), State->Knowing.FirstContactCivID, One);
		const int32 First = FindLine(*State, CosmicRows::LogFirstContact, State->Tick);
		if (TestTrue(TEXT("and is told as the first contact"), First != INDEX_NONE))
		{
			TestEqual(TEXT("about the place the caller names"), State->Log[First].FocusSystemID, State->Systems[2].SystemID);
		}
		TestTrue(TEXT("A second people is met"), Sim->NoteMet(Two));
		TestEqual(TEXT("and the first contact stays as it was"), State->Knowing.FirstContactCivID, One);
		const int32 Second = FindLine(*State, CosmicRows::LogContact, State->Tick);
		if (TestTrue(TEXT("The second meeting is told as a meeting"), Second != INDEX_NONE))
		{
			TestTrue(TEXT("with no place when the caller names none"), State->Log[Second].FocusSystemID.IsNone());
		}
		TestFalse(TEXT("A people the state does not hold is not met"), Sim->NoteMet(FName(TEXT("civ_gone"))));
		State->Civs[3].Tier = EAscensionTier::Tier4_Transcendent;
		TestFalse(TEXT("A people three rungs above the viewer is not met"), Sim->NoteMet(Three));
		State->Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		TestTrue(TEXT("one two rungs above is"), Sim->NoteMet(Three));
		TestTrue(TEXT("NoteKnown knows a system"), Sim->NoteKnown(State->Systems[6].SystemID));
		TestFalse(TEXT("once"), Sim->NoteKnown(State->Systems[6].SystemID));
		TestFalse(TEXT("A system the state does not hold is not known"), Sim->NoteKnown(FName(TEXT("sys_gone"))));
		TestEqual(TEXT("Neither draws from the sim stream"), Sim->GetRng().GetCurrentSeed(), SimBefore);
		State->Calling.LifeState = ELifeState::Interregnum;
		TestFalse(TEXT("Without a vessel NoteKnown does nothing"), Sim->NoteKnown(State->Systems[7].SystemID));
		State->Civs[3].Tier = EAscensionTier::Tier1_Planetary;
		State->Knowing.MetCivs.RemoveAll([Three](const FKnownMark& Mark) { return Mark.ID == Three; });
		TestFalse(TEXT("nor NoteMet"), Sim->NoteMet(Three));
		State->Calling.LifeState = ELifeState::Embodied;
	}

	// ---- A system the vessel's people receives through an effect is known before the line ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the gift is laid"), State))
		{
			return false;
		}
		// People 1 holds 2, 3 and 1, so that none of the three is its last.
		CosmicTests::Give(*State, 1, { 3, 1 });
		const FName ToRuled(TEXT("event_test_gift_ruled"));
		const FName ToAI(TEXT("event_test_gift_ai"));
		UDataTable* EventRows = NewTable(FCosmicEventDef::StaticStruct());
		FCosmicEventDef Gift;
		Gift.Category = Tag(TEXT("Event.Civ.Society"));
		Gift.LogCategory = Tag(TEXT("Log.Civ"));
		Gift.Trigger = ECosmicEventTrigger::Named;
		Gift.Subject = ECosmicEventSubject::SettledSystem;
		Gift.Severity = 1;
		Gift.LogTemplates = { FText::AsCultureInvariant(TEXT("gift {System}")) };
		FCosmicEffect Transfer;
		Transfer.Type = EEffectType::TransferSystem;
		Transfer.Key = State->Civs[0].CivID;
		Gift.Effects = { Transfer };
		EventRows->AddRow(ToRuled, Gift);
		Gift.Effects[0].Key = State->Civs[3].CivID;
		EventRows->AddRow(ToAI, Gift);
		Sim->SetTables(TablesWith(Original, Rules, nullptr, EventRows));

		Log->SetTellAll(false);
		{
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			Ctx.CivIndex = 0;
			Ctx.SystemIndex = 2;
			Ctx.GalaxyRow = State->Systems[2].GalaxyRow;
			TestTrue(TEXT("The gift to the vessel's people fires"), Events->FireRow(ToRuled, Ctx));
		}
		TestEqual(TEXT("The system is the vessel's people's"), State->Systems[2].OwnerCivID, State->Civs[0].CivID);
		TestTrue(TEXT("and known"), Knows(*State, 2));
		const int32 Told = FindLine(*State, ToRuled, State->Tick);
		if (TestTrue(TEXT("The line about the received system is stored: it was known before the line"), Told != INDEX_NONE))
		{
			TestFalse(TEXT("and told"), State->Log[Told].bUntold);
		}
		{
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			Ctx.CivIndex = 3;
			Ctx.SystemIndex = 3;
			Ctx.GalaxyRow = State->Systems[3].GalaxyRow;
			TestTrue(TEXT("The gift to an AI people fires"), Events->FireRow(ToAI, Ctx));
		}
		TestEqual(TEXT("That system is the AI people's"), State->Systems[3].OwnerCivID, State->Civs[3].CivID);
		TestFalse(TEXT("and stays unknown"), Knows(*State, 3));
		Log->SetTellAll(true);
		Sim->SetTables(Tables);
	}

	// ---- The ruler's settlement makes its target known at once ----
	{
		FCosmicRulesDef Settling = Rules;
		Settling.RealmCapByTier = { 1, 3, 4, 7, 10 };
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Settling));
		if (!TestNotNull(TEXT("The board of the settlement is laid"), State))
		{
			return false;
		}
		State->Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		TestFalse(TEXT("The target is not known before"), Knows(*State, 1));
		TestTrue(TEXT("The ruler settles the target"), PlayerRole->StartMegaproject(State->Systems[1].SystemID, CosmicRows::ProjectSettle));
		TestTrue(TEXT("which is known at once"), Knows(*State, 1));
		Sim->SetTables(Tables);
	}

	// ---- A deposed ruler's new eye: where it stands and the lanes around, in the same galaxy row ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the deposition is laid"), State))
		{
			return false;
		}
		State->Player.CurrentSystemID = State->Systems[4].SystemID;
		State->Systems[10].GalaxyRow = FName(TEXT("gal_test_other"));
		PlayerRole->DeposeRuler();
		TestTrue(TEXT("The vessel was deposed"), State->Player.Role == EPlayerRole::Wanderer);
		TestTrue(TEXT("Its new eye knows where it stands and the lanes around it in its galaxy row, in Lanes order"),
			IDs(State->Knowing.KnownSystems) == SystemIDs(*State, { 0, 4, 3, 5 }));
		TestFalse(TEXT("It meets nobody by it: that is the next look's"), HasMet(*State, State->Civs[1].CivID));
	}

	// ---- A life that starts as a wanderer sees around it; Install never writes the first contact ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the wanderer's birth is laid"), State))
		{
			return false;
		}
		PlayerRole->Install(State->Systems[7].SystemID, State->Civs[2].CivID, EPlayerRole::Wanderer);
		TestTrue(TEXT("A wanderer born in 7 knows 7 and its lanes"), IDs(State->Knowing.KnownSystems) == SystemIDs(*State, { 0, 7, 6, 8 }));
		TestTrue(TEXT("Install writes no first contact"), State->Knowing.FirstContactCivID.IsNone());
	}

	// ---- The writers refuse a handler ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the handler is laid"), State))
		{
			return false;
		}
		AddExpectedError(TEXT("NoteMet called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		AddExpectedError(TEXT("NoteKnown called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		const FName Three = State->Civs[3].CivID;
		const FName Nine = State->Systems[9].SystemID;
		bool bTried = false;
		bool bMetInHandler = true;
		bool bKnownInHandler = true;
		{
			FScopedLogProbe Probe(*Log);
			Probe.Probe->OnLog = [&](const FCosmicLogEntry&)
			{
				if (!bTried)
				{
					bTried = true;
					bMetInHandler = Sim->NoteMet(Three);
					bKnownInHandler = Sim->NoteKnown(Nine);
				}
			};
			FFormatNamedArguments Args;
			Args.Add(TEXT("HomeSystem"), State->Systems[0].DisplayName);
			Log->PushRow(CosmicRows::LogFirstLife, 2, Args, State->Systems[0].SystemID);
		}
		TestTrue(TEXT("A handler ran"), bTried);
		TestFalse(TEXT("NoteMet refuses a handler"), bMetInHandler);
		TestFalse(TEXT("NoteKnown refuses a handler"), bKnownInHandler);
		TestFalse(TEXT("and nothing was met"), HasMet(*State, Three));
		TestFalse(TEXT("or known"), Knows(*State, 9));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLogTellScopeTest, "Cosmic_Ascension.Log.TellScope", CosmicTests::Flags)
bool FCosmicLogTellScopeTest::RunTest(const FString& Parameters)
{
	using namespace CosmicKnowingTests;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	if (!TestNotNull(TEXT("The log subsystem exists"), Log))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Log->SetTellAll(true); Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FCosmicRulesDef Rules = Knobs(Sim->GetRules());
	FGalaxyState* Laid = LayBoard(*Sim, TablesWith(Original, Rules));
	if (!TestNotNull(TEXT("The board of the scope is laid"), Laid))
	{
		return false;
	}
	FGalaxyState& State = *Laid;
	Log->SetTellAll(false);

	// The calling knows 0, 6 and 11 and has met peoples 0, 2 and 3; people 1 is unmet, people 2's system 8 unknown.
	Mark(State.Knowing.KnownSystems, State.Systems[6].SystemID);
	Mark(State.Knowing.KnownSystems, State.Systems[11].SystemID);
	Mark(State.Knowing.MetCivs, State.Civs[2].CivID);
	Mark(State.Knowing.MetCivs, State.Civs[3].CivID);
	const FName One = State.Civs[1].CivID;
	const FName Two = State.Civs[2].CivID;
	const FName Three = State.Civs[3].CivID;
	const FName Six = State.Systems[6].SystemID;
	const FName Eight = State.Systems[8].SystemID;
	const FName Eleven = State.Systems[11].SystemID;
	const FGameplayTag CivTag = Tag(TEXT("Log.Civ"));
	const int32 SimSeed = Sim->GetRng().GetCurrentSeed();

	// One line: what it left in the log and how many numbers the tell stream moved.
	struct FTold
	{
		bool bReturned = false;
		int32 Added = 0;
		bool bLastUntold = false;
		int32 Draws = -1;
	};
	int32 Serial = 0;
	auto Tell = [&](FGameplayTag Category, uint8 Severity, FName FocusCiv, FName FocusSystem, uint8 MinTier = 0, uint8 MaxTier = 0)
	{
		FCosmicLine Line;
		Line.Category = Category;
		Line.Severity = Severity;
		Line.FocusCiv = FocusCiv;
		Line.FocusSystem = FocusSystem;
		Line.SourceRow = FName(TEXT("log_test_scope"));
		Line.MinViewerTier = MinTier;
		Line.MaxViewerTier = MaxTier;
		// A sentence of its own every time, so that no variant is ever passed over.
		const TArray<FText> Variants = { FText::AsCultureInvariant(FString::Printf(TEXT("scope line %d"), ++Serial)) };
		const int32 Before = State.TellRngState;
		const int32 EntriesBefore = State.Log.Num();
		FTold Result;
		Result.bReturned = Log->PushVariants(Line, Variants, FFormatNamedArguments());
		Result.Added = State.Log.Num() - EntriesBefore;
		Result.bLastUntold = Result.Added > 0 && State.Log[EntriesBefore].bUntold;
		Result.Draws = DrawsBetween(Before, State.TellRngState);
		return Result;
	};

	// ---- The subject and the window ----
	{
		const FTold NoSubject = Tell(CivTag, 1, NAME_None, NAME_None);
		TestEqual(TEXT("A line without a subject is stored"), NoSubject.Added, 1);
		TestFalse(TEXT("and told"), NoSubject.bLastUntold);
		const FTold Unmet = Tell(CivTag, 1, One, NAME_None);
		TestEqual(TEXT("A line about an unmet people is not stored"), Unmet.Added, 0);
		TestTrue(TEXT("but PushVariants still returns true"), Unmet.bReturned);
		TestEqual(TEXT("and draws once"), Unmet.Draws, 1);
		TestEqual(TEXT("A line about a met people in an unknown system is not stored"), Tell(CivTag, 1, Two, Eight).Added, 0);
		TestEqual(TEXT("A line about a met people in a known system is stored"), Tell(CivTag, 1, Two, Six).Added, 1);
		State.Civs[3].Tier = EAscensionTier::Tier4_Transcendent;
		TestEqual(TEXT("A line about a met people three rungs above is not stored"), Tell(CivTag, 1, Three, Eleven).Added, 0);
		State.Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		TestEqual(TEXT("two rungs above, it is"), Tell(CivTag, 1, Three, Eleven).Added, 1);
		State.Civs[0].Tier = EAscensionTier::Tier4_Transcendent;
		State.Civs[3].Tier = EAscensionTier::Tier1_Planetary;
		TestEqual(TEXT("A people three rungs below the viewer is never held back"), Tell(CivTag, 1, Three, Eleven).Added, 1);
		State.Civs[0].Tier = EAscensionTier::Tier1_Planetary;
		TestEqual(TEXT("Below the window of the line it is not stored"), Tell(CivTag, 1, NAME_None, NAME_None, 3, 0).Added, 0);
		State.Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		TestEqual(TEXT("Above it neither"), Tell(CivTag, 1, NAME_None, NAME_None, 0, 1).Added, 0);
		TestEqual(TEXT("Inside it, it is"), Tell(CivTag, 1, NAME_None, NAME_None, 2, 2).Added, 1);
		State.Civs[0].Tier = EAscensionTier::Tier1_Planetary;
		TestEqual(TEXT("The calling's own line is told whatever its subject"), Tell(Tag(TEXT("Log.Calling")), 1, One, Eight).Added, 1);
		// The viewer is the vessel's people while it rules, the calling's own tier otherwise.
		MakeWanderer(State, 0, EAscensionTier::Tier3_Causal);
		State.Civs[0].Tier = EAscensionTier::Tier1_Planetary;
		TestEqual(TEXT("A wanderer sees from its own tier: inside a window its people does not reach"), Tell(CivTag, 1, NAME_None, NAME_None, 3, 0).Added, 1);
		State.Civs[0].bIsPlayerRuled = true;
		State.Player.Role = EPlayerRole::Ruler;
		State.Player.RuledCivID = State.Civs[0].CivID;
		State.Calling.PersonalTier = EAscensionTier::Tier1_Planetary;
	}

	// ---- What is kept untold, and the far lines ----
	{
		FScopedLogProbe Probe(*Log);
		int32 Announced = 0;
		Probe.Probe->OnLog = [&Announced](const FCosmicLogEntry&) { ++Announced; };

		State.Knowing.FarLineUntilTick = 0;
		const int32 EntriesBefore = State.Log.Num();
		const FTold Mythic = Tell(CivTag, 2, One, NAME_None);
		TestEqual(TEXT("A Severity 2 line about an unmet people is stored, with a far line after it"), Mythic.Added, 2);
		TestTrue(TEXT("itself untold"), Mythic.bLastUntold);
		TestTrue(TEXT("the far line an event line"), State.Log.IsValidIndex(EntriesBefore + 1) && State.Log[EntriesBefore + 1].SourceRow == CosmicRows::LogFarEvent);
		TestTrue(TEXT("with no subject"), State.Log.IsValidIndex(EntriesBefore + 1) && State.Log[EntriesBefore + 1].FocusCivID.IsNone());
		TestEqual(TEXT("The call draws twice: its line and the far line"), Mythic.Draws, 2);
		TestEqual(TEXT("Only the far line is announced"), Announced, 1);
		TestEqual(TEXT("The far event line sets its next tick"), State.Knowing.FarLineUntilTick, State.Tick + Rules.FarLineCooldownTicks);
		const FTold Again = Tell(CivTag, 2, One, NAME_None);
		TestEqual(TEXT("Within the cooldown no far event line follows"), Again.Added, 1);
		TestEqual(TEXT("and the call draws once"), Again.Draws, 1);
		State.Knowing.FarLineUntilTick = 0;
		const FTold WindowAlone = Tell(CivTag, 2, NAME_None, NAME_None, 3, 0);
		TestEqual(TEXT("A Severity 2 line out of its window alone is kept untold"), WindowAlone.Added, 1);
		TestTrue(TEXT("untold"), WindowAlone.bLastUntold);
		TestEqual(TEXT("with no far line, the cooldown over as it is"), WindowAlone.Draws, 1);
		const FTold Claim = Tell(Tag(TEXT("Log.Claim")), 0, One, NAME_None);
		TestEqual(TEXT("A claim line out of scope is kept"), Claim.Added, 1);
		TestTrue(TEXT("untold"), Claim.bLastUntold);
		const FTold Record = Tell(Tag(TEXT("Log.Record")), 1, One, NAME_None);
		TestEqual(TEXT("A record line out of scope is kept"), Record.Added, 1);
		TestTrue(TEXT("untold"), Record.bLastUntold);
		TestEqual(TEXT("Neither has a far line (Severity below 2)"), Claim.Draws + Record.Draws, 2);

		// Ascents: the third rung or above is told every time and leaves the cooldown of the far event line alone.
		const FGameplayTag AscensionTag = Tag(TEXT("Log.Ascension"));
		State.Civs[1].Tier = EAscensionTier::Tier3_Causal;
		State.Knowing.FarLineUntilTick = State.Tick + 100;
		const int64 Until = State.Knowing.FarLineUntilTick;
		const int32 BeforeAscent = State.Log.Num();
		const FTold Ascent = Tell(AscensionTag, 2, One, NAME_None);
		TestEqual(TEXT("An unmet people's ascent to the third rung is followed by the far ascent line"), Ascent.Added, 2);
		TestTrue(TEXT("(that line)"), State.Log.IsValidIndex(BeforeAscent + 1) && State.Log[BeforeAscent + 1].SourceRow == CosmicRows::LogFarAscent);
		TestEqual(TEXT("A second ascent is followed by it again"), Tell(AscensionTag, 2, One, NAME_None).Added, 2);
		TestEqual(TEXT("The far ascent line leaves the far event line's next tick alone"), State.Knowing.FarLineUntilTick, Until);
		State.Knowing.FarLineUntilTick = 0;
		State.Civs[1].Tier = EAscensionTier::Tier2_Stellar;
		const int32 BeforeLower = State.Log.Num();
		const FTold Lower = Tell(AscensionTag, 2, One, NAME_None);
		TestEqual(TEXT("An ascent to the second rung is followed by a far event line"), Lower.Added, 2);
		TestTrue(TEXT("(that line)"), State.Log.IsValidIndex(BeforeLower + 1) && State.Log[BeforeLower + 1].SourceRow == CosmicRows::LogFarEvent);
		TestEqual(TEXT("A Severity 0 line out of scope is not stored"), Tell(CivTag, 0, One, NAME_None).Added, 0);
		TestEqual(TEXT("Telling, far lines and all, never draws from the sim stream"), Sim->GetRng().GetCurrentSeed(), SimSeed);
	}

	// ---- The same sentence never follows itself in the feed: the last told line, not the last stored entry ----
	{
		const TArray<FText> Twins = { FText::AsCultureInvariant(TEXT("twin one")), FText::AsCultureInvariant(TEXT("twin two")) };
		FCosmicLine Told;
		Told.Category = CivTag;
		Told.Severity = 1;
		Told.SourceRow = FName(TEXT("log_test_twin"));
		Log->PushVariants(Told, { FText::AsCultureInvariant(TEXT("twin one")) }, FFormatNamedArguments());
		FCosmicLine Hidden = Told;
		Hidden.Severity = 2;
		Hidden.FocusCiv = One;
		State.Knowing.FarLineUntilTick = State.Tick + 1000;
		Log->PushVariants(Hidden, { FText::AsCultureInvariant(TEXT("hidden twin")) }, FFormatNamedArguments());
		TestTrue(TEXT("An untold entry stands between"), State.Log.Last().bUntold);
		// A tell stream that draws the first twin.
		int32 Seed = 1;
		for (; Seed < 1000; ++Seed)
		{
			FRandomStream Probe(Seed);
			if (Probe.RandHelper(2) == 0)
			{
				break;
			}
		}
		State.TellRngState = Seed;
		Log->PushVariants(Told, Twins, FFormatNamedArguments());
		TestEqual(TEXT("The drawn sentence equal to the last told line gives way to the next"), State.Log.Last().Message.ToString(), FString(TEXT("twin two")));
	}

	// ---- A people's breakthrough calls no place: told once the people is met, its capital unknown ----
	{
		const FName Society(TEXT("Society"));
		for (const int32 CivIndex : { 1, 2 })
		{
			State.Civs[CivIndex].Research.Add(Society, 0.4995f);
			State.Civs[CivIndex].ResearchFocus.Add(Society, 1.f);
		}
		Sim->StepOnce();
		auto Breakthrough = [&State](FName CivID)
		{
			return State.Log.IndexOfByPredicate([&State, CivID](const FCosmicLogEntry& Entry)
			{
				return Entry.Tick == State.Tick && Entry.FocusCivID == CivID && Entry.SourceRow.ToString().StartsWith(TEXT("log_breakthrough_"));
			});
		};
		const int32 Met = Breakthrough(Two);
		if (TestTrue(TEXT("A met people's breakthrough is told though its capital is not known"), Met != INDEX_NONE))
		{
			TestTrue(TEXT("The breakthrough line has no subject system"), State.Log[Met].FocusSystemID.IsNone());
		}
		TestEqual(TEXT("An unmet people's breakthrough is not (the board can tell)"), Breakthrough(One), static_cast<int32>(INDEX_NONE));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicKnowingInertTest, "Cosmic_Ascension.Sim.KnowingInert", CosmicTests::Flags)
bool FCosmicKnowingInertTest::RunTest(const FString& Parameters)
{
	using namespace CosmicKnowingTests;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	if (!TestNotNull(TEXT("The log subsystem exists"), Log))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Log->SetTellAll(true); Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FCosmicRulesDef GameRules = Sim->GetRules();

	struct FRun
	{
		uint32 World = 0;
		uint32 Whole = 0;
		int32 Rng = 0;
		int32 Known = 0;
		int32 Met = 0;
	};
	// The game's data, 3,000 ticks of seed 42, no input.
	auto Run = [&](const FCosmicRulesDef& Rules, bool bTellAll)
	{
		Sim->SetTables(WithRules(Original, Rules));
		Log->SetTellAll(bTellAll);
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		Sim->RunHeadless(3000);
		FRun Result;
		Result.World = Sim->ComputeWorldHash();
		Result.Whole = Sim->ComputeStateHash();
		Result.Rng = Sim->GetRng().GetCurrentSeed();
		Result.Known = Sim->GetState().Knowing.KnownSystems.Num();
		Result.Met = Sim->GetState().Knowing.MetCivs.Num();
		return Result;
	};

	const FRun Seeing = Run(GameRules, true);
	FCosmicRulesDef Blind = GameRules;
	Blind.SightIntervalTicks = 1000000;
	Blind.FirstSightTick = 1000000;
	const FRun NotSeeing = Run(Blind, true);
	FCosmicRulesDef Far = GameRules;
	Far.SightLanesByTier = { 10, 10, 10, 10, 10 };
	const FRun FarSeeing = Run(Far, true);
	const FRun Scoped = Run(GameRules, false);

	TestEqual(TEXT("The world is the same whether the sight stage looks or not"), NotSeeing.World, Seeing.World);
	TestEqual(TEXT("and so is the sim stream"), NotSeeing.Rng, Seeing.Rng);
	TestEqual(TEXT("The world is the same whatever the reach of the sight"), FarSeeing.World, Seeing.World);
	TestEqual(TEXT("and so is the sim stream (reach)"), FarSeeing.Rng, Seeing.Rng);
	TestEqual(TEXT("The world is the same whether the log tells all or only the scope"), Scoped.World, Seeing.World);
	TestEqual(TEXT("and so is the sim stream (scope)"), Scoped.Rng, Seeing.Rng);
	// The controls: each run did change knowing or telling, so the equal worlds mean something.
	TestTrue(TEXT("The sight stage knows more than no sight (the case can tell)"), Seeing.Known > NotSeeing.Known);
	TestTrue(TEXT("and meets more"), Seeing.Met > NotSeeing.Met);
	TestTrue(TEXT("A farther reach knows more (the case can tell)"), FarSeeing.Known > Seeing.Known);
	TestTrue(TEXT("The scope changed the telling (the case can tell)"), Scoped.Whole != Seeing.Whole);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSightGrowsOnlyTest, "Cosmic_Ascension.Sim.SightGrowsOnly", CosmicTests::Flags)
bool FCosmicSightGrowsOnlyTest::RunTest(const FString& Parameters)
{
	using namespace CosmicKnowingTests;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UCosmicSaveSubsystem* Save = Sim ? Sim->GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The save subsystem exists"), Save))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	Sim->SetTables(Original);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	const FGalaxyState& State = Sim->GetState();
	// The vessel walks from the start, a wanderer of the second rung, and travels one lane every hundred ticks: what it
	// knows grows over many looks, in an order of discovery no sort would keep.
	{
		FGalaxyState& Mutable = Sim->GetMutableState();
		Mutable.Civs[0].bIsPlayerRuled = false;
		Mutable.Player.Role = EPlayerRole::Wanderer;
		Mutable.Player.RuledCivID = NAME_None;
		Mutable.Calling.PersonalTier = EAscensionTier::Tier2_Stellar;
	}

	// Whether Now starts with Then: the same IDs at the same ticks, in the same order.
	auto KeepsPrefix = [](const TArray<FKnownMark>& Then, const TArray<FKnownMark>& Now)
	{
		if (Now.Num() < Then.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Then.Num(); ++Index)
		{
			if (Now[Index].ID != Then[Index].ID || Now[Index].Tick != Then[Index].Tick)
			{
				return false;
			}
		}
		return true;
	};

	FKnowingState Then = State.Knowing;
	int64 KnownBroken = 0;
	int64 MetBroken = 0;
	int64 FirstChanged = 0;
	for (int32 Sample = 0; Sample < 60; ++Sample)
	{
		const int32 Here = Sim->GetStateIndex().FindSystem(State, State.Player.CurrentSystemID);
		if (State.Systems.IsValidIndex(Here) && !State.Systems[Here].Lanes.IsEmpty())
		{
			const TArray<FName>& Lanes = State.Systems[Here].Lanes;
			Sim->GetMutableState().Player.TravelTargetSystemID = Lanes[Sample % Lanes.Num()];
			Sim->GetMutableState().Player.TravelTicksRemaining = 1;
		}
		Sim->RunHeadless(100);
		const FKnowingState& Now = State.Knowing;
		if (KnownBroken == 0 && !KeepsPrefix(Then.KnownSystems, Now.KnownSystems))
		{
			KnownBroken = State.Tick;
		}
		if (MetBroken == 0 && !KeepsPrefix(Then.MetCivs, Now.MetCivs))
		{
			MetBroken = State.Tick;
		}
		if (FirstChanged == 0 && !Then.FirstContactCivID.IsNone() && Now.FirstContactCivID != Then.FirstContactCivID)
		{
			FirstChanged = State.Tick;
		}
		Then = Now;
	}
	TestEqual(TEXT("Known systems never shrink or change order (the first sample that did)"), KnownBroken, int64(0));
	TestEqual(TEXT("Met peoples never shrink or change order (the first sample that did)"), MetBroken, int64(0));
	TestEqual(TEXT("The first contact, once written, never changes (the first sample that did)"), FirstChanged, int64(0));
	TestTrue(TEXT("The calling came to know systems (the run is not empty)"), State.Knowing.KnownSystems.Num() > 1);
	TSet<int64> Looks;
	for (const FKnownMark& Mark : State.Knowing.KnownSystems)
	{
		Looks.Add(Mark.Tick);
	}
	TestTrue(TEXT("over many looks (the order can tell)"), Looks.Num() >= 5);
	TestFalse(TEXT("and made its first contact"), State.Knowing.FirstContactCivID.IsNone());

	// The save keeps it as it was.
	const FKnowingState AtSave = State.Knowing;
	const FGuid GalaxyID = State.GalaxyID;
	if (TestTrue(TEXT("Save succeeds"), Save->SaveCurrentGalaxy()))
	{
		Sim->RunHeadless(500);
		if (TestTrue(TEXT("Load succeeds"), Save->LoadGalaxy(GalaxyID)))
		{
			const FKnowingState& Loaded = Sim->GetState().Knowing;
			TestTrue(TEXT("The known systems come back as they were saved"), KeepsPrefix(AtSave.KnownSystems, Loaded.KnownSystems) && Loaded.KnownSystems.Num() == AtSave.KnownSystems.Num());
			TestTrue(TEXT("and the met peoples"), KeepsPrefix(AtSave.MetCivs, Loaded.MetCivs) && Loaded.MetCivs.Num() == AtSave.MetCivs.Num());
			TestEqual(TEXT("and the first contact"), Loaded.FirstContactCivID, AtSave.FirstContactCivID);
			TestEqual(TEXT("and the next tick of the far line"), Loaded.FarLineUntilTick, AtSave.FarLineUntilTick);
		}
		Save->DeleteSave(GalaxyID);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicFirstContactTimelineTest, "Cosmic_Ascension.Sim.FirstContactTimeline", CosmicTests::Flags)
bool FCosmicFirstContactTimelineTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	Sim->SetTables(Original);
	const FCosmicRulesDef& Rules = Sim->GetRules();
	const int64 Opens = Rules.FirstSightTick;
	const int64 Latest = Opens + Rules.SightIntervalTicks;

	// Seeds 1 to 50, no input: the first contact comes at the opening of the eye or the look after it (section 7.4).
	FString Late;
	FString Early;
	FString Untold;
	for (int32 Seed = 1; Seed <= 50; ++Seed)
	{
		Sim->NewGalaxy(Seed, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		Sim->RunHeadless(Latest);
		const FGalaxyState& State = Sim->GetState();
		const FName First = State.Knowing.FirstContactCivID;
		const FKnownMark* Mark = First.IsNone() ? nullptr : State.Knowing.MetCivs.FindByPredicate([First](const FKnownMark& Each) { return Each.ID == First; });
		if (!Mark)
		{
			Late += FString::Printf(TEXT(" %d"), Seed);
			continue;
		}
		if (Mark->Tick < Opens)
		{
			Early += FString::Printf(TEXT(" %d"), Seed);
		}
		const int64 Tick = Mark->Tick;
		auto IsContactLine = [First, Tick](const FCosmicLogEntry& Entry) { return Entry.SourceRow == CosmicRows::LogFirstContact && Entry.Tick == Tick && Entry.FocusCivID == First; };
		if (!State.Log.ContainsByPredicate(IsContactLine) && !State.Chronicle.ContainsByPredicate(IsContactLine))
		{
			Untold += FString::Printf(TEXT(" %d"), Seed);
		}
	}
	TestEqual(TEXT("Every seed has made its first contact by FirstSightTick + SightIntervalTicks (seeds that had not)"), Late, FString());
	TestEqual(TEXT("and none before FirstSightTick - Phase 2 and 3 (seeds that had)"), Early, FString());
	TestEqual(TEXT("The first contact is told by log_first_contact at its tick (seeds where it was not)"), Untold, FString());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

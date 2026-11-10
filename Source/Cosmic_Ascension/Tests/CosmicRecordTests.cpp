// Cosmic Ascension - automation tests of the first part of the two books (milestone B item B5, [B-5d1]): the record line of
// a people's irreversible loss - one Log.Record line per settled system destroyed and per system a claim joins to a fallen
// sphere, told by the crisis stage right after the expiry line and before a claim's sign and first person, kept untold
// whatever the scope, for every people whatever its faith traits, and never by the cleanup of the crossing stage.
// Design: docs/60_DECISIONS/headers/B5_death_succession_two_books.md section 2.12, 3.3, 6 and decisions 10, 11.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Log/CosmicLogSubsystem.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "Tests/CosmicTestProbe.h"

#include <initializer_list>

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicRecordTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// The rows of these tests.
	const FName Nova(TEXT("crisis_test_record_nova"));
	const FName Claim(TEXT("crisis_test_record_claim"));
	const FName Plague(TEXT("crisis_test_record_plague"));
	const FName Fallen(TEXT("tr_test_record_fallen"));

	// A hazard row no draw places (chance 0), whose lines name the moment, so a case can tell an expiry from a resolution.
	FHazardDef HazardRow(const TCHAR* TagName, ECosmicHazardSpawn Rule)
	{
		auto Lines = [TagName](const TCHAR* Moment)
		{
			TArray<FText> Variants;
			for (const TCHAR* Number : { TEXT("one"), TEXT("two"), TEXT("three") })
			{
				Variants.Add(FText::AsCultureInvariant(FString::Printf(TEXT("%s %s %s"), TagName, Moment, Number)));
			}
			return Variants;
		};
		FHazardDef Row;
		Row.HazardTag = Tag(TagName);
		Row.DisplayName = FText::AsCultureInvariant(FString::Printf(TEXT("<%s>"), TagName));
		Row.CountdownTicks = 1;
		Row.SpawnRule = Rule;
		Row.SpawnChancePerTick = 0.f;
		Row.LogCategory = Tag(TEXT("Log.Crisis"));
		Row.Severity = 1;
		Row.OutbreakTemplates = Lines(TEXT("outbreak"));
		Row.ExpireTemplates = Lines(TEXT("expiry"));
		Row.ResolveTemplates = Lines(TEXT("resolution"));
		return Row;
	}

	/**
	 * The game's tables with: the rules without the inherited edict, the event slot, the first crisis, the periodic petition,
	 * drift, fleets, spread and settlement, claims one lane out; no projects or feats; three hazard rows of the tests (a
	 * supernova, a claim and a plague, none drawn); the game's grace of the remnant as the one event row; one fallen row of
	 * the fourth rung that speaks when it joins; no card; the existence bricks inert (B7x - CosmicTests::WithInertBricks;
	 * LayBoard strips the bricks off the peoples). With bRecordRows false, the game's DT_LogTemplates without the two
	 * record rows (CosmicRows::LogRecordDestroyed, LogRecordAnnexed) - the same world told without its record.
	 */
	FCosmicDataTables TablesOf(const FCosmicDataTables& Original, bool bRecordRows)
	{
		const FCosmicRulesDef* GameRules = Original.FindRules();
		FCosmicRulesDef Rules = CosmicTests::WithInertBricks(CosmicTests::WithoutStartingEdict(GameRules ? *GameRules : FCosmicRulesDef()));
		Rules.EventSlotTicks = 10;
		Rules.EventChancePerSlot = 0.f;
		Rules.FirstCrisisTick = 0;
		Rules.PetitionPeriodTicks = 100000;
		Rules.SpiritualPressureStabilityDrain = 0.f;
		Rules.RelationDriftRate = 0.f;
		Rules.FleetPerSettledByTier = { 0.f, 0.f, 0.f, 0.f, 0.f };
		Rules.PlagueSpreadMultiplier = 1.f;
		Rules.ClaimAdjacencyLanes = 1;
		Rules.RealmCapByTier = { 1, 1, 1, 1, 1 };
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));

		UDataTable* Hazards = CosmicTests::NewTable(FHazardDef::StaticStruct());
		Hazards->AddRow(Nova, HazardRow(TEXT("Hazard.PreSupernova"), ECosmicHazardSpawn::Draw));
		Hazards->AddRow(Claim, HazardRow(TEXT("Hazard.Claim"), ECosmicHazardSpawn::ClaimRule));
		Hazards->AddRow(Plague, HazardRow(TEXT("Hazard.Plague"), ECosmicHazardSpawn::Draw));
		Tables.Hazards = Hazards;

		UDataTable* Events = CosmicTests::NewTable(FCosmicEventDef::StaticStruct());
		if (const FCosmicEventDef* Grace = Original.FindEvent(CosmicRows::EventGraceRemnant))
		{
			Events->AddRow(CosmicRows::EventGraceRemnant, *Grace);
		}
		Tables.Events = Events;

		FTranscendentDef Speaker;
		Speaker.Doctrine = Tag(TEXT("Doctrine.Silence"));
		Speaker.Allegiance = Tag(TEXT("Realm.Abyssal"));
		Speaker.Tier = EAscensionTier::Tier4_Transcendent;
		Speaker.Depth = 0.f;
		Speaker.MaxInfluencedSystems = 12;
		Speaker.SpawnWeight = 0.f;
		Speaker.ClaimTemplates = { FText::AsCultureInvariant(TEXT("voice of {System} one")), FText::AsCultureInvariant(TEXT("voice of {System} two")),
			FText::AsCultureInvariant(TEXT("voice of {System} three")) };
		UDataTable* Transcendents = CosmicTests::NewTable(FTranscendentDef::StaticStruct());
		Transcendents->AddRow(Fallen, Speaker);
		Tables.Transcendents = Transcendents;

		Tables.Encounters = CosmicTests::NewTable(FEncounterDef::StaticStruct());
		if (!bRecordRows)
		{
			Tables.LogTemplates = CosmicTests::CopyWithout<FLogTemplateDef>(Original.LogTemplates, { CosmicRows::LogRecordDestroyed, CosmicRows::LogRecordAnnexed });
		}
		return Tables;
	}

	/**
	 * The board of these tests: the game's galaxy of seed 42 with twelve systems on the chain of CosmicTests::WireChain (0-1-
	 * ...-9, 4-10, 10-11). The vessel rules people 0, which holds 0, 1 and 2 with its capital in 0, and stands in 0; people 1
	 * holds 6, people 2 holds 7 and 8, people 3 holds 11; the rest is wild. Every people on the first rung at Stability 0.75
	 * with no notable or edict and no faith trait; no hazard, project, transcendent or flag; the calling knows the board.
	 * Null when the galaxy does not give that.
	 */
	FGalaxyState* LayBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Tables)
	{
		Sim.SetTables(Tables);
		Sim.NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim.GetMutableState();
		CosmicTests::StripBricks(State);
		if (!CosmicTests::WireChain(State) || State.Civs.Num() != 4 || State.Player.RuledCivID != State.Civs[0].CivID)
		{
			return nullptr;
		}
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { 0, 1, 2 });
		CosmicTests::Give(State, 1, { 6 });
		CosmicTests::Give(State, 2, { 7, 8 });
		CosmicTests::Give(State, 3, { 11 });
		State.Transcendents.Reset();
		State.Flags.Reset();
		for (FStarSystemState& System : State.Systems)
		{
			CosmicHazards::EmptyHazardSlot(System);
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
		}
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.Tier = EAscensionTier::Tier1_Planetary;
			Civ.Stability = 0.75f;
			Civ.LowStabilityTicks = 0;
			Civ.Notables.Reset();
			Civ.ActiveEdicts.Reset();
			TArray<FGameplayTag> Faith;
			for (const FGameplayTag& Trait : Civ.Traits)
			{
				if (Trait.ToString().StartsWith(TEXT("Trait.Faith.")))
				{
					Faith.Add(Trait);
				}
			}
			for (const FGameplayTag& Trait : Faith)
			{
				Civ.Traits.RemoveTag(Trait);
			}
		}
		State.Player.CurrentSystemID = State.Systems[0].SystemID;
		CosmicTests::KnowAll(State);
		return &State;
	}

	void PlaceByHand(FGalaxyState& State, int32 SystemIndex, const TCHAR* HazardTag)
	{
		State.Systems[SystemIndex].ActiveHazard = Tag(HazardTag);
		State.Systems[SystemIndex].HazardTicksRemaining = 1;
	}

	void AddFallen(FGalaxyState& State, std::initializer_list<int32> Sphere)
	{
		TArray<FName> Systems;
		for (const int32 SystemIndex : Sphere)
		{
			Systems.Add(State.Systems[SystemIndex].SystemID);
		}
		CosmicTests::AddInstance(State, Fallen, Systems).Doctrine = Tag(TEXT("Doctrine.Silence"));
	}

	// Index into State.Log of the first line of Row at the current tick whose text carries Word. INDEX_NONE for none.
	int32 LineAt(const FGalaxyState& State, FName Row, const TCHAR* Word)
	{
		for (int32 Index = 0; Index < State.Log.Num(); ++Index)
		{
			const FCosmicLogEntry& Entry = State.Log[Index];
			if (Entry.Tick == State.Tick && Entry.SourceRow == Row && Entry.Message.ToString().Contains(Word))
			{
				return Index;
			}
		}
		return INDEX_NONE;
	}

	// The record lines at the current tick, about SystemID (any when None).
	int32 RecordsAt(const FGalaxyState& State, FName SystemID = NAME_None)
	{
		int32 Count = 0;
		for (const FCosmicLogEntry& Entry : State.Log)
		{
			Count += (Entry.Tick == State.Tick && Entry.Category == Tag(TEXT("Log.Record")) && (SystemID.IsNone() || Entry.FocusSystemID == SystemID)) ? 1 : 0;
		}
		return Count;
	}

	// Whether every first-person line of a claim (Log.Claim - the second part of the two books), in the log and in the
	// chronicle, has a record line (Log.Record - the first part) about the same system: the first part holds the second.
	bool EveryClaimRecorded(const FGalaxyState& State)
	{
		TSet<FName> Recorded;
		for (const TArray<FCosmicLogEntry>* Lines : { &State.Chronicle, &State.Log })
		{
			for (const FCosmicLogEntry& Entry : *Lines)
			{
				if (Entry.Category == Tag(TEXT("Log.Record")))
				{
					Recorded.Add(Entry.FocusSystemID);
				}
			}
		}
		for (const TArray<FCosmicLogEntry>* Lines : { &State.Chronicle, &State.Log })
		{
			for (const FCosmicLogEntry& Entry : *Lines)
			{
				if (Entry.Category == Tag(TEXT("Log.Claim")) && !Recorded.Contains(Entry.FocusSystemID))
				{
					return false;
				}
			}
		}
		return true;
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
}

// The first part holds the second (B5 header section 6, decisions 10-ga and 11): a claim that joins a sphere is recorded
// once - the expiry line, the record line (the game's log_record_annexed: Log.Record, Severity 1, about the system and the
// people that lost it, {System} alone), the sign, the first person - and the record line is kept untold and never announced,
// with the tell-all mode on and off; a settled system destroyed is recorded once, right after its expiry line; an expiry that
// takes nothing (a plague, a supernova the grace keeps, a claim that joins nothing) records nothing, nor does the destruction
// of a system not settled (one under its settlement, a wild one - set by hand); on a board whose grace
// was used, the tick that takes a people's last system and joins another system records each loss once, and no record line
// follows the first person - the cleanup of the crossing stage that takes the people away records nothing; every first
// person has its record; the record lines move nothing of the world and of the sim stream and draw one tell number each.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLogTwoBooksSameLossTest, "Cosmic_Ascension.Log.TwoBooksSameLoss", CosmicTests::Flags)
bool FCosmicLogTwoBooksSameLossTest::RunTest(const FString& Parameters)
{
	using namespace CosmicRecordTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UCosmicLogSubsystem* Log = Sim ? Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("(the log subsystem)"), Log))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const bool bWasTellAll = Log->IsTellAll();
	ON_SCOPE_EXIT
	{
		Log->SetTellAll(bWasTellAll);
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
	};
	const FLogTemplateDef* Destroyed = Original.FindLogTemplate(CosmicRows::LogRecordDestroyed);
	const FLogTemplateDef* Annexed = Original.FindLogTemplate(CosmicRows::LogRecordAnnexed);
	if (!TestNotNull(TEXT("(the game has the record row log_record_destroyed)"), Destroyed) || !TestNotNull(TEXT("(the game has the record row log_record_annexed)"), Annexed)
		|| !TestNotNull(TEXT("(the game has the sign row log_sign_claim)"), Original.FindLogTemplate(CosmicRows::LogSignClaim))
		|| !TestNotNull(TEXT("(the game has the grace row of the remnant)"), Original.FindEvent(CosmicRows::EventGraceRemnant)))
	{
		return false;
	}
	const FGameplayTag RecordTag = Tag(TEXT("Log.Record"));
	TestTrue(TEXT("The record rows are Log.Record rows"), Destroyed->Category == RecordTag && Annexed->Category == RecordTag);
	const FCosmicDataTables Tables = TablesOf(Original, true);

	// ---- A claim joins: the expiry, the record, the sign, the first person; the record untold in both modes ----
	for (const bool bTellAll : { true, false })
	{
		Log->SetTellAll(bTellAll);
		const TCHAR* Mode = bTellAll ? TEXT("tell-all") : TEXT("the scope");
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(*FString::Printf(TEXT("(the board of the join - %s)"), Mode), State))
		{
			return false;
		}
		// The fallen holds the wild 3, next to 2: its claim on 2 joins (0 and 1 stay the people's unclaimed systems).
		AddFallen(*State, { 3 });
		PlaceByHand(*State, 2, TEXT("Hazard.Claim"));
		const FName SystemID = State->Systems[2].SystemID;
		int32 Announced = 0;
		{
			CosmicTests::FScopedProbe Probe(*Sim);
			Probe.Probe->OnLog = [&Announced, RecordTag](const FCosmicLogEntry& Entry) { Announced += Entry.Category == RecordTag ? 1 : 0; };
			Sim->StepOnce();
		}
		if (!TestTrue(*FString::Printf(TEXT("(the claim joined the sphere - %s)"), Mode), State->Transcendents[0].InfluencedSystems.Contains(SystemID)))
		{
			return false;
		}
		const int32 Expiry = LineAt(*State, Claim, TEXT("expiry"));
		if (!TestTrue(*FString::Printf(TEXT("(the expiry line and three lines after it - %s)"), Mode), Expiry != INDEX_NONE && State->Log.IsValidIndex(Expiry + 3)))
		{
			return false;
		}
		const FCosmicLogEntry& Record = State->Log[Expiry + 1];
		TestEqual(*FString::Printf(TEXT("The record line follows the expiry line - %s"), Mode), Record.SourceRow, CosmicRows::LogRecordAnnexed);
		TestTrue(TEXT("a Log.Record line"), Record.Category == RecordTag);
		TestEqual(TEXT("of Severity 1"), static_cast<int32>(Record.Severity), 1);
		TestEqual(TEXT("about the system joined"), Record.FocusSystemID, SystemID);
		TestEqual(TEXT("and the people that lost it"), Record.FocusCivID, State->Civs[0].CivID);
		TestTrue(TEXT("naming the system"), Record.Message.ToString().Contains(State->Systems[2].DisplayName.ToString()));
		TestTrue(*FString::Printf(TEXT("kept untold, though the board is known - %s"), Mode), Record.bUntold);
		TestEqual(*FString::Printf(TEXT("and never announced - %s"), Mode), Announced, 0);
		TestTrue(TEXT("The sign follows the record line"), State->Log[Expiry + 2].Category == Tag(TEXT("Log.Sign")));
		TestTrue(TEXT("and the first person follows the sign"), State->Log[Expiry + 3].Category == Tag(TEXT("Log.Claim")) && State->Log[Expiry + 3].FocusSystemID == SystemID);
		TestEqual(TEXT("One loss, one record line"), RecordsAt(*State), 1);
		TestTrue(TEXT("The first person has its record"), EveryClaimRecorded(*State));
	}
	Log->SetTellAll(true);

	// ---- A settled system destroyed: one record line, right after its expiry line ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("(the board of the destruction)"), State))
		{
			return false;
		}
		PlaceByHand(*State, 1, TEXT("Hazard.PreSupernova"));
		const FName SystemID = State->Systems[1].SystemID;
		Sim->StepOnce();
		TestTrue(TEXT("(the system is destroyed)"), CosmicHazards::IsDeadNode(State->Systems[1]));
		const int32 Expiry = LineAt(*State, Nova, TEXT("expiry"));
		if (TestTrue(TEXT("(the expiry line and one after it - the destruction)"), Expiry != INDEX_NONE && State->Log.IsValidIndex(Expiry + 1)))
		{
			const FCosmicLogEntry& Record = State->Log[Expiry + 1];
			TestEqual(TEXT("The record line of the destruction follows its expiry line"), Record.SourceRow, CosmicRows::LogRecordDestroyed);
			TestTrue(TEXT("a Log.Record line"), Record.Category == RecordTag);
			TestEqual(TEXT("about the destroyed system"), Record.FocusSystemID, SystemID);
			TestEqual(TEXT("and the people that lost it"), Record.FocusCivID, State->Civs[0].CivID);
			TestTrue(TEXT("kept untold"), Record.bUntold);
		}
		TestEqual(TEXT("One destruction, one record line"), RecordsAt(*State), 1);
	}

	// ---- Nothing lost, nothing recorded: a plague, a last system the grace keeps, a claim that joins nothing ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("(the board of the losses that are not)"), State))
		{
			return false;
		}
		// 0 and 1 lie in the sphere: 2 is people 0's last unclaimed settled system, which no claim joins (Law.Remnant).
		AddFallen(*State, { 3, 0, 1 });
		PlaceByHand(*State, 6, TEXT("Hazard.Plague"));
		PlaceByHand(*State, 11, TEXT("Hazard.PreSupernova"));
		PlaceByHand(*State, 2, TEXT("Hazard.Claim"));
		Sim->StepOnce();
		TestFalse(TEXT("(the grace kept the last system)"), CosmicHazards::IsDeadNode(State->Systems[11]));
		TestFalse(TEXT("(the claim joined nothing)"), State->Transcendents[0].InfluencedSystems.Contains(State->Systems[2].SystemID));
		TestTrue(TEXT("(the three expired - the plague's line)"), LineAt(*State, Plague, TEXT("expiry")) != INDEX_NONE);
		TestEqual(TEXT("An expiry that takes nothing records nothing"), RecordsAt(*State), 0);
	}

	// ---- Only a settled system's destruction is recorded (decision 11-ga - the guard in ExpireHazard): a supernova set by
	// hand (CosmicHazards::Place refuses both) on a system its people still settles (project_settle in its slot) and on a
	// wild one destroys them and records neither ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("(the board of the unsettled destructions)"), State))
		{
			return false;
		}
		// 1 is people 0's, under its settlement (the tables have no project row: the slot keeps it); 5 is wild.
		State->Systems[1].ActiveMegaproject = CosmicRows::ProjectSettle;
		PlaceByHand(*State, 1, TEXT("Hazard.PreSupernova"));
		PlaceByHand(*State, 5, TEXT("Hazard.PreSupernova"));
		const FName Settling = State->Systems[1].SystemID;
		const FName Wild = State->Systems[5].SystemID;
		TestTrue(TEXT("(1 is owned and not settled, 5 is wild)"), !State->Systems[1].OwnerCivID.IsNone() && !CosmicState::IsSettled(State->Systems[1])
			&& State->Systems[5].OwnerCivID.IsNone());
		Sim->StepOnce();
		TestTrue(TEXT("(both are destroyed)"), CosmicHazards::IsDeadNode(State->Systems[1]) && CosmicHazards::IsDeadNode(State->Systems[5]));
		TestTrue(TEXT("(both expired - their lines)"), LineAt(*State, Nova, TEXT("expiry")) != INDEX_NONE);
		TestEqual(TEXT("An owned system under its settlement destroyed: no record line"), RecordsAt(*State, Settling), 0);
		TestEqual(TEXT("a wild system destroyed: no record line"), RecordsAt(*State, Wild), 0);
		TestEqual(TEXT("(no record line at all)"), RecordsAt(*State), 0);
	}

	// ---- The run's grace used: a last system destroyed and a system joined on one tick ----
	auto GraceUsed = [&](const FCosmicDataTables& Of) -> FGalaxyState*
	{
		FGalaxyState* State = LayBoard(*Sim, Of);
		if (!State)
		{
			return nullptr;
		}
		State->Flags.AddTag(Tag(TEXT("Flag.Grace.Spent")));
		// 6 is people 1's only system; the fallen holds the wild 9, next to 8 (people 2 keeps 7 unclaimed). 6 runs out first.
		AddFallen(*State, { 9 });
		PlaceByHand(*State, 6, TEXT("Hazard.PreSupernova"));
		PlaceByHand(*State, 8, TEXT("Hazard.Claim"));
		Sim->StepOnce();
		return State;
	};
	{
		FGalaxyState* StateLaid = LayBoard(*Sim, Tables);
		const FName LostPeople = StateLaid ? StateLaid->Civs[1].CivID : NAME_None;
		const FName Last = StateLaid ? StateLaid->Systems[6].SystemID : NAME_None;
		const FName Joined = StateLaid ? StateLaid->Systems[8].SystemID : NAME_None;
		FGalaxyState* State = GraceUsed(Tables);
		if (!TestNotNull(TEXT("(the board of the used grace)"), State))
		{
			return false;
		}
		TestTrue(TEXT("(the last system went)"), CosmicHazards::IsDeadNode(State->Systems[6]));
		TestTrue(TEXT("(the claim joined 8)"), State->Transcendents[0].InfluencedSystems.Contains(Joined));
		TestEqual(TEXT("(the people that lost its last system left in the crossing stage of the tick)"), Sim->GetStateIndex().FindCiv(*State, LostPeople), static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("The last system destroyed is recorded once"), RecordsAt(*State, Last), 1);
		TestEqual(TEXT("the system joined once"), RecordsAt(*State, Joined), 1);
		TestEqual(TEXT("and nothing else is recorded: two losses, two record lines"), RecordsAt(*State), 2);
		const int32 Expiry = LineAt(*State, Claim, TEXT("expiry"));
		int32 Voice = INDEX_NONE;
		for (int32 Index = 0; Index < State->Log.Num(); ++Index)
		{
			if (State->Log[Index].Tick == State->Tick && State->Log[Index].Category == Tag(TEXT("Log.Claim")))
			{
				Voice = Index;
			}
		}
		if (TestTrue(TEXT("(the join's lines)"), Expiry != INDEX_NONE && Voice == Expiry + 3))
		{
			TestTrue(TEXT("On that tick too the join is told expiry, record, sign, first person"),
				State->Log[Expiry + 1].Category == RecordTag && State->Log[Expiry + 2].Category == Tag(TEXT("Log.Sign")));
		}
		bool bRecordAfterVoice = false;
		for (int32 Index = Voice + 1; Voice != INDEX_NONE && Index < State->Log.Num(); ++Index)
		{
			bRecordAfterVoice |= State->Log[Index].Tick == State->Tick && State->Log[Index].Category == RecordTag;
		}
		TestFalse(TEXT("No record line follows the first person: the cleanup that took the people away records nothing"), bRecordAfterVoice);
		TestTrue(TEXT("Every first person has its record"), EveryClaimRecorded(*State));

		// The same tick without the record rows: the same world and the same sim stream; two tell numbers fewer.
		const uint32 World = Sim->ComputeWorldHash();
		const uint32 Full = Sim->ComputeStateHash();
		const int32 SimSeed = State->RngState;
		const int32 Tell = State->TellRngState;
		FGalaxyState* Unrecorded = GraceUsed(TablesOf(Original, false));
		if (TestNotNull(TEXT("(the board of the used grace without the record rows)"), Unrecorded))
		{
			TestEqual(TEXT("(no record line without the rows)"), RecordsAt(*Unrecorded), 0);
			TestEqual(TEXT("The record lines move nothing of the world"), Sim->ComputeWorldHash(), World);
			TestEqual(TEXT("nor of the sim stream"), Unrecorded->RngState, SimSeed);
			TestEqual(TEXT("They draw one tell number each"), DrawsBetween(Unrecorded->TellRngState, Tell), 2);
			TestNotEqual(TEXT("and the story holds them (the full hash)"), Sim->ComputeStateHash(), Full);
		}
	}
	return true;
}

// Every people's loss (B5 decision 11-ga): the same two losses - a system a claim joins, a settled system destroyed - give the
// same record lines whether the peoples that lose them hold faith traits or none at all; the first part of the two books is
// no roll of the faithful.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLogRecordEveryPeopleTest, "Cosmic_Ascension.Log.RecordEveryPeople", CosmicTests::Flags)
bool FCosmicLogRecordEveryPeopleTest::RunTest(const FString& Parameters)
{
	using namespace CosmicRecordTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
	};
	if (!TestNotNull(TEXT("(the game has the record row log_record_destroyed)"), Original.FindLogTemplate(CosmicRows::LogRecordDestroyed))
		|| !TestNotNull(TEXT("(the game has the record row log_record_annexed)"), Original.FindLogTemplate(CosmicRows::LogRecordAnnexed)))
	{
		return false;
	}
	const FCosmicDataTables Tables = TablesOf(Original, true);
	// The records of the two losses: of 2 (joined - people 0) and of 7 (destroyed - people 2, which keeps 8), with the
	// peoples that lose them holding Faith or nothing.
	auto Losses = [&](std::initializer_list<const TCHAR*> Faith, int32& OutJoined, int32& OutDestroyed) -> bool
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!State)
		{
			return false;
		}
		for (const int32 CivIndex : { 0, 2 })
		{
			for (const TCHAR* Trait : Faith)
			{
				State->Civs[CivIndex].Traits.AddTag(Tag(Trait));
			}
		}
		AddFallen(*State, { 3 });
		PlaceByHand(*State, 2, TEXT("Hazard.Claim"));
		PlaceByHand(*State, 7, TEXT("Hazard.PreSupernova"));
		const FName JoinedID = State->Systems[2].SystemID;
		const FName DestroyedID = State->Systems[7].SystemID;
		Sim->StepOnce();
		if (!State->Transcendents[0].InfluencedSystems.Contains(JoinedID) || !CosmicHazards::IsDeadNode(State->Systems[7]))
		{
			return false;
		}
		OutJoined = RecordsAt(*State, JoinedID);
		OutDestroyed = RecordsAt(*State, DestroyedID);
		return true;
	};
	int32 PlainJoined = -1;
	int32 PlainDestroyed = -1;
	int32 FaithJoined = -2;
	int32 FaithDestroyed = -2;
	if (!TestTrue(TEXT("(the losses of peoples with no faith trait)"), Losses({}, PlainJoined, PlainDestroyed))
		|| !TestTrue(TEXT("(the losses of peoples with faith traits)"), Losses({ TEXT("Trait.Faith.Remnant"), TEXT("Trait.Faith.Seeking") }, FaithJoined, FaithDestroyed)))
	{
		return false;
	}
	TestEqual(TEXT("A people with no faith trait has its join recorded"), PlainJoined, 1);
	TestEqual(TEXT("and its destruction"), PlainDestroyed, 1);
	TestEqual(TEXT("A people with faith traits has the same record of its join"), FaithJoined, PlainJoined);
	TestEqual(TEXT("and of its destruction"), FaithDestroyed, PlainDestroyed);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

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

	FCosmicEffect Eff(EEffectType Type, const TCHAR* TagName = nullptr, FName Key = NAME_None, float Value = 0.f)
	{
		FCosmicEffect Effect;
		Effect.Type = Type;
		Effect.Tag = TagName ? Tag(TagName) : FGameplayTag();
		Effect.Key = Key;
		Effect.Value = Value;
		return Effect;
	}

	// A hazard of the tests that never spawns by itself: Countdown ticks, one line of each kind that names its system.
	FHazardDef HazardRow(const TCHAR* TagName, int32 Countdown, const TCHAR* Stem)
	{
		FHazardDef Row;
		Row.HazardTag = Tag(TagName);
		Row.DisplayName = FText::AsCultureInvariant(FString::Printf(TEXT("<%s>"), Stem));
		Row.CountdownTicks = Countdown;
		Row.SpawnRule = ECosmicHazardSpawn::Draw;
		Row.SpawnChancePerTick = 0.f;
		Row.LogCategory = Tag(TEXT("Log.Crisis"));
		Row.Severity = 1;
		Row.OutbreakTemplates = { FText::AsCultureInvariant(FString::Printf(TEXT("%s outbreak {System}"), Stem)) };
		Row.ExpireTemplates = { FText::AsCultureInvariant(FString::Printf(TEXT("%s expiry {System}"), Stem)) };
		Row.ResolveTemplates = { FText::AsCultureInvariant(FString::Printf(TEXT("%s resolution {System}"), Stem)) };
		return Row;
	}

	// The game's row RowName of DT_Events or DT_Hazards, copied into a table of the test. False when the game lacks it.
	bool CopyEvent(const FCosmicDataTables& Original, UDataTable* Into, FName RowName)
	{
		const FCosmicEventDef* Row = Original.FindEvent(RowName);
		if (Row)
		{
			Into->AddRow(RowName, *Row);
		}
		return Row != nullptr;
	}

	bool CopyHazard(const FCosmicDataTables& Original, UDataTable* Into, FName RowName)
	{
		const FHazardDef* Row = Original.Hazards ? Original.Hazards->FindRow<FHazardDef>(RowName, TEXT("CopyHazard")) : nullptr;
		if (Row)
		{
			Into->AddRow(RowName, *Row);
		}
		return Row != nullptr;
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

	// Whether some told or untold entry carries Text.
	bool AnyLineSays(const FGalaxyState& State, const FString& Text)
	{
		return State.Log.ContainsByPredicate([&Text](const FCosmicLogEntry& Entry) { return Entry.Message.ToString().Contains(Text); });
	}

	// How many numbers a stream moved from Before to After, or -1 beyond four.
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
	UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	if (!TestNotNull(TEXT("The log subsystem exists"), Log))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Log->SetTellAll(true); Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FCosmicRulesDef Rules = Knobs(Sim->GetRules());
	const FCosmicDataTables Tables = TablesWith(Original, Rules);

	// ---- A ruler's eye opens at FirstSightTick; the first meeting is the first contact (the game's way of telling) ----
	Log->SetTellAll(false);
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
		if (TestTrue(TEXT("and sees one lane on Tier 1"), IDs(State->Knowing.KnownSystems) == SystemIDs(*State, { 0, 1 })))
		{
			TestEqual(TEXT("What the eye sees is marked with the tick it opened"), State->Knowing.KnownSystems[1].Tick, static_cast<int64>(Rules.FirstSightTick));
		}
		TestEqual(TEXT("The owner of the system seen is the first contact"), State->Knowing.FirstContactCivID, One);
		const FKnownMark* Met = State->Knowing.MetCivs.FindByPredicate([One](const FKnownMark& Mark) { return Mark.ID == One; });
		if (TestNotNull(TEXT("The meeting is on the record"), Met))
		{
			TestEqual(TEXT("at its tick"), Met->Tick, Opened);
		}
		const int32 Line = FindLine(*State, CosmicRows::LogFirstContact, Opened);
		if (TestTrue(TEXT("The first contact is told, the log judging its scope"), Line != INDEX_NONE))
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
		if (TestTrue(TEXT("A later meeting is told as a meeting, the log judging its scope"), Contact != INDEX_NONE))
		{
			TestEqual(TEXT("about that people"), State->Log[Contact].FocusCivID, One);
			TestEqual(TEXT("and where it was seen"), State->Log[Contact].FocusSystemID, State->Systems[2].SystemID);
		}
	}
	Log->SetTellAll(true);

	// ---- Before FirstSightTick a ruler's eye adds nothing, but the owner of a system it knows is met at the next look ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the early meeting is laid"), State))
		{
			return false;
		}
		// A system known before the eye opens (in the game: a system that changed hands by an effect).
		TestTrue(TEXT("(people 1's system 2 is known by hand)"), Sim->NoteKnown(State->Systems[2].SystemID));
		Sim->RunHeadless(Rules.SightIntervalTicks);
		TestTrue(TEXT("Before FirstSightTick the owner of a known system is met at the next look"), HasMet(*State, State->Civs[1].CivID));
		TestEqual(TEXT("as the first contact"), State->Knowing.FirstContactCivID, State->Civs[1].CivID);
		TestEqual(TEXT("while the eye adds no system"), State->Knowing.KnownSystems.Num(), 2);
	}

	// ---- A people's distance is that of its nearest known system, and a known system beyond the reach counts its lanes ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the nearest system is laid"), State))
		{
			return false;
		}
		const FName One = State->Civs[1].CivID;
		const FName Two = State->Civs[2].CivID;
		CosmicTests::ClearOwners(*State);
		CosmicTests::Give(*State, 0, { 0 });
		// People 1 holds 1 (one lane) and 7 (seven lanes, known first); people 2 holds 2 (two lanes).
		CosmicTests::Give(*State, 1, { 1, 7 });
		CosmicTests::Give(*State, 2, { 2 });
		CosmicTests::Give(*State, 3, { 11 });
		State->Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		// Known by hand just before the eye opens, so that both peoples are met in the same look.
		Sim->RunHeadless(Rules.FirstSightTick - 1);
		TestTrue(TEXT("(people 1's far system 7 is known first, by hand)"), Sim->NoteKnown(State->Systems[7].SystemID));
		Sim->StepOnce();
		TestTrue(TEXT("A people is as near as its nearest known system, not its first known one"),
			IDs(State->Knowing.MetCivs) == TArray<FName>({ State->Civs[0].CivID, One, Two }));
		const int32 First = FindLine(*State, CosmicRows::LogFirstContact, State->Tick);
		if (TestTrue(TEXT("The first contact is told"), First != INDEX_NONE))
		{
			TestEqual(TEXT("at that nearest system"), State->Log[First].FocusSystemID, State->Systems[1].SystemID);
		}
	}

	// ---- A known system the walk cannot reach is the farthest, and one beyond the reach counts its true lanes ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the unreachable system is laid"), State))
		{
			return false;
		}
		const FName One = State->Civs[1].CivID;
		const FName Two = State->Civs[2].CivID;
		if (!TestTrue(TEXT("The board's peoples 1 and 2 are in that order by ID"), FNameLexicalLess()(One, Two)))
		{
			return false;
		}
		CosmicTests::ClearOwners(*State);
		CosmicTests::Give(*State, 0, { 0 });
		// People 1 (the lower ID) holds 10, in another galaxy row: known by hand, out of the walk. People 2 holds 5: known
		// by hand, five lanes off, beyond the reach of Tier 1.
		CosmicTests::Give(*State, 1, { 10 });
		CosmicTests::Give(*State, 2, { 5 });
		CosmicTests::Give(*State, 3, { 11 });
		State->Systems[10].GalaxyRow = FName(TEXT("gal_test_other"));
		TestTrue(TEXT("(system 10 is known by hand)"), Sim->NoteKnown(State->Systems[10].SystemID));
		TestTrue(TEXT("(system 5 is known by hand)"), Sim->NoteKnown(State->Systems[5].SystemID));
		// Both are met in the first look, before the eye opens.
		Sim->RunHeadless(Rules.SightIntervalTicks);
		TestEqual(TEXT("A people beyond the reach is met before one the walk cannot reach, whatever their IDs"), State->Knowing.FirstContactCivID, Two);
		TestTrue(TEXT("and the unreachable one after it"), IDs(State->Knowing.MetCivs) == TArray<FName>({ State->Civs[0].CivID, Two, One }));
	}

	// ---- A ruler's eye stands only in systems its people owns ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the stale entry is laid"), State))
		{
			return false;
		}
		// System 6 is people 1's, yet the ruled people lists it too.
		CosmicTests::Give(*State, 1, { 6 });
		State->Civs[0].OwnedSystems.Add(State->Systems[6].SystemID);
		Sim->RunHeadless(Rules.FirstSightTick);
		TestTrue(TEXT("(the eye opened from the system its people owns)"), Knows(*State, 1));
		TestFalse(TEXT("A listed system another people owns is no place of the eye"), Knows(*State, 6));
		TestFalse(TEXT("and its lanes are not walked: 5"), Knows(*State, 5));
		TestFalse(TEXT("nor 7"), Knows(*State, 7));
	}

	// ---- The knobs below their range read as their bound (the schema keeps the range; the code does not divide by 0) ----
	{
		FCosmicRulesDef Low = Rules;
		Low.SightIntervalTicks = 0;
		Low.SightLanesByTier.Reset();
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Low));
		if (!TestNotNull(TEXT("The board of the low knobs is laid"), State))
		{
			return false;
		}
		MakeWanderer(*State, 4, EAscensionTier::Tier2_Stellar);
		Sim->StepOnce();
		TestTrue(TEXT("An interval of 0 looks every tick, and a missing reach reads as one lane"), IDs(State->Knowing.KnownSystems) == SystemIDs(*State, { 0, 4, 3, 5, 10 }));

		Low.SightIntervalTicks = -3;
		Low.SightLanesByTier = { 0, -1, 0, 0, 0 };
		State = LayBoard(*Sim, TablesWith(Original, Low));
		if (!TestNotNull(TEXT("The board of the negative knobs is laid"), State))
		{
			return false;
		}
		MakeWanderer(*State, 4, EAscensionTier::Tier2_Stellar);
		Sim->StepOnce();
		TestTrue(TEXT("A negative interval looks every tick, and a reach below 1 reads as one lane"), IDs(State->Knowing.KnownSystems) == SystemIDs(*State, { 0, 4, 3, 5, 10 }));
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
		TestTrue(TEXT("A journey's end off the period: the stage looks from where the vessel arrived"), Knows(*State, 9));
		TestTrue(TEXT("and sees its lanes"), Knows(*State, 8));
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
		// People 1 holds 2, 3 and 1, people 2 holds 8 and 7, so that none of them is the last of its people.
		CosmicTests::Give(*State, 1, { 3, 1 });
		CosmicTests::Give(*State, 2, { 7 });
		const FName One = State->Civs[1].CivID;
		const FName Two = State->Civs[2].CivID;
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
			// As a slot fires it: about the owner of the subject system, people 1, which the calling has not met.
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			Ctx.CivIndex = 1;
			Ctx.SystemIndex = 2;
			Ctx.GalaxyRow = State->Systems[2].GalaxyRow;
			TestTrue(TEXT("The gift to the vessel's people fires"), Events->FireRow(ToRuled, Ctx));
		}
		TestEqual(TEXT("The system is the vessel's people's"), State->Systems[2].OwnerCivID, State->Civs[0].CivID);
		TestTrue(TEXT("and known"), Knows(*State, 2));
		TestTrue(TEXT("The people it came from is met: the other party of the event"), HasMet(*State, One));
		const int32 Told = FindLine(*State, ToRuled, State->Tick);
		if (TestTrue(TEXT("The line about the received system is stored: it was known, and its giver met, before the line"), Told != INDEX_NONE))
		{
			TestFalse(TEXT("and told"), State->Log[Told].bUntold);
			TestEqual(TEXT("about the people that gave it"), State->Log[Told].FocusCivID, One);
		}
		const int32 Meeting = FindLine(*State, CosmicRows::LogFirstContact, State->Tick);
		if (TestTrue(TEXT("The meeting with the giver is told"), Meeting != INDEX_NONE))
		{
			TestEqual(TEXT("at the system received"), State->Log[Meeting].FocusSystemID, State->Systems[2].SystemID);
			if (Told != INDEX_NONE)
			{
				TestTrue(TEXT("before the row's line"), Meeting < Told);
			}
		}
		{
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			Ctx.CivIndex = 2;
			Ctx.SystemIndex = 7;
			Ctx.GalaxyRow = State->Systems[7].GalaxyRow;
			TestTrue(TEXT("The gift to an AI people fires"), Events->FireRow(ToAI, Ctx));
		}
		TestEqual(TEXT("That system is the AI people's"), State->Systems[7].OwnerCivID, State->Civs[3].CivID);
		TestFalse(TEXT("and stays unknown"), Knows(*State, 7));
		TestFalse(TEXT("and its giver unmet"), HasMet(*State, Two));
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
		// People 1 also holds 5, beside where the vessel stands.
		CosmicTests::Give(*State, 1, { 5 });
		PlayerRole->DeposeRuler();
		TestTrue(TEXT("The vessel was deposed"), State->Player.Role == EPlayerRole::Wanderer);
		TestTrue(TEXT("Its new eye knows where it stands and the lanes around it in its galaxy row, in Lanes order"),
			IDs(State->Knowing.KnownSystems) == SystemIDs(*State, { 0, 4, 3, 5 }));
		TestFalse(TEXT("It meets nobody by it, not the owner of a neighbour: that is the next look's"), HasMet(*State, State->Civs[1].CivID));
		TestEqual(TEXT("and tells no meeting"), FindLine(*State, CosmicRows::LogFirstContact, State->Tick), static_cast<int32>(INDEX_NONE));
	}

	// ---- A life that starts as a wanderer sees around it; Install never writes the first contact ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the wanderer's birth is laid"), State))
		{
			return false;
		}
		// People 3 also holds 6, beside the birth system.
		CosmicTests::Give(*State, 3, { 6 });
		const int32 SimBefore = Sim->GetRng().GetCurrentSeed();
		PlayerRole->Install(State->Systems[7].SystemID, State->Civs[2].CivID, EPlayerRole::Wanderer);
		TestEqual(TEXT("Install draws one number, the name, for a wanderer too: its new eye draws nothing"), DrawsBetween(SimBefore, Sim->GetRng().GetCurrentSeed()), 1);
		TestTrue(TEXT("A wanderer born in 7 knows 7 and its lanes"), IDs(State->Knowing.KnownSystems) == SystemIDs(*State, { 0, 7, 6, 8 }));
		TestTrue(TEXT("Install writes no first contact"), State->Knowing.FirstContactCivID.IsNone());
		TestFalse(TEXT("and meets no owner of a neighbour"), HasMet(*State, State->Civs[3].CivID));
	}

	// ---- FollowUpKnowing takes out the systems received and leaves the other notes, in their order ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the notes is laid"), State))
		{
			return false;
		}
		CosmicTests::Give(*State, 1, { 3, 1 });
		const FName Sickness(TEXT("crisis_test_sickness"));
		const FName Spread(TEXT("event_test_spread"));
		UDataTable* HazardRows = NewTable(FHazardDef::StaticStruct());
		FHazardDef Sick = HazardRow(TEXT("Hazard.Plague"), 50, TEXT("sickness"));
		Sick.OutbreakRow = Spread;
		HazardRows->AddRow(Sickness, Sick);
		UDataTable* EventRows = NewTable(FCosmicEventDef::StaticStruct());
		FCosmicEventDef Outbreak;
		Outbreak.Category = Tag(TEXT("Event.Civ.Society"));
		Outbreak.LogCategory = Tag(TEXT("Log.Civ"));
		Outbreak.Trigger = ECosmicEventTrigger::Named;
		Outbreak.Subject = ECosmicEventSubject::SettledSystem;
		Outbreak.LogTemplates = { FText::AsCultureInvariant(TEXT("spread {System}")) };
		EventRows->AddRow(Spread, Outbreak);
		Sim->SetTables(TablesWith(Original, Rules, HazardRows, EventRows));
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = 1;
		Ctx.SystemIndex = 3;
		Ctx.GalaxyRow = State->Systems[3].GalaxyRow;
		// 3 goes to the vessel's people and a plague breaks out there; then one breaks out on people 1's 1.
		TestTrue(TEXT("(3 is handed to the vessel's people)"), UCosmicEventSubsystem::Apply(Eff(EEffectType::TransferSystem, nullptr, State->Civs[0].CivID), Ctx));
		TestTrue(TEXT("(a plague is placed on 3)"), UCosmicEventSubsystem::Apply(Eff(EEffectType::SpawnHazard, TEXT("Hazard.Plague")), Ctx));
		Ctx.SystemIndex = 1;
		TestTrue(TEXT("(a plague is placed on 1)"), UCosmicEventSubsystem::Apply(Eff(EEffectType::SpawnHazard, TEXT("Hazard.Plague")), Ctx));
		if (!TestEqual(TEXT("(three notes)"), Ctx.Notes.Num(), 3))
		{
			return false;
		}
		Events->FollowUpKnowing(Ctx);
		TestTrue(TEXT("The system received is known"), Knows(*State, 3));
		if (TestEqual(TEXT("The two hazard notes stay"), Ctx.Notes.Num(), 2))
		{
			TestEqual(TEXT("in their order: 3 first"), Ctx.Notes[0].SystemIndex, 3);
			TestEqual(TEXT("then 1"), Ctx.Notes[1].SystemIndex, 1);
		}
		Events->FollowUpNotes(Ctx);
		const int32 First = FindLine(*State, Sickness, State->Tick);
		if (TestTrue(TEXT("The outbreaks are told"), First != INDEX_NONE))
		{
			TestEqual(TEXT("the one on 3 first"), State->Log[First].FocusSystemID, State->Systems[3].SystemID);
		}
		TestEqual(TEXT("and each fired its outbreak row"), State->Log.FilterByPredicate([Spread](const FCosmicLogEntry& Entry) { return Entry.SourceRow == Spread; }).Num(), 2);
		Sim->SetTables(Tables);
	}

	// ---- An expiry that hands its system to the vessel's people: known, and its giver met, before the expiry line ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the expiry is laid"), State))
		{
			return false;
		}
		// People 1 holds 2 and 3; a plague on 3 hands 3 to the vessel's people when it runs out.
		CosmicTests::Give(*State, 1, { 3 });
		const FName Gift(TEXT("crisis_test_gift"));
		UDataTable* HazardRows = NewTable(FHazardDef::StaticStruct());
		FHazardDef Row = HazardRow(TEXT("Hazard.Plague"), 1, TEXT("gift"));
		Row.OnExpireEffects = { Eff(EEffectType::TransferSystem, nullptr, State->Civs[0].CivID) };
		HazardRows->AddRow(Gift, Row);
		Sim->SetTables(TablesWith(Original, Rules, HazardRows));
		State->Systems[3].ActiveHazard = Tag(TEXT("Hazard.Plague"));
		State->Systems[3].HazardTicksRemaining = 1;
		Log->SetTellAll(false);
		Sim->StepOnce();
		Log->SetTellAll(true);
		TestEqual(TEXT("The expiry handed its system to the vessel's people"), State->Systems[3].OwnerCivID, State->Civs[0].CivID);
		TestTrue(TEXT("which knows it"), Knows(*State, 3));
		TestTrue(TEXT("and has met the people it came from"), HasMet(*State, State->Civs[1].CivID));
		// A Severity 1 line out of scope is not stored at all: stored is told.
		TestTrue(TEXT("The expiry line is told: the system was known, and its giver met, before it"), FindLine(*State, Gift, State->Tick) != INDEX_NONE);
		Sim->SetTables(Tables);
	}

	// ---- The last-system gate after such an expiry: the grace line is about a system known before it ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the gate is laid"), State))
		{
			return false;
		}
		const FName NovaGift(TEXT("crisis_test_nova_gift"));
		UDataTable* HazardRows = NewTable(FHazardDef::StaticStruct());
		FHazardDef Row = HazardRow(TEXT("Hazard.PreSupernova"), 1, TEXT("nova gift"));
		Row.OnExpireEffects = { Eff(EEffectType::TransferSystem, nullptr, State->Civs[0].CivID) };
		HazardRows->AddRow(NovaGift, Row);
		UDataTable* EventRows = NewTable(FCosmicEventDef::StaticStruct());
		if (!TestTrue(TEXT("The game has the remnant row"), CopyEvent(Original, EventRows, CosmicRows::EventGraceRemnant)))
		{
			return false;
		}
		Sim->SetTables(TablesWith(Original, Rules, HazardRows, EventRows));
		// The vessel's people holds nothing this tick (as between a collapse and its deposition); people 1 holds 2 and 3.
		CosmicTests::ClearOwners(*State);
		CosmicTests::Give(*State, 1, { 2, 3 });
		CosmicTests::Give(*State, 2, { 8 });
		CosmicTests::Give(*State, 3, { 11 });
		State->Systems[3].ActiveHazard = Tag(TEXT("Hazard.PreSupernova"));
		State->Systems[3].HazardTicksRemaining = 1;
		AddExpectedError(TEXT("the last-system gate is asked again"), EAutomationExpectedErrorFlags::Contains, 1);
		Log->SetTellAll(false);
		Sim->StepOnce();
		Log->SetTellAll(true);
		TestFalse(TEXT("The grace kept the system the expiry handed over"), CosmicHazards::IsDeadNode(State->Systems[3]));
		TestEqual(TEXT("to the vessel's people"), State->Systems[3].OwnerCivID, State->Civs[0].CivID);
		const int32 Grace = FindLine(*State, CosmicRows::EventGraceRemnant, State->Tick);
		if (TestTrue(TEXT("The grace line is stored"), Grace != INDEX_NONE))
		{
			TestFalse(TEXT("and told: the system was known before the gate"), State->Log[Grace].bUntold);
		}
		Sim->SetTables(Tables);
	}

	// ---- A completion whose resolution hands the system to the vessel's people: known before the completion line ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the completion is laid"), State))
		{
			return false;
		}
		// People 1 holds 2 and 3, and is met: only the place is in question. It answers a plague on 3.
		CosmicTests::Give(*State, 1, { 3 });
		Mark(State->Knowing.MetCivs, State->Civs[1].CivID);
		const FName Seceding(TEXT("crisis_test_seceding"));
		const FName Answer(TEXT("project_test_answer"));
		UDataTable* HazardRows = NewTable(FHazardDef::StaticStruct());
		FHazardDef Row = HazardRow(TEXT("Hazard.Plague"), 50, TEXT("seceding"));
		Row.OnResolveEffects = { Eff(EEffectType::TransferSystem, nullptr, State->Civs[0].CivID) };
		Row.RulerResolutions = { Answer };
		HazardRows->AddRow(Seceding, Row);
		FCosmicDataTables WithAnswer = TablesWith(Original, Rules, HazardRows);
		FMegaprojectDef Project;
		Project.DisplayName = FText::AsCultureInvariant(TEXT("<answer>"));
		Project.RequiredTier = EAscensionTier::Tier1_Planetary;
		Project.RequiredProgress = 1.f;
		Project.CompleteTemplates = { FText::AsCultureInvariant(TEXT("answer done {System}")) };
		WithAnswer.Megaprojects->AddRow(Answer, Project);
		Sim->SetTables(WithAnswer);
		State->Systems[3].ActiveHazard = Tag(TEXT("Hazard.Plague"));
		State->Systems[3].HazardTicksRemaining = 50;
		State->Systems[3].ResponseMegaproject = Answer;
		Log->SetTellAll(false);
		Sim->StepOnce();
		Log->SetTellAll(true);
		TestEqual(TEXT("The answer's resolution handed the system to the vessel's people"), State->Systems[3].OwnerCivID, State->Civs[0].CivID);
		// A Severity 0 line out of scope is not stored at all: stored is told.
		TestTrue(TEXT("The completion line is told: the system was known before it"), FindLine(*State, Answer, State->Tick) != INDEX_NONE);
		Sim->SetTables(Tables);
	}

	// ---- FollowUpKnowing refuses a handler and a detached state, and leaves the notes ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the refusals is laid"), State))
		{
			return false;
		}
		AddExpectedError(TEXT("FollowUpKnowing called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		AddExpectedError(TEXT("FollowUpKnowing: the context is not over the live state"), EAutomationExpectedErrorFlags::Contains, 1);
		// A note as Transfer leaves it: 9 received by the vessel's people from people 1.
		const FCosmicWorldNote Received = { ECosmicWorldNote::SystemReceived, 9, 0, NAME_None, 1 };
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.Notes.Add(Received);
		bool bTried = false;
		{
			FScopedLogProbe Probe(*Log);
			Probe.Probe->OnLog = [&](const FCosmicLogEntry&)
			{
				if (!bTried)
				{
					bTried = true;
					Events->FollowUpKnowing(Ctx);
				}
			};
			FFormatNamedArguments Args;
			Args.Add(TEXT("HomeSystem"), State->Systems[0].DisplayName);
			Log->PushRow(CosmicRows::LogFirstLife, 2, Args, State->Systems[0].SystemID);
		}
		TestTrue(TEXT("A handler ran"), bTried);
		TestFalse(TEXT("FollowUpKnowing refuses a handler: the system stays unknown"), Knows(*State, 9));
		TestFalse(TEXT("and the giver unmet"), HasMet(*State, State->Civs[1].CivID));
		TestEqual(TEXT("and the note stays"), Ctx.Notes.Num(), 1);

		FGalaxyState Detached = *State;
		FRandomStream Stream(7);
		FCosmicEvalContext Away(Detached, Stream, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
		Away.Notes.Add(Received);
		Events->FollowUpKnowing(Away);
		TestFalse(TEXT("FollowUpKnowing refuses a detached state: the live knowing is untouched"), Knows(*State, 9));
		TestFalse(TEXT("and so is the copy's"), Knows(Detached, 9));
		TestEqual(TEXT("and its note stays"), Away.Notes.Num(), 1);
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
	UCosmicEventSubsystem* Events = Sim->GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	if (!TestNotNull(TEXT("The log subsystem exists"), Log) || !TestNotNull(TEXT("The event subsystem exists"), Events))
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
		// In the interregnum the calling sees from its own tier, though the role still reads Ruler.
		State.Calling.LifeState = ELifeState::Interregnum;
		TestEqual(TEXT("In the interregnum the calling sees from its own tier, not from the people it ruled"), Tell(CivTag, 1, NAME_None, NAME_None, 3, 0).Added, 1);
		State.Calling.LifeState = ELifeState::Embodied;
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
		State.Knowing.FarLineUntilTick = 0;
		const FTold Claim = Tell(Tag(TEXT("Log.Claim")), 0, One, NAME_None);
		TestEqual(TEXT("A claim line out of scope is kept"), Claim.Added, 1);
		TestTrue(TEXT("untold"), Claim.bLastUntold);
		TestEqual(TEXT("with no far line (Severity below 2)"), Claim.Draws, 1);
		const FTold Record = Tell(Tag(TEXT("Log.Record")), 1, One, NAME_None);
		TestEqual(TEXT("A record line out of scope is kept"), Record.Added, 1);
		TestTrue(TEXT("untold"), Record.bLastUntold);
		TestEqual(TEXT("with no far line either"), Record.Draws, 1);

		// Out of scope by its subject and by its window: the subject calls for the far line.
		const int32 BeforeBoth = State.Log.Num();
		TestEqual(TEXT("A Severity 2 line out of scope by its subject and its window is followed by a far line"), Tell(CivTag, 2, One, NAME_None, 3, 0).Added, 2);
		TestTrue(TEXT("(that line)"), State.Log.IsValidIndex(BeforeBoth + 1) && State.Log[BeforeBoth + 1].SourceRow == CosmicRows::LogFarEvent);
		// A met people in a system the calling does not know: the far line names neither.
		State.Knowing.FarLineUntilTick = 0;
		const int32 BeforeUnknown = State.Log.Num();
		TestEqual(TEXT("A Severity 2 line about a met people in an unknown system is followed by a far line"), Tell(CivTag, 2, Two, Eight).Added, 2);
		if (TestTrue(TEXT("(that far line)"), State.Log.IsValidIndex(BeforeUnknown + 1)))
		{
			TestTrue(TEXT("which has no subject system"), State.Log[BeforeUnknown + 1].FocusSystemID.IsNone());
			TestTrue(TEXT("and no subject people"), State.Log[BeforeUnknown + 1].FocusCivID.IsNone());
		}
		// The calling's own line holds its subject whatever it names; below its window it is kept untold, with no far line.
		State.Knowing.FarLineUntilTick = 0;
		const FTold CallingBelow = Tell(Tag(TEXT("Log.Calling")), 2, One, Eight, 3, 0);
		TestEqual(TEXT("The calling's own Severity 2 line below its window is kept"), CallingBelow.Added, 1);
		TestTrue(TEXT("untold"), CallingBelow.bLastUntold);
		TestEqual(TEXT("with no far line: its subject holds"), CallingBelow.Draws, 1);
		// Push, the one-template path, is judged as PushVariants is.
		{
			const FName PushRow(TEXT("log_test_push"));
			const int32 BeforePush = State.Log.Num();
			const int32 TellBefore = State.TellRngState;
			Log->Push(CivTag, 1, FText::AsCultureInvariant(TEXT("push about the unmet")), FFormatNamedArguments(), NAME_None, PushRow, One);
			TestEqual(TEXT("Push judges the scope: a line about an unmet people is not stored"), State.Log.Num(), BeforePush);
			TestEqual(TEXT("and Push draws nothing"), State.TellRngState, TellBefore);
			State.Knowing.FarLineUntilTick = 0;
			Log->Push(CivTag, 2, FText::AsCultureInvariant(TEXT("mythic push about the unmet")), FFormatNamedArguments(), NAME_None, PushRow, One);
			TestEqual(TEXT("A mythic Push out of scope is kept, with its far line"), State.Log.Num(), BeforePush + 2);
			TestTrue(TEXT("(kept untold)"), State.Log.IsValidIndex(BeforePush) && State.Log[BeforePush].bUntold);
		}

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
		// A mythic line about a people on the third rung that is no ascent follows the far event line and its cooldown.
		State.Knowing.FarLineUntilTick = 0;
		const int32 BeforeCrisis = State.Log.Num();
		TestEqual(TEXT("A mythic line that is no ascent, about a people on the third rung, is followed by a far line"), Tell(Tag(TEXT("Log.Crisis")), 2, One, NAME_None).Added, 2);
		TestTrue(TEXT("the far event line, not the far ascent line"), State.Log.IsValidIndex(BeforeCrisis + 1) && State.Log[BeforeCrisis + 1].SourceRow == CosmicRows::LogFarEvent);
		TestEqual(TEXT("which sets its next tick"), State.Knowing.FarLineUntilTick, State.Tick + Rules.FarLineCooldownTicks);
		// A met people on the third rung whose capital is unknown: the far ascent line names neither.
		State.Civs[2].Tier = EAscensionTier::Tier3_Causal;
		const int32 BeforeMetAscent = State.Log.Num();
		TestEqual(TEXT("A met people's ascent in an unknown system is followed by the far ascent line"), Tell(AscensionTag, 2, Two, Eight).Added, 2);
		if (TestTrue(TEXT("(that far line)"), State.Log.IsValidIndex(BeforeMetAscent + 1)))
		{
			TestEqual(TEXT("the far ascent line"), State.Log[BeforeMetAscent + 1].SourceRow, CosmicRows::LogFarAscent);
			TestTrue(TEXT("with no subject system"), State.Log[BeforeMetAscent + 1].FocusSystemID.IsNone());
		}
		State.Civs[2].Tier = EAscensionTier::Tier1_Planetary;
		State.Knowing.FarLineUntilTick = 0;
		State.Civs[1].Tier = EAscensionTier::Tier2_Stellar;
		const int32 BeforeLower = State.Log.Num();
		const FTold Lower = Tell(AscensionTag, 2, One, NAME_None);
		TestEqual(TEXT("An ascent to the second rung is followed by a far event line"), Lower.Added, 2);
		TestTrue(TEXT("(that line)"), State.Log.IsValidIndex(BeforeLower + 1) && State.Log[BeforeLower + 1].SourceRow == CosmicRows::LogFarEvent);
		TestEqual(TEXT("A Severity 0 line out of scope is not stored"), Tell(CivTag, 0, One, NAME_None).Added, 0);
		TestEqual(TEXT("Telling, far lines and all, never draws from the sim stream"), Sim->GetRng().GetCurrentSeed(), SimSeed);
	}

	// ---- A far line cooldown below 0 reads as 0 ----
	{
		FCosmicRulesDef Negative = Rules;
		Negative.FarLineCooldownTicks = -5;
		Sim->SetTables(TablesWith(Original, Negative));
		State.Knowing.FarLineUntilTick = 0;
		TestEqual(TEXT("(a mythic line about the unmet people and its far line)"), Tell(CivTag, 2, One, NAME_None).Added, 2);
		TestEqual(TEXT("A cooldown below 0 reads as 0: the far event line may follow again from this tick"), State.Knowing.FarLineUntilTick, State.Tick);
		Sim->SetTables(TablesWith(Original, Rules));
	}

	// ---- An untold entry leaves the log for the chronicle with its mark (section 8.4) ----
	{
		const TArray<FCosmicLogEntry> Kept = State.Log;
		State.Log.Reset();
		State.Chronicle.Reset();
		State.Knowing.FarLineUntilTick = State.Tick + 1000;
		TestTrue(TEXT("(an untold entry opens the log)"), Tell(CivTag, 2, One, NAME_None).bLastUntold);
		for (int32 Index = 0; Index < UCosmicLogSubsystem::MaxLogEntries; ++Index)
		{
			Tell(CivTag, 0, NAME_None, NAME_None);
		}
		if (TestEqual(TEXT("The untold entry left the log for the chronicle"), State.Chronicle.Num(), 1))
		{
			TestTrue(TEXT("still untold"), State.Chronicle[0].bUntold);
		}
		State.Log = Kept;
		State.Chronicle.Reset();
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
		TestTrue(TEXT("(people 1 crossed the threshold too: the case can tell)"), State.Civs[1].Research.FindRef(Society) >= 0.5f);
		TestEqual(TEXT("An unmet people's breakthrough is not told"), Breakthrough(One), static_cast<int32>(INDEX_NONE));
	}

	// ---- A rebellion's start and end are judged on the same capital: an unknown one hides both, and its name ----
	{
		const FName Rebellion(TEXT("crisis_rebellion"));
		const FName Rises(TEXT("event_rebellion_rises"));
		UDataTable* HazardRows = NewTable(FHazardDef::StaticStruct());
		UDataTable* EventRows = NewTable(FCosmicEventDef::StaticStruct());
		if (!TestTrue(TEXT("The game has the rebellion"), CopyHazard(Original, HazardRows, Rebellion)) || !TestTrue(TEXT("The game has the rising"), CopyEvent(Original, EventRows, Rises)))
		{
			return false;
		}
		FGalaxyState* Again = LayBoard(*Sim, TablesWith(Original, Rules, HazardRows, EventRows));
		if (!TestNotNull(TEXT("The board of the rebellion is laid"), Again))
		{
			return false;
		}
		// People 1 is met; its capital, 2, is not known.
		Mark(Again->Knowing.MetCivs, Again->Civs[1].CivID);
		const FString Capital = Again->Systems[2].DisplayName.ToString();
		auto Rise = [&]()
		{
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			Ctx.CivIndex = 1;
			Ctx.SystemIndex = 2;
			Ctx.GalaxyRow = Again->Systems[2].GalaxyRow;
			const bool bPlaced = UCosmicEventSubsystem::Apply(Eff(EEffectType::SpawnHazard, TEXT("Hazard.Rebellion")), Ctx);
			Events->FollowUpNotes(Ctx);
			return bPlaced;
		};
		auto ToldSays = [Again](const FString& Text)
		{
			return Again->Log.ContainsByPredicate([&Text](const FCosmicLogEntry& Entry) { return !Entry.bUntold && Entry.Message.ToString().Contains(Text); });
		};
		TestTrue(TEXT("(the rebellion breaks out in people 1's capital)"), Rise());
		TestEqual(TEXT("A rising in an unknown capital is not told"), FindLine(*Again, Rises, Again->Tick), static_cast<int32>(INDEX_NONE));
		Again->Systems[2].HazardTicksRemaining = 1;
		Sim->StepOnce();
		TestFalse(TEXT("(the rebellion ran out)"), Again->Systems[2].ActiveHazard.IsValid());
		TestEqual(TEXT("and neither is its end"), FindLine(*Again, Rebellion, Again->Tick), static_cast<int32>(INDEX_NONE));
		TestFalse(TEXT("The name of the unknown capital never reaches the feed"), ToldSays(Capital));
		// The control: once the capital is known, both are told.
		TestTrue(TEXT("(the capital becomes known)"), Sim->NoteKnown(Again->Systems[2].SystemID));
		TestTrue(TEXT("(the rebellion breaks out again)"), Rise());
		TestTrue(TEXT("A rising in a known capital is told"), FindLine(*Again, Rises, Again->Tick) != INDEX_NONE);
		Again->Systems[2].HazardTicksRemaining = 1;
		Sim->StepOnce();
		TestTrue(TEXT("and so is its end"), FindLine(*Again, Rebellion, Again->Tick) != INDEX_NONE);
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
	// The game's data, 3,000 ticks of seed 42, no input: the first life rules people 0 and gives no command. With
	// bWanderer the vessel walks from the start instead, a wanderer of the second rung standing in its birth system.
	auto Run = [&](const FCosmicRulesDef& Rules, bool bTellAll, bool bWanderer)
	{
		Sim->SetTables(WithRules(Original, Rules));
		Log->SetTellAll(bTellAll);
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		if (bWanderer)
		{
			FGalaxyState& Mutable = Sim->GetMutableState();
			Mutable.Civs[0].bIsPlayerRuled = false;
			Mutable.Player.Role = EPlayerRole::Wanderer;
			Mutable.Player.RuledCivID = NAME_None;
			Mutable.Calling.PersonalTier = EAscensionTier::Tier2_Stellar;
		}
		Sim->RunHeadless(3000);
		FRun Result;
		Result.World = Sim->ComputeWorldHash();
		Result.Whole = Sim->ComputeStateHash();
		Result.Rng = Sim->GetRng().GetCurrentSeed();
		Result.Known = Sim->GetState().Knowing.KnownSystems.Num();
		Result.Met = Sim->GetState().Knowing.MetCivs.Num();
		return Result;
	};

	const FRun Seeing = Run(GameRules, true, false);
	FCosmicRulesDef Blind = GameRules;
	Blind.SightIntervalTicks = 1000000;
	Blind.FirstSightTick = 1000000;
	const FRun NotSeeing = Run(Blind, true, false);
	FCosmicRulesDef Far = GameRules;
	Far.SightLanesByTier = { 10, 10, 10, 10, 10 };
	const FRun FarSeeing = Run(Far, true, false);
	const FRun Scoped = Run(GameRules, false, false);
	const FRun Walking = Run(GameRules, true, true);
	const FRun WalkingBlind = Run(Blind, true, true);

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
	TestEqual(TEXT("The world is the same whether a wanderer's eye looks or not"), WalkingBlind.World, Walking.World);
	TestEqual(TEXT("and so is the sim stream (wanderer)"), WalkingBlind.Rng, Walking.Rng);
	TestTrue(TEXT("A wanderer's eye knows more than none (the case can tell)"), Walking.Known > WalkingBlind.Known);
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

	// The save keeps it as it was - the next tick of the far line too, set here to a value the save writes (no far line is
	// told in the tell-all mode, and a field at its initializer is not written).
	Sim->GetMutableState().Knowing.FarLineUntilTick = State.Tick + 777;
	const FKnowingState AtSave = State.Knowing;
	const int64 SavedAt = State.Tick;
	const FGuid GalaxyID = State.GalaxyID;
	if (TestTrue(TEXT("Save succeeds"), Save->SaveCurrentGalaxy()))
	{
		Sim->RunHeadless(500);
		if (TestTrue(TEXT("Load succeeds"), Save->LoadGalaxy(GalaxyID)))
		{
			TestEqual(TEXT("The load replaced the state: the tick of the save"), Sim->GetState().Tick, SavedAt);
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
	UCosmicLogSubsystem* Log = Sim ? Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The log subsystem exists"), Log))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Log->SetTellAll(true); Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	Sim->SetTables(Original);
	// The game's way of telling: the scope judges the line of the first contact.
	Log->SetTellAll(false);
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
		auto IsContactLine = [First, Tick](const FCosmicLogEntry& Entry) { return Entry.SourceRow == CosmicRows::LogFirstContact && Entry.Tick == Tick && Entry.FocusCivID == First && !Entry.bUntold; };
		if (!State.Log.ContainsByPredicate(IsContactLine) && !State.Chronicle.ContainsByPredicate(IsContactLine))
		{
			Untold += FString::Printf(TEXT(" %d"), Seed);
		}
	}
	TestEqual(TEXT("Every seed has made its first contact by FirstSightTick + SightIntervalTicks (seeds that had not)"), Late, FString());
	TestEqual(TEXT("and none before FirstSightTick - Phase 2 and 3 (seeds that had)"), Early, FString());
	TestEqual(TEXT("The first contact is told by log_first_contact at its tick, the scope judging it (seeds where it was not)"), Untold, FString());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

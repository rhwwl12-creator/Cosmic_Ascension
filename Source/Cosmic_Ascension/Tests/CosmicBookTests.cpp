// Cosmic Ascension - automation tests of the fallen-sphere counter and the book rows. Design: guideline.md section 5.11,
// 7.4, 9.3, 16 #25.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicHazards.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/NameTypes.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#include <initializer_list>

namespace CosmicBookTests
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

	const FName Fallen(TEXT("tr_test_fallen"));
	const FName Apart(TEXT("tr_test_apart"));
	const FName Rowless(TEXT("tr_test_rowless"));
	const FName TestClosed(TEXT("event_test_closed"));
	const FName TestReading(TEXT("event_test_reading"));
	const FName GameClosed(TEXT("event_book_closed"));
	const FName GameReading(TEXT("event_book_reading"));

	// A fallen row of Silence and an independent row, both of tier 4: the transcendents of the test.
	UDataTable* TranscendentRows()
	{
		UDataTable* Table = NewTable(FTranscendentDef::StaticStruct());
		FTranscendentDef Silence;
		Silence.Doctrine = Tag(TEXT("Doctrine.Silence"));
		Silence.Allegiance = Tag(TEXT("Realm.Abyssal"));
		Silence.Tier = EAscensionTier::Tier4_Transcendent;
		Silence.Depth = 0.f;
		Table->AddRow(Fallen, Silence);
		FTranscendentDef Stillness = Silence;
		Stillness.Doctrine = Tag(TEXT("Doctrine.Stillness"));
		Stillness.Allegiance = Tag(TEXT("Realm.Independent"));
		Table->AddRow(Apart, Stillness);
		return Table;
	}

	// The test's tables: its transcendent rows, these event rows, no hazard row (a hazard of the game would land on the
	// board at random), no project and feat of the game (CosmicTests::WithoutProjects) and its own knobs (a slot every ten
	// ticks, no Lottery chance, no Stability drain, no recovery and no inherited edict - a case reads a row's change of
	// Stability exactly), over the rest of the game's tables.
	FCosmicDataTables TablesWith(const FCosmicDataTables& Base, const FCosmicRulesDef& BaseRules, UDataTable* Events)
	{
		FCosmicRulesDef Knobs = CosmicTests::WithoutStartingEdict(BaseRules);
		Knobs.EventSlotTicks = 10;
		Knobs.EventChancePerSlot = 0.f;
		Knobs.SpiritualPressureStabilityDrain = 0.f;
		Knobs.StabilityRecoveryPerTick = 0.f;
		UDataTable* RulesTable = NewTable(FCosmicRulesDef::StaticStruct());
		RulesTable->AddRow(CosmicRows::Rules, Knobs);
		FCosmicDataTables Tables = Base;
		Tables.Rules = RulesTable;
		Tables.Transcendents = TranscendentRows();
		Tables.Events = Events;
		Tables.Hazards = NewTable(FHazardDef::StaticStruct());
		return CosmicTests::WithoutProjects(Tables);
	}

	// The game's tables without its hazard rows, projects and feats: the cases that read the game's event rows run hundreds
	// of ticks, and a hazard of the game landing on the board at random, or the AI paying for its projects, would change
	// what they read.
	FCosmicDataTables GameRowsWithoutHazards(const FCosmicDataTables& Original)
	{
		FCosmicDataTables Tables = Original;
		Tables.Hazards = NewTable(FHazardDef::StaticStruct());
		return CosmicTests::WithoutProjects(Tables);
	}

	// No research progress at all, so that no breakthrough of its own falls inside a case (a Society mark would fire the
	// found row).
	void NoResearch(FCivilizationState& Civ)
	{
		for (TPair<FName, float>& Focus : Civ.ResearchFocus)
		{
			Focus.Value = 0.f;
		}
	}

	// A board the test knows: the chain of CosmicTests::WireChain, civilization 0 holds systems 0 and 1, the others one
	// system each (6, 9, 11), no transcendent, civilization 0 has the book and no other people holds a book tag, and
	// nobody researches. False below twelve systems or four peoples.
	bool Board(FGalaxyState& State)
	{
		if (!CosmicTests::WireChain(State) || State.Civs.Num() < 4)
		{
			return false;
		}
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { 0, 1 });
		CosmicTests::Give(State, 1, { 6 });
		CosmicTests::Give(State, 2, { 9 });
		CosmicTests::Give(State, 3, { 11 });
		State.Transcendents.Reset();
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.Traits.RemoveTag(Tag(TEXT("Trait.Faith.Book")));
			Civ.Traits.RemoveTag(Tag(TEXT("Trait.Faith.BookShut")));
			NoResearch(Civ);
		}
		State.Civs[0].Traits.AddTag(Tag(TEXT("Trait.Faith.Book")));
		return true;
	}

	void AddInstance(FGalaxyState& State, FName TranscendentID, const TCHAR* Doctrine, std::initializer_list<int32> Sphere)
	{
		FTranscendentState& Transcendent = State.Transcendents.AddDefaulted_GetRef();
		Transcendent.TranscendentID = TranscendentID;
		Transcendent.Doctrine = Tag(Doctrine);
		for (const int32 SystemIndex : Sphere)
		{
			Transcendent.InfluencedSystems.Add(State.Systems[SystemIndex].SystemID);
		}
	}

	FCosmicCondition Cond(EConditionType Type, const TCHAR* TagName, const TCHAR* Key)
	{
		FCosmicCondition Condition;
		Condition.Type = Type;
		Condition.Tag = TagName ? Tag(TagName) : FGameplayTag();
		Condition.Key = Key ? FName(Key) : FName();
		return Condition;
	}

	FCosmicEffect Eff(EEffectType Type, const TCHAR* TagName, float Value = 0.f)
	{
		FCosmicEffect Effect;
		Effect.Type = Type;
		Effect.Tag = TagName ? Tag(TagName) : FGameplayTag();
		Effect.Value = Value;
		return Effect;
	}

	TArray<FText> Lines(const TCHAR* Stem)
	{
		return {
			FText::AsCultureInvariant(FString::Printf(TEXT("%s one"), Stem)),
			FText::AsCultureInvariant(FString::Printf(TEXT("%s two"), Stem)),
			FText::AsCultureInvariant(FString::Printf(TEXT("%s three"), Stem))
		};
	}

	// The book rows of the test: closed after 300 ticks in a sphere of Doctrine (a Standing row), found by name, and a
	// reading of every slot while the book is open (a Standing row without cooldown) whose name sorts after the closing.
	UDataTable* BookRows(const TCHAR* Doctrine)
	{
		UDataTable* Events = NewTable(FCosmicEventDef::StaticStruct());
		FCosmicEventDef Closed;
		Closed.Category = Tag(TEXT("Event.Civ.Society"));
		Closed.LogCategory = Tag(TEXT("Log.Civ"));
		Closed.Trigger = ECosmicEventTrigger::Standing;
		Closed.Subject = ECosmicEventSubject::Civilization;
		Closed.BaseWeight = 0.f;
		Closed.CooldownTicks = 0;
		Closed.Conditions = { Cond(EConditionType::CivHasTrait, TEXT("Trait.Faith.Book"), nullptr), Cond(EConditionType::CivFallenSphereTicksReached, Doctrine, TEXT("300")) };
		Closed.Effects = { Eff(EEffectType::RemoveTrait, TEXT("Trait.Faith.Book")), Eff(EEffectType::AddTrait, TEXT("Trait.Faith.BookShut")) };
		Closed.LogTemplates = Lines(TEXT("closed"));
		Events->AddRow(TestClosed, Closed);
		FCosmicEventDef Reading = Closed;
		Reading.Conditions = { Cond(EConditionType::CivHasTrait, TEXT("Trait.Faith.Book"), nullptr) };
		Reading.Effects.Reset();
		Reading.LogTemplates = Lines(TEXT("reading"));
		Events->AddRow(TestReading, Reading);
		FCosmicEventDef Found = Closed;
		Found.Trigger = ECosmicEventTrigger::Named;
		Found.Conditions = { Cond(EConditionType::CivHasTrait, TEXT("Trait.Faith.BookShut"), nullptr) };
		Found.Effects = { Eff(EEffectType::RemoveTrait, TEXT("Trait.Faith.BookShut")), Eff(EEffectType::AddTrait, TEXT("Trait.Faith.Book")), Eff(EEffectType::ChangeStability, nullptr, 0.2f) };
		Found.LogTemplates = { FText::AsCultureInvariant(TEXT("found")) };
		Events->AddRow(CosmicRows::EventBookFound, Found);
		return Events;
	}

	bool HasBook(const FCivilizationState& Civ)
	{
		return Civ.Traits.HasTagExact(Tag(TEXT("Trait.Faith.Book")));
	}

	bool HasShutBook(const FCivilizationState& Civ)
	{
		return Civ.Traits.HasTagExact(Tag(TEXT("Trait.Faith.BookShut")));
	}

	// Research that crosses the first mark of one field in the next tick: everything spent goes to that field.
	void ToTheMark(FCivilizationState& Civ, const TCHAR* Field)
	{
		NoResearch(Civ);
		Civ.ResearchFocus.Add(FName(Field), 1.f);
		Civ.Research.Add(FName(Field), 0.2499f);
		Civ.Population = 1.f;
		Civ.Stockpile.Insight = 100.f;
	}

	// Log lines of a row about a people from a tick on.
	int32 LinesFrom(const FGalaxyState& State, FName Row, FName CivID, int64 FromTick)
	{
		return State.Log.FilterByPredicate([Row, CivID, FromTick](const FCosmicLogEntry& Entry)
		{
			return Entry.SourceRow == Row && Entry.FocusCivID == CivID && Entry.Tick >= FromTick;
		}).Num();
	}

	// Log lines of a row about a people, at one tick or at any tick (Tick < 0).
	int32 LinesOf(const FGalaxyState& State, FName Row, FName CivID, int64 Tick = -1)
	{
		return State.Log.FilterByPredicate([Row, CivID, Tick](const FCosmicLogEntry& Entry)
		{
			return Entry.SourceRow == Row && Entry.FocusCivID == CivID && (Tick < 0 || Entry.Tick == Tick);
		}).Num();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicFallenSphereTicksTest, "Cosmic_Ascension.Sim.FallenSphereTicks", CosmicTests::Flags)
bool FCosmicFallenSphereTicksTest::RunTest(const FString& Parameters)
{
	using namespace CosmicBookTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	// No event row: nothing but the economy stage touches what the test reads.
	Sim->SetTables(TablesWith(Original, BaseRules, NewTable(FCosmicEventDef::StaticStruct())));
	Sim->NewGalaxy(42, 12, 4);
	FGalaxyState& State = Sim->GetMutableState();
	if (!TestTrue(TEXT("The board of the test is laid"), Board(State)))
	{
		return false;
	}
	AddInstance(State, Fallen, TEXT("Doctrine.Silence"), { 1 });
	// Read through the arrays after every run, never through a reference held across one.
	auto Count = [&State](int32 CivIndex)
	{
		return State.Civs.IsValidIndex(CivIndex) ? State.Civs[CivIndex].FallenSphereTicks : static_cast<int32>(INDEX_NONE);
	};
	auto Silence = [&State]() -> FTranscendentState&
	{
		return State.Transcendents[0];
	};
	auto Sphere = [&State, &Silence](std::initializer_list<int32> Systems)
	{
		Silence().InfluencedSystems.Reset();
		for (const int32 SystemIndex : Systems)
		{
			Silence().InfluencedSystems.Add(State.Systems[SystemIndex].SystemID);
		}
	};

	Sim->RunHeadless(5);
	if (!TestEqual(TEXT("The one transcendent of the test is still there"), State.Transcendents.Num(), 1))
	{
		return false;
	}
	TestEqual(TEXT("A settled system inside a fallen sphere counts one a tick"), Count(0), 5);
	TestEqual(TEXT("A people without a system inside counts nothing"), Count(1), 0);
	Sphere({ 0, 1 });
	Sim->RunHeadless(3);
	TestEqual(TEXT("Two systems inside still count one a tick"), Count(0), 8);
	Sphere({});
	Sim->RunHeadless(1);
	TestEqual(TEXT("Leaving the sphere starts the count over"), Count(0), 0);
	Sphere({ 1 });
	Sim->RunHeadless(2);
	TestEqual(TEXT("Coming back counts from one"), Count(0), 2);

	// The tier of the people does not matter: on the transcendent's own tier the sphere still counts.
	State.Civs[0].Tier = EAscensionTier::Tier4_Transcendent;
	Sim->RunHeadless(1);
	TestEqual(TEXT("A people on the transcendent's own tier still counts"), Count(0), 3);
	State.Civs[0].Tier = EAscensionTier::Tier1_Planetary;

	// A lane away is not inside.
	Sphere({ 2 });
	Sim->RunHeadless(1);
	TestEqual(TEXT("A system one lane from the sphere is not inside"), Count(0), 0);
	Sphere({ 1 });
	Sim->RunHeadless(1);
	TestEqual(TEXT("Inside again after the lane case"), Count(0), 1);

	// A system being settled is owned, not settled.
	State.Systems[1].ActiveMegaproject = CosmicRows::ProjectSettle;
	Sim->RunHeadless(1);
	TestEqual(TEXT("A system being settled does not count"), Count(0), 0);
	State.Systems[1].ActiveMegaproject = NAME_None;
	Sim->RunHeadless(1);
	TestEqual(TEXT("Settled again, it counts"), Count(0), 1);

	// A sealed transcendent, an instance without a row and an independent sphere press no fallen sphere on anyone.
	Silence().bSealed = true;
	Sim->RunHeadless(1);
	TestEqual(TEXT("The sphere of a sealed transcendent does not count"), Count(0), 0);
	Silence().bSealed = false;
	Sim->RunHeadless(1);
	TestEqual(TEXT("Unsealed again, it counts"), Count(0), 1);
	Silence().TranscendentID = Rowless;
	Sim->RunHeadless(1);
	TestEqual(TEXT("The sphere of an instance without a row does not count"), Count(0), 0);
	Silence().TranscendentID = Apart;
	Silence().Doctrine = Tag(TEXT("Doctrine.Stillness"));
	Sim->RunHeadless(1);
	TestEqual(TEXT("An independent sphere is not a fallen one"), Count(0), 0);
	Silence().TranscendentID = Fallen;
	Silence().Doctrine = Tag(TEXT("Doctrine.Silence"));
	Sim->RunHeadless(1);
	TestEqual(TEXT("The fallen row again, it counts"), Count(0), 1);

	// A system of another people counts for that people only.
	Sphere({ 6 });
	Sim->RunHeadless(1);
	TestEqual(TEXT("Another people's system inside does not count for this one"), Count(0), 0);
	TestEqual(TEXT("It counts for the people that holds it"), Count(1), 1);
	TestEqual(TEXT("A third people without a system inside still counts nothing"), Count(2), 0);

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

// The endured pressure (B6b header section 2.7, #156), counted with the fallen-sphere counter at the end of the economy stage
// for every people: a tick on which the drain took Stability while the capital lay in no unsealed fallen sphere adds one, any
// other tick starts it over - a capital inside a fallen sphere is subjected, not enduring (mutation: counting it anyway); a
// people that is not the ruled one counts the same (mutation: only the ruled people); an independent sphere over the capital is
// no subjection (mutation: any sphere), and the drain of an independent presser is a drain (mutation: only a drain beside a
// fallen sphere); no drain (no sphere, a sealed transcendent, the knob at 0) is no tick; CosmicHazards::MoveCapital starts it
// over (mutation: without that reset the count runs on through the loss of the capital). It takes no draw; from B-6b2a
// CivPressureEnduredTicksReached reads it (Sim.EraSelector).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicPressureEnduredTest, "Cosmic_Ascension.Sim.PressureEndured", CosmicTests::Flags)
bool FCosmicPressureEnduredTest::RunTest(const FString& Parameters)
{
	using namespace CosmicBookTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	// The tables of the sphere counter's test, with a drain: no event, no hazard, no recovery, so only the drain moves
	// Stability and nothing moves a capital but the test.
	auto WithDrain = [&](float Drain)
	{
		FCosmicDataTables Tables = TablesWith(Original, BaseRules, NewTable(FCosmicEventDef::StaticStruct()));
		FCosmicRulesDef Knobs = *Tables.Rules->FindRow<FCosmicRulesDef>(CosmicRows::Rules, TEXT("PressureEndured"));
		Knobs.SpiritualPressureStabilityDrain = Drain;
		UDataTable* RulesTable = NewTable(FCosmicRulesDef::StaticStruct());
		RulesTable->AddRow(CosmicRows::Rules, Knobs);
		Tables.Rules = RulesTable;
		return Tables;
	};
	Sim->SetTables(WithDrain(0.001f));
	Sim->NewGalaxy(42, 12, 4);
	FGalaxyState& State = Sim->GetMutableState();
	if (!TestTrue(TEXT("The board of the test is laid"), Board(State)))
	{
		return false;
	}
	// The capitals the cases read: system 0 for the people of systems 0 and 1, system 6 for the people of systems 6 and 7.
	CosmicTests::Give(State, 1, { 7 });
	State.Civs[0].HomeSystemID = State.Systems[0].SystemID;
	State.Civs[1].HomeSystemID = State.Systems[6].SystemID;
	AddInstance(State, Fallen, TEXT("Doctrine.Silence"), { 1 });
	auto Count = [&State](int32 CivIndex)
	{
		return State.Civs.IsValidIndex(CivIndex) ? State.Civs[CivIndex].PressureEnduredTicks : static_cast<int32>(INDEX_NONE);
	};
	auto Sphere = [&State](std::initializer_list<int32> Systems)
	{
		State.Transcendents[0].InfluencedSystems.Reset();
		for (const int32 SystemIndex : Systems)
		{
			State.Transcendents[0].InfluencedSystems.Add(State.Systems[SystemIndex].SystemID);
		}
	};

	const float Before = State.Civs[0].Stability;
	Sim->RunHeadless(4);
	if (!TestEqual(TEXT("The one transcendent of the test is still there"), State.Transcendents.Num(), 1))
	{
		return false;
	}
	TestTrue(TEXT("(the drain takes Stability from the people of the sphere)"), State.Civs[0].Stability < Before);
	TestEqual(TEXT("A drained people whose capital is outside the sphere counts one a tick"), Count(0), 4);
	TestEqual(TEXT("A people out of the drain counts nothing"), Count(1), 0);

	// The capital decides, for every people: inside is subjected.
	Sphere({ 0, 1 });
	Sim->RunHeadless(1);
	TestEqual(TEXT("With the capital inside the sphere the count starts over"), Count(0), 0);
	Sphere({ 1, 6 });
	Sim->RunHeadless(2);
	TestEqual(TEXT("Outside again, it counts from one"), Count(0), 2);
	TestEqual(TEXT("Another people drained with its capital inside counts nothing"), Count(1), 0);

	// No drain is no tick.
	Sphere({});
	Sim->RunHeadless(1);
	TestEqual(TEXT("Out of every sphere the count starts over"), Count(0), 0);
	// A people that is not the ruled one: drained through system 7 with its capital 6 outside the sphere, it counts.
	Sphere({ 7 });
	Sim->RunHeadless(2);
	TestEqual(TEXT("The second people endures with its capital outside the sphere"), Count(1), 2);
	TestEqual(TEXT("(the first people is out of the drain)"), Count(0), 0);
	Sphere({ 1 });
	State.Transcendents[0].bSealed = true;
	Sim->RunHeadless(1);
	TestEqual(TEXT("A sealed transcendent drains nothing - no tick"), Count(0), 0);
	State.Transcendents[0].bSealed = false;
	Sim->RunHeadless(3);
	TestEqual(TEXT("Unsealed, it counts again"), Count(0), 3);

	// Only a fallen sphere subjects: an independent sphere over the capital leaves the count running, and with the fallen
	// sphere empty the independent presser's drain alone still counts.
	AddInstance(State, Apart, TEXT("Doctrine.Stillness"), { 0 });
	Sim->RunHeadless(1);
	TestEqual(TEXT("A capital inside an independent sphere still endures"), Count(0), 4);
	Sphere({});
	Sim->RunHeadless(1);
	TestEqual(TEXT("The drain of an independent presser alone is a drain"), Count(0), 5);
	State.Transcendents.RemoveAt(1);
	Sphere({ 1 });

	// The loss of the capital starts it over, and the next drained tick counts from one.
	CosmicHazards::MoveCapital(State, Sim->GetStateIndex(), 0);
	TestEqual(TEXT("MoveCapital starts the count over"), Count(0), 0);
	Sim->RunHeadless(1);
	TestEqual(TEXT("and the next drained tick counts one"), Count(0), 1);

	// The knob is the guard: a drain of 0 takes nothing and counts nothing.
	{
		const FName CivID = State.Civs[0].CivID;
		Sim->SetTables(WithDrain(0.f));
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& Undrained = Sim->GetMutableState();
		if (TestTrue(TEXT("The board of the knob case is laid"), Board(Undrained)))
		{
			Undrained.Civs[0].HomeSystemID = Undrained.Systems[0].SystemID;
			AddInstance(Undrained, Fallen, TEXT("Doctrine.Silence"), { 1 });
			Sim->RunHeadless(3);
			TestEqual(TEXT("With the drain knob at 0 nothing counts"), Undrained.Civs[0].PressureEnduredTicks, 0);
			TestEqual(TEXT("(the same people of the same seed)"), Undrained.Civs[0].CivID, CivID);
		}
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicBookCycleTest, "Cosmic_Ascension.Events.BookCycle", CosmicTests::Flags)
bool FCosmicBookCycleTest::RunTest(const FString& Parameters)
{
	using namespace CosmicBookTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	// The book is closed in the one slot in which the stay in Silence's sphere reaches 300 ticks, and the reading of that
	// slot, which sorts after the closing, finds no book left to read.
	{
		Sim->SetTables(TablesWith(Original, BaseRules, BookRows(TEXT("Doctrine.Silence"))));
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the closing cases is laid"), Board(State)))
		{
			return false;
		}
		AddInstance(State, Fallen, TEXT("Doctrine.Silence"), { 1 });
		const FName CivID = State.Civs[0].CivID;
		bool bClosedEarly = false;
		int32 ClosedAt = INDEX_NONE;
		int64 ClosedTick = -1;
		for (int32 Tick = 0; Tick < 320 && ClosedAt == INDEX_NONE; ++Tick)
		{
			Sim->RunHeadless(1);
			if (!HasBook(State.Civs[0]))
			{
				ClosedAt = State.Civs[0].FallenSphereTicks;
				ClosedTick = State.Tick;
				bClosedEarly = ClosedAt < 300;
			}
		}
		TestNotEqual(TEXT("The book is closed within 320 ticks of the stay"), ClosedAt, static_cast<int32>(INDEX_NONE));
		TestFalse(TEXT("The book is not closed before the stay reaches 300 ticks"), bClosedEarly);
		TestTrue(TEXT("The book is closed by the slot of the stay's 309th tick"), ClosedAt < 310);
		TestTrue(TEXT("The closed book carries BookShut"), HasShutBook(State.Civs[0]));
		if (ClosedAt != INDEX_NONE)
		{
			TestEqual(TEXT("The closing tells its line in that tick"), LinesOf(State, TestClosed, CivID, ClosedTick), 1);
			TestEqual(TEXT("No reading is told in the tick of the closing"), LinesOf(State, TestReading, CivID, ClosedTick), 0);
		}
		TestTrue(TEXT("The reading of the test was told before, in the slots of the open book"), LinesOf(State, TestReading, CivID) > 0);

		// Found again during the same stay, it is not closed again.
		State.Civs[0].Traits.RemoveTag(Tag(TEXT("Trait.Faith.BookShut")));
		State.Civs[0].Traits.AddTag(Tag(TEXT("Trait.Faith.Book")));
		Sim->RunHeadless(40);
		TestTrue(TEXT("A book found during the stay stays open while the stay lasts"), HasBook(State.Civs[0]));

		// Out and in again: a new stay of 300 ticks closes it again, and not a tick before.
		State.Transcendents[0].InfluencedSystems.Reset();
		Sim->RunHeadless(1);
		State.Transcendents[0].InfluencedSystems.Add(State.Systems[1].SystemID);
		Sim->RunHeadless(299);
		TestTrue(TEXT("A new stay of 299 ticks has not closed the book"), HasBook(State.Civs[0]));
		Sim->RunHeadless(11);
		TestFalse(TEXT("A new stay of 310 ticks has closed the book"), HasBook(State.Civs[0]));
	}

	// The doctrine of the condition: a row that asks for Zeal does not close the book in Silence's sphere.
	{
		Sim->SetTables(TablesWith(Original, BaseRules, BookRows(TEXT("Doctrine.Zeal"))));
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the doctrine case is laid"), Board(State)))
		{
			return false;
		}
		AddInstance(State, Fallen, TEXT("Doctrine.Silence"), { 1 });
		Sim->RunHeadless(320);
		TestEqual(TEXT("The stay of the doctrine case reached the window"), State.Civs[0].FallenSphereTicks, 320);
		TestTrue(TEXT("A row that asks for another doctrine does not close the book"), HasBook(State.Civs[0]));
	}

	// The book is found at a Society breakthrough of a people whose book is shut, the people that broke through, after
	// the line of its breakthrough. Two peoples find theirs in the same tick. Two controls: a Physics breakthrough of a
	// shut book, a Society breakthrough of an open one.
	{
		Sim->SetTables(TablesWith(Original, BaseRules, BookRows(TEXT("Doctrine.Silence"))));
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the found cases is laid"), Board(State)))
		{
			return false;
		}
		for (const int32 CivIndex : { 0, 3 })
		{
			FCivilizationState& Shut = State.Civs[CivIndex];
			Shut.Traits.RemoveTag(Tag(TEXT("Trait.Faith.Book")));
			Shut.Traits.AddTag(Tag(TEXT("Trait.Faith.BookShut")));
			Shut.Stability = 0.5f;
			ToTheMark(Shut, TEXT("Society"));
		}
		State.Civs[1].Traits.AddTag(Tag(TEXT("Trait.Faith.BookShut")));
		State.Civs[1].Stability = 0.5f;
		ToTheMark(State.Civs[1], TEXT("Physics"));
		State.Civs[2].Traits.AddTag(Tag(TEXT("Trait.Faith.Book")));
		State.Civs[2].Stability = 0.5f;
		ToTheMark(State.Civs[2], TEXT("Society"));
		Sim->RunHeadless(1);

		for (const int32 CivIndex : { 0, 3 })
		{
			const FCivilizationState& Found = State.Civs[CivIndex];
			TestTrue(*FString::Printf(TEXT("A Society breakthrough finds the shut book (people %d)"), CivIndex), HasBook(Found));
			TestFalse(*FString::Printf(TEXT("The found book no longer carries BookShut (people %d)"), CivIndex), HasShutBook(Found));
			TestEqual(*FString::Printf(TEXT("Finding the book raises Stability by the row's value (people %d)"), CivIndex), Found.Stability, 0.7f, 1e-4f);
			TestEqual(*FString::Printf(TEXT("The found row tells one line about the finding people (people %d)"), CivIndex),
				LinesOf(State, CosmicRows::EventBookFound, Found.CivID), 1);
			const int32 FoundLine = State.Log.IndexOfByPredicate([&Found](const FCosmicLogEntry& Entry)
			{
				return Entry.SourceRow == CosmicRows::EventBookFound && Entry.FocusCivID == Found.CivID;
			});
			if (TestTrue(*FString::Printf(TEXT("A line comes before the found line (people %d)"), CivIndex), FoundLine > 0))
			{
				TestEqual(*FString::Printf(TEXT("The line just before is the Society breakthrough (people %d)"), CivIndex), State.Log[FoundLine - 1].SourceRow, CosmicRows::LogBreakthrough[1]);
				TestEqual(*FString::Printf(TEXT("The breakthrough before it is the finding people's (people %d)"), CivIndex), State.Log[FoundLine - 1].FocusCivID, Found.CivID);
			}
		}
		TestTrue(TEXT("The Physics control crossed its mark"), State.Civs[1].Research.FindRef(FName(TEXT("Physics"))) >= 0.25f);
		TestTrue(TEXT("A Physics breakthrough leaves the book shut"), HasShutBook(State.Civs[1]));
		TestFalse(TEXT("A Physics breakthrough does not open the book"), HasBook(State.Civs[1]));
		TestTrue(TEXT("The open-book control crossed its Society mark"), State.Civs[2].Research.FindRef(FName(TEXT("Society"))) >= 0.25f);
		TestEqual(TEXT("An open book is not found again: the row's condition picks the shut ones"), State.Civs[2].Stability, 0.5f, 1e-4f);
		TestEqual(TEXT("The found row tells nothing about the Physics control"), LinesOf(State, CosmicRows::EventBookFound, State.Civs[1].CivID), 0);
		TestEqual(TEXT("The found row tells nothing about the open-book control"), LinesOf(State, CosmicRows::EventBookFound, State.Civs[2].CivID), 0);
	}

	// The rows of the game turn the cycle once: closed in Silence's sphere between 300 and 300 + EventSlotTicks - 1
	// ticks of the stay, found at a Society breakthrough. A people without the book gains nothing at its own mark.
	{
		Sim->SetTables(GameRowsWithoutHazards(Original));
		const int32 SlotTicks = FMath::Max(1, Sim->GetRules().EventSlotTicks);
		const FCosmicEventDef* GameFound = Original.FindEvent(CosmicRows::EventBookFound);
		if (TestNotNull(TEXT("The game has the found row"), GameFound))
		{
			TestTrue(TEXT("The game's found row is fired by name only"), GameFound->Trigger == ECosmicEventTrigger::Named);
			TestTrue(TEXT("The game's found row asks for a shut book"), GameFound->Conditions.ContainsByPredicate([](const FCosmicCondition& Condition)
			{
				return Condition.Type == EConditionType::CivHasTrait && Condition.Tag == Tag(TEXT("Trait.Faith.BookShut"));
			}));
		}
		TestNotNull(TEXT("The game has the closing row"), Original.FindEvent(GameClosed));
		TestNotNull(TEXT("The game has the reading row"), Original.FindEvent(GameReading));
		const TArray<FName> SlotOrder = Original.GetEventRowNames();
		TestTrue(TEXT("A slot looks at the game's closing row before its reading row"),
			SlotOrder.IndexOfByKey(GameClosed) != INDEX_NONE && SlotOrder.IndexOfByKey(GameClosed) < SlotOrder.IndexOfByKey(GameReading));

		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the game rows is laid"), Board(State)))
		{
			return false;
		}
		AddInstance(State, CosmicRows::TransSilence, TEXT("Doctrine.Silence"), { 1 });
		int32 ClosedAt = INDEX_NONE;
		for (int32 Tick = 0; Tick < 300 + 2 * SlotTicks && ClosedAt == INDEX_NONE; ++Tick)
		{
			Sim->RunHeadless(1);
			if (!HasBook(State.Civs[0]))
			{
				ClosedAt = State.Civs[0].FallenSphereTicks;
			}
		}
		TestNotEqual(TEXT("The game's closing row closes the book"), ClosedAt, static_cast<int32>(INDEX_NONE));
		if (ClosedAt != INDEX_NONE)
		{
			TestTrue(TEXT("The game's closing row waits for a stay of 300 ticks"), ClosedAt >= 300);
			TestTrue(TEXT("The game's closing row closes in the slot that finds the stay in its window"), ClosedAt < 300 + SlotTicks);
		}
		TestTrue(TEXT("The game's closing row leaves the book shut"), HasShutBook(State.Civs[0]));
		TestEqual(TEXT("The game's closing row tells its line"), LinesOf(State, GameClosed, State.Civs[0].CivID), 1);
		ToTheMark(State.Civs[0], TEXT("Society"));
		ToTheMark(State.Civs[1], TEXT("Society"));
		Sim->RunHeadless(1);
		TestTrue(TEXT("The game's found row opens the book at a Society breakthrough"), HasBook(State.Civs[0]));
		TestFalse(TEXT("The game's found row takes BookShut away"), HasShutBook(State.Civs[0]));
		TestTrue(TEXT("The AI control crossed its Society mark"), State.Civs[1].Research.FindRef(FName(TEXT("Society"))) >= 0.25f);
		TestFalse(TEXT("A people without the book gains none at its Society mark"), HasBook(State.Civs[1]));
		TestEqual(TEXT("The game's found row tells nothing about that people"), LinesOf(State, CosmicRows::EventBookFound, State.Civs[1].CivID), 0);
	}

	// The reading row of the game: nothing before tick 1,800, then once in the first slot after it, for the people that
	// holds the book only.
	{
		Sim->SetTables(GameRowsWithoutHazards(Original));
		const int32 SlotTicks = FMath::Max(1, Sim->GetRules().EventSlotTicks);
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the reading case is laid"), Board(State)))
		{
			return false;
		}
		Sim->RunHeadless(1799 - State.Tick);
		TestEqual(TEXT("No reading before tick 1,800"), LinesOf(State, GameReading, State.Civs[0].CivID), 0);
		Sim->RunHeadless(SlotTicks);
		TestEqual(TEXT("One reading in the first slot from tick 1,800"), LinesFrom(State, GameReading, State.Civs[0].CivID, 1800), 1);
		for (int32 CivIndex = 1; CivIndex < State.Civs.Num(); ++CivIndex)
		{
			TestEqual(*FString::Printf(TEXT("No reading for a people without the book (people %d)"), CivIndex), LinesOf(State, GameReading, State.Civs[CivIndex].CivID), 0);
		}
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

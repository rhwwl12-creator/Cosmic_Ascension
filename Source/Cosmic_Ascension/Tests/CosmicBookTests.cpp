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
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
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

	// The test's tables: its transcendent rows, these event rows and its own knobs (a slot every ten ticks, no Lottery
	// chance, no Stability drain), over the rest of the game's tables.
	FCosmicDataTables TablesWith(const FCosmicDataTables& Base, const FCosmicRulesDef& BaseRules, UDataTable* Events)
	{
		FCosmicRulesDef Knobs = BaseRules;
		Knobs.EventSlotTicks = 10;
		Knobs.EventChancePerSlot = 0.f;
		Knobs.SpiritualPressureStabilityDrain = 0.f;
		UDataTable* RulesTable = NewTable(FCosmicRulesDef::StaticStruct());
		RulesTable->AddRow(CosmicRows::Rules, Knobs);
		FCosmicDataTables Tables = Base;
		Tables.Rules = RulesTable;
		Tables.Transcendents = TranscendentRows();
		Tables.Events = Events;
		return Tables;
	}

	// A board the test knows: the chain of CosmicTests::WireChain, civilization 0 holds systems 0 and 1, the others one
	// system each (6, 9, 11), no transcendent, and civilization 0 has the book. False below twelve systems or four peoples.
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
		State.Civs[0].Traits.AddTag(Tag(TEXT("Trait.Faith.Book")));
		State.Civs[0].Traits.RemoveTag(Tag(TEXT("Trait.Faith.BookShut")));
		return true;
	}

	FTranscendentState& AddInstance(FGalaxyState& State, FName TranscendentID, const TCHAR* Doctrine, std::initializer_list<int32> Sphere)
	{
		FTranscendentState& Transcendent = State.Transcendents.AddDefaulted_GetRef();
		Transcendent.TranscendentID = TranscendentID;
		Transcendent.Doctrine = Tag(Doctrine);
		for (const int32 SystemIndex : Sphere)
		{
			Transcendent.InfluencedSystems.Add(State.Systems[SystemIndex].SystemID);
		}
		return Transcendent;
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

	// The book rows of the test: shut after Key ticks in a sphere of Doctrine (a Standing row), found by name.
	UDataTable* BookRows(const TCHAR* Doctrine)
	{
		UDataTable* Events = NewTable(FCosmicEventDef::StaticStruct());
		FCosmicEventDef Shut;
		Shut.Category = Tag(TEXT("Event.Civ.Society"));
		Shut.LogCategory = Tag(TEXT("Log.Civ"));
		Shut.Trigger = ECosmicEventTrigger::Standing;
		Shut.Subject = ECosmicEventSubject::Civilization;
		Shut.BaseWeight = 0.f;
		Shut.CooldownTicks = 0;
		Shut.Conditions = { Cond(EConditionType::CivHasTrait, TEXT("Trait.Faith.Book"), nullptr), Cond(EConditionType::CivFallenSphereTicksReached, Doctrine, TEXT("300")) };
		Shut.Effects = { Eff(EEffectType::RemoveTrait, TEXT("Trait.Faith.Book")), Eff(EEffectType::AddTrait, TEXT("Trait.Faith.BookShut")) };
		Shut.LogTemplates = { FText::AsCultureInvariant(TEXT("shut one")), FText::AsCultureInvariant(TEXT("shut two")), FText::AsCultureInvariant(TEXT("shut three")) };
		Events->AddRow(FName(TEXT("event_test_shut")), Shut);
		FCosmicEventDef Found = Shut;
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
		for (TPair<FName, float>& Focus : Civ.ResearchFocus)
		{
			Focus.Value = 0.f;
		}
		Civ.ResearchFocus.Add(FName(Field), 1.f);
		Civ.Research.Add(FName(Field), 0.2499f);
		Civ.Population = 1.f;
		Civ.Stockpile.Insight = 100.f;
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
	FTranscendentState& Silence = AddInstance(State, Fallen, TEXT("Doctrine.Silence"), { 1 });
	auto Sphere = [&State, &Silence](std::initializer_list<int32> Systems)
	{
		Silence.InfluencedSystems.Reset();
		for (const int32 SystemIndex : Systems)
		{
			Silence.InfluencedSystems.Add(State.Systems[SystemIndex].SystemID);
		}
	};
	const int32& Count = State.Civs[0].FallenSphereTicks;

	Sim->RunHeadless(5);
	TestEqual(TEXT("A settled system inside a fallen sphere counts one a tick"), Count, 5);
	TestEqual(TEXT("A people without a system inside counts nothing"), State.Civs[1].FallenSphereTicks, 0);
	Sphere({ 0, 1 });
	Sim->RunHeadless(3);
	TestEqual(TEXT("Two systems inside still count one a tick"), Count, 8);
	Sphere({});
	Sim->RunHeadless(1);
	TestEqual(TEXT("Leaving the sphere starts the count over"), Count, 0);
	Sphere({ 1 });
	Sim->RunHeadless(2);
	TestEqual(TEXT("Coming back counts from one"), Count, 2);

	// A lane away is not inside.
	Sphere({ 2 });
	Sim->RunHeadless(1);
	TestEqual(TEXT("A system one lane from the sphere is not inside"), Count, 0);
	Sphere({ 1 });
	Sim->RunHeadless(1);
	TestEqual(TEXT("Inside again after the lane case"), Count, 1);

	// A system being settled is owned, not settled.
	State.Systems[1].ActiveMegaproject = CosmicRows::ProjectSettle;
	Sim->RunHeadless(1);
	TestEqual(TEXT("A system being settled does not count"), Count, 0);
	State.Systems[1].ActiveMegaproject = NAME_None;
	Sim->RunHeadless(1);
	TestEqual(TEXT("Settled again, it counts"), Count, 1);

	// A sealed transcendent, an instance without a row and an independent sphere press no fallen sphere on anyone.
	Silence.bSealed = true;
	Sim->RunHeadless(1);
	TestEqual(TEXT("The sphere of a sealed transcendent does not count"), Count, 0);
	Silence.bSealed = false;
	Sim->RunHeadless(1);
	TestEqual(TEXT("Unsealed again, it counts"), Count, 1);
	Silence.TranscendentID = Rowless;
	Sim->RunHeadless(1);
	TestEqual(TEXT("The sphere of an instance without a row does not count"), Count, 0);
	Silence.TranscendentID = Apart;
	Silence.Doctrine = Tag(TEXT("Doctrine.Stillness"));
	Sim->RunHeadless(1);
	TestEqual(TEXT("An independent sphere is not a fallen one"), Count, 0);
	Silence.TranscendentID = Fallen;
	Silence.Doctrine = Tag(TEXT("Doctrine.Silence"));
	Sim->RunHeadless(1);
	TestEqual(TEXT("The fallen row again, it counts"), Count, 1);

	// A system of another people counts for that people only.
	Sphere({ 6 });
	Sim->RunHeadless(1);
	TestEqual(TEXT("Another people's system inside does not count for this one"), Count, 0);
	TestEqual(TEXT("It counts for the people that holds it"), State.Civs[1].FallenSphereTicks, 1);

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

	// The book is shut in the one slot in which the stay in Silence's sphere reaches 300 ticks.
	{
		Sim->SetTables(TablesWith(Original, BaseRules, BookRows(TEXT("Doctrine.Silence"))));
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the shut cases is laid"), Board(State)))
		{
			return false;
		}
		FTranscendentState& Silence = AddInstance(State, Fallen, TEXT("Doctrine.Silence"), { 1 });
		FCivilizationState& Civ = State.Civs[0];
		bool bShutEarly = false;
		int32 ShutAt = INDEX_NONE;
		for (int32 Tick = 0; Tick < 320 && ShutAt == INDEX_NONE; ++Tick)
		{
			Sim->RunHeadless(1);
			if (!HasBook(Civ))
			{
				ShutAt = Civ.FallenSphereTicks;
				bShutEarly = Civ.FallenSphereTicks < 300;
			}
		}
		TestNotEqual(TEXT("The book is shut within 320 ticks of the stay"), ShutAt, static_cast<int32>(INDEX_NONE));
		TestFalse(TEXT("The book is not shut before the stay reaches 300 ticks"), bShutEarly);
		TestTrue(TEXT("The book is shut by the slot of the stay's 309th tick"), ShutAt < 310);
		TestTrue(TEXT("The shut book carries BookShut"), HasShutBook(Civ));

		// Found again during the same stay, it is not shut again.
		Civ.Traits.RemoveTag(Tag(TEXT("Trait.Faith.BookShut")));
		Civ.Traits.AddTag(Tag(TEXT("Trait.Faith.Book")));
		Sim->RunHeadless(40);
		TestTrue(TEXT("A book found during the stay stays open while the stay lasts"), HasBook(Civ));

		// Out and in again: a new stay of 300 ticks shuts it again, and not a tick before.
		Silence.InfluencedSystems.Reset();
		Sim->RunHeadless(1);
		Silence.InfluencedSystems.Add(State.Systems[1].SystemID);
		Sim->RunHeadless(299);
		TestTrue(TEXT("A new stay of 299 ticks has not shut the book"), HasBook(Civ));
		Sim->RunHeadless(11);
		TestFalse(TEXT("A new stay of 310 ticks has shut the book"), HasBook(Civ));
	}

	// The doctrine of the condition: a row that asks for Zeal does not shut the book in Silence's sphere.
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
		TestTrue(TEXT("A row that asks for another doctrine does not shut the book"), HasBook(State.Civs[0]));
	}

	// The book is found at a Society breakthrough of a people whose book is shut, after the line of the breakthrough.
	// Two controls in the same tick: a Physics breakthrough of a shut book, a Society breakthrough of an open one.
	{
		Sim->SetTables(TablesWith(Original, BaseRules, BookRows(TEXT("Doctrine.Silence"))));
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the found cases is laid"), Board(State)))
		{
			return false;
		}
		FCivilizationState& Shut = State.Civs[0];
		Shut.Traits.RemoveTag(Tag(TEXT("Trait.Faith.Book")));
		Shut.Traits.AddTag(Tag(TEXT("Trait.Faith.BookShut")));
		Shut.Stability = 0.5f;
		ToTheMark(Shut, TEXT("Society"));
		FCivilizationState& OtherField = State.Civs[1];
		OtherField.Traits.AddTag(Tag(TEXT("Trait.Faith.BookShut")));
		OtherField.Stability = 0.5f;
		ToTheMark(OtherField, TEXT("Physics"));
		FCivilizationState& Open = State.Civs[2];
		Open.Traits.AddTag(Tag(TEXT("Trait.Faith.Book")));
		Open.Stability = 0.5f;
		ToTheMark(Open, TEXT("Society"));
		Sim->RunHeadless(1);

		TestTrue(TEXT("A Society breakthrough finds the shut book"), HasBook(Shut));
		TestFalse(TEXT("The found book no longer carries BookShut"), HasShutBook(Shut));
		TestEqual(TEXT("Finding the book raises Stability by the row's value"), Shut.Stability, 0.7f, 1e-4f);
		TestTrue(TEXT("A Physics breakthrough leaves the book shut"), HasShutBook(OtherField));
		TestFalse(TEXT("A Physics breakthrough does not open the book"), HasBook(OtherField));
		TestEqual(TEXT("An open book is not found again: the row's condition picks the shut ones"), Open.Stability, 0.5f, 1e-4f);
		const TArray<FCosmicLogEntry>& Log = State.Log;
		const int32 FoundLine = Log.IndexOfByPredicate([](const FCosmicLogEntry& Entry) { return Entry.SourceRow == CosmicRows::EventBookFound; });
		TestTrue(TEXT("The found row tells its line"), FoundLine != INDEX_NONE);
		if (TestTrue(TEXT("A line comes before the found line"), FoundLine > 0))
		{
			TestEqual(TEXT("The line just before is the Society breakthrough"), Log[FoundLine - 1].SourceRow, CosmicRows::LogBreakthrough[1]);
			TestEqual(TEXT("The breakthrough before it is the finding people's"), Log[FoundLine - 1].FocusCivID, Shut.CivID);
		}
		TestEqual(TEXT("The found row tells one line in that tick"),
			Log.FilterByPredicate([](const FCosmicLogEntry& Entry) { return Entry.SourceRow == CosmicRows::EventBookFound; }).Num(), 1);
	}

	// The rows of the game turn the cycle once: shut in Silence's sphere, found at a Society breakthrough.
	{
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the game rows is laid"), Board(State)))
		{
			return false;
		}
		AddInstance(State, CosmicRows::TransSilence, TEXT("Doctrine.Silence"), { 1 });
		FCivilizationState& Civ = State.Civs[0];
		Sim->RunHeadless(310);
		TestTrue(TEXT("The game's row shuts the book after 300 ticks in Silence's sphere"), HasShutBook(Civ));
		TestFalse(TEXT("The game's shut row takes the open book away"), HasBook(Civ));
		TestTrue(TEXT("The game's shut row tells its line"),
			State.Log.ContainsByPredicate([](const FCosmicLogEntry& Entry) { return Entry.SourceRow == FName(TEXT("event_book_shut")); }));
		ToTheMark(Civ, TEXT("Society"));
		Sim->RunHeadless(1);
		TestTrue(TEXT("The game's found row opens the book at a Society breakthrough"), HasBook(Civ));
		TestFalse(TEXT("The game's found row takes BookShut away"), HasShutBook(Civ));
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

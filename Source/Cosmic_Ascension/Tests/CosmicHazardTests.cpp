// Cosmic Ascension - automation tests of the hazard stage, the last-system gate, the claim rule, the throne and the
// deposition pipeline. Design: guideline.md section 4.1, 4.2, 4.5, 5.10, 7.2, 7.4, 7.5, 9.2, 9.3, 16 #29 #52 #53 #56.

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
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/Function.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/NameTypes.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#include <initializer_list>
#include <limits>

namespace CosmicHazardTests
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

	FCosmicCondition Cond(EConditionType Type, const TCHAR* TagName = nullptr, const TCHAR* Key = nullptr, float Value = 0.f)
	{
		FCosmicCondition Condition;
		Condition.Type = Type;
		Condition.Tag = TagName ? Tag(TagName) : FGameplayTag();
		Condition.Key = Key ? FName(Key) : FName();
		Condition.Value = Value;
		return Condition;
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

	TArray<FText> Lines(const FString& Stem)
	{
		return {
			FText::AsCultureInvariant(Stem + TEXT(" one")),
			FText::AsCultureInvariant(Stem + TEXT(" two")),
			FText::AsCultureInvariant(Stem + TEXT(" three"))
		};
	}

	// The rows of the tests.
	const FName Storm(TEXT("crisis_test_storm"));
	const FName Unrest(TEXT("crisis_test_unrest"));
	const FName ClaimRow(TEXT("crisis_test_claim"));
	const FName Nova(TEXT("crisis_test_nova"));
	const FName Waking(TEXT("crisis_test_waking"));
	const FName Erosion(TEXT("crisis_test_erosion"));
	const FName Spark(TEXT("crisis_test_spark"));
	const FName Outbreak(TEXT("event_test_outbreak"));
	const FName Cure(TEXT("event_test_cure"));
	const FName Fallen(TEXT("tr_test_fallen"));
	const FName Zealous(TEXT("tr_test_zeal"));
	const FName Other(TEXT("tr_test_other"));
	const FName Small(TEXT("tr_test_small"));
	const FName Pair(TEXT("tr_test_pair"));
	const FName Heaven(TEXT("tr_test_heaven"));
	const FName Apart(TEXT("tr_test_apart"));
	const FName Matter(TEXT("Matter"));
	const FName Causality(TEXT("Causality"));
	const FName Energy(TEXT("Energy"));

	// A hazard row of the tests: its lines name the moment, so a case can tell an outbreak from an expiry.
	FHazardDef HazardRow(const TCHAR* TagName, ECosmicHazardSpawn Rule, float Chance, int32 Countdown)
	{
		FHazardDef Row;
		Row.HazardTag = Tag(TagName);
		Row.DisplayName = FText::AsCultureInvariant(FString::Printf(TEXT("<%s>"), TagName));
		Row.CountdownTicks = Countdown;
		Row.SpawnRule = Rule;
		Row.SpawnChancePerTick = Chance;
		Row.LogCategory = Tag(TEXT("Log.Crisis"));
		Row.Severity = 1;
		Row.OutbreakTemplates = Lines(FString(TagName) + TEXT(" outbreak"));
		Row.ExpireTemplates = Lines(FString(TagName) + TEXT(" expiry"));
		Row.ResolveTemplates = Lines(FString(TagName) + TEXT(" resolution"));
		return Row;
	}

	FTranscendentDef TranscendentRow(const TCHAR* Doctrine, const TCHAR* Allegiance, float Depth, int32 MaxSphere)
	{
		FTranscendentDef Row;
		Row.Doctrine = Tag(Doctrine);
		Row.Allegiance = Tag(Allegiance);
		Row.Tier = EAscensionTier::Tier4_Transcendent;
		Row.Depth = Depth;
		Row.MaxInfluencedSystems = MaxSphere;
		Row.SpawnWeight = 0.f;
		return Row;
	}

	// Five fallen rows of tier 4 (two of depth 0, one of depth 1, one with a sphere of one system, one of two), a celestial
	// row and an independent row. Their weights are 0: the roster of NewGalaxy makes no instance of them.
	UDataTable* TranscendentRows()
	{
		UDataTable* Table = NewTable(FTranscendentDef::StaticStruct());
		Table->AddRow(Fallen, TranscendentRow(TEXT("Doctrine.Silence"), TEXT("Realm.Abyssal"), 0.f, 12));
		Table->AddRow(Zealous, TranscendentRow(TEXT("Doctrine.Zeal"), TEXT("Realm.Abyssal"), 1.f, 12));
		Table->AddRow(Other, TranscendentRow(TEXT("Doctrine.Assimilation"), TEXT("Realm.Abyssal"), 0.f, 12));
		Table->AddRow(Small, TranscendentRow(TEXT("Doctrine.Recurrence"), TEXT("Realm.Abyssal"), 0.f, 1));
		Table->AddRow(Pair, TranscendentRow(TEXT("Doctrine.Silence"), TEXT("Realm.Abyssal"), 0.f, 2));
		Table->AddRow(Heaven, TranscendentRow(TEXT("Doctrine.Covenant"), TEXT("Realm.Celestial"), 0.f, 12));
		Table->AddRow(Apart, TranscendentRow(TEXT("Doctrine.Stillness"), TEXT("Realm.Independent"), 0.f, 12));
		return Table;
	}

	// The knobs of the tests: a slot every ten ticks, no Lottery chance, no Stability drain, claims one lane out.
	FCosmicRulesDef Knobs(const FCosmicRulesDef& BaseRules)
	{
		FCosmicRulesDef Rules = BaseRules;
		Rules.EventSlotTicks = 10;
		Rules.EventChancePerSlot = 0.f;
		Rules.SpiritualPressureStabilityDrain = 0.f;
		Rules.ClaimAdjacencyLanes = 1;
		return Rules;
	}

	// The game's tables with the test's rules, hazard rows, event rows (none when null) and transcendent rows.
	FCosmicDataTables TablesWith(const FCosmicDataTables& Original, const FCosmicRulesDef& Rules, UDataTable* Hazards, UDataTable* Events)
	{
		UDataTable* RulesTable = NewTable(FCosmicRulesDef::StaticStruct());
		RulesTable->AddRow(CosmicRows::Rules, Rules);
		FCosmicDataTables Tables = Original;
		Tables.Rules = RulesTable;
		Tables.Hazards = Hazards ? Hazards : NewTable(FHazardDef::StaticStruct());
		Tables.Events = Events ? Events : NewTable(FCosmicEventDef::StaticStruct());
		Tables.Transcendents = TranscendentRows();
		return Tables;
	}

	// A copy of a row of the game's DT_Events in a table of the test. False when the game lacks it.
	bool CopyEvent(const FCosmicDataTables& Original, UDataTable* Into, FName RowName)
	{
		const FCosmicEventDef* Row = Original.FindEvent(RowName);
		if (Row)
		{
			Into->AddRow(RowName, *Row);
		}
		return Row != nullptr;
	}

	// The board of the tests: the chain of CosmicTests::WireChain (0-1-...-9, 4-10, 10-11); civilization 0 (the vessel's)
	// holds 0, 1 and 2 with its capital in 0, civilization 1 holds 6, civilization 2 holds 7 and civilization 3 holds 11.
	// Nobody researches; every Stability 0.75; no transcendent, hazard, megaproject, notable or flag. False below twelve
	// systems or four peoples.
	bool Board(FGalaxyState& State)
	{
		if (!CosmicTests::WireChain(State) || State.Civs.Num() < 4)
		{
			return false;
		}
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { 0, 1, 2 });
		CosmicTests::Give(State, 1, { 6 });
		CosmicTests::Give(State, 2, { 7 });
		CosmicTests::Give(State, 3, { 11 });
		State.Transcendents.Reset();
		State.Flags.Reset();
		for (FStarSystemState& System : State.Systems)
		{
			System.ActiveHazard = FGameplayTag();
			System.HazardTicksRemaining = 0;
			System.ActiveMegaproject = NAME_None;
		}
		for (FCivilizationState& Civ : State.Civs)
		{
			for (TPair<FName, float>& Focus : Civ.ResearchFocus)
			{
				Focus.Value = 0.f;
			}
			Civ.Stability = 0.75f;
			Civ.LowStabilityTicks = 0;
			Civ.Notables.Reset();
		}
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

	void PlaceByHand(FGalaxyState& State, int32 SystemIndex, const TCHAR* HazardTag, int32 Ticks)
	{
		State.Systems[SystemIndex].ActiveHazard = Tag(HazardTag);
		State.Systems[SystemIndex].HazardTicksRemaining = Ticks;
	}

	// Index into State.Log of the first line of Row at Tick (any tick when below 0) whose text carries Word, from Start
	// on. INDEX_NONE when there is none.
	int32 FindLine(const FGalaxyState& State, FName Row, int64 Tick, const TCHAR* Word = nullptr, int32 Start = 0)
	{
		for (int32 Index = FMath::Max(0, Start); Index < State.Log.Num(); ++Index)
		{
			const FCosmicLogEntry& Entry = State.Log[Index];
			if (Entry.SourceRow == Row && (Tick < 0 || Entry.Tick == Tick) && (!Word || Entry.Message.ToString().Contains(Word)))
			{
				return Index;
			}
		}
		return INDEX_NONE;
	}

	int32 CountLines(const FGalaxyState& State, FName Row, int64 Tick = -1, const TCHAR* Word = nullptr)
	{
		int32 Count = 0;
		for (const FCosmicLogEntry& Entry : State.Log)
		{
			Count += (Entry.SourceRow == Row && (Tick < 0 || Entry.Tick == Tick) && (!Word || Entry.Message.ToString().Contains(Word))) ? 1 : 0;
		}
		return Count;
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

	bool InSphere(const FGalaxyState& State, int32 TranscendentIndex, int32 SystemIndex)
	{
		return State.Transcendents.IsValidIndex(TranscendentIndex) && State.Transcendents[TranscendentIndex].InfluencedSystems.Contains(State.Systems[SystemIndex].SystemID);
	}

	// A detached state with its own index and stream: what Apply changes stays here.
	struct FBench
	{
		FBench(const FGalaxyState& InState, UGalaxySimulationSubsystem& Sim)
			: State(InState), Stream(7), Ctx(State, Stream, Index, Sim.GetTables(), Sim.GetRules())
		{
			Index.Rebuild(State);
			Ctx.SourceRow = FName(TEXT("event_test_bench"));
		}

		FGalaxyState State;
		FCosmicStateIndex Index;
		FRandomStream Stream;
		FCosmicEvalContext Ctx;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicEventsHazardEffectsTest, "Cosmic_Ascension.Events.HazardEffects", CosmicTests::Flags)
bool FCosmicEventsHazardEffectsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHazardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	UDataTable* Hazards = NewTable(FHazardDef::StaticStruct());
	Hazards->AddRow(Storm, HazardRow(TEXT("Hazard.Plague"), ECosmicHazardSpawn::Draw, 0.f, 100));
	Hazards->AddRow(Unrest, HazardRow(TEXT("Hazard.Rebellion"), ECosmicHazardSpawn::Capital, 0.f, 200));
	Hazards->AddRow(ClaimRow, HazardRow(TEXT("Hazard.Claim"), ECosmicHazardSpawn::ClaimRule, 0.f, 300));
	FHazardDef WakingRow = HazardRow(TEXT("Hazard.AIAwakening"), ECosmicHazardSpawn::Draw, 0.f, 150);
	WakingRow.OnResolveEffects = { Eff(EEffectType::ChangeStability, nullptr, NAME_None, 0.1f), Eff(EEffectType::SpawnHazard, TEXT("Hazard.Plague")) };
	Hazards->AddRow(Waking, WakingRow);
	Hazards->AddRow(Erosion, HazardRow(TEXT("Hazard.SubspaceErosion"), ECosmicHazardSpawn::Draw, 0.f, 0));
	Sim->SetTables(TablesWith(Original, Knobs(BaseRules), Hazards, nullptr));
	Sim->NewGalaxy(42, 12, 4);
	FGalaxyState Laid = Sim->GetState();
	if (!TestTrue(TEXT("The board of the test is laid"), Board(Laid)))
	{
		return false;
	}
	const FGameplayTag Plague = Tag(TEXT("Hazard.Plague"));
	auto Spawn = [](const TCHAR* TagName) { return Eff(EEffectType::SpawnHazard, TagName); };
	auto Resolve = [](const TCHAR* TagName) { return Eff(EEffectType::ResolveHazard, TagName); };
	auto Transfer = [](FName Key) { return Eff(EEffectType::TransferSystem, nullptr, Key); };

	// SpawnHazard: the one path, on a settled system with a free slot.
	{
		FBench Bench(Laid, *Sim);
		FGalaxyState& State = Bench.State;
		Bench.Ctx.CivIndex = 2;
		Bench.Ctx.SystemIndex = 1;
		TestTrue(TEXT("SpawnHazard places the row of its tag on a settled system"), UCosmicEventSubsystem::Apply(Spawn(TEXT("Hazard.Plague")), Bench.Ctx));
		TestTrue(TEXT("The slot holds the tag of the row"), State.Systems[1].ActiveHazard == Plague);
		TestEqual(TEXT("The countdown is the row's"), State.Systems[1].HazardTicksRemaining, 100);
		if (TestEqual(TEXT("The placement leaves one note"), Bench.Ctx.Notes.Num(), 1))
		{
			const FCosmicWorldNote& Note = Bench.Ctx.Notes[0];
			TestTrue(TEXT("The note says a hazard was placed"), Note.Kind == ECosmicWorldNote::HazardPlaced);
			TestEqual(TEXT("The note names the system"), Note.SystemIndex, 1);
			TestEqual(TEXT("The note names the owner, not the civilization of the effect"), Note.CivIndex, 0);
			TestEqual(TEXT("The note names the hazard row"), Note.HazardRow, Storm);
		}
		TestFalse(TEXT("A taken slot refuses a second hazard"), UCosmicEventSubsystem::Apply(Spawn(TEXT("Hazard.AIAwakening")), Bench.Ctx));
		TestTrue(TEXT("The first hazard stays"), State.Systems[1].ActiveHazard == Plague);
		TestEqual(TEXT("A refused placement leaves no note"), Bench.Ctx.Notes.Num(), 1);

		Bench.Ctx.SystemIndex = 3;
		TestFalse(TEXT("An unowned system gets no hazard"), UCosmicEventSubsystem::Apply(Spawn(TEXT("Hazard.Plague")), Bench.Ctx));
		State.Systems[2].ActiveMegaproject = CosmicRows::ProjectSettle;
		Bench.Ctx.SystemIndex = 2;
		TestFalse(TEXT("A system being settled gets no hazard"), UCosmicEventSubsystem::Apply(Spawn(TEXT("Hazard.Plague")), Bench.Ctx));
		State.Systems[2].ActiveMegaproject = NAME_None;
		TestFalse(TEXT("A Capital row stays away from a system that is not its owner's capital"), UCosmicEventSubsystem::Apply(Spawn(TEXT("Hazard.Rebellion")), Bench.Ctx));
		Bench.Ctx.SystemIndex = 0;
		TestTrue(TEXT("A Capital row lands on its owner's capital"), UCosmicEventSubsystem::Apply(Spawn(TEXT("Hazard.Rebellion")), Bench.Ctx));
		Bench.Ctx.SystemIndex = 6;
		TestFalse(TEXT("SpawnHazard never places the claim"), UCosmicEventSubsystem::Apply(Spawn(TEXT("Hazard.Claim")), Bench.Ctx));
		TestFalse(TEXT("A tag no row holds places nothing"), UCosmicEventSubsystem::Apply(Spawn(TEXT("Hazard.Cult")), Bench.Ctx));
		Bench.Ctx.bPerTickSource = true;
		TestFalse(TEXT("A per-tick source places nothing"), UCosmicEventSubsystem::Apply(Spawn(TEXT("Hazard.Plague")), Bench.Ctx));
		Bench.Ctx.bPerTickSource = false;
		TestFalse(TEXT("Nothing was placed on that system so far"), State.Systems[6].ActiveHazard.IsValid());
		Bench.Ctx.SystemIndex = INDEX_NONE;
		TestFalse(TEXT("Without a system SpawnHazard places nothing"), UCosmicEventSubsystem::Apply(Spawn(TEXT("Hazard.Plague")), Bench.Ctx));
		Bench.Ctx.SystemIndex = 6;
		TestTrue(TEXT("A row whose countdown is below 1 is placed"), UCosmicEventSubsystem::Apply(Spawn(TEXT("Hazard.SubspaceErosion")), Bench.Ctx));
		TestEqual(TEXT("Its countdown reads as 1"), State.Systems[6].HazardTicksRemaining, 1);
	}

	// ResolveHazard: the slot first, then the row's effects on the system and its owner.
	{
		FBench Bench(Laid, *Sim);
		FGalaxyState& State = Bench.State;
		State.Tick = 40;
		PlaceByHand(State, 0, TEXT("Hazard.Rebellion"), 50);
		FNotableRecord& Rebel = State.Civs[0].Notables.AddDefaulted_GetRef();
		Rebel.Role = Tag(TEXT("Notable.Rebel"));
		Rebel.FirstTick = 10;
		Rebel.SourceRow = FName(TEXT("event_test_rises"));
		State.Civs[0].LowStabilityTicks = 150;
		Bench.Ctx.CivIndex = 0;
		Bench.Ctx.SystemIndex = 0;
		TestTrue(TEXT("ResolveHazard resolves the hazard of the subject system"), UCosmicEventSubsystem::Apply(Resolve(nullptr), Bench.Ctx));
		TestFalse(TEXT("Its slot is empty"), State.Systems[0].ActiveHazard.IsValid());
		TestEqual(TEXT("Its countdown is 0"), State.Systems[0].HazardTicksRemaining, 0);
		if (TestEqual(TEXT("The rebel is still on the record"), State.Civs[0].Notables.Num(), 1))
		{
			TestEqual(TEXT("The rebel leaves the record at the tick of the resolution"), State.Civs[0].Notables[0].LastTick, static_cast<int64>(40));
		}
		TestEqual(TEXT("The count of discontent starts over"), State.Civs[0].LowStabilityTicks, 0);
		if (TestEqual(TEXT("The resolution leaves one note"), Bench.Ctx.Notes.Num(), 1))
		{
			TestTrue(TEXT("The note says a hazard was resolved"), Bench.Ctx.Notes[0].Kind == ECosmicWorldNote::HazardResolved);
			TestEqual(TEXT("The note names the hazard row"), Bench.Ctx.Notes[0].HazardRow, Unrest);
		}
		TestFalse(TEXT("An empty slot has nothing to resolve"), UCosmicEventSubsystem::Apply(Resolve(nullptr), Bench.Ctx));

		PlaceByHand(State, 1, TEXT("Hazard.Plague"), 50);
		Bench.Ctx.SystemIndex = 1;
		TestFalse(TEXT("A Tag that does not match leaves the hazard"), UCosmicEventSubsystem::Apply(Resolve(TEXT("Hazard.Rebellion")), Bench.Ctx));
		TestTrue(TEXT("The hazard that did not match stays"), State.Systems[1].ActiveHazard == Plague);
		TestTrue(TEXT("A Tag that matches resolves it"), UCosmicEventSubsystem::Apply(Resolve(TEXT("Hazard.Plague")), Bench.Ctx));

		// The row's effects land on the system and its owner, after the note of the resolution.
		PlaceByHand(State, 6, TEXT("Hazard.AIAwakening"), 50);
		State.Civs[1].Stability = 0.5f;
		State.Civs[2].Stability = 0.5f;
		Bench.Ctx.Notes.Reset();
		Bench.Ctx.CivIndex = 2;
		Bench.Ctx.SystemIndex = 6;
		TestTrue(TEXT("A hazard with effects of its own is resolved"), UCosmicEventSubsystem::Apply(Resolve(nullptr), Bench.Ctx));
		TestEqual(TEXT("The owner takes the effect of the resolution"), State.Civs[1].Stability, 0.5f + 0.1f);
		TestEqual(TEXT("The civilization of the effect takes nothing"), State.Civs[2].Stability, 0.5f);
		TestTrue(TEXT("The resolution's own SpawnHazard found the slot empty"), State.Systems[6].ActiveHazard == Plague);
		if (TestEqual(TEXT("Two notes"), Bench.Ctx.Notes.Num(), 2))
		{
			TestTrue(TEXT("The resolution comes first"), Bench.Ctx.Notes[0].Kind == ECosmicWorldNote::HazardResolved && Bench.Ctx.Notes[0].HazardRow == Waking);
			TestTrue(TEXT("What its effects placed comes after"), Bench.Ctx.Notes[1].Kind == ECosmicWorldNote::HazardPlaced && Bench.Ctx.Notes[1].HazardRow == Storm);
			TestEqual(TEXT("The placement is about the owner"), Bench.Ctx.Notes[1].CivIndex, 1);
		}
		TestEqual(TEXT("The subject civilization of the context is back"), Bench.Ctx.CivIndex, 2);
		TestEqual(TEXT("The source row of the context is back"), Bench.Ctx.SourceRow, FName(TEXT("event_test_bench")));

		// A tag no row holds is emptied, and nothing is left to tell.
		PlaceByHand(State, 9, TEXT("Hazard.Cult"), 50);
		Bench.Ctx.Notes.Reset();
		Bench.Ctx.SystemIndex = 9;
		TestTrue(TEXT("A hazard without a row is resolved"), UCosmicEventSubsystem::Apply(Resolve(nullptr), Bench.Ctx));
		TestFalse(TEXT("Its slot is empty"), State.Systems[9].ActiveHazard.IsValid());
		TestEqual(TEXT("No line is due for it"), Bench.Ctx.Notes.Num(), 0);
	}

	// TransferSystem: one path for a system to change hands; never the last owned system.
	{
		FBench Bench(Laid, *Sim);
		FGalaxyState& State = Bench.State;
		const FName Civ1 = State.Civs[1].CivID;
		State.Systems[2].ActiveMegaproject = FName(TEXT("project_test"));
		State.Systems[2].MegaprojectProgress = 0.5f;
		State.Systems[2].MegaprojectCompletedTick = 7;
		PlaceByHand(State, 2, TEXT("Hazard.Plague"), 50);
		Bench.Ctx.CivIndex = 0;
		Bench.Ctx.SystemIndex = 2;
		TestTrue(TEXT("TransferSystem hands a system to the people its Key names"), UCosmicEventSubsystem::Apply(Transfer(Civ1), Bench.Ctx));
		TestEqual(TEXT("The new owner holds it"), State.Systems[2].OwnerCivID, Civ1);
		TestFalse(TEXT("It left the old owner's systems"), State.Civs[0].OwnedSystems.Contains(State.Systems[2].SystemID));
		TestTrue(TEXT("It joined the end of the new owner's systems"), !State.Civs[1].OwnedSystems.IsEmpty() && State.Civs[1].OwnedSystems.Last() == State.Systems[2].SystemID);
		TestEqual(TEXT("Its megaproject ended"), State.Systems[2].ActiveMegaproject, FName());
		TestEqual(TEXT("Its progress is gone"), State.Systems[2].MegaprojectProgress, 0.f);
		TestEqual(TEXT("A completion stays on the record"), State.Systems[2].MegaprojectCompletedTick, static_cast<int64>(7));
		TestTrue(TEXT("A drawn hazard goes with the system"), State.Systems[2].ActiveHazard == Plague);
		TestEqual(TEXT("The old owner keeps its capital"), State.Civs[0].HomeSystemID, State.Systems[0].SystemID);
		TestEqual(TEXT("The old owner keeps its Stability"), State.Civs[0].Stability, 0.75f);

		PlaceByHand(State, 1, TEXT("Hazard.Plague"), 50);
		Bench.Ctx.SystemIndex = 1;
		TestTrue(TEXT("An empty Key lets a system go to no one"), UCosmicEventSubsystem::Apply(Transfer(NAME_None), Bench.Ctx));
		TestTrue(TEXT("The system let go has no owner"), State.Systems[1].OwnerCivID.IsNone());
		TestFalse(TEXT("The system let go loses its hazard"), State.Systems[1].ActiveHazard.IsValid());
		TestFalse(TEXT("The system let go left its owner's systems"), State.Civs[0].OwnedSystems.Contains(State.Systems[1].SystemID));

		Bench.Ctx.SystemIndex = 0;
		TestFalse(TEXT("The last owned system is never handed over"), UCosmicEventSubsystem::Apply(Transfer(Civ1), Bench.Ctx));
		TestFalse(TEXT("The last owned system is never let go"), UCosmicEventSubsystem::Apply(Transfer(NAME_None), Bench.Ctx));
		TestEqual(TEXT("The last owned system stays with its people"), State.Systems[0].OwnerCivID, State.Civs[0].CivID);
		Bench.Ctx.SystemIndex = 1;
		TestFalse(TEXT("An unowned system cannot be let go"), UCosmicEventSubsystem::Apply(Transfer(NAME_None), Bench.Ctx));
		Bench.Ctx.SystemIndex = 2;
		TestFalse(TEXT("A system is not handed to the people that holds it"), UCosmicEventSubsystem::Apply(Transfer(Civ1), Bench.Ctx));
		TestFalse(TEXT("A Key that names no living people is turned away"), UCosmicEventSubsystem::Apply(Transfer(FName(TEXT("civ_gone"))), Bench.Ctx));
		State.Systems[3].StarClass = FGameplayTag();
		Bench.Ctx.SystemIndex = 3;
		TestFalse(TEXT("A dead node is nobody's to take"), UCosmicEventSubsystem::Apply(Transfer(Civ1), Bench.Ctx));
		Bench.Ctx.SystemIndex = 1;
		TestTrue(TEXT("An unowned living system can be handed to a people"), UCosmicEventSubsystem::Apply(Transfer(Civ1), Bench.Ctx));
		TestEqual(TEXT("TransferSystem leaves no note"), Bench.Ctx.Notes.Num(), 0);
	}

	// A capital that changes hands: the discontent of the old owner stays behind and the capital moves.
	{
		FBench Bench(Laid, *Sim);
		FGalaxyState& State = Bench.State;
		const FName Civ3 = State.Civs[3].CivID;
		State.Systems[1].ActiveMegaproject = CosmicRows::ProjectSettle;
		PlaceByHand(State, 0, TEXT("Hazard.Rebellion"), 50);
		Bench.Ctx.CivIndex = 0;
		Bench.Ctx.SystemIndex = 0;
		TestTrue(TEXT("A capital can be handed over while its people keeps other systems"), UCosmicEventSubsystem::Apply(Transfer(Civ3), Bench.Ctx));
		TestFalse(TEXT("The discontent of the old owner does not follow its capital"), State.Systems[0].ActiveHazard.IsValid());
		TestEqual(TEXT("The capital moves to the first settled system"), State.Civs[0].HomeSystemID, State.Systems[2].SystemID);
		TestEqual(TEXT("Losing the capital costs Stability"), State.Civs[0].Stability, 0.75f - 0.3f);
		Bench.Ctx.SystemIndex = 2;
		TestTrue(TEXT("The new capital can be handed over too"), UCosmicEventSubsystem::Apply(Transfer(Civ3), Bench.Ctx));
		TestEqual(TEXT("Without a settled system the capital is the system being settled"), State.Civs[0].HomeSystemID, State.Systems[1].SystemID);
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicHazardLifecycleTest, "Cosmic_Ascension.Sim.HazardLifecycle", CosmicTests::Flags)
bool FCosmicHazardLifecycleTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHazardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	UDataTable* Events = NewTable(FCosmicEventDef::StaticStruct());
	FCosmicEventDef OutbreakRow;
	OutbreakRow.Category = Tag(TEXT("Event.Civ.Economy"));
	OutbreakRow.LogCategory = Tag(TEXT("Log.Civ"));
	OutbreakRow.Trigger = ECosmicEventTrigger::Named;
	OutbreakRow.Subject = ECosmicEventSubject::SettledSystem;
	OutbreakRow.BaseWeight = 0.f;
	OutbreakRow.CooldownTicks = 0;
	OutbreakRow.Conditions = { Cond(EConditionType::SystemHasHazard, TEXT("Hazard.Plague")) };
	OutbreakRow.Effects = { Eff(EEffectType::AddResource, nullptr, Energy, 1.f) };
	OutbreakRow.LogTemplates = { FText::AsCultureInvariant(TEXT("the outbreak row of {Hazard}")) };
	Events->AddRow(Outbreak, OutbreakRow);
	FCosmicEventDef CureRow = OutbreakRow;
	CureRow.Trigger = ECosmicEventTrigger::Standing;
	CureRow.Conditions = { Cond(EConditionType::SystemHasHazard, TEXT("Hazard.Plague")), Cond(EConditionType::CivHasTrait, TEXT("Trait.Faith.Steward")) };
	CureRow.Effects = { Eff(EEffectType::ResolveHazard, TEXT("Hazard.Plague")) };
	CureRow.LogTemplates = Lines(TEXT("the cure of {Hazard}"));
	Events->AddRow(Cure, CureRow);

	UDataTable* Hazards = NewTable(FHazardDef::StaticStruct());
	FHazardDef StormRow = HazardRow(TEXT("Hazard.Plague"), ECosmicHazardSpawn::Draw, 1.f, 3);
	StormRow.SpawnConditions = { Cond(EConditionType::CivHasTrait, TEXT("Trait.Faith.Seeking")) };
	StormRow.OnExpireEffects = { Eff(EEffectType::AddResource, nullptr, Causality, 10.f) };
	StormRow.OnExpireNeighbourEffects = { Eff(EEffectType::ChangeStability, nullptr, NAME_None, -0.1f) };
	StormRow.OutbreakRow = Outbreak;
	Hazards->AddRow(Storm, StormRow);
	FHazardDef UnrestRow = HazardRow(TEXT("Hazard.Rebellion"), ECosmicHazardSpawn::Capital, 0.f, 50);
	UnrestRow.SpawnConditions = { Cond(EConditionType::CivStabilityBelow, nullptr, nullptr, 0.3f) };
	Hazards->AddRow(Unrest, UnrestRow);
	// The vessel's archetype weighs the drawn hazard by 0.
	UDataTable* Civs = NewTable(FCivArchetypeDef::StaticStruct());
	for (const FName RowName : Original.GetCivRowNames())
	{
		FCivArchetypeDef Row = *Original.FindCiv(RowName);
		if (RowName == BaseRules.PlayerArchetypeRow)
		{
			Row.HazardSpawnMultipliers.Add(Tag(TEXT("Hazard.Plague")), 0.f);
		}
		Civs->AddRow(RowName, Row);
	}
	FCosmicDataTables Tables = TablesWith(Original, Knobs(BaseRules), Hazards, Events);
	Tables.Civs = Civs;
	Sim->SetTables(Tables);
	Sim->NewGalaxy(42, 12, 4);
	FGalaxyState& State = Sim->GetMutableState();
	if (!TestTrue(TEXT("The board of the test is laid"), Board(State)))
	{
		return false;
	}
	State.Civs[1].Traits.AddTag(Tag(TEXT("Trait.Faith.Seeking")));
	State.Civs[0].Traits.AddTag(Tag(TEXT("Trait.Faith.Seeking")));
	const FGameplayTag Plague = Tag(TEXT("Hazard.Plague"));

	// The draw lands where the conditions hold, with its whole count, and tells its outbreak before the outbreak row.
	const float EnergyBefore = State.Civs[1].Stockpile.Energy;
	Sim->StepOnce();
	const int64 Landed = State.Tick;
	TestTrue(TEXT("The drawn hazard landed where its conditions hold"), State.Systems[6].ActiveHazard == Plague);
	TestEqual(TEXT("It keeps its whole count in the tick it landed"), State.Systems[6].HazardTicksRemaining, 3);
	for (int32 SystemIndex : { 7, 11 })
	{
		TestFalse(*FString::Printf(TEXT("No hazard where its conditions fail (system %d)"), SystemIndex), State.Systems[SystemIndex].ActiveHazard.IsValid());
	}
	for (int32 SystemIndex : { 0, 1, 2 })
	{
		TestFalse(*FString::Printf(TEXT("No hazard where the archetype weighs it by 0 (system %d)"), SystemIndex), State.Systems[SystemIndex].ActiveHazard.IsValid());
	}
	const int32 OutbreakLine = FindLine(State, Storm, Landed, TEXT("outbreak"));
	const int32 RowLine = FindLine(State, Outbreak, Landed);
	if (TestTrue(TEXT("The outbreak is told"), OutbreakLine != INDEX_NONE) && TestTrue(TEXT("The outbreak row fired"), RowLine != INDEX_NONE))
	{
		TestTrue(TEXT("The outbreak line comes before the line of the outbreak row"), OutbreakLine < RowLine);
		TestEqual(TEXT("The outbreak line is about the system"), State.Log[OutbreakLine].FocusSystemID, State.Systems[6].SystemID);
		TestEqual(TEXT("The outbreak line is about its owner"), State.Log[OutbreakLine].FocusCivID, State.Civs[1].CivID);
		TestTrue(TEXT("The outbreak row reads the hazard in its {Hazard} slot"), State.Log[RowLine].Message.ToString().Contains(TEXT("<Hazard.Plague>")));
	}
	TestEqual(TEXT("The outbreak row applied its effect to the owner"), State.Civs[1].Stockpile.Energy - State.Civs[1].IncomePerTick.Energy, EnergyBefore + 1.f, 0.01f);

	// The expiry: its effects, the damage to a neighbour that has a people, its line, then the next draw.
	Sim->StepOnce();
	Sim->StepOnce();
	TestEqual(TEXT("The countdown went down one a tick"), State.Systems[6].HazardTicksRemaining, 1);
	Sim->StepOnce();
	const int64 Expired = State.Tick;
	TestEqual(TEXT("The expiry applied its effect to the owner"), State.Civs[1].Stockpile.Causality, 10.f);
	TestEqual(TEXT("The owner of a lane neighbour took the damage"), State.Civs[2].Stability, 0.75f + -0.1f);
	TestEqual(TEXT("A people that is no neighbour took nothing"), State.Civs[3].Stability, 0.75f);
	const int32 ExpiryLine = FindLine(State, Storm, Expired, TEXT("expiry"));
	const int32 NextOutbreak = FindLine(State, Storm, Expired, TEXT("outbreak"));
	if (TestTrue(TEXT("The expiry is told"), ExpiryLine != INDEX_NONE) && TestTrue(TEXT("The draw of the same tick placed the next one"), NextOutbreak != INDEX_NONE))
	{
		TestTrue(TEXT("The expiry comes before the spawn of the same tick"), ExpiryLine < NextOutbreak);
	}
	TestEqual(TEXT("The new hazard keeps its whole count"), State.Systems[6].HazardTicksRemaining, 3);

	// The branch takes the place of the effects.
	if (FHazardDef* Live = Hazards->FindRow<FHazardDef>(Storm, TEXT("HazardLifecycle")))
	{
		Live->BranchChance = 1.f;
		Live->OnExpireBranchEffects = { Eff(EEffectType::AddResource, nullptr, Causality, 100.f) };
	}
	Sim->RunHeadless(3);
	TestEqual(TEXT("A branch of chance 1 is applied instead of the effects"), State.Civs[1].Stockpile.Causality, 110.f);

	// A row that resolves: its line, then the hazard's resolution line, in the same tick.
	State.Civs[1].Traits.AddTag(Tag(TEXT("Trait.Faith.Steward")));
	int64 Cured = INDEX_NONE;
	for (int32 Step = 0; Step < 12 && Cured == INDEX_NONE; ++Step)
	{
		Sim->StepOnce();
		if (FindLine(State, Cure, State.Tick) != INDEX_NONE)
		{
			Cured = State.Tick;
		}
	}
	State.Civs[1].Traits.RemoveTag(Tag(TEXT("Trait.Faith.Steward")));
	if (TestTrue(TEXT("The cure fired in a slot"), Cured != INDEX_NONE))
	{
		const int32 CureLine = FindLine(State, Cure, Cured);
		const int32 Resolution = FindLine(State, Storm, Cured, TEXT("resolution"));
		if (TestTrue(TEXT("The resolution is told"), Resolution != INDEX_NONE))
		{
			TestTrue(TEXT("The row's line comes before the resolution it caused"), CureLine < Resolution);
		}
		TestTrue(TEXT("The row's {Hazard} is the hazard it resolved"), State.Log[CureLine].Message.ToString().Contains(TEXT("<Hazard.Plague>")));
		TestFalse(TEXT("The cure emptied the slot until the next draw"), State.Systems[6].ActiveHazard.IsValid());
	}

	// The capital rule: deterministic, on the capital alone.
	State.Civs[1].Traits.RemoveTag(Tag(TEXT("Trait.Faith.Seeking")));
	State.Civs[3].Stability = 0.1f;
	Sim->StepOnce();
	TestTrue(TEXT("The Capital row lands on the capital of the people whose conditions hold"), State.Systems[11].ActiveHazard == Tag(TEXT("Hazard.Rebellion")));
	TestFalse(TEXT("It does not land on another people's capital"), State.Systems[0].ActiveHazard.IsValid());

	// Two expiries in one tick, the first resolving the second through its neighbours: the second is passed by.
	{
		UDataTable* Pairing = NewTable(FHazardDef::StaticStruct());
		FHazardDef Mender = HazardRow(TEXT("Hazard.AIAwakening"), ECosmicHazardSpawn::Draw, 0.f, 1);
		Mender.OnExpireNeighbourEffects = { Eff(EEffectType::ResolveHazard) };
		Pairing->AddRow(Waking, Mender);
		FHazardDef Mended = HazardRow(TEXT("Hazard.Plague"), ECosmicHazardSpawn::Draw, 0.f, 1);
		Mended.OnExpireEffects = { Eff(EEffectType::AddResource, nullptr, Causality, 1000.f) };
		Pairing->AddRow(Storm, Mended);
		Sim->SetTables(TablesWith(Original, Knobs(BaseRules), Pairing, nullptr));
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& Paired = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the paired expiries is laid"), Board(Paired)))
		{
			return false;
		}
		PlaceByHand(Paired, 6, TEXT("Hazard.AIAwakening"), 1);
		PlaceByHand(Paired, 7, TEXT("Hazard.Plague"), 1);
		Sim->StepOnce();
		TestTrue(TEXT("The first expiry resolved its neighbour"), FindLine(Paired, Storm, Paired.Tick, TEXT("resolution")) != INDEX_NONE);
		TestEqual(TEXT("The resolved hazard does not expire as well"), CountLines(Paired, Storm, Paired.Tick, TEXT("expiry")), 0);
		TestEqual(TEXT("and applies none of its effects"), Paired.Civs[2].Stockpile.Causality, 0.f);
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicHazardDrawsTest, "Cosmic_Ascension.Sim.HazardDraws", CosmicTests::Flags)
bool FCosmicHazardDrawsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHazardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	UDataTable* Rows = NewTable(FHazardDef::StaticStruct());
	Rows->AddRow(Storm, HazardRow(TEXT("Hazard.Plague"), ECosmicHazardSpawn::Draw, 0.f, 3));
	FHazardDef UnrestRow = HazardRow(TEXT("Hazard.Rebellion"), ECosmicHazardSpawn::Capital, 0.f, 50);
	UnrestRow.SpawnConditions = { Cond(EConditionType::CivStabilityBelow, nullptr, nullptr, 0.f) };
	Rows->AddRow(Unrest, UnrestRow);
	Rows->AddRow(ClaimRow, HazardRow(TEXT("Hazard.Claim"), ECosmicHazardSpawn::ClaimRule, 0.f, 3));
	const FCosmicDataTables WithRows = TablesWith(Original, Knobs(BaseRules), Rows, nullptr);
	const FCosmicDataTables NoRows = TablesWith(Original, Knobs(BaseRules), nullptr, nullptr);

	Sim->SetTables(NoRows);
	Sim->NewGalaxy(42, 12, 4);
	if (!TestTrue(TEXT("The board of the test is laid"), Board(Sim->GetMutableState())))
	{
		return false;
	}
	Sim->SyncRngState();
	const FGalaxyState Start = Sim->GetState();
	auto DrawsOfTick = [Sim](const FGalaxyState& From, const FCosmicDataTables& Tables)
	{
		Sim->SetTables(Tables);
		Sim->LoadState(From);
		Sim->StepOnce();
		return DrawsBetween(From.RngState, Sim->GetState().RngState);
	};

	const int32 Base = DrawsOfTick(Start, NoRows);
	TestTrue(TEXT("A tick draws at least one number per settled system"), Base >= 5);
	TestEqual(TEXT("The rows of DT_Hazards move no draw"), DrawsOfTick(Start, WithRows), Base);
	FGalaxyState More = Start;
	CosmicTests::Give(More, 0, { 3 });
	TestEqual(TEXT("One more settled system, one more draw"), DrawsOfTick(More, NoRows), Base + 1);
	FGalaxyState Settling = Start;
	Settling.Systems[1].ActiveMegaproject = CosmicRows::ProjectSettle;
	TestEqual(TEXT("A system being settled draws nothing"), DrawsOfTick(Settling, NoRows), Base - 1);
	FGalaxyState Expiring = Start;
	PlaceByHand(Expiring, 1, TEXT("Hazard.Plague"), 1);
	PlaceByHand(Expiring, 6, TEXT("Hazard.Plague"), 1);
	TestEqual(TEXT("Two expiries draw two numbers each"), DrawsOfTick(Expiring, WithRows), Base + 4);
	TestEqual(TEXT("An expiry draws its numbers whatever the tables hold"), DrawsOfTick(Expiring, NoRows), Base + 4);
	FGalaxyState Waiting = Start;
	PlaceByHand(Waiting, 1, TEXT("Hazard.Plague"), 2);
	PlaceByHand(Waiting, 6, TEXT("Hazard.Plague"), 2);
	TestEqual(TEXT("A taken slot still draws, and a countdown that does not run out draws nothing"), DrawsOfTick(Waiting, WithRows), Base);

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicWildWorldsQuietTest, "Cosmic_Ascension.Sim.WildWorldsQuiet", CosmicTests::Flags)
bool FCosmicWildWorldsQuietTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHazardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	// A hazard on every settled system that can take one, whose expiry tries to spread to every lane neighbour (unowned
	// ones and the one being settled among them: the spread goes through SpawnHazard, a path outside the draw), and claims
	// that run out quickly.
	UDataTable* Hazards = NewTable(FHazardDef::StaticStruct());
	FHazardDef Spreading = HazardRow(TEXT("Hazard.Plague"), ECosmicHazardSpawn::Draw, 1.f, 4);
	Spreading.OnExpireNeighbourEffects = { Eff(EEffectType::SpawnHazard, TEXT("Hazard.AIAwakening")) };
	Hazards->AddRow(Storm, Spreading);
	Hazards->AddRow(Waking, HazardRow(TEXT("Hazard.AIAwakening"), ECosmicHazardSpawn::Draw, 0.f, 4));
	Hazards->AddRow(ClaimRow, HazardRow(TEXT("Hazard.Claim"), ECosmicHazardSpawn::ClaimRule, 0.f, 4));
	Sim->SetTables(TablesWith(Original, Knobs(BaseRules), Hazards, nullptr));
	Sim->NewGalaxy(42, 12, 4);
	FGalaxyState& State = Sim->GetMutableState();
	if (!TestTrue(TEXT("The board of the test is laid"), Board(State)))
	{
		return false;
	}
	// Civilization 0 settles 0 and 2 and is settling 1; a fallen core lies on the unowned system 3, next to 2.
	State.Systems[1].ActiveMegaproject = CosmicRows::ProjectSettle;
	AddInstance(State, Fallen, TEXT("Doctrine.Silence"), { 3 });

	bool bWildQuiet = true;
	bool bSettlingQuiet = true;
	bool bJoinsSettled = true;
	bool bAnyHazard = false;
	int32 Joins = 0;
	TArray<FName> Sphere = State.Transcendents[0].InfluencedSystems;
	for (int32 Step = 0; Step < 80; ++Step)
	{
		Sim->StepOnce();
		for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num(); ++SystemIndex)
		{
			const FStarSystemState& System = State.Systems[SystemIndex];
			if (System.OwnerCivID.IsNone())
			{
				bWildQuiet &= !System.ActiveHazard.IsValid();
			}
			else if (!CosmicState::IsSettled(System))
			{
				bSettlingQuiet &= !System.ActiveHazard.IsValid();
			}
			else
			{
				bAnyHazard |= System.ActiveHazard.IsValid();
			}
		}
		for (const FName Member : State.Transcendents[0].InfluencedSystems)
		{
			if (!Sphere.Contains(Member))
			{
				++Joins;
				const int32 Joined = Sim->GetStateIndex().FindSystem(State, Member);
				bJoinsSettled &= State.Systems.IsValidIndex(Joined) && CosmicState::IsSettled(State.Systems[Joined]);
			}
		}
		Sphere = State.Transcendents[0].InfluencedSystems;
	}
	TestTrue(TEXT("Hazards stood on settled systems"), bAnyHazard);
	TestTrue(TEXT("No unowned system ever held a hazard"), bWildQuiet);
	TestTrue(TEXT("No system being settled ever held a hazard"), bSettlingQuiet);
	TestTrue(TEXT("A claim joined the sphere"), Joins >= 1);
	TestTrue(TEXT("The sphere took only systems a people had settled"), bJoinsSettled);
	TestTrue(TEXT("The core stays where it began"), Sphere.Contains(State.Systems[3].SystemID));

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicDeadNodeTest, "Cosmic_Ascension.Sim.DeadNode", CosmicTests::Flags)
bool FCosmicDeadNodeTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHazardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	UDataTable* Hazards = NewTable(FHazardDef::StaticStruct());
	FHazardDef NovaRow = HazardRow(TEXT("Hazard.PreSupernova"), ECosmicHazardSpawn::Draw, 0.f, 1);
	NovaRow.OnExpireNeighbourEffects = { Eff(EEffectType::ChangeStability, nullptr, NAME_None, -0.1f) };
	Hazards->AddRow(Nova, NovaRow);
	UDataTable* Events = NewTable(FCosmicEventDef::StaticStruct());
	if (!TestTrue(TEXT("The game has the grace row of the remnant"), CopyEvent(Original, Events, CosmicRows::EventGraceRemnant)))
	{
		return false;
	}
	Sim->SetTables(TablesWith(Original, Knobs(BaseRules), Hazards, Events));
	const FGameplayTag Spent = Tag(TEXT("Flag.Grace.Spent"));

	// A capital that is not the last system, inside a sphere, with the ruling vessel standing in it.
	{
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the capital case is laid"), Board(State)))
		{
			return false;
		}
		AddInstance(State, Fallen, TEXT("Doctrine.Silence"), { 0, 3 });
		State.Systems[1].ActiveMegaproject = CosmicRows::ProjectSettle;
		State.Systems[0].ActiveMegaproject = FName(TEXT("project_test"));
		State.Systems[0].MegaprojectProgress = 0.4f;
		State.Systems[0].MegaprojectCompletedTick = 7;
		PlaceByHand(State, 0, TEXT("Hazard.PreSupernova"), 1);
		const int32 SystemCount = State.Systems.Num();
		const FName Sol = State.Systems[0].SystemID;
		Sim->StepOnce();
		const FStarSystemState& Dead = State.Systems[0];
		TestEqual(TEXT("No system is removed"), State.Systems.Num(), SystemCount);
		TestTrue(TEXT("The destroyed system has no owner"), Dead.OwnerCivID.IsNone());
		TestEqual(TEXT("The destroyed system has no lane"), Dead.Lanes.Num(), 0);
		TestTrue(TEXT("The destroyed system is a dead node"), CosmicHazards::IsDeadNode(Dead));
		TestFalse(TEXT("The destroyed system holds no hazard"), Dead.ActiveHazard.IsValid());
		TestEqual(TEXT("Its megaproject ended"), Dead.ActiveMegaproject, FName());
		TestEqual(TEXT("Its progress is gone"), Dead.MegaprojectProgress, 0.f);
		TestEqual(TEXT("A completion stays on the record"), Dead.MegaprojectCompletedTick, static_cast<int64>(7));
		TestFalse(TEXT("No lane leads to it from its neighbour"), State.Systems[1].Lanes.Contains(Sol));
		TestFalse(TEXT("It left the sphere"), State.Transcendents[0].InfluencedSystems.Contains(Sol));
		TestTrue(TEXT("The rest of the sphere stays"), State.Transcendents[0].InfluencedSystems.Contains(State.Systems[3].SystemID));
		TestFalse(TEXT("It left its people's systems"), State.Civs[0].OwnedSystems.Contains(Sol));
		TestEqual(TEXT("The capital moved to the first settled system left"), State.Civs[0].HomeSystemID, State.Systems[2].SystemID);
		TestEqual(TEXT("The people lost Stability to the neighbour damage and to its capital"), State.Civs[0].Stability, FMath::Max(0.f, FMath::Clamp(0.75f + -0.1f, 0.f, 1.f) - 0.3f));
		TestEqual(TEXT("The ruling vessel was carried to the new capital, not to the first neighbour"), State.Player.CurrentSystemID, State.Systems[2].SystemID);
		TestFalse(TEXT("Losing a capital records no role effect"), State.Calling.PendingEffect.IsValid());
		TestTrue(TEXT("The expiry is told about the destroyed system"), FindLine(State, Nova, State.Tick, TEXT("expiry")) != INDEX_NONE);
	}

	// A wanderer standing there is carried to the first living lane neighbour, unless its death is already recorded.
	{
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the wanderer case is laid"), Board(State)))
		{
			return false;
		}
		State.Player.Role = EPlayerRole::Wanderer;
		State.Player.RuledCivID = NAME_None;
		State.Civs[0].bIsPlayerRuled = false;
		State.Player.CurrentSystemID = State.Systems[1].SystemID;
		PlaceByHand(State, 1, TEXT("Hazard.PreSupernova"), 1);
		Sim->StepOnce();
		TestEqual(TEXT("The wanderer is carried to the first lane neighbour"), State.Player.CurrentSystemID, State.Systems[0].SystemID);

		State.Player.CurrentSystemID = State.Systems[2].SystemID;
		State.Calling.PendingEffect = Tag(TEXT("Effect.KillPlayer"));
		PlaceByHand(State, 2, TEXT("Hazard.PreSupernova"), 1);
		Sim->StepOnce();
		TestEqual(TEXT("A vessel whose death is recorded is not carried"), State.Player.CurrentSystemID, State.Systems[2].SystemID);
		State.Calling.PendingEffect = FGameplayTag();

		// No living neighbour: it stays. System 11 is the last system of its people, so the run's grace is spent first.
		State.Flags.AddTag(Spent);
		State.Systems[10].StarClass = FGameplayTag();
		State.Player.CurrentSystemID = State.Systems[11].SystemID;
		PlaceByHand(State, 11, TEXT("Hazard.PreSupernova"), 1);
		Sim->StepOnce();
		TestTrue(TEXT("The last system went"), CosmicHazards::IsDeadNode(State.Systems[11]));
		TestEqual(TEXT("A vessel with no living neighbour stays where it stood"), State.Player.CurrentSystemID, State.Systems[11].SystemID);
		TestEqual(TEXT("A people that lost its last system keeps the ID of its capital"), State.Civs[3].HomeSystemID, State.Systems[11].SystemID);
		TestEqual(TEXT("and its Stability"), State.Civs[3].Stability, 0.75f);
	}

	// The collapse of the people the vessel rules: the deposition pipeline, at the tick boundary.
	{
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the collapse case is laid"), Board(State)))
		{
			return false;
		}
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { 0 });
		CosmicTests::Give(State, 1, { 6 });
		State.Flags.AddTag(Spent);
		PlaceByHand(State, 0, TEXT("Hazard.PreSupernova"), 1);
		Sim->StepOnce();
		TestTrue(TEXT("The last system went"), CosmicHazards::IsDeadNode(State.Systems[0]));
		TestEqual(TEXT("The people has no system left"), State.Civs[0].OwnedSystems.Num(), 0);
		TestTrue(TEXT("The ruler fell: the vessel walks"), State.Player.Role == EPlayerRole::Wanderer);
		TestTrue(TEXT("as a fallen monarch"), State.Player.StatusTags.HasTagExact(Tag(TEXT("Status.FallenMonarch"))));
		TestFalse(TEXT("The role effect was resolved at the tick boundary"), State.Calling.PendingEffect.IsValid());
		TestTrue(TEXT("The fallen monarch is told"), FindLine(State, CosmicRows::LogFallenMonarch, State.Tick) != INDEX_NONE);
		TestEqual(TEXT("The vessel was carried out of the dead system"), State.Player.CurrentSystemID, State.Systems[1].SystemID);
		const FLifeRecord* Life = State.Calling.Lives.IsEmpty() ? nullptr : &State.Calling.Lives.Last();
		TestTrue(TEXT("The reign ended as a deposition"), Life && !Life->ReignEndReasons.IsEmpty() && Life->ReignEndReasons.Last() == Tag(TEXT("Reign.Deposed")));
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicGraceRemnantAtLossTest, "Cosmic_Ascension.Sim.GraceRemnantAtLoss", CosmicTests::Flags)
bool FCosmicGraceRemnantAtLossTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHazardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	UDataTable* Hazards = NewTable(FHazardDef::StaticStruct());
	FHazardDef NovaRow = HazardRow(TEXT("Hazard.PreSupernova"), ECosmicHazardSpawn::Draw, 0.f, 1);
	NovaRow.OnExpireEffects = { Eff(EEffectType::AddResource, nullptr, Causality, 5.f) };
	Hazards->AddRow(Nova, NovaRow);
	UDataTable* Events = NewTable(FCosmicEventDef::StaticStruct());
	if (!TestTrue(TEXT("The game has the grace row of the remnant"), CopyEvent(Original, Events, CosmicRows::EventGraceRemnant)))
	{
		return false;
	}
	Sim->SetTables(TablesWith(Original, Knobs(BaseRules), Hazards, Events));
	const FGameplayTag Spent = Tag(TEXT("Flag.Grace.Spent"));
	const FGameplayTag Remnant = Tag(TEXT("Trait.Faith.Remnant"));

	Sim->NewGalaxy(42, 12, 4);
	FGalaxyState& State = Sim->GetMutableState();
	if (!TestTrue(TEXT("The board of the test is laid"), Board(State)))
	{
		return false;
	}

	// The first people about to lose its last system is kept, and the loss does not happen at all.
	PlaceByHand(State, 11, TEXT("Hazard.PreSupernova"), 1);
	Sim->StepOnce();
	const int64 Kept = State.Tick;
	TestFalse(TEXT("The kept system is no dead node"), CosmicHazards::IsDeadNode(State.Systems[11]));
	TestEqual(TEXT("The kept system keeps its people"), State.Systems[11].OwnerCivID, State.Civs[3].CivID);
	TestTrue(TEXT("The kept system keeps its lanes"), State.Systems[11].Lanes.Num() > 0);
	TestFalse(TEXT("The kept system's slot is empty"), State.Systems[11].ActiveHazard.IsValid());
	TestTrue(TEXT("The grace of the run is spent"), State.Flags.HasTagExact(Spent));
	TestTrue(TEXT("The kept people carries the remnant"), State.Civs[3].Traits.HasTagExact(Remnant));
	TestEqual(TEXT("The expiry's effects were not applied"), State.Civs[3].Stockpile.Causality, 0.f);
	TestEqual(TEXT("No expiry line is told for a kept system"), CountLines(State, Nova, Kept), 0);
	const int32 GraceLine = FindLine(State, CosmicRows::EventGraceRemnant, Kept);
	if (TestTrue(TEXT("The grace is told"), GraceLine != INDEX_NONE))
	{
		TestEqual(TEXT("as a mythic line"), static_cast<int32>(State.Log[GraceLine].Severity), 2);
		TestEqual(TEXT("about the kept system"), State.Log[GraceLine].FocusSystemID, State.Systems[11].SystemID);
		TestEqual(TEXT("and its people"), State.Log[GraceLine].FocusCivID, State.Civs[3].CivID);
	}

	// Once in a run: the next people about to lose its last system loses it.
	PlaceByHand(State, 7, TEXT("Hazard.PreSupernova"), 1);
	Sim->StepOnce();
	TestTrue(TEXT("After the grace a last system goes"), CosmicHazards::IsDeadNode(State.Systems[7]));
	TestEqual(TEXT("The second people has no system left"), State.Civs[2].OwnedSystems.Num(), 0);
	TestFalse(TEXT("The second people carries no remnant"), State.Civs[2].Traits.HasTagExact(Remnant));
	TestEqual(TEXT("The grace was told once"), CountLines(State, CosmicRows::EventGraceRemnant), 1);

	// A system that is not its people's last asks no gate: the grace is not spent on it.
	Sim->NewGalaxy(42, 12, 4);
	if (!TestTrue(TEXT("The board of the second case is laid"), Board(State)))
	{
		return false;
	}
	PlaceByHand(State, 1, TEXT("Hazard.PreSupernova"), 1);
	Sim->StepOnce();
	TestTrue(TEXT("A system that is not the last goes"), CosmicHazards::IsDeadNode(State.Systems[1]));
	TestFalse(TEXT("and the grace of the run is not spent on it"), State.Flags.HasTagExact(Spent));

	// The vessel's people is kept like any other, and its ruler stays.
	CosmicTests::ClearOwners(State);
	CosmicTests::Give(State, 0, { 0 });
	CosmicTests::Give(State, 1, { 6 });
	PlaceByHand(State, 0, TEXT("Hazard.PreSupernova"), 1);
	Sim->StepOnce();
	TestFalse(TEXT("The vessel's last system is kept"), CosmicHazards::IsDeadNode(State.Systems[0]));
	TestTrue(TEXT("Its ruler still rules"), State.Player.Role == EPlayerRole::Ruler);
	TestFalse(TEXT("No role effect is recorded for a kept system"), State.Calling.PendingEffect.IsValid());

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

namespace CosmicHazardTests
{
	// One expiry on system 2 of the board (civilization 0 holds 0, 1 and 2), the transcendents laid by Lay, then the tick.
	// The test's claim runs out two ticks after it is placed.
	FGalaxyState& ExpireNextToSpheres(UGalaxySimulationSubsystem& Sim, TFunctionRef<void(FGalaxyState&)> Lay)
	{
		Sim.NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim.GetMutableState();
		Board(State);
		Lay(State);
		PlaceByHand(State, 2, TEXT("Hazard.Plague"), 1);
		Sim.StepOnce();
		return State;
	}

	UDataTable* ClaimHazards()
	{
		UDataTable* Hazards = NewTable(FHazardDef::StaticStruct());
		Hazards->AddRow(Spark, HazardRow(TEXT("Hazard.Plague"), ECosmicHazardSpawn::Draw, 0.f, 1));
		Hazards->AddRow(ClaimRow, HazardRow(TEXT("Hazard.Claim"), ECosmicHazardSpawn::ClaimRule, 0.f, 2));
		return Hazards;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicClaimRuleTest, "Cosmic_Ascension.Sim.ClaimRule", CosmicTests::Flags)
bool FCosmicClaimRuleTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHazardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };
	Sim->SetTables(TablesWith(Original, Knobs(BaseRules), ClaimHazards(), nullptr));
	const FGameplayTag Claim = Tag(TEXT("Hazard.Claim"));

	// The claimant: the largest term of pressure among the fallen that reach the system and do not hold it yet.
	{
		FGalaxyState& State = ExpireNextToSpheres(*Sim, [](FGalaxyState& S)
		{
			AddInstance(S, Fallen, TEXT("Doctrine.Silence"), { 3 });
			AddInstance(S, Zealous, TEXT("Doctrine.Zeal"), { 3 });
		});
		TestTrue(TEXT("An expiry next to a fallen sphere places a claim"), State.Systems[2].ActiveHazard == Claim);
		TestEqual(TEXT("The claim counts down from its row"), State.Systems[2].HazardTicksRemaining, 2);
		const int32 ExpiryLine = FindLine(State, Spark, State.Tick, TEXT("expiry"));
		const int32 ClaimLine = FindLine(State, ClaimRow, State.Tick, TEXT("outbreak"));
		if (TestTrue(TEXT("The expiry is told"), ExpiryLine != INDEX_NONE) && TestTrue(TEXT("The claim is told"), ClaimLine != INDEX_NONE))
		{
			TestTrue(TEXT("The claim is told after the expiry it follows"), ExpiryLine < ClaimLine);
		}
		Sim->RunHeadless(2);
		TestTrue(TEXT("The deeper fallen joins the system to its sphere"), InSphere(State, 1, 2));
		TestFalse(TEXT("The shallower does not"), InSphere(State, 0, 2));
		TestTrue(TEXT("The join is told"), FindLine(State, ClaimRow, State.Tick, TEXT("expiry")) != INDEX_NONE);
	}
	{
		FGalaxyState& State = ExpireNextToSpheres(*Sim, [](FGalaxyState& S)
		{
			AddInstance(S, Other, TEXT("Doctrine.Assimilation"), { 3 });
			AddInstance(S, Fallen, TEXT("Doctrine.Silence"), { 3 });
		});
		Sim->RunHeadless(2);
		TestTrue(TEXT("Equal terms go to the earlier instance"), InSphere(State, 0, 2));
		TestFalse(TEXT("The later instance of an equal term does not claim"), InSphere(State, 1, 2));
	}
	{
		FGalaxyState& State = ExpireNextToSpheres(*Sim, [](FGalaxyState& S)
		{
			AddInstance(S, Fallen, TEXT("Doctrine.Silence"), { 3 });
			AddInstance(S, Zealous, TEXT("Doctrine.Zeal"), { 3 });
			S.Transcendents[1].bSealed = true;
		});
		Sim->RunHeadless(2);
		TestTrue(TEXT("A sealed fallen does not claim: the unsealed one does"), InSphere(State, 0, 2));
		TestFalse(TEXT("The sealed sphere does not grow"), InSphere(State, 1, 2));
	}
	{
		FGalaxyState& State = ExpireNextToSpheres(*Sim, [](FGalaxyState& S)
		{
			AddInstance(S, Fallen, TEXT("Doctrine.Silence"), { 2, 3 });
		});
		TestFalse(TEXT("A fallen does not claim what its sphere holds"), State.Systems[2].ActiveHazard.IsValid());
	}
	{
		FGalaxyState& State = ExpireNextToSpheres(*Sim, [](FGalaxyState& S)
		{
			AddInstance(S, Fallen, TEXT("Doctrine.Silence"), { 2, 3 });
			AddInstance(S, Zealous, TEXT("Doctrine.Zeal"), { 3 });
		});
		TestTrue(TEXT("Another fallen claims a system in someone else's sphere"), State.Systems[2].ActiveHazard == Claim);
	}

	// Law.Remnant: never the last settled system of a people outside every fallen sphere.
	{
		FGalaxyState& State = ExpireNextToSpheres(*Sim, [](FGalaxyState& S)
		{
			AddInstance(S, Fallen, TEXT("Doctrine.Silence"), { 3, 0, 1 });
		});
		TestFalse(TEXT("No claim is placed on a people's last unclaimed settled system"), State.Systems[2].ActiveHazard.IsValid());
	}
	{
		FGalaxyState& State = ExpireNextToSpheres(*Sim, [](FGalaxyState& S)
		{
			AddInstance(S, Fallen, TEXT("Doctrine.Silence"), { 3 });
		});
		if (TestTrue(TEXT("The claim of the late case is placed"), State.Systems[2].ActiveHazard == Claim))
		{
			State.Transcendents[0].InfluencedSystems.Add(State.Systems[0].SystemID);
			State.Transcendents[0].InfluencedSystems.Add(State.Systems[1].SystemID);
			Sim->RunHeadless(2);
			TestFalse(TEXT("A claim does not join what became the last unclaimed settled system"), InSphere(State, 0, 2));
			TestTrue(TEXT("A claim that joins nothing is told as stilled"), FindLine(State, ClaimRow, State.Tick, TEXT("resolution")) != INDEX_NONE);
			TestEqual(TEXT("and not as joined"), CountLines(State, ClaimRow, State.Tick, TEXT("expiry")), 0);
		}
	}

	// The cap of a sphere, at the placement and at the join.
	{
		FGalaxyState& State = ExpireNextToSpheres(*Sim, [](FGalaxyState& S)
		{
			AddInstance(S, Pair, TEXT("Doctrine.Silence"), { 3 });
		});
		if (TestTrue(TEXT("The claim of a sphere with room is placed"), State.Systems[2].ActiveHazard == Claim))
		{
			State.Transcendents[0].InfluencedSystems.Add(State.Systems[8].SystemID);
			Sim->RunHeadless(2);
			TestFalse(TEXT("A claim does not join a sphere that filled up meanwhile"), InSphere(State, 0, 2));
		}
	}
	{
		FGalaxyState& State = ExpireNextToSpheres(*Sim, [](FGalaxyState& S)
		{
			AddInstance(S, Small, TEXT("Doctrine.Recurrence"), { 3 });
		});
		TestFalse(TEXT("A full sphere places no claim"), State.Systems[2].ActiveHazard.IsValid());
	}

	// Two lanes out with the knob at 2; one lane out with the knob at 1.
	{
		FGalaxyState& Near = ExpireNextToSpheres(*Sim, [](FGalaxyState& S)
		{
			AddInstance(S, Fallen, TEXT("Doctrine.Silence"), { 4 });
		});
		TestFalse(TEXT("A sphere two lanes off does not claim with the knob at 1"), Near.Systems[2].ActiveHazard.IsValid());
		FCosmicRulesDef Wide = Knobs(BaseRules);
		Wide.ClaimAdjacencyLanes = 2;
		Sim->SetTables(TablesWith(Original, Wide, ClaimHazards(), nullptr));
		FGalaxyState& Far = ExpireNextToSpheres(*Sim, [](FGalaxyState& S)
		{
			AddInstance(S, Fallen, TEXT("Doctrine.Silence"), { 4 });
		});
		TestTrue(TEXT("A sphere two lanes off claims with the knob at 2"), Far.Systems[2].ActiveHazard == Claim);
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicNoClaimsFromCelestialTest, "Cosmic_Ascension.Sim.NoClaimsFromCelestial", CosmicTests::Flags)
bool FCosmicNoClaimsFromCelestialTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHazardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	// Expiries every other tick on every settled system, next to a celestial or an independent sphere with room to grow:
	// one sphere at a time, so only its allegiance keeps it from claiming.
	UDataTable* Hazards = NewTable(FHazardDef::StaticStruct());
	Hazards->AddRow(Spark, HazardRow(TEXT("Hazard.Plague"), ECosmicHazardSpawn::Draw, 1.f, 2));
	Hazards->AddRow(ClaimRow, HazardRow(TEXT("Hazard.Claim"), ECosmicHazardSpawn::ClaimRule, 0.f, 2));
	Sim->SetTables(TablesWith(Original, Knobs(BaseRules), Hazards, nullptr));
	for (const FName Row : { Heaven, Apart })
	{
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the test is laid"), Board(State)))
		{
			return false;
		}
		AddInstance(State, Row, Row == Heaven ? TEXT("Doctrine.Covenant") : TEXT("Doctrine.Stillness"), { 3 });
		bool bNoClaim = true;
		int32 Expiries = 0;
		for (int32 Step = 0; Step < 60; ++Step)
		{
			Sim->StepOnce();
			Expiries += CountLines(State, Spark, State.Tick, TEXT("expiry"));
			for (const FStarSystemState& System : State.Systems)
			{
				bNoClaim &= System.ActiveHazard != Tag(TEXT("Hazard.Claim"));
			}
		}
		const FString Who = Row == Heaven ? TEXT("celestial") : TEXT("independent");
		TestTrue(*FString::Printf(TEXT("Hazards ran out next to the %s sphere"), *Who), Expiries > 0);
		TestTrue(*FString::Printf(TEXT("The %s sphere never claims"), *Who), bNoClaim);
		TestEqual(*FString::Printf(TEXT("The %s sphere does not grow"), *Who), State.Transcendents[0].InfluencedSystems.Num(), 1);
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

namespace CosmicHazardTests
{
	// A board for long runs of claims: civilization 0 holds 0 to 3, civilization 1 holds 5 to 7, civilization 3 holds 10
	// and 11; a fallen core on 4 touches 3, 5 and 10. Hazards land often and run out quickly.
	void LayClaimField(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Original, const FCosmicRulesDef& BaseRules)
	{
		UDataTable* Hazards = NewTable(FHazardDef::StaticStruct());
		Hazards->AddRow(Spark, HazardRow(TEXT("Hazard.Plague"), ECosmicHazardSpawn::Draw, 0.05f, 5));
		Hazards->AddRow(ClaimRow, HazardRow(TEXT("Hazard.Claim"), ECosmicHazardSpawn::ClaimRule, 0.f, 10));
		Sim.SetTables(TablesWith(Original, Knobs(BaseRules), Hazards, nullptr));
		Sim.NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim.GetMutableState();
		Board(State);
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { 0, 1, 2, 3 });
		CosmicTests::Give(State, 1, { 5, 6, 7 });
		CosmicTests::Give(State, 2, { 9 });
		CosmicTests::Give(State, 3, { 10, 11 });
		AddInstance(State, Fallen, TEXT("Doctrine.Silence"), { 4 });
	}

	// For every people with a settled system: one of them lies in no fallen sphere.
	bool NoPeopleWhollyClaimed(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables)
	{
		for (const FCivilizationState& Civ : State.Civs)
		{
			bool bAnySettled = false;
			bool bAnyFree = false;
			for (const FName SystemID : Civ.OwnedSystems)
			{
				const int32 SystemIndex = Index.FindSystem(State, SystemID);
				if (!State.Systems.IsValidIndex(SystemIndex) || !CosmicState::IsSettled(State.Systems[SystemIndex]))
				{
					continue;
				}
				bAnySettled = true;
				bool bClaimed = false;
				for (const FTranscendentState& Transcendent : State.Transcendents)
				{
					const FTranscendentDef* Def = Tables.FindTranscendent(Transcendent.TranscendentID);
					bClaimed |= Def && Def->Allegiance == Tag(TEXT("Realm.Abyssal")) && Transcendent.InfluencedSystems.Contains(SystemID);
				}
				bAnyFree |= !bClaimed;
			}
			if (bAnySettled && !bAnyFree)
			{
				return false;
			}
		}
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicClaimNeverTotalTest, "Cosmic_Ascension.Sim.ClaimNeverTotal", CosmicTests::Flags)
bool FCosmicClaimNeverTotalTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHazardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };
	LayClaimField(*Sim, Original, BaseRules);
	FGalaxyState& State = Sim->GetMutableState();

	bool bNeverTotal = true;
	for (int32 Step = 0; Step < 3000; ++Step)
	{
		Sim->StepOnce();
		bNeverTotal &= NoPeopleWhollyClaimed(State, Sim->GetStateIndex(), Sim->GetTables());
	}
	// Civilization 0 had four settled systems, the third of them next to the core: claims walked up its chain.
	int32 Claimed = 0;
	for (int32 SystemIndex : { 0, 1, 2, 3 })
	{
		Claimed += InSphere(State, 0, SystemIndex) ? 1 : 0;
	}
	TestTrue(TEXT("Claims joined the sphere"), Claimed >= 2);
	TestEqual(TEXT("A people that lost all but one settled system keeps the last"), Claimed, 3);
	TestTrue(TEXT("No people ever had every settled system in a fallen sphere"), bNeverTotal);

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSphereGrowthResponsiveTest, "Cosmic_Ascension.Sim.SphereGrowthResponsive", CosmicTests::Flags)
bool FCosmicSphereGrowthResponsiveTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHazardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };
	UCosmicEventSubsystem* Events = Sim->GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	if (!TestNotNull(TEXT("The event subsystem"), Events))
	{
		return false;
	}

	// The sphere's growth over a stretch, left alone or with every hazard within a lane of the sphere resolved before
	// each tick, the way a ruler who answers every claim would.
	auto Growth = [&](bool bResolve)
	{
		LayClaimField(*Sim, Original, BaseRules);
		FGalaxyState& State = Sim->GetMutableState();
		const int32 Before = State.Transcendents[0].InfluencedSystems.Num();
		for (int32 Step = 0; Step < 1500; ++Step)
		{
			if (bResolve)
			{
				for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num(); ++SystemIndex)
				{
					if (!State.Systems[SystemIndex].ActiveHazard.IsValid()
						|| UGalaxySimulationSubsystem::SphereReachIn(State, State.Transcendents[0], SystemIndex) == ECosmicSphereReach::None)
					{
						continue;
					}
					FCosmicEvalContext Ctx = Sim->MakeEvalContext();
					Ctx.SystemIndex = SystemIndex;
					Ctx.CivIndex = Sim->GetStateIndex().FindCiv(State, State.Systems[SystemIndex].OwnerCivID);
					UCosmicEventSubsystem::Apply(Eff(EEffectType::ResolveHazard), Ctx);
					Events->FollowUpNotes(Ctx);
				}
				Sim->SyncRngState();
			}
			Sim->StepOnce();
		}
		return State.Transcendents[0].InfluencedSystems.Num() - Before;
	};
	const int32 LeftAlone = Growth(false);
	const int32 Answered = Growth(true);
	TestTrue(TEXT("Left alone the sphere grows"), LeftAlone > 0);
	TestTrue(TEXT("Answering the hazards next to the sphere at least halves its growth"), Answered * 2 <= LeftAlone);

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicRevoltTelegraphTest, "Cosmic_Ascension.Sim.RevoltTelegraph", CosmicTests::Flags)
bool FCosmicRevoltTelegraphTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHazardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	// The game's rebellion, its two precursors and nothing else of the game that could land on the capital.
	const FName GameRebellion(TEXT("crisis_rebellion"));
	const FName Murmur(TEXT("event_rebellion_murmur"));
	const FName Rises(TEXT("event_rebellion_rises"));
	UDataTable* Hazards = NewTable(FHazardDef::StaticStruct());
	const FHazardDef* Rebellion = Original.FindHazard(GameRebellion);
	if (!TestNotNull(TEXT("The game has the rebellion row"), Rebellion))
	{
		return false;
	}
	Hazards->AddRow(GameRebellion, *Rebellion);
	UDataTable* Events = NewTable(FCosmicEventDef::StaticStruct());
	if (!TestTrue(TEXT("The game has the murmur"), CopyEvent(Original, Events, Murmur)) || !TestTrue(TEXT("The game has the rising"), CopyEvent(Original, Events, Rises)))
	{
		return false;
	}
	FCosmicRulesDef Rules = Knobs(BaseRules);
	Rules.EventSlotTicks = BaseRules.EventSlotTicks;
	Sim->SetTables(TablesWith(Original, Rules, Hazards, Events));

	auto Lay = [Sim]() -> FGalaxyState*
	{
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		return Board(State) ? &State : nullptr;
	};

	// After the birth protection: the rebellion stands in the tick the discontent reaches 100 ticks, told by two murmurs
	// before it and the rising in its tick.
	{
		FGalaxyState* State = Lay();
		if (!TestNotNull(TEXT("The board of the test is laid"), State))
		{
			return false;
		}
		Sim->RunHeadless(300);
		const int64 Start = State->Tick;
		int64 RoseAt = INDEX_NONE;
		for (int32 Step = 0; Step < 200 && RoseAt == INDEX_NONE; ++Step)
		{
			State->Civs[1].Stability = 0.2f;
			Sim->StepOnce();
			if (State->Systems[6].ActiveHazard == Tag(TEXT("Hazard.Rebellion")))
			{
				RoseAt = State->Tick;
			}
		}
		if (TestTrue(TEXT("The rebellion stood"), RoseAt != INDEX_NONE))
		{
			TestEqual(TEXT("It stood in the tick the discontent reached 100 ticks"), RoseAt, Start + 100);
			int32 Murmurs = 0;
			for (const FCosmicLogEntry& Entry : State->Log)
			{
				Murmurs += (Entry.SourceRow == Murmur && Entry.FocusCivID == State->Civs[1].CivID && Entry.Tick < RoseAt && Entry.Tick > RoseAt - 160) ? 1 : 0;
			}
			TestTrue(TEXT("Two murmurs or more came before it"), Murmurs >= 2);
			TestTrue(TEXT("The rising is told in its tick"), FindLine(*State, Rises, RoseAt) != INDEX_NONE);
			const FNotableRecord* Rebel = State->Civs[1].Notables.FindByPredicate([](const FNotableRecord& Record) { return Record.Role == Tag(TEXT("Notable.Rebel")); });
			if (TestNotNull(TEXT("The rising made a rebel"), Rebel))
			{
				TestEqual(TEXT("The rebel appears in the tick of the rising"), Rebel->FirstTick, RoseAt);
				TestEqual(TEXT("The rebel comes from the rising row"), Rebel->SourceRow, Rises);
			}
			TestFalse(TEXT("No rebellion stands in another people's capital"), State->Systems[11].ActiveHazard.IsValid());
		}
	}

	// Inside the birth protection the discontent waits for tick 300.
	{
		FGalaxyState* State = Lay();
		if (!TestNotNull(TEXT("The board of the early case is laid"), State))
		{
			return false;
		}
		int64 RoseAt = INDEX_NONE;
		for (int32 Step = 0; Step < 320 && RoseAt == INDEX_NONE; ++Step)
		{
			State->Civs[1].Stability = 0.2f;
			Sim->StepOnce();
			if (State->Systems[6].ActiveHazard.IsValid())
			{
				RoseAt = State->Tick;
			}
		}
		TestEqual(TEXT("A rebellion never stands before tick 300"), RoseAt, static_cast<int64>(300));
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicRebellionThroneTest, "Cosmic_Ascension.Sim.RebellionThrone", CosmicTests::Flags)
bool FCosmicRebellionThroneTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHazardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	const FName Rises(TEXT("event_rebellion_rises"));
	UDataTable* Hazards = NewTable(FHazardDef::StaticStruct());
	FHazardDef UnrestRow = HazardRow(TEXT("Hazard.Rebellion"), ECosmicHazardSpawn::Capital, 0.f, 3);
	UnrestRow.SpawnConditions = { Cond(EConditionType::CivStabilityBelowFor, nullptr, TEXT("100"), 0.3f) };
	UnrestRow.OnExpireEffects = { Eff(EEffectType::DeposeRuler, TEXT("Reign.Deposed")), Eff(EEffectType::ShiftEthics, nullptr, FName(TEXT("Openness")), 0.1f) };
	UnrestRow.OutbreakRow = Rises;
	Hazards->AddRow(Unrest, UnrestRow);
	UDataTable* Events = NewTable(FCosmicEventDef::StaticStruct());
	if (!TestTrue(TEXT("The game has the rising"), CopyEvent(Original, Events, Rises)))
	{
		return false;
	}
	Sim->SetTables(TablesWith(Original, Knobs(BaseRules), Hazards, Events));
	const FGameplayTag RulerTag = Tag(TEXT("Notable.Ruler"));
	const FGameplayTag RebelTag = Tag(TEXT("Notable.Rebel"));
	const FGameplayTag RebellionTag = Tag(TEXT("Hazard.Rebellion"));

	// Discontent that has lasted: the rebellion stands at the next tick.
	auto Discontent = [](FCivilizationState& Civ)
	{
		Civ.Stability = 0.1f;
		Civ.LowStabilityTicks = 150;
	};

	// An AI people: the throne changes.
	{
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the AI case is laid"), Board(State)))
		{
			return false;
		}
		State.Tick = 400;
		FNotableRecord& OldRuler = State.Civs[1].Notables.AddDefaulted_GetRef();
		OldRuler.Role = RulerTag;
		OldRuler.FirstTick = 1;
		OldRuler.SourceRow = FName(TEXT("event_test_old"));
		Discontent(State.Civs[1]);
		State.Civs[1].AIProfile = FGameplayTag();
		const float OpennessBefore = State.Civs[1].Ethics.Openness;
		Sim->StepOnce();
		const int64 Stood = State.Tick;
		TestTrue(TEXT("The rebellion stood"), State.Systems[6].ActiveHazard == RebellionTag);
		Sim->RunHeadless(2);
		// Nothing draws before the hazard stage in these ticks: the outcome number is the first number of the next tick.
		FRandomStream Probe(State.RngState);
		const FGameplayTag Expected = CosmicState::PickAIProfile(Probe.GetFraction());
		Sim->StepOnce();
		const int64 Fell = State.Tick;
		TestFalse(TEXT("The rebellion ran out"), State.Systems[6].ActiveHazard.IsValid());
		const FCivilizationState& Civ = State.Civs[1];
		const FNotableRecord* Closed = Civ.Notables.FindByPredicate([](const FNotableRecord& Record) { return Record.SourceRow == FName(TEXT("event_test_old")); });
		if (TestNotNull(TEXT("The old ruler is still on the record"), Closed))
		{
			TestEqual(TEXT("The old ruler left the record when the throne changed"), Closed->LastTick, Fell);
		}
		const FNotableRecord* NewRuler = Civ.Notables.FindByPredicate([Stood](const FNotableRecord& Record) { return Record.FirstTick == Stood; });
		if (TestNotNull(TEXT("The rebel of the rising is on the record"), NewRuler))
		{
			TestTrue(TEXT("The rebel became the ruler"), NewRuler->Role == RulerTag);
			TestEqual(TEXT("The new ruler's record is open"), NewRuler->LastTick, static_cast<int64>(0));
		}
		TestTrue(TEXT("The new profile is the one the outcome number picks"), Civ.AIProfile == Expected);
		TestEqual(TEXT("The expiry's effects were applied"), Civ.Ethics.Openness, FMath::Clamp(OpennessBefore + 0.1f, -1.f, 1.f));
		TestEqual(TEXT("The count of discontent starts over"), Civ.LowStabilityTicks, 0);
		TestTrue(TEXT("The change of throne is told"), FindLine(State, Unrest, Fell, TEXT("expiry")) != INDEX_NONE);
		TestFalse(TEXT("No role effect is recorded for an AI people"), State.Calling.PendingEffect.IsValid());

		// The discontent has to last again before the next rebellion.
		for (int32 Step = 0; Step < 99; ++Step)
		{
			State.Civs[1].Stability = 0.1f;
			Sim->StepOnce();
		}
		TestFalse(TEXT("No rebellion stands before the discontent has lasted again"), State.Systems[6].ActiveHazard.IsValid());
		State.Civs[1].Stability = 0.1f;
		Sim->StepOnce();
		TestTrue(TEXT("The next rebellion stands when it has"), State.Systems[6].ActiveHazard == RebellionTag);
	}

	// The vessel's people: the throne changes with the deposition the expiry recorded.
	{
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the vessel's case is laid"), Board(State)))
		{
			return false;
		}
		State.Tick = 400;
		Discontent(State.Civs[0]);
		Sim->StepOnce();
		const int64 Stood = State.Tick;
		Sim->RunHeadless(3);
		const FNotableRecord* Rebel = State.Civs[0].Notables.FindByPredicate([Stood](const FNotableRecord& Record) { return Record.FirstTick == Stood; });
		if (TestNotNull(TEXT("The vessel's people got a rebel"), Rebel))
		{
			TestTrue(TEXT("The rebel took the throne of the vessel's people"), Rebel->Role == RulerTag);
		}
		TestTrue(TEXT("The vessel was deposed"), State.Player.Role == EPlayerRole::Wanderer);
	}

	// A death recorded first: the deposition is dropped, and the throne does not change.
	{
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the dropped case is laid"), Board(State)))
		{
			return false;
		}
		State.Tick = 400;
		Discontent(State.Civs[0]);
		Sim->StepOnce();
		const int64 Stood = State.Tick;
		Sim->StepOnce();
		State.Calling.PendingEffect = Tag(TEXT("Effect.KillPlayer"));
		Sim->RunHeadless(2);
		const FNotableRecord* Rebel = State.Civs[0].Notables.FindByPredicate([Stood](const FNotableRecord& Record) { return Record.FirstTick == Stood; });
		if (TestNotNull(TEXT("The rebel of the dropped case is on the record"), Rebel))
		{
			TestTrue(TEXT("Without its deposition the rebel stays a rebel"), Rebel->Role == RebelTag);
		}
		TestTrue(TEXT("The vessel still rules"), State.Player.Role == EPlayerRole::Ruler);
		TestEqual(TEXT("The count of discontent starts over all the same"), State.Civs[0].LowStabilityTicks, 0);
		State.Calling.PendingEffect = FGameplayTag();
	}

	// A rebellion resolved: the rebel leaves the record.
	{
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (!TestTrue(TEXT("The board of the resolved case is laid"), Board(State)))
		{
			return false;
		}
		State.Tick = 400;
		Discontent(State.Civs[1]);
		Sim->StepOnce();
		const int64 Stood = State.Tick;
		UCosmicEventSubsystem* EventSubsystem = Sim->GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = 1;
		Ctx.SystemIndex = 6;
		TestTrue(TEXT("The rebellion is resolved"), UCosmicEventSubsystem::Apply(Eff(EEffectType::ResolveHazard), Ctx));
		EventSubsystem->FollowUpNotes(Ctx);
		Sim->SyncRngState();
		const FNotableRecord* Rebel = State.Civs[1].Notables.FindByPredicate([Stood](const FNotableRecord& Record) { return Record.FirstTick == Stood; });
		if (TestNotNull(TEXT("The rebel of the resolved case is on the record"), Rebel))
		{
			TestEqual(TEXT("The rebel left the record at the resolution"), Rebel->LastTick, State.Tick);
			TestTrue(TEXT("The rebel is no ruler"), Rebel->Role == RebelTag);
		}
		TestTrue(TEXT("The resolution is told"), FindLine(State, Unrest, State.Tick, TEXT("resolution")) != INDEX_NONE);
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicDepositionPipelineTest, "Cosmic_Ascension.Sim.DepositionPipeline", CosmicTests::Flags)
bool FCosmicDepositionPipelineTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHazardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };
	Sim->SetTables(TablesWith(Original, Knobs(BaseRules), nullptr, nullptr));
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	if (!TestNotNull(TEXT("The role subsystem"), Role))
	{
		return false;
	}

	// The profile one number picks: four, evenly, in a fixed order; a number outside [0, 1) picks an end.
	TestEqual(TEXT("0 picks the first profile"), CosmicState::PickAIProfile(0.f).GetTagName(), FName(TEXT("AI.Profile.Conservative")));
	TestEqual(TEXT("0.3 picks the second"), CosmicState::PickAIProfile(0.3f).GetTagName(), FName(TEXT("AI.Profile.Expansionist")));
	TestEqual(TEXT("0.6 picks the third"), CosmicState::PickAIProfile(0.6f).GetTagName(), FName(TEXT("AI.Profile.Scholarly")));
	TestEqual(TEXT("0.99 picks the fourth"), CosmicState::PickAIProfile(0.99f).GetTagName(), FName(TEXT("AI.Profile.Militarist")));
	TestEqual(TEXT("A number above 1 picks the last"), CosmicState::PickAIProfile(1.5f).GetTagName(), FName(TEXT("AI.Profile.Militarist")));
	TestEqual(TEXT("A number below 0 picks the first"), CosmicState::PickAIProfile(-1.f).GetTagName(), FName(TEXT("AI.Profile.Conservative")));
	TestEqual(TEXT("A number that is not a number picks the first"), CosmicState::PickAIProfile(std::numeric_limits<float>::quiet_NaN()).GetTagName(), FName(TEXT("AI.Profile.Conservative")));

	// A deposition recorded in a tick and resolved at its boundary.
	auto Record = [Sim](const TCHAR* EffectTag, const TCHAR* Cause)
	{
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = 0;
		FCosmicEffect Effect;
		Effect.Type = FCString::Strcmp(EffectTag, TEXT("Kill")) == 0 ? EEffectType::KillPlayer : EEffectType::DeposeRuler;
		Effect.Tag = Tag(Cause);
		return UCosmicEventSubsystem::Apply(Effect, Ctx);
	};
	Sim->NewGalaxy(42, 12, 4);
	FGalaxyState& State = Sim->GetMutableState();
	if (!TestTrue(TEXT("The board of the test is laid"), Board(State)))
	{
		return false;
	}
	State.Tick = 400;
	State.Civs[0].ReverenceToPlayer = 0.8f;
	State.Civs[0].Stability = 0.6f;
	const FName Ruled = State.Civs[0].CivID;
	const FName Standing = State.Player.CurrentSystemID;
	Sim->SyncRngState();
	const FGalaxyState Before = State;
	if (!TestTrue(TEXT("The deposition is recorded"), Record(TEXT("Depose"), TEXT("Reign.Deposed"))))
	{
		return false;
	}
	Sim->SyncRngState();
	const FGalaxyState Recorded = State;
	Sim->StepOnce();
	const FPlayerEntityState& Player = State.Player;
	const FCivilizationState& Civ = State.Civs[0];
	TestTrue(TEXT("The vessel walks"), Player.Role == EPlayerRole::Wanderer);
	TestTrue(TEXT("It rules nothing"), Player.RuledCivID.IsNone());
	TestEqual(TEXT("It remembers the people it left"), Player.FormerCivID, Ruled);
	TestEqual(TEXT("It stays where it stood"), Player.CurrentSystemID, Standing);
	TestTrue(TEXT("It is a fallen monarch"), Player.StatusTags.HasTagExact(Tag(TEXT("Status.FallenMonarch"))));
	TestFalse(TEXT("The people is no longer ruled by the vessel"), Civ.bIsPlayerRuled);
	TestTrue(TEXT("A regent profile runs the people"), Civ.AIProfile.MatchesTag(Tag(TEXT("AI.Profile"))));
	TestEqual(TEXT("Reverence is halved"), Civ.ReverenceToPlayer, 0.8f * BaseRules.ReverenceDecay);
	TestTrue(TEXT("Stability is at most the post-rebellion value"), Civ.Stability <= BaseRules.PostRebellionStability);
	TestEqual(TEXT("The throne is locked"), Civ.ThroneLockedUntilTick, State.Tick + BaseRules.ThroneLockTicks);
	const FLifeRecord* Life = State.Calling.Lives.IsEmpty() ? nullptr : &State.Calling.Lives.Last();
	TestTrue(TEXT("The reign ended as a deposition"), Life && !Life->ReignEndReasons.IsEmpty() && Life->ReignEndReasons.Last() == Tag(TEXT("Reign.Deposed")));
	const int32 Line = FindLine(State, CosmicRows::LogFallenMonarch, State.Tick);
	if (TestTrue(TEXT("The fallen monarch is told"), Line != INDEX_NONE))
	{
		TestEqual(TEXT("as a mythic line"), static_cast<int32>(State.Log[Line].Severity), 2);
		TestEqual(TEXT("about the people that was ruled"), State.Log[Line].FocusCivID, Ruled);
	}
	TestFalse(TEXT("The record is cleared"), State.Calling.PendingEffect.IsValid());
	const int32 WithDeposition = DrawsBetween(Recorded.RngState, State.RngState);
	Sim->LoadState(Before);
	Sim->StepOnce();
	const int32 Without = DrawsBetween(Before.RngState, Sim->GetState().RngState);
	TestEqual(TEXT("A deposition draws one number"), WithDeposition - Without, 1);

	// A death waits for Phase 4b: it stays recorded and the vessel stays.
	Sim->NewGalaxy(42, 12, 4);
	FGalaxyState& Fresh = Sim->GetMutableState();
	Board(Fresh);
	Fresh.Tick = 400;
	if (TestTrue(TEXT("The death is recorded"), Record(TEXT("Kill"), TEXT("Death.Hazard"))))
	{
		Sim->StepOnce();
		TestTrue(TEXT("A death stays recorded until its pipeline"), Fresh.Calling.PendingEffect == Tag(TEXT("Effect.KillPlayer")));
		TestTrue(TEXT("The vessel still rules"), Fresh.Player.Role == EPlayerRole::Ruler);
	}

	// BecomeWanderer by itself: the status follows the reason, and nothing happens without a throne.
	Sim->NewGalaxy(42, 12, 4);
	Role->BecomeWanderer(Tag(TEXT("AI.Profile.Scholarly")), Tag(TEXT("Reign.Abdicated")));
	TestTrue(TEXT("An abdication leaves its status"), Sim->GetState().Player.StatusTags.HasTagExact(Tag(TEXT("Status.Abdicated"))));
	TestTrue(TEXT("The regent profile is the one handed in"), Sim->GetState().Civs[0].AIProfile == Tag(TEXT("AI.Profile.Scholarly")));
	Role->BecomeWanderer(Tag(TEXT("AI.Profile.Militarist")), Tag(TEXT("Reign.Deposed")));
	TestFalse(TEXT("A wanderer has no throne to leave"), Sim->GetState().Player.StatusTags.HasTagExact(Tag(TEXT("Status.FallenMonarch"))));
	TestTrue(TEXT("and no people of its own to hand over"), Sim->GetState().Civs[0].AIProfile == Tag(TEXT("AI.Profile.Scholarly")));
	Sim->NewGalaxy(42, 12, 4);
	Role->BecomeWanderer(FGameplayTag(), Tag(TEXT("Reign.Died")));
	TestEqual(TEXT("A death leaves no status"), Sim->GetState().Player.StatusTags.Num(), 0);

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

namespace CosmicHazardTests
{
	// The rows of the game behind a record: an event, hazard or log-template row of the tables.
	bool IsGameRow(const FCosmicDataTables& Tables, FName Row)
	{
		return Tables.FindEvent(Row) || Tables.FindHazard(Row) || Tables.FindLogTemplate(Row);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicNotablesCappedTest, "Cosmic_Ascension.Sim.NotablesCapped", CosmicTests::Flags)
bool FCosmicNotablesCappedTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHazardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	const FGalaxyState& State = Sim->GetState();
	const int32 Cap = Sim->GetRules().MaxNotablesPerCiv;
	// The guards of a nameless people and of the fourth rung are Events.NotableDrawFixed's: no people of the slice meets
	// them in this run.
	bool bCapped = true;
	bool bOnePerRole = true;
	bool bClosedInOrder = true;
	int32 Made = 0;
	for (int32 Step = 0; Step < 20000; ++Step)
	{
		Sim->StepOnce();
		for (const FCivilizationState& Civ : State.Civs)
		{
			bCapped &= Civ.Notables.Num() <= Cap;
			TSet<FGameplayTag> OpenRoles;
			for (const FNotableRecord& Record : Civ.Notables)
			{
				Made += Record.FirstTick == State.Tick ? 1 : 0;
				if (Record.LastTick == 0)
				{
					bool bAlready = false;
					OpenRoles.Add(Record.Role, &bAlready);
					bOnePerRole &= !bAlready;
				}
				else
				{
					bClosedInOrder &= Record.LastTick >= Record.FirstTick;
				}
			}
		}
	}
	TestTrue(TEXT("Records were made over the run"), Made >= 1);
	TestTrue(TEXT("No people ever held more records than the cap"), bCapped);
	TestTrue(TEXT("No people ever held two open records of one role"), bOnePerRole);
	TestTrue(TEXT("Every closed record closed after it opened"), bClosedInOrder);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicNotablesFromEventsOnlyTest, "Cosmic_Ascension.Sim.NotablesFromEventsOnly", CosmicTests::Flags)
bool FCosmicNotablesFromEventsOnlyTest::RunTest(const FString& Parameters)
{
	using namespace CosmicHazardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(20000);
	const FGalaxyState& State = Sim->GetState();
	const FCosmicDataTables& Tables = Sim->GetTables();
	auto Told = [&State](FName Row, int64 Tick)
	{
		auto Matches = [Row, Tick](const FCosmicLogEntry& Entry) { return Entry.SourceRow == Row && Entry.Tick == Tick; };
		return State.Log.ContainsByPredicate(Matches) || State.Chronicle.ContainsByPredicate(Matches);
	};
	// That no grace row, messenger or cult carries NameNotable is a rule of the schema (Data.Integrity).
	int32 Records = 0;
	bool bRealRows = true;
	bool bToldAtFirstTick = true;
	for (const FCivilizationState& Civ : State.Civs)
	{
		for (const FNotableRecord& Record : Civ.Notables)
		{
			++Records;
			bRealRows &= IsGameRow(Tables, Record.SourceRow);
			bToldAtFirstTick &= Told(Record.SourceRow, Record.FirstTick);
		}
	}
	TestTrue(TEXT("Records stand at the end of the run"), Records >= 1);
	TestTrue(TEXT("Every record comes from a row of the game"), bRealRows);
	TestTrue(TEXT("Every record was told in the tick it appeared, by the row that made it"), bToldAtFirstTick);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

// Cosmic Ascension - automation tests of the megaprojects, the ruler's command, the AI's projects and the feat judgment.
// Design: guideline.md section 3.2, 4.3, 5.7, 7.4, 7.5, 9.3, 16 #23.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Ascension/AscensionSubsystem.h"
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
#include "Sim/CosmicEndings.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/Tuple.h"
#include "Tests/CosmicTestHelpers.h"
#include "Tests/CosmicTestProbe.h"
#include "UObject/NameTypes.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#include <initializer_list>
#include <limits>

namespace CosmicAscensionTests
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

	FCosmicCondition Cond(EConditionType Type, FName Key = NAME_None, float Value = 0.f, const TCHAR* TagName = nullptr)
	{
		FCosmicCondition Condition;
		Condition.Type = Type;
		Condition.Key = Key;
		Condition.Value = Value;
		Condition.Tag = TagName ? Tag(TagName) : FGameplayTag();
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

	// Three variants that name every slot of Slots before the stem: a told line has to fill them all.
	TArray<FText> SlotLines(const FString& Slots, const FString& Stem)
	{
		return {
			FText::AsCultureInvariant(Slots + TEXT(" ") + Stem + TEXT(" one")),
			FText::AsCultureInvariant(Slots + TEXT(" ") + Stem + TEXT(" two")),
			FText::AsCultureInvariant(Slots + TEXT(" ") + Stem + TEXT(" three"))
		};
	}

	// The slots of a completion line and of an ascension line, with the particle modifier the game's rows use (its two
	// forms in ASCII: the modifier picks one by the last letter of the name and takes any text).
	const TCHAR* const ProjectSlots = TEXT("{Civ}|hpp(eun,neun) {System} {Project}");
	const TCHAR* const FeatSlots = TEXT("{Civ}|hpp(eun,neun) {HomeSystem}");

	// The rows of the tests.
	const FName Build(TEXT("project_test_build"));
	const FName Rebuild(TEXT("project_test_rebuild"));
	const FName High(TEXT("project_test_high"));
	const FName Pricey(TEXT("project_test_pricey"));
	const FName Answer(TEXT("project_test_answer"));
	const FName Second(TEXT("project_test_second"));
	const FName Costly(TEXT("project_test_costly"));
	const FName HighAnswer(TEXT("project_test_high_answer"));
	const FName Dear(TEXT("project_test_dear"));
	const FName Refund(TEXT("project_test_refund"));
	const FName Endless(TEXT("project_test_endless"));
	const FName Storm(TEXT("crisis_test_storm"));
	const FName Unrest(TEXT("crisis_test_unrest"));
	const FName Quake(TEXT("crisis_test_quake"));

	FMegaprojectDef Project(EAscensionTier Tier, float Energy, float Matter, float Required, const TCHAR* Stem)
	{
		FMegaprojectDef Row;
		Row.DisplayName = FText::AsCultureInvariant(FString::Printf(TEXT("<%s>"), Stem));
		Row.RequiredTier = Tier;
		Row.CostPerTick.Energy = Energy;
		Row.CostPerTick.Matter = Matter;
		Row.RequiredProgress = Required;
		Row.CompleteTemplates = SlotLines(ProjectSlots, FString(Stem) + TEXT(" done"));
		return Row;
	}

	// The projects of the tests: Build and Rebuild (construction, Tier 1, Energy 10, 3), High (Tier 2), Pricey (Energy more
	// than any stockpile of a test, Matter 5), Answer and Second (the answers of Storm, Matter 5, 2), Costly (an answer of
	// Storm no stockpile pays), HighAnswer (Tier 2, the first answer of Quake), Dear (one Insight, Influence and Causality,
	// 5), Refund (a cost below 0, 5), Endless (a progress that is not a number), and a project_settle row (Tier 1, Energy
	// 1, 3), which Start never takes: a settlement starts through CosmicMegaprojects::Settle.
	UDataTable* ProjectRows()
	{
		UDataTable* Table = NewTable(FMegaprojectDef::StaticStruct());
		Table->AddRow(Build, Project(EAscensionTier::Tier1_Planetary, 10.f, 0.f, 3.f, TEXT("build")));
		Table->AddRow(Rebuild, Project(EAscensionTier::Tier1_Planetary, 10.f, 0.f, 3.f, TEXT("rebuild")));
		Table->AddRow(High, Project(EAscensionTier::Tier2_Stellar, 10.f, 0.f, 3.f, TEXT("high")));
		Table->AddRow(Pricey, Project(EAscensionTier::Tier1_Planetary, 1.0e9f, 5.f, 3.f, TEXT("pricey")));
		Table->AddRow(Answer, Project(EAscensionTier::Tier1_Planetary, 0.f, 5.f, 2.f, TEXT("answer")));
		Table->AddRow(Second, Project(EAscensionTier::Tier1_Planetary, 0.f, 5.f, 2.f, TEXT("second")));
		Table->AddRow(Costly, Project(EAscensionTier::Tier1_Planetary, 0.f, 1.0e9f, 2.f, TEXT("costly")));
		Table->AddRow(HighAnswer, Project(EAscensionTier::Tier2_Stellar, 0.f, 5.f, 2.f, TEXT("high answer")));
		FMegaprojectDef DearRow = Project(EAscensionTier::Tier1_Planetary, 0.f, 0.f, 5.f, TEXT("dear"));
		DearRow.CostPerTick.Insight = 1.f;
		DearRow.CostPerTick.Influence = 1.f;
		DearRow.CostPerTick.Causality = 1.f;
		Table->AddRow(Dear, DearRow);
		Table->AddRow(Refund, Project(EAscensionTier::Tier1_Planetary, -5.f, 0.f, 5.f, TEXT("refund")));
		Table->AddRow(Endless, Project(EAscensionTier::Tier1_Planetary, 1.f, 0.f, std::numeric_limits<float>::quiet_NaN(), TEXT("endless")));
		Table->AddRow(CosmicRows::ProjectSettle, Project(EAscensionTier::Tier1_Planetary, 1.f, 0.f, 3.f, TEXT("settle")));
		return Table;
	}

	FHazardDef HazardRow(const TCHAR* TagName, ECosmicHazardSpawn Rule, int32 Countdown, TArray<FName> Answers)
	{
		FHazardDef Row;
		Row.HazardTag = Tag(TagName);
		Row.DisplayName = FText::AsCultureInvariant(FString::Printf(TEXT("<%s>"), TagName));
		Row.CountdownTicks = Countdown;
		Row.SpawnRule = Rule;
		Row.SpawnChancePerTick = 0.f;
		Row.LogCategory = Tag(TEXT("Log.Crisis"));
		Row.Severity = 1;
		Row.RulerResolutions = MoveTemp(Answers);
		Row.OutbreakTemplates = Lines(FString(TagName) + TEXT(" outbreak"));
		Row.ExpireTemplates = Lines(FString(TagName) + TEXT(" expiry"));
		Row.ResolveTemplates = Lines(FString(TagName) + TEXT(" resolution"));
		return Row;
	}

	// Storm: a plague answered by Answer, then Second, then Costly. Quake: an awakening answered by HighAnswer (Tier 2),
	// then Answer. Unrest: a rebellion that lists no project of the tests and never stands by itself.
	UDataTable* HazardRows()
	{
		UDataTable* Table = NewTable(FHazardDef::StaticStruct());
		Table->AddRow(Storm, HazardRow(TEXT("Hazard.Plague"), ECosmicHazardSpawn::Draw, 50, { Answer, Second, Costly }));
		Table->AddRow(Quake, HazardRow(TEXT("Hazard.AIAwakening"), ECosmicHazardSpawn::Draw, 50, { HighAnswer, Answer }));
		// The capital rule never holds for it: the tests place their rebellions by hand.
		FHazardDef Rebellion = HazardRow(TEXT("Hazard.Rebellion"), ECosmicHazardSpawn::Capital, 200, {});
		Rebellion.SpawnConditions = { Cond(EConditionType::CivStabilityBelow, NAME_None, 0.f) };
		Table->AddRow(Unrest, Rebellion);
		return Table;
	}

	FAscensionFeatDef FeatRow(EAscensionTier From, EAscensionTier To, TArray<FCosmicCondition> Conditions, const TCHAR* Stem)
	{
		FAscensionFeatDef Row;
		Row.Title = FText::AsCultureInvariant(Stem);
		Row.RequiredTier = From;
		Row.TargetTier = To;
		Row.Path = EPlayerRole::Ruler;
		Row.CompletionConditions = MoveTemp(Conditions);
		Row.LogTemplates = SlotLines(FeatSlots, FString(Stem) + TEXT(" climbed"));
		return Row;
	}

	// The knobs of the tests: no Lottery chance, no pressure drain, AI projects twice as fast, and a realm cap of one on
	// every tier, so that no people of a board settles (a settlement would draw and take wild systems the tests reason
	// about; Sim.Settlement brings its own cap).
	FCosmicRulesDef Knobs(const FCosmicRulesDef& BaseRules)
	{
		FCosmicRulesDef Rules = BaseRules;
		Rules.EventSlotTicks = 10;
		Rules.EventChancePerSlot = 0.f;
		Rules.SpiritualPressureStabilityDrain = 0.f;
		Rules.AIAscensionSpeedMultiplier = 2.f;
		Rules.RealmCapByTier = { 1, 1, 1, 1, 1 };
		return Rules;
	}

	FCosmicDataTables TablesWith(const FCosmicDataTables& Original, const FCosmicRulesDef& Rules, UDataTable* Feats)
	{
		UDataTable* RulesTable = NewTable(FCosmicRulesDef::StaticStruct());
		RulesTable->AddRow(CosmicRows::Rules, Rules);
		FCosmicDataTables Tables = Original;
		Tables.Rules = RulesTable;
		Tables.Hazards = HazardRows();
		Tables.Events = NewTable(FCosmicEventDef::StaticStruct());
		Tables.Transcendents = NewTable(FTranscendentDef::StaticStruct());
		Tables.Megaprojects = ProjectRows();
		Tables.Feats = Feats ? Feats : NewTable(FAscensionFeatDef::StaticStruct());
		return Tables;
	}

	// The board: the chain of CosmicTests::WireChain; the vessel's people 0 holds 0 and 1 (capital 0), people 1 holds 6,
	// people 2 holds 7 and people 3 holds 11. No hazard, project, record, transcendent, flag or research; every stockpile
	// 1000 of each but Causality; every Stability 0.75; every people of the archetype arch_default (a bank of person names)
	// without Trait.Mind.NoSelf, so that a record of a person can be made whatever the game's DT_Civs gives seed 42. False
	// below twelve systems or four peoples.
	bool Board(FGalaxyState& State)
	{
		if (!CosmicTests::WireChain(State) || State.Civs.Num() < 4)
		{
			return false;
		}
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { 0, 1 });
		CosmicTests::Give(State, 1, { 6 });
		CosmicTests::Give(State, 2, { 7 });
		CosmicTests::Give(State, 3, { 11 });
		State.Transcendents.Reset();
		State.Flags.Reset();
		for (FStarSystemState& System : State.Systems)
		{
			CosmicHazards::EmptyHazardSlot(System);
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
			System.CompletedMegaprojects.Reset();
			System.MegaprojectCompletedTick = 0;
		}
		for (FCivilizationState& Civ : State.Civs)
		{
			for (TPair<FName, float>& Field : Civ.Research)
			{
				Field.Value = 0.f;
			}
			for (TPair<FName, float>& Focus : Civ.ResearchFocus)
			{
				Focus.Value = 0.f;
			}
			Civ.Stockpile.Energy = 1000.f;
			Civ.Stockpile.Matter = 1000.f;
			Civ.Stockpile.Insight = 1000.f;
			Civ.Stockpile.Influence = 1000.f;
			Civ.Stability = 0.75f;
			Civ.LowStabilityTicks = 0;
			Civ.Notables.Reset();
			Civ.Tier = EAscensionTier::Tier1_Planetary;
			Civ.ArchetypeRow = CosmicRows::ArchDefault;
			Civ.Traits.RemoveTag(Tag(TEXT("Trait.Mind.NoSelf")));
		}
		return true;
	}

	// A new galaxy under Tables with the board laid; null when it cannot be laid.
	FGalaxyState* LayBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Tables)
	{
		Sim.SetTables(Tables);
		Sim.NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim.GetMutableState();
		return Board(State) ? &State : nullptr;
	}

	void PlaceByHand(FGalaxyState& State, int32 SystemIndex, const TCHAR* HazardTag, int32 Ticks)
	{
		State.Systems[SystemIndex].ActiveHazard = Tag(HazardTag);
		State.Systems[SystemIndex].HazardTicksRemaining = Ticks;
	}

	void Record(FStarSystemState& System, FName Row, int64 Tick)
	{
		FMegaprojectRecord& Added = System.CompletedMegaprojects.AddDefaulted_GetRef();
		Added.Row = Row;
		Added.Tick = Tick;
	}

	const FMegaprojectRecord* RecordOf(const FStarSystemState& System, FName Row)
	{
		return System.CompletedMegaprojects.FindByPredicate([Row](const FMegaprojectRecord& Candidate) { return Candidate.Row == Row; });
	}

	int32 FindLine(const FGalaxyState& State, FName Row, int64 Tick, const TCHAR* Word = nullptr)
	{
		for (int32 Index = 0; Index < State.Log.Num(); ++Index)
		{
			const FCosmicLogEntry& Entry = State.Log[Index];
			if (Entry.SourceRow == Row && Entry.Tick == Tick && (!Word || Entry.Message.ToString().Contains(Word)))
			{
				return Index;
			}
		}
		return INDEX_NONE;
	}

	// A told line fills every slot: no brace is left, and each name of Slots is in it - one assertion per slot, each with
	// its own message.
	void ExpectFilled(FAutomationTestBase& Test, const FCosmicLogEntry& Entry, std::initializer_list<TPair<FString, const TCHAR*>> Slots)
	{
		const FString Message = Entry.Message.ToString();
		Test.TestFalse(TEXT("No slot of the line is left unfilled"), Message.Contains(TEXT("{")));
		for (const TPair<FString, const TCHAR*>& Slot : Slots)
		{
			Test.TestTrue(Slot.Value, !Slot.Key.IsEmpty() && Message.Contains(Slot.Key));
		}
	}

	// The index of the first line of Row at Tick about the system SystemID whose text holds Word; INDEX_NONE when none.
	int32 FindLineAbout(const FGalaxyState& State, FName Row, int64 Tick, FName SystemID, const TCHAR* Word)
	{
		for (int32 Index = 0; Index < State.Log.Num(); ++Index)
		{
			const FCosmicLogEntry& Entry = State.Log[Index];
			if (Entry.SourceRow == Row && Entry.Tick == Tick && Entry.FocusSystemID == SystemID && Entry.Message.ToString().Contains(Word))
			{
				return Index;
			}
		}
		return INDEX_NONE;
	}

	int32 CountLines(const FGalaxyState& State, FName Row)
	{
		int32 Count = 0;
		for (const FCosmicLogEntry& Entry : State.Log)
		{
			Count += Entry.SourceRow == Row ? 1 : 0;
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

	// A probe on the log, the event notices and the ascensions for the lifetime of the object.
	struct FProbe
	{
		FProbe(UGalaxySimulationSubsystem& Sim)
			: Log(Sim.GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>())
			, Ascension(Sim.GetGameInstance()->GetSubsystem<UAscensionSubsystem>())
			, Probe(NewObject<UCosmicTestProbe>())
		{
			Probe->AddToRoot();
			Log->OnLogEntry.AddDynamic(Probe, &UCosmicTestProbe::HandleLogEntry);
			Ascension->OnTierAscended.AddDynamic(Probe, &UCosmicTestProbe::HandleTierAscended);
		}

		~FProbe()
		{
			Log->OnLogEntry.RemoveDynamic(Probe, &UCosmicTestProbe::HandleLogEntry);
			Ascension->OnTierAscended.RemoveDynamic(Probe, &UCosmicTestProbe::HandleTierAscended);
			Probe->RemoveFromRoot();
		}

		UCosmicLogSubsystem* Log;
		UAscensionSubsystem* Ascension;
		UCosmicTestProbe* Probe;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicMegaprojectProgressTest, "Cosmic_Ascension.Sim.MegaprojectProgress", CosmicTests::Flags)
bool FCosmicMegaprojectProgressTest::RunTest(const FString& Parameters)
{
	using namespace CosmicAscensionTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FCosmicDataTables Tables = TablesWith(Original, Knobs(BaseRules), nullptr);

	FGalaxyState* Laid = LayBoard(*Sim, Tables);
	if (!TestNotNull(TEXT("The board of the projects is laid"), Laid))
	{
		return false;
	}
	FGalaxyState& State = *Laid;
	// The vessel's people builds on 0, people 1 (AI) builds on 6, people 2 (AI) cannot pay on 7, the vessel's people
	// answers a plague on 1, and people 3 (AI) answers a plague on 11.
	State.Systems[0].ActiveMegaproject = Build;
	State.Systems[6].ActiveMegaproject = Build;
	State.Systems[7].ActiveMegaproject = Pricey;
	PlaceByHand(State, 1, TEXT("Hazard.Plague"), 50);
	State.Systems[1].ResponseMegaproject = Answer;
	PlaceByHand(State, 11, TEXT("Hazard.Plague"), 50);
	State.Systems[11].ResponseMegaproject = Answer;
	const FGalaxyState Before = State;
	Sim->StepOnce();
	const FCivilizationState& Player = State.Civs[0];
	TestEqual(TEXT("A paid tick of the people the player rules progresses by 1"), State.Systems[0].MegaprojectProgress, 1.f);
	TestEqual(TEXT("and pays the whole cost"), Player.Stockpile.Energy, Before.Civs[0].Stockpile.Energy + Player.IncomePerTick.Energy - 10.f, 0.01f);
	TestEqual(TEXT("A paid tick of an AI people progresses by the multiplier"), State.Systems[6].MegaprojectProgress, 2.f);
	TestEqual(TEXT("A tick that cannot be paid in full progresses nothing"), State.Systems[7].MegaprojectProgress, 0.f);
	TestEqual(TEXT("and costs nothing"), State.Civs[2].Stockpile.Energy, Before.Civs[2].Stockpile.Energy + State.Civs[2].IncomePerTick.Energy, 0.01f);
	TestEqual(TEXT("not even the member it could pay"), State.Civs[2].Stockpile.Matter, Before.Civs[2].Stockpile.Matter + State.Civs[2].IncomePerTick.Matter, 0.01f);
	TestEqual(TEXT("The answer progresses in its own slot"), State.Systems[1].ResponseProgress, 1.f);
	TestEqual(TEXT("and pays its own cost"), Player.Stockpile.Matter, Before.Civs[0].Stockpile.Matter + Player.IncomePerTick.Matter - 5.f, 0.01f);
	// Required 2 at the multiplier 2: one paid tick completes the AI answer and resolves the plague.
	TestTrue(TEXT("An AI answer progresses by the multiplier too: it completes in one paid tick"), RecordOf(State.Systems[11], Answer) != nullptr);
	TestFalse(TEXT("and resolves its hazard"), State.Systems[11].ActiveHazard.IsValid());

	// Whether a slot paid, read across the tick (CosmicMegaprojects::PaidNothing - the system panel's 'paused' mark and the
	// commandlet read it so): exact against the payments of this tick.
	{
		using CosmicMegaprojects::ECosmicProjectSlot;
		auto Unpaid = [&](int32 SystemIndex, ECosmicProjectSlot Slot)
		{
			return CosmicMegaprojects::PaidNothing(CosmicMegaprojects::ReadSlot(Before.Systems[SystemIndex], Slot), CosmicMegaprojects::ReadSlot(State.Systems[SystemIndex], Slot));
		};
		TestTrue(TEXT("A slot that could not pay paid nothing"), Unpaid(7, ECosmicProjectSlot::Construction));
		TestFalse(TEXT("A paid slot of the people the player rules did not"), Unpaid(0, ECosmicProjectSlot::Construction));
		TestFalse(TEXT("A paid slot of an AI people did not"), Unpaid(6, ECosmicProjectSlot::Construction));
		TestFalse(TEXT("A paid answer did not"), Unpaid(1, ECosmicProjectSlot::Response));
		TestFalse(TEXT("A slot that completed and emptied did not"), Unpaid(11, ECosmicProjectSlot::Response));
		TestFalse(TEXT("An empty slot did not"), Unpaid(2, ECosmicProjectSlot::Construction));
		const CosmicMegaprojects::FCosmicSlotReading Read = CosmicMegaprojects::ReadSlot(State.Systems[1], ECosmicProjectSlot::Response);
		TestEqual(TEXT("ReadSlot reads the response slot's row"), Read.Row, Answer);
		TestEqual(TEXT("and its progress"), Read.Progress, State.Systems[1].ResponseProgress);
		const CosmicMegaprojects::FCosmicSlotReading Built = CosmicMegaprojects::ReadSlot(State.Systems[0], ECosmicProjectSlot::Construction);
		TestEqual(TEXT("ReadSlot reads the construction slot's row"), Built.Row, Build);
		TestEqual(TEXT("and its progress"), Built.Progress, State.Systems[0].MegaprojectProgress);
	}

	// The projects draw nothing, a completion and its resolution included: the same tick without them draws as much.
	{
		FGalaxyState Bare = Before;
		for (FStarSystemState& System : Bare.Systems)
		{
			System.ActiveMegaproject = NAME_None;
			System.ResponseMegaproject = NAME_None;
		}
		const FGalaxyState After = State;
		Sim->LoadState(Bare);
		Sim->StepOnce();
		TestEqual(TEXT("The projects of a tick draw nothing from the sim stream"), Sim->GetState().RngState, After.RngState);
		Sim->LoadState(After);
	}
	FGalaxyState& Live = Sim->GetMutableState();

	// The AI build completes on its second tick, the answer completes and resolves the plague.
	Sim->StepOnce();
	const int64 Second2 = Live.Tick;
	TestTrue(TEXT("The AI people completed its build"), Live.Systems[6].ActiveMegaproject.IsNone());
	const FMegaprojectRecord* AIRecord = RecordOf(Live.Systems[6], Build);
	if (TestNotNull(TEXT("The completion is recorded on the system"), AIRecord))
	{
		TestEqual(TEXT("at the tick of completion"), AIRecord->Tick, Second2);
	}
	TestEqual(TEXT("The latest completion is the system's completion tick"), Live.Systems[6].MegaprojectCompletedTick, Second2);
	const int32 AILine = FindLine(Live, Build, Second2);
	if (TestTrue(TEXT("The AI completion is told"), AILine != INDEX_NONE))
	{
		TestEqual(TEXT("as a line of Severity 0"), static_cast<int32>(Live.Log[AILine].Severity), 0);
		TestTrue(TEXT("in Log.Civ"), Live.Log[AILine].Category == Tag(TEXT("Log.Civ")));
		TestEqual(TEXT("about the people"), Live.Log[AILine].FocusCivID, Live.Civs[1].CivID);
		TestEqual(TEXT("and the system"), Live.Log[AILine].FocusSystemID, Live.Systems[6].SystemID);
		ExpectFilled(*this, Live.Log[AILine], { { Live.Civs[1].CivName.ToString(), TEXT("{Civ} names the people") },
			{ Live.Systems[6].DisplayName.ToString(), TEXT("{System} names the system") }, { FString(TEXT("<build>")), TEXT("{Project} names the project") } });
	}
	TestFalse(TEXT("The answered hazard is resolved"), Live.Systems[1].ActiveHazard.IsValid());
	TestTrue(TEXT("The answer is recorded too"), RecordOf(Live.Systems[1], Answer) != nullptr);
	const int32 AnswerLine = FindLine(Live, Answer, Second2);
	const int32 Resolution = FindLine(Live, Storm, Second2, TEXT("resolution"));
	const bool bAnswerTold = TestTrue(TEXT("The answer's completion is told"), AnswerLine != INDEX_NONE);
	const bool bResolutionTold = TestTrue(TEXT("The resolution is told"), Resolution != INDEX_NONE);
	if (bAnswerTold && bResolutionTold)
	{
		TestTrue(TEXT("The completion line comes before the resolution line"), AnswerLine < Resolution);
		TestEqual(TEXT("The completion of the people the player rules is Severity 1"), static_cast<int32>(Live.Log[AnswerLine].Severity), 1);
		// System 1 is not the capital (0): the line is about the system that completed.
		TestEqual(TEXT("The completion line is about the system that completed, not the capital"), Live.Log[AnswerLine].FocusSystemID, Live.Systems[1].SystemID);
		TestTrue(TEXT("and {System} names that system"), Live.Log[AnswerLine].Message.ToString().Contains(Live.Systems[1].DisplayName.ToString()));
		TestFalse(TEXT("not the capital"), Live.Log[AnswerLine].Message.ToString().Contains(Live.Systems[0].DisplayName.ToString()));
	}
	TestTrue(TEXT("The vessel's life records what its people completed"), !Live.Calling.Lives.IsEmpty() && Live.Calling.Lives.Last().MegaprojectsCompleted.Contains(Answer));
	TestFalse(TEXT("The vessel's life records nothing of another people"), !Live.Calling.Lives.IsEmpty() && Live.Calling.Lives.Last().MegaprojectsCompleted.Contains(Build));

	// Both slots of one system complete in one tick: two records, neither lost.
	{
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the two completions is laid"), Again))
		{
			return false;
		}
		Again->Systems[0].ActiveMegaproject = Build;
		Again->Systems[0].MegaprojectProgress = 2.f;
		PlaceByHand(*Again, 0, TEXT("Hazard.Plague"), 50);
		Again->Systems[0].ResponseMegaproject = Answer;
		Again->Systems[0].ResponseProgress = 1.f;
		Sim->StepOnce();
		const int64 Both = Again->Tick;
		const FMegaprojectRecord* Built = RecordOf(Again->Systems[0], Build);
		const FMegaprojectRecord* Answered = RecordOf(Again->Systems[0], Answer);
		TestTrue(TEXT("The construction of the same tick is recorded"), Built && Built->Tick == Both);
		TestTrue(TEXT("and so is the answer"), Answered && Answered->Tick == Both);
		// B3 (section 16 #73): a new record names the people that completed it.
		TestTrue(TEXT("A new record names the people that completed it"), Built && Built->CivID == Again->Civs[0].CivID);
		TestTrue(TEXT("(the answer's too)"), Answered && Answered->CivID == Again->Civs[0].CivID);
		TestEqual(TEXT("Two records on one system"), Again->Systems[0].CompletedMegaprojects.Num(), 2);
		TestTrue(TEXT("Both completions are told: the construction"), FindLine(*Again, Build, Both) != INDEX_NONE);
		TestTrue(TEXT("and the answer"), FindLine(*Again, Answer, Both) != INDEX_NONE);
	}

	// A second completion of a row moves its tick; it adds no record.
	{
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the second completion is laid"), Again))
		{
			return false;
		}
		Record(Again->Systems[0], Build, 3);
		// The old record names another people (as after a conquest, B3): the re-completion rewrites it to the completer -
		// without this seed the record would already name the owner and a writer that skips old records would pass.
		Again->Systems[0].CompletedMegaprojects.Last().CivID = Again->Civs[1].CivID;
		Again->Systems[0].ActiveMegaproject = Build;
		Again->Systems[0].MegaprojectProgress = 2.f;
		Sim->StepOnce();
		TestEqual(TEXT("One record per row"), Again->Systems[0].CompletedMegaprojects.Num(), 1);
		const FMegaprojectRecord* Built = RecordOf(Again->Systems[0], Build);
		TestTrue(TEXT("whose tick moved to the new completion"), Built && Built->Tick == Again->Tick);
		TestTrue(TEXT("and whose completer is the people that completed it again"), Built && Built->CivID == Again->Civs[0].CivID);
	}

	// A record from a save made before B3 (no completer) is rewritten on its next completion too.
	{
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the old record is laid"), Again))
		{
			return false;
		}
		Record(Again->Systems[0], Build, 3);
		Again->Systems[0].ActiveMegaproject = Build;
		Again->Systems[0].MegaprojectProgress = 2.f;
		Sim->StepOnce();
		const FMegaprojectRecord* Built = RecordOf(Again->Systems[0], Build);
		TestTrue(TEXT("An old record without a completer gets one"), Built && Built->CivID == Again->Civs[0].CivID);
	}

	// The last tick of a hazard: a completion in step (a) saves the system before the countdown runs out.
	{
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the last tick is laid"), Again))
		{
			return false;
		}
		PlaceByHand(*Again, 1, TEXT("Hazard.Plague"), 1);
		Again->Systems[1].ResponseMegaproject = Answer;
		Again->Systems[1].ResponseProgress = 1.f;
		Sim->StepOnce();
		TestFalse(TEXT("The hazard of the last tick is resolved"), Again->Systems[1].ActiveHazard.IsValid());
		TestTrue(TEXT("its resolution is told"), FindLine(*Again, Storm, Again->Tick, TEXT("resolution")) != INDEX_NONE);
		TestEqual(TEXT("and it does not expire"), FindLine(*Again, Storm, Again->Tick, TEXT("expiry")), static_cast<int32>(INDEX_NONE));
	}

	// The four ways a hazard leaves its slot take its answer with it.
	{
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the leaving hazards is laid"), Again))
		{
			return false;
		}
		for (const int32 SystemIndex : { 0, 1, 6, 7 })
		{
			PlaceByHand(*Again, SystemIndex, TEXT("Hazard.Plague"), 50);
			Again->Systems[SystemIndex].ResponseMegaproject = Answer;
			Again->Systems[SystemIndex].ResponseProgress = 1.f;
		}
		// An answer of the plague no stockpile pays: it cannot complete, whatever the economy earns.
		Again->Systems[7].ResponseMegaproject = Costly;
		Sim->RebuildStateIndex();
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = 0;
		Ctx.SystemIndex = 0;
		TestTrue(TEXT("A resolution by effect"), UCosmicEventSubsystem::Apply(Eff(EEffectType::ResolveHazard), Ctx));
		TestTrue(TEXT("takes the answer with the hazard"), Again->Systems[0].ResponseMegaproject.IsNone());
		TestEqual(TEXT("and its progress"), Again->Systems[0].ResponseProgress, 0.f);
		Ctx.SystemIndex = 1;
		TestTrue(TEXT("Letting a plagued system go"), UCosmicEventSubsystem::Apply(Eff(EEffectType::TransferSystem), Ctx));
		TestTrue(TEXT("takes the answer with the hazard"), Again->Systems[1].ResponseMegaproject.IsNone());
		// Handed to another people, a drawn hazard goes with the system; the old owner's answer does not.
		CosmicTests::Give(*Again, 0, { 2 });
		PlaceByHand(*Again, 2, TEXT("Hazard.Plague"), 50);
		Again->Systems[2].ResponseMegaproject = Answer;
		Again->Systems[2].ResponseProgress = 1.f;
		Ctx.SystemIndex = 2;
		TestTrue(TEXT("Handing a plagued system over"), UCosmicEventSubsystem::Apply(Eff(EEffectType::TransferSystem, nullptr, Again->Civs[3].CivID), Ctx));
		TestTrue(TEXT("lets the drawn hazard go with it"), Again->Systems[2].ActiveHazard == Tag(TEXT("Hazard.Plague")));
		TestTrue(TEXT("but not the old owner's answer"), Again->Systems[2].ResponseMegaproject.IsNone());
		CosmicHazards::Destroy(Ctx, 6);
		TestTrue(TEXT("A destruction takes the answer"), Again->Systems[6].ResponseMegaproject.IsNone());
		Sim->SyncRngState();
		Again->Systems[7].HazardTicksRemaining = 1;
		Sim->StepOnce();
		TestFalse(TEXT("The unanswered plague ran out"), Again->Systems[7].ActiveHazard.IsValid());
		TestTrue(TEXT("An expiry takes the answer"), Again->Systems[7].ResponseMegaproject.IsNone());
		TestEqual(TEXT("and its progress"), Again->Systems[7].ResponseProgress, 0.f);
		TestTrue(TEXT("without a completion"), RecordOf(Again->Systems[7], Costly) == nullptr);
	}

	// An answer the row of the hazard no longer lists ends without result; a construction row the table does not hold ends.
	{
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the stale rows is laid"), Again))
		{
			return false;
		}
		PlaceByHand(*Again, 0, TEXT("Hazard.Rebellion"), 50);
		Again->Systems[0].ResponseMegaproject = Answer;
		Again->Systems[0].ResponseProgress = 1.f;
		Again->Systems[1].ActiveMegaproject = FName(TEXT("project_test_gone"));
		Again->Systems[1].MegaprojectProgress = 2.f;
		Sim->StepOnce();
		TestTrue(TEXT("An answer the hazard's row does not list ends"), Again->Systems[0].ResponseMegaproject.IsNone());
		TestTrue(TEXT("without a completion"), RecordOf(Again->Systems[0], Answer) == nullptr);
		TestTrue(TEXT("The hazard stays"), Again->Systems[0].ActiveHazard == Tag(TEXT("Hazard.Rebellion")));
		TestTrue(TEXT("A construction row no table holds ends"), Again->Systems[1].ActiveMegaproject.IsNone());
		TestTrue(TEXT("without a record"), Again->Systems[1].CompletedMegaprojects.IsEmpty());
	}

	// Every member of a cost is paid in full or none is: Insight, Influence and Causality as much as Energy and Matter.
	{
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the members is laid"), Again))
		{
			return false;
		}
		Again->Systems[0].ActiveMegaproject = Dear;
		Again->Civs[0].Stockpile.Causality = 0.f;
		const FGalaxyState Short = *Again;
		Sim->StepOnce();
		TestEqual(TEXT("A tick short of one member progresses nothing"), Again->Systems[0].MegaprojectProgress, 0.f);
		TestEqual(TEXT("and takes no member it could pay"), Again->Civs[0].Stockpile.Influence, Short.Civs[0].Stockpile.Influence + Again->Civs[0].IncomePerTick.Influence, 0.01f);
		Again->Civs[0].Stockpile.Causality = 10.f;
		const FGalaxyState Stocked = *Again;
		Sim->StepOnce();
		const float InsightPaid = Again->Civs[0].Stockpile.Insight;
		TestEqual(TEXT("A tick that pays every member progresses"), Again->Systems[0].MegaprojectProgress, 1.f);
		TestEqual(TEXT("and takes its Influence"), Again->Civs[0].Stockpile.Influence, Stocked.Civs[0].Stockpile.Influence + Again->Civs[0].IncomePerTick.Influence - 1.f, 0.01f);
		TestEqual(TEXT("its Causality"), Again->Civs[0].Stockpile.Causality, Stocked.Civs[0].Stockpile.Causality + Again->Civs[0].IncomePerTick.Causality - 1.f, 0.01f);
		// Research spends Insight as well: the same tick without the project tells what the project took.
		FGalaxyState Unbuilt = Stocked;
		Unbuilt.Systems[0].ActiveMegaproject = NAME_None;
		Sim->LoadState(Unbuilt);
		Sim->StepOnce();
		TestEqual(TEXT("and its Insight"), Sim->GetState().Civs[0].Stockpile.Insight - InsightPaid, 1.f, 0.01f);
	}

	// A settlement is one more construction row (section 5.7): paid, progressed and completed like the others, with its
	// record, its line and the vessel's life; the system it completes on is settled from then on.
	{
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the settling system is laid"), Again))
		{
			return false;
		}
		// The test's settle row costs Energy 1 and needs 3: one paid tick completes it.
		Again->Systems[1].ActiveMegaproject = CosmicRows::ProjectSettle;
		Again->Systems[1].MegaprojectProgress = 1.f;
		Sim->StepOnce();
		TestEqual(TEXT("A paid tick of a settlement progresses it"), Again->Systems[1].MegaprojectProgress, 2.f);
		TestFalse(TEXT("and the system is still being settled"), CosmicState::IsSettled(Again->Systems[1]));
		const FGalaxyState Settling = *Again;
		Sim->StepOnce();
		TestTrue(TEXT("A settlement that completes empties its slot"), Again->Systems[1].ActiveMegaproject.IsNone());
		TestEqual(TEXT("and its progress"), Again->Systems[1].MegaprojectProgress, 0.f);
		TestTrue(TEXT("The system it completed on is settled"), CosmicState::IsSettled(Again->Systems[1]));
		const FMegaprojectRecord* Settled = RecordOf(Again->Systems[1], CosmicRows::ProjectSettle);
		TestTrue(TEXT("The settlement is recorded on its system"), Settled != nullptr);
		TestEqual(TEXT("at the tick it completed"), Settled ? Settled->Tick : int64(0), Again->Tick);
		TestEqual(TEXT("which is the latest completion of the system"), Again->Systems[1].MegaprojectCompletedTick, Again->Tick);
		TestTrue(TEXT("The life of the vessel that rules the people takes it"), !Again->Calling.Lives.IsEmpty() && Again->Calling.Lives.Last().MegaprojectsCompleted.Contains(CosmicRows::ProjectSettle));
		TestEqual(TEXT("It costs its row"), Again->Civs[0].Stockpile.Energy, Settling.Civs[0].Stockpile.Energy + Again->Civs[0].IncomePerTick.Energy - 1.f, 0.01f);
		const int32 Done = FindLine(*Again, CosmicRows::ProjectSettle, Again->Tick, TEXT("settle done"));
		if (TestTrue(TEXT("Its completion is told"), Done != INDEX_NONE))
		{
			TestEqual(TEXT("about the system it settled"), Again->Log[Done].FocusSystemID, Again->Systems[1].SystemID);
		}
	}

	// A settlement waits for its row: a missing row does not settle the system at once.
	{
		FCosmicDataTables NoSettle = Tables;
		UDataTable* Rows = ProjectRows();
		Rows->RemoveRow(CosmicRows::ProjectSettle);
		NoSettle.Megaprojects = Rows;
		FGalaxyState* Again = LayBoard(*Sim, NoSettle);
		if (!TestNotNull(TEXT("The board without the settle row is laid"), Again))
		{
			return false;
		}
		Again->Systems[1].ActiveMegaproject = CosmicRows::ProjectSettle;
		Again->Systems[1].MegaprojectProgress = 2.f;
		Sim->StepOnce();
		TestEqual(TEXT("Without the settle row the slot keeps its row"), Again->Systems[1].ActiveMegaproject, CosmicRows::ProjectSettle);
		TestEqual(TEXT("and its progress"), Again->Systems[1].MegaprojectProgress, 2.f);
		TestFalse(TEXT("and the system is still being settled"), CosmicState::IsSettled(Again->Systems[1]));
	}

	// What the guards read: an AI rate that is not above 0 or is not a number as 1 (one error line a tick), a cost below 0
	// as 0, a required progress below 1 or not a number as 1.
	{
		FMegaprojectDef Reading;
		Reading.RequiredProgress = 0.5f;
		TestEqual(TEXT("A required progress below 1 reads as 1"), CosmicMegaprojects::RequiredProgressOf(Reading), 1.f);
		Reading.RequiredProgress = std::numeric_limits<float>::quiet_NaN();
		TestEqual(TEXT("A required progress that is not a number reads as 1"), CosmicMegaprojects::RequiredProgressOf(Reading), 1.f);
		Reading.RequiredProgress = 600.f;
		TestEqual(TEXT("Any other reads as it is"), CosmicMegaprojects::RequiredProgressOf(Reading), 600.f);
		AddExpectedError(TEXT("AIAscensionSpeedMultiplier"), EAutomationExpectedErrorFlags::Contains, 2);
		for (const float BadRate : { 0.f, std::numeric_limits<float>::quiet_NaN() })
		{
			FCosmicRulesDef Bad = Knobs(BaseRules);
			Bad.AIAscensionSpeedMultiplier = BadRate;
			FGalaxyState* Again = LayBoard(*Sim, TablesWith(Original, Bad, nullptr));
			if (!TestNotNull(TEXT("The board of a bad rate is laid"), Again))
			{
				return false;
			}
			Again->Systems[6].ActiveMegaproject = Rebuild;
			Sim->StepOnce();
			TestEqual(FString::Printf(TEXT("An AI rate of %f progresses by 1"), BadRate), Again->Systems[6].MegaprojectProgress, 1.f);
		}
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the bad rows is laid"), Again))
		{
			return false;
		}
		Again->Systems[0].ActiveMegaproject = Refund;
		Again->Systems[1].ActiveMegaproject = Endless;
		const FGalaxyState Guarded = *Again;
		Sim->StepOnce();
		TestEqual(TEXT("A cost below 0 progresses for nothing"), Again->Systems[0].MegaprojectProgress, 1.f);
		// Endless costs Energy 1; Refund costs none.
		TestEqual(TEXT("and gives nothing back"), Again->Civs[0].Stockpile.Energy, Guarded.Civs[0].Stockpile.Energy + Again->Civs[0].IncomePerTick.Energy - 1.f, 0.01f);
		TestTrue(TEXT("A required progress that is not a number completes as 1"), RecordOf(Again->Systems[1], Endless) != nullptr);
		TestTrue(TEXT("and empties its slot"), Again->Systems[1].ActiveMegaproject.IsNone());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicStartMegaprojectTest, "Cosmic_Ascension.Sim.StartMegaproject", CosmicTests::Flags)
bool FCosmicStartMegaprojectTest::RunTest(const FString& Parameters)
{
	using namespace CosmicAscensionTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	FGalaxyState* Laid = LayBoard(*Sim, TablesWith(Original, Knobs(BaseRules), nullptr));
	if (!TestNotNull(TEXT("The board of the command is laid"), Laid))
	{
		return false;
	}
	FGalaxyState& State = *Laid;
	const FName Home = State.Systems[0].SystemID;
	// The live stream itself: State.RngState is only its copy at the end of a tick, which no command writes.
	const int32 SeedBefore = Sim->GetRng().GetCurrentSeed();

	TestTrue(TEXT("The ruler starts a construction on a system of its people"), Role->StartMegaproject(Home, Build));
	TestEqual(TEXT("in the construction slot"), State.Systems[0].ActiveMegaproject, Build);
	TestFalse(TEXT("The same row is not started again"), Role->StartMegaproject(Home, Build));
	State.Systems[0].MegaprojectProgress = 2.f;
	TestTrue(TEXT("Another construction takes the slot over"), Role->StartMegaproject(Home, Rebuild));
	TestEqual(TEXT("and runs there"), State.Systems[0].ActiveMegaproject, Rebuild);
	TestEqual(TEXT("from 0: the progress of the first is lost"), State.Systems[0].MegaprojectProgress, 0.f);
	TestFalse(TEXT("A system of another people is refused"), Role->StartMegaproject(State.Systems[6].SystemID, Build));
	TestFalse(TEXT("A row the table does not hold is refused"), Role->StartMegaproject(Home, FName(TEXT("project_test_gone"))));
	TestFalse(TEXT("A row above the people's tier is refused"), Role->StartMegaproject(Home, High));
	PlaceByHand(State, 1, TEXT("Hazard.AIAwakening"), 50);
	TestFalse(TEXT("An answer above the people's tier is refused"), Role->StartMegaproject(State.Systems[1].SystemID, HighAnswer));
	State.Civs[0].Tier = EAscensionTier::Tier2_Stellar;
	TestTrue(TEXT("and taken at its tier"), Role->StartMegaproject(Home, High));
	TestTrue(TEXT("and so is the answer"), Role->StartMegaproject(State.Systems[1].SystemID, HighAnswer));
	CosmicHazards::EmptyHazardSlot(State.Systems[1]);
	TestFalse(TEXT("The command does not settle a system the people already owns"), Role->StartMegaproject(State.Systems[1].SystemID, CosmicRows::ProjectSettle));
	TestTrue(TEXT("Start would take a construction on that system"), CosmicMegaprojects::CanStart(State, Sim->GetStateIndex(), Sim->GetTables(), 1, Build));
	TestFalse(TEXT("but never project_settle: a settlement starts through Settle alone"), CosmicMegaprojects::CanStart(State, Sim->GetStateIndex(), Sim->GetTables(), 1, CosmicRows::ProjectSettle));
	TestFalse(TEXT("An answer without its hazard is refused"), Role->StartMegaproject(State.Systems[1].SystemID, Answer));
	PlaceByHand(State, 1, TEXT("Hazard.Rebellion"), 50);
	TestFalse(TEXT("An answer to another hazard is refused"), Role->StartMegaproject(State.Systems[1].SystemID, Answer));
	PlaceByHand(State, 1, TEXT("Hazard.Plague"), 50);
	TestTrue(TEXT("An answer to the hazard that stands there is taken"), Role->StartMegaproject(State.Systems[1].SystemID, Answer));
	TestEqual(TEXT("in the response slot"), State.Systems[1].ResponseMegaproject, Answer);
	TestTrue(TEXT("leaving the construction slot alone"), State.Systems[1].ActiveMegaproject.IsNone());
	TestFalse(TEXT("A full response slot takes no second answer"), Role->StartMegaproject(State.Systems[1].SystemID, Second));
	// The key starts the first of what the AI would start: the answer of the capital's hazard.
	PlaceByHand(State, 0, TEXT("Hazard.Plague"), 50);
	TestTrue(TEXT("The key starts a project"), Role->StartNextMegaproject());
	TestEqual(TEXT("the answer of the capital's hazard"), State.Systems[0].ResponseMegaproject, Answer);
	State.Systems[1].ActiveMegaproject = CosmicRows::ProjectSettle;
	TestFalse(TEXT("A system being settled takes no project"), Role->StartMegaproject(State.Systems[1].SystemID, Rebuild));
	State.Systems[1].ActiveMegaproject = NAME_None;
	const FGameplayTag Star = State.Systems[1].StarClass;
	State.Systems[1].StarClass = FGameplayTag();
	TestFalse(TEXT("A dead node takes no project"), Role->StartMegaproject(State.Systems[1].SystemID, Rebuild));
	State.Systems[1].StarClass = Star;
	State.Player.Role = EPlayerRole::Wanderer;
	TestFalse(TEXT("A wanderer gives no command"), Role->StartMegaproject(State.Systems[1].SystemID, Rebuild));
	State.Player.Role = EPlayerRole::Ruler;
	State.Calling.LifeState = ELifeState::Interregnum;
	TestFalse(TEXT("Nobody gives a command during the interregnum"), Role->StartMegaproject(State.Systems[1].SystemID, Rebuild));
	TestFalse(TEXT("nor presses the project key"), Role->StartNextMegaproject());
	State.Calling.LifeState = ELifeState::Embodied;
	TestEqual(TEXT("A command draws nothing"), Sim->GetRng().GetCurrentSeed(), SeedBefore);

	// A handler of a Core Logic delegate gives no command.
	{
		const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
		Sim->SetBroadcastEnabled(true);
		ON_SCOPE_EXIT { Sim->SetBroadcastEnabled(bWasBroadcasting); };
		State.Systems[0].ActiveMegaproject = Build;
		State.Systems[0].MegaprojectProgress = 2.f;
		State.Civs[0].Tier = EAscensionTier::Tier1_Planetary;
		FProbe Probe(*Sim);
		bool bTried = false;
		bool bStarted = true;
		Probe.Probe->OnLog = [&](const FCosmicLogEntry& Entry)
		{
			if (bTried || Entry.SourceRow != Build)
			{
				return;
			}
			bTried = true;
			bStarted = Role->StartMegaproject(State.Systems[0].SystemID, Rebuild) || Role->StartNextMegaproject();
		};
		AddExpectedError(TEXT("StartMegaproject called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		AddExpectedError(TEXT("StartNextMegaproject called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		Sim->StepOnce();
		TestTrue(TEXT("The handler of the completion line ran"), bTried);
		TestFalse(TEXT("and started nothing"), bStarted);
		TestTrue(TEXT("The construction slot stays empty after the completion"), State.Systems[0].ActiveMegaproject.IsNone());
	}
	return true;
}

namespace CosmicAscensionTests
{
	// The rule of guideline 5.7 as Phase 2 wrote it (the body of CosmicMegaprojects::CanStart at 75af02b): change both
	// together. StartBlockOf asks the same refusals in another order and must take exactly what this takes.
	bool ReferenceCanStart(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 SystemIndex, FName RowName)
	{
		const FMegaprojectDef* Row = Tables.FindMegaproject(RowName);
		if (!Row || RowName == CosmicRows::ProjectSettle || !State.Systems.IsValidIndex(SystemIndex))
		{
			return false;
		}
		const FStarSystemState& System = State.Systems[SystemIndex];
		const int32 OwnerIndex = Index.FindCiv(State, System.OwnerCivID);
		if (CosmicHazards::IsDeadNode(System) || !CosmicState::IsSettled(System) || !State.Civs.IsValidIndex(OwnerIndex))
		{
			return false;
		}
		if (static_cast<int32>(State.Civs[OwnerIndex].Tier) < static_cast<int32>(Row->RequiredTier))
		{
			return false;
		}
		if (Tables.IsHazardAnswer(RowName))
		{
			const FHazardDef* Hazard = Tables.FindHazardByTag(System.ActiveHazard);
			return Hazard && Hazard->RulerResolutions.Contains(RowName) && System.ResponseMegaproject.IsNone();
		}
		return System.ActiveMegaproject != RowName;
	}

	// For every system from -1 to Systems.Num() and every row of the tables plus NAME_None and a row no table holds: how
	// often the reason's None and the reference disagree, how often CanStart and the reason disagree, and how many pairs
	// the reference took and refused (the control group: a sweep that visits nothing, or tables that did not take, would
	// disagree nowhere).
	struct FSweep
	{
		int32 ReasonDisagrees = 0;
		int32 CanStartDisagrees = 0;
		int32 Taken = 0;
		int32 Refused = 0;
	};

	FSweep SweepStarts(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables)
	{
		FSweep Sweep;
		TArray<FName> Rows = Tables.Megaprojects ? Tables.Megaprojects->GetRowNames() : TArray<FName>();
		Rows.Add(NAME_None);
		Rows.Add(FName(TEXT("project_test_gone")));
		for (int32 SystemIndex = -1; SystemIndex <= State.Systems.Num(); ++SystemIndex)
		{
			for (const FName Row : Rows)
			{
				const bool bReference = ReferenceCanStart(State, Index, Tables, SystemIndex, Row);
				const bool bNone = CosmicMegaprojects::StartBlockOf(State, Index, Tables, SystemIndex, Row) == CosmicMegaprojects::ECosmicStartBlock::None;
				Sweep.ReasonDisagrees += bNone == bReference ? 0 : 1;
				Sweep.CanStartDisagrees += CosmicMegaprojects::CanStart(State, Index, Tables, SystemIndex, Row) == bNone ? 0 : 1;
				Sweep.Taken += bReference ? 1 : 0;
				Sweep.Refused += bReference ? 0 : 1;
			}
		}
		return Sweep;
	}
}

// Why Start refuses a row (the system panel's reasons, Phase 3 item 12): one case per reason on the projects' board, the
// order where two stand at once (the hazard before the rung, the rung before the slot), and on nine states - the board in
// six forms and the game's galaxy at three rungs - the reason's None is exactly the Phase 2 rule (a copy of its body) and
// CanStart. The order names the reason and decides nothing: no start of the command or of the AI moves.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicStartBlockReasonsTest, "Cosmic_Ascension.Sim.StartBlockReasons", CosmicTests::Flags)
bool FCosmicStartBlockReasonsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicAscensionTests;
	using CosmicMegaprojects::ECosmicStartBlock;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FCosmicDataTables Tables = TablesWith(Original, Knobs(BaseRules), nullptr);
	FGalaxyState* Laid = LayBoard(*Sim, Tables);
	if (!TestNotNull(TEXT("The board of the reasons is laid"), Laid))
	{
		return false;
	}
	FGalaxyState& State = *Laid;
	auto Block = [&](int32 SystemIndex, FName Row)
	{
		return CosmicMegaprojects::StartBlockOf(State, Sim->GetStateIndex(), Sim->GetTables(), SystemIndex, Row);
	};
	auto Is = [&](const TCHAR* What, int32 SystemIndex, FName Row, ECosmicStartBlock Expected)
	{
		TestEqual(What, static_cast<int32>(Block(SystemIndex, Row)), static_cast<int32>(Expected));
	};
	// Read only: the hashes and the live stream before every call of this test's cases.
	const uint32 StateHash = Sim->ComputeStateHash();
	const uint32 WorldHash = Sim->ComputeWorldHash();
	const int32 SeedBefore = Sim->GetRng().GetCurrentSeed();

	// ---- One case per reason ----
	Is(TEXT("A construction the people can start has no reason"), 0, Build, ECosmicStartBlock::None);
	TestTrue(TEXT("CanStart is the reason's None"), CosmicMegaprojects::CanStart(State, Sim->GetStateIndex(), Sim->GetTables(), 0, Build));
	Is(TEXT("A row no table holds is NoRow"), 0, FName(TEXT("project_test_gone")), ECosmicStartBlock::NoRow);
	Is(TEXT("NAME_None is NoRow"), 0, NAME_None, ECosmicStartBlock::NoRow);
	Is(TEXT("project_settle is the settle row"), 0, CosmicRows::ProjectSettle, ECosmicStartBlock::SettleRow);
	Is(TEXT("An index below the systems is NoSystem"), -1, Build, ECosmicStartBlock::NoSystem);
	Is(TEXT("An index past the systems is NoSystem"), State.Systems.Num(), Build, ECosmicStartBlock::NoSystem);
	{
		const FGameplayTag OwnedClass = State.Systems[1].StarClass;
		const FGameplayTag WildClass = State.Systems[2].StarClass;
		State.Systems[1].StarClass = FGameplayTag();
		State.Systems[2].StarClass = FGameplayTag();
		Is(TEXT("An owned dead node is Dead"), 1, Build, ECosmicStartBlock::Dead);
		Is(TEXT("An unowned dead node is Dead too, not NotSettled: the node before the owner"), 2, Build, ECosmicStartBlock::Dead);
		State.Systems[1].StarClass = OwnedClass;
		State.Systems[2].StarClass = WildClass;
	}
	Is(TEXT("A wild system is not settled"), 3, Build, ECosmicStartBlock::NotSettled);
	State.Systems[1].ActiveMegaproject = CosmicRows::ProjectSettle;
	Is(TEXT("A settlement under way is not settled"), 1, Build, ECosmicStartBlock::NotSettled);
	State.Systems[1].ActiveMegaproject = NAME_None;
	{
		const FName Owner = State.Systems[5].OwnerCivID;
		State.Systems[5].OwnerCivID = FName(TEXT("civ_99"));
		Is(TEXT("An owner that names no people is NoOwner"), 5, Build, ECosmicStartBlock::NoOwner);
		State.Systems[5].OwnerCivID = Owner;
	}
	Is(TEXT("An answer with no hazard standing there is NoHazard"), 0, Answer, ECosmicStartBlock::NoHazard);
	PlaceByHand(State, 0, TEXT("Hazard.Rebellion"), 50);
	Is(TEXT("An answer the standing hazard's row does not list is OtherHazard"), 0, Answer, ECosmicStartBlock::OtherHazard);
	CosmicHazards::EmptyHazardSlot(State.Systems[0]);

	// ---- The hazard before the rung, the rung before the slot ----
	Is(TEXT("A second-rung answer on a quiet first-rung system is NoHazard, not RowTier"), 0, HighAnswer, ECosmicStartBlock::NoHazard);
	PlaceByHand(State, 0, TEXT("Hazard.Plague"), 50);
	Is(TEXT("A second-rung answer under a hazard that does not list it is OtherHazard, not RowTier"), 0, HighAnswer, ECosmicStartBlock::OtherHazard);
	CosmicHazards::EmptyHazardSlot(State.Systems[0]);
	Is(TEXT("A second-rung construction on the first rung is RowTier"), 0, High, ECosmicStartBlock::RowTier);
	PlaceByHand(State, 0, TEXT("Hazard.AIAwakening"), 50);
	Is(TEXT("A second-rung answer under its own hazard is RowTier"), 0, HighAnswer, ECosmicStartBlock::RowTier);
	State.Systems[0].ResponseMegaproject = Answer;
	Is(TEXT("with the response slot full it is still RowTier: the rung before the slot"), 0, HighAnswer, ECosmicStartBlock::RowTier);
	CosmicHazards::EmptyHazardSlot(State.Systems[0]);

	// ---- The slots ----
	PlaceByHand(State, 1, TEXT("Hazard.Plague"), 50);
	State.Systems[1].ResponseMegaproject = Answer;
	Is(TEXT("The answer running in the response slot is Answering"), 1, Answer, ECosmicStartBlock::Answering);
	Is(TEXT("Another answer of that hazard is ResponseBusy"), 1, Second, ECosmicStartBlock::ResponseBusy);
	CosmicHazards::EmptyHazardSlot(State.Systems[1]);
	State.Systems[0].ActiveMegaproject = Build;
	Is(TEXT("A construction running in the construction slot is Running"), 0, Build, ECosmicStartBlock::Running);
	Is(TEXT("Another construction over it has no reason: the replacement rule stays"), 0, Rebuild, ECosmicStartBlock::None);
	Is(TEXT("A construction no stockpile pays has no reason: the cost is no reason"), 0, Pricey, ECosmicStartBlock::None);
	State.Systems[0].ActiveMegaproject = NAME_None;

	// ---- Any owner: the rule reads the owner, not the vessel ----
	Is(TEXT("Another people's system takes a construction too"), 6, Build, ECosmicStartBlock::None);
	Is(TEXT("and refuses a row above that people's rung"), 6, High, ECosmicStartBlock::RowTier);
	State.Systems[6].ActiveMegaproject = Build;
	Is(TEXT("and says Running for its own construction"), 6, Build, ECosmicStartBlock::Running);
	State.Systems[6].ActiveMegaproject = NAME_None;
	State.Systems[6].MegaprojectProgress = 0.f;

	TestEqual(TEXT("The reasons change no state"), Sim->ComputeStateHash(), StateHash);
	TestEqual(TEXT("and no world"), Sim->ComputeWorldHash(), WorldHash);
	TestEqual(TEXT("and draw nothing"), Sim->GetRng().GetCurrentSeed(), SeedBefore);

	// ---- The sweep: nine states ----
	auto Swept = [&](const TCHAR* Name)
	{
		const FSweep Sweep = SweepStarts(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables());
		TestEqual(*FString::Printf(TEXT("On %s, the reason is None exactly where the Phase 2 rule takes the row"), Name), Sweep.ReasonDisagrees, 0);
		TestEqual(*FString::Printf(TEXT("On %s, CanStart agrees with the reason"), Name), Sweep.CanStartDisagrees, 0);
		TestTrue(*FString::Printf(TEXT("(on %s the rule takes a row somewhere)"), Name), Sweep.Taken > 0);
		TestTrue(*FString::Printf(TEXT("(on %s the rule refuses a row somewhere)"), Name), Sweep.Refused > 0);
	};
	Swept(TEXT("the laid board"));
	PlaceByHand(State, 0, TEXT("Hazard.Plague"), 50);
	PlaceByHand(State, 1, TEXT("Hazard.AIAwakening"), 50);
	PlaceByHand(State, 6, TEXT("Hazard.Rebellion"), 50);
	PlaceByHand(State, 11, TEXT("Hazard.Plague"), 50);
	Swept(TEXT("the board with hazards"));
	State.Systems[0].ResponseMegaproject = Second;
	State.Systems[1].ResponseMegaproject = Answer;
	State.Systems[11].ResponseMegaproject = Costly;
	State.Systems[0].ActiveMegaproject = Rebuild;
	Swept(TEXT("the board with its slots full"));
	{
		const FGameplayTag StarClass = State.Systems[7].StarClass;
		State.Systems[7].StarClass = FGameplayTag();
		Swept(TEXT("the board with a dead node"));
		State.Systems[7].StarClass = StarClass;
	}
	State.Systems[1].ActiveMegaproject = CosmicRows::ProjectSettle;
	Swept(TEXT("the board with a settlement under way"));
	State.Systems[1].ActiveMegaproject = NAME_None;
	State.Civs[0].Tier = EAscensionTier::Tier2_Stellar;
	Swept(TEXT("the board with people 0 on the second rung"));

	// The game's tables on the game's galaxy, some way in - the inherited edict revoked first, as a player does (B2: kept,
	// it ends in a rebellion and a deposition before tick 2,000).
	Sim->SetTables(Original);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	CosmicTests::RevokeInheritedEdict(*Sim);
	Sim->RunHeadless(2000);
	FGalaxyState& Game = Sim->GetMutableState();
	const int32 Ruled = Sim->GetStateIndex().FindCiv(Game, Game.Player.RuledCivID);
	const int32 Capital = Game.Civs.IsValidIndex(Ruled) ? Sim->GetStateIndex().FindSystem(Game, Game.Civs[Ruled].HomeSystemID) : INDEX_NONE;
	if (TestTrue(TEXT("(the game's galaxy has a ruled people with its capital)"), Game.Systems.IsValidIndex(Capital)))
	{
		CosmicHazards::EmptyHazardSlot(Game.Systems[Capital]);
		PlaceByHand(Game, Capital, TEXT("Hazard.PreSupernova"), 300);
		for (const EAscensionTier Tier : { EAscensionTier::Tier1_Planetary, EAscensionTier::Tier2_Stellar, EAscensionTier::Tier3_Causal })
		{
			Game.Civs[Ruled].Tier = Tier;
			Swept(*FString::Printf(TEXT("the game's galaxy at rung %d"), static_cast<int32>(Tier) + 1));
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicCivAIProjectsTest, "Cosmic_Ascension.Sim.CivAIProjects", CosmicTests::Flags)
bool FCosmicCivAIProjectsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicAscensionTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	// The feat of the first rung names Build (and a research no people of the board has); a wanderer's feat names
	// Rebuild, a feat of the second rung names High.
	UDataTable* Feats = NewTable(FAscensionFeatDef::StaticStruct());
	Feats->AddRow(FName(TEXT("feat_test_build")), FeatRow(EAscensionTier::Tier1_Planetary, EAscensionTier::Tier2_Stellar,
		{ Cond(EConditionType::MegaprojectComplete, Build), Cond(EConditionType::CivResearchAtLeast, FName(TEXT("Physics")), 0.5f) }, TEXT("build")));
	FAscensionFeatDef Wanderer = FeatRow(EAscensionTier::Tier1_Planetary, EAscensionTier::Tier2_Stellar, { Cond(EConditionType::MegaprojectComplete, Rebuild) }, TEXT("walk"));
	Wanderer.Path = EPlayerRole::Wanderer;
	Feats->AddRow(FName(TEXT("feat_test_a_walk")), Wanderer);
	Feats->AddRow(FName(TEXT("feat_test_high")), FeatRow(EAscensionTier::Tier2_Stellar, EAscensionTier::Tier3_Causal, { Cond(EConditionType::MegaprojectComplete, High) }, TEXT("high")));
	const FCosmicDataTables Tables = TablesWith(Original, Knobs(BaseRules), Feats);

	FGalaxyState* Laid = LayBoard(*Sim, Tables);
	if (!TestNotNull(TEXT("The board of the AI is laid"), Laid))
	{
		return false;
	}
	FGalaxyState& State = *Laid;
	PlaceByHand(State, 7, TEXT("Hazard.Plague"), 50);
	PlaceByHand(State, 1, TEXT("Hazard.Plague"), 50);
	Record(State.Systems[11], Build, 5);
	// The first answer of the awakening is above people 3's tier.
	PlaceByHand(State, 11, TEXT("Hazard.AIAwakening"), 50);
	State.Civs[2].Tier = EAscensionTier::Tier2_Stellar;
	const FGalaxyState Before = State;
	Sim->StepOnce();
	TestEqual(TEXT("An AI people builds what its next feat names, on its capital"), State.Systems[6].ActiveMegaproject, Build);
	TestEqual(TEXT("An AI people answers the hazard of its system"), State.Systems[7].ResponseMegaproject, Answer);
	TestEqual(TEXT("and builds for the feat of its own rung"), State.Systems[7].ActiveMegaproject, High);
	TestTrue(TEXT("A project a people completed is not built again"), State.Systems[11].ActiveMegaproject.IsNone());
	TestEqual(TEXT("An AI people takes the first answer its tier allows"), State.Systems[11].ResponseMegaproject, Answer);
	TestTrue(TEXT("The AI does not touch the people the player rules: no construction"), State.Systems[0].ActiveMegaproject.IsNone());
	TestTrue(TEXT("and no answer"), State.Systems[1].ResponseMegaproject.IsNone());

	// The AI's starts draw nothing: without the rows that make it start, the tick draws as much.
	{
		const FGalaxyState After = State;
		Sim->SetTables(TablesWith(Original, Knobs(BaseRules), nullptr));
		Sim->LoadState(Before);
		Sim->StepOnce();
		TestEqual(TEXT("The AI's starts draw nothing from the sim stream"), Sim->GetState().RngState, After.RngState);
		Sim->SetTables(Tables);
		// Nor does the stage draw once per people: the same tick with every people marked as ruled by the player (so that
		// the stage looks at none) draws as much.
		FGalaxyState NoAI = Before;
		for (FCivilizationState& Civ : NoAI.Civs)
		{
			Civ.bIsPlayerRuled = true;
		}
		Sim->LoadState(NoAI);
		Sim->StepOnce();
		TestEqual(TEXT("Beyond the settlement number (no people may settle under the test's realm cap of one), the AI stage draws nothing per people it looks at"), Sim->GetState().RngState, After.RngState);
		Sim->LoadState(After);
	}

	// A feat that names project_settle builds nothing: Start never takes it (a settlement starts through Settle alone).
	{
		UDataTable* SettleFeat = NewTable(FAscensionFeatDef::StaticStruct());
		SettleFeat->AddRow(FName(TEXT("feat_test_settle")), FeatRow(EAscensionTier::Tier1_Planetary, EAscensionTier::Tier2_Stellar,
			{ Cond(EConditionType::MegaprojectComplete, CosmicRows::ProjectSettle) }, TEXT("settle")));
		FGalaxyState* Again = LayBoard(*Sim, TablesWith(Original, Knobs(BaseRules), SettleFeat));
		if (!TestNotNull(TEXT("The board of the settle feat is laid"), Again))
		{
			return false;
		}
		// People 1 also holds 5, settled: a settlement on its capital would stay in the slot to be seen, not go with the
		// people's last settled system in the crossing stage.
		CosmicTests::Give(*Again, 1, { 5 });
		Sim->StepOnce();
		TestTrue(TEXT("An AI capital takes no project_settle into its construction slot"), Again->Systems[6].ActiveMegaproject.IsNone());
		TestTrue(TEXT("and stays settled"), CosmicState::IsSettled(Again->Systems[6]));
		TestEqual(TEXT("and stays its people's"), Again->Systems[6].OwnerCivID, Again->Civs[1].CivID);
		Sim->SetTables(Tables);
	}

	// An AI people never takes over its own construction slot.
	{
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the busy slot is laid"), Again))
		{
			return false;
		}
		Again->Systems[6].ActiveMegaproject = Rebuild;
		Sim->StepOnce();
		TestEqual(TEXT("A busy construction slot keeps its row"), Again->Systems[6].ActiveMegaproject, Rebuild);
	}

	// The project key starts for the people the player rules what the AI would start first, counting from the vessel's
	// birth: a completion before it does not count, as in the player's judgment.
	{
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the key is laid"), Again))
		{
			return false;
		}
		PlaceByHand(*Again, 1, TEXT("Hazard.Plague"), 50);
		Again->Calling.BornTick = 10;
		Record(Again->Systems[0], Build, 5);
		const TArray<TPair<int32, FName>> FromStart = CosmicMegaprojects::Choices(*Again, Sim->GetStateIndex(), Sim->GetTables(), 0, 0);
		TestEqual(TEXT("Counted from the beginning the completion stands: only the answer"), FromStart.Num(), 1);
		const TArray<TPair<int32, FName>> Choices = CosmicMegaprojects::Choices(*Again, Sim->GetStateIndex(), Sim->GetTables(), 0, Again->Calling.BornTick);
		if (TestEqual(TEXT("Two choices from the birth: the answer, then the construction"), Choices.Num(), 2))
		{
			TestEqual(TEXT("The answer comes first"), Choices[0].Value, Answer);
			TestEqual(TEXT("on its system"), Choices[0].Key, 1);
			TestEqual(TEXT("then the construction"), Choices[1].Value, Build);
			TestEqual(TEXT("on the capital"), Choices[1].Key, 0);
		}
		TestTrue(TEXT("The key starts a project"), Role->StartNextMegaproject());
		TestEqual(TEXT("the first choice"), Again->Systems[1].ResponseMegaproject, Answer);
		TestTrue(TEXT("and only that one"), Again->Systems[0].ActiveMegaproject.IsNone());
		TestTrue(TEXT("A second press starts the next"), Role->StartNextMegaproject());
		TestEqual(TEXT("the construction"), Again->Systems[0].ActiveMegaproject, Build);
		TestFalse(TEXT("With nothing left the key starts nothing"), Role->StartNextMegaproject());
	}

	// Only the systems a people owns count, and a construction running on one of them is not started again.
	{
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the owned systems is laid"), Again))
		{
			return false;
		}
		// People 1 lists system 11, which people 3 owns, with a hazard and a completion of Build on it.
		Again->Civs[1].OwnedSystems.Add(Again->Systems[11].SystemID);
		PlaceByHand(*Again, 11, TEXT("Hazard.Plague"), 50);
		Record(Again->Systems[11], Build, 5);
		const TArray<TPair<int32, FName>> Listed = CosmicMegaprojects::Choices(*Again, Sim->GetStateIndex(), Sim->GetTables(), 1, 0);
		if (TestEqual(TEXT("A system a people lists but does not own gives it nothing to answer"), Listed.Num(), 1))
		{
			TestEqual(TEXT("and its completion counts for nothing: the feat's construction"), Listed[0].Value, Build);
			TestEqual(TEXT("on the capital"), Listed[0].Key, 6);
		}
		// The vessel's people builds Build on its second system.
		Again->Systems[1].ActiveMegaproject = Build;
		TestEqual(TEXT("A construction running on another system of the people is not offered again"),
			CosmicMegaprojects::Choices(*Again, Sim->GetStateIndex(), Sim->GetTables(), 0, Again->Calling.BornTick).Num(), 0);
		TestFalse(TEXT("nor started by the key"), Role->StartNextMegaproject());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicFeatsEvaluateTest, "Cosmic_Ascension.Feats.Evaluate", CosmicTests::Flags)
bool FCosmicFeatsEvaluateTest::RunTest(const FString& Parameters)
{
	using namespace CosmicAscensionTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UAscensionSubsystem* Ascension = Sim->GetGameInstance()->GetSubsystem<UAscensionSubsystem>();
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	// Two judgments are refused below: one from the handler of the player's ascension, one from the handler of an AI
	// people's ascension line. (An expected error is kept per pattern: a second call with the same one would replace it.)
	AddExpectedError(TEXT("EvaluateFeats called from a handler"), EAutomationExpectedErrorFlags::Contains, 2);

	// Two feats of the first rung on the same completion (the first by name is the one), a second rung whose price places
	// a plague, a split feat (its two conditions on one system), a third rung and a fourth.
	const FName FeatA(TEXT("feat_test_a"));
	const FName FeatB(TEXT("feat_test_b"));
	const FName FeatSecond(TEXT("feat_test_second"));
	const FName FeatThird(TEXT("feat_test_third"));
	const FName FeatFourth(TEXT("feat_test_fourth"));
	UDataTable* Feats = NewTable(FAscensionFeatDef::StaticStruct());
	FAscensionFeatDef First = FeatRow(EAscensionTier::Tier1_Planetary, EAscensionTier::Tier2_Stellar, { Cond(EConditionType::MegaprojectComplete, Build), Cond(EConditionType::SystemHasHazard, NAME_None, 0.f, TEXT("Hazard.Plague")) }, TEXT("first"));
	// Its price also resolves the plague of its system, so that a line follows the player's climb.
	First.AscensionCost = { Eff(EEffectType::AddResource, nullptr, FName(TEXT("Causality")), 7.f), Eff(EEffectType::ResolveHazard) };
	First.GrantedPowers.AddTag(Tag(TEXT("Way.Received")));
	// B goes into the table first: the judgment reads the rows by name, not in the table's order.
	Feats->AddRow(FeatB, FeatRow(EAscensionTier::Tier1_Planetary, EAscensionTier::Tier2_Stellar, { Cond(EConditionType::MegaprojectComplete, Build), Cond(EConditionType::SystemHasHazard, NAME_None, 0.f, TEXT("Hazard.Plague")) }, TEXT("other")));
	Feats->AddRow(FeatA, First);
	FAscensionFeatDef SecondRung = FeatRow(EAscensionTier::Tier2_Stellar, EAscensionTier::Tier3_Causal, { Cond(EConditionType::MegaprojectComplete, Build) }, TEXT("second"));
	SecondRung.AscensionCost = { Eff(EEffectType::SpawnHazard, TEXT("Hazard.Plague")) };
	Feats->AddRow(FeatSecond, SecondRung);
	Feats->AddRow(FeatThird, FeatRow(EAscensionTier::Tier3_Causal, EAscensionTier::Tier4_Transcendent, { Cond(EConditionType::MegaprojectComplete, Rebuild) }, TEXT("third")));
	FAscensionFeatDef Walk = FeatRow(EAscensionTier::Tier4_Transcendent, EAscensionTier::Tier5_EternalOrder, { Cond(EConditionType::MegaprojectComplete, Build) }, TEXT("walk"));
	Walk.Path = EPlayerRole::Wanderer;
	Feats->AddRow(FeatFourth, Walk);
	const FCosmicDataTables Tables = TablesWith(Original, Knobs(BaseRules), Feats);

	FGalaxyState* Laid = LayBoard(*Sim, Tables);
	if (!TestNotNull(TEXT("The board of the feats is laid"), Laid))
	{
		return false;
	}
	FGalaxyState& State = *Laid;
	// The vessel was born at tick 50. A completion before it does not count for the player.
	State.Tick = 60;
	State.Calling.BornTick = 50;
	Record(State.Systems[0], Build, 49);
	PlaceByHand(State, 0, TEXT("Hazard.Plague"), 500);
	// A split feat: people 1 completed Build on 6 and has the plague on 5 (its second system).
	CosmicTests::Give(State, 1, { 5 });
	Record(State.Systems[6], Build, 10);
	PlaceByHand(State, 5, TEXT("Hazard.Plague"), 500);
	// People 2 reaches the third rung by a completion on its second system, 8 (its capital stays 7); it has a waiting rebel.
	State.Civs[2].Tier = EAscensionTier::Tier2_Stellar;
	CosmicTests::Give(State, 2, { 8 });
	Record(State.Systems[8], Build, 10);
	// People 3, at the third rung with an open rebel, reaches the fourth.
	State.Civs[3].Tier = EAscensionTier::Tier3_Causal;
	Record(State.Systems[11], Rebuild, 10);
	FNotableRecord& Rebel = State.Civs[3].Notables.AddDefaulted_GetRef();
	Rebel.Role = Tag(TEXT("Notable.Rebel"));
	Rebel.FirstTick = 20;
	Sim->RebuildStateIndex();

	FProbe Probe(*Sim);
	TArray<TTuple<EAscensionTier, FName, EPlayerRole>> Heard;
	auto Listen = [&Heard](EAscensionTier NewTier, FName FeatRow, EPlayerRole Path) { Heard.Emplace(NewTier, FeatRow, Path); };
	Probe.Probe->OnTier = Listen;
	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	Sim->SetBroadcastEnabled(true);
	ON_SCOPE_EXIT { Sim->SetBroadcastEnabled(bWasBroadcasting); };
	const FGalaxyState Before = State;
	Sim->StepOnce();
	TestEqual(TEXT("A completion before the vessel's birth does not lift the player's people"), State.Civs[0].Tier, EAscensionTier::Tier1_Planetary);
	TestEqual(TEXT("Conditions held on two systems lift no people"), State.Civs[1].Tier, EAscensionTier::Tier1_Planetary);
	TestEqual(TEXT("An AI people climbs by the feat of its rung"), State.Civs[2].Tier, EAscensionTier::Tier3_Causal);
	TestEqual(TEXT("An AI people reaches the fourth rung"), State.Civs[3].Tier, EAscensionTier::Tier4_Transcendent);
	const FNotableRecord* Ascendant = State.Civs[2].Notables.FindByPredicate([](const FNotableRecord& Candidate) { return Candidate.Role == Tag(TEXT("Notable.Ascendant")); });
	if (TestNotNull(TEXT("An AI people that reaches the third rung gets the one who crossed first"), Ascendant))
	{
		TestEqual(TEXT("from the feat row"), Ascendant->SourceRow, FeatSecond);
		TestEqual(TEXT("open"), Ascendant->LastTick, static_cast<int64>(0));
	}
	TestEqual(TEXT("At the fourth rung the open records close in that tick"), State.Civs[3].Notables.Num() > 0 ? State.Civs[3].Notables[0].LastTick : int64(-1), State.Tick);
	TestFalse(TEXT("and no one who crossed first is made there"), State.Civs[3].Notables.ContainsByPredicate([](const FNotableRecord& Candidate) { return Candidate.Role == Tag(TEXT("Notable.Ascendant")); }));
	const int32 Line = FindLine(State, FeatSecond, State.Tick);
	if (TestTrue(TEXT("The ascension is told"), Line != INDEX_NONE))
	{
		TestEqual(TEXT("as a line of Severity 2"), static_cast<int32>(State.Log[Line].Severity), 2);
		TestTrue(TEXT("in Log.Ascension"), State.Log[Line].Category == Tag(TEXT("Log.Ascension")));
		TestEqual(TEXT("about the people"), State.Log[Line].FocusCivID, State.Civs[2].CivID);
		TestEqual(TEXT("and its capital, not the system of the feat"), State.Log[Line].FocusSystemID, State.Systems[7].SystemID);
		ExpectFilled(*this, State.Log[Line], { { State.Civs[2].CivName.ToString(), TEXT("{Civ} names the people that climbed") },
			{ State.Systems[7].DisplayName.ToString(), TEXT("{HomeSystem} names its capital") } });
		// The price placed a plague on the system of the feat: its outbreak line follows the ascension line.
		const int32 Outbreak = FindLine(State, Storm, State.Tick, TEXT("outbreak"));
		if (TestTrue(TEXT("The lines that follow the price are told"), Outbreak != INDEX_NONE))
		{
			TestTrue(TEXT("after the ascension line"), Outbreak > Line);
		}
	}
	TestEqual(TEXT("No ascension of an AI people is heard on OnTierAscended"), Heard.Num(), 0);

	// The draws: one for the name of the one who crossed first, none for the rest of the judgment.
	{
		const FGalaxyState After = State;
		FGalaxyState NoThird = Before;
		NoThird.Civs[2].Tier = EAscensionTier::Tier4_Transcendent;
		NoThird.Civs[2].Notables.Reset();
		Sim->LoadState(NoThird);
		Sim->StepOnce();
		TestEqual(TEXT("The third rung of an AI people draws one number, for the name"), DrawsBetween(Before.RngState, After.RngState) - DrawsBetween(NoThird.RngState, Sim->GetState().RngState), 1);
		Sim->LoadState(After);
	}
	FGalaxyState& Live = Sim->GetMutableState();

	// The boundary tick counts: the vessel's people climbs, its cost applies, the calling keeps the feat. A handler of the
	// player's ascension hears it after its line, and it judges nothing, starts nothing and runs no tick.
	{
		FMegaprojectRecord* Old = Live.Systems[0].CompletedMegaprojects.FindByPredicate([](const FMegaprojectRecord& Candidate) { return Candidate.Row == Build; });
		if (!TestNotNull(TEXT("The completion before the birth is on the record"), Old))
		{
			return false;
		}
		Old->Tick = Live.Calling.BornTick;
		const float CausalityBefore = Live.Civs[0].Stockpile.Causality;
		const int64 TickBefore = Live.Tick;
		Heard.Reset();
		bool bTried = false;
		bool bLineFirst = false;
		bool bFollowFirst = false;
		bool bStarted = true;
		const FName PlayerCapital = Live.Systems[0].SystemID;
		Probe.Probe->OnTier = [&](EAscensionTier NewTier, FName FeatRow, EPlayerRole Path)
		{
			Listen(NewTier, FeatRow, Path);
			if (bTried)
			{
				return;
			}
			bTried = true;
			bLineFirst = FindLine(Sim->GetState(), FeatRow, Sim->GetState().Tick) != INDEX_NONE;
			bFollowFirst = FindLineAbout(Sim->GetState(), Storm, Sim->GetState().Tick, PlayerCapital, TEXT("resolution")) != INDEX_NONE;
			FRandomStream Local(1);
			Ascension->EvaluateFeats(Sim->GetMutableState(), Local);
			bStarted = Role->StartNextMegaproject();
			Sim->StepOnce();
		};
		AddExpectedError(TEXT("StartNextMegaproject called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		AddExpectedError(TEXT("StepOnce called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		Sim->StepOnce();
		Probe.Probe->OnTier = Listen;
		TestEqual(TEXT("A completion at the birth tick lifts the player's people"), Live.Civs[0].Tier, EAscensionTier::Tier2_Stellar);
		TestEqual(TEXT("by the first feat by name"), Live.Calling.CompletedFeats.Num(), 1);
		TestTrue(TEXT("which is feat_test_a"), Live.Calling.CompletedFeats.Contains(FeatA));
		TestTrue(TEXT("The open life records the feat"), !Live.Calling.Lives.IsEmpty() && Live.Calling.Lives.Last().Feats.Contains(FeatA));
		TestEqual(TEXT("The life's peak follows the rung of the people the vessel rules"),
			Live.Calling.Lives.IsEmpty() ? EAscensionTier::Tier1_Planetary : Live.Calling.Lives.Last().PeakTier, EAscensionTier::Tier2_Stellar);
		TestTrue(TEXT("The calling takes the powers of the feat"), Live.Calling.Powers.HasTagExact(Tag(TEXT("Way.Received"))));
		TestEqual(TEXT("The price of the rung is paid (a people of the first rung earns no Causality)"), Live.Civs[0].Stockpile.Causality, CausalityBefore + 7.f, 0.0001f);
		TestEqual(TEXT("The personal tier stays"), Live.Calling.PersonalTier, EAscensionTier::Tier1_Planetary);
		if (TestEqual(TEXT("The player's ascension is heard once"), Heard.Num(), 1))
		{
			TestEqual(TEXT("with the new tier"), Heard[0].Get<0>(), EAscensionTier::Tier2_Stellar);
			TestEqual(TEXT("and the feat row"), Heard[0].Get<1>(), FeatA);
			TestEqual(TEXT("on the ruler's path"), Heard[0].Get<2>(), EPlayerRole::Ruler);
		}
		TestTrue(TEXT("The handler of the player's ascension ran"), bTried);
		TestTrue(TEXT("after the ascension line was told"), bLineFirst);
		TestTrue(TEXT("and after the lines that follow its price (the resolution of the plague)"), bFollowFirst);
		TestFalse(TEXT("and started nothing"), bStarted);
		TestEqual(TEXT("and ran no tick"), Live.Tick, TickBefore + 1);
		Sim->StepOnce();
		TestEqual(TEXT("One rung per tick: the next rung comes on the next tick"), Live.Civs[0].Tier, EAscensionTier::Tier3_Causal);
		TestFalse(TEXT("The player's people gets no one who crossed first at the third rung"), Live.Civs[0].Notables.ContainsByPredicate([](const FNotableRecord& Candidate) { return Candidate.Role == Tag(TEXT("Notable.Ascendant")); }));

		// The fourth rung closes the open records of the player's people as well.
		Record(Live.Systems[0], Rebuild, Live.Tick);
		FNotableRecord& Ruler = Live.Civs[0].Notables.AddDefaulted_GetRef();
		Ruler.Role = Tag(TEXT("Notable.Ruler"));
		Ruler.FirstTick = 55;
		Sim->StepOnce();
		TestEqual(TEXT("The player's people reaches the fourth rung"), Live.Civs[0].Tier, EAscensionTier::Tier4_Transcendent);
		TestEqual(TEXT("and its open records close in that tick"), Live.Civs[0].Notables.Num() > 0 ? Live.Civs[0].Notables[0].LastTick : int64(-1), Live.Tick);
		TestEqual(TEXT("The life's peak is the fourth rung"),
			Live.Calling.Lives.IsEmpty() ? EAscensionTier::Tier1_Planetary : Live.Calling.Lives.Last().PeakTier, EAscensionTier::Tier4_Transcendent);
	}

	// No judgment of the player's people during the interregnum; a wanderer's feat is not judged.
	{
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the interregnum is laid"), Again))
		{
			return false;
		}
		Record(Again->Systems[0], Build, 5);
		PlaceByHand(*Again, 0, TEXT("Hazard.Plague"), 500);
		Again->Calling.LifeState = ELifeState::Interregnum;
		Again->Civs[1].Tier = EAscensionTier::Tier4_Transcendent;
		Record(Again->Systems[6], Build, 5);
		Sim->StepOnce();
		TestEqual(TEXT("The people the player rules is not judged during the interregnum"), Again->Civs[0].Tier, EAscensionTier::Tier1_Planetary);
		TestEqual(TEXT("A wanderer's feat lifts no people"), Again->Civs[1].Tier, EAscensionTier::Tier4_Transcendent);
	}

	// Only the settled systems a people owns hold its feats: a system it lists but another people owns, and a system it
	// is settling, lift it no rung.
	{
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the owned systems is laid"), Again))
		{
			return false;
		}
		// People 1 lists system 11, which people 3 owns (at the fourth rung, where no row of the test judges it).
		Again->Civs[1].OwnedSystems.Add(Again->Systems[11].SystemID);
		Again->Civs[3].Tier = EAscensionTier::Tier4_Transcendent;
		Record(Again->Systems[11], Build, 10);
		PlaceByHand(*Again, 11, TEXT("Hazard.Plague"), 500);
		// People 2 is settling its second system, 8.
		CosmicTests::Give(*Again, 2, { 8 });
		Again->Systems[8].ActiveMegaproject = CosmicRows::ProjectSettle;
		Record(Again->Systems[8], Build, 10);
		PlaceByHand(*Again, 8, TEXT("Hazard.Plague"), 500);
		Sim->StepOnce();
		TestEqual(TEXT("A system a people lists but does not own lifts it no rung"), Again->Civs[1].Tier, EAscensionTier::Tier1_Planetary);
		TestEqual(TEXT("A system a people is settling lifts it no rung"), Again->Civs[2].Tier, EAscensionTier::Tier1_Planetary);
	}

	// A row that is not one rung up, or that holds no condition, is not judged (an error line each time it is looked at),
	// and the judgment goes on to the next row. In name order: a row that skips a rung (Rebuild), a row that stays on its
	// rung (Build), a good row (Build) and a row without a condition.
	{
		UDataTable* BadFeats = NewTable(FAscensionFeatDef::StaticStruct());
		const FName Skip(TEXT("feat_test_0_skip"));
		const FName Same(TEXT("feat_test_1_same"));
		const FName Good(TEXT("feat_test_b_good"));
		BadFeats->AddRow(Skip, FeatRow(EAscensionTier::Tier1_Planetary, EAscensionTier::Tier3_Causal, { Cond(EConditionType::MegaprojectComplete, Rebuild) }, TEXT("skip")));
		BadFeats->AddRow(Same, FeatRow(EAscensionTier::Tier1_Planetary, EAscensionTier::Tier1_Planetary, { Cond(EConditionType::MegaprojectComplete, Build) }, TEXT("same")));
		BadFeats->AddRow(Good, FeatRow(EAscensionTier::Tier1_Planetary, EAscensionTier::Tier2_Stellar, { Cond(EConditionType::MegaprojectComplete, Build) }, TEXT("good")));
		BadFeats->AddRow(FName(TEXT("feat_test_bare")), FeatRow(EAscensionTier::Tier1_Planetary, EAscensionTier::Tier2_Stellar, {}, TEXT("bare")));
		FGalaxyState* Again = LayBoard(*Sim, TablesWith(Original, Knobs(BaseRules), BadFeats));
		if (!TestNotNull(TEXT("The board of the bad rows is laid"), Again))
		{
			return false;
		}
		// People 1 completed both: the two bad rows would hold for it.
		Record(Again->Systems[6], Build, 5);
		Record(Again->Systems[6], Rebuild, 5);
		AddExpectedError(TEXT("is not one rung up with a condition"), EAutomationExpectedErrorFlags::Contains, 0);
		Sim->StepOnce();
		TestEqual(TEXT("Past the refused rows the people climbs exactly one rung"), Again->Civs[1].Tier, EAscensionTier::Tier2_Stellar);
		TestTrue(TEXT("by the good row"), FindLine(*Again, Good, Again->Tick) != INDEX_NONE);
		TestEqual(TEXT("A row that skips a rung is not judged: nothing is told of it"), CountLines(*Again, Skip), 0);
		TestEqual(TEXT("A row that stays on its rung is not judged: nothing is told of it"), CountLines(*Again, Same), 0);
		TestEqual(TEXT("A row without a condition lifts no people"), Again->Civs[2].Tier, EAscensionTier::Tier1_Planetary);
		// The AI builds for the good row, not for the first row by name, which the judgment refuses.
		TestEqual(TEXT("The AI builds only for a row the judgment looks at"), Again->Systems[7].ActiveMegaproject, Build);
	}

	// With the gate closed (a leap, a balance run) the player's ascension is not heard; a state that is not the live one
	// is not judged.
	{
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the closed gate is laid"), Again))
		{
			return false;
		}
		Record(Again->Systems[0], Build, 5);
		PlaceByHand(*Again, 0, TEXT("Hazard.Plague"), 500);
		// A peak above the rung the people reaches (a vessel that stood higher before) is not lowered.
		if (!Again->Calling.Lives.IsEmpty())
		{
			Again->Calling.Lives.Last().PeakTier = EAscensionTier::Tier4_Transcendent;
		}
		{
			FGalaxyState Copy = *Again;
			FRandomStream Local(1);
			const int32 LinesBefore = Again->Log.Num();
			AddExpectedError(TEXT("EvaluateFeats: the state handed in is not the live state"), EAutomationExpectedErrorFlags::Contains, 1);
			Ascension->EvaluateFeats(Copy, Local);
			TestEqual(TEXT("A copy of the state is not judged"), Copy.Civs[0].Tier, EAscensionTier::Tier1_Planetary);
			TestEqual(TEXT("and nothing of it is told"), Again->Log.Num(), LinesBefore);
		}
		Heard.Reset();
		Sim->SetBroadcastEnabled(false);
		Sim->StepOnce();
		Sim->SetBroadcastEnabled(true);
		TestEqual(TEXT("With the gate closed the player's people climbs"), Again->Civs[0].Tier, EAscensionTier::Tier2_Stellar);
		TestEqual(TEXT("and nobody hears it"), Heard.Num(), 0);
		TestEqual(TEXT("A higher peak of the life is not lowered by the climb"),
			Again->Calling.Lives.IsEmpty() ? EAscensionTier::Tier1_Planetary : Again->Calling.Lives.Last().PeakTier, EAscensionTier::Tier4_Transcendent);
	}

	// A handler of a Core Logic delegate does not judge.
	{
		FGalaxyState* Again = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the handler is laid"), Again))
		{
			return false;
		}
		// Two rungs are ready for people 2: a judgment the handler could run would lift it a second time in this tick.
		Again->Civs[2].Tier = EAscensionTier::Tier2_Stellar;
		Record(Again->Systems[7], Build, 5);
		Record(Again->Systems[7], Rebuild, 5);
		bool bTried = false;
		Probe.Probe->OnLog = [&](const FCosmicLogEntry& Entry)
		{
			if (bTried || Entry.SourceRow != FeatSecond)
			{
				return;
			}
			bTried = true;
			FRandomStream Local(1);
			Ascension->EvaluateFeats(Sim->GetMutableState(), Local);
		};
		Sim->StepOnce();
		TestTrue(TEXT("The handler of the ascension line ran"), bTried);
		TestEqual(TEXT("and lifted nobody again"), Again->Civs[2].Tier, EAscensionTier::Tier3_Causal);
		Probe.Probe->OnLog = nullptr;
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicFeatsReachableTest, "Cosmic_Ascension.Feats.Reachable", CosmicTests::Flags)
bool FCosmicFeatsReachableTest::RunTest(const FString& Parameters)
{
	using namespace CosmicAscensionTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	ON_SCOPE_EXIT { Sim->NewGalaxy(42, 5, 3); };
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(20000);
	const FGalaxyState& State = Sim->GetState();
	int32 Third = 0;
	for (const FCivilizationState& Civ : State.Civs)
	{
		Third += (!Civ.bIsPlayerRuled && static_cast<int32>(Civ.Tier) >= static_cast<int32>(EAscensionTier::Tier3_Causal)) ? 1 : 0;
	}
	TestTrue(TEXT("An AI people reaches the third rung in 20,000 ticks of the game's data"), Third >= 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicPlayerAscendsToT3Test, "Cosmic_Ascension.Sim.PlayerAscendsToT3", CosmicTests::Flags)
bool FCosmicPlayerAscendsToT3Test::RunTest(const FString& Parameters)
{
	using namespace CosmicAscensionTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	ON_SCOPE_EXIT { Sim->NewGalaxy(42, 5, 3); };
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	// The player's first act (B2): the inherited edict revoked - kept, it ends the reign before the climb does.
	TestEqual(TEXT("(the inherited edict revoked, when the game's data names one)"), CosmicTests::RevokeInheritedEdict(*Sim), !Sim->GetRules().StartingEdict.IsNone());
	const FGalaxyState& State = Sim->GetState();
	const int32 CivIndex = Sim->GetStateIndex().FindCiv(State, State.Player.RuledCivID);
	const int32 Home = State.Civs.IsValidIndex(CivIndex) ? Sim->GetStateIndex().FindSystem(State, State.Civs[CivIndex].HomeSystemID) : INDEX_NONE;
	if (!TestTrue(TEXT("The vessel rules a people with a capital"), State.Civs.IsValidIndex(CivIndex) && State.Systems.IsValidIndex(Home)))
	{
		return false;
	}

	// The project key builds what the first feat of the game names.
	TestTrue(TEXT("The project key starts a project"), Role->StartNextMegaproject());
	TestEqual(TEXT("the Dyson swarm on the capital"), State.Systems[Home].ActiveMegaproject, FName(TEXT("project_dyson")));
	for (int32 Chunk = 0; Chunk < 60 && State.Civs[CivIndex].Tier == EAscensionTier::Tier1_Planetary; ++Chunk)
	{
		Sim->RunHeadless(100);
	}
	if (!TestEqual(TEXT("The people climbs to the second rung"), State.Civs[CivIndex].Tier, EAscensionTier::Tier2_Stellar))
	{
		return false;
	}
	TestTrue(TEXT("by the feat of the Dyson swarm"), State.Calling.CompletedFeats.Contains(FName(TEXT("feat_dyson_swarm"))));
	// The vessel climbed as the ruler of its people: no rebellion took the throne meanwhile.
	TestEqual(TEXT("The vessel is still embodied"), State.Calling.LifeState, ELifeState::Embodied);
	TestEqual(TEXT("and rules"), State.Player.Role, EPlayerRole::Ruler);
	TestEqual(TEXT("the same people"), State.Player.RuledCivID, State.Civs[CivIndex].CivID);

	// A supernova threatens the capital, with the countdown of the game's row; the key answers it. A hazard standing there
	// first is resolved, so that its leader's record ends as a resolution ends it.
	const FHazardDef* NovaRow = Sim->GetTables().FindHazardByTag(Tag(TEXT("Hazard.PreSupernova")));
	if (!TestNotNull(TEXT("The game has the row of the supernova"), NovaRow))
	{
		return false;
	}
	const int32 Countdown = NovaRow->CountdownTicks;
	FGalaxyState& Live = Sim->GetMutableState();
	if (Live.Systems[Home].ActiveHazard.IsValid())
	{
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = CivIndex;
		Ctx.SystemIndex = Home;
		UCosmicEventSubsystem::Apply(Eff(EEffectType::ResolveHazard), Ctx);
	}
	const bool bGraceSpent = Live.Flags.HasTagExact(Tag(TEXT("Flag.Grace.Spent")));
	PlaceByHand(Live, Home, TEXT("Hazard.PreSupernova"), Countdown);
	TestTrue(TEXT("The project key answers the supernova"), Role->StartNextMegaproject());
	TestEqual(TEXT("with the stabilization"), Live.Systems[Home].ResponseMegaproject, FName(TEXT("project_stabilize")));
	// The third rung is no end (B6a decision 2-ga, [B-6a3a]): no provisional end holds in any tick of the climb, the tick of
	// the third rung with it, and the run goes on (the clock is switched off for the test: this run may pass tick 6,000 on
	// its way up).
	FCosmicRulesDef NoClock = Sim->GetRules();
	NoClock.ProvisionalEndTick = 0;
	int64 ThirdRungTick = -1;
	bool bEndOnTheWay = false;
	for (int32 Step = 0; Step < Countdown && Live.Civs[CivIndex].Tier == EAscensionTier::Tier2_Stellar; ++Step)
	{
		Sim->StepOnce();
		bEndOnTheWay |= CosmicEndings::ProvisionalEndOf(Live, NoClock) != ECosmicProvisionalEnd::None;
		ThirdRungTick = (ThirdRungTick < 0 && Live.Civs[CivIndex].Tier == EAscensionTier::Tier3_Causal) ? Live.Tick : ThirdRungTick;
	}
	TestTrue(TEXT("(the third rung is reached in a stepped tick)"), ThirdRungTick >= 0 && ThirdRungTick == Live.Tick);
	TestEqual(TEXT("The third rung ends no run: no provisional end in its tick"), CosmicEndings::ProvisionalEndOf(Live, NoClock), ECosmicProvisionalEnd::None);
	TestFalse(TEXT("nor in any tick of the climb"), bEndOnTheWay);
	TestTrue(TEXT("The stabilization completed within the supernova's countdown"), CosmicMegaprojects::HasCompleted(Live.Systems[Home], FName(TEXT("project_stabilize")), Live.Calling.BornTick, NAME_None));
	TestFalse(TEXT("The capital still stands"), CosmicHazards::IsDeadNode(Live.Systems[Home]));
	TestFalse(TEXT("The supernova is resolved"), Live.Systems[Home].ActiveHazard == Tag(TEXT("Hazard.PreSupernova")));
	TestEqual(TEXT("not kept by the grace"), Live.Flags.HasTagExact(Tag(TEXT("Flag.Grace.Spent"))), bGraceSpent);
	TestEqual(TEXT("The people climbs to the third rung"), Live.Civs[CivIndex].Tier, EAscensionTier::Tier3_Causal);
	TestTrue(TEXT("by the feat of the stabilized supernova"), Live.Calling.CompletedFeats.Contains(FName(TEXT("feat_supernova_stabilized"))));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

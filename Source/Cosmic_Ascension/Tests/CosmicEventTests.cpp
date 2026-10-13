// Cosmic Ascension - automation tests of the event subsystem. Design: guideline.md section 7.5, 9.2, 9.3.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicSeed.h"
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
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "Tests/CosmicTestProbe.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace CosmicEventTests
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

	// The tables handed in, with this DT_Events and a rules row that carries other slot knobs.
	FCosmicDataTables WithEvents(const FCosmicDataTables& Base, const FCosmicRulesDef& BaseRules, UDataTable* Events, int32 SlotTicks, float Chance)
	{
		FCosmicRulesDef RulesRow = BaseRules;
		RulesRow.EventSlotTicks = SlotTicks;
		RulesRow.EventChancePerSlot = Chance;
		UDataTable* RulesTable = NewTable(FCosmicRulesDef::StaticStruct());
		RulesTable->AddRow(CosmicRows::Rules, RulesRow);

		FCosmicDataTables Tables = Base;
		Tables.Rules = RulesTable;
		Tables.Events = Events;
		return Tables;
	}

	// The DT_Civs of the game with three person names on every row, so that NameNotable can make a record.
	UDataTable* CivsWithNames(const FCosmicDataTables& Base)
	{
		UDataTable* Civs = NewTable(FCivArchetypeDef::StaticStruct());
		for (const FName RowName : Base.GetCivRowNames())
		{
			FCivArchetypeDef Def = *Base.FindCiv(RowName);
			Def.PersonNameBank = { FText::AsCultureInvariant(TEXT("name a")), FText::AsCultureInvariant(TEXT("name b")), FText::AsCultureInvariant(TEXT("name c")) };
			Civs->AddRow(RowName, Def);
		}
		return Civs;
	}

	FCosmicEventDef MakeRow(ECosmicEventTrigger Trigger, ECosmicEventSubject Subject, float Weight, int32 Cooldown)
	{
		FCosmicEventDef Def;
		Def.Category = Tag(TEXT("Event.Civ.Economy"));
		Def.LogCategory = Tag(TEXT("Log.Civ"));
		Def.Trigger = Trigger;
		Def.Subject = Subject;
		Def.BaseWeight = Weight;
		Def.CooldownTicks = Cooldown;
		return Def;
	}

	FCosmicCondition Cond(EConditionType Type, const TCHAR* TagName, FName Key, float Value)
	{
		FCosmicCondition Condition;
		Condition.Type = Type;
		Condition.Tag = TagName ? Tag(TagName) : FGameplayTag();
		Condition.Key = Key;
		Condition.Value = Value;
		return Condition;
	}

	FCosmicEffect Eff(EEffectType Type, const TCHAR* TagName, FName Key, float Value)
	{
		FCosmicEffect Effect;
		Effect.Type = Type;
		Effect.Tag = TagName ? Tag(TagName) : FGameplayTag();
		Effect.Key = Key;
		Effect.Value = Value;
		return Effect;
	}

	TArray<FText> ThreeLines(const TCHAR* Stem)
	{
		return {
			FText::AsCultureInvariant(FString::Printf(TEXT("%s one"), Stem)),
			FText::AsCultureInvariant(FString::Printf(TEXT("%s two"), Stem)),
			FText::AsCultureInvariant(FString::Printf(TEXT("%s three"), Stem))
		};
	}

	// The slot phase of a civilization, as section 7.5 defines it.
	int64 SlotPhase(const FGalaxyState& State, FName CivID, int32 SlotTicks)
	{
		return static_cast<int64>(CosmicSeed::DeriveForName(State.Seed, CivID, CosmicSeed::Purpose::EventSlot) % static_cast<uint32>(SlotTicks));
	}

	// A probe on both delegates for the lifetime of the object.
	struct FScopedProbe
	{
		FScopedProbe(UGalaxySimulationSubsystem& Sim)
			: Log(Sim.GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>())
			, Events(Sim.GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>())
			, Probe(NewObject<UCosmicTestProbe>())
		{
			Probe->AddToRoot();
			Log->OnLogEntry.AddDynamic(Probe, &UCosmicTestProbe::HandleLogEntry);
			Events->OnEventFired.AddDynamic(Probe, &UCosmicTestProbe::HandleEventFired);
		}

		~FScopedProbe()
		{
			Log->OnLogEntry.RemoveDynamic(Probe, &UCosmicTestProbe::HandleLogEntry);
			Events->OnEventFired.RemoveDynamic(Probe, &UCosmicTestProbe::HandleEventFired);
			Probe->RemoveFromRoot();
		}

		UCosmicLogSubsystem* Log;
		UCosmicEventSubsystem* Events;
		UCosmicTestProbe* Probe;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicEventsCheckIsPureTest, "Cosmic_Ascension.Events.CheckIsPure", CosmicTests::Flags)
bool FCosmicEventsCheckIsPureTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEventTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	// A detached state with an index of its own: Check is static and needs nothing of the live game.
	Sim->NewGalaxy(42, 5, 3);
	FGalaxyState Detached = Sim->GetState();
	const FName Civ1 = Detached.Civs[1].CivID;
	const FName TrActive(TEXT("tr_test"));
	const FName TrSealed(TEXT("tr_sealed"));
	const FName Relic(TEXT("relic_test"));
	Detached.Tick = 100;
	{
		FCivilizationState& Civ = Detached.Civs[0];
		Civ.Tier = EAscensionTier::Tier2_Stellar;
		Civ.Traits.AddTag(Tag(TEXT("Trait.Faith.Seeking")));
		Civ.Stockpile.Energy = 50.f;
		Civ.Stability = 0.4f;
		Civ.Ethics.Openness = 0.5f;
		Civ.LowStabilityTicks = 120;
		Civ.Relations.Add(Civ1, 0.6f);
		Civ.ReverenceToPlayer = 0.5f;
		FNotableRecord& Rebel = Civ.Notables.AddDefaulted_GetRef();
		Rebel.Role = Tag(TEXT("Notable.Rebel"));
		Rebel.FirstTick = 90;
	}
	Detached.Systems[0].ActiveHazard = Tag(TEXT("Hazard.Plague"));
	Detached.Systems[0].MegaprojectCompletedTick = 50;
	{
		FTranscendentState& Active = Detached.Transcendents.AddDefaulted_GetRef();
		Active.TranscendentID = TrActive;
		Active.Doctrine = Tag(TEXT("Doctrine.Silence"));
		Active.InfluencedSystems.Add(Detached.Systems[0].SystemID);
		Active.bAwareOfPlayer = true;
		FTranscendentState& Sealed = Detached.Transcendents.AddDefaulted_GetRef();
		Sealed.TranscendentID = TrSealed;
		Sealed.Doctrine = Tag(TEXT("Doctrine.Zeal"));
		Sealed.bSealed = true;
		Sealed.SealedTick = 40;
	}
	Detached.Player.Body = 3;
	Detached.Player.Relics.Add(Relic);
	Detached.Player.StatusTags.AddTag(Tag(TEXT("Status.Abdicated")));
	Detached.Calling.PersonalTier = EAscensionTier::Tier2_Stellar;
	Detached.Calling.LifeIndex = 2;
	Detached.Flags.AddTag(Tag(TEXT("Flag.Grace.Spent")));
	FCosmicStateIndex Index;
	Index.Rebuild(Detached);

	// One line per case: the condition, the lower tick bound of the view, the expected answer for subject (civ 0, sol).
	struct FCase
	{
		FCosmicCondition Condition;
		int64 SinceTick;
		bool bExpected;
	};
	const FCase Cases[] = {
		{ Cond(EConditionType::CivTierAtLeast, nullptr, NAME_None, 2.f), 0, true },
		{ Cond(EConditionType::CivTierAtLeast, nullptr, NAME_None, 3.f), 0, false },
		{ Cond(EConditionType::CivHasTrait, TEXT("Trait.Faith.Seeking"), NAME_None, 0.f), 0, true },
		{ Cond(EConditionType::CivHasTrait, TEXT("Trait.Faith.Remnant"), NAME_None, 0.f), 0, false },
		{ Cond(EConditionType::CivResourceAtLeast, nullptr, FName(TEXT("Energy")), 50.f), 0, true },
		{ Cond(EConditionType::CivResourceAtLeast, nullptr, FName(TEXT("Energy")), 51.f), 0, false },
		{ Cond(EConditionType::CivResourceAtLeast, nullptr, FName(TEXT("NoSuchResource")), 0.f), 0, false },
		{ Cond(EConditionType::CivStabilityBelow, nullptr, NAME_None, 0.5f), 0, true },
		{ Cond(EConditionType::CivStabilityBelow, nullptr, NAME_None, 0.4f), 0, false },
		{ Cond(EConditionType::CivEthicsAxisAbove, nullptr, FName(TEXT("Openness")), 0.4f), 0, true },
		{ Cond(EConditionType::CivEthicsAxisAbove, nullptr, FName(TEXT("Openness")), 0.5f), 0, false },
		{ Cond(EConditionType::CivStabilityBelowFor, nullptr, FName(TEXT("100")), 0.3f), 0, true },
		{ Cond(EConditionType::CivStabilityBelowFor, nullptr, FName(TEXT("121")), 0.3f), 0, false },
		{ Cond(EConditionType::CivRelationAtLeast, nullptr, Civ1, 0.6f), 0, true },
		{ Cond(EConditionType::CivRelationAtLeast, nullptr, Civ1, 0.7f), 0, false },
		{ Cond(EConditionType::CivReverenceAtLeast, nullptr, NAME_None, 0.5f), 0, true },
		{ Cond(EConditionType::CivReverenceAtLeast, nullptr, NAME_None, 0.6f), 0, false },
		{ Cond(EConditionType::SystemHasHazard, TEXT("Hazard.Plague"), NAME_None, 0.f), 0, true },
		{ Cond(EConditionType::SystemHasHazard, nullptr, NAME_None, 0.f), 0, true },
		{ Cond(EConditionType::SystemHasHazard, TEXT("Hazard.Rebellion"), NAME_None, 0.f), 0, false },
		{ Cond(EConditionType::SystemOwnedBy, nullptr, NAME_None, 0.f), 0, true },
		{ Cond(EConditionType::SystemOwnedBy, nullptr, Civ1, 0.f), 0, false },
		{ Cond(EConditionType::MegaprojectComplete, nullptr, NAME_None, 0.f), 0, true },
		{ Cond(EConditionType::MegaprojectComplete, nullptr, NAME_None, 0.f), 51, false },
		{ Cond(EConditionType::SystemPressureAtLeast, nullptr, NAME_None, 0.f), 0, false },
		{ Cond(EConditionType::SystemInfluencedBy, nullptr, TrActive, 0.f), 0, true },
		{ Cond(EConditionType::SystemInfluencedBy, nullptr, TrSealed, 0.f), 0, false },
		{ Cond(EConditionType::PlayerTierAtLeast, nullptr, NAME_None, 2.f), 0, true },
		{ Cond(EConditionType::PlayerTierAtLeast, nullptr, NAME_None, 3.f), 0, false },
		{ Cond(EConditionType::PlayerInSystemWithHazard, nullptr, NAME_None, 0.f), 0, true },
		{ Cond(EConditionType::PlayerInSystemWithHazard, TEXT("Hazard.Rebellion"), NAME_None, 0.f), 0, false },
		{ Cond(EConditionType::PlayerStatAtLeast, nullptr, FName(TEXT("Body")), 3.f), 0, true },
		{ Cond(EConditionType::PlayerStatAtLeast, nullptr, FName(TEXT("Body")), 4.f), 0, false },
		{ Cond(EConditionType::PlayerHasRelic, nullptr, Relic, 0.f), 0, true },
		{ Cond(EConditionType::PlayerHasRelic, nullptr, FName(TEXT("relic_other")), 0.f), 0, false },
		{ Cond(EConditionType::PlayerRoleIs, nullptr, FName(TEXT("Ruler")), 0.f), 0, true },
		{ Cond(EConditionType::PlayerRoleIs, nullptr, FName(TEXT("Wanderer")), 0.f), 0, false },
		{ Cond(EConditionType::PlayerHasStatus, TEXT("Status.Abdicated"), NAME_None, 0.f), 0, true },
		{ Cond(EConditionType::PlayerHasStatus, TEXT("Status.Returned"), NAME_None, 0.f), 0, false },
		{ Cond(EConditionType::PlayerLifeIndexAtLeast, nullptr, NAME_None, 2.f), 0, true },
		{ Cond(EConditionType::PlayerLifeIndexAtLeast, nullptr, NAME_None, 3.f), 0, false },
		{ Cond(EConditionType::PlayerAgeAtLeast, nullptr, NAME_None, 0.f), 0, false },
		{ Cond(EConditionType::GalaxyTickAtLeast, nullptr, NAME_None, 100.f), 0, true },
		{ Cond(EConditionType::GalaxyTickAtLeast, nullptr, NAME_None, 101.f), 0, false },
		{ Cond(EConditionType::TranscendentSealed, nullptr, TrSealed, 0.f), 0, true },
		{ Cond(EConditionType::TranscendentSealed, nullptr, TrSealed, 0.f), 41, false },
		{ Cond(EConditionType::TranscendentSealed, nullptr, TrActive, 0.f), 0, false },
		{ Cond(EConditionType::TranscendentAwareOfPlayer, nullptr, TrActive, 0.f), 0, true },
		{ Cond(EConditionType::TranscendentAwareOfPlayer, nullptr, TrSealed, 0.f), 0, false },
		{ Cond(EConditionType::FlagSet, TEXT("Flag.Grace.Spent"), NAME_None, 0.f), 0, true },
		{ Cond(EConditionType::FlagNotSet, TEXT("Flag.Grace.Spent"), NAME_None, 0.f), 0, false },
		{ Cond(EConditionType::TranscendentDoctrinePresent, TEXT("Doctrine.Silence"), NAME_None, 0.f), 0, true },
		{ Cond(EConditionType::TranscendentDoctrinePresent, TEXT("Doctrine.Zeal"), NAME_None, 0.f), 0, false },
		{ Cond(EConditionType::CivHasNotable, TEXT("Notable.Rebel"), NAME_None, 0.f), 0, true },
		{ Cond(EConditionType::CivHasNotable, TEXT("Notable.Ruler"), NAME_None, 0.f), 0, false },
	};

	auto Answers = [&](const FGalaxyState& State, int32 CivIndex, int32 SystemIndex)
	{
		TArray<bool> Out;
		for (const FCase& Case : Cases)
		{
			FCosmicEvalView View(State, Index, Sim->GetTables(), Sim->GetRules());
			View.CivIndex = CivIndex;
			View.SystemIndex = SystemIndex;
			View.SinceTick = Case.SinceTick;
			Out.Add(UCosmicEventSubsystem::Check(Case.Condition, View));
		}
		return Out;
	};
	auto IsSubjectType = [](EConditionType Type)
	{
		return Type <= EConditionType::SystemInfluencedBy || Type == EConditionType::CivHasNotable;
	};
	auto IsPlayerType = [](EConditionType Type)
	{
		return Type >= EConditionType::PlayerTierAtLeast && Type <= EConditionType::PlayerAgeAtLeast;
	};

	// Every condition type has a case, and each case answers as written.
	TSet<EConditionType> Types;
	const TArray<bool> Plain = Answers(Detached, 0, 0);
	for (int32 CaseIndex = 0; CaseIndex < static_cast<int32>(UE_ARRAY_COUNT(Cases)); ++CaseIndex)
	{
		Types.Add(Cases[CaseIndex].Condition.Type);
		TestEqual(*FString::Printf(TEXT("Case %d (condition type %d)"), CaseIndex, static_cast<int32>(Cases[CaseIndex].Condition.Type)), Plain[CaseIndex], Cases[CaseIndex].bExpected);
	}
	TestEqual(TEXT("Every condition type is covered"), Types.Num(), static_cast<int32>(EConditionType::CivHasNotable) + 1);

	// Without a subject every Civ and System condition is false and nothing else changes.
	const TArray<bool> NoSubject = Answers(Detached, INDEX_NONE, INDEX_NONE);
	bool bNoSubjectRule = true;
	for (int32 CaseIndex = 0; CaseIndex < static_cast<int32>(UE_ARRAY_COUNT(Cases)); ++CaseIndex)
	{
		bNoSubjectRule &= NoSubject[CaseIndex] == (IsSubjectType(Cases[CaseIndex].Condition.Type) ? false : Cases[CaseIndex].bExpected);
	}
	TestTrue(TEXT("A condition whose subject is missing is false"), bNoSubjectRule);

	// Knowing and telling are never read.
	FGalaxyState Told = Detached;
	Told.Knowing.MetCivs.AddDefaulted_GetRef().ID = Civ1;
	Told.Knowing.FirstContactCivID = Civ1;
	Told.Log.AddDefaulted();
	Told.Chronicle.AddDefaulted();
	Told.TellRngState += 7;
	TestTrue(TEXT("Knowing, Log, Chronicle and TellRngState change no answer"), Answers(Told, 0, 0) == Plain);

	// During the interregnum there is no vessel: every Player condition is false, and nothing else changes.
	FGalaxyState Between = Detached;
	Between.Calling.LifeState = ELifeState::Interregnum;
	const TArray<bool> Interregnum = Answers(Between, 0, 0);
	bool bInterregnumRule = true;
	for (int32 CaseIndex = 0; CaseIndex < static_cast<int32>(UE_ARRAY_COUNT(Cases)); ++CaseIndex)
	{
		bInterregnumRule &= Interregnum[CaseIndex] == (IsPlayerType(Cases[CaseIndex].Condition.Type) ? false : Cases[CaseIndex].bExpected);
	}
	TestTrue(TEXT("During the interregnum every Player condition is false and no other answer changes"), bInterregnumRule);

	// The live state: a thousand checks write nothing and draw nothing.
	const uint32 Full = Sim->ComputeStateHash();
	const uint32 World = Sim->ComputeWorldHash();
	const int32 Seed = Sim->GetRng().GetCurrentSeed();
	for (int32 Round = 0; Round < 20; ++Round)
	{
		for (const FCase& Case : Cases)
		{
			FCosmicEvalView View(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
			View.CivIndex = 0;
			View.SystemIndex = 0;
			UCosmicEventSubsystem::Check(Case.Condition, View);
		}
	}
	TestTrue(TEXT("Check writes nothing and draws nothing"),
		Sim->ComputeStateHash() == Full && Sim->ComputeWorldHash() == World && Sim->GetRng().GetCurrentSeed() == Seed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicEventsSlotDrawsTest, "Cosmic_Ascension.Events.SlotDraws", CosmicTests::Flags)
bool FCosmicEventsSlotDrawsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEventTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicEventSubsystem* Events = Sim->GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();

	// The error line of the out-of-range knob at the end.
	AddExpectedError(TEXT("EventSlotTicks 0 and EventChancePerSlot"), EAutomationExpectedErrorFlags::Contains, 1);

	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };
	const int32 SlotTicks = BaseRules.EventSlotTicks;
	if (!TestTrue(TEXT("DT_Rules holds a slot period (run Tools/make_data_tables.py)"), SlotTicks >= 1))
	{
		return false;
	}

	// One call draws once per civilization that has a slot in that tick, and for nobody else.
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	FGalaxyState& State = Sim->GetMutableState();
	bool bDrawsFollowSlots = true;
	int32 SlotsSeen = 0;
	for (int64 Tick = 1; Tick <= 2 * SlotTicks; ++Tick)
	{
		State.Tick = Tick;
		int32 Expected = 0;
		for (const FCivilizationState& Civ : State.Civs)
		{
			Expected += (SlotPhase(State, Civ.CivID, SlotTicks) == Tick % SlotTicks) ? 1 : 0;
		}
		FRandomStream Handed(123);
		FRandomStream Probe(123);
		Events->EvaluateAndFire(State, Handed);
		for (int32 Draw = 0; Draw < Expected; ++Draw)
		{
			Probe.GetFraction();
		}
		bDrawsFollowSlots &= Handed.GetCurrentSeed() == Probe.GetCurrentSeed();
		SlotsSeen += Expected;
	}
	TestTrue(TEXT("EvaluateAndFire draws once per civilization that has a slot"), bDrawsFollowSlots);
	TestEqual(TEXT("Every civilization has one slot per period"), SlotsSeen, 2 * State.Civs.Num());

	// The sim stream after every tick does not depend on DT_Events: no table, no row, rows that fire, rows on cooldown
	// and a chance of zero all draw the same.
	uint32 WorldAfter = 0;
	auto SeedsOver = [Sim, &WorldAfter](const FCosmicDataTables& Tables)
	{
		Sim->SetTables(Tables);
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		TArray<int32> Seeds;
		for (int32 Tick = 0; Tick < 200; ++Tick)
		{
			Sim->RunHeadless(1);
			Seeds.Add(Sim->GetRng().GetCurrentSeed());
		}
		WorldAfter = Sim->ComputeWorldHash();
		return Seeds;
	};
	auto FortyRows = [](int32 Cooldown)
	{
		UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
		for (int32 RowIndex = 0; RowIndex < 40; ++RowIndex)
		{
			FCosmicEventDef Def = MakeRow((RowIndex % 2 == 0) ? ECosmicEventTrigger::Lottery : ECosmicEventTrigger::Standing,
				(RowIndex % 3 == 0) ? ECosmicEventSubject::SettledSystem : ECosmicEventSubject::Civilization, 1.f, Cooldown);
			Def.Effects.Add(Eff(EEffectType::ChangeStability, nullptr, NAME_None, (RowIndex % 2 == 0) ? 0.01f : -0.01f));
			Table->AddRow(FName(*FString::Printf(TEXT("evt_test_%02d"), RowIndex)), Def);
		}
		return Table;
	};
	const TArray<int32> NoTable = SeedsOver(WithEvents(Original, BaseRules, nullptr, SlotTicks, 0.5f));
	const uint32 QuietWorld = WorldAfter;
	TestTrue(TEXT("An empty DT_Events draws like no table"), SeedsOver(WithEvents(Original, BaseRules, NewTable(FCosmicEventDef::StaticStruct()), SlotTicks, 0.5f)) == NoTable);
	TestEqual(TEXT("An empty DT_Events leaves the world as no table does"), static_cast<int64>(WorldAfter), static_cast<int64>(QuietWorld));
	TestTrue(TEXT("Forty rows that fire draw like no table"), SeedsOver(WithEvents(Original, BaseRules, FortyRows(0), SlotTicks, 0.5f)) == NoTable);
	TestTrue(TEXT("The rows that fired changed the world all the same"), WorldAfter != QuietWorld);
	TestTrue(TEXT("Forty rows on a long cooldown draw like no table"), SeedsOver(WithEvents(Original, BaseRules, FortyRows(100000), SlotTicks, 0.5f)) == NoTable);
	TestTrue(TEXT("A chance of zero draws like no table"), SeedsOver(WithEvents(Original, BaseRules, FortyRows(0), SlotTicks, 0.f)) == NoTable);

	// A Standing row that always fires shows the slot ticks of every civilization. Removing one civilization from the
	// array leaves the slot ticks of the others where they were: the phase hangs on the CivID, not on the array index.
	{
		UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
		Table->AddRow(FName(TEXT("evt_test_beat")), MakeRow(ECosmicEventTrigger::Standing, ECosmicEventSubject::Civilization, 0.f, 0));
		Sim->SetTables(WithEvents(Original, BaseRules, Table, SlotTicks, 0.5f));
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);

		TMap<FName, int64> PhaseSeen;
		bool bPhaseStable = true;
		FScopedProbe Scoped(*Sim);
		Scoped.Probe->OnEvent = [&PhaseSeen, &bPhaseStable, SlotTicks](const FCosmicEventNotice& Notice)
		{
			const int64 Phase = Notice.Tick % SlotTicks;
			if (const int64* Before = PhaseSeen.Find(Notice.CivID))
			{
				bPhaseStable &= *Before == Phase;
			}
			PhaseSeen.Add(Notice.CivID, Phase);
		};
		for (int32 Tick = 0; Tick < 2 * SlotTicks; ++Tick)
		{
			Sim->StepOnce();
		}
		TestEqual(TEXT("Every civilization had its slot"), PhaseSeen.Num(), State.Civs.Num());
		bool bPhaseIsDerived = true;
		for (const FCivilizationState& Civ : State.Civs)
		{
			const int64* Seen = PhaseSeen.Find(Civ.CivID);
			bPhaseIsDerived &= Seen && *Seen == SlotPhase(State, Civ.CivID, SlotTicks);
		}
		TestTrue(TEXT("The slot tick of a civilization is the phase derived from its CivID"), bPhaseIsDerived);

		State.Civs.RemoveAt(1);
		Sim->RebuildStateIndex();
		for (int32 Tick = 0; Tick < 2 * SlotTicks; ++Tick)
		{
			Sim->StepOnce();
		}
		TestTrue(TEXT("Removing a civilization leaves the slot ticks of the others unchanged"), bPhaseStable);
	}

	// No civilization: nothing is drawn and nothing fires.
	{
		Sim->NewGalaxy(42, 5, 3);
		State.Civs.Empty();
		Sim->RebuildStateIndex();
		FRandomStream Handed(123);
		TestEqual(TEXT("A state without civilizations fires nothing"), Events->EvaluateAndFire(State, Handed), 0);
		TestEqual(TEXT("A state without civilizations draws nothing"), Handed.GetCurrentSeed(), FRandomStream(123).GetCurrentSeed());
	}

	// A slot period below 1 is read as 1, with an error line: every civilization has a slot in every tick.
	{
		Sim->SetTables(WithEvents(Original, BaseRules, nullptr, 0, 0.5f));
		Sim->NewGalaxy(42, 5, 3);
		State.Tick = 7;
		FRandomStream Handed(123);
		FRandomStream Probe(123);
		Events->EvaluateAndFire(State, Handed);
		for (int32 Draw = 0; Draw < State.Civs.Num(); ++Draw)
		{
			Probe.GetFraction();
		}
		TestEqual(TEXT("With a slot period of 0 every civilization draws in every tick"), Handed.GetCurrentSeed(), Probe.GetCurrentSeed());
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicEventsRowOrderInertTest, "Cosmic_Ascension.Events.RowOrderInert", CosmicTests::Flags)
bool FCosmicEventsRowOrderInertTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEventTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	// Rows that change the world, added to the table in one order and in the reverse order.
	auto Rows = [](bool bReverse)
	{
		TArray<TPair<FName, FCosmicEventDef>> Defs;
		for (int32 RowIndex = 0; RowIndex < 8; ++RowIndex)
		{
			FCosmicEventDef Def = MakeRow((RowIndex % 2 == 0) ? ECosmicEventTrigger::Lottery : ECosmicEventTrigger::Standing,
				(RowIndex % 3 == 0) ? ECosmicEventSubject::SettledSystem : ECosmicEventSubject::Civilization, 1.f + RowIndex, 20 + 10 * RowIndex);
			Def.Effects.Add(Eff(EEffectType::ChangeStability, nullptr, NAME_None, (RowIndex % 2 == 0) ? 0.02f : -0.03f));
			Def.Effects.Add(Eff(EEffectType::AddResource, nullptr, FName(TEXT("Matter")), 1.f + RowIndex));
			Def.LogTemplates = ThreeLines(TEXT("row order"));
			Defs.Emplace(FName(*FString::Printf(TEXT("evt_test_%d"), RowIndex)), Def);
		}
		UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
		for (int32 Step = 0; Step < Defs.Num(); ++Step)
		{
			const TPair<FName, FCosmicEventDef>& Def = Defs[bReverse ? Defs.Num() - 1 - Step : Step];
			Table->AddRow(Def.Key, Def.Value);
		}
		return Table;
	};
	auto Run = [Sim](const FCosmicDataTables& Tables)
	{
		Sim->SetTables(Tables);
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		Sim->RunHeadless(1000);
	};

	Run(WithEvents(Original, BaseRules, Rows(false), 10, 0.05f));
	const uint32 World = Sim->ComputeWorldHash();
	const uint32 Full = Sim->ComputeStateHash();
	const int32 Seed = Sim->GetState().RngState;
	TestTrue(TEXT("The rows fired"), !Sim->GetState().EventCooldownUntilTick.IsEmpty() && Sim->GetState().Log.Num() > 10);

	Run(WithEvents(Original, BaseRules, Rows(true), 10, 0.05f));
	TestEqual(TEXT("The row order of DT_Events does not change the world"), static_cast<int64>(Sim->ComputeWorldHash()), static_cast<int64>(World));
	TestEqual(TEXT("The row order of DT_Events does not change the telling"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Full));
	TestEqual(TEXT("The row order of DT_Events does not change the sim stream"), Sim->GetState().RngState, Seed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicEventsLotterySharesTest, "Cosmic_Ascension.Events.LotteryShares", CosmicTests::Flags)
bool FCosmicEventsLotterySharesTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEventTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicEventSubsystem* Events = Sim->GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	const FName RowA(TEXT("evt_test_a"));
	const FName RowB(TEXT("evt_test_b"));

	// Runs Rounds calls with a slot for every civilization in every call (slot period 1) and a stream the test knows.
	// Calls Judge with the number each civilization drew and the rows that fired for it, in order.
	FScopedProbe Scoped(*Sim);
	auto Play = [&](UDataTable* Table, float Chance, int32 Rounds, TFunctionRef<void(float, const TArray<FCosmicEventNotice>&)> Judge)
	{
		Sim->SetTables(WithEvents(Original, BaseRules, Table, 1, Chance));
		FGalaxyState& State = Sim->GetMutableState();
		FRandomStream Handed(777);
		FRandomStream Probe(777);
		for (int32 Round = 0; Round < Rounds; ++Round)
		{
			TMap<FName, TArray<FCosmicEventNotice>> FiredFor;
			Scoped.Probe->OnEvent = [&FiredFor](const FCosmicEventNotice& Notice) { FiredFor.FindOrAdd(Notice.CivID).Add(Notice); };
			++State.Tick;
			Events->EvaluateAndFire(State, Handed);
			for (const FCivilizationState& Civ : State.Civs)
			{
				Judge(Probe.GetFraction(), FiredFor.FindRef(Civ.CivID));
			}
		}
		Scoped.Probe->OnEvent = nullptr;
	};

	Sim->NewGalaxy(42, 5, 3);

	// One row of weight 1 at chance 0.5: it fires exactly in the slots whose number is below 0.5.
	{
		UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
		Table->AddRow(RowA, MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 0));
		bool bByShare = true;
		int32 Fires = 0;
		Play(Table, 0.5f, 40, [&](float Roll, const TArray<FCosmicEventNotice>& Fired)
		{
			bByShare &= Fired.Num() == ((Roll < 0.5f) ? 1 : 0);
			Fires += Fired.Num();
		});
		TestTrue(TEXT("A row of share 0.5 fires exactly in the slots whose number is below 0.5"), bByShare);
		TestTrue(TEXT("Some slots fired and some did not"), Fires > 20 && Fires < 100);
	}

	// Two rows: the number picks by the interval it falls in, in row-name order.
	{
		UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
		// Added in reverse: the order of the pick is the order of the names.
		Table->AddRow(RowB, MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 0.5f, 0));
		Table->AddRow(RowA, MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 0));
		bool bByInterval = true;
		Play(Table, 0.5f, 40, [&](float Roll, const TArray<FCosmicEventNotice>& Fired)
		{
			const FName Expected = (Roll < 0.5f) ? RowA : (Roll < 0.75f) ? RowB : FName();
			bByInterval &= Expected.IsNone() ? Fired.IsEmpty() : (Fired.Num() == 1 && Fired[0].EventRow == Expected);
		});
		TestTrue(TEXT("Two rows are picked by the interval the number falls in, in row-name order"), bByInterval);
	}

	// Shares that add up to more than 1 are scaled together: every slot fires exactly one row.
	{
		UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
		Table->AddRow(RowA, MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 0));
		Table->AddRow(RowB, MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 0));
		bool bScaled = true;
		Play(Table, 1.f, 40, [&](float Roll, const TArray<FCosmicEventNotice>& Fired)
		{
			bScaled &= Fired.Num() == 1 && Fired[0].EventRow == ((Roll < 0.5f) ? RowA : RowB);
		});
		TestTrue(TEXT("Shares above a total of 1 are scaled together and every slot fires one row"), bScaled);
	}

	// Rows whose names sort after the first leave the fires of the first as they were.
	{
		auto FiresOfA = [&](bool bWithOthers)
		{
			UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
			Table->AddRow(RowA, MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 0));
			for (int32 Other = 0; bWithOthers && Other < 4; ++Other)
			{
				Table->AddRow(FName(*FString::Printf(TEXT("evt_test_later_%d"), Other)), MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 0));
			}
			TArray<bool> Out;
			Play(Table, 0.1f, 40, [&](float Roll, const TArray<FCosmicEventNotice>& Fired)
			{
				Out.Add(Fired.Num() == 1 && Fired[0].EventRow == RowA);
			});
			return Out;
		};
		const TArray<bool> Alone = FiresOfA(false);
		TestTrue(TEXT("The first row fired in some slots"), Alone.Contains(true) && Alone.Contains(false));
		TestTrue(TEXT("Rows that sort later leave the fires of the first row unchanged"), FiresOfA(true) == Alone);
	}

	// A SettledSystem row is a candidate once per settled system, in OwnedSystems order.
	{
		Sim->NewGalaxy(42, 5, 3);
		FGalaxyState& State = Sim->GetMutableState();
		FCivilizationState& Civ = State.Civs[0];
		for (FStarSystemState& System : State.Systems)
		{
			if (System.OwnerCivID.IsNone())
			{
				System.OwnerCivID = Civ.CivID;
				Civ.OwnedSystems.Add(System.SystemID);
			}
		}
		const TArray<FName> Owned = Civ.OwnedSystems;
		const FName CivID = Civ.CivID;
		if (TestEqual(TEXT("The test civilization owns three systems"), Owned.Num(), 3))
		{
			UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
			Table->AddRow(RowA, MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::SettledSystem, 1.f, 0));
			bool bPerSystem = true;
			TSet<FName> Hit;
			int32 CivTurn = 0;
			Play(Table, 0.2f, 60, [&](float Roll, const TArray<FCosmicEventNotice>& Fired)
			{
				// The civilizations are judged in Civs order: the first of every round is the one with three systems.
				if (CivTurn++ % 3 != 0)
				{
					return;
				}
				const int32 Expected = (Roll < 0.2f) ? 0 : (Roll < 0.4f) ? 1 : (Roll < 0.6f) ? 2 : INDEX_NONE;
				bPerSystem &= (Expected == INDEX_NONE) ? Fired.IsEmpty() : (Fired.Num() == 1 && Fired[0].SystemID == Owned[Expected] && Fired[0].CivID == CivID);
				if (Fired.Num() == 1)
				{
					Hit.Add(Fired[0].SystemID);
				}
			});
			TestTrue(TEXT("A SettledSystem row has one share per settled system, in OwnedSystems order"), bPerSystem);
			TestEqual(TEXT("Every settled system was picked at least once"), Hit.Num(), 3);
		}
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicEventsCooldownPerCivTest, "Cosmic_Ascension.Events.CooldownPerCiv", CosmicTests::Flags)
bool FCosmicEventsCooldownPerCivTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEventTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicEventSubsystem* Events = Sim->GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	const FName Beat(TEXT("evt_test_beat"));
	const FName Named(TEXT("evt_test_named"));
	const FName Lot(TEXT("evt_test_lot"));
	UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
	Table->AddRow(Beat, MakeRow(ECosmicEventTrigger::Standing, ECosmicEventSubject::Civilization, 0.f, 60));
	Table->AddRow(Named, MakeRow(ECosmicEventTrigger::Named, ECosmicEventSubject::Civilization, 0.f, 0));
	Table->AddRow(Lot, MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 45));
	Sim->SetTables(WithEvents(Original, BaseRules, Table, 10, 1.f));
	Sim->NewGalaxy(42, 5, 3);
	const FGalaxyState& State = Sim->GetState();

	// The Standing row keeps its own beat for every civilization: the first slot, then exactly 60 ticks later, again.
	TMap<FName, TArray<int64>> BeatTicks;
	TMap<FName, TArray<int64>> LotTicks;
	int32 NamedFires = 0;
	FScopedProbe Scoped(*Sim);
	Scoped.Probe->OnEvent = [&](const FCosmicEventNotice& Notice)
	{
		if (Notice.EventRow == Beat)
		{
			BeatTicks.FindOrAdd(Notice.CivID).Add(Notice.Tick);
		}
		else if (Notice.EventRow == Lot)
		{
			LotTicks.FindOrAdd(Notice.CivID).Add(Notice.Tick);
		}
		else if (Notice.EventRow == Named)
		{
			++NamedFires;
		}
	};
	for (int32 Tick = 0; Tick < 130; ++Tick)
	{
		Sim->StepOnce();
	}
	bool bBeatPerCiv = BeatTicks.Num() == State.Civs.Num();
	bool bLotPerCiv = LotTicks.Num() == State.Civs.Num();
	for (const FCivilizationState& Civ : State.Civs)
	{
		const TArray<int64> Beats = BeatTicks.FindRef(Civ.CivID);
		bBeatPerCiv &= Beats.Num() == 3 && Beats[0] <= 10 && Beats[1] == Beats[0] + 60 && Beats[2] == Beats[1] + 60;
		// Chance 1: the Lottery row fires in the first slot after its cooldown of 45 ticks, which is 50 ticks later.
		const TArray<int64> Lots = LotTicks.FindRef(Civ.CivID);
		bLotPerCiv &= Lots.Num() == 3 && Lots[0] <= 10 && Lots[1] == Lots[0] + 50 && Lots[2] == Lots[1] + 50;
	}
	TestTrue(TEXT("A Standing row with a cooldown of 60 fires for every civilization on its own beat"), bBeatPerCiv);
	TestTrue(TEXT("A Lottery pick writes its cooldown key and waits it out"), bLotPerCiv);

	// The key is exactly "<row>@<CivID>", and it holds the tick the cooldown is over.
	const FName CivID = State.Civs[1].CivID;
	const FName Key(*FString::Printf(TEXT("%s@%s"), *Beat.ToString(), *CivID.ToString()));
	TestEqual(TEXT("CooldownKey is <row>@<CivID>"), UCosmicEventSubsystem::CooldownKey(Beat, CivID), Key);
	const int64* Until = State.EventCooldownUntilTick.Find(Key);
	TestTrue(TEXT("The cooldown entry holds the last fire plus the cooldown"), Until && *Until == BeatTicks.FindRef(CivID).Last() + 60);

	// A Named row fires as often as it is fired by name and writes no key.
	{
		const int32 KeysBefore = State.EventCooldownUntilTick.Num();
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = 1;
		TestTrue(TEXT("FireRow fires a Named row"), Events->FireRow(Named, Ctx));
		TestTrue(TEXT("FireRow fires it again at once"), Events->FireRow(Named, Ctx));
		TestFalse(TEXT("FireRow of a missing row is false"), Events->FireRow(FName(TEXT("evt_no_such_row")), Ctx));
		TestEqual(TEXT("The Named row fired twice"), NamedFires, 2);
		TestEqual(TEXT("A row fired by name writes no cooldown key"), State.EventCooldownUntilTick.Num(), KeysBefore);
	}

	// The sweep at a multiple of 600 ticks drops the entries that are over and keeps the others.
	{
		FGalaxyState& Mutable = Sim->GetMutableState();
		const FName Over(TEXT("evt_test_over@civ_x"));
		const FName Alive(TEXT("evt_test_alive@civ_x"));
		Mutable.EventCooldownUntilTick.Add(Over, 500);
		Mutable.EventCooldownUntilTick.Add(Alive, 700);
		Mutable.Tick = 598;
		Sim->StepOnce();
		TestTrue(TEXT("Before the sweep an entry that is over is still there"), State.EventCooldownUntilTick.Contains(Over));
		Sim->StepOnce();
		TestTrue(TEXT("The sweep at tick 600 drops the entry that is over"), State.Tick == 600 && !State.EventCooldownUntilTick.Contains(Over));
		TestTrue(TEXT("The sweep keeps the entry that is not over"), State.EventCooldownUntilTick.Contains(Alive));
	}

	// A Standing row on settled systems: with a cooldown it fires for one unit per slot (the first that passes), with
	// none for every unit.
	for (const int32 Cooldown : { 60, 0 })
	{
		UDataTable* Units = NewTable(FCosmicEventDef::StaticStruct());
		Units->AddRow(Beat, MakeRow(ECosmicEventTrigger::Standing, ECosmicEventSubject::SettledSystem, 0.f, Cooldown));
		Sim->SetTables(WithEvents(Original, BaseRules, Units, 1, 1.f));
		Sim->NewGalaxy(42, 5, 3);
		FGalaxyState& Mutable = Sim->GetMutableState();
		FCivilizationState& Civ = Mutable.Civs[0];
		for (FStarSystemState& System : Mutable.Systems)
		{
			if (System.OwnerCivID.IsNone())
			{
				System.OwnerCivID = Civ.CivID;
				Civ.OwnedSystems.Add(System.SystemID);
			}
		}
		TArray<FName> FiredSystems;
		const FName Three = Civ.CivID;
		Scoped.Probe->OnEvent = [&FiredSystems, Three](const FCosmicEventNotice& Notice)
		{
			if (Notice.CivID == Three)
			{
				FiredSystems.Add(Notice.SystemID);
			}
		};
		Sim->StepOnce();
		if (Cooldown > 0)
		{
			TestTrue(TEXT("With a cooldown a SettledSystem Standing row fires for the first unit only"), FiredSystems.Num() == 1 && FiredSystems[0] == Civ.OwnedSystems[0]);
		}
		else
		{
			TestTrue(TEXT("Without a cooldown a SettledSystem Standing row fires for every unit, in OwnedSystems order"), FiredSystems == Civ.OwnedSystems);
		}
	}

	Scoped.Probe->OnEvent = nullptr;
	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicEventsRowAtomicTest, "Cosmic_Ascension.Events.RowAtomic", CosmicTests::Flags)
bool FCosmicEventsRowAtomicTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEventTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicEventSubsystem* Events = Sim->GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	// The one error line of the FireRow issued from inside a handler.
	AddExpectedError(TEXT("FireRow called from a handler of a Core Logic delegate"), EAutomationExpectedErrorFlags::Contains, 1);

	// A row with three effects of different kinds and a line.
	const FName RowName(TEXT("evt_test_atomic"));
	const FName SlotRow(TEXT("evt_test_slot"));
	FCosmicEventDef Def = MakeRow(ECosmicEventTrigger::Named, ECosmicEventSubject::Civilization, 0.f, 0);
	Def.Effects.Add(Eff(EEffectType::ChangeStability, nullptr, NAME_None, -0.1f));
	Def.Effects.Add(Eff(EEffectType::NameNotable, TEXT("Notable.Hearer"), NAME_None, 0.f));
	Def.Effects.Add(Eff(EEffectType::AddResource, nullptr, FName(TEXT("Matter")), 5.f));
	Def.LogTemplates = ThreeLines(TEXT("atomic"));
	Def.Severity = 1;
	FCosmicEventDef SlotDef = Def;
	SlotDef.Trigger = ECosmicEventTrigger::Standing;
	SlotDef.CooldownTicks = 500;
	UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
	Table->AddRow(RowName, Def);
	FCosmicDataTables Tables = WithEvents(Original, BaseRules, Table, 1, 1.f);
	Tables.Civs = CivsWithNames(Original);
	Sim->SetTables(Tables);
	Sim->NewGalaxy(42, 5, 3);
	const FGalaxyState& State = Sim->GetState();
	const FName CivID = State.Civs[1].CivID;

	// What the handlers saw: the world hash at the line and at the notice, and the order of the two.
	TArray<uint32> HashAtLine;
	TArray<FCosmicEventNotice> Notices;
	TArray<int32> LinesAtNotice;
	bool bKeyAtLine = false;
	bool bNestedFireRefused = true;
	bool bTryNestedFire = false;
	FScopedProbe Scoped(*Sim);
	Scoped.Probe->OnLog = [&](const FCosmicLogEntry& Entry)
	{
		HashAtLine.Add(Sim->ComputeWorldHash());
		if (Entry.FocusCivID == CivID)
		{
			bKeyAtLine = State.EventCooldownUntilTick.Contains(UCosmicEventSubsystem::CooldownKey(SlotRow, CivID));
		}
		if (bTryNestedFire)
		{
			// A handler that breaks its contract: the fire is refused and nothing changes. The context is built by hand:
			// MakeEvalContext would report the same mistake a second time. Tried once: a fire that is not refused must
			// fail this test, not run away.
			bTryNestedFire = false;
			const uint32 Before = Sim->ComputeStateHash();
			FCosmicEvalContext Ctx(Sim->GetMutableState(), Sim->GetRng(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
			Ctx.CivIndex = 1;
			bNestedFireRefused &= !Events->FireRow(RowName, Ctx) && Sim->ComputeStateHash() == Before;
		}
	};
	Scoped.Probe->OnEvent = [&](const FCosmicEventNotice& Notice)
	{
		Notices.Add(Notice);
		LinesAtNotice.Add(HashAtLine.Num());
	};

	// Fired by name.
	{
		const float StabilityBefore = State.Civs[1].Stability;
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = 1;
		TestTrue(TEXT("The row fires"), Events->FireRow(RowName, Ctx));
		Sim->SyncRngState();
		TestTrue(TEXT("Every effect was applied"), State.Civs[1].Stability < StabilityBefore && State.Civs[1].Notables.Num() == 1);
		TestTrue(TEXT("The line left once, after the last effect: the world at the line is the world after the row"),
			HashAtLine.Num() == 1 && HashAtLine[0] == Sim->ComputeWorldHash());
		TestTrue(TEXT("OnEventFired came once, after the line"), Notices.Num() == 1 && LinesAtNotice[0] == 1);
		if (Notices.Num() == 1)
		{
			TestTrue(TEXT("The notice carries the row, the subject and the tick"),
				Notices[0].EventRow == RowName && Notices[0].CivID == CivID && Notices[0].SystemID.IsNone()
				&& Notices[0].Tick == State.Tick && Notices[0].Severity == 1 && Notices[0].Category == Tag(TEXT("Event.Civ.Economy")));
		}
		TestTrue(TEXT("The entry carries the row, its log category and the people"),
			State.Log.Last().SourceRow == RowName && State.Log.Last().Category == Tag(TEXT("Log.Civ")) && State.Log.Last().FocusCivID == CivID
			&& State.Log.Last().FocusSystemID.IsNone() && State.Log.Last().Severity == 1);
		TestEqual(TEXT("The record names the row that made it"), State.Civs[1].Notables[0].SourceRow, RowName);
	}

	// A fire issued from a handler is refused with one error line and changes nothing.
	{
		bTryNestedFire = true;
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = 2;
		TestTrue(TEXT("The outer row still fires"), Events->FireRow(RowName, Ctx));
		bTryNestedFire = false;
		TestTrue(TEXT("A FireRow issued from a handler is refused and changes nothing"), bNestedFireRefused);
	}

	// On the slot path the cooldown key is already written when the line leaves.
	{
		UDataTable* SlotTable = NewTable(FCosmicEventDef::StaticStruct());
		SlotTable->AddRow(SlotRow, SlotDef);
		Tables.Events = SlotTable;
		Sim->SetTables(Tables);
		Sim->NewGalaxy(42, 5, 3);
		HashAtLine.Reset();
		Notices.Reset();
		bKeyAtLine = false;
		Sim->StepOnce();
		TestTrue(TEXT("The slot fired the Standing row for every civilization, each about its capital"),
			Notices.Num() == State.Civs.Num() && Notices[1].CivID == CivID && Notices[1].SystemID == State.Civs[1].HomeSystemID);
		TestTrue(TEXT("On the slot path the cooldown key is there when the line leaves"), bKeyAtLine);
	}

	// With the broadcast gate closed no handler is called and the world comes out the same.
	{
		const uint32 OpenWorld = Sim->ComputeWorldHash();
		Sim->NewGalaxy(42, 5, 3);
		HashAtLine.Reset();
		Notices.Reset();
		Sim->RunHeadless(1);
		TestTrue(TEXT("With the gate closed no handler is called"), HashAtLine.IsEmpty() && Notices.IsEmpty());
		TestEqual(TEXT("With the gate closed the world is the same"), static_cast<int64>(Sim->ComputeWorldHash()), static_cast<int64>(OpenWorld));
	}

	Scoped.Probe->OnLog = nullptr;
	Scoped.Probe->OnEvent = nullptr;
	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicEventsNotableDrawFixedTest, "Cosmic_Ascension.Events.NotableDrawFixed", CosmicTests::Flags)
bool FCosmicEventsNotableDrawFixedTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEventTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	FCosmicDataTables Named = Original;
	Named.Civs = CivsWithNames(Original);
	FCosmicRulesDef SmallRoster = BaseRules;
	SmallRoster.MaxNotablesPerCiv = 2;

	Sim->NewGalaxy(42, 5, 3);
	const FGalaxyState Fresh = Sim->GetState();
	const FCosmicEffect NameRebel = Eff(EEffectType::NameNotable, TEXT("Notable.Rebel"), NAME_None, 0.f);

	// Applies the effect to a detached copy and says whether a record was made and how far the stream moved.
	auto Try = [&](TFunctionRef<void(FGalaxyState&)> Prepare, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex, const FCosmicEffect& Effect, bool& bOutMade)
	{
		FGalaxyState Detached = Fresh;
		Prepare(Detached);
		FCosmicStateIndex Index;
		Index.Rebuild(Detached);
		FRandomStream Handed(55);
		FRandomStream Probe(55);
		FCosmicEvalContext Ctx(Detached, Handed, Index, Tables, Rules);
		Ctx.CivIndex = CivIndex;
		Ctx.SourceRow = FName(TEXT("evt_test"));
		bOutMade = UCosmicEventSubsystem::Apply(Effect, Ctx);
		Probe.GetFraction();
		return Handed.GetCurrentSeed() == Probe.GetCurrentSeed();
	};
	auto Nothing = [](FGalaxyState&) {};

	bool bMade = false;
	TestTrue(TEXT("A record made: one draw"), Try(Nothing, Named, BaseRules, 1, NameRebel, bMade) && bMade);
	TestTrue(TEXT("No civilization: refused, one draw"), Try(Nothing, Named, BaseRules, INDEX_NONE, NameRebel, bMade) && !bMade);
	TestTrue(TEXT("An empty name bank: refused, one draw"), Try(Nothing, Original, BaseRules, 1, NameRebel, bMade) && !bMade);
	TestTrue(TEXT("Trait.Mind.NoSelf: refused, one draw"),
		Try([](FGalaxyState& State) { State.Civs[1].Traits.AddTag(Tag(TEXT("Trait.Mind.NoSelf"))); }, Named, BaseRules, 1, NameRebel, bMade) && !bMade);
	TestTrue(TEXT("A people of the fourth rung: refused, one draw"),
		Try([](FGalaxyState& State) { State.Civs[1].Tier = EAscensionTier::Tier4_Transcendent; }, Named, BaseRules, 1, NameRebel, bMade) && !bMade);
	TestTrue(TEXT("An empty role tag: refused, one draw"), Try(Nothing, Named, BaseRules, 1, Eff(EEffectType::NameNotable, nullptr, NAME_None, 0.f), bMade) && !bMade);
	TestTrue(TEXT("A full roster of an open ruler and an open regent: refused, one draw"),
		Try([](FGalaxyState& State)
		{
			FNotableRecord& Ruler = State.Civs[1].Notables.AddDefaulted_GetRef();
			Ruler.Role = Tag(TEXT("Notable.Ruler"));
			Ruler.FirstTick = 1;
			FNotableRecord& Regent = State.Civs[1].Notables.AddDefaulted_GetRef();
			Regent.Role = Tag(TEXT("Notable.Regent"));
			Regent.NameIndex = 1;
			Regent.FirstTick = 2;
		}, Named, SmallRoster, 1, NameRebel, bMade) && !bMade);

	// The guards of a record that is made: the open record of the role is closed, the roster keeps its cap, the name is
	// one nobody carries.
	{
		FGalaxyState Detached = Fresh;
		Detached.Tick = 77;
		FCosmicStateIndex Index;
		Index.Rebuild(Detached);
		FRandomStream Handed(55);
		FCosmicEvalContext Ctx(Detached, Handed, Index, Named, SmallRoster);
		Ctx.CivIndex = 1;
		Ctx.SourceRow = FName(TEXT("evt_test"));
		UCosmicEventSubsystem::Apply(NameRebel, Ctx);
		UCosmicEventSubsystem::Apply(NameRebel, Ctx);
		const TArray<FNotableRecord>& Notables = Detached.Civs[1].Notables;
		TestTrue(TEXT("A second rebel closes the first: one closed and one open record"),
			Notables.Num() == 2 && Notables[0].LastTick == 77 && Notables[1].LastTick == 0 && Notables[1].FirstTick == 77 && Notables[1].SourceRow == Ctx.SourceRow);
		TestTrue(TEXT("The second record carries another name"), Notables.Num() == 2 && Notables[0].NameIndex != Notables[1].NameIndex);
		UCosmicEventSubsystem::Apply(NameRebel, Ctx);
		TestTrue(TEXT("At the cap the record closed longest ago leaves"), Notables.Num() == 2 && Notables[1].LastTick == 0);
	}

	// A whole run: the same sim stream with empty name banks and with filled ones, Install's draw included.
	auto SeedAfter = [Sim](const FCosmicDataTables& Tables)
	{
		Sim->SetTables(Tables);
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		Sim->RunHeadless(1000);
		return Sim->GetRng().GetCurrentSeed();
	};
	TestEqual(TEXT("Filling the name banks does not move the sim stream"), SeedAfter(Named), SeedAfter(Original));

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicEventsRoleEffectRulesTest, "Cosmic_Ascension.Events.RoleEffectRules", CosmicTests::Flags)
bool FCosmicEventsRoleEffectRulesTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEventTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	// The three effects that wait for the hazard step each leave one error line.
	AddExpectedError(TEXT("reaches into the hazard stage"), EAutomationExpectedErrorFlags::Contains, 3);

	Sim->NewGalaxy(42, 5, 3);
	const FGalaxyState Fresh = Sim->GetState();
	const FCosmicEffect Kill = Eff(EEffectType::KillPlayer, TEXT("Death.Hazard"), NAME_None, 0.f);
	const FCosmicEffect Depose = Eff(EEffectType::DeposeRuler, TEXT("Reign.Deposed"), NAME_None, 0.f);
	const FGameplayTag KillTag = Tag(TEXT("Effect.KillPlayer"));
	const FGameplayTag DeposeTag = Tag(TEXT("Effect.DeposeRuler"));
	const FName Row(TEXT("evt_test"));
	const int64 PastProtection = Sim->GetRules().BirthProtectionTicks + 1;

	// One detached state per case: the first-life ruler of civ 0, standing in sol, at a tick the test picks.
	struct FBench
	{
		FBench(const FGalaxyState& InState, UGalaxySimulationSubsystem& InSim, int64 InTick)
			: State(InState), Stream(9), Ctx(State, Stream, Index, InSim.GetTables(), InSim.GetRules())
		{
			State.Tick = InTick;
			Index.Rebuild(State);
			Ctx.SourceRow = FName(TEXT("evt_test"));
		}

		FGalaxyState State;
		FCosmicStateIndex Index;
		FRandomStream Stream;
		FCosmicEvalContext Ctx;
	};

	// First recorded wins, in both orders.
	{
		FBench Bench(Fresh, *Sim, PastProtection);
		Bench.Ctx.CivIndex = 0;
		TestTrue(TEXT("DeposeRuler of the ruled civilization is recorded"), UCosmicEventSubsystem::Apply(Depose, Bench.Ctx));
		TestFalse(TEXT("A KillPlayer after it is turned away"), UCosmicEventSubsystem::Apply(Kill, Bench.Ctx));
		const FCallingState& Calling = Bench.State.Calling;
		TestTrue(TEXT("The deposition stays pending with its cause and its row"),
			Calling.PendingEffect == DeposeTag && Calling.PendingCause == Depose.Tag && Calling.PendingSourceRow == Row);
		TestTrue(TEXT("Nothing ran in place: the vessel still rules"), Bench.State.Player.Role == EPlayerRole::Ruler && Bench.State.Civs[0].bIsPlayerRuled);
	}
	{
		FBench Bench(Fresh, *Sim, PastProtection);
		Bench.Ctx.CivIndex = 0;
		TestTrue(TEXT("KillPlayer without a system reaches the ruler of the subject civilization"), UCosmicEventSubsystem::Apply(Kill, Bench.Ctx));
		TestFalse(TEXT("A DeposeRuler after it is turned away"), UCosmicEventSubsystem::Apply(Depose, Bench.Ctx));
		TestTrue(TEXT("The kill stays pending"), Bench.State.Calling.PendingEffect == KillTag && Bench.State.Calling.PendingCause == Kill.Tag);
	}

	// Birth protection is judged at the record: a shielded kill leaves the slot free.
	{
		FBench Bench(Fresh, *Sim, PastProtection - 2);
		Bench.Ctx.CivIndex = 0;
		TestFalse(TEXT("A KillPlayer inside the birth protection is turned away"), UCosmicEventSubsystem::Apply(Kill, Bench.Ctx));
		TestFalse(TEXT("A shielded kill leaves the slot free"), Bench.State.Calling.PendingEffect.IsValid());
		TestTrue(TEXT("A DeposeRuler after a shielded kill is recorded"), UCosmicEventSubsystem::Apply(Depose, Bench.Ctx) && Bench.State.Calling.PendingEffect == DeposeTag);
	}

	// What happens to a system reaches only a wanderer standing in it.
	{
		FBench Bench(Fresh, *Sim, PastProtection);
		Bench.Ctx.CivIndex = 0;
		Bench.Ctx.SystemIndex = 0;
		TestFalse(TEXT("A hazard on the capital does not kill the ruler standing in it"), UCosmicEventSubsystem::Apply(Kill, Bench.Ctx));
		TestFalse(TEXT("Nothing is pending for the ruler"), Bench.State.Calling.PendingEffect.IsValid());

		Bench.State.Player.Role = EPlayerRole::Wanderer;
		Bench.State.Player.RuledCivID = NAME_None;
		Bench.Ctx.SystemIndex = 1;
		TestFalse(TEXT("A wanderer in another system is not reached"), UCosmicEventSubsystem::Apply(Kill, Bench.Ctx));
		Bench.Ctx.SystemIndex = 0;
		TestTrue(TEXT("A wanderer standing in the subject system is reached"), UCosmicEventSubsystem::Apply(Kill, Bench.Ctx) && Bench.State.Calling.PendingEffect == KillTag);
	}

	// Without a system: only the ruler of the subject civilization, or anyone when no civilization is named.
	{
		FBench Bench(Fresh, *Sim, PastProtection);
		Bench.Ctx.CivIndex = 1;
		TestFalse(TEXT("The extinction of another people does not kill the vessel"), UCosmicEventSubsystem::Apply(Kill, Bench.Ctx));
		TestFalse(TEXT("DeposeRuler of a civilization the vessel does not rule is turned away"), UCosmicEventSubsystem::Apply(Depose, Bench.Ctx));
		Bench.Ctx.CivIndex = INDEX_NONE;
		TestTrue(TEXT("A subject that names nothing reaches the vessel wherever it is"), UCosmicEventSubsystem::Apply(Kill, Bench.Ctx));
	}

	// During the interregnum there is no vessel: no role effect, no vessel effect.
	{
		FBench Bench(Fresh, *Sim, PastProtection);
		Bench.State.Calling.LifeState = ELifeState::Interregnum;
		Bench.Ctx.CivIndex = 0;
		const FPlayerEntityState Vessel = Bench.State.Player;
		const bool bAny = UCosmicEventSubsystem::Apply(Kill, Bench.Ctx) || UCosmicEventSubsystem::Apply(Depose, Bench.Ctx)
			|| UCosmicEventSubsystem::Apply(Eff(EEffectType::PlayerStatDelta, nullptr, FName(TEXT("Body")), 2.f), Bench.Ctx)
			|| UCosmicEventSubsystem::Apply(Eff(EEffectType::GrantPower, TEXT("Way.Received"), NAME_None, 0.f), Bench.Ctx);
		TestFalse(TEXT("During the interregnum role effects and vessel effects are turned away"), bAny);
		TestTrue(TEXT("During the interregnum nothing is pending and the calling gains no power"),
			!Bench.State.Calling.PendingEffect.IsValid() && Bench.State.Calling.Powers.IsEmpty() && Bench.State.Player.Body == Vessel.Body);
	}
	{
		FBench Bench(Fresh, *Sim, PastProtection);
		TestTrue(TEXT("While embodied PlayerStatDelta and GrantPower apply"),
			UCosmicEventSubsystem::Apply(Eff(EEffectType::PlayerStatDelta, nullptr, FName(TEXT("Body")), 2.f), Bench.Ctx)
			&& UCosmicEventSubsystem::Apply(Eff(EEffectType::GrantPower, TEXT("Way.Received"), NAME_None, 0.f), Bench.Ctx)
			&& Bench.State.Player.Body == Fresh.Player.Body + 2 && Bench.State.Calling.Powers.HasTagExact(Tag(TEXT("Way.Received"))));
	}

	// MultiplyIncome belongs to the per-tick sources.
	{
		FBench Bench(Fresh, *Sim, PastProtection);
		Bench.Ctx.CivIndex = 0;
		Bench.State.Civs[0].IncomePerTick.Energy = 4.f;
		const FCosmicEffect Halve = Eff(EEffectType::MultiplyIncome, nullptr, FName(TEXT("Energy")), 0.5f);
		TestFalse(TEXT("MultiplyIncome outside a per-tick source is turned away"), UCosmicEventSubsystem::Apply(Halve, Bench.Ctx));
		TestEqual(TEXT("IncomePerTick is as it was"), Bench.State.Civs[0].IncomePerTick.Energy, 4.f);
		Bench.Ctx.bPerTickSource = true;
		TestTrue(TEXT("MultiplyIncome from a per-tick source applies"), UCosmicEventSubsystem::Apply(Halve, Bench.Ctx));
		TestEqual(TEXT("IncomePerTick is scaled"), Bench.State.Civs[0].IncomePerTick.Energy, 2.f);
	}

	// The plain world effects, and the three that wait for the hazard step.
	{
		FBench Bench(Fresh, *Sim, PastProtection);
		Bench.Ctx.CivIndex = 1;
		Bench.Ctx.SystemIndex = 1;
		Bench.Ctx.OtherID = Fresh.Civs[2].CivID;
		FCivilizationState& Civ = Bench.State.Civs[1];
		Civ.Stockpile.Matter = 3.f;
		Civ.Stability = 0.5f;
		Civ.Population = 1.f;
		const FGameplayTag Seeking = Tag(TEXT("Trait.Faith.Seeking"));
		bool bAll = UCosmicEventSubsystem::Apply(Eff(EEffectType::AddResource, nullptr, FName(TEXT("Matter")), -10.f), Bench.Ctx);
		bAll &= UCosmicEventSubsystem::Apply(Eff(EEffectType::ShiftEthics, nullptr, FName(TEXT("Openness")), 5.f), Bench.Ctx);
		bAll &= UCosmicEventSubsystem::Apply(Eff(EEffectType::AddTrait, TEXT("Trait.Faith.Seeking"), NAME_None, 0.f), Bench.Ctx);
		bAll &= UCosmicEventSubsystem::Apply(Eff(EEffectType::ChangeStability, nullptr, NAME_None, 0.25f), Bench.Ctx);
		bAll &= UCosmicEventSubsystem::Apply(Eff(EEffectType::ChangeRelation, nullptr, NAME_None, -0.5f), Bench.Ctx);
		bAll &= UCosmicEventSubsystem::Apply(Eff(EEffectType::ChangeReverence, nullptr, NAME_None, 0.3f), Bench.Ctx);
		bAll &= UCosmicEventSubsystem::Apply(Eff(EEffectType::SetFlag, TEXT("Flag.Grace.Spent"), NAME_None, 0.f), Bench.Ctx);
		bAll &= UCosmicEventSubsystem::Apply(Eff(EEffectType::ChangePopulation, nullptr, NAME_None, -0.4f), Bench.Ctx);
		TestTrue(TEXT("The world effects apply to the subject civilization"), bAll);
		TestTrue(TEXT("The world effects wrote what they say"),
			Civ.Stockpile.Matter == 0.f && Civ.Ethics.Openness == 1.f && Civ.Traits.HasTagExact(Seeking) && Civ.Stability == 0.75f
			&& Civ.Relations.FindRef(Fresh.Civs[2].CivID) == -0.5f && Civ.ReverenceToPlayer == 0.3f
			&& Bench.State.Flags.HasTagExact(Tag(TEXT("Flag.Grace.Spent"))) && FMath::IsNearlyEqual(Civ.Population, 0.6f));
		TestTrue(TEXT("RemoveTrait takes the trait away again"),
			UCosmicEventSubsystem::Apply(Eff(EEffectType::RemoveTrait, TEXT("Trait.Faith.Seeking"), NAME_None, 0.f), Bench.Ctx) && !Civ.Traits.HasTagExact(Seeking));

		const bool bHazardEffects = UCosmicEventSubsystem::Apply(Eff(EEffectType::SpawnHazard, TEXT("Hazard.Plague"), NAME_None, 0.f), Bench.Ctx)
			|| UCosmicEventSubsystem::Apply(Eff(EEffectType::ResolveHazard, nullptr, NAME_None, 0.f), Bench.Ctx)
			|| UCosmicEventSubsystem::Apply(Eff(EEffectType::TransferSystem, nullptr, NAME_None, 0.f), Bench.Ctx);
		TestFalse(TEXT("SpawnHazard, ResolveHazard and TransferSystem are turned away until the hazard step"), bHazardEffects);
		TestTrue(TEXT("The turned-away effects left the system alone"),
			!Bench.State.Systems[1].ActiveHazard.IsValid() && Bench.State.Systems[1].OwnerCivID == Fresh.Systems[1].OwnerCivID);

		Bench.Ctx.CivIndex = INDEX_NONE;
		TestFalse(TEXT("A civ effect without a civilization is turned away"), UCosmicEventSubsystem::Apply(Eff(EEffectType::ChangeStability, nullptr, NAME_None, 0.25f), Bench.Ctx));
	}

	Sim->NewGalaxy(42, 5, 3);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

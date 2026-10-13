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
#include "UObject/Class.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#include <limits>

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

	// The same table with no person name on any row: the control side of the name-bank comparisons. Built here and not
	// taken from the game, so that the comparison stays one of empty against filled when the game's banks are filled.
	UDataTable* CivsWithoutNames(const FCosmicDataTables& Base)
	{
		UDataTable* Civs = NewTable(FCivArchetypeDef::StaticStruct());
		for (const FName RowName : Base.GetCivRowNames())
		{
			FCivArchetypeDef Def = *Base.FindCiv(RowName);
			Def.PersonNameBank.Reset();
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
		// The two transcendents of the cases and no other: whatever roster the tables of the game gave is taken out.
		Detached.Transcendents.Reset();
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
	Detached.Calling.BornTick = 10;
	Detached.Flags.AddTag(Tag(TEXT("Flag.Grace.Spent")));
	FCosmicStateIndex Index;
	Index.Rebuild(Detached);

	// The rules the cases are judged under: aging off, whatever DT_Rules says today.
	FCosmicRulesDef NoAging = Sim->GetRules();
	NoAging.BaseLifespanTicks = 0;

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
		// The transcendent of the test has no row in DT_Transcendents, so nothing presses on sol: 0 is reached, more is not.
		{ Cond(EConditionType::SystemPressureAtLeast, nullptr, NAME_None, 0.f), 0, true },
		{ Cond(EConditionType::SystemPressureAtLeast, nullptr, NAME_None, 0.1f), 0, false },
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
		// The boundary of every "at least": the bound itself passes.
		{ Cond(EConditionType::CivStabilityBelowFor, nullptr, FName(TEXT("120")), 0.3f), 0, true },
		{ Cond(EConditionType::MegaprojectComplete, nullptr, NAME_None, 0.f), 50, true },
		{ Cond(EConditionType::TranscendentSealed, nullptr, TrSealed, 0.f), 40, true },
		// A tick count that is not digits alone is not a tick count: false, never its numeric prefix.
		{ Cond(EConditionType::CivStabilityBelowFor, nullptr, FName(TEXT("1,000")), 0.3f), 0, false },
		{ Cond(EConditionType::CivStabilityBelowFor, nullptr, FName(TEXT("100ticks")), 0.3f), 0, false },
		// Nine digits are read (leading zeros and all), ten are not a count any more.
		{ Cond(EConditionType::CivStabilityBelowFor, nullptr, FName(TEXT("000000100")), 0.3f), 0, true },
		{ Cond(EConditionType::CivStabilityBelowFor, nullptr, FName(TEXT("0000000100")), 0.3f), 0, false },
		// The other party has to be a living people or a transcendent of the run. A missing entry reads as 0.
		{ Cond(EConditionType::CivRelationAtLeast, nullptr, FName(TEXT("no_such_party")), -1.f), 0, false },
		{ Cond(EConditionType::CivRelationAtLeast, nullptr, TrActive, 0.f), 0, true },
		{ Cond(EConditionType::CivRelationAtLeast, nullptr, TrActive, 0.1f), 0, false },
	};

	auto Answers = [&](const FGalaxyState& State, int32 CivIndex, int32 SystemIndex)
	{
		TArray<bool> Out;
		for (const FCase& Case : Cases)
		{
			FCosmicEvalView View(State, Index, Sim->GetTables(), NoAging);
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
	// The count comes from reflection (the last entry of a UENUM is the generated _MAX), so an enumerator appended later
	// fails here until it has a case.
	TestEqual(TEXT("Every condition type is covered"), Types.Num(), StaticEnum<EConditionType>()->NumEnums() - 1);

	// The two flag conditions the other way round: the cases above read a flag that is set.
	{
		FGalaxyState NoFlags = Detached;
		NoFlags.Flags.Reset();
		FCosmicEvalView View(NoFlags, Index, Sim->GetTables(), NoAging);
		TestFalse(TEXT("FlagSet is false for a flag that is not set"), UCosmicEventSubsystem::Check(Cond(EConditionType::FlagSet, TEXT("Flag.Grace.Spent"), NAME_None, 0.f), View));
		TestTrue(TEXT("FlagNotSet is true for a flag that is not set"), UCosmicEventSubsystem::Check(Cond(EConditionType::FlagNotSet, TEXT("Flag.Grace.Spent"), NAME_None, 0.f), View));
	}

	// PlayerAgeAtLeast with aging on: the age is the ticks since the birth of the vessel (tick 100, born at 10).
	{
		FCosmicRulesDef Aging = NoAging;
		Aging.BaseLifespanTicks = 1000;
		FGalaxyState Unborn = Detached;
		Unborn.Calling.LifeState = ELifeState::Interregnum;
		auto AgeAtLeast = [&](const FGalaxyState& State, const FCosmicRulesDef& Rules, float Value)
		{
			FCosmicEvalView View(State, Index, Sim->GetTables(), Rules);
			return UCosmicEventSubsystem::Check(Cond(EConditionType::PlayerAgeAtLeast, nullptr, NAME_None, Value), View);
		};
		TestTrue(TEXT("With aging on PlayerAgeAtLeast is true at the age of the vessel"), AgeAtLeast(Detached, Aging, 90.f));
		TestFalse(TEXT("With aging on PlayerAgeAtLeast is false one tick above the age"), AgeAtLeast(Detached, Aging, 91.f));
		TestFalse(TEXT("With aging on PlayerAgeAtLeast is false during the interregnum"), AgeAtLeast(Unborn, Aging, 90.f));
		TestFalse(TEXT("With aging off PlayerAgeAtLeast is false at any age"), AgeAtLeast(Detached, NoAging, 0.f));
	}

	// SystemPressureAtLeast reads the pressure function (section 4.2): with a row for the transcendent whose sphere holds
	// sol, the pressure there is the weight of a tier-4 source. The name of a celestial row of DT_Transcendents is a
	// relation party without an instance in the run (a celestial transcendent has none); a fallen row is a party only
	// as an instance.
	{
		const FName Celestial(TEXT("tr_test_celestial"));
		FTranscendentDef Presser;
		Presser.Doctrine = Tag(TEXT("Doctrine.Silence"));
		Presser.Allegiance = Tag(TEXT("Realm.Abyssal"));
		Presser.Tier = EAscensionTier::Tier4_Transcendent;
		Presser.Depth = 0.f;
		FTranscendentDef Heavenly;
		Heavenly.Doctrine = Tag(TEXT("Doctrine.Covenant"));
		Heavenly.Allegiance = Tag(TEXT("Realm.Celestial"));
		// A fallen row the roster did not draw: in the table, without an instance in the run.
		const FName Absent(TEXT("tr_test_absent"));
		FTranscendentDef NotDrawn = Presser;
		NotDrawn.Doctrine = Tag(TEXT("Doctrine.Recurrence"));
		UDataTable* Table = NewTable(FTranscendentDef::StaticStruct());
		Table->AddRow(TrActive, Presser);
		Table->AddRow(Celestial, Heavenly);
		Table->AddRow(Absent, NotDrawn);
		FCosmicDataTables WithRows = Sim->GetTables();
		WithRows.Transcendents = Table;
		const float Weight = NoAging.PressureBySourceTier.IsValidIndex(3) ? NoAging.PressureBySourceTier[3] : 0.f;
		auto CheckWith = [&](const FGalaxyState& State, const FCosmicDataTables& Tables, const FCosmicCondition& Condition, int32 SystemIndex)
		{
			FCosmicEvalView View(State, Index, Tables, NoAging);
			View.CivIndex = 0;
			View.SystemIndex = SystemIndex;
			return UCosmicEventSubsystem::Check(Condition, View);
		};
		TestTrue(TEXT("The rules give a tier-4 source a weight"), Weight > 0.f);
		TestTrue(TEXT("SystemPressureAtLeast is true at the pressure of the system"),
			CheckWith(Detached, WithRows, Cond(EConditionType::SystemPressureAtLeast, nullptr, NAME_None, Weight), 0));
		TestFalse(TEXT("SystemPressureAtLeast is false above the pressure of the system"),
			CheckWith(Detached, WithRows, Cond(EConditionType::SystemPressureAtLeast, nullptr, NAME_None, Weight * 1.01f), 0));
		TestFalse(TEXT("SystemPressureAtLeast is false without a system"),
			CheckWith(Detached, WithRows, Cond(EConditionType::SystemPressureAtLeast, nullptr, NAME_None, 0.f), INDEX_NONE));
		TestTrue(TEXT("The name of a celestial row is a relation party without an instance in the run"),
			CheckWith(Detached, WithRows, Cond(EConditionType::CivRelationAtLeast, nullptr, Celestial, 0.f), 0));
		TestFalse(TEXT("The same name is no party when the tables do not hold the row"),
			CheckWith(Detached, Sim->GetTables(), Cond(EConditionType::CivRelationAtLeast, nullptr, Celestial, 0.f), 0));
		TestFalse(TEXT("The name of a fallen row without an instance in the run is no party"),
			CheckWith(Detached, WithRows, Cond(EConditionType::CivRelationAtLeast, nullptr, Absent, 0.f), 0));

		// The pressure never reads knowing or telling, shown where the pressure is not zero: a zero would stay a zero
		// whatever it were multiplied by.
		FGalaxyState ToldUnderPressure = Detached;
		ToldUnderPressure.Knowing.MetCivs.AddDefaulted_GetRef().ID = Civ1;
		ToldUnderPressure.Knowing.FirstContactCivID = Civ1;
		ToldUnderPressure.Log.AddDefaulted();
		ToldUnderPressure.Chronicle.AddDefaulted();
		ToldUnderPressure.TellRngState += 7;
		const float PressureKnown = UGalaxySimulationSubsystem::ComputePressureIn(ToldUnderPressure, Index, WithRows, NoAging, 0).P;
		const float PressurePlain = UGalaxySimulationSubsystem::ComputePressureIn(Detached, Index, WithRows, NoAging, 0).P;
		TestTrue(TEXT("Knowing, Log, Chronicle and TellRngState change no pressure above zero and no answer of the pressure condition"),
			PressureKnown == PressurePlain && PressurePlain == Weight && Weight > 0.f
			&& CheckWith(ToldUnderPressure, WithRows, Cond(EConditionType::SystemPressureAtLeast, nullptr, NAME_None, Weight), 0)
			&& !CheckWith(ToldUnderPressure, WithRows, Cond(EConditionType::SystemPressureAtLeast, nullptr, NAME_None, Weight * 1.01f), 0));
	}

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

	// The error lines of the two out-of-range knobs at the end: each names its own knob and nothing else.
	AddExpectedError(TEXT("EventSlotTicks 0 is below 1, read as 1"), EAutomationExpectedErrorFlags::Contains, 1);
	// Twice: for a chance of -0.1 and for a chance that is not a number.
	AddExpectedError(TEXT("EventChancePerSlot .* is below 0, read as 0"), EAutomationExpectedErrorFlags::Contains, 2);

	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };
	const int32 SlotTicks = BaseRules.EventSlotTicks;
	if (!TestTrue(TEXT("DT_Rules holds a slot period (run Tools/make_data_tables.py)"), SlotTicks >= 1))
	{
		return false;
	}

	// One call draws once per civilization that has a slot in that tick, and for nobody else. A slot needs a settled
	// system: a civilization whose only system is still being settled and one that owns nothing have none.
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	FGalaxyState& State = Sim->GetMutableState();
	{
		const FName Settling = State.Civs[1].CivID;
		const FName Landless = State.Civs[2].CivID;
		for (FStarSystemState& System : State.Systems)
		{
			if (System.OwnerCivID == Settling)
			{
				System.ActiveMegaproject = CosmicRows::ProjectSettle;
			}
			else if (System.OwnerCivID == Landless)
			{
				System.OwnerCivID = NAME_None;
			}
		}
		State.Civs[2].OwnedSystems.Reset();
	}
	// The predicate written out, not taken from the code under test.
	auto HasSettledSystem = [&State](const FCivilizationState& Civ)
	{
		for (const FStarSystemState& System : State.Systems)
		{
			if (System.OwnerCivID == Civ.CivID && System.ActiveMegaproject != CosmicRows::ProjectSettle && Civ.OwnedSystems.Contains(System.SystemID))
			{
				return true;
			}
		}
		return false;
	};
	bool bDrawsFollowSlots = true;
	int32 SlotsSeen = 0;
	int32 SlotsByPhaseAlone = 0;
	for (int64 Tick = 1; Tick <= 2 * SlotTicks; ++Tick)
	{
		State.Tick = Tick;
		int32 Expected = 0;
		for (const FCivilizationState& Civ : State.Civs)
		{
			const bool bPhase = SlotPhase(State, Civ.CivID, SlotTicks) == Tick % SlotTicks;
			Expected += (bPhase && HasSettledSystem(Civ)) ? 1 : 0;
			SlotsByPhaseAlone += bPhase ? 1 : 0;
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
	// The check above is only worth something if the two civilizations without a settled system were due in these ticks.
	TestTrue(TEXT("The two periods held slots, and four more by phase alone for the two civilizations without a settled system"),
		SlotsSeen > 0 && SlotsByPhaseAlone == SlotsSeen + 4);

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

	// The units of a slot follow ownership and the settled predicate: a civilization without a settled system has no
	// slot and no row; a system that is still being settled, or that is listed in OwnedSystems but owned by another
	// people, is no unit; the capital is HomeSystemID, wherever it stands in OwnedSystems.
	{
		const FName BeatRow(TEXT("evt_test_beat"));
		const FName UnitRow(TEXT("evt_test_unit"));
		UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
		Table->AddRow(BeatRow, MakeRow(ECosmicEventTrigger::Standing, ECosmicEventSubject::Civilization, 0.f, 0));
		Table->AddRow(UnitRow, MakeRow(ECosmicEventTrigger::Standing, ECosmicEventSubject::SettledSystem, 0.f, 0));
		Sim->SetTables(WithEvents(Original, BaseRules, Table, 1, 0.5f));
		Sim->NewGalaxy(42, 7, 4);

		FCivilizationState& Wide = State.Civs[0];
		TArray<FName> Free;
		for (FStarSystemState& System : State.Systems)
		{
			if (System.OwnerCivID.IsNone())
			{
				System.OwnerCivID = Wide.CivID;
				Free.Add(System.SystemID);
			}
		}
		if (TestTrue(TEXT("The test galaxy has four civilizations and three free systems"), State.Civs.Num() == 4 && Free.Num() == 3))
		{
			const FName WideID = Wide.CivID;
			const FName Home = Wide.HomeSystemID;
			const FName OtherID = State.Civs[3].CivID;
			const FName OtherHome = State.Civs[3].HomeSystemID;
			// Civilization 0: a free system first, then its capital, then two more (the last still being settled), then
			// the home of civilization 3, which it lists but does not own.
			Wide.OwnedSystems = { Free[0], Home, Free[1], Free[2], OtherHome };
			State.Systems[Sim->GetStateIndex().FindSystem(State, Free[2])].ActiveMegaproject = CosmicRows::ProjectSettle;
			// Civilization 1: its only system is still being settled. Civilization 2: owns nothing.
			State.Systems[Sim->GetStateIndex().FindSystem(State, State.Civs[1].HomeSystemID)].ActiveMegaproject = CosmicRows::ProjectSettle;
			State.Systems[Sim->GetStateIndex().FindSystem(State, State.Civs[2].HomeSystemID)].OwnerCivID = NAME_None;
			State.Civs[2].OwnedSystems.Reset();

			TArray<FCosmicEventNotice> Notices;
			FScopedProbe Scoped(*Sim);
			Scoped.Probe->OnEvent = [&Notices](const FCosmicEventNotice& Notice) { Notices.Add(Notice); };
			State.Tick = 5;
			FRandomStream Handed(123);
			FRandomStream Probe(123);
			const int32 Fired = Events->EvaluateAndFire(State, Handed);
			Probe.GetFraction();
			Probe.GetFraction();
			TestEqual(TEXT("Only the two civilizations with a settled system draw"), Handed.GetCurrentSeed(), Probe.GetCurrentSeed());

			// Row-name order, then OwnedSystems order.
			struct FExpected
			{
				FName Row;
				FName CivID;
				FName SystemID;
			};
			const FExpected Expected[] = {
				{ BeatRow, WideID, Home }, { UnitRow, WideID, Free[0] }, { UnitRow, WideID, Home }, { UnitRow, WideID, Free[1] },
				{ BeatRow, OtherID, OtherHome }, { UnitRow, OtherID, OtherHome }
			};
			bool bUnits = Fired == static_cast<int32>(UE_ARRAY_COUNT(Expected)) && Notices.Num() == Fired;
			for (int32 NoticeIndex = 0; bUnits && NoticeIndex < Notices.Num(); ++NoticeIndex)
			{
				bUnits &= Notices[NoticeIndex].EventRow == Expected[NoticeIndex].Row && Notices[NoticeIndex].CivID == Expected[NoticeIndex].CivID
					&& Notices[NoticeIndex].SystemID == Expected[NoticeIndex].SystemID;
			}
			TestTrue(TEXT("Rows fire on the settled systems a civilization owns and on its capital, and on nothing else"), bUnits);
		}
	}

	// The numbers of a tick are all drawn before any row is looked at: a Standing row whose effect draws (NameNotable)
	// does not move the Lottery number of the civilizations that come after.
	{
		const FName LotRow(TEXT("evt_test_lot"));
		UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
		FCosmicEventDef Naming = MakeRow(ECosmicEventTrigger::Standing, ECosmicEventSubject::Civilization, 0.f, 0);
		Naming.Effects.Add(Eff(EEffectType::NameNotable, TEXT("Notable.Hearer"), NAME_None, 0.f));
		Table->AddRow(FName(TEXT("evt_test_naming")), Naming);
		Table->AddRow(LotRow, MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 0));
		FCosmicDataTables Tables = WithEvents(Original, BaseRules, Table, 1, 0.5f);
		Tables.Civs = CivsWithNames(Original);
		Sim->SetTables(Tables);
		Sim->NewGalaxy(42, 5, 3);

		TSet<FName> LotFor;
		FScopedProbe Scoped(*Sim);
		Scoped.Probe->OnEvent = [&LotFor, LotRow](const FCosmicEventNotice& Notice)
		{
			if (Notice.EventRow == LotRow)
			{
				LotFor.Add(Notice.CivID);
			}
		};
		FRandomStream Handed(321);
		FRandomStream Probe(321);
		bool bRollsFirst = true;
		int32 LotFires = 0;
		for (int32 Round = 0; Round < 20; ++Round)
		{
			LotFor.Reset();
			++State.Tick;
			Events->EvaluateAndFire(State, Handed);
			TArray<float> Rolls;
			for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
			{
				Rolls.Add(Probe.GetFraction());
			}
			// Then one number for the NameNotable of every civilization's Standing row.
			for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
			{
				Probe.GetFraction();
			}
			for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
			{
				bRollsFirst &= LotFor.Contains(State.Civs[CivIndex].CivID) == (Rolls[CivIndex] < 0.5f);
			}
			bRollsFirst &= Handed.GetCurrentSeed() == Probe.GetCurrentSeed();
			LotFires += LotFor.Num();
		}
		TestTrue(TEXT("Every civilization's Lottery number is the one drawn before any row fired"), bRollsFirst);
		TestTrue(TEXT("The Lottery row fired in some slots and not in others"), LotFires > 10 && LotFires < 50);
	}

	// No civilization: nothing is drawn and nothing fires.
	{
		Sim->SetTables(Original);
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

	// A chance below 0, and one that is not a number, is read as 0, with an error line that names that knob alone: the
	// period is read as it is, the draws stay and no Lottery row fires.
	for (const float BadChance : { -0.1f, std::numeric_limits<float>::quiet_NaN() })
	{
		UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
		Table->AddRow(FName(TEXT("evt_test_lot")), MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 0));
		Sim->SetTables(WithEvents(Original, BaseRules, Table, 10, BadChance));
		Sim->NewGalaxy(42, 5, 3);
		// A tick in which at least the first civilization is due.
		State.Tick = 10 + SlotPhase(State, State.Civs[0].CivID, 10);
		int32 Due = 0;
		for (const FCivilizationState& Civ : State.Civs)
		{
			Due += (SlotPhase(State, Civ.CivID, 10) == State.Tick % 10) ? 1 : 0;
		}
		FRandomStream Handed(123);
		FRandomStream Probe(123);
		const int32 Fired = Events->EvaluateAndFire(State, Handed);
		for (int32 Draw = 0; Draw < Due; ++Draw)
		{
			Probe.GetFraction();
		}
		TestTrue(TEXT("With a chance out of range the slots still draw by the period of 10 and no row fires"),
			Due >= 1 && Fired == 0 && Handed.GetCurrentSeed() == Probe.GetCurrentSeed());
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

	// The same at the top of the range: ten shares of 0.1 add up to a hair above 1 in float, and the two largest numbers
	// the stream can give (1 - 2^-23 and 1 - 2^-22) still land on a row. The number is scaled, not each share: dividing
	// every share would leave a sliver at the top on which no row fires.
	{
		UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
		for (int32 RowIndex = 0; RowIndex < 10; ++RowIndex)
		{
			Table->AddRow(FName(*FString::Printf(TEXT("evt_test_ten_%d"), RowIndex)), MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 0));
		}
		Sim->SetTables(WithEvents(Original, BaseRules, Table, 1, 0.1f));
		FGalaxyState& State = Sim->GetMutableState();
		const FName First = State.Civs[0].CivID;
		// The first number of each of these streams is the roll of the first civilization.
		for (const int32 StreamSeed : { -381761311, 1004703457 })
		{
			if (!TestTrue(TEXT("The stream seed gives one of the two largest fractions first"), FRandomStream(StreamSeed).GetFraction() > 0.9999997f))
			{
				continue;
			}
			int32 FiresForFirst = 0;
			Scoped.Probe->OnEvent = [&FiresForFirst, First](const FCosmicEventNotice& Notice) { FiresForFirst += (Notice.CivID == First) ? 1 : 0; };
			FRandomStream Handed(StreamSeed);
			++State.Tick;
			Events->EvaluateAndFire(State, Handed);
			TestEqual(TEXT("With shares above a total of 1 the largest numbers of the stream still pick a row"), FiresForFirst, 1);
		}
		Scoped.Probe->OnEvent = nullptr;
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

	// A row that is cooling down gives up its own share and nobody else's: the row before it keeps its interval.
	{
		const FName Steady(TEXT("evt_test_cd_a"));
		const FName Cooling(TEXT("evt_test_cd_b"));
		UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
		Table->AddRow(Steady, MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 0));
		Table->AddRow(Cooling, MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 100000));
		Sim->NewGalaxy(42, 5, 3);
		const int32 CivCount = Sim->GetState().Civs.Num();
		TArray<bool> bCoolingFor;
		bCoolingFor.Init(false, CivCount);
		bool bOwnShareOnly = true;
		int32 CivTurn = 0;
		Play(Table, 0.25f, 40, [&](float Roll, const TArray<FCosmicEventNotice>& Fired)
		{
			const int32 CivIndex = CivTurn++ % CivCount;
			FName Expected;
			if (Roll < 0.25f)
			{
				Expected = Steady;
			}
			else if (Roll < 0.5f && !bCoolingFor[CivIndex])
			{
				Expected = Cooling;
				bCoolingFor[CivIndex] = true;
			}
			bOwnShareOnly &= Expected.IsNone() ? Fired.IsEmpty() : (Fired.Num() == 1 && Fired[0].EventRow == Expected);
		});
		TestTrue(TEXT("A row on cooldown leaves the share of the row before it where it was"), bOwnShareOnly);
		TestTrue(TEXT("The row with the cooldown fired once for every civilization"), !bCoolingFor.Contains(false));
	}

	// The cooldown is per row and civilization: once a SettledSystem row fired for one system, every unit of that row
	// is out for that civilization, not only one of them.
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
		const int32 CivCount = State.Civs.Num();
		if (TestEqual(TEXT("The test civilization owns three systems again"), Civ.OwnedSystems.Num(), 3))
		{
			UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
			Table->AddRow(RowA, MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::SettledSystem, 1.f, 100000));
			int32 FiresForThree = 0;
			int32 CivTurn = 0;
			Play(Table, 0.2f, 60, [&](float Roll, const TArray<FCosmicEventNotice>& Fired)
			{
				if (CivTurn++ % CivCount == 0)
				{
					FiresForThree += Fired.Num();
				}
			});
			TestEqual(TEXT("A SettledSystem row with a cooldown fires once for a civilization, whatever the number of its systems"), FiresForThree, 1);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicEventsConditionGateTest, "Cosmic_Ascension.Events.ConditionGate", CosmicTests::Flags)
bool FCosmicEventsConditionGateTest::RunTest(const FString& Parameters)
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

	// Every civilization of a fresh galaxy is at tier 1: one condition that holds for all of them and one for none.
	const FCosmicCondition Holds = Cond(EConditionType::CivTierAtLeast, nullptr, NAME_None, 1.f);
	const FCosmicCondition Fails = Cond(EConditionType::CivTierAtLeast, nullptr, NAME_None, 5.f);

	// CheckAll: AND in array order, true for no condition.
	{
		Sim->NewGalaxy(42, 5, 3);
		FCosmicEvalView View(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
		View.CivIndex = 0;
		TestTrue(TEXT("CheckAll of no condition is true"), UCosmicEventSubsystem::CheckAll(TArray<FCosmicCondition>(), View));
		TestTrue(TEXT("CheckAll of two conditions that hold is true"), UCosmicEventSubsystem::CheckAll(TArray<FCosmicCondition>({ Holds, Holds }), View));
		TestFalse(TEXT("CheckAll is false when the last condition fails"), UCosmicEventSubsystem::CheckAll(TArray<FCosmicCondition>({ Holds, Fails }), View));
		TestFalse(TEXT("CheckAll is false when the first condition fails"), UCosmicEventSubsystem::CheckAll(TArray<FCosmicCondition>({ Fails, Holds }), View));
	}

	TArray<FCosmicEventNotice> Notices;
	FScopedProbe Scoped(*Sim);
	Scoped.Probe->OnEvent = [&Notices](const FCosmicEventNotice& Notice) { Notices.Add(Notice); };

	// One call of the slots with a slot for every civilization (slot period 1), against a fresh galaxy.
	auto SlotsOnce = [&](UDataTable* Table, float Chance, TFunctionRef<void(FGalaxyState&)> Prepare)
	{
		Sim->SetTables(WithEvents(Original, BaseRules, Table, 1, Chance));
		Sim->NewGalaxy(42, 5, 3);
		FGalaxyState& State = Sim->GetMutableState();
		Prepare(State);
		Notices.Reset();
		State.Tick = 3;
		FRandomStream Handed(5);
		return Events->EvaluateAndFire(State, Handed);
	};
	auto Untouched = [](FGalaxyState&) {};

	// A SettledSystem row is judged on each settled system by itself: it fires where its condition holds and nowhere else.
	{
		const FName Hazy(TEXT("evt_test_hazy"));
		FCosmicEventDef Def = MakeRow(ECosmicEventTrigger::Standing, ECosmicEventSubject::SettledSystem, 0.f, 0);
		Def.Conditions.Add(Cond(EConditionType::SystemHasHazard, nullptr, NAME_None, 0.f));
		UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
		Table->AddRow(Hazy, Def);
		FName Marked;
		FName Owner;
		SlotsOnce(Table, 0.5f, [&](FGalaxyState& State)
		{
			// The first civilization takes the free systems; the second of its three carries a hazard.
			FCivilizationState& Civ = State.Civs[0];
			for (FStarSystemState& System : State.Systems)
			{
				if (System.OwnerCivID.IsNone())
				{
					System.OwnerCivID = Civ.CivID;
					Civ.OwnedSystems.Add(System.SystemID);
				}
			}
			Owner = Civ.CivID;
			Marked = Civ.OwnedSystems.Num() == 3 ? Civ.OwnedSystems[1] : FName();
			const int32 MarkedIndex = Sim->GetStateIndex().FindSystem(State, Marked);
			if (State.Systems.IsValidIndex(MarkedIndex))
			{
				State.Systems[MarkedIndex].ActiveHazard = Tag(TEXT("Hazard.Plague"));
			}
		});
		TestTrue(TEXT("A SettledSystem row fires for the one system where its condition holds"),
			!Marked.IsNone() && Notices.Num() == 1 && Notices[0].EventRow == Hazy && Notices[0].CivID == Owner && Notices[0].SystemID == Marked);
	}

	// The conditions are judged again right before a Standing pair fires: the row that fires first in a slot sets a flag,
	// and the row gathered with it, which needs the flag unset, does not fire after it.
	{
		const FName Spend(TEXT("evt_test_a_spend"));
		const FName Once(TEXT("evt_test_b_once"));
		FCosmicEventDef SpendDef = MakeRow(ECosmicEventTrigger::Standing, ECosmicEventSubject::Civilization, 0.f, 0);
		SpendDef.Effects.Add(Eff(EEffectType::SetFlag, TEXT("Flag.Grace.Spent"), NAME_None, 0.f));
		FCosmicEventDef OnceDef = MakeRow(ECosmicEventTrigger::Standing, ECosmicEventSubject::Civilization, 0.f, 0);
		OnceDef.Conditions.Add(Cond(EConditionType::FlagNotSet, TEXT("Flag.Grace.Spent"), NAME_None, 0.f));

		// Alone, the gated row fires for every civilization: nothing ever sets the flag.
		UDataTable* Alone = NewTable(FCosmicEventDef::StaticStruct());
		Alone->AddRow(Once, OnceDef);
		TestEqual(TEXT("A row whose condition holds fires for every civilization"), SlotsOnce(Alone, 0.5f, Untouched), Sim->GetState().Civs.Num());

		UDataTable* Both = NewTable(FCosmicEventDef::StaticStruct());
		Both->AddRow(Once, OnceDef);
		Both->AddRow(Spend, SpendDef);
		SlotsOnce(Both, 0.5f, Untouched);
		int32 SpendFires = 0;
		int32 OnceFires = 0;
		for (const FCosmicEventNotice& Notice : Notices)
		{
			SpendFires += (Notice.EventRow == Spend) ? 1 : 0;
			OnceFires += (Notice.EventRow == Once) ? 1 : 0;
		}
		TestEqual(TEXT("The row that sets the flag fires for every civilization"), SpendFires, Sim->GetState().Civs.Num());
		TestEqual(TEXT("A Standing row whose condition an earlier row of the same slot turned false does not fire"), OnceFires, 0);
	}

	// Lottery rows pass the same gate: at a chance of 1 a row whose condition fails never fires and one whose condition
	// holds fires for every civilization.
	{
		FCosmicEventDef Closed = MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 0);
		Closed.Conditions.Add(Fails);
		UDataTable* ClosedTable = NewTable(FCosmicEventDef::StaticStruct());
		ClosedTable->AddRow(FName(TEXT("evt_test_closed")), Closed);
		TestEqual(TEXT("A Lottery row whose condition fails never fires"), SlotsOnce(ClosedTable, 1.f, Untouched), 0);

		FCosmicEventDef Open = MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 0);
		Open.Conditions.Add(Holds);
		UDataTable* OpenTable = NewTable(FCosmicEventDef::StaticStruct());
		OpenTable->AddRow(FName(TEXT("evt_test_open")), Open);
		TestEqual(TEXT("A Lottery row whose condition holds fires for every civilization at a chance of 1"), SlotsOnce(OpenTable, 1.f, Untouched), Sim->GetState().Civs.Num());
	}

	// A row fired by name: false when a condition fails, with nothing drawn, written, told or announced.
	{
		const FName ClosedRow(TEXT("evt_test_named_closed"));
		const FName OpenRow(TEXT("evt_test_named_open"));
		FCosmicEventDef Def = MakeRow(ECosmicEventTrigger::Named, ECosmicEventSubject::Civilization, 0.f, 0);
		Def.Effects.Add(Eff(EEffectType::ChangeStability, nullptr, NAME_None, -0.1f));
		Def.LogTemplates = ThreeLines(TEXT("gate"));
		UDataTable* Table = NewTable(FCosmicEventDef::StaticStruct());
		Def.Conditions = { Fails };
		Table->AddRow(ClosedRow, Def);
		Def.Conditions = { Holds, Holds };
		Table->AddRow(OpenRow, Def);
		Sim->SetTables(WithEvents(Original, BaseRules, Table, 1, 0.5f));
		Sim->NewGalaxy(42, 5, 3);
		Notices.Reset();
		const FGalaxyState& State = Sim->GetState();

		const uint32 Full = Sim->ComputeStateHash();
		const uint32 World = Sim->ComputeWorldHash();
		const int32 SimSeed = Sim->GetRng().GetCurrentSeed();
		const int32 TellSeed = State.TellRngState;
		const int32 Lines = State.Log.Num();
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = 1;
		TestFalse(TEXT("FireRow is false when a condition of the row fails"), Events->FireRow(ClosedRow, Ctx));
		TestTrue(TEXT("A refused row draws nothing, writes nothing, tells nothing and announces nothing"),
			Sim->ComputeStateHash() == Full && Sim->ComputeWorldHash() == World && Sim->GetRng().GetCurrentSeed() == SimSeed
			&& State.TellRngState == TellSeed && State.Log.Num() == Lines && Notices.IsEmpty());
		TestTrue(TEXT("The same row with conditions that hold fires"),
			Events->FireRow(OpenRow, Ctx) && Notices.Num() == 1 && State.Log.Num() == Lines + 1 && Sim->ComputeWorldHash() != World);
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

	// The error lines of the calls issued from inside a handler: FireRow and EvaluateAndFire once from each of the two
	// delegates, MakeEvalContext once from OnEventFired. Then the two calls over a state that is not the live one.
	AddExpectedError(TEXT("FireRow called from a handler of a Core Logic delegate"), EAutomationExpectedErrorFlags::Contains, 2);
	AddExpectedError(TEXT("EvaluateAndFire called from a handler of a Core Logic delegate"), EAutomationExpectedErrorFlags::Contains, 2);
	AddExpectedError(TEXT("MakeEvalContext called from a handler of a Core Logic delegate"), EAutomationExpectedErrorFlags::Contains, 1);
	AddExpectedError(TEXT("StepOnce called from a handler of a Core Logic delegate"), EAutomationExpectedErrorFlags::Contains, 1);
	AddExpectedError(TEXT("RunHeadless called from a handler of a Core Logic delegate"), EAutomationExpectedErrorFlags::Contains, 1);
	AddExpectedError(TEXT("EvaluateAndFire: the state handed in is not the live state"), EAutomationExpectedErrorFlags::Contains, 1);
	AddExpectedError(TEXT("FireRow: the context is not over the live state"), EAutomationExpectedErrorFlags::Contains, 1);

	// A row with a line and four effects: the first is always turned away in a fired row (MultiplyIncome belongs to the
	// per-tick sources), the other three are of different kinds.
	const FName RowName(TEXT("evt_test_atomic"));
	const FName SlotRow(TEXT("evt_test_slot"));
	FCosmicEventDef Def = MakeRow(ECosmicEventTrigger::Named, ECosmicEventSubject::Civilization, 0.f, 0);
	Def.Effects.Add(Eff(EEffectType::MultiplyIncome, nullptr, FName(TEXT("Energy")), 2.f));
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
			// A handler that breaks its contract: a fire by name and the slots are refused and nothing changes, the sim
			// stream included. The context is built by hand: MakeEvalContext would report the same mistake once more.
			// Tried once: a call that is not refused must fail this test, not run away.
			bTryNestedFire = false;
			const uint32 Before = Sim->ComputeStateHash();
			const int32 SeedBefore = Sim->GetRng().GetCurrentSeed();
			FCosmicEvalContext Ctx(Sim->GetMutableState(), Sim->GetRng(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
			Ctx.CivIndex = 1;
			bNestedFireRefused &= !Events->FireRow(RowName, Ctx);
			bNestedFireRefused &= Events->EvaluateAndFire(Sim->GetMutableState(), Sim->GetRng()) == 0;
			// A tick asked for from a handler is refused as well: the tick number and the hash stay.
			Sim->StepOnce();
			bNestedFireRefused &= Sim->ComputeStateHash() == Before && Sim->GetRng().GetCurrentSeed() == SeedBefore;
		}
	};
	TArray<uint32> HashAtNotice;
	bool bNestedFromNoticeRefused = true;
	bool bTryNestedFromNotice = false;
	Scoped.Probe->OnEvent = [&](const FCosmicEventNotice& Notice)
	{
		Notices.Add(Notice);
		LinesAtNotice.Add(HashAtLine.Num());
		HashAtNotice.Add(Sim->ComputeWorldHash());
		if (bTryNestedFromNotice)
		{
			// The same broken contract from the other delegate, tried once: a fire by name, the slots, and a context
			// asked of the simulation. All three are refused and nothing changes, the sim stream included.
			bTryNestedFromNotice = false;
			const uint32 Before = Sim->ComputeStateHash();
			const int32 SeedBefore = Sim->GetRng().GetCurrentSeed();
			FCosmicEvalContext Ctx(Sim->GetMutableState(), Sim->GetRng(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
			Ctx.CivIndex = 1;
			bNestedFromNoticeRefused &= !Events->FireRow(RowName, Ctx);
			bNestedFromNoticeRefused &= Events->EvaluateAndFire(Sim->GetMutableState(), Sim->GetRng()) == 0;
			(void)Sim->MakeEvalContext();
			Sim->RunHeadless(3);
			bNestedFromNoticeRefused &= Sim->ComputeStateHash() == Before && Sim->GetRng().GetCurrentSeed() == SeedBefore;
		}
	};

	// Fired by name.
	{
		const float StabilityBefore = State.Civs[1].Stability;
		const float MatterBefore = State.Civs[1].Stockpile.Matter;
		// An income above zero, so that a doubling would show (before the first tick it is 0). The caller's context even
		// claims to be a per-tick source: a fire runs on a context of its own and does not take that over.
		Sim->GetMutableState().Civs[1].IncomePerTick.Energy = 4.f;
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = 1;
		Ctx.bPerTickSource = true;
		TestTrue(TEXT("The row fires"), Events->FireRow(RowName, Ctx));
		Sim->SyncRngState();
		TestEqual(TEXT("The effect that is turned away changes nothing"), State.Civs[1].IncomePerTick.Energy, 4.f);
		TestTrue(TEXT("Every effect after the one that was turned away was applied, the last one too"),
			State.Civs[1].Stability < StabilityBefore && State.Civs[1].Notables.Num() == 1 && FMath::IsNearlyEqual(State.Civs[1].Stockpile.Matter, MatterBefore + 5.f));
		TestTrue(TEXT("The line left once, after the last effect: the world at the line is the world after the row"),
			HashAtLine.Num() == 1 && HashAtLine[0] == Sim->ComputeWorldHash());
		TestTrue(TEXT("OnEventFired came once, after the line, and saw the world after the row"),
			Notices.Num() == 1 && LinesAtNotice[0] == 1 && HashAtNotice[0] == Sim->ComputeWorldHash());
		if (Notices.Num() == 1)
		{
			TestTrue(TEXT("The notice carries the row, the subject and the tick"),
				Notices[0].EventRow == RowName && Notices[0].CivID == CivID && Notices[0].SystemID.IsNone()
				&& Notices[0].Tick == State.Tick && Notices[0].Severity == 1 && Notices[0].Category == Tag(TEXT("Event.Civ.Economy")));
		}
		TestTrue(TEXT("The entry carries the row, its log category and the people"),
			State.Log.Last().SourceRow == RowName && State.Log.Last().Category == Tag(TEXT("Log.Civ")) && State.Log.Last().FocusCivID == CivID
			&& State.Log.Last().FocusSystemID.IsNone() && State.Log.Last().Severity == 1);
		// Guarded: a regression that makes no record has to fail this test, not end the process on an array check.
		if (State.Civs[1].Notables.Num() == 1)
		{
			TestEqual(TEXT("The record names the row that made it"), State.Civs[1].Notables[0].SourceRow, RowName);
		}
	}

	// A fire issued from a handler is refused with one error line and changes nothing: from OnLogEntry, then from
	// OnEventFired, where the slots and MakeEvalContext are tried as well.
	{
		bTryNestedFire = true;
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = 2;
		TestTrue(TEXT("The outer row still fires"), Events->FireRow(RowName, Ctx));
		bTryNestedFire = false;
		TestTrue(TEXT("FireRow, EvaluateAndFire and StepOnce issued from a handler of OnLogEntry are refused and change nothing"), bNestedFireRefused);

		bTryNestedFromNotice = true;
		TestTrue(TEXT("The outer row fires again"), Events->FireRow(RowName, Ctx));
		TestTrue(TEXT("The handler of OnEventFired made its attempt"), !bTryNestedFromNotice);
		bTryNestedFromNotice = false;
		TestTrue(TEXT("FireRow, EvaluateAndFire, MakeEvalContext and RunHeadless issued from a handler of OnEventFired are refused and change nothing"), bNestedFromNoticeRefused);
		Sim->SyncRngState();
	}

	// Neither member runs over a state that is not the live one: one error line each, nothing drawn, nothing changed.
	{
		FGalaxyState Detached = Sim->GetState();
		FCosmicStateIndex DetachedIndex;
		DetachedIndex.Rebuild(Detached);
		FRandomStream Handed(11);
		const uint32 LiveBefore = Sim->ComputeStateHash();
		const float StabilityBefore = Detached.Civs[1].Stability;
		const int32 LinesBefore = Sim->GetState().Log.Num();
		TestEqual(TEXT("EvaluateAndFire over a detached state fires nothing"), Events->EvaluateAndFire(Detached, Handed), 0);
		FCosmicEvalContext Ctx(Detached, Handed, DetachedIndex, Sim->GetTables(), Sim->GetRules());
		Ctx.CivIndex = 1;
		TestFalse(TEXT("FireRow over a detached state is false"), Events->FireRow(RowName, Ctx));
		TestTrue(TEXT("The refused calls drew nothing and changed neither state"),
			Handed.GetCurrentSeed() == FRandomStream(11).GetCurrentSeed() && Sim->ComputeStateHash() == LiveBefore
			&& Detached.Civs[1].Stability == StabilityBefore && Sim->GetState().Log.Num() == LinesBefore);
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
	FCosmicDataTables Nameless = Original;
	Nameless.Civs = CivsWithoutNames(Original);
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
	TestTrue(TEXT("An empty name bank: refused, one draw"), Try(Nothing, Nameless, BaseRules, 1, NameRebel, bMade) && !bMade);
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

	// Guard 3: the open record of the role is closed and a new one is made. Closed means LastTick above zero, also when
	// the seat changes hands before the first tick: 0 is the mark of an open record.
	for (const int64 Tick : { 77, 0 })
	{
		FGalaxyState Detached = Fresh;
		Detached.Tick = Tick;
		FCosmicStateIndex Index;
		Index.Rebuild(Detached);
		FRandomStream Handed(55);
		FCosmicEvalContext Ctx(Detached, Handed, Index, Named, SmallRoster);
		Ctx.CivIndex = 1;
		Ctx.SourceRow = FName(TEXT("evt_test"));
		UCosmicEventSubsystem::Apply(NameRebel, Ctx);
		UCosmicEventSubsystem::Apply(NameRebel, Ctx);
		const TArray<FNotableRecord>& Notables = Detached.Civs[1].Notables;
		if (Tick > 0)
		{
			TestTrue(TEXT("A second rebel closes the first: one closed and one open record"),
				Notables.Num() == 2 && Notables[0].LastTick == 77 && Notables[1].LastTick == 0 && Notables[1].FirstTick == 77 && Notables[1].SourceRow == Ctx.SourceRow);
		}
		else
		{
			TestTrue(TEXT("At tick 0 the record that was closed still reads as closed"),
				Notables.Num() == 2 && Notables[0].LastTick > 0 && Notables[1].LastTick == 0);
		}
	}

	// Guard 4 and the name, on rosters written out by hand so that every record differs in what the rule reads.
	{
		auto Rec = [](const TCHAR* Role, int32 NameIndex, int64 FirstTick, int64 LastTick)
		{
			FNotableRecord Record;
			Record.Role = Tag(Role);
			Record.NameIndex = NameIndex;
			Record.FirstTick = FirstTick;
			Record.LastTick = LastTick;
			return Record;
		};
		// The roster of civilization 1 after one application at tick 90, with the given cap and stream.
		auto RosterAfter = [&](const TArray<FNotableRecord>& Roster, int32 Cap, const FCosmicEffect& Effect, int32 StreamSeed, bool& bOutMade)
		{
			FGalaxyState Detached = Fresh;
			Detached.Tick = 90;
			Detached.Civs[1].Notables = Roster;
			FCosmicStateIndex Index;
			Index.Rebuild(Detached);
			FRandomStream Handed(StreamSeed);
			FCosmicRulesDef Rules = BaseRules;
			Rules.MaxNotablesPerCiv = Cap;
			FCosmicEvalContext Ctx(Detached, Handed, Index, Named, Rules);
			Ctx.CivIndex = 1;
			Ctx.SourceRow = FName(TEXT("evt_test"));
			bOutMade = UCosmicEventSubsystem::Apply(Effect, Ctx);
			return Detached.Civs[1].Notables;
		};
		auto HasFirstTick = [](const TArray<FNotableRecord>& Roster, int64 FirstTick)
		{
			return Roster.ContainsByPredicate([FirstTick](const FNotableRecord& Record) { return Record.FirstTick == FirstTick; });
		};

		// Two closed records and an open one that is older than both: the record closed longest ago leaves, not the one
		// closed last and not the open one.
		{
			const TArray<FNotableRecord> After = RosterAfter(
				{ Rec(TEXT("Notable.Loyalist"), 0, 10, 50), Rec(TEXT("Notable.Loyalist"), 1, 20, 40), Rec(TEXT("Notable.Hearer"), 2, 5, 0) }, 3, NameRebel, 55, bMade);
			TestTrue(TEXT("At the cap the record closed longest ago leaves, before an older open one"),
				bMade && After.Num() == 3 && !HasFirstTick(After, 20) && HasFirstTick(After, 10) && HasFirstTick(After, 5) && HasFirstTick(After, 90));
		}
		// Closed in the same tick: the older record leaves, although it stands later in the array.
		{
			const TArray<FNotableRecord> After = RosterAfter(
				{ Rec(TEXT("Notable.Loyalist"), 0, 75, 80), Rec(TEXT("Notable.Loyalist"), 1, 70, 80), Rec(TEXT("Notable.Hearer"), 2, 60, 0) }, 3, NameRebel, 55, bMade);
			TestTrue(TEXT("Of two records closed in the same tick the older one leaves"),
				bMade && After.Num() == 3 && !HasFirstTick(After, 70) && HasFirstTick(After, 75) && HasFirstTick(After, 60));
		}
		// No closed record: the oldest open record that is neither ruler nor regent leaves.
		{
			const TArray<FNotableRecord> After = RosterAfter(
				{ Rec(TEXT("Notable.Ruler"), 0, 1, 0), Rec(TEXT("Notable.Hearer"), 1, 30, 0), Rec(TEXT("Notable.Ascendant"), 2, 20, 0) }, 3, NameRebel, 55, bMade);
			TestTrue(TEXT("With no closed record the oldest open record that is neither ruler nor regent leaves"),
				bMade && After.Num() == 3 && !HasFirstTick(After, 20) && HasFirstTick(After, 1) && HasFirstTick(After, 30) && HasFirstTick(After, 90));
		}
		// The name: with two of the three bank indices taken the new record gets the third, whatever number was drawn.
		{
			bool bFreeName = true;
			for (int32 StreamSeed = 1; StreamSeed <= 24; ++StreamSeed)
			{
				const TArray<FNotableRecord> After = RosterAfter({ Rec(TEXT("Notable.Hearer"), 0, 30, 0), Rec(TEXT("Notable.Ascendant"), 2, 20, 0) }, 8, NameRebel, StreamSeed, bMade);
				bFreeName &= bMade && After.Num() == 3 && After.Last().NameIndex == 1;
			}
			TestTrue(TEXT("The new record carries the one name nobody of the roster carries"), bFreeName);
			const TArray<FNotableRecord> Full = RosterAfter(
				{ Rec(TEXT("Notable.Hearer"), 0, 30, 0), Rec(TEXT("Notable.Ascendant"), 2, 20, 0), Rec(TEXT("Notable.Loyalist"), 1, 10, 0) }, 8, NameRebel, 55, bMade);
			TestTrue(TEXT("When every name is taken the bank is open again"), bMade && Full.Num() == 4 && Full.Last().NameIndex >= 0 && Full.Last().NameIndex <= 2);
		}
	}

	// A whole run: the same sim stream with empty name banks and with filled ones, Install's draw included. Both sides
	// are tables built here: the comparison does not lean on the game's banks being empty.
	auto SeedAfter = [Sim](const FCosmicDataTables& Tables)
	{
		Sim->SetTables(Tables);
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		Sim->RunHeadless(1000);
		return Sim->GetRng().GetCurrentSeed();
	};
	TestEqual(TEXT("Filling the name banks does not move the sim stream"), SeedAfter(Named), SeedAfter(Nameless));

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicEventsSphereWeightsTest, "Cosmic_Ascension.Events.SphereWeights", CosmicTests::Flags)
bool FCosmicEventsSphereWeightsTest::RunTest(const FString& Parameters)
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

	const FName RowEco(TEXT("evt_test_a_eco"));
	const FName RowSoc(TEXT("evt_test_b_soc"));
	const FName RowNames[] = { FName(TEXT("tr_test_first")), FName(TEXT("tr_test_second")) };
	const FGameplayTag Economy = Tag(TEXT("Event.Civ.Economy"));
	const float Chance = 0.1f;
	// The step of the test, not of the game: every case plays under a rules row of its own (StepOfCase below).
	const float Step = 0.1f;

	// Two Lottery rows of weight 1: one of the category the transcendents weigh, one of another.
	UDataTable* Rows = NewTable(FCosmicEventDef::StaticStruct());
	Rows->AddRow(RowEco, MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 0));
	FCosmicEventDef Society = MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, 1.f, 0);
	Society.Category = Tag(TEXT("Event.Civ.Society"));
	Rows->AddRow(RowSoc, Society);

	// The reach of a sphere as section 4.2 words it, written out here on its own: the system in it, or one lane from it.
	auto Reaches = [](const FGalaxyState& State, const TArray<FName>& Sphere, FName SystemID)
	{
		const FStarSystemState* System = State.Systems.FindByPredicate([SystemID](const FStarSystemState& Each) { return Each.SystemID == SystemID; });
		return Sphere.Contains(SystemID) || (System && System->Lanes.ContainsByPredicate([&Sphere](FName Lane) { return Sphere.Contains(Lane); }));
	};

	// A sphere of two systems that splits the peoples three ways: a capital in it (2), a capital one lane from it (1)
	// and a capital out of its reach (0).
	Sim->NewGalaxy(42, 12, 4);
	TArray<FName> Sphere;
	TArray<int32> Kinds;
	{
		const FGalaxyState& State = Sim->GetState();
		for (int32 A = 0; A < State.Systems.Num() && Sphere.IsEmpty(); ++A)
		{
			for (int32 B = A + 1; B < State.Systems.Num() && Sphere.IsEmpty(); ++B)
			{
				const TArray<FName> Candidate = { State.Systems[A].SystemID, State.Systems[B].SystemID };
				TArray<int32> CandidateKinds;
				for (const FCivilizationState& Civ : State.Civs)
				{
					CandidateKinds.Add(Candidate.Contains(Civ.HomeSystemID) ? 2 : Reaches(State, Candidate, Civ.HomeSystemID) ? 1 : 0);
				}
				if (CandidateKinds.Contains(0) && CandidateKinds.Contains(1) && CandidateKinds.Contains(2))
				{
					Sphere = Candidate;
					Kinds = CandidateKinds;
				}
			}
		}
	}
	if (!TestTrue(TEXT("The galaxy has a sphere of two systems with a capital inside, a capital one lane away and a capital out of reach"), Sphere.Num() == 2))
	{
		return false;
	}

	// One transcendent of a case: the category it weighs, by how much, and whether it is sealed. All hold the same sphere.
	struct FWeigher
	{
		FGameplayTag Category;
		float Modifier;
		bool bSealed;
	};

	// The row a number picks when the Economy row has this weight factor: the arithmetic of the slot, written out again.
	auto RowFor = [&](float Roll, float Factor)
	{
		const float Eco = 1.f * Factor * Chance;
		const float Soc = 1.f * 1.f * Chance;
		const float Total = Eco + Soc;
		const float Pick = Total > 1.f ? Roll * Total : Roll;
		return Pick < Eco ? RowEco : (Pick < Eco + Soc) ? RowSoc : FName();
	};

	// Plays 80 slots for every civilization with the transcendents of a case in place. bMatch says that every fire was
	// the row the test picks from Factors (one factor per civilization); Rolls keeps the numbers, civilization by
	// civilization and round by round.
	struct FPlayed
	{
		bool bMatch = true;
		TArray<float> Rolls;
	};
	FScopedProbe Scoped(*Sim);
	// The step of the rules a case plays under: the step of the game unless a case sets another.
	float StepOfCase = Step;
	auto Play = [&](const TArray<FWeigher>& Weighers, const TArray<float>& Factors)
	{
		UDataTable* Transcendents = NewTable(FTranscendentDef::StaticStruct());
		for (int32 WeigherIndex = 0; WeigherIndex < Weighers.Num(); ++WeigherIndex)
		{
			FTranscendentDef Def;
			Def.Doctrine = Tag(TEXT("Doctrine.Zeal"));
			Def.Allegiance = Tag(TEXT("Realm.Abyssal"));
			Def.EventWeightModifiers.Add(Weighers[WeigherIndex].Category, Weighers[WeigherIndex].Modifier);
			Transcendents->AddRow(RowNames[WeigherIndex], Def);
		}
		FCosmicRulesDef RulesOfCase = BaseRules;
		RulesOfCase.SphereEventWeightStep = StepOfCase;
		FCosmicDataTables Tables = WithEvents(Original, RulesOfCase, Rows, 1, Chance);
		Tables.Transcendents = Transcendents;
		Sim->SetTables(Tables);
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		// The case places its own transcendents: whatever roster the tables gave is taken out.
		State.Transcendents.Reset();
		for (int32 WeigherIndex = 0; WeigherIndex < Weighers.Num(); ++WeigherIndex)
		{
			FTranscendentState& Transcendent = State.Transcendents.AddDefaulted_GetRef();
			Transcendent.TranscendentID = RowNames[WeigherIndex];
			Transcendent.InfluencedSystems = Sphere;
			Transcendent.bSealed = Weighers[WeigherIndex].bSealed;
		}

		FPlayed Played;
		FRandomStream Handed(777);
		FRandomStream Probe(777);
		for (int32 Round = 0; Round < 80; ++Round)
		{
			TMap<FName, TArray<FName>> FiredFor;
			Scoped.Probe->OnEvent = [&FiredFor](const FCosmicEventNotice& Notice) { FiredFor.FindOrAdd(Notice.CivID).Add(Notice.EventRow); };
			++State.Tick;
			Events->EvaluateAndFire(State, Handed);
			for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
			{
				const float Roll = Probe.GetFraction();
				const FName Expected = RowFor(Roll, Factors[CivIndex]);
				const TArray<FName> Fired = FiredFor.FindRef(State.Civs[CivIndex].CivID);
				Played.bMatch &= Expected.IsNone() ? Fired.IsEmpty() : (Fired.Num() == 1 && Fired[0] == Expected);
				Played.Rolls.Add(Roll);
			}
		}
		Scoped.Probe->OnEvent = nullptr;
		return Played;
	};
	// In how many of the played slots another set of factors would have picked another row: what a case is able to tell.
	// Every assertion below asks for bMatch and for a count above zero, so it fails when the code weighs otherwise and
	// never passes on numbers that could not have told the difference.
	auto Differing = [&](const FPlayed& Played, const TArray<float>& Factors, const TArray<float>& Against)
	{
		int32 Count = 0;
		for (int32 RollIndex = 0; RollIndex < Played.Rolls.Num(); ++RollIndex)
		{
			const int32 CivIndex = RollIndex % Factors.Num();
			Count += (RowFor(Played.Rolls[RollIndex], Factors[CivIndex]) != RowFor(Played.Rolls[RollIndex], Against[CivIndex])) ? 1 : 0;
		}
		return Count;
	};
	// One factor per civilization from the kind of its capital.
	auto ByKind = [&Kinds](float Inside, float OneLane, float OutOfReach)
	{
		TArray<float> Factors;
		for (const int32 Kind : Kinds)
		{
			Factors.Add(Kind == 2 ? Inside : Kind == 1 ? OneLane : OutOfReach);
		}
		return Factors;
	};

	// The factor of one transcendent with a modifier of 2 and this sphere: min(3, modifier x (1 + step x size)).
	const float Sized = 1.f + Step * 2.f;
	const float Weighed = FMath::Min(3.f, 2.f * Sized);
	const TArray<float> Plain = ByKind(1.f, 1.f, 1.f);
	const TArray<float> Reached = ByKind(Weighed, Weighed, 1.f);
	if (!TestTrue(TEXT("The step of the test makes a sphere of two weigh more than a sphere of none, below the cap"), Weighed > 2.f && Weighed < 3.f))
	{
		return false;
	}

	// One transcendent that doubles the Economy rows.
	{
		const FPlayed Played = Play({ { Economy, 2.f, false } }, Reached);
		TestTrue(TEXT("The share of a row of the weighed category is min(3, modifier x (1 + step x size of the sphere)) times its own, and the row of another category keeps its share"),
			Played.bMatch && Differing(Played, Reached, Plain) > 0);
		TestTrue(TEXT("A capital one lane from the sphere is weighed like a capital inside it"), Played.bMatch && Differing(Played, Reached, ByKind(Weighed, 1.f, 1.f)) > 0);
		TestTrue(TEXT("A capital out of reach is not weighed"), Played.bMatch && Differing(Played, Reached, ByKind(Weighed, Weighed, Weighed)) > 0);
		TestTrue(TEXT("The size of the sphere is part of the factor"), Played.bMatch && Differing(Played, Reached, ByKind(2.f, 2.f, 1.f)) > 0);
	}

	// The cap: a modifier of 5 gives 3, not 5 x (1 + step x size).
	{
		const TArray<float> Capped = ByKind(3.f, 3.f, 1.f);
		const FPlayed Played = Play({ { Economy, 5.f, false } }, Capped);
		TestTrue(TEXT("The factor of one transcendent stops at 3"), Played.bMatch && Differing(Played, Capped, ByKind(5.f * Sized, 5.f * Sized, 1.f)) > 0);
	}

	// A modifier below 1 lowers the share.
	{
		const float Lowered = FMath::Min(3.f, 0.5f * Sized);
		const TArray<float> Low = ByKind(Lowered, Lowered, 1.f);
		const FPlayed Played = Play({ { Economy, 0.5f, false } }, Low);
		TestTrue(TEXT("A modifier below 1 lowers the share"), Played.bMatch && Differing(Played, Low, Plain) > 0);
	}

	// A step below zero is a mistake in the data: the factor stops at 0 and the row of the category does not fire, but
	// the other row keeps its share.
	{
		StepOfCase = -1.f;
		const TArray<float> Silenced = ByKind(0.f, 0.f, 1.f);
		const FPlayed Played = Play({ { Economy, 2.f, false } }, Silenced);
		StepOfCase = Step;
		TestTrue(TEXT("A factor never goes below 0, whatever the step of the rules"), Played.bMatch && Differing(Played, Silenced, Plain) > 0);
	}

	// A sealed transcendent weighs nothing.
	{
		const FPlayed Played = Play({ { Economy, 2.f, true } }, Plain);
		TestTrue(TEXT("A sealed transcendent leaves every share as it was"), Played.bMatch && Differing(Played, Plain, Reached) > 0);
	}

	// The category has to be the very same tag: a modifier on the parent does not reach its children.
	{
		const FPlayed Played = Play({ { Tag(TEXT("Event.Civ")), 2.f, false } }, Plain);
		TestTrue(TEXT("A modifier on a parent category does not weigh the rows of its children"), Played.bMatch && Differing(Played, Plain, Reached) > 0);
	}

	// Two transcendents multiply, and the cap is on each of them, not on the product.
	{
		const float Twice = (1.f * Weighed) * Weighed;
		const TArray<float> Both = ByKind(Twice, Twice, 1.f);
		const FPlayed Played = Play({ { Economy, 2.f, false }, { Economy, 2.f, false } }, Both);
		TestTrue(TEXT("The factors of two transcendents multiply"), Played.bMatch && Differing(Played, Both, Reached) > 0);
		TestTrue(TEXT("The cap of 3 is on each factor, not on the product"), Played.bMatch && Twice > 3.f && Differing(Played, Both, ByKind(3.f, 3.f, 1.f)) > 0);
	}

	// A modifier that is not a number reads as 0, like one below zero: the row of the category does not fire.
	{
		const TArray<float> Silenced = ByKind(0.f, 0.f, 1.f);
		const FPlayed Played = Play({ { Economy, std::numeric_limits<float>::quiet_NaN(), false } }, Silenced);
		TestTrue(TEXT("A modifier that is not a number reads as 0"), Played.bMatch && Differing(Played, Silenced, ByKind(3.f, 3.f, 1.f)) > 0);
	}

	// A SettledSystem row is weighed by the system of each pair, not by the capital of its people. The galaxy is wired by
	// the test (CosmicTests::WireChain: a chain 0..9 with a branch 4-10-11): civilization 1 holds systems 2 and 7, and
	// the sphere holds one of the two.
	{
		const FName RowSys(TEXT("evt_test_c_sys"));
		FCosmicEventDef PerSystem = MakeRow(ECosmicEventTrigger::Lottery, ECosmicEventSubject::SettledSystem, 1.f, 0);
		// A condition that every unit passes: a row without one hands CheckAll an empty array.
		PerSystem.Conditions.Add(Cond(EConditionType::SystemOwnedBy, nullptr, NAME_None, 0.f));
		UDataTable* SystemRows = NewTable(FCosmicEventDef::StaticStruct());
		SystemRows->AddRow(RowSys, PerSystem);
		FTranscendentDef Def;
		Def.Doctrine = Tag(TEXT("Doctrine.Zeal"));
		Def.Allegiance = Tag(TEXT("Realm.Abyssal"));
		Def.EventWeightModifiers.Add(Economy, 2.f);
		UDataTable* Transcendents = NewTable(FTranscendentDef::StaticStruct());
		Transcendents->AddRow(RowNames[0], Def);
		FCosmicRulesDef RulesOfCase = BaseRules;
		RulesOfCase.SphereEventWeightStep = Step;
		FCosmicDataTables Tables = WithEvents(Original, RulesOfCase, SystemRows, 1, Chance);
		Tables.Transcendents = Transcendents;
		Sim->SetTables(Tables);
		// The factor of a sphere of one system.
		const float WeighedOne = FMath::Min(3.f, 2.f * (1.f + Step * 1.f));

		// Plays 80 slots with the sphere on SphereSystem and the capital of civilization 1 on system 2. True when every
		// fire of that people named the system its roll picks from the two shares. OutDecided counts the slots in which a
		// factor taken from the capital for both pairs would have picked otherwise.
		auto PlayTwoSystems = [&](int32 SphereSystem, int32& OutDecided)
		{
			OutDecided = 0;
			Sim->NewGalaxy(42, 12, 4);
			FGalaxyState& State = Sim->GetMutableState();
			if (State.Civs.Num() != 4 || !CosmicTests::WireChain(State))
			{
				return false;
			}
			State.Transcendents.Reset();
			CosmicTests::ClearOwners(State);
			CosmicTests::Give(State, 0, { 0 });
			CosmicTests::Give(State, 1, { 2, 7 });
			CosmicTests::Give(State, 2, { 11 });
			CosmicTests::Give(State, 3, { 9 });
			State.Civs[1].HomeSystemID = State.Systems[2].SystemID;
			FTranscendentState& Transcendent = State.Transcendents.AddDefaulted_GetRef();
			Transcendent.TranscendentID = RowNames[0];
			Transcendent.InfluencedSystems.Add(State.Systems[SphereSystem].SystemID);
			const FName CivID = State.Civs[1].CivID;
			const FName SystemTwo = State.Systems[2].SystemID;
			const FName SystemSeven = State.Systems[7].SystemID;
			// The pairs of the people come in OwnedSystems order: system 2, then system 7.
			const float ShareTwo = 1.f * (SphereSystem == 2 ? WeighedOne : 1.f) * Chance;
			const float ShareSeven = 1.f * (SphereSystem == 7 ? WeighedOne : 1.f) * Chance;
			// What both pairs would get if the factor came from the capital, system 2.
			const float ShareByCapital = 1.f * (SphereSystem == 2 ? WeighedOne : 1.f) * Chance;
			bool bMatch = true;
			FRandomStream Handed(777);
			FRandomStream Probe(777);
			for (int32 Round = 0; Round < 80; ++Round)
			{
				TArray<FName> FiredSystems;
				Scoped.Probe->OnEvent = [&FiredSystems, CivID](const FCosmicEventNotice& Notice)
				{
					if (Notice.CivID == CivID)
					{
						FiredSystems.Add(Notice.SystemID);
					}
				};
				++State.Tick;
				Events->EvaluateAndFire(State, Handed);
				for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
				{
					const float Roll = Probe.GetFraction();
					if (CivIndex != 1)
					{
						continue;
					}
					const FName Expected = Roll < ShareTwo ? SystemTwo : (Roll < ShareTwo + ShareSeven) ? SystemSeven : FName();
					bMatch &= Expected.IsNone() ? FiredSystems.IsEmpty() : (FiredSystems.Num() == 1 && FiredSystems[0] == Expected);
					const FName ByCapital = Roll < ShareByCapital ? SystemTwo : (Roll < ShareByCapital + ShareByCapital) ? SystemSeven : FName();
					OutDecided += (ByCapital != Expected) ? 1 : 0;
				}
			}
			Scoped.Probe->OnEvent = nullptr;
			return bMatch;
		};
		int32 DecidedByColony = 0;
		int32 DecidedByCapital = 0;
		const bool bColonyInSphere = PlayTwoSystems(7, DecidedByColony);
		const bool bCapitalInSphere = PlayTwoSystems(2, DecidedByCapital);
		TestTrue(TEXT("A SettledSystem row is weighed on the system inside the sphere, though the capital of its people lies out of reach"),
			bColonyInSphere && DecidedByColony > 0);
		TestTrue(TEXT("A SettledSystem row is not weighed on a system out of reach, though the capital of its people lies in the sphere"),
			bCapitalInSphere && DecidedByCapital > 0);
	}

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

	// The three effects that wait for the crisis handling work each leave one error line (not from a per-tick source,
	// which turns them away before that).
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
	// The shield covers the ticks of a life below BirthProtectionTicks: the last of them is shielded, the next is not.
	{
		const int64 Protection = Sim->GetRules().BirthProtectionTicks;
		FBench Last(Fresh, *Sim, Fresh.Calling.BornTick + Protection - 1);
		Last.Ctx.CivIndex = 0;
		TestFalse(TEXT("The last tick of the birth protection is shielded"), UCosmicEventSubsystem::Apply(Kill, Last.Ctx));
		FBench Next(Fresh, *Sim, Fresh.Calling.BornTick + Protection);
		Next.Ctx.CivIndex = 0;
		TestTrue(TEXT("The tick after the birth protection is not shielded"), UCosmicEventSubsystem::Apply(Kill, Next.Ctx) && Next.State.Calling.PendingEffect == KillTag);
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

	// A factor below zero, not a number or not finite reads as 0, and a row with a Key gives the same float as a row
	// without one: both are one multiplication.
	{
		auto EnergyAfter = [&](const FCosmicEffect& Scale)
		{
			FBench Bench(Fresh, *Sim, PastProtection);
			Bench.Ctx.CivIndex = 0;
			Bench.Ctx.bPerTickSource = true;
			Bench.State.Civs[0].IncomePerTick.Energy = 1.11f;
			UCosmicEventSubsystem::Apply(Scale, Bench.Ctx);
			return Bench.State.Civs[0].IncomePerTick.Energy;
		};
		const float Keyed = EnergyAfter(Eff(EEffectType::MultiplyIncome, nullptr, FName(TEXT("Energy")), 0.9f));
		const float Unkeyed = EnergyAfter(Eff(EEffectType::MultiplyIncome, nullptr, NAME_None, 0.9f));
		TestTrue(TEXT("MultiplyIncome with a Key and without one give the same float: one multiplication, income x Value"),
			Keyed == Unkeyed && Keyed == 1.11f * 0.9f);
		TestTrue(TEXT("A MultiplyIncome factor below zero, not a number or not finite reads as 0"),
			EnergyAfter(Eff(EEffectType::MultiplyIncome, nullptr, FName(TEXT("Energy")), -0.2f)) == 0.f
			&& EnergyAfter(Eff(EEffectType::MultiplyIncome, nullptr, NAME_None, -0.2f)) == 0.f
			&& EnergyAfter(Eff(EEffectType::MultiplyIncome, nullptr, FName(TEXT("Energy")), std::numeric_limits<float>::quiet_NaN())) == 0.f
			&& EnergyAfter(Eff(EEffectType::MultiplyIncome, nullptr, FName(TEXT("Energy")), std::numeric_limits<float>::infinity())) == 0.f);
	}

	// A per-tick source applies the seven effects that change an amount and turns every other away before anything is
	// read or drawn (section 7.5). One line per effect type: the count comes from reflection, so a type appended to the
	// enum fails here until it has a line and a decision.
	{
		struct FPerTickCase
		{
			FCosmicEffect Effect;
			bool bApplies;
		};
		const FPerTickCase PerTick[] = {
			{ Eff(EEffectType::AddResource, nullptr, FName(TEXT("Matter")), 1.f), true },
			{ Eff(EEffectType::MultiplyIncome, nullptr, FName(TEXT("Energy")), 0.5f), true },
			{ Eff(EEffectType::ShiftEthics, nullptr, FName(TEXT("Openness")), 0.1f), true },
			{ Eff(EEffectType::AddTrait, TEXT("Trait.Faith.Seeking"), NAME_None, 0.f), false },
			{ Eff(EEffectType::RemoveTrait, TEXT("Trait.Faith.Seeking"), NAME_None, 0.f), false },
			{ Eff(EEffectType::ChangeStability, nullptr, NAME_None, -0.1f), true },
			{ Eff(EEffectType::ChangeRelation, nullptr, NAME_None, -0.1f), true },
			{ Eff(EEffectType::SpawnHazard, TEXT("Hazard.Plague"), NAME_None, 0.f), false },
			{ Eff(EEffectType::ResolveHazard, nullptr, NAME_None, 0.f), false },
			{ Eff(EEffectType::TransferSystem, nullptr, NAME_None, 0.f), false },
			{ Eff(EEffectType::ChangeReverence, nullptr, NAME_None, 0.1f), true },
			{ Eff(EEffectType::GrantPower, TEXT("Way.Received"), NAME_None, 0.f), false },
			{ Eff(EEffectType::PlayerStatDelta, nullptr, FName(TEXT("Body")), 1.f), false },
			{ Eff(EEffectType::SetFlag, TEXT("Flag.Grace.Spent"), NAME_None, 0.f), false },
			{ Kill, false },
			{ Depose, false },
			{ Eff(EEffectType::ChangePopulation, nullptr, NAME_None, -0.1f), true },
			{ Eff(EEffectType::NameNotable, TEXT("Notable.Rebel"), NAME_None, 0.f), false },
		};
		TSet<EEffectType> Types;
		bool bAsListed = true;
		bool bRefusedUntouched = true;
		for (const FPerTickCase& Case : PerTick)
		{
			Types.Add(Case.Effect.Type);
			// The vessel rules civilization 0 and stands in sol, past the birth protection: both role effects, the vessel
			// effects and the notable would all go through if the source were an event row.
			FBench Bench(Fresh, *Sim, PastProtection);
			Bench.Ctx.CivIndex = 0;
			Bench.Ctx.OtherID = Fresh.Civs[1].CivID;
			Bench.Ctx.bPerTickSource = true;
			Bench.State.Civs[0].IncomePerTick.Energy = 4.f;
			const int32 SeedBefore = Bench.Stream.GetCurrentSeed();
			const bool bApplied = UCosmicEventSubsystem::Apply(Case.Effect, Bench.Ctx);
			bAsListed &= bApplied == Case.bApplies;
			if (!Case.bApplies)
			{
				const FCivilizationState& After = Bench.State.Civs[0];
				bRefusedUntouched &= Bench.Stream.GetCurrentSeed() == SeedBefore && !Bench.State.Calling.PendingEffect.IsValid()
					&& Bench.State.Player.Body == Fresh.Player.Body && Bench.State.Calling.Powers == Fresh.Calling.Powers
					&& Bench.State.Flags == Fresh.Flags && After.Traits == Fresh.Civs[0].Traits && After.Notables.Num() == Fresh.Civs[0].Notables.Num();
			}
		}
		TestTrue(TEXT("A per-tick source applies the seven effects that change an amount and no other"), bAsListed);
		TestTrue(TEXT("An effect a per-tick source may not apply draws nothing, records no role effect and leaves the vessel, the flags, the traits and the notables as they were"),
			bRefusedUntouched);
		TestEqual(TEXT("Every effect type has its line in the per-tick table"), Types.Num(), StaticEnum<EEffectType>()->NumEnums() - 1);
	}

	// The plain world effects, and the three that wait for the crisis handling work.
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
		TestFalse(TEXT("SpawnHazard, ResolveHazard and TransferSystem are turned away until the crisis handling work"), bHazardEffects);
		TestTrue(TEXT("The turned-away effects left the system alone"),
			!Bench.State.Systems[1].ActiveHazard.IsValid() && Bench.State.Systems[1].OwnerCivID == Fresh.Systems[1].OwnerCivID);

		// The other party of a relation has to be a living people or a transcendent of the run: a name that is neither
		// (a selector word no code resolves, a people that is gone) is turned away and leaves no entry behind.
		const int32 EntriesBefore = Civ.Relations.Num();
		TestFalse(TEXT("ChangeRelation towards a name that is no party is turned away"),
			UCosmicEventSubsystem::Apply(Eff(EEffectType::ChangeRelation, nullptr, FName(TEXT("Former")), 0.1f), Bench.Ctx));
		Bench.Ctx.OtherID = FName(TEXT("civ_gone"));
		TestFalse(TEXT("ChangeRelation towards a people that is gone is turned away"),
			UCosmicEventSubsystem::Apply(Eff(EEffectType::ChangeRelation, nullptr, NAME_None, 0.1f), Bench.Ctx));
		TestEqual(TEXT("A relation that was turned away leaves no entry"), Civ.Relations.Num(), EntriesBefore);
		const FName Transcendent(TEXT("tr_test"));
		Bench.State.Transcendents.AddDefaulted_GetRef().TranscendentID = Transcendent;
		TestTrue(TEXT("A transcendent of the run is a party: the entry is made on first write"),
			UCosmicEventSubsystem::Apply(Eff(EEffectType::ChangeRelation, nullptr, Transcendent, 0.2f), Bench.Ctx)
			&& Civ.Relations.Num() == EntriesBefore + 1 && Civ.Relations.FindRef(Transcendent) == 0.2f);

		// A celestial transcendent has no instance in the run and is a party all the same (section 4.2, 5.5): the name of
		// its DT_Transcendents row resolves, with tables that hold it. A fallen row resolves only as an instance.
		{
			const FName Celestial(TEXT("tr_test_celestial"));
			FTranscendentDef Heavenly;
			Heavenly.Doctrine = Tag(TEXT("Doctrine.Covenant"));
			Heavenly.Allegiance = Tag(TEXT("Realm.Celestial"));
			UDataTable* Table = NewTable(FTranscendentDef::StaticStruct());
			Table->AddRow(Celestial, Heavenly);
			// A fallen row the roster did not draw is in the table and not in the run: no party, and no key in the save.
			const FName Absent(TEXT("tr_test_absent"));
			FTranscendentDef NotDrawn;
			NotDrawn.Doctrine = Tag(TEXT("Doctrine.Recurrence"));
			NotDrawn.Allegiance = Tag(TEXT("Realm.Abyssal"));
			Table->AddRow(Absent, NotDrawn);
			FCosmicDataTables WithRow = Sim->GetTables();
			WithRow.Transcendents = Table;
			TestFalse(TEXT("ChangeRelation towards a row name the tables do not hold is turned away"),
				UCosmicEventSubsystem::Apply(Eff(EEffectType::ChangeRelation, nullptr, Celestial, 0.3f), Bench.Ctx));
			FCosmicEvalContext WithRowCtx(Bench.State, Bench.Stream, Bench.Index, WithRow, Sim->GetRules());
			WithRowCtx.CivIndex = 1;
			TestTrue(TEXT("The name of a celestial row is a party without an instance in the run"),
				UCosmicEventSubsystem::Apply(Eff(EEffectType::ChangeRelation, nullptr, Celestial, 0.3f), WithRowCtx)
				&& Civ.Relations.FindRef(Celestial) == 0.3f
				&& !Bench.State.Transcendents.ContainsByPredicate([Celestial](const FTranscendentState& Each) { return Each.TranscendentID == Celestial; }));
			const int32 EntriesWithCelestial = Civ.Relations.Num();
			TestTrue(TEXT("ChangeRelation towards a fallen row without an instance in the run is turned away and leaves no entry"),
				!UCosmicEventSubsystem::Apply(Eff(EEffectType::ChangeRelation, nullptr, Absent, 0.3f), WithRowCtx)
				&& Civ.Relations.Num() == EntriesWithCelestial && !Civ.Relations.Contains(Absent));
		}

		Bench.Ctx.CivIndex = INDEX_NONE;
		TestFalse(TEXT("A civ effect without a civilization is turned away"), UCosmicEventSubsystem::Apply(Eff(EEffectType::ChangeStability, nullptr, NAME_None, 0.25f), Bench.Ctx));
	}

	Sim->NewGalaxy(42, 5, 3);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

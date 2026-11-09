// Cosmic Ascension - automation tests of the mechanism of the pressure cards (B4): the effects EnactEdict and DelayHazard,
// the condition CivHasEdict, and the one gate of a card's choice. Design:
// docs/60_DECISIONS/headers/B4_transcendent_voice_and_pressure_cards.md section 2.2, 2.4, 2.5, 6.

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
#include "Events/CosmicPetitions.h"
#include "GameplayTagContainer.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "World/CosmicRulerPolicy.h"

#include <limits>

namespace CosmicPressureCardTests
{
	using CosmicEdicts::ECosmicEdictBlock;
	using CosmicConflict::ECosmicResistBlock;

	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// The rows of this file: the test builds them, so no case leans on the values of the game's rows.
	const FName Plain(TEXT("edict_test_plain"));    // no cost
	const FName Taxing(TEXT("edict_test_taxing"));  // cost 10 Influence
	const FName High(TEXT("edict_test_high"));      // Tier 2
	const FName Bound(TEXT("edict_test_bound"));    // a condition no people meets
	const FName Dear(TEXT("edict_test_dear"));      // a cost no stockpile pays
	const FName Gate(TEXT("petition_test_gate"));   // the card of the gate cases
	const FName Fallen(TEXT("tr_test_fallen"));     // a fourth-rung fallen row: the claimant beside the capital

	FCosmicEffect Effect(EEffectType Type, const TCHAR* TagName, FName Key, float Value)
	{
		FCosmicEffect Out;
		Out.Type = Type;
		Out.Tag = TagName ? Tag(TagName) : FGameplayTag();
		Out.Key = Key;
		Out.Value = Value;
		return Out;
	}

	UDataTable* Edicts()
	{
		UDataTable* Table = CosmicTests::NewTable(FEdictDef::StaticStruct());
		auto Add = [Table](FName Name, TFunctionRef<void(FEdictDef&)> Fill)
		{
			FEdictDef Row;
			Row.DisplayName = FText::AsCultureInvariant(Name.ToString());
			Fill(Row);
			Table->AddRow(Name, Row);
		};
		Add(Plain, [](FEdictDef&) {});
		Add(Taxing, [](FEdictDef& Row) { Row.EnactCost.Influence = 10.f; });
		Add(High, [](FEdictDef& Row) { Row.RequiredTier = EAscensionTier::Tier2_Stellar; });
		Add(Bound, [](FEdictDef& Row)
		{
			FCosmicCondition Book;
			Book.Type = EConditionType::CivHasTrait;
			Book.Tag = Tag(TEXT("Trait.Faith.Book"));
			Row.EnactConditions = { Book };
		});
		Add(Dear, [](FEdictDef& Row) { Row.EnactCost.Influence = 1.0e9f; });
		return Table;
	}

	// A plague (a Draw row of no chance), a claim, and a rebellion (a Capital row the capital rule never places).
	UDataTable* Hazards()
	{
		UDataTable* Table = CosmicTests::NewTable(FHazardDef::StaticStruct());
		auto Add = [Table](const TCHAR* Name, const TCHAR* TagName, ECosmicHazardSpawn Rule)
		{
			FHazardDef Row;
			Row.HazardTag = Tag(TagName);
			Row.DisplayName = FText::AsCultureInvariant(Name);
			Row.CountdownTicks = 300;
			Row.SpawnRule = Rule;
			Row.SpawnChancePerTick = 0.f;
			FCosmicCondition Never;
			Never.Type = EConditionType::GalaxyTickAtLeast;
			Never.Value = 1.0e9f;
			Row.SpawnConditions = { Never };
			Table->AddRow(FName(Name), Row);
		};
		Add(TEXT("crisis_test_sick"), TEXT("Hazard.Plague"), ECosmicHazardSpawn::Draw);
		Add(TEXT("crisis_test_claim"), TEXT("Hazard.Claim"), ECosmicHazardSpawn::ClaimRule);
		Add(TEXT("crisis_test_unrest"), TEXT("Hazard.Rebellion"), ECosmicHazardSpawn::Capital);
		return Table;
	}

	// The card of the gate cases, on a settled system: an edict without a gate's refusal of its own, an edict of the second
	// rung, the fleet's time on the plague, on the rebellion, and a watch with no effect.
	UDataTable* Cards()
	{
		UDataTable* Table = CosmicTests::NewTable(FEncounterDef::StaticStruct());
		FEncounterDef Card;
		Card.Trigger = ECosmicEventTrigger::Named;
		Card.Subject = ECosmicEventSubject::SettledSystem;
		Card.BaseWeight = 0.f;
		Card.Title = FText::AsCultureInvariant(TEXT("a test card"));
		Card.Body = FText::AsCultureInvariant(TEXT("a test card."));
		auto Choice = [](const TArray<FCosmicEffect>& OnSuccess)
		{
			FEncounterChoice Out;
			Out.Label = FText::AsCultureInvariant(TEXT("a choice"));
			Out.OnSuccess = OnSuccess;
			Out.SuccessText = FText::AsCultureInvariant(TEXT("done."));
			return Out;
		};
		Card.Choices = {
			Choice({ Effect(EEffectType::EnactEdict, nullptr, Plain, 0.f) }),
			Choice({ Effect(EEffectType::EnactEdict, nullptr, High, 0.f) }),
			Choice({ Effect(EEffectType::DelayHazard, TEXT("Hazard.Plague"), NAME_None, 50.f) }),
			Choice({ Effect(EEffectType::DelayHazard, TEXT("Hazard.Rebellion"), NAME_None, 50.f) }),
			Choice({}),
		};
		Table->AddRow(Gate, Card);
		return Table;
	}

	// A board of twelve systems and four peoples under quiet rules (no Lottery event, no drain, no first crisis, no
	// projects, no inherited edict, no drift) with this file's edicts, hazards, card and a fallen row; fleets from the second
	// rung and a resistance that keeps half the fleet. The player rules civ 0.
	FGalaxyState* LayBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Original)
	{
		const FCosmicRulesDef* Game = Original.FindRules();
		if (!Game)
		{
			return nullptr;
		}
		FCosmicRulesDef Rules = CosmicTests::WithoutStartingEdict(*Game);
		Rules.EventChancePerSlot = 0.f;
		Rules.SpiritualPressureStabilityDrain = 0.f;
		Rules.FirstCrisisTick = 0;
		Rules.RelationDriftRate = 0.f;
		Rules.FleetPerSettledByTier = { 0.f, 1.f, 1.f, 1.f, 1.f };
		Rules.FleetEffectivenessByTierGap = { 1.f, 0.5f, 0.f };
		Rules.ResistFleetFactor = 0.5f;
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));
		Tables.Edicts = Edicts();
		Tables.Hazards = Hazards();
		Tables.Encounters = Cards();
		Tables.Events = nullptr;
		UDataTable* Transcendents = CosmicTests::NewTable(FTranscendentDef::StaticStruct());
		FTranscendentDef Row;
		Row.Doctrine = Tag(TEXT("Doctrine.Silence"));
		Row.Allegiance = Tag(TEXT("Realm.Abyssal"));
		Row.Tier = EAscensionTier::Tier4_Transcendent;
		Row.MaxInfluencedSystems = 12;
		Transcendents->AddRow(Fallen, Row);
		Sim.SetTables(CosmicTests::WithTranscendents(Tables, Transcendents));
		Sim.NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim.GetMutableState();
		State.Transcendents.Reset();
		return &State;
	}

	int32 CapitalOf(const UGalaxySimulationSubsystem& Sim, int32 CivIndex)
	{
		const FGalaxyState& State = Sim.GetState();
		return Sim.GetStateIndex().FindSystem(State, State.Civs[CivIndex].HomeSystemID);
	}

	// The board of the pressure policies: the game's hazards, projects and edicts (so the claim's relief, the plague's answer,
	// the warning's stabilisation and the vigil are the game's), two test pressure cards, and the chain of
	// CosmicTests::WireChain owned as Sim.RulerDiplomacyPolicy owns it - people 0 holds {0, 1} (capital 0), 1 holds {3, 2, 4},
	// 2 {6, 5, 7}, 3 {9, 8, 10, 11} - every people met; a fourth-rung fallen sphere over system 2, one lane from people 0's
	// system 1 (PressNear) and two from its capital. Quiet rules (no event, no drain, no first crisis, no drift, no sight
	// stage, no inherited edict); fleets from the second rung.
	const FName PressFirst(TEXT("petition_test_press"));
	const FName PressSecond(TEXT("petition_test_press_two"));
	const FName PressVigil(TEXT("edict_vigil"));
	const FName PressOther(TEXT("edict_open_thought"));
	const FName PressAnswer(TEXT("edict_machine_rights"));
	const FName PressRelief(TEXT("project_relief"));
	const FName PressCalm(TEXT("project_stabilize"));
	constexpr int32 PressNear = 1;

	UDataTable* PressureCards()
	{
		UDataTable* Table = CosmicTests::NewTable(FEncounterDef::StaticStruct());
		auto Choice = [](const TArray<FCosmicEffect>& OnSuccess, float Insight, FName Next)
		{
			FEncounterChoice Out;
			Out.Label = FText::AsCultureInvariant(TEXT("a choice"));
			Out.OnSuccess = OnSuccess;
			Out.Cost.Insight = Insight;
			Out.NextEncounter = Next;
			Out.SuccessText = FText::AsCultureInvariant(TEXT("done."));
			return Out;
		};
		FEncounterDef Card;
		Card.Trigger = ECosmicEventTrigger::Named;
		Card.Subject = ECosmicEventSubject::SettledSystem;
		Card.BaseWeight = 0.f;
		Card.Title = FText::AsCultureInvariant(TEXT("a test card"));
		Card.Body = FText::AsCultureInvariant(TEXT("a test card."));
		Card.Choices = { Choice({ Effect(EEffectType::EnactEdict, nullptr, PressVigil, 0.f) }, 0.f, NAME_None), Choice({}, 200.f, PressSecond), Choice({}, 0.f, NAME_None) };
		Table->AddRow(PressFirst, Card);
		Card.Choices = { Choice({ Effect(EEffectType::EnactEdict, nullptr, PressOther, 0.f) }, 0.f, NAME_None),
			Choice({ Effect(EEffectType::DelayHazard, TEXT("Hazard.Claim"), NAME_None, 50.f) }, 0.f, NAME_None), Choice({}, 0.f, NAME_None) };
		Table->AddRow(PressSecond, Card);
		return Table;
	}

	FGalaxyState* LayPressureBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Original, UDataTable* Cards = nullptr)
	{
		const FCosmicRulesDef* Game = Original.FindRules();
		if (!Game)
		{
			return nullptr;
		}
		FCosmicRulesDef Rules = CosmicTests::WithoutStartingEdict(*Game);
		Rules.EventChancePerSlot = 0.f;
		Rules.SpiritualPressureStabilityDrain = 0.f;
		Rules.FirstCrisisTick = 0;
		Rules.RelationDriftRate = 0.f;
		Rules.FleetPerSettledByTier = { 0.f, 1.f, 1.f, 1.f, 1.f };
		Rules.FirstSightTick = 1000000;
		Rules.SightIntervalTicks = 1000000;
		FCosmicDataTables Tables = CosmicTests::WithRules(Original, Rules);
		Tables.Encounters = Cards ? Cards : PressureCards();
		Tables.Events = nullptr;
		UDataTable* Transcendents = CosmicTests::NewTable(FTranscendentDef::StaticStruct());
		FTranscendentDef Row;
		Row.Doctrine = Tag(TEXT("Doctrine.Silence"));
		Row.Allegiance = Tag(TEXT("Realm.Abyssal"));
		Row.Tier = EAscensionTier::Tier4_Transcendent;
		Row.MaxInfluencedSystems = 12;
		Transcendents->AddRow(Fallen, Row);
		Sim.SetTables(CosmicTests::WithTranscendents(Tables, Transcendents));
		Sim.NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim.GetMutableState();
		if (State.Civs.Num() != 4 || !CosmicTests::WireChain(State))
		{
			return nullptr;
		}
		State.Transcendents.Reset();
		CosmicTests::ClearOwners(State);
		for (FStarSystemState& System : State.Systems)
		{
			System.ActiveHazard = FGameplayTag();
			System.HazardTicksRemaining = 0;
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
			System.ResponseMegaproject = NAME_None;
			System.ResponseProgress = 0.f;
			System.CompletedMegaprojects.Reset();
		}
		CosmicTests::Give(State, 0, { 0, 1 });
		CosmicTests::Give(State, 1, { 3, 2, 4 });
		CosmicTests::Give(State, 2, { 6, 5, 7 });
		CosmicTests::Give(State, 3, { 9, 8, 10, 11 });
		const int32 Capitals[] = { 0, 3, 6, 9 };
		for (int32 CivIndex = 0; CivIndex < 4; ++CivIndex)
		{
			FCivilizationState& Civ = State.Civs[CivIndex];
			Civ.HomeSystemID = State.Systems[Capitals[CivIndex]].SystemID;
			Civ.Tier = EAscensionTier::Tier1_Planetary;
			Civ.FleetPower = 0.f;
			Civ.FleetTargetSystemID = NAME_None;
			Civ.FleetTargetSinceTick = 0;
			Civ.Ties.Reset();
			Civ.Relations.Reset();
			Civ.ActiveEdicts.Reset();
			Civ.Stability = 0.75f;
			Civ.Stockpile = FResourcePool();
		}
		CosmicTests::AddInstance(State, Fallen, { State.Systems[2].SystemID });
		CosmicTests::KnowAll(State);
		return &State;
	}

	// One application of Effect with civ CivIndex and system SystemIndex as subject, through a context of the live state.
	bool ApplyOn(UGalaxySimulationSubsystem& Sim, const FCosmicEffect& Effect, int32 CivIndex, int32 SystemIndex, bool bPerTick = false)
	{
		FCosmicEvalContext Ctx = Sim.MakeEvalContext();
		Ctx.CivIndex = CivIndex;
		Ctx.SystemIndex = SystemIndex;
		Ctx.SourceRow = Gate;
		Ctx.bPerTickSource = bPerTick;
		return UCosmicEventSubsystem::Apply(Effect, Ctx);
	}
}

// EnactEdict (header section 2.2): through EnactBlockOf alone - every refusal changes nothing; an enactment pays EnactCost
// once and appends the row; no line, no draw; a per-tick source never applies it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimEnactEdictEffectTest, "Cosmic_Ascension.Sim.EnactEdictEffect", CosmicTests::Flags)
bool FCosmicSimEnactEdictEffectTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPressureCardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* State = LayBoard(*Sim, Original);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	FCivilizationState& Ruled = State->Civs[0];
	Ruled.Stockpile.Influence = 50.f;
	const int32 Capital = CapitalOf(*Sim, 0);
	auto Enact = [&](FName Key, bool bPerTick = false)
	{
		return ApplyOn(*Sim, Effect(EEffectType::EnactEdict, nullptr, Key, 0.f), 0, Capital, bPerTick);
	};
	// A refusal changes nothing: the state, both streams, the log.
	auto Refused = [&](FName Key, bool bPerTick = false)
	{
		const uint32 Hash = Sim->ComputeStateHash();
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		const int32 Tell = State->TellRngState;
		const int32 Lines = State->Log.Num();
		return !Enact(Key, bPerTick) && Sim->ComputeStateHash() == Hash && Sim->GetRng().GetCurrentSeed() == Seed && State->TellRngState == Tell && State->Log.Num() == Lines;
	};

	TestTrue(TEXT("A Key that is no edict is refused (NoRow) and changes nothing"), Refused(FName(TEXT("edict_test_nobody"))));
	TestTrue(TEXT("An edict of a higher rung is refused (Tier)"), Refused(High));
	TestTrue(TEXT("An edict whose conditions fail is refused (Conditions)"), Refused(Bound));
	TestTrue(TEXT("An edict the stockpile cannot pay is refused (Cost)"), Refused(Dear));
	TestTrue(TEXT("A per-tick source never applies it"), Refused(Taxing, true));
	{
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		const int32 Tell = State->TellRngState;
		const int32 Lines = State->Log.Num();
		TestTrue(TEXT("An edict the gate takes is enacted"), Enact(Taxing));
		TestEqual(TEXT("paying its cost once"), Ruled.Stockpile.Influence, 40.f);
		TestTrue(TEXT("and appending it"), Ruled.ActiveEdicts == TArray<FName>({ Taxing }));
		TestTrue(TEXT("with no line and no draw of either stream"), State->Log.Num() == Lines && State->TellRngState == Tell && Sim->GetRng().GetCurrentSeed() == Seed);
	}
	TestTrue(TEXT("An edict held is refused (Held)"), Refused(Taxing));
	TestTrue(TEXT("With the one slot taken another is refused (Slots)"), Refused(Plain));
	TestEqual(TEXT("(nothing more was paid)"), Ruled.Stockpile.Influence, 40.f);
	return true;
}

// CivHasEdict (header section 2.2): a held row the tables know; never one they do not; the AI peoples after their first AI
// stage (they give up the inherited edict); and Check changes nothing.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimCivHasEdictConditionTest, "Cosmic_Ascension.Sim.CivHasEdictCondition", CosmicTests::Flags)
bool FCosmicSimCivHasEdictConditionTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPressureCardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	if (!TestNotNull(TEXT("The board is laid"), LayBoard(*Sim, Original)))
	{
		return false;
	}
	// The board again with Plain as every people's inherited edict.
	FCosmicRulesDef Inheriting = Sim->GetRules();
	Inheriting.StartingEdict = Plain;
	Sim->SetTables(CosmicTests::WithRules(Sim->GetTables(), Inheriting));
	Sim->NewGalaxy(42, 12, 4);
	FGalaxyState& State = Sim->GetMutableState();
	const FName Gone(TEXT("edict_test_gone"));
	State.Civs[0].ActiveEdicts.Add(Gone);
	auto Holds = [&](int32 CivIndex, FName Key)
	{
		FCosmicCondition Condition;
		Condition.Type = EConditionType::CivHasEdict;
		Condition.Key = Key;
		FCosmicEvalView View(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
		View.CivIndex = CivIndex;
		return UCosmicEventSubsystem::Check(Condition, View);
	};
	const uint32 Hash = Sim->ComputeStateHash();
	TestTrue(TEXT("A held edict the tables know holds"), Holds(0, Plain));
	TestFalse(TEXT("one not held does not"), Holds(0, Taxing));
	TestFalse(TEXT("nor a held name the tables do not know"), Holds(0, Gone));
	TestTrue(TEXT("(an AI people holds the inherited edict before its first AI stage)"), Holds(1, Plain));
	TestEqual(TEXT("Check changes nothing"), Sim->ComputeStateHash(), Hash);
	Sim->RunHeadless(1);
	TestFalse(TEXT("After its first AI stage an AI people holds none"), Holds(1, Plain));
	TestTrue(TEXT("(the ruled people keeps it)"), Holds(0, Plain));
	return true;
}

// The one gate of a card's choice (header section 2.4): the answer refuses a gated choice before its draw, with an error line;
// the policy passes it by through the same function; an open choice is answered with its one draw; a card that no longer
// stands is withdrawn with its draw as before.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimPetitionChoiceGateTest, "Cosmic_Ascension.Sim.PetitionChoiceGate", CosmicTests::Flags)
bool FCosmicSimPetitionChoiceGateTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPressureCardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* State = LayBoard(*Sim, Original);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const int32 Capital = CapitalOf(*Sim, 0);
	const FEncounterDef* Card = Sim->GetTables().FindEncounter(Gate);
	if (!TestNotNull(TEXT("(the test card)"), Card) || !TestTrue(TEXT("(the capital)"), State->Systems.IsValidIndex(Capital)))
	{
		return false;
	}
	FCivilizationState& Ruled = State->Civs[0];
	Ruled.Stockpile.Influence = 50.f;
	State->Systems[Capital].ActiveHazard = Tag(TEXT("Hazard.Plague"));
	State->Systems[Capital].HazardTicksRemaining = 100;
	auto Put = [&]()
	{
		State->Knowing.PendingPetitionRow = Gate;
		State->Knowing.PendingPetitionSystemID = State->Systems[Capital].SystemID;
	};
	auto GateOf = [&](int32 Choice)
	{
		return CosmicPetitions::ChoiceGateOf(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0, Capital, NAME_None, Card->Choices[Choice]);
	};
	// A refused answer: nothing drawn, nothing changed, the card waits; one error line.
	auto RefusedAnswer = [&](int32 Choice)
	{
		Put();
		const uint32 Hash = Sim->ComputeStateHash();
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		AddExpectedError(FString::Printf(TEXT("gate refuses choice %d"), Choice), EAutomationExpectedErrorFlags::Contains, 1);
		const FCosmicPetitionAnswer Answer = Role->AnswerPetition(Choice);
		return Answer.Outcome == ECosmicPetitionOutcome::Refused && Sim->GetRng().GetCurrentSeed() == Seed && Sim->ComputeStateHash() == Hash
			&& State->Knowing.PendingPetitionRow == Gate;
	};

	// The first rung: the slot taken by Taxing, the second-rung edict, no fleet.
	Ruled.ActiveEdicts = { Taxing };
	TestTrue(TEXT("An edict with every slot taken is gated: Edict, Slots"), GateOf(0).Kind == ECosmicChoiceGateKind::Edict && GateOf(0).Edict == ECosmicEdictBlock::Slots);
	TestTrue(TEXT("and its answer is refused before the draw"), RefusedAnswer(0));
	TestTrue(TEXT("An edict of a higher rung: Edict, Tier"), GateOf(1).Kind == ECosmicChoiceGateKind::Edict && GateOf(1).Edict == ECosmicEdictBlock::Tier);
	TestTrue(TEXT("refused too"), RefusedAnswer(1));
	TestTrue(TEXT("The fleet's time on the first rung: Resist, NoFleet"), GateOf(2).Kind == ECosmicChoiceGateKind::Resist && GateOf(2).Resist == ECosmicResistBlock::NoFleet);
	TestTrue(TEXT("refused too"), RefusedAnswer(2));
	TestTrue(TEXT("The fleet's time on a hazard the system does not hold: Resist, NoHazard"), GateOf(3).Kind == ECosmicChoiceGateKind::Resist && GateOf(3).Resist == ECosmicResistBlock::NoHazard);
	TestTrue(TEXT("A choice with no gated effect is open"), GateOf(4).IsOpen());
	Put();
	TestEqual(TEXT("The policy passes every gated choice by and watches"),
		CosmicRulerPolicy::ChoiceOf(*State, Sim->GetStateIndex(), *Card, 0, Gate, 0, &Sim->GetRules(), &Sim->GetTables()), 4);

	// Held is a refusal too (a choice that would change nothing draws no number).
	Ruled.ActiveEdicts = { Plain };
	TestTrue(TEXT("An edict held: Edict, Held"), GateOf(0).Kind == ECosmicChoiceGateKind::Edict && GateOf(0).Edict == ECosmicEdictBlock::Held);
	TestTrue(TEXT("refused"), RefusedAnswer(0));

	// The second rung with no fleet now: Spent; a rebellion on the card's system: Capital.
	Ruled.ActiveEdicts.Reset();
	Ruled.Tier = EAscensionTier::Tier2_Stellar;
	Ruled.FleetPower = 0.f;
	TestTrue(TEXT("The fleet's time with the fleet spent: Resist, Spent"), GateOf(2).Kind == ECosmicChoiceGateKind::Resist && GateOf(2).Resist == ECosmicResistBlock::Spent);
	TestTrue(TEXT("refused"), RefusedAnswer(2));
	State->Systems[Capital].ActiveHazard = Tag(TEXT("Hazard.Rebellion"));
	TestTrue(TEXT("The fleet's time on a rebellion: Resist, Capital"), GateOf(3).Kind == ECosmicChoiceGateKind::Resist && GateOf(3).Resist == ECosmicResistBlock::Capital);
	TestTrue(TEXT("refused"), RefusedAnswer(3));
	State->Systems[Capital].ActiveHazard = Tag(TEXT("Hazard.Plague"));

	// An open choice: answered with its one draw.
	{
		Put();
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		const FCosmicPetitionAnswer Answer = Role->AnswerPetition(0);
		TestTrue(TEXT("An open edict choice is answered"), Answer.Outcome == ECosmicPetitionOutcome::Succeeded && Sim->GetRng().GetCurrentSeed() != Seed);
		TestTrue(TEXT("and enacts its edict"), Ruled.ActiveEdicts == TArray<FName>({ Plain }));
	}

	// A card that no longer stands - its system lost - is withdrawn with its draw, even on a gated choice.
	{
		Ruled.ActiveEdicts = { Taxing };
		Put();
		const int32 Other = State->Civs.IsValidIndex(1) ? 1 : INDEX_NONE;
		State->Systems[Capital].OwnerCivID = State->Civs[Other].CivID;
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		const FCosmicPetitionAnswer Answer = Role->AnswerPetition(0);
		TestTrue(TEXT("A card that no longer stands is withdrawn, with its draw"), Answer.Outcome == ECosmicPetitionOutcome::Withdrawn && Sim->GetRng().GetCurrentSeed() != Seed
			&& State->Knowing.PendingPetitionRow.IsNone());
		State->Systems[Capital].OwnerCivID = Ruled.CivID;
	}
	return true;
}

// DelayHazard (header section 2.4, 2.5): through ResistBlockOf alone - a standing hazard gains Value once (whatever the tier
// gap to its source), the fleet keeps ResistFleetFactor of itself, the hazard stays; every refusal changes nothing and the
// choice's gate gives the same first reason; the countdown is cut at MAX_int32.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimResistDelayTest, "Cosmic_Ascension.Sim.ResistDelay", CosmicTests::Flags)
bool FCosmicSimResistDelayTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPressureCardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* State = LayBoard(*Sim, Original);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const int32 Capital = CapitalOf(*Sim, 0);
	if (!TestTrue(TEXT("(the capital has a lane)"), State->Systems.IsValidIndex(Capital) && !State->Systems[Capital].Lanes.IsEmpty()))
	{
		return false;
	}
	// A claim on the capital, its claimant a fourth-rung fallen sphere one lane away: two rungs above a second-rung people.
	CosmicTests::AddInstance(*State, Fallen, { State->Systems[Capital].Lanes[0] });
	FCivilizationState& Ruled = State->Civs[0];
	Ruled.Tier = EAscensionTier::Tier2_Stellar;
	Ruled.FleetPower = 1.f;
	FStarSystemState& System = State->Systems[Capital];
	System.ActiveHazard = Tag(TEXT("Hazard.Claim"));
	System.HazardTicksRemaining = 100;
	const FCosmicEffect OnClaim = Effect(EEffectType::DelayHazard, TEXT("Hazard.Claim"), NAME_None, 50.f);
	{
		const uint32 Hash = Sim->ComputeStateHash();
		TestTrue(TEXT("A per-tick source never buys a hazard time"), !ApplyOn(*Sim, OnClaim, 0, Capital, true) && Sim->ComputeStateHash() == Hash
			&& System.HazardTicksRemaining == 100 && Ruled.FleetPower == 1.f);
	}
	TestEqual(TEXT("(the claimant is the fourth-rung sphere beside the capital)"),
		CosmicHazards::ClaimantOf(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), Capital), 0);
	{
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		TestTrue(TEXT("The fleet buys a standing claim time"), ApplyOn(*Sim, OnClaim, 0, Capital));
		TestEqual(TEXT("its Value once, though its source stands two rungs above"), System.HazardTicksRemaining, 150);
		TestEqual(TEXT("and the fleet keeps ResistFleetFactor of itself"), Ruled.FleetPower, 0.5f);
		TestTrue(TEXT("The claim still stands"), System.ActiveHazard == Tag(TEXT("Hazard.Claim")));
		TestEqual(TEXT("No draw"), Sim->GetRng().GetCurrentSeed(), Seed);
	}

	// Every refusal changes nothing, and the choice's gate gives the same first reason.
	FEncounterChoice Choice;
	auto Case = [&](const TCHAR* What, const FCosmicEffect& Delay, int32 CivIndex, ECosmicResistBlock Expected)
	{
		const uint32 Hash = Sim->ComputeStateHash();
		const ECosmicResistBlock Block = CosmicConflict::ResistBlockOf(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), CivIndex, Capital, Delay.Tag);
		Choice.OnSuccess = { Delay };
		const FCosmicChoiceGate ChoiceGate = CosmicPetitions::ChoiceGateOf(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), CivIndex, Capital, NAME_None, Choice);
		TestTrue(*FString::Printf(TEXT("%s: the gate's reason"), What), Block == Expected);
		TestTrue(*FString::Printf(TEXT("%s: the choice's gate gives the same"), What), ChoiceGate.Kind == ECosmicChoiceGateKind::Resist && ChoiceGate.Resist == Expected);
		TestTrue(*FString::Printf(TEXT("%s: refused, nothing changed"), What), !ApplyOn(*Sim, Delay, CivIndex, Capital) && Sim->ComputeStateHash() == Hash);
	};
	Case(TEXT("A hazard the system does not hold"), Effect(EEffectType::DelayHazard, TEXT("Hazard.Plague"), NAME_None, 50.f), 0, ECosmicResistBlock::NoHazard);
	Case(TEXT("A people that does not own the system"), OnClaim, 1, ECosmicResistBlock::NotOwn);
	Ruled.FleetPower = 0.f;
	Case(TEXT("A fleet spent"), OnClaim, 0, ECosmicResistBlock::Spent);
	Ruled.FleetPower = 1.f;
	Ruled.Tier = EAscensionTier::Tier1_Planetary;
	Case(TEXT("The first rung, no fleet"), OnClaim, 0, ECosmicResistBlock::NoFleet);
	Ruled.Tier = EAscensionTier::Tier2_Stellar;
	System.ActiveHazard = Tag(TEXT("Hazard.Rebellion"));
	Case(TEXT("A rebellion"), Effect(EEffectType::DelayHazard, TEXT("Hazard.Rebellion"), NAME_None, 50.f), 0, ECosmicResistBlock::Capital);
	System.ActiveHazard = Tag(TEXT("Hazard.Claim"));
	{
		// A Value below 1 (outside the lint) adds no tick and spends no fleet, on a board whose gate is open.
		const uint32 Hash = Sim->ComputeStateHash();
		TestFalse(TEXT("A Value below 1 is refused on an open gate"), ApplyOn(*Sim, Effect(EEffectType::DelayHazard, TEXT("Hazard.Claim"), NAME_None, 0.5f), 0, Capital));
		TestEqual(TEXT("and changes nothing"), Sim->ComputeStateHash(), Hash);
	}

	// The countdown is cut at MAX_int32; the share of the fleet is the knob in [0, 1], else 1.
	System.HazardTicksRemaining = MAX_int32 - 10;
	TestTrue(TEXT("Near the top"), ApplyOn(*Sim, OnClaim, 0, Capital));
	TestEqual(TEXT("the countdown stops at MAX_int32"), System.HazardTicksRemaining, MAX_int32);
	FCosmicRulesDef Knob = Sim->GetRules();
	Knob.ResistFleetFactor = 0.f;
	const bool bZero = CosmicConflict::ResistShareOf(Knob) == 0.f;
	Knob.ResistFleetFactor = 1.5f;
	const bool bAbove = CosmicConflict::ResistShareOf(Knob) == 1.f;
	Knob.ResistFleetFactor = std::numeric_limits<float>::quiet_NaN();
	const bool bNaN = CosmicConflict::ResistShareOf(Knob) == 1.f;
	TestTrue(TEXT("A factor of 0 spends the whole fleet, one above 1 or not a number keeps it"), bZero && bAbove && bNaN);

	// The ticks a Value adds ([B-4c]: the effect and the card window read the one function): the whole part from 1, cut at
	// 2^31; below 1, infinite or not a number, none.
	TestEqual(TEXT("A Value of 50 adds 50 ticks"), CosmicConflict::ResistTicksOf(50.f), int64(50));
	TestEqual(TEXT("A Value of 1 adds one tick"), CosmicConflict::ResistTicksOf(1.f), int64(1));
	TestEqual(TEXT("A Value of 1.9 adds its whole part"), CosmicConflict::ResistTicksOf(1.9f), int64(1));
	TestEqual(TEXT("A Value below 1 adds none"), CosmicConflict::ResistTicksOf(0.9f), int64(0));
	TestEqual(TEXT("A Value below 0 adds none"), CosmicConflict::ResistTicksOf(-50.f), int64(0));
	TestEqual(TEXT("A Value that is not a number adds none"), CosmicConflict::ResistTicksOf(std::numeric_limits<float>::quiet_NaN()), int64(0));
	TestEqual(TEXT("An infinite Value adds none"), CosmicConflict::ResistTicksOf(std::numeric_limits<float>::infinity()), int64(0));
	TestEqual(TEXT("A Value of 3e9 is cut at 2^31"), CosmicConflict::ResistTicksOf(3.0e9f), int64(2147483648));
	return true;
}

// The pressure policies of the script (header section 2.9 - measurement only): the order of the kinds on a pressure card,
// Tribute refused, the edicts a card enacted and the vigil kept while a claim stands, the cards counted by the ruled people's
// rung (one bin per rung - B6b [B-6b3b]), no relief on a claim, the guard of -Guard=1, and the near-sphere crises
// -LeaveNearSphere=1 leaves alone; Round as before.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimRulerPressurePolicyTest, "Cosmic_Ascension.Sim.RulerPressurePolicy", CosmicTests::Flags)
bool FCosmicSimRulerPressurePolicyTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPressureCardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	// The words of -PlayerPolicy.
	{
		ECosmicPressurePolicy Read = ECosmicPressurePolicy::Round;
		TestTrue(TEXT("Cautious, Reckless and Faithful are read"), CosmicRulerPolicy::PressurePolicyOf(TEXT("cautious"), Read) && Read == ECosmicPressurePolicy::Cautious
			&& CosmicRulerPolicy::PressurePolicyOf(TEXT("Reckless"), Read) && Read == ECosmicPressurePolicy::Reckless
			&& CosmicRulerPolicy::PressurePolicyOf(TEXT("FAITHFUL"), Read) && Read == ECosmicPressurePolicy::Faithful);
		TestFalse(TEXT("Tribute is refused (B4 has no tribute)"), CosmicRulerPolicy::PressurePolicyOf(TEXT("Tribute"), Read));
		TestFalse(TEXT("and so is any other word"), CosmicRulerPolicy::PressurePolicyOf(TEXT("Bold"), Read));
	}

	FGalaxyState* State = LayPressureBoard(*Sim, Original);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const FCosmicDataTables& Tables = Sim->GetTables();
	const FEncounterDef* First = Tables.FindEncounter(PressFirst);
	const FEncounterDef* Second = Tables.FindEncounter(PressSecond);
	if (!TestNotNull(TEXT("(the two test pressure cards)"), First) || !TestNotNull(TEXT("(the second)"), Second))
	{
		return false;
	}
	FCivilizationState& Ruled = State->Civs[0];
	const int32 Capital = CapitalOf(*Sim, 0);
	const int32 Near = PressNear;
	// A claim on the near system, the second card's fleet time possible (second rung, a fleet).
	State->Systems[Near].ActiveHazard = Tag(TEXT("Hazard.Claim"));
	State->Systems[Near].HazardTicksRemaining = 200;
	Ruled.Tier = EAscensionTier::Tier2_Stellar;
	Ruled.FleetPower = 1.f;
	Ruled.Stockpile.Insight = 500.f;
	Ruled.Stockpile.Influence = 500.f;
	Ruled.Research.Add(FName(TEXT("Society")), 1.f);
	auto Choose = [&](const FEncounterDef& Card, ECosmicPressurePolicy Pressure, int32 Answers = 0)
	{
		State->Knowing.PendingPetitionRow = &Card == First ? PressFirst : PressSecond;
		State->Knowing.PendingPetitionSystemID = State->Systems[Near].SystemID;
		return CosmicRulerPolicy::ChoiceOf(*State, Sim->GetStateIndex(), Card, Answers, State->Knowing.PendingPetitionRow, 1, &Sim->GetRules(), &Tables, Pressure);
	};

	// The order of the kinds. The first card: [0] the vigil, [1] discern (Insight, the next card), [2] the watch. The second:
	// [0] the other edict, [1] the fleet's time, [2] the watch.
	TestEqual(TEXT("Faithful takes the vigil"), Choose(*First, ECosmicPressurePolicy::Faithful), 0);
	TestEqual(TEXT("Reckless discerns"), Choose(*First, ECosmicPressurePolicy::Reckless), 1);
	TestEqual(TEXT("Cautious watches"), Choose(*First, ECosmicPressurePolicy::Cautious), 2);
	TestEqual(TEXT("On the second card Faithful takes the other edict"), Choose(*Second, ECosmicPressurePolicy::Faithful), 0);
	TestEqual(TEXT("Reckless sends the fleet"), Choose(*Second, ECosmicPressurePolicy::Reckless), 1);
	TestEqual(TEXT("Cautious watches"), Choose(*Second, ECosmicPressurePolicy::Cautious), 2);
	// Where the round would choose otherwise (one answer so far: the round starts at [1]).
	TestEqual(TEXT("Faithful takes the vigil where the round would discern"), Choose(*First, ECosmicPressurePolicy::Faithful, 1), 0);
	TestEqual(TEXT("and the other edict where the round would send the fleet"), Choose(*Second, ECosmicPressurePolicy::Faithful, 1), 0);
	bool bRounds = true;
	for (int32 Answers = 0; Answers < 3; ++Answers)
	{
		bRounds &= Choose(*First, ECosmicPressurePolicy::Round, Answers) == Answers;
	}
	TestTrue(TEXT("Round takes its turn as for every card"), bRounds);
	// Blocked kinds pass to the next: the vigil held, Faithful discerns; with no Insight to discern, it watches; the fleet spent,
	// Reckless watches on the second card.
	Ruled.ActiveEdicts = { PressVigil };
	TestEqual(TEXT("With the vigil held Faithful discerns"), Choose(*First, ECosmicPressurePolicy::Faithful), 1);
	Ruled.Stockpile.Insight = 0.f;
	TestEqual(TEXT("with nothing to pay for it, it watches"), Choose(*First, ECosmicPressurePolicy::Faithful), 2);
	TestEqual(TEXT("and so does Reckless"), Choose(*First, ECosmicPressurePolicy::Reckless), 2);
	Ruled.FleetPower = 0.f;
	TestEqual(TEXT("The fleet spent, Reckless watches on the second card"), Choose(*Second, ECosmicPressurePolicy::Reckless), 2);
	Ruled.ActiveEdicts.Reset();
	Ruled.FleetPower = 1.f;
	Ruled.Stockpile.Insight = 500.f;

	// A card's edict and the vigil stay while the claim stands. Faithful answers the first card: the vigil, enacted by the card.
	const FCosmicRulerPolicy Faithful{ true, false, false, true, NAME_None, 0, ECosmicPressurePolicy::Faithful, 0, false };
	{
		FCosmicRulerPolicyCounts Counts;
		// One answer so far: the round would discern - only the policy Act hands to ChoiceOf takes the vigil.
		Counts.Answers = 1;
		State->Knowing.PendingPetitionRow = PressFirst;
		State->Knowing.PendingPetitionSystemID = State->Systems[Near].SystemID;
		CosmicRulerPolicy::Act(Faithful, *Sim, *Role, NAME_None, Counts);
		TestTrue(TEXT("Faithful's answer enacted the vigil"), Ruled.ActiveEdicts.Contains(PressVigil));
		TestTrue(TEXT("which the counts keep as the card's"), Counts.CardEdicts.Contains(PressVigil));
		// Every slot taken (Society 1.0: three) and a hazard that wants an answer edict: room is made, but not with the vigil.
		Ruled.ActiveEdicts = { PressVigil, PressOther, FName(TEXT("edict_mobilization")) };
		State->Systems[Capital].ActiveHazard = Tag(TEXT("Hazard.AIAwakening"));
		State->Systems[Capital].HazardTicksRemaining = 100;
		Ruled.Stability = 0.9f;
		CosmicRulerPolicy::ActEdicts(Faithful, *Sim, *Role, Counts);
		TestTrue(TEXT("While the claim stands, room for an answer is never made with the card's vigil"), Ruled.ActiveEdicts.Contains(PressVigil));
		TestTrue(TEXT("(room was made with another edict)"), Ruled.ActiveEdicts.Contains(PressAnswer));
		TestTrue(TEXT("and at the resting level the vigil is not revoked while the claim stands"), Ruled.ActiveEdicts.Contains(PressVigil));
		// The claim over: the vigil goes once the danger passed.
		State->Systems[Near].ActiveHazard = FGameplayTag();
		State->Systems[Near].HazardTicksRemaining = 0;
		CosmicRulerPolicy::ActEdicts(Faithful, *Sim, *Role, Counts);
		TestFalse(TEXT("The claim over, the card's vigil goes at the resting level"), Ruled.ActiveEdicts.Contains(PressVigil));
		State->Systems[Capital].ActiveHazard = FGameplayTag();
		State->Systems[Capital].HazardTicksRemaining = 0;
		State->Systems[Near].ActiveHazard = Tag(TEXT("Hazard.Claim"));
		State->Systems[Near].HazardTicksRemaining = 200;
	}

	// The pressure cards by the ruled people's rung, one bin per rung (B6b [B-6b3b] - the fourth and the fifth are bins of their
	// own, mutation: the three bins of B4, the third and up): a silent policy (no answer, no input) counts the card that came.
	{
		const FCosmicRulerPolicy Silent{ false, false, false, false, NAME_None, 0, ECosmicPressurePolicy::Round, 0, false, false };
		const EAscensionTier TierWas = Ruled.Tier;
		const FName CardWas = State->Knowing.PendingPetitionRow;
		const FName CardSystemWas = State->Knowing.PendingPetitionSystemID;
		TestTrue(TEXT("(the vessel rules the board's people)"), State->Player.Role == EPlayerRole::Ruler && Ruled.bIsPlayerRuled);
		State->Knowing.PendingPetitionRow = PressFirst;
		State->Knowing.PendingPetitionSystemID = State->Systems[Near].SystemID;
		Ruled.Tier = EAscensionTier::Tier4_Transcendent;
		FCosmicRulerPolicyCounts Counts;
		CosmicRulerPolicy::Act(Silent, *Sim, *Role, NAME_None, Counts);
		TestTrue(TEXT("A pressure card at the fourth rung is counted in the fourth bin, not the third"), Counts.PressureCards == 1 && Counts.PressureCardsByTier[3] == 1
			&& Counts.PressureCardsByTier[2] == 0);
		Ruled.Tier = EAscensionTier::Tier5_EternalOrder;
		FCosmicRulerPolicyCounts Fifth;
		CosmicRulerPolicy::Act(Silent, *Sim, *Role, NAME_None, Fifth);
		TestTrue(TEXT("and at the fifth rung in the fifth"), Fifth.PressureCardsByTier[4] == 1 && Fifth.PressureCardsByTier[3] == 0 && Fifth.PressureCardsByTier[2] == 0);
		Ruled.Tier = TierWas;
		State->Knowing.PendingPetitionRow = CardWas;
		State->Knowing.PendingPetitionSystemID = CardSystemWas;
	}

	// The vigil the script enacted itself (step (3)) while the claim stands: a pressure policy keeps it - in step (4) and when
	// room is made; Round revokes it at the resting level as before B4. A card's vigil stays under Round too.
	{
		const FCosmicRulerPolicy RoundEdicts{ false, false, false, true, NAME_None, 0, ECosmicPressurePolicy::Round, 0, false };
		Ruled.ActiveEdicts = { PressVigil };
		Ruled.Stability = 0.9f;
		FCosmicRulerPolicyCounts Own;
		Own.VigilEnacted = PressVigil;
		CosmicRulerPolicy::ActEdicts(Faithful, *Sim, *Role, Own);
		TestTrue(TEXT("A pressure policy keeps the script's own vigil while the claim stands"), Ruled.ActiveEdicts.Contains(PressVigil));
		Ruled.ActiveEdicts = { PressVigil, PressOther, FName(TEXT("edict_mobilization")) };
		State->Systems[Capital].ActiveHazard = Tag(TEXT("Hazard.AIAwakening"));
		State->Systems[Capital].HazardTicksRemaining = 100;
		CosmicRulerPolicy::ActEdicts(Faithful, *Sim, *Role, Own);
		TestTrue(TEXT("and makes no room with it"), Ruled.ActiveEdicts.Contains(PressVigil) && Ruled.ActiveEdicts.Contains(PressAnswer));
		State->Systems[Capital].ActiveHazard = FGameplayTag();
		State->Systems[Capital].HazardTicksRemaining = 0;
		Ruled.ActiveEdicts = { PressVigil };
		FCosmicRulerPolicyCounts RoundOwn;
		RoundOwn.VigilEnacted = PressVigil;
		CosmicRulerPolicy::ActEdicts(RoundEdicts, *Sim, *Role, RoundOwn);
		TestFalse(TEXT("Round revokes its own vigil at the resting level, claim or not (as before B4)"), Ruled.ActiveEdicts.Contains(PressVigil));
		Ruled.ActiveEdicts = { PressVigil };
		FCosmicRulerPolicyCounts RoundCard;
		RoundCard.CardEdicts = { PressVigil };
		CosmicRulerPolicy::ActEdicts(RoundEdicts, *Sim, *Role, RoundCard);
		TestTrue(TEXT("but keeps a card's vigil while the claim stands"), Ruled.ActiveEdicts.Contains(PressVigil));
		Ruled.ActiveEdicts.Reset();
	}

	// No relief on a claim: the project key's list without the claim's answer, then a settlement.
	{
		const FCosmicRulerPolicy RoundBuild{ false, true, false, false, NAME_None, 0, ECosmicPressurePolicy::Round, 0, false };
		const FCosmicRulerPolicy FaithfulBuild{ false, true, false, false, NAME_None, 0, ECosmicPressurePolicy::Faithful, 0, false };
		const TArray<TPair<int32, FName>> All = CosmicRulerPolicy::ProjectChoicesOf(RoundBuild, *State, Sim->GetStateIndex(), Tables, Sim->GetRules(), 0);
		const TArray<TPair<int32, FName>> Kept = CosmicRulerPolicy::ProjectChoicesOf(FaithfulBuild, *State, Sim->GetStateIndex(), Tables, Sim->GetRules(), 0);
		const bool bReliefOffered = All.ContainsByPredicate([&](const TPair<int32, FName>& Choice) { return Choice.Key == Near && Choice.Value == PressRelief; });
		TestTrue(TEXT("(the claim's relief is among the project key's choices)"), bReliefOffered);
		TestFalse(TEXT("A pressure policy leaves the claim's relief out"), Kept.ContainsByPredicate([&](const TPair<int32, FName>& Choice) { return Choice.Key == Near && Choice.Value == PressRelief; }));
		// The claim's relief the only project left (the capital's construction slot taken) and a wild system one lane from the
		// capital: the pressure policy settles it, Round starts the relief.
		const int32 Wild = 11;
		FCivilizationState& Fourth = State->Civs[3];
		Fourth.OwnedSystems.Remove(State->Systems[Wild].SystemID);
		State->Systems[Wild].OwnerCivID = NAME_None;
		State->Systems[Capital].Lanes.AddUnique(State->Systems[Wild].SystemID);
		State->Systems[Wild].Lanes.AddUnique(State->Systems[Capital].SystemID);
		State->Systems[Capital].ActiveMegaproject = FName(TEXT("project_dyson"));
		TestTrue(TEXT("(the pressure policy keeps no project)"), CosmicRulerPolicy::ProjectChoicesOf(FaithfulBuild, *State, Sim->GetStateIndex(), Tables, Sim->GetRules(), 0).IsEmpty());
		TestTrue(TEXT("(the wild system can be settled)"), CosmicMegaprojects::SettleTargets(*State, Sim->GetStateIndex(), Tables, Sim->GetRules(), 0).Contains(Wild));
		FCosmicRulerPolicyCounts Counts;
		CosmicRulerPolicy::Act(FaithfulBuild, *Sim, *Role, NAME_None, Counts);
		TestTrue(TEXT("and its project key starts no relief there"), State->Systems[Near].ResponseMegaproject != PressRelief);
		TestTrue(TEXT("but a settlement, its list being empty"), Counts.Started >= 1 && State->Systems[Wild].ActiveMegaproject == CosmicRows::ProjectSettle);
		FCosmicRulerPolicyCounts RoundCounts;
		CosmicRulerPolicy::Act(RoundBuild, *Sim, *Role, NAME_None, RoundCounts);
		TestTrue(TEXT("Round's project key starts the relief"), State->Systems[Near].ResponseMegaproject == PressRelief);
		State->Systems[Near].ResponseMegaproject = NAME_None;
		State->Systems[Near].ResponseProgress = 0.f;
	}

	// The near-sphere crises: a plague on a settled system a claimant could reach is left alone under a pressure policy with
	// -LeaveNearSphere=1, answered under Round; a supernova's warning is always answered.
	{
		State->Systems[Near].ActiveHazard = Tag(TEXT("Hazard.Plague"));
		State->Systems[Near].HazardTicksRemaining = 100;
		const FCosmicRulerPolicy Leave{ false, true, false, false, NAME_None, 0, ECosmicPressurePolicy::Faithful, 0, true };
		const FCosmicRulerPolicy RoundLeave{ false, true, false, false, NAME_None, 0, ECosmicPressurePolicy::Round, 0, true };
		auto Offers = [&](const FCosmicRulerPolicy& Policy, FName Project)
		{
			return CosmicRulerPolicy::ProjectChoicesOf(Policy, *State, Sim->GetStateIndex(), Tables, Sim->GetRules(), 0).ContainsByPredicate([&](const TPair<int32, FName>& Choice)
			{
				return Choice.Key == Near && Choice.Value == Project;
			});
		};
		TestTrue(TEXT("(a claimant could reach the near system)"), CosmicHazards::ClaimantOf(*State, Sim->GetStateIndex(), Tables, Sim->GetRules(), Near) != INDEX_NONE);
		TestFalse(TEXT("-LeaveNearSphere=1 leaves a plague there unanswered"), Offers(Leave, PressRelief));
		TestTrue(TEXT("but not under Round"), Offers(RoundLeave, PressRelief));
		const FCosmicRulerPolicy Stay{ false, true, false, false, NAME_None, 0, ECosmicPressurePolicy::Faithful, 0, false };
		TestTrue(TEXT("nor under a pressure policy without the switch"), Offers(Stay, PressRelief));
		const FCosmicRulerPolicy RecklessLeave{ false, true, false, false, NAME_None, 0, ECosmicPressurePolicy::Reckless, 0, true };
		const FCosmicRulerPolicy CautiousLeave{ false, true, false, false, NAME_None, 0, ECosmicPressurePolicy::Cautious, 0, true };
		TestTrue(TEXT("The switch leaves it alone under Reckless and Cautious too"), !Offers(RecklessLeave, PressRelief) && !Offers(CautiousLeave, PressRelief));
		State->Systems[Near].ActiveHazard = Tag(TEXT("Hazard.PreSupernova"));
		TestTrue(TEXT("A supernova's warning there is answered"), Offers(Leave, PressCalm));
		State->Systems[Near].ActiveHazard = Tag(TEXT("Hazard.Claim"));
		State->Systems[Near].HazardTicksRemaining = 200;
	}

	// The near-sphere crises' answer edicts too: an awakening on the near system - its answer edict is not enacted under a
	// pressure policy with -LeaveNearSphere=1, and is under Round.
	{
		State->Systems[PressNear].ActiveHazard = Tag(TEXT("Hazard.AIAwakening"));
		State->Systems[PressNear].HazardTicksRemaining = 100;
		Ruled.ActiveEdicts.Reset();
		const FCosmicRulerPolicy LeaveEdicts{ false, false, false, true, NAME_None, 0, ECosmicPressurePolicy::Faithful, 0, true };
		const FCosmicRulerPolicy RoundEdicts{ false, false, false, true, NAME_None, 0, ECosmicPressurePolicy::Round, 0, true };
		FCosmicRulerPolicyCounts LeaveCounts;
		CosmicRulerPolicy::ActEdicts(LeaveEdicts, *Sim, *Role, LeaveCounts);
		TestFalse(TEXT("-LeaveNearSphere=1 enacts no answer to an awakening a claimant could follow"), Ruled.ActiveEdicts.Contains(PressAnswer));
		FCosmicRulerPolicyCounts RoundCounts;
		CosmicRulerPolicy::ActEdicts(RoundEdicts, *Sim, *Role, RoundCounts);
		TestTrue(TEXT("Round answers it"), Ruled.ActiveEdicts.Contains(PressAnswer));
		Ruled.ActiveEdicts.Reset();
		const FCosmicRulerPolicy StayEdicts{ false, false, false, true, NAME_None, 0, ECosmicPressurePolicy::Faithful, 0, false };
		FCosmicRulerPolicyCounts StayCounts;
		CosmicRulerPolicy::ActEdicts(StayEdicts, *Sim, *Role, StayCounts);
		TestTrue(TEXT("and so does a pressure policy without the switch"), Ruled.ActiveEdicts.Contains(PressAnswer));
		Ruled.ActiveEdicts.Reset();
		State->Systems[PressNear].ActiveHazard = Tag(TEXT("Hazard.Claim"));
		State->Systems[PressNear].HazardTicksRemaining = 200;
	}

	// The guard of -Guard=1: the system one lane from the higher sphere (PressNear), kept across boundaries without being
	// taken back and set again, the treaty step running behind it; with no diplomacy and after the end too; the capital
	// when no settled system lies near a higher sphere. -Guard=0 after the end withdraws nothing.
	{
		State = LayPressureBoard(*Sim, Original);
		if (!TestNotNull(TEXT("The board is laid again"), State))
		{
			return false;
		}
		FCivilizationState& Guarding = State->Civs[0];
		Guarding.Tier = EAscensionTier::Tier2_Stellar;
		Guarding.FleetPower = 1.f;
		Guarding.Stockpile.Influence = 500.f;
		const FName NearID = State->Systems[PressNear].SystemID;
		const FCosmicRulerPolicy Guard{ false, false, false, false, NAME_None, 1, ECosmicPressurePolicy::Round, 1, false };
		const FCosmicRulerPolicy GuardOnly{ false, false, false, false, NAME_None, 0, ECosmicPressurePolicy::Round, 1, false };
		const FCosmicRulerPolicy Peace{ false, false, false, false, NAME_None, 1, ECosmicPressurePolicy::Round, 0, false };
		TestEqual(TEXT("The guard keeps the system one lane from the higher sphere"),
			CosmicRulerPolicy::GuardTargetOf(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0), NearID);
		FCosmicRulerPolicyCounts Counts;
		CosmicRulerPolicy::ActDiplomacy(Guard, *Sim, *Role, Counts);
		TestTrue(TEXT("-Guard=1 sets the guard there"), Guarding.FleetTargetSystemID == NearID && Counts.FleetTargetChanges == 1);
		const int64 Since = Guarding.FleetTargetSinceTick;
		for (int32 Boundary = 0; Boundary < 9; ++Boundary)
		{
			Sim->GetMutableState().Tick += 1;
			CosmicRulerPolicy::ActDiplomacy(Guard, *Sim, *Role, Counts);
		}
		TestTrue(TEXT("and keeps it over ten boundaries without taking it back"), Guarding.FleetTargetSystemID == NearID && Guarding.FleetTargetSinceTick == Since
			&& Counts.FleetTargetChanges == 1);
		TestTrue(TEXT("while the treaty step runs behind it"), !Counts.TreatyLooks.IsEmpty());
		// The sphere moves over the near system: the capital is beside it now and the guard moves there, once, and returns.
		{
			const TArray<FName> SphereBefore = State->Transcendents[0].InfluencedSystems;
			State->Transcendents[0].InfluencedSystems = { NearID };
			Sim->GetMutableState().Tick += 1;
			CosmicRulerPolicy::ActDiplomacy(Guard, *Sim, *Role, Counts);
			TestTrue(TEXT("When the system to keep changes the guard moves to it, one change"), Guarding.FleetTargetSystemID == Guarding.HomeSystemID
				&& Counts.FleetTargetChanges == 2);
			State->Transcendents[0].InfluencedSystems = SphereBefore;
			Guarding.FleetTargetSystemID = NearID;
		}
		// A siege on a system of the people comes first, also with no diplomacy.
		{
			const FName CapitalID = Guarding.HomeSystemID;
			State->Civs[1].FleetTargetSystemID = CapitalID;
			FCosmicRulerPolicyCounts Besieged;
			CosmicRulerPolicy::ActDiplomacy(GuardOnly, *Sim, *Role, Besieged);
			TestEqual(TEXT("-Diplomacy=0 -Guard=1 guards a besieged system first"), Guarding.FleetTargetSystemID, CapitalID);
			State->Civs[1].FleetTargetSystemID = NAME_None;
			Guarding.FleetTargetSystemID = NearID;
		}

		// With no diplomacy the guard still stands; after the end too.
		Guarding.FleetTargetSystemID = NAME_None;
		FCosmicRulerPolicyCounts Alone;
		CosmicRulerPolicy::ActDiplomacy(GuardOnly, *Sim, *Role, Alone);
		TestTrue(TEXT("-Diplomacy=0 -Guard=1 guards"), Guarding.FleetTargetSystemID == NearID);
		Guarding.FleetTargetSystemID = NAME_None;
		FCosmicRulerPolicyCounts Ended;
		Ended.bEnded = true;
		CosmicRulerPolicy::ActDiplomacy(GuardOnly, *Sim, *Role, Ended);
		TestTrue(TEXT("and so it does after the end"), Guarding.FleetTargetSystemID == NearID);
		// -Guard=0 after the end: step (1) does not run - the guard is not withdrawn.
		FCosmicRulerPolicyCounts PeaceEnded;
		PeaceEnded.bEnded = true;
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, PeaceEnded);
		TestTrue(TEXT("-Guard=0 withdraws nothing after the end"), Guarding.FleetTargetSystemID == NearID && PeaceEnded.FleetTargetChanges == 0);
		// Before the end -Guard=0 withdraws a guard over a system no siege threatens (as before B4).
		FCosmicRulerPolicyCounts PeaceBefore;
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, PeaceBefore);
		TestTrue(TEXT("(and before it withdraws, as before B4)"), Guarding.FleetTargetSystemID.IsNone());
		// Inside a higher sphere with none beside: that system. Beside before inside, whatever the order of OwnedSystems.
		{
			const FName CapitalID = Guarding.HomeSystemID;
			State->Transcendents[0].InfluencedSystems = { NearID };
			State->Systems[0].Lanes.Remove(NearID);
			State->Systems[PressNear].Lanes.Remove(CapitalID);
			TestEqual(TEXT("A system inside a higher sphere, none beside: the guard keeps it"),
				CosmicRulerPolicy::GuardTargetOf(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0), NearID);
			State->Systems[0].Lanes.Add(NearID);
			State->Systems[PressNear].Lanes.Add(CapitalID);
			Guarding.OwnedSystems = { NearID, CapitalID };
			TestEqual(TEXT("Beside comes before inside"), CosmicRulerPolicy::GuardTargetOf(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0), CapitalID);
			Guarding.OwnedSystems = { CapitalID, NearID };
		}
		// No settled system near a higher sphere: the capital.
		State->Transcendents.Reset();
		TestEqual(TEXT("With no higher sphere near, the guard keeps the capital"),
			CosmicRulerPolicy::GuardTargetOf(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0), Guarding.HomeSystemID);
		// The first rung has no fleet: no guard.
		Guarding.Tier = EAscensionTier::Tier1_Planetary;
		TestTrue(TEXT("On the first rung there is nothing to guard with"), CosmicRulerPolicy::GuardTargetOf(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0).IsNone());
	}
	return true;
}

// The game's two pressure cards (B4 header section 3.3, [B-4d2]): the first comes when a claim stands on a settled system
// of the ruled people from tick 1,800 (not before, not on an AI people's system); its watch changes nothing, its intercession
// enacts the vigil, its gates grey a choice with the first reason; its discernment pays its insight and puts the second
// card by name on the same system - asking the second card's conditions -; the second card's gates on the first rung (the
// new edict's rung, no fleet) and its resistance on the second (the claim's time +50, the fleet's half); no answer of the
// second card chains; each card holds a choice that costs nothing and has no gate.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimPressureChainTest, "Cosmic_Ascension.Sim.PressureChain", CosmicTests::Flags)
bool FCosmicSimPressureChainTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPressureCardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FEncounterDef* ClaimCard = Original.FindEncounter(CosmicRows::PetitionPressureClaim);
	const FEncounterDef* MeasuredCard = Original.FindEncounter(CosmicRows::PetitionPressureMeasured);
	if (!TestNotNull(TEXT("(the game has the claim card)"), ClaimCard) || !TestNotNull(TEXT("(and the measured card)"), MeasuredCard)
		|| !TestEqual(TEXT("(three choices each)"), ClaimCard->Choices.Num() + MeasuredCard->Choices.Num(), 6))
	{
		return false;
	}
	const FGameplayTag Claim = Tag(TEXT("Hazard.Claim"));
	// The game's cards without the first contact card (every people is met on the board).
	UDataTable* GameCards = CosmicTests::CopyWithout<FEncounterDef>(Original.Encounters, { CosmicRows::PetitionFirstContact });
	// The board at Tick with a claim on SystemIndex (people 0's PressNear by default), then one tick.
	auto Lay = [&](UDataTable* Cards, int64 Tick, int32 SystemIndex) -> FGalaxyState*
	{
		FGalaxyState* State = LayPressureBoard(*Sim, Original, Cards);
		if (!State)
		{
			return nullptr;
		}
		State->Tick = Tick;
		State->Systems[SystemIndex].ActiveHazard = Claim;
		State->Systems[SystemIndex].HazardTicksRemaining = 300;
		Sim->StepOnce();
		return State;
	};
	auto GateOf = [&](const FEncounterDef& Card, int32 At)
	{
		return CosmicPetitions::ChoiceGateOf(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0, PressNear, NAME_None, Card.Choices[At]);
	};

	// Not before tick 1,800; not on an AI people's system.
	{
		FGalaxyState* State = Lay(GameCards, 1000, PressNear);
		if (!TestNotNull(TEXT("(the board - before 1,800)"), State))
		{
			return false;
		}
		TestNotEqual(TEXT("Before tick 1,800 no pressure card comes"), State->Knowing.PendingPetitionRow, CosmicRows::PetitionPressureClaim);
		State = Lay(GameCards, 1799, 4);
		if (!TestNotNull(TEXT("(the board - an AI people's system)"), State))
		{
			return false;
		}
		TestNotEqual(TEXT("A claim on an AI people's system brings no card"), State->Knowing.PendingPetitionRow, CosmicRows::PetitionPressureClaim);
	}

	// The first card comes; the watch changes nothing.
	{
		FGalaxyState* State = Lay(GameCards, 1799, PressNear);
		if (!TestNotNull(TEXT("(the board - the first card)"), State))
		{
			return false;
		}
		if (!TestEqual(TEXT("From tick 1,800 a claim on the ruled people's system brings the first card"), State->Knowing.PendingPetitionRow, CosmicRows::PetitionPressureClaim))
		{
			return false;
		}
		TestEqual(TEXT("about that system"), State->Knowing.PendingPetitionSystemID, State->Systems[PressNear].SystemID);
		FCivilizationState& Ruled = State->Civs[0];
		Ruled.Stockpile.Influence = 100.f;
		Ruled.Stockpile.Insight = 300.f;
		const int32 Left = State->Systems[PressNear].HazardTicksRemaining;
		TestTrue(TEXT("(the watch is answered)"), Role->AnswerPetition(2).Outcome == ECosmicPetitionOutcome::Succeeded);
		TestEqual(TEXT("The watch costs no influence"), Ruled.Stockpile.Influence, 100.f);
		TestEqual(TEXT("nor insight"), Ruled.Stockpile.Insight, 300.f);
		TestTrue(TEXT("enacts nothing"), Ruled.ActiveEdicts.IsEmpty());
		TestTrue(TEXT("leaves the claim"), State->Systems[PressNear].ActiveHazard == Claim);
		TestEqual(TEXT("and its time"), State->Systems[PressNear].HazardTicksRemaining, Left);
		TestTrue(TEXT("and puts no card"), State->Knowing.PendingPetitionRow.IsNone());
	}

	// The intercession enacts the vigil; its gate greys it with the first reason.
	{
		FGalaxyState* State = Lay(GameCards, 1799, PressNear);
		if (!TestNotNull(TEXT("(the board - the vigil)"), State) || !TestEqual(TEXT("(the first card again)"), State->Knowing.PendingPetitionRow, CosmicRows::PetitionPressureClaim))
		{
			return false;
		}
		FCivilizationState& Ruled = State->Civs[0];
		TestTrue(TEXT("Without influence the intercession is greyed by the edict's cost"), GateOf(*ClaimCard, 0).Edict == CosmicEdicts::ECosmicEdictBlock::Cost);
		Ruled.ActiveEdicts = { PressOther };
		TestTrue(TEXT("with its slot taken, by the slots"), GateOf(*ClaimCard, 0).Edict == CosmicEdicts::ECosmicEdictBlock::Slots);
		Ruled.ActiveEdicts.Reset();
		Ruled.Stockpile.Influence = 100.f;
		TestTrue(TEXT("Paid, its gate opens"), GateOf(*ClaimCard, 0).IsOpen());
		TestTrue(TEXT("(the intercession is answered)"), Role->AnswerPetition(0).Outcome == ECosmicPetitionOutcome::Succeeded);
		TestTrue(TEXT("The intercession enacts the vigil"), Ruled.ActiveEdicts.Contains(PressVigil));
		TestEqual(TEXT("for the vigil's own cost"), Ruled.Stockpile.Influence, 60.f);
		TestTrue(TEXT("and puts no card"), State->Knowing.PendingPetitionRow.IsNone());
	}

	// The discernment pays its insight and puts the second card by name; the second card's gates on the first rung, and
	// its resistance on the second.
	{
		FGalaxyState* State = Lay(GameCards, 1799, PressNear);
		if (!TestNotNull(TEXT("(the board - the discernment)"), State) || !TestEqual(TEXT("(the first card - discernment)"), State->Knowing.PendingPetitionRow, CosmicRows::PetitionPressureClaim))
		{
			return false;
		}
		FCivilizationState& Ruled = State->Civs[0];
		Ruled.Stockpile.Insight = 300.f;
		TestTrue(TEXT("(the discernment is answered)"), Role->AnswerPetition(1).Outcome == ECosmicPetitionOutcome::Succeeded);
		TestEqual(TEXT("The discernment pays its insight"), Ruled.Stockpile.Insight, 100.f);
		TestEqual(TEXT("and puts the second card"), State->Knowing.PendingPetitionRow, CosmicRows::PetitionPressureMeasured);
		TestEqual(TEXT("on the same system"), State->Knowing.PendingPetitionSystemID, State->Systems[PressNear].SystemID);
		TestTrue(TEXT("On the first rung the new edict is greyed by its rung"), GateOf(*MeasuredCard, 0).Edict == CosmicEdicts::ECosmicEdictBlock::Tier);
		TestTrue(TEXT("the fleet by no fleet"), GateOf(*MeasuredCard, 1).Resist == CosmicConflict::ECosmicResistBlock::NoFleet);
		TestTrue(TEXT("and the further watch stays open"), GateOf(*MeasuredCard, 2).IsOpen());
		Ruled.Tier = EAscensionTier::Tier2_Stellar;
		Ruled.FleetPower = 4.f;
		const int32 Left = State->Systems[PressNear].HazardTicksRemaining;
		TestTrue(TEXT("(the resistance is answered)"), Role->AnswerPetition(1).Outcome == ECosmicPetitionOutcome::Succeeded);
		TestEqual(TEXT("The resistance buys the claim 50 ticks"), State->Systems[PressNear].HazardTicksRemaining, Left + 50);
		TestEqual(TEXT("for the half of the fleet (the game's knob)"), Ruled.FleetPower, 2.f);
		TestTrue(TEXT("and no card follows it"), State->Knowing.PendingPetitionRow.IsNone());
	}

	// The chain asks the second card's conditions: a second card whose condition fails is not put.
	{
		UDataTable* Cards = CosmicTests::CopyWithout<FEncounterDef>(GameCards, { CosmicRows::PetitionPressureMeasured });
		FEncounterDef Shut = *MeasuredCard;
		FCosmicCondition Never;
		Never.Type = EConditionType::GalaxyTickAtLeast;
		Never.Value = 1.0e9f;
		Shut.Conditions.Add(Never);
		Cards->AddRow(CosmicRows::PetitionPressureMeasured, Shut);
		FGalaxyState* State = Lay(Cards, 1799, PressNear);
		if (!TestNotNull(TEXT("(the board - a shut second card)"), State) || !TestEqual(TEXT("(the first card - a shut second card)"), State->Knowing.PendingPetitionRow, CosmicRows::PetitionPressureClaim))
		{
			return false;
		}
		State->Civs[0].Stockpile.Insight = 300.f;
		TestTrue(TEXT("(the discernment is answered - a shut second card)"), Role->AnswerPetition(1).Outcome == ECosmicPetitionOutcome::Succeeded);
		TestTrue(TEXT("A second card whose conditions fail is not put"), State->Knowing.PendingPetitionRow.IsNone());
	}

	// Each card holds a choice that costs nothing, has no gate, no effect and no next card (the cards are never blocked).
	for (const FEncounterDef* Card : { ClaimCard, MeasuredCard })
	{
		bool bFree = false;
		for (const FEncounterChoice& Choice : Card->Choices)
		{
			const FResourcePool Cost = CosmicPetitions::CostOf(Choice);
			const bool bPriced = Cost.Energy > 0.f || Cost.Matter > 0.f || Cost.Insight > 0.f || Cost.Influence > 0.f || Cost.Causality > 0.f;
			bFree |= !bPriced && Choice.OnSuccess.IsEmpty() && Choice.NextEncounter.IsNone();
		}
		TestTrue(TEXT("A pressure card holds a free choice with no effect"), bFree);
	}
	return true;
}

// The vigil's draw row (B4 header section 2.8, [B-4d2] - the game's event_claim_eased): it eases a claim on a system of a
// people that holds the vigil - its line, then the claim's resolution line -, never one of a people without it (an AI
// people holds no vigil); no grace row waits on an edict.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimVigilEaseTest, "Cosmic_Ascension.Sim.VigilEase", CosmicTests::Flags)
bool FCosmicSimVigilEaseTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPressureCardTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FCosmicEventDef* Eased = Original.FindEvent(CosmicRows::EventClaimEased);
	if (!TestNotNull(TEXT("(the game has the vigil's row)"), Eased))
	{
		return false;
	}
	const FGameplayTag Claim = Tag(TEXT("Hazard.Claim"));
	FName ClaimRow;
	if (!TestNotNull(TEXT("(the game has the claim's row)"), Original.FindHazardByTag(Claim, &ClaimRow)))
	{
		return false;
	}
	// The board with the vigil's row alone, a slot every tick for every people and a chance that makes the row's share 1.
	auto Lay = [&](int32 SystemIndex, bool bVigil) -> FGalaxyState*
	{
		FGalaxyState* State = LayPressureBoard(*Sim, Original, CosmicTests::NewTable(FEncounterDef::StaticStruct()));
		if (!State)
		{
			return nullptr;
		}
		UDataTable* Events = CosmicTests::NewTable(FCosmicEventDef::StaticStruct());
		Events->AddRow(CosmicRows::EventClaimEased, *Eased);
		FCosmicRulesDef Rules = Sim->GetRules();
		Rules.EventSlotTicks = 1;
		Rules.EventChancePerSlot = 1.f / Eased->BaseWeight;
		FCosmicDataTables Tables = CosmicTests::WithRules(Sim->GetTables(), Rules);
		Tables.Events = Events;
		Sim->SetTables(Tables);
		State->Systems[SystemIndex].ActiveHazard = Claim;
		State->Systems[SystemIndex].HazardTicksRemaining = 300;
		const int32 Owner = Sim->GetStateIndex().FindCiv(*State, State->Systems[SystemIndex].OwnerCivID);
		if (bVigil && State->Civs.IsValidIndex(Owner))
		{
			State->Civs[Owner].ActiveEdicts = { PressVigil };
		}
		Sim->StepOnce();
		return State;
	};
	auto LineAt = [](const FGalaxyState& State, FName Row)
	{
		for (int32 Index = 0; Index < State.Log.Num(); ++Index)
		{
			if (State.Log[Index].Tick == State.Tick && State.Log[Index].SourceRow == Row)
			{
				return Index;
			}
		}
		return int32(INDEX_NONE);
	};
	{
		FGalaxyState* State = Lay(PressNear, true);
		if (!TestNotNull(TEXT("(the board - the vigil held)"), State))
		{
			return false;
		}
		TestFalse(TEXT("A claim of a people that holds the vigil is eased"), State->Systems[PressNear].ActiveHazard.IsValid());
		const int32 RowLine = LineAt(*State, CosmicRows::EventClaimEased);
		const int32 Resolution = LineAt(*State, ClaimRow);
		TestTrue(TEXT("The row tells its line"), RowLine != INDEX_NONE);
		TestTrue(TEXT("and the claim's resolution line follows it"), Resolution != INDEX_NONE && Resolution > RowLine);
	}
	{
		FGalaxyState* State = Lay(PressNear, false);
		if (!TestNotNull(TEXT("(the board - no vigil)"), State))
		{
			return false;
		}
		TestTrue(TEXT("A claim of a people without the vigil stands"), State->Systems[PressNear].ActiveHazard == Claim);
	}
	{
		FGalaxyState* State = Lay(4, false);
		if (!TestNotNull(TEXT("(the board - an AI people)"), State))
		{
			return false;
		}
		TestTrue(TEXT("A claim of an AI people (no vigil) stands"), State->Systems[4].ActiveHazard == Claim);
	}
	// No grace row of the game waits on an edict (grace is never drawn by one).
	const FGameplayTag Grace = Tag(TEXT("Event.Grace"));
	bool bGraceOnEdict = false;
	for (const FName Row : Original.GetEventRowNames())
	{
		const FCosmicEventDef* Def = Original.FindEvent(Row);
		bGraceOnEdict |= Def && Def->Category.MatchesTag(Grace)
			&& Def->Conditions.ContainsByPredicate([](const FCosmicCondition& Condition) { return Condition.Type == EConditionType::CivHasEdict; });
	}
	TestFalse(TEXT("No grace row waits on an edict"), bGraceOnEdict);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

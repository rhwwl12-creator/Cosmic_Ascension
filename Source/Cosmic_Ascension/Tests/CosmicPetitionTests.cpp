// Cosmic Ascension - automation tests of the petition cards (MVP item 2). Design: guideline.md section 5.9, 5.10, 9.2;
// docs/60_DECISIONS/headers/A2_petition_cards.md.

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
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"

#include <initializer_list>

namespace CosmicPetitionTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
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

	FCosmicEffect Eff(EEffectType Type, const TCHAR* TagName = nullptr, const TCHAR* Key = nullptr, float Value = 0.f)
	{
		FCosmicEffect Effect;
		Effect.Type = Type;
		Effect.Tag = TagName ? Tag(TagName) : FGameplayTag();
		Effect.Key = Key ? FName(Key) : FName();
		Effect.Value = Value;
		return Effect;
	}

	FEncounterChoice Choice(const TCHAR* Success, std::initializer_list<FCosmicEffect> OnSuccess = {}, ECosmicCheckStat Check = ECosmicCheckStat::NoCheck,
		int32 Difficulty = 0, std::initializer_list<FCosmicEffect> OnFailure = {}, const TCHAR* Failure = TEXT(""))
	{
		FEncounterChoice Out;
		Out.Label = FText::AsCultureInvariant(TEXT("choice"));
		Out.CheckStat = Check;
		Out.Difficulty = Difficulty;
		Out.OnSuccess = OnSuccess;
		Out.OnFailure = OnFailure;
		Out.SuccessText = FText::AsCultureInvariant(Success);
		Out.FailureText = FText::AsCultureInvariant(Failure);
		return Out;
	}

	FEncounterDef Card(ECosmicEventTrigger Trigger, ECosmicEventSubject Subject, std::initializer_list<FCosmicCondition> Conditions, TArray<FEncounterChoice> Choices,
		float Weight = 1.f)
	{
		FEncounterDef Row;
		Row.Title = FText::AsCultureInvariant(TEXT("card"));
		Row.Body = FText::AsCultureInvariant(TEXT("{Civ} asks."));
		Row.Trigger = Trigger;
		Row.Subject = Subject;
		Row.Conditions = Conditions;
		Row.Conditions.Insert(Cond(EConditionType::PlayerRoleIs, nullptr, TEXT("Ruler")), 0);
		Row.Choices = MoveTemp(Choices);
		Row.BaseWeight = Trigger == ECosmicEventTrigger::Named ? 0.f : Weight;
		return Row;
	}

	// The game's tables with DT_Encounters replaced (nullptr = no table), the projects and feats out of the way, and
	// PetitionPeriodTicks set.
	FCosmicDataTables TablesWith(const FCosmicDataTables& Original, UDataTable* Encounters, int32 PeriodTicks = 600)
	{
		FCosmicRulesDef Rules = Original.FindRules() ? *Original.FindRules() : FCosmicRulesDef();
		Rules.PetitionPeriodTicks = PeriodTicks;
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));
		Tables.Encounters = Encounters;
		return Tables;
	}

	struct FNamedCard
	{
		const TCHAR* Name;
		FEncounterDef Row;
	};

	UDataTable* CardTable(std::initializer_list<FNamedCard> Rows)
	{
		UDataTable* Table = CosmicTests::NewTable(FEncounterDef::StaticStruct());
		for (const FNamedCard& Card : Rows)
		{
			Table->AddRow(FName(Card.Name), Card.Row);
		}
		return Table;
	}

	// The number the next GetFraction of a stream gives, read as a roll 0..5, without moving the stream.
	int32 NextRoll(const FRandomStream& Stream)
	{
		FRandomStream Copy = Stream;
		return FMath::Min(static_cast<int32>(Copy.GetFraction() * 6.f), 5);
	}

	// The seed a stream holds after one more GetFraction.
	int32 SeedAfterOneDraw(const FRandomStream& Stream)
	{
		FRandomStream Copy = Stream;
		Copy.GetFraction();
		return Copy.GetCurrentSeed();
	}

	void Put(FGalaxyState& State, FName Row, FName SystemID = NAME_None)
	{
		State.Knowing.PendingPetitionRow = Row;
		State.Knowing.PendingPetitionSystemID = SystemID;
	}
}

// Which card the knowing stage puts before the ruler, and when: the edge of a Standing row, the periodic slot of the
// Lottery rows, the waiting card that blocks, the derived pick (section 5.10). And the presentation changes no world.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimPetitionPresentTest, "Cosmic_Ascension.Sim.PetitionPresent", CosmicTests::Flags)
bool FCosmicSimPetitionPresentTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPetitionTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FName Tick30(TEXT("petition_test_tick"));
	const FName Low(TEXT("petition_test_low"));
	const FName Place(TEXT("petition_test_place"));
	const FName LotA(TEXT("petition_test_lot_a"));
	const FName LotB(TEXT("petition_test_lot_b"));
	const TArray<FEncounterChoice> Two = { Choice(TEXT("a.")), Choice(TEXT("b.")) };

	// 1. Through the tick: a row whose condition comes to hold at tick 30 is put in tick 30, not before, and not again
	// while it holds (the head of the tick reads the tick before it advances).
	{
		Sim->SetTables(TablesWith(Original, CardTable({ { TEXT("petition_test_tick"), Card(ECosmicEventTrigger::Standing, ECosmicEventSubject::Civilization,
			{ Cond(EConditionType::GalaxyTickAtLeast, nullptr, nullptr, 30.f) }, Two) } })));
		Sim->NewGalaxy(42, 5, 3);
		const FGalaxyState& State = Sim->GetState();
		bool bEarly = false;
		for (int32 Step = 1; Step < 30; ++Step)
		{
			Sim->StepOnce();
			bEarly |= !State.Knowing.PendingPetitionRow.IsNone();
		}
		TestFalse(TEXT("No card before the tick its condition comes to hold in"), bEarly);
		Sim->StepOnce();
		TestEqual(TEXT("The card arrives in tick 30"), State.Knowing.PendingPetitionRow, Tick30);
		TestTrue(TEXT("A Civilization card names no system"), State.Knowing.PendingPetitionSystemID.IsNone());
		CosmicPetitions::Clear(Sim->GetMutableState());
		Sim->RunHeadless(20);
		TestTrue(TEXT("A condition that keeps holding puts no card again"), State.Knowing.PendingPetitionRow.IsNone());
	}

	// 2. Present itself over the head it is given.
	Sim->SetTables(TablesWith(Original, CardTable({
		{ TEXT("petition_test_low"), Card(ECosmicEventTrigger::Standing, ECosmicEventSubject::Civilization, { Cond(EConditionType::CivStabilityBelow, nullptr, nullptr, 0.5f) }, Two) },
		{ TEXT("petition_test_place"), Card(ECosmicEventTrigger::Standing, ECosmicEventSubject::SettledSystem, { Cond(EConditionType::SystemHasHazard, TEXT("Hazard.Plague")) }, Two) },
		{ TEXT("petition_test_lot_a"), Card(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, {}, Two, 1.f) },
		{ TEXT("petition_test_lot_b"), Card(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, {}, Two, 1.f) },
	}), 50));
	Sim->NewGalaxy(42, 5, 3);
	FGalaxyState& State = Sim->GetMutableState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicDataTables& Tables = Sim->GetTables();
	const FCosmicRulesDef& Rules = Sim->GetRules();
	const int32 Ruled = CosmicPetitions::RuledCivIndexOf(State, Index);
	if (!TestTrue(TEXT("The first life rules a people"), Ruled != INDEX_NONE))
	{
		return false;
	}
	const int32 Home = Index.FindSystem(State, State.Civs[Ruled].HomeSystemID);
	State.Tick = 7;
	State.Civs[Ruled].Stability = 0.4f;
	const TArray<FCosmicPetitionUnit> Now = CosmicPetitions::StandingHeld(State, Index, Tables, Rules);
	TestTrue(TEXT("StandingHeld lists the low-stability row on the people's capital"), Now.Contains(FCosmicPetitionUnit{ Low, Home }));
	TestFalse(TEXT("and no unit of a row whose condition does not hold"), Now.ContainsByPredicate([&Place](const FCosmicPetitionUnit& Unit) { return Unit.Row == Place; }));
	TestFalse(TEXT("A condition that held at the head of the tick puts no card"), CosmicPetitions::Present(State, Index, Tables, Rules, Now));
	TestTrue(TEXT("(and nothing waits)"), State.Knowing.PendingPetitionRow.IsNone());
	TestTrue(TEXT("A condition that came to hold in the tick puts its card"), CosmicPetitions::Present(State, Index, Tables, Rules, {}));
	TestEqual(TEXT("(the low-stability card)"), State.Knowing.PendingPetitionRow, Low);
	TestTrue(TEXT("StandingHeld is empty while a card waits"), CosmicPetitions::StandingHeld(State, Index, Tables, Rules).IsEmpty());
	State.Systems[Home].ActiveHazard = Tag(TEXT("Hazard.Plague"));
	TestFalse(TEXT("A waiting card blocks every other card"), CosmicPetitions::Present(State, Index, Tables, Rules, {}));
	TestEqual(TEXT("(the waiting card stays)"), State.Knowing.PendingPetitionRow, Low);
	{
		// The periodic slot too: a Lottery row does not replace the waiting card.
		const int64 TickBefore = State.Tick;
		const int64 BornBefore = State.Calling.BornTick;
		State.Calling.BornTick = 0;
		State.Tick = 50;
		TestFalse(TEXT("A waiting card blocks the periodic slot"), CosmicPetitions::Present(State, Index, Tables, Rules, {}));
		TestEqual(TEXT("(the waiting card still stays)"), State.Knowing.PendingPetitionRow, Low);
		State.Tick = TickBefore;
		State.Calling.BornTick = BornBefore;
	}

	// A card about a place names its system; the pick among two new rows is the derived one, the same every time.
	CosmicPetitions::Clear(State);
	State.Civs[Ruled].Stability = 0.75f;
	TestTrue(TEXT("A hazard that came on a settled system of the people puts the card about it"), CosmicPetitions::Present(State, Index, Tables, Rules, {}));
	TestEqual(TEXT("(the place card)"), State.Knowing.PendingPetitionRow, Place);
	TestEqual(TEXT("(about that system)"), State.Knowing.PendingPetitionSystemID, State.Systems[Home].SystemID);
	TestTrue(TEXT("The card stands while its conditions hold"), CosmicPetitions::StillStands(State, Index, Tables, Rules));
	State.Systems[Home].ActiveHazard = FGameplayTag();
	TestFalse(TEXT("and no longer once they do not"), CosmicPetitions::StillStands(State, Index, Tables, Rules));
	State.Systems[Home].ActiveHazard = Tag(TEXT("Hazard.Plague"));
	CosmicPetitions::Clear(State);
	State.Civs[Ruled].Stability = 0.4f;
	FGalaxyState Twin = State;
	CosmicPetitions::Present(State, Index, Tables, Rules, {});
	CosmicPetitions::Present(Twin, Index, Tables, Rules, {});
	TestTrue(TEXT("Two new rows: one is put"), State.Knowing.PendingPetitionRow == Low || State.Knowing.PendingPetitionRow == Place);
	TestEqual(TEXT("and the same state picks the same one"), Twin.Knowing.PendingPetitionRow, State.Knowing.PendingPetitionRow);
	State.Systems[Home].ActiveHazard = FGameplayTag();

	// The periodic slot of the life: every PetitionPeriodTicks from the birth, Lottery rows only, not the last choices.
	CosmicPetitions::Clear(State);
	State.Civs[Ruled].Stability = 0.75f;
	State.Calling.BornTick = 0;
	State.Tick = 49;
	TestFalse(TEXT("No Lottery card outside the periodic slot"), CosmicPetitions::Present(State, Index, Tables, Rules, {}));
	State.Tick = 50;
	TestTrue(TEXT("A Lottery card in the periodic slot"), CosmicPetitions::Present(State, Index, Tables, Rules, {}));
	TestTrue(TEXT("(one of the two Lottery rows)"), State.Knowing.PendingPetitionRow == LotA || State.Knowing.PendingPetitionRow == LotB);
	CosmicPetitions::Clear(State);
	State.Calling.BornTick = 10;
	TestFalse(TEXT("The slot counts from the birth of the life"), CosmicPetitions::Present(State, Index, Tables, Rules, {}));
	State.Tick = 60;
	TestTrue(TEXT("(fifty ticks after a birth at tick 10)"), CosmicPetitions::Present(State, Index, Tables, Rules, {}));
	CosmicPetitions::Clear(State);
	State.Tick = 10;
	TestFalse(TEXT("The birth tick itself is no slot"), CosmicPetitions::Present(State, Index, Tables, Rules, {}));
	State.Tick = 60;
	State.Calling.Lives.Last().LastChoiceRows = { LotA };
	CosmicPetitions::Present(State, Index, Tables, Rules, {});
	TestEqual(TEXT("A row among the last choices of the life is not put again"), State.Knowing.PendingPetitionRow, LotB);
	CosmicPetitions::Clear(State);
	State.Calling.Lives.Last().LastChoiceRows = { LotA, LotB };
	TestTrue(TEXT("With every Lottery row among them the slot still puts one (MVP item 8)"), CosmicPetitions::Present(State, Index, Tables, Rules, {}));
	TestEqual(TEXT("not the most recent one"), State.Knowing.PendingPetitionRow, LotA);
	CosmicPetitions::Clear(State);
	State.Calling.Lives.Last().LastChoiceRows = { LotB, LotA };
	CosmicPetitions::Present(State, Index, Tables, Rules, {});
	TestEqual(TEXT("(the other way round, the other one)"), State.Knowing.PendingPetitionRow, LotB);
	CosmicPetitions::Clear(State);
	State.Calling.Lives.Last().LastChoiceRows = { LotA, LotB, Low };
	CosmicPetitions::Present(State, Index, Tables, Rules, {});
	TestEqual(TEXT("A Standing answer after them does not count as the most recent periodic card"), State.Knowing.PendingPetitionRow, LotA);
	CosmicPetitions::Clear(State);
	State.Calling.Lives.Last().LastChoiceRows.Reset();

	// Only an embodied ruler gets cards.
	State.Player.Role = EPlayerRole::Wanderer;
	TestFalse(TEXT("A wanderer gets no petition"), CosmicPetitions::Present(State, Index, Tables, Rules, {}));
	TestEqual(TEXT("and has no ruled people"), CosmicPetitions::RuledCivIndexOf(State, Index), static_cast<int32>(INDEX_NONE));
	State.Player.Role = EPlayerRole::Ruler;
	State.Calling.LifeState = ELifeState::Interregnum;
	TestFalse(TEXT("Neither does the calling between vessels"), CosmicPetitions::Present(State, Index, Tables, Rules, {}));
	State.Calling.LifeState = ELifeState::Embodied;

	// A single periodic card, answered last: the fallback passes it over too - the slot puts nothing rather than the same card
	// again.
	{
		Sim->SetTables(TablesWith(Original, CardTable({
			{ TEXT("petition_test_lot_a"), Card(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, {}, Two, 1.f) },
		}), 50));
		Sim->NewGalaxy(42, 5, 3);
		FGalaxyState& One = Sim->GetMutableState();
		One.Calling.BornTick = 0;
		One.Tick = 50;
		One.Calling.Lives.Last().LastChoiceRows = { LotA };
		TestFalse(TEXT("A single periodic card answered last is not put again"), CosmicPetitions::Present(One, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), {}));
		One.Calling.Lives.Last().LastChoiceRows.Reset();
		TestTrue(TEXT("(and is put when it was not)"), CosmicPetitions::Present(One, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), {}));
	}

	// 3. The cards change no world: the same board with and without the game's cards has the same world hash, and the
	// presentation draws nothing from the sim stream.
	{
		Sim->SetTables(TablesWith(Original, Original.Encounters));
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		Sim->RunHeadless(700);
		const uint32 WorldWith = Sim->ComputeWorldHash();
		const uint32 FullWith = Sim->ComputeStateHash();
		const bool bCardWaits = !Sim->GetState().Knowing.PendingPetitionRow.IsNone();
		Sim->SetTables(TablesWith(Original, nullptr));
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		Sim->RunHeadless(700);
		TestTrue(TEXT("The game's cards put one before the unanswering ruler"), Original.Encounters != nullptr && bCardWaits);
		TestEqual(TEXT("The world hash of a board with cards is the world hash without them"), static_cast<int64>(WorldWith), static_cast<int64>(Sim->ComputeWorldHash()));
		TestNotEqual(TEXT("(the waiting card is in the full hash)"), static_cast<int64>(FullWith), static_cast<int64>(Sim->ComputeStateHash()));
	}
	return true;
}

// The answer (section 5.9, 5.10, 9.2): the refusals that draw nothing, the one draw of an answer taken, the cost, the
// check at its bound, the effects and the line, the withdrawn card, the chain, the remembered rows, the deposition.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimPetitionAnswerTest, "Cosmic_Ascension.Sim.PetitionAnswer", CosmicTests::Flags)
bool FCosmicSimPetitionAnswerTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPetitionTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	AddExpectedError(TEXT("AnswerPetition: the waiting card petition_test_nowhere is no row"), EAutomationExpectedErrorFlags::Contains, 1);
	AddExpectedError(TEXT("AnswerPetition: petition_test_ask has no choice 3"), EAutomationExpectedErrorFlags::Contains, 1);
	AddExpectedError(TEXT("AnswerPetition: the stockpile cannot pay choice 0"), EAutomationExpectedErrorFlags::Contains, 1);

	const FName Ask(TEXT("petition_test_ask"));
	const FName Next(TEXT("petition_test_next"));
	const FName Place(TEXT("petition_test_place"));
	// The ask row: a paid choice without a check, a Stability check against 8, and a choice that chains the next row.
	FEncounterDef AskRow = Card(ECosmicEventTrigger::Standing, ECosmicEventSubject::Civilization, {}, {
		Choice(TEXT("{Civ} paid."), { Eff(EEffectType::AddResource, nullptr, TEXT("Matter"), 5.f) }),
		Choice(TEXT("held."), { Eff(EEffectType::ChangeStability, nullptr, nullptr, 0.1f) }, ECosmicCheckStat::Stability, 8,
			{ Eff(EEffectType::ChangeStability, nullptr, nullptr, -0.1f) }, TEXT("fell.")),
		Choice(TEXT("on.")) });
	AskRow.Choices[0].Cost.Energy = 30.f;
	AskRow.Choices[2].NextEncounter = Next;
	Sim->SetTables(TablesWith(Original, CardTable({
		{ TEXT("petition_test_ask"), AskRow },
		{ TEXT("petition_test_next"), Card(ECosmicEventTrigger::Named, ECosmicEventSubject::Civilization, {}, { Choice(TEXT("x.")), Choice(TEXT("y.")) }) },
		{ TEXT("petition_test_spot"), Card(ECosmicEventTrigger::Standing, ECosmicEventSubject::SettledSystem, {}, { Choice(TEXT("x.")), Choice(TEXT("y.")) }) },
		{ TEXT("petition_test_place"), Card(ECosmicEventTrigger::Standing, ECosmicEventSubject::SettledSystem, { Cond(EConditionType::SystemHasHazard, TEXT("Hazard.Plague")) }, {
			Choice(TEXT("{System} healed."), { Eff(EEffectType::ResolveHazard, TEXT("Hazard.Plague")) }),
			Choice(TEXT("never."), {}, ECosmicCheckStat::Body, 16, { Eff(EEffectType::DeposeRuler, TEXT("Reign.Deposed")) }, TEXT("{System} fell.")) }) },
	})));
	Sim->NewGalaxy(42, 5, 3);
	FGalaxyState& State = Sim->GetMutableState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const int32 Ruled = CosmicPetitions::RuledCivIndexOf(State, Index);
	if (!TestTrue(TEXT("The first life rules a people"), Ruled != INDEX_NONE))
	{
		return false;
	}
	const int32 Home = Index.FindSystem(State, State.Civs[Ruled].HomeSystemID);
	State.Civs[Ruled].Stockpile = FResourcePool();
	State.Civs[Ruled].Stockpile.Energy = 100.f;

	// Refusals: nothing drawn, nothing changed.
	auto Refused = [&](const TCHAR* What, int32 ChoiceIndex)
	{
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		const uint32 Hash = Sim->ComputeStateHash();
		const FCosmicPetitionAnswer Answer = Role->AnswerPetition(ChoiceIndex);
		TestTrue(*FString::Printf(TEXT("%s is refused"), What), Answer.Outcome == ECosmicPetitionOutcome::Refused && Answer.Roll == INDEX_NONE);
		TestTrue(*FString::Printf(TEXT("%s draws nothing and changes nothing"), What), Sim->GetRng().GetCurrentSeed() == Seed && Sim->ComputeStateHash() == Hash);
	};
	Refused(TEXT("An answer with no card waiting"), 0);
	Put(State, FName(TEXT("petition_test_nowhere")));
	Refused(TEXT("An answer to a card DT_Encounters lacks"), 0);
	Put(State, Ask);
	Refused(TEXT("A choice out of range"), 3);
	State.Civs[Ruled].Stockpile.Energy = 29.f;
	Refused(TEXT("A choice the stockpile cannot pay"), 0);
	State.Civs[Ruled].Stockpile.Energy = 100.f;
	State.Player.Role = EPlayerRole::Wanderer;
	Refused(TEXT("An answer of a wanderer"), 0);
	State.Player.Role = EPlayerRole::Ruler;

	// An answer taken: one draw, the cost, the effects, the line, the life remembers the row, the card goes.
	{
		const int32 Expected = SeedAfterOneDraw(Sim->GetRng());
		const int32 LinesBefore = State.Log.Num();
		const FCosmicPetitionAnswer Answer = Role->AnswerPetition(0);
		TestTrue(TEXT("A choice without a check succeeds"), Answer.Outcome == ECosmicPetitionOutcome::Succeeded);
		TestEqual(TEXT("An answer taken draws one number of the sim stream"), Sim->GetRng().GetCurrentSeed(), Expected);
		TestEqual(TEXT("and the state holds the stream"), State.RngState, Expected);
		TestEqual(TEXT("The cost is paid from the stockpile"), State.Civs[Ruled].Stockpile.Energy, 70.f);
		TestEqual(TEXT("The effects of the outcome run"), State.Civs[Ruled].Stockpile.Matter, 5.f);
		TestTrue(TEXT("The outcome line is told"), State.Log.Num() == LinesBefore + 1);
		if (State.Log.Num() == LinesBefore + 1)
		{
			const FCosmicLogEntry& Line = State.Log.Last();
			TestTrue(TEXT("about the people, of the card's row, in Log.Civ with Severity 1"), Line.FocusCivID == State.Civs[Ruled].CivID && Line.FocusSystemID.IsNone()
				&& Line.SourceRow == Ask && Line.Category == Tag(TEXT("Log.Civ")) && Line.Severity == 1);
			TestEqual(TEXT("with the card's arguments"), Line.Message.ToString(), State.Civs[Ruled].CivName.ToString() + TEXT(" paid."));
			TestEqual(TEXT("The answer returns the line"), Answer.Line.ToString(), Line.Message.ToString());
		}
		TestTrue(TEXT("The card is taken away"), State.Knowing.PendingPetitionRow.IsNone());
		TestTrue(TEXT("The life remembers the row"), State.Calling.Lives.Last().LastChoiceRows == TArray<FName>{ Ask });
	}

	// The check at its bound: Stability 0..1 reads 0..10; value + roll = Difficulty passes, one less fails.
	for (const bool bPass : { true, false })
	{
		Put(State, Ask);
		const int32 Roll = NextRoll(Sim->GetRng());
		const int32 Value = 8 - Roll - (bPass ? 0 : 1);
		State.Civs[Ruled].Stability = static_cast<float>(Value) / 10.f;
		TestEqual(*FString::Printf(TEXT("Stability %d/10 is checked as %d"), Value, Value), CosmicPetitions::CheckValueOf(State, Ruled, ECosmicCheckStat::Stability), Value);
		const float StabilityBefore = State.Civs[Ruled].Stability;
		const FCosmicPetitionAnswer Answer = Role->AnswerPetition(1);
		TestEqual(TEXT("The answer reports the number drawn"), Answer.Roll, Roll);
		TestTrue(bPass ? TEXT("Value + roll = Difficulty passes") : TEXT("Value + roll = Difficulty - 1 fails"),
			Answer.Outcome == (bPass ? ECosmicPetitionOutcome::Succeeded : ECosmicPetitionOutcome::Failed));
		TestTrue(TEXT("and runs the effects of its outcome"), FMath::IsNearlyEqual(State.Civs[Ruled].Stability, StabilityBefore + (bPass ? 0.1f : -0.1f), 1.e-5f));
		TestEqual(TEXT("and tells the line of its outcome"), Answer.Line.ToString(), FString(bPass ? TEXT("held.") : TEXT("fell.")));
	}
	State.Civs[Ruled].Stability = 0.7f;
	TestEqual(TEXT("A Stability of 0.7 reads 7, not 6"), CosmicPetitions::CheckValueOf(State, Ruled, ECosmicCheckStat::Stability), 7);
	// As the effects reach it: 0.45 - 0.05 is a little below 0.4 as a float (0.39999998), and still reads 4.
	State.Civs[Ruled].Stability = 0.45f;
	State.Civs[Ruled].Stability -= 0.05f;
	TestEqual(TEXT("A Stability of 0.45 - 0.05 reads 4, not 3"), CosmicPetitions::CheckValueOf(State, Ruled, ECosmicCheckStat::Stability), 4);
	State.Player.Body = 10;
	FEncounterChoice Impossible = Choice(TEXT("."), {}, ECosmicCheckStat::Body, 16);
	TestFalse(TEXT("Difficulty 16 fails the highest stat with the highest roll"), CosmicPetitions::Passes(State, Ruled, Impossible, 5));
	TestEqual(TEXT("and no roll passes it"), CosmicPetitions::PassingRollsOf(State, Ruled, Impossible), 0);
	State.Player.Body = 5;
	Impossible.Difficulty = 8;
	TestEqual(TEXT("Body 5 against 8 passes with three rolls of six"), CosmicPetitions::PassingRollsOf(State, Ruled, Impossible), 3);
	TestEqual(TEXT("A choice without a check passes with all six"), CosmicPetitions::PassingRollsOf(State, Ruled, Choice(TEXT("."))), 6);

	// The life remembers the last three rows only.
	TestTrue(TEXT("Three answers remembered"), State.Calling.Lives.Last().LastChoiceRows.Num() == 3);
	CosmicPetitions::Remember(State, Next);
	TestTrue(TEXT("A fourth pushes the oldest out"), State.Calling.Lives.Last().LastChoiceRows == TArray<FName>({ Ask, Ask, Next }));

	// The chain: the next row becomes the card, by name, without a draw of its own.
	{
		Put(State, Ask);
		const int32 Expected = SeedAfterOneDraw(Sim->GetRng());
		Role->AnswerPetition(2);
		TestEqual(TEXT("A choice with a next card puts it"), State.Knowing.PendingPetitionRow, Next);
		TestEqual(TEXT("(one draw for the answer, none for the next card)"), Sim->GetRng().GetCurrentSeed(), Expected);
	}

	// A withdrawn card: its subject no longer stands. One draw; no cost, no effect, no line.
	{
		State.Systems[Home].ActiveHazard = FGameplayTag();
		Put(State, Place, State.Systems[Home].SystemID);
		const int32 Expected = SeedAfterOneDraw(Sim->GetRng());
		const int32 LinesBefore = State.Log.Num();
		const FResourcePool Stockpile = State.Civs[Ruled].Stockpile;
		const FCosmicPetitionAnswer Answer = Role->AnswerPetition(0);
		TestTrue(TEXT("A card whose conditions no longer hold is withdrawn"), Answer.Outcome == ECosmicPetitionOutcome::Withdrawn);
		TestEqual(TEXT("(its answer still drew one number)"), Sim->GetRng().GetCurrentSeed(), Expected);
		TestTrue(TEXT("and nothing was paid or told"), State.Log.Num() == LinesBefore && State.Civs[Ruled].Stockpile.Energy == Stockpile.Energy);
		TestTrue(TEXT("and the card is gone"), State.Knowing.PendingPetitionRow.IsNone());
	}

	// A card about a place stands only while the place is a settled system of the ruled people, whatever its conditions.
	{
		const FName Spot(TEXT("petition_test_spot"));
		const FCosmicDataTables& Tables = Sim->GetTables();
		Put(State, Spot, State.Systems[Home].SystemID);
		TestTrue(TEXT("A card about a settled system of the people stands"), CosmicPetitions::StillStands(State, Index, Tables, Sim->GetRules()));
		const int32 Other = State.Systems.IndexOfByPredicate([](const FStarSystemState& System) { return System.OwnerCivID.IsNone() && !CosmicState::IsSanctuary(System.SystemID); });
		if (TestTrue(TEXT("(the board has a wild system)"), Other != INDEX_NONE))
		{
			Put(State, Spot, State.Systems[Other].SystemID);
			TestFalse(TEXT("A card about a system the people does not hold does not stand"), CosmicPetitions::StillStands(State, Index, Tables, Sim->GetRules()));
		}
		Put(State, Spot);
		TestFalse(TEXT("nor one about no system"), CosmicPetitions::StillStands(State, Index, Tables, Sim->GetRules()));
		CosmicPetitions::Clear(State);
	}

	// A card about a place: the answer resolves its hazard and tells about the place.
	{
		State.Systems[Home].ActiveHazard = Tag(TEXT("Hazard.Plague"));
		State.Systems[Home].HazardTicksRemaining = 100;
		Put(State, Place, State.Systems[Home].SystemID);
		const FCosmicPetitionAnswer Answer = Role->AnswerPetition(0);
		TestTrue(TEXT("The place card's answer succeeds"), Answer.Outcome == ECosmicPetitionOutcome::Succeeded);
		TestFalse(TEXT("and resolves the hazard of its system"), State.Systems[Home].ActiveHazard.IsValid());
		TestTrue(TEXT("Its line is about the place"), State.Log.ContainsByPredicate([&](const FCosmicLogEntry& Line)
		{
			return Line.SourceRow == Place && Line.FocusSystemID == State.Systems[Home].SystemID;
		}));
	}

	// The deposition of a failed answer is recorded, and the next tick's role stage deposes and takes any card away.
	{
		State.Systems[Home].ActiveHazard = Tag(TEXT("Hazard.Plague"));
		State.Systems[Home].HazardTicksRemaining = 100;
		Put(State, Place, State.Systems[Home].SystemID);
		const FCosmicPetitionAnswer Answer = Role->AnswerPetition(1);
		TestTrue(TEXT("An impossible check fails"), Answer.Outcome == ECosmicPetitionOutcome::Failed);
		TestTrue(TEXT("and its deposition is recorded"), State.Calling.PendingEffect == Tag(TEXT("Effect.DeposeRuler")));
		TestEqual(TEXT("(still a ruler until the role stage)"), State.Player.Role, EPlayerRole::Ruler);
		Put(State, Ask);
		Sim->StepOnce();
		TestEqual(TEXT("The role stage deposes the ruler"), State.Player.Role, EPlayerRole::Wanderer);
		TestTrue(TEXT("and the role change takes the waiting card away"), State.Knowing.PendingPetitionRow.IsNone() && State.Knowing.PendingPetitionSystemID.IsNone());
	}

	// A new life starts with no card.
	Put(State, Ask);
	Role->Install(State.Civs[Ruled].HomeSystemID, State.Civs[Ruled].CivID, EPlayerRole::Ruler);
	TestTrue(TEXT("Install takes a waiting card away"), State.Knowing.PendingPetitionRow.IsNone());

	// The research focus of the founding petition: the field takes its share, the others the rest, the sum stays 1.
	{
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = Ruled;
		const TMap<FName, float> Before = State.Civs[Ruled].ResearchFocus;
		TestFalse(TEXT("FocusResearch refuses a Key that is no field"), UCosmicEventSubsystem::Apply(Eff(EEffectType::FocusResearch, nullptr, TEXT("Alchemy"), 0.5f), Ctx));
		TestFalse(TEXT("and a share of 0"), UCosmicEventSubsystem::Apply(Eff(EEffectType::FocusResearch, nullptr, TEXT("Society"), 0.f), Ctx));
		TestFalse(TEXT("and a share above 1"), UCosmicEventSubsystem::Apply(Eff(EEffectType::FocusResearch, nullptr, TEXT("Society"), 1.5f), Ctx));
		TestTrue(TEXT("(a refused focus leaves the focus)"), State.Civs[Ruled].ResearchFocus.OrderIndependentCompareEqual(Before));
		TestTrue(TEXT("FocusResearch applies a field and a share"), UCosmicEventSubsystem::Apply(Eff(EEffectType::FocusResearch, nullptr, TEXT("Society"), 0.55f), Ctx));
		float Sum = 0.f;
		bool bOthers = true;
		for (const TPair<FName, float>& Field : State.Civs[Ruled].ResearchFocus)
		{
			Sum += Field.Value;
			bOthers &= Field.Key == FName(TEXT("Society")) ? FMath::IsNearlyEqual(Field.Value, 0.55f) : FMath::IsNearlyEqual(Field.Value, 0.15f, 1.e-5f);
		}
		TestTrue(TEXT("The field takes the share and the three others 0.15 each"), bOthers && State.Civs[Ruled].ResearchFocus.Num() == 4);
		TestTrue(TEXT("and the shares still sum to 1"), FMath::IsNearlyEqual(Sum, 1.f, 1.e-5f));
	}
	return true;
}

// A card that waits is saved: the answer after a load is the answer without it (section 16 #26).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSavePetitionTest, "Cosmic_Ascension.Sim.SavePetition", CosmicTests::Flags)
bool FCosmicSimSavePetitionTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	UCosmicSaveSubsystem* Save = Sim->GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>();
	ON_SCOPE_EXIT { Sim->NewGalaxy(42, 5, 3); };
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(40);
	const FName Waiting = Sim->GetState().Knowing.PendingPetitionRow;
	if (!TestEqual(TEXT("The game's founding petition waits after tick 30"), Waiting, FName(TEXT("petition_wise_heart"))))
	{
		return false;
	}
	const FGuid Galaxy = Sim->GetState().GalaxyID;
	if (!TestTrue(TEXT("(the galaxy with its card is saved)"), Save->SaveCurrentGalaxy()))
	{
		return false;
	}
	Role->AnswerPetition(1);
	Sim->RunHeadless(100);
	const uint32 Straight = Sim->ComputeStateHash();
	Sim->NewGalaxy(7, 5, 3);
	TestTrue(TEXT("The saved galaxy loads"), Save->LoadGalaxy(Galaxy));
	TestEqual(TEXT("with its card waiting"), Sim->GetState().Knowing.PendingPetitionRow, Waiting);
	Role->AnswerPetition(1);
	Sim->RunHeadless(100);
	TestEqual(TEXT("The same answer after the load gives the same galaxy"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Straight));
	Save->DeleteSave(Galaxy);
	return true;
}

// Many answers on the game's own cards: no error line, the shares and the stability in range, and the same inputs give
// the same galaxy.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimPetitionSoakTest, "Cosmic_Ascension.Sim.PetitionSoak", CosmicTests::Flags)
bool FCosmicSimPetitionSoakTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	ON_SCOPE_EXIT { Sim->NewGalaxy(42, 5, 3); };
	constexpr int32 Ticks = 6000;
	for (const int32 Seed : { 42, 7 })
	{
		TArray<uint32> Hashes;
		int32 Answers = 0;
		bool bInRange = true;
		for (int32 Pass = 0; Pass < 2; ++Pass)
		{
			Sim->NewGalaxy(Seed, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
			Answers = 0;
			for (int32 Step = 0; Step < Ticks; ++Step)
			{
				Sim->RunHeadless(1);
				const FGalaxyState& State = Sim->GetState();
				const FEncounterDef* Row = State.Knowing.PendingPetitionRow.IsNone() ? nullptr : Sim->GetTables().FindEncounter(State.Knowing.PendingPetitionRow);
				const int32 Ruled = CosmicPetitions::RuledCivIndexOf(State, Sim->GetStateIndex());
				if (!Row || Ruled == INDEX_NONE)
				{
					continue;
				}
				// The first payable choice from the one the tick names whose gate is open (B4: the answer refuses a gated one
				// with an error line - CosmicPetitions::ChoiceGateOf).
				const int32 CardSystem = CosmicPetitions::SubjectOf(State, Sim->GetStateIndex(), Ruled, Row->Subject, State.Knowing.PendingPetitionSystemID);
				const FName Other = State.Knowing.PendingPetitionRow == CosmicRows::PetitionFirstContact ? State.Knowing.PendingContactCivID : NAME_None;
				for (int32 Offset = 0; Offset < Row->Choices.Num(); ++Offset)
				{
					const int32 ChoiceIndex = (Step + Offset) % Row->Choices.Num();
					if (CosmicMegaprojects::CanPay(State.Civs[Ruled].Stockpile, CosmicPetitions::CostOf(Row->Choices[ChoiceIndex]))
						&& CosmicPetitions::ChoiceGateOf(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), Ruled, CardSystem, Other, Row->Choices[ChoiceIndex]).IsOpen())
					{
						Answers += Role->AnswerPetition(ChoiceIndex).Outcome != ECosmicPetitionOutcome::Refused ? 1 : 0;
						break;
					}
				}
				for (const FCivilizationState& Civ : Sim->GetState().Civs)
				{
					float Sum = 0.f;
					for (const TPair<FName, float>& Field : Civ.ResearchFocus)
					{
						Sum += Field.Value;
					}
					bInRange &= Civ.Stability >= 0.f && Civ.Stability <= 1.f && FMath::IsNearlyEqual(Sum, 1.f, 1.e-4f);
				}
			}
			Hashes.Add(Sim->ComputeStateHash());
		}
		TestTrue(*FString::Printf(TEXT("Seed %d: the ruler answered several cards (%d)"), Seed, Answers), Answers >= 3);
		TestTrue(*FString::Printf(TEXT("Seed %d: stability and research shares stay in range"), Seed), bInRange);
		TestEqual(*FString::Printf(TEXT("Seed %d: the same answers give the same galaxy"), Seed), static_cast<int64>(Hashes[0]), static_cast<int64>(Hashes[1]));
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

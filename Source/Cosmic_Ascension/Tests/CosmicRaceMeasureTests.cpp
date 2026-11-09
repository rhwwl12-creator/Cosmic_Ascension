// Cosmic Ascension - automation tests of the balance commandlet's race measures (B6b [B-6b3b]): the pure predicates of
// Tests/CosmicRaceMeasure.h one by one - the race check, the count of the fourth rung pinned to the brake's window, the first
// end of the fifth rung, a claim's end and its words, the galaxy laws as they stood when a hazard was placed - and the copy of
// the galaxy rows -FourthRungCap hands in (CosmicTests::GalaxiesWithCap). BrakeHeldOf is held together with the judgment in
// Sim.FourthRungBrake (Tests/CosmicUpperRungTests.cpp).
// Design: docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md section 5.1, 6; notes section 2.1.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Containers/ArrayView.h"
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Sim/CosmicAscent.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicEndings.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicRaceMeasure.h"
#include "Tests/CosmicTestHelpers.h"

#include <initializer_list>

namespace CosmicRaceMeasureTests
{
	// One people of a tick for the race check.
	CosmicRaceMeasure::FRungStep RaceStepOf(const TCHAR* CivID, EAscensionTier Before, EAscensionTier After, bool bRuled = false)
	{
		CosmicRaceMeasure::FRungStep Step;
		Step.CivID = FName(CivID);
		Step.Before = Before;
		Step.After = After;
		Step.bRuled = bRuled;
		return Step;
	}

	// A people of a hand-built state: its rung, its galaxy and whether the player rules it.
	struct FRacePeople
	{
		const TCHAR* CivID;
		EAscensionTier Tier;
		FName GalaxyRow;
		bool bRuled;
	};

	// A state with the peoples given, in Civs order (no system, no crossing).
	FGalaxyState RacePeoplesOf(std::initializer_list<FRacePeople> Peoples)
	{
		FGalaxyState State;
		for (const FRacePeople& People : Peoples)
		{
			FCivilizationState& Civ = State.Civs.AddDefaulted_GetRef();
			Civ.CivID = FName(People.CivID);
			Civ.Tier = People.Tier;
			Civ.GalaxyRow = People.GalaxyRow;
			Civ.bIsPlayerRuled = People.bRuled;
		}
		return State;
	}

	// A people of an end's state, with its rung.
	struct FRaceRung
	{
		const TCHAR* CivID;
		EAscensionTier Tier;
	};

	// The form of Sim.EndingOf's hand-built states: at Tick, one life (its peak Peak, ruling the first people) - embodied, the
	// personal rung the first - and the peoples with their rungs, in Civs order.
	FGalaxyState RaceEndStateOf(int64 Tick, EAscensionTier Peak, std::initializer_list<FRaceRung> Peoples)
	{
		FGalaxyState State;
		State.Tick = Tick;
		FLifeRecord& Life = State.Calling.Lives.AddDefaulted_GetRef();
		Life.LifeIndex = 1;
		Life.PeakTier = Peak;
		for (const FRaceRung& People : Peoples)
		{
			FCivilizationState& Civ = State.Civs.AddDefaulted_GetRef();
			Civ.CivID = FName(People.CivID);
			Civ.Tier = People.Tier;
		}
		if (!State.Civs.IsEmpty())
		{
			Life.RuledCivIDs.Add(State.Civs[0].CivID);
			Life.BirthCivID = State.Civs[0].CivID;
		}
		return State;
	}

	// The calling in the silence: the last life died at DiedTick and no tick of the leap is left.
	void PutInSilence(FGalaxyState& State, int64 DiedTick)
	{
		State.Calling.LifeState = ELifeState::Interregnum;
		State.Calling.PendingLeapTicks = 0;
		State.Calling.Lives.Last().DiedTick = DiedTick;
	}

	// The edicts of the laws' case: three galaxy laws, x (+40 on a plague's countdown), y (+7) and z (+100), and an edict of the
	// people's scope, own (+3).
	const FName RaceLawX(TEXT("edict_race_law_x"));
	const FName RaceLawY(TEXT("edict_race_law_y"));
	const FName RaceLawZ(TEXT("edict_race_law_z"));
	const FName RaceOwn(TEXT("edict_race_own"));

	UDataTable* RaceEdicts()
	{
		UDataTable* Table = CosmicTests::NewTable(FEdictDef::StaticStruct());
		auto Add = [Table](FName Name, ECosmicEdictScope Scope, int32 Bonus)
		{
			FEdictDef Row;
			Row.DisplayName = FText::AsCultureInvariant(Name.ToString());
			Row.Scope = Scope;
			Row.HazardCountdownBonus.Add(CosmicTags::HazardPlague(), Bonus);
			Table->AddRow(Name, Row);
		};
		Add(RaceLawX, ECosmicEdictScope::HomeGalaxy, 40);
		Add(RaceLawY, ECosmicEdictScope::HomeGalaxy, 7);
		Add(RaceLawZ, ECosmicEdictScope::HomeGalaxy, 100);
		Add(RaceOwn, ECosmicEdictScope::People, 3);
		return Table;
	}

	// A people of the laws' board, with the edicts it holds.
	struct FRaceHolder
	{
		const TCHAR* CivID;
		TArray<FName> Edicts;
	};

	// The laws' board: one galaxy, systems a (people A's), b (B's) and w (no one's); the peoples given with their edicts.
	FGalaxyState RaceLawBoardOf(std::initializer_list<FRaceHolder> Peoples)
	{
		const FName Galaxy(TEXT("gal_race"));
		FGalaxyState State;
		for (const TCHAR* const Owner : { TEXT("civ_race_a"), TEXT("civ_race_b"), TEXT("") })
		{
			FStarSystemState& System = State.Systems.AddDefaulted_GetRef();
			System.SystemID = FName(*FString::Printf(TEXT("sys_race_%d"), State.Systems.Num()));
			System.OwnerCivID = *Owner ? FName(Owner) : FName();
			System.GalaxyRow = Galaxy;
		}
		for (const FRaceHolder& People : Peoples)
		{
			FCivilizationState& Civ = State.Civs.AddDefaulted_GetRef();
			Civ.CivID = FName(People.CivID);
			Civ.GalaxyRow = Galaxy;
			Civ.ActiveEdicts = People.Edicts;
		}
		return State;
	}

	CosmicRaceMeasure::FPeopleLaws RaceLawsOf(TArray<FName> Laws)
	{
		CosmicRaceMeasure::FPeopleLaws Entry;
		Entry.GalaxyRow = FName(TEXT("gal_race"));
		Entry.Laws = MoveTemp(Laws);
		return Entry;
	}
}

// The race measures (header section 5.1): (1) FourthRungCheckOf - the peoples on the fourth rung before the tick, the ruled
// people standing there counted (mutation: left out), plus those that climbed to it in the tick, the ruled people counted
// (mutation: left out), against the cap (mutations: strictly below; the count before the tick dropped), asked only when a
// people not ruled climbed to the fourth rung (mutations: asked on the ruled people's climb; asked on a climb to the fifth) and
// the cap is above 0 (mutation: asked at 0). (2) FourthRungCountOf counts the ruled people (mutation: skipped) and is pinned to
// CosmicAscent::FourthRungOpenOf - open exactly while the count is below the cap - on boards of 1, 2 and 3 at the fourth rung
// or above with a people of the third and one of another galaxy beside them. (3) FifthRungEndOf is EndingOf's answer for the
// three fifth-rung kinds alone: the ruled people's end (mutation: dropped) and another people's (mutation: dropped) pass, the
// clock (mutation: kept) and the silence (mutation: taken) give none, and a fifth rung in the silence is another people's end.
// (4) ClaimEndOf's precedence, one case a step (mutations: eased before joined, answered before eased, expired before relief,
// no expired, expired by default), and ClaimEndNameOf's seven words (mutation: two swapped). (5) LawsAsPlacedOf: a law enacted
// in the tick is dropped (mutation: kept), a people gone in the tick brings its laws back with its galaxy (mutation: not), an
// arrival's laws are dropped (mutation: kept), the people's own edicts stay, the countdown bonus of the answer is the bonus of
// the laws before the tick on every system, and an unchanged tick answers false with Out untouched (mutation: true).
// (6) CosmicTests::GalaxiesWithCap on the game's tables: the home row's cap is the one given and its every other field the
// game's (mutation: the home row blanked but its cap), every other row is equal (mutation: every row's cap set) and no row is
// left out (mutation: the home row alone).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimRaceMeasureTest, "Cosmic_Ascension.Sim.RaceMeasure", CosmicTests::Flags)
bool FCosmicSimRaceMeasureTest::RunTest(const FString& Parameters)
{
	using namespace CosmicRaceMeasureTests;
	using CosmicRaceMeasure::FFourthRungCheck;
	using CosmicRaceMeasure::FRungStep;
	const EAscensionTier T2 = EAscensionTier::Tier2_Stellar;
	const EAscensionTier T3 = EAscensionTier::Tier3_Causal;
	const EAscensionTier T4 = EAscensionTier::Tier4_Transcendent;
	const EAscensionTier T5 = EAscensionTier::Tier5_EternalOrder;

	// ---- (1) The race check ----
	{
		const FRungStep Steps[] = { RaceStepOf(TEXT("civ_x"), T4, T4), RaceStepOf(TEXT("civ_y"), T3, T4) };
		const FFourthRungCheck Check = CosmicRaceMeasure::FourthRungCheckOf(Steps, 2);
		TestTrue(TEXT("Cap 2, one people on the fourth rung and one AI climb: asked, and 1 + 1 <= 2 holds"), Check.bAsked && Check.Holds());
		TestTrue(TEXT("(1 before the tick, 1 climbed)"), Check.Before == 1 && Check.Climbed == 1 && Check.Cap == 2);
	}
	{
		const FRungStep Steps[] = { RaceStepOf(TEXT("civ_x"), T4, T4), RaceStepOf(TEXT("civ_w"), T4, T5), RaceStepOf(TEXT("civ_y"), T3, T4) };
		const FFourthRungCheck Check = CosmicRaceMeasure::FourthRungCheckOf(Steps, 2);
		TestTrue(TEXT("Cap 2, two on the fourth rung or above and one AI climb: 2 + 1 > 2 fails"), Check.bAsked && !Check.Holds());
		TestTrue(TEXT("(a climb to the fifth rung is no climb to the fourth: 2 before, 1 climbed)"), Check.Before == 2 && Check.Climbed == 1);
	}
	{
		const FRungStep Steps[] = { RaceStepOf(TEXT("civ_ruled"), T3, T4, true), RaceStepOf(TEXT("civ_x"), T4, T4), RaceStepOf(TEXT("civ_y"), T3, T4) };
		const FFourthRungCheck Check = CosmicRaceMeasure::FourthRungCheckOf(Steps, 2);
		TestTrue(TEXT("The ruled people's climb is counted - the judgment sees it first: 1 + 2 > 2 fails"), Check.bAsked && !Check.Holds());
		TestTrue(TEXT("(1 before, 2 climbed)"), Check.Before == 1 && Check.Climbed == 2);
	}
	{
		const FRungStep Steps[] = { RaceStepOf(TEXT("civ_x"), T4, T4), RaceStepOf(TEXT("civ_w"), T4, T4), RaceStepOf(TEXT("civ_ruled"), T3, T4, true) };
		const FFourthRungCheck Check = CosmicRaceMeasure::FourthRungCheckOf(Steps, 2);
		TestTrue(TEXT("Only the ruled people climbed past the cap: not asked (the brake never holds it), and it holds"), !Check.bAsked && Check.Holds());
	}
	{
		const FRungStep Steps[] = { RaceStepOf(TEXT("civ_ruled"), T4, T4, true), RaceStepOf(TEXT("civ_x"), T4, T4), RaceStepOf(TEXT("civ_y"), T3, T4) };
		const FFourthRungCheck Check = CosmicRaceMeasure::FourthRungCheckOf(Steps, 2);
		TestTrue(TEXT("The ruled people standing on the fourth rung is counted before the tick: 2 + 1 > 2 fails"), Check.bAsked && !Check.Holds());
		TestTrue(TEXT("(2 before, the ruled one among them, 1 climbed)"), Check.Before == 2 && Check.Climbed == 1);
	}
	{
		const FRungStep Steps[] = { RaceStepOf(TEXT("civ_x"), T4, T5), RaceStepOf(TEXT("civ_ruled"), T3, T4, true) };
		const FFourthRungCheck Check = CosmicRaceMeasure::FourthRungCheckOf(Steps, 1);
		TestTrue(TEXT("An AI climb from the fourth rung to the fifth is not asked, though the ruled people passed the cap of 1"), !Check.bAsked && Check.Holds());
	}
	{
		const FRungStep Steps[] = { RaceStepOf(TEXT("civ_x"), T4, T4), RaceStepOf(TEXT("civ_w"), T4, T4), RaceStepOf(TEXT("civ_y"), T3, T4) };
		const FFourthRungCheck Check = CosmicRaceMeasure::FourthRungCheckOf(Steps, 0);
		TestTrue(TEXT("A cap of 0 is no brake: not asked, and it holds"), !Check.bAsked && Check.Holds());
	}

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables& Game = Sim->GetTables();
	const FCosmicRulesDef* GameRules = Game.FindRules();
	if (!TestNotNull(TEXT("(the game has its rules row)"), GameRules) || !TestNotNull(TEXT("(and its galaxy rows)"), Game.Galaxies.Get()))
	{
		return false;
	}
	const FName Home = GameRules->HomeGalaxyRow;
	const FName Far(TEXT("gal_race_far"));

	// ---- (2) The count of the fourth rung, and its pin to the brake's window ----
	TestEqual(TEXT("The count of the fourth rung counts the ruled people"), CosmicRaceMeasure::FourthRungCountOf(RacePeoplesOf({ { TEXT("civ_ruled"), T4, Home, true } }), Home), 1);
	{
		const FCosmicDataTables Capped = CosmicTests::WithGalaxies(Game, CosmicTests::GalaxiesWithCap(Game, 2));
		const FRacePeople Up[] = { { TEXT("civ_ruled"), T4, Home, true }, { TEXT("civ_up_a"), T5, Home, false }, { TEXT("civ_up_b"), T4, Home, false } };
		for (int32 Count = 1; Count <= 3; ++Count)
		{
			FGalaxyState Board = RacePeoplesOf({ { TEXT("civ_third"), T3, Home, false }, { TEXT("civ_far"), T4, Far, false } });
			for (int32 At = 0; At < Count; ++At)
			{
				FCivilizationState& Civ = Board.Civs.AddDefaulted_GetRef();
				Civ.CivID = FName(Up[At].CivID);
				Civ.Tier = Up[At].Tier;
				Civ.GalaxyRow = Up[At].GalaxyRow;
				Civ.bIsPlayerRuled = Up[At].bRuled;
			}
			const int32 Counted = CosmicRaceMeasure::FourthRungCountOf(Board, Home);
			TestEqual(*FString::Printf(TEXT("%d of the home galaxy on the fourth rung or above (the ruled people first) are counted - not the third, not another galaxy's"), Count), Counted, Count);
			TestTrue(*FString::Printf(TEXT("The pin at %d: the brake's window is open exactly while the count is below the cap of 2"), Count),
				CosmicAscent::FourthRungOpenOf(Board, Capped, Home) == (Counted < 2));
		}
	}

	// ---- (3) The first end of the fifth rung ----
	{
		FCosmicRulesDef Rules;
		Rules.ProvisionalEndTick = 6000;
		const FName PeopleA(TEXT("civ_race_end_a"));
		const FName PeopleB(TEXT("civ_race_end_b"));
		const FGalaxyState Clock = RaceEndStateOf(6000, T2, { { TEXT("civ_race_end_a"), T2 } });
		TestTrue(TEXT("(the clock alone: EndingOf answers the clock)"), CosmicEndings::EndingOf(Clock, Rules).Kind == CosmicTags::EndingClock());
		TestFalse(TEXT("The clock alone is no fifth-rung end"), CosmicRaceMeasure::FifthRungEndOf(Clock, Rules).Kind.IsValid());
		const FGalaxyState Other = RaceEndStateOf(100, T2, { { TEXT("civ_race_end_a"), T2 }, { TEXT("civ_race_end_b"), T5 } });
		const FCosmicEndingRecord OtherEnd = CosmicRaceMeasure::FifthRungEndOf(Other, Rules);
		TestTrue(TEXT("An AI people on the fifth rung: another people's end, with its people and the tick"), OtherEnd.Kind == CosmicTags::EndingOtherPeople()
			&& OtherEnd.CivID == PeopleB && OtherEnd.Tick == 100);
		const FGalaxyState Ruled = RaceEndStateOf(100, T5, { { TEXT("civ_race_end_a"), T5 } });
		const FCosmicEndingRecord RuledEnd = CosmicRaceMeasure::FifthRungEndOf(Ruled, Rules);
		TestTrue(TEXT("A life's peak on the fifth rung: the ruled people's end, with its people"), RuledEnd.Kind == CosmicTags::EndingRuledPeople() && RuledEnd.CivID == PeopleA);
		FGalaxyState Silent = RaceEndStateOf(100, T2, { { TEXT("civ_race_end_a"), T2 } });
		PutInSilence(Silent, 90);
		TestTrue(TEXT("(the silence alone: EndingOf answers it)"), CosmicEndings::EndingOf(Silent, Rules).Kind == CosmicTags::EndingNoCandidate());
		TestFalse(TEXT("The silence alone is no fifth-rung end"), CosmicRaceMeasure::FifthRungEndOf(Silent, Rules).Kind.IsValid());
		FGalaxyState SilentFifth = RaceEndStateOf(100, T2, { { TEXT("civ_race_end_a"), T2 }, { TEXT("civ_race_end_b"), T5 } });
		PutInSilence(SilentFifth, 90);
		const FCosmicEndingRecord SilentEnd = CosmicRaceMeasure::FifthRungEndOf(SilentFifth, Rules);
		TestTrue(TEXT("A fifth rung in the silence is another people's end (the silence ranks after it)"), SilentEnd.Kind == CosmicTags::EndingOtherPeople() && SilentEnd.CivID == PeopleB);
	}

	// ---- (4) A claim's end ----
	TestEqual(TEXT("Joined and eased: joined"), CosmicRaceMeasure::ClaimEndOf(true, true, false, false), 0);
	TestEqual(TEXT("Eased and answered: eased"), CosmicRaceMeasure::ClaimEndOf(false, true, true, false), 1);
	TestEqual(TEXT("Answered and expired: relief"), CosmicRaceMeasure::ClaimEndOf(false, false, true, true), 2);
	TestEqual(TEXT("Expired alone: expired"), CosmicRaceMeasure::ClaimEndOf(false, false, false, true), 4);
	TestEqual(TEXT("None of the four: another end"), CosmicRaceMeasure::ClaimEndOf(false, false, false, false), 3);
	{
		FString Words;
		for (int32 End = -1; End <= 7; ++End)
		{
			Words += FString(CosmicRaceMeasure::ClaimEndNameOf(End)) + TEXT(" ");
		}
		TestEqual(TEXT("The words of the ends, 0 to 6, and '-' outside them"), Words, FString(TEXT("- joined eased relief other expired input standing - ")));
	}

	// ---- (5) The galaxy laws as they stood when a hazard was placed ----
	{
		FCosmicDataTables Laws;
		Laws.Edicts = RaceEdicts();
		const FGameplayTag Plague = CosmicTags::HazardPlague();
		// Before the tick: A holds law x (and its own edict), B nothing, C law y. In the tick A and the arrival D enact law z,
		// and the crossing stage takes C.
		TMap<FName, CosmicRaceMeasure::FPeopleLaws> Before;
		Before.Add(FName(TEXT("civ_race_a")), RaceLawsOf({ RaceLawX }));
		Before.Add(FName(TEXT("civ_race_b")), RaceLawsOf({}));
		Before.Add(FName(TEXT("civ_race_c")), RaceLawsOf({ RaceLawY }));
		const FGalaxyState After = RaceLawBoardOf({ { TEXT("civ_race_a"), { RaceOwn, RaceLawX, RaceLawZ } }, { TEXT("civ_race_b"), {} }, { TEXT("civ_race_d"), { RaceLawZ } } });
		FGalaxyState Placed;
		const bool bChanged = CosmicRaceMeasure::LawsAsPlacedOf(Before, After, Laws, Placed);
		TestTrue(TEXT("A tick that changed the laws answers true"), bChanged);
		if (TestEqual(TEXT("Every people after the tick, and the one gone in it appended"), Placed.Civs.Num(), 4) && TestEqual(TEXT("(and the systems)"), Placed.Systems.Num(), 3))
		{
			TestTrue(TEXT("A law enacted in the tick is dropped; the people's own edict stays (A: own, x)"), Placed.Civs[0].ActiveEdicts == TArray<FName>{ RaceOwn, RaceLawX });
			TestTrue(TEXT("An arrival's laws are dropped (D)"), Placed.Civs[2].ActiveEdicts.IsEmpty());
			TestTrue(TEXT("A people gone in the tick is appended with its galaxy and its laws (C: y)"), Placed.Civs[3].CivID == FName(TEXT("civ_race_c"))
				&& Placed.Civs[3].GalaxyRow == FName(TEXT("gal_race")) && Placed.Civs[3].ActiveEdicts == TArray<FName>{ RaceLawY });
		}
		// The bonus of the answer is the bonus of the laws before the tick, on each system (the owner's own and the galaxy's).
		const FGalaxyState AsBefore = RaceLawBoardOf({ { TEXT("civ_race_a"), { RaceOwn, RaceLawX } }, { TEXT("civ_race_b"), {} }, { TEXT("civ_race_c"), { RaceLawY } } });
		FCosmicStateIndex PlacedIndex;
		PlacedIndex.Rebuild(Placed);
		FCosmicStateIndex BeforeIndex;
		BeforeIndex.Rebuild(AsBefore);
		FCosmicStateIndex AfterIndex;
		AfterIndex.Rebuild(After);
		for (int32 SystemIndex = 0; SystemIndex < AsBefore.Systems.Num(); ++SystemIndex)
		{
			TestEqual(*FString::Printf(TEXT("System %d: the countdown bonus of the laws as placed is the bonus before the tick"), SystemIndex),
				CosmicEdicts::CountdownBonusOf(Placed, PlacedIndex, Laws, SystemIndex, Plague), CosmicEdicts::CountdownBonusOf(AsBefore, BeforeIndex, Laws, SystemIndex, Plague));
		}
		TestEqual(TEXT("(the bonus before the tick on A's system: own 3 + x 40 + C's y 7)"), CosmicEdicts::CountdownBonusOf(AsBefore, BeforeIndex, Laws, 0, Plague), 50);
		TestNotEqual(TEXT("(the bonus after the tick differs: the case can tell)"), CosmicEdicts::CountdownBonusOf(After, AfterIndex, Laws, 0, Plague), 50);

		// Nothing changed: false, and Out untouched.
		TMap<FName, CosmicRaceMeasure::FPeopleLaws> Same;
		Same.Add(FName(TEXT("civ_race_a")), RaceLawsOf({ RaceLawX }));
		Same.Add(FName(TEXT("civ_race_b")), RaceLawsOf({}));
		const FGalaxyState Unchanged = RaceLawBoardOf({ { TEXT("civ_race_a"), { RaceOwn, RaceLawX } }, { TEXT("civ_race_b"), {} } });
		FGalaxyState Untouched;
		Untouched.Tick = 777;
		TestFalse(TEXT("A tick that changed no law answers false"), CosmicRaceMeasure::LawsAsPlacedOf(Same, Unchanged, Laws, Untouched));
		TestTrue(TEXT("and leaves Out untouched"), Untouched.Tick == 777 && Untouched.Civs.IsEmpty() && Untouched.Systems.IsEmpty());
	}

	// ---- (6) The galaxy rows of -FourthRungCap ----
	{
		UDataTable* Five = CosmicTests::GalaxiesWithCap(Game, 5);
		const FGalaxyDef* HomeRow = Five ? Five->FindRow<FGalaxyDef>(Home, TEXT("Sim.RaceMeasure"), false) : nullptr;
		if (TestNotNull(TEXT("The copy holds the home row"), HomeRow))
		{
			TestEqual(TEXT("whose cap is the one given"), HomeRow->AIFourthRungCap, 5);
			if (const FGalaxyDef* GameHome = Game.FindGalaxy(Home))
			{
				TestNotEqual(TEXT("(the game's home cap is not 5: the case can tell)"), GameHome->AIFourthRungCap, 5);
				FGalaxyDef Expected = *GameHome;
				Expected.AIFourthRungCap = 5;
				TestTrue(TEXT("and every other field of the home row is the game's: the same world, the cap alone differs"),
					FGalaxyDef::StaticStruct()->CompareScriptStruct(&Expected, HomeRow, 0));
			}
		}
		TestTrue(TEXT("(the game has rows besides the home row)"), Game.GetGalaxyRowNames().Num() > 1);
		TestEqual(TEXT("No row is left out"), Five ? Five->GetRowMap().Num() : -1, Game.Galaxies->GetRowMap().Num());
		for (const FName Row : Game.GetGalaxyRowNames())
		{
			const FGalaxyDef* Was = Game.FindGalaxy(Row);
			const FGalaxyDef* Copied = Five ? Five->FindRow<FGalaxyDef>(Row, TEXT("Sim.RaceMeasure"), false) : nullptr;
			if (Row != Home && Was && TestNotNull(*FString::Printf(TEXT("The copy holds %s"), *Row.ToString()), Copied))
			{
				TestTrue(*FString::Printf(TEXT("Every other row is equal: %s"), *Row.ToString()), FGalaxyDef::StaticStruct()->CompareScriptStruct(Was, Copied, 0));
			}
		}
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

// Cosmic Ascension - automation tests of diplomacy and conflict (B3): the pair records and their rules, the diplomacy
// stage, the four commands, the fleets, the sieges, the fleet's delay, the plague's spread, the AI's hostility, #73 and the
// conflict lines. Design: docs/60_DECISIONS/headers/B3_diplomacy_and_conflict.md section 2, 6.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicTags.h"
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
#include "Log/CosmicChronicle.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "World/CosmicAutoPause.h"
#include "World/CosmicRulerPolicy.h"

namespace CosmicDiplomacyTests
{
	using CosmicConflict::ECosmicGuardBlock;
	using CosmicConflict::ECosmicSiegeBlock;
	using CosmicDiplomacy::ECosmicStanceBlock;
	using CosmicDiplomacy::ECosmicTreatyBlock;

	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	const FGameplayTag Friendly() { return CosmicTags::StanceFriendly(); }
	const FGameplayTag Hostile() { return CosmicTags::StanceHostile(); }
	const FGameplayTag Trade() { return CosmicTags::TreatyTrade(); }
	const FGameplayTag NonAggression() { return CosmicTags::TreatyNonAggression(); }

	// The rows of this file.
	const FName Mobilize(TEXT("edict_test_mobilize"));  // fleet growth x1.5
	const FName FreeTrade(TEXT("edict_test_trade"));    // treaty acceptance -0.2
	const FName Shut(TEXT("edict_test_shut"));          // plague share x0
	const FName Sick(TEXT("crisis_test_sick"));
	const FName Unrest(TEXT("crisis_test_unrest"));
	const FName Claimed(TEXT("crisis_test_claim"));
	const FName Endless(TEXT("crisis_test_endless"));    // a countdown a few ticks short of the int32 limit
	const FName Silence(TEXT("trans_silence"));          // the game's fallen transcendent (Tier 4)

	UDataTable* Edicts()
	{
		UDataTable* Table = CosmicTests::NewTable(FEdictDef::StaticStruct());
		auto Add = [Table](FName Name, TFunctionRef<void(FEdictDef&)> Fill)
		{
			FEdictDef Row;
			Row.DisplayName = FText::AsCultureInvariant(Name.ToString());
			Row.Summary = FText::AsCultureInvariant(TEXT("a test edict."));
			Fill(Row);
			Table->AddRow(Name, Row);
		};
		Add(Mobilize, [](FEdictDef& Row) { Row.FleetGrowthMultiplier = 1.5f; });
		Add(FreeTrade, [](FEdictDef& Row) { Row.TreatyAcceptBonus = 0.2f; });
		Add(Shut, [](FEdictDef& Row) { Row.HazardSpawnMultipliers.Add(Tag(TEXT("Hazard.Plague")), 0.f); });
		return Table;
	}

	UDataTable* Hazards(float SickChance)
	{
		UDataTable* Table = CosmicTests::NewTable(FHazardDef::StaticStruct());
		FHazardDef Plague;
		Plague.HazardTag = Tag(TEXT("Hazard.Plague"));
		Plague.DisplayName = FText::AsCultureInvariant(TEXT("sick"));
		Plague.CountdownTicks = 200;
		Plague.SpawnRule = ECosmicHazardSpawn::Draw;
		Plague.SpawnChancePerTick = SickChance;
		Table->AddRow(Sick, Plague);
		FHazardDef Rebellion;
		Rebellion.HazardTag = Tag(TEXT("Hazard.Rebellion"));
		Rebellion.DisplayName = FText::AsCultureInvariant(TEXT("unrest"));
		Rebellion.CountdownTicks = 200;
		Rebellion.SpawnRule = ECosmicHazardSpawn::Capital;
		FCosmicCondition Never;
		Never.Type = EConditionType::GalaxyTickAtLeast;
		Never.Value = 1.0e9f;
		Rebellion.SpawnConditions = { Never };
		Table->AddRow(Unrest, Rebellion);
		FHazardDef Claim;
		Claim.HazardTag = Tag(TEXT("Hazard.Claim"));
		Claim.DisplayName = FText::AsCultureInvariant(TEXT("claim"));
		Claim.CountdownTicks = 300;
		Claim.SpawnRule = ECosmicHazardSpawn::ClaimRule;
		Table->AddRow(Claimed, Claim);
		FHazardDef Long;
		Long.HazardTag = Tag(TEXT("Hazard.AIAwakening"));
		Long.DisplayName = FText::AsCultureInvariant(TEXT("endless"));
		Long.CountdownTicks = MAX_int32 - 10;
		Long.SpawnRule = ECosmicHazardSpawn::Draw;
		Long.SpawnChancePerTick = 0.f;
		Table->AddRow(Endless, Long);
		return Table;
	}

	/**
	 * A board of twelve living systems wired as CosmicTests::WireChain (0-1-...-9, 4-10, 10-11) and four peoples that own
	 * every one of them, all settled, under quiet rules (no event, no drain, no first crisis, no projects, no inherited
	 * edict, no transcendent) with this file's edicts and hazards; every system known and every people met; the player rules
	 * people 0. Capitals: 0, 3, 6, 9. People 0 holds {0, 1}, 1 holds {3, 2, 4}, 2 holds {6, 5, 7}, 3 holds {9, 8, 10, 11}:
	 * 0 borders 1 (lane 1-2), 1 borders 2 (4-5) and 3 (4-10), 2 borders 3 (7-8). Realm caps {4, 4, 5, 5, 5}, so no settlement
	 * has a target and a T2 people may still take a system. Every people on the first rung with ethics 0. Diplomacy and
	 * conflict off (no drift, no fleet, no spread - whatever DT_Rules holds), the AI's gate toward the ruled people the general
	 * one, and no sight stage (FirstSightTick far away):
	 * the case turns on what it reads, and every knowing write it sees comes from the path under test.
	 */
	FGalaxyState* LayBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Original, TFunctionRef<void(FCosmicRulesDef&)> Edit, float SickChance = 0.f)
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
		Rules.SocietyRecoveryBonus = 0.f;
		Rules.RealmCapByTier = { 4, 4, 5, 5, 5 };
		Rules.RelationDriftRate = 0.f;
		Rules.FleetPerSettledByTier = { 0.f, 0.f, 0.f, 0.f, 0.f };
		Rules.PlagueSpreadMultiplier = 1.f;
		Rules.FirstSightTick = 1000000;
		Rules.SightIntervalTicks = 1000000;
		// Toward the ruled people the AI's gate is the general one (B3 decision 21's knobs - a case that reads them sets them).
		Rules.AIHostileRelationTowardRuled = Rules.AIHostileRelation;
		Rules.AIHostilePowerRatioTowardRuled = Rules.AIHostilePowerRatio;
		Edit(Rules);
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));
		Tables.Edicts = Edicts();
		Tables.Hazards = Hazards(SickChance);
		Tables.Events = nullptr;
		Sim.SetTables(Tables);
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
			Civ.Ethics = FEthicsAxes();
			Civ.FleetPower = 0.f;
			Civ.FleetTargetSystemID = NAME_None;
			Civ.FleetTargetSinceTick = 0;
			Civ.Ties.Reset();
			Civ.Relations.Reset();
			Civ.ActiveEdicts.Reset();
			Civ.Stability = 0.75f;
			Civ.Stockpile = FResourcePool();
		}
		State.Player.CurrentSystemID = State.Systems[0].SystemID;
		TArray<int32> Every;
		for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num(); ++SystemIndex)
		{
			Every.Add(SystemIndex);
		}
		State.Knowing = FKnowingState();
		CosmicTests::SetKnowing(State, {}, { 0, 1, 2, 3 });
		for (const int32 SystemIndex : Every)
		{
			FKnownMark& Mark = State.Knowing.KnownSystems.AddDefaulted_GetRef();
			Mark.ID = State.Systems[SystemIndex].SystemID;
		}
		State.Knowing.FirstContactCivID = State.Civs[1].CivID;
		Sim.RebuildStateIndex();
		Sim.SyncRngState();
		return &State;
	}

	void NoEdit(FCosmicRulesDef&) {}

	// Fleets on: one per settled system from the second rung, the game's growth and upkeep.
	void Fleets(FCosmicRulesDef& Rules)
	{
		Rules.FleetPerSettledByTier = { 0.f, 1.f, 1.f, 1.f, 1.f };
	}

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

	// The sim draws of one tick from a state (loaded first).
	int32 DrawsOfTick(UGalaxySimulationSubsystem& Sim, const FGalaxyState& From)
	{
		Sim.LoadState(From);
		Sim.StepOnce();
		return DrawsBetween(From.RngState, Sim.GetState().RngState);
	}

	int32 LinesOf(const FGalaxyState& State, FName Row)
	{
		int32 Count = 0;
		for (const FCosmicLogEntry& Entry : State.Log)
		{
			Count += Entry.SourceRow == Row ? 1 : 0;
		}
		return Count;
	}

	const FCosmicLogEntry* LastLineOf(const FGalaxyState& State, FName Row)
	{
		for (int32 Index = State.Log.Num() - 1; Index >= 0; --Index)
		{
			if (State.Log[Index].SourceRow == Row)
			{
				return &State.Log[Index];
			}
		}
		return nullptr;
	}

	FName IDOf(const FGalaxyState& State, int32 SystemIndex)
	{
		return State.Systems[SystemIndex].SystemID;
	}

	// Knowing as a case starts from it: the people CivIndex not met, the system SystemIndex (when given) not known.
	void Unmeet(FGalaxyState& State, int32 CivIndex, int32 SystemIndex = INDEX_NONE)
	{
		const FName CivID = State.Civs[CivIndex].CivID;
		State.Knowing.MetCivs.RemoveAll([CivID](const FKnownMark& Mark) { return Mark.ID == CivID; });
		if (State.Systems.IsValidIndex(SystemIndex))
		{
			const FName SystemID = State.Systems[SystemIndex].SystemID;
			State.Knowing.KnownSystems.RemoveAll([SystemID](const FKnownMark& Mark) { return Mark.ID == SystemID; });
		}
	}

	bool HasMet(const FGalaxyState& State, FName CivID)
	{
		return State.Knowing.MetCivs.ContainsByPredicate([CivID](const FKnownMark& Mark) { return Mark.ID == CivID; });
	}

	bool Knows(const FGalaxyState& State, int32 SystemIndex)
	{
		const FName SystemID = State.Systems[SystemIndex].SystemID;
		return State.Knowing.KnownSystems.ContainsByPredicate([SystemID](const FKnownMark& Mark) { return Mark.ID == SystemID; });
	}

	// The index in State.Log of the last line of Row about the people CivID (any people when None); INDEX_NONE without one.
	int32 LineIndexOf(const FGalaxyState& State, FName Row, FName CivID = NAME_None)
	{
		for (int32 Index = State.Log.Num() - 1; Index >= 0; --Index)
		{
			if (State.Log[Index].SourceRow == Row && (CivID.IsNone() || State.Log[Index].FocusCivID == CivID))
			{
				return Index;
			}
		}
		return INDEX_NONE;
	}

	// Hands SystemIndex from its owner to the people To by hand (a board change, no rule).
	void HandOver(FGalaxyState& State, int32 SystemIndex, int32 To)
	{
		FStarSystemState& System = State.Systems[SystemIndex];
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.OwnedSystems.Remove(System.SystemID);
		}
		System.OwnerCivID = State.Civs[To].CivID;
		State.Civs[To].OwnedSystems.Add(System.SystemID);
	}

	// The board's sieges: the attacker hostile toward the owner of SystemIndex, its fleet aimed at it since Since.
	void Besiege(FGalaxyState& State, int32 Attacker, int32 Owner, int32 SystemIndex, int64 Since)
	{
		CosmicDiplomacy::SetStanceOf(State.Civs[Attacker], State.Civs[Owner].CivID, CosmicTags::StanceHostile());
		State.Civs[Attacker].FleetTargetSystemID = State.Systems[SystemIndex].SystemID;
		State.Civs[Attacker].FleetTargetSinceTick = Since;
	}
}

// The pair records and their rules (header section 2.2, 2.3): borders, the ethics distance, the target and the view, the
// normal form, the one writer of a treaty, the first reason of a stance, a treaty, a siege and a guard, the odds.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimDiplomacyRulesTest, "Cosmic_Ascension.Sim.DiplomacyRules", CosmicTests::Flags)
bool FCosmicSimDiplomacyRulesTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDiplomacyTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* State = LayBoard(*Sim, Original, Fleets);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicRulesDef& Rules = Sim->GetRules();
	const FCosmicDataTables& Tables = Sim->GetTables();
	TArray<FCivilizationState>& Civs = State->Civs;

	// Borders: settled systems one lane apart in one galaxy row.
	{
		TestTrue(TEXT("0 borders 1 (lane 1-2)"), CosmicDiplomacy::Borders(*State, Index, 0, 1) && CosmicDiplomacy::Borders(*State, Index, 1, 0));
		TestTrue(TEXT("1 borders 2 and 3"), CosmicDiplomacy::Borders(*State, Index, 1, 2) && CosmicDiplomacy::Borders(*State, Index, 1, 3));
		TestTrue(TEXT("2 borders 3"), CosmicDiplomacy::Borders(*State, Index, 2, 3));
		TestFalse(TEXT("0 does not border 2"), CosmicDiplomacy::Borders(*State, Index, 0, 2));
		TestFalse(TEXT("nor 3"), CosmicDiplomacy::Borders(*State, Index, 0, 3));
		TestFalse(TEXT("A people does not border itself"), CosmicDiplomacy::Borders(*State, Index, 0, 0));
		State->Systems[2].ActiveMegaproject = CosmicRows::ProjectSettle;
		TestFalse(TEXT("A system being settled is no border"), CosmicDiplomacy::Borders(*State, Index, 0, 1));
		State->Systems[2].ActiveMegaproject = NAME_None;
		const FName Row = State->Systems[2].GalaxyRow;
		State->Systems[2].GalaxyRow = FName(TEXT("galaxy_test_far"));
		TestFalse(TEXT("A lane into another galaxy row is no border"), CosmicDiplomacy::Borders(*State, Index, 0, 1));
		State->Systems[2].GalaxyRow = Row;
		const TArray<TArray<int32>> Table = CosmicDiplomacy::BorderTableOf(*State, Index);
		bool bSame = Table.Num() == Civs.Num();
		for (int32 A = 0; bSame && A < Civs.Num(); ++A)
		{
			for (int32 B = 0; B < Civs.Num(); ++B)
			{
				bSame &= Table[A].Contains(B) == CosmicDiplomacy::Borders(*State, Index, A, B);
			}
		}
		TestTrue(TEXT("The table of the stage is Borders, pair by pair"), bSame);
	}

	// The ethics distance and the target: + - x only.
	{
		FEthicsAxes A;
		A.Pacifism = 1.f;
		FEthicsAxes B;
		B.Pacifism = -1.f;
		B.Spiritualism = 0.5f;
		TestEqual(TEXT("The distance is the mean of the three gaps"), CosmicDiplomacy::EthicsDistanceOf(A, B), 2.5f / 3.f, 1.0e-6f);
		Civs[1].Ethics.Pacifism = 0.6f;
		TestEqual(TEXT("The target: -0.6 x distance"), CosmicDiplomacy::TargetRelationOf(*State, Rules, 0, 1), -0.6f * 0.2f, 1.0e-6f);
		CosmicDiplomacy::SetStanceOf(Civs[1], Civs[0].CivID, Friendly());
		TestEqual(TEXT("plus the stance the other holds toward the viewer"), CosmicDiplomacy::TargetRelationOf(*State, Rules, 0, 1), -0.12f + 0.4f, 1.0e-6f);
		TestEqual(TEXT("(the viewer's own stance is not the term)"), CosmicDiplomacy::TargetRelationOf(*State, Rules, 1, 0), -0.12f, 1.0e-6f);
		CosmicDiplomacy::Sign(*State, 0, 1, Trade());
		TestEqual(TEXT("plus 0.2 a treaty"), CosmicDiplomacy::TargetRelationOf(*State, Rules, 0, 1), -0.12f + 0.4f + 0.2f, 1.0e-6f);
		CosmicDiplomacy::SetStanceOf(Civs[1], Civs[0].CivID, Hostile());
		TestEqual(TEXT("A hostile other takes 0.4"), CosmicDiplomacy::TargetRelationOf(*State, Rules, 0, 1), -0.12f - 0.4f + 0.2f, 1.0e-6f);
		CosmicDiplomacy::SetStanceOf(Civs[1], Civs[0].CivID, FGameplayTag());
		CosmicDiplomacy::Break(*State, 0, 1, Trade());
		Civs[1].Ethics = FEthicsAxes();
	}

	// The normal form and the one writer of a treaty.
	{
		CosmicDiplomacy::SetStanceOf(Civs[0], Civs[1].CivID, Friendly());
		TestTrue(TEXT("A stance makes an entry"), Civs[0].Ties.Contains(Civs[1].CivID) && CosmicDiplomacy::FriendlyCountOf(Civs[0]) == 1);
		CosmicDiplomacy::SetStanceOf(Civs[0], Civs[1].CivID, FGameplayTag());
		TestFalse(TEXT("Neutral with no treaty leaves no entry"), Civs[0].Ties.Contains(Civs[1].CivID));
		CosmicDiplomacy::SetStanceOf(Civs[0], Civs[1].CivID, FGameplayTag());
		TestTrue(TEXT("(and neutral toward one with no entry writes none)"), Civs[0].Ties.IsEmpty());
		CosmicDiplomacy::Sign(*State, 0, 1, Trade());
		TestTrue(TEXT("A treaty is written on both sides"), CosmicDiplomacy::HasTreaty(Civs[0], Civs[1].CivID, Trade()) && CosmicDiplomacy::HasTreaty(Civs[1], Civs[0].CivID, Trade()));
		CosmicDiplomacy::Sign(*State, 0, 1, Trade());
		TestEqual(TEXT("Signed twice, it is held once"), CosmicDiplomacy::TreatyCountOf(Civs[0], Civs[1].CivID), 1);
		CosmicDiplomacy::SetStanceOf(Civs[0], Civs[1].CivID, Friendly());
		CosmicDiplomacy::SetStanceOf(Civs[0], Civs[1].CivID, FGameplayTag());
		TestTrue(TEXT("Neutral with a treaty keeps the entry"), Civs[0].Ties.Contains(Civs[1].CivID));
		TestTrue(TEXT("A break removes the treaty from both sides"), CosmicDiplomacy::Break(*State, 1, 0, Trade()));
		TestTrue(TEXT("and the empty entries"), !Civs[0].Ties.Contains(Civs[1].CivID) && !Civs[1].Ties.Contains(Civs[0].CivID));
		TestFalse(TEXT("A treaty not held is not broken"), CosmicDiplomacy::Break(*State, 0, 1, Trade()));
		CosmicDiplomacy::SetStanceOf(Civs[2], Civs[3].CivID, Hostile());
		CosmicDiplomacy::Sign(*State, 0, 3, NonAggression());
		CosmicDiplomacy::ForgetCiv(*State, Civs[3].CivID);
		TestTrue(TEXT("ForgetCiv removes the key from every people"), !Civs[2].Ties.Contains(Civs[3].CivID) && !Civs[0].Ties.Contains(Civs[3].CivID));
	}

	// Hostility breaks the trade treaty on both sides, and only then lowers the other's view.
	{
		CosmicDiplomacy::Sign(*State, 0, 1, Trade());
		CosmicDiplomacy::Sign(*State, 0, 1, NonAggression());
		TestTrue(TEXT("Turning hostile on a trade partner breaks the treaty"), CosmicDiplomacy::TurnHostile(*State, 0, 1, 0.3f));
		TestTrue(TEXT("on both sides"), !CosmicDiplomacy::HasTreaty(Civs[0], Civs[1].CivID, Trade()) && !CosmicDiplomacy::HasTreaty(Civs[1], Civs[0].CivID, Trade()));
		TestTrue(TEXT("(another treaty stays)"), CosmicDiplomacy::HasTreaty(Civs[1], Civs[0].CivID, NonAggression()));
		TestEqual(TEXT("and the partner thinks less of the breaker"), Civs[1].Relations.FindRef(Civs[0].CivID), -0.3f, 1.0e-6f);
		TestTrue(TEXT("The stance is hostile"), CosmicDiplomacy::StanceOf(Civs[0], Civs[1].CivID) == Hostile());
		TestFalse(TEXT("With no trade treaty nothing breaks"), CosmicDiplomacy::TurnHostile(*State, 2, 1, 0.3f));
		TestFalse(TEXT("and no view falls"), Civs[1].Relations.Contains(Civs[2].CivID));
		Civs[0].Ties.Reset();
		Civs[1].Ties.Reset();
		Civs[2].Ties.Reset();
		Civs[1].Relations.Reset();
	}

	// The view: the stored relation of a bordering pair, else the target without the stance term.
	{
		Civs[1].Relations.Add(Civs[0].CivID, -0.45f);
		TestEqual(TEXT("A bordering pair is seen through its stored relation"), CosmicDiplomacy::ViewOf(*State, Index, Rules, 1, 0), -0.45f, 1.0e-6f);
		Civs[2].Ethics.Pacifism = -1.f;
		Civs[2].Relations.Add(Civs[0].CivID, 0.9f);
		TestEqual(TEXT("A pair that does not border is seen through the target"), CosmicDiplomacy::ViewOf(*State, Index, Rules, 2, 0), -0.6f / 3.f, 1.0e-6f);
		CosmicDiplomacy::SetStanceOf(Civs[0], Civs[2].CivID, Friendly());
		TestEqual(TEXT("without the stance term (a friendly stance does not buy a treaty)"), CosmicDiplomacy::ViewOf(*State, Index, Rules, 2, 0), -0.2f, 1.0e-6f);
		CosmicDiplomacy::SetStanceOf(Civs[0], Civs[2].CivID, FGameplayTag());
		Civs[1].Relations.Reset();
		Civs[2].Relations.Reset();
		Civs[2].Ethics = FEthicsAxes();
		TestEqual(TEXT("Band: -0.5 is hostile"), CosmicDiplomacy::ViewBandOf(-0.5f), 0);
		TestEqual(TEXT("-0.49 cold"), CosmicDiplomacy::ViewBandOf(-0.49f), 1);
		TestEqual(TEXT("-0.2 cold"), CosmicDiplomacy::ViewBandOf(-0.2f), 1);
		TestEqual(TEXT("-0.19 indifferent"), CosmicDiplomacy::ViewBandOf(-0.19f), 2);
		TestEqual(TEXT("0.19 indifferent"), CosmicDiplomacy::ViewBandOf(0.19f), 2);
		TestEqual(TEXT("0.2 warm"), CosmicDiplomacy::ViewBandOf(0.2f), 3);
		TestEqual(TEXT("0.49 warm"), CosmicDiplomacy::ViewBandOf(0.49f), 3);
		TestEqual(TEXT("0.5 trusting"), CosmicDiplomacy::ViewBandOf(0.5f), 4);
	}

	// The first reason of a stance.
	{
		const FName One = Civs[1].CivID;
		TestTrue(TEXT("Stance: no such people"), CosmicDiplomacy::StanceBlockOf(*State, Index, Rules, 0, FName(TEXT("civ_nobody")), Friendly(), true) == ECosmicStanceBlock::NoCiv);
		TestTrue(TEXT("a tag that is no stance"), CosmicDiplomacy::StanceBlockOf(*State, Index, Rules, 0, One, Trade(), true) == ECosmicStanceBlock::NoCiv);
		TestTrue(TEXT("itself"), CosmicDiplomacy::StanceBlockOf(*State, Index, Rules, 0, Civs[0].CivID, Friendly(), true) == ECosmicStanceBlock::Self);
		TestTrue(TEXT("not met comes before the same stance"), CosmicDiplomacy::StanceBlockOf(*State, Index, Rules, 0, One, FGameplayTag(), false) == ECosmicStanceBlock::NotMet);
		TestTrue(TEXT("the same stance"), CosmicDiplomacy::StanceBlockOf(*State, Index, Rules, 0, One, FGameplayTag(), true) == ECosmicStanceBlock::Same);
		TestTrue(TEXT("Friendly is free"), CosmicDiplomacy::StanceBlockOf(*State, Index, Rules, 0, One, Friendly(), true) == ECosmicStanceBlock::None);
		CosmicDiplomacy::SetStanceOf(Civs[0], One, Friendly());
		TestTrue(TEXT("The one slot of the first rung is taken"), CosmicDiplomacy::StanceBlockOf(*State, Index, Rules, 0, Civs[2].CivID, Friendly(), true) == ECosmicStanceBlock::FriendlySlots);
		TestTrue(TEXT("Hostile on the first rung"), CosmicDiplomacy::StanceBlockOf(*State, Index, Rules, 0, Civs[2].CivID, Hostile(), true) == ECosmicStanceBlock::Tier);
		CosmicDiplomacy::Sign(*State, 0, 2, NonAggression());
		TestTrue(TEXT("(the rung comes before a non-aggression treaty)"), CosmicDiplomacy::StanceBlockOf(*State, Index, Rules, 0, Civs[2].CivID, Hostile(), true) == ECosmicStanceBlock::Tier);
		CosmicDiplomacy::Break(*State, 0, 2, NonAggression());
		Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		TestTrue(TEXT("The second rung has one slot too"), CosmicDiplomacy::StanceBlockOf(*State, Index, Rules, 0, Civs[2].CivID, Friendly(), true) == ECosmicStanceBlock::FriendlySlots);
		CosmicDiplomacy::Sign(*State, 0, 2, NonAggression());
		TestTrue(TEXT("Hostile under a non-aggression treaty"), CosmicDiplomacy::StanceBlockOf(*State, Index, Rules, 0, Civs[2].CivID, Hostile(), true) == ECosmicStanceBlock::NonAggression);
		CosmicDiplomacy::Break(*State, 0, 2, NonAggression());
		TestTrue(TEXT("Hostile from the second rung"), CosmicDiplomacy::StanceBlockOf(*State, Index, Rules, 0, Civs[2].CivID, Hostile(), true) == ECosmicStanceBlock::None);
		Civs[0].Ties.Reset();
		Civs[0].Tier = EAscensionTier::Tier1_Planetary;
	}

	// The first reason of a treaty.
	{
		const FName One = Civs[1].CivID;
		auto Block = [&](int32 Proposer, FName Other, FGameplayTag Treaty, bool bMet = true)
		{
			return CosmicDiplomacy::TreatyBlockOf(*State, Index, Tables, Rules, Proposer, Other, Treaty, bMet);
		};
		Civs[0].Stockpile.Influence = 100.f;
		TestTrue(TEXT("Treaty: no such people"), Block(0, FName(TEXT("civ_nobody")), Trade()) == ECosmicTreatyBlock::NoCiv);
		TestTrue(TEXT("a tag that is no treaty"), Block(0, One, Friendly()) == ECosmicTreatyBlock::NoCiv);
		TestTrue(TEXT("itself"), Block(0, Civs[0].CivID, Trade()) == ECosmicTreatyBlock::Self);
		TestTrue(TEXT("not met"), Block(0, One, Trade(), false) == ECosmicTreatyBlock::NotMet);
		TestTrue(TEXT("the first rung"), Block(0, One, Trade()) == ECosmicTreatyBlock::Tier);
		CosmicDiplomacy::SetStanceOf(Civs[1], Civs[0].CivID, Hostile());
		TestTrue(TEXT("(the rung comes before a hostility)"), Block(0, One, Trade()) == ECosmicTreatyBlock::Tier);
		CosmicDiplomacy::SetStanceOf(Civs[1], Civs[0].CivID, FGameplayTag());
		CosmicDiplomacy::Sign(*State, 0, 1, Trade());
		TestTrue(TEXT("a treaty held comes before the rung"), Block(0, One, Trade()) == ECosmicTreatyBlock::Held);
		CosmicDiplomacy::Break(*State, 0, 1, Trade());
		Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		TestTrue(TEXT("From the second rung, accepted"), Block(0, One, Trade()) == ECosmicTreatyBlock::None);
		CosmicDiplomacy::SetStanceOf(Civs[1], Civs[0].CivID, Hostile());
		TestTrue(TEXT("Their hostility"), Block(0, One, Trade()) == ECosmicTreatyBlock::Hostile);
		Civs[1].Relations.Add(Civs[0].CivID, -0.9f);
		TestTrue(TEXT("(a hostility comes before a refusal)"), Block(0, One, Trade()) == ECosmicTreatyBlock::Hostile);
		Civs[1].Relations.Reset();
		CosmicDiplomacy::SetStanceOf(Civs[1], Civs[0].CivID, FGameplayTag());
		CosmicDiplomacy::SetStanceOf(Civs[0], One, Hostile());
		TestTrue(TEXT("or ours"), Block(0, One, Trade()) == ECosmicTreatyBlock::Hostile);
		CosmicDiplomacy::SetStanceOf(Civs[0], One, FGameplayTag());
		Civs[1].Relations.Add(Civs[0].CivID, -0.11f);
		TestTrue(TEXT("Their view below -0.1 refuses"), Block(0, One, Trade()) == ECosmicTreatyBlock::Refused);
		Civs[0].Stockpile.Influence = 49.f;
		TestTrue(TEXT("(a refusal comes before the cost)"), Block(0, One, Trade()) == ECosmicTreatyBlock::Refused);
		Civs[0].Stockpile.Influence = 100.f;
		Civs[1].Relations.Add(Civs[0].CivID, -0.1f);
		TestTrue(TEXT("-0.1 accepts"), Block(0, One, Trade()) == ECosmicTreatyBlock::None);
		Civs[1].Relations.Add(Civs[0].CivID, -0.29f);
		Civs[0].ActiveEdicts = { FreeTrade };
		TestTrue(TEXT("An edict's acceptance bonus lowers the bar"), Block(0, One, Trade()) == ECosmicTreatyBlock::None);
		TestEqual(TEXT("(by its sum)"), CosmicDiplomacy::TreatyAcceptBonusOf(Civs[0], Tables), 0.2f, 1.0e-6f);
		Civs[0].ActiveEdicts.Reset();
		Civs[1].Relations.Reset();
		Civs[0].Stockpile.Influence = 49.f;
		TestTrue(TEXT("The cost last"), Block(0, One, Trade()) == ECosmicTreatyBlock::Cost);
		Civs[2].Ethics.Pacifism = -1.f;
		Civs[0].Stockpile.Influence = 100.f;
		TestTrue(TEXT("A people that does not border judges by the target (-0.2 here)"), Block(0, Civs[2].CivID, Trade()) == ECosmicTreatyBlock::Refused);
		Civs[2].Ethics = FEthicsAxes();
		Civs[0].Tier = EAscensionTier::Tier1_Planetary;
	}

	// The first reason of a siege and of a guard; the odds.
	{
		Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		Civs[1].Tier = EAscensionTier::Tier2_Stellar;
		Civs[0].FleetPower = 1.f;
		auto Siege = [&](int32 SystemIndex, bool bAssume = false) { return CosmicConflict::SiegeBlockOf(*State, Index, Rules, 0, SystemIndex, bAssume); };
		const int32 Sanctuary = State->Systems.Num() - 1;
		TestTrue(TEXT("Siege: the sanctuary first"), Siege(Sanctuary) == ECosmicSiegeBlock::Sanctuary);
		TestTrue(TEXT("no system"), Siege(INDEX_NONE) == ECosmicSiegeBlock::NoSystem);
		TestTrue(TEXT("its own system"), Siege(1) == ECosmicSiegeBlock::Own);
		State->Systems[2].ActiveMegaproject = CosmicRows::ProjectSettle;
		TestTrue(TEXT("a system being settled"), Siege(2) == ECosmicSiegeBlock::NotSettled);
		State->Systems[2].ActiveMegaproject = NAME_None;
		TestTrue(TEXT("the owner's capital"), Siege(3) == ECosmicSiegeBlock::Capital);
		State->Systems[3].ActiveMegaproject = CosmicRows::ProjectSettle;
		State->Systems[4].ActiveMegaproject = CosmicRows::ProjectSettle;
		State->Civs[1].HomeSystemID = IDOf(*State, 2);
		State->Systems[3].ActiveMegaproject = NAME_None;
		State->Civs[1].HomeSystemID = IDOf(*State, 3);
		State->Systems[3].ActiveMegaproject = CosmicRows::ProjectSettle;
		TestTrue(TEXT("the owner's last settled system"), Siege(2) == ECosmicSiegeBlock::LastSettled);
		State->Systems[3].ActiveMegaproject = NAME_None;
		State->Systems[4].ActiveMegaproject = NAME_None;
		TestTrue(TEXT("a system no settled system of ours lies next to"), Siege(4) == ECosmicSiegeBlock::NotAdjacent);
		State->Civs[2].FleetTargetSystemID = IDOf(*State, 2);
		TestTrue(TEXT("another people's siege"), Siege(2) == ECosmicSiegeBlock::Besieged);
		State->Civs[3].FleetTargetSystemID = IDOf(*State, 4);
		TestTrue(TEXT("(the lane comes before another's siege)"), Siege(4) == ECosmicSiegeBlock::NotAdjacent);
		State->Civs[3].FleetTargetSystemID = NAME_None;
		{
			const TArray<FName> Owned = State->Civs[0].OwnedSystems;
			State->Civs[0].OwnedSystems.Add(IDOf(*State, 5));
			State->Civs[0].OwnedSystems.Add(IDOf(*State, 7));
			State->Systems[5].OwnerCivID = State->Civs[0].CivID;
			State->Systems[7].OwnerCivID = State->Civs[0].CivID;
			TestTrue(TEXT("(another's siege comes before the realm cap)"), Siege(2) == ECosmicSiegeBlock::Besieged);
			State->Civs[2].FleetTargetSystemID = NAME_None;
			Civs[0].FleetPower = 0.f;
			TestTrue(TEXT("(the realm cap comes before the fleet)"), Siege(2) == ECosmicSiegeBlock::RealmCap);
			Civs[0].FleetPower = 1.f;
			State->Civs[0].OwnedSystems = Owned;
			State->Systems[5].OwnerCivID = State->Civs[2].CivID;
			State->Systems[7].OwnerCivID = State->Civs[2].CivID;
			State->Civs[2].FleetTargetSystemID = IDOf(*State, 2);
		}
		State->Civs[1].FleetTargetSystemID = IDOf(*State, 2);
		State->Civs[2].FleetTargetSystemID = NAME_None;
		TestFalse(TEXT("(the owner's own guard is no siege)"), Siege(2) == ECosmicSiegeBlock::Besieged);
		State->Civs[1].FleetTargetSystemID = NAME_None;
		CosmicTests::Give(*State, 0, { 5 });
		TestTrue(TEXT("the realm cap (a conquest counts, #42) - 3 of 4 is free"), Siege(2) != ECosmicSiegeBlock::RealmCap);
		CosmicTests::Give(*State, 0, { 7 });
		State->Civs[2].OwnedSystems.Remove(IDOf(*State, 5));
		State->Civs[2].OwnedSystems.Remove(IDOf(*State, 7));
		TestTrue(TEXT("4 of 4 is the cap"), Siege(2) == ECosmicSiegeBlock::RealmCap);
		State->Systems[5].OwnerCivID = State->Civs[2].CivID;
		State->Systems[7].OwnerCivID = State->Civs[2].CivID;
		State->Civs[2].OwnedSystems.Add(IDOf(*State, 5));
		State->Civs[2].OwnedSystems.Add(IDOf(*State, 7));
		State->Civs[0].OwnedSystems.Remove(IDOf(*State, 5));
		State->Civs[0].OwnedSystems.Remove(IDOf(*State, 7));
		Civs[0].FleetPower = 0.f;
		TestTrue(TEXT("no fleet"), Siege(2) == ECosmicSiegeBlock::NoFleet);
		CosmicDiplomacy::Sign(*State, 0, 1, NonAggression());
		TestTrue(TEXT("(the fleet comes before a non-aggression treaty)"), Siege(2) == ECosmicSiegeBlock::NoFleet);
		CosmicDiplomacy::Break(*State, 0, 1, NonAggression());
		Civs[0].FleetPower = 1.f;
		Civs[1].Tier = EAscensionTier::Tier4_Transcendent;
		TestTrue(TEXT("no fleet that counts two rungs up"), Siege(2) == ECosmicSiegeBlock::NoFleet);
		Civs[1].Tier = EAscensionTier::Tier2_Stellar;
		CosmicDiplomacy::Sign(*State, 0, 1, NonAggression());
		TestTrue(TEXT("a non-aggression treaty"), Siege(2) == ECosmicSiegeBlock::NonAggression);
		CosmicDiplomacy::Break(*State, 0, 1, NonAggression());
		TestTrue(TEXT("not hostile, last"), Siege(2) == ECosmicSiegeBlock::NotHostile);
		TestTrue(TEXT("unless hostility is assumed"), Siege(2, true) == ECosmicSiegeBlock::None);
		CosmicDiplomacy::SetStanceOf(Civs[0], Civs[1].CivID, Hostile());
		TestTrue(TEXT("Hostile, the siege may stand"), Siege(2) == ECosmicSiegeBlock::None);

		auto Guard = [&](int32 SystemIndex) { return CosmicConflict::GuardBlockOf(*State, Index, Rules, 0, SystemIndex); };
		TestTrue(TEXT("Guard: the sanctuary first"), Guard(Sanctuary) == ECosmicGuardBlock::Sanctuary);
		TestTrue(TEXT("another's system"), Guard(2) == ECosmicGuardBlock::NotOwn);
		TestTrue(TEXT("its own"), Guard(1) == ECosmicGuardBlock::None);
		Civs[0].Tier = EAscensionTier::Tier1_Planetary;
		TestTrue(TEXT("no fleet on the first rung (the cap is 0)"), Guard(1) == ECosmicGuardBlock::NoFleet);
		Civs[0].Tier = EAscensionTier::Tier2_Stellar;

		// The odds: A / (A + D), the defender's share by its target.
		Civs[1].FleetPower = 1.f;
		TestEqual(TEXT("No target: half defends"), CosmicConflict::SiegeOddsOf(*State, Index, Rules, 0, 2), 1.f / 1.5f, 1.0e-6f);
		Civs[1].FleetTargetSystemID = IDOf(*State, 2);
		TestEqual(TEXT("Guarding it: all of it"), CosmicConflict::SiegeOddsOf(*State, Index, Rules, 0, 2), 0.5f, 1.0e-6f);
		Civs[1].FleetTargetSystemID = IDOf(*State, 3);
		TestEqual(TEXT("Guarding another: a quarter"), CosmicConflict::SiegeOddsOf(*State, Index, Rules, 0, 2), 1.f / 1.25f, 1.0e-6f);
		Civs[1].FleetTargetSystemID = NAME_None;
		Civs[1].Tier = EAscensionTier::Tier3_Causal;
		TestEqual(TEXT("One rung up, the attacker counts half"), CosmicConflict::SiegeOddsOf(*State, Index, Rules, 0, 2), 0.5f / 1.f, 1.0e-6f);
		Civs[1].Tier = EAscensionTier::Tier1_Planetary;
		TestEqual(TEXT("The defender's rung is never a factor"), CosmicConflict::SiegeOddsOf(*State, Index, Rules, 0, 2), 1.f / 1.5f, 1.0e-6f);
		Civs[1].FleetPower = 0.f;
		TestEqual(TEXT("No defender: 1"), CosmicConflict::SiegeOddsOf(*State, Index, Rules, 0, 2), 1.f);
		Civs[0].FleetPower = 0.f;
		TestEqual(TEXT("No attacker: 0"), CosmicConflict::SiegeOddsOf(*State, Index, Rules, 0, 2), 0.f);
		TestFalse(TEXT("A tie stays with the defender"), CosmicConflict::Takes(0.5f, 0.5f));
		TestTrue(TEXT("below it the siege takes"), CosmicConflict::Takes(0.49f, 0.5f));
		TestEqual(TEXT("Bands from our side: 0.6 ahead"), CosmicConflict::OddsBandOf(0.6f), 2);
		TestEqual(TEXT("0.59 even"), CosmicConflict::OddsBandOf(0.59f), 1);
		TestEqual(TEXT("0.4 behind"), CosmicConflict::OddsBandOf(0.4f), 0);
	}
	return true;
}

// The diplomacy stage (header section 2.5): the drift of bordering pairs toward the target within [-1, 1], no entry at a
// rate of 0, the friendly upkeep and its lapses, the trade income, a transcendent's key unmoved, the ties of a people gone.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimDiplomacyStageTest, "Cosmic_Ascension.Sim.DiplomacyStage", CosmicTests::Flags)
bool FCosmicSimDiplomacyStageTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDiplomacyTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	// A rate of 0: no entry, whatever the targets.
	{
		FGalaxyState* State = LayBoard(*Sim, Original, [](FCosmicRulesDef& Rules) { Rules.RelationDriftRate = 0.f; });
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		State->Civs[1].Ethics.Pacifism = 1.f;
		for (int32 Tick = 0; Tick < 3; ++Tick)
		{
			Sim->StepOnce();
		}
		bool bNone = true;
		for (const FCivilizationState& Civ : Sim->GetState().Civs)
		{
			bNone &= Civ.Relations.IsEmpty() && Civ.Ties.IsEmpty();
		}
		TestTrue(TEXT("At a drift rate of 0 no relation entry is made"), bNone);
	}

	FGalaxyState* State = LayBoard(*Sim, Original, [](FCosmicRulesDef& Rules) { Rules.RelationDriftRate = 0.1f; Rules.RelationEthicsWeight = 1.f; });
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	TArray<FCivilizationState>& Civs = State->Civs;
	const FName Zero = Civs[0].CivID;
	const FName One = Civs[1].CivID;
	const FName Two = Civs[2].CivID;
	Civs[1].Ethics.Pacifism = 0.6f;
	Civs[2].Ethics.Pacifism = -1.f;
	Civs[2].Ethics.Spiritualism = 1.f;
	Civs[2].Ethics.Openness = 1.f;
	Civs[1].Relations.Add(Two, -0.99f);
	Civs[0].Relations.Add(Silence, 0.5f);
	const float Target01 = CosmicDiplomacy::TargetRelationOf(*State, Sim->GetRules(), 0, 1);
	Sim->SyncRngState();
	const FGalaxyState Before = *State;
	Sim->StepOnce();
	const FGalaxyState& After = Sim->GetState();
	TestEqual(TEXT("A bordering pair moves a tenth of the way to its target"), After.Civs[0].Relations.FindRef(One), 0.1f * Target01, 1.0e-6f);
	TestEqual(TEXT("(and the other way, toward its own target)"), After.Civs[1].Relations.FindRef(Zero), 0.1f * Target01, 1.0e-6f);
	TestFalse(TEXT("A pair that does not border gets no entry"), After.Civs[0].Relations.Contains(Two));
	TestEqual(TEXT("A drift past -1 stops at -1 (target -1.2, from -0.99)"), After.Civs[1].Relations.FindRef(Two), -1.f);
	TestEqual(TEXT("A transcendent's key does not drift"), After.Civs[0].Relations.FindRef(Silence), 0.5f);
	{
		// The draws of a tick with the drift running against the same tick at a rate of 0 (the stage returns before its table).
		const int32 Drifting = DrawsBetween(Before.RngState, After.RngState);
		const FCosmicDataTables Kept = Sim->GetTables();
		FCosmicRulesDef Still = Sim->GetRules();
		Still.RelationDriftRate = 0.f;
		Sim->SetTables(CosmicTests::WithRules(Kept, Still));
		TestEqual(TEXT("The stage draws nothing"), DrawsOfTick(*Sim, Before), Drifting);
		Sim->SetTables(Kept);
	}
	{
		FGalaxyState Plain = Before;
		for (FCivilizationState& Civ : Plain.Civs)
		{
			Civ.Relations.Reset();
			Civ.Ethics = FEthicsAxes();
		}
		TestEqual(TEXT("whatever the relations and the ethics"), DrawsOfTick(*Sim, Plain), DrawsOfTick(*Sim, Before));
	}

	// The friendly upkeep: paid from the stockpile after the income; a lapse to neutral, told for the ruled people only.
	{
		FGalaxyState Paying = Before;
		CosmicDiplomacy::SetStanceOf(Paying.Civs[0], One, Friendly());
		Paying.Civs[0].Stockpile.Influence = 10.f;
		FGalaxyState Free = Before;
		Free.Civs[0].Stockpile.Influence = 10.f;
		Sim->LoadState(Free);
		Sim->StepOnce();
		const float FreeInfluence = Sim->GetState().Civs[0].Stockpile.Influence;
		Sim->LoadState(Paying);
		Sim->StepOnce();
		TestEqual(TEXT("A friendly stance costs 0.2 Influence a tick"), Sim->GetState().Civs[0].Stockpile.Influence, FreeInfluence - 0.2f, 1.0e-4f);
		TestTrue(TEXT("and stands"), CosmicDiplomacy::StanceOf(Sim->GetState().Civs[0], One) == Friendly());
	}
	{
		FGalaxyState Lapsing = Before;
		CosmicDiplomacy::SetStanceOf(Lapsing.Civs[0], One, Friendly());
		CosmicDiplomacy::SetStanceOf(Lapsing.Civs[2], One, Friendly());
		Lapsing.Civs[0].Stockpile.Influence = 0.f;
		Lapsing.Civs[2].Stockpile.Influence = 0.f;
		Sim->LoadState(Lapsing);
		FCosmicRulesDef Dear = Sim->GetRules();
		Dear.FriendlyUpkeepInfluence = 1.0e6f;
		const FCosmicDataTables Kept = Sim->GetTables();
		Sim->SetTables(CosmicTests::WithRules(Kept, Dear));
		Sim->LoadState(Lapsing);
		const int32 Lines = Lapsing.Log.Num();
		Sim->StepOnce();
		TestFalse(TEXT("An upkeep that cannot be paid lapses"), Sim->GetState().Civs[0].Ties.Contains(One));
		const FCosmicLogEntry* Line = LastLineOf(Sim->GetState(), CosmicRows::LogStanceLapsed);
		TestTrue(TEXT("and the ruled people is told"), Line && Line->FocusCivID == Zero && Line->Severity == 1);
		TestEqual(TEXT("(once - the AI's lapse is not told)"), LinesOf(Sim->GetState(), CosmicRows::LogStanceLapsed) - LinesOf(Lapsing, CosmicRows::LogStanceLapsed), 1);
		TestTrue(TEXT("(new lines were told)"), Sim->GetState().Log.Num() > Lines);
		Sim->SetTables(Kept);
	}
	{
		FGalaxyState Crowded = Before;
		CosmicDiplomacy::SetStanceOf(Crowded.Civs[0], One, Friendly());
		CosmicDiplomacy::SetStanceOf(Crowded.Civs[0], Two, Friendly());
		Crowded.Civs[0].Stockpile.Influence = 10.f;
		const FName Later = FNameLexicalLess()(One, Two) ? Two : One;
		const FName Earlier = Later == One ? Two : One;
		Sim->LoadState(Crowded);
		Sim->StepOnce();
		TestFalse(TEXT("A friendly stance beyond the slots lapses, the later key first"), Sim->GetState().Civs[0].Ties.Contains(Later));
		TestTrue(TEXT("and the one within them stays"), CosmicDiplomacy::StanceOf(Sim->GetState().Civs[0], Earlier) == Friendly());
	}

	// The trade income: +0.2 Influence a treaty, in the income of the tick after it was signed.
	{
		FGalaxyState Trading = Before;
		CosmicDiplomacy::Sign(Trading, 0, 1, Trade());
		Sim->LoadState(Before);
		Sim->StepOnce();
		const float Plain = Sim->GetState().Civs[0].IncomePerTick.Influence;
		const float PlainOther = Sim->GetState().Civs[1].IncomePerTick.Influence;
		Sim->LoadState(Trading);
		Sim->StepOnce();
		TestEqual(TEXT("A trade treaty adds 0.2 to the Influence income"), Sim->GetState().Civs[0].IncomePerTick.Influence, Plain + 0.2f, 1.0e-5f);
		TestEqual(TEXT("(on both sides)"), Sim->GetState().Civs[1].IncomePerTick.Influence, PlainOther + 0.2f, 1.0e-5f);
	}

	// A people that leaves Civs leaves no tie behind.
	{
		FGalaxyState Leaving = Before;
		CosmicDiplomacy::Sign(Leaving, 0, 3, Trade());
		CosmicDiplomacy::SetStanceOf(Leaving.Civs[2], Leaving.Civs[3].CivID, Hostile());
		for (const FName SystemID : Leaving.Civs[3].OwnedSystems)
		{
			for (FStarSystemState& System : Leaving.Systems)
			{
				if (System.SystemID == SystemID)
				{
					System.OwnerCivID = NAME_None;
				}
			}
		}
		Leaving.Civs[3].OwnedSystems.Reset();
		const FName Three = Leaving.Civs[3].CivID;
		Sim->LoadState(Leaving);
		Sim->StepOnce();
		const FGalaxyState& Gone = Sim->GetState();
		TestEqual(TEXT("(the people without a system is gone)"), Gone.Civs.Num(), 3);
		TestTrue(TEXT("and no people keeps a tie to it"), !Gone.Civs[0].Ties.Contains(Three) && !Gone.Civs[2].Ties.Contains(Three));
		const float Income = Gone.Civs[0].IncomePerTick.Influence;
		Sim->StepOnce();
		TestEqual(TEXT("so its treaty brings no income from the next tick"), Sim->GetState().Civs[0].IncomePerTick.Influence, Income - 0.2f, 1.0e-5f);
	}
	return true;
}

// The four commands (header section 2.4): every refusal changes nothing; an acceptance pays and tells one line; a broken
// treaty lowers the partner's view; a hostile stance breaks the trade treaty; the fleet's target keeps its tick.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimDiplomacyCommandsTest, "Cosmic_Ascension.Sim.DiplomacyCommands", CosmicTests::Flags)
bool FCosmicSimDiplomacyCommandsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDiplomacyTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	UCosmicLogSubsystem* Log = Sim ? Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("(the role subsystem)"), Role) || !TestNotNull(TEXT("(the log subsystem)"), Log))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* State = LayBoard(*Sim, Original, Fleets);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	TArray<FCivilizationState>& Civs = State->Civs;
	const FName One = Civs[1].CivID;
	const FName Two = Civs[2].CivID;
	Civs[0].Stockpile.Influence = 100.f;

	// Refusals: nothing changes - the state, both streams.
	auto Unchanged = [&](const TCHAR* What, TFunctionRef<bool()> Refused)
	{
		Sim->SyncRngState();
		const uint32 Hash = Sim->ComputeStateHash();
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		const int32 Tell = State->TellRngState;
		TestTrue(What, Refused());
		TestTrue(*FString::Printf(TEXT("%s - and changes nothing"), What), Sim->ComputeStateHash() == Hash && Sim->GetRng().GetCurrentSeed() == Seed && State->TellRngState == Tell);
	};
	State->Knowing.MetCivs.RemoveAll([Two](const FKnownMark& Mark) { return Mark.ID == Two; });
	Unchanged(TEXT("A stance toward a people not met is NotMet"), [&] { return Role->SetStance(Two, Friendly()) == ECosmicStanceBlock::NotMet; });
	Unchanged(TEXT("A treaty with a people not met is NotMet"), [&] { return Role->ProposeTreaty(Two, Trade()) == ECosmicTreatyBlock::NotMet; });
	CosmicTests::SetKnowing(*State, {}, { 0, 1, 2, 3 });
	Unchanged(TEXT("Hostility on the first rung is Tier"), [&] { return Role->SetStance(One, Hostile()) == ECosmicStanceBlock::Tier; });
	Unchanged(TEXT("A treaty on the first rung is Tier"), [&] { return Role->ProposeTreaty(One, Trade()) == ECosmicTreatyBlock::Tier; });
	Unchanged(TEXT("A treaty not held is not broken"), [&] { return !Role->BreakTreaty(One, Trade()); });
	Unchanged(TEXT("A guard with no fleet (first rung) is NoFleet"), [&] { return Role->AllocateFleet(IDOf(*State, 1)) == ECosmicSiegeBlock::NoFleet; });
	Unchanged(TEXT("A siege of a system that is no neighbour is NotAdjacent"), [&] { return Role->AllocateFleet(IDOf(*State, 5)) == ECosmicSiegeBlock::NotAdjacent; });

	// A friendly stance: one line, no draw; the slot is then full.
	{
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		const int32 Lines = State->Log.Num();
		TestTrue(TEXT("The ruler turns friendly"), Role->SetStance(One, Friendly()) == ECosmicStanceBlock::None);
		TestTrue(TEXT("toward people 1"), CosmicDiplomacy::StanceOf(Civs[0], One) == Friendly());
		TestTrue(TEXT("with one line, Severity 1, of the ruled people"), State->Log.Num() == Lines + 1 && State->Log.Last().SourceRow == CosmicRows::LogStanceChanged
			&& State->Log.Last().Severity == 1 && State->Log.Last().FocusCivID == Civs[0].CivID);
		TestEqual(TEXT("and no draw"), Sim->GetRng().GetCurrentSeed(), Seed);
		Unchanged(TEXT("The second friendly stance on the first rung is FriendlySlots"), [&] { return Role->SetStance(Two, Friendly()) == ECosmicStanceBlock::FriendlySlots; });
		Unchanged(TEXT("The same stance again is Same"), [&] { return Role->SetStance(One, Friendly()) == ECosmicStanceBlock::Same; });
	}

	// The second rung: treaties, their cost and their acceptance.
	Civs[0].Tier = EAscensionTier::Tier2_Stellar;
	{
		Civs[1].Relations.Add(Civs[0].CivID, -0.25f);
		Unchanged(TEXT("A view of -0.25 refuses"), [&] { return Role->ProposeTreaty(One, NonAggression()) == ECosmicTreatyBlock::Refused; });
		Civs[0].ActiveEdicts = { FreeTrade };
		const int32 Lines = State->Log.Num();
		TestTrue(TEXT("Free trade lowers the bar to -0.3: accepted"), Role->ProposeTreaty(One, NonAggression()) == ECosmicTreatyBlock::None);
		TestEqual(TEXT("paying 50 Influence"), Civs[0].Stockpile.Influence, 50.f, 1.0e-4f);
		TestTrue(TEXT("signed on both sides"), CosmicDiplomacy::HasTreaty(Civs[1], Civs[0].CivID, NonAggression()));
		TestTrue(TEXT("with the signing's line"), State->Log.Num() == Lines + 1 && State->Log.Last().SourceRow == CosmicRows::LogTreatySigned);
		Unchanged(TEXT("Hostility under a non-aggression treaty is NonAggression"), [&] { return Role->SetStance(One, Hostile()) == ECosmicStanceBlock::NonAggression; });
		TestTrue(TEXT("A trade treaty too"), Role->ProposeTreaty(One, Trade()) == ECosmicTreatyBlock::None);
		Civs[0].Stockpile.Influence = 10.f;
		Unchanged(TEXT("A treaty held is Held"), [&] { return Role->ProposeTreaty(One, Trade()) == ECosmicTreatyBlock::Held; });
		Unchanged(TEXT("A treaty the stockpile cannot pay is Cost"), [&] { return Role->ProposeTreaty(Civs[3].CivID, Trade()) == ECosmicTreatyBlock::Cost; });
		Civs[0].ActiveEdicts.Reset();
		const float View = Civs[1].Relations.FindRef(Civs[0].CivID);
		TestTrue(TEXT("The ruler breaks the non-aggression treaty"), Role->BreakTreaty(One, NonAggression()));
		TestEqual(TEXT("and the partner thinks 0.3 less of it"), Civs[1].Relations.FindRef(Civs[0].CivID), View - 0.3f, 1.0e-6f);
		TestTrue(TEXT("with the breaking's line"), State->Log.Last().SourceRow == CosmicRows::LogTreatyBroken);
		TestTrue(TEXT("(the friendly stance lapses first: the ruler turns neutral)"), Role->SetStance(One, FGameplayTag()) == ECosmicStanceBlock::None);
		const int32 Before = State->Log.Num();
		const float ViewBefore = Civs[1].Relations.FindRef(Civs[0].CivID);
		TestTrue(TEXT("A hostile stance is taken"), Role->SetStance(One, Hostile()) == ECosmicStanceBlock::None);
		TestFalse(TEXT("and breaks the trade treaty"), CosmicDiplomacy::HasTreaty(Civs[1], Civs[0].CivID, Trade()));
		TestEqual(TEXT("which lowers the partner's view"), Civs[1].Relations.FindRef(Civs[0].CivID), ViewBefore - 0.3f, 1.0e-6f);
		TestEqual(TEXT("Two lines: the stance and the treaty"), State->Log.Num(), Before + 2);
	}

	// The fleet: a guard, a siege begun with its line, the same target keeps its tick, the withdrawal.
	{
		Civs[0].FleetPower = 1.f;
		State->Tick = 50;
		TestTrue(TEXT("A guard is taken"), Role->AllocateFleet(IDOf(*State, 1)) == ECosmicSiegeBlock::None);
		TestTrue(TEXT("and stands since now"), Civs[0].FleetTargetSystemID == IDOf(*State, 1) && Civs[0].FleetTargetSinceTick == 50);
		const int32 Lines = State->Log.Num();
		State->Tick = 60;
		TestTrue(TEXT("A siege is begun"), Role->AllocateFleet(IDOf(*State, 2)) == ECosmicSiegeBlock::None);
		TestTrue(TEXT("with one line, Severity 1"), State->Log.Num() == Lines + 1 && State->Log.Last().SourceRow == CosmicRows::LogSiegeBegun && State->Log.Last().Severity == 1);
		State->Tick = 70;
		TestTrue(TEXT("The same target again is taken"), Role->AllocateFleet(IDOf(*State, 2)) == ECosmicSiegeBlock::None);
		TestEqual(TEXT("and does not wind the siege back"), Civs[0].FleetTargetSinceTick, static_cast<int64>(60));
		TestTrue(TEXT("The withdrawal is always taken"), Role->AllocateFleet(NAME_None) == ECosmicSiegeBlock::None);
		TestTrue(TEXT("and starts no siege"), Civs[0].FleetTargetSystemID.IsNone() && Civs[0].FleetTargetSinceTick == 70);
	}

	// Only an embodied ruler; never from a handler.
	{
		// A treaty held with people 2: a command run where it should not would break it.
		CosmicDiplomacy::Sign(*State, 0, 2, Trade());
		const EPlayerRole Was = State->Player.Role;
		State->Player.Role = EPlayerRole::Wanderer;
		Unchanged(TEXT("A wanderer sets no stance"), [&] { return Role->SetStance(Two, Friendly()) == ECosmicStanceBlock::NotRuler; });
		Unchanged(TEXT("proposes no treaty"), [&] { return Role->ProposeTreaty(Two, NonAggression()) == ECosmicTreatyBlock::NotRuler; });
		Unchanged(TEXT("breaks none"), [&] { return !Role->BreakTreaty(Two, Trade()); });
		Unchanged(TEXT("and sends no fleet"), [&] { return Role->AllocateFleet(IDOf(*State, 1)) == ECosmicSiegeBlock::NotRuler; });
		State->Player.Role = Was;
		const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
		Sim->SetBroadcastEnabled(true);
		ON_SCOPE_EXIT { Sim->SetBroadcastEnabled(bWasBroadcasting); };
		CosmicTests::FScopedProbe Probe(*Sim);
		bool bTried = false;
		ECosmicStanceBlock Stance = ECosmicStanceBlock::None;
		ECosmicTreatyBlock Treaty = ECosmicTreatyBlock::None;
		ECosmicSiegeBlock Fleet = ECosmicSiegeBlock::None;
		bool bBroke = true;
		Probe.Probe->OnLog = [&](const FCosmicLogEntry&)
		{
			if (!bTried)
			{
				bTried = true;
				Stance = Role->SetStance(Two, Friendly());
				Treaty = Role->ProposeTreaty(Two, Trade());
				bBroke = Role->BreakTreaty(Two, Trade());
				Fleet = Role->AllocateFleet(IDOf(*State, 1));
			}
		};
		AddExpectedError(TEXT("SetStance called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		AddExpectedError(TEXT("ProposeTreaty called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		AddExpectedError(TEXT("BreakTreaty called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		AddExpectedError(TEXT("AllocateFleet called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		Log->Push(Tag(TEXT("Log.Civ")), 0, FText::AsCultureInvariant(TEXT("a line")), FFormatNamedArguments(), NAME_None, FName(TEXT("log_test")));
		TestTrue(TEXT("The handler ran"), bTried);
		TestTrue(TEXT("and every command answered Handler"), Stance == ECosmicStanceBlock::Handler && Treaty == ECosmicTreatyBlock::Handler && Fleet == ECosmicSiegeBlock::Handler && !bBroke);
		TestTrue(TEXT("(the treaty the handler tried to break is held)"), CosmicDiplomacy::HasTreaty(Civs[0], Two, Trade()));
	}
	return true;
}

// The fleets (header section 2.6 (1)): the cap is the rung's factor times the settled systems, the growth under paid upkeep
// and the held edicts' factor, the shrinking when it is not paid, the attrition inside a higher sphere only, never below 0.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimFleetTest, "Cosmic_Ascension.Sim.Fleet", CosmicTests::Flags)
bool FCosmicSimFleetTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDiplomacyTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* State = LayBoard(*Sim, Original, Fleets);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicRulesDef& Rules = Sim->GetRules();
	State->Civs[0].Tier = EAscensionTier::Tier2_Stellar;
	State->Civs[1].Tier = EAscensionTier::Tier2_Stellar;
	TestEqual(TEXT("The cap: 1 a settled system on the second rung"), CosmicConflict::FleetCapOf(*State, Index, 0, Rules), 2.f);
	TestEqual(TEXT("(three settled, three)"), CosmicConflict::FleetCapOf(*State, Index, 1, Rules), 3.f);
	State->Systems[4].ActiveMegaproject = CosmicRows::ProjectSettle;
	TestEqual(TEXT("A system being settled adds nothing"), CosmicConflict::FleetCapOf(*State, Index, 1, Rules), 2.f);
	State->Systems[4].ActiveMegaproject = NAME_None;
	TestEqual(TEXT("The first rung has no fleet"), CosmicConflict::FleetCapOf(*State, Index, 2, Rules), 0.f);
	Sim->SyncRngState();
	const FGalaxyState Start = *State;

	// Growth and upkeep.
	{
		Sim->LoadState(Start);
		Sim->StepOnce();
		TestEqual(TEXT("A fleet grows by 0.001 a tick"), Sim->GetState().Civs[0].FleetPower, 0.001f, 1.0e-7f);
		TestEqual(TEXT("not on the first rung"), Sim->GetState().Civs[2].FleetPower, 0.f);
		FGalaxyState Held = Start;
		Held.Civs[0].ActiveEdicts = { Mobilize };
		Sim->LoadState(Held);
		Sim->StepOnce();
		TestEqual(TEXT("A held edict multiplies the growth"), Sim->GetState().Civs[0].FleetPower, 0.0015f, 1.0e-7f);
		FGalaxyState Big = Start;
		Big.Civs[0].FleetPower = 2.5f;
		Big.Civs[0].Stockpile.Energy = 100.f;
		Sim->LoadState(Big);
		Sim->StepOnce();
		TestEqual(TEXT("A fleet above its cap (a system lost) comes down to it"), Sim->GetState().Civs[0].FleetPower, 2.f);
		FGalaxyState Paying = Start;
		Paying.Civs[0].FleetPower = 1.f;
		Paying.Civs[0].Stockpile.Energy = 100.f;
		FGalaxyState NotPaying = Paying;
		NotPaying.Civs[0].FleetPower = 0.f;
		Sim->LoadState(NotPaying);
		Sim->StepOnce();
		const float EnergyWithout = Sim->GetState().Civs[0].Stockpile.Energy;
		Sim->LoadState(Paying);
		Sim->StepOnce();
		TestEqual(TEXT("A fleet of 1 costs 0.5 Energy a tick"), Sim->GetState().Civs[0].Stockpile.Energy, EnergyWithout - 0.5f, 1.0e-3f);
		TestEqual(TEXT("and grows"), Sim->GetState().Civs[0].FleetPower, 1.001f, 1.0e-6f);
	}
	{
		FCosmicRulesDef Dear = Sim->GetRules();
		Dear.FleetUpkeepEnergyPerPower = 1.0e9f;
		const FCosmicDataTables Kept = Sim->GetTables();
		Sim->SetTables(CosmicTests::WithRules(Kept, Dear));
		FGalaxyState Broke = Start;
		Broke.Civs[0].FleetPower = 1.f;
		Broke.Civs[0].Stockpile.Energy = 5.f;
		FGalaxyState Empty = Broke;
		Empty.Civs[0].FleetPower = 0.f;
		Sim->LoadState(Empty);
		Sim->StepOnce();
		const float EnergyWithout = Sim->GetState().Civs[0].Stockpile.Energy;
		Sim->LoadState(Broke);
		Sim->StepOnce();
		TestEqual(TEXT("An upkeep that cannot be paid is not paid at all"), Sim->GetState().Civs[0].Stockpile.Energy, EnergyWithout, 1.0e-4f);
		TestEqual(TEXT("and the fleet shrinks by the growth"), Sim->GetState().Civs[0].FleetPower, 0.999f, 1.0e-6f);
		Sim->SetTables(Kept);
	}

	// The attrition: only the terms of an unsealed transcendent above the people's rung whose sphere holds the target.
	{
		FGalaxyState Inside = Start;
		CosmicTests::AddInstance(Inside, Silence, { IDOf(Inside, 1) });
		// On the third rung the owner presses 0.25 of its own: the stage must not count it.
		Inside.Civs[0].Tier = EAscensionTier::Tier3_Causal;
		Inside.Civs[0].FleetPower = 1.f;
		Inside.Civs[0].Stockpile.Energy = 100.f;
		Inside.Civs[0].FleetTargetSystemID = IDOf(Inside, 1);
		Sim->LoadState(Inside);
		const float P = UGalaxySimulationSubsystem::ComputeAttritionPressureIn(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0, 1).P;
		TestEqual(TEXT("A fourth-rung sphere presses 2 inside"), P, 2.f, 1.0e-6f);
		{
			// The owner's term and a wanderer's are pressure, not attrition: the owner on the third rung presses 0.25, and so
			// does a wanderer of the third rung standing there.
			FGalaxyState Pressed = Inside;
			Pressed.Civs[0].Tier = EAscensionTier::Tier3_Causal;
			Pressed.Player.Role = EPlayerRole::Wanderer;
			Pressed.Player.CurrentSystemID = IDOf(Pressed, 1);
			Pressed.Calling.LifeState = ELifeState::Embodied;
			Pressed.Calling.PersonalTier = EAscensionTier::Tier3_Causal;
			const FCosmicStateIndex& PressedIndex = Sim->GetStateIndex();
			const float Attrition = UGalaxySimulationSubsystem::ComputeAttritionPressureIn(Pressed, PressedIndex, Sim->GetTables(), Sim->GetRules(), 0, 1).P;
			const float Full = UGalaxySimulationSubsystem::ComputePressureIn(Pressed, PressedIndex, Sim->GetTables(), Sim->GetRules(), 1).P;
			TestEqual(TEXT("The attrition counts neither the owner's rung nor a wanderer"), Attrition, 2.f, 1.0e-6f);
			TestEqual(TEXT("(the full pressure counts both)"), Full, 2.5f, 1.0e-6f);
		}
		TestEqual(TEXT("A lane from the sphere is no attrition"),
			UGalaxySimulationSubsystem::ComputeAttritionPressureIn(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0, 0).P, 0.f);
		Sim->StepOnce();
		TestEqual(TEXT("A guard inside it loses 0.01 x 2 a tick"), Sim->GetState().Civs[0].FleetPower, 1.001f - 0.02f, 1.0e-6f);
		FGalaxyState Peer = Inside;
		Peer.Civs[0].Tier = EAscensionTier::Tier4_Transcendent;
		Sim->LoadState(Peer);
		TestEqual(TEXT("A sphere of the people's own rung is no attrition"),
			UGalaxySimulationSubsystem::ComputeAttritionPressureIn(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0, 1).P, 0.f);
		FGalaxyState Thin = Inside;
		Thin.Civs[0].FleetPower = 0.005f;
		Sim->LoadState(Thin);
		Sim->StepOnce();
		TestEqual(TEXT("The attrition stops at 0"), Sim->GetState().Civs[0].FleetPower, 0.f);
		FGalaxyState Sealed = Inside;
		Sealed.Transcendents[0].bSealed = true;
		Sim->LoadState(Sealed);
		TestEqual(TEXT("A sealed one presses nothing"),
			UGalaxySimulationSubsystem::ComputeAttritionPressureIn(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0, 1).P, 0.f);
	}
	return true;
}

// The sieges (header section 2.6): one number for each siege at its end, all drawn first - a siege that loses its gate on
// that tick too; the gate asked again right before each end; taken, held, lost and failed with their lines; a target lifted
// by its gate, by a destruction and by a taking; one siege on a system; a shorter knob ends a standing siege at once.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSiegeTest, "Cosmic_Ascension.Sim.Siege", CosmicTests::Flags)
bool FCosmicSimSiegeTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDiplomacyTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* State = LayBoard(*Sim, Original, [](FCosmicRulesDef& Rules)
	{
		Fleets(Rules);
		Rules.SiegeTicks = 5;
		Rules.SiegeRestTicks = 1000000;
	});
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	for (FCivilizationState& Civ : State->Civs)
	{
		Civ.Tier = EAscensionTier::Tier2_Stellar;
		Civ.Stockpile.Energy = 1000.f;
		Civ.FleetPower = 1.f;
	}
	State->Tick = 100;
	Sim->SyncRngState();
	const FGalaxyState Start = *State;
	const FName Zero = Start.Civs[0].CivID;
	const FName One = Start.Civs[1].CivID;

	// The draws: one for each siege that reaches its end on the tick, before any end is applied.
	{
		const int32 Base = DrawsOfTick(*Sim, Start);
		FGalaxyState Standing = Start;
		Besiege(Standing, 0, 1, 2, 100);
		TestEqual(TEXT("A siege short of its end draws nothing"), DrawsOfTick(*Sim, Standing), Base);
		FGalaxyState Due = Start;
		Besiege(Due, 0, 1, 2, 96);
		TestEqual(TEXT("A siege at its end draws one number"), DrawsOfTick(*Sim, Due), Base + 1);
		FGalaxyState Late = Start;
		Besiege(Late, 0, 1, 2, 10);
		TestEqual(TEXT("one long past its end too"), DrawsOfTick(*Sim, Late), Base + 1);
		FGalaxyState Two = Due;
		Besiege(Two, 2, 1, 4, 96);
		TestEqual(TEXT("Two sieges at their end draw two"), DrawsOfTick(*Sim, Two), Base + 2);
		FGalaxyState Closed = Due;
		CosmicDiplomacy::Sign(Closed, 0, 1, NonAggression());
		TestEqual(TEXT("A siege that loses its gate on its last tick still draws its number"), DrawsOfTick(*Sim, Closed), Base + 1);
		TestTrue(TEXT("and is lifted"), Sim->GetState().Civs[0].FleetTargetSystemID.IsNone() && Sim->GetState().Civs[0].FleetTargetSinceTick == 101);
		TestTrue(TEXT("with the lifting's line to the ruled side"), LastLineOf(Sim->GetState(), CosmicRows::LogSiegeLifted) != nullptr);
		TestEqual(TEXT("(the system is still people 1's)"), Sim->GetState().Systems[2].OwnerCivID, One);
	}

	// Taken: the odds are 1 when no fleet defends.
	{
		FGalaxyState Sure = Start;
		Besiege(Sure, 0, 1, 2, 96);
		Sure.Civs[1].FleetPower = 0.f;
		Sure.Civs[1].Relations.Add(Zero, 0.1f);
		Sure.Civs[2].Relations.Add(Zero, 0.1f);
		Unmeet(Sure, 1, 2);
		Sim->LoadState(Sure);
		Sim->StepOnce();
		const FGalaxyState& After = Sim->GetState();
		TestEqual(TEXT("The siege takes the system"), After.Systems[2].OwnerCivID, Zero);
		TestTrue(TEXT("The attacker turns neutral and rests"), !After.Civs[0].Ties.Contains(One) && After.Civs[0].FleetTargetSystemID.IsNone() && After.Civs[0].FleetTargetSinceTick == 101);
		TestEqual(TEXT("The loser thinks 0.2 less of the conqueror"), After.Civs[1].Relations.FindRef(Zero), 0.1f - 0.2f, 1.0e-6f);
		TestEqual(TEXT("A people that does not border the conqueror does not"), After.Civs[2].Relations.FindRef(Zero), 0.1f, 1.0e-6f);
		const FCosmicLogEntry* Line = LastLineOf(After, CosmicRows::LogSiegeTaken);
		TestTrue(TEXT("The ruled conqueror is told, Severity 2"), Line && Line->Severity == 2 && Line->FocusCivID == Zero && Line->FocusSystemID == IDOf(After, 2));
		TestNull(TEXT("and not the loser's line"), LastLineOf(After, CosmicRows::LogSiegeLost));
		TestTrue(TEXT("The taken system is known (FollowUpKnowing)"), Knows(After, 2));
		TestTrue(TEXT("and the loser met"), HasMet(After, One));
		const int32 Met = LineIndexOf(After, CosmicRows::LogContact, One);
		TestTrue(TEXT("before the line"), Met != INDEX_NONE && Met < LineIndexOf(After, CosmicRows::LogSiegeTaken));
	}

	// The loser thinks less of the conqueror even when the taking leaves them no border.
	{
		FGalaxyState CutOff = Start;
		Besiege(CutOff, 0, 1, 2, 96);
		CutOff.Civs[1].FleetPower = 0.f;
		CutOff.Civs[1].Relations.Add(Zero, 0.1f);
		CutOff.Systems[3].ActiveMegaproject = CosmicRows::ProjectSettle;
		Sim->LoadState(CutOff);
		Sim->StepOnce();
		const FGalaxyState& After = Sim->GetState();
		TestEqual(TEXT("(people 0 takes 2)"), After.Systems[2].OwnerCivID, Zero);
		TestFalse(TEXT("(and the loser no longer borders it)"), CosmicDiplomacy::Borders(After, Sim->GetStateIndex(), 1, 0));
		TestEqual(TEXT("The loser always counts"), After.Civs[1].Relations.FindRef(Zero), 0.1f - 0.2f, 1.0e-6f);
	}

	// Lost: the guard of the old owner is lifted with the taking and turns into no siege.
	{
		FGalaxyState Losing = Start;
		Besiege(Losing, 1, 0, 1, 96);
		Losing.Civs[0].FleetPower = 0.f;
		Losing.Civs[0].FleetTargetSystemID = IDOf(Losing, 1);
		Losing.Civs[0].FleetTargetSinceTick = 50;
		Losing.Player.CurrentSystemID = IDOf(Losing, 1);
		Unmeet(Losing, 1);
		Sim->LoadState(Losing);
		Sim->StepOnce();
		const FGalaxyState& After = Sim->GetState();
		TestEqual(TEXT("The ruled people loses the system"), After.Systems[1].OwnerCivID, One);
		TestTrue(TEXT("its guard is lifted, no siege of its own"), After.Civs[0].FleetTargetSystemID.IsNone() && After.Civs[0].FleetTargetSinceTick == 101);
		const FCosmicLogEntry* Line = LastLineOf(After, CosmicRows::LogSiegeLost);
		TestTrue(TEXT("and it is told the loss, Severity 2"), Line && Line->Severity == 2 && Line->FocusCivID == Zero);
		TestNull(TEXT("(not the conqueror's line)"), LastLineOf(After, CosmicRows::LogSiegeTaken));
		TestEqual(TEXT("The ruler's vessel on the lost system goes to the capital"), After.Player.CurrentSystemID, IDOf(After, 0));
		const int32 Met = LineIndexOf(After, CosmicRows::LogContact, One);
		TestTrue(TEXT("The conqueror is met before the line"), HasMet(After, One) && Met != INDEX_NONE && Met < LineIndexOf(After, CosmicRows::LogSiegeLost));
	}

	// Held and failed: the odds near 0 when the attacker is small.
	{
		FGalaxyState Held = Start;
		Besiege(Held, 1, 0, 1, 96);
		Held.Civs[1].FleetPower = 0.0001f;
		Held.Civs[0].FleetPower = 3.f;
		Held.Civs[0].FleetTargetSystemID = IDOf(Held, 1);
		Unmeet(Held, 1);
		Sim->LoadState(Held);
		Sim->StepOnce();
		const FGalaxyState& After = Sim->GetState();
		TestEqual(TEXT("A siege that fails leaves the owner"), After.Systems[1].OwnerCivID, Zero);
		TestEqual(TEXT("and halves the attacker's fleet (after this tick's growth)"), After.Civs[1].FleetPower, (0.0001f + 0.001f) * 0.5f, 1.0e-7f);
		TestTrue(TEXT("The ruled defender is told it held"), LastLineOf(After, CosmicRows::LogSiegeHeld) != nullptr && LastLineOf(After, CosmicRows::LogSiegeHeld)->Severity == 2);
		TestTrue(TEXT("and keeps its guard"), After.Civs[0].FleetTargetSystemID == IDOf(After, 1));
		TestTrue(TEXT("The attacker is met before the line"), LineIndexOf(After, CosmicRows::LogContact, One) != INDEX_NONE
			&& LineIndexOf(After, CosmicRows::LogContact, One) < LineIndexOf(After, CosmicRows::LogSiegeHeld));
		FGalaxyState Failed = Start;
		Besiege(Failed, 0, 1, 2, 96);
		Failed.Civs[0].FleetPower = 0.0001f;
		Failed.Civs[1].FleetPower = 3.f;
		Failed.Civs[1].FleetTargetSystemID = IDOf(Failed, 2);
		Unmeet(Failed, 1);
		Sim->LoadState(Failed);
		Sim->StepOnce();
		TestTrue(TEXT("The ruled attacker is told it failed"), LastLineOf(Sim->GetState(), CosmicRows::LogSiegeFailed) != nullptr);
		TestEqual(TEXT("(the fleet x0.5, after this tick's growth)"), Sim->GetState().Civs[0].FleetPower, (0.0001f + 0.001f) * 0.5f, 1.0e-7f);
		TestTrue(TEXT("The defender is met before the line"), LineIndexOf(Sim->GetState(), CosmicRows::LogContact, One) != INDEX_NONE
			&& LineIndexOf(Sim->GetState(), CosmicRows::LogContact, One) < LineIndexOf(Sim->GetState(), CosmicRows::LogSiegeFailed));
	}

	// The gate asked again before each end: the first taking closes the second siege's lane.
	{
		FCosmicRulesDef NoShare = Sim->GetRules();
		NoShare.SiegeDefenceShares = { 1.f, 0.5f, 0.f };
		const FCosmicDataTables Kept = Sim->GetTables();
		Sim->SetTables(CosmicTests::WithRules(Kept, NoShare));
		FGalaxyState Crossing = Start;
		Besiege(Crossing, 0, 1, 2, 96);
		Besiege(Crossing, 1, 0, 1, 96);
		Sim->LoadState(Crossing);
		Sim->StepOnce();
		const FGalaxyState& After = Sim->GetState();
		TestEqual(TEXT("People 0 comes first and takes 2"), After.Systems[2].OwnerCivID, Zero);
		TestEqual(TEXT("so people 1 no longer lies next to 1: its siege is lifted"), After.Systems[1].OwnerCivID, Zero);
		TestEqual(TEXT("(lifted, not ended: its fleet is whole after this tick's growth)"), After.Civs[1].FleetPower, 1.001f, 1.0e-6f);
		TestTrue(TEXT("(and its target is gone)"), After.Civs[1].FleetTargetSystemID.IsNone());
		TestTrue(TEXT("No end was told for it"), LineIndexOf(After, CosmicRows::LogSiegeHeld) == INDEX_NONE && LineIndexOf(After, CosmicRows::LogSiegeLost) == INDEX_NONE);
		TestTrue(TEXT("but its lifting"), LineIndexOf(After, CosmicRows::LogSiegeLifted) != INDEX_NONE);
		Sim->SetTables(Kept);
	}

	// A guard whose gate closes is lifted by the check, with no line.
	{
		FCosmicRulesDef NoFleet = Sim->GetRules();
		NoFleet.FleetPerSettledByTier = { 0.f, 0.f, 0.f, 0.f, 0.f };
		const FCosmicDataTables Kept = Sim->GetTables();
		Sim->SetTables(CosmicTests::WithRules(Kept, NoFleet));
		FGalaxyState Guarding = Start;
		Guarding.Civs[0].FleetTargetSystemID = IDOf(Guarding, 1);
		Guarding.Civs[0].FleetTargetSinceTick = 50;
		Sim->LoadState(Guarding);
		Sim->StepOnce();
		TestTrue(TEXT("A guard with no fleet to give is lifted"), Sim->GetState().Civs[0].FleetTargetSystemID.IsNone() && Sim->GetState().Civs[0].FleetTargetSinceTick == 101);
		TestEqual(TEXT("untold"), LineIndexOf(Sim->GetState(), CosmicRows::LogSiegeLifted), INDEX_NONE);
		Sim->SetTables(Kept);
	}

	// A target lifted by a destruction, and one siege on a system.
	{
		FGalaxyState Doomed = Start;
		Besiege(Doomed, 0, 1, 2, 100);
		Doomed.Civs[1].FleetTargetSystemID = IDOf(Doomed, 2);
		Besiege(Doomed, 2, 1, 4, 100);
		Sim->LoadState(Doomed);
		TestTrue(TEXT("A second siege on a system is Besieged (4: people 2 is there first, people 3 lies next to it too)"),
			CosmicConflict::SiegeBlockOf(Sim->GetState(), Sim->GetStateIndex(), Sim->GetRules(), 3, 4, true) == ECosmicSiegeBlock::Besieged);
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		CosmicHazards::Destroy(Ctx, 2);
		TestTrue(TEXT("A destruction lifts every target on the system"), Sim->GetState().Civs[0].FleetTargetSystemID.IsNone() && Sim->GetState().Civs[1].FleetTargetSystemID.IsNone());
		TestEqual(TEXT("(the rest starts now)"), Sim->GetState().Civs[0].FleetTargetSinceTick, Sim->GetState().Tick);
		Sim->SyncRngState();
		const FGalaxyState After = Sim->GetState();
		// A target left on the dead system (as if nothing had lifted it), at its end: no owner, no draw - and the check lifts it.
		FGalaxyState Left = After;
		Left.Civs[0].FleetTargetSystemID = IDOf(Left, 2);
		Left.Civs[0].FleetTargetSinceTick = 96;
		const int32 Clean = DrawsOfTick(*Sim, After);
		TestEqual(TEXT("A target on a system with no owner draws nothing at its end"), DrawsOfTick(*Sim, Left), Clean);
		TestTrue(TEXT("and is lifted by the check"), Sim->GetState().Civs[0].FleetTargetSystemID.IsNone());
	}

	// A shorter knob ends a standing siege at the next stage.
	{
		FGalaxyState Long = Start;
		Besiege(Long, 0, 1, 2, 99);
		const int32 Base = DrawsOfTick(*Sim, Start);
		TestEqual(TEXT("(5 ticks: not due)"), DrawsOfTick(*Sim, Long), Base);
		FCosmicRulesDef Short = Sim->GetRules();
		Short.SiegeTicks = 1;
		const FCosmicDataTables Kept = Sim->GetTables();
		Sim->SetTables(CosmicTests::WithRules(Kept, Short));
		TestEqual(TEXT("A shorter knob ends the standing siege at the next stage, with one number"), DrawsOfTick(*Sim, Long), Base + 1);
		Short.SiegeTicks = 0;
		Sim->SetTables(CosmicTests::WithRules(Kept, Short));
		FGalaxyState Fresh = Start;
		Besiege(Fresh, 0, 1, 2, 101);
		TestEqual(TEXT("A knob of 0 reads as 1: a siege laid on the stage's own tick is not yet due"), DrawsOfTick(*Sim, Fresh), Base);
		Sim->SetTables(Kept);
	}
	return true;
}

// The fleet's delay (header section 2.7 (1)): a hazard placed on a guarded system gains FleetDelayPerPower x the effective
// power within the cap; a rebellion never; a standing hazard never; a claim by the gap to its claimant.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimFleetDelayTest, "Cosmic_Ascension.Sim.FleetDelay", CosmicTests::Flags)
bool FCosmicSimFleetDelayTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDiplomacyTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* State = LayBoard(*Sim, Original, Fleets);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	FCivilizationState& Ruled = State->Civs[0];
	Ruled.Tier = EAscensionTier::Tier2_Stellar;
	Ruled.FleetPower = 1.f;
	auto PlaceOn = [&](int32 SystemIndex, FName Row)
	{
		State->Systems[SystemIndex].ActiveHazard = FGameplayTag();
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		return CosmicHazards::Place(Ctx, SystemIndex, Row) ? State->Systems[SystemIndex].HazardTicksRemaining : INDEX_NONE;
	};
	TestEqual(TEXT("Unguarded: the row's countdown"), PlaceOn(1, Sick), 200);
	Ruled.FleetTargetSystemID = IDOf(*State, 1);
	TestEqual(TEXT("Guarded by 1: +50"), PlaceOn(1, Sick), 250);
	Ruled.FleetPower = 3.f;
	TestEqual(TEXT("by 3: the cap of 100"), PlaceOn(1, Sick), 300);
	TestEqual(TEXT("Another system gains nothing"), PlaceOn(0, Sick), 200);
	Ruled.FleetTargetSystemID = IDOf(*State, 0);
	State->Systems[0].ActiveHazard = FGameplayTag();
	TestEqual(TEXT("A rebellion in a guarded capital is not put off"), PlaceOn(0, Unrest), 200);
	State->Systems[1].ActiveHazard = Tag(TEXT("Hazard.Plague"));
	State->Systems[1].HazardTicksRemaining = 7;
	Ruled.FleetTargetSystemID = IDOf(*State, 1);
	Sim->SyncRngState();
	Sim->StepOnce();
	TestEqual(TEXT("A standing hazard's countdown never changes"), Sim->GetState().Systems[1].HazardTicksRemaining, 6);
	State = &Sim->GetMutableState();
	{
		FCivilizationState& Again = State->Civs[0];
		Again.FleetPower = 1.f;
		State->Systems[1].ActiveHazard = FGameplayTag();
		CosmicTests::AddInstance(*State, Silence, { IDOf(*State, 2) });
		TestEqual(TEXT("A claim of a source two rungs up is not put off"), PlaceOn(1, Claimed), 300);
		Again.Tier = EAscensionTier::Tier3_Causal;
		TestEqual(TEXT("one rung up, by half"), PlaceOn(1, Claimed), 325);
		State->Transcendents.Reset();
		Again.Tier = EAscensionTier::Tier2_Stellar;
	}
	TestEqual(TEXT("The delay of a hazard row is FleetDelayOf's"), CosmicConflict::FleetDelayOf(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 1,
		*Sim->GetTables().FindHazard(Sick)), 50);
	TestEqual(TEXT("The countdown with the delay is still cut at the int32 limit"), PlaceOn(1, Endless), MAX_int32);
	{
		FCosmicRulesDef Below = Sim->GetRules();
		Below.FleetDelayPerPower = -50.f;
		const FCosmicDataTables Kept = Sim->GetTables();
		Sim->SetTables(CosmicTests::WithRules(Kept, Below));
		TestEqual(TEXT("A knob below 0 puts nothing off and takes nothing away"), PlaceOn(1, Sick), 200);
		Sim->SetTables(Kept);
	}
	return true;
}

// The plague's spread (header section 2.7 (2)): the plague share of a system next to a standing plague times the multiplier,
// read from a table built as the step begins; the number of draws unchanged; a held factor of 0 keeps it out.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimPlagueSpreadTest, "Cosmic_Ascension.Sim.PlagueSpread", CosmicTests::Flags)
bool FCosmicSimPlagueSpreadTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDiplomacyTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	// A chance of 0.02 times 50 is 1: next to a plague, certain; elsewhere one in fifty.
	FGalaxyState* State = LayBoard(*Sim, Original, [](FCosmicRulesDef& Rules) { Rules.PlagueSpreadMultiplier = 50.f; }, 0.02f);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const FGameplayTag Plague = Tag(TEXT("Hazard.Plague"));
	State->Systems[0].ActiveHazard = Plague;
	State->Systems[0].HazardTicksRemaining = 100;
	Sim->SyncRngState();
	const FGalaxyState Start = *State;
	Sim->StepOnce();
	TestTrue(TEXT("A system next to a plague takes it"), Sim->GetState().Systems[1].ActiveHazard == Plague);
	TestFalse(TEXT("The table is built as the step begins: 2, next to the plague placed on 1 in the same step, is not certain"),
		Sim->GetState().Systems[2].ActiveHazard == Plague);
	{
		const FCosmicDataTables Kept = Sim->GetTables();
		FCosmicRulesDef Plain = Sim->GetRules();
		Plain.PlagueSpreadMultiplier = 1.f;
		Sim->SetTables(CosmicTests::WithRules(Kept, Plain));
		const int32 Without = DrawsOfTick(*Sim, Start);
		Sim->SetTables(Kept);
		TestEqual(TEXT("The spread changes the shares, never the draws"), DrawsOfTick(*Sim, Start), Without);
		Plain.PlagueSpreadMultiplier = 0.f;
		Sim->SetTables(CosmicTests::WithRules(Kept, Plain));
		Sim->LoadState(Start);
		Sim->StepOnce();
		TestFalse(TEXT("A multiplier of 0 keeps it from a neighbour"), Sim->GetState().Systems[1].ActiveHazard == Plague);
		Sim->SetTables(Kept);
	}
	{
		FGalaxyState Quarantined = Start;
		Quarantined.Civs[0].ActiveEdicts = { Shut };
		Sim->LoadState(Quarantined);
		Sim->StepOnce();
		TestFalse(TEXT("A held edict's factor of 0 keeps out the plague that spreads"), Sim->GetState().Systems[1].ActiveHazard == Plague);
	}
	return true;
}

// The AI's diplomacy (header section 2.8): no friendly stance and no guard; a hostility only with a siege - every gate in
// turn, the siege at once, the line to the ruled people; a hostility without a siege ends; the rest; the ruled people is
// passed by; no draw; the coronation lifts what the new ruler could not see.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimDiplomacyAITest, "Cosmic_Ascension.Sim.DiplomacyAI", CosmicTests::Flags)
bool FCosmicSimDiplomacyAITest::RunTest(const FString& Parameters)
{
	using namespace CosmicDiplomacyTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* State = LayBoard(*Sim, Original, [](FCosmicRulesDef& Rules)
	{
		Fleets(Rules);
		Rules.SiegeRestTicks = 50;
		Rules.SiegeTicks = 1000;
		// The gate at -0.3 and 1.5, toward the ruled people too (the cases below read them; the game's values are the data's).
		Rules.AIHostileRelation = -0.3f;
		Rules.AIHostilePowerRatio = 1.5f;
		Rules.AIHostileRelationTowardRuled = -0.3f;
		Rules.AIHostilePowerRatioTowardRuled = 1.5f;
	});
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	for (FCivilizationState& Civ : State->Civs)
	{
		Civ.Tier = EAscensionTier::Tier2_Stellar;
		Civ.Stockpile.Energy = 1000.f;
		Civ.FleetPower = 1.f;
	}
	// People 1 sees people 0 at -0.4 and has the fleet to go: its gate is open toward 0 (system 1).
	State->Civs[1].FleetPower = 2.f;
	State->Civs[1].Ethics.Pacifism = -0.5f;
	State->Civs[1].Relations.Add(State->Civs[0].CivID, -0.4f);
	State->Tick = 100;
	Sim->SyncRngState();
	const FGalaxyState Start = *State;
	const FName Zero = Start.Civs[0].CivID;
	const FName One = Start.Civs[1].CivID;

	{
		FGalaxyState Unmet = Start;
		Unmeet(Unmet, 1);
		Sim->LoadState(Unmet);
		Sim->StepOnce();
		const FGalaxyState& After = Sim->GetState();
		TestTrue(TEXT("The AI turns hostile"), CosmicDiplomacy::StanceOf(After.Civs[1], Zero) == Hostile());
		TestTrue(TEXT("and lays its siege at once on the first system it may take"), After.Civs[1].FleetTargetSystemID == IDOf(After, 1) && After.Civs[1].FleetTargetSinceTick == 101);
		const FCosmicLogEntry* Line = LastLineOf(After, CosmicRows::LogSiegeOnUs);
		TestTrue(TEXT("The ruled people is told, Severity 2"), Line && Line->Severity == 2 && Line->FocusCivID == Zero && Line->FocusSystemID == IDOf(After, 1));
		const int32 Met = LineIndexOf(After, CosmicRows::LogContact, One);
		TestTrue(TEXT("after it has met the besieger"), HasMet(After, One) && Met != INDEX_NONE && Met < LineIndexOf(After, CosmicRows::LogSiegeOnUs));
		TestEqual(TEXT("(no treaty to break, no such line)"), LineIndexOf(After, CosmicRows::LogTreatyBroken), INDEX_NONE);
	}

	// The opening breaks the pair's trade treaty on both sides, with no penalty, and tells it first.
	{
		FGalaxyState Trading = Start;
		CosmicDiplomacy::Sign(Trading, 0, 1, CosmicTags::TreatyTrade());
		Sim->LoadState(Trading);
		Sim->StepOnce();
		const FGalaxyState& After = Sim->GetState();
		TestTrue(TEXT("(the AI opens)"), CosmicDiplomacy::StanceOf(After.Civs[1], Zero) == Hostile());
		TestTrue(TEXT("The trade treaty is broken on both sides"), !CosmicDiplomacy::HasTreaty(After.Civs[0], One, CosmicTags::TreatyTrade())
			&& !CosmicDiplomacy::HasTreaty(After.Civs[1], Zero, CosmicTags::TreatyTrade()));
		TestFalse(TEXT("with no penalty (the AI's opening)"), After.Civs[0].Relations.Contains(One));
		const int32 Broken = LineIndexOf(After, CosmicRows::LogTreatyBroken, Zero);
		TestTrue(TEXT("The ruled people is told of the treaty before the siege"), Broken != INDEX_NONE && Broken < LineIndexOf(After, CosmicRows::LogSiegeOnUs));
	}

	// The world reads no knowing: the same start with every system known and every people met, and with nothing known, runs
	// to the same world and the same sim stream through an opening and a siege's end (Sim.KnowingInert's form).
	{
		FCosmicRulesDef Quick = Sim->GetRules();
		Quick.SiegeTicks = 10;
		const FCosmicDataTables Kept = Sim->GetTables();
		Sim->SetTables(CosmicTests::WithRules(Kept, Quick));
		auto RunFrom = [&](const FGalaxyState& From)
		{
			Sim->LoadState(From);
			for (int32 Tick = 0; Tick < 15; ++Tick)
			{
				Sim->StepOnce();
			}
			Sim->SyncRngState();
			return Sim->GetState();
		};
		const FGalaxyState Seeing = RunFrom(Start);
		FGalaxyState Dark = Start;
		Dark.Knowing = FKnowingState();
		const FGalaxyState Blind = RunFrom(Dark);
		Sim->LoadState(Seeing);
		const uint32 SeeingWorld = Sim->ComputeWorldHash();
		Sim->LoadState(Blind);
		const uint32 BlindWorld = Sim->ComputeWorldHash();
		TestEqual(TEXT("The world is the same whatever the calling knows"), BlindWorld, SeeingWorld);
		TestEqual(TEXT("and so is the sim stream"), Blind.RngState, Seeing.RngState);
		TestTrue(TEXT("(a siege ended in the run: the case can tell)"), Seeing.Civs[1].FleetTargetSystemID.IsNone() && Seeing.Civs[1].FleetTargetSinceTick == 111);
		TestTrue(TEXT("(and the two runs knew differently)"), Blind.Knowing.KnownSystems.Num() < Seeing.Knowing.KnownSystems.Num());
		Sim->SetTables(Kept);
	}
	{
		FGalaxyState Closed = Start;
		Closed.Civs[1].Relations.Add(Zero, -0.4f);
		TestEqual(TEXT("The AI's diplomacy draws nothing"), DrawsOfTick(*Sim, Closed), [&]() { FGalaxyState Idle = Start; Idle.Civs[1].FleetPower = 0.f; return DrawsOfTick(*Sim, Idle); }());
	}

	// Each gate closed in turn.
	auto StaysNeutral = [&](const TCHAR* What, TFunctionRef<void(FGalaxyState&)> Close)
	{
		FGalaxyState Case = Start;
		Close(Case);
		Sim->LoadState(Case);
		Sim->StepOnce();
		TestFalse(What, CosmicDiplomacy::StanceOf(Sim->GetState().Civs[1], Zero) == Hostile());
	};
	StaysNeutral(TEXT("A non-aggression treaty"), [](FGalaxyState& Case) { CosmicDiplomacy::Sign(Case, 0, 1, CosmicTags::TreatyNonAggression()); });
	StaysNeutral(TEXT("A view above -0.3"), [Zero](FGalaxyState& Case) { Case.Civs[1].Relations.Add(Zero, -0.29f); });
	StaysNeutral(TEXT("A fleet below 1.5 times theirs"), [](FGalaxyState& Case) { Case.Civs[1].FleetPower = 1.4f; });
	StaysNeutral(TEXT("No fleet at all"), [](FGalaxyState& Case) { Case.Civs[1].FleetPower = 0.f; Case.Civs[0].FleetPower = 0.f; });
	StaysNeutral(TEXT("A pacifism of 0.5"), [](FGalaxyState& Case) { Case.Civs[1].Ethics.Pacifism = 0.5f; });
	StaysNeutral(TEXT("Another AI already hostile toward them"), [Zero](FGalaxyState& Case) { CosmicDiplomacy::SetStanceOf(Case.Civs[3], Zero, CosmicTags::StanceHostile()); });
	StaysNeutral(TEXT("The rest after a siege"), [](FGalaxyState& Case) { Case.Civs[1].FleetTargetSinceTick = 80; });
	StaysNeutral(TEXT("A neighbour of the first rung that holds its capital alone"), [](FGalaxyState& Case)
	{
		// People 0 keeps 0 alone and borders people 1 over lane 0-1; people 1 holds 3 of its 4 (4 goes to people 2).
		HandOver(Case, 1, 1);
		HandOver(Case, 4, 2);
		Case.Civs[0].Tier = EAscensionTier::Tier1_Planetary;
	});
	StaysNeutral(TEXT("Their only system to take inside a higher sphere"), [](FGalaxyState& Case) { CosmicTests::AddInstance(Case, Silence, { Case.Systems[1].SystemID }); });
	// (A people that does not border them is no case of its own: a system the siege gate passes lies one lane from a settled
	// system of the attacker, which is the border, and the view of a pair that does not border ignores the stored relation.)
	{
		FGalaxyState Besieged = Start;
		Besieged.Civs[2].Relations.Add(One, -0.9f);
		Besieged.Civs[2].FleetPower = 2.f;
		Besieged.Civs[2].Ethics.Pacifism = -0.5f;
		Besieged.Civs[1].Relations.Reset();
		Besiege(Besieged, 0, 1, 2, 100);
		Sim->LoadState(Besieged);
		Sim->StepOnce();
		TestFalse(TEXT("The ruled people's siege on them closes the gate"), CosmicDiplomacy::StanceOf(Sim->GetState().Civs[2], One) == Hostile());
	}

	// Toward the people the player rules the gate reads the two TowardRuled knobs (B3 decision 21), toward any other people
	// the general two.
	{
		const FCosmicDataTables Laid = Sim->GetTables();
		const FName Two = Start.Civs[2].CivID;
		auto WithGate = [&](float Ratio, float RelationTowardRuled, float RatioTowardRuled)
		{
			FCosmicRulesDef Gate = Sim->GetRules();
			Gate.AIHostilePowerRatio = Ratio;
			Gate.AIHostileRelationTowardRuled = RelationTowardRuled;
			Gate.AIHostilePowerRatioTowardRuled = RatioTowardRuled;
			Sim->SetTables(CosmicTests::WithRules(Laid, Gate));
		};
		auto HostileAfter = [&](const FGalaxyState& Case, FName Toward)
		{
			Sim->LoadState(Case);
			Sim->StepOnce();
			return CosmicDiplomacy::StanceOf(Sim->GetState().Civs[1], Toward) == Hostile();
		};
		WithGate(1.5f, -0.5f, 1.5f);
		TestFalse(TEXT("A stricter relation toward the ruled people keeps the AI neutral at -0.4"), HostileAfter(Start, Zero));
		WithGate(1.5f, -0.3f, 2.5f);
		TestFalse(TEXT("and so does a stricter power ratio toward it (2 against 1 under 2.5)"), HostileAfter(Start, Zero));
		{
			FGalaxyState Mild = Start;
			Mild.Civs[1].Relations.Add(Zero, -0.1f);
			Mild.Civs[1].FleetPower = 0.7f;
			WithGate(1.5f, 0.f, 0.6f);
			TestTrue(TEXT("A milder gate toward the ruled people opens at -0.1 and 0.7 against 1"), HostileAfter(Mild, Zero));
		}
		{
			FGalaxyState Other = Start;
			Other.Civs[1].Relations.Add(Zero, 0.f);
			Other.Civs[1].Relations.Add(Two, -0.4f);
			WithGate(1.5f, -0.9f, 9.f);
			TestTrue(TEXT("Toward a people the player does not rule the general gate opens"), HostileAfter(Other, Two));
		}
		// Below 1 the besieged may answer: an AI 0.79 times as strong as its besieger opens on it (counter-sieges stand - a
		// war between near-equals).
		{
			FGalaxyState Answer = Start;
			Answer.Civs[1].Relations.Add(Zero, 0.f);
			Answer.Civs[1].Relations.Add(Two, -0.4f);
			Answer.Civs[1].FleetPower = 1.f;
			Answer.Civs[2].FleetPower = 1.2f;
			Besiege(Answer, 2, 1, 4, 90);
			WithGate(0.79f, -0.3f, 1.5f);
			TestTrue(TEXT("Under 0.79 a besieged AI opens on its besieger"), HostileAfter(Answer, Two) && Sim->GetState().Civs[1].FleetTargetSystemID == IDOf(Sim->GetState(), 5));
			WithGate(1.5f, -0.3f, 1.5f);
			TestFalse(TEXT("(under 1.5 it does not)"), HostileAfter(Answer, Two));
		}
		Sim->SetTables(Laid);
	}

	// A hostility without a siege ends; a friendly stance and a guard are let go; the ruled people is passed by.
	{
		FGalaxyState Leftover = Start;
		Leftover.Civs[1].Relations.Reset();
		CosmicDiplomacy::SetStanceOf(Leftover.Civs[1], Leftover.Civs[2].CivID, CosmicTags::StanceHostile());
		CosmicDiplomacy::SetStanceOf(Leftover.Civs[2], One, CosmicTags::StanceFriendly());
		Leftover.Civs[3].FleetTargetSystemID = IDOf(Leftover, 9);
		CosmicDiplomacy::SetStanceOf(Leftover.Civs[0], One, CosmicTags::StanceHostile());
		Sim->LoadState(Leftover);
		Sim->StepOnce();
		const FGalaxyState& After = Sim->GetState();
		TestFalse(TEXT("A hostility with no siege ends"), CosmicDiplomacy::StanceOf(After.Civs[1], After.Civs[2].CivID) == Hostile());
		TestFalse(TEXT("An AI holds no friendly stance"), CosmicDiplomacy::StanceOf(After.Civs[2], One) == Friendly());
		TestTrue(TEXT("nor a guard (its rest starts)"), After.Civs[3].FleetTargetSystemID.IsNone() && After.Civs[3].FleetTargetSinceTick == 101);
		TestTrue(TEXT("The ruled people's stance is its own"), CosmicDiplomacy::StanceOf(After.Civs[0], One) == Hostile());
	}

	// The coronation: the new ruler's siege of a system and its hostility are lifted, its treaties stay.
	{
		Sim->LoadState(Start);
		FGalaxyState& Live = Sim->GetMutableState();
		Besiege(Live, 2, 3, 8, 100);
		CosmicDiplomacy::Sign(Live, 2, 1, CosmicTags::TreatyTrade());
		Live.Player.CurrentSystemID = IDOf(Live, 6);
		Role->BecomeWanderer(Tag(TEXT("AI.Profile.Conservative")), Tag(TEXT("Reign.Deposed")));
		FGalaxyState& Wandering = Sim->GetMutableState();
		Wandering.Player.CurrentSystemID = IDOf(Wandering, 6);
		Wandering.Civs[2].ReverenceToPlayer = 0.7f;
		Wandering.Civs[2].Stability = 0.5f;
		for (FCivilizationState& Civ : Wandering.Civs)
		{
			Civ.ThroneLockedUntilTick = 0;
		}
		Wandering.Knowing.PendingPetitionRow = NAME_None;
		CosmicTests::SetKnowing(Wandering, {}, { 0, 1, 2, 3 });
		Wandering.Knowing.PendingContactCivID = Wandering.Civs[1].CivID;
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		if (TestTrue(TEXT("The vessel is crowned by people 2"), Role->BecomeRuler(Wandering.Civs[2].CivID) == CosmicWanderer::ECrownBlock::None))
		{
			TestTrue(TEXT("The coronation clears a waiting first contact"), Sim->GetState().Knowing.PendingContactCivID.IsNone());
			const FCivilizationState& Crowned = Sim->GetState().Civs[2];
			TestTrue(TEXT("Its siege is lifted"), Crowned.FleetTargetSystemID.IsNone() && Crowned.FleetTargetSinceTick == Sim->GetState().Tick);
			TestFalse(TEXT("and its hostility"), Crowned.Ties.Contains(Sim->GetState().Civs[3].CivID));
			TestTrue(TEXT("Its treaties stay"), CosmicDiplomacy::HasTreaty(Crowned, One, CosmicTags::TreatyTrade()));
			TestEqual(TEXT("(no draw)"), Sim->GetRng().GetCurrentSeed(), Seed);
		}
	}
	return true;
}

// #73 (header section 2.10): a feat counts the people's own completions; a record of no people is the owner's and a
// conquest does not make it the conqueror's; a conquered system counts among the settled systems.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimOwnCompletionsTest, "Cosmic_Ascension.Sim.OwnCompletions", CosmicTests::Flags)
bool FCosmicSimOwnCompletionsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDiplomacyTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* State = LayBoard(*Sim, Original, NoEdit);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const FName Row(TEXT("project_test"));
	const FName Zero = State->Civs[0].CivID;
	const FName One = State->Civs[1].CivID;
	FMegaprojectRecord& Theirs = State->Systems[2].CompletedMegaprojects.AddDefaulted_GetRef();
	Theirs.Row = Row;
	Theirs.Tick = 10;
	Theirs.CivID = Zero;
	TestFalse(TEXT("Another people's completion is not the owner's"), CosmicMegaprojects::HasCompleted(State->Systems[2], Row, 0, One));
	TestTrue(TEXT("but the people's that made it"), CosmicMegaprojects::HasCompleted(State->Systems[2], Row, 0, Zero));
	TestFalse(TEXT("An empty people asks for the owner"), CosmicMegaprojects::HasCompleted(State->Systems[2], Row, 0, NAME_None));
	{
		FCosmicCondition Done;
		Done.Type = EConditionType::MegaprojectComplete;
		Done.Key = Row;
		FCosmicEvalView View(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
		View.CivIndex = 1;
		View.SystemIndex = 2;
		TestFalse(TEXT("The condition asks for the context people"), UCosmicEventSubsystem::Check(Done, View));
		View.CivIndex = 0;
		TestTrue(TEXT("(and the maker passes)"), UCosmicEventSubsystem::Check(Done, View));
	}
	Theirs.CivID = NAME_None;
	TestTrue(TEXT("A record of no people is the owner's"), CosmicMegaprojects::HasCompleted(State->Systems[2], Row, 0, One));
	{
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		TestTrue(TEXT("(people 0 takes the system)"), CosmicHazards::Transfer(Ctx, 2, 0));
	}
	TestEqual(TEXT("The taking writes the old owner into it"), State->Systems[2].CompletedMegaprojects[0].CivID, One);
	TestFalse(TEXT("so the conquest does not make it the conqueror's"), CosmicMegaprojects::HasCompleted(State->Systems[2], Row, 0, Zero));
	{
		FCosmicCondition Three;
		Three.Type = EConditionType::CivSettledSystemsAtLeast;
		Three.Value = 3.f;
		FCosmicEvalView View(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
		View.CivIndex = 0;
		View.SystemIndex = 0;
		TestTrue(TEXT("A conquered system counts among the settled systems"), UCosmicEventSubsystem::Check(Three, View));
	}
	return true;
}

// The conflict lines (header section 3.4): one row per side in the game's data; the siege the player begins does not stop
// the game, a siege on the ruled people does; the stance lines stay out of the chronicle, the treaty and siege lines in.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLogConflictLinesTest, "Cosmic_Ascension.Log.ConflictLines", CosmicTests::Flags)
bool FCosmicLogConflictLinesTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDiplomacyTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const UDataTable* Templates = Sim->GetTables().LogTemplates;
	if (!TestNotNull(TEXT("The game's log templates are loaded"), Templates))
	{
		return false;
	}
	const FName Rows[] = { CosmicRows::LogStanceChanged, CosmicRows::LogStanceLapsed, CosmicRows::LogTreatySigned, CosmicRows::LogTreatyBroken,
		CosmicRows::LogSiegeBegun, CosmicRows::LogSiegeOnUs, CosmicRows::LogSiegeTaken, CosmicRows::LogSiegeLost, CosmicRows::LogSiegeHeld,
		CosmicRows::LogSiegeFailed, CosmicRows::LogSiegeLifted };
	TSet<FString> Seen;
	for (const FName Row : Rows)
	{
		const FLogTemplateDef* Def = Templates->FindRow<FLogTemplateDef>(Row, TEXT("ConflictLines"), false);
		if (!TestNotNull(*FString::Printf(TEXT("The row %s is in the game's data"), *Row.ToString()), Def))
		{
			continue;
		}
		TestTrue(*FString::Printf(TEXT("%s tells in three variants"), *Row.ToString()), Def->Templates.Num() >= 3);
		for (const FText& Text : Def->Templates)
		{
			TestFalse(*FString::Printf(TEXT("%s shares no template with another row"), *Row.ToString()), Seen.Contains(Text.ToString()));
			Seen.Add(Text.ToString());
		}
		const bool bSiege = Row.ToString().StartsWith(TEXT("log_siege_"));
		TestTrue(*FString::Printf(TEXT("%s is in its category"), *Row.ToString()), Def->Category == Tag(bSiege ? TEXT("Log.Conflict") : TEXT("Log.Civ")));
	}

	FCosmicLogEntry Entry;
	Entry.Category = Tag(TEXT("Log.Conflict"));
	Entry.FocusCivID = FName(TEXT("civ_test"));
	const TArray<FName> Peoples = { Entry.FocusCivID };
	Entry.SourceRow = CosmicRows::LogSiegeBegun;
	Entry.Severity = 1;
	TestFalse(TEXT("A siege the ruler begins does not stop the game"), CosmicAutoPause::StopsFor(Entry));
	TestTrue(TEXT("but goes into the chronicle"), CosmicChronicle::Admits(Entry, Peoples));
	Entry.SourceRow = CosmicRows::LogSiegeOnUs;
	Entry.Severity = 2;
	TestTrue(TEXT("A siege on the ruled people stops it"), CosmicAutoPause::StopsFor(Entry));
	Entry.Category = Tag(TEXT("Log.Civ"));
	Entry.Severity = 1;
	Entry.SourceRow = CosmicRows::LogStanceChanged;
	TestFalse(TEXT("A stance line stays out of the chronicle"), CosmicChronicle::Admits(Entry, Peoples));
	Entry.SourceRow = CosmicRows::LogStanceLapsed;
	TestFalse(TEXT("and a lapsed stance's"), CosmicChronicle::Admits(Entry, Peoples));
	Entry.SourceRow = CosmicRows::LogTreatySigned;
	TestTrue(TEXT("A treaty's line goes in"), CosmicChronicle::Admits(Entry, Peoples));
	Entry.SourceRow = CosmicRows::LogSiegeLifted;
	TestTrue(TEXT("and a lifted siege's"), CosmicChronicle::Admits(Entry, Peoples));
	return true;
}

// The ruler policy's diplomacy (B3 header section 5): the first contact card's friendly choice whatever the answers so far;
// one command a boundary - the guard first, then friendship, then non-aggression and trade from the second rung; the
// guard withdrawn once the siege is gone; the peace policy never hostile, the attack policy hostile and then besieging;
// nothing without the policy or after the end; the decisions count the three commands.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimRulerDiplomacyPolicyTest, "Cosmic_Ascension.Sim.RulerDiplomacyPolicy", CosmicTests::Flags)
bool FCosmicSimRulerDiplomacyPolicyTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDiplomacyTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FCosmicRulerPolicy Peace{ false, false, false, false, NAME_None, 1, ECosmicPressurePolicy::Round, 0, false };
	const FCosmicRulerPolicy Attack{ false, false, false, false, NAME_None, 2, ECosmicPressurePolicy::Round, 0, false };
	const FCosmicRulerPolicy None{ false, false, false, false, NAME_None, 0, ECosmicPressurePolicy::Round, 0, false };

	// The first contact card: reach out whatever the answers so far; without the policy the round of every card.
	const FEncounterDef* Card = Original.FindEncounter(CosmicRows::PetitionFirstContact);
	if (!TestNotNull(TEXT("(the game's tables hold the first contact card)"), Card))
	{
		return false;
	}
	{
		FGalaxyState* State = LayBoard(*Sim, Original, NoEdit);
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		int32 Friendly = INDEX_NONE;
		for (int32 Choice = 0; Choice < Card->Choices.Num() && Friendly == INDEX_NONE; ++Choice)
		{
			Friendly = Card->Choices[Choice].OnSuccess.ContainsByPredicate([](const FCosmicEffect& Effect)
			{
				return Effect.Type == EEffectType::SetStance && Effect.Tag == CosmicTags::StanceFriendly();
			}) ? Choice : INDEX_NONE;
		}
		TestTrue(TEXT("(the card has a friendly choice)"), Friendly != INDEX_NONE);
		bool bAlways = true;
		bool bRounds = false;
		for (int32 Answers = 0; Answers < 6; ++Answers)
		{
			bAlways &= CosmicRulerPolicy::ChoiceOf(*State, Sim->GetStateIndex(), *Card, Answers, CosmicRows::PetitionFirstContact, 1) == Friendly
				&& CosmicRulerPolicy::ChoiceOf(*State, Sim->GetStateIndex(), *Card, Answers, CosmicRows::PetitionFirstContact, 2) == Friendly;
			bRounds |= CosmicRulerPolicy::ChoiceOf(*State, Sim->GetStateIndex(), *Card, Answers, CosmicRows::PetitionFirstContact, 0) != Friendly;
		}
		TestTrue(TEXT("With a diplomacy policy the first contact card always reaches out"), bAlways);
		TestTrue(TEXT("without one it takes its turn in the round"), bRounds);
	}

	// One command a boundary: friendship, then (from the second rung) non-aggression and trade, then nothing.
	{
		FGalaxyState* State = LayBoard(*Sim, Original, Fleets);
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		TArray<FCivilizationState>& Civs = State->Civs;
		const FName One = Civs[1].CivID;
		Civs[0].Stockpile.Influence = 200.f;
		Civs[0].IncomePerTick.Influence = 5.f;
		FCosmicRulerPolicyCounts Counts;
		CosmicRulerPolicy::ActDiplomacy(None, *Sim, *Role, Counts);
		TestTrue(TEXT("Without the policy nothing is done"), Civs[0].Ties.IsEmpty() && Counts.StanceChanges == 0);
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestTrue(TEXT("The first boundary turns the bordering people friendly"), CosmicDiplomacy::StanceOf(Civs[0], One) == CosmicTags::StanceFriendly()
			&& Counts.StanceChanges == 1 && Counts.TreatiesSigned == 0);
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestTrue(TEXT("On the first rung no treaty follows"), Counts.StanceChanges == 1 && Counts.TreatiesSigned == 0 && Counts.TreatyLooks.IsEmpty());
		Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestTrue(TEXT("From the second rung: non-aggression first"), CosmicDiplomacy::HasTreaty(Civs[0], One, CosmicTags::TreatyNonAggression())
			&& !CosmicDiplomacy::HasTreaty(Civs[0], One, CosmicTags::TreatyTrade()) && Counts.TreatiesSigned == 1);
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestTrue(TEXT("then trade, one a boundary"), CosmicDiplomacy::HasTreaty(Civs[0], One, CosmicTags::TreatyTrade()) && Counts.TreatiesSigned == 2
			&& Counts.TreatiesProposedBorder == 2);
		const int32 Done = Counts.StanceChanges + Counts.TreatiesSigned + Counts.FleetTargetChanges;
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestEqual(TEXT("and then nothing more"), Counts.StanceChanges + Counts.TreatiesSigned + Counts.FleetTargetChanges, Done);
		TestTrue(TEXT("The met peoples that do not border were looked at once each (no proposal to them)"),
			Counts.TreatiesProposedOther + Counts.TreatiesRefusedOther == 4 && Counts.TreatyLooks.Num() == 6);
		TestEqual(TEXT("The decisions count the diplomacy"), CosmicRulerPolicy::DecisionsOf(Counts), 3);

		// The guard: a siege on our system is guarded against first; withdrawn once it is gone.
		Civs[0].FleetPower = 1.f;
		Civs[2].Tier = EAscensionTier::Tier2_Stellar;
		Civs[2].FleetPower = 1.f;
		Civs[2].FleetTargetSystemID = IDOf(*State, 1);
		Civs[2].FleetTargetSinceTick = State->Tick;
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestTrue(TEXT("A siege on our system is guarded against"), Civs[0].FleetTargetSystemID == IDOf(*State, 1) && Counts.FleetTargetChanges == 1);
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestEqual(TEXT("(and the guard stands)"), Counts.FleetTargetChanges, 1);
		Civs[2].FleetTargetSystemID = NAME_None;
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestTrue(TEXT("Once the siege is gone the guard is withdrawn"), Civs[0].FleetTargetSystemID.IsNone() && Counts.FleetTargetChanges == 2);
		TestEqual(TEXT("The decisions count the fleet commands too"), CosmicRulerPolicy::DecisionsOf(Counts), 5);
		TestFalse(TEXT("The peace policy never turned hostile"), Civs[0].Ties.Contains(Civs[2].CivID) && Civs[0].Ties[Civs[2].CivID].Stance == CosmicTags::StanceHostile());

		// After the end: nothing.
		Counts.bEnded = true;
		Civs[2].FleetTargetSystemID = IDOf(*State, 1);
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestTrue(TEXT("Nothing after the end"), Civs[0].FleetTargetSystemID.IsNone() && Counts.FleetTargetChanges == 2);
	}

	// The attack policy: with friendship and treaties out of reach, hostility, then the siege - one a boundary.
	{
		FGalaxyState* State = LayBoard(*Sim, Original, Fleets);
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		TArray<FCivilizationState>& Civs = State->Civs;
		const FName One = Civs[1].CivID;
		Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		Civs[0].FleetPower = 1.f;
		Civs[0].Stockpile.Influence = 0.f;
		CosmicDiplomacy::SetStanceOf(Civs[0], Civs[3].CivID, CosmicTags::StanceFriendly());
		FCosmicRulerPolicyCounts Counts;
		for (int32 Boundary = 0; Boundary < 5; ++Boundary)
		{
			CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		}
		TestTrue(TEXT("(the peace policy does nothing here)"), Counts.StanceChanges + Counts.TreatiesSigned + Counts.FleetTargetChanges == 0);
		CosmicRulerPolicy::ActDiplomacy(Attack, *Sim, *Role, Counts);
		TestTrue(TEXT("The attack policy turns the bordering people hostile"), CosmicDiplomacy::StanceOf(Civs[0], One) == CosmicTags::StanceHostile()
			&& Counts.StanceChanges == 1 && Civs[0].FleetTargetSystemID.IsNone());
		CosmicRulerPolicy::ActDiplomacy(Attack, *Sim, *Role, Counts);
		TestTrue(TEXT("and at the next boundary besieges its first system the gate takes"), Civs[0].FleetTargetSystemID == IDOf(*State, 2) && Counts.FleetTargetChanges == 1);
		// Besieging, it guards nothing: a siege on its own system meanwhile leaves the fleet where it is.
		Civs[1].FleetPower = 1.f;
		Civs[1].FleetTargetSystemID = IDOf(*State, 1);
		CosmicRulerPolicy::ActDiplomacy(Attack, *Sim, *Role, Counts);
		TestTrue(TEXT("While it besieges, a siege on its own system draws no guard"), Civs[0].FleetTargetSystemID == IDOf(*State, 2) && Counts.FleetTargetChanges == 1);
		Civs[1].FleetTargetSystemID = NAME_None;
		// A hostility with no system to besiege (people 2 borders none of ours) is turned neutral before another opens.
		Civs[0].FleetTargetSystemID = NAME_None;
		CosmicDiplomacy::SetStanceOf(Civs[0], One, FGameplayTag());
		CosmicDiplomacy::SetStanceOf(Civs[0], Civs[2].CivID, CosmicTags::StanceHostile());
		const int32 Stances = Counts.StanceChanges;
		CosmicRulerPolicy::ActDiplomacy(Attack, *Sim, *Role, Counts);
		TestTrue(TEXT("A hostility with nothing to besiege is turned neutral first"), !CosmicDiplomacy::StanceOf(Civs[0], Civs[2].CivID).IsValid()
			&& !CosmicDiplomacy::StanceOf(Civs[0], One).IsValid() && Counts.StanceChanges == Stances + 1);
	}

	// One boundary, a siege and a free friendly slot: the guard first.
	{
		FGalaxyState* State = LayBoard(*Sim, Original, Fleets);
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		TArray<FCivilizationState>& Civs = State->Civs;
		Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		Civs[0].FleetPower = 1.f;
		Civs[0].Stockpile.Influence = 200.f;
		Civs[0].IncomePerTick.Influence = 5.f;
		Civs[2].Tier = EAscensionTier::Tier2_Stellar;
		Civs[2].FleetPower = 1.f;
		Civs[2].FleetTargetSystemID = IDOf(*State, 1);
		FCosmicRulerPolicyCounts Counts;
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestTrue(TEXT("A siege and a free friendly slot at one boundary: the guard first"), Civs[0].FleetTargetSystemID == IDOf(*State, 1)
			&& Counts.FleetTargetChanges == 1 && Counts.StanceChanges == 0);
		// The people of a waiting first contact card is left to the card.
		Civs[2].FleetTargetSystemID = NAME_None;
		Civs[0].FleetTargetSystemID = IDOf(*State, 1);
		State->Knowing.PendingContactCivID = Civs[1].CivID;
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestTrue(TEXT("(the guard withdrawn)"), Civs[0].FleetTargetSystemID.IsNone());
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestEqual(TEXT("The people of a waiting first contact card gets no friendship from the policy"), Counts.StanceChanges, 0);
		State->Knowing.PendingContactCivID = NAME_None;
		// A friendship the income cannot keep is not set.
		Civs[0].IncomePerTick.Influence = 0.f;
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestEqual(TEXT("Without the Influence income to keep it, no friendship"), Counts.StanceChanges, 0);
		Civs[0].IncomePerTick.Influence = 5.f;
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestEqual(TEXT("with it, the friendship"), Counts.StanceChanges, 1);
	}

	// One command a boundary also when a treaty could follow the friendship at once (from the second rung).
	{
		FGalaxyState* State = LayBoard(*Sim, Original, NoEdit);
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		TArray<FCivilizationState>& Civs = State->Civs;
		const FName One = Civs[1].CivID;
		Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		Civs[0].Stockpile.Influence = 200.f;
		Civs[0].IncomePerTick.Influence = 5.f;
		FCosmicRulerPolicyCounts Counts;
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestTrue(TEXT("On the second rung the friendship ends the boundary"), CosmicDiplomacy::StanceOf(Civs[0], One) == CosmicTags::StanceFriendly()
			&& Counts.StanceChanges == 1 && Counts.TreatiesSigned == 0);
		CosmicRulerPolicy::ActDiplomacy(Peace, *Sim, *Role, Counts);
		TestTrue(TEXT("(a treaty was there to take: the next boundary signs it)"), Counts.TreatiesSigned == 1);
	}

	// The attack policy offers no treaty (a non-aggression would close the siege gate on the neighbour, a trade would make
	// the siege a betrayal), and keeps a guard while the siege on its system stands.
	{
		FGalaxyState* State = LayBoard(*Sim, Original, NoEdit);
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		TArray<FCivilizationState>& Civs = State->Civs;
		const FName One = Civs[1].CivID;
		Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		Civs[0].Stockpile.Influence = 200.f;
		Civs[0].IncomePerTick.Influence = 5.f;
		CosmicDiplomacy::SetStanceOf(Civs[0], One, CosmicTags::StanceFriendly());
		FCosmicRulerPolicyCounts Counts;
		for (int32 Boundary = 0; Boundary < 3; ++Boundary)
		{
			CosmicRulerPolicy::ActDiplomacy(Attack, *Sim, *Role, Counts);
		}
		TestTrue(TEXT("The attack policy offers no treaty"), !CosmicDiplomacy::HasTreaty(Civs[0], One, CosmicTags::TreatyTrade())
			&& !CosmicDiplomacy::HasTreaty(Civs[0], One, CosmicTags::TreatyNonAggression()) && Counts.TreatiesSigned == 0);
		// With fleets: a siege on its system is guarded, and the guard stands while the siege does - no hostility opened.
		FGalaxyState* Armed = LayBoard(*Sim, Original, Fleets);
		if (!TestNotNull(TEXT("The board is laid"), Armed))
		{
			return false;
		}
		TArray<FCivilizationState>& Peoples = Armed->Civs;
		Peoples[0].Tier = EAscensionTier::Tier2_Stellar;
		Peoples[0].FleetPower = 1.f;
		CosmicDiplomacy::SetStanceOf(Peoples[0], Peoples[3].CivID, CosmicTags::StanceFriendly());
		Peoples[2].Tier = EAscensionTier::Tier2_Stellar;
		Peoples[2].FleetPower = 1.f;
		Peoples[2].FleetTargetSystemID = IDOf(*Armed, 1);
		FCosmicRulerPolicyCounts Guarding;
		CosmicRulerPolicy::ActDiplomacy(Attack, *Sim, *Role, Guarding);
		TestTrue(TEXT("The attack policy guards a siege on its system"), Peoples[0].FleetTargetSystemID == IDOf(*Armed, 1) && Guarding.FleetTargetChanges == 1);
		CosmicRulerPolicy::ActDiplomacy(Attack, *Sim, *Role, Guarding);
		TestTrue(TEXT("and while the siege stands the guard holds - no hostility opened"), Peoples[0].FleetTargetSystemID == IDOf(*Armed, 1)
			&& Guarding.StanceChanges == 0 && Guarding.FleetTargetChanges == 1);
	}

	// The attack policy leaves its last realm slot to a conquest (a people at its cap besieges nothing): on the second rung
	// with two systems of three it starts no settlement, where the peace policy does.
	{
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		FGalaxyState& Fresh = Sim->GetMutableState();
		const int32 Ruled = Sim->GetStateIndex().FindCiv(Fresh, Fresh.Player.RuledCivID);
		const int32 Home = Sim->GetStateIndex().FindSystem(Fresh, Fresh.Civs[Ruled].HomeSystemID);
		int32 Second = INDEX_NONE;
		for (const FName Lane : Fresh.Systems[Home].Lanes)
		{
			const int32 Next = Sim->GetStateIndex().FindSystem(Fresh, Lane);
			if (Second == INDEX_NONE && Fresh.Systems.IsValidIndex(Next) && Fresh.Systems[Next].OwnerCivID.IsNone() && !CosmicState::IsSanctuary(Fresh.Systems[Next].SystemID))
			{
				Second = Next;
			}
		}
		if (!TestTrue(TEXT("(the ruled people's home has an unowned neighbour)"), Second != INDEX_NONE))
		{
			return false;
		}
		CosmicTests::Give(Fresh, Ruled, { Second });
		Fresh.Civs[Ruled].Tier = EAscensionTier::Tier2_Stellar;
		Fresh.Civs[Ruled].Stockpile.Energy = Fresh.Civs[Ruled].Stockpile.Matter = Fresh.Civs[Ruled].Stockpile.Influence = 100000.f;
		Sim->RebuildStateIndex();
		const FGalaxyState Laid = Fresh;
		TestEqual(TEXT("(two systems of a cap of three)"), CosmicConflict::OwnedCountOf(Laid, Sim->GetStateIndex(), Ruled) + 1,
			CosmicMegaprojects::RealmCapOf(Sim->GetRules(), EAscensionTier::Tier2_Stellar));
		FCosmicRulerPolicyCounts AttackCounts;
		CosmicRulerPolicy::Act(FCosmicRulerPolicy{ false, true, false, false, NAME_None, 2, ECosmicPressurePolicy::Round, 0, false }, *Sim, *Role, NAME_None, AttackCounts);
		TestEqual(TEXT("The attack policy starts no settlement into its last realm slot"), AttackCounts.Started, 0);
		Sim->LoadState(Laid);
		FCosmicRulerPolicyCounts PeaceCounts;
		CosmicRulerPolicy::Act(FCosmicRulerPolicy{ false, true, false, false, NAME_None, 1, ECosmicPressurePolicy::Round, 0, false }, *Sim, *Role, NAME_None, PeaceCounts);
		TestEqual(TEXT("the peace policy does"), PeaceCounts.Started, 1);
	}

	// Through Act, the path the commandlet and the hash run play: the card reaches out whatever the round says.
	{
		FGalaxyState* State = LayBoard(*Sim, Original, NoEdit);
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		State->Knowing.FirstContactCivID = NAME_None;
		Unmeet(*State, 1);
		Sim->NoteMet(State->Civs[1].CivID);
		TestTrue(TEXT("(the card is put)"), CosmicPetitions::Present(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), TArray<FCosmicPetitionUnit>())
			&& State->Knowing.PendingPetitionRow == CosmicRows::PetitionFirstContact);
		FCosmicRulerPolicyCounts Counts;
		Counts.Answers = 1;
		Counts.bEnded = true;
		CosmicRulerPolicy::Act(FCosmicRulerPolicy{ true, false, false, false, NAME_None, 1, ECosmicPressurePolicy::Round, 0, false }, *Sim, *Role, NAME_None, Counts);
		TestTrue(TEXT("Act answers the card by reaching out (the round alone would watch)"), CosmicDiplomacy::StanceOf(State->Civs[0], State->Civs[1].CivID) == CosmicTags::StanceFriendly()
			&& Counts.Answers == 2 && Counts.StanceChanges == 0);
	}
	return true;
}

// The first contact card (B3 header section 2.9): the first meeting writes its slot (a ruler, the row in the table); the end
// of the knowing stage puts it while no petition waits, after a Standing card that came to hold in the tick and before the
// periodic slot - it waits behind a waiting petition, and answering that one keeps the slot;
// its answer is about the people met (OtherID, {OtherCiv}): one draw, friendly through the stance gate or nothing, the
// slot cleared; a card whose people is gone is withdrawn; the slot comes back after a load and goes with a new life or a
// wanderer's.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimFirstContactCardTest, "Cosmic_Ascension.Sim.FirstContactCard", CosmicTests::Flags)
bool FCosmicSimFirstContactCardTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDiplomacyTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	if (!TestNotNull(TEXT("(the game's tables hold the first contact card)"), Original.FindEncounter(CosmicRows::PetitionFirstContact)))
	{
		return false;
	}
	const FName Feast(TEXT("petition_feast"));
	// A board on which people 1 is not met yet and nothing has been met first; every card payable.
	auto Lay = [&](const FCosmicDataTables& From) -> FGalaxyState*
	{
		FGalaxyState* Laid = LayBoard(*Sim, From, NoEdit);
		if (Laid)
		{
			Laid->Knowing.FirstContactCivID = NAME_None;
			Unmeet(*Laid, 1);
			FResourcePool Plenty;
			Plenty.Energy = Plenty.Matter = Plenty.Insight = Plenty.Influence = Plenty.Causality = 1000.f;
			Laid->Civs[0].Stockpile = Plenty;
		}
		return Laid;
	};
	auto Put = [&](FGalaxyState& Live)
	{
		return CosmicPetitions::Present(Live, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), TArray<FCosmicPetitionUnit>());
	};

	// The slot: written at the first meeting of the run, about that people - and not without the row.
	{
		FGalaxyState* State = Lay(CosmicTests::WithoutDiplomacy(Original));
		if (!TestNotNull(TEXT("(the board without the card's row)"), State))
		{
			return false;
		}
		TestTrue(TEXT("(a first meeting)"), Sim->NoteMet(State->Civs[1].CivID));
		TestTrue(TEXT("Without the row the first meeting leaves the slot empty"), State->Knowing.PendingContactCivID.IsNone()
			&& State->Knowing.FirstContactCivID == State->Civs[1].CivID);
	}
	FGalaxyState* State = Lay(Original);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	TArray<FCivilizationState>& Civs = State->Civs;
	const FName One = Civs[1].CivID;
	const FName Two = Civs[2].CivID;
	TestTrue(TEXT("(a first meeting)"), Sim->NoteMet(One));
	TestEqual(TEXT("The first meeting writes the slot with the people met"), State->Knowing.PendingContactCivID, One);
	Unmeet(*State, 2);
	TestTrue(TEXT("(a later meeting)"), Sim->NoteMet(Two));
	TestEqual(TEXT("and a later meeting leaves it"), State->Knowing.PendingContactCivID, One);

	// It waits behind a petition, and answering that one keeps it: it comes at the end of the next knowing stage.
	State->Knowing.PendingPetitionRow = Feast;
	TestFalse(TEXT("While a petition waits the card is not put"), Put(*State));
	TestTrue(TEXT("(that petition still waits)"), State->Knowing.PendingPetitionRow == Feast && State->Knowing.PendingContactCivID == One);
	const FCosmicPetitionAnswer Other = Role->AnswerPetition(0);
	TestTrue(TEXT("Answering the other petition keeps the slot"), Other.Outcome != ECosmicPetitionOutcome::Refused && State->Knowing.PendingContactCivID == One);
	TestTrue(TEXT("and puts no card: the first contact card waits for the knowing stage"), State->Knowing.PendingPetitionRow.IsNone());
	Sim->StepOnce();
	TestEqual(TEXT("and the card comes at the end of the next knowing stage"), State->Knowing.PendingPetitionRow, CosmicRows::PetitionFirstContact);
	TestTrue(TEXT("(about the people, no system)"), State->Knowing.PendingPetitionSystemID.IsNone());

	// It comes back after a load.
	{
		FGalaxyState Saved = *State;
		Saved.Knowing.PendingPetitionRow = NAME_None;
		Sim->LoadState(Saved);
		State = &Sim->GetMutableState();
		TestTrue(TEXT("After a load the slot puts the card again"), Put(*State) && State->Knowing.PendingPetitionRow == CosmicRows::PetitionFirstContact);
	}

	// Its words name the people met.
	const FEncounterDef* Card = Sim->GetTables().FindEncounter(CosmicRows::PetitionFirstContact);
	{
		const FFormatNamedArguments Args = CosmicPetitions::ArgsOf(*State, Sim->GetTables(), 0, INDEX_NONE, One);
		TestTrue(TEXT("{OtherCiv} is the people met"), Card && FText::Format(Card->Title, Args).ToString().Contains(State->Civs[1].CivName.ToString()));
		const FFormatNamedArguments None = CosmicPetitions::ArgsOf(*State, Sim->GetTables(), 0, INDEX_NONE);
		TestTrue(TEXT("and empty words without one"), Card && !FText::Format(Card->Title, None).ToString().Contains(TEXT("{")));
	}

	// [Reach out]: one draw, the people met turned friendly (the context's OtherID), its line naming it, the slot cleared.
	{
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		const FCosmicPetitionAnswer Answer = Role->AnswerPetition(0);
		TestTrue(TEXT("Reaching out is taken"), Answer.Outcome == ECosmicPetitionOutcome::Succeeded);
		TestEqual(TEXT("with one draw"), DrawsBetween(Seed, Sim->GetState().RngState), 1);
		TestTrue(TEXT("The people met is friendly now"), CosmicDiplomacy::StanceOf(State->Civs[0], One) == CosmicTags::StanceFriendly()
			&& CosmicDiplomacy::FriendlyCountOf(State->Civs[0]) == 1);
		TestTrue(TEXT("(no other people)"), CosmicDiplomacy::StanceOf(State->Civs[0], Two) != CosmicTags::StanceFriendly());
		TestTrue(TEXT("Its line names the people"), Answer.Line.ToString().Contains(State->Civs[1].CivName.ToString()) && !Answer.Line.ToString().Contains(TEXT("{")));
		TestTrue(TEXT("The slot and the card are cleared"), State->Knowing.PendingContactCivID.IsNone() && State->Knowing.PendingPetitionRow.IsNone());
		TestFalse(TEXT("and nothing is put again"), Put(*State));
		Unmeet(*State, 3);
		TestTrue(TEXT("(a new meeting after the card was answered)"), Sim->NoteMet(State->Civs[3].CivID));
		TestTrue(TEXT("A meeting after the card was answered writes no slot"), State->Knowing.PendingContactCivID.IsNone());
		TestFalse(TEXT("and puts no card"), Put(*State));
	}

	// [Watch]: one draw, nothing changes but the slot.
	State = Lay(Original);
	Sim->NoteMet(State->Civs[1].CivID);
	TestTrue(TEXT("(the card is put)"), Put(*State));
	{
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		const FCosmicPetitionAnswer Answer = Role->AnswerPetition(1);
		TestTrue(TEXT("Watching is taken with one draw"), Answer.Outcome == ECosmicPetitionOutcome::Succeeded && DrawsBetween(Seed, Sim->GetState().RngState) == 1);
		TestTrue(TEXT("and changes no stance"), State->Civs[0].Ties.IsEmpty() && State->Knowing.PendingContactCivID.IsNone());
	}

	// The effect passes the stance gate: with the only friendly slot taken, reaching out changes nothing.
	State = Lay(Original);
	Sim->NoteMet(State->Civs[1].CivID);
	CosmicDiplomacy::SetStanceOf(State->Civs[0], State->Civs[2].CivID, CosmicTags::StanceFriendly());
	TestTrue(TEXT("(the card is put)"), Put(*State));
	{
		TestTrue(TEXT("With the only friendly slot taken, the gate refuses reaching out"), Card && CosmicPetitions::StanceBlockOfChoice(*State, Sim->GetStateIndex(),
			Sim->GetRules(), 0, State->Civs[1].CivID, Card->Choices[0]) == CosmicDiplomacy::ECosmicStanceBlock::FriendlySlots);
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		AddExpectedError(TEXT("the choice's gate refuses choice 0"), EAutomationExpectedErrorFlags::Contains, 1);
		const FCosmicPetitionAnswer Answer = Role->AnswerPetition(0);
		TestTrue(TEXT("and the answer refuses it as an unpaid cost: nothing drawn, nothing changed"), Answer.Outcome == ECosmicPetitionOutcome::Refused
			&& Sim->GetRng().GetCurrentSeed() == Seed && CosmicDiplomacy::StanceOf(State->Civs[0], State->Civs[1].CivID) != CosmicTags::StanceFriendly());
		TestTrue(TEXT("(the card and its slot wait)"), State->Knowing.PendingPetitionRow == CosmicRows::PetitionFirstContact
			&& State->Knowing.PendingContactCivID == State->Civs[1].CivID);
		TestEqual(TEXT("The policy then watches"), CosmicRulerPolicy::ChoiceOf(*State, Sim->GetStateIndex(), *Card, 0, CosmicRows::PetitionFirstContact, 1,
			&Sim->GetRules()), 1);
		TestTrue(TEXT("Watching is taken"), Role->AnswerPetition(1).Outcome == ECosmicPetitionOutcome::Succeeded && State->Knowing.PendingContactCivID.IsNone());
	}

	// A Standing card that came to hold in the tick goes first (it would not come again); the contact card waits one answer
	// and then goes before the periodic slot.
	State = Lay(Original);
	Sim->NoteMet(State->Civs[1].CivID);
	State->Tick = State->Calling.BornTick + FMath::Max<int64>(1, Sim->GetRules().PetitionPeriodTicks);
	{
		TestTrue(TEXT("A Standing card that came to hold goes before the contact card"), Put(*State)
			&& State->Knowing.PendingPetitionRow == FName(TEXT("petition_wise_heart")) && State->Knowing.PendingContactCivID == State->Civs[1].CivID);
		TestTrue(TEXT("(answered)"), Role->AnswerPetition(0).Outcome == ECosmicPetitionOutcome::Succeeded);
		const TArray<FCosmicPetitionUnit> Held = CosmicPetitions::StandingHeld(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
		TestTrue(TEXT("Then the contact card goes before the periodic slot (open at this tick)"),
			CosmicPetitions::Present(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), Held)
			&& State->Knowing.PendingPetitionRow == CosmicRows::PetitionFirstContact);
	}

	// A withdrawn other petition keeps the slot, and the card still comes.
	State = Lay(Original);
	Sim->NoteMet(State->Civs[1].CivID);
	State->Knowing.PendingPetitionRow = FName(TEXT("petition_walls"));
	State->Knowing.PendingPetitionSystemID = NAME_None;
	State->Civs[0].Stability = 0.9f;
	{
		const FCosmicPetitionAnswer Answer = Role->AnswerPetition(1);
		TestTrue(TEXT("A withdrawn other petition keeps the slot"), Answer.Outcome == ECosmicPetitionOutcome::Withdrawn
			&& State->Knowing.PendingContactCivID == State->Civs[1].CivID);
		Sim->StepOnce();
		TestEqual(TEXT("and the card still comes"), State->Knowing.PendingPetitionRow, CosmicRows::PetitionFirstContact);
	}

	// The card's own conditions gate it: a row that does not hold is not put, and its slot waits.
	{
		FCosmicDataTables Gated = Original;
		Gated.Encounters = CosmicTests::NewTable(FEncounterDef::StaticStruct());
		for (const TPair<FName, uint8*>& Each : Original.Encounters->GetRowMap())
		{
			FEncounterDef Row = *reinterpret_cast<const FEncounterDef*>(Each.Value);
			if (Each.Key == CosmicRows::PetitionFirstContact)
			{
				FCosmicCondition Never;
				Never.Type = EConditionType::GalaxyTickAtLeast;
				Never.Value = 1.0e9f;
				Row.Conditions.Add(Never);
			}
			Gated.Encounters->AddRow(Each.Key, Row);
		}
		State = Lay(Gated);
		Sim->NoteMet(State->Civs[1].CivID);
		TestFalse(TEXT("A first contact card whose row does not hold is not put"), Put(*State) && State->Knowing.PendingPetitionRow == CosmicRows::PetitionFirstContact);
		TestEqual(TEXT("and its slot waits"), State->Knowing.PendingContactCivID, State->Civs[1].CivID);
	}

	// Its words for a people gone: the card's own words, no empty name - for the slot the crossing stage's cleanup clears
	// (the card waiting) and for a people not in Civs; without the first contact card's flag, empty words.
	{
		const FFormatNamedArguments Cleared = CosmicPetitions::ArgsOf(*State, Sim->GetTables(), 0, INDEX_NONE, NAME_None, true);
		TestEqual(TEXT("A first contact card whose slot was cleared names the people by the card's own words"), FText::Format(INVTEXT("{OtherCiv}"), Cleared).ToString(),
			FText::FromStringTable(CosmicLabels::TableId, TEXT("Petition.OtherGone"), EStringTableLoadingPolicy::FindOrFullyLoad).ToString());
		TestTrue(TEXT("(another card names no one)"), FText::Format(INVTEXT("{OtherCiv}"), CosmicPetitions::ArgsOf(*State, Sim->GetTables(), 0, INDEX_NONE)).IsEmpty());
	}
	{
		const FFormatNamedArguments Gone = CosmicPetitions::ArgsOf(*State, Sim->GetTables(), 0, INDEX_NONE, FName(TEXT("civ_test_gone")), true);
		TestEqual(TEXT("A people gone is named by the card's own words"), FText::Format(INVTEXT("{OtherCiv}"), Gone).ToString(),
			FText::FromStringTable(CosmicLabels::TableId, TEXT("Petition.OtherGone"), EStringTableLoadingPolicy::FindOrFullyLoad).ToString());
	}

	// A people gone: the card no longer stands - withdrawn, one draw, the slot cleared.
	State = Lay(Original);
	Sim->NoteMet(State->Civs[1].CivID);
	TestTrue(TEXT("(the card is put)"), Put(*State));
	State->Knowing.PendingContactCivID = FName(TEXT("civ_test_gone"));
	TestFalse(TEXT("A card about a people gone does not stand"), CosmicPetitions::StillStands(*State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules()));
	{
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		const FCosmicPetitionAnswer Answer = Role->AnswerPetition(0);
		TestTrue(TEXT("Its answer is withdrawn with one draw"), Answer.Outcome == ECosmicPetitionOutcome::Withdrawn && DrawsBetween(Seed, Sim->GetState().RngState) == 1);
		TestTrue(TEXT("and clears the slot"), State->Knowing.PendingContactCivID.IsNone() && State->Knowing.PendingPetitionRow.IsNone());
	}
	State->Knowing.PendingContactCivID = FName(TEXT("civ_test_gone"));
	TestFalse(TEXT("Nor is it put for a people gone"), Put(*State));

	// A new life and a wanderer's take the slot away.
	State = Lay(Original);
	Sim->NoteMet(State->Civs[1].CivID);
	Role->Install(IDOf(*State, 0), State->Civs[0].CivID, EPlayerRole::Ruler);
	TestTrue(TEXT("Install clears the slot"), State->Knowing.PendingContactCivID.IsNone());
	State = Lay(Original);
	Sim->NoteMet(State->Civs[1].CivID);
	Role->BecomeWanderer(Tag(TEXT("AI.Profile.Conservative")), Tag(TEXT("Reign.Deposed")));
	TestTrue(TEXT("and so does becoming a wanderer"), State->Knowing.PendingContactCivID.IsNone());
	State = Lay(Original);
	Role->BecomeWanderer(Tag(TEXT("AI.Profile.Conservative")), Tag(TEXT("Reign.Deposed")));
	State = &Sim->GetMutableState();
	State->Knowing.FirstContactCivID = NAME_None;
	Unmeet(*State, 1);
	TestTrue(TEXT("(a wanderer's first meeting)"), Sim->NoteMet(State->Civs[1].CivID));
	TestTrue(TEXT("A wanderer's first meeting is the first contact but writes no slot"), State->Knowing.FirstContactCivID == State->Civs[1].CivID
		&& State->Knowing.PendingContactCivID.IsNone());
	return true;
}

#endif

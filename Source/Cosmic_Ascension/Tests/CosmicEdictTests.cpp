// Cosmic Ascension - automation tests of the edicts, the Society recovery and the ruler's research shares (B2).
// Design: docs/60_DECISIONS/headers/B2_edicts_and_research.md section 2, 6; docs/10_DESIGN/systems.md section 5.4, 5.8.

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
#include "GameplayTagContainer.h"
#include "Log/CosmicChronicle.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicResearch.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "Tests/CosmicTestProbe.h"
#include "World/CosmicRulerPolicy.h"

#include <initializer_list>
#include <limits>

namespace CosmicEdictTests
{
	using CosmicEdicts::ECosmicEdictBlock;

	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// The rows of this file: the test builds them, so no case leans on the values of the game's edicts.
	const FName Taxing(TEXT("edict_test_taxing"));    // cost 10 Influence, upkeep 0.5, Energy x1.1, Stability -0.01
	const FName Lifting(TEXT("edict_test_lifting"));  // no cost, Energy x1.3, Stability +0.002
	const FName Plain(TEXT("edict_test_plain"));      // nothing at all
	const FName Answer(TEXT("edict_test_answer"));    // the answer of crisis_test_wake
	const FName Guard(TEXT("edict_test_guard"));      // plague share x0, claim +100, rebellion +200
	const FName High(TEXT("edict_test_high"));        // Tier 2
	const FName Bound(TEXT("edict_test_bound"));      // a condition no people meets
	const FName Dear(TEXT("edict_test_dear"));        // a cost no stockpile pays
	const FName Heavy(TEXT("edict_test_heavy"));      // an upkeep no stockpile pays, Stability -0.01
	const FName Kin(TEXT("edict_test_kin"));          // a relation with no party named
	const FName Faint(TEXT("edict_test_faint"));      // a Stability cost too small to move a float of 0.5
	// B6b: four galaxy laws of the fourth rung (Scope HomeGalaxy), in table order a, b, c, d - a with a Stability cost and an
	// Influence cost of 5, the others with nothing.
	const FName LawA(TEXT("edict_test_law_a"));
	const FName LawB(TEXT("edict_test_law_b"));
	const FName LawC(TEXT("edict_test_law_c"));
	const FName LawD(TEXT("edict_test_law_d"));

	const FName Wake(TEXT("crisis_test_wake"));
	const FName Sick(TEXT("crisis_test_sick"));
	const FName Claimed(TEXT("crisis_test_claim"));
	const FName Unrest(TEXT("crisis_test_unrest"));

	FCosmicEffect Effect(EEffectType Type, const TCHAR* Key, float Value)
	{
		FCosmicEffect Out;
		Out.Type = Type;
		Out.Key = Key ? FName(Key) : FName();
		Out.Value = Value;
		return Out;
	}

	FResourcePool Influence(float Amount)
	{
		FResourcePool Pool;
		Pool.Influence = Amount;
		return Pool;
	}

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
		Add(Taxing, [](FEdictDef& Row)
		{
			Row.EnactCost = Influence(10.f);
			Row.UpkeepPerTick = Influence(0.5f);
			Row.EffectsPerTick = { Effect(EEffectType::MultiplyIncome, TEXT("Energy"), 1.1f), Effect(EEffectType::ChangeStability, nullptr, -0.01f) };
		});
		Add(Lifting, [](FEdictDef& Row)
		{
			Row.EffectsPerTick = { Effect(EEffectType::MultiplyIncome, TEXT("Energy"), 1.3f), Effect(EEffectType::ChangeStability, nullptr, 0.002f) };
		});
		Add(Plain, [](FEdictDef&) {});
		Add(Answer, [](FEdictDef&) {});
		Add(Guard, [](FEdictDef& Row)
		{
			Row.HazardSpawnMultipliers.Add(Tag(TEXT("Hazard.Plague")), 0.f);
			Row.HazardCountdownBonus.Add(Tag(TEXT("Hazard.Claim")), 100);
			Row.HazardCountdownBonus.Add(Tag(TEXT("Hazard.Rebellion")), 200);
		});
		Add(High, [](FEdictDef& Row) { Row.RequiredTier = EAscensionTier::Tier2_Stellar; });
		Add(Bound, [](FEdictDef& Row)
		{
			FCosmicCondition Book;
			Book.Type = EConditionType::CivHasTrait;
			Book.Tag = Tag(TEXT("Trait.Faith.Book"));
			Row.EnactConditions = { Book };
		});
		Add(Dear, [](FEdictDef& Row) { Row.EnactCost = Influence(1.0e9f); });
		Add(Heavy, [](FEdictDef& Row)
		{
			Row.UpkeepPerTick = Influence(1.0e9f);
			Row.UpkeepPerTick.Energy = 0.01f;
			Row.EffectsPerTick = { Effect(EEffectType::ChangeStability, nullptr, -0.01f) };
		});
		Add(Kin, [](FEdictDef& Row) { Row.EffectsPerTick = { Effect(EEffectType::ChangeRelation, nullptr, 0.1f) }; });
		Add(Faint, [](FEdictDef& Row) { Row.EffectsPerTick = { Effect(EEffectType::ChangeStability, nullptr, -1.0e-30f) }; });
		for (const FName Law : { LawA, LawB, LawC, LawD })
		{
			Add(Law, [Law](FEdictDef& Row)
			{
				Row.Scope = ECosmicEdictScope::HomeGalaxy;
				Row.RequiredTier = EAscensionTier::Tier4_Transcendent;
				if (Law == LawA)
				{
					Row.EnactCost = Influence(5.f);
					Row.EffectsPerTick = { Effect(EEffectType::ChangeStability, nullptr, -0.01f) };
				}
			});
		}
		return Table;
	}

	// The hazards of this file: no chance of their own unless a case says so.
	UDataTable* Hazards(float SickChance)
	{
		UDataTable* Table = CosmicTests::NewTable(FHazardDef::StaticStruct());
		FHazardDef Woken;
		Woken.HazardTag = Tag(TEXT("Hazard.AIAwakening"));
		Woken.DisplayName = FText::AsCultureInvariant(TEXT("wake"));
		Woken.CountdownTicks = 3;
		Woken.SpawnRule = ECosmicHazardSpawn::Draw;
		Woken.SpawnChancePerTick = 0.f;
		Woken.OnExpireEffects = { Effect(EEffectType::ChangeStability, nullptr, -0.2f) };
		Woken.OnResolveEffects = { Effect(EEffectType::ChangeStability, nullptr, 0.05f) };
		Woken.RulerResolutions = { Answer };
		Woken.ExpireTemplates = { FText::AsCultureInvariant(TEXT("{System} expired.")) };
		Woken.ResolveTemplates = { FText::AsCultureInvariant(TEXT("{System} resolved.")) };
		Table->AddRow(Wake, Woken);
		FHazardDef Plague;
		Plague.HazardTag = Tag(TEXT("Hazard.Plague"));
		Plague.DisplayName = FText::AsCultureInvariant(TEXT("sick"));
		Plague.CountdownTicks = 200;
		Plague.SpawnRule = ECosmicHazardSpawn::Draw;
		Plague.SpawnChancePerTick = SickChance;
		Table->AddRow(Sick, Plague);
		FHazardDef Claim;
		Claim.HazardTag = Tag(TEXT("Hazard.Claim"));
		Claim.DisplayName = FText::AsCultureInvariant(TEXT("claim"));
		Claim.CountdownTicks = 300;
		Claim.SpawnRule = ECosmicHazardSpawn::ClaimRule;
		Table->AddRow(Claimed, Claim);
		FHazardDef Rebellion;
		Rebellion.HazardTag = Tag(TEXT("Hazard.Rebellion"));
		Rebellion.DisplayName = FText::AsCultureInvariant(TEXT("unrest"));
		Rebellion.CountdownTicks = 200;
		Rebellion.SpawnRule = ECosmicHazardSpawn::Capital;
		// The capital rule never places it by itself: a case places it.
		FCosmicCondition Never;
		Never.Type = EConditionType::GalaxyTickAtLeast;
		Never.Value = 1.0e9f;
		Rebellion.SpawnConditions = { Never };
		Table->AddRow(Unrest, Rebellion);
		return Table;
	}

	// A board of twelve systems and four peoples under quiet rules (no Lottery event, no drain, no first crisis, no projects,
	// no inherited edict) with this file's edicts and hazards. The player rules civ 0.
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
		Edit(Rules);
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));
		Tables.Edicts = Edicts();
		Tables.Hazards = Hazards(SickChance);
		Tables.Events = nullptr;
		Sim.SetTables(Tables);
		Sim.NewGalaxy(42, 12, 4);
		return &Sim.GetMutableState();
	}

	void NoEdit(FCosmicRulesDef&) {}

	// B6b: the tables of the board with one feat row of the fourth rung that reads the held laws, so that the row judged there
	// asks for laws (CosmicEdicts::AsksForLawsOf) - the AI's law step and the measuring policy's. Rung puts the row on another
	// rung (one rung up from it), for a case that only the rung check of AsksForLawsOf keeps still.
	void AskForLaws(UGalaxySimulationSubsystem& Sim, EAscensionTier Rung = EAscensionTier::Tier4_Transcendent)
	{
		FAscensionFeatDef Row;
		Row.Title = FText::AsCultureInvariant(TEXT("laws"));
		Row.RequiredTier = Rung;
		Row.TargetTier = static_cast<EAscensionTier>(static_cast<uint8>(Rung) + 1);
		FCosmicCondition Held;
		Held.Type = EConditionType::CivLawsHeldTicksAtLeast;
		Held.Key = FName(TEXT("600"));
		Row.CompletionConditions = { Held };
		UDataTable* Feats = CosmicTests::NewTable(FAscensionFeatDef::StaticStruct());
		Feats->AddRow(FName(TEXT("feat_test_laws")), Row);
		FCosmicDataTables Tables = Sim.GetTables();
		Tables.Feats = Feats;
		Sim.SetTables(Tables);
	}

	int32 CapitalOf(const UGalaxySimulationSubsystem& Sim, int32 CivIndex)
	{
		const FGalaxyState& State = Sim.GetState();
		return Sim.GetStateIndex().FindSystem(State, State.Civs[CivIndex].HomeSystemID);
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
}

// The commands (header section 2.3): every refusal changes nothing, an enactment pays its cost in full and tells one line,
// a revocation keeps the order of the rest, and neither draws from the sim stream.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimEdictCommandTest, "Cosmic_Ascension.Sim.EdictCommand", CosmicTests::Flags)
bool FCosmicSimEdictCommandTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEdictTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	UCosmicLogSubsystem* Log = Sim ? Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("(the role subsystem)"), Role) || !TestNotNull(TEXT("(the log subsystem)"), Log))
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
	FCivilizationState& Ruled = State->Civs[0];
	TestTrue(TEXT("(the player rules civ 0)"), Ruled.bIsPlayerRuled && State->Player.RuledCivID == Ruled.CivID);
	Ruled.Stockpile.Influence = 50.f;

	// Refusals: nothing changes - the state, both streams, the log.
	{
		const uint32 HashBefore = Sim->ComputeStateHash();
		const int32 SeedBefore = Sim->GetRng().GetCurrentSeed();
		const int32 TellBefore = State->TellRngState;
		TestTrue(TEXT("A name that is no edict is NoRow"), Role->EnactEdict(FName(TEXT("edict_nobody"))) == ECosmicEdictBlock::NoRow);
		TestTrue(TEXT("An edict of a higher tier is Tier"), Role->EnactEdict(High) == ECosmicEdictBlock::Tier);
		TestTrue(TEXT("An edict whose conditions fail is Conditions"), Role->EnactEdict(Bound) == ECosmicEdictBlock::Conditions);
		TestTrue(TEXT("An edict the stockpile cannot pay is Cost"), Role->EnactEdict(Dear) == ECosmicEdictBlock::Cost);
		TestFalse(TEXT("A revocation of what is not held is refused"), Role->RevokeEdict(Taxing));
		TestEqual(TEXT("The refusals change nothing"), Sim->ComputeStateHash(), HashBefore);
		TestEqual(TEXT("and draw nothing"), Sim->GetRng().GetCurrentSeed(), SeedBefore);
		TestEqual(TEXT("not even a line"), State->TellRngState, TellBefore);
	}

	// An enactment: the cost in full, the row at the end, one line of the ruled people; no draw of the sim stream.
	{
		const int32 SeedBefore = Sim->GetRng().GetCurrentSeed();
		const int32 Lines = State->Log.Num();
		TestTrue(TEXT("The ruler enacts an edict"), Role->EnactEdict(Taxing) == ECosmicEdictBlock::None);
		TestEqual(TEXT("paying its cost in full"), Ruled.Stockpile.Influence, 40.f);
		TestTrue(TEXT("and holding it"), Ruled.ActiveEdicts == TArray<FName>({ Taxing }));
		TestEqual(TEXT("One line is told"), State->Log.Num(), Lines + 1);
		if (State->Log.Num() == Lines + 1)
		{
			const FCosmicLogEntry& Line = State->Log.Last();
			TestTrue(TEXT("the enactment's, Severity 1, about the ruled people"), Line.SourceRow == CosmicRows::LogEdictEnacted && Line.Severity == 1 && Line.FocusCivID == Ruled.CivID);
		}
		TestEqual(TEXT("The command draws nothing from the sim stream"), Sim->GetRng().GetCurrentSeed(), SeedBefore);
		TestTrue(TEXT("An edict held is Held"), Role->EnactEdict(Taxing) == ECosmicEdictBlock::Held);
		TestTrue(TEXT("With one slot taken, another is Slots"), Role->EnactEdict(Plain) == ECosmicEdictBlock::Slots);
	}

	// Three slots at Society 1.0; a revocation keeps the order of the rest and tells a line.
	{
		Ruled.Research.Add(FName(TEXT("Society")), 1.f);
		TestTrue(TEXT("At Society 1.0 a second edict"), Role->EnactEdict(Plain) == ECosmicEdictBlock::None);
		TestTrue(TEXT("and a third"), Role->EnactEdict(Lifting) == ECosmicEdictBlock::None);
		TestTrue(TEXT("(held in the order enacted)"), Ruled.ActiveEdicts == TArray<FName>({ Taxing, Plain, Lifting }));
		const int32 Lines = State->Log.Num();
		TestTrue(TEXT("The first one is revoked"), Role->RevokeEdict(Taxing));
		TestTrue(TEXT("and the other two keep their order"), Ruled.ActiveEdicts == TArray<FName>({ Plain, Lifting }));
		TestTrue(TEXT("with the revocation's line"), State->Log.Num() == Lines + 1 && State->Log.Last().SourceRow == CosmicRows::LogEdictRevoked);
		TestEqual(TEXT("A revocation is free"), Ruled.Stockpile.Influence, 40.f);
		TestTrue(TEXT("(the line names the edict revoked)"), State->Log.Last().FocusCivID == Ruled.CivID);
	}

	// Only an embodied ruler gives an edict.
	{
		const EPlayerRole RoleWas = State->Player.Role;
		State->Player.Role = EPlayerRole::Wanderer;
		TestTrue(TEXT("A wanderer is NotRuler"), Role->EnactEdict(Plain) == ECosmicEdictBlock::NotRuler);
		TestFalse(TEXT("and revokes nothing"), Role->RevokeEdict(Plain));
		State->Player.Role = RoleWas;
	}

	// A handler of a Core Logic delegate gives no command: an error line and nothing done.
	{
		const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
		Sim->SetBroadcastEnabled(true);
		ON_SCOPE_EXIT { Sim->SetBroadcastEnabled(bWasBroadcasting); };
		CosmicTests::FScopedProbe Probe(*Sim);
		bool bTried = false;
		ECosmicEdictBlock Enacted = ECosmicEdictBlock::None;
		bool bRevoked = true;
		Probe.Probe->OnLog = [&](const FCosmicLogEntry&)
		{
			if (!bTried)
			{
				bTried = true;
				Enacted = Role->EnactEdict(Taxing);
				bRevoked = Role->RevokeEdict(Plain);
			}
		};
		AddExpectedError(TEXT("EnactEdict called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		AddExpectedError(TEXT("RevokeEdict called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		Log->Push(Tag(TEXT("Log.Civ")), 0, FText::AsCultureInvariant(TEXT("a line")), FFormatNamedArguments(), NAME_None, FName(TEXT("log_test")));
		TestTrue(TEXT("The handler ran"), bTried);
		TestTrue(TEXT("and enacted nothing (Handler)"), Enacted == ECosmicEdictBlock::Handler);
		TestFalse(TEXT("and revoked nothing"), bRevoked);
		TestTrue(TEXT("(the edicts are as they were)"), Ruled.ActiveEdicts == TArray<FName>({ Plain, Lifting }));
	}
	return true;
}

// The economy stage (header section 2.4): the edicts' per-tick effects after the income and in ActiveEdicts order, before
// the drain and the transcendents (whose subject they never borrow); the upkeep after the income, in full or the edict lapses.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimEdictEconomyTest, "Cosmic_Ascension.Sim.EdictEconomy", CosmicTests::Flags)
bool FCosmicSimEdictEconomyTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEdictTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	// The income of a tick without edicts, the reference.
	FGalaxyState* State = LayBoard(*Sim, Original, NoEdit);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	State->Civs[0].Stockpile = FResourcePool();
	Sim->StepOnce();
	const int32 Seed0 = Sim->GetRng().GetCurrentSeed();
	const float Energy0 = State->Civs[0].IncomePerTick.Energy;
	const float Influence0 = State->Civs[0].IncomePerTick.Influence;
	TestTrue(TEXT("(the capital earns Energy and Influence)"), Energy0 > 0.f && Influence0 > 0.5f);

	// Two edicts in one order and in the other: the multiplications in ActiveEdicts order, after the income.
	auto RunWith = [&](const TArray<FName>& Held, float Stability) -> FGalaxyState*
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		Board->Civs[0].Stockpile = FResourcePool();
		Board->Civs[0].ActiveEdicts = Held;
		Board->Civs[0].Stability = Stability;
		Sim->StepOnce();
		return Board;
	};
	{
		FGalaxyState* Board = RunWith({ Taxing, Lifting }, 0.305f);
		const FCivilizationState& Civ = Board->Civs[0];
		TestEqual(TEXT("Taxing then lifting: the income times 1.1 then 1.3"), Civ.IncomePerTick.Energy, (Energy0 * 1.1f) * 1.3f, 0.f);
		TestEqual(TEXT("The edicts draw nothing from the sim stream"), Sim->GetRng().GetCurrentSeed(), Seed0);
		// The recovery first (below 0.6), then -0.01, then +0.002 - all in this tick's discontent count.
		const float Expected = FMath::Clamp(FMath::Clamp(FMath::Min(0.6f, 0.305f + 0.001f) + -0.01f, 0.f, 1.f) + 0.002f, 0.f, 1.f);
		TestEqual(TEXT("The Stability of the effects, after the recovery, in order"), Civ.Stability, Expected, 0.f);
		TestEqual(TEXT("counted in the same tick's discontent"), Civ.LowStabilityTicks, 1);
		TestEqual(TEXT("The upkeep is paid after this tick's income (an empty stockpile pays)"), Civ.Stockpile.Influence, Influence0 - 0.5f, 0.f);
		TestTrue(TEXT("and both edicts stand"), Civ.ActiveEdicts == TArray<FName>({ Taxing, Lifting }));
	}
	{
		FGalaxyState* Board = RunWith({ Lifting, Taxing }, 0.5f);
		TestEqual(TEXT("Lifting then taxing: the other order"), Board->Civs[0].IncomePerTick.Energy, (Energy0 * 1.3f) * 1.1f, 0.f);
	}
	// The order shows through a clamp: from 0.005 (0.006 after the recovery) taxing then lifting ends at 0.002, the other
	// order at 0.
	TestEqual(TEXT("Taxing then lifting from 0.005: 0.002"), RunWith({ Taxing, Lifting }, 0.005f)->Civs[0].Stability, 0.002f, 0.f);
	TestEqual(TEXT("Lifting then taxing from 0.005: 0"), RunWith({ Lifting, Taxing }, 0.005f)->Civs[0].Stability, 0.f, 0.f);
	// A Stability edict lifts a people out of discontent in the same tick: the count starts over.
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		Board->Civs[0].ActiveEdicts = { Lifting };
		Board->Civs[0].Stability = 0.299f;
		Board->Civs[0].LowStabilityTicks = 5;
		Sim->StepOnce();
		TestEqual(TEXT("0.299 + 0.001 + 0.002 leaves the discontent: the count is 0"), Board->Civs[0].LowStabilityTicks, 0);
	}
	// The upkeep in ActiveEdicts order: two edicts of 0.6 of the income each on an empty stockpile - the first pays, the
	// second lapses.
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		UDataTable* Table = Sim->GetTables().Edicts;
		const FName First(TEXT("edict_test_first"));
		const FName Second(TEXT("edict_test_second"));
		FEdictDef Costly;
		Costly.UpkeepPerTick = Influence(0.6f * Influence0);
		Table->AddRow(First, Costly);
		Table->AddRow(Second, Costly);
		Board->Civs[0].Stockpile = FResourcePool();
		Board->Civs[0].ActiveEdicts = { First, Second };
		Sim->StepOnce();
		TestTrue(TEXT("The first edict pays its upkeep and the second lapses"), Board->Civs[0].ActiveEdicts == TArray<FName>({ First }));
	}

	// An upkeep that cannot be paid: the edict lapses that tick, its effects of the tick applied; the ruled people is told,
	// an AI people is not.
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, [](FCosmicRulesDef& Rules) { Rules.AIEdictRevokeStability = 0.f; });
		Board->Civs[0].Stockpile = FResourcePool();
		Board->Civs[0].ActiveEdicts = { Heavy };
		Board->Civs[0].Stability = 0.8f;
		Board->Civs[1].Stockpile = FResourcePool();
		Board->Civs[1].ActiveEdicts = { Heavy };
		Board->Civs[1].Stability = 0.8f;
		const int32 Lapses = LinesOf(*Board, CosmicRows::LogEdictLapsed);
		Sim->StepOnce();
		TestTrue(TEXT("An upkeep no stockpile pays: the edict lapses"), Board->Civs[0].ActiveEdicts.IsEmpty());
		TestEqual(TEXT("with its effect of that tick applied"), Board->Civs[0].Stability, 0.8f - 0.01f);
		TestEqual(TEXT("and nothing paid: the payable part of the upkeep stays in the stockpile"), Board->Civs[0].Stockpile.Energy, Board->Civs[0].IncomePerTick.Energy, 0.f);
		TestEqual(TEXT("(the Influence too)"), Board->Civs[0].Stockpile.Influence, Board->Civs[0].IncomePerTick.Influence, 0.f);
		TestTrue(TEXT("(the AI people's lapses too)"), Board->Civs[1].ActiveEdicts.IsEmpty());
		TestEqual(TEXT("Only the ruled people is told"), LinesOf(*Board, CosmicRows::LogEdictLapsed), Lapses + 1);
	}

	// The subject of an edict's effect is the people alone: the transcendents' loop of the people before leaves its OtherID
	// behind, and an edict that names no party changes no relation.
	{
		UDataTable* Transcendents = CosmicTests::NewTable(FTranscendentDef::StaticStruct());
		FTranscendentDef Fallen;
		Fallen.Doctrine = Tag(TEXT("Doctrine.Silence"));
		Fallen.Allegiance = Tag(TEXT("Realm.Abyssal"));
		Fallen.Tier = EAscensionTier::Tier4_Transcendent;
		Fallen.PressureEffectsPerTick = { Effect(EEffectType::ChangeStability, nullptr, 0.f) };
		const FName Presser(TEXT("trans_test_presser"));
		Transcendents->AddRow(Presser, Fallen);
		auto Pressed = [&](bool bKin) -> const FCivilizationState&
		{
			FGalaxyState* Board = LayBoard(*Sim, Original, [](FCosmicRulesDef& Rules) { Rules.AIEdictRevokeStability = 0.f; });
			FCosmicDataTables Tables = Sim->GetTables();
			Tables.Transcendents = Transcendents;
			Sim->SetTables(Tables);
			Board->Transcendents.Reset();
			CosmicTests::AddInstance(*Board, Presser, { Board->Civs[0].HomeSystemID });
			Board->Civs[1].ActiveEdicts = bKin ? TArray<FName>({ Kin }) : TArray<FName>();
			Sim->StepOnce();
			return Board->Civs[1];
		};
		TestFalse(TEXT("(the people after the pressed one has no relation to the presser)"), Pressed(false).Relations.Contains(Presser));
		TestFalse(TEXT("An edict with no party does not borrow the presser of the people before"), Pressed(true).Relations.Contains(Presser));
	}
	return true;
}

// The crisis stage (header section 2.5): the spawn share times the owner's edicts (the draws do not change), the countdown
// bonus at placement only, and the resolution at the expiry when the owner holds an answer.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimEdictHazardsTest, "Cosmic_Ascension.Sim.EdictHazards", CosmicTests::Flags)
bool FCosmicSimEdictHazardsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEdictTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	// (1) The spawn share: a plague certain on every free settled system, and none on the guarded people's.
	{
		auto Tick = [&](bool bGuard, int32& OutSeed) -> bool
		{
			FGalaxyState* Board = LayBoard(*Sim, Original, [](FCosmicRulesDef& Rules) { Rules.AIEdictRevokeStability = 0.f; }, 1.f);
			Board->Civs[0].ActiveEdicts = bGuard ? TArray<FName>({ Guard }) : TArray<FName>();
			Sim->StepOnce();
			OutSeed = Sim->GetRng().GetCurrentSeed();
			return Board->Systems[CapitalOf(*Sim, 0)].ActiveHazard == Tag(TEXT("Hazard.Plague"));
		};
		int32 SeedOpen = 0;
		int32 SeedGuarded = 0;
		TestTrue(TEXT("(without the edict the plague falls on the capital)"), Tick(false, SeedOpen));
		TestFalse(TEXT("With a share factor of 0 it does not"), Tick(true, SeedGuarded));
		TestEqual(TEXT("and the sim stream draws as many numbers"), SeedGuarded, SeedOpen);
	}

	// (2) The countdown: added when a hazard is placed on the people, never to one standing.
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		const int32 Capital = CapitalOf(*Sim, 0);
		Board->Civs[0].ActiveEdicts = { Guard };
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		TestTrue(TEXT("(a claim is placed)"), CosmicHazards::Place(Ctx, Capital, Claimed));
		TestEqual(TEXT("A claim on a guarded people: 300 + 100"), Board->Systems[Capital].HazardTicksRemaining, 400);
		CosmicHazards::EmptyHazardSlot(Board->Systems[Capital]);
		TestTrue(TEXT("(a rebellion is placed)"), CosmicHazards::Place(Ctx, Capital, Unrest));
		TestEqual(TEXT("A rebellion: 200 + 200"), Board->Systems[Capital].HazardTicksRemaining, 400);
		CosmicHazards::EmptyHazardSlot(Board->Systems[Capital]);
		Board->Civs[0].ActiveEdicts.Reset();
		TestTrue(TEXT("(a claim is placed without the edict)"), CosmicHazards::Place(Ctx, Capital, Claimed));
		Board->Civs[0].ActiveEdicts = { Guard };
		Sim->StepOnce();
		TestEqual(TEXT("A standing claim keeps its count when the edict comes"), Board->Systems[Capital].HazardTicksRemaining, 299);
	}

	// (3) The expiry: the answer held resolves (the resolution's effects, not the expiry's); not held, it expires. A
	// hazard whose answers are edicts only gives the AI and the project key nothing to start.
	auto Expire = [&](bool bHeld) -> float
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		const int32 Capital = CapitalOf(*Sim, 0);
		Board->Civs[0].Stability = 0.6f;
		Board->Civs[0].ActiveEdicts = bHeld ? TArray<FName>({ Answer }) : TArray<FName>();
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		CosmicHazards::Place(Ctx, Capital, Wake);
		TestTrue(TEXT("(the answer is no project: nothing to start)"), !CosmicMegaprojects::Choices(*Board, Sim->GetStateIndex(), Sim->GetTables(), 0, 0)
			.ContainsByPredicate([Capital](const TPair<int32, FName>& Choice) { return Choice.Key == Capital; }));
		Sim->StepOnce();
		TestTrue(TEXT("No project fills the response slot of a hazard answered by an edict"), Board->Systems[Capital].ResponseMegaproject.IsNone());
		Sim->RunHeadless(3);
		TestFalse(TEXT("(the hazard has ended)"), Board->Systems[Capital].ActiveHazard.IsValid());
		return Board->Civs[0].Stability;
	};
	TestEqual(TEXT("Held at the expiry: resolved (+0.05)"), Expire(true), 0.65f);
	TestEqual(TEXT("Not held: it expires (-0.2)"), Expire(false), 0.4f, 0.0015f);

	// (4) With a fallen sphere one lane from the system (and another settled system of the people outside it, so it is not
	// the last unclaimed one), an expiry places a claim; an answer held resolves instead, places none and tells the
	// resolution line.
	{
		UDataTable* Transcendents = CosmicTests::NewTable(FTranscendentDef::StaticStruct());
		FTranscendentDef Claimer;
		Claimer.Doctrine = Tag(TEXT("Doctrine.Silence"));
		Claimer.Allegiance = Tag(TEXT("Realm.Abyssal"));
		Claimer.Tier = EAscensionTier::Tier4_Transcendent;
		Claimer.MaxInfluencedSystems = 12;
		const FName ClaimerName(TEXT("trans_test_claimer"));
		Transcendents->AddRow(ClaimerName, Claimer);
		auto Run = [&](bool bHeld, FString& OutLine) -> FGameplayTag
		{
			FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
			if (!TestTrue(TEXT("(the chain is wired)"), CosmicTests::WireChain(*Board)))
			{
				return FGameplayTag();
			}
			CosmicTests::ClearOwners(*Board);
			CosmicTests::Give(*Board, 0, { 2, 0 });
			CosmicTests::Give(*Board, 1, { 6 });
			CosmicTests::Give(*Board, 2, { 8 });
			CosmicTests::Give(*Board, 3, { 11 });
			FCosmicDataTables Tables = Sim->GetTables();
			Tables.Transcendents = Transcendents;
			Sim->SetTables(Tables);
			Board->Transcendents.Reset();
			CosmicTests::AddInstance(*Board, ClaimerName, { Board->Systems[3].SystemID });
			Board->Civs[0].ActiveEdicts = bHeld ? TArray<FName>({ Answer }) : TArray<FName>();
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			CosmicHazards::Place(Ctx, 2, Wake);
			Sim->RunHeadless(3);
			OutLine.Reset();
			for (int32 At = Board->Log.Num() - 1; At >= 0; --At)
			{
				if (Board->Log[At].SourceRow == Wake)
				{
					OutLine = Board->Log[At].Message.ToString();
					break;
				}
			}
			return Board->Systems[2].ActiveHazard;
		};
		FString Line;
		TestTrue(TEXT("Not held: the expiry's claim rule places a claim"), Run(false, Line) == Tag(TEXT("Hazard.Claim")));
		TestTrue(TEXT("after the expiry's line"), Line.Contains(TEXT("expired")));
		TestFalse(TEXT("Held: the resolution places no claim"), Run(true, Line).IsValid());
		TestTrue(TEXT("and tells the resolution line"), Line.Contains(TEXT("resolved")));
	}
	return true;
}

// The AI's edicts (header section 2.6): it revokes, it never enacts; the ruled people is passed by, a regent is not.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimEdictAITest, "Cosmic_Ascension.Sim.EdictAI", CosmicTests::Flags)
bool FCosmicSimEdictAITest::RunTest(const FString& Parameters)
{
	using namespace CosmicEdictTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	// With the threshold 0.5: an edict with a Stability cost stays at or above it and goes below; any other goes at once.
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, [](FCosmicRulesDef& Rules) { Rules.AIEdictRevokeStability = 0.5f; });
		Board->Civs[1].ActiveEdicts = { Taxing, Plain };
		Board->Civs[1].Stability = 0.8f;
		Board->Civs[1].Stockpile.Influence = 1000.f;
		Board->Civs[2].ActiveEdicts = { Taxing };
		Board->Civs[2].Stability = 0.5f + 0.01f;
		Board->Civs[2].Stockpile.Influence = 1000.f;
		Board->Civs[3].ActiveEdicts = { Taxing };
		Board->Civs[3].Stability = 0.5f + 0.0089f;
		Board->Civs[3].Stockpile.Influence = 1000.f;
		Board->Civs[0].ActiveEdicts = { Taxing, Plain };
		Board->Civs[0].Stability = 0.2f;
		Board->Civs[0].Stockpile.Influence = 1000.f;
		Sim->StepOnce();
		TestTrue(TEXT("A Stability cost above the threshold stays; another edict goes at once"), Board->Civs[1].ActiveEdicts == TArray<FName>({ Taxing }));
		TestTrue(TEXT("Just above the threshold (0.51 + 0.001 - 0.01) it stays"), Board->Civs[2].ActiveEdicts == TArray<FName>({ Taxing }));
		TestTrue(TEXT("Below it (0.5089 + 0.001 - 0.01), it goes"), Board->Civs[3].ActiveEdicts.IsEmpty());
		TestTrue(TEXT("The ruled people is passed by"), Board->Civs[0].ActiveEdicts == TArray<FName>({ Taxing, Plain }));
	}
	// Exactly at the threshold (no recovery, a cost too small to move 0.5): it stays - the rule is 'below'.
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, [](FCosmicRulesDef& Rules) { Rules.AIEdictRevokeStability = 0.5f; Rules.StabilityRecoveryPerTick = 0.f; });
		Board->Civs[1].ActiveEdicts = { Faint };
		Board->Civs[1].Stability = 0.5f;
		Sim->StepOnce();
		TestEqual(TEXT("(the people stands at 0.5)"), Board->Civs[1].Stability, 0.5f, 0.f);
		TestTrue(TEXT("At exactly the threshold the edict stays"), Board->Civs[1].ActiveEdicts == TArray<FName>({ Faint }));
	}

	// The AI never enacts, not even the answer to its own standing hazard; a regent (a people the player no longer rules)
	// revokes what the ruler left. No draw.
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		CosmicHazards::Place(Ctx, CapitalOf(*Sim, 1), Wake);
		Board->Civs[1].Stockpile.Influence = 1000.f;
		Board->Civs[0].ActiveEdicts = { Answer, Plain };
		Board->Civs[0].bIsPlayerRuled = false;
		Sim->StepOnce();
		TestTrue(TEXT("The AI does not enact the answer to its hazard"), Board->Civs[1].ActiveEdicts.IsEmpty());
		TestTrue(TEXT("A regent revokes what the ruler left"), Board->Civs[0].ActiveEdicts.IsEmpty());
	}
	{
		int32 Seeds[2] = { 0, 0 };
		for (int32 Pass = 0; Pass < 2; ++Pass)
		{
			FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
			Board->Civs[2].ActiveEdicts = Pass == 0 ? TArray<FName>() : TArray<FName>({ Plain, Answer });
			Sim->StepOnce();
			Seeds[Pass] = Sim->GetRng().GetCurrentSeed();
		}
		TestEqual(TEXT("The revocations draw nothing"), Seeds[1], Seeds[0]);
	}

	// B6b (decision 11): AIRevokesOf passes a galaxy law by - one with a Stability cost too, below the threshold - and a regent's
	// people keeps the law the ruler left, while what else it holds goes (mutation: AIRevokesOf walks the laws as well).
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		Board->Civs[1].ActiveEdicts = { LawA, Plain };
		Board->Civs[1].Stability = 0.5f;
		TestTrue(TEXT("AIRevokesOf names the edict of the people's scope and not the law"),
			CosmicEdicts::AIRevokesOf(Board->Civs[1], Sim->GetTables(), Sim->GetRules()) == TArray<FName>({ Plain }));
		Board->Civs[0].ActiveEdicts = { LawB, Taxing };
		Board->Civs[0].bIsPlayerRuled = false;
		Board->Civs[0].Stockpile.Influence = 1000.f;
		Sim->StepOnce();
		TestTrue(TEXT("An AI people keeps a galaxy law, its Stability cost and all"), Board->Civs[1].ActiveEdicts == TArray<FName>({ LawA }));
		TestTrue(TEXT("A regent's people keeps the law the ruler left, and lets the rest go"), Board->Civs[0].ActiveEdicts == TArray<FName>({ LawB }));
	}
	return true;
}

// The slots (header section 2.7): 1 + the Society marks reached; full slots refuse an enactment; holding more than the
// slots revokes nothing; a name the table does not hold takes no slot.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimEdictSlotsTest, "Cosmic_Ascension.Sim.EdictSlots", CosmicTests::Flags)
bool FCosmicSimEdictSlotsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEdictTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
	if (!TestNotNull(TEXT("The board is laid"), Board))
	{
		return false;
	}
	const FCosmicRulesDef& Rules = Sim->GetRules();
	FCivilizationState& Civ = Board->Civs[0];
	const FName Society(TEXT("Society"));
	const struct { float Society; int32 Slots; } Cases[] = { { 0.f, 1 }, { 0.4999f, 1 }, { 0.5f, 2 }, { 0.9999f, 2 }, { 1.f, 3 } };
	for (const auto& Case : Cases)
	{
		Civ.Research.Add(Society, Case.Society);
		TestEqual(*FString::Printf(TEXT("At Society %.4f, %d slots"), Case.Society, Case.Slots), CosmicEdicts::SlotsOf(Civ, Rules), Case.Slots);
	}
	Civ.Research.Add(Society, 0.f);
	Civ.ActiveEdicts = { Plain, FName(TEXT("edict_gone")) };
	TestEqual(TEXT("A name the table does not hold takes no slot"), CosmicEdicts::HeldCountOf(Civ, Sim->GetTables()), 1);
	TestTrue(TEXT("One known edict fills the first slot"), CosmicEdicts::EnactBlockOf(*Board, Sim->GetStateIndex(), Sim->GetTables(), Rules, 0, Taxing) == ECosmicEdictBlock::Slots);
	Civ.ActiveEdicts = { Plain, Lifting };
	Civ.Stockpile.Influence = 1000.f;
	Sim->StepOnce();
	TestTrue(TEXT("Holding more than the slots revokes nothing"), Civ.ActiveEdicts == TArray<FName>({ Plain, Lifting }));
	return true;
}

// The inherited edict (header section 2.7): every people of a new galaxy holds it, or none when the knob is empty or names
// no row; the AI lets it go at its first stage (the threshold 1); the same seed gives the same galaxy.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimStartingEdictTest, "Cosmic_Ascension.Sim.StartingEdict", CosmicTests::Flags)
bool FCosmicSimStartingEdictTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEdictTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	auto Lay = [&](FName Starting) { return LayBoard(*Sim, Original, [Starting](FCosmicRulesDef& Rules) { Rules.StartingEdict = Starting; Rules.AIEdictRevokeStability = 1.f; }); };
	{
		FGalaxyState* Board = Lay(Taxing);
		bool bAll = Board->Civs.Num() == 4;
		for (const FCivilizationState& Civ : Board->Civs)
		{
			bAll = bAll && Civ.ActiveEdicts == TArray<FName>({ Taxing });
		}
		TestTrue(TEXT("Every people starts holding the inherited edict"), bAll);
		const uint32 Hash = Sim->ComputeStateHash();
		const uint32 World = Sim->ComputeWorldHash();
		Sim->StepOnce();
		TestTrue(TEXT("The ruled people keeps it"), Board->Civs[0].ActiveEdicts == TArray<FName>({ Taxing }));
		TestTrue(TEXT("an AI people lets it go at its first stage"), Board->Civs[1].ActiveEdicts.IsEmpty() && Board->Civs[3].ActiveEdicts.IsEmpty());
		FGalaxyState* Again = Lay(Taxing);
		TestEqual(TEXT("The same seed gives the same galaxy"), Sim->ComputeStateHash(), Hash);
		TestEqual(TEXT("and the same world"), Sim->ComputeWorldHash(), World);
		TestNotNull(TEXT("(again)"), Again);
	}
	TestTrue(TEXT("An empty knob gives none"), Lay(NAME_None)->Civs[2].ActiveEdicts.IsEmpty());
	TestTrue(TEXT("A knob naming no row gives none"), Lay(FName(TEXT("edict_nobody")))->Civs[0].ActiveEdicts.IsEmpty());

	// The game's own data (the balance commit, B-2c): every people of the home galaxy inherits mobilization, an edict with a
	// Stability cost; the AI lets it go at its first stage and the ruled people keeps it; machine rights answers the
	// machines' awakening; and a ruler who gives no command is deposed by a rebellion after the first murmurs (measured for
	// seed 42: the rebellion at tick 872, the fall at 1,072).
	{
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		const FGalaxyState& Game = Sim->GetState();
		const FName Inherited = Sim->GetRules().StartingEdict;
		const FEdictDef* Row = Original.FindEdict(Inherited);
		TestEqual(TEXT("The game's inherited edict is mobilization"), Inherited, FName(TEXT("edict_mobilization")));
		TestTrue(TEXT("an edict with a Stability cost"), Row && CosmicEdicts::HasStabilityCost(*Row));
		bool bAllHold = Game.Civs.Num() > 1;
		for (const FCivilizationState& Civ : Game.Civs)
		{
			bAllHold = bAllHold && Civ.ActiveEdicts == TArray<FName>({ Inherited });
		}
		TestTrue(TEXT("Every people of the game's galaxy starts holding it"), bAllHold);
		Sim->StepOnce();
		const int32 Ruled = Sim->GetStateIndex().FindCiv(Game, Game.Player.RuledCivID);
		bool bAIFree = Game.Civs.IsValidIndex(Ruled);
		for (int32 CivIndex = 0; CivIndex < Game.Civs.Num(); ++CivIndex)
		{
			bAIFree = bAIFree && (CivIndex == Ruled || Game.Civs[CivIndex].ActiveEdicts.IsEmpty());
		}
		TestTrue(TEXT("After the first tick the ruled people keeps it and every AI people has let it go"), bAIFree && Game.Civs[Ruled].ActiveEdicts == TArray<FName>({ Inherited }));
		const FHazardDef* Awakening = Original.FindHazardByTag(Tag(TEXT("Hazard.AIAwakening")));
		const FName Rights(TEXT("edict_machine_rights"));
		TestTrue(TEXT("Machine rights answers the machines' awakening"), Awakening && Awakening->RulerResolutions.Contains(Rights) && Original.FindEdict(Rights));
		Sim->RunHeadless(299);
		TestEqual(TEXT("Left alone, the ruler still rules at tick 300"), Game.Player.Role, EPlayerRole::Ruler);
		Sim->RunHeadless(1200);
		const bool bFell = Game.Player.Role != EPlayerRole::Ruler && !Game.Calling.Lives.IsEmpty() && !Game.Calling.Lives.Last().ReignEndReasons.IsEmpty()
			&& Game.Calling.Lives.Last().ReignEndReasons.Last() == Tag(TEXT("Reign.Deposed"));
		TestTrue(TEXT("and is deposed by a rebellion by tick 1,500: neglect ends the reign"), bFell);
	}
	return true;
}

// The Society recovery (header section 2.7): rate x (1 + bonus x Society), Society read 0..1; a bonus that is not above 0
// and finite is the old recovery; the inherited edict's invariant.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSocietyRecoveryTest, "Cosmic_Ascension.Sim.SocietyRecovery", CosmicTests::Flags)
bool FCosmicSimSocietyRecoveryTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEdictTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FName Society(TEXT("Society"));
	auto Rise = [&](float Bonus, float SocietyValue) -> float
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, [Bonus](FCosmicRulesDef& Rules)
		{
			Rules.StabilityRecoveryPerTick = 0.001f;
			Rules.StabilityRestingLevel = 0.6f;
			Rules.SocietyRecoveryBonus = Bonus;
		});
		Board->Civs[0].Stability = 0.3f;
		Board->Civs[0].Research.Add(Society, SocietyValue);
		Sim->StepOnce();
		return Board->Civs[0].Stability;
	};
	TestEqual(TEXT("Bonus 0.25 at Society 0.5: 0.001 x (1 + 0.25 x 0.5)"), Rise(0.25f, 0.5f), 0.3f + 0.001f * (1.f + 0.25f * 0.5f));
	TestEqual(TEXT("Society above 1 reads as 1"), Rise(0.25f, 1.5f), 0.3f + 0.001f * (1.f + 0.25f * 1.f));
	TestEqual(TEXT("Bonus 0 is the old recovery"), Rise(0.f, 1.f), 0.3f + 0.001f);
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, [](FCosmicRulesDef& Rules) { Rules.StabilityRecoveryPerTick = 0.f; Rules.SocietyRecoveryBonus = 0.25f; });
		Board->Civs[0].Stability = 0.3f;
		Board->Civs[0].Research.Add(Society, 1.f);
		Sim->StepOnce();
		TestEqual(TEXT("With the recovery off the bonus raises nothing"), Board->Civs[0].Stability, 0.3f, 0.f);
	}
	const float NaN = std::numeric_limits<float>::quiet_NaN();
	const float Infinity = std::numeric_limits<float>::infinity();
	TestEqual(TEXT("A bonus that is not a number is 0"), Rise(NaN, 1.f), 0.3f + 0.001f);
	TestEqual(TEXT("an infinite bonus is 0"), Rise(Infinity, 0.f), 0.3f + 0.001f);
	TestEqual(TEXT("a bonus below 0 is 0"), Rise(-0.5f, 1.f), 0.3f + 0.001f);

	// The invariant (Data.Integrity holds the game's data to it).
	FCosmicRulesDef Rules = Sim->GetRules();
	FCosmicDataTables Tables = Sim->GetTables();
	UDataTable* Drains = CosmicTests::NewTable(FEdictDef::StaticStruct());
	FEdictDef Drain;
	Drain.EffectsPerTick = { Effect(EEffectType::ChangeStability, nullptr, -0.0015f) };
	Drains->AddRow(FName(TEXT("edict_test_drain")), Drain);
	Drains->AddRow(FName(TEXT("edict_test_calm")), FEdictDef());
	Tables.Edicts = Drains;
	Rules.StabilityRecoveryPerTick = 0.001f;
	Rules.StartingEdict = NAME_None;
	TestTrue(TEXT("No inherited edict: nothing to hold"), CosmicEdicts::InheritedDrainOutrunsRecovery(Rules, Tables));
	Rules.StartingEdict = FName(TEXT("edict_test_drain"));
	Rules.SocietyRecoveryBonus = 0.25f;
	TestTrue(TEXT("0.001 x 1.25 < 0.0015"), CosmicEdicts::InheritedDrainOutrunsRecovery(Rules, Tables));
	Rules.SocietyRecoveryBonus = 0.5f;
	TestFalse(TEXT("0.001 x 1.5 is not below 0.0015"), CosmicEdicts::InheritedDrainOutrunsRecovery(Rules, Tables));
	Rules.SocietyRecoveryBonus = 0.f;
	Rules.StartingEdict = FName(TEXT("edict_test_calm"));
	TestFalse(TEXT("An inherited edict with no Stability cost has no failure end"), CosmicEdicts::InheritedDrainOutrunsRecovery(Rules, Tables));
	Rules.StartingEdict = FName(TEXT("edict_nobody"));
	TestFalse(TEXT("nor one the table does not hold"), CosmicEdicts::InheritedDrainOutrunsRecovery(Rules, Tables));
	return true;
}

// The ruler's research shares (header section 2.3, 2.8): integer weights over the fields but transcendence, the
// transcendence share kept, a weight of 0 keeping its key, the refusals; no draw.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimResearchSharesTest, "Cosmic_Ascension.Sim.ResearchShares", CosmicTests::Flags)
bool FCosmicSimResearchSharesTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEdictTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
	if (!TestNotNull(TEXT("The board is laid"), Board))
	{
		return false;
	}
	const FName Physics(TEXT("Physics"));
	const FName Society(TEXT("Society"));
	const FName Engineering(TEXT("Engineering"));
	const FName Transcendence = CosmicResearch::TranscendenceField();
	TMap<FName, float>& Focus = Board->Civs[0].ResearchFocus;
	TestTrue(TEXT("The transcendence field is one of the generated keys"), Focus.Contains(Transcendence) && Focus.Num() == 4);
	const int32 SeedBefore = Sim->GetRng().GetCurrentSeed();
	const uint32 HashBefore = Sim->ComputeStateHash();

	auto Weights = [&](int32 P, int32 S, int32 E)
	{
		TMap<FName, int32> Out;
		Out.Add(Physics, P);
		Out.Add(Society, S);
		Out.Add(Engineering, E);
		return Out;
	};
	TestFalse(TEXT("A weight sum of 0 is refused"), Role->SetResearchShares(Weights(0, 0, 0)));
	TestFalse(TEXT("a weight above 20"), Role->SetResearchShares(Weights(21, 1, 1)));
	TestFalse(TEXT("a weight below 0"), Role->SetResearchShares(Weights(-1, 1, 1)));
	TMap<FName, int32> Three = Weights(1, 1, 1);
	Three.Remove(Engineering);
	TestFalse(TEXT("weights that leave a field out"), Role->SetResearchShares(Three));
	TMap<FName, int32> WithTranscendence = Weights(1, 1, 1);
	WithTranscendence.Add(Transcendence, 1);
	TestFalse(TEXT("weights that hold the transcendence field"), Role->SetResearchShares(WithTranscendence));
	TMap<FName, int32> TranscendenceForOne = Weights(1, 1, 1);
	TranscendenceForOne.Remove(Engineering);
	TranscendenceForOne.Add(Transcendence, 1);
	TestFalse(TEXT("three weights with transcendence in place of a field"), Role->SetResearchShares(TranscendenceForOne));
	TMap<FName, int32> Other = Weights(1, 1, 1);
	Other.Remove(Engineering);
	Other.Add(FName(TEXT("Alchemy")), 1);
	TestFalse(TEXT("weights with a key that is no field"), Role->SetResearchShares(Other));
	TestEqual(TEXT("The refusals change nothing"), Sim->ComputeStateHash(), HashBefore);

	TestTrue(TEXT("Weights 2:1:0 are taken"), Role->SetResearchShares(Weights(2, 1, 0)));
	const float Rest = 1.f - 0.25f;
	TestEqual(TEXT("Physics takes (1 - T) x 2 / 3"), Focus.FindRef(Physics), Rest * 2.f / 3.f);
	TestEqual(TEXT("Society (1 - T) x 1 / 3"), Focus.FindRef(Society), Rest * 1.f / 3.f);
	TestTrue(TEXT("a weight of 0 keeps its key at 0"), Focus.Contains(Engineering) && Focus.FindRef(Engineering) == 0.f);
	TestEqual(TEXT("and transcendence keeps its share"), Focus.FindRef(Transcendence), 0.25f);
	TestFalse(TEXT("The same weights again change nothing"), Role->SetResearchShares(Weights(2, 1, 0)));
	TestTrue(TEXT("Weights 4:2:0 make the same shares"), !Role->SetResearchShares(Weights(4, 2, 0)) && Focus.FindRef(Physics) == Rest * 2.f / 3.f);
	TestEqual(TEXT("No draw"), Sim->GetRng().GetCurrentSeed(), SeedBefore);
	TestEqual(TEXT("The slider's map is mixed"), CosmicResearch::ShapeOf(Focus), ECosmicFocusShape::Mixed);
	return true;
}

// A galaxy saved with edicts held runs on after a load as it would have run (header section 2.9).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveEdictsTest, "Cosmic_Ascension.Sim.SaveEdicts", CosmicTests::Flags)
bool FCosmicSimSaveEdictsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEdictTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	UCosmicSaveSubsystem* Save = Sim ? Sim->GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("(the role subsystem)"), Role) || !TestNotNull(TEXT("(the save subsystem)"), Save))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* Board = LayBoard(*Sim, Original, [](FCosmicRulesDef& Rules) { Rules.AIEdictRevokeStability = 0.f; });
	if (!TestNotNull(TEXT("The board is laid"), Board))
	{
		return false;
	}
	Board->Civs[0].Stockpile.Influence = 100.f;
	Board->Civs[0].Research.Add(FName(TEXT("Society")), 1.f);
	Role->EnactEdict(Taxing);
	Role->EnactEdict(Lifting);
	Board->Civs[2].ActiveEdicts = { Guard };
	const FGuid Galaxy = Board->GalaxyID;
	const TArray<FName> Held = Board->Civs[0].ActiveEdicts;
	if (!TestTrue(TEXT("(the galaxy is saved)"), Save->SaveCurrentGalaxy()))
	{
		return false;
	}
	auto Script = [&]()
	{
		Sim->RunHeadless(20);
		Role->RevokeEdict(Taxing);
		Sim->RunHeadless(20);
	};
	Script();
	const uint32 Straight = Sim->ComputeStateHash();
	Sim->NewGalaxy(7, 5, 3);
	TestTrue(TEXT("The saved galaxy loads"), Save->LoadGalaxy(Galaxy));
	TestTrue(TEXT("with the edicts held in their order"), Sim->GetState().Civs[0].ActiveEdicts == Held && Sim->GetState().Civs[2].ActiveEdicts == TArray<FName>({ Guard }));
	Script();
	TestEqual(TEXT("The same inputs after the load give the same galaxy"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Straight));
	return true;
}

// The chronicle leaves the edict lines out (header section 3.4): the ruler's bookkeeping, not the life's story.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLogChronicleEdictsTest, "Cosmic_Ascension.Log.ChronicleEdicts", CosmicTests::Flags)
bool FCosmicLogChronicleEdictsTest::RunTest(const FString& Parameters)
{
	const FName People(TEXT("civ_0"));
	FCosmicLogEntry Entry;
	Entry.Category = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Civ")));
	Entry.Severity = 1;
	Entry.FocusCivID = People;
	Entry.SourceRow = FName(TEXT("event_test_harvest"));
	const TArray<FName> Peoples = { People };
	TestTrue(TEXT("(a Severity 1 line of the life's people is taken)"), CosmicChronicle::Admits(Entry, Peoples));
	for (const FName Row : { CosmicRows::LogEdictEnacted, CosmicRows::LogEdictRevoked, CosmicRows::LogEdictLapsed })
	{
		Entry.SourceRow = Row;
		TestFalse(*FString::Printf(TEXT("%s is left out"), *Row.ToString()), CosmicChronicle::Admits(Entry, Peoples));
	}
	return true;
}

// The ruler policy's edicts (header section 5, CosmicRulerPolicy::ActEdicts): the kept row stands without turning, a
// Stability edict goes below 0.5 once and comes back only at the resting level, an answer is enacted for a standing hazard
// (making room) and goes when nothing is left to answer, the vigil comes before a rebellion only and goes when the danger
// passed; nothing after the provisional end.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimRulerEdictPolicyTest, "Cosmic_Ascension.Sim.RulerEdictPolicy", CosmicTests::Flags)
bool FCosmicSimRulerEdictPolicyTest::RunTest(const FString& Parameters)
{
	using namespace CosmicEdictTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	// The policy's rows: the file's answer (crisis_test_wake), its vigil (edict_test_guard - rebellion +200) and two kept rows.
	auto Policy = [](FName Row) { return FCosmicRulerPolicy{ false, false, false, true, Row, 0, ECosmicPressurePolicy::Round, 0, false }; };
	auto Boundary = [&](const FCosmicRulerPolicy& With, FCosmicRulerPolicyCounts& Counts, int32 Times)
	{
		for (int32 Done = 0; Done < Times; ++Done)
		{
			CosmicRulerPolicy::ActEdicts(With, *Sim, *Role, Counts);
		}
	};
	auto Changes = [](const FCosmicRulerPolicyCounts& Counts) { return Counts.EdictsEnacted + Counts.EdictsRevoked; };

	// The kept row: enacted once in a free slot, then left standing - with no hazard to answer it is not revoked as idle.
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		if (!TestNotNull(TEXT("The board is laid"), Board))
		{
			return false;
		}
		Board->Civs[0].Stockpile.Influence = 1000.f;
		FCosmicRulerPolicyCounts Counts;
		for (int32 Tick = 0; Tick < 300; ++Tick)
		{
			Sim->RunHeadless(1);
			CosmicRulerPolicy::ActEdicts(Policy(Answer), *Sim, *Role, Counts);
		}
		TestEqual(TEXT("Over 300 ticks with no crisis the kept answer is enacted once"), Changes(Counts), 1);
		TestTrue(TEXT("and stands"), Board->Civs[0].ActiveEdicts == TArray<FName>({ Answer }));
	}

	// A Stability edict below 0.5: one revocation, and no enactment again until the resting level.
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		Board->Civs[0].Stockpile.Influence = 1000.f;
		Board->Civs[0].ActiveEdicts = { Taxing };
		Board->Civs[0].Stability = 0.45f;
		FCosmicRulerPolicyCounts Counts;
		Boundary(Policy(Taxing), Counts, 3);
		TestEqual(TEXT("Below 0.5 the kept Stability edict is revoked once"), Counts.EdictsRevoked, 1);
		TestEqual(TEXT("and not enacted again below the resting level"), Counts.EdictsEnacted, 0);
		Board->Civs[0].Stability = 0.55f;
		Boundary(Policy(Taxing), Counts, 1);
		TestEqual(TEXT("nor at 0.55"), Counts.EdictsEnacted, 0);
		Board->Civs[0].Stability = 0.6f;
		Boundary(Policy(Taxing), Counts, 1);
		TestTrue(TEXT("At 0.6 it comes back"), Counts.EdictsEnacted == 1 && Board->Civs[0].ActiveEdicts == TArray<FName>({ Taxing }));
	}

	// An answer for a standing hazard: room made by revoking a held edict that answers nothing; revoked when the hazard goes.
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		Board->Civs[0].Stockpile.Influence = 1000.f;
		Board->Civs[0].Stability = 0.8f;
		Board->Civs[0].ActiveEdicts = { Plain };
		const int32 Capital = CapitalOf(*Sim, 0);
		Board->Systems[Capital].ActiveHazard = Tag(TEXT("Hazard.AIAwakening"));
		Board->Systems[Capital].HazardTicksRemaining = 3;
		FCosmicRulerPolicyCounts Counts;
		Boundary(Policy(NAME_None), Counts, 1);
		TestTrue(TEXT("A standing hazard's answer is enacted, the edict that answers nothing revoked for its slot"),
			Board->Civs[0].ActiveEdicts == TArray<FName>({ Answer }) && Counts.EdictsRevoked == 1 && Counts.EdictsEnacted == 1);
		Boundary(Policy(NAME_None), Counts, 1);
		TestEqual(TEXT("While it stands nothing turns"), Changes(Counts), 2);
		Board->Systems[Capital].ActiveHazard = FGameplayTag();
		Boundary(Policy(NAME_None), Counts, 1);
		TestTrue(TEXT("With nothing left to answer the answer goes"), Board->Civs[0].ActiveEdicts.IsEmpty() && Counts.EdictsRevoked == 2);
		// Kept, it stays.
		Board->Civs[0].ActiveEdicts = { Answer };
		FCosmicRulerPolicyCounts Kept;
		Boundary(Policy(Answer), Kept, 2);
		TestTrue(TEXT("The kept row is never revoked as an idle answer"), Changes(Kept) == 0 && Board->Civs[0].ActiveEdicts == TArray<FName>({ Answer }));
	}

	// Room is made with the kept row last: given up when it alone fills the slot (and back once the hazard is gone), kept when
	// another edict can give way; and nothing is given up for an answer that would be refused (its rung, its cost).
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		Board->Civs[0].Stockpile.Influence = 1000.f;
		Board->Civs[0].Stability = 0.8f;
		Board->Civs[0].ActiveEdicts = { Taxing };
		const int32 Capital = CapitalOf(*Sim, 0);
		Board->Systems[Capital].ActiveHazard = Tag(TEXT("Hazard.AIAwakening"));
		Board->Systems[Capital].HazardTicksRemaining = 3;
		FCosmicRulerPolicyCounts Sole;
		Boundary(Policy(Taxing), Sole, 1);
		TestTrue(TEXT("The kept row alone in the slot gives way to the hazard's answer"), Board->Civs[0].ActiveEdicts == TArray<FName>({ Answer }) && Sole.EdictsRevoked == 1);
		Board->Systems[Capital].ActiveHazard = FGameplayTag();
		Boundary(Policy(Taxing), Sole, 1);
		TestTrue(TEXT("and comes back once nothing is left to answer"), Board->Civs[0].ActiveEdicts == TArray<FName>({ Taxing }));

		Board->Civs[0].Research.Add(FName(TEXT("Society")), 0.5f);
		Board->Civs[0].ActiveEdicts = { Lifting, Plain };
		Board->Systems[Capital].ActiveHazard = Tag(TEXT("Hazard.AIAwakening"));
		Board->Systems[Capital].HazardTicksRemaining = 3;
		FCosmicRulerPolicyCounts Two;
		Boundary(Policy(Lifting), Two, 1);
		TestTrue(TEXT("With two slots the other edict gives way and the kept row stays"), Board->Civs[0].ActiveEdicts == TArray<FName>({ Lifting, Answer })
			&& Two.EdictsRevoked == 1 && Two.EdictsEnacted == 1);

		// An answer of a higher rung, then one no stockpile pays: the kept row stays, nothing turns.
		FHazardDef* Woken = Sim->GetTables().Hazards->FindRow<FHazardDef>(Wake, TEXT("test"));
		if (!TestNotNull(TEXT("(the test's awakening row)"), Woken))
		{
			return false;
		}
		Board->Civs[0].Research.Add(FName(TEXT("Society")), 0.f);
		for (const FName Refused : { High, Dear })
		{
			Woken->RulerResolutions = { Refused };
			Board->Civs[0].ActiveEdicts = { Taxing };
			FCosmicRulerPolicyCounts Gated;
			Boundary(Policy(Taxing), Gated, 4);
			TestTrue(*FString::Printf(TEXT("No edict is given up for an answer that would be refused (%s)"), *Refused.ToString()),
				Changes(Gated) == 0 && Board->Civs[0].ActiveEdicts == TArray<FName>({ Taxing }));
		}
		Woken->RulerResolutions = { Answer };
	}

	// The vigil: before a rebellion only, and gone once Stability rests again with no rebellion standing.
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		Board->Civs[0].Stockpile.Influence = 1000.f;
		Board->Civs[0].Stability = 0.25f;
		Board->Civs[0].LowStabilityTicks = 5;
		const int32 Capital = CapitalOf(*Sim, 0);
		Board->Systems[Capital].ActiveHazard = Tag(TEXT("Hazard.Rebellion"));
		Board->Systems[Capital].HazardTicksRemaining = 100;
		FCosmicRulerPolicyCounts Counts;
		Boundary(Policy(NAME_None), Counts, 1);
		TestFalse(TEXT("With a rebellion standing the vigil is not enacted (its bonus would not reach it)"), Board->Civs[0].ActiveEdicts.Contains(Guard));
		Board->Systems[Capital].ActiveHazard = FGameplayTag();
		Boundary(Policy(NAME_None), Counts, 1);
		TestTrue(TEXT("Before one, it is"), Board->Civs[0].ActiveEdicts.Contains(Guard) && Counts.VigilEnacted == Guard);
		Board->Civs[0].Stability = 0.5f;
		Boundary(Policy(NAME_None), Counts, 1);
		TestTrue(TEXT("It stays below the resting level"), Board->Civs[0].ActiveEdicts.Contains(Guard));
		Board->Civs[0].Stability = 0.6f;
		Board->Civs[0].LowStabilityTicks = 0;
		Boundary(Policy(NAME_None), Counts, 1);
		TestTrue(TEXT("and goes at it"), !Board->Civs[0].ActiveEdicts.Contains(Guard) && Counts.VigilEnacted.IsNone());
	}

	// B6b (header section 2.4): the law step (4') under a row of the fourth rung that reads the held laws, three to hold
	// (GalaxyLawsToHold) and four laws in the table. Every case: the ruled people on the fourth rung, three slots (Society 1).
	auto LawBoard = [&](std::initializer_list<FName> Held, float Society = 1.f)
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, [](FCosmicRulesDef& Rules) { Rules.GalaxyLawsToHold = 3; });
		AskForLaws(*Sim);
		Board->Civs[0].Tier = EAscensionTier::Tier4_Transcendent;
		Board->Civs[0].Research.Add(FName(TEXT("Society")), Society);
		Board->Civs[0].Stockpile.Influence = 1000.f;
		Board->Civs[0].Stability = 0.8f;
		Board->Civs[0].ActiveEdicts = Held;
		return Board;
	};
	{
		// The laws it lacks, in table order: a and b into the free slots, c into the slot of the edict that is no law (mutation
		// b6b2a_policy_law_step_free_slot_only: the law step takes free slots only - c finds no room); d finds no room - a law is
		// never given up for a law, so the next boundaries change nothing (mutation b6b2a_policy_law_for_law: the law step gives
		// up a law for a law - the laws turn over).
		FGalaxyState* Board = LawBoard({ Plain });
		FCosmicRulerPolicyCounts Counts;
		Boundary(Policy(NAME_None), Counts, 1);
		TestTrue(TEXT("The law step enacts the laws in table order, making room by giving up the edict that is no law"),
			Board->Civs[0].ActiveEdicts == TArray<FName>({ LawA, LawB, LawC }) && Counts.EdictsEnacted == 3 && Counts.EdictsRevoked == 1);
		Boundary(Policy(NAME_None), Counts, 2);
		TestTrue(TEXT("and the fourth law gives up none of the three over two more boundaries"),
			Changes(Counts) == 4 && Board->Civs[0].ActiveEdicts == TArray<FName>({ LawA, LawB, LawC }));
	}
	{
		// (1) gives up the edicts that cost Stability below 0.5 - never a law, though law a costs Stability (mutation: it does).
		FGalaxyState* Board = LawBoard({ Taxing, LawA });
		Board->Civs[0].Stability = 0.45f;
		FCosmicRulerPolicyCounts Counts;
		Boundary(Policy(NAME_None), Counts, 1);
		TestFalse(TEXT("Below 0.5 the Stability edict goes"), Board->Civs[0].ActiveEdicts.Contains(Taxing));
		TestTrue(TEXT("and the law with a Stability cost stays"), Board->Civs[0].ActiveEdicts.Contains(LawA));
	}
	{
		// (2) makes room for a hazard's answer with the laws last: the edict that is no law goes (mutation: the first held edict
		// goes, a law).
		FGalaxyState* Board = LawBoard({ LawA, LawB, Plain });
		const int32 Capital = CapitalOf(*Sim, 0);
		Board->Systems[Capital].ActiveHazard = Tag(TEXT("Hazard.AIAwakening"));
		Board->Systems[Capital].HazardTicksRemaining = 3;
		FCosmicRulerPolicyCounts Counts;
		Boundary(Policy(NAME_None), Counts, 1);
		TestTrue(TEXT("An answer takes the slot of the edict that is no law, the laws last"),
			Board->Civs[0].ActiveEdicts == TArray<FName>({ LawA, LawB, Answer }) && Counts.EdictsRevoked == 1);
	}
	{
		// The same with that edict the kept row: EdictRow is given up before a law, the laws last of all, and (5) does not
		// bring it back - no slot is free and it was revoked at this boundary (mutation: a law goes before EdictRow).
		FGalaxyState* Board = LawBoard({ LawA, LawB, Plain });
		const int32 Capital = CapitalOf(*Sim, 0);
		Board->Systems[Capital].ActiveHazard = Tag(TEXT("Hazard.AIAwakening"));
		Board->Systems[Capital].HazardTicksRemaining = 3;
		FCosmicRulerPolicyCounts Counts;
		Boundary(Policy(Plain), Counts, 1);
		TestTrue(TEXT("An answer takes the slot of the kept row before a law's"),
			Board->Civs[0].ActiveEdicts == TArray<FName>({ LawA, LawB, Answer }) && Counts.EdictsRevoked == 1);
	}
	{
		// One law and the awakening's answer in three slots: while the answer holds a slot the law step stands still, a free slot
		// though there is (mutation: the law step does not stand still for an answer - law a takes the free slot).
		FGalaxyState* Board = LawBoard({ LawB, Answer });
		const int32 Capital = CapitalOf(*Sim, 0);
		Board->Systems[Capital].ActiveHazard = Tag(TEXT("Hazard.AIAwakening"));
		Board->Systems[Capital].HazardTicksRemaining = 3;
		FCosmicRulerPolicyCounts Counts;
		Boundary(Policy(NAME_None), Counts, 1);
		TestTrue(TEXT("While an answer holds a slot the law step leaves the free slot alone"),
			Changes(Counts) == 0 && Board->Civs[0].ActiveEdicts == TArray<FName>({ LawB, Answer }));
	}
	{
		// The vigil of (3) before a rebellion, three laws in the three slots: (3) gives up the first law for it, and the law step
		// does not take its slot back while (3) would enact it again - over two more boundaries nothing changes (mutation
		// b6b2a_policy_vigil_for_law: the law step takes the vigil's slot - the vigil and a law swap every two boundaries). Nor
		// once the discontent is no longer counted (Stability 0.3 or more) while Stability is below 0.5: the band (3) and (4)
		// keep (mutation b6b2a_policy_vigil_band: the law takes the slot back at 0.45 - under a steady drain the vigil and a law
		// swap each time Stability climbs back to 0.3). At 0.5 or more (0.55) the law takes the vigil's slot back while (4)
		// still keeps the vigil (mutation b6b2a_policy_vigil_waits_for_rest: the law waits for (4) at RestAt - the stall the
		// verdict measured).
		FGalaxyState* Board = LawBoard({ LawA, LawB, LawC });
		Board->Civs[0].Stability = 0.25f;
		Board->Civs[0].LowStabilityTicks = 5;
		FCosmicRulerPolicyCounts Counts;
		Boundary(Policy(NAME_None), Counts, 1);
		TestTrue(TEXT("The vigil takes the first law's slot"),
			Board->Civs[0].ActiveEdicts == TArray<FName>({ LawB, LawC, Guard }) && Changes(Counts) == 2 && Counts.VigilEnacted == Guard);
		Boundary(Policy(NAME_None), Counts, 2);
		TestTrue(TEXT("While the discontent is counted no law takes the vigil's slot over two boundaries"),
			Changes(Counts) == 2 && Board->Civs[0].ActiveEdicts == TArray<FName>({ LawB, LawC, Guard }));
		Board->Civs[0].Stability = 0.45f;
		Board->Civs[0].LowStabilityTicks = 0;
		Boundary(Policy(NAME_None), Counts, 1);
		TestTrue(TEXT("With the discontent no longer counted but Stability below 0.5 no law takes the vigil's slot"),
			Changes(Counts) == 2 && Board->Civs[0].ActiveEdicts == TArray<FName>({ LawB, LawC, Guard }) && Counts.VigilEnacted == Guard);
		Board->Civs[0].Stability = 0.55f;
		Boundary(Policy(NAME_None), Counts, 1);
		TestTrue(TEXT("At Stability 0.5 or more the first law takes the vigil's slot back, below the resting level where (4) keeps it"),
			Changes(Counts) == 4 && Board->Civs[0].ActiveEdicts == TArray<FName>({ LawB, LawC, LawA }) && Counts.VigilEnacted.IsNone());
	}
	{
		// The same vigil, the danger passed at once (Stability 0.6, the discontent no longer counted): (4) lets the vigil go and
		// the law comes back into the slot it leaves at the same boundary.
		FGalaxyState* Board = LawBoard({ LawA, LawB, LawC });
		Board->Civs[0].Stability = 0.25f;
		Board->Civs[0].LowStabilityTicks = 5;
		FCosmicRulerPolicyCounts Counts;
		Boundary(Policy(NAME_None), Counts, 1);
		Board->Civs[0].Stability = 0.6f;
		Board->Civs[0].LowStabilityTicks = 0;
		Boundary(Policy(NAME_None), Counts, 1);
		TestTrue(TEXT("Once the danger passed the vigil goes and the first law comes back"),
			Changes(Counts) == 4 && Board->Civs[0].ActiveEdicts == TArray<FName>({ LawB, LawC, LawA }) && Counts.VigilEnacted.IsNone());
	}
	{
		// Three laws held, the awakening's answer the only one: (2) gives up the first law for it. While the answer holds a slot
		// the law step stands still - over two more boundaries no law is given up for the one that went (it holds while either
		// guard does - no law given up for a law, no step while an answer holds a slot: the mutation of the first alone is
		// killed by the first law case, the second's by the case of the free slot above); when nothing is left to answer, the
		// answer goes and the law comes back into its slot.
		FGalaxyState* Board = LawBoard({ LawA, LawB, LawC });
		const int32 Capital = CapitalOf(*Sim, 0);
		Board->Systems[Capital].ActiveHazard = Tag(TEXT("Hazard.AIAwakening"));
		Board->Systems[Capital].HazardTicksRemaining = 3;
		FCosmicRulerPolicyCounts Counts;
		Boundary(Policy(NAME_None), Counts, 1);
		TestTrue(TEXT("With three laws held the answer takes the first law's slot"),
			Board->Civs[0].ActiveEdicts == TArray<FName>({ LawB, LawC, Answer }) && Changes(Counts) == 2);
		Boundary(Policy(NAME_None), Counts, 2);
		TestTrue(TEXT("While the answer holds a slot no law is given up for another over two boundaries"),
			Changes(Counts) == 2 && Board->Civs[0].ActiveEdicts == TArray<FName>({ LawB, LawC, Answer }));
		Board->Systems[Capital].ActiveHazard = FGameplayTag();
		Boundary(Policy(NAME_None), Counts, 1);
		TestTrue(TEXT("With nothing left to answer the answer goes and the first law comes back"),
			Board->Civs[0].ActiveEdicts == TArray<FName>({ LawB, LawC, LawA }));
	}
	{
		// The law step stands still while the slots are fewer than the laws to hold (mutation b6b2a_policy_law_step_ignores_slots:
		// it enacts into the two it has), below the fourth rung, and without a row that reads the laws (mutation
		// b6b2a_policy_law_step_unasked: the step runs unasked). Below the fourth rung the case gives the third rung a row that
		// reads the laws (third to fourth) and laws of the third rung the people may enact, so only the rung check of
		// CosmicEdicts::AsksForLawsOf keeps the step still (mutation b6b2a_asks_for_laws_any_rung: without it the step enacts).
		FGalaxyState* Board = LawBoard({}, 0.5f);
		FCosmicRulerPolicyCounts Two;
		Boundary(Policy(NAME_None), Two, 1);
		TestTrue(TEXT("With two slots for three laws the law step enacts none"), Changes(Two) == 0 && Board->Civs[0].ActiveEdicts.IsEmpty());
		Board = LawBoard({});
		Board->Civs[0].Tier = EAscensionTier::Tier3_Causal;
		AskForLaws(*Sim, EAscensionTier::Tier3_Causal);
		for (const FName Law : { LawA, LawB, LawC, LawD })
		{
			FEdictDef* Row = Sim->GetTables().Edicts->FindRow<FEdictDef>(Law, TEXT("test"));
			if (!TestNotNull(TEXT("(the test's law row)"), Row))
			{
				return false;
			}
			Row->RequiredTier = EAscensionTier::Tier3_Causal;
		}
		TestTrue(TEXT("(a law of the third rung is open to the people of the third rung)"),
			CosmicEdicts::EnactBlockOf(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0, LawB) == ECosmicEdictBlock::None);
		FCosmicRulerPolicyCounts Below;
		Boundary(Policy(NAME_None), Below, 1);
		TestTrue(TEXT("Below the fourth rung no row asks for laws"), Changes(Below) == 0 && Board->Civs[0].ActiveEdicts.IsEmpty());
		Board = LayBoard(*Sim, Original, NoEdit);
		Board->Civs[0].Tier = EAscensionTier::Tier4_Transcendent;
		Board->Civs[0].Research.Add(FName(TEXT("Society")), 1.f);
		Board->Civs[0].Stockpile.Influence = 1000.f;
		FCosmicRulerPolicyCounts Unasked;
		Boundary(Policy(NAME_None), Unasked, 1);
		TestTrue(TEXT("Without a row that reads the laws no law is enacted"), Changes(Unasked) == 0);
	}

	// Nothing after the provisional end, nothing without -Edict.
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, NoEdit);
		Board->Civs[0].Stockpile.Influence = 1000.f;
		FCosmicRulerPolicyCounts Ended;
		Ended.bEnded = true;
		Boundary(Policy(Answer), Ended, 1);
		FCosmicRulerPolicyCounts Off;
		Boundary(FCosmicRulerPolicy{ true, true, true, false, Answer, 0, ECosmicPressurePolicy::Round, 0, false }, Off, 1);
		TestTrue(TEXT("No edict after the end, nor without the switch"), Changes(Ended) == 0 && Changes(Off) == 0 && Board->Civs[0].ActiveEdicts.IsEmpty());
		Ended.Answers = 2;
		Ended.Started = 3;
		Ended.FocusChanges = 4;
		Ended.EdictsEnacted = 5;
		Ended.EdictsRevoked = 6;
		TestEqual(TEXT("The decisions add the edict commands"), CosmicRulerPolicy::DecisionsOf(Ended), 20);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

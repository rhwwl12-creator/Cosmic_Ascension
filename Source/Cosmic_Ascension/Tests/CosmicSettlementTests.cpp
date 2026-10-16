// Cosmic Ascension - automation tests of settlement, the realm cap, the removal of a civilization without systems and the
// regrowth of the population. Design: guideline.md section 4.1, 4.2, 5.7, 5.12, 7.4, 9.2, 9.3.

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
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/NameTypes.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#include <initializer_list>
#include <limits>

namespace CosmicSettlementTests
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

	TArray<FText> Lines(const FString& Stem)
	{
		return {
			FText::AsCultureInvariant(TEXT("{Civ} {System} {Project} ") + Stem + TEXT(" one")),
			FText::AsCultureInvariant(TEXT("{Civ} {System} {Project} ") + Stem + TEXT(" two")),
			FText::AsCultureInvariant(TEXT("{Civ} {System} {Project} ") + Stem + TEXT(" three"))
		};
	}

	const FName Answer(TEXT("project_test_answer"));
	const FName Storm(TEXT("crisis_test_storm"));
	const FName Always(TEXT("crisis_test_always"));
	const FName Nova(TEXT("crisis_test_nova"));
	const FName Sphere(TEXT("trans_test_sphere"));

	FMegaprojectDef Project(EAscensionTier Tier, float Energy, float Matter, float Required, const TCHAR* Stem)
	{
		FMegaprojectDef Row;
		Row.DisplayName = FText::AsCultureInvariant(FString::Printf(TEXT("<%s>"), Stem));
		Row.RequiredTier = Tier;
		Row.CostPerTick.Energy = Energy;
		Row.CostPerTick.Matter = Matter;
		Row.RequiredProgress = Required;
		Row.CompleteTemplates = Lines(FString(Stem) + TEXT(" done"));
		return Row;
	}

	// The projects of the tests: the settle row (Tier 2, Energy 1, 3 - three paid ticks) unless SettleTier is null, and
	// the answer of Storm (Tier 1, Matter 5, 2). With bCausality both cost one Causality a tick instead, a resource no
	// people earns: the stockpile alone decides which slot is paid.
	UDataTable* ProjectRows(const EAscensionTier* SettleTier, bool bCausality = false)
	{
		UDataTable* Table = NewTable(FMegaprojectDef::StaticStruct());
		FMegaprojectDef Settle = Project(SettleTier ? *SettleTier : EAscensionTier::Tier1_Planetary, 1.f, 0.f, 3.f, TEXT("settle"));
		FMegaprojectDef Answering = Project(EAscensionTier::Tier1_Planetary, 0.f, 5.f, 2.f, TEXT("answer"));
		if (bCausality)
		{
			Settle.CostPerTick = FResourcePool();
			Settle.CostPerTick.Causality = 1.f;
			Answering.CostPerTick = FResourcePool();
			Answering.CostPerTick.Causality = 1.f;
		}
		if (SettleTier)
		{
			Table->AddRow(CosmicRows::ProjectSettle, Settle);
		}
		Table->AddRow(Answer, Answering);
		return Table;
	}

	FHazardDef HazardRow(const TCHAR* TagName, float Chance, int32 Countdown, TArray<FName> Answers)
	{
		FHazardDef Row;
		Row.HazardTag = Tag(TagName);
		Row.DisplayName = FText::AsCultureInvariant(FString::Printf(TEXT("<%s>"), TagName));
		Row.CountdownTicks = Countdown;
		Row.SpawnRule = ECosmicHazardSpawn::Draw;
		Row.SpawnChancePerTick = Chance;
		Row.LogCategory = Tag(TEXT("Log.Crisis"));
		Row.Severity = 1;
		Row.RulerResolutions = MoveTemp(Answers);
		Row.OutbreakTemplates = { FText::AsCultureInvariant(FString(TagName) + TEXT(" outbreak")) };
		Row.ExpireTemplates = { FText::AsCultureInvariant(FString(TagName) + TEXT(" expiry")) };
		Row.ResolveTemplates = { FText::AsCultureInvariant(FString(TagName) + TEXT(" resolution")) };
		return Row;
	}

	// Storm: a plague that never spawns by itself, answered by Answer. Always: an awakening that spawns on every free slot
	// of a settled system and lasts long. Nova: a supernova that never spawns by itself.
	UDataTable* HazardRows(bool bWithAlways)
	{
		UDataTable* Table = NewTable(FHazardDef::StaticStruct());
		Table->AddRow(Storm, HazardRow(TEXT("Hazard.Plague"), 0.f, 1000, { Answer }));
		Table->AddRow(Nova, HazardRow(TEXT("Hazard.PreSupernova"), 0.f, 1, {}));
		if (bWithAlways)
		{
			Table->AddRow(Always, HazardRow(TEXT("Hazard.AIAwakening"), 1.f, 1000, {}));
		}
		return Table;
	}

	// One fallen row of Tier 4 that the roster never picks (weight 0).
	UDataTable* TranscendentRows()
	{
		UDataTable* Table = NewTable(FTranscendentDef::StaticStruct());
		FTranscendentDef Row;
		Row.Doctrine = Tag(TEXT("Doctrine.Silence"));
		Row.Allegiance = Tag(TEXT("Realm.Abyssal"));
		Row.Tier = EAscensionTier::Tier4_Transcendent;
		Row.Depth = 1.f;
		Row.MaxInfluencedSystems = 12;
		Row.SpawnWeight = 0.f;
		Table->AddRow(Sphere, Row);
		return Table;
	}

	// The knobs of the tests, not the game's: a slot every ten ticks, no Lottery chance, no pressure drain, no regrowth,
	// AI projects at the player's pace, and the realm cap 1/3/4/7/10.
	FCosmicRulesDef Knobs(const FCosmicRulesDef& BaseRules)
	{
		FCosmicRulesDef Rules = BaseRules;
		Rules.EventSlotTicks = 10;
		Rules.EventChancePerSlot = 0.f;
		Rules.SpiritualPressureStabilityDrain = 0.f;
		Rules.PopulationRegrowthPerTick = 0.f;
		Rules.AIAscensionSpeedMultiplier = 1.f;
		Rules.RealmCapByTier = { 1, 3, 4, 7, 10 };
		return Rules;
	}

	// The game's tables with the test's rules, projects, hazards (Storm and Nova when null), feats and events (none when
	// null) and transcendent row.
	FCosmicDataTables TablesWith(const FCosmicDataTables& Original, const FCosmicRulesDef& Rules, UDataTable* Projects, UDataTable* Hazards, UDataTable* Feats = nullptr, UDataTable* Events = nullptr)
	{
		UDataTable* RulesTable = NewTable(FCosmicRulesDef::StaticStruct());
		RulesTable->AddRow(CosmicRows::Rules, Rules);
		FCosmicDataTables Tables = Original;
		Tables.Rules = RulesTable;
		Tables.Hazards = Hazards ? Hazards : HazardRows(false);
		Tables.Events = Events ? Events : NewTable(FCosmicEventDef::StaticStruct());
		Tables.Transcendents = TranscendentRows();
		Tables.Megaprojects = Projects;
		Tables.Feats = Feats ? Feats : NewTable(FAscensionFeatDef::StaticStruct());
		return Tables;
	}

	// The board: the chain of CosmicTests::WireChain (0-1-...-9, 4-10, 10-11). The vessel's people 0 holds 0, people 1
	// holds 5 (Tier 2: its targets are 4 and 6), people 2 holds 8 and people 3 holds 11 (both Tier 1, at their cap). No
	// hazard, project, record, transcendent, flag or research; every stockpile 1000 of each but Causality; every
	// Stability 0.75 and Population 1. False below twelve systems or four peoples.
	bool Board(FGalaxyState& State)
	{
		if (!CosmicTests::WireChain(State) || State.Civs.Num() < 4)
		{
			return false;
		}
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { 0 });
		CosmicTests::Give(State, 1, { 5 });
		CosmicTests::Give(State, 2, { 8 });
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
			Civ.Tier = EAscensionTier::Tier1_Planetary;
			Civ.Stockpile.Energy = 1000.f;
			Civ.Stockpile.Matter = 1000.f;
			Civ.Stockpile.Insight = 1000.f;
			Civ.Stockpile.Influence = 1000.f;
			Civ.Stability = 0.75f;
			Civ.Population = 1.f;
		}
		State.Civs[1].Tier = EAscensionTier::Tier2_Stellar;
		return true;
	}

	FGalaxyState* LayBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Tables)
	{
		Sim.SetTables(Tables);
		Sim.NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim.GetMutableState();
		return Board(State) ? &State : nullptr;
	}

	// Hands SystemIndex to the people CivIndex by hand (a test's board, not a world function).
	void Hand(FGalaxyState& State, int32 CivIndex, int32 SystemIndex)
	{
		State.Systems[SystemIndex].OwnerCivID = State.Civs[CivIndex].CivID;
		State.Civs[CivIndex].OwnedSystems.Add(State.Systems[SystemIndex].SystemID);
	}

	// Hands SystemIndex to the people CivIndex as a settlement under way (a test's board).
	void HandSettling(FGalaxyState& State, int32 CivIndex, int32 SystemIndex)
	{
		Hand(State, CivIndex, SystemIndex);
		State.Systems[SystemIndex].ActiveMegaproject = CosmicRows::ProjectSettle;
	}

	void AddInstance(FGalaxyState& State, FName TranscendentID, std::initializer_list<int32> Members)
	{
		FTranscendentState& Transcendent = State.Transcendents.AddDefaulted_GetRef();
		Transcendent.TranscendentID = TranscendentID;
		Transcendent.Doctrine = Tag(TEXT("Doctrine.Silence"));
		for (const int32 SystemIndex : Members)
		{
			Transcendent.InfluencedSystems.Add(State.Systems[SystemIndex].SystemID);
		}
	}

	int32 SettlingCount(const FGalaxyState& State, const FCivilizationState& Civ)
	{
		int32 Count = 0;
		for (const FStarSystemState& System : State.Systems)
		{
			Count += (System.OwnerCivID == Civ.CivID && System.ActiveMegaproject == CosmicRows::ProjectSettle) ? 1 : 0;
		}
		return Count;
	}

	// The system the people with CivID is settling, or INDEX_NONE.
	int32 SettlingTarget(const FGalaxyState& State, FName CivID)
	{
		return State.Systems.IndexOfByPredicate([CivID](const FStarSystemState& System)
		{
			return System.OwnerCivID == CivID && System.ActiveMegaproject == CosmicRows::ProjectSettle;
		});
	}

	int32 OwnedCount(const FGalaxyState& State, const FCivilizationState& Civ)
	{
		int32 Count = 0;
		for (const FStarSystemState& System : State.Systems)
		{
			Count += System.OwnerCivID == Civ.CivID ? 1 : 0;
		}
		return Count;
	}

	// The index PickIndex gives a number among Count targets.
	int32 PickOf(float Roll, int32 Count)
	{
		return FMath::Min(static_cast<int32>(Roll * static_cast<float>(Count)), Count - 1);
	}

	/**
	 * The draws of the AI stage in one tick from From under Tables: the stream at the end of that tick against the stream
	 * at the end of the same tick with every people marked as ruled by the player, so that the AI stage looks at none (its
	 * starts and the feat judgment of the AI peoples then draw nothing, and on these boards nothing after the AI stage
	 * draws). Returns that count, or -1 when no count up to 16 matches. OutAfter is the state after the tick; OutBefore,
	 * when given, the stream the AI stage starts from.
	 */
	int32 AIStageDraws(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Tables, const FGalaxyState& From, FGalaxyState& OutAfter, int32* OutBefore = nullptr)
	{
		Sim.SetTables(Tables);
		FGalaxyState NoAI = From;
		for (FCivilizationState& Civ : NoAI.Civs)
		{
			Civ.bIsPlayerRuled = true;
		}
		Sim.LoadState(NoAI);
		Sim.StepOnce();
		const int32 Without = Sim.GetState().RngState;
		if (OutBefore)
		{
			*OutBefore = Without;
		}
		Sim.LoadState(From);
		Sim.StepOnce();
		OutAfter = Sim.GetState();
		FRandomStream Walk(Without);
		for (int32 Count = 0; Count <= 16; ++Count)
		{
			if (Walk.GetCurrentSeed() == OutAfter.RngState)
			{
				return Count;
			}
			Walk.GetFraction();
		}
		return -1;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSettlementTest, "Cosmic_Ascension.Sim.Settlement", CosmicTests::Flags)
bool FCosmicSettlementTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSettlementTests;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* PlayerRole = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), PlayerRole))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FCosmicRulesDef Rules = Knobs(Sim->GetRules());
	const EAscensionTier Stellar = EAscensionTier::Tier2_Stellar;
	const EAscensionTier Causal = EAscensionTier::Tier3_Causal;
	const FCosmicDataTables Tables = TablesWith(Original, Rules, ProjectRows(&Stellar), nullptr);

	// ---- Who may settle and where: the world functions on copies of one board ----
	FGalaxyState* Laid = LayBoard(*Sim, Tables);
	if (!TestNotNull(TEXT("The board is laid"), Laid))
	{
		return false;
	}
	const FGalaxyState Base = *Laid;
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	auto Targets = [&](const FGalaxyState& State, const FCosmicDataTables& With, int32 CivIndex)
	{
		return CosmicMegaprojects::SettleTargets(State, Index, With, Rules, CivIndex);
	};
	auto Can = [&](const FGalaxyState& State, int32 CivIndex, int32 SystemIndex)
	{
		return CosmicMegaprojects::CanSettle(State, Index, Tables, Rules, CivIndex, SystemIndex);
	};

	TestTrue(TEXT("The targets are the wild systems one lane from the people's settled systems, in Systems order"), Targets(Base, Tables, 1) == TArray<int32>({ 4, 6 }));
	TestFalse(TEXT("A wild system two lanes away is no target"), Can(Base, 1, 7));
	{
		FGalaxyState Case = Base;
		Hand(Case, 2, 6);
		TestFalse(TEXT("A system another people owns is no target"), Can(Case, 1, 6));
	}
	{
		// People 1 also holds 6, next to its 5: its own system is no target, though a settled system of it lies beside.
		FGalaxyState Case = Base;
		Hand(Case, 1, 6);
		TestFalse(TEXT("A system the people itself owns is no target"), Can(Case, 1, 6));
		TestFalse(TEXT("nor among its targets"), Targets(Case, Tables, 1).Contains(6));
	}
	{
		FGalaxyState Case = Base;
		Case.Systems[6].StarClass = FGameplayTag();
		TestFalse(TEXT("A dead node is no target, even with its lanes"), Can(Case, 1, 6));
	}
	{
		FGalaxyState Case = Base;
		Case.Systems[6].GalaxyRow = FName(TEXT("gal_andromeda"));
		TestFalse(TEXT("A system of another galaxy row is no target"), Can(Case, 1, 6));
	}
	{
		// System 3 is people 2's, and also listed among people 1's: it is no foothold of people 1.
		FGalaxyState Case = Base;
		Hand(Case, 2, 3);
		Case.Civs[1].OwnedSystems.Add(Case.Systems[3].SystemID);
		TestFalse(TEXT("A listed system another people owns is no foothold"), Can(Case, 1, 2));
		FGalaxyState Own = Base;
		Hand(Own, 1, 3);
		TestTrue(TEXT("A system the people owns is a foothold"), Can(Own, 1, 2));
		// Its footholds are 5 then 3 in its own order; their targets come in Systems order all the same.
		TestTrue(TEXT("The targets of two footholds come in Systems order"), Targets(Own, Tables, 1) == TArray<int32>({ 2, 4, 6 }));
	}
	{
		FGalaxyState Case = Base;
		AddInstance(Case, Sphere, { 6 });
		TestFalse(TEXT("Below Tier 3 a people does not settle inside the sphere of an unsealed transcendent"), Can(Case, 1, 6));
		TestTrue(TEXT("and still settles outside it"), Can(Case, 1, 4));
		Case.Civs[1].Tier = Causal;
		TestTrue(TEXT("On Tier 3 a people settles inside the sphere"), Can(Case, 1, 6));
		FGalaxyState Sealed = Base;
		AddInstance(Sealed, Sphere, { 6 });
		Sealed.Transcendents[0].bSealed = true;
		TestTrue(TEXT("A sealed sphere holds no settlement back"), Can(Sealed, 1, 6));
		FGalaxyState Rowless = Base;
		AddInstance(Rowless, FName(TEXT("trans_test_missing")), { 6 });
		TestTrue(TEXT("The sphere of an instance without a row holds no settlement back"), Can(Rowless, 1, 6));
		FGalaxyState Border = Base;
		AddInstance(Border, Sphere, { 7 });
		TestTrue(TEXT("Bordering a sphere holds no settlement back: no threshold on the pressure"), Can(Border, 1, 6));
	}
	{
		FCosmicDataTables NoRow = TablesWith(Original, Rules, ProjectRows(nullptr), nullptr);
		TestTrue(TEXT("Without the settle row there is no target"), Targets(Base, NoRow, 1).IsEmpty());
		TestFalse(TEXT("and no settlement"), CosmicMegaprojects::CanSettle(Base, Index, NoRow, Rules, 1, 6));
		FCosmicDataTables HighRow = TablesWith(Original, Rules, ProjectRows(&Causal), nullptr);
		TestTrue(TEXT("Below the row's Tier there is no target"), Targets(Base, HighRow, 1).IsEmpty());
		TestTrue(TEXT("MaySettle stays true: the row hides only the targets"), CosmicMegaprojects::MaySettle(Base, Index, Rules, 1));
	}

	TestEqual(TEXT("The realm cap of Tier 1"), CosmicMegaprojects::RealmCapOf(Rules, EAscensionTier::Tier1_Planetary), 1);
	TestEqual(TEXT("The realm cap of Tier 2"), CosmicMegaprojects::RealmCapOf(Rules, Stellar), 3);
	TestEqual(TEXT("The realm cap of Tier 5"), CosmicMegaprojects::RealmCapOf(Rules, EAscensionTier::Tier5_EternalOrder), 10);
	{
		FCosmicRulesDef Short = Rules;
		Short.RealmCapByTier = { 1, 3 };
		TestEqual(TEXT("A tier the cap array lacks has a cap of 0"), CosmicMegaprojects::RealmCapOf(Short, Causal), 0);
		FGalaxyState Case = Base;
		Case.Civs[1].Tier = Causal;
		TestFalse(TEXT("so a people of that tier may not settle"), CosmicMegaprojects::MaySettle(Case, Index, Short, 1));
	}
	TestTrue(TEXT("A Tier 2 people with one settled system may settle"), CosmicMegaprojects::MaySettle(Base, Index, Rules, 1));
	TestFalse(TEXT("A Tier 1 people with one system is at its cap"), CosmicMegaprojects::MaySettle(Base, Index, Rules, 2));
	{
		FGalaxyState Case = Base;
		Hand(Case, 1, 1);
		Hand(Case, 1, 9);
		TestFalse(TEXT("A Tier 2 people with three systems is at its cap"), CosmicMegaprojects::MaySettle(Case, Index, Rules, 1));
		Case.Systems[9].OwnerCivID = NAME_None;
		TestTrue(TEXT("A listed system the people does not own does not count toward the cap"), CosmicMegaprojects::MaySettle(Case, Index, Rules, 1));
	}
	{
		FGalaxyState Case = Base;
		HandSettling(Case, 1, 4);
		TestTrue(TEXT("A people with a settlement under way is settling"), CosmicMegaprojects::IsSettling(Case, Index, 1));
		TestFalse(TEXT("and may not start another"), CosmicMegaprojects::MaySettle(Case, Index, Rules, 1));
		TestTrue(TEXT("so it has no target"), Targets(Case, Tables, 1).IsEmpty());
	}
	{
		FGalaxyState Case = Base;
		Case.Systems[5].OwnerCivID = NAME_None;
		Case.Civs[1].OwnedSystems.Reset();
		TestFalse(TEXT("A people without a system may not settle"), CosmicMegaprojects::MaySettle(Case, Index, Rules, 1));
	}
	{
		FGalaxyState Case = Base;
		Case.Civs[0].Tier = Stellar;
		TestTrue(TEXT("The people the player rules may settle under the same rule"), CosmicMegaprojects::MaySettle(Case, Index, Rules, 0));
	}

	// ---- Starting a settlement ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the start is laid"), State))
		{
			return false;
		}
		// A wild system may carry a leftover slot from an earlier owner: the settlement starts from 0 whatever it holds.
		State->Systems[6].ActiveMegaproject = FName(TEXT("project_test_leftover"));
		State->Systems[6].MegaprojectProgress = 2.5f;
		const int32 SeedBefore = Sim->GetRng().GetCurrentSeed();
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		TestFalse(TEXT("A settlement of a system that is no target is refused"), CosmicMegaprojects::Settle(Ctx, 1, 7));
		TestTrue(TEXT("and the system stays wild"), State->Systems[7].OwnerCivID.IsNone());
		TestTrue(TEXT("A settlement of a target starts"), CosmicMegaprojects::Settle(Ctx, 1, 6));
		TestEqual(TEXT("The target becomes the people's"), State->Systems[6].OwnerCivID, State->Civs[1].CivID);
		TestEqual(TEXT("at the end of its systems"), State->Civs[1].OwnedSystems.Last(), State->Systems[6].SystemID);
		TestEqual(TEXT("Its construction slot holds the settlement"), State->Systems[6].ActiveMegaproject, CosmicRows::ProjectSettle);
		TestEqual(TEXT("from 0, whatever the slot held"), State->Systems[6].MegaprojectProgress, 0.f);
		TestFalse(TEXT("It is not settled yet"), CosmicState::IsSettled(State->Systems[6]));
		TestFalse(TEXT("A second settlement waits for the first"), CosmicMegaprojects::Settle(Ctx, 1, 4));
		TestTrue(TEXT("and its target stays wild"), State->Systems[4].OwnerCivID.IsNone());
		TestEqual(TEXT("Starting a settlement draws nothing"), Sim->GetRng().GetCurrentSeed(), SeedBefore);
	}

	// ---- A settlement in the ticks: no income and no hazard until it completes, then both ----
	{
		const FCosmicDataTables Hazardous = TablesWith(Original, Rules, ProjectRows(&Stellar), HazardRows(true));
		FGalaxyState* State = LayBoard(*Sim, Hazardous);
		if (!TestNotNull(TEXT("The board of the ticks is laid"), State))
		{
			return false;
		}
		// The income of people 1 with 6 wild, being settled and settled, on the same tick.
		const FGalaxyState Wild = *State;
		Sim->StepOnce();
		const FResourcePool IncomeWild = Sim->GetState().Civs[1].IncomePerTick;
		FGalaxyState Settled = Wild;
		Hand(Settled, 1, 6);
		Sim->LoadState(Settled);
		Sim->StepOnce();
		const FResourcePool IncomeSettled = Sim->GetState().Civs[1].IncomePerTick;
		TestTrue(TEXT("A settled system pays (the comparison can tell)"), IncomeSettled.Energy > IncomeWild.Energy);
		// People 1 also lists 8, which people 2 owns: a listed system another people owns pays it nothing.
		FGalaxyState Listed = Wild;
		Listed.Civs[1].OwnedSystems.Add(Listed.Systems[8].SystemID);
		Sim->LoadState(Listed);
		Sim->StepOnce();
		TestEqual(TEXT("A listed system another people owns pays nothing"), Sim->GetState().Civs[1].IncomePerTick.Energy, IncomeWild.Energy);
		FGalaxyState Starting = Wild;
		Sim->LoadState(Starting);
		{
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			TestTrue(TEXT("The settlement of the ticks starts"), CosmicMegaprojects::Settle(Ctx, 1, 6));
		}
		FGalaxyState& Live = Sim->GetMutableState();
		Sim->StepOnce();
		TestEqual(TEXT("A system being settled pays no Energy"), Live.Civs[1].IncomePerTick.Energy, IncomeWild.Energy);
		TestEqual(TEXT("and no Matter"), Live.Civs[1].IncomePerTick.Matter, IncomeWild.Matter);
		TestTrue(TEXT("A hazard that spawns on every settled system stands on the people's settled system"), Live.Systems[5].ActiveHazard.IsValid());
		TestFalse(TEXT("but not on the system being settled"), Live.Systems[6].ActiveHazard.IsValid());
		TestEqual(TEXT("A paid tick progresses the settlement"), Live.Systems[6].MegaprojectProgress, 1.f);
		Sim->StepOnce();
		TestFalse(TEXT("Two paid ticks do not complete it"), CosmicState::IsSettled(Live.Systems[6]));
		Sim->StepOnce();
		const int64 Done = Live.Tick;
		TestTrue(TEXT("The third paid tick completes it: the system is settled"), CosmicState::IsSettled(Live.Systems[6]));
		const FMegaprojectRecord* Record = Live.Systems[6].CompletedMegaprojects.FindByPredicate([](const FMegaprojectRecord& Each) { return Each.Row == CosmicRows::ProjectSettle; });
		if (TestNotNull(TEXT("and records the settlement"), Record))
		{
			TestEqual(TEXT("at the tick it completed"), Record->Tick, Done);
		}
		if (TestFalse(TEXT("The vessel has a life"), Live.Calling.Lives.IsEmpty()))
		{
			TestFalse(TEXT("The life of the vessel does not take an AI people's settlement"), Live.Calling.Lives.Last().MegaprojectsCompleted.Contains(CosmicRows::ProjectSettle));
		}
		const FCosmicLogEntry* Line = Live.Log.FindByPredicate([Done](const FCosmicLogEntry& Each) { return Each.SourceRow == CosmicRows::ProjectSettle && Each.Tick == Done; });
		if (TestNotNull(TEXT("The completion is told"), Line))
		{
			TestEqual(TEXT("about the settled system"), Line->FocusSystemID, Live.Systems[6].SystemID);
			TestEqual(TEXT("and its people"), Line->FocusCivID, Live.Civs[1].CivID);
			TestEqual(TEXT("as a line of an AI people"), static_cast<int32>(Line->Severity), 0);
			TestTrue(TEXT("of the civilization category"), Line->Category == Tag(TEXT("Log.Civ")));
			TestFalse(TEXT("with every slot filled"), Line->Message.ToString().Contains(TEXT("{")));
		}
		TestEqual(TEXT("In the tick a settlement completes the AI starts its next one"), SettlingCount(Live, Live.Civs[1]), 1);
		TestEqual(TEXT("so the people holds three systems"), OwnedCount(Live, Live.Civs[1]), 3);
		Sim->StepOnce();
		TestEqual(TEXT("From the next tick the settled system pays Energy"), Live.Civs[1].IncomePerTick.Energy, IncomeSettled.Energy);
		TestEqual(TEXT("and Matter"), Live.Civs[1].IncomePerTick.Matter, IncomeSettled.Matter);
	}

	// ---- An AI people's settlement: unpaid it waits, paid it goes at the AI's pace ----
	{
		FCosmicRulesDef Doubled = Rules;
		Doubled.AIAscensionSpeedMultiplier = 2.f;
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Doubled, ProjectRows(&Stellar, true), nullptr));
		if (!TestNotNull(TEXT("The board of the pace is laid"), State))
		{
			return false;
		}
		{
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			TestTrue(TEXT("The settlement of the pace starts"), CosmicMegaprojects::Settle(Ctx, 1, 6));
		}
		FGalaxyState& Live = Sim->GetMutableState();
		// Half a step already made: an unpaid tick neither adds to it nor throws it away.
		Live.Systems[6].MegaprojectProgress = 0.5f;
		Sim->StepOnce();
		TestEqual(TEXT("A settlement that cannot be paid neither progresses nor loses its progress"), Live.Systems[6].MegaprojectProgress, 0.5f);
		TestEqual(TEXT("and waits in its slot"), Live.Systems[6].ActiveMegaproject, CosmicRows::ProjectSettle);
		TestFalse(TEXT("so the system is not settled for free"), CosmicState::IsSettled(Live.Systems[6]));
		Live.Civs[1].Stockpile.Causality = 10.f;
		Sim->StepOnce();
		TestEqual(TEXT("A paid tick of an AI people's settlement progresses at the AI's pace"), Live.Systems[6].MegaprojectProgress, 2.5f);
	}

	// ---- Answers before constructions: a settlement on a lower system does not take what the capital's answer needs ----
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Rules, ProjectRows(&Stellar, true), nullptr));
		if (!TestNotNull(TEXT("The board of the payment order is laid"), State))
		{
			return false;
		}
		// People 1 holds its capital 5 and settles 4 (a lower index); a storm stands on 5 with its answer under way. One
		// Causality pays one of the two rows.
		{
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			TestTrue(TEXT("The settlement of the payment order starts"), CosmicMegaprojects::Settle(Ctx, 1, 4));
		}
		FGalaxyState& Live = Sim->GetMutableState();
		Live.Systems[5].ActiveHazard = Tag(TEXT("Hazard.Plague"));
		Live.Systems[5].HazardTicksRemaining = 1000;
		Live.Systems[5].ResponseMegaproject = Answer;
		Live.Civs[1].Stockpile.Causality = 1.f;
		const FGalaxyState OneUnit = Live;
		Sim->StepOnce();
		TestEqual(TEXT("The answer is paid first"), Live.Systems[5].ResponseProgress, 1.f);
		TestEqual(TEXT("and the settlement on a lower system waits"), Live.Systems[4].MegaprojectProgress, 0.f);
		FGalaxyState TwoUnits = OneUnit;
		TwoUnits.Civs[1].Stockpile.Causality = 2.f;
		Sim->LoadState(TwoUnits);
		Sim->StepOnce();
		TestEqual(TEXT("With enough for both the settlement is paid too"), Sim->GetState().Systems[4].MegaprojectProgress, 1.f);
	}

	// ---- Answers before constructions on one system: the capital's construction waits for its answer, and each slot is
	// paid once a tick ----
	{
		const FName Building(TEXT("project_test_build"));
		UDataTable* Projects = ProjectRows(&Stellar, true);
		FMegaprojectDef Build = Project(EAscensionTier::Tier1_Planetary, 0.f, 0.f, 9.f, TEXT("build"));
		Build.CostPerTick.Causality = 1.f;
		Projects->AddRow(Building, Build);
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Rules, Projects, nullptr));
		if (!TestNotNull(TEXT("The board of the payment order on one system is laid"), State))
		{
			return false;
		}
		// People 1's capital 5 builds and answers a storm at once. One Causality pays one of the two rows; three pay both
		// with one to spare.
		State->Systems[5].ActiveMegaproject = Building;
		State->Systems[5].ActiveHazard = Tag(TEXT("Hazard.Plague"));
		State->Systems[5].HazardTicksRemaining = 1000;
		State->Systems[5].ResponseMegaproject = Answer;
		State->Civs[1].Stockpile.Causality = 1.f;
		const FGalaxyState OneUnit = *State;
		Sim->StepOnce();
		TestEqual(TEXT("On one system the answer is paid first"), Sim->GetState().Systems[5].ResponseProgress, 1.f);
		TestEqual(TEXT("and the construction beside it waits"), Sim->GetState().Systems[5].MegaprojectProgress, 0.f);
		FGalaxyState ThreeUnits = OneUnit;
		ThreeUnits.Civs[1].Stockpile.Causality = 3.f;
		Sim->LoadState(ThreeUnits);
		Sim->StepOnce();
		TestEqual(TEXT("With more than enough the construction is paid once a tick"), Sim->GetState().Systems[5].MegaprojectProgress, 1.f);
		TestEqual(TEXT("and the answer once"), Sim->GetState().Systems[5].ResponseProgress, 1.f);
	}

	// ---- The AI's draw: one per people that may settle, before the targets are looked at, picking the target ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the draws is laid"), State))
		{
			return false;
		}
		const FGalaxyState Board0 = *State;
		FGalaxyState After;
		TestEqual(TEXT("One people that may settle: one draw"), AIStageDraws(*Sim, Tables, Board0, After), 1);
		TestEqual(TEXT("and it settles one of its targets"), SettlingCount(After, After.Civs[1]), 1);
		{
			FGalaxyState Case = Board0;
			Case.Civs[2].Tier = Stellar;
			TestEqual(TEXT("Two peoples that may settle: two draws"), AIStageDraws(*Sim, Tables, Case, After), 2);
		}
		{
			// People 0 takes 4 and 6: people 1 may settle and has no target.
			FGalaxyState Case = Board0;
			Hand(Case, 0, 4);
			Hand(Case, 0, 6);
			TestEqual(TEXT("A people that may settle draws with no target too"), AIStageDraws(*Sim, Tables, Case, After), 1);
			TestEqual(TEXT("and settles nothing"), SettlingCount(After, After.Civs[1]), 0);
		}
		{
			FGalaxyState Case = Board0;
			Hand(Case, 1, 1);
			Hand(Case, 1, 9);
			TestEqual(TEXT("A people at its cap draws nothing"), AIStageDraws(*Sim, Tables, Case, After), 0);
		}
		{
			// People 1 holds 5 and the settlement on 4: two of its three, so only the settlement keeps it from drawing.
			FGalaxyState Case = Board0;
			HandSettling(Case, 1, 4);
			TestEqual(TEXT("A people with a settlement under way draws nothing"), AIStageDraws(*Sim, Tables, Case, After), 0);
		}
		{
			FGalaxyState Case = Board0;
			Case.Civs[0].Tier = Stellar;
			TestEqual(TEXT("The people the player rules draws nothing"), AIStageDraws(*Sim, Tables, Case, After), 1);
			TestEqual(TEXT("and settles nothing by itself"), SettlingCount(After, After.Civs[0]), 0);
		}
		{
			FGalaxyState Case = Board0;
			Case.Civs[2].Tier = Stellar;
			Case.Systems[8].OwnerCivID = NAME_None;
			Case.Civs[2].OwnedSystems.Reset();
			TestEqual(TEXT("A people without a system draws nothing"), AIStageDraws(*Sim, Tables, Case, After), 1);
		}
		{
			const FCosmicDataTables NoRow = TablesWith(Original, Rules, ProjectRows(nullptr), nullptr);
			TestEqual(TEXT("Without the settle row the draws are the same"), AIStageDraws(*Sim, NoRow, Board0, After), 1);
			TestEqual(TEXT("and nothing is settled"), SettlingCount(After, After.Civs[1]), 0);
			const FCosmicDataTables HighRow = TablesWith(Original, Rules, ProjectRows(&Causal), nullptr);
			TestEqual(TEXT("Under a row of a higher Tier the draws are the same"), AIStageDraws(*Sim, HighRow, Board0, After), 1);
			TestEqual(TEXT("and nothing is settled either"), SettlingCount(After, After.Civs[1]), 0);
		}

		// The number picks the target: target PickIndex(number, 2) of {4, 6}, over several streams so that both occur.
		{
			const int32 Choices[] = { 4, 6 };
			int32 Mismatches = 0;
			bool bSawFirst = false;
			bool bSawSecond = false;
			for (int32 Seed = 1; Seed <= 16; ++Seed)
			{
				FGalaxyState Case = Board0;
				Case.RngState = Seed * 7919;
				int32 Before = 0;
				AIStageDraws(*Sim, Tables, Case, After, &Before);
				FRandomStream Walk(Before);
				const int32 Expected = Choices[PickOf(Walk.GetFraction(), 2)];
				const int32 Settled = SettlingTarget(After, After.Civs[1].CivID);
				Mismatches += Settled == Expected ? 0 : 1;
				bSawFirst |= Settled == 4;
				bSawSecond |= Settled == 6;
			}
			TestEqual(TEXT("Every settlement number picks the target it falls on"), Mismatches, 0);
			TestTrue(TEXT("The first target was picked by some number"), bSawFirst);
			TestTrue(TEXT("and so was the second"), bSawSecond);
		}

		// Two peoples share one target: the earlier in Civs order takes it, and the later still draws.
		{
			FGalaxyState Case = Board0;
			Case.Civs[2].Tier = Stellar;
			Hand(Case, 2, 7);
			Hand(Case, 3, 4);
			Hand(Case, 3, 9);
			TestEqual(TEXT("Two peoples that share a target draw twice"), AIStageDraws(*Sim, Tables, Case, After), 2);
			TestEqual(TEXT("and the earlier people in Civs order takes the shared target"), After.Systems[6].OwnerCivID, After.Civs[1].CivID);
			TestEqual(TEXT("the later one settles nothing"), SettlingCount(After, After.Civs[2]), 0);
		}

		// The settlement number comes before the name a feat draws in the same tick: the AI stage settles, then judges.
		{
			UDataTable* Feats = NewTable(FAscensionFeatDef::StaticStruct());
			FAscensionFeatDef Climb;
			Climb.Title = FText::AsCultureInvariant(TEXT("climb"));
			Climb.RequiredTier = Stellar;
			Climb.TargetTier = Causal;
			Climb.Path = EPlayerRole::Ruler;
			FCosmicCondition Always0;
			Always0.Type = EConditionType::CivResearchAtLeast;
			Always0.Key = FName(TEXT("Physics"));
			Always0.Value = 0.f;
			Climb.CompletionConditions = { Always0 };
			Climb.LogTemplates = { FText::AsCultureInvariant(TEXT("{Civ} climbed")) };
			Feats->AddRow(FName(TEXT("feat_test_climb")), Climb);
			const FCosmicDataTables Climbing = TablesWith(Original, Rules, ProjectRows(&Stellar), nullptr, Feats);
			const int32 Choices[] = { 4, 6 };
			bool bTold = false;
			for (int32 Seed = 1; Seed <= 32 && !bTold; ++Seed)
			{
				FGalaxyState Case = Board0;
				Case.RngState = Seed * 104729;
				int32 Before = 0;
				const int32 Draws = AIStageDraws(*Sim, Climbing, Case, After, &Before);
				FRandomStream Walk(Before);
				const float First = Walk.GetFraction();
				const float Second = Walk.GetFraction();
				if (PickOf(First, 2) == PickOf(Second, 2))
				{
					continue;
				}
				bTold = true;
				TestEqual(TEXT("A people that settles and climbs in one tick draws twice"), Draws, 2);
				TestEqual(TEXT("and climbs"), static_cast<int32>(After.Civs[1].Tier), static_cast<int32>(Causal));
				TestEqual(TEXT("Its settlement takes the first number"), SettlingTarget(After, After.Civs[1].CivID), Choices[PickOf(First, 2)]);
			}
			TestTrue(TEXT("Some stream told the two numbers apart"), bTold);
		}
	}

	// ---- The ruler's command and the prototype's project key ----
	{
		FCosmicRulesDef Doubled = Rules;
		Doubled.AIAscensionSpeedMultiplier = 2.f;
		const FCosmicDataTables Stormy = TablesWith(Original, Doubled, ProjectRows(&Stellar), HazardRows(false));
		FGalaxyState* State = LayBoard(*Sim, Stormy);
		if (!TestNotNull(TEXT("The board of the command is laid"), State))
		{
			return false;
		}
		// People 0 (the vessel's) moves to 4 on Tier 2: its targets are 3 and 10 (5 is people 1's).
		CosmicTests::ClearOwners(*State);
		CosmicTests::Give(*State, 0, { 4 });
		CosmicTests::Give(*State, 1, { 5 });
		CosmicTests::Give(*State, 2, { 8 });
		CosmicTests::Give(*State, 3, { 11 });
		State->Civs[0].Tier = Stellar;
		Sim->RebuildStateIndex();
		const FGalaxyState Ruling = *State;
		const FName Three = State->Systems[3].SystemID;
		const FName Ten = State->Systems[10].SystemID;

		State->Player.Role = EPlayerRole::Wanderer;
		TestFalse(TEXT("A wanderer does not settle"), PlayerRole->StartMegaproject(Three, CosmicRows::ProjectSettle));
		State->Player.Role = EPlayerRole::Ruler;
		TestFalse(TEXT("The ruler does not settle a system that is no target"), PlayerRole->StartMegaproject(State->Systems[2].SystemID, CosmicRows::ProjectSettle));
		TestFalse(TEXT("nor the people's own system"), PlayerRole->StartMegaproject(State->Systems[4].SystemID, CosmicRows::ProjectSettle));
		const int32 SeedBefore = Sim->GetRng().GetCurrentSeed();
		TestTrue(TEXT("The ruler settles a target"), PlayerRole->StartMegaproject(Three, CosmicRows::ProjectSettle));
		TestEqual(TEXT("which becomes the people's"), State->Systems[3].OwnerCivID, State->Civs[0].CivID);
		TestEqual(TEXT("and is being settled"), State->Systems[3].ActiveMegaproject, CosmicRows::ProjectSettle);
		TestEqual(TEXT("The command draws nothing"), Sim->GetRng().GetCurrentSeed(), SeedBefore);
		TestFalse(TEXT("The ruler's second settlement waits for the first"), PlayerRole->StartMegaproject(Ten, CosmicRows::ProjectSettle));
		Sim->StepOnce();
		TestEqual(TEXT("The settlement of the ruler's people progresses at the player's pace, not the AI's"), State->Systems[3].MegaprojectProgress, 1.f);

		// The key: what Choices holds first, then a settlement of the first target.
		Sim->LoadState(Ruling);
		FGalaxyState& Keyed = Sim->GetMutableState();
		Keyed.Systems[4].ActiveHazard = Tag(TEXT("Hazard.Plague"));
		Keyed.Systems[4].HazardTicksRemaining = 1000;
		TestTrue(TEXT("The key starts what there is to answer first"), PlayerRole->StartNextMegaproject());
		TestEqual(TEXT("the answer of the storm"), Keyed.Systems[4].ResponseMegaproject, Answer);
		TestTrue(TEXT("and settles nothing then"), Keyed.Systems[3].OwnerCivID.IsNone());
		TestTrue(TEXT("With nothing to answer or build the key settles"), PlayerRole->StartNextMegaproject());
		TestEqual(TEXT("the first target in Systems order"), Keyed.Systems[3].ActiveMegaproject, CosmicRows::ProjectSettle);
		TestFalse(TEXT("With a settlement under way and nothing else the key does nothing"), PlayerRole->StartNextMegaproject());

		// The cap, on Tier 2: two systems settle, three do not.
		Sim->LoadState(Ruling);
		Hand(Sim->GetMutableState(), 0, 2);
		const FGalaxyState Below = Sim->GetState();
		Hand(Sim->GetMutableState(), 0, 0);
		TestFalse(TEXT("The ruler's people at its cap does not settle"), PlayerRole->StartMegaproject(Three, CosmicRows::ProjectSettle));
		TestFalse(TEXT("and the key has nothing to start"), PlayerRole->StartNextMegaproject());
		Sim->LoadState(Below);
		TestTrue(TEXT("One system below the cap the ruler's people settles"), PlayerRole->StartMegaproject(Three, CosmicRows::ProjectSettle));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicRealmCapTest, "Cosmic_Ascension.Sim.RealmCap", CosmicTests::Flags)
bool FCosmicRealmCapTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSettlementTests;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	// The game's data, 20,000 ticks: no people ever holds more systems than the cap of its Tier (the game's rows hand no
	// system over, so every system a people gains comes from a settlement), and no people settles two at once. The bound
	// is read from DT_Rules itself, not through the function under test, and some people does reach its cap.
	{
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		const FGalaxyState& State = Sim->GetState();
		const TArray<int32> Caps = Sim->GetRules().RealmCapByTier;
		int64 OverCapTick = 0;
		int64 TwoAtOnceTick = 0;
		int64 ReachedCapTick = 0;
		int32 MostSystems = 0;
		int32 Settlements = 0;
		for (int32 Step = 0; Step < 20000; ++Step)
		{
			Sim->StepOnce();
			for (const FCivilizationState& Civ : State.Civs)
			{
				const int32 Owned = OwnedCount(State, Civ);
				const int32 TierIndex = static_cast<int32>(Civ.Tier);
				const int32 Cap = Caps.IsValidIndex(TierIndex) ? Caps[TierIndex] : 0;
				MostSystems = FMath::Max(MostSystems, Owned);
				if (OverCapTick == 0 && Owned > Cap)
				{
					OverCapTick = State.Tick;
				}
				if (ReachedCapTick == 0 && Owned == Cap && Cap > 1)
				{
					ReachedCapTick = State.Tick;
				}
				if (TwoAtOnceTick == 0 && SettlingCount(State, Civ) > 1)
				{
					TwoAtOnceTick = State.Tick;
				}
			}
		}
		for (const FStarSystemState& System : State.Systems)
		{
			Settlements += System.CompletedMegaprojects.ContainsByPredicate([](const FMegaprojectRecord& Each) { return Each.Row == CosmicRows::ProjectSettle; }) ? 1 : 0;
		}
		TestEqual(TEXT("No people holds more systems than the cap of its Tier (the first tick it did)"), OverCapTick, int64(0));
		TestEqual(TEXT("No people settles two systems at once (the first tick it did)"), TwoAtOnceTick, int64(0));
		TestTrue(TEXT("The peoples do settle in the game (the run is not empty)"), Settlements > 0);
		TestTrue(TEXT("Some people holds more than one system"), MostSystems > 1);
		TestTrue(TEXT("Some people reaches a cap above one (the bound is met, not only kept)"), ReachedCapTick > 0);
	}

	// A people over its cap by a hand-over keeps what it was handed and settles no more, by the AI's draw or the command.
	{
		const EAscensionTier Stellar = EAscensionTier::Tier2_Stellar;
		const FCosmicDataTables Tables = TablesWith(Original, Knobs(Sim->GetRules()), ProjectRows(&Stellar), nullptr);
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the hand-over is laid"), State))
		{
			return false;
		}
		// People 1 (Tier 2) holds 5, 1 and 9; people 2 hands it 8: four systems over a cap of three.
		Hand(*State, 1, 1);
		Hand(*State, 1, 9);
		Hand(*State, 2, 7);
		{
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			TestTrue(TEXT("The hand-over goes through"), CosmicHazards::Transfer(Ctx, 8, 1));
		}
		TestEqual(TEXT("The people holds four systems"), OwnedCount(*State, State->Civs[1]), 4);
		TestFalse(TEXT("A people over its cap may not settle"), CosmicMegaprojects::MaySettle(*State, Sim->GetStateIndex(), Sim->GetRules(), 1));
		Sim->StepOnce();
		TestEqual(TEXT("It keeps what it was handed and settles nothing"), OwnedCount(*State, State->Civs[1]), 4);
		TestEqual(TEXT("and starts no settlement"), SettlingCount(*State, State->Civs[1]), 0);

		// The people the player rules, over its cap by hand (0, 2, 3 and 10 on Tier 2), is held back by the command alike:
		// 4 lies one lane from 3 and 10.
		State->Civs[0].Tier = Stellar;
		Hand(*State, 0, 2);
		Hand(*State, 0, 3);
		Hand(*State, 0, 10);
		TestTrue(TEXT("The ruler's target is still wild"), State->Systems[4].OwnerCivID.IsNone());
		TestFalse(TEXT("and the ruler's people is not settling"), CosmicMegaprojects::IsSettling(*State, Sim->GetStateIndex(), 0));
		UPlayerRoleSubsystem* PlayerRole = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
		if (TestNotNull(TEXT("The role subsystem exists"), PlayerRole))
		{
			TestFalse(TEXT("The ruler's people over its cap does not settle"), PlayerRole->StartMegaproject(State->Systems[4].SystemID, CosmicRows::ProjectSettle));
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicCivRemovalTest, "Cosmic_Ascension.Sim.CivRemoval", CosmicTests::Flags)
bool FCosmicCivRemovalTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSettlementTests;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const EAscensionTier Stellar = EAscensionTier::Tier2_Stellar;
	FCosmicRulesDef Rules = Knobs(Sim->GetRules());
	Rules.RealmCapByTier = { 1, 1, 1, 1, 1 };
	// Settlements cost a Causality no board pays: they stand still where the tests put them.
	const FCosmicDataTables Tables = TablesWith(Original, Rules, ProjectRows(&Stellar, true), HazardRows(false));

	// Peoples 2 and 3 lose everything between ticks: 2 by hand, 3 because the system it lists is people 1's. What the
	// cleanup clears and what it leaves.
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the cleanup is laid"), State))
		{
			return false;
		}
		const FName Zero = State->Civs[0].CivID;
		const FName One = State->Civs[1].CivID;
		const FName Two = State->Civs[2].CivID;
		const FName Three = State->Civs[3].CivID;
		State->Civs[0].Relations.Add(Two, 0.4f);
		State->Civs[0].Relations.Add(Three, -0.2f);
		State->Civs[0].Relations.Add(One, 0.1f);
		State->Civs[0].Relations.Add(Sphere, -0.5f);
		State->Civs[1].Relations.Add(Two, 0.3f);
		State->Knowing.PendingContactCivID = Three;
		State->Knowing.FirstContactCivID = Two;
		FKnownMark& Met = State->Knowing.MetCivs.AddDefaulted_GetRef();
		Met.ID = Two;
		Met.Tick = 0;
		const FName CooldownKey = UCosmicEventSubsystem::CooldownKey(FName(TEXT("event_test_row")), Two);
		State->EventCooldownUntilTick.Add(CooldownKey, 99);
		State->Player.FormerCivID = Three;
		State->Systems[8].OwnerCivID = NAME_None;
		State->Civs[2].OwnedSystems.Reset();
		State->Systems[11].OwnerCivID = One;
		Sim->StepOnce();
		if (!TestEqual(TEXT("Two peoples without a system leave the civilizations"), State->Civs.Num(), 2))
		{
			return false;
		}
		TestEqual(TEXT("The first that stays keeps its place"), State->Civs[0].CivID, Zero);
		TestEqual(TEXT("and so does the second, in array order"), State->Civs[1].CivID, One);
		TestFalse(TEXT("A removed people leaves the relations of the others"), State->Civs[0].Relations.Contains(Two));
		TestFalse(TEXT("both removed peoples do"), State->Civs[0].Relations.Contains(Three));
		TestFalse(TEXT("in every people"), State->Civs[1].Relations.Contains(Two));
		TestEqual(TEXT("The relation with a people that stays is kept"), State->Civs[0].Relations.FindRef(One), 0.1f);
		TestEqual(TEXT("and so is the relation with a transcendent"), State->Civs[0].Relations.FindRef(Sphere), -0.5f);
		TestTrue(TEXT("A pending contact with the last of two removed peoples is cleared"), State->Knowing.PendingContactCivID.IsNone());
		TestEqual(TEXT("The first contact stays on the record"), State->Knowing.FirstContactCivID, Two);
		TestTrue(TEXT("and so does the meeting"), State->Knowing.MetCivs.ContainsByPredicate([Two](const FKnownMark& Mark) { return Mark.ID == Two; }));
		TestTrue(TEXT("and the cooldown of a removed people (the sweep takes it later)"), State->EventCooldownUntilTick.Contains(CooldownKey));
		TestEqual(TEXT("and the former people of the vessel"), State->Player.FormerCivID, Three);
		TestEqual(TEXT("The index finds the people that stays"), Sim->GetStateIndex().FindCiv(*State, One), 1);
		TestEqual(TEXT("and no longer finds a removed people"), Sim->GetStateIndex().FindCiv(*State, Two), static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("nor the other"), Sim->GetStateIndex().FindCiv(*State, Three), static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("A removal tells no line"), State->Log.FilterByPredicate([State](const FCosmicLogEntry& Each) { return Each.Tick == State->Tick; }).Num(), 0);
	}

	// The same two peoples leave, the pending contact naming the first of them.
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the first removed contact is laid"), State))
		{
			return false;
		}
		State->Knowing.PendingContactCivID = State->Civs[2].CivID;
		State->Systems[8].OwnerCivID = NAME_None;
		State->Civs[2].OwnedSystems.Reset();
		State->Systems[11].OwnerCivID = State->Civs[1].CivID;
		Sim->StepOnce();
		TestEqual(TEXT("The two peoples leave again"), State->Civs.Num(), 2);
		TestTrue(TEXT("A pending contact with the first of two removed peoples is cleared"), State->Knowing.PendingContactCivID.IsNone());
	}

	// A people from the middle of Civs: those after it move up in order, and their keys to it go.
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the middle removal is laid"), State))
		{
			return false;
		}
		const FName Zero = State->Civs[0].CivID;
		const FName One = State->Civs[1].CivID;
		const FName Two = State->Civs[2].CivID;
		const FName Three = State->Civs[3].CivID;
		State->Civs[3].Relations.Add(One, 0.2f);
		State->Systems[5].OwnerCivID = NAME_None;
		State->Civs[1].OwnedSystems.Reset();
		Sim->StepOnce();
		if (!TestEqual(TEXT("A people from the middle leaves"), State->Civs.Num(), 3))
		{
			return false;
		}
		TestEqual(TEXT("The first keeps its place"), State->Civs[0].CivID, Zero);
		TestEqual(TEXT("the one after the removed moves up"), State->Civs[1].CivID, Two);
		TestEqual(TEXT("and the last after it, in array order"), State->Civs[2].CivID, Three);
		TestEqual(TEXT("The index finds the last at its new place"), Sim->GetStateIndex().FindCiv(*State, Three), 2);
		TestFalse(TEXT("A people after the removed one loses its key to it"), State->Civs[2].Relations.Contains(One));
	}

	// A pending contact with a people that stays is left alone.
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the pending contact is laid"), State))
		{
			return false;
		}
		const FName One = State->Civs[1].CivID;
		State->Knowing.PendingContactCivID = One;
		State->Systems[8].OwnerCivID = NAME_None;
		State->Civs[2].OwnedSystems.Reset();
		Sim->StepOnce();
		TestEqual(TEXT("A pending contact with a people that stays is kept"), State->Knowing.PendingContactCivID, One);
	}

	// The last settled system: a people that holds a settled system and a settlement stays; losing the settled one lets
	// the settlement go with it, and the people leaves.
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the last settled system is laid"), State))
		{
			return false;
		}
		const FName One = State->Civs[1].CivID;
		HandSettling(*State, 1, 4);
		State->Systems[4].MegaprojectProgress = 2.f;
		// System 4 completed a project under an earlier owner: its record stays whoever holds the system.
		const FName Earlier(TEXT("project_test_earlier"));
		FMegaprojectRecord& Record = State->Systems[4].CompletedMegaprojects.AddDefaulted_GetRef();
		Record.Row = Earlier;
		Record.Tick = 7;
		State->Systems[4].MegaprojectCompletedTick = 7;
		Sim->StepOnce();
		TestTrue(TEXT("A people with a settled system and a stalled settlement stays"), Sim->GetStateIndex().FindCiv(*State, One) != INDEX_NONE);
		TestEqual(TEXT("and keeps its settlement"), State->Systems[4].ActiveMegaproject, CosmicRows::ProjectSettle);
		TestEqual(TEXT("with its progress"), State->Systems[4].MegaprojectProgress, 2.f);
		{
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			TestFalse(TEXT("Its last settled system is not handed over, though a settlement remains"), CosmicHazards::Transfer(Ctx, 5, 2));
		}
		TestTrue(TEXT("The settled system is the last for the gate"), CosmicHazards::IsLastSettledSystem(*State, Sim->GetStateIndex(), 5));
		TestFalse(TEXT("the settlement is not"), CosmicHazards::IsLastSettledSystem(*State, Sim->GetStateIndex(), 4));
		State->Systems[5].ActiveHazard = Tag(TEXT("Hazard.PreSupernova"));
		State->Systems[5].HazardTicksRemaining = 1;
		Sim->StepOnce();
		TestTrue(TEXT("The last settled system went"), CosmicHazards::IsDeadNode(State->Systems[5]));
		TestTrue(TEXT("and the settlement went with it, to no one"), State->Systems[4].OwnerCivID.IsNone());
		TestTrue(TEXT("its slot empty"), State->Systems[4].ActiveMegaproject.IsNone());
		TestEqual(TEXT("and its progress gone"), State->Systems[4].MegaprojectProgress, 0.f);
		const FMegaprojectRecord* Kept = State->Systems[4].CompletedMegaprojects.FindByPredicate([Earlier](const FMegaprojectRecord& Each) { return Each.Row == Earlier; });
		if (TestNotNull(TEXT("The completion records of a settlement let go stay"), Kept))
		{
			TestEqual(TEXT("with their tick"), Kept->Tick, static_cast<int64>(7));
		}
		TestEqual(TEXT("and so does the system's completion tick"), State->Systems[4].MegaprojectCompletedTick, static_cast<int64>(7));
		TestEqual(TEXT("The people that had nothing left leaves the civilizations"), Sim->GetStateIndex().FindCiv(*State, One), static_cast<int32>(INDEX_NONE));
	}

	// The gate asks for the last settled system, a settlement under way or not: the grace keeps it.
	{
		UDataTable* Events = NewTable(FCosmicEventDef::StaticStruct());
		const FCosmicEventDef* Grace = Original.FindEvent(CosmicRows::EventGraceRemnant);
		if (!TestNotNull(TEXT("The game has the grace row of the remnant"), Grace))
		{
			return false;
		}
		Events->AddRow(CosmicRows::EventGraceRemnant, *Grace);
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Rules, ProjectRows(&Stellar, true), HazardRows(false), nullptr, Events));
		if (!TestNotNull(TEXT("The board of the grace is laid"), State))
		{
			return false;
		}
		HandSettling(*State, 1, 4);
		State->Systems[5].ActiveHazard = Tag(TEXT("Hazard.PreSupernova"));
		State->Systems[5].HazardTicksRemaining = 1;
		Sim->StepOnce();
		TestFalse(TEXT("The grace keeps a last settled system beside a settlement"), CosmicHazards::IsDeadNode(State->Systems[5]));
		TestTrue(TEXT("the grace of the run is spent on it"), State->Flags.HasTagExact(Tag(TEXT("Flag.Grace.Spent"))));
		TestEqual(TEXT("and the settlement stays the people's"), State->Systems[4].OwnerCivID, State->Civs[1].CivID);
	}

	// The people the player rules lists its settled capital 0 and system 5, which people 1 owns (a stale entry): the stale
	// entry is no home. The gate is asked for the capital and the grace keeps it; with the grace spent the capital goes,
	// the ruler falls, the stale entry is never made the capital, and the vessel is carried to a lane neighbour of the dead
	// system, not into the stale entry.
	{
		UDataTable* Events = NewTable(FCosmicEventDef::StaticStruct());
		const FCosmicEventDef* Grace = Original.FindEvent(CosmicRows::EventGraceRemnant);
		if (!TestNotNull(TEXT("The game has the grace row of the remnant for the stale entry"), Grace))
		{
			return false;
		}
		Events->AddRow(CosmicRows::EventGraceRemnant, *Grace);
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Rules, ProjectRows(&Stellar, true), HazardRows(false), nullptr, Events));
		if (!TestNotNull(TEXT("The board of the stale entry is laid"), State))
		{
			return false;
		}
		const FName Ruled = State->Civs[0].CivID;
		const FGameplayTag Spent = Tag(TEXT("Flag.Grace.Spent"));
		State->Civs[0].OwnedSystems.Add(State->Systems[5].SystemID);
		State->Systems[0].ActiveHazard = Tag(TEXT("Hazard.PreSupernova"));
		State->Systems[0].HazardTicksRemaining = 1;
		State->Player.CurrentSystemID = State->Systems[0].SystemID;
		const FGalaxyState Listed = *State;
		Sim->StepOnce();
		TestFalse(TEXT("The gate is asked for a capital beside a stale entry: the grace keeps it"), CosmicHazards::IsDeadNode(State->Systems[0]));
		TestTrue(TEXT("and the grace of the run is spent on it"), State->Flags.HasTagExact(Spent));

		FGalaxyState NoGrace = Listed;
		NoGrace.Flags.AddTag(Spent);
		Sim->LoadState(NoGrace);
		FGalaxyState& Live = Sim->GetMutableState();
		Sim->StepOnce();
		TestTrue(TEXT("With the grace spent the capital beside the stale entry goes"), CosmicHazards::IsDeadNode(Live.Systems[0]));
		TestTrue(TEXT("and the ruler falls, though the stale entry remains"), Live.Player.Role == EPlayerRole::Wanderer);
		const int32 Kept = Sim->GetStateIndex().FindCiv(Live, Ruled);
		if (TestTrue(TEXT("The people is kept for the tick of its loss"), Live.Civs.IsValidIndex(Kept)))
		{
			TestEqual(TEXT("The stale entry is never made the capital"), Live.Civs[Kept].HomeSystemID, Live.Systems[0].SystemID);
		}
		TestEqual(TEXT("The vessel is carried to a lane neighbour, not into the stale entry"), Live.Player.CurrentSystemID, Live.Systems[1].SystemID);
		Sim->StepOnce();
		TestEqual(TEXT("and the next tick's cleanup takes the people"), Sim->GetStateIndex().FindCiv(Live, Ruled), static_cast<int32>(INDEX_NONE));

		// The capital itself made the stale entry by hand (no path of the game does so): a ruler standing on the system
		// of its people that goes is not carried into a system its people does not own.
		FGalaxyState StaleHome = Listed;
		StaleHome.Flags.AddTag(Spent);
		StaleHome.Civs[0].HomeSystemID = StaleHome.Systems[5].SystemID;
		Sim->LoadState(StaleHome);
		Sim->StepOnce();
		TestEqual(TEXT("A ruler is never carried into a listed system its people does not own"), Sim->GetState().Player.CurrentSystemID, Sim->GetState().Systems[1].SystemID);
	}

	// The people the player rules loses its last settled system while it settles: the settlement goes, the ruler falls,
	// the people stays for the tick of its loss, researches nothing the next tick, and the next tick's cleanup takes it.
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the ruler's people is laid"), State))
		{
			return false;
		}
		const FName Ruled = State->Civs[0].CivID;
		HandSettling(*State, 0, 1);
		State->Systems[0].ActiveHazard = Tag(TEXT("Hazard.PreSupernova"));
		State->Systems[0].HazardTicksRemaining = 1;
		Sim->StepOnce();
		TestTrue(TEXT("The last settled system of the ruler's people went"), CosmicHazards::IsDeadNode(State->Systems[0]));
		TestTrue(TEXT("and its settlement went with it"), State->Systems[1].OwnerCivID.IsNone());
		TestTrue(TEXT("The ruler fell"), State->Player.Role == EPlayerRole::Wanderer);
		const int32 Kept = Sim->GetStateIndex().FindCiv(*State, Ruled);
		if (!TestTrue(TEXT("The people the player ruled in the tick of the loss stays for that tick"), State->Civs.IsValidIndex(Kept)))
		{
			return false;
		}
		TestFalse(TEXT("and no longer has the player"), State->Civs[Kept].bIsPlayerRuled);
		TestEqual(TEXT("It keeps the ID of its lost capital"), State->Civs[Kept].HomeSystemID, State->Systems[0].SystemID);
		// Both the kept people and people 1 are one step short of a Society mark: only people 1 crosses it.
		const FName Society(TEXT("Society"));
		for (const int32 CivIndex : { Kept, Sim->GetStateIndex().FindCiv(*State, State->Civs[1].CivID) })
		{
			State->Civs[CivIndex].Research.Add(Society, 0.4995f);
			State->Civs[CivIndex].ResearchFocus.Add(Society, 1.f);
		}
		const FName Control = State->Civs[1].CivID;
		Sim->StepOnce();
		auto ToldOf = [State](FName CivID)
		{
			return State->Log.ContainsByPredicate([State, CivID](const FCosmicLogEntry& Each)
			{
				return Each.Tick == State->Tick && Each.FocusCivID == CivID && Each.SourceRow.ToString().StartsWith(TEXT("log_breakthrough_"));
			});
		};
		TestTrue(TEXT("A people with a system crosses its mark (the board can tell)"), ToldOf(Control));
		TestFalse(TEXT("The kept people researches nothing: no breakthrough is told of it"), ToldOf(Ruled));
		TestEqual(TEXT("The next tick's cleanup takes it"), Sim->GetStateIndex().FindCiv(*State, Ruled), static_cast<int32>(INDEX_NONE));
		if (TestEqual(TEXT("and only it"), State->Civs.Num(), 3))
		{
			TestEqual(TEXT("The others move up in order"), State->Civs[0].CivID, Control);
		}
	}

	// A people holding only a settlement (made by hand: in play its last settled system takes it along) lets it go in the
	// crossing stage and leaves.
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the settlement alone is laid"), State))
		{
			return false;
		}
		const FName Two = State->Civs[2].CivID;
		State->Systems[8].OwnerCivID = NAME_None;
		State->Civs[2].OwnedSystems.Reset();
		HandSettling(*State, 2, 9);
		TestFalse(TEXT("A settlement is no last settled system, even when its people holds nothing else"), CosmicHazards::IsLastSettledSystem(*State, Sim->GetStateIndex(), 9));
		Sim->StepOnce();
		TestTrue(TEXT("A settlement whose people has no settled system goes to no one"), State->Systems[9].OwnerCivID.IsNone());
		TestTrue(TEXT("with an empty slot"), State->Systems[9].ActiveMegaproject.IsNone());
		TestEqual(TEXT("and its people leaves"), Sim->GetStateIndex().FindCiv(*State, Two), static_cast<int32>(INDEX_NONE));
	}

	// The people the player rules made to hold only a settlement by hand (no stage does so in play): the crossing stage
	// lets the settlement go, nothing deposes the ruler, so the people is kept, and an error line says so.
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the ruler's settlement alone is laid"), State))
		{
			return false;
		}
		const FName Ruled = State->Civs[0].CivID;
		State->Systems[0].OwnerCivID = NAME_None;
		State->Civs[0].OwnedSystems.Reset();
		HandSettling(*State, 0, 1);
		AddExpectedError(TEXT("held only settlements"), EAutomationExpectedErrorFlags::Contains, 1);
		Sim->StepOnce();
		TestTrue(TEXT("The settlement of the ruler's people goes to no one"), State->Systems[1].OwnerCivID.IsNone());
		TestTrue(TEXT("The people the player rules is kept"), Sim->GetStateIndex().FindCiv(*State, Ruled) != INDEX_NONE);
		TestTrue(TEXT("and its ruler still rules: nothing recorded a collapse"), State->Player.Role == EPlayerRole::Ruler);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicPopulationRegrowthTest, "Cosmic_Ascension.Sim.PopulationRegrowth", CosmicTests::Flags)
bool FCosmicPopulationRegrowthTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSettlementTests;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const EAscensionTier Stellar = EAscensionTier::Tier2_Stellar;
	FCosmicRulesDef Rules = Knobs(Sim->GetRules());
	Rules.RealmCapByTier = { 1, 1, 1, 1, 1 };

	// The board with the people the player rules at 0.7, people 1 at 0.5, people 2 just under 1, people 3 at Last (above
	// 1 unless given); then one tick under Knob.
	auto OneTick = [&](float Knob, float Last = 1.5f) -> FGalaxyState
	{
		FCosmicRulesDef With = Rules;
		With.PopulationRegrowthPerTick = Knob;
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, With, ProjectRows(&Stellar), nullptr));
		if (!State)
		{
			return FGalaxyState();
		}
		State->Civs[0].Population = 0.7f;
		State->Civs[1].Population = 0.5f;
		State->Civs[2].Population = 0.995f;
		State->Civs[3].Population = Last;
		Sim->StepOnce();
		return Sim->GetState();
	};

	// The population of people CivIndex in State, or -1 when the board was not laid.
	auto PopulationOf = [](const FGalaxyState& State, int32 CivIndex)
	{
		return State.Civs.IsValidIndex(CivIndex) ? State.Civs[CivIndex].Population : -1.f;
	};
	auto InsightOf = [](const FGalaxyState& State, int32 CivIndex)
	{
		return State.Civs.IsValidIndex(CivIndex) ? State.Civs[CivIndex].IncomePerTick.Insight : -1.f;
	};

	const FGalaxyState Grown = OneTick(0.01f);
	TestEqual(TEXT("A people below 1 grows by the knob"), PopulationOf(Grown, 1), 0.5f + 0.01f);
	TestEqual(TEXT("The people the player rules grows alike"), PopulationOf(Grown, 0), 0.7f + 0.01f);
	TestEqual(TEXT("A people near 1 stops at 1"), PopulationOf(Grown, 2), 1.f);
	TestEqual(TEXT("A people above 1 is left as it is"), PopulationOf(Grown, 3), 1.5f);
	TestEqual(TEXT("A people at 1 stays at 1"), PopulationOf(OneTick(0.01f, 1.f), 3), 1.f);

	const FGalaxyState Still = OneTick(0.f);
	TestEqual(TEXT("A knob of 0 grows nothing"), PopulationOf(Still, 1), 0.5f);
	TestEqual(TEXT("A knob below 0 takes nothing"), PopulationOf(OneTick(-0.01f), 1), 0.5f);
	TestEqual(TEXT("A knob that is not a number grows nothing"), PopulationOf(OneTick(std::numeric_limits<float>::quiet_NaN()), 1), 0.5f);
	TestEqual(TEXT("An infinite knob grows nothing"), PopulationOf(OneTick(std::numeric_limits<float>::infinity()), 1), 0.5f);
	// The regrowth comes before the income: the Insight of the tick follows the grown population.
	TestEqual(TEXT("The income of the tick reads the grown population"), InsightOf(Grown, 1), InsightOf(Still, 1) * (0.51f / 0.5f), 0.0001f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

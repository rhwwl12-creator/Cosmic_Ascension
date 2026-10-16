// Cosmic Ascension - automation tests of settlement, the realm cap, the removal of a civilization without systems and the
// regrowth of the population. Design: guideline.md section 4.1, 4.2, 5.7, 5.12, 7.4, 9.2, 9.3.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
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
	// the answer of Storm (Tier 1, Matter 5, 2).
	UDataTable* ProjectRows(const EAscensionTier* SettleTier)
	{
		UDataTable* Table = NewTable(FMegaprojectDef::StaticStruct());
		if (SettleTier)
		{
			Table->AddRow(CosmicRows::ProjectSettle, Project(*SettleTier, 1.f, 0.f, 3.f, TEXT("settle")));
		}
		Table->AddRow(Answer, Project(EAscensionTier::Tier1_Planetary, 0.f, 5.f, 2.f, TEXT("answer")));
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

	FCosmicDataTables TablesWith(const FCosmicDataTables& Original, const FCosmicRulesDef& Rules, UDataTable* Projects, UDataTable* Hazards)
	{
		UDataTable* RulesTable = NewTable(FCosmicRulesDef::StaticStruct());
		RulesTable->AddRow(CosmicRows::Rules, Rules);
		FCosmicDataTables Tables = Original;
		Tables.Rules = RulesTable;
		Tables.Hazards = Hazards ? Hazards : HazardRows(false);
		Tables.Events = NewTable(FCosmicEventDef::StaticStruct());
		Tables.Transcendents = TranscendentRows();
		Tables.Megaprojects = Projects;
		Tables.Feats = NewTable(FAscensionFeatDef::StaticStruct());
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

	int32 OwnedCount(const FGalaxyState& State, const FCivilizationState& Civ)
	{
		int32 Count = 0;
		for (const FStarSystemState& System : State.Systems)
		{
			Count += System.OwnerCivID == Civ.CivID ? 1 : 0;
		}
		return Count;
	}

	/**
	 * The settlement draws of one tick from From: the tick under Tables against the same tick under Capped (the same
	 * tables with a realm cap of 0 on every tier, so that no people may settle, whatever it owns). No draw of the tick
	 * comes after the AI stage on these boards, so the stream of the first is the stream of the second walked on by the
	 * settlement draws.
	 * Returns that count, or -1 when no count up to 16 matches. OutAfter is the state after the tick under Tables.
	 */
	int32 SettlementDraws(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Tables, const FCosmicDataTables& Capped, const FGalaxyState& From, FGalaxyState& OutAfter)
	{
		Sim.SetTables(Capped);
		Sim.LoadState(From);
		Sim.StepOnce();
		const int32 Without = Sim.GetState().RngState;
		Sim.SetTables(Tables);
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
	FCosmicRulesDef CapNone = Rules;
	CapNone.RealmCapByTier = { 0, 0, 0, 0, 0 };
	const FCosmicDataTables Capped = TablesWith(Original, CapNone, ProjectRows(&Stellar), nullptr);

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
		TestTrue(TEXT("Whether a people may settle does not read the row"), CosmicMegaprojects::MaySettle(Base, Index, Rules, 1));
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
		Hand(Case, 1, 4);
		Case.Systems[4].ActiveMegaproject = CosmicRows::ProjectSettle;
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
		const int32 SeedBefore = Sim->GetRng().GetCurrentSeed();
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		TestFalse(TEXT("A settlement of a system that is no target is refused"), CosmicMegaprojects::Settle(Ctx, 1, 7));
		TestTrue(TEXT("and the system stays wild"), State->Systems[7].OwnerCivID.IsNone());
		TestTrue(TEXT("A settlement of a target starts"), CosmicMegaprojects::Settle(Ctx, 1, 6));
		TestEqual(TEXT("The target becomes the people's"), State->Systems[6].OwnerCivID, State->Civs[1].CivID);
		TestEqual(TEXT("at the end of its systems"), State->Civs[1].OwnedSystems.Last(), State->Systems[6].SystemID);
		TestEqual(TEXT("Its construction slot holds the settlement"), State->Systems[6].ActiveMegaproject, CosmicRows::ProjectSettle);
		TestEqual(TEXT("from 0"), State->Systems[6].MegaprojectProgress, 0.f);
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
		TestFalse(TEXT("The life of the vessel does not take an AI people's settlement"), !Live.Calling.Lives.IsEmpty() && Live.Calling.Lives.Last().MegaprojectsCompleted.Contains(CosmicRows::ProjectSettle));
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

	// ---- The AI's draw: one per people that may settle, before the targets are looked at ----
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the draws is laid"), State))
		{
			return false;
		}
		const FGalaxyState Board0 = *State;
		FGalaxyState After;
		TestEqual(TEXT("One people that may settle: one draw"), SettlementDraws(*Sim, Tables, Capped, Board0, After), 1);
		TestEqual(TEXT("and it settles one of its targets"), SettlingCount(After, After.Civs[1]), 1);
		{
			FGalaxyState Case = Board0;
			Case.Civs[2].Tier = Stellar;
			TestEqual(TEXT("Two peoples that may settle: two draws"), SettlementDraws(*Sim, Tables, Capped, Case, After), 2);
		}
		{
			// People 0 takes 4 and 6: people 1 may settle and has no target.
			FGalaxyState Case = Board0;
			Hand(Case, 0, 4);
			Hand(Case, 0, 6);
			TestEqual(TEXT("A people that may settle draws with no target too"), SettlementDraws(*Sim, Tables, Capped, Case, After), 1);
			TestEqual(TEXT("and settles nothing"), SettlingCount(After, After.Civs[1]), 0);
		}
		{
			FGalaxyState Case = Board0;
			Hand(Case, 1, 1);
			Hand(Case, 1, 9);
			TestEqual(TEXT("A people at its cap draws nothing"), SettlementDraws(*Sim, Tables, Capped, Case, After), 0);
		}
		{
			FGalaxyState Case = Board0;
			Hand(Case, 1, 4);
			Case.Systems[4].ActiveMegaproject = CosmicRows::ProjectSettle;
			TestEqual(TEXT("A people with a settlement under way draws nothing"), SettlementDraws(*Sim, Tables, Capped, Case, After), 0);
		}
		{
			FGalaxyState Case = Board0;
			Case.Civs[0].Tier = Stellar;
			TestEqual(TEXT("The people the player rules draws nothing"), SettlementDraws(*Sim, Tables, Capped, Case, After), 1);
			TestEqual(TEXT("and settles nothing by itself"), SettlingCount(After, After.Civs[0]), 0);
		}
		{
			FGalaxyState Case = Board0;
			Case.Civs[2].Tier = Stellar;
			Case.Systems[8].OwnerCivID = NAME_None;
			Case.Civs[2].OwnedSystems.Reset();
			TestEqual(TEXT("A people without a system draws nothing"), SettlementDraws(*Sim, Tables, Capped, Case, After), 1);
		}
		{
			const FCosmicDataTables NoRow = TablesWith(Original, Rules, ProjectRows(nullptr), nullptr);
			const FCosmicDataTables NoRowCapped = TablesWith(Original, CapNone, ProjectRows(nullptr), nullptr);
			TestEqual(TEXT("Without the settle row the draws are the same"), SettlementDraws(*Sim, NoRow, NoRowCapped, Board0, After), 1);
			TestEqual(TEXT("and nothing is settled"), SettlingCount(After, After.Civs[1]), 0);
			const FCosmicDataTables HighRow = TablesWith(Original, Rules, ProjectRows(&Causal), nullptr);
			const FCosmicDataTables HighRowCapped = TablesWith(Original, CapNone, ProjectRows(&Causal), nullptr);
			TestEqual(TEXT("Under a row of a higher Tier the draws are the same"), SettlementDraws(*Sim, HighRow, HighRowCapped, Board0, After), 1);
			TestEqual(TEXT("and nothing is settled either"), SettlingCount(After, After.Civs[1]), 0);
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

		Sim->LoadState(Ruling);
		FGalaxyState& Capped1 = Sim->GetMutableState();
		Capped1.Civs[0].Tier = EAscensionTier::Tier1_Planetary;
		TestFalse(TEXT("The ruler's people at its cap does not settle"), PlayerRole->StartMegaproject(Three, CosmicRows::ProjectSettle));
		TestFalse(TEXT("and the key has nothing to start"), PlayerRole->StartNextMegaproject());
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
	// system over, so every system a people gains comes from a settlement), and no people settles two at once.
	{
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		const FGalaxyState& State = Sim->GetState();
		const FCosmicRulesDef& GameRules = Sim->GetRules();
		int64 OverCapTick = 0;
		int64 TwoAtOnceTick = 0;
		int32 MostSystems = 0;
		int32 Settlements = 0;
		for (int32 Step = 0; Step < 20000; ++Step)
		{
			Sim->StepOnce();
			for (const FCivilizationState& Civ : State.Civs)
			{
				const int32 Owned = OwnedCount(State, Civ);
				MostSystems = FMath::Max(MostSystems, Owned);
				if (OverCapTick == 0 && Owned > CosmicMegaprojects::RealmCapOf(GameRules, Civ.Tier))
				{
					OverCapTick = State.Tick;
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
	const FCosmicDataTables Tables = TablesWith(Original, Rules, ProjectRows(&Stellar), HazardRows(false));

	// Peoples 2 and 3 lose everything between ticks: 2 by hand, 3 because the system it lists is people 1's.
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
		State->Knowing.PendingContactCivID = Two;
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
		TestTrue(TEXT("A pending contact with a removed people is cleared"), State->Knowing.PendingContactCivID.IsNone());
		TestEqual(TEXT("The index finds the people that stays at its new place"), Sim->GetStateIndex().FindCiv(*State, One), 1);
		TestEqual(TEXT("and no longer finds a removed people"), Sim->GetStateIndex().FindCiv(*State, Two), static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("nor the other"), Sim->GetStateIndex().FindCiv(*State, Three), static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("A removal tells no line"), State->Log.FilterByPredicate([State](const FCosmicLogEntry& Each) { return Each.Tick == State->Tick; }).Num(), 0);
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

	// The people the player rules loses its last system: it stays for the tick of the loss, the role-effect stage lets go
	// of it, and the next tick's cleanup takes it.
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the ruler's people is laid"), State))
		{
			return false;
		}
		const FName Ruled = State->Civs[0].CivID;
		State->Systems[0].ActiveHazard = Tag(TEXT("Hazard.PreSupernova"));
		State->Systems[0].HazardTicksRemaining = 1;
		Sim->StepOnce();
		TestTrue(TEXT("The last system of the ruler's people went"), CosmicHazards::IsDeadNode(State->Systems[0]));
		TestTrue(TEXT("The ruler fell"), State->Player.Role == EPlayerRole::Wanderer);
		const int32 Kept = Sim->GetStateIndex().FindCiv(*State, Ruled);
		TestTrue(TEXT("The people the player ruled in the tick of the loss stays for that tick"), Kept != INDEX_NONE);
		TestTrue(TEXT("and no longer has the player"), State->Civs.IsValidIndex(Kept) && !State->Civs[Kept].bIsPlayerRuled);
		Sim->StepOnce();
		TestEqual(TEXT("The next tick's cleanup takes it"), Sim->GetStateIndex().FindCiv(*State, Ruled), static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("and only it"), State->Civs.Num(), 3);
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

	// The board with people 1 at 0.5, people 2 just under 1, people 3 above 1; then one tick under Knob.
	auto OneTick = [&](float Knob) -> FGalaxyState
	{
		FCosmicRulesDef With = Rules;
		With.PopulationRegrowthPerTick = Knob;
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, With, ProjectRows(&Stellar), nullptr));
		if (!State)
		{
			return FGalaxyState();
		}
		State->Civs[1].Population = 0.5f;
		State->Civs[2].Population = 0.995f;
		State->Civs[3].Population = 1.5f;
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
	TestEqual(TEXT("A people near 1 stops at 1"), PopulationOf(Grown, 2), 1.f);
	TestEqual(TEXT("A people above 1 is left as it is"), PopulationOf(Grown, 3), 1.5f);
	TestEqual(TEXT("A people at 1 stays at 1"), PopulationOf(Grown, 0), 1.f);

	const FGalaxyState Still = OneTick(0.f);
	TestEqual(TEXT("A knob of 0 grows nothing"), PopulationOf(Still, 1), 0.5f);
	TestEqual(TEXT("A knob below 0 takes nothing"), PopulationOf(OneTick(-0.01f), 1), 0.5f);
	TestEqual(TEXT("A knob that is not a number grows nothing"), PopulationOf(OneTick(std::numeric_limits<float>::quiet_NaN()), 1), 0.5f);
	// The regrowth comes before the income: the Insight of the tick follows the grown population.
	TestEqual(TEXT("The income of the tick reads the grown population"), InsightOf(Grown, 1), InsightOf(Still, 1) * (0.51f / 0.5f), 0.0001f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

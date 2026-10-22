// Cosmic Ascension - automation tests of the first crisis, the recovery of Stability, the AI's claim relief and the second
// T2->T3 feat (MVP item 3).
// Design: docs/60_DECISIONS/headers/A3_first_ten_minutes.md; guideline.md section 2.4, 4.1, 4.3, 16 #68 #71 #91.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicIDs.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicNextRung.h"
#include "UI/CosmicUILabels.h"

#include <limits>

namespace CosmicWorldMendTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	const FName Awakening(TEXT("crisis_ai_awakening"));
	const FName Rebellion(TEXT("crisis_rebellion"));
	const FName Claim(TEXT("crisis_claim"));
	const FName Plague(TEXT("crisis_plague"));

	// A copy of a row of the game's DT_Hazards with no chance of its own: a test sees only what it places. False when the
	// game lacks the row.
	bool CopyHazard(const FCosmicDataTables& Original, UDataTable* Into, FName RowName)
	{
		const FHazardDef* Row = Original.FindHazard(RowName);
		if (!Row)
		{
			return false;
		}
		FHazardDef Copy = *Row;
		Copy.SpawnChancePerTick = 0.f;
		Into->AddRow(RowName, Copy);
		return true;
	}

	// The game's rules with no Lottery chance and no drain, and the knobs of this file set by the caller.
	FCosmicRulesDef Quiet(const FCosmicRulesDef& BaseRules)
	{
		FCosmicRulesDef Rules = BaseRules;
		Rules.EventChancePerSlot = 0.f;
		Rules.SpiritualPressureStabilityDrain = 0.f;
		return Rules;
	}

	FCosmicDataTables TablesWith(const FCosmicDataTables& Original, const FCosmicRulesDef& Rules, UDataTable* Hazards)
	{
		FCosmicDataTables Tables = CosmicTests::WithRules(Original, Rules);
		Tables.Hazards = Hazards;
		return Tables;
	}

	int32 CapitalOf(const UGalaxySimulationSubsystem& Sim, int32 CivIndex)
	{
		const FGalaxyState& State = Sim.GetState();
		return Sim.GetStateIndex().FindSystem(State, State.Civs[CivIndex].HomeSystemID);
	}
}

// The first crisis (section 2.4, 16 #91): at FirstCrisisTick every people's capital receives the Draw row of the tag, a
// busy slot keeps its own hazard, nothing is drawn for it, and the ruler's card comes in the same tick.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimFirstCrisisTest, "Cosmic_Ascension.Sim.FirstCrisis", CosmicTests::Flags)
bool FCosmicSimFirstCrisisTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWorldMendTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	TestEqual(TEXT("The game's first crisis comes at tick 150"), BaseRules.FirstCrisisTick, 150);
	TestEqual(TEXT("and it is the machines' awakening"), BaseRules.FirstCrisisHazard, Tag(TEXT("Hazard.AIAwakening")));

	UDataTable* Hazards = CosmicTests::NewTable(FHazardDef::StaticStruct());
	if (!TestTrue(TEXT("The game has the awakening, the plague and the rebellion"),
		CopyHazard(Original, Hazards, Awakening) && CopyHazard(Original, Hazards, Plague) && CopyHazard(Original, Hazards, Rebellion)))
	{
		return false;
	}
	const FGameplayTag AwakeningTag = Tag(TEXT("Hazard.AIAwakening"));
	const FGameplayTag PlagueTag = Tag(TEXT("Hazard.Plague"));
	const int32 Countdown = Original.FindHazard(Awakening)->CountdownTicks;

	// Runs a board of four peoples to tick 149 with the rules given, the waiting card taken away, and a plague on the
	// capital of people 2 when bBusy. Returns the live state.
	auto RunTo149 = [&](const FCosmicRulesDef& Rules, bool bBusy) -> FGalaxyState&
	{
		Sim->SetTables(TablesWith(Original, Rules, Hazards));
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		Sim->RunHeadless(149);
		State.Knowing.PendingPetitionRow = NAME_None;
		State.Knowing.PendingPetitionSystemID = NAME_None;
		if (bBusy)
		{
			FStarSystemState& Busy = State.Systems[CapitalOf(*Sim, 2)];
			Busy.ActiveHazard = PlagueTag;
			Busy.HazardTicksRemaining = 50;
		}
		return State;
	};

	// The game's knobs: every capital at tick 150, nothing before, the stream as if nothing were placed.
	{
		FCosmicRulesDef Off = Quiet(BaseRules);
		Off.FirstCrisisTick = 0;
		FGalaxyState& Control = RunTo149(Off, true);
		Sim->StepOnce();
		const int32 ControlStream = Control.RngState;

		FGalaxyState& State = RunTo149(Quiet(BaseRules), true);
		int32 Before = 0;
		for (const FStarSystemState& System : State.Systems)
		{
			Before += System.ActiveHazard.IsValid() ? 1 : 0;
		}
		TestEqual(TEXT("Before tick 150 only the plague of the test stands"), Before, 1);
		Sim->StepOnce();
		TestEqual(TEXT("The step ran tick 150"), State.Tick, static_cast<int64>(150));
		TestEqual(TEXT("The first crisis draws nothing from the sim stream"), State.RngState, ControlStream);
		for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
		{
			const FStarSystemState& Capital = State.Systems[CapitalOf(*Sim, CivIndex)];
			if (CivIndex == 2)
			{
				TestEqual(TEXT("A capital whose slot holds a hazard keeps it"), Capital.ActiveHazard, PlagueTag);
				TestEqual(TEXT("and its count runs on"), Capital.HazardTicksRemaining, 49);
				continue;
			}
			TestEqual(*FString::Printf(TEXT("People %d's capital holds the awakening"), CivIndex), Capital.ActiveHazard, AwakeningTag);
			TestEqual(*FString::Printf(TEXT("with its whole count (placed after the countdown) - people %d"), CivIndex), Capital.HazardTicksRemaining, Countdown);
		}
		int32 Elsewhere = 0;
		for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num(); ++SystemIndex)
		{
			const bool bCapital = State.Civs.ContainsByPredicate([&](const FCivilizationState& Civ) { return Civ.HomeSystemID == State.Systems[SystemIndex].SystemID; });
			Elsewhere += (!bCapital && State.Systems[SystemIndex].ActiveHazard.IsValid()) ? 1 : 0;
		}
		TestEqual(TEXT("No other system holds a hazard"), Elsewhere, 0);
		const int32 RuledCapital = CapitalOf(*Sim, 0);
		TestTrue(TEXT("The outbreak on the ruled people's capital is told in that tick"), State.Log.ContainsByPredicate([&](const FCosmicLogEntry& Entry)
		{
			return Entry.Tick == 150 && Entry.SourceRow == Awakening && Entry.FocusSystemID == State.Systems[RuledCapital].SystemID;
		}));
		TestEqual(TEXT("and the ruler's card on it comes in the same tick"), State.Knowing.PendingPetitionRow, FName(TEXT("petition_awakening")));
		Sim->StepOnce();
		int32 Again = 0;
		for (const FStarSystemState& System : State.Systems)
		{
			Again += (System.ActiveHazard == AwakeningTag && System.HazardTicksRemaining == Countdown) ? 1 : 0;
		}
		TestEqual(TEXT("Tick 151 places nothing new"), Again, 0);
		// A capital freed after the first crisis is not given another.
		FStarSystemState& Freed = State.Systems[CapitalOf(*Sim, 1)];
		Freed.ActiveHazard = FGameplayTag();
		Freed.HazardTicksRemaining = 0;
		Sim->StepOnce();
		TestFalse(TEXT("A capital freed at tick 151 stays free at tick 152"), Freed.ActiveHazard.IsValid());
	}

	// A capital another people holds is not the first people's to receive: its new owner's own capital is.
	{
		FGalaxyState& State = RunTo149(Quiet(BaseRules), false);
		const int32 Lost = CapitalOf(*Sim, 3);
		FStarSystemState& System = State.Systems[Lost];
		State.Civs[3].OwnedSystems.Remove(System.SystemID);
		System.OwnerCivID = State.Civs[2].CivID;
		State.Civs[2].OwnedSystems.Add(System.SystemID);
		Sim->StepOnce();
		TestFalse(TEXT("A capital another people holds receives no first crisis"), System.ActiveHazard.IsValid());
		TestEqual(TEXT("while its holder's own capital does"), State.Systems[CapitalOf(*Sim, 2)].ActiveHazard, AwakeningTag);
	}

	// FirstCrisisTick 0: none.
	{
		FCosmicRulesDef Rules = Quiet(BaseRules);
		Rules.FirstCrisisTick = 0;
		FGalaxyState& State = RunTo149(Rules, false);
		Sim->RunHeadless(2);
		int32 Placed = 0;
		for (const FStarSystemState& System : State.Systems)
		{
			Placed += System.ActiveHazard.IsValid() ? 1 : 0;
		}
		TestEqual(TEXT("With the knob at 0 no first crisis comes"), Placed, 0);
	}

	// The tag of a row that is not a Draw row: an error line and nothing placed.
	{
		FCosmicRulesDef Rules = Quiet(BaseRules);
		Rules.FirstCrisisHazard = Tag(TEXT("Hazard.Rebellion"));
		FGalaxyState& State = RunTo149(Rules, false);
		AddExpectedError(TEXT("is not the tag of a Draw row"), EAutomationExpectedErrorFlags::Contains, 1);
		Sim->StepOnce();
		int32 Placed = 0;
		for (const FStarSystemState& System : State.Systems)
		{
			Placed += System.ActiveHazard.IsValid() ? 1 : 0;
		}
		TestEqual(TEXT("The capital rule's row is not placed as the first crisis"), Placed, 0);
	}
	return true;
}

// The recovery of Stability (section 4.1, 16 #68): below the resting level a people rises by the knob and stops at the
// level, above it nothing changes, a knob that is not a number gives none, the same tick's count of discontent sees it,
// and the resolution of a rebellion raises Stability by the game's row.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimStabilityRecoveryTest, "Cosmic_Ascension.Sim.StabilityRecovery", CosmicTests::Flags)
bool FCosmicSimStabilityRecoveryTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWorldMendTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	TestEqual(TEXT("The game's recovery is 0.001 a tick"), BaseRules.StabilityRecoveryPerTick, 0.001f);
	TestEqual(TEXT("toward 0.6"), BaseRules.StabilityRestingLevel, 0.6f);

	UDataTable* NoHazards = CosmicTests::NewTable(FHazardDef::StaticStruct());
	auto Lay = [&](float Rate, float Level) -> FGalaxyState&
	{
		FCosmicRulesDef Rules = Quiet(BaseRules);
		Rules.StabilityRecoveryPerTick = Rate;
		Rules.StabilityRestingLevel = Level;
		Rules.FirstCrisisTick = 0;
		Sim->SetTables(CosmicTests::WithoutProjects(TablesWith(Original, Rules, NoHazards)));
		Sim->NewGalaxy(42, 12, 4);
		return Sim->GetMutableState();
	};

	{
		FGalaxyState& State = Lay(0.001f, 0.6f);
		State.Civs[0].Stability = 0.3f;
		State.Civs[1].Stability = 0.5995f;
		State.Civs[2].Stability = 0.9f;
		State.Civs[3].Stability = 0.2995f;
		State.Civs[3].LowStabilityTicks = 5;
		Sim->StepOnce();
		TestEqual(TEXT("Below the level a people rises by the knob"), State.Civs[0].Stability, 0.3f + 0.001f, 1.0e-6f);
		TestEqual(TEXT("and stops at the level"), State.Civs[1].Stability, 0.6f);
		TestEqual(TEXT("Above the level nothing changes"), State.Civs[2].Stability, 0.9f);
		TestTrue(TEXT("The rise is counted in the same tick: out of discontent at 0.3"), State.Civs[3].Stability >= 0.3f);
		TestEqual(TEXT("so the count of discontent starts over in that tick"), State.Civs[3].LowStabilityTicks, 0);
		Sim->RunHeadless(400);
		TestEqual(TEXT("Recovery ends at the level"), State.Civs[0].Stability, 0.6f);
	}

	const float NaN = std::numeric_limits<float>::quiet_NaN();
	const struct { float Rate; float Level; const TCHAR* What; } Off[] = {
		{ 0.f, 0.6f, TEXT("a rate of 0") },
		{ -0.001f, 0.6f, TEXT("a rate below 0") },
		{ NaN, 0.6f, TEXT("a rate that is not a number") },
		{ 1.5f, 0.6f, TEXT("a rate above 1") },
		{ 0.001f, NaN, TEXT("a level that is not a number") },
		{ 0.001f, 1.5f, TEXT("a level above 1") },
	};
	for (const auto& Case : Off)
	{
		FGalaxyState& State = Lay(Case.Rate, Case.Level);
		State.Civs[0].Stability = 0.3f;
		Sim->StepOnce();
		TestEqual(*FString::Printf(TEXT("No recovery with %s"), Case.What), State.Civs[0].Stability, 0.3f);
	}

	// The game's rebellion row: its resolution raises Stability by 0.1 (R, W and the AI resolve through Resolve).
	{
		const FHazardDef* Row = Original.FindHazard(Rebellion);
		if (TestNotNull(TEXT("The game has its rebellion row"), Row))
		{
			UDataTable* Rebellions = CosmicTests::NewTable(FHazardDef::StaticStruct());
			Rebellions->AddRow(Rebellion, *Row);
			FCosmicRulesDef Rules = Quiet(BaseRules);
			Rules.StabilityRecoveryPerTick = 0.f;
			Rules.FirstCrisisTick = 0;
			Sim->SetTables(CosmicTests::WithoutProjects(TablesWith(Original, Rules, Rebellions)));
			Sim->NewGalaxy(42, 12, 4);
			FGalaxyState& State = Sim->GetMutableState();
			const int32 Capital = CapitalOf(*Sim, 1);
			State.Systems[Capital].ActiveHazard = Row->HazardTag;
			State.Systems[Capital].HazardTicksRemaining = 100;
			State.Civs[1].Stability = 0.1f;
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			TestTrue(TEXT("The rebellion is resolved"), CosmicHazards::Resolve(Ctx, Capital));
			TestEqual(TEXT("and its people's Stability rises by 0.1"), State.Civs[1].Stability, 0.2f, 1.0e-6f);
		}
	}
	return true;
}

// The AI's claim relief (16 #71): a people whose profile leaves claims alone does not start the claim's answer, the other
// profiles (an empty one too) do, every other answer is started whatever the profile, and the ruler's project key does not
// read the knob.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimClaimReliefTest, "Cosmic_Ascension.Sim.ClaimRelief", CosmicTests::Flags)
bool FCosmicSimClaimReliefTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWorldMendTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	TestTrue(TEXT("The game's expansionist peoples leave claims alone"), BaseRules.ClaimLeaveProfiles.HasTagExact(Tag(TEXT("AI.Profile.Expansionist"))));
	TestTrue(TEXT("and so do the militarist ones"), BaseRules.ClaimLeaveProfiles.HasTagExact(Tag(TEXT("AI.Profile.Militarist"))));
	TestEqual(TEXT("and no other"), BaseRules.ClaimLeaveProfiles.Num(), 2);

	UDataTable* Hazards = CosmicTests::NewTable(FHazardDef::StaticStruct());
	if (!TestTrue(TEXT("The game has the claim and the rebellion"), CopyHazard(Original, Hazards, Claim) && CopyHazard(Original, Hazards, Rebellion)))
	{
		return false;
	}
	FCosmicRulesDef Rules = Quiet(BaseRules);
	Rules.FirstCrisisTick = 0;
	Rules.StabilityRecoveryPerTick = 0.f;
	Rules.RealmCapByTier = { 1, 1, 1, 1, 1 };
	Sim->SetTables(TablesWith(Original, Rules, Hazards));
	Sim->NewGalaxy(42, 14, 7);
	FGalaxyState& State = Sim->GetMutableState();
	if (!TestTrue(TEXT("The board has seven peoples"), State.Civs.Num() >= 7))
	{
		return false;
	}
	const FGameplayTag ClaimTag = Tag(TEXT("Hazard.Claim"));
	const FGameplayTag RebellionTag = Original.FindHazard(Rebellion)->HazardTag;
	const FGameplayTag Expansionist = Tag(TEXT("AI.Profile.Expansionist"));
	// People 0 is the ruled one, its profile one that leaves claims alone: the knob must not reach it. People 6 is
	// expansionist with a rebellion in its capital instead of a claim.
	const FGameplayTag Profiles[] = { Expansionist, Expansionist, Tag(TEXT("AI.Profile.Conservative")), Tag(TEXT("AI.Profile.Militarist")),
		Tag(TEXT("AI.Profile.Scholarly")), FGameplayTag(), Expansionist };
	for (int32 CivIndex = 0; CivIndex < 7; ++CivIndex)
	{
		FCivilizationState& Civ = State.Civs[CivIndex];
		Civ.AIProfile = Profiles[CivIndex];
		Civ.Stockpile.Energy = Civ.Stockpile.Matter = Civ.Stockpile.Influence = 1000.f;
		FStarSystemState& Capital = State.Systems[CapitalOf(*Sim, CivIndex)];
		Capital.ActiveHazard = CivIndex == 6 ? RebellionTag : ClaimTag;
		Capital.HazardTicksRemaining = 200;
	}
	Sim->StepOnce();
	const FName Relief(TEXT("project_relief"));
	const FName Reform(TEXT("project_reform"));
	auto AnswerOf = [&](int32 CivIndex) { return State.Systems[CapitalOf(*Sim, CivIndex)].ResponseMegaproject; };
	TestEqual(TEXT("An expansionist people leaves its claim alone"), AnswerOf(1), FName());
	TestEqual(TEXT("A conservative people relieves it"), AnswerOf(2), Relief);
	TestEqual(TEXT("A militarist people leaves its claim alone"), AnswerOf(3), FName());
	TestEqual(TEXT("A scholarly people relieves it"), AnswerOf(4), Relief);
	TestEqual(TEXT("A people with no profile relieves it"), AnswerOf(5), Relief);
	TestEqual(TEXT("An expansionist people answers its rebellion"), AnswerOf(6), Reform);
	TestEqual(TEXT("and one that leaves its claim alone still builds what its next feat asks"), State.Systems[CapitalOf(*Sim, 1)].ActiveMegaproject, FName(TEXT("project_dyson")));
	TestEqual(TEXT("The ruled people starts nothing by itself"), AnswerOf(0), FName());
	TestTrue(TEXT("The ruler's project key starts the relief though the people's profile leaves claims alone"), Role->StartNextMegaproject());
	TestEqual(TEXT("on the ruled people's claim"), AnswerOf(0), Relief);
	return true;
}

// The second T2->T3 feat (MVP item 3, 16 #72): three settled systems and Engineering 1.0 lift a people of the second rung,
// two systems or a share below 1.0 do not, a system still being settled does not count, and the top bar's next rung shows
// both ways up of the second rung.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSecondFeatTest, "Cosmic_Ascension.Sim.SecondFeat", CosmicTests::Flags)
bool FCosmicSimSecondFeatTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWorldMendTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	const FName Worlds(TEXT("feat_three_worlds"));
	const FName Nova(TEXT("feat_supernova_stabilized"));
	const FName Engineering(TEXT("Engineering"));
	const FAscensionFeatDef* Row = Original.FindFeat(Worlds);
	if (!TestNotNull(TEXT("The game has the second T2->T3 feat"), Row))
	{
		return false;
	}
	TestTrue(TEXT("It lifts a ruler's people from the second rung to the third"), Row->Path == EPlayerRole::Ruler && Row->RequiredTier == EAscensionTier::Tier2_Stellar
		&& Row->TargetTier == EAscensionTier::Tier3_Causal);
	TestTrue(TEXT("on three settled systems and Engineering 1.0"), Row->CompletionConditions.Num() == 2
		&& Row->CompletionConditions[0].Type == EConditionType::CivSettledSystemsAtLeast && Row->CompletionConditions[0].Value == 3.f
		&& Row->CompletionConditions[1].Type == EConditionType::CivResearchAtLeast && Row->CompletionConditions[1].Key == Engineering
		&& Row->CompletionConditions[1].Value == 1.f);

	FCosmicRulesDef Rules = Quiet(BaseRules);
	Rules.FirstCrisisTick = 0;
	Rules.StabilityRecoveryPerTick = 0.f;
	UDataTable* NoHazards = CosmicTests::NewTable(FHazardDef::StaticStruct());
	// A people of the second rung holding Settled systems (its home first, then wild ones), the last of them still being
	// settled when bLastSettling, at an Engineering share. People 1 is an AI people; people 0 the ruled one.
	auto Lay = [&](int32 CivIndex, int32 Systems, float Share, bool bLastSettling) -> FGalaxyState&
	{
		Sim->SetTables(TablesWith(Original, Rules, NoHazards));
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		FCivilizationState& Civ = State.Civs[CivIndex];
		Civ.Tier = EAscensionTier::Tier2_Stellar;
		Civ.Research.Add(Engineering, Share);
		int32 Given = 1;
		for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num() && Given < Systems; ++SystemIndex)
		{
			FStarSystemState& System = State.Systems[SystemIndex];
			if (System.OwnerCivID.IsNone() && !CosmicState::IsSanctuary(System.SystemID))
			{
				System.OwnerCivID = Civ.CivID;
				Civ.OwnedSystems.Add(System.SystemID);
				++Given;
				if (bLastSettling && Given == Systems)
				{
					System.ActiveMegaproject = CosmicRows::ProjectSettle;
				}
			}
		}
		return State;
	};

	{
		FGalaxyState& State = Lay(1, 3, 1.f, false);
		Sim->StepOnce();
		TestEqual(TEXT("Three settled systems and Engineering 1.0 lift the people to the third rung"), State.Civs[1].Tier, EAscensionTier::Tier3_Causal);
		TestTrue(TEXT("by the second feat's line"), State.Log.ContainsByPredicate([&](const FCosmicLogEntry& Entry) { return Entry.Tick == State.Tick && Entry.SourceRow == Worlds; }));
	}
	{
		FGalaxyState& State = Lay(1, 2, 1.f, false);
		Sim->StepOnce();
		TestEqual(TEXT("Two settled systems do not"), State.Civs[1].Tier, EAscensionTier::Tier2_Stellar);
	}
	{
		FGalaxyState& State = Lay(1, 3, 0.99f, false);
		Sim->StepOnce();
		TestEqual(TEXT("Engineering 0.99 does not"), State.Civs[1].Tier, EAscensionTier::Tier2_Stellar);
	}
	{
		FGalaxyState& State = Lay(1, 3, 1.f, true);
		Sim->StepOnce();
		TestEqual(TEXT("A third system still being settled does not count"), State.Civs[1].Tier, EAscensionTier::Tier2_Stellar);
	}

	// The next rung of the ruled people on the second rung: both ways up, the judgment's order, and the settled systems.
	{
		FGalaxyState& State = Lay(0, 2, 0.4f, false);
		const TArray<FCosmicNextRung> Rungs = CosmicNextRung::AllOf(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
		if (TestEqual(TEXT("The second rung has two ways up"), Rungs.Num(), 2))
		{
			TestEqual(TEXT("the stabilized star first (the judgment's order)"), Rungs[0].FeatRow, Nova);
			TestEqual(TEXT("then the second feat"), Rungs[1].FeatRow, Worlds);
			TestEqual(TEXT("Of is the first of them"), CosmicNextRung::Of(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules()).FeatRow, Nova);
			const FCosmicRungPart* Realm = Rungs[1].Parts.FindByPredicate([](const FCosmicRungPart& Part) { return Part.Kind == ECosmicRungPartKind::Realm; });
			if (TestNotNull(TEXT("The second feat shows its settled systems"), Realm))
			{
				TestEqual(TEXT("two held"), Realm->Progress, 2.f);
				TestEqual(TEXT("of three asked"), Realm->Needed, 3.f);
				TestFalse(TEXT("not met"), Realm->bMet);
			}
			const FString Text = CosmicNextRung::TextOfAll(Rungs, Sim->GetTables()).ToString();
			TestTrue(TEXT("The tooltip says either way does"), Text.StartsWith(CosmicUI::Label(TEXT("Rung.NextAny")).ToString()));
			TestTrue(TEXT("names both feats"), Text.Contains(Original.FindFeat(Nova)->Title.ToString()) && Text.Contains(Row->Title.ToString()));
			TestTrue(TEXT("and shows the systems held against those asked"),
				Text.Contains(FText::Format(CosmicUI::Label(TEXT("Rung.Realm")), CosmicUI::WholeNumber(2.f), CosmicUI::WholeNumber(3.f)).ToString()));
			TestFalse(TEXT("leaving no slot unfilled"), Text.Contains(TEXT("{")));
			TestEqual(TEXT("One way up reads as before"), CosmicNextRung::TextOfAll({ Rungs[0] }, Sim->GetTables()).ToString(), CosmicNextRung::TextOf(Rungs[0], Sim->GetTables()).ToString());
		}
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

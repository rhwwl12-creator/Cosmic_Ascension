// Cosmic Ascension - automation tests of the system panel's two warning lines (milestone B item B6a, commit [B-6a4]): the
// claim that may follow a hazard's expiry (CosmicHazards::ClaimRuleHoldsOf - the claim rule the crisis stage calls - and
// ClaimMayFollowOf - the screen's answer) and the rebellion watch on the capital (CosmicHazards::RebellionWatchOf and the
// threshold the economy stage counts with).
// Design: docs/60_DECISIONS/headers/B6a_endings_and_the_run_past_the_third_rung.md section 4, 6 and decision 14.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicUILabels.h"
#include "UObject/NameTypes.h"
#include "World/CosmicGameMode.h"

#include <initializer_list>

namespace CosmicWarningLineTests
{
	FGameplayTag WarnTag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	FString WarnWords(const FText& Text)
	{
		return Text.ToString();
	}

	FString WarnWords(const TCHAR* Key)
	{
		return CosmicUI::Label(Key).ToString();
	}

	// The rows of these tests.
	const FName PlagueRow(TEXT("crisis_test_warn_plague"));
	const FName ClaimRow(TEXT("crisis_test_warn_claim"));
	const FName NovaRow(TEXT("crisis_test_warn_nova"));
	const FName WideFallen(TEXT("tr_test_warn_wide"));      // a sphere of twelve
	const FName NarrowFallen(TEXT("tr_test_warn_narrow"));  // a sphere of one
	const FName AnswerEdict(TEXT("edict_test_warn_answer"));

	// The systems of the board (LayWarnBoard), by index into State.Systems.
	constexpr int32 VesselSystem = 0;   // people 0, settled - the ruler stands here
	constexpr int32 WarnedSystem = 2;   // people 0, settled, one lane from the sphere - the hazard of each case stands here
	constexpr int32 SphereSystem = 3;   // nobody's - the fallen's sphere
	constexpr int32 LaterSystem = 4;    // people 0, settled, one lane from the sphere
	constexpr int32 FarSystem = 9;      // nobody's, far from people 0's systems - a sphere out of reach

	// A hazard row of the tests: no spawn chance, three lines for each moment.
	FHazardDef WarnHazardRowOf(const TCHAR* TagName, ECosmicHazardSpawn Rule, int32 Countdown)
	{
		auto Lines = [TagName](const TCHAR* Moment)
		{
			const FString Stem = FString::Printf(TEXT("%s %s"), TagName, Moment);
			return TArray<FText>{ FText::AsCultureInvariant(Stem + TEXT(" one")), FText::AsCultureInvariant(Stem + TEXT(" two")), FText::AsCultureInvariant(Stem + TEXT(" three")) };
		};
		FHazardDef Row;
		Row.HazardTag = WarnTag(TagName);
		Row.DisplayName = FText::AsCultureInvariant(FString::Printf(TEXT("<%s>"), TagName));
		Row.CountdownTicks = Countdown;
		Row.SpawnRule = Rule;
		Row.SpawnChancePerTick = 0.f;
		Row.LogCategory = WarnTag(TEXT("Log.Crisis"));
		Row.Severity = 1;
		Row.OutbreakTemplates = Lines(TEXT("outbreak"));
		Row.ExpireTemplates = Lines(TEXT("expiry"));
		Row.ResolveTemplates = Lines(TEXT("resolution"));
		return Row;
	}

	// A fallen row of tier 4 with a sphere of MaxSphere; its weight 0, so the roster of NewGalaxy makes no instance of it.
	FTranscendentDef WarnFallenRowOf(int32 MaxSphere)
	{
		FTranscendentDef Row;
		Row.Doctrine = WarnTag(TEXT("Doctrine.Silence"));
		Row.Allegiance = WarnTag(TEXT("Realm.Abyssal"));
		Row.Tier = EAscensionTier::Tier4_Transcendent;
		Row.Depth = 0.f;
		Row.MaxInfluencedSystems = MaxSphere;
		Row.SpawnWeight = 0.f;
		return Row;
	}

	// The game's tables made quiet for the board (CosmicAwarenessTests' form): no inherited edict, no event slot, no first
	// crisis, no periodic petition, no drift and no fleet, no pressure drain, a realm cap of one, claims one lane out, no
	// project or feat, no event or card row. The hazards: a plague of the tests (one tick; AnswerEdict answers it), the claim
	// row (300 ticks) and a supernova of the tests (one tick). The transcendents: WideFallen and NarrowFallen. The edicts: the
	// game's and AnswerEdict (no upkeep).
	FCosmicDataTables WarnTablesOf(const FCosmicDataTables& Original, const FCosmicRulesDef& GameRules)
	{
		FCosmicRulesDef Rules = CosmicTests::WithoutStartingEdict(GameRules);
		Rules.EventChancePerSlot = 0.f;
		Rules.FirstCrisisTick = 0;
		Rules.PetitionPeriodTicks = 100000;
		Rules.RealmCapByTier = { 1, 1, 1, 1, 1 };
		Rules.RelationDriftRate = 0.f;
		Rules.FleetPerSettledByTier = { 0.f, 0.f, 0.f, 0.f, 0.f };
		Rules.SpiritualPressureStabilityDrain = 0.f;
		Rules.ClaimAdjacencyLanes = 1;
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));

		UDataTable* Hazards = CosmicTests::NewTable(FHazardDef::StaticStruct());
		FHazardDef Plague = WarnHazardRowOf(TEXT("Hazard.Plague"), ECosmicHazardSpawn::Draw, 1);
		Plague.RulerResolutions = { AnswerEdict };
		Hazards->AddRow(PlagueRow, Plague);
		Hazards->AddRow(ClaimRow, WarnHazardRowOf(TEXT("Hazard.Claim"), ECosmicHazardSpawn::ClaimRule, 300));
		Hazards->AddRow(NovaRow, WarnHazardRowOf(TEXT("Hazard.PreSupernova"), ECosmicHazardSpawn::Draw, 1));
		Tables.Hazards = Hazards;

		UDataTable* Transcendents = CosmicTests::NewTable(FTranscendentDef::StaticStruct());
		Transcendents->AddRow(WideFallen, WarnFallenRowOf(12));
		Transcendents->AddRow(NarrowFallen, WarnFallenRowOf(1));
		Tables.Transcendents = Transcendents;

		UDataTable* Edicts = CosmicTests::CopyWithout<FEdictDef>(Original.Edicts, {});
		if (Edicts)
		{
			FEdictDef Answer;
			Answer.DisplayName = FText::AsCultureInvariant(TEXT("<answer>"));
			Edicts->AddRow(AnswerEdict, Answer);
			Tables.Edicts = Edicts;
		}

		Tables.Events = CosmicTests::NewTable(FCosmicEventDef::StaticStruct());
		Tables.Encounters = CosmicTests::NewTable(FEncounterDef::StaticStruct());
		return Tables;
	}

	// Hazard (a tag of the tests' rows) on System, running out at the next tick.
	void StandOn(FGalaxyState& State, int32 System, const TCHAR* Hazard)
	{
		State.Systems[System].ActiveHazard = WarnTag(Hazard);
		State.Systems[System].HazardTicksRemaining = 1;
	}

	// The board on the live state (CosmicAwarenessTests' board): a galaxy of seed 42 with twelve living systems wired as
	// CosmicTests::WireChain (0-1-...-9, 4-10, 10-11); people 0 - the vessel's, on the second rung - holds 0, 1, 2 and 4;
	// people 1 holds 6, people 2 holds 7, people 3 holds 11; every Stability 0.75 and a stockpile no upkeep empties; no
	// hazard, project or notable; one instance of Fallen holding the systems of Sphere (unaware, unsealed); the vessel rules
	// people 0 standing in VesselSystem; the whole board known; tick 400. The plague of the tests runs out on WarnedSystem at
	// the next tick. Null when the galaxy does not give that.
	FGalaxyState* LayWarnBoard(UGalaxySimulationSubsystem& Sim, FName Fallen, std::initializer_list<int32> Sphere)
	{
		Sim.NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim.GetMutableState();
		if (CosmicTests::LivingSystems(State) < 12 || State.Civs.Num() < 4 || State.Player.RuledCivID != State.Civs[0].CivID
			|| State.Calling.LifeState != ELifeState::Embodied || !CosmicTests::WireChain(State))
		{
			return nullptr;
		}
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { VesselSystem, 1, WarnedSystem, LaterSystem });
		CosmicTests::Give(State, 1, { 6 });
		CosmicTests::Give(State, 2, { 7 });
		CosmicTests::Give(State, 3, { 11 });
		for (FStarSystemState& System : State.Systems)
		{
			CosmicHazards::EmptyHazardSlot(System);
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
		}
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.Stability = 0.75f;
			Civ.LowStabilityTicks = 0;
			Civ.Notables.Reset();
			Civ.ActiveEdicts.Reset();
			Civ.Stockpile.Energy = 1.0e6f;
			Civ.Stockpile.Matter = 1.0e6f;
			Civ.Stockpile.Insight = 1.0e6f;
			Civ.Stockpile.Influence = 1.0e6f;
		}
		State.Civs[0].Tier = EAscensionTier::Tier2_Stellar;
		State.Transcendents.Reset();
		const FTranscendentDef* Def = Sim.GetTables().FindTranscendent(Fallen);
		if (!Def)
		{
			return nullptr;
		}
		TArray<FName> Members;
		for (const int32 Member : Sphere)
		{
			Members.Add(State.Systems[Member].SystemID);
		}
		FTranscendentState& Instance = CosmicTests::AddInstance(State, Fallen, Members);
		Instance.Doctrine = Def->Doctrine;
		State.Player.CurrentSystemID = State.Systems[VesselSystem].SystemID;
		State.Tick = 400;
		CosmicTests::KnowAll(State);
		StandOn(State, WarnedSystem, TEXT("Hazard.Plague"));
		return &State;
	}

	// The systems that hold a claim.
	int32 ClaimCount(const FGalaxyState& State)
	{
		const FGameplayTag Claim = WarnTag(TEXT("Hazard.Claim"));
		int32 Count = 0;
		for (const FStarSystemState& System : State.Systems)
		{
			Count += System.ActiveHazard == Claim ? 1 : 0;
		}
		return Count;
	}

	// The live controller; null (with a test error) without Slate or a controller of the game.
	ACosmicPlayerController* WarnControllerOf(FAutomationTestBase& Test, UGalaxySimulationSubsystem& Sim)
	{
		if (!FSlateApplication::IsInitialized())
		{
			Test.AddError(TEXT("No Slate application: run the automation tests with -game (see CLAUDE.md)."));
			return nullptr;
		}
		UWorld* World = Sim.GetGameInstance()->GetWorld();
		ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(World ? World->GetFirstPlayerController() : nullptr);
		Test.TestNotNull(TEXT("The first player controller is the game's"), Controller);
		return Controller;
	}
}

// Pins the claim's warning (B6a header section 4, 6 UI.ClaimMayFollow, decision 14): for each case a fixed answer of the claim
// rule (ClaimRuleHoldsOf) and of the screen's answer (ClaimMayFollowOf), and what the simulation does when the hazard runs
// out - so the screen's answer is held to the game's. An ordinary settled system in a claimant's reach: true, and a claim
// stands after the expiry (the line under the hazard's and the details' tooltip on the panel) - and the panel stays silent
// where the answer still holds but the line is not the viewer's: another people's settled system (mutation: the line for
// any owner breaks it), a wanderer, the interregnum, an ended run (latched as UI.RunEnd latches it). The owner's last unclaimed
// settled system (Law.Remnant): false and no claim (mutation: dropping the last-unclaimed check from ClaimRuleHoldsOf breaks
// the fixed answer - and, the simulation calling it, the outcome). A claimant whose sphere is full; no claimant in reach:
// false and no claim. A hazard the owner's held edict answers (mutation: dropping the edict gate from ClaimMayFollowOf breaks
// it), Hazard.PreSupernova and Hazard.Claim: false, no new claim after the expiry, though the rule itself holds there.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIClaimMayFollowTest, "Cosmic_Ascension.UI.ClaimMayFollow", CosmicTests::Flags)
bool FCosmicUIClaimMayFollowTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWarningLineTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	ACosmicPlayerController* Controller = WarnControllerOf(*this, *Sim);
	if (!Controller)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef GameRules = Sim->GetRules();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	const FCosmicDataTables Tables = WarnTablesOf(Original, GameRules);
	if (!TestNotNull(TEXT("(the game's edict table)"), Tables.Edicts.Get()) || !TestTrue(TEXT("(the answer edict is in the tables)"), Tables.FindEdict(AnswerEdict) != nullptr))
	{
		return false;
	}
	Sim->SetTables(Tables);
	const FGalaxyState& State = Sim->GetState();
	const FGameplayTag Claim = WarnTag(TEXT("Hazard.Claim"));
	const FString ClaimLine = WarnWords(TEXT("Panel.ClaimMayFollow"));
	TestFalse(TEXT("(the string table holds Panel.ClaimMayFollow)"), ClaimLine.Contains(FStringTableEntry::GetPlaceholderSourceString()));

	UCosmicSystemPanelWidget* Panel = CreateWidget<UCosmicSystemPanelWidget>(Controller);
	if (!TestNotNull(TEXT("The system panel can be created"), Panel))
	{
		return false;
	}
	const TSharedRef<SWidget> SlatePanel = Panel->TakeWidget();

	// The two answers on the live state for WarnedSystem.
	auto RuleHolds = [Sim]() { return CosmicHazards::ClaimRuleHoldsOf(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), WarnedSystem); };
	auto MayFollow = [Sim]() { return CosmicHazards::ClaimMayFollowOf(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), WarnedSystem); };
	// What the panel shows on WarnedSystem: its detail lines and their tooltip.
	auto PanelWarns = [&]()
	{
		Panel->ShowSystem(State.Systems[WarnedSystem].SystemID);
		const UTextBlock* Details = Panel->GetDetailText();
		const bool bLine = Details && Details->GetText().ToString().Contains(ClaimLine);
		const bool bTip = Details && Details->GetToolTipText().ToString() == ClaimLine;
		return bLine && bTip;
	};
	auto PanelSilent = [&]()
	{
		Panel->ShowSystem(State.Systems[WarnedSystem].SystemID);
		const UTextBlock* Details = Panel->GetDetailText();
		return Details && !Details->GetText().ToString().Contains(ClaimLine) && Details->GetToolTipText().IsEmpty();
	};

	// (a) An ordinary settled system one lane from the sphere: a claim may follow, and one stands after the expiry.
	{
		FGalaxyState* Laid = LayWarnBoard(*Sim, WideFallen, { SphereSystem });
		if (!TestNotNull(TEXT("(the board of the ordinary case is laid)"), Laid))
		{
			return false;
		}
		TestTrue(TEXT("An ordinary settled system in a claimant's reach: the claim rule holds"), RuleHolds());
		TestTrue(TEXT("and a claim may follow the plague"), MayFollow());
		TestTrue(TEXT("The panel says so under the hazard's line and in the details' tooltip"), PanelWarns());

		// The line is for the people the vessel rules, in a run that goes on (the panel's gate - the answer itself asks
		// no owner, role or end). Each step restores the board, and the warning comes back.
		// Another people's settled system: people 1 holds WarnedSystem (its 6 stays unclaimed - not its last).
		const FName Warned = Laid->Systems[WarnedSystem].SystemID;
		const TArray<FName> WasOwned0 = Laid->Civs[0].OwnedSystems;
		const TArray<FName> WasOwned1 = Laid->Civs[1].OwnedSystems;
		Laid->Systems[WarnedSystem].OwnerCivID = Laid->Civs[1].CivID;
		Laid->Civs[0].OwnedSystems.Remove(Warned);
		Laid->Civs[1].OwnedSystems.Add(Warned);
		TestTrue(TEXT("(on another people's system a claim may follow too)"), MayFollow());
		TestTrue(TEXT("On another people's settled system the panel says nothing of a claim (mutation: the line for any owner breaks it)"), PanelSilent());
		Laid->Systems[WarnedSystem].OwnerCivID = Laid->Civs[0].CivID;
		Laid->Civs[0].OwnedSystems = WasOwned0;
		Laid->Civs[1].OwnedSystems = WasOwned1;
		TestTrue(TEXT("(the warning is back on the ruled people's system)"), PanelWarns());
		// A wanderer rules no people.
		const EPlayerRole WasRole = Laid->Player.Role;
		const FName WasRuled = Laid->Player.RuledCivID;
		Laid->Player.Role = EPlayerRole::Wanderer;
		Laid->Player.RuledCivID = NAME_None;
		TestTrue(TEXT("(for a wanderer a claim may follow too)"), MayFollow());
		TestTrue(TEXT("For a wanderer the panel says nothing of a claim"), PanelSilent());
		Laid->Player.Role = WasRole;
		Laid->Player.RuledCivID = WasRuled;
		TestTrue(TEXT("(the warning is back for the ruler)"), PanelWarns());
		// The interregnum: Player still holds the dead ruler and its people (the owner is the ruled people's), but no vessel
		// is embodied (mutation: dropping the embodied ruler's gate, bRules, breaks it).
		Laid->Calling.LifeState = ELifeState::Interregnum;
		TestTrue(TEXT("In the interregnum the panel says nothing of a claim on the dead ruler's people"), PanelSilent());
		Laid->Calling.LifeState = ELifeState::Embodied;
		TestTrue(TEXT("(the warning is back with the vessel)"), PanelWarns());
		// An ended run, latched as the controller latches it (UI.RunEnd): the clock's record at this tick.
		const int64 WasTick = Laid->Tick;
		CosmicTests::EndRunAt(*Laid, CosmicTags::EndingClock(), WasTick);
		Controller->FollowEnd(false);
		TestTrue(TEXT("(the end latched)"), Controller->IsRunEnded());
		TestTrue(TEXT("(in an ended run a claim may follow too)"), MayFollow());
		TestTrue(TEXT("In an ended run the panel says nothing of a claim"), PanelSilent());
		Controller->ClearRunEnd();
		Laid->Tick = WasTick;
		Laid->Ending = FCosmicEndingRecord();
		TestTrue(TEXT("(the warning is back in the run that goes on)"), PanelWarns());

		Sim->RunHeadless(1);
		TestTrue(TEXT("After the expiry the claim stands there"), State.Systems[WarnedSystem].ActiveHazard == Claim);
		Panel->ShowSystem(NAME_None);
	}

	// (a') No hazard stands: nothing to warn of, though the rule holds.
	{
		FGalaxyState* Laid = LayWarnBoard(*Sim, WideFallen, { SphereSystem });
		if (!TestNotNull(TEXT("(the board with no hazard is laid)"), Laid))
		{
			return false;
		}
		CosmicHazards::EmptyHazardSlot(Laid->Systems[WarnedSystem]);
		TestTrue(TEXT("(the rule holds on the empty slot)"), RuleHolds());
		TestFalse(TEXT("No hazard standing: no warning"), MayFollow());
	}

	// (b) The owner's last unclaimed settled system (Law.Remnant): its other settled systems lie in the sphere.
	{
		if (!TestNotNull(TEXT("(the board of the last unclaimed system is laid)"), LayWarnBoard(*Sim, WideFallen, { SphereSystem, VesselSystem, 1, LaterSystem })))
		{
			return false;
		}
		TestTrue(TEXT("(it is the owner's last unclaimed settled system, in a claimant's reach)"), CosmicHazards::IsLastUnclaimed(State, Sim->GetStateIndex(), Sim->GetTables(), WarnedSystem)
			&& CosmicHazards::ClaimantOf(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), WarnedSystem) == 0);
		TestFalse(TEXT("The owner's last unclaimed settled system: the claim rule does not hold"), RuleHolds());
		TestFalse(TEXT("and no claim may follow"), MayFollow());
		TestTrue(TEXT("The panel says nothing of a claim"), PanelSilent());
		Sim->RunHeadless(1);
		TestFalse(TEXT("(the plague ran out)"), State.Systems[WarnedSystem].ActiveHazard.IsValid());
		TestEqual(TEXT("After the expiry no claim stands"), ClaimCount(State), 0);
		Panel->ShowSystem(NAME_None);
	}

	// (c) A claimant whose sphere is full.
	{
		if (!TestNotNull(TEXT("(the board of the full sphere is laid)"), LayWarnBoard(*Sim, NarrowFallen, { SphereSystem })))
		{
			return false;
		}
		TestTrue(TEXT("(a claimant is in reach)"), CosmicHazards::ClaimantOf(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), WarnedSystem) == 0);
		TestFalse(TEXT("A claimant whose sphere is full: the claim rule does not hold"), RuleHolds());
		TestFalse(TEXT("and no claim may follow"), MayFollow());
		Sim->RunHeadless(1);
		TestEqual(TEXT("After the expiry no claim stands"), ClaimCount(State), 0);
	}

	// (d) No claimant in reach: the sphere lies seven lanes away.
	{
		if (!TestNotNull(TEXT("(the board of the far sphere is laid)"), LayWarnBoard(*Sim, WideFallen, { FarSystem })))
		{
			return false;
		}
		TestEqual(TEXT("(no claimant in reach)"), CosmicHazards::ClaimantOf(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), WarnedSystem), static_cast<int32>(INDEX_NONE));
		TestFalse(TEXT("No claimant in reach: the claim rule does not hold"), RuleHolds());
		TestFalse(TEXT("and no claim may follow"), MayFollow());
		Sim->RunHeadless(1);
		TestEqual(TEXT("After the expiry no claim stands"), ClaimCount(State), 0);
	}

	// (e) The owner holds the edict that answers the plague: the expiry resolves it and passes no claim rule.
	{
		FGalaxyState* Laid = LayWarnBoard(*Sim, WideFallen, { SphereSystem });
		if (!TestNotNull(TEXT("(the board of the held answer is laid)"), Laid))
		{
			return false;
		}
		Laid->Civs[0].ActiveEdicts.Add(AnswerEdict);
		TestTrue(TEXT("(the claim rule itself holds there)"), RuleHolds());
		TestFalse(TEXT("A hazard the owner's held edict answers: no claim may follow"), MayFollow());
		TestTrue(TEXT("The panel says nothing of a claim"), PanelSilent());
		Sim->RunHeadless(1);
		TestTrue(TEXT("(the edict was still held at the expiry)"), State.Civs[0].ActiveEdicts.Contains(AnswerEdict));
		TestFalse(TEXT("(the plague was resolved)"), State.Systems[WarnedSystem].ActiveHazard.IsValid());
		TestEqual(TEXT("After the resolution no claim stands"), ClaimCount(State), 0);
		Panel->ShowSystem(NAME_None);
	}

	// (f) Hazard.PreSupernova: the expiry destroys the system (the owner keeps others - no grace), and no claim follows.
	{
		FGalaxyState* Laid = LayWarnBoard(*Sim, WideFallen, { SphereSystem });
		if (!TestNotNull(TEXT("(the board of the supernova is laid)"), Laid))
		{
			return false;
		}
		StandOn(*Laid, WarnedSystem, TEXT("Hazard.PreSupernova"));
		TestTrue(TEXT("(the claim rule itself holds there)"), RuleHolds());
		TestFalse(TEXT("Hazard.PreSupernova: no claim may follow"), MayFollow());
		Sim->RunHeadless(1);
		TestTrue(TEXT("(the supernova destroyed the system)"), CosmicHazards::IsDeadNode(State.Systems[WarnedSystem]));
		TestEqual(TEXT("After the expiry no claim stands"), ClaimCount(State), 0);
	}

	// (g) Hazard.Claim: its expiry joins the system to the sphere, and no new claim follows it.
	{
		FGalaxyState* Laid = LayWarnBoard(*Sim, WideFallen, { SphereSystem });
		if (!TestNotNull(TEXT("(the board of the standing claim is laid)"), Laid))
		{
			return false;
		}
		StandOn(*Laid, WarnedSystem, TEXT("Hazard.Claim"));
		TestTrue(TEXT("(the claim rule itself holds there before the join)"), RuleHolds());
		TestFalse(TEXT("Hazard.Claim: no new claim may follow"), MayFollow());
		Sim->RunHeadless(1);
		TestTrue(TEXT("(the claim's expiry joined the system to the sphere)"), State.Transcendents.Num() == 1
			&& State.Transcendents[0].InfluencedSystems.Contains(State.Systems[WarnedSystem].SystemID));
		TestEqual(TEXT("After the expiry no new claim stands"), ClaimCount(State), 0);
	}
	return true;
}

// Pins the rebellion watch (B6a header section 4, 6 UI.RebellionWatch, decision 14): the ticks the line waits for are the
// Hazard.Rebellion row's (a table whose row waits 50 gives 50 - mutation: writing 100 in code breaks it), the threshold is
// the one constant the economy stage counts with (CosmicHazards::LowStabilityThreshold, the value the row's condition carries
// for the reader); the row's first tick (GalaxyTickAtLeast - the largest, at its ceiling) and the wait the line names: before
// that tick the count the run reaches there, after it the larger of the row's count and the count now (mutation: ignoring
// NotBeforeTick breaks it); shown below the threshold only, not at or above it, not with a count of 0 (mutation: dropping
// LowTicks > 0 breaks it), not while a rebellion stands on the capital (mutation: dropping that gate breaks it); the capital
// panel's line with the threshold, the ticks counted and the wait (mutation: printing NeededTicks breaks it), and none on
// another people's capital.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIRebellionWatchTest, "Cosmic_Ascension.UI.RebellionWatch", CosmicTests::Flags)
bool FCosmicUIRebellionWatchTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWarningLineTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	ACosmicPlayerController* Controller = WarnControllerOf(*this, *Sim);
	if (!Controller)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	const FGameplayTag Rebellion = CosmicTags::HazardRebellion();

	// The game's rebellion row and the count its spawn condition waits for.
	FName RebellionRow;
	const FHazardDef* GameRow = Original.FindHazardByTag(Rebellion, &RebellionRow);
	if (!TestNotNull(TEXT("(the game's tables hold the rebellion row)"), GameRow))
	{
		return false;
	}
	int32 GameNeeded = 0;
	float GameValue = -1.f;
	float GameFloorValue = 0.f;
	for (const FCosmicCondition& Condition : GameRow->SpawnConditions)
	{
		if (Condition.Type == EConditionType::CivStabilityBelowFor)
		{
			GameNeeded = FCString::Atoi(*Condition.Key.ToString());
			GameValue = Condition.Value;
		}
		else if (Condition.Type == EConditionType::GalaxyTickAtLeast && Condition.Value > GameFloorValue)
		{
			GameFloorValue = Condition.Value;
		}
	}
	if (!TestTrue(TEXT("(the game's row waits some ticks below the threshold, not 50)"), GameNeeded > 0 && GameNeeded != 50))
	{
		return false;
	}
	// The first tick of the run the game's row holds at: a whole tick past 200 (the cases below look before and after it).
	const int64 GameFloor = static_cast<int64>(GameFloorValue);
	if (!TestTrue(TEXT("(the game's row holds from a whole tick past 200 - GalaxyTickAtLeast)"), GameFloor > 200 && static_cast<float>(GameFloor) == GameFloorValue))
	{
		return false;
	}
	// The tables of the test: the game's, with the rebellion row waiting 50 ticks.
	FCosmicDataTables Fifty = Original;
	UDataTable* Hazards = CosmicTests::CopyWithout<FHazardDef>(Original.Hazards, {});
	FHazardDef* FiftyRow = Hazards ? Hazards->FindRow<FHazardDef>(RebellionRow, TEXT("UI.RebellionWatch")) : nullptr;
	if (!TestNotNull(TEXT("(the copied rebellion row)"), FiftyRow))
	{
		return false;
	}
	for (FCosmicCondition& Condition : FiftyRow->SpawnConditions)
	{
		if (Condition.Type == EConditionType::CivStabilityBelowFor)
		{
			Condition.Key = FName(TEXT("50"));
		}
	}
	Fifty.Hazards = Hazards;
	// The tables of the floor's cases: the row that waits 50, holding from two ticks, 260.25 then 120 (the largest binds,
	// and the evaluator's float(Tick) >= Value first holds at its ceiling, 261).
	FCosmicDataTables Floors = Original;
	UDataTable* FloorHazards = CosmicTests::CopyWithout<FHazardDef>(Original.Hazards, {});
	FHazardDef* FloorRow = FloorHazards ? FloorHazards->FindRow<FHazardDef>(RebellionRow, TEXT("UI.RebellionWatch")) : nullptr;
	if (!TestNotNull(TEXT("(the second copied rebellion row)"), FloorRow))
	{
		return false;
	}
	FloorRow->SpawnConditions.RemoveAll([](const FCosmicCondition& Condition) { return Condition.Type == EConditionType::GalaxyTickAtLeast; });
	for (FCosmicCondition& Condition : FloorRow->SpawnConditions)
	{
		if (Condition.Type == EConditionType::CivStabilityBelowFor)
		{
			Condition.Key = FName(TEXT("50"));
		}
	}
	for (const float FloorValue : { 260.25f, 120.f })
	{
		FCosmicCondition Floor;
		Floor.Type = EConditionType::GalaxyTickAtLeast;
		Floor.Value = FloorValue;
		FloorRow->SpawnConditions.Add(Floor);
	}
	Floors.Hazards = FloorHazards;

	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->SetTimeScale(0.f);
	FGalaxyState& Live = Sim->GetMutableState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const int32 Ruled = Index.FindCiv(Live, Live.Player.RuledCivID);
	const int32 Capital = Live.Civs.IsValidIndex(Ruled) ? Index.FindSystem(Live, Live.Civs[Ruled].HomeSystemID) : INDEX_NONE;
	if (!TestTrue(TEXT("(the vessel rules a people with its capital)"), Live.Civs.IsValidIndex(Ruled) && Live.Systems.IsValidIndex(Capital)))
	{
		return false;
	}
	CosmicHazards::EmptyHazardSlot(Live.Systems[Capital]);
	FCivilizationState& Civ = Live.Civs[Ruled];
	const float Threshold = CosmicHazards::LowStabilityThreshold;

	// Below the threshold: shown, with the simulation's constant, the people's count and the row's.
	Civ.Stability = Threshold - 0.05f;
	Civ.LowStabilityTicks = 37;
	{
		const CosmicHazards::FCosmicRebellionWatch Watch = CosmicHazards::RebellionWatchOf(Live, Original, Ruled);
		TestTrue(TEXT("Below the threshold the watch is shown"), Watch.bShown);
		TestEqual(TEXT("its threshold is the one the economy stage counts with"), Watch.Threshold, CosmicHazards::LowStabilityThreshold);
		TestEqual(TEXT("(the value the rebellion row's condition carries for the reader)"), Watch.Threshold, GameValue);
		TestEqual(TEXT("its count is the people's"), Watch.LowTicks, 37);
		TestEqual(TEXT("and the ticks waited for are the game row's"), Watch.NeededTicks, GameNeeded);
		TestEqual(TEXT("its first tick is the game row's GalaxyTickAtLeast"), Watch.NotBeforeTick, GameFloor);
		TestEqual(TEXT("Before the row's first tick the wait counts the ticks left to it: the count now plus them"), static_cast<int64>(Watch.WaitTicks),
			FMath::Max<int64>(GameNeeded, 37 + GameFloor - Live.Tick));
	}
	{
		const CosmicHazards::FCosmicRebellionWatch Watch = CosmicHazards::RebellionWatchOf(Live, Fifty, Ruled);
		TestEqual(TEXT("A row that waits 50 ticks gives 50: the count comes from the row"), Watch.NeededTicks, 50);
		TestTrue(TEXT("(still shown)"), Watch.bShown);
		TestEqual(TEXT("(its wait at tick 0 is the floor's too)"), static_cast<int64>(Watch.WaitTicks), FMath::Max<int64>(50, 37 + GameFloor - Live.Tick));
	}
	// The wait against the row's first tick, on the game's row: at tick 200 a count of 160 - past the row's count - still
	// waits for the floor (the count it will have reached then); past the floor the wait is the larger of the row's count
	// and the count now.
	Live.Tick = 200;
	Civ.LowStabilityTicks = 160;
	{
		const CosmicHazards::FCosmicRebellionWatch Watch = CosmicHazards::RebellionWatchOf(Live, Original, Ruled);
		TestTrue(TEXT("At tick 200 with 160 counted the watch is shown"), Watch.bShown);
		TestEqual(TEXT("and its wait is the count at the row's first tick, not the row's count"), static_cast<int64>(Watch.WaitTicks), 160 + GameFloor - 200);
	}
	Live.Tick = GameFloor + 50;
	Civ.LowStabilityTicks = 37;
	{
		const CosmicHazards::FCosmicRebellionWatch Watch = CosmicHazards::RebellionWatchOf(Live, Original, Ruled);
		TestEqual(TEXT("Past the row's first tick the wait is the row's count"), Watch.WaitTicks, FMath::Max(GameNeeded, 37));
	}
	Civ.LowStabilityTicks = GameNeeded + 133;
	TestEqual(TEXT("and a count past the row's is the wait itself"), CosmicHazards::RebellionWatchOf(Live, Original, Ruled).WaitTicks, GameNeeded + 133);
	// The floor's cases (the row that waits 50 and holds from 260.25 and from 120): the largest binds, at its ceiling.
	Live.Tick = 200;
	Civ.LowStabilityTicks = 160;
	{
		const CosmicHazards::FCosmicRebellionWatch Watch = CosmicHazards::RebellionWatchOf(Live, Floors, Ruled);
		TestEqual(TEXT("Of two GalaxyTickAtLeast the largest binds, at the first tick float(Tick) >= Value holds"), Watch.NotBeforeTick, static_cast<int64>(261));
		TestEqual(TEXT("At tick 200 with 160 counted the wait is 160 + 61"), Watch.WaitTicks, 221);
	}
	Live.Tick = 261;
	TestEqual(TEXT("At the first tick itself the wait is the count now"), CosmicHazards::RebellionWatchOf(Live, Floors, Ruled).WaitTicks, 160);
	Civ.LowStabilityTicks = 10;
	TestEqual(TEXT("and with 10 counted the row's 50"), CosmicHazards::RebellionWatchOf(Live, Floors, Ruled).WaitTicks, 50);
	Live.Tick = 0;
	// Below the threshold with nothing counted yet (the stability fell after the economy stage counted - a card's answer
	// between ticks): no run, no line.
	Civ.LowStabilityTicks = 0;
	{
		const CosmicHazards::FCosmicRebellionWatch Watch = CosmicHazards::RebellionWatchOf(Live, Original, Ruled);
		TestFalse(TEXT("Below the threshold with a count of 0 the watch is not shown"), Watch.bShown);
		TestEqual(TEXT("(its count 0)"), Watch.LowTicks, 0);
	}
	Civ.LowStabilityTicks = 37;
	// At and above the threshold: not shown (the economy counts strictly below it).
	Civ.Stability = Threshold;
	TestFalse(TEXT("At the threshold the watch is not shown"), CosmicHazards::RebellionWatchOf(Live, Original, Ruled).bShown);
	Civ.Stability = 0.75f;
	TestFalse(TEXT("Above it neither"), CosmicHazards::RebellionWatchOf(Live, Original, Ruled).bShown);
	// A rebellion standing on the capital: its own line speaks.
	Civ.Stability = Threshold - 0.05f;
	Live.Systems[Capital].ActiveHazard = Rebellion;
	Live.Systems[Capital].HazardTicksRemaining = 50;
	{
		const CosmicHazards::FCosmicRebellionWatch Watch = CosmicHazards::RebellionWatchOf(Live, Original, Ruled);
		TestFalse(TEXT("While a rebellion stands on the capital the watch is not shown"), Watch.bShown);
		TestEqual(TEXT("(its count still the people's)"), Watch.LowTicks, 37);
	}
	CosmicHazards::EmptyHazardSlot(Live.Systems[Capital]);
	TestFalse(TEXT("An index out of range shows nothing"), CosmicHazards::RebellionWatchOf(Live, Original, INDEX_NONE).bShown);

	// The capital panel's line, read through the simulation's tables (the row that waits 50).
	Sim->SetTables(Fifty);
	CosmicTests::KnowAll(Live);
	const FString Pattern = WarnWords(TEXT("Panel.RebellionWatch"));
	TestFalse(TEXT("(the string table holds Panel.RebellionWatch)"), Pattern.Contains(FStringTableEntry::GetPlaceholderSourceString()));
	FString Tail;
	if (!TestTrue(TEXT("(the line's words end after the waited ticks)"), Pattern.Split(TEXT("{2}"), nullptr, &Tail) && !Tail.IsEmpty()))
	{
		return false;
	}
	UCosmicSystemPanelWidget* Panel = CreateWidget<UCosmicSystemPanelWidget>(Controller);
	if (!TestNotNull(TEXT("The system panel can be created"), Panel))
	{
		return false;
	}
	const TSharedRef<SWidget> SlatePanel = Panel->TakeWidget();
	auto CapitalWords = [&]()
	{
		Panel->ShowSystem(Live.Systems[Capital].SystemID);
		const UTextBlock* Details = Panel->GetDetailText();
		return Details ? Details->GetText().ToString() : FString();
	};
	// At tick 0 the line names the wait, the count the run reaches at the row's first tick (mutation: printing the row's
	// count breaks it).
	const int64 FloorWait = FMath::Max<int64>(50, 37 + GameFloor - Live.Tick);
	const FString FloorLine = WarnWords(FText::Format(CosmicUI::Label(TEXT("Panel.RebellionWatch")), CosmicUI::Percent(Threshold), FText::AsNumber(37), FText::AsNumber(FloorWait)));
	TestTrue(TEXT("Before the row's first tick the capital panel names the count at that tick"), CapitalWords().Contains(FloorLine));
	// Past the row's first tick: the row's 50.
	Live.Tick = GameFloor + 100;
	const FString Line = WarnWords(FText::Format(CosmicUI::Label(TEXT("Panel.RebellionWatch")), CosmicUI::Percent(Threshold), FText::AsNumber(37), FText::AsNumber(50)));
	TestTrue(TEXT("Below the threshold the capital panel says the threshold, the ticks under it and the row's 50"), CapitalWords().Contains(Line));
	Civ.Stability = 0.75f;
	TestFalse(TEXT("Above it the panel has no such line"), CapitalWords().Contains(Tail));
	Civ.Stability = Threshold - 0.05f;
	Live.Systems[Capital].ActiveHazard = Rebellion;
	Live.Systems[Capital].HazardTicksRemaining = 50;
	TestFalse(TEXT("nor while a rebellion stands on the capital"), CapitalWords().Contains(Tail));
	CosmicHazards::EmptyHazardSlot(Live.Systems[Capital]);
	Civ.LowStabilityTicks = 0;
	TestFalse(TEXT("nor below the threshold with a count of 0 (mutation: dropping LowTicks > 0 breaks it)"), CapitalWords().Contains(Tail));
	Civ.LowStabilityTicks = 37;
	TestTrue(TEXT("(the line is back with the count)"), CapitalWords().Contains(Line));

	// Another people's capital below the threshold: the watch itself would show (it asks no owner), the panel does not -
	// its line is for the capital of the people the vessel rules.
	int32 Other = INDEX_NONE;
	int32 OtherCapital = INDEX_NONE;
	for (int32 CivIndex = 0; CivIndex < Live.Civs.Num() && Other == INDEX_NONE; ++CivIndex)
	{
		const int32 Home = Index.FindSystem(Live, Live.Civs[CivIndex].HomeSystemID);
		if (CivIndex != Ruled && Live.Systems.IsValidIndex(Home) && Live.Systems[Home].OwnerCivID == Live.Civs[CivIndex].CivID)
		{
			Other = CivIndex;
			OtherCapital = Home;
		}
	}
	if (!TestTrue(TEXT("(another people holds its capital)"), Other != INDEX_NONE))
	{
		return false;
	}
	const FGameplayTag OtherHazard = Live.Systems[OtherCapital].ActiveHazard;
	const int32 OtherHazardTicks = Live.Systems[OtherCapital].HazardTicksRemaining;
	const float OtherStability = Live.Civs[Other].Stability;
	const int32 OtherLowTicks = Live.Civs[Other].LowStabilityTicks;
	CosmicHazards::EmptyHazardSlot(Live.Systems[OtherCapital]);
	Live.Civs[Other].Stability = Threshold - 0.05f;
	Live.Civs[Other].LowStabilityTicks = 37;
	TestTrue(TEXT("(the other people's watch would show)"), CosmicHazards::RebellionWatchOf(Live, Fifty, Other).bShown);
	Panel->ShowSystem(Live.Systems[OtherCapital].SystemID);
	const UTextBlock* OtherDetails = Panel->GetDetailText();
	TestTrue(TEXT("Another people's capital below the threshold has no rebellion line on the panel"), OtherDetails && !OtherDetails->GetText().ToString().Contains(Tail));
	Live.Civs[Other].Stability = OtherStability;
	Live.Civs[Other].LowStabilityTicks = OtherLowTicks;
	Live.Systems[OtherCapital].ActiveHazard = OtherHazard;
	Live.Systems[OtherCapital].HazardTicksRemaining = OtherHazardTicks;
	Live.Tick = 0;
	Panel->ShowSystem(NAME_None);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

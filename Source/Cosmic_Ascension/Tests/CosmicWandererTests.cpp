// Cosmic Ascension - automation tests of the wanderer's loop (milestone B item B1): the move, the scan, the wanderer's
// cards and their answer, the coronation and the return, the abdication, the chronicle's lines and the save in the middle
// of it. Design: guideline.md section 5.9, 5.10; docs/60_DECISIONS/headers/B1_wanderer_loop.md.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicIDs.h"
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
#include "Events/CosmicPetitions.h"
#include "GameplayTagContainer.h"
#include "Log/CosmicChronicle.h"
#include "Log/CosmicLogSubsystem.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/CosmicEndings.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "Tests/CosmicTestProbe.h"
#include "World/CosmicRulerPolicy.h"
#include "World/CosmicWandererPolicy.h"

#include <initializer_list>

namespace CosmicWandererTests
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
		int32 Difficulty = 0, const TCHAR* Failure = TEXT(""))
	{
		FEncounterChoice Out;
		Out.Label = FText::AsCultureInvariant(TEXT("choice"));
		Out.CheckStat = Check;
		Out.Difficulty = Difficulty;
		Out.OnSuccess = OnSuccess;
		Out.SuccessText = FText::AsCultureInvariant(Success);
		Out.FailureText = FText::AsCultureInvariant(Failure);
		return Out;
	}

	// A wanderer's card: about the system the vessel stands in, put before a wanderer.
	FEncounterDef WandererCard(ECosmicEventTrigger Trigger, std::initializer_list<FCosmicCondition> Conditions, TArray<FEncounterChoice> Choices, float Weight = 1.f)
	{
		FEncounterDef Row;
		Row.Title = FText::AsCultureInvariant(TEXT("card"));
		Row.Body = FText::AsCultureInvariant(TEXT("{System} waits."));
		Row.Trigger = Trigger;
		Row.Subject = ECosmicEventSubject::VesselSystem;
		Row.Conditions = Conditions;
		Row.Conditions.Insert(Cond(EConditionType::PlayerRoleIs, nullptr, TEXT("Wanderer")), 0);
		Row.Choices = MoveTemp(Choices);
		Row.BaseWeight = Weight;
		return Row;
	}

	// A ruler's petition (the rows of MVP item 2).
	FEncounterDef Petition(ECosmicEventTrigger Trigger, ECosmicEventSubject Subject, std::initializer_list<FCosmicCondition> Conditions, float Weight = 1.f)
	{
		FEncounterDef Row;
		Row.Title = FText::AsCultureInvariant(TEXT("plea"));
		Row.Body = FText::AsCultureInvariant(TEXT("{Civ} asks."));
		Row.Trigger = Trigger;
		Row.Subject = Subject;
		Row.Conditions = Conditions;
		Row.Conditions.Insert(Cond(EConditionType::PlayerRoleIs, nullptr, TEXT("Ruler")), 0);
		Row.Choices = { Choice(TEXT("a.")), Choice(TEXT("b.")) };
		Row.BaseWeight = Weight;
		return Row;
	}

	const FName Tend(TEXT("enc_test_tend"));
	const FName ArriveA(TEXT("enc_test_arrive_a"));
	const FName ArriveB(TEXT("enc_test_arrive_b"));
	const FName PlaceRow(TEXT("petition_test_place"));
	const FName LowRow(TEXT("petition_test_low"));
	const FName LotA(TEXT("petition_test_lot_a"));
	const FName LotB(TEXT("petition_test_lot_b"));

	// The cards of these tests: a Standing wanderer's card on a plague (a Mind check of Difficulty, reverence and the
	// plague's end on success), two arrival cards (Body +1), a ruler's Standing petition on a plague, one on low Stability,
	// and two periodic petitions.
	UDataTable* Cards(int32 Difficulty = 4, bool bWandererRows = true)
	{
		UDataTable* Table = CosmicTests::NewTable(FEncounterDef::StaticStruct());
		if (bWandererRows)
		{
			Table->AddRow(Tend, WandererCard(ECosmicEventTrigger::Standing, { Cond(EConditionType::PlayerInSystemWithHazard, TEXT("Hazard.Plague")) },
				{ Choice(TEXT("tended."), { Eff(EEffectType::ChangeReverence, nullptr, nullptr, 0.3f), Eff(EEffectType::ResolveHazard, TEXT("Hazard.Plague")) },
					ECosmicCheckStat::Mind, Difficulty, TEXT("fell.")), Choice(TEXT("left.")) }));
			Table->AddRow(ArriveA, WandererCard(ECosmicEventTrigger::Lottery, {}, { Choice(TEXT("cut."), { Eff(EEffectType::PlayerStatDelta, nullptr, TEXT("Body"), 1.f) }),
				Choice(TEXT("pushed.")) }));
			Table->AddRow(ArriveB, WandererCard(ECosmicEventTrigger::Lottery, {}, { Choice(TEXT("cut."), { Eff(EEffectType::PlayerStatDelta, nullptr, TEXT("Body"), 1.f) }),
				Choice(TEXT("pushed.")) }));
		}
		Table->AddRow(PlaceRow, Petition(ECosmicEventTrigger::Standing, ECosmicEventSubject::SettledSystem, { Cond(EConditionType::SystemHasHazard, TEXT("Hazard.Plague")) }));
		Table->AddRow(LowRow, Petition(ECosmicEventTrigger::Standing, ECosmicEventSubject::Civilization, { Cond(EConditionType::CivStabilityBelow, nullptr, nullptr, 0.3f) }));
		Table->AddRow(LotA, Petition(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, {}));
		Table->AddRow(LotB, Petition(ECosmicEventTrigger::Lottery, ECosmicEventSubject::Civilization, {}));
		return Table;
	}

	// The game's hazard rows that no draw places: what a test puts in a slot expires and resolves as in the game, and
	// nothing else arrives.
	UDataTable* QuietHazards(const FCosmicDataTables& Original)
	{
		UDataTable* Table = CosmicTests::NewTable(FHazardDef::StaticStruct());
		for (const FName RowName : Original.Hazards ? Original.Hazards->GetRowNames() : TArray<FName>())
		{
			if (const FHazardDef* Row = Original.Hazards->FindRow<FHazardDef>(RowName, TEXT("QuietHazards")))
			{
				FHazardDef Quiet = *Row;
				Quiet.SpawnChancePerTick = 0.f;
				Table->AddRow(RowName, Quiet);
			}
		}
		return Table;
	}

	// The game's tables with these cards, quiet hazards, no projects or feats, no event slot, no first crisis, no periodic
	// petition (unless asked), the wanderer's knobs as given.
	FCosmicDataTables TablesWith(const FCosmicDataTables& Original, UDataTable* Encounters, TFunctionRef<void(FCosmicRulesDef&)> Tune)
	{
		FCosmicRulesDef Rules = Original.FindRules() ? *Original.FindRules() : FCosmicRulesDef();
		Rules.EventChancePerSlot = 0.f;
		Rules.FirstCrisisTick = 0;
		Rules.PetitionPeriodTicks = 100000;
		Tune(Rules);
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));
		Tables.Encounters = Encounters;
		Tables.Hazards = QuietHazards(Original);
		return Tables;
	}

	FCosmicDataTables TablesWith(const FCosmicDataTables& Original, UDataTable* Encounters)
	{
		return TablesWith(Original, Encounters, [](FCosmicRulesDef&) {});
	}

	// The board of the loop: the game's galaxy of seed 42 on the chain of CosmicTests::WireChain (0-1-...-9, 4-10, 10-11).
	// The vessel rules people 0 from system 0; people 1 holds 5 and 6, people 2 holds 8, people 3 holds 11 (the rest is
	// wild); every people on the first rung at Stability 0.8 with no reverence; no hazard, no project, no transcendent; the
	// calling knows system 0 and has met people 0. Null when the galaxy does not give that.
	FGalaxyState* LayBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Tables)
	{
		Sim.SetTables(Tables);
		Sim.NewGalaxy(42, 0, 0);
		FGalaxyState& State = Sim.GetMutableState();
		if (!CosmicTests::WireChain(State) || State.Civs.Num() < 4 || State.Player.RuledCivID != State.Civs[0].CivID)
		{
			return nullptr;
		}
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { 0 });
		CosmicTests::Give(State, 1, { 5, 6 });
		CosmicTests::Give(State, 2, { 8 });
		CosmicTests::Give(State, 3, { 11 });
		State.Transcendents.Reset();
		State.Log.Reset();
		for (FStarSystemState& System : State.Systems)
		{
			CosmicHazards::EmptyHazardSlot(System);
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
		}
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.Tier = EAscensionTier::Tier1_Planetary;
			Civ.Stability = 0.8f;
			Civ.ReverenceToPlayer = 0.f;
			Civ.ThroneLockedUntilTick = 0;
		}
		State.Calling.PersonalTier = EAscensionTier::Tier1_Planetary;
		State.Player.CurrentSystemID = State.Systems[0].SystemID;
		State.Knowing = FKnowingState();
		CosmicTests::SetKnowing(State, { 0 }, { 0 });
		return &State;
	}

	// The vessel leaves its throne from SystemIndex (it stands there first) through BecomeWanderer: a deposed wanderer that
	// knows where it stands and the lanes around.
	void Walk(UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, int32 SystemIndex)
	{
		Sim.GetMutableState().Player.CurrentSystemID = Sim.GetState().Systems[SystemIndex].SystemID;
		Role.BecomeWanderer(Tag(TEXT("AI.Profile.Conservative")), Tag(TEXT("Reign.Deposed")));
	}

	void Plague(FGalaxyState& State, int32 SystemIndex, int32 Ticks = 150)
	{
		State.Systems[SystemIndex].ActiveHazard = Tag(TEXT("Hazard.Plague"));
		State.Systems[SystemIndex].HazardTicksRemaining = Ticks;
	}

	bool Knows(const FGalaxyState& State, int32 SystemIndex)
	{
		const FName ID = State.Systems[SystemIndex].SystemID;
		return State.Knowing.KnownSystems.ContainsByPredicate([ID](const FKnownMark& Mark) { return Mark.ID == ID; });
	}

	bool HasMet(const FGalaxyState& State, FName CivID)
	{
		return State.Knowing.MetCivs.ContainsByPredicate([CivID](const FKnownMark& Mark) { return Mark.ID == CivID; });
	}

	void Meet(FGalaxyState& State, int32 CivIndex)
	{
		if (!HasMet(State, State.Civs[CivIndex].CivID))
		{
			FKnownMark& Mark = State.Knowing.MetCivs.AddDefaulted_GetRef();
			Mark.ID = State.Civs[CivIndex].CivID;
			Mark.Tick = State.Tick;
		}
	}

	int32 LinesOf(const FGalaxyState& State, FName SourceRow)
	{
		int32 Count = 0;
		for (const FCosmicLogEntry& Entry : State.Log)
		{
			Count += Entry.SourceRow == SourceRow ? 1 : 0;
		}
		return Count;
	}

	int32 SeedAfterDraws(const FRandomStream& Stream, int32 Draws)
	{
		FRandomStream Copy = Stream;
		for (int32 Draw = 0; Draw < Draws; ++Draw)
		{
			Copy.GetFraction();
		}
		return Copy.GetCurrentSeed();
	}

	int32 NextRoll(const FRandomStream& Stream)
	{
		FRandomStream Copy = Stream;
		return FMath::Min(static_cast<int32>(Copy.GetFraction() * 6.f), 5);
	}

	int32 OpenRecords(const FCivilizationState& Civ, const TCHAR* Role)
	{
		int32 Count = 0;
		for (const FNotableRecord& Record : Civ.Notables)
		{
			Count += (Record.LastTick == 0 && Record.Role == Tag(Role)) ? 1 : 0;
		}
		return Count;
	}
}

// The move (B1 header section 2.3, 2.4): one lane a command, the reasons a move is refused, the arrival on the tick the
// countdown ends, a target destroyed on the way, an origin destroyed under the vessel.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimTravelToTest, "Cosmic_Ascension.Sim.TravelTo", CosmicTests::Flags)
bool FCosmicSimTravelToTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWandererTests;
	using CosmicWanderer::ETravelBlock;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Cards(), [](FCosmicRulesDef& Rules) { Rules.EncounterChancePerArrival = 0.f; }));
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const int32 Lane = Sim->GetRules().LaneTravelTicks;
	const FName Sys3 = State->Systems[3].SystemID;
	const FName Sys4 = State->Systems[4].SystemID;
	const FName Sys5 = State->Systems[5].SystemID;
	TestTrue(TEXT("A ruler does not travel"), Role->TravelTo(State->Systems[1].SystemID) == ETravelBlock::NotWanderer);

	Walk(*Sim, *Role, 4);
	TestTrue(TEXT("The deposed vessel walks from system 4"), CosmicWanderer::IsEmbodiedWanderer(*State) && State->Player.CurrentSystemID == Sys4);
	for (const FName Neighbour : TArray<FName>(State->Systems[4].Lanes))
	{
		TestTrue(TEXT("A wanderer's lane neighbours are all known"), Knows(*State, Sim->GetStateIndex().FindSystem(*State, Neighbour)));
	}

	// Refused: nothing changes, nothing is drawn.
	{
		const uint32 Before = Sim->ComputeStateHash();
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		TestTrue(TEXT("A system two lanes off is no neighbour"), Role->TravelTo(State->Systems[6].SystemID) == ETravelBlock::NotNeighbour);
		TestTrue(TEXT("The system it stands in is no neighbour"), Role->TravelTo(Sys4) == ETravelBlock::NotNeighbour);
		TestTrue(TEXT("The sanctuary is the first reason about the target"), Role->TravelTo(CosmicIDs::Sanctuary) == ETravelBlock::Sanctuary);
		State->Knowing.PendingPetitionRow = Tend;
		TestTrue(TEXT("A waiting card comes first"), Role->TravelTo(Sys5) == ETravelBlock::CardWaiting);
		State->Knowing.PendingPetitionRow = NAME_None;
		TestEqual(TEXT("A refused move changes nothing"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Before));
		TestEqual(TEXT("and draws nothing"), Sim->GetRng().GetCurrentSeed(), Seed);
	}

	// Taken: the countdown of a lane, the move on its last tick.
	{
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		TestTrue(TEXT("A lane neighbour is taken"), Role->TravelTo(Sys5) == ETravelBlock::None);
		TestEqual(TEXT("without a draw"), Sim->GetRng().GetCurrentSeed(), Seed);
		TestEqual(TEXT("The journey has a target"), State->Player.TravelTargetSystemID, Sys5);
		TestEqual(TEXT("and a lane's ticks"), State->Player.TravelTicksRemaining, Lane);
		TestTrue(TEXT("A travelling vessel takes no second move"), Role->TravelTo(Sys3) == ETravelBlock::Travelling);
		Sim->RunHeadless(Lane - 1);
		TestEqual(TEXT("A tick before the end the vessel still stands where it left"), State->Player.CurrentSystemID, Sys4);
		Sim->StepOnce();
		TestEqual(TEXT("On the last tick it arrives"), State->Player.CurrentSystemID, Sys5);
		TestTrue(TEXT("and the journey's two fields empty together"), State->Player.TravelTargetSystemID.IsNone() && State->Player.TravelTicksRemaining == 0);
	}

	// A target destroyed on the way: the journey ends where it began.
	{
		TestTrue(TEXT("Back towards 4"), Role->TravelTo(Sys4) == ETravelBlock::None);
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		CosmicHazards::Destroy(Ctx, 4);
		Sim->RunHeadless(Lane);
		TestEqual(TEXT("A target destroyed on the way: the vessel stays"), State->Player.CurrentSystemID, Sys5);
		TestTrue(TEXT("and the journey is over"), State->Player.TravelTargetSystemID.IsNone() && State->Player.TravelTicksRemaining == 0);
	}

	// An origin destroyed under the vessel: carried to a living neighbour, the journey emptied.
	{
		TestTrue(TEXT("Towards 6"), Role->TravelTo(State->Systems[6].SystemID) == ETravelBlock::None);
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		CosmicHazards::Destroy(Ctx, 5);
		TestEqual(TEXT("The destruction carried the vessel to its first living neighbour"), State->Player.CurrentSystemID, State->Systems[6].SystemID);
		TestTrue(TEXT("and ended the journey"), State->Player.TravelTargetSystemID.IsNone() && State->Player.TravelTicksRemaining == 0);
	}

	// A lane of 0 ticks reads as one.
	{
		Sim->SetTables(TablesWith(Original, Cards(), [](FCosmicRulesDef& Rules) { Rules.LaneTravelTicks = 0; Rules.EncounterChancePerArrival = 0.f; }));
		TestTrue(TEXT("A lane of 0 ticks still takes the move"), Role->TravelTo(State->Systems[7].SystemID) == ETravelBlock::None);
		TestEqual(TEXT("with one tick to go"), State->Player.TravelTicksRemaining, 1);
		Sim->StepOnce();
		TestEqual(TEXT("and arrives in the next tick"), State->Player.CurrentSystemID, State->Systems[7].SystemID);
	}

	// A journey that ends where it began (its target destroyed on the way) brings no card for a crisis that stood at the
	// origin throughout: at the head of a tick a travelling vessel counts as standing where it left (section 2.4).
	{
		FGalaxyState* Run = LayBoard(*Sim, TablesWith(Original, Cards(), [](FCosmicRulesDef& Rules) { Rules.EncounterChancePerArrival = 0.f; }));
		if (!TestNotNull(TEXT("The board of the failed journey is laid"), Run))
		{
			return false;
		}
		Plague(*Run, 5, 400);
		Walk(*Sim, *Role, 5);
		Sim->StepOnce();
		TestTrue(TEXT("A plague that stood when the vessel began to walk is no card"), Run->Knowing.PendingPetitionRow.IsNone());
		TestTrue(TEXT("(off towards 4)"), Role->TravelTo(Run->Systems[4].SystemID) == ETravelBlock::None);
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		CosmicHazards::Destroy(Ctx, 4);
		Sim->RunHeadless(Lane);
		TestTrue(TEXT("(the journey failed: the vessel is still at 5)"), Run->Player.CurrentSystemID == Run->Systems[5].SystemID && !CosmicWanderer::IsTravelling(*Run));
		TestTrue(TEXT("A failed journey brings no card for the plague that stood throughout"), Run->Knowing.PendingPetitionRow.IsNone());
	}

	// A journey draws no number of the sim stream: the same board with and without one ends on the same number.
	{
		TArray<int32> Seeds;
		for (const bool bGo : { true, false })
		{
			FGalaxyState* Run = LayBoard(*Sim, TablesWith(Original, Cards(), [](FCosmicRulesDef& Rules) { Rules.EncounterChancePerArrival = 1.f; }));
			if (!TestNotNull(TEXT("The board of the journey is laid"), Run))
			{
				return false;
			}
			Walk(*Sim, *Role, 4);
			if (bGo)
			{
				Role->TravelTo(Run->Systems[5].SystemID);
			}
			Sim->RunHeadless(Lane + 2);
			Seeds.Add(Sim->GetRng().GetCurrentSeed());
		}
		TestEqual(TEXT("A journey and its arrival draw no number of the sim stream"), Seeds[0], Seeds[1]);
	}

	// Inside a handler: an error line, nothing.
	{
		CosmicTests::FScopedProbe Probe(*Sim);
		bool bTried = false;
		ETravelBlock Tried = ETravelBlock::None;
		const FName Six = State->Systems[6].SystemID;
		Probe.Probe->OnLog = [&bTried, &Tried, Role, Six](const FCosmicLogEntry&)
		{
			if (!bTried)
			{
				bTried = true;
				Tried = Role->TravelTo(Six);
			}
		};
		AddExpectedError(TEXT("TravelTo called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>()->Push(Tag(TEXT("Log.Civ")), 1, FText::AsCultureInvariant(TEXT("a line.")), FFormatNamedArguments(), NAME_None, NAME_None);
		TestTrue(TEXT("The handler ran"), bTried);
		TestTrue(TEXT("and the move was refused"), Tried == ETravelBlock::Handler);
		TestTrue(TEXT("with no journey"), State->Player.TravelTargetSystemID.IsNone());
	}
	return true;
}

// The scan (B1 header section 2.3): one look a lane farther than the wanderer sees, knowing only, a line on a wild system
// that showed something new.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimScanTest, "Cosmic_Ascension.Sim.Scan", CosmicTests::Flags)
bool FCosmicSimScanTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWandererTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Cards(), [](FCosmicRulesDef& Rules)
	{
		Rules.SightLanesByTier = { 2, 3, 4, 6, 10 };
		Rules.ScanSightBonusLanes = 1;
	}));
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	TestEqual(TEXT("A ruler does not scan"), Role->ScanCurrentSystem(), INDEX_NONE);
	Walk(*Sim, *Role, 4);
	// From 4 the wanderer of the first rung sees two lanes (3, 5, 10; then 2, 6, 11); the scan sees three (1 and 7 too).
	const uint32 World = Sim->ComputeWorldHash();
	const uint32 Full = Sim->ComputeStateHash();
	const int32 Seed = Sim->GetRng().GetCurrentSeed();
	const int32 Learned = Role->ScanCurrentSystem();
	TestEqual(TEXT("The scan learns what lies within three lanes and was not known (2, 6, 11, 1, 7)"), Learned, 5);
	TestTrue(TEXT("It knows the systems three lanes off"), Knows(*State, 1) && Knows(*State, 7));
	TestFalse(TEXT("and not four"), Knows(*State, 8));
	TestEqual(TEXT("The world is the same"), static_cast<int64>(Sim->ComputeWorldHash()), static_cast<int64>(World));
	TestNotEqual(TEXT("knowing is not"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Full));
	TestEqual(TEXT("No number of the sim stream"), Sim->GetRng().GetCurrentSeed(), Seed);
	TestEqual(TEXT("System 4 is wild: one line of the wild scan"), LinesOf(*State, CosmicRows::LogScanWild), 1);
	TestTrue(TEXT("It met the owners it saw (people 1 at 5)"), HasMet(*State, State->Civs[1].CivID));
	TestEqual(TEXT("A second scan learns nothing"), Role->ScanCurrentSystem(), 0);
	TestEqual(TEXT("and tells nothing"), LinesOf(*State, CosmicRows::LogScanWild), 1);
	TestTrue(TEXT("Under way the vessel does not scan"), Role->TravelTo(State->Systems[5].SystemID) == CosmicWanderer::ETravelBlock::None);
	TestEqual(TEXT("(refused)"), Role->ScanCurrentSystem(), INDEX_NONE);
	Sim->RunHeadless(Sim->GetRules().LaneTravelTicks);
	// At 5 - people 1's - a scan that learns tells no wild line.
	const int32 AtFive = Role->ScanCurrentSystem();
	TestTrue(TEXT("A scan of an owned system learns (8 and 9)"), AtFive > 0);
	TestEqual(TEXT("and tells no wild line"), LinesOf(*State, CosmicRows::LogScanWild), 1);

	// Inside a handler: an error line, no scan.
	{
		CosmicTests::FScopedProbe Probe(*Sim);
		bool bTried = false;
		int32 Tried = 0;
		const int32 KnownBefore = State->Knowing.KnownSystems.Num();
		Probe.Probe->OnLog = [&bTried, &Tried, Role](const FCosmicLogEntry&)
		{
			if (!bTried)
			{
				bTried = true;
				Tried = Role->ScanCurrentSystem();
			}
		};
		AddExpectedError(TEXT("ScanCurrentSystem called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>()->Push(Tag(TEXT("Log.Civ")), 1, FText::AsCultureInvariant(TEXT("a line.")), FFormatNamedArguments(), NAME_None, NAME_None);
		TestTrue(TEXT("The handler ran"), bTried);
		TestEqual(TEXT("and the scan was refused"), Tried, INDEX_NONE);
		TestEqual(TEXT("learning nothing"), State->Knowing.KnownSystems.Num(), KnownBefore);
	}
	return true;
}

// The wanderer's card (B1 header section 2.5): the place and the moment, the most recent card not again, the gap, the
// arrival card's share and pick, the ruler's rows apart, and a world that does not move while the cards wait.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimEncounterPresentTest, "Cosmic_Ascension.Sim.EncounterPresent", CosmicTests::Flags)
bool FCosmicSimEncounterPresentTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWandererTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	// 1. Through the ticks: the plague at 5 is a card on the tick the vessel arrives there, not on the way, not again
	// while it stays.
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Cards(), [](FCosmicRulesDef& Rules) { Rules.EncounterChancePerArrival = 0.f; }));
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		Walk(*Sim, *Role, 4);
		Plague(*State, 5);
		Role->TravelTo(State->Systems[5].SystemID);
		const int32 Lane = Sim->GetRules().LaneTravelTicks;
		Sim->RunHeadless(Lane - 1);
		TestTrue(TEXT("No card on the way"), State->Knowing.PendingPetitionRow.IsNone());
		Sim->StepOnce();
		TestEqual(TEXT("The plague's card on the tick of the arrival"), State->Knowing.PendingPetitionRow, Tend);
		TestEqual(TEXT("about the system it came to"), State->Knowing.PendingPetitionSystemID, State->Systems[5].SystemID);
		TestTrue(TEXT("whose people it met first"), HasMet(*State, State->Civs[1].CivID));
		CosmicPetitions::Clear(*State);
		Sim->RunHeadless(20);
		TestTrue(TEXT("Staying beside the plague brings no card again"), State->Knowing.PendingPetitionRow.IsNone());
	}

	// 2. Present itself.
	FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Cards(), [](FCosmicRulesDef& Rules) { Rules.EncounterChancePerArrival = 1.f; }));
	if (!TestNotNull(TEXT("The board is laid again"), State))
	{
		return false;
	}
	Walk(*Sim, *Role, 4);
	State->Player.CurrentSystemID = State->Systems[5].SystemID;
	Plague(*State, 5);
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicDataTables& Tables = Sim->GetTables();
	const FCosmicRulesDef& Rules = Sim->GetRules();
	const TArray<FCosmicPetitionUnit> Held = CosmicPetitions::StandingHeld(*State, Index, Tables, Rules);
	TestTrue(TEXT("StandingHeld of a wanderer lists its card on the system it stands in"), Held.Num() == 1 && Held[0] == (FCosmicPetitionUnit{ Tend, 5 }));
	TestTrue(TEXT("Present puts the card that came to hold"), CosmicPetitions::Present(*State, Index, Tables, Rules, {}, false));
	TestEqual(TEXT("(the plague's)"), State->Knowing.PendingPetitionRow, Tend);
	CosmicPetitions::Clear(*State);
	TestFalse(TEXT("A card that held at the head of the tick is not put"), CosmicPetitions::Present(*State, Index, Tables, Rules, Held, false));
	State->Calling.Lives.Last().LastChoiceRows = { Tend };
	TestFalse(TEXT("The life's most recent answered card does not come again"), CosmicPetitions::Present(*State, Index, Tables, Rules, {}, false));
	State->Calling.Lives.Last().LastChoiceRows = { Tend, ArriveA };
	TestTrue(TEXT("After another answer it comes again"), CosmicPetitions::Present(*State, Index, Tables, Rules, {}, false) && State->Knowing.PendingPetitionRow == Tend);
	CosmicPetitions::Clear(*State);
	State->Calling.Lives.Last().LastChoiceRows.Reset();
	State->Civs[1].Tier = EAscensionTier::Tier3_Causal;
	TestFalse(TEXT("A people two rungs above the wanderer gets no card"), CosmicPetitions::Present(*State, Index, Tables, Rules, {}, false));
	TestTrue(TEXT("but an arrival there brings the arrival card, which speaks of the place alone"), CosmicPetitions::Present(*State, Index, Tables, Rules, {}, true)
		&& (State->Knowing.PendingPetitionRow == ArriveA || State->Knowing.PendingPetitionRow == ArriveB));
	CosmicPetitions::Clear(*State);
	State->Civs[1].Tier = EAscensionTier::Tier2_Stellar;
	TestTrue(TEXT("one rung above does"), CosmicPetitions::Present(*State, Index, Tables, Rules, {}, false));
	CosmicPetitions::Clear(*State);
	State->Player.TravelTargetSystemID = State->Systems[6].SystemID;
	State->Player.TravelTicksRemaining = 5;
	TestTrue(TEXT("A travelling vessel holds nothing"), CosmicPetitions::StandingHeld(*State, Index, Tables, Rules).IsEmpty());
	TestFalse(TEXT("and gets no card"), CosmicPetitions::Present(*State, Index, Tables, Rules, {}, true));
	State->Player.TravelTargetSystemID = NAME_None;
	State->Player.TravelTicksRemaining = 0;
	for (const FCosmicPetitionUnit& Unit : CosmicPetitions::StandingHeld(*State, Index, Tables, Rules))
	{
		TestFalse(TEXT("A ruler's petition is never a wanderer's unit"), Unit.Row == PlaceRow);
	}

	// 3. The arrival card: only on an arrival, by the share, and the pick reaches both rows; a wild system takes one too.
	State->Player.CurrentSystemID = State->Systems[7].SystemID;
	TestFalse(TEXT("Without an arrival no arrival card"), CosmicPetitions::Present(*State, Index, Tables, Rules, {}, false));
	TestTrue(TEXT("At the share 1 an arrival brings one, on a wild system too"), CosmicPetitions::Present(*State, Index, Tables, Rules, {}, true));
	CosmicPetitions::Clear(*State);
	Sim->SetTables(TablesWith(Original, Cards(), [](FCosmicRulesDef& Tuned) { Tuned.EncounterChancePerArrival = 0.f; }));
	TestFalse(TEXT("At the share 0 never"), CosmicPetitions::Present(*State, Index, Sim->GetTables(), Sim->GetRules(), {}, true));
	Sim->SetTables(TablesWith(Original, Cards(), [](FCosmicRulesDef& Tuned) { Tuned.EncounterChancePerArrival = 0.25f; }));
	{
		int32 Agreed = 0;
		int32 Came = 0;
		int32 Missed = 0;
		int32 A = 0;
		int32 B = 0;
		const int64 TickBefore = State->Tick;
		for (int32 Tick = 1; Tick <= 200; ++Tick)
		{
			State->Tick = Tick;
			const double Unit = static_cast<double>(CosmicSeed::DeriveForName(State->Seed, State->Systems[7].SystemID, CosmicSeed::Purpose::Encounter, static_cast<uint32>(Tick))) / 4294967296.0;
			const bool bPut = CosmicPetitions::Present(*State, Index, Sim->GetTables(), Sim->GetRules(), {}, true);
			Agreed += bPut == (Unit < 0.25) ? 1 : 0;
			Came += bPut ? 1 : 0;
			Missed += bPut ? 0 : 1;
			A += State->Knowing.PendingPetitionRow == ArriveA ? 1 : 0;
			B += State->Knowing.PendingPetitionRow == ArriveB ? 1 : 0;
			CosmicPetitions::Clear(*State);
		}
		// Frozen values (header section 6): seed 42, sys_7, purpose 11 and the share 0.25 - computed once from the
		// expression of docs/20 section 9.2. A renumbered purpose moves the loop above with the code, not these.
		TestEqual(TEXT("(the board's system 7 is sys_7)"), State->Systems[7].SystemID, FName(TEXT("sys_7")));
		const TPair<int32, FName> Frozen[] = { { 1, NAME_None }, { 2, NAME_None }, { 3, ArriveA }, { 22, ArriveB } };
		for (const TPair<int32, FName>& Row : Frozen)
		{
			State->Tick = Row.Key;
			CosmicPetitions::Present(*State, Index, Sim->GetTables(), Sim->GetRules(), {}, true);
			TestEqual(*FString::Printf(TEXT("An arrival at sys_7 in tick %d brings %s"), Row.Key, *Row.Value.ToString()), State->Knowing.PendingPetitionRow, Row.Value);
			CosmicPetitions::Clear(*State);
		}
		State->Tick = TickBefore;
		TestEqual(TEXT("An arrival card comes exactly when the derived value of the place and the tick is below the share"), Agreed, 200);
		TestTrue(TEXT("(both cases occur)"), Came > 0 && Missed > 0);
		TestTrue(TEXT("The pick reaches both arrival rows"), A > 0 && B > 0);
	}

	// 4. The ruler's last choices count petitions only: after a wanderer's card, the periodic slot still passes over the
	// most recent periodic petition.
	{
		FGalaxyState* Ruled = LayBoard(*Sim, TablesWith(Original, Cards(), [](FCosmicRulesDef& Tuned) { Tuned.PetitionPeriodTicks = 10; }));
		if (!TestNotNull(TEXT("The board of the ruler is laid"), Ruled))
		{
			return false;
		}
		Ruled->Calling.Lives.Last().LastChoiceRows = { LotA, LotB, ArriveA };
		int32 Again = 0;
		int32 Put = 0;
		for (int32 Slot = 1; Slot <= 20; ++Slot)
		{
			Ruled->Tick = Ruled->Calling.BornTick + Slot * 10;
			if (CosmicPetitions::Present(*Ruled, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), {}, false))
			{
				++Put;
				Again += Ruled->Knowing.PendingPetitionRow == LotB ? 1 : 0;
			}
			CosmicPetitions::Clear(*Ruled);
		}
		TestEqual(TEXT("The periodic slot puts a card"), Put, 20);
		TestEqual(TEXT("and never the most recent periodic petition, a wanderer's card after it or not"), Again, 0);
	}

	// 5. The cards change no world: the same journey with the wanderer's rows and without gives the same world.
	{
		TArray<uint32> Worlds;
		for (const bool bRows : { true, false })
		{
			FGalaxyState* Run = LayBoard(*Sim, TablesWith(Original, Cards(4, bRows), [](FCosmicRulesDef& Tuned) { Tuned.EncounterChancePerArrival = 1.f; }));
			if (!TestNotNull(TEXT("The board of the journey is laid"), Run))
			{
				return false;
			}
			Walk(*Sim, *Role, 4);
			Plague(*Run, 5, 400);
			for (const int32 Next : { 5, 6, 5, 4, 3 })
			{
				CosmicPetitions::Clear(*Run);
				Role->TravelTo(Run->Systems[Next].SystemID);
				Sim->RunHeadless(Sim->GetRules().LaneTravelTicks + 5);
			}
			Worlds.Add(Sim->ComputeWorldHash());
		}
		TestEqual(TEXT("A wanderer's cards that wait change no world"), static_cast<int64>(Worlds[0]), static_cast<int64>(Worlds[1]));
	}

	// 6. The card's people is met before the card names it: a plague that comes to hold in the tick under a vessel that
	// stands still, its owner not met, on a tick the sight does not look.
	{
		UDataTable* Loud = QuietHazards(Original);
		FHazardDef* PlagueRow = Loud->FindRow<FHazardDef>(FName(TEXT("crisis_plague")), TEXT("EncounterPresent"));
		if (!TestNotNull(TEXT("(the game has crisis_plague)"), PlagueRow))
		{
			return false;
		}
		PlagueRow->SpawnChancePerTick = 1.f;
		FCosmicDataTables LoudTables = TablesWith(Original, Cards(), [](FCosmicRulesDef& Tuned)
		{
			Tuned.SightIntervalTicks = 100000;
			Tuned.EncounterChancePerArrival = 0.f;
		});
		LoudTables.Hazards = Loud;
		FGalaxyState* Still = LayBoard(*Sim, LoudTables);
		if (!TestNotNull(TEXT("The board of the still vessel is laid"), Still))
		{
			return false;
		}
		Walk(*Sim, *Role, 5);
		const FName People1 = Still->Civs[1].CivID;
		Still->Knowing.MetCivs.RemoveAll([People1](const FKnownMark& Mark) { return Mark.ID == People1; });
		for (int32 Step = 0; Step < 50 && !Still->Systems[5].ActiveHazard.IsValid(); ++Step)
		{
			Sim->StepOnce();
		}
		TestTrue(TEXT("(a plague came to hold at 5)"), Still->Systems[5].ActiveHazard == Tag(TEXT("Hazard.Plague")));
		TestEqual(TEXT("A plague that came to hold under the vessel is its card"), Still->Knowing.PendingPetitionRow, Tend);
		TestTrue(TEXT("and the card's people is met with it"), HasMet(*Still, People1));
	}
	return true;
}

// The wanderer's answer (B1 header section 2.6): one number, the vessel's stat, the reverence of the system's people, the
// card withdrawn when the vessel left, the other role refused.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimEncounterAnswerTest, "Cosmic_Ascension.Sim.EncounterAnswer", CosmicTests::Flags)
bool FCosmicSimEncounterAnswerTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWandererTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	// A wanderer standing at 5 (people 1's) beside a plague, its card waiting, the check Mind 1 + roll >= Difficulty.
	auto Setup = [&](int32 Difficulty) -> FGalaxyState*
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Cards(Difficulty)));
		if (!State)
		{
			return nullptr;
		}
		Walk(*Sim, *Role, 4);
		State->Player.CurrentSystemID = State->Systems[5].SystemID;
		Plague(*State, 5);
		State->Player.Mind = 1;
		State->Knowing.PendingPetitionRow = Tend;
		State->Knowing.PendingPetitionSystemID = State->Systems[5].SystemID;
		return State;
	};

	// Passing on the boundary: Mind 1 + roll == Difficulty.
	{
		FGalaxyState* Probe = Setup(4);
		if (!TestNotNull(TEXT("The board is laid"), Probe))
		{
			return false;
		}
		const int32 Roll = NextRoll(Sim->GetRng());
		FGalaxyState* State = Setup(1 + Roll);
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		const float Former = State->Civs[0].ReverenceToPlayer;
		const FCosmicPetitionAnswer Answer = Role->AnswerPetition(0);
		TestTrue(TEXT("Mind 1 and the roll reach the difficulty: it passes"), Answer.Outcome == ECosmicPetitionOutcome::Succeeded);
		TestEqual(TEXT("One number of the sim stream for the answer"), Sim->GetRng().GetCurrentSeed(), SeedAfterDraws(FRandomStream(Seed), 1));
		TestEqual(TEXT("The reverence of the system's people rises"), State->Civs[1].ReverenceToPlayer, 0.3f);
		TestEqual(TEXT("not the people the vessel left"), State->Civs[0].ReverenceToPlayer, Former);
		TestFalse(TEXT("The card's success ended the plague"), State->Systems[5].ActiveHazard.IsValid());
		TestTrue(TEXT("The card is taken away"), State->Knowing.PendingPetitionRow.IsNone());
		TestEqual(TEXT("and remembered in the life"), State->Calling.Lives.Last().LastChoiceRows.Last(), Tend);
	}
	// One short: it fails.
	{
		FGalaxyState* Probe = Setup(4);
		const int32 Roll = NextRoll(Sim->GetRng());
		FGalaxyState* State = Setup(2 + Roll);
		if (!TestNotNull(TEXT("The board is laid"), Probe) || !State)
		{
			return false;
		}
		TestTrue(TEXT("One short of the difficulty: it fails"), Role->AnswerPetition(0).Outcome == ECosmicPetitionOutcome::Failed);
		TestEqual(TEXT("and the people's reverence stays"), State->Civs[1].ReverenceToPlayer, 0.f);
	}
	// The vessel left: withdrawn, one number, nothing else - also when it stands beside the same plague in another system of
	// the same people, where the card would hold but for the place.
	{
		FGalaxyState* State = Setup(4);
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		Plague(*State, 6);
		State->Player.CurrentSystemID = State->Systems[6].SystemID;
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		TestTrue(TEXT("A card whose system the vessel left is withdrawn"), Role->AnswerPetition(0).Outcome == ECosmicPetitionOutcome::Withdrawn);
		TestEqual(TEXT("with one number"), Sim->GetRng().GetCurrentSeed(), SeedAfterDraws(FRandomStream(Seed), 1));
		TestEqual(TEXT("and no reverence"), State->Civs[1].ReverenceToPlayer, 0.f);
	}
	// The other role: refused, nothing drawn.
	{
		FGalaxyState* State = Setup(4);
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		State->Knowing.PendingPetitionRow = PlaceRow;
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		TestTrue(TEXT("A wanderer does not answer a petition"), Role->AnswerPetition(0).Outcome == ECosmicPetitionOutcome::Refused);
		TestEqual(TEXT("(nothing drawn)"), Sim->GetRng().GetCurrentSeed(), Seed);
		FGalaxyState* Ruled = LayBoard(*Sim, TablesWith(Original, Cards()));
		Ruled->Knowing.PendingPetitionRow = Tend;
		Ruled->Knowing.PendingPetitionSystemID = Ruled->Systems[0].SystemID;
		TestTrue(TEXT("A ruler does not answer a wanderer's card"), Role->AnswerPetition(0).Outcome == ECosmicPetitionOutcome::Refused);
	}
	// A wild system's arrival card: the vessel's stat, no people, no error line.
	{
		FGalaxyState* State = Setup(4);
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		State->Player.CurrentSystemID = State->Systems[7].SystemID;
		State->Knowing.PendingPetitionRow = ArriveA;
		State->Knowing.PendingPetitionSystemID = State->Systems[7].SystemID;
		const int32 Body = State->Player.Body;
		TestTrue(TEXT("The arrival card on a wild system is answered"), Role->AnswerPetition(0).Outcome == ECosmicPetitionOutcome::Succeeded);
		TestEqual(TEXT("and the vessel's Body rises"), State->Player.Body, Body + 1);
	}
	return true;
}

// The coronation and the return (B1 header section 2.7): the reasons at their bounds, the lock on every people, the
// return by the lock alone, what the throne writes, and the petitions that stood when the vessel was crowned.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimCoronationTest, "Cosmic_Ascension.Sim.Coronation", CosmicTests::Flags)
bool FCosmicSimCoronationTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWandererTests;
	using CosmicWanderer::ECrownBlock;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Cards()));
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicRulesDef& Rules = Sim->GetRules();
	const FName People1 = State->Civs[1].CivID;
	TestTrue(TEXT("A ruler is not crowned"), CosmicWanderer::CrownBlockOf(*State, Index, Rules, People1, true, false) == ECrownBlock::NotWanderer);

	// The deposition locks every people's throne, not only the people left.
	State->Player.CurrentSystemID = State->Systems[4].SystemID;
	Role->DeposeRuler();
	const int64 Until = State->Tick + FMath::Max(0, Rules.ThroneLockTicks);
	bool bEveryLocked = true;
	for (const FCivilizationState& Civ : State->Civs)
	{
		bEveryLocked &= Civ.ThroneLockedUntilTick >= Until;
	}
	TestTrue(TEXT("A deposition locks every people's throne"), bEveryLocked);
	State->Player.CurrentSystemID = State->Systems[5].SystemID;
	Meet(*State, 1);
	State->Civs[1].ReverenceToPlayer = 0.6f;
	TestTrue(TEXT("A locked throne refuses"), CosmicWanderer::CrownBlockOf(*State, Index, Rules, People1, true, false) == ECrownBlock::ThroneLocked);
	State->Tick = State->Civs[1].ThroneLockedUntilTick - 1;
	TestTrue(TEXT("one tick before the lock ends"), CosmicWanderer::CrownBlockOf(*State, Index, Rules, People1, true, false) == ECrownBlock::ThroneLocked);
	State->Tick = State->Civs[1].ThroneLockedUntilTick;

	// The bounds of the two ways up.
	State->Civs[1].Stability = 0.5f;
	TestTrue(TEXT("Reverence 0.6 crowns"), CosmicWanderer::CrownBlockOf(*State, Index, Rules, People1, true, false) == ECrownBlock::None);
	State->Civs[1].ReverenceToPlayer = 0.599f;
	TestTrue(TEXT("0.599 does not"), CosmicWanderer::CrownBlockOf(*State, Index, Rules, People1, true, false) == ECrownBlock::Lacking);
	State->Civs[1].Stability = 0.2f;
	TestTrue(TEXT("Stability 0.2 is no vacuum"), CosmicWanderer::CrownBlockOf(*State, Index, Rules, People1, true, false) == ECrownBlock::Lacking);
	State->Civs[1].Stability = 0.199f;
	TestTrue(TEXT("0.199 is"), CosmicWanderer::CrownBlockOf(*State, Index, Rules, People1, true, false) == ECrownBlock::None);
	TestTrue(TEXT("An unmet people crowns no one"), CosmicWanderer::CrownBlockOf(*State, Index, Rules, People1, false, false) == ECrownBlock::Unmet);
	TestTrue(TEXT("A waiting card comes first"), CosmicWanderer::CrownBlockOf(*State, Index, Rules, People1, true, true) == ECrownBlock::CardWaiting);
	TestTrue(TEXT("Another people's throne is not where the vessel stands"), CosmicWanderer::CrownBlockOf(*State, Index, Rules, State->Civs[2].CivID, true, false) == ECrownBlock::NotStanding);
	State->Systems[5].ActiveMegaproject = CosmicRows::ProjectSettle;
	TestTrue(TEXT("A system being settled is no seat"), CosmicWanderer::CrownBlockOf(*State, Index, Rules, People1, true, false) == ECrownBlock::NotSettled);
	State->Systems[5].ActiveMegaproject = NAME_None;

	// The return: the people the vessel left takes it back by the lock alone.
	{
		const FName People0 = State->Civs[0].CivID;
		const FName Stand = State->Player.CurrentSystemID;
		State->Player.CurrentSystemID = State->Systems[0].SystemID;
		State->Civs[0].ReverenceToPlayer = 0.f;
		State->Civs[0].Stability = 0.9f;
		TestTrue(TEXT("(the vessel left people 0)"), State->Player.FormerCivID == People0);
		TestTrue(TEXT("The return needs no reverence"), CosmicWanderer::CrownBlockOf(*State, Index, Rules, People0, true, false) == ECrownBlock::None);
		const int64 Now = State->Tick;
		State->Tick = State->Civs[0].ThroneLockedUntilTick - 1;
		TestTrue(TEXT("only the lock"), CosmicWanderer::CrownBlockOf(*State, Index, Rules, People0, true, false) == ECrownBlock::ThroneLocked);
		State->Tick = Now;
		State->Player.CurrentSystemID = Stand;
	}

	// The command reads the meeting itself (rule 2: the input's definition): a people not met is refused, nothing written.
	{
		State->Knowing.MetCivs.RemoveAll([People1](const FKnownMark& Mark) { return Mark.ID == People1; });
		State->Civs[1].Stability = 0.15f;
		const uint32 Before = Sim->ComputeStateHash();
		TestTrue(TEXT("The command refuses a people the vessel has not met"), Role->BecomeRuler(People1) == ECrownBlock::Unmet);
		TestEqual(TEXT("and writes nothing"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Before));
		Meet(*State, 1);
	}

	// Crowned: what the throne writes. People 1 also holds 9, which the vessel does not know yet; a rebel took its throne
	// before (an open Ruler record), its Stability is that of a vacuum, and it stands on the third rung (the life's peak
	// must not rise with it: the provisional end reads the peak).
	CosmicTests::Give(*State, 1, { 9 });
	FNotableRecord& Rebel = State->Civs[1].Notables.AddDefaulted_GetRef();
	Rebel.Role = Tag(TEXT("Notable.Ruler"));
	Rebel.FirstTick = 1;
	State->Civs[1].Stability = 0.15f;
	State->Civs[1].Tier = EAscensionTier::Tier3_Causal;
	TestFalse(TEXT("(9 is not known)"), Knows(*State, 9));
	const int32 Seed = Sim->GetRng().GetCurrentSeed();
	const EAscensionTier Peak = State->Calling.Lives.Last().PeakTier;
	TestTrue(TEXT("The coronation is taken"), Role->BecomeRuler(People1) == ECrownBlock::None);
	const FLifeRecord& Life = State->Calling.Lives.Last();
	TestTrue(TEXT("The vessel rules people 1"), State->Player.Role == EPlayerRole::Ruler && State->Player.RuledCivID == People1 && State->Civs[1].bIsPlayerRuled);
	TestTrue(TEXT("The three reign arrays grow together"), Life.RuledCivIDs.Num() == Life.ReignEndReasons.Num() && Life.ReignTicks.Num() == Life.RuledCivIDs.Num()
		&& Life.RuledCivIDs.Last() == People1 && !Life.ReignEndReasons.Last().IsValid() && Life.ReignTicks.Last() == 0);
	TestEqual(TEXT("The rebel's throne is closed"), OpenRecords(State->Civs[1], TEXT("Notable.Ruler")), 0);
	TestTrue(TEXT("The new people's systems are known at once"), Knows(*State, 9));
	TestEqual(TEXT("No number of the sim stream"), Sim->GetRng().GetCurrentSeed(), Seed);
	TestTrue(TEXT("The life's peak stays"), State->Calling.Lives.Last().PeakTier == Peak);
	TestTrue(TEXT("and no provisional end comes of the people's rung"), CosmicEndings::ProvisionalEndOf(*State, Rules) != ECosmicProvisionalEnd::Rung);
	TestEqual(TEXT("The coronation is told"), LinesOf(*State, CosmicRows::LogCrowned), 1);
	TestTrue(TEXT("The vessel left people 0 and keeps it in mind"), State->Player.FormerCivID == State->Civs[0].CivID);
	Sim->StepOnce();
	TestEqual(TEXT("The first tick of the reign brings the petition that already stood (low Stability)"), State->Knowing.PendingPetitionRow, LowRow);

	// Inside a handler: an error line, nothing.
	{
		FGalaxyState* Again = LayBoard(*Sim, TablesWith(Original, Cards()));
		Walk(*Sim, *Role, 5);
		Meet(*Again, 1);
		Again->Civs[1].ReverenceToPlayer = 0.9f;
		CosmicTests::FScopedProbe Probe(*Sim);
		ECrownBlock Tried = ECrownBlock::None;
		bool bTried = false;
		Probe.Probe->OnLog = [&](const FCosmicLogEntry&)
		{
			if (!bTried)
			{
				bTried = true;
				Tried = Role->BecomeRuler(Again->Civs[1].CivID);
			}
		};
		AddExpectedError(TEXT("BecomeRuler called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>()->Push(Tag(TEXT("Log.Civ")), 1, FText::AsCultureInvariant(TEXT("a line.")), FFormatNamedArguments(), NAME_None, NAME_None);
		TestTrue(TEXT("The coronation inside a handler is refused"), Tried == ECrownBlock::Handler);
		TestTrue(TEXT("and the vessel still walks"), Again->Player.Role == EPlayerRole::Wanderer);
	}
	return true;
}

// The abdication (B1 header section 2.8): the command records, the tenth stage leaves; two numbers, the regent's record,
// reverence kept; the throne card's last choice the same way; the script never takes it; a rebellion that ends with it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimAbdicationTest, "Cosmic_Ascension.Sim.Abdication", CosmicTests::Flags)
bool FCosmicSimAbdicationTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWandererTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FGameplayTag Abdicated = Tag(TEXT("Reign.Abdicated"));

	// 1. The command records; the tenth stage of the next tick leaves.
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Cards()));
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		State->Civs[0].ReverenceToPlayer = 0.4f;
		const float Stability = State->Civs[0].Stability;
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		TestTrue(TEXT("The abdication is recorded"), Role->Abdicate());
		TestEqual(TEXT("without a draw"), Sim->GetRng().GetCurrentSeed(), Seed);
		TestTrue(TEXT("as the end of the reign by the vessel's will"), State->Calling.PendingEffect == Tag(TEXT("Effect.DeposeRuler")) && State->Calling.PendingCause == Abdicated);
		TestEqual(TEXT("from the line of the empty throne"), State->Calling.PendingSourceRow, CosmicRows::LogThroneVacated);
		TestTrue(TEXT("The vessel still rules until the tick ends"), State->Player.Role == EPlayerRole::Ruler);
		TestFalse(TEXT("A second abdication is not recorded"), Role->Abdicate());
		Sim->StepOnce();
		TestTrue(TEXT("Then it walks"), State->Player.Role == EPlayerRole::Wanderer);
		TestTrue(TEXT("having left of its own will"), State->Player.StatusTags.HasTagExact(Tag(TEXT("Status.Abdicated"))));
		TestTrue(TEXT("The reign ended by abdication"), State->Calling.Lives.Last().ReignEndReasons.Last() == Abdicated);
		TestEqual(TEXT("A regent holds the throne"), OpenRecords(State->Civs[0], TEXT("Notable.Regent")), 1);
		const FNotableRecord* Regent = State->Civs[0].Notables.FindByPredicate([](const FNotableRecord& Record) { return Record.Role == Tag(TEXT("Notable.Regent")); });
		TestTrue(TEXT("named by the line of the empty throne"), Regent && Regent->SourceRow == CosmicRows::LogThroneVacated);
		TestEqual(TEXT("The people keeps its reverence"), State->Civs[0].ReverenceToPlayer, 0.4f);
		TestTrue(TEXT("and loses a little of its calm"), FMath::IsNearlyEqual(State->Civs[0].Stability, FMath::Max(0.f, Stability - Sim->GetRules().AbdicationStabilityLoss), 0.002f));
		bool bEveryLocked = true;
		for (const FCivilizationState& Civ : State->Civs)
		{
			bEveryLocked &= Civ.ThroneLockedUntilTick >= State->Tick + Sim->GetRules().ThroneLockTicks;
		}
		TestTrue(TEXT("Every throne is locked"), bEveryLocked);
		TestEqual(TEXT("The empty throne is told"), LinesOf(*State, CosmicRows::LogThroneVacated), 1);

		// The return through the command (section 2.7): after the lock the people left takes the vessel back - the regent
		// leaves the throne, and the return is told as a return.
		State->Tick = State->Civs[0].ThroneLockedUntilTick;
		TestTrue(TEXT("After the lock the vessel returns"), Role->BecomeRuler(State->Civs[0].CivID) == CosmicWanderer::ECrownBlock::None);
		TestEqual(TEXT("The regent leaves the throne"), OpenRecords(State->Civs[0], TEXT("Notable.Regent")), 0);
		TestEqual(TEXT("The return is told as a return"), LinesOf(*State, CosmicRows::LogReturned), 1);
		TestEqual(TEXT("not as a coronation"), LinesOf(*State, CosmicRows::LogCrowned), 0);
	}

	// 2. Two numbers, always - with a regent named, and for a people that names no one (the fourth rung).
	for (const EAscensionTier Tier : { EAscensionTier::Tier1_Planetary, EAscensionTier::Tier4_Transcendent })
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Cards()));
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		State->Civs[0].Tier = Tier;
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		Role->AbdicateRuler();
		TestEqual(TEXT("The pipeline draws exactly two numbers"), Sim->GetRng().GetCurrentSeed(), SeedAfterDraws(FRandomStream(Seed), 2));
		TestEqual(TEXT("and names a regent only below the fourth rung"), OpenRecords(State->Civs[0], TEXT("Notable.Regent")), Tier == EAscensionTier::Tier1_Planetary ? 1 : 0);
	}

	// 3. The throne card's last choice: the same pipeline, recorded by the answer.
	const FName Throne(TEXT("petition_keep_throne"));
	const FEncounterDef* ThroneRow = Original.FindEncounter(Throne);
	if (TestNotNull(TEXT("The game has the throne card"), ThroneRow) && TestEqual(TEXT("with three choices"), ThroneRow->Choices.Num(), 3))
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Original.Encounters));
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		State->Systems[0].ActiveHazard = Tag(TEXT("Hazard.Rebellion"));
		State->Systems[0].HazardTicksRemaining = 40;
		State->Knowing.PendingPetitionRow = Throne;
		State->Knowing.PendingPetitionSystemID = State->Systems[0].SystemID;
		TestTrue(TEXT("The last choice is taken"), Role->AnswerPetition(2).Outcome == ECosmicPetitionOutcome::Succeeded);
		TestTrue(TEXT("and records the abdication"), State->Calling.PendingCause == Abdicated && State->Calling.PendingSourceRow == Throne);
		Sim->StepOnce();
		TestTrue(TEXT("which the tenth stage carries out"), State->Player.Role == EPlayerRole::Wanderer && OpenRecords(State->Civs[0], TEXT("Notable.Regent")) == 1);
		// The rebellion that stood outlives the reign: when it ends the rebels' leader rules, not the regent.
		FNotableRecord& Leader = State->Civs[0].Notables.AddDefaulted_GetRef();
		Leader.Role = Tag(TEXT("Notable.Rebel"));
		Leader.FirstTick = State->Tick;
		State->Systems[0].HazardTicksRemaining = 1;
		Sim->StepOnce();
		TestEqual(TEXT("Under a regent a rebellion that wins closes the regent"), OpenRecords(State->Civs[0], TEXT("Notable.Regent")), 0);
		TestEqual(TEXT("and its leader rules"), OpenRecords(State->Civs[0], TEXT("Notable.Ruler")), 1);

		// The script turns through the other two only.
		bool bLeaves = false;
		FGalaxyState* Ruled = LayBoard(*Sim, TablesWith(Original, Original.Encounters));
		for (int32 Answers = 0; Answers < 6; ++Answers)
		{
			bLeaves |= CosmicRulerPolicy::ChoiceOf(*Ruled, Sim->GetStateIndex(), *ThroneRow, Answers) == 2;
		}
		TestFalse(TEXT("The ruler's script never leaves the throne"), bLeaves);
	}

	// 4. A rebellion that expires on the tick after the abdication was recorded: the throne does not go to the rebels.
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Cards()));
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		State->Systems[0].ActiveHazard = Tag(TEXT("Hazard.Rebellion"));
		State->Systems[0].HazardTicksRemaining = 1;
		FNotableRecord& Leader = State->Civs[0].Notables.AddDefaulted_GetRef();
		Leader.Role = Tag(TEXT("Notable.Rebel"));
		Leader.FirstTick = 1;
		TestTrue(TEXT("The abdication is recorded with one tick of the rebellion left"), Role->Abdicate());
		Sim->StepOnce();
		TestEqual(TEXT("The rebels' record closes"), OpenRecords(State->Civs[0], TEXT("Notable.Rebel")), 0);
		TestEqual(TEXT("and no rebel rules"), OpenRecords(State->Civs[0], TEXT("Notable.Ruler")), 0);
		TestEqual(TEXT("the regent does"), OpenRecords(State->Civs[0], TEXT("Notable.Regent")), 1);
		TestEqual(TEXT("The expiry is told without the throne"), LinesOf(*State, CosmicRows::LogRebellionUnseated), 1);
	}

	// 5. Inside a handler: an error line, nothing.
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Cards()));
		CosmicTests::FScopedProbe Probe(*Sim);
		bool bTried = false;
		bool bTaken = true;
		Probe.Probe->OnLog = [&](const FCosmicLogEntry&)
		{
			if (!bTried)
			{
				bTried = true;
				bTaken = Role->Abdicate();
			}
		};
		AddExpectedError(TEXT("Abdicate called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>()->Push(Tag(TEXT("Log.Civ")), 1, FText::AsCultureInvariant(TEXT("a line.")), FFormatNamedArguments(), NAME_None, NAME_None);
		TestFalse(TEXT("The abdication inside a handler is refused"), bTaken);
		TestFalse(TEXT("and nothing is recorded"), State && State->Calling.PendingEffect.IsValid());
	}
	return true;
}

// The chronicle keeps the wanderer's story (B1 header section 2.9): a wanderer's card answered about another people, the
// prefix read without case; a petition about another people stays out.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLogChronicleWandererTest, "Cosmic_Ascension.Log.ChronicleWanderer", CosmicTests::Flags)
bool FCosmicLogChronicleWandererTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWandererTests;
	const TArray<FName> Peoples = { FName(TEXT("civ_0")) };
	FCosmicLogEntry Entry;
	Entry.Category = Tag(TEXT("Log.Civ"));
	Entry.Severity = 1;
	Entry.FocusCivID = FName(TEXT("civ_7"));
	Entry.SourceRow = FName(TEXT("enc_rebel_parley"));
	TestTrue(TEXT("A wanderer's card answered about another people is kept"), CosmicChronicle::Admits(Entry, Peoples));
	Entry.SourceRow = FName(TEXT("ENC_Rebel_Parley"));
	TestTrue(TEXT("whatever the case of its name"), CosmicChronicle::Admits(Entry, Peoples));
	Entry.SourceRow = FName(TEXT("petition_plague"));
	TestFalse(TEXT("A petition about another people is not"), CosmicChronicle::Admits(Entry, Peoples));
	Entry.SourceRow = FName(TEXT("enc_rebel_parley"));
	Entry.Severity = 0;
	TestFalse(TEXT("nor a Severity 0 line"), CosmicChronicle::Admits(Entry, Peoples));
	Entry.Severity = 2;
	Entry.Category = Tag(TEXT("Log.Calling"));
	Entry.SourceRow = CosmicRows::LogCrowned;
	TestTrue(TEXT("The coronation, the return and the empty throne are the calling's own lines"), CosmicChronicle::Admits(Entry, Peoples));
	return true;
}

// The save in the middle of the loop (B1 header section 2.10): a journey, a waiting wanderer's card and a recorded
// abdication come back from a save, and the same inputs give the same galaxy.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveTravelTest, "Cosmic_Ascension.Sim.SaveTravel", CosmicTests::Flags)
bool FCosmicSimSaveTravelTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWandererTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	UCosmicSaveSubsystem* Save = Sim ? Sim->GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role) || !TestNotNull(TEXT("The save subsystem exists"), Save))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	Sim->SetTables(TablesWith(Original, Cards(), [](FCosmicRulesDef& Rules) { Rules.EncounterChancePerArrival = 0.f; }));

	// After the save: the moves of a script, then a run.
	auto Script = [&](int32 Moment)
	{
		if (Moment == 0)
		{
			Sim->RunHeadless(30);
		}
		else if (Moment == 1)
		{
			Role->AnswerPetition(0);
			Role->TravelTo(Sim->GetState().Systems[6].SystemID);
			Sim->RunHeadless(40);
		}
		else
		{
			Sim->RunHeadless(10);
			Role->TravelTo(Sim->GetState().Systems[Sim->GetStateIndex().FindSystem(Sim->GetState(), Sim->GetState().Player.CurrentSystemID) == 4 ? 5 : 4].SystemID);
			Sim->RunHeadless(30);
		}
	};
	for (int32 Moment = 0; Moment < 3; ++Moment)
	{
		FGalaxyState* State = LayBoard(*Sim, Sim->GetTables());
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		if (Moment == 0)
		{
			Walk(*Sim, *Role, 4);
			Role->TravelTo(State->Systems[5].SystemID);
			Sim->RunHeadless(7);
		}
		else if (Moment == 1)
		{
			Walk(*Sim, *Role, 4);
			State->Player.CurrentSystemID = State->Systems[5].SystemID;
			Plague(*State, 5);
			State->Knowing.PendingPetitionRow = Tend;
			State->Knowing.PendingPetitionSystemID = State->Systems[5].SystemID;
		}
		else
		{
			Role->Abdicate();
		}
		const FGuid Galaxy = State->GalaxyID;
		const FPlayerEntityState Vessel = State->Player;
		const FCallingState Calling = State->Calling;
		if (!TestTrue(TEXT("(the galaxy is saved)"), Save->SaveCurrentGalaxy()))
		{
			return false;
		}
		Script(Moment);
		const uint32 Straight = Sim->ComputeStateHash();
		Sim->NewGalaxy(7, 5, 3);
		TestTrue(TEXT("The saved galaxy loads"), Save->LoadGalaxy(Galaxy));
		const FGalaxyState& Loaded = Sim->GetState();
		TestTrue(TEXT("with the journey as it was"), Loaded.Player.TravelTargetSystemID == Vessel.TravelTargetSystemID && Loaded.Player.TravelTicksRemaining == Vessel.TravelTicksRemaining);
		TestTrue(TEXT("the vessel's stats"), Loaded.Player.Body == Vessel.Body && Loaded.Player.Mind == Vessel.Mind);
		TestTrue(TEXT("and the recorded effect"), Loaded.Calling.PendingEffect == Calling.PendingEffect && Loaded.Calling.PendingCause == Calling.PendingCause);
		Script(Moment);
		TestEqual(FString::Printf(TEXT("Moment %d: the same inputs after the load give the same galaxy"), Moment), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Straight));
		Save->DeleteSave(Galaxy);
	}
	return true;
}

// The wanderer's loop through its policy (B1 header section 6 - the commandlet's -Abdicate=1200 -Wander=1 with the ruler's
// -Answer=1 -Build=1 -Focus=1): seed 42 leaves the throne at 1,200 and takes one again before tick 6,000; the same inputs
// give the same two hashes twice; no error line.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimWandererLoopTest, "Cosmic_Ascension.Sim.WandererLoop", CosmicTests::Flags)
bool FCosmicSimWandererLoopTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	ON_SCOPE_EXIT { Sim->NewGalaxy(42, 5, 3); };
	TArray<TPair<uint32, uint32>> Hashes;
	for (int32 Run = 0; Run < 2; ++Run)
	{
		Sim->NewGalaxy(42, 0, 0);
		const FCosmicRulerPolicy Ruler{ true, true, true };
		FCosmicRulerPolicyCounts RulerCounts;
		const FCosmicWandererPolicy Wanderer{ 1200, true };
		FCosmicWandererPolicyCounts Counts;
		for (int64 Done = 0; Done < 6000 && Counts.CrownedAt < 0; ++Done)
		{
			const FName CardBefore = Sim->GetState().Knowing.PendingPetitionRow;
			Sim->RunHeadless(1);
			CosmicWandererPolicy::AbdicateIfDue(Wanderer, *Sim, *Role, Counts);
			CosmicRulerPolicy::Act(Ruler, *Sim, *Role, CardBefore, RulerCounts);
			CosmicWandererPolicy::Act(Wanderer, *Sim, *Role, CardBefore, Counts);
		}
		TestEqual(TEXT("The vessel walks from the tick after the abdication"), Counts.LeftAt, static_cast<int64>(1201));
		TestTrue(TEXT("and takes a throne again before tick 6,000"), Counts.CrownedAt > Counts.LeftAt && Counts.CrownedAt < 6000);
		const FGalaxyState& Played = Sim->GetState();
		TestTrue(TEXT("It took back the throne of the people it left"), Counts.CrownedCivID == Played.Player.FormerCivID && FString(Counts.CrownedPath) == TEXT("return"));
		TestTrue(TEXT("a second reign after an abdication"), Played.Calling.Lives.Last().RuledCivIDs.Num() == 2 && Played.Calling.Lives.Last().RuledCivIDs.Last() == Counts.CrownedCivID
			&& Played.Calling.Lives.Last().ReignEndReasons[0] == FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Abdicated"))));
		const int32 JourneysThen = Counts.Journeys;
		for (int32 More = 0; More < 2; ++More)
		{
			const FName CardBefore = Played.Knowing.PendingPetitionRow;
			Sim->RunHeadless(1);
			CosmicWandererPolicy::AbdicateIfDue(Wanderer, *Sim, *Role, Counts);
			CosmicRulerPolicy::Act(Ruler, *Sim, *Role, CardBefore, RulerCounts);
			CosmicWandererPolicy::Act(Wanderer, *Sim, *Role, CardBefore, Counts);
		}
		TestTrue(TEXT("It rules on and walks no more"), Played.Player.Role == EPlayerRole::Ruler && Played.Player.RuledCivID == Counts.CrownedCivID && Counts.Journeys == JourneysThen);
		AddInfo(FString::Printf(TEXT("Seed 42: left at %lld, crowned at %lld over %s by %s, journeys %d, scans %d, cards %d, answers %d"),
			Counts.LeftAt, Counts.CrownedAt, *Counts.CrownedCivID.ToString(), Counts.CrownedPath, Counts.Journeys, Counts.Scans, Counts.Cards, Counts.Answers));
		Hashes.Emplace(Sim->ComputeStateHash(), Sim->ComputeWorldHash());
	}
	TestTrue(TEXT("The same inputs give the same state and world twice"), Hashes.Num() == 2 && Hashes[0] == Hashes[1]);
	return true;
}

// The wanderer's policy, rule by rule on the board (B1 header section 5): the first lane neighbour with a crisis standing,
// not straight back unless it must; one scan on each arrival; the card's choices in turn; no input after the provisional end.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimWandererPolicyTest, "Cosmic_Ascension.Sim.WandererPolicy", CosmicTests::Flags)
bool FCosmicSimWandererPolicyTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWandererTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FCosmicWandererPolicy Walks{ 0, true };

	// The travel choice: from 4 (lanes 3, 5, 10) to 5, where a plague stands; not back to 5 just left - 3 instead.
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Cards(), [](FCosmicRulesDef& Rules) { Rules.EncounterChancePerArrival = 0.f; }));
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		Walk(*Sim, *Role, 4);
		Plague(*State, 5);
		FCosmicWandererPolicyCounts Counts;
		CosmicWandererPolicy::Act(Walks, *Sim, *Role, NAME_None, Counts);
		TestEqual(TEXT("The vessel sets off to the neighbour with a crisis"), State->Player.TravelTargetSystemID, State->Systems[5].SystemID);
		TestEqual(TEXT("having scanned where it stood"), Counts.Scans, 1);
		State->Player.TravelTargetSystemID = NAME_None;
		State->Player.TravelTicksRemaining = 0;
		FCosmicWandererPolicyCounts Back;
		Back.LastLeft = State->Systems[5].SystemID;
		Back.ScannedAt = State->Systems[4].SystemID;
		CosmicWandererPolicy::Act(Walks, *Sim, *Role, NAME_None, Back);
		TestEqual(TEXT("Not straight back to the system it left: the first other open neighbour"), State->Player.TravelTargetSystemID, State->Systems[3].SystemID);
		TestEqual(TEXT("and no second scan of the same arrival"), Back.Scans, 0);
	}

	// One scan on each arrival: a vessel with no open lane stays and scans once.
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Cards()));
		Walk(*Sim, *Role, 4);
		State->Systems[4].Lanes.Reset();
		FCosmicWandererPolicyCounts Counts;
		CosmicWandererPolicy::Act(Walks, *Sim, *Role, NAME_None, Counts);
		CosmicWandererPolicy::Act(Walks, *Sim, *Role, NAME_None, Counts);
		TestTrue(TEXT("Stranded, it scans once and goes nowhere"), Counts.Scans == 1 && Counts.Journeys == 0 && !CosmicWanderer::IsTravelling(*State));
	}

	// The card's choices in turn, from the count of answers: the second answer takes the second choice.
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Cards()));
		Walk(*Sim, *Role, 4);
		State->Player.CurrentSystemID = State->Systems[7].SystemID;
		State->Knowing.PendingPetitionRow = ArriveA;
		State->Knowing.PendingPetitionSystemID = State->Systems[7].SystemID;
		const int32 Body = State->Player.Body;
		FCosmicWandererPolicyCounts Counts;
		Counts.Answers = 1;
		CosmicWandererPolicy::Act(Walks, *Sim, *Role, ArriveA, Counts);
		TestTrue(TEXT("The second answer takes the second choice (no Body)"), State->Knowing.PendingPetitionRow.IsNone() && State->Player.Body == Body && Counts.Answers == 2);
	}

	// No input after the provisional end: no abdication recorded, no scan, no journey.
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, Cards()));
		State->Calling.Lives.Last().PeakTier = EAscensionTier::Tier3_Causal;
		State->Tick = 10;
		FCosmicWandererPolicyCounts Counts;
		CosmicWandererPolicy::AbdicateIfDue(FCosmicWandererPolicy{ 10, true }, *Sim, *Role, Counts);
		TestFalse(TEXT("After the end the policy records no abdication"), State->Calling.PendingEffect.IsValid());
		Walk(*Sim, *Role, 4);
		FCosmicWandererPolicyCounts Walking;
		CosmicWandererPolicy::Act(Walks, *Sim, *Role, NAME_None, Walking);
		TestTrue(TEXT("and the wanderer neither scans nor travels"), Walking.bEnded && Walking.Scans == 0 && Walking.Journeys == 0 && !CosmicWanderer::IsTravelling(*State));
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

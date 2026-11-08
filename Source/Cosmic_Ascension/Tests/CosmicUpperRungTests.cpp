// Cosmic Ascension - automation tests of the fourth and fifth rungs (B6b): from B-6b1 the two saved counters of a people
// (LawsHeldTicks, PressureEnduredTicks) through the save and the hashes; from B-6b2a the count of the held laws, the two hooks
// of a galaxy law, the AI's law and treaty steps and the vocabulary of the endured era (the condition's window, the selector
// word Ruled, the Key Borders). The count of the endured pressure is Sim.PressureEndured (Tests/CosmicBookTests.cpp, beside
// the fallen-sphere counter it is counted with).
// Design: docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md section 2.1, 2.3, 2.4, 2.7, 6; docs/20 section 9.1.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Containers/StringConv.h"
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Events/CosmicEventSubsystem.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "Misc/ScopeExit.h"
#include "Save/CosmicSaveGame.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/Function.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/Package.h"

namespace CosmicUpperRungTests
{
	// The engine's save bytes of a state (the carrier the game's slot wraps - Save/CosmicSaveSubsystem.cpp).
	TArray<uint8> SaveBytesOf(const FGalaxyState& State)
	{
		UCosmicSaveGame* Carrier = NewObject<UCosmicSaveGame>(GetTransientPackage());
		Carrier->State = State;
		TArray<uint8> Raw;
		UGameplayStatics::SaveGameToMemory(Carrier, Raw);
		return Raw;
	}

	// Renames every saved member called Name to a name no build knows (its last letter changed, so the bytes keep their
	// length): what a save made before the member was added reads like. Returns how many it renamed.
	int32 Forget(TArray<uint8>& Raw, const TCHAR* Name)
	{
		const auto Ansi = StringCast<ANSICHAR>(Name);
		int32 Renamed = 0;
		for (int32 At = 0; At + Ansi.Length() <= Raw.Num(); ++At)
		{
			if (FMemory::Memcmp(Raw.GetData() + At, Ansi.Get(), Ansi.Length()) == 0)
			{
				Raw[At + Ansi.Length() - 1] = static_cast<uint8>('Q');
				++Renamed;
			}
		}
		return Renamed;
	}
}

// The two counters through the save (header section 2.1, 6): their initializers are the frozen 0 (CLAUDE.md rule 7 -
// mutation: an initializer of 1 breaks it); off 0 they round trip civilization by civilization through the game's slot with
// the hash they were saved with; and a save without them - written before B-6b1 - loads them as 0 while the rest of each
// civilization comes back (an element of Civs is written whole, so the old bytes simply lack the two members).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveLawsHeldTest, "Cosmic_Ascension.Sim.SaveLawsHeld", CosmicTests::Flags)
bool FCosmicSimSaveLawsHeldTest::RunTest(const FString& Parameters)
{
	using namespace CosmicUpperRungTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicSaveSubsystem* Save = Sim->GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>();
	if (!TestNotNull(TEXT("The save subsystem exists"), Save))
	{
		return false;
	}
	FGuid Galaxy;
	ON_SCOPE_EXIT
	{
		if (Galaxy.IsValid())
		{
			Save->DeleteSave(Galaxy);
		}
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};

	// (1) The frozen initializers.
	const FCivilizationState Fresh;
	TestEqual(TEXT("The frozen LawsHeldTicks is 0"), Fresh.LawsHeldTicks, 0);
	TestEqual(TEXT("The frozen PressureEnduredTicks is 0"), Fresh.PressureEnduredTicks, 0);

	// (2) Off 0, both round trip through the game's slot, each on its own people, with the hash.
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(200);
	FGalaxyState& Live = Sim->GetMutableState();
	Galaxy = Live.GalaxyID;
	if (!TestTrue(TEXT("The slice has three peoples"), Live.Civs.Num() >= 3))
	{
		return false;
	}
	Live.Civs[1].LawsHeldTicks = 321;
	Live.Civs[2].PressureEnduredTicks = 654;
	Live.Civs[2].FallenSphereTicks = 77;
	const uint32 Hash = Sim->ComputeStateHash();
	const uint32 World = Sim->ComputeWorldHash();
	if (TestTrue(TEXT("The run with both counters off 0 saves"), Save->SaveCurrentGalaxy()))
	{
		Live.Civs[1].LawsHeldTicks = 0;
		Live.Civs[2].PressureEnduredTicks = 0;
		if (TestTrue(TEXT("and loads"), Save->LoadGalaxy(Galaxy)))
		{
			const FGalaxyState& Loaded = Sim->GetState();
			TestEqual(TEXT("LawsHeldTicks survives the round trip"), Loaded.Civs[1].LawsHeldTicks, 321);
			TestEqual(TEXT("on its own people only"), Loaded.Civs[2].LawsHeldTicks, 0);
			TestEqual(TEXT("PressureEnduredTicks survives the round trip"), Loaded.Civs[2].PressureEnduredTicks, 654);
			TestEqual(TEXT("on its own people only"), Loaded.Civs[1].PressureEnduredTicks, 0);
			TestEqual(TEXT("with the hash it was saved with"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Hash));
			TestEqual(TEXT("and the world hash"), static_cast<int64>(Sim->ComputeWorldHash()), static_cast<int64>(World));
		}
	}

	// (3) A save without the two members (an old save) loads them as 0, and the rest of the element as it was.
	TArray<uint8> Raw = SaveBytesOf(Sim->GetState());
	TestEqual(TEXT("Every people writes LawsHeldTicks"), Forget(Raw, TEXT("LawsHeldTicks")), Sim->GetState().Civs.Num());
	TestEqual(TEXT("Every people writes PressureEnduredTicks"), Forget(Raw, TEXT("PressureEnduredTicks")), Sim->GetState().Civs.Num());
	const UCosmicSaveGame* Old = Cast<UCosmicSaveGame>(UGameplayStatics::LoadGameFromMemory(Raw));
	if (TestNotNull(TEXT("The bytes without the two members load"), Old) && TestEqual(TEXT("with every people"), Old->State.Civs.Num(), Sim->GetState().Civs.Num()))
	{
		TestEqual(TEXT("An old save reads LawsHeldTicks as 0"), Old->State.Civs[1].LawsHeldTicks, 0);
		TestEqual(TEXT("An old save reads PressureEnduredTicks as 0"), Old->State.Civs[2].PressureEnduredTicks, 0);
		TestEqual(TEXT("and the member beside them as it was saved"), Old->State.Civs[2].FallenSphereTicks, 77);
		TestEqual(TEXT("and the people's name"), Old->State.Civs[2].CivID, Sim->GetState().Civs[2].CivID);
	}
	return true;
}

// The two counters are world state (header section 2.1): either of them off its value moves both hashes, and putting it back
// restores both. The exclusion lists are untouched (CLAUDE.md rule 6); they name top-level members of FGalaxyState only, so a
// people's counter cannot be left out there - the mutation is a hasher that skips either name, which leaves both hashes where
// they were (Tools/mutations/B6b/b6b1_hasher_skips_counters.patch).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimHashCoversNewCountersTest, "Cosmic_Ascension.Sim.HashCoversNewCounters", CosmicTests::Flags)
bool FCosmicSimHashCoversNewCountersTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	ON_SCOPE_EXIT
	{
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(200);
	FGalaxyState& State = Sim->GetMutableState();
	if (!TestTrue(TEXT("The slice has two peoples"), State.Civs.Num() >= 2))
	{
		return false;
	}
	const uint32 World = Sim->ComputeWorldHash();
	const uint32 Full = Sim->ComputeStateHash();

	auto Moves = [&](const TCHAR* What, TFunctionRef<int32&(FCivilizationState&)> Counter)
	{
		int32& Value = Counter(State.Civs[1]);
		const int32 Was = Value;
		Value = Was + 600;
		TestTrue(*FString::Printf(TEXT("%s moves the world hash"), What), Sim->ComputeWorldHash() != World);
		TestTrue(*FString::Printf(TEXT("%s moves the full hash"), What), Sim->ComputeStateHash() != Full);
		Value = Was;
		TestTrue(*FString::Printf(TEXT("(both hashes are back after %s is put back)"), What), Sim->ComputeWorldHash() == World && Sim->ComputeStateHash() == Full);
	};
	Moves(TEXT("LawsHeldTicks"), [](FCivilizationState& Civ) -> int32& { return Civ.LawsHeldTicks; });
	Moves(TEXT("PressureEnduredTicks"), [](FCivilizationState& Civ) -> int32& { return Civ.PressureEnduredTicks; });
	return true;
}

// ---- B-6b2a: the galaxy laws, the AI's two steps and the vocabulary of the endured era ----

namespace CosmicUpperRungTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	FCosmicCondition Cond(EConditionType Type, const TCHAR* Key = nullptr, float Value = 0.f)
	{
		FCosmicCondition Condition;
		Condition.Type = Type;
		Condition.Key = Key ? FName(Key) : FName();
		Condition.Value = Value;
		return Condition;
	}

	FCosmicEffect Eff(EEffectType Type, const TCHAR* Key, float Value)
	{
		FCosmicEffect Effect;
		Effect.Type = Type;
		Effect.Key = Key ? FName(Key) : FName();
		Effect.Value = Value;
		return Effect;
	}

	// The edicts of the boards below (the test builds them: no case leans on the game's rows). Galaxy laws of the fourth rung,
	// in table order: a (plague share x0.5, plague countdown +40, Influence 10), b (a condition no people meets), c (Matter +7
	// a tick to its holder, Energy 0.5 a tick of upkeep), d (nothing), dear (an upkeep no stockpile pays), shut (plague share
	// x0); and an edict of the people's scope, own (plague share x2, plague countdown +7).
	const FName LawA(TEXT("edict_law_a"));
	const FName LawB(TEXT("edict_law_b"));
	const FName LawC(TEXT("edict_law_c"));
	const FName LawD(TEXT("edict_law_d"));
	const FName LawDear(TEXT("edict_law_dear"));
	const FName LawShut(TEXT("edict_law_shut"));
	const FName Own(TEXT("edict_own"));
	const FName Sick(TEXT("crisis_test_sick"));

	UDataTable* Edicts()
	{
		UDataTable* Table = CosmicTests::NewTable(FEdictDef::StaticStruct());
		auto Add = [Table](FName Name, ECosmicEdictScope Scope, TFunctionRef<void(FEdictDef&)> Fill)
		{
			FEdictDef Row;
			Row.DisplayName = FText::AsCultureInvariant(Name.ToString());
			Row.Summary = FText::AsCultureInvariant(TEXT("a test edict."));
			Row.Scope = Scope;
			Row.RequiredTier = Scope == ECosmicEdictScope::HomeGalaxy ? EAscensionTier::Tier4_Transcendent : EAscensionTier::Tier1_Planetary;
			Fill(Row);
			Table->AddRow(Name, Row);
		};
		Add(LawA, ECosmicEdictScope::HomeGalaxy, [](FEdictDef& Row)
		{
			Row.HazardSpawnMultipliers.Add(Tag(TEXT("Hazard.Plague")), 0.5f);
			Row.HazardCountdownBonus.Add(Tag(TEXT("Hazard.Plague")), 40);
			Row.EnactCost.Influence = 10.f;
		});
		Add(LawB, ECosmicEdictScope::HomeGalaxy, [](FEdictDef& Row)
		{
			FCosmicCondition Book;
			Book.Type = EConditionType::CivHasTrait;
			Book.Tag = Tag(TEXT("Trait.Faith.Book"));
			Row.EnactConditions = { Book };
		});
		Add(LawC, ECosmicEdictScope::HomeGalaxy, [](FEdictDef& Row)
		{
			Row.EffectsPerTick = { Eff(EEffectType::AddResource, TEXT("Matter"), 7.f) };
			Row.UpkeepPerTick.Energy = 0.5f;
		});
		Add(LawD, ECosmicEdictScope::HomeGalaxy, [](FEdictDef&) {});
		Add(LawDear, ECosmicEdictScope::HomeGalaxy, [](FEdictDef& Row) { Row.UpkeepPerTick.Energy = 1.0e9f; });
		Add(LawShut, ECosmicEdictScope::HomeGalaxy, [](FEdictDef& Row) { Row.HazardSpawnMultipliers.Add(Tag(TEXT("Hazard.Plague")), 0.f); });
		Add(Own, ECosmicEdictScope::People, [](FEdictDef& Row)
		{
			Row.HazardSpawnMultipliers.Add(Tag(TEXT("Hazard.Plague")), 2.f);
			Row.HazardCountdownBonus.Add(Tag(TEXT("Hazard.Plague")), 7);
		});
		return Table;
	}

	// One plague row: a Draw row of countdown 200 and the chance a case gives.
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
		return Table;
	}

	// A feat row of the ruler's path, one rung up from From, with the conditions given.
	FAscensionFeatDef FeatRow(EAscensionTier From, TArray<FCosmicCondition> Conditions)
	{
		FAscensionFeatDef Row;
		Row.Title = FText::AsCultureInvariant(TEXT("a test feat"));
		Row.RequiredTier = From;
		Row.TargetTier = static_cast<EAscensionTier>(static_cast<uint8>(From) + 1);
		Row.Path = EPlayerRole::Ruler;
		Row.CompletionConditions = MoveTemp(Conditions);
		return Row;
	}

	void NoEdit(FCosmicRulesDef&) {}

	/**
	 * A board of twelve systems wired as CosmicTests::WireChain and four peoples that own every one of them, all settled:
	 * people 0 (the player's) holds {0, 1}, 1 holds {3, 2, 4}, 2 holds {6, 5, 7}, 3 holds {9, 8, 10, 11}; capitals 0, 3, 6, 9.
	 * 0 borders 1, 1 borders 2 and 3, 2 borders 3 (the board of Tests/CosmicDiplomacyTests.cpp). Quiet rules (no event, no
	 * drain, no first crisis, no projects, no inherited edict, no transcendent, no card, no drift, no fleet, no sight stage),
	 * this file's edicts and plague and the feat rows given (none: an empty table). Every people on the first rung with three
	 * slots (Society 1), Stability 0.75 and a stockpile of 1000 of each resource.
	 */
	FGalaxyState* LayBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Original, UDataTable* Feats,
		TFunctionRef<void(FCosmicRulesDef&)> Edit, float SickChance = 0.f)
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
		Edit(Rules);
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));
		Tables.Edicts = Edicts();
		Tables.Hazards = Hazards(SickChance);
		Tables.Events = nullptr;
		Tables.Encounters = nullptr;
		Tables.Transcendents = nullptr;
		if (Feats)
		{
			Tables.Feats = Feats;
		}
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
			CosmicHazards::EmptyHazardSlot(System);
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
			Civ.Research.Add(FName(TEXT("Society")), 1.f);
			Civ.Stockpile.Energy = 1000.f;
			Civ.Stockpile.Matter = 1000.f;
			Civ.Stockpile.Insight = 1000.f;
			Civ.Stockpile.Influence = 1000.f;
			Civ.Stockpile.Causality = 0.f;
		}
		Sim.RebuildStateIndex();
		Sim.SyncRngState();
		return &State;
	}

	// The two rows the AI's steps answer to: on the third rung CivHasTreaty 2 (with a condition no tick reaches, so no people
	// climbs by it), on the fourth CivLawsHeldTicksAtLeast 600.
	UDataTable* AskingFeats()
	{
		UDataTable* Table = CosmicTests::NewTable(FAscensionFeatDef::StaticStruct());
		Table->AddRow(FName(TEXT("feat_test_treaty")), FeatRow(EAscensionTier::Tier3_Causal,
			{ Cond(EConditionType::CivHasTreaty, nullptr, 2.f), Cond(EConditionType::GalaxyTickAtLeast, nullptr, 1.0e9f) }));
		Table->AddRow(FName(TEXT("feat_test_laws")), FeatRow(EAscensionTier::Tier4_Transcendent, { Cond(EConditionType::CivLawsHeldTicksAtLeast, TEXT("600")) }));
		return Table;
	}
}

// The held laws (header section 2.4, decision 10): at the economy end of a tick, a people on the fourth rung or above that
// holds DT_Rules.GalaxyLawsToHold galaxy laws adds one, any other tick starts over at 0 - below the fourth rung, with fewer
// laws, or when a law lapsed by its upkeep in this tick (the count comes after the upkeep - mutation: counted before it). The
// knob counts the laws (below 1 read as 1); a law the table does not hold counts for nothing; the count draws nothing.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimLawsHeldTest, "Cosmic_Ascension.Sim.LawsHeld", CosmicTests::Flags)
bool FCosmicSimLawsHeldTest::RunTest(const FString& Parameters)
{
	using namespace CosmicUpperRungTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* Board = LayBoard(*Sim, Original, nullptr, [](FCosmicRulesDef& Rules) { Rules.GalaxyLawsToHold = 3; });
	if (!TestNotNull(TEXT("The board is laid"), Board))
	{
		return false;
	}
	// 0 (the player's): the fourth rung, laws a, c and dear - dear lapses in this tick's upkeep. 1: the fourth rung, three
	// laws. 2: the third rung, the same three. 3: the fourth rung, two laws and an edict of its people. Each counted 10 before.
	const TArray<FName> Three = { LawA, LawC, LawD };
	Board->Civs[0].Tier = EAscensionTier::Tier4_Transcendent;
	Board->Civs[0].ActiveEdicts = { LawA, LawC, LawDear };
	Board->Civs[1].Tier = EAscensionTier::Tier4_Transcendent;
	Board->Civs[1].ActiveEdicts = Three;
	Board->Civs[2].Tier = EAscensionTier::Tier3_Causal;
	Board->Civs[2].ActiveEdicts = Three;
	Board->Civs[3].Tier = EAscensionTier::Tier4_Transcendent;
	Board->Civs[3].ActiveEdicts = { LawA, LawC, Own };
	for (FCivilizationState& Civ : Board->Civs)
	{
		Civ.LawsHeldTicks = 10;
	}
	Sim->StepOnce();
	TestEqual(TEXT("On the fourth rung with three laws the count grows by one"), Board->Civs[1].LawsHeldTicks, 11);
	TestEqual(TEXT("On the third rung the count is 0, the laws held or not"), Board->Civs[2].LawsHeldTicks, 0);
	TestEqual(TEXT("Two laws of three: 0"), Board->Civs[3].LawsHeldTicks, 0);
	TestFalse(TEXT("(the dear law lapsed in the upkeep)"), Board->Civs[0].ActiveEdicts.Contains(LawDear));
	TestEqual(TEXT("A law that lapsed in this tick is not held at the count: 0"), Board->Civs[0].LawsHeldTicks, 0);
	TestTrue(TEXT("(an AI people keeps its laws)"), Board->Civs[1].ActiveEdicts == Three);
	Sim->StepOnce();
	TestEqual(TEXT("and grows again the next tick"), Board->Civs[1].LawsHeldTicks, 12);

	// The knob: two laws are enough under 2; under 0 (read as 1) one law is; a law the table does not hold is none.
	{
		FGalaxyState* Two = LayBoard(*Sim, Original, nullptr, [](FCosmicRulesDef& Rules) { Rules.GalaxyLawsToHold = 2; });
		Two->Civs[3].Tier = EAscensionTier::Tier4_Transcendent;
		Two->Civs[3].ActiveEdicts = { LawA, LawC };
		Sim->StepOnce();
		TestEqual(TEXT("Under GalaxyLawsToHold 2, two laws count"), Two->Civs[3].LawsHeldTicks, 1);
		FGalaxyState* Zero = LayBoard(*Sim, Original, nullptr, [](FCosmicRulesDef& Rules) { Rules.GalaxyLawsToHold = 0; });
		Zero->Civs[1].Tier = EAscensionTier::Tier4_Transcendent;
		Zero->Civs[1].ActiveEdicts = { LawD };
		Zero->Civs[2].Tier = EAscensionTier::Tier4_Transcendent;
		Zero->Civs[2].ActiveEdicts = { FName(TEXT("edict_law_gone")) };
		Zero->Civs[3].Tier = EAscensionTier::Tier4_Transcendent;
		Sim->StepOnce();
		TestEqual(TEXT("Under GalaxyLawsToHold 0, read as 1, one law counts"), Zero->Civs[1].LawsHeldTicks, 1);
		TestEqual(TEXT("A name the table does not hold is no law"), Zero->Civs[2].LawsHeldTicks, 0);
		TestEqual(TEXT("and no law at all is not one"), Zero->Civs[3].LawsHeldTicks, 0);
	}

	// No draw: the run with every people holding laws ends on the seed of the run with none.
	{
		int32 Seeds[2] = { 0, 0 };
		for (int32 Pass = 0; Pass < 2; ++Pass)
		{
			FGalaxyState* Again = LayBoard(*Sim, Original, nullptr, NoEdit);
			for (FCivilizationState& Civ : Again->Civs)
			{
				Civ.Tier = EAscensionTier::Tier4_Transcendent;
				Civ.ActiveEdicts = Pass == 0 ? TArray<FName>() : Three;
			}
			Sim->StepOnce();
			Seeds[Pass] = Sim->GetRng().GetCurrentSeed();
		}
		TestEqual(TEXT("The count draws nothing"), Seeds[1], Seeds[0]);
	}
	return true;
}

// The two hooks of a galaxy law (header section 2.4, decision 9): its spawn factor (1) and its countdown bonus (2) reach every
// settled system of its holder's galaxy - another people's too - once a row whoever else holds it (mutation: counted again
// for each holder); the owner's own edicts first, an edict of the people's scope reaching its holder's systems alone; a system
// of another galaxy is not reached. Placement and the spawn draw read them (the draws as many). Every other part - the
// per-tick effects, the upkeep, the answer at an expiry - stays the holder's.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimGalaxyLawsTest, "Cosmic_Ascension.Sim.GalaxyLaws", CosmicTests::Flags)
bool FCosmicSimGalaxyLawsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicUpperRungTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* Board = LayBoard(*Sim, Original, nullptr, NoEdit);
	if (!TestNotNull(TEXT("The board is laid"), Board))
	{
		return false;
	}
	const FGameplayTag Plague = Tag(TEXT("Hazard.Plague"));
	const FGameplayTag Rebellion = Tag(TEXT("Hazard.Rebellion"));
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicDataTables& Tables = Sim->GetTables();
	auto Share = [&](int32 SystemIndex, FGameplayTag Of = FGameplayTag()) { return CosmicEdicts::SpawnMultiplierOf(*Board, Index, Tables, SystemIndex, Of.IsValid() ? Of : Plague); };
	auto Bonus = [&](int32 SystemIndex, FGameplayTag Of = FGameplayTag()) { return CosmicEdicts::CountdownBonusOf(*Board, Index, Tables, SystemIndex, Of.IsValid() ? Of : Plague); };

	// Held by people 1: its own capital (3) and people 2's (6) and 3's settled systems (8) alike.
	Board->Civs[1].ActiveEdicts = { LawA };
	TestEqual(TEXT("A galaxy law's spawn factor reaches another people's capital"), Share(6), 0.5f);
	TestEqual(TEXT("and its countdown bonus"), Bonus(6), 40);
	TestEqual(TEXT("and a settled system of a third people"), Share(8), 0.5f);
	TestEqual(TEXT("and its holder's own"), Bonus(3), 40);
	TestEqual(TEXT("A tag it names nothing for is untouched (factor)"), Share(6, Rebellion), 1.f);
	TestEqual(TEXT("A tag it names nothing for is untouched (bonus)"), Bonus(6, Rebellion), 0);
	// Held by three peoples: once a row.
	Board->Civs[2].ActiveEdicts = { LawA };
	Board->Civs[3].ActiveEdicts = { LawA };
	TestEqual(TEXT("Held by three peoples, the law's factor counts once"), Share(6), 0.5f);
	TestEqual(TEXT("and so does its bonus"), Bonus(6), 40);
	TestEqual(TEXT("on every system of the galaxy (factor)"), Share(8), 0.5f);
	TestEqual(TEXT("on every system of the galaxy (bonus)"), Bonus(3), 40);
	// The owner's edict of the people's scope first, then the laws - it reaches no other people's system.
	Board->Civs[2].ActiveEdicts = { Own, LawA };
	const auto Hooks = CosmicEdicts::HookEdictsOf(*Board, Index, Tables, 6);
	TestTrue(TEXT("The hooks of a system: the owner's edicts first, then the galaxy's laws, each once"),
		Hooks.Num() == 2 && Hooks[0] == Tables.FindEdict(Own) && Hooks[1] == Tables.FindEdict(LawA));
	TestEqual(TEXT("The owner's own edict and the law multiply"), Share(6), 1.f);
	TestEqual(TEXT("and add"), Bonus(6), 47);
	TestEqual(TEXT("An edict of the people's scope reaches no other people's system (factor)"), Share(3), 0.5f);
	TestEqual(TEXT("An edict of the people's scope reaches no other people's system (bonus)"), Bonus(3), 40);
	// A system of another galaxy row.
	const FName HomeRow = Board->Systems[6].GalaxyRow;
	Board->Systems[6].GalaxyRow = FName(TEXT("gal_test_far"));
	Board->Civs[2].ActiveEdicts.Reset();
	TestEqual(TEXT("A system of another galaxy is not reached (factor)"), Share(6), 1.f);
	TestEqual(TEXT("A system of another galaxy is not reached (bonus)"), Bonus(6), 0);
	Board->Systems[6].GalaxyRow = HomeRow;

	// Placement reads hook (2): a plague placed on people 2's capital counts the law held by people 1.
	Board->Civs[2].ActiveEdicts.Reset();
	Board->Civs[3].ActiveEdicts.Reset();
	{
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		if (TestTrue(TEXT("The plague is placed"), CosmicHazards::Place(Ctx, 6, Sick)))
		{
			TestEqual(TEXT("with the countdown of the row and the law's 40"), Board->Systems[6].HazardTicksRemaining, 240);
		}
	}

	// The spawn draw reads hook (1): with a chance of 1, a law of factor 0 held by people 1 keeps the plague off every settled
	// system of the galaxy; without it the plague stands on people 2's capital. As many draws either way.
	{
		int32 Seeds[2] = { 0, 0 };
		bool bPlagued[2] = { false, false };
		for (int32 Pass = 0; Pass < 2; ++Pass)
		{
			FGalaxyState* Drawn = LayBoard(*Sim, Original, nullptr, NoEdit, 1.f);
			Drawn->Civs[1].ActiveEdicts = Pass == 0 ? TArray<FName>() : TArray<FName>({ LawShut });
			Sim->StepOnce();
			Seeds[Pass] = Sim->GetRng().GetCurrentSeed();
			for (const FStarSystemState& System : Drawn->Systems)
			{
				bPlagued[Pass] |= System.ActiveHazard == Plague;
			}
		}
		TestTrue(TEXT("Without the law the plague stands"), bPlagued[0]);
		TestFalse(TEXT("With a law of factor 0 held by one people, no settled system of the galaxy draws it"), bPlagued[1]);
		TestEqual(TEXT("and the draws are as many"), Seeds[1], Seeds[0]);
	}

	// Every other part is the holder's: law c adds Matter and costs Energy to people 1 alone; the answer at an expiry stays the
	// holder's (a hazard row listing a law - the schema keeps them apart).
	{
		float Matter[2][2];
		float Energy[2][2];
		for (int32 Pass = 0; Pass < 2; ++Pass)
		{
			FGalaxyState* Held = LayBoard(*Sim, Original, nullptr, NoEdit);
			Held->Civs[1].Tier = EAscensionTier::Tier4_Transcendent;
			Held->Civs[1].ActiveEdicts = Pass == 0 ? TArray<FName>() : TArray<FName>({ LawC });
			Sim->StepOnce();
			for (int32 Civ = 1; Civ <= 2; ++Civ)
			{
				Matter[Pass][Civ - 1] = Held->Civs[Civ].Stockpile.Matter;
				Energy[Pass][Civ - 1] = Held->Civs[Civ].Stockpile.Energy;
			}
		}
		TestEqual(TEXT("The law's per-tick effect is its holder's"), Matter[1][0] - Matter[0][0], 7.f, 0.001f);
		TestEqual(TEXT("and its upkeep"), Energy[0][0] - Energy[1][0], 0.5f, 0.001f);
		TestEqual(TEXT("Another people of the galaxy gains no Matter by it"), Matter[1][1], Matter[0][1]);
		TestEqual(TEXT("and pays no upkeep"), Energy[1][1], Energy[0][1]);
		FHazardDef Asking;
		Asking.HazardTag = Plague;
		Asking.RulerResolutions = { LawA };
		Board = &Sim->GetMutableState();
		Board->Civs[1].ActiveEdicts = { LawA };
		TestEqual(TEXT("The answer at an expiry is the holder's"), CosmicEdicts::HeldAnswerOf(Board->Civs[1], Asking, Sim->GetTables()), LawA);
		TestTrue(TEXT("and no other people's"), CosmicEdicts::HeldAnswerOf(Board->Civs[2], Asking, Sim->GetTables()).IsNone());
	}
	return true;
}

// The AI's two steps (header section 2.3, 2.4, decision 1, 11): only under a row its rung is judged by that asks. The treaty
// step - one a tick - signs a non-aggression with the first bordering people in Civs order the player does not rule (the
// ruled people, first in Civs order and bordering, is passed by - mutation: it is not) for which the gate takes it, and pays
// as the command pays; no line, no knowing write, no draw (mutation: one number drawn). The law step enacts the laws in table
// order wherever the one gate takes them (a refused condition, then full slots). Without the rows nothing of the kind.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimAIRowStepsTest, "Cosmic_Ascension.Sim.AIRowSteps", CosmicTests::Flags)
bool FCosmicSimAIRowStepsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicUpperRungTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FGameplayTag NonAggression = CosmicTags::TreatyNonAggression();
	// People 1 on the third rung (its row asks two treaties), people 2 on the fourth (its row asks for laws).
	auto Lay = [&](UDataTable* Feats)
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, Feats, NoEdit);
		if (Board)
		{
			Board->Civs[1].Tier = EAscensionTier::Tier3_Causal;
			Board->Civs[2].Tier = EAscensionTier::Tier4_Transcendent;
		}
		return Board;
	};
	FGalaxyState* Plain = Lay(nullptr);
	if (!TestNotNull(TEXT("The board is laid"), Plain))
	{
		return false;
	}
	Sim->StepOnce();
	const float PlainInfluence1 = Plain->Civs[1].Stockpile.Influence;
	const float PlainInfluence2 = Plain->Civs[2].Stockpile.Influence;
	const int32 PlainLines = Plain->Log.Num();
	const int32 PlainMet = Plain->Knowing.MetCivs.Num();
	const int32 PlainSeed = Sim->GetRng().GetCurrentSeed();
	TestTrue(TEXT("Without a row that asks, the AI signs no treaty"), Plain->Civs[1].Ties.IsEmpty() && Plain->Civs[2].Ties.IsEmpty());
	TestTrue(TEXT("and enacts no law"), Plain->Civs[2].ActiveEdicts.IsEmpty());

	FGalaxyState* Asked = Lay(AskingFeats());
	Sim->StepOnce();
	FCivilizationState& One = Asked->Civs[1];
	TestTrue(TEXT("The treaty step signs a non-aggression with the first bordering people the player does not rule"),
		CosmicDiplomacy::HasTreaty(One, Asked->Civs[2].CivID, NonAggression) && CosmicDiplomacy::HasTreaty(Asked->Civs[2], One.CivID, NonAggression));
	TestFalse(TEXT("never with the ruled people, though it borders and comes first"), CosmicDiplomacy::TreatyCountOf(One, Asked->Civs[0].CivID) > 0);
	TestEqual(TEXT("one treaty in a tick"), CosmicDiplomacy::TreatyPartnersOf(One, FGameplayTag()), 1);
	TestEqual(TEXT("paid as the command pays"), PlainInfluence1 - One.Stockpile.Influence, CosmicDiplomacy::TreatyCostOf(Sim->GetRules()), 0.001f);
	TestEqual(TEXT("no line"), Asked->Log.Num(), PlainLines);
	TestEqual(TEXT("no knowing write"), Asked->Knowing.MetCivs.Num(), PlainMet);
	TestEqual(TEXT("no draw"), Sim->GetRng().GetCurrentSeed(), PlainSeed);
	TestTrue(TEXT("The law step enacts the laws in table order through the gate: a, then c and d (b's condition refused, the rest past the slots)"),
		Asked->Civs[2].ActiveEdicts == TArray<FName>({ LawA, LawC, LawD }));
	TestEqual(TEXT("and pays them"), PlainInfluence2 - Asked->Civs[2].Stockpile.Influence, 10.f, 0.001f);
	TestFalse(TEXT("The people whose row asks laws signs no treaty of its own"), CosmicDiplomacy::HasTreaty(Asked->Civs[2], Asked->Civs[3].CivID, NonAggression));
	Sim->StepOnce();
	TestTrue(TEXT("The next tick the second treaty, with the next bordering people"), CosmicDiplomacy::HasTreaty(One, Asked->Civs[3].CivID, NonAggression));
	TestEqual(TEXT("(two partners)"), CosmicDiplomacy::TreatyPartnersOf(One, NonAggression), 2);
	Sim->StepOnce();
	TestEqual(TEXT("With the row's treaties held, no more"), CosmicDiplomacy::TreatyPartnersOf(One, FGameplayTag()), 2);

	// The gate decides: a people hostile toward people 1 is passed by for the next one.
	{
		FGalaxyState* Hostile = Lay(AskingFeats());
		CosmicDiplomacy::SetStanceOf(Hostile->Civs[2], Hostile->Civs[1].CivID, CosmicTags::StanceHostile());
		Sim->StepOnce();
		TestTrue(TEXT("A partner the gate refuses (hostile) is passed by for the next"),
			CosmicDiplomacy::HasTreaty(Hostile->Civs[1], Hostile->Civs[3].CivID, NonAggression) && !CosmicDiplomacy::HasTreaty(Hostile->Civs[1], Hostile->Civs[2].CivID, NonAggression));
	}
	return true;
}

// The vocabulary of the endured era (header section 2.7, #156 - decision 22, the answer A): CivPressureEnduredTicksReached is
// true in one slot of a stretch - under slots of one tick only at the tick its count reaches the Key (mutation: the next tick
// too); SystemOwnedBy Ruled holds for the people the vessel rules and for no AI people (mutation: Ruled read as the context
// people); ChangeReverence Borders adds to every people bordering the context people once, the context people untouched
// (mutation: twice), clamped at 1; another Key is turned away.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimEraSelectorTest, "Cosmic_Ascension.Sim.EraSelector", CosmicTests::Flags)
bool FCosmicSimEraSelectorTest::RunTest(const FString& Parameters)
{
	using namespace CosmicUpperRungTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* Board = LayBoard(*Sim, Original, nullptr, NoEdit);
	if (!TestNotNull(TEXT("The board is laid"), Board))
	{
		return false;
	}
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	auto Holds = [&](const FCosmicCondition& Condition, int32 CivIndex, int32 SystemIndex, int32 SlotTicks)
	{
		FCosmicRulesDef Rules = Sim->GetRules();
		Rules.EventSlotTicks = SlotTicks;
		FCosmicEvalView View(*Board, Index, Sim->GetTables(), Rules);
		View.CivIndex = CivIndex;
		View.SystemIndex = SystemIndex;
		return UCosmicEventSubsystem::Check(Condition, View);
	};

	// (1) The window: under slots of one tick, only the count 600; under slots of ten, 600 to 609.
	const FCosmicCondition Endured = Cond(EConditionType::CivPressureEnduredTicksReached, TEXT("600"));
	const struct { int32 Count; int32 Slot; bool bHolds; } Windows[] = {
		{ 599, 1, false }, { 600, 1, true }, { 601, 1, false }, { 609, 10, true }, { 610, 10, false }, { 0, 10, false } };
	for (const auto& Window : Windows)
	{
		Board->Civs[1].PressureEnduredTicks = Window.Count;
		TestEqual(*FString::Printf(TEXT("An endured stretch of %d under slots of %d ticks"), Window.Count, Window.Slot), Holds(Endured, 1, 3, Window.Slot), Window.bHolds);
	}

	// (2) Ruled: the people the vessel rules, its capital.
	if (TestTrue(TEXT("(the vessel rules people 0)"), Board->Calling.LifeState == ELifeState::Embodied && Board->Player.Role == EPlayerRole::Ruler
		&& Board->Player.RuledCivID == Board->Civs[0].CivID))
	{
		const FCosmicCondition Ruled = Cond(EConditionType::SystemOwnedBy, TEXT("Ruled"));
		TestTrue(TEXT("SystemOwnedBy Ruled holds for the ruled people's capital"), Holds(Ruled, 0, 0, 10));
		TestFalse(TEXT("and not for an AI people's capital, the AI the context"), Holds(Ruled, 1, 3, 10));
		TestFalse(TEXT("nor for another people's system under the ruled people's context"), Holds(Ruled, 0, 3, 10));
	}

	// (3) Borders: people 1 borders 0, 2 and 3; people 0 borders 1 alone.
	auto Apply = [&](int32 CivIndex, const TCHAR* Key, float Value)
	{
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = CivIndex;
		return UCosmicEventSubsystem::Apply(Eff(EEffectType::ChangeReverence, Key, Value), Ctx);
	};
	for (FCivilizationState& Civ : Board->Civs)
	{
		Civ.ReverenceToPlayer = 0.f;
	}
	Board->Civs[3].ReverenceToPlayer = 0.9f;
	TestTrue(TEXT("ChangeReverence Borders applies"), Apply(1, TEXT("Borders"), 0.2f));
	TestEqual(TEXT("to a bordering people once"), Board->Civs[0].ReverenceToPlayer, 0.2f);
	TestEqual(TEXT("to every bordering people"), Board->Civs[2].ReverenceToPlayer, 0.2f);
	TestEqual(TEXT("clamped at 1"), Board->Civs[3].ReverenceToPlayer, 1.f);
	TestEqual(TEXT("and not to the context people itself"), Board->Civs[1].ReverenceToPlayer, 0.f);
	TestTrue(TEXT("From people 0"), Apply(0, TEXT("Borders"), 0.2f));
	TestEqual(TEXT("only its one neighbour gains"), Board->Civs[1].ReverenceToPlayer, 0.2f);
	TestEqual(TEXT("(a people beyond the border does not)"), Board->Civs[2].ReverenceToPlayer, 0.2f);
	TestTrue(TEXT("An empty Key is the context people"), Apply(2, nullptr, 0.1f) && FMath::IsNearlyEqual(Board->Civs[2].ReverenceToPlayer, 0.3f));
	const float Before = Board->Civs[0].ReverenceToPlayer;
	TestFalse(TEXT("Another Key is turned away"), Apply(1, TEXT("Neighbours"), 0.2f));
	TestEqual(TEXT("and changes nothing"), Board->Civs[0].ReverenceToPlayer, Before);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

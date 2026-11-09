// Cosmic Ascension - automation tests of the fourth and fifth rungs (B6b): from B-6b1 the two saved counters of a people
// (LawsHeldTicks, PressureEnduredTicks) through the save and the hashes; from B-6b2a the count of the held laws, the two hooks
// of a galaxy law, the AI's law and treaty steps and the vocabulary of the endured era (the condition's window, the selector
// word Ruled, the Key Borders); from B-6b2b the extinction's expiry (the gate, LetPeopleGo, the ruler's death, the record line);
// from B-6b2c the ark's slot, the delay above the third rung, the brake on the fourth rung and the binding.
// The count of the endured pressure is Sim.PressureEndured (Tests/CosmicBookTests.cpp, beside the fallen-sphere counter it is
// counted with).
// Design: docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md section 2.1 - 2.7, 6; docs/20 section 9.1; docs/21 section
// 7.4 (the count of the fourth rung).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Containers/StringConv.h"
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
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RandomStream.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Save/CosmicSaveGame.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicAscent.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicSeal.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/Function.h"
#include "Tests/CosmicRaceMeasure.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/Package.h"

#include <initializer_list>
#include <limits>

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

// ---- B-6b2b: the extinction's expiry ----

namespace CosmicUpperRungTests
{
	// The rows of the extinction's boards: the extinction of the tests (the board's table replaces the game's, whose
	// crisis_species_extinction is there from [B-6b6]) and a plague that CosmicTests::StandingProject answers, each with three
	// lines for every moment and no effect; the grace row of the remnant is the game's.
	const FName Extinct(TEXT("crisis_test_extinct"));
	const FName Plague(TEXT("crisis_test_plague"));

	FHazardDef LinedHazardOf(const TCHAR* TagName)
	{
		auto Lines = [TagName](const TCHAR* Moment)
		{
			const FString Stem = FString::Printf(TEXT("%s %s"), TagName, Moment);
			return TArray<FText>{ FText::AsCultureInvariant(Stem + TEXT(" one")), FText::AsCultureInvariant(Stem + TEXT(" two")), FText::AsCultureInvariant(Stem + TEXT(" three")) };
		};
		FHazardDef Row;
		Row.HazardTag = Tag(TagName);
		Row.DisplayName = FText::AsCultureInvariant(FString::Printf(TEXT("<%s>"), TagName));
		Row.CountdownTicks = 250;
		Row.SpawnRule = ECosmicHazardSpawn::Draw;
		Row.SpawnChancePerTick = 0.f;
		Row.LogCategory = Tag(TEXT("Log.Crisis"));
		Row.Severity = 1;
		Row.OutbreakTemplates = Lines(TEXT("outbreak"));
		Row.ExpireTemplates = Lines(TEXT("expiry"));
		Row.ResolveTemplates = Lines(TEXT("resolution"));
		return Row;
	}

	FHazardDef AnsweredPlague()
	{
		FHazardDef Row = LinedHazardOf(TEXT("Hazard.Plague"));
		Row.RulerResolutions = { CosmicTests::StandingProject() };
		return Row;
	}

	/**
	 * LayBoard's board (people 0 - the vessel's, ruled from system 0 - holds {0, 1}; 1 holds {3, 2, 4}; 2 {6, 5, 7}; 3 {9, 8,
	 * 10, 11}) with the extinction and the plague of this file as its hazards and the game's grace row of the remnant as its
	 * one event row; realm caps of 1, so that no AI people settles a system an extinction let go; the tick 400, past the first
	 * vessel's birth protection, and the grace of the run spent unless bGrace. Null when the board is not laid or the game has
	 * no grace row.
	 */
	FGalaxyState* LayExtinctionBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Original, bool bGrace)
	{
		const FCosmicEventDef* Grace = Original.Events ? Original.Events->FindRow<FCosmicEventDef>(CosmicRows::EventGraceRemnant, TEXT("CosmicUpperRungTests"), false) : nullptr;
		FGalaxyState* Board = Grace ? LayBoard(Sim, Original, nullptr, [](FCosmicRulesDef& Rules) { Rules.RealmCapByTier = { 1, 1, 1, 1, 1 }; }) : nullptr;
		if (!Board)
		{
			return nullptr;
		}
		FCosmicDataTables Tables = Sim.GetTables();
		UDataTable* HazardRows = CosmicTests::NewTable(FHazardDef::StaticStruct());
		HazardRows->AddRow(Extinct, LinedHazardOf(TEXT("Hazard.Extinction")));
		HazardRows->AddRow(Plague, AnsweredPlague());
		Tables.Hazards = HazardRows;
		UDataTable* EventRows = CosmicTests::NewTable(FCosmicEventDef::StaticStruct());
		EventRows->AddRow(CosmicRows::EventGraceRemnant, *Grace);
		Tables.Events = EventRows;
		Sim.SetTables(Tables);
		Board->Tick = 400;
		Board->Player.CurrentSystemID = Board->Systems[0].SystemID;
		if (!bGrace)
		{
			Board->Flags.AddTag(Tag(TEXT("Flag.Grace.Spent")));
		}
		Sim.SyncRngState();
		return Board;
	}

	// A completion record of CosmicTests::StandingProject at Tick by the people CivID (none for a save from before B3).
	FMegaprojectRecord RecordOf(int64 Tick, FName CivID)
	{
		FMegaprojectRecord Record;
		Record.Row = CosmicTests::StandingProject();
		Record.Tick = Tick;
		Record.CivID = CivID;
		return Record;
	}

	// The hazard Hazard on SystemIndex, running out at the next tick.
	void ExpiresNext(FGalaxyState& State, int32 SystemIndex, const TCHAR* Hazard)
	{
		State.Systems[SystemIndex].ActiveHazard = Tag(Hazard);
		State.Systems[SystemIndex].HazardTicksRemaining = 1;
	}

	// The index in the log of the first line of Row told at Tick; INDEX_NONE for none.
	int32 LineOf(const FGalaxyState& State, FName Row, int64 Tick)
	{
		return State.Log.IndexOfByPredicate([Row, Tick](const FCosmicLogEntry& Entry) { return Entry.SourceRow == Row && Entry.Tick == Tick; });
	}

	// How many numbers a stream drew from FromSeed to ToSeed; INDEX_NONE beyond a bound no tick of a test reaches.
	int32 DrawsFrom(int32 FromSeed, int32 ToSeed)
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
}

// The extinction's expiry (header section 2.2, 2.11, decision 5 - CosmicHazards::LetPeopleGo, KillsTheRulerOf): (1) the gate
// comes first and is asked whatever the number of the people's systems - a grace that fires keeps the whole people, its
// systems, its ruler's life and every line of the loss (mutation: the gate after the loss; or asked of the last settled
// system alone); (1') the grace answers the people's extinction, not one system: a second extinction of the people expiring
// on the same tick ends with the first, without result (mutation: the grace empties its own slot only); (2) otherwise the
// people lets every system go - no owner, both project slots and the hazard slot empty, its
// other hazards ending without their expiry, every fleet target on them lifted, the completion records kept (a record of no
// people stamped with the people) - and the ruler dies (Death.Hazard, the row as its source -
// mutation: the system as the subject; the hook without its KillPlayer; the deposition first), told by the extinction's own
// death line; the expiry line, then the record line right after it (mutation: the record first); the stage clock notes the
// death on the hook's subject; the ruled people leaves the civilizations the next tick; (2') LetPeopleGo itself lifts every
// fleet target on the people's systems (mutation: no lift - the AI stage clears a target on an unowned system within the tick,
// so the window is called alone) and ends a rebellion on them as Transfer does - its leader's record closed, the count of
// discontent at 0 (mutation: left open); (3) the birth protection hides the death and the deposition is written - the
// settlement under way goes with the rest, or the shielded ruler would keep a people with nothing to rule (mutation: only the
// settled systems go); (4) an AI people leaves that tick, the vessel untouched, and the expiry draws its two
// numbers and no more (mutation: one more); (4') the grace does not choose its people - an AI people's extinction asks the
// gate as much as the ruled one's, and a grace that fires keeps it and ends its other extinction (mutation: only the ruled
// people asks the gate); another people's extinction stands (mutation: the grace ends every extinction); (4'') the death
// reaches the ruler of the people, not a wanderer standing on its system (mutation: a KillPlayer on the system as well);
// (5) KillsTheRulerOf is the extinction's alone (mutation: another tag).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimExtinctionExpiryTest, "Cosmic_Ascension.Sim.ExtinctionExpiry", CosmicTests::Flags)
bool FCosmicSimExtinctionExpiryTest::RunTest(const FString& Parameters)
{
	using namespace CosmicUpperRungTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FGameplayTag ExtinctionTag = Tag(TEXT("Hazard.Extinction"));
	const FGameplayTag Spent = Tag(TEXT("Flag.Grace.Spent"));

	// (1) The gate first: people 0 holds two settled systems, the extinction stands on 1 and the grace is unspent.
	{
		FGalaxyState* Board = LayExtinctionBoard(*Sim, Original, true);
		if (!TestNotNull(TEXT("The board of the grace is laid"), Board))
		{
			return false;
		}
		ExpiresNext(*Board, 1, TEXT("Hazard.Extinction"));
		Board->Player.CurrentSystemID = Board->Systems[5].SystemID;
		Sim->StepOnce();
		const int64 Kept = Board->Tick;
		TestTrue(TEXT("A people with two settled systems asks the gate: the grace fires"), Board->Flags.HasTagExact(Spent) && LineOf(*Board, CosmicRows::EventGraceRemnant, Kept) != INDEX_NONE);
		TestTrue(TEXT("and the people keeps both systems"), Board->Systems[0].OwnerCivID == Board->Civs[0].CivID && Board->Systems[1].OwnerCivID == Board->Civs[0].CivID
			&& Board->Civs[0].OwnedSystems.Num() == 2);
		TestFalse(TEXT("The kept system's slot is empty"), Board->Systems[1].ActiveHazard.IsValid());
		TestTrue(TEXT("The ruler lives and rules: no death and no deposition was recorded"), Board->Calling.LifeState == ELifeState::Embodied
			&& Board->Calling.Lives.Last().DiedTick == 0 && Board->Player.Role == EPlayerRole::Ruler && Board->Civs[0].bIsPlayerRuled);
		TestEqual(TEXT("No expiry line is told"), LineOf(*Board, Extinct, Kept), static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("and no record line"), LineOf(*Board, CosmicRows::LogRecordExtinct, Kept), static_cast<int32>(INDEX_NONE));
	}

	// (1') Two extinctions of people 0, on 0 and 1, running out on the same tick, the grace unspent: the first in Systems order
	// asks the gate, and the grace ends the second with it.
	{
		FGalaxyState* Board = LayExtinctionBoard(*Sim, Original, true);
		if (!TestNotNull(TEXT("The board of the two extinctions is laid"), Board))
		{
			return false;
		}
		ExpiresNext(*Board, 0, TEXT("Hazard.Extinction"));
		ExpiresNext(*Board, 1, TEXT("Hazard.Extinction"));
		Sim->StepOnce();
		const int64 Kept = Board->Tick;
		int32 Graces = 0;
		for (const FCosmicLogEntry& Entry : Board->Log)
		{
			Graces += Entry.SourceRow == CosmicRows::EventGraceRemnant && Entry.Tick == Kept ? 1 : 0;
		}
		TestEqual(TEXT("One grace line for the people's two extinctions"), Graces, 1);
		TestTrue(TEXT("The people keeps both systems"), Board->Systems[0].OwnerCivID == Board->Civs[0].CivID && Board->Systems[1].OwnerCivID == Board->Civs[0].CivID
			&& Board->Civs[0].OwnedSystems.Num() == 2);
		TestTrue(TEXT("and both slots are empty"), !Board->Systems[0].ActiveHazard.IsValid() && !Board->Systems[1].ActiveHazard.IsValid());
		TestTrue(TEXT("The ruler lives and rules"), Board->Calling.LifeState == ELifeState::Embodied && Board->Calling.Lives.Last().DiedTick == 0
			&& Board->Player.Role == EPlayerRole::Ruler && Board->Civs[0].bIsPlayerRuled && !Board->Calling.PendingEffect.IsValid());
		TestEqual(TEXT("No expiry line is told"), LineOf(*Board, Extinct, Kept), static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("and no record line"), LineOf(*Board, CosmicRows::LogRecordExtinct, Kept), static_cast<int32>(INDEX_NONE));
	}

	// (2) The loss of the people the vessel rules, the vessel standing in people 2's system 5 and the grace spent: the extinction
	// on 0, a plague on 1 running out on the same tick (later in Systems order: its expiry comes after the extinction's); a
	// project in both construction slots and the plague's answer in 1's response slot; a completion record of no people on 0
	// and one of people 0 on 1; people 1's fleet aims at 1.
	{
		FGalaxyState* Board = LayExtinctionBoard(*Sim, Original, false);
		if (!TestNotNull(TEXT("The board of the loss is laid"), Board))
		{
			return false;
		}
		const FName Ruled = Board->Civs[0].CivID;
		ExpiresNext(*Board, 0, TEXT("Hazard.Extinction"));
		ExpiresNext(*Board, 1, TEXT("Hazard.Plague"));
		Board->Systems[0].ActiveMegaproject = CosmicTests::StandingProject();
		Board->Systems[0].MegaprojectProgress = 30.f;
		Board->Systems[1].ActiveMegaproject = CosmicTests::StandingProject();
		Board->Systems[1].MegaprojectProgress = 10.f;
		Board->Systems[1].ResponseMegaproject = CosmicTests::StandingProject();
		Board->Systems[1].ResponseProgress = 20.f;
		Board->Systems[0].CompletedMegaprojects.Add(RecordOf(120, NAME_None));
		Board->Systems[1].CompletedMegaprojects.Add(RecordOf(100, Ruled));
		Board->Civs[1].FleetTargetSystemID = Board->Systems[1].SystemID;
		Board->Player.CurrentSystemID = Board->Systems[5].SystemID;
		Sim->GetStageClock() = FCosmicStageClock();
		Sim->GetStageClock().bOn = true;
		Sim->StepOnce();
		const int64 Lost = Board->Tick;
		const FCosmicStageClock::FExpiry* Noted = Sim->GetStageClock().Expiries.FindByPredicate([Board](const FCosmicStageClock::FExpiry& Expiry) { return Expiry.SystemID == Board->Systems[0].SystemID; });
		TestTrue(TEXT("The stage clock notes that the death reaches the ruler standing elsewhere (the hook's subject: no system)"), Noted && Noted->bReached && !Noted->bGrace);
		Sim->GetStageClock() = FCosmicStageClock();

		for (const int32 SystemIndex : { 0, 1 })
		{
			const FStarSystemState& System = Board->Systems[SystemIndex];
			TestTrue(*FString::Printf(TEXT("System %d is let go to no one"), SystemIndex), System.OwnerCivID.IsNone());
			TestTrue(*FString::Printf(TEXT("System %d: both project slots and the hazard slot are empty"), SystemIndex), System.ActiveMegaproject.IsNone() && System.MegaprojectProgress == 0.f
				&& System.ResponseMegaproject.IsNone() && System.ResponseProgress == 0.f && !System.ActiveHazard.IsValid());
		}
		const int32 RuledIndex = Sim->GetStateIndex().FindCiv(*Board, Ruled);
		if (TestTrue(TEXT("The ruled people stays in the civilizations through the tick of its loss"), Board->Civs.IsValidIndex(RuledIndex)))
		{
			TestEqual(TEXT("owning nothing"), Board->Civs[RuledIndex].OwnedSystems.Num(), 0);
		}
		TestTrue(TEXT("The completion records stay"), Board->Systems[0].CompletedMegaprojects.Num() == 1 && Board->Systems[1].CompletedMegaprojects.Num() == 1
			&& Board->Systems[1].CompletedMegaprojects[0].CivID == Ruled);
		TestTrue(TEXT("and a record of no people is the people's"), Board->Systems[0].CompletedMegaprojects.Num() == 1 && Board->Systems[0].CompletedMegaprojects[0].CivID == Ruled);
		TestTrue(TEXT("(no fleet target on them is left after the tick - the lift itself is case (2'))"), Board->Civs[1].FleetTargetSystemID.IsNone());
		TestEqual(TEXT("The plague of 1 ended with its slot: its expiry turned back, untold"), LineOf(*Board, Plague, Lost), static_cast<int32>(INDEX_NONE));

		const FLifeRecord& Life = Board->Calling.Lives.Last();
		TestEqual(TEXT("The ruler died on the tick of the expiry, standing in another people's system"), Life.DiedTick, Lost);
		TestTrue(TEXT("by Death.Hazard"), Life.CauseOfDeath == Tag(TEXT("Death.Hazard")));
		TestEqual(TEXT("with the extinction's row as the source"), Life.CauseSourceRow, Extinct);
		TestTrue(TEXT("Its reign ended by the death"), !Life.ReignEndReasons.IsEmpty() && Life.ReignEndReasons.Last() == Tag(TEXT("Reign.Died")));
		TestTrue(TEXT("The calling is in the interregnum"), Board->Calling.LifeState == ELifeState::Interregnum);
		TestTrue(TEXT("The death is told by the extinction's own death line"), LineOf(*Board, CosmicRows::LogDeathExtinction, Lost) != INDEX_NONE);
		TestEqual(TEXT("not by the line of one person a star lost"), LineOf(*Board, CosmicRows::LogDeathHazard, Lost), static_cast<int32>(INDEX_NONE));

		const int32 Expiry = LineOf(*Board, Extinct, Lost);
		const int32 Record = LineOf(*Board, CosmicRows::LogRecordExtinct, Lost);
		TestTrue(TEXT("The expiry line is told"), Expiry != INDEX_NONE);
		TestTrue(TEXT("and the record line right after it"), Record != INDEX_NONE && Record == Expiry + 1);
		if (Board->Log.IsValidIndex(Record))
		{
			TestTrue(TEXT("The record line is about the system and the people that lost it, kept untold"), Board->Log[Record].FocusSystemID == Board->Systems[0].SystemID
				&& Board->Log[Record].FocusCivID == Ruled && Board->Log[Record].bUntold && Board->Log[Record].Category == CosmicTags::LogRecord());
		}

		Sim->StepOnce();
		TestEqual(TEXT("The next tick's crossing stage takes the ruled people"), Sim->GetStateIndex().FindCiv(*Board, Ruled), static_cast<int32>(INDEX_NONE));
	}

	// (2') LetPeopleGo itself, outside a tick (the AI stage after the crisis stage lets go of a fleet target on a system no one
	// owns, so a tick cannot show the lift): every fleet target on the people's systems is lifted, another stays; a rebellion
	// on its capital ends as Transfer ends one (and the crossing stage would take the people's records with it, so a tick
	// cannot show that either); nothing is drawn; a second call finds nothing left to let go.
	{
		FGalaxyState* Board = LayExtinctionBoard(*Sim, Original, false);
		if (!TestNotNull(TEXT("The board of the window is laid"), Board))
		{
			return false;
		}
		Board->Civs[1].FleetTargetSystemID = Board->Systems[1].SystemID;
		Board->Civs[2].FleetTargetSystemID = Board->Systems[0].SystemID;
		Board->Civs[3].FleetTargetSystemID = Board->Systems[7].SystemID;
		Board->Systems[0].ActiveHazard = Tag(TEXT("Hazard.Rebellion"));
		Board->Systems[0].HazardTicksRemaining = 50;
		const int32 RebelIndex = Board->Civs[0].Notables.Num();
		FNotableRecord& Rebel = Board->Civs[0].Notables.AddDefaulted_GetRef();
		Rebel.Role = Tag(TEXT("Notable.Rebel"));
		Rebel.FirstTick = 350;
		Board->Civs[0].LowStabilityTicks = 37;
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		TestTrue(TEXT("LetPeopleGo lets the people's systems go"), CosmicHazards::LetPeopleGo(Ctx, 0));
		TestTrue(TEXT("A rebellion on a system let go closes its leader's record"), Board->Civs[0].Notables.IsValidIndex(RebelIndex)
			&& Board->Civs[0].Notables[RebelIndex].LastTick == FMath::Max<int64>(Board->Tick, 1));
		TestEqual(TEXT("and the count of discontent starts over"), Board->Civs[0].LowStabilityTicks, 0);
		TestFalse(TEXT("and its slot is empty"), Board->Systems[0].ActiveHazard.IsValid());
		TestTrue(TEXT("and lifts every fleet target on them"), Board->Civs[1].FleetTargetSystemID.IsNone() && Board->Civs[2].FleetTargetSystemID.IsNone());
		TestEqual(TEXT("A target on another people's system stays"), Board->Civs[3].FleetTargetSystemID, Board->Systems[7].SystemID);
		TestTrue(TEXT("The people owns nothing"), Board->Civs[0].OwnedSystems.IsEmpty() && Board->Systems[0].OwnerCivID.IsNone() && Board->Systems[1].OwnerCivID.IsNone());
		TestEqual(TEXT("It draws nothing"), Sim->GetRng().GetCurrentSeed(), Seed);
		TestFalse(TEXT("A second call has nothing left to let go"), CosmicHazards::LetPeopleGo(Ctx, 0));
	}

	// (3) The birth protection hides the death: the deposition is written, and the people leaves all the same. People 0 also
	// holds 2, a settlement under way (project_settle of WithoutProjects - unpaid, it stays one through the tick): it goes with
	// the rest, or the people would keep a system, no deposition would be written and the crossing stage would keep a ruled
	// people with nothing to rule.
	{
		FGalaxyState* Board = LayExtinctionBoard(*Sim, Original, false);
		if (!TestNotNull(TEXT("The board of the shielded death is laid"), Board))
		{
			return false;
		}
		Board->Civs[1].OwnedSystems.Remove(Board->Systems[2].SystemID);
		Board->Systems[2].OwnerCivID = NAME_None;
		CosmicTests::Give(*Board, 0, { 2 });
		Board->Systems[2].ActiveMegaproject = CosmicRows::ProjectSettle;
		Sim->RebuildStateIndex();
		ExpiresNext(*Board, 1, TEXT("Hazard.Extinction"));
		Board->Calling.BornTick = Board->Tick;
		Sim->StepOnce();
		const FLifeRecord& Life = Board->Calling.Lives.Last();
		TestEqual(TEXT("A vessel within its birth protection does not die"), Life.DiedTick, static_cast<int64>(0));
		TestTrue(TEXT("The deposition the death left free was written: the vessel walks"), Board->Calling.LifeState == ELifeState::Embodied && Board->Player.Role == EPlayerRole::Wanderer);
		TestTrue(TEXT("its reign ended as a deposition"), !Life.ReignEndReasons.IsEmpty() && Life.ReignEndReasons.Last() == Tag(TEXT("Reign.Deposed")));
		TestTrue(TEXT("The people let every system go, the one being settled too"), Board->Systems[0].OwnerCivID.IsNone() && Board->Systems[1].OwnerCivID.IsNone()
			&& Board->Systems[2].OwnerCivID.IsNone() && Board->Systems[2].ActiveMegaproject.IsNone());
	}

	// (4) An AI people: people 3 loses {9, 8, 10, 11} to the extinction on 8. Beside it, the same tick from the same state with
	// people 3's systems let go by hand and no extinction: the expiry draws its two numbers and nothing more.
	{
		FGalaxyState* Board = LayExtinctionBoard(*Sim, Original, false);
		if (!TestNotNull(TEXT("The board of the AI people is laid"), Board))
		{
			return false;
		}
		const FName Gone = Board->Civs[3].CivID;
		const FGalaxyState Bare = *Board;
		ExpiresNext(*Board, 8, TEXT("Hazard.Extinction"));
		const FGalaxyState Before = *Board;
		Sim->StepOnce();
		const int64 Lost = Board->Tick;
		const int32 Seed = Board->RngState;
		TestEqual(TEXT("An AI people with no system left leaves the civilizations on the tick of its loss"), Sim->GetStateIndex().FindCiv(*Board, Gone), static_cast<int32>(INDEX_NONE));
		TestTrue(TEXT("Its four systems are no one's"), Board->Systems[8].OwnerCivID.IsNone() && Board->Systems[9].OwnerCivID.IsNone()
			&& Board->Systems[10].OwnerCivID.IsNone() && Board->Systems[11].OwnerCivID.IsNone());
		TestTrue(TEXT("The vessel is untouched: it rules on, no role effect waits"), Board->Calling.LifeState == ELifeState::Embodied && Board->Player.Role == EPlayerRole::Ruler
			&& Board->Calling.Lives.Last().DiedTick == 0 && !Board->Calling.PendingEffect.IsValid());
		TestTrue(TEXT("The record line follows the expiry line"), LineOf(*Board, Extinct, Lost) != INDEX_NONE && LineOf(*Board, CosmicRows::LogRecordExtinct, Lost) == LineOf(*Board, Extinct, Lost) + 1);

		FGalaxyState Emptied = Bare;
		for (const int32 SystemIndex : { 8, 9, 10, 11 })
		{
			Emptied.Systems[SystemIndex].OwnerCivID = NAME_None;
		}
		Emptied.Civs[3].OwnedSystems.Reset();
		Sim->LoadState(Emptied);
		Sim->StepOnce();
		TestEqual(TEXT("(the people emptied by hand leaves the civilizations too)"), Sim->GetStateIndex().FindCiv(Sim->GetState(), Gone), static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("The extinction's tick draws two numbers more than the same tick without it - its expiry's two, nothing in its code"),
			DrawsFrom(Sim->GetState().RngState, Seed), 2);
		TestEqual(TEXT("(both ticks began from one stream)"), Before.RngState, Bare.RngState);
	}

	// (4') The grace does not choose its people: people 3 (AI) with the grace unspent, the extinction on 8 running out and a
	// second one standing on 10; another standing on 1, the ruled people's.
	{
		FGalaxyState* Board = LayExtinctionBoard(*Sim, Original, true);
		if (!TestNotNull(TEXT("The board of the AI people's grace is laid"), Board))
		{
			return false;
		}
		const FName Kept3 = Board->Civs[3].CivID;
		ExpiresNext(*Board, 8, TEXT("Hazard.Extinction"));
		Board->Systems[10].ActiveHazard = ExtinctionTag;
		Board->Systems[10].HazardTicksRemaining = 50;
		Board->Systems[1].ActiveHazard = ExtinctionTag;
		Board->Systems[1].HazardTicksRemaining = 50;
		Sim->StepOnce();
		const int64 Kept = Board->Tick;
		TestTrue(TEXT("An AI people's extinction asks the gate: the grace fires"), Board->Flags.HasTagExact(Spent) && LineOf(*Board, CosmicRows::EventGraceRemnant, Kept) != INDEX_NONE);
		const int32 Index3 = Sim->GetStateIndex().FindCiv(*Board, Kept3);
		TestTrue(TEXT("and the people stays in the civilizations with its four systems"), Board->Civs.IsValidIndex(Index3) && Board->Civs[Index3].OwnedSystems.Num() == 4);
		for (const int32 SystemIndex : { 8, 9, 10, 11 })
		{
			TestEqual(*FString::Printf(TEXT("System %d stays the people's"), SystemIndex), Board->Systems[SystemIndex].OwnerCivID, Kept3);
		}
		TestFalse(TEXT("The kept system's slot is empty"), Board->Systems[8].ActiveHazard.IsValid());
		TestFalse(TEXT("and the people's other extinction ended with it"), Board->Systems[10].ActiveHazard.IsValid());
		TestTrue(TEXT("Another people's extinction stands through the grace"), Board->Systems[1].ActiveHazard == ExtinctionTag && Board->Systems[1].HazardTicksRemaining == 49);
		TestEqual(TEXT("No expiry line"), LineOf(*Board, Extinct, Kept), static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("and no record line"), LineOf(*Board, CosmicRows::LogRecordExtinct, Kept), static_cast<int32>(INDEX_NONE));
		TestTrue(TEXT("The vessel's people is untouched"), Board->Civs[0].bIsPlayerRuled && Board->Player.Role == EPlayerRole::Ruler);
	}

	// (4'') A wanderer standing on the extinct people's system 8 (the deposition's way - no draw; tick 400, past the first
	// vessel's birth protection): the death goes to the ruler of the people, so it reaches no one; the people goes all the same.
	{
		FGalaxyState* Board = LayExtinctionBoard(*Sim, Original, false);
		UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
		if (!TestNotNull(TEXT("The board of the wanderer is laid"), Board) || !TestNotNull(TEXT("The role subsystem"), Role))
		{
			return false;
		}
		const FName Gone = Board->Civs[3].CivID;
		Board->Player.CurrentSystemID = Board->Systems[8].SystemID;
		Role->BecomeWanderer(Tag(TEXT("AI.Profile.Conservative")), Tag(TEXT("Reign.Deposed")));
		Sim->SyncRngState();
		ExpiresNext(*Board, 8, TEXT("Hazard.Extinction"));
		Sim->StepOnce();
		const int64 Lost = Board->Tick;
		TestTrue(TEXT("The wanderer on the extinct people's system lives: no death was recorded"), !Board->Calling.PendingEffect.IsValid()
			&& Board->Calling.Lives.Last().DiedTick == 0 && Board->Calling.LifeState == ELifeState::Embodied);
		TestTrue(TEXT("and walks on where it stood"), Board->Player.Role == EPlayerRole::Wanderer && Board->Player.CurrentSystemID == Board->Systems[8].SystemID);
		TestEqual(TEXT("No death line"), LineOf(*Board, CosmicRows::LogDeathExtinction, Lost), static_cast<int32>(INDEX_NONE));
		TestTrue(TEXT("The people let its four systems go all the same"), Board->Systems[8].OwnerCivID.IsNone() && Board->Systems[9].OwnerCivID.IsNone()
			&& Board->Systems[10].OwnerCivID.IsNone() && Board->Systems[11].OwnerCivID.IsNone());
		TestEqual(TEXT("and left the civilizations that tick"), Sim->GetStateIndex().FindCiv(*Board, Gone), static_cast<int32>(INDEX_NONE));
	}

	// (5) The one answer: the extinction kills the ruler, no other hazard does.
	TestTrue(TEXT("KillsTheRulerOf Hazard.Extinction"), CosmicHazards::KillsTheRulerOf(ExtinctionTag));
	for (const TCHAR* Other : { TEXT("Hazard.PreSupernova"), TEXT("Hazard.AIAwakening"), TEXT("Hazard.Plague"), TEXT("Hazard.SubspaceErosion"), TEXT("Hazard.Cult"),
		TEXT("Hazard.Rebellion"), TEXT("Hazard.Claim"), TEXT("Hazard.Visitation") })
	{
		TestFalse(*FString::Printf(TEXT("KillsTheRulerOf %s"), Other), CosmicHazards::KillsTheRulerOf(Tag(Other)));
	}
	TestFalse(TEXT("KillsTheRulerOf no tag"), CosmicHazards::KillsTheRulerOf(FGameplayTag()));
	return true;
}

// ---- B-6b2c: the ark's slot, the delay above the third rung, the brake on the fourth rung and the binding ----

namespace CosmicUpperRungTests
{
	// The rows of the boards below (the boards' own ark - the game's project_ark is in the data from [B-6b6] - and the
	// game's binding row's values are [B-6b7]'s): the binding of the fourth rung, a construction of the third (Upper) and
	// of the second (Lower), the ark of the third that answers the extinction - each 4 of progress at Energy 1 a tick,
	// three lines -, and an independent transcendent beside the game's fallen row.
	const FName Ark(TEXT("project_test_ark"));
	const FName Upper(TEXT("project_test_upper"));
	const FName Lower(TEXT("project_test_lower"));
	const FName Still(TEXT("trans_test_still"));

	FMegaprojectDef ProjectOf(EAscensionTier Tier, const TCHAR* Stem)
	{
		FMegaprojectDef Row;
		Row.DisplayName = FText::AsCultureInvariant(FString::Printf(TEXT("<%s>"), Stem));
		Row.RequiredTier = Tier;
		Row.CostPerTick.Energy = 1.f;
		Row.RequiredProgress = 4.f;
		for (const TCHAR* Variant : { TEXT("one"), TEXT("two"), TEXT("three") })
		{
			Row.CompleteTemplates.Add(FText::AsCultureInvariant(FString::Printf(TEXT("{Civ} {System} {Project} %s done %s"), Stem, Variant)));
		}
		return Row;
	}

	/**
	 * LayBoard's board (people 0 - the vessel's, capital 0 - holds {0, 1}; 1 holds {3, 2, 4}; 2 {6, 5, 7}; 3 {9, 8, 10, 11})
	 * with this block's projects (and WithoutProjects' unpaid project_settle), the extinction of LinedHazardOf answered by the
	 * ark as its one hazard row, and the game's fallen row beside an independent copy of it (no instance: a case adds its own).
	 * Null when the board is not laid or the game has no fallen row.
	 */
	FGalaxyState* LayBindingBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Original, UDataTable* Feats, TFunctionRef<void(FCosmicRulesDef&)> Edit)
	{
		const FTranscendentDef* Silence = Original.FindTranscendent(CosmicRows::TransSilence);
		FGalaxyState* Board = Silence ? LayBoard(Sim, Original, Feats, Edit) : nullptr;
		if (!Board)
		{
			return nullptr;
		}
		FCosmicDataTables Tables = Sim.GetTables();
		UDataTable* Projects = CosmicTests::NewTable(FMegaprojectDef::StaticStruct());
		Projects->AddRow(CosmicRows::ProjectBinding, ProjectOf(EAscensionTier::Tier4_Transcendent, TEXT("binding")));
		Projects->AddRow(Upper, ProjectOf(EAscensionTier::Tier3_Causal, TEXT("upper")));
		Projects->AddRow(Lower, ProjectOf(EAscensionTier::Tier2_Stellar, TEXT("lower")));
		Projects->AddRow(Ark, ProjectOf(EAscensionTier::Tier3_Causal, TEXT("ark")));
		if (const FMegaprojectDef* Settle = Tables.FindMegaproject(CosmicRows::ProjectSettle))
		{
			Projects->AddRow(CosmicRows::ProjectSettle, *Settle);
		}
		Tables.Megaprojects = Projects;
		UDataTable* HazardRows = CosmicTests::NewTable(FHazardDef::StaticStruct());
		FHazardDef Extinction = LinedHazardOf(TEXT("Hazard.Extinction"));
		Extinction.RulerResolutions = { Ark };
		HazardRows->AddRow(Extinct, Extinction);
		Tables.Hazards = HazardRows;
		UDataTable* Transcendents = CosmicTests::NewTable(FTranscendentDef::StaticStruct());
		Transcendents->AddRow(CosmicRows::TransSilence, *Silence);
		FTranscendentDef Independent = *Silence;
		Independent.Allegiance = Tag(TEXT("Realm.Independent"));
		Transcendents->AddRow(Still, Independent);
		Tables.Transcendents = Transcendents;
		Sim.SetTables(Tables);
		Sim.SyncRngState();
		return Board;
	}

	// Whether a system holds a completion record of the binding's row.
	bool HoldsBinding(const FStarSystemState& System)
	{
		return System.CompletedMegaprojects.ContainsByPredicate([](const FMegaprojectRecord& Each) { return Each.Row == CosmicRows::ProjectBinding; });
	}
}

// The ark's slot (header section 2.6, notes 2 b): an AI people's answer to an extinction starts in the AI stage and runs in the
// response slot at the AI rate alone - never delayed, though the ark is a row of the third rung and the delay above it is 0.25
// (mutation: the delay on the response slot) -, completes within the countdown, resolves the extinction (no expiry, the
// people keeps its systems) and records its maker.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimArkAnswersTest, "Cosmic_Ascension.Sim.ArkAnswers", CosmicTests::Flags)
bool FCosmicSimArkAnswersTest::RunTest(const FString& Parameters)
{
	using namespace CosmicUpperRungTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* Board = LayBindingBoard(*Sim, Original, nullptr, [](FCosmicRulesDef& Rules)
	{
		Rules.AIAscensionSpeedMultiplier = 1.f;
		Rules.AIUpperRungProgressMultiplier = 0.25f;
	});
	if (!TestNotNull(TEXT("The board of the ark is laid"), Board))
	{
		return false;
	}
	const FName Maker = Board->Civs[1].CivID;
	Board->Civs[1].Tier = EAscensionTier::Tier3_Causal;
	Board->Systems[3].ActiveHazard = Tag(TEXT("Hazard.Extinction"));
	Board->Systems[3].HazardTicksRemaining = 50;
	Sim->StepOnce();
	TestEqual(TEXT("The AI people starts the ark in the response slot of the extinction's system"), Board->Systems[3].ResponseMegaproject, Ark);
	TestEqual(TEXT("(in the AI stage, after this tick's crisis stage: no progress yet)"), Board->Systems[3].ResponseProgress, 0.f);
	Sim->StepOnce();
	TestEqual(TEXT("The ark progresses by the AI rate, 1 - the delay above the third rung is the construction slot's alone"), Board->Systems[3].ResponseProgress, 1.f);
	Sim->StepOnce();
	Sim->StepOnce();
	Sim->StepOnce();
	const int64 Done = Board->Tick;
	TestTrue(TEXT("At its fourth paid tick the ark completes and empties its slot"), Board->Systems[3].ResponseMegaproject.IsNone() && Board->Systems[3].ResponseProgress == 0.f);
	TestFalse(TEXT("and the extinction is resolved"), Board->Systems[3].ActiveHazard.IsValid());
	const FMegaprojectRecord* Made = Board->Systems[3].CompletedMegaprojects.FindByPredicate([](const FMegaprojectRecord& Each) { return Each.Row == Ark; });
	if (TestNotNull(TEXT("The ark's completion is recorded"), Made))
	{
		TestEqual(TEXT("at its tick"), Made->Tick, Done);
		TestEqual(TEXT("by the people that built it"), Made->CivID, Maker);
	}
	TestEqual(TEXT("The people keeps its three systems"), Board->Civs[1].OwnedSystems.Num(), 3);
	TestFalse(TEXT("No expiry line is told"), Board->Log.ContainsByPredicate([](const FCosmicLogEntry& Entry) { return Entry.Message.ToString().Contains(TEXT("Hazard.Extinction expiry")); }));
	TestTrue(TEXT("but its resolution line"), Board->Log.ContainsByPredicate([Done](const FCosmicLogEntry& Entry) { return Entry.Tick == Done && Entry.Message.ToString().Contains(TEXT("Hazard.Extinction resolution")); }));
	return true;
}

// The delay above the third rung (header section 2.6, 3.4, decision 17): DT_Rules.AIUpperRungProgressMultiplier multiplies the
// AI rate in the construction slot of a people the player does not rule, for a row of the third rung or above (the binding
// among them) - not a row of the second (mutation: the delay on a second-rung row too) and not the people the player rules,
// which progresses by 1. A knob not above 0, or not a number, is read as 1 with one error line (mutation: read as it is).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimUpperRungDelayTest, "Cosmic_Ascension.Sim.UpperRungDelay", CosmicTests::Flags)
bool FCosmicSimUpperRungDelayTest::RunTest(const FString& Parameters)
{
	using namespace CosmicUpperRungTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	// Every people on the fourth rung: 0 (the vessel's) builds Upper on 0, 1 Upper on 3, 2 Lower on 6, 3 the binding on 9 (a
	// fallen instance to bind). AI rate 2, the delay as given - 0.25 is exact in binary.
	auto Lay = [&](float Delay)
	{
		FGalaxyState* Board = LayBindingBoard(*Sim, Original, nullptr, [Delay](FCosmicRulesDef& Rules)
		{
			Rules.AIAscensionSpeedMultiplier = 2.f;
			Rules.AIUpperRungProgressMultiplier = Delay;
		});
		if (Board)
		{
			for (FCivilizationState& Civ : Board->Civs)
			{
				Civ.Tier = EAscensionTier::Tier4_Transcendent;
			}
			CosmicTests::AddInstance(*Board, CosmicRows::TransSilence, {});
			Board->Systems[0].ActiveMegaproject = Upper;
			Board->Systems[3].ActiveMegaproject = Upper;
			Board->Systems[6].ActiveMegaproject = Lower;
			Board->Systems[9].ActiveMegaproject = CosmicRows::ProjectBinding;
		}
		return Board;
	};
	FGalaxyState* Board = Lay(0.25f);
	if (!TestNotNull(TEXT("The board of the delay is laid"), Board))
	{
		return false;
	}
	Sim->StepOnce();
	TestEqual(TEXT("An AI people's construction of the third rung progresses by 2 x 0.25"), Board->Systems[3].MegaprojectProgress, 0.5f);
	TestEqual(TEXT("and so does its binding, of the fourth"), Board->Systems[9].MegaprojectProgress, 0.5f);
	TestEqual(TEXT("A construction of the second rung is not delayed: 2"), Board->Systems[6].MegaprojectProgress, 2.f);
	TestEqual(TEXT("The people the player rules progresses by 1"), Board->Systems[0].MegaprojectProgress, 1.f);

	// The knob's 1: the AI rate itself.
	FGalaxyState* Even = Lay(1.f);
	if (!TestNotNull(TEXT("The board of no delay is laid"), Even))
	{
		return false;
	}
	Sim->StepOnce();
	TestEqual(TEXT("At the knob's 1 the third rung's construction progresses by the AI rate, 2"), Even->Systems[3].MegaprojectProgress, 2.f);

	// A knob not above 0 or not a number: 1, with one error line a tick for each (one slot reads it - people 1's Upper; the
	// binding and the ruled people's Upper are taken out).
	AddExpectedError(TEXT("AIUpperRungProgressMultiplier"), EAutomationExpectedErrorFlags::Contains, 2);
	for (const float Bad : { 0.f, std::numeric_limits<float>::quiet_NaN() })
	{
		FGalaxyState* Again = Lay(Bad);
		if (!TestNotNull(TEXT("The board of a bad delay is laid"), Again))
		{
			return false;
		}
		Again->Systems[0].ActiveMegaproject = NAME_None;
		Again->Systems[9].ActiveMegaproject = NAME_None;
		Sim->StepOnce();
		TestEqual(*FString::Printf(TEXT("A delay of %f is read as 1: the AI rate, 2"), Bad), Again->Systems[3].MegaprojectProgress, 2.f);
	}
	return true;
}

// The brake on the fourth rung (header section 2.6, decision 16; docs/21 section 7.4, the count of the fourth rung - case by
// case): a people the player does not rule climbs from the third rung only while the peoples of its galaxy on the fourth rung
// or above - the ruled people counted (mutation: left out), another galaxy's not (mutation: every galaxy's) - and the
// crossings bound for it (mutation: bound anywhere) that have not landed stay below the home row's AIFourthRungCap. The judgment's order decides: the people first in Civs passes first (mutation: the order
// reversed), and a climb earlier in the tick is counted (mutation: the count read once a tick). The people the player rules
// (mutation: braked too) and the climb to the fifth rung (mutation: braked too) are never braked; a cap of 0 (mutation: still
// braking) and a galaxy row the tables do not hold (mutation: closed) are no brake. A blocked people climbs on the tick the
// count drops - a people of the fourth rung that let every system go leaves at the crossing stage (mutation: the count from the
// third rung, the asking people counting itself), a crossing that landed is no longer inbound (mutation: a landed crossing
// counted). No draw, no line. The race line's CosmicRaceMeasure::BrakeHeldOf (B6b [B-6b3b] - the judgment's walk without
// Ascend, read at the boundary) agrees with the judgment: the people held is held, the one that climbed is not, and neither is
// a people of the third under a cap of 0 (mutation: the window not asked), one whose deed is unmet (mutation: the deed not
// asked), one whose deed is met only by a row of another rung (mutation: every row asked), one whose every system is being
// settled (mutation: the systems being settled asked) or the people the player rules (mutation: counted).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimFourthRungBrakeTest, "Cosmic_Ascension.Sim.FourthRungBrake", CosmicTests::Flags)
bool FCosmicSimFourthRungBrakeTest::RunTest(const FString& Parameters)
{
	using namespace CosmicUpperRungTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FCosmicRulesDef* GameRules = Original.FindRules();
	if (!TestNotNull(TEXT("(the game has its rules row)"), GameRules))
	{
		return false;
	}
	const FName Home = GameRules->HomeGalaxyRow;
	// The rows: from the third rung, a condition every tick holds; from the fourth, the completion of a row no table holds,
	// laid by hand where a case wants a people to climb to the fifth.
	const FName Five(TEXT("project_test_five"));
	UDataTable* Feats = CosmicTests::NewTable(FAscensionFeatDef::StaticStruct());
	FAscensionFeatDef Fourth = FeatRow(EAscensionTier::Tier3_Causal, { Cond(EConditionType::GalaxyTickAtLeast, nullptr, 0.f) });
	FAscensionFeatDef Fifth = FeatRow(EAscensionTier::Tier4_Transcendent, { Cond(EConditionType::MegaprojectComplete, TEXT("project_test_five")) });
	for (const TCHAR* Variant : { TEXT("one"), TEXT("two"), TEXT("three") })
	{
		Fourth.LogTemplates.Add(FText::AsCultureInvariant(FString::Printf(TEXT("{Civ} climbed %s"), Variant)));
		Fifth.LogTemplates.Add(FText::AsCultureInvariant(FString::Printf(TEXT("{Civ} climbed on %s"), Variant)));
	}
	Feats->AddRow(FName(TEXT("feat_test_fourth")), Fourth);
	Feats->AddRow(FName(TEXT("feat_test_fifth")), Fifth);
	using T = EAscensionTier;
	// A board under the cap given (INDEX_NONE: no galaxy table at all), the four peoples on the rungs given.
	auto Lay = [&](int32 Cap, T Rung0, T Rung1, T Rung2, T Rung3) -> FGalaxyState*
	{
		FGalaxyState* Board = LayBoard(*Sim, Original, Feats, NoEdit);
		if (!Board)
		{
			return nullptr;
		}
		FCosmicDataTables Tables = Sim->GetTables();
		Tables.Galaxies = Cap == INDEX_NONE ? nullptr : CosmicTests::GalaxiesWithCap(Original, Cap);
		Sim->SetTables(Tables);
		Board->Civs[0].Tier = Rung0;
		Board->Civs[1].Tier = Rung1;
		Board->Civs[2].Tier = Rung2;
		Board->Civs[3].Tier = Rung3;
		Sim->SyncRngState();
		return Board;
	};
	auto RecordFifth = [&](FGalaxyState& Board, int32 CivIndex)
	{
		const int32 Capital = Sim->GetStateIndex().FindSystem(Board, Board.Civs[CivIndex].HomeSystemID);
		FMegaprojectRecord& Made = Board.Systems[Capital].CompletedMegaprojects.AddDefaulted_GetRef();
		Made.Row = Five;
		Made.Tick = 1;
		Made.CivID = Board.Civs[CivIndex].CivID;
	};

	// (1) Cap 2, the ruled people on the fourth rung, peoples 1 and 2 on the third: 1 climbs (1 < 2), then 2 is held (2 = 2).
	FGalaxyState* Board = Lay(2, T::Tier4_Transcendent, T::Tier3_Causal, T::Tier3_Causal, T::Tier1_Planetary);
	if (!TestNotNull(TEXT("The board of the brake is laid"), Board))
	{
		return false;
	}
	TestTrue(TEXT("(every people of the board stands in the home galaxy)"), Board->Civs[0].GalaxyRow == Home && Board->Civs[1].GalaxyRow == Home && Board->Civs[2].GalaxyRow == Home);
	TestTrue(TEXT("Open below the cap"), CosmicAscent::FourthRungOpenOf(*Board, Sim->GetTables(), Home));
	const int32 LinesBefore = Board->Log.Num();
	Sim->StepOnce();
	const int32 CappedSeed = Sim->GetRng().GetCurrentSeed();
	TestEqual(TEXT("The people first in Civs climbs while the count - the ruled people's one - is below the cap"), Board->Civs[1].Tier, T::Tier4_Transcendent);
	TestEqual(TEXT("The next is held: the ruled people and the climb just made fill the cap"), Board->Civs[2].Tier, T::Tier3_Causal);
	TestFalse(TEXT("Closed at the cap"), CosmicAscent::FourthRungOpenOf(*Board, Sim->GetTables(), Home));
	// B6b [B-6b3b]: the race line's count of the held (CosmicRaceMeasure::BrakeHeldOf - the judgment's walk without Ascend, read
	// at the boundary) agrees with the judgment: people 2 was held, people 1 climbed (mutation: the brake's window not asked).
	TestTrue(TEXT("BrakeHeldOf: the held people was held"), CosmicRaceMeasure::BrakeHeldOf(*Board, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 2));
	TestFalse(TEXT("BrakeHeldOf: the people that climbed was not"), CosmicRaceMeasure::BrakeHeldOf(*Board, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 1));
	{
		// The deed asked (mutation: not): a third rung's row no tick of the board meets holds nobody.
		UDataTable* Unmet = CosmicTests::NewTable(FAscensionFeatDef::StaticStruct());
		Unmet->AddRow(FName(TEXT("feat_test_fourth")), FeatRow(EAscensionTier::Tier3_Causal, { Cond(EConditionType::GalaxyTickAtLeast, nullptr, 1000000.f) }));
		FCosmicDataTables Later = Sim->GetTables();
		Later.Feats = Unmet;
		TestFalse(TEXT("BrakeHeldOf: a people the brake holds whose deed is unmet is not counted"), CosmicRaceMeasure::BrakeHeldOf(*Board, Sim->GetStateIndex(), Later, Sim->GetRules(), 2));
	}
	{
		// The judgment's rows alone (mutation: every row asked): a row of another rung whose deed is met holds nobody.
		UDataTable* OtherRung = CosmicTests::NewTable(FAscensionFeatDef::StaticStruct());
		OtherRung->AddRow(FName(TEXT("feat_test_fourth")), FeatRow(EAscensionTier::Tier3_Causal, { Cond(EConditionType::GalaxyTickAtLeast, nullptr, 1000000.f) }));
		OtherRung->AddRow(FName(TEXT("feat_test_third")), FeatRow(EAscensionTier::Tier2_Stellar, { Cond(EConditionType::GalaxyTickAtLeast, nullptr, 0.f) }));
		FCosmicDataTables Later = Sim->GetTables();
		Later.Feats = OtherRung;
		TestFalse(TEXT("BrakeHeldOf: a row of another rung whose deed is met is not asked"), CosmicRaceMeasure::BrakeHeldOf(*Board, Sim->GetStateIndex(), Later, Sim->GetRules(), 2));
	}
	{
		// The judgment's systems alone (mutation: a system being settled asked): a people whose every system is being settled has
		// no settled system for the deed to hold on.
		TArray<int32> Settling;
		bool bOwned = !Board->Civs[2].OwnedSystems.IsEmpty();
		for (const FName SystemID : Board->Civs[2].OwnedSystems)
		{
			const int32 SystemIndex = Sim->GetStateIndex().FindSystem(*Board, SystemID);
			bOwned &= Board->Systems.IsValidIndex(SystemIndex);
			Settling.Add(SystemIndex);
		}
		if (TestTrue(TEXT("(people 2 owns systems)"), bOwned))
		{
			for (const int32 SystemIndex : Settling)
			{
				Board->Systems[SystemIndex].ActiveMegaproject = CosmicRows::ProjectSettle;
			}
			TestFalse(TEXT("BrakeHeldOf: a people whose every system is being settled is not held"), CosmicRaceMeasure::BrakeHeldOf(*Board, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 2));
			for (const int32 SystemIndex : Settling)
			{
				Board->Systems[SystemIndex].ActiveMegaproject = NAME_None;
			}
		}
	}
	int32 Ascents = 0;
	for (int32 At = LinesBefore; At < Board->Log.Num(); ++At)
	{
		Ascents += Board->Log[At].Category == Tag(TEXT("Log.Ascension")) ? 1 : 0;
	}
	TestEqual(TEXT("One ascension line, the climb's: the brake tells nothing"), Ascents, 1);
	{
		FGalaxyState* Free = Lay(0, T::Tier4_Transcendent, T::Tier3_Causal, T::Tier3_Causal, T::Tier1_Planetary);
		TestFalse(TEXT("BrakeHeldOf: under a cap of 0 a people of the third with its deed met is not held"), CosmicRaceMeasure::BrakeHeldOf(*Free, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 1));
		Sim->StepOnce();
		TestEqual(TEXT("Under a cap of 0 both climb"), Free->Civs[2].Tier, T::Tier4_Transcendent);
		TestEqual(TEXT("The brake draws nothing: the held tick draws as the free one"), CappedSeed, Sim->GetRng().GetCurrentSeed());
	}

	// (2) Cap 2, peoples 1 and 2 on the fourth rung with the fifth's record, the ruled people on the third: it climbs, and so
	// do both to the fifth, at the cap.
	{
		FGalaxyState* Full = Lay(2, T::Tier3_Causal, T::Tier4_Transcendent, T::Tier4_Transcendent, T::Tier1_Planetary);
		RecordFifth(*Full, 1);
		RecordFifth(*Full, 2);
		// BrakeHeldOf never counts the ruled people (mutation: counted): on the third rung, the window closed (2 of 2), its deed met.
		TestTrue(TEXT("(the ruled people on the third rung, the window closed)"), Full->Civs[0].bIsPlayerRuled && !CosmicAscent::FourthRungOpenOf(*Full, Sim->GetTables(), Home));
		TestFalse(TEXT("BrakeHeldOf: the people the player rules is never held"), CosmicRaceMeasure::BrakeHeldOf(*Full, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0));
		Sim->StepOnce();
		TestEqual(TEXT("The people the player rules is never braked"), Full->Civs[0].Tier, T::Tier4_Transcendent);
		TestEqual(TEXT("The climb to the fifth rung is never braked"), Full->Civs[1].Tier, T::Tier5_EternalOrder);
		TestEqual(TEXT("(both peoples of the fourth climb to it)"), Full->Civs[2].Tier, T::Tier5_EternalOrder);
	}

	// (3) No brake: a cap of 0, and no galaxy table at all - three peoples of the third climb past a ruled people of the fourth.
	for (const int32 Cap : { 0, static_cast<int32>(INDEX_NONE) })
	{
		FGalaxyState* Free = Lay(Cap, T::Tier4_Transcendent, T::Tier3_Causal, T::Tier3_Causal, T::Tier3_Causal);
		Sim->StepOnce();
		const bool bAll = Free->Civs[1].Tier == T::Tier4_Transcendent && Free->Civs[2].Tier == T::Tier4_Transcendent && Free->Civs[3].Tier == T::Tier4_Transcendent;
		TestTrue(Cap == 0 ? TEXT("A cap of 0 is no brake: every people of the third climbs") : TEXT("A galaxy row the tables do not hold is no brake"), bAll);
	}

	// (4) The count drops: peoples 1 and 2 on the fourth rung, 3 on the third, held at the cap of 2. People 2 lets every
	// system go between two ticks: on the next tick it is still among the peoples at the AI stage (the crossing stage comes
	// after) and 3 is held once more; on the tick after, 3 climbs.
	{
		FGalaxyState* Drop = Lay(2, T::Tier1_Planetary, T::Tier4_Transcendent, T::Tier4_Transcendent, T::Tier3_Causal);
		Sim->StepOnce();
		TestEqual(TEXT("The people of the third is held at the cap"), Drop->Civs[3].Tier, T::Tier3_Causal);
		const FName Gone = Drop->Civs[2].CivID;
		const FName Held = Drop->Civs[3].CivID;
		{
			FCosmicEvalContext Ctx = Sim->MakeEvalContext();
			TestTrue(TEXT("(people 2 lets every system go)"), CosmicHazards::LetPeopleGo(Ctx, 2));
		}
		Sim->StepOnce();
		TestEqual(TEXT("(the crossing stage took it)"), Sim->GetStateIndex().FindCiv(*Drop, Gone), static_cast<int32>(INDEX_NONE));
		const int32 HeldIndex = Sim->GetStateIndex().FindCiv(*Drop, Held);
		TestTrue(TEXT("On that tick the people still counted holds the third at the cap"), Drop->Civs.IsValidIndex(HeldIndex) && Drop->Civs[HeldIndex].Tier == T::Tier3_Causal);
		Sim->StepOnce();
		TestTrue(TEXT("On the tick after, the count below the cap, the held people climbs"), Drop->Civs.IsValidIndex(HeldIndex) && Drop->Civs[HeldIndex].Tier == T::Tier4_Transcendent);
	}

	// (5) A crossing bound for the home galaxy counts until it lands: people 1 on the fourth rung, 3 on the third, one crossing
	// arriving at the tick that runs next - held (1 + 1 = 2); on the tick after, the crossing landed, it climbs.
	{
		FGalaxyState* Inbound = Lay(2, T::Tier1_Planetary, T::Tier4_Transcendent, T::Tier1_Planetary, T::Tier3_Causal);
		FTransitRecord& Crossing = Inbound->Transits.AddDefaulted_GetRef();
		Crossing.ToGalaxyRow = Home;
		Crossing.DepartTick = Inbound->Tick;
		Crossing.ArriveTick = Inbound->Tick + 1;
		Sim->StepOnce();
		TestEqual(TEXT("A crossing that lands on this tick is still inbound: the people of the third is held"), Inbound->Civs[3].Tier, T::Tier3_Causal);
		Sim->StepOnce();
		TestEqual(TEXT("Landed, it is no longer counted: the people climbs"), Inbound->Civs[3].Tier, T::Tier4_Transcendent);
		Inbound->Transits.Reset();
	}

	// (6) Only its galaxy: the ruled people on the fourth rung at home, people 1 on the fourth rung in another galaxy and a
	// crossing bound there that has not landed, 3 on the third at home - the home count is 1, so 3 climbs (mutations: the
	// peoples of every galaxy counted; the crossings bound anywhere counted - either alone fills the cap of 2).
	{
		const FName Far(TEXT("gal_andromeda"));
		FGalaxyState* Apart = Lay(2, T::Tier4_Transcendent, T::Tier4_Transcendent, T::Tier1_Planetary, T::Tier3_Causal);
		TestTrue(TEXT("(the tables hold the far row, another than home)"), Far != Home && Sim->GetTables().FindGalaxy(Far) != nullptr);
		Apart->Civs[1].GalaxyRow = Far;
		FTransitRecord& Away = Apart->Transits.AddDefaulted_GetRef();
		Away.ToGalaxyRow = Far;
		Away.DepartTick = Apart->Tick;
		Away.ArriveTick = Apart->Tick + 100;
		TestTrue(TEXT("Open: a people and a crossing of another galaxy are not counted"), CosmicAscent::FourthRungOpenOf(*Apart, Sim->GetTables(), Home));
		Sim->StepOnce();
		TestEqual(TEXT("The people of the third climbs past a people and a crossing of another galaxy"), Apart->Civs[3].Tier, T::Tier4_Transcendent);
		Apart->Transits.Reset();
	}
	return true;
}

// The binding (header section 2.5, decisions 12-15, 32): (1) CosmicSeal::BindTargetOf is the first unsealed instance of a
// fallen row in Transcendents order - an independent instance, one whose row the tables do not hold and a sealed one are
// passed by (mutations: any realm; a sealed one taken again; the walk from the end); none left, INDEX_NONE. (2) A binding
// that completes writes its record (the maker), then seals the target - bSealed and SealedTick = the tick, in one place
// (mutation: no seal) -, and the binding's line follows the completion line at once (mutation: told before it), {System}
// filled, with the completion line's Severity (the ruled people's 1 - mutation: told with 0); the fallen's claim no longer
// stands (ClaimantOf); nothing of the sim stream (mutation: a draw), one number of the tell stream more than another
// completion (mutation: a second tell draw). (3) A binding whose target is bound already ends at its next Advance without
// result - nothing paid, recorded or told, no number of the tell stream, SealedTick unmoved (mutation: the sealed one bound
// again). (4) Two bindings completing on one tick: the first in Systems order binds, the second ends without result and its
// people holds no completion (mutation: the target read once for the pass).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimBindingTest, "Cosmic_Ascension.Sim.Binding", CosmicTests::Flags)
bool FCosmicSimBindingTest::RunTest(const FString& Parameters)
{
	using namespace CosmicUpperRungTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	FGalaxyState* Board = LayBindingBoard(*Sim, Original, nullptr, NoEdit);
	if (!TestNotNull(TEXT("The board of the binding is laid"), Board))
	{
		return false;
	}

	// (1) The target.
	{
		const FCosmicDataTables& Tables = Sim->GetTables();
		CosmicTests::AddInstance(*Board, Still, {});
		CosmicTests::AddInstance(*Board, FName(TEXT("trans_test_gone")), {});
		TestEqual(TEXT("No fallen instance: no target"), CosmicSeal::BindTargetOf(*Board, Tables), static_cast<int32>(INDEX_NONE));
		CosmicTests::AddInstance(*Board, CosmicRows::TransSilence, {}).bSealed = true;
		TestEqual(TEXT("A sealed fallen one is no target"), CosmicSeal::BindTargetOf(*Board, Tables), static_cast<int32>(INDEX_NONE));
		CosmicTests::AddInstance(*Board, CosmicRows::TransSilence, {});
		TestEqual(TEXT("The first unsealed fallen one, past an independent one, a row-less one and a sealed one"), CosmicSeal::BindTargetOf(*Board, Tables), 3);
		CosmicTests::AddInstance(*Board, CosmicRows::TransSilence, {});
		TestEqual(TEXT("Of two unsealed fallen ones, the first in Transcendents order"), CosmicSeal::BindTargetOf(*Board, Tables), 3);
		Board->Transcendents.Reset();
	}

	// (2) People 1 on the fourth rung completes its binding on its capital 3 this tick; the target stands behind an
	// independent instance, its sphere {11} - next to 10, where its claim would stand.
	const FName Maker = Board->Civs[1].CivID;
	Board->Civs[1].Tier = EAscensionTier::Tier4_Transcendent;
	CosmicTests::AddInstance(*Board, Still, {});
	CosmicTests::AddInstance(*Board, CosmicRows::TransSilence, { Board->Systems[11].SystemID });
	Board->Systems[3].ActiveMegaproject = CosmicRows::ProjectBinding;
	Board->Systems[3].MegaprojectProgress = 3.f;
	const FGalaxyState Twin = *Board;
	TestEqual(TEXT("(before the binding the fallen one claims next to its sphere)"),
		CosmicHazards::ClaimantOf(*Board, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 10), 1);
	const float EnergyBefore = Board->Civs[1].Stockpile.Energy;
	Sim->StepOnce();
	const int64 Bound = Board->Tick;
	const int32 BoundSeed = Sim->GetRng().GetCurrentSeed();
	const int32 BoundTell = Board->TellRngState;
	TestTrue(TEXT("The binding seals the target"), Board->Transcendents[1].bSealed);
	TestEqual(TEXT("at the tick of its completion"), Board->Transcendents[1].SealedTick, Bound);
	TestFalse(TEXT("The independent instance stays as it was"), Board->Transcendents[0].bSealed);
	TestEqual(TEXT("The sphere stays: the binding freezes it"), Board->Transcendents[1].InfluencedSystems.Num(), 1);
	const FMegaprojectRecord* Made = Board->Systems[3].CompletedMegaprojects.FindByPredicate([](const FMegaprojectRecord& Each) { return Each.Row == CosmicRows::ProjectBinding; });
	TestTrue(TEXT("Its completion is recorded at the tick, by its maker"), Made && Made->Tick == Bound && Made->CivID == Maker);
	TestTrue(TEXT("and the slot is empty"), Board->Systems[3].ActiveMegaproject.IsNone() && Board->Systems[3].MegaprojectProgress == 0.f);
	TestEqual(TEXT("It paid its tick"), Board->Civs[1].Stockpile.Energy, EnergyBefore + Board->Civs[1].IncomePerTick.Energy - 1.f, 0.01f);
	const int32 Completion = LineOf(*Board, CosmicRows::ProjectBinding, Bound);
	const int32 BoundLine = LineOf(*Board, CosmicRows::LogFallenBound, Bound);
	TestTrue(TEXT("The completion line is told"), Completion != INDEX_NONE);
	TestTrue(TEXT("and the binding's line right after it"), BoundLine != INDEX_NONE && BoundLine == Completion + 1);
	if (Board->Log.IsValidIndex(BoundLine))
	{
		const FCosmicLogEntry& Line = Board->Log[BoundLine];
		TestTrue(TEXT("The binding's line names the system the work stood on, every slot filled"), Line.Message.ToString().Contains(Board->Systems[3].DisplayName.ToString())
			&& !Line.Message.ToString().Contains(TEXT("{")));
		TestTrue(TEXT("with the completion line's focus"), Line.FocusSystemID == Board->Systems[3].SystemID && Line.FocusCivID == Maker);
		TestEqual(TEXT("and its Severity (0: an AI people's work)"), static_cast<int32>(Line.Severity), 0);
	}
	TestEqual(TEXT("After the binding no claim stands by the bound one"), CosmicHazards::ClaimantOf(*Board, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 10),
		static_cast<int32>(INDEX_NONE));
	{
		// The same tick with an Upper completing in the slot instead: no seal, the same draws.
		FGalaxyState Other = Twin;
		Other.Systems[3].ActiveMegaproject = Upper;
		Sim->LoadState(Other);
		Sim->StepOnce();
		TestFalse(TEXT("(another construction seals nothing)"), Sim->GetState().Transcendents[1].bSealed);
		TestEqual(TEXT("The binding draws nothing of the sim stream"), Sim->GetRng().GetCurrentSeed(), BoundSeed);
		TestEqual(TEXT("The binding's line draws one number of the tell stream more than the twin's completion"),
			DrawsFrom(Sim->GetState().TellRngState, BoundTell), 1);
	}
	{
		// The ruled people's binding (notes 5, block 2): the binding's line takes the completion line's Severity, 1
		// (mutation: told with 0).
		FGalaxyState Ruled = Twin;
		Ruled.Systems[3].ActiveMegaproject = NAME_None;
		Ruled.Systems[3].MegaprojectProgress = 0.f;
		Ruled.Civs[0].Tier = EAscensionTier::Tier4_Transcendent;
		Ruled.Systems[0].ActiveMegaproject = CosmicRows::ProjectBinding;
		Ruled.Systems[0].MegaprojectProgress = 3.f;
		Sim->LoadState(Ruled);
		Sim->StepOnce();
		const FGalaxyState& Live = Sim->GetState();
		const int32 Done = LineOf(Live, CosmicRows::ProjectBinding, Live.Tick);
		const int32 RuledLine = LineOf(Live, CosmicRows::LogFallenBound, Live.Tick);
		TestTrue(TEXT("(the ruled people binds, its line right after the completion line)"), Live.Civs[0].bIsPlayerRuled && Live.Transcendents[1].bSealed
			&& Done != INDEX_NONE && RuledLine == Done + 1);
		TestTrue(TEXT("The ruled people's binding line has its completion line's Severity, 1"), Live.Log.IsValidIndex(RuledLine)
			&& Live.Log[Done].Severity == 1 && Live.Log[RuledLine].Severity == 1);
	}

	// (3) The target is bound: people 2 completes its own binding on 6 at the next tick - it ends without result.
	{
		Sim->LoadState(Twin);
		Sim->StepOnce();
		FGalaxyState& Live = Sim->GetMutableState();
		const int64 FirstBound = Live.Tick;
		Live.Civs[2].Tier = EAscensionTier::Tier4_Transcendent;
		// The same tick with no binding on 6, for the tell stream below.
		const FGalaxyState Plain = Live;
		Live.Systems[6].ActiveMegaproject = CosmicRows::ProjectBinding;
		Live.Systems[6].MegaprojectProgress = 3.f;
		const float Before = Live.Civs[2].Stockpile.Energy;
		Sim->StepOnce();
		TestEqual(TEXT("A seal is never moved: SealedTick stays the first binding's"), Live.Transcendents[1].SealedTick, FirstBound);
		TestTrue(TEXT("A binding with no target left ends: its slot empty, its progress 0"), Live.Systems[6].ActiveMegaproject.IsNone() && Live.Systems[6].MegaprojectProgress == 0.f);
		TestFalse(TEXT("nothing recorded"), HoldsBinding(Live.Systems[6]));
		TestEqual(TEXT("nothing paid"), Live.Civs[2].Stockpile.Energy, Before + Live.Civs[2].IncomePerTick.Energy, 0.01f);
		TestEqual(TEXT("no completion line"), LineOf(Live, CosmicRows::ProjectBinding, Live.Tick), static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("no binding's line"), LineOf(Live, CosmicRows::LogFallenBound, Live.Tick), static_cast<int32>(INDEX_NONE));
		const int32 EndedTell = Live.TellRngState;
		Sim->LoadState(Plain);
		Sim->StepOnce();
		TestEqual(TEXT("and no number of the tell stream: it draws as the same tick without the binding"), DrawsFrom(Sim->GetState().TellRngState, EndedTell), 0);
	}

	// (4) Two bindings completing on one tick, one fallen one: people 1 on 3 and people 2 on 6 (3 comes first in Systems order).
	{
		FGalaxyState Both = Twin;
		Both.Civs[2].Tier = EAscensionTier::Tier4_Transcendent;
		Both.Systems[6].ActiveMegaproject = CosmicRows::ProjectBinding;
		Both.Systems[6].MegaprojectProgress = 3.f;
		Sim->LoadState(Both);
		FGalaxyState& Live = Sim->GetMutableState();
		const float Before = Live.Civs[2].Stockpile.Energy;
		Sim->StepOnce();
		const int64 Tick = Live.Tick;
		TestTrue(TEXT("The first in Systems order binds"), Live.Transcendents[1].bSealed && Live.Transcendents[1].SealedTick == Tick && HoldsBinding(Live.Systems[3]));
		TestTrue(TEXT("The second ends without result in the same pass"), Live.Systems[6].ActiveMegaproject.IsNone() && Live.Systems[6].MegaprojectProgress == 0.f
			&& !HoldsBinding(Live.Systems[6]));
		TestEqual(TEXT("and pays nothing"), Live.Civs[2].Stockpile.Energy, Before + Live.Civs[2].IncomePerTick.Energy, 0.01f);
		FCosmicEvalView View(Live, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules());
		View.CivIndex = 2;
		View.SystemIndex = 6;
		TestFalse(TEXT("Its people holds no completion of the binding: MegaprojectComplete project_binding is false"),
			UCosmicEventSubsystem::Check(Cond(EConditionType::MegaprojectComplete, TEXT("project_binding")), View));
		View.CivIndex = 1;
		View.SystemIndex = 3;
		TestTrue(TEXT("while the maker's holds"), UCosmicEventSubsystem::Check(Cond(EConditionType::MegaprojectComplete, TEXT("project_binding")), View));
		int32 BoundLines = 0;
		for (const FCosmicLogEntry& Entry : Live.Log)
		{
			BoundLines += Entry.SourceRow == CosmicRows::LogFallenBound && Entry.Tick == Tick ? 1 : 0;
		}
		TestEqual(TEXT("One binding's line"), BoundLines, 1);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

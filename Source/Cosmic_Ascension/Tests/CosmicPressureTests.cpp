// Cosmic Ascension - automation tests of spiritual pressure and the fallen roster. Design: guideline.md section 3.2,
// 4.2, 7.4, 9.3.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#include <limits>

namespace CosmicPressureTests
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

	// A fallen transcendent row of a tier and a depth.
	FTranscendentDef Fallen(EAscensionTier Tier, float Depth, const TCHAR* Doctrine = TEXT("Doctrine.Silence"))
	{
		FTranscendentDef Def;
		Def.Doctrine = Tag(Doctrine);
		Def.Allegiance = Tag(TEXT("Realm.Abyssal"));
		Def.Tier = Tier;
		Def.Depth = Depth;
		return Def;
	}

	// An effect of a row: Key and Tag may be left out.
	FCosmicEffect Effect(EEffectType Type, const TCHAR* Key, float Value, const TCHAR* TagName = nullptr)
	{
		FCosmicEffect Out;
		Out.Type = Type;
		Out.Key = Key ? FName(Key) : FName();
		Out.Value = Value;
		Out.Tag = TagName ? Tag(TagName) : FGameplayTag();
		return Out;
	}

	FCosmicDataTables WithTranscendents(const FCosmicDataTables& Base, UDataTable* Transcendents)
	{
		FCosmicDataTables Tables = Base;
		Tables.Transcendents = Transcendents;
		return Tables;
	}

	FCosmicDataTables WithRules(const FCosmicDataTables& Base, const FCosmicRulesDef& RulesRow)
	{
		UDataTable* RulesTable = NewTable(FCosmicRulesDef::StaticStruct());
		RulesTable->AddRow(CosmicRows::Rules, RulesRow);
		FCosmicDataTables Tables = Base;
		Tables.Rules = RulesTable;
		return Tables;
	}

	FTranscendentState& AddInstance(FGalaxyState& State, FName TranscendentID, const TArray<FName>& Sphere)
	{
		FTranscendentState& Transcendent = State.Transcendents.AddDefaulted_GetRef();
		Transcendent.TranscendentID = TranscendentID;
		Transcendent.InfluencedSystems = Sphere;
		return Transcendent;
	}

	// The three systems of the reach cases on the chain a test wires (CosmicTests::WireChain: 0-1-...-9, 4-10-11): X is
	// a free system, Near is one lane from it, Far is out of its reach.
	struct FReachTriple
	{
		int32 X = 6;
		int32 Near = 7;
		int32 Far = 9;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicRealmPressureMonotonicTest, "Cosmic_Ascension.Sim.RealmPressureMonotonic", CosmicTests::Flags)
bool FCosmicRealmPressureMonotonicTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPressureTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	Sim->NewGalaxy(42, 12, 4);
	// The cases place their own transcendents on a topology the test wires itself (CosmicTests::WireChain), so that no
	// case leans on what the generator made. Nobody owns a system but the player's people, which keeps system 0.
	FGalaxyState Generated = Sim->GetState();
	Generated.Transcendents.Reset();
	const bool bWired = Generated.Civs.Num() == 4 && CosmicTests::WireChain(Generated);
	if (bWired)
	{
		CosmicTests::ClearOwners(Generated);
		CosmicTests::Give(Generated, 0, { 0 });
	}
	const FGalaxyState Fresh = Generated;
	// The knobs of the test, not of the game: the values below are compared exactly.
	FCosmicRulesDef Knobs = Sim->GetRules();
	Knobs.PressureBySourceTier = { 0.f, 0.f, 0.25f, 1.f, 2.f };
	Knobs.PressureAdjacencyFalloff = 0.5f;
	const FCosmicRulesDef Rules = Knobs;
	FCosmicStateIndex Index;
	Index.Rebuild(Fresh);
	const FReachTriple Where = FReachTriple();
	if (!TestTrue(TEXT("The galaxy has twelve systems and four peoples to wire"), bWired))
	{
		return false;
	}
	const TArray<float>& Weight = Rules.PressureBySourceTier;
	const FName XID = Fresh.Systems[Where.X].SystemID;
	const FName NearID = Fresh.Systems[Where.Near].SystemID;

	// One row per tier, all of depth 0, and a second tier-4 row for the tie.
	const FName RowOfTier[] = { FName(TEXT("tr_one")), FName(TEXT("tr_two")), FName(TEXT("tr_three")), FName(TEXT("tr_four")), FName(TEXT("tr_five")) };
	const FName FourTwin(TEXT("tr_four_twin"));
	UDataTable* Table = NewTable(FTranscendentDef::StaticStruct());
	for (int32 TierIndex = 0; TierIndex < 5; ++TierIndex)
	{
		Table->AddRow(RowOfTier[TierIndex], Fallen(static_cast<EAscensionTier>(TierIndex), 0.f));
	}
	Table->AddRow(FourTwin, Fallen(EAscensionTier::Tier4_Transcendent, 0.f));
	const FCosmicDataTables Tables = WithRules(WithTranscendents(Original, Table), Rules);

	// The index was built for Fresh; every state below is a copy with the same systems and civilizations.
	auto Read = [&](const FGalaxyState& State, int32 SystemIndex)
	{
		return UGalaxySimulationSubsystem::ComputePressureIn(State, Index, Tables, Rules, SystemIndex);
	};

	// Nothing presses on an unowned system of a fresh galaxy.
	{
		const FCosmicPressure Nothing = Read(Fresh, Where.X);
		TestTrue(TEXT("No source: no pressure and no dominant source"),
			Nothing.P == 0.f && Nothing.DominantSource == ECosmicPressureSource::None && Nothing.DominantSourceID.IsNone());
		TestTrue(TEXT("A system index out of range reads as no pressure"), Read(Fresh, INDEX_NONE).P == 0.f && Read(Fresh, Fresh.Systems.Num()).P == 0.f);
	}

	// A transcendent whose sphere holds the system: the weight of its tier, nothing below tier 3, rising with the tier.
	{
		float Previous = 0.f;
		bool bByTier = true;
		bool bRising = true;
		for (int32 TierIndex = 0; TierIndex < 5; ++TierIndex)
		{
			FGalaxyState State = Fresh;
			AddInstance(State, RowOfTier[TierIndex], { XID });
			const FCosmicPressure Reading = Read(State, Where.X);
			bByTier &= Reading.P == Weight[TierIndex];
			if (TierIndex >= 2)
			{
				bRising &= Reading.P > Previous && Reading.DominantSource == ECosmicPressureSource::Transcendent
					&& Reading.DominantSourceID == RowOfTier[TierIndex] && Reading.DominantTier == static_cast<EAscensionTier>(TierIndex);
			}
			else
			{
				bRising &= Reading.P == 0.f && Reading.DominantSource == ECosmicPressureSource::None;
			}
			Previous = Reading.P;
		}
		TestTrue(TEXT("Inside the sphere the pressure is the weight of the source's tier"), bByTier);
		TestTrue(TEXT("Tiers 1 and 2 do not press; from tier 3 on the pressure rises with the tier and names its source"), bRising);
	}

	// Reach: 1 inside the sphere, the falloff one lane from it, nothing farther away.
	{
		FGalaxyState State = Fresh;
		const FTranscendentState& Four = AddInstance(State, RowOfTier[3], { XID });
		TestTrue(TEXT("SphereReachIn tells inside, one lane away and out of reach apart"),
			UGalaxySimulationSubsystem::SphereReachIn(State, Four, Where.X) == ECosmicSphereReach::Inside
			&& UGalaxySimulationSubsystem::SphereReachIn(State, Four, Where.Near) == ECosmicSphereReach::Adjacent
			&& UGalaxySimulationSubsystem::SphereReachIn(State, Four, Where.Far) == ECosmicSphereReach::None
			&& UGalaxySimulationSubsystem::SphereReachIn(State, Four, INDEX_NONE) == ECosmicSphereReach::None);
		TestEqual(TEXT("One lane from the sphere the pressure is the weight times the falloff"), Read(State, Where.Near).P, Weight[3] * Rules.PressureAdjacencyFalloff);
		TestEqual(TEXT("Farther away there is no pressure"), Read(State, Where.Far).P, 0.f);
	}

	// A sealed transcendent and an instance without a row do not press.
	{
		FGalaxyState Sealed = Fresh;
		AddInstance(Sealed, RowOfTier[3], { XID }).bSealed = true;
		FGalaxyState Rowless = Fresh;
		AddInstance(Rowless, FName(TEXT("tr_no_such_row")), { XID });
		TestTrue(TEXT("A sealed transcendent and an instance without a row add nothing"), Read(Sealed, Where.X).P == 0.f && Read(Rowless, Where.X).P == 0.f);
	}

	// The civilization that owns the system presses with its tier, whoever rules it.
	{
		FGalaxyState State = Fresh;
		State.Systems[Where.X].OwnerCivID = State.Civs[1].CivID;
		TestEqual(TEXT("A tier-1 owner adds nothing"), Read(State, Where.X).P, 0.f);
		State.Civs[1].Tier = EAscensionTier::Tier4_Transcendent;
		const FCosmicPressure Owned = Read(State, Where.X);
		TestTrue(TEXT("A tier-4 owner presses on its own system and is the dominant source"),
			Owned.P == Weight[3] && Owned.DominantSource == ECosmicPressureSource::Civilization && Owned.DominantSourceID == State.Civs[1].CivID
			&& Owned.DominantTier == EAscensionTier::Tier4_Transcendent);

		// The civilization the vessel rules: the same term with a living ruler, with a mighty ruler and with none.
		const int32 Home = Index.FindSystem(Fresh, Fresh.Civs[0].HomeSystemID);
		FGalaxyState Ruled = Fresh;
		Ruled.Civs[0].Tier = EAscensionTier::Tier3_Causal;
		const float WithRuler = Read(Ruled, Home).P;
		Ruled.Calling.PersonalTier = EAscensionTier::Tier5_EternalOrder;
		const float WithMightyRuler = Read(Ruled, Home).P;
		Ruled.Calling.LifeState = ELifeState::Interregnum;
		const float WithoutRuler = Read(Ruled, Home).P;
		TestTrue(TEXT("A ruling vessel adds no term of its own: the ruled civilization presses with its tier, with or without a living ruler"),
			Fresh.Player.Role == EPlayerRole::Ruler && WithRuler == Weight[2] && WithMightyRuler == Weight[2] && WithoutRuler == Weight[2]);
	}

	// The vessel as a wanderer presses where it stands, and only while there is a vessel.
	{
		FGalaxyState State = Fresh;
		State.Player.Role = EPlayerRole::Wanderer;
		State.Player.RuledCivID = NAME_None;
		State.Player.CurrentSystemID = XID;
		State.Calling.PersonalTier = EAscensionTier::Tier4_Transcendent;
		const FCosmicPressure Standing = Read(State, Where.X);
		TestTrue(TEXT("A tier-4 wanderer presses on the system it stands in and has no ID"),
			Standing.P == Weight[3] && Standing.DominantSource == ECosmicPressureSource::Wanderer && Standing.DominantSourceID.IsNone()
			&& Standing.DominantTier == EAscensionTier::Tier4_Transcendent);
		TestEqual(TEXT("A wanderer does not press on the system next to it"), Read(State, Where.Near).P, 0.f);
		State.Calling.LifeState = ELifeState::Interregnum;
		TestEqual(TEXT("During the interregnum the dead vessel does not press"), Read(State, Where.X).P, 0.f);
	}

	// The sum, and who leads when terms are equal: transcendents in array order, then the civilization, then the wanderer.
	{
		FGalaxyState State = Fresh;
		AddInstance(State, RowOfTier[3], { XID });
		State.Systems[Where.X].OwnerCivID = State.Civs[1].CivID;
		State.Civs[1].Tier = EAscensionTier::Tier3_Causal;
		const FCosmicPressure Sum = Read(State, Where.X);
		TestTrue(TEXT("The terms add up and the largest names the dominant source"),
			Sum.P == Weight[3] + Weight[2] && Sum.DominantSource == ECosmicPressureSource::Transcendent && Sum.DominantSourceID == RowOfTier[3]);

		FGalaxyState Twins = Fresh;
		AddInstance(Twins, FourTwin, { XID });
		AddInstance(Twins, RowOfTier[3], { XID });
		FGalaxyState TwinsSwapped = Fresh;
		AddInstance(TwinsSwapped, RowOfTier[3], { XID });
		AddInstance(TwinsSwapped, FourTwin, { XID });
		TestTrue(TEXT("Of two equal transcendents the earlier in the array is the dominant source"),
			Read(Twins, Where.X).DominantSourceID == FourTwin && Read(TwinsSwapped, Where.X).DominantSourceID == RowOfTier[3] && Read(Twins, Where.X).P == Weight[3] + Weight[3]);

		State.Civs[1].Tier = EAscensionTier::Tier4_Transcendent;
		TestTrue(TEXT("A transcendent leads a civilization of the same weight"), Read(State, Where.X).DominantSource == ECosmicPressureSource::Transcendent);

		FGalaxyState Met = Fresh;
		Met.Systems[Where.X].OwnerCivID = Met.Civs[1].CivID;
		Met.Civs[1].Tier = EAscensionTier::Tier4_Transcendent;
		Met.Player.Role = EPlayerRole::Wanderer;
		Met.Player.RuledCivID = NAME_None;
		Met.Player.CurrentSystemID = XID;
		Met.Calling.PersonalTier = EAscensionTier::Tier4_Transcendent;
		const FCosmicPressure Both = Read(Met, Where.X);
		TestTrue(TEXT("A civilization leads a wanderer of the same weight, and both are counted"),
			Both.DominantSource == ECosmicPressureSource::Civilization && Both.P == Weight[3] + Weight[3]);
	}

	// Knobs out of range are read at their bounds: the ranges are a lint, the guards are in the function.
	{
		FGalaxyState FourInside = Fresh;
		AddInstance(FourInside, RowOfTier[3], { XID });
		FGalaxyState FiveInside = Fresh;
		AddInstance(FiveInside, RowOfTier[4], { XID });
		auto ReadWith = [&](const FCosmicRulesDef& RulesOfCase, const FGalaxyState& State, int32 SystemIndex)
		{
			return UGalaxySimulationSubsystem::ComputePressureIn(State, Index, Tables, RulesOfCase, SystemIndex).P;
		};

		FCosmicRulesDef Short = Rules;
		Short.PressureBySourceTier.SetNum(4);
		FCosmicRulesDef Negative = Rules;
		Negative.PressureBySourceTier[3] = -1.f;
		TestTrue(TEXT("A weight table too short for the tier and a weight below zero read as 0"),
			ReadWith(Rules, FiveInside, Where.X) == Weight[4] && Weight[4] > 0.f && ReadWith(Short, FiveInside, Where.X) == 0.f
			&& ReadWith(Negative, FourInside, Where.X) == 0.f);

		FCosmicRulesDef Wide = Rules;
		Wide.PressureAdjacencyFalloff = 1.5f;
		FCosmicRulesDef Below = Rules;
		Below.PressureAdjacencyFalloff = -1.f;
		FCosmicRulesDef NotANumber = Rules;
		NotANumber.PressureAdjacencyFalloff = std::numeric_limits<float>::quiet_NaN();
		TestTrue(TEXT("A falloff above 1 reads as 1: a neighbour never presses harder than the sphere itself"), ReadWith(Wide, FourInside, Where.Near) == Weight[3]);
		TestTrue(TEXT("A falloff below zero or not a number reads as 0"),
			ReadWith(Below, FourInside, Where.Near) == 0.f && ReadWith(NotANumber, FourInside, Where.Near) == 0.f);
	}

	// The member reads the live state and gives what the static gives.
	{
		FGalaxyState Live = Fresh;
		AddInstance(Live, RowOfTier[3], { NearID });
		Sim->SetTables(Tables);
		Sim->LoadState(Live);
		FName DominantID;
		const float P = Sim->ComputeSpiritualPressure(XID, DominantID);
		const FCosmicPressure Static = UGalaxySimulationSubsystem::ComputePressureIn(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), Where.X);
		TestTrue(TEXT("The member gives the pressure and the dominant source of the static"),
			P == Static.P && P == Weight[3] * Rules.PressureAdjacencyFalloff && DominantID == Static.DominantSourceID && DominantID == RowOfTier[3]);
		FName NoID(TEXT("stale"));
		TestTrue(TEXT("An unknown SystemID gives no pressure and no source"), Sim->ComputeSpiritualPressure(FName(TEXT("sys_no_such")), NoID) == 0.f && NoID.IsNone());
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicPressureDepthMonotonicTest, "Cosmic_Ascension.Sim.PressureDepthMonotonic", CosmicTests::Flags)
bool FCosmicPressureDepthMonotonicTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPressureTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();

	Sim->NewGalaxy(42, 12, 4);
	// The cases place their own transcendents on a topology the test wires itself (CosmicTests::WireChain), so that no
	// case leans on what the generator made. Nobody owns a system but the player's people, which keeps system 0.
	FGalaxyState Generated = Sim->GetState();
	Generated.Transcendents.Reset();
	const bool bWired = Generated.Civs.Num() == 4 && CosmicTests::WireChain(Generated);
	if (bWired)
	{
		CosmicTests::ClearOwners(Generated);
		CosmicTests::Give(Generated, 0, { 0 });
	}
	const FGalaxyState Fresh = Generated;
	// The knobs of the test, not of the game: the values below are compared exactly.
	FCosmicRulesDef Knobs = Sim->GetRules();
	Knobs.PressureBySourceTier = { 0.f, 0.f, 0.25f, 1.f, 2.f };
	Knobs.PressureAdjacencyFalloff = 0.5f;
	const FCosmicRulesDef Rules = Knobs;
	FCosmicStateIndex Index;
	Index.Rebuild(Fresh);
	const FReachTriple Where = FReachTriple();
	if (!TestTrue(TEXT("The galaxy has twelve systems and four peoples to wire"), bWired))
	{
		return false;
	}
	const TArray<float>& Weight = Rules.PressureBySourceTier;
	const FName XID = Fresh.Systems[Where.X].SystemID;

	const FName Shallow(TEXT("tr_shallow"));
	const FName Middle(TEXT("tr_middle"));
	const FName Deep(TEXT("tr_deep"));
	const FName Over(TEXT("tr_over"));
	const FName Under(TEXT("tr_under"));
	const FName DeepThree(TEXT("tr_deep_three"));
	const FName ShallowFive(TEXT("tr_shallow_five"));
	const FName NoNumber(TEXT("tr_no_number"));
	UDataTable* Table = NewTable(FTranscendentDef::StaticStruct());
	Table->AddRow(Shallow, Fallen(EAscensionTier::Tier4_Transcendent, 0.f));
	Table->AddRow(Middle, Fallen(EAscensionTier::Tier4_Transcendent, 0.5f));
	Table->AddRow(Deep, Fallen(EAscensionTier::Tier4_Transcendent, 1.f));
	Table->AddRow(Over, Fallen(EAscensionTier::Tier4_Transcendent, 3.f));
	Table->AddRow(Under, Fallen(EAscensionTier::Tier4_Transcendent, -1.f));
	Table->AddRow(DeepThree, Fallen(EAscensionTier::Tier3_Causal, 1.f));
	Table->AddRow(ShallowFive, Fallen(EAscensionTier::Tier5_EternalOrder, 0.f));
	Table->AddRow(NoNumber, Fallen(EAscensionTier::Tier4_Transcendent, std::numeric_limits<float>::quiet_NaN()));
	const FCosmicDataTables Tables = WithRules(WithTranscendents(Original, Table), Rules);

	// The pressure of one transcendent alone, with the given system in its sphere, read at ReadAt.
	auto Alone = [&](FName RowName, int32 SphereSystem, int32 ReadAt, const FCosmicRulesDef* RulesOfCase = nullptr)
	{
		FGalaxyState State = Fresh;
		AddInstance(State, RowName, { State.Systems[SphereSystem].SystemID });
		return UGalaxySimulationSubsystem::ComputePressureIn(State, Index, Tables, RulesOfCase ? *RulesOfCase : Rules, ReadAt).P;
	};

	const float AtShallow = Alone(Shallow, Where.X, Where.X);
	const float AtMiddle = Alone(Middle, Where.X, Where.X);
	const float AtDeep = Alone(Deep, Where.X, Where.X);
	TestTrue(TEXT("With the same tier and the same sphere a deeper transcendent presses harder"), AtShallow < AtMiddle && AtMiddle < AtDeep);
	TestTrue(TEXT("Depth 0 is the plain weight of the tier and depth 1 exactly twice that"), AtShallow == Weight[3] && AtDeep == 2.f * Weight[3] && AtMiddle == 1.5f * Weight[3]);
	TestTrue(TEXT("A depth out of range is read as 0..1"), Alone(Over, Where.X, Where.X) == AtDeep && Alone(Under, Where.X, Where.X) == AtShallow);
	TestTrue(TEXT("A depth that is not a number is read as 0"), Alone(NoNumber, Where.X, Where.X) == AtShallow);
	TestEqual(TEXT("A row that does not set its depth stands in the middle of its tier"), FTranscendentDef().Depth, 0.5f);
	// Section 3.2: depth does not cross a tier. This pair is read with the weights of the game, which it is meant to
	// hold: the deepest tier 3 is below the shallowest tier 4, and the deepest tier 4 is not above the shallowest tier 5.
	const FCosmicRulesDef GameRules = Sim->GetRules();
	TestTrue(TEXT("Depth does not cross a tier: with the weights of the game the deepest tier 3 stays below the shallowest tier 4"),
		Alone(DeepThree, Where.X, Where.X, &GameRules) < Alone(Shallow, Where.X, Where.X, &GameRules));
	TestTrue(TEXT("Depth does not cross a tier: with the weights of the game the deepest tier 4 is not above the shallowest tier 5"),
		Alone(Deep, Where.X, Where.X, &GameRules) <= Alone(ShallowFive, Where.X, Where.X, &GameRules));
	TestEqual(TEXT("One lane away the depth scales the term as well"), Alone(Deep, Where.X, Where.Near), 2.f * Weight[3] * Rules.PressureAdjacencyFalloff);

	// The civilization and the wanderer have no depth: a deep row elsewhere changes nothing about their terms.
	{
		FGalaxyState State = Fresh;
		AddInstance(State, Deep, { State.Systems[Where.Far].SystemID });
		State.Systems[Where.X].OwnerCivID = State.Civs[1].CivID;
		State.Civs[1].Tier = EAscensionTier::Tier4_Transcendent;
		State.Player.Role = EPlayerRole::Wanderer;
		State.Player.RuledCivID = NAME_None;
		State.Player.CurrentSystemID = XID;
		State.Calling.PersonalTier = EAscensionTier::Tier3_Causal;
		// The deep transcendent sits on Far, out of reach of X.
		TestEqual(TEXT("The civilization term and the wanderer term carry no depth"),
			UGalaxySimulationSubsystem::ComputePressureIn(State, Index, Tables, Rules, Where.X).P, Weight[3] + Weight[2]);
	}

	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicFallenRosterPickTest, "Cosmic_Ascension.Sim.FallenRosterPick", CosmicTests::Flags)
bool FCosmicFallenRosterPickTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPressureTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	// A knob outside 0..3 is read at its bound with one error line per new galaxy: three such galaxies in the last block.
	AddExpectedError(TEXT("FallenTranscendentCount -?[0-9]+ is outside 0"), EAutomationExpectedErrorFlags::Contains, 3);

	const FName Zeal(TEXT("trans_zeal"));
	const FName ZealTwin(TEXT("trans_zeal_twin"));
	const FName Assimilation(TEXT("trans_assimilation"));
	const FName Recurrence(TEXT("trans_recurrence"));
	const FName Covenant(TEXT("trans_covenant"));
	const FName Stillness(TEXT("trans_stillness"));

	using FRowList = TArray<TPair<FName, FTranscendentDef>>;
	auto Row = [](const TCHAR* Doctrine, float Weight, const TCHAR* Realm = TEXT("Realm.Abyssal"))
	{
		FTranscendentDef Def = Fallen(EAscensionTier::Tier4_Transcendent, 0.5f, Doctrine);
		Def.Allegiance = Tag(Realm);
		Def.SpawnWeight = Weight;
		return Def;
	};
	auto TableOf = [](const FRowList& Rows, bool bReverse = false)
	{
		UDataTable* Table = NewTable(FTranscendentDef::StaticStruct());
		for (int32 Step = 0; Step < Rows.Num(); ++Step)
		{
			const TPair<FName, FTranscendentDef>& Each = Rows[bReverse ? Rows.Num() - 1 - Step : Step];
			Table->AddRow(Each.Key, Each.Value);
		}
		return Table;
	};
	auto TablesFor = [&](int32 Count, UDataTable* Table)
	{
		FCosmicRulesDef RulesRow = BaseRules;
		RulesRow.FallenTranscendentCount = Count;
		return WithTranscendents(WithRules(Original, RulesRow), Table);
	};
	// The TranscendentIDs of the roster a seed gives under the tables set last.
	auto RosterOf = [Sim](int32 Seed)
	{
		Sim->NewGalaxy(Seed, 12, 4);
		TArray<FName> IDs;
		for (const FTranscendentState& Transcendent : Sim->GetState().Transcendents)
		{
			IDs.Add(Transcendent.TranscendentID);
		}
		return IDs;
	};
	// Where the sim stream stands after NewGalaxy of one seed under these tables.
	auto SeedAfter = [Sim](const FCosmicDataTables& Tables)
	{
		Sim->SetTables(Tables);
		Sim->NewGalaxy(7, 12, 4);
		return Sim->GetRng().GetCurrentSeed();
	};

	// Four fallen doctrines (one of them twice), a celestial row and an independent row. The two rows that must never
	// be drawn carry the largest weights on purpose.
	FRowList Full;
	Full.Emplace(CosmicRows::TransSilence, Row(TEXT("Doctrine.Silence"), 1.f));
	Full.Emplace(Zeal, Row(TEXT("Doctrine.Zeal"), 3.f));
	Full.Emplace(ZealTwin, Row(TEXT("Doctrine.Zeal"), 3.f));
	Full.Emplace(Assimilation, Row(TEXT("Doctrine.Assimilation"), 2.f));
	Full.Emplace(Recurrence, Row(TEXT("Doctrine.Recurrence"), 2.f));
	Full.Emplace(Covenant, Row(TEXT("Doctrine.Covenant"), 50.f, TEXT("Realm.Celestial")));
	Full.Emplace(Stillness, Row(TEXT("Doctrine.Stillness"), 50.f, TEXT("Realm.Independent")));
	FRowList WithoutSilence = Full;
	WithoutSilence.RemoveAt(0);

	// Thirty runs with three places: silence first, two more fallen rows, no doctrine twice, nobody of another realm.
	Sim->SetTables(TablesFor(3, TableOf(Full)));
	TSet<FName> Seen;
	bool bRoster = true;
	TArray<TArray<FName>> Rosters;
	for (int32 Seed = 1; Seed <= 30; ++Seed)
	{
		const TArray<FName> IDs = RosterOf(Seed);
		Rosters.Add(IDs);
		const TArray<FTranscendentState>& Active = Sim->GetState().Transcendents;
		bRoster &= Active.Num() == 3 && IDs[0] == CosmicRows::TransSilence && !IDs.Contains(Covenant) && !IDs.Contains(Stillness);
		TSet<FGameplayTag> Doctrines;
		for (const FTranscendentState& Transcendent : Active)
		{
			Doctrines.Add(Transcendent.Doctrine);
			bRoster &= Transcendent.InfluencedSystems.IsEmpty() && !Transcendent.bSealed && Transcendent.Doctrine.IsValid();
			Seen.Add(Transcendent.TranscendentID);
		}
		bRoster &= Doctrines.Num() == Active.Num();
	}
	TestTrue(TEXT("Every roster is silence plus two more fallen rows, each doctrine once, with empty spheres and nobody of another realm"), bRoster);
	TestTrue(TEXT("Over thirty seeds every fallen row with a weight is drawn, both rows of the doctrine that has two"),
		Seen.Contains(Zeal) && Seen.Contains(ZealTwin) && Seen.Contains(Assimilation) && Seen.Contains(Recurrence));

	// The order of the rows in the table is not the order of the draw.
	{
		Sim->SetTables(TablesFor(3, TableOf(Full, true)));
		bool bSame = true;
		for (int32 Seed = 1; Seed <= 30; ++Seed)
		{
			bSame &= RosterOf(Seed) == Rosters[Seed - 1];
		}
		TestTrue(TEXT("Reversing the rows of DT_Transcendents changes no roster"), bSame);
	}

	// The number of draws follows the knob alone: the same stream position with a full table, an empty one and none,
	// and with rows that have a starting core or not. Each place beyond the first is one roster draw and each place is
	// one core draw (section 7.4): three places draw four times more than one place, one place once more than none.
	{
		const int32 Three = SeedAfter(TablesFor(3, TableOf(Full)));
		TestEqual(TEXT("An empty DT_Transcendents draws like a full one"), SeedAfter(TablesFor(3, NewTable(FTranscendentDef::StaticStruct()))), Three);
		TestEqual(TEXT("No DT_Transcendents draws like a full one"), SeedAfter(TablesFor(3, nullptr)), Three);
		TestEqual(TEXT("A table without the silence row draws like a full one"), SeedAfter(TablesFor(3, TableOf(WithoutSilence))), Three);
		FRowList WithCores = Full;
		for (TPair<FName, FTranscendentDef>& Each : WithCores)
		{
			Each.Value.InitialInfluencedSystems = 1;
		}
		TestEqual(TEXT("Rows with a starting core draw like rows without"), SeedAfter(TablesFor(3, TableOf(WithCores))), Three);
		const int32 One = SeedAfter(TablesFor(1, TableOf(Full)));
		FRandomStream Probe(One);
		for (int32 Draw = 0; Draw < 4; ++Draw)
		{
			Probe.GetFraction();
		}
		TestEqual(TEXT("Three places draw exactly four times more than one place (two roster draws, two core draws)"), Three, Probe.GetCurrentSeed());
		FRandomStream FromNone(SeedAfter(TablesFor(0, TableOf(Full))));
		FromNone.GetFraction();
		TestEqual(TEXT("One place draws exactly once more than no place: its core draw"), One, FromNone.GetCurrentSeed());
	}

	// One place is silence alone, no place is nobody, and without the silence row nobody takes its place.
	{
		Sim->SetTables(TablesFor(1, TableOf(Full)));
		const TArray<FName> One = RosterOf(3);
		Sim->SetTables(TablesFor(0, TableOf(Full)));
		const TArray<FName> None = RosterOf(3);
		Sim->SetTables(TablesFor(3, TableOf(WithoutSilence)));
		const TArray<FName> NoSilence = RosterOf(3);
		TestTrue(TEXT("One place is silence alone and no place is nobody"), One.Num() == 1 && One[0] == CosmicRows::TransSilence && None.IsEmpty());
		TestTrue(TEXT("Without the silence row the two drawn places are all there is"), NoSilence.Num() == 2 && !NoSilence.Contains(CosmicRows::TransSilence));
	}

	// The fixed place belongs to a fallen silence: a trans_silence row of another realm gets no instance.
	{
		FRowList Rows;
		Rows.Emplace(CosmicRows::TransSilence, Row(TEXT("Doctrine.Silence"), 1.f, TEXT("Realm.Celestial")));
		Rows.Emplace(Assimilation, Row(TEXT("Doctrine.Assimilation"), 1.f));
		Sim->SetTables(TablesFor(1, TableOf(Rows)));
		const TArray<FName> One = RosterOf(3);
		Sim->SetTables(TablesFor(2, TableOf(Rows)));
		const TArray<FName> Two = RosterOf(3);
		TestTrue(TEXT("A trans_silence row that is not Realm.Abyssal gets no instance, and nobody takes the fixed place"),
			One.IsEmpty() && Two.Num() == 1 && Two[0] == Assimilation);
	}

	// The pick follows SpawnWeight. One drawn place and two candidates: the one that outweighs the other a million times
	// is drawn in every one of thirty seeds, whichever of the two it is. A pick that ignored the weights would give the
	// same rosters for both tables.
	{
		auto DrawnOverThirty = [&](float WeightOfAssimilation, float WeightOfRecurrence)
		{
			FRowList Rows;
			Rows.Emplace(CosmicRows::TransSilence, Row(TEXT("Doctrine.Silence"), 1.f));
			Rows.Emplace(Assimilation, Row(TEXT("Doctrine.Assimilation"), WeightOfAssimilation));
			Rows.Emplace(Recurrence, Row(TEXT("Doctrine.Recurrence"), WeightOfRecurrence));
			Sim->SetTables(TablesFor(2, TableOf(Rows)));
			TSet<FName> Drawn;
			bool bShape = true;
			for (int32 Seed = 1; Seed <= 30; ++Seed)
			{
				const TArray<FName> IDs = RosterOf(Seed);
				bShape &= IDs.Num() == 2 && IDs[0] == CosmicRows::TransSilence;
				if (IDs.Num() == 2)
				{
					Drawn.Add(IDs[1]);
				}
			}
			return bShape ? Drawn : TSet<FName>();
		};
		const TSet<FName> HeavyAssimilation = DrawnOverThirty(1000000.f, 1.f);
		const TSet<FName> HeavyRecurrence = DrawnOverThirty(1.f, 1000000.f);
		TestTrue(TEXT("The draw follows SpawnWeight: the row that outweighs the other is the one drawn, whichever of the two it is"),
			HeavyAssimilation.Num() == 1 && HeavyAssimilation.Contains(Assimilation) && HeavyRecurrence.Num() == 1 && HeavyRecurrence.Contains(Recurrence));
	}

	// A weight that is zero, below zero, not a number or not finite makes no candidate.
	{
		FRowList Weightless;
		Weightless.Emplace(CosmicRows::TransSilence, Row(TEXT("Doctrine.Silence"), 1.f));
		Weightless.Emplace(Assimilation, Row(TEXT("Doctrine.Assimilation"), 0.f));
		Weightless.Emplace(Recurrence, Row(TEXT("Doctrine.Recurrence"), -1.f));
		Weightless.Emplace(Zeal, Row(TEXT("Doctrine.Zeal"), std::numeric_limits<float>::quiet_NaN()));
		Sim->SetTables(TablesFor(2, TableOf(Weightless)));
		bool bAlone = true;
		for (int32 Seed = 1; Seed <= 10; ++Seed)
		{
			const TArray<FName> IDs = RosterOf(Seed);
			bAlone &= IDs.Num() == 1 && IDs[0] == CosmicRows::TransSilence;
		}
		TestTrue(TEXT("A row whose weight is zero, below zero or not a number is never drawn"), bAlone);

		// The row with the infinite weight sorts last on purpose: a sum that is not finite would fall back to the last
		// candidate.
		FRowList WithInfinite;
		WithInfinite.Emplace(CosmicRows::TransSilence, Row(TEXT("Doctrine.Silence"), 1.f));
		WithInfinite.Emplace(Assimilation, Row(TEXT("Doctrine.Assimilation"), 1.f));
		WithInfinite.Emplace(Zeal, Row(TEXT("Doctrine.Zeal"), std::numeric_limits<float>::infinity()));
		Sim->SetTables(TablesFor(2, TableOf(WithInfinite)));
		bool bFiniteDrawn = true;
		for (int32 Seed = 1; Seed <= 30; ++Seed)
		{
			const TArray<FName> IDs = RosterOf(Seed);
			bFiniteDrawn &= IDs.Num() == 2 && IDs[1] == Assimilation;
		}
		TestTrue(TEXT("A row whose weight is not finite is no candidate: the row with a weight is drawn"), bFiniteDrawn);
	}

	// The knob is read as 0..3 (at most three active fallen per run): 4 gives the roster and the draws of 3.
	{
		Sim->SetTables(TablesFor(3, TableOf(Full)));
		const TArray<FName> Three = RosterOf(3);
		Sim->SetTables(TablesFor(4, TableOf(Full)));
		const TArray<FName> Four = RosterOf(3);
		const int32 SeedOfThree = SeedAfter(TablesFor(3, TableOf(Full)));
		TestTrue(TEXT("A knob of 4 is read as 3: the same roster of three and the same number of draws"),
			Four.Num() == 3 && Four == Three && SeedAfter(TablesFor(4, TableOf(Full))) == SeedOfThree);
		Sim->SetTables(TablesFor(-2, TableOf(Full)));
		TestTrue(TEXT("A knob below zero is read as 0: nobody"), RosterOf(3).IsEmpty());
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicPressurePerCivilizationTest, "Cosmic_Ascension.Sim.PressurePerCivilization", CosmicTests::Flags)
bool FCosmicPressurePerCivilizationTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPressureTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	// The knobs of this test, not of the game: the numbers below are compared exactly and the game's are tuned.
	FCosmicRulesDef Knobs = Sim->GetRules();
	Knobs.PressureBySourceTier = { 0.f, 0.f, 0.25f, 1.f, 2.f };
	Knobs.PressureAdjacencyFalloff = 0.5f;
	Knobs.SpiritualPressureStabilityDrain = 0.002f;
	const float Start = 0.8f;
	const float Drain = 0.002f;
	const float Inside = 1.f;
	const float Beside = 0.5f;

	const FName RowA(TEXT("tr_a"));
	const FName RowB(TEXT("tr_b"));
	const FCosmicEffect OneCausality = Effect(EEffectType::AddResource, TEXT("Causality"), 1.f);

	// The tables of a case: its rules, its tier-4 rows of depth 0 with their per-tick effects, and no event rows (the
	// slots of the tick still draw their numbers).
	struct FRowOfCase
	{
		FName Name;
		TArray<FCosmicEffect> Effects;
	};
	using FRowsOfCase = TArray<FRowOfCase>;
	auto TablesOf = [&](const FCosmicRulesDef& RulesRow, const FRowsOfCase& Rows)
	{
		UDataTable* Table = NewTable(FTranscendentDef::StaticStruct());
		for (const FRowOfCase& Row : Rows)
		{
			FTranscendentDef Def = Fallen(EAscensionTier::Tier4_Transcendent, 0.f);
			Def.PressureEffectsPerTick = Row.Effects;
			Table->AddRow(Row.Name, Def);
		}
		FCosmicDataTables Tables = WithTranscendents(WithRules(Original, RulesRow), Table);
		Tables.Events = nullptr;
		return Tables;
	};

	// A galaxy the test wires itself (CosmicTests::WireChain: a chain 0..9 with a branch 4-10-11), so that no case
	// leans on what the generator made. The player's people holds system 0, the wide people systems 3, 4, 5 and 6 in
	// that order, the border people system 10 and the single people system 8. A sphere is a row and system indices.
	struct FSphereOfCase
	{
		FName Row;
		TArray<int32> Systems;
	};
	using FSpheres = TArray<FSphereOfCase>;
	bool bWired = true;
	auto Arrange = [&](const FSpheres& Spheres)
	{
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		if (State.Civs.Num() != 4 || !CosmicTests::WireChain(State))
		{
			bWired = false;
			return;
		}
		// The case places its own transcendents: whatever roster the tables gave is taken out.
		State.Transcendents.Reset();
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { 0 });
		CosmicTests::Give(State, 1, { 3, 4, 5, 6 });
		CosmicTests::Give(State, 2, { 10 });
		CosmicTests::Give(State, 3, { 8 });
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.Stability = Start;
		}
		for (const FSphereOfCase& Sphere : Spheres)
		{
			TArray<FName> IDs;
			for (const int32 SystemIndex : Sphere.Systems)
			{
				IDs.Add(State.Systems[SystemIndex].SystemID);
			}
			AddInstance(State, Sphere.Row, IDs);
		}
	};
	const int32 PlayerCiv = 0;
	const int32 WideCiv = 1;
	const int32 BorderCiv = 2;
	const int32 SingleCiv = 3;
	const FGalaxyState& State = Sim->GetState();

	Sim->SetTables(TablesOf(Knobs, { { RowA, { OneCausality } } }));
	Arrange({ { RowA, { 4, 5 } } });
	if (!TestTrue(TEXT("The galaxy has twelve systems and four peoples to wire"), bWired))
	{
		return false;
	}

	// One sphere over systems 4 and 5. The wide people has two systems in it and two beside it, the border people one
	// beside it, the other two are out of reach.
	{
		auto PressureAt = [Sim](int32 SystemIndex)
		{
			return UGalaxySimulationSubsystem::ComputePressureIn(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), SystemIndex).P;
		};
		TestTrue(TEXT("The public function reads the wired sphere: 1 inside, the falloff one lane away, 0 farther off"),
			PressureAt(3) == Beside && PressureAt(4) == Inside && PressureAt(5) == Inside && PressureAt(6) == Beside
			&& PressureAt(10) == Beside && PressureAt(8) == 0.f && PressureAt(0) == 0.f);

		Sim->RunHeadless(1);
		TestTrue(TEXT("A people with systems in the sphere and beside it is drained once, by the largest of their pressures"),
			State.Civs[WideCiv].Stability == Start - Drain * Inside);
		TestTrue(TEXT("A people that only borders the sphere is drained by the falloff"), State.Civs[BorderCiv].Stability == Start - Drain * Beside);
		TestTrue(TEXT("A people out of reach keeps its Stability"), State.Civs[SingleCiv].Stability == Start && State.Civs[PlayerCiv].Stability == Start);
		TestTrue(TEXT("The per-tick effects are applied once to a people with two systems in the sphere"), State.Civs[WideCiv].Stockpile.Causality == 1.f);
		TestTrue(TEXT("A people that only borders the sphere, and a people out of reach, take no per-tick effect"),
			State.Civs[BorderCiv].Stockpile.Causality == 0.f && State.Civs[SingleCiv].Stockpile.Causality == 0.f && State.Civs[PlayerCiv].Stockpile.Causality == 0.f);
	}

	// A sphere that also holds the one system of the single people: one system in a sphere costs what two cost. A
	// system that is still being settled, or that is listed by a people that does not own it, counts for neither.
	{
		Arrange({ { RowA, { 4, 5, 8 } } });
		Sim->RunHeadless(1);
		TestTrue(TEXT("One system in the sphere costs what two cost, and takes the effects once as well"),
			State.Civs[SingleCiv].Stability == Start - Drain * Inside && State.Civs[SingleCiv].Stability == State.Civs[WideCiv].Stability
			&& State.Civs[SingleCiv].Stockpile.Causality == 1.f && State.Civs[WideCiv].Stockpile.Causality == 1.f);

		Arrange({ { RowA, { 4, 5, 8 } } });
		Sim->GetMutableState().Systems[8].ActiveMegaproject = CosmicRows::ProjectSettle;
		Sim->GetMutableState().Civs[PlayerCiv].OwnedSystems.Add(State.Systems[4].SystemID);
		Sim->RunHeadless(1);
		TestTrue(TEXT("A system that is still being settled neither drains its people nor brings it the per-tick effects"),
			State.Civs[SingleCiv].Stability == Start && State.Civs[SingleCiv].Stockpile.Causality == 0.f);
		TestTrue(TEXT("A system listed by a people that does not own it neither drains that people nor brings it the effects"),
			State.Civs[PlayerCiv].Stability == Start && State.Civs[PlayerCiv].Stockpile.Causality == 0.f);
		TestTrue(TEXT("The people that owns settled systems in the sphere is pressed as before"),
			State.Civs[WideCiv].Stability == Start - Drain * Inside && State.Civs[WideCiv].Stockpile.Causality == 1.f);
	}

	// A people on the tier of the transcendent is not drained and takes no effect; the people next to it still is.
	{
		Arrange({ { RowA, { 4, 5, 8 } } });
		Sim->GetMutableState().Civs[WideCiv].Tier = EAscensionTier::Tier4_Transcendent;
		Sim->RunHeadless(1);
		TestTrue(TEXT("A source that does not stand above the civilization neither drains it nor applies its effects"),
			State.Civs[WideCiv].Stability == Start && State.Civs[WideCiv].Stockpile.Causality == 0.f);
		TestTrue(TEXT("The tier of one people does not shield the next: a lower people on the same sphere is still pressed"),
			State.Civs[SingleCiv].Stability == Start - Drain * Inside && State.Civs[SingleCiv].Stockpile.Causality == 1.f
			&& State.Civs[BorderCiv].Stability == Start - Drain * Beside);
	}

	// A sealed transcendent presses on nobody.
	{
		Arrange({ { RowA, { 4, 5, 8 } } });
		Sim->GetMutableState().Transcendents[0].bSealed = true;
		Sim->RunHeadless(1);
		bool bUntouched = true;
		for (const FCivilizationState& Civ : State.Civs)
		{
			bUntouched &= Civ.Stability == Start && Civ.Stockpile.Causality == 0.f;
		}
		TestTrue(TEXT("A sealed transcendent neither drains nor applies its effects"), bUntouched);
	}

	// Two transcendents over the wide people: both apply, once each, in the order of State.Transcendents, each with
	// itself as the other party. The two lists do not commute (a stockpile stops at zero), so the order shows.
	{
		const FCosmicEffect Estrange = Effect(EEffectType::ChangeRelation, nullptr, -0.1f);
		const FCosmicEffect FiveLess = Effect(EEffectType::AddResource, TEXT("Causality"), -5.f);
		Sim->SetTables(TablesOf(Knobs, { { RowA, { OneCausality, Estrange } }, { RowB, { FiveLess } } }));

		Arrange({ { RowA, { 4, 5 } }, { RowB, { 5 } } });
		Sim->RunHeadless(1);
		const float CausalityAThenB = State.Civs[WideCiv].Stockpile.Causality;
		const bool bRelationToA = State.Civs[WideCiv].Relations.Num() == 1 && State.Civs[WideCiv].Relations.FindRef(RowA) == -0.1f;
		TestTrue(TEXT("Where two spheres overlap their terms add up, and the people is drained by that sum"),
			State.Civs[WideCiv].Stability == Start - Drain * (Inside + Inside));

		Arrange({ { RowB, { 5 } }, { RowA, { 4, 5 } } });
		Sim->RunHeadless(1);
		TestTrue(TEXT("Two transcendents apply their per-tick effects once each, in the order of State.Transcendents"),
			CausalityAThenB == 0.f && State.Civs[WideCiv].Stockpile.Causality == 1.f);
		TestTrue(TEXT("The other party of a per-tick effect is the transcendent whose row it is"),
			bRelationToA && State.Civs[WideCiv].Relations.Num() == 1 && State.Civs[WideCiv].Relations.FindRef(RowA) == -0.1f);
	}

	// MultiplyIncome as a per-tick source: the income of the tick is scaled before it reaches the stockpile, and a factor
	// below zero is read as 0.
	{
		Sim->SetTables(TablesOf(Knobs, { { RowA, { Effect(EEffectType::MultiplyIncome, TEXT("Energy"), 0.5f) } } }));
		Arrange({});
		const float EnergyBefore = State.Civs[WideCiv].Stockpile.Energy;
		Sim->RunHeadless(1);
		const float FreeIncome = State.Civs[WideCiv].IncomePerTick.Energy;

		Arrange({ { RowA, { 4, 5 } } });
		Sim->RunHeadless(1);
		TestTrue(TEXT("A per-tick MultiplyIncome halves the income of the tick, and the halved income is what reaches the stockpile"),
			FreeIncome > 0.f && State.Civs[WideCiv].IncomePerTick.Energy == 0.5f * FreeIncome
			&& State.Civs[WideCiv].Stockpile.Energy == EnergyBefore + State.Civs[WideCiv].IncomePerTick.Energy);

		Sim->SetTables(TablesOf(Knobs, { { RowA, { Effect(EEffectType::MultiplyIncome, TEXT("Energy"), -0.2f) } } }));
		Arrange({ { RowA, { 4, 5 } } });
		Sim->RunHeadless(1);
		TestTrue(TEXT("A per-tick MultiplyIncome below zero is read as 0: no income, and the stockpile does not go down"),
			State.Civs[WideCiv].IncomePerTick.Energy == 0.f && State.Civs[WideCiv].Stockpile.Energy == EnergyBefore);
	}

	// A per-tick source changes amounts and nothing else. A row full of other effects over the capital of the people
	// the vessel rules leaves the tick exactly as a row without effects leaves it: no draw, no role effect, the vessel
	// as it was.
	{
		auto TickOver = [&](const TArray<FCosmicEffect>& Effects)
		{
			Sim->SetTables(TablesOf(Knobs, { { RowA, Effects } }));
			Arrange({ { RowA, { 0 } } });
			Sim->RunHeadless(1);
			return TPair<uint32, int32>(Sim->ComputeStateHash(), Sim->GetRng().GetCurrentSeed());
		};
		const TPair<uint32, int32> Plain = TickOver({});
		const bool bPressed = State.Civs[PlayerCiv].Stability == Start - Drain * Inside;
		const TPair<uint32, int32> Loaded = TickOver({
			Effect(EEffectType::NameNotable, nullptr, 0.f, TEXT("Notable.Rebel")),
			Effect(EEffectType::DeposeRuler, nullptr, 0.f, TEXT("Reign.Deposed")),
			Effect(EEffectType::KillPlayer, nullptr, 0.f, TEXT("Death.Hazard")),
			Effect(EEffectType::PlayerStatDelta, TEXT("Body"), 1.f),
			Effect(EEffectType::GrantPower, nullptr, 0.f, TEXT("Way.Received")),
			Effect(EEffectType::AddTrait, nullptr, 0.f, TEXT("Trait.Faith.Seeking")),
			Effect(EEffectType::SetFlag, nullptr, 0.f, TEXT("Flag.Grace.Spent")) });
		TestTrue(TEXT("The capital of the vessel's people lies in the sphere and the tick drains it"), bPressed && State.Civs[PlayerCiv].Stability == Start - Drain * Inside);
		TestTrue(TEXT("The economy stage draws nothing, whatever a per-tick row holds"), Loaded.Value == Plain.Value);
		TestTrue(TEXT("A per-tick row records no role effect and changes nothing but amounts: the state is the state of the plain tick"),
			!State.Calling.PendingEffect.IsValid() && Loaded.Key == Plain.Key);

		// KillPlayer on its own, where it would be recorded if it got through: the vessel as a wanderer standing in the
		// first pressed system, with no birth protection. (Over a ruler the effect is turned away for other reasons, so
		// the row above cannot show it.)
		FCosmicRulesDef NoShield = Knobs;
		NoShield.BirthProtectionTicks = 0;
		auto WandererTick = [&](const TArray<FCosmicEffect>& Effects)
		{
			Sim->SetTables(TablesOf(NoShield, { { RowA, Effects } }));
			Arrange({ { RowA, { 0 } } });
			FGalaxyState& Mutable = Sim->GetMutableState();
			Mutable.Player.Role = EPlayerRole::Wanderer;
			Mutable.Player.RuledCivID = NAME_None;
			Mutable.Player.CurrentSystemID = Mutable.Systems[0].SystemID;
			Sim->RunHeadless(1);
			return TPair<uint32, int32>(Sim->ComputeStateHash(), Sim->GetRng().GetCurrentSeed());
		};
		const TPair<uint32, int32> PlainWanderer = WandererTick({});
		const TPair<uint32, int32> KilledWanderer = WandererTick({ Effect(EEffectType::KillPlayer, nullptr, 0.f, TEXT("Death.Hazard")) });
		TestTrue(TEXT("A per-tick KillPlayer over the system a wanderer stands in records no role effect"),
			!State.Calling.PendingEffect.IsValid() && KilledWanderer.Key == PlainWanderer.Key && KilledWanderer.Value == PlainWanderer.Value);
	}

	// The drain knob out of range takes nothing (below zero, not a number), and Stability stops at zero.
	{
		const FRowsOfCase NoEffects = { { RowA, {} } };
		FCosmicRulesDef Negative = Knobs;
		Negative.SpiritualPressureStabilityDrain = -1.f;
		Sim->SetTables(TablesOf(Negative, NoEffects));
		Arrange({ { RowA, { 4, 5 } } });
		Sim->RunHeadless(1);
		const bool bNegativeTakesNothing = State.Civs[WideCiv].Stability == Start;

		FCosmicRulesDef NotANumber = Knobs;
		NotANumber.SpiritualPressureStabilityDrain = std::numeric_limits<float>::quiet_NaN();
		Sim->SetTables(TablesOf(NotANumber, NoEffects));
		Arrange({ { RowA, { 4, 5 } } });
		Sim->RunHeadless(1);
		TestTrue(TEXT("A drain knob below zero or not a number takes nothing and gives nothing"), bNegativeTakesNothing && State.Civs[WideCiv].Stability == Start);

		Sim->SetTables(TablesOf(Knobs, NoEffects));
		Arrange({ { RowA, { 4, 5 } } });
		Sim->GetMutableState().Civs[WideCiv].Stability = 0.5f * Drain;
		Sim->RunHeadless(1);
		TestTrue(TEXT("Stability stops at zero under the drain"), State.Civs[WideCiv].Stability == 0.f);
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicStabilityCountBoundaryTest, "Cosmic_Ascension.Sim.StabilityCountBoundary", CosmicTests::Flags)
bool FCosmicStabilityCountBoundaryTest::RunTest(const FString& Parameters)
{
	using namespace CosmicPressureTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef BaseRules = Sim->GetRules();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	// A Stability change by a per-tick source is counted in the tick it happens: the drain of pressure takes a
	// civilization below the threshold and LowStabilityTicks is 1 at the end of that tick. The knobs are the test's own
	// and the tick has no event rows.
	{
		const FName Presser(TEXT("tr_presser"));
		UDataTable* Table = NewTable(FTranscendentDef::StaticStruct());
		Table->AddRow(Presser, Fallen(EAscensionTier::Tier4_Transcendent, 0.f));
		FCosmicRulesDef Knobs = BaseRules;
		Knobs.PressureBySourceTier = { 0.f, 0.f, 0.25f, 1.f, 2.f };
		Knobs.SpiritualPressureStabilityDrain = 0.002f;
		FCosmicDataTables Tables = WithTranscendents(WithRules(Original, Knobs), Table);
		Tables.Events = nullptr;
		Sim->SetTables(Tables);
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		State.Transcendents.Reset();
		AddInstance(State, Presser, { State.Civs[1].HomeSystemID });
		// Half a drain above the threshold: one tick under a tier-4 source of depth 0 (pressure 1) crosses it.
		const float JustAbove = 0.3f + 0.5f * 0.002f;
		State.Civs[1].Stability = JustAbove;
		const bool bStartsUncounted = State.Civs[1].LowStabilityTicks == 0 && JustAbove > 0.3f;
		Sim->RunHeadless(1);
		TestTrue(TEXT("The drain of pressure that crosses the threshold in a tick is counted in that tick"),
			bStartsUncounted && State.Civs[1].Stability < 0.3f && State.Civs[1].LowStabilityTicks == 1);
	}

	// Any other change is first counted one tick later: a slot row lowers Stability in the hazard stage of tick 1, after
	// the count of tick 1, and the count of tick 2 is the first to see it.
	{
		FCosmicEventDef Def;
		Def.Category = Tag(TEXT("Event.Civ.Society"));
		Def.LogCategory = Tag(TEXT("Log.Civ"));
		Def.Trigger = ECosmicEventTrigger::Standing;
		Def.Subject = ECosmicEventSubject::Civilization;
		Def.BaseWeight = 0.f;
		Def.CooldownTicks = 100000;
		Def.Effects.Add(Effect(EEffectType::ChangeStability, nullptr, -0.5f));
		UDataTable* Events = NewTable(FCosmicEventDef::StaticStruct());
		Events->AddRow(FName(TEXT("evt_test_unrest")), Def);

		FCosmicRulesDef RulesRow = BaseRules;
		RulesRow.EventSlotTicks = 1;
		FCosmicDataTables Tables = WithRules(Original, RulesRow);
		Tables.Events = Events;
		Sim->SetTables(Tables);
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		State.Transcendents.Reset();
		State.Civs[1].Stability = 0.7f;
		Sim->RunHeadless(1);
		const bool bLoweredNotCounted = State.Civs[1].Stability < 0.3f && State.Civs[1].LowStabilityTicks == 0;
		Sim->RunHeadless(1);
		TestTrue(TEXT("A slot row that lowers Stability in tick 1 is first counted in tick 2"), bLoweredNotCounted && State.Civs[1].LowStabilityTicks == 1);
	}

	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

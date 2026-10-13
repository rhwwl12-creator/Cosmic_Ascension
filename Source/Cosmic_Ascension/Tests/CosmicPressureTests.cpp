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

	// Three systems of a state for the reach cases: X is unowned, Near is one lane from X, Far is neither X nor one lane
	// from it. INDEX_NONE when the galaxy has no such triple.
	struct FReachTriple
	{
		int32 X = INDEX_NONE;
		int32 Near = INDEX_NONE;
		int32 Far = INDEX_NONE;
		bool IsValid() const { return X != INDEX_NONE && Near != INDEX_NONE && Far != INDEX_NONE; }
	};

	FReachTriple FindReachTriple(const FGalaxyState& State, const FCosmicStateIndex& Index)
	{
		FReachTriple Triple;
		for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num() && !Triple.IsValid(); ++SystemIndex)
		{
			const FStarSystemState& System = State.Systems[SystemIndex];
			if (!System.OwnerCivID.IsNone() || System.Lanes.IsEmpty())
			{
				continue;
			}
			Triple.X = SystemIndex;
			Triple.Near = Index.FindSystem(State, System.Lanes[0]);
			Triple.Far = INDEX_NONE;
			for (int32 Other = 0; Other < State.Systems.Num(); ++Other)
			{
				if (Other != SystemIndex && !System.Lanes.Contains(State.Systems[Other].SystemID) && !State.Systems[Other].Lanes.Contains(System.SystemID))
				{
					Triple.Far = Other;
					break;
				}
			}
		}
		return Triple;
	}
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
	// The cases place their own transcendents: whatever roster the tables of the game gave is taken out.
	FGalaxyState Generated = Sim->GetState();
	Generated.Transcendents.Reset();
	const FGalaxyState Fresh = Generated;
	const FCosmicRulesDef Rules = Sim->GetRules();
	FCosmicStateIndex Index;
	Index.Rebuild(Fresh);
	const FReachTriple Where = FindReachTriple(Fresh, Index);
	if (!TestTrue(TEXT("The rules hold five pressure weights and the galaxy holds a free system, a neighbour and a far system"),
		Rules.PressureBySourceTier.Num() == 5 && Where.IsValid()))
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
	const FCosmicDataTables Tables = WithTranscendents(Original, Table);

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
		// Near and Far may be owned: a fresh civilization is tier 1 and adds nothing.
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
	// The cases place their own transcendents: whatever roster the tables of the game gave is taken out.
	FGalaxyState Generated = Sim->GetState();
	Generated.Transcendents.Reset();
	const FGalaxyState Fresh = Generated;
	const FCosmicRulesDef Rules = Sim->GetRules();
	FCosmicStateIndex Index;
	Index.Rebuild(Fresh);
	const FReachTriple Where = FindReachTriple(Fresh, Index);
	if (!TestTrue(TEXT("The rules hold five pressure weights and the galaxy holds a free system and a neighbour"), Rules.PressureBySourceTier.Num() == 5 && Where.IsValid()))
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
	UDataTable* Table = NewTable(FTranscendentDef::StaticStruct());
	Table->AddRow(Shallow, Fallen(EAscensionTier::Tier4_Transcendent, 0.f));
	Table->AddRow(Middle, Fallen(EAscensionTier::Tier4_Transcendent, 0.5f));
	Table->AddRow(Deep, Fallen(EAscensionTier::Tier4_Transcendent, 1.f));
	Table->AddRow(Over, Fallen(EAscensionTier::Tier4_Transcendent, 3.f));
	Table->AddRow(Under, Fallen(EAscensionTier::Tier4_Transcendent, -1.f));
	Table->AddRow(DeepThree, Fallen(EAscensionTier::Tier3_Causal, 1.f));
	Table->AddRow(ShallowFive, Fallen(EAscensionTier::Tier5_EternalOrder, 0.f));
	const FCosmicDataTables Tables = WithTranscendents(Original, Table);

	// The pressure of one transcendent alone, with the given system in its sphere, read at ReadAt.
	auto Alone = [&](FName RowName, int32 SphereSystem, int32 ReadAt)
	{
		FGalaxyState State = Fresh;
		AddInstance(State, RowName, { State.Systems[SphereSystem].SystemID });
		return UGalaxySimulationSubsystem::ComputePressureIn(State, Index, Tables, Rules, ReadAt).P;
	};

	const float AtShallow = Alone(Shallow, Where.X, Where.X);
	const float AtMiddle = Alone(Middle, Where.X, Where.X);
	const float AtDeep = Alone(Deep, Where.X, Where.X);
	TestTrue(TEXT("With the same tier and the same sphere a deeper transcendent presses harder"), AtShallow < AtMiddle && AtMiddle < AtDeep);
	TestTrue(TEXT("Depth 0 is the plain weight of the tier and depth 1 exactly twice that"), AtShallow == Weight[3] && AtDeep == 2.f * Weight[3] && AtMiddle == 1.5f * Weight[3]);
	TestTrue(TEXT("A depth out of range is read as 0..1"), Alone(Over, Where.X, Where.X) == AtDeep && Alone(Under, Where.X, Where.X) == AtShallow);
	// Section 3.2: depth does not cross a tier. With the weights of the game the deepest tier 3 is below the shallowest
	// tier 4, and the deepest tier 4 is not above the shallowest tier 5.
	TestTrue(TEXT("Depth does not cross a tier: the deepest tier 3 stays below the shallowest tier 4"), Alone(DeepThree, Where.X, Where.X) < AtShallow);
	TestTrue(TEXT("Depth does not cross a tier: the deepest tier 4 is not above the shallowest tier 5"), AtDeep <= Alone(ShallowFive, Where.X, Where.X));
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
		// X is not reached by the deep transcendent only when Far is not one lane from X, which FindReachTriple guarantees.
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

	const FName Zeal(TEXT("trans_zeal"));
	const FName ZealTwin(TEXT("trans_zeal_twin"));
	const FName Assimilation(TEXT("trans_assimilation"));
	const FName Recurrence(TEXT("trans_recurrence"));
	const FName Covenant(TEXT("trans_covenant"));
	const FName Stillness(TEXT("trans_stillness"));

	// Four fallen doctrines (one of them twice), a celestial row and an independent row. The two rows that must never
	// be drawn carry the largest weights on purpose.
	auto Rows = [&](bool bReverse, bool bWithSilence)
	{
		TArray<TPair<FName, FTranscendentDef>> Defs;
		if (bWithSilence)
		{
			Defs.Emplace(CosmicRows::TransSilence, Fallen(EAscensionTier::Tier4_Transcendent, 1.f, TEXT("Doctrine.Silence")));
		}
		FTranscendentDef ZealDef = Fallen(EAscensionTier::Tier4_Transcendent, 0.6f, TEXT("Doctrine.Zeal"));
		ZealDef.SpawnWeight = 3.f;
		Defs.Emplace(Zeal, ZealDef);
		Defs.Emplace(ZealTwin, ZealDef);
		FTranscendentDef AssimilationDef = Fallen(EAscensionTier::Tier4_Transcendent, 0.5f, TEXT("Doctrine.Assimilation"));
		AssimilationDef.SpawnWeight = 2.f;
		Defs.Emplace(Assimilation, AssimilationDef);
		FTranscendentDef RecurrenceDef = Fallen(EAscensionTier::Tier4_Transcendent, 0.7f, TEXT("Doctrine.Recurrence"));
		RecurrenceDef.SpawnWeight = 2.f;
		Defs.Emplace(Recurrence, RecurrenceDef);
		FTranscendentDef CovenantDef = Fallen(EAscensionTier::Tier4_Transcendent, 0.5f, TEXT("Doctrine.Covenant"));
		CovenantDef.Allegiance = Tag(TEXT("Realm.Celestial"));
		CovenantDef.SpawnWeight = 50.f;
		Defs.Emplace(Covenant, CovenantDef);
		FTranscendentDef StillnessDef = Fallen(EAscensionTier::Tier4_Transcendent, 0.5f, TEXT("Doctrine.Stillness"));
		StillnessDef.Allegiance = Tag(TEXT("Realm.Independent"));
		StillnessDef.SpawnWeight = 50.f;
		Defs.Emplace(Stillness, StillnessDef);

		UDataTable* Table = NewTable(FTranscendentDef::StaticStruct());
		for (int32 Step = 0; Step < Defs.Num(); ++Step)
		{
			const TPair<FName, FTranscendentDef>& Def = Defs[bReverse ? Defs.Num() - 1 - Step : Step];
			Table->AddRow(Def.Key, Def.Value);
		}
		return Table;
	};
	auto TablesFor = [&](int32 Count, UDataTable* Table)
	{
		FCosmicRulesDef RulesRow = BaseRules;
		RulesRow.FallenTranscendentCount = Count;
		return WithTranscendents(WithRules(Original, RulesRow), Table);
	};
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

	// Thirty runs with three places: silence first, two more fallen rows, no doctrine twice, nobody of another realm.
	Sim->SetTables(TablesFor(3, Rows(false, true)));
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
	TestTrue(TEXT("Over thirty seeds every fallen doctrine with a weight is drawn"),
		(Seen.Contains(Zeal) || Seen.Contains(ZealTwin)) && Seen.Contains(Assimilation) && Seen.Contains(Recurrence));

	// The order of the rows in the table is not the order of the draw.
	{
		Sim->SetTables(TablesFor(3, Rows(true, true)));
		bool bSame = true;
		for (int32 Seed = 1; Seed <= 30; ++Seed)
		{
			bSame &= RosterOf(Seed) == Rosters[Seed - 1];
		}
		TestTrue(TEXT("Reversing the rows of DT_Transcendents changes no roster"), bSame);
	}

	// The number of draws follows the knob alone: the same stream position with a full table, an empty one and none,
	// and exactly two draws more than with one place.
	{
		auto SeedAfter = [Sim](const FCosmicDataTables& Tables)
		{
			Sim->SetTables(Tables);
			Sim->NewGalaxy(7, 12, 4);
			return Sim->GetRng().GetCurrentSeed();
		};
		const int32 Full = SeedAfter(TablesFor(3, Rows(false, true)));
		TestEqual(TEXT("An empty DT_Transcendents draws like a full one"), SeedAfter(TablesFor(3, NewTable(FTranscendentDef::StaticStruct()))), Full);
		TestEqual(TEXT("No DT_Transcendents draws like a full one"), SeedAfter(TablesFor(3, nullptr)), Full);
		TestEqual(TEXT("A table without the silence row draws like a full one"), SeedAfter(TablesFor(3, Rows(false, false))), Full);
		FRandomStream Probe(SeedAfter(TablesFor(1, Rows(false, true))));
		Probe.GetFraction();
		Probe.GetFraction();
		TestEqual(TEXT("Three places draw exactly twice more than one place"), Full, Probe.GetCurrentSeed());
	}

	// One place is silence alone, no place is nobody, and without the silence row nobody takes its place.
	{
		Sim->SetTables(TablesFor(1, Rows(false, true)));
		const TArray<FName> One = RosterOf(3);
		Sim->SetTables(TablesFor(0, Rows(false, true)));
		const TArray<FName> None = RosterOf(3);
		Sim->SetTables(TablesFor(3, Rows(false, false)));
		const TArray<FName> NoSilence = RosterOf(3);
		TestTrue(TEXT("One place is silence alone and no place is nobody"), One.Num() == 1 && One[0] == CosmicRows::TransSilence && None.IsEmpty());
		TestTrue(TEXT("Without the silence row the two drawn places are all there is"), NoSilence.Num() == 2 && !NoSilence.Contains(CosmicRows::TransSilence));
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

	const FName Presser(TEXT("tr_presser"));
	const float StartStability = 0.8f;

	// A tier-4 transcendent of depth 0 whose per-tick effect leaves a countable trace, or halves the Energy income.
	auto TablesWith = [&](bool bHalveEnergy)
	{
		FTranscendentDef Def = Fallen(EAscensionTier::Tier4_Transcendent, 0.f);
		FCosmicEffect Effect;
		if (bHalveEnergy)
		{
			Effect.Type = EEffectType::MultiplyIncome;
			Effect.Key = FName(TEXT("Energy"));
			Effect.Value = 0.5f;
		}
		else
		{
			// Causality stays at zero in the economy of the first tiers: every unit of it comes from this effect.
			Effect.Type = EEffectType::AddResource;
			Effect.Key = FName(TEXT("Causality"));
			Effect.Value = 1.f;
		}
		Def.PressureEffectsPerTick.Add(Effect);
		UDataTable* Table = NewTable(FTranscendentDef::StaticStruct());
		Table->AddRow(Presser, Def);
		return WithTranscendents(Original, Table);
	};

	// A fresh galaxy in which civilization 1 owns three settled systems and civilization 2 one. With bSphere all four
	// lie in the sphere of the transcendent.
	auto Arrange = [&](bool bSphere)
	{
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		State.Transcendents.Reset();
		FCivilizationState& Wide = State.Civs[1];
		for (FStarSystemState& System : State.Systems)
		{
			if (System.OwnerCivID.IsNone() && Wide.OwnedSystems.Num() < 3)
			{
				System.OwnerCivID = Wide.CivID;
				Wide.OwnedSystems.Add(System.SystemID);
			}
		}
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.Stability = StartStability;
		}
		if (bSphere)
		{
			TArray<FName> Sphere = Wide.OwnedSystems;
			Sphere.Add(State.Civs[2].HomeSystemID);
			AddInstance(State, Presser, Sphere);
		}
	};
	// The largest pressure over the settled systems of a civilization whose dominant source stands above it, read
	// before the tick with the public function.
	auto LargestOver = [Sim](const FCivilizationState& Civ)
	{
		float Largest = 0.f;
		for (const FName SystemID : Civ.OwnedSystems)
		{
			const FCosmicPressure Reading = UGalaxySimulationSubsystem::ComputePressureIn(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(),
				Sim->GetStateIndex().FindSystem(Sim->GetState(), SystemID));
			if (Reading.DominantSource != ECosmicPressureSource::None && static_cast<int32>(Reading.DominantTier) > static_cast<int32>(Civ.Tier))
			{
				Largest = FMath::Max(Largest, Reading.P);
			}
		}
		return Largest;
	};

	// One tick: the drain is taken once per civilization, at its largest pressure, and the effects once per transcendent.
	{
		Sim->SetTables(TablesWith(false));
		Arrange(true);
		const FGalaxyState& State = Sim->GetState();
		const float Drain = Sim->GetRules().SpiritualPressureStabilityDrain;
		TArray<float> Expected;
		for (const FCivilizationState& Civ : State.Civs)
		{
			Expected.Add(StartStability - Drain * LargestOver(Civ));
		}
		TestTrue(TEXT("The test civilization owns three settled systems, all under a pressure of exactly the tier-4 weight"),
			State.Civs[1].OwnedSystems.Num() == 3 && LargestOver(State.Civs[1]) == Sim->GetRules().PressureBySourceTier[3] && Drain > 0.f);

		Sim->RunHeadless(1);
		bool bDrain = true;
		for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
		{
			bDrain &= State.Civs[CivIndex].Stability == Expected[CivIndex];
		}
		TestTrue(TEXT("Every civilization loses the drain times its largest pressure, once"), bDrain);
		TestTrue(TEXT("Three systems in the sphere cost what one costs"),
			State.Civs[1].Stability == State.Civs[2].Stability && State.Civs[1].Stability < StartStability);
		TestTrue(TEXT("The per-tick effects of a transcendent are applied once per civilization, not once per system"),
			State.Civs[1].Stockpile.Causality == 1.f && State.Civs[2].Stockpile.Causality == 1.f);
	}

	// A civilization on the tier of the transcendent is not drained and does not take its effects.
	{
		Sim->SetTables(TablesWith(false));
		Arrange(true);
		Sim->GetMutableState().Civs[1].Tier = EAscensionTier::Tier4_Transcendent;
		Sim->RunHeadless(1);
		TestTrue(TEXT("A source that does not stand above the civilization neither drains it nor applies its effects"),
			Sim->GetState().Civs[1].Stability == StartStability && Sim->GetState().Civs[1].Stockpile.Causality == 0.f);
	}

	// A sealed transcendent presses on nobody.
	{
		Sim->SetTables(TablesWith(false));
		Arrange(true);
		Sim->GetMutableState().Transcendents[0].bSealed = true;
		Sim->RunHeadless(1);
		TestTrue(TEXT("A sealed transcendent neither drains nor applies its effects"),
			Sim->GetState().Civs[1].Stability == StartStability && Sim->GetState().Civs[1].Stockpile.Causality == 0.f);
	}

	// MultiplyIncome as a per-tick source: the income of this tick is scaled before it reaches the stockpile.
	{
		Sim->SetTables(TablesWith(true));
		Arrange(false);
		const float EnergyBefore = Sim->GetState().Civs[1].Stockpile.Energy;
		Sim->RunHeadless(1);
		const float FreeIncome = Sim->GetState().Civs[1].IncomePerTick.Energy;

		Arrange(true);
		Sim->RunHeadless(1);
		const FCivilizationState& Pressed = Sim->GetState().Civs[1];
		TestTrue(TEXT("A per-tick MultiplyIncome halves the income of the tick, and the halved income is what reaches the stockpile"),
			FreeIncome > 0.f && Pressed.IncomePerTick.Energy == 0.5f * FreeIncome && Pressed.Stockpile.Energy == EnergyBefore + Pressed.IncomePerTick.Energy);
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
	// civilization below the threshold and LowStabilityTicks is 1 at the end of that tick.
	{
		const FName Presser(TEXT("tr_presser"));
		UDataTable* Table = NewTable(FTranscendentDef::StaticStruct());
		Table->AddRow(Presser, Fallen(EAscensionTier::Tier4_Transcendent, 0.f));
		Sim->SetTables(WithTranscendents(Original, Table));
		Sim->NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim->GetMutableState();
		State.Transcendents.Reset();
		AddInstance(State, Presser, { State.Civs[1].HomeSystemID });
		// What one tick takes from a tier-1 people whose capital lies in the sphere of a tier-4 source of depth 0.
		const float Drain = Sim->GetRules().SpiritualPressureStabilityDrain * Sim->GetRules().PressureBySourceTier[3];
		// Half a drain above the threshold: one tick of pressure crosses it.
		const float JustAbove = 0.3f + 0.5f * Drain;
		State.Civs[1].Stability = JustAbove;
		const bool bStartsUncounted = State.Civs[1].LowStabilityTicks == 0 && Drain > 0.f;
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
		FCosmicEffect Effect;
		Effect.Type = EEffectType::ChangeStability;
		Effect.Value = -0.5f;
		Def.Effects.Add(Effect);
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

// Cosmic Ascension - automation tests of the table reads B6b [B-6br] made cheaper without changing an answer (the time
// gate G1 of bundle B): the sorted row names of FCosmicDataTables, answered from a memo checked against the table's keys
// on every call, and IsHazardAnswer, which walks the row map without building an array of names.
// Design: docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md section 5 [B-6br]; engine facts in
// docs/45_ENGINE_NOTES.md (UDataTable::GetRowNames, FNameLexicalLess, FName::IsEqual).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Ascension/AscensionSubsystem.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicB6bRowNames.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/NameTypes.h"

#include <initializer_list>

namespace CosmicTableNamesTests
{
	// The code before [B-6br]: the table's names gathered and sorted on every call.
	TArray<FName> Uncached(const UDataTable* Table)
	{
		TArray<FName> Names = Table ? Table->GetRowNames() : TArray<FName>();
		Names.Sort(FNameLexicalLess());
		return Names;
	}

	// Element by element as text, case and number included (TArray<FName>::operator== does not see case).
	bool Same(const TArray<FName>& Got, const TArray<FName>& Want)
	{
		if (Got.Num() != Want.Num())
		{
			return false;
		}
		for (int32 At = 0; At < Got.Num(); ++At)
		{
			if (!Got[At].ToString().Equals(Want[At].ToString(), ESearchCase::CaseSensitive))
			{
				return false;
			}
		}
		return true;
	}

	bool Same(const TArray<FName>& Got, std::initializer_list<const TCHAR*> Want)
	{
		if (Got.Num() != static_cast<int32>(Want.size()))
		{
			return false;
		}
		int32 At = 0;
		for (const TCHAR* Each : Want)
		{
			if (!Got[At++].ToString().Equals(Each, ESearchCase::CaseSensitive))
			{
				return false;
			}
		}
		return true;
	}

	// The walk IsHazardAnswer made before [B-6br].
	bool UncachedAnswer(const FCosmicDataTables& Tables, FName RowName)
	{
		if (RowName.IsNone() || !Tables.Hazards)
		{
			return false;
		}
		for (const FName HazardRow : Tables.Hazards->GetRowNames())
		{
			const FHazardDef* Row = Tables.FindHazard(HazardRow);
			if (Row && Row->RulerResolutions.Contains(RowName))
			{
				return true;
			}
		}
		return false;
	}
}

// Every sorting accessor of FCosmicDataTables gives the names a fresh sort would give - on the game's tables (fresh, and
// warmed by 300 ticks of the sim), on a table edited in place after the memo was made (a row added, one taken out and
// another put in, no table, an empty table, a name in another case, a name with another number), and on a copy that
// carries a warm memo and whose table was replaced (the pattern of CosmicTests::WithoutB6bData), for all eight.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicDataSortedNamesFollowTableTest, "Cosmic_Ascension.Data.SortedNamesFollowTable", CosmicTests::Flags)

bool FCosmicDataSortedNamesFollowTableTest::RunTest(const FString& Parameters)
{
	using namespace CosmicTableNamesTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	FCosmicDataTables Fresh;
	Fresh.LoadFromFixedPaths();
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };
	Sim->SetTables(Fresh);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(300);
	// A reference to the member the stages warmed, never a copy.
	const FCosmicDataTables& Live = Sim->GetTables();

	struct FAccessor
	{
		const TCHAR* Name;
		TArray<FName> (FCosmicDataTables::*Get)() const;
		TObjectPtr<UDataTable> FCosmicDataTables::*Table;
		UDataTable* (*CopyLess)(const UDataTable*, FName);
	};
	const FAccessor Accessors[] = {
		{ TEXT("GetCivRowNames"), &FCosmicDataTables::GetCivRowNames, &FCosmicDataTables::Civs,
			[](const UDataTable* T, FName N) { return CosmicTests::CopyWithout<FCivArchetypeDef>(T, { N }); } },
		{ TEXT("GetEventRowNames"), &FCosmicDataTables::GetEventRowNames, &FCosmicDataTables::Events,
			[](const UDataTable* T, FName N) { return CosmicTests::CopyWithout<FCosmicEventDef>(T, { N }); } },
		{ TEXT("GetTranscendentRowNames"), &FCosmicDataTables::GetTranscendentRowNames, &FCosmicDataTables::Transcendents,
			[](const UDataTable* T, FName N) { return CosmicTests::CopyWithout<FTranscendentDef>(T, { N }); } },
		{ TEXT("GetHazardRowNames"), &FCosmicDataTables::GetHazardRowNames, &FCosmicDataTables::Hazards,
			[](const UDataTable* T, FName N) { return CosmicTests::CopyWithout<FHazardDef>(T, { N }); } },
		{ TEXT("GetFeatRowNames"), &FCosmicDataTables::GetFeatRowNames, &FCosmicDataTables::Feats,
			[](const UDataTable* T, FName N) { return CosmicTests::CopyWithout<FAscensionFeatDef>(T, { N }); } },
		{ TEXT("GetEncounterRowNames"), &FCosmicDataTables::GetEncounterRowNames, &FCosmicDataTables::Encounters,
			[](const UDataTable* T, FName N) { return CosmicTests::CopyWithout<FEncounterDef>(T, { N }); } },
		{ TEXT("GetEdictRowNames"), &FCosmicDataTables::GetEdictRowNames, &FCosmicDataTables::Edicts,
			[](const UDataTable* T, FName N) { return CosmicTests::CopyWithout<FEdictDef>(T, { N }); } },
		{ TEXT("GetGalaxyRowNames"), &FCosmicDataTables::GetGalaxyRowNames, &FCosmicDataTables::Galaxies,
			[](const UDataTable* T, FName N) { return CosmicTests::CopyWithout<FGalaxyDef>(T, { N }); } },
	};

	// (i) The game's tables, every accessor, twice: the fresh set first (its first call is the miss path), then the set
	// the stages warmed (its first call may be a hit).
	struct FSet
	{
		const TCHAR* Label;
		const FCosmicDataTables* Tables;
	};
	const FSet Sets[] = { { TEXT("fresh"), &Fresh }, { TEXT("live"), &Live } };
	for (const FSet& Set : Sets)
	{
		for (const FAccessor& A : Accessors)
		{
			const UDataTable* Table = ((*Set.Tables).*A.Table).Get();
			if (!TestNotNull(FString::Printf(TEXT("(the %s tables hold the table of %s)"), Set.Label, A.Name), Table))
			{
				continue;
			}
			const TArray<FName> Want = Uncached(Table);
			TestTrue(FString::Printf(TEXT("i-1: %s on the %s tables gives the uncached names"), A.Name, Set.Label),
				Same(((*Set.Tables).*A.Get)(), Want));
			TestTrue(FString::Printf(TEXT("i-2: and so does its second call, from the memo (%s, %s)"), A.Name, Set.Label),
				Same(((*Set.Tables).*A.Get)(), Want));
		}
	}

	// (ii) One table edited in place after the memo was made: a stale memo would answer the old names.
	{
		UDataTable* Feats = CosmicTests::NewTable(FAscensionFeatDef::StaticStruct());
		Feats->AddRow(FName(TEXT("feat_b")), FAscensionFeatDef());
		Feats->AddRow(FName(TEXT("feat_a")), FAscensionFeatDef());
		FCosmicDataTables Board;
		Board.Feats = Feats;
		TestTrue(TEXT("ii-1: Rows added b, a are named a, b"), Same(Board.GetFeatRowNames(), { TEXT("feat_a"), TEXT("feat_b") }));
		TestTrue(TEXT("ii-2: and again from the memo"), Same(Board.GetFeatRowNames(), { TEXT("feat_a"), TEXT("feat_b") }));
		Feats->AddRow(FName(TEXT("feat_c")), FAscensionFeatDef());
		TestTrue(TEXT("ii-3: A row added after the first call is named"),
			Same(Board.GetFeatRowNames(), { TEXT("feat_a"), TEXT("feat_b"), TEXT("feat_c") }));
		Feats->RemoveRow(FName(TEXT("feat_a")));
		Feats->AddRow(FName(TEXT("feat_d")), FAscensionFeatDef());
		TestTrue(TEXT("ii-4: A row taken out and another put in (the same count) is followed"),
			Same(Board.GetFeatRowNames(), { TEXT("feat_b"), TEXT("feat_c"), TEXT("feat_d") }));
		// Right after a non-empty memo.
		Board.Feats = nullptr;
		TestTrue(TEXT("ii-5: No table: no names, not the memo's"), Board.GetFeatRowNames().IsEmpty());
		Board.Feats = CosmicTests::NewTable(FAscensionFeatDef::StaticStruct());
		TestTrue(TEXT("ii-6: An empty table: no names"), Board.GetFeatRowNames().IsEmpty());
	}

	// (ii') The case and the number of a row's name, on a second table and instance. The renamed row is always the last
	// one added, so its place in the map does not move (a sparse set reuses its slot, a compact set appends it again):
	// only the name differs from the memo's key. The case pair needs the editor binary, which keeps case.
	{
		UDataTable* Feats = CosmicTests::NewTable(FAscensionFeatDef::StaticStruct());
		Feats->AddRow(FName(TEXT("feat_c")), FAscensionFeatDef());
		Feats->AddRow(FName(TEXT("feat_b")), FAscensionFeatDef());
		Feats->AddRow(FName(TEXT("feat_d")), FAscensionFeatDef());
		FCosmicDataTables Board;
		Board.Feats = Feats;
		TestTrue(TEXT("ii'-1: Rows added c, b, d are named b, c, d"),
			Same(Board.GetFeatRowNames(), { TEXT("feat_b"), TEXT("feat_c"), TEXT("feat_d") }));
		Feats->RemoveRow(FName(TEXT("feat_d")));
		Feats->AddRow(FName(TEXT("Feat_D")), FAscensionFeatDef());
		TestTrue(TEXT("ii'-2: A row renamed in another case is named in its new case"),
			Same(Board.GetFeatRowNames(), { TEXT("feat_b"), TEXT("feat_c"), TEXT("Feat_D") }));
		Feats->RemoveRow(FName(TEXT("Feat_D")));
		Feats->AddRow(FName(TEXT("feat_n_1")), FAscensionFeatDef());
		TestTrue(TEXT("ii'-3: A row with a number is named with it"),
			Same(Board.GetFeatRowNames(), { TEXT("feat_b"), TEXT("feat_c"), TEXT("feat_n_1") }));
		Feats->RemoveRow(FName(TEXT("feat_n_1")));
		Feats->AddRow(FName(TEXT("feat_n_2")), FAscensionFeatDef());
		TestTrue(TEXT("ii'-4: A row renumbered is named with its new number"),
			Same(Board.GetFeatRowNames(), { TEXT("feat_b"), TEXT("feat_c"), TEXT("feat_n_2") }));
	}

	// (iii) A copy that carries a warm memo and whose table was replaced, for every accessor (the pattern of
	// CosmicTests::WithoutB6bData with the memos warm - the commandlet's -WithoutB6bData=1 copies cold memos). The copy
	// loses the lexically first name, so the count always changes.
	for (const FAccessor& A : Accessors)
	{
		const TArray<FName> Names = (Fresh.*A.Get)();
		if (!TestTrue(FString::Printf(TEXT("(the game's table of %s has rows)"), A.Name), Names.Num() > 0))
		{
			continue;
		}
		FCosmicDataTables Copy = Fresh;
		Copy.*A.Table = A.CopyLess((Fresh.*A.Table).Get(), Names[0]);
		TestTrue(FString::Printf(TEXT("iii: %s: a copy whose table lost a row gives that table's names"), A.Name),
			Same((Copy.*A.Get)(), Uncached((Copy.*A.Table).Get())));
	}
	return true;
}

// IsHazardAnswer walks the row map without an array of names: an answer only the last row lists is an answer, a name no
// row lists is none, and every DT_Megaprojects and DT_Edicts row of the game gets the answer of the walk before [B-6br].
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicDataHazardAnswerAnyRowTest, "Cosmic_Ascension.Data.HazardAnswerAnyRow", CosmicTests::Flags)

bool FCosmicDataHazardAnswerAnyRowTest::RunTest(const FString& Parameters)
{
	using namespace CosmicTableNamesTests;
	{
		UDataTable* Hazards = CosmicTests::NewTable(FHazardDef::StaticStruct());
		Hazards->AddRow(FName(TEXT("crisis_test_a")), FHazardDef());
		FHazardDef One;
		One.RulerResolutions = { FName(TEXT("project_test_one")) };
		Hazards->AddRow(FName(TEXT("crisis_test_b")), One);
		FHazardDef Two;
		Two.RulerResolutions = { FName(TEXT("project_test_two")) };
		Hazards->AddRow(FName(TEXT("crisis_test_c")), Two);
		FCosmicDataTables Board;
		Board.Hazards = Hazards;
		// The last row comes last in both the table's order and the lexical one.
		TestTrue(TEXT("b-1: A project only the last row lists answers a hazard"), Board.IsHazardAnswer(FName(TEXT("project_test_two"))));
		TestFalse(TEXT("b-2: A project no row lists answers none"), Board.IsHazardAnswer(FName(TEXT("project_test_none"))));
	}

	FCosmicDataTables Game;
	Game.LoadFromFixedPaths();
	// DT_Hazards holds crisis_species_extinction last, listing project_ark.
	TestTrue(TEXT("(the game's ark answers the extinction, its last hazard row)"), UncachedAnswer(Game, CosmicB6bRows::ProjectArk));
	// The two kinds of names the callers pass: megaproject rows, and edict rows (Sim/CosmicEdicts.cpp).
	const UDataTable* Kinds[] = { Game.Megaprojects.Get(), Game.Edicts.Get() };
	for (const UDataTable* Kind : Kinds)
	{
		if (!TestNotNull(TEXT("(the game's tables hold DT_Megaprojects and DT_Edicts)"), Kind))
		{
			continue;
		}
		for (const FName Row : Kind->GetRowNames())
		{
			TestEqual(FString::Printf(TEXT("b-3: IsHazardAnswer(%s) is the answer of the walk before [B-6br]"), *Row.ToString()),
				Game.IsHazardAnswer(Row), UncachedAnswer(Game, Row));
		}
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

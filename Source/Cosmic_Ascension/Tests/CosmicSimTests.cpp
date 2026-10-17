// Cosmic Ascension - automation tests of the simulation core. Design: guideline.md section 9.2, 9.3.
// Run with the automation command in CLAUDE.md, twice in a row after a build (see Sim.SaveRestart).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Containers/StringConv.h"
#include "Containers/StringView.h"
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
#include "GameplayTagContainer.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Internationalization/Text.h"
#include "Kismet/GameplayStatics.h"
#include "Log/CosmicLogSubsystem.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Modules/ModuleManager.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Save/CosmicSaveGame.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/Package.h"
#include "UObject/TextProperty.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"

#include <limits>

namespace CosmicSimTests
{
	// The run of Sim.SaveRestart: past the first second rung, the first settlements and their draws, and the first feats
	// (seed 42), so that the second process re-checks them.
	constexpr int64 RestartTicks = 3000;

	uint32 HashAfter(UGalaxySimulationSubsystem& Sim, int32 Seed, int64 Ticks)
	{
		Sim.NewGalaxy(Seed, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		Sim.RunHeadless(Ticks);
		return Sim.ComputeStateHash();
	}

	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// Lanes from one system, walked by the test itself (not by CosmicState::LaneDistancesFrom): INDEX_NONE = unreachable.
	TArray<int32> LanesFrom(const FGalaxyState& State, int32 From)
	{
		TMap<FName, int32> ByID;
		for (int32 Index = 0; Index < State.Systems.Num(); ++Index)
		{
			ByID.Add(State.Systems[Index].SystemID, Index);
		}
		TArray<int32> Distance;
		Distance.Init(INDEX_NONE, State.Systems.Num());
		TArray<int32> Queue = { From };
		Distance[From] = 0;
		for (int32 Head = 0; Head < Queue.Num(); ++Head)
		{
			for (const FName Lane : State.Systems[Queue[Head]].Lanes)
			{
				const int32* Next = ByID.Find(Lane);
				if (Next && Distance[*Next] == INDEX_NONE)
				{
					Distance[*Next] = Distance[Queue[Head]] + 1;
					Queue.Add(*Next);
				}
			}
		}
		return Distance;
	}

	// A DT_Galaxies with one home row under the given name: counts, star names "star <n>" and catalog pools of the
	// given sizes ("p<n>" x "s<n>").
	UDataTable* GalaxyTable(FName RowName, int32 LivingWorlds, int32 Peoples, int32 Stars, int32 Prefixes, int32 Stems)
	{
		UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
		Table->RowStruct = FGalaxyDef::StaticStruct();
		FGalaxyDef Row;
		Row.LivingWorlds = LivingWorlds;
		Row.Peoples = Peoples;
		for (int32 Index = 0; Index < Stars; ++Index)
		{
			Row.StarNameBank.Add(FText::AsCultureInvariant(FString::Printf(TEXT("star %d"), Index)));
		}
		for (int32 Index = 0; Index < Prefixes; ++Index)
		{
			Row.CatalogPrefixes.Add(FText::AsCultureInvariant(FString::Printf(TEXT("p%d"), Index)));
		}
		for (int32 Index = 0; Index < Stems; ++Index)
		{
			Row.CatalogStems.Add(FText::AsCultureInvariant(FString::Printf(TEXT("s%d"), Index)));
		}
		Table->AddRow(RowName, Row);
		return Table;
	}

	// A DT_Transcendents with the silence row and, with Extra above 0, more fallen rows of other doctrines. Every row has
	// a starting core when bCores.
	UDataTable* TranscendentTable(bool bCores, int32 Extra = 0)
	{
		UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
		Table->RowStruct = FTranscendentDef::StaticStruct();
		const TCHAR* Doctrines[] = { TEXT("Doctrine.Silence"), TEXT("Doctrine.Zeal"), TEXT("Doctrine.Assimilation") };
		const TCHAR* Names[] = { TEXT("trans_silence"), TEXT("trans_zeal"), TEXT("trans_assimilation") };
		for (int32 Index = 0; Index <= Extra && Index < 3; ++Index)
		{
			FTranscendentDef Row;
			Row.Doctrine = Tag(Doctrines[Index]);
			Row.Allegiance = Tag(TEXT("Realm.Abyssal"));
			Row.InitialInfluencedSystems = bCores ? 1 : 0;
			Table->AddRow(FName(Names[Index]), Row);
		}
		return Table;
	}

	// Counts non-finite floating point values anywhere under a struct, by reflection.
	int32 CountNonFinite(const FProperty* Property, const void* Value);

	int32 CountNonFiniteInStruct(const UStruct* Struct, const void* Data)
	{
		int32 Count = 0;
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			for (int32 Index = 0; Index < It->ArrayDim; ++Index)
			{
				Count += CountNonFinite(*It, It->ContainerPtrToValuePtr<void>(Data, Index));
			}
		}
		return Count;
	}

	int32 CountNonFinite(const FProperty* Property, const void* Value)
	{
		if (const FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
		{
			return (NumericProperty->IsFloatingPoint() && !FMath::IsFinite(NumericProperty->GetFloatingPointPropertyValue(Value))) ? 1 : 0;
		}
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			return CountNonFiniteInStruct(StructProperty->Struct, Value);
		}
		if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			int32 Count = 0;
			FScriptArrayHelper Helper(ArrayProperty, Value);
			for (int32 Index = 0; Index < Helper.Num(); ++Index)
			{
				Count += CountNonFinite(ArrayProperty->Inner, Helper.GetRawPtr(Index));
			}
			return Count;
		}
		if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
		{
			int32 Count = 0;
			FScriptMapHelper Helper(MapProperty, Value);
			for (int32 Index = 0; Index < Helper.GetMaxIndex(); ++Index)
			{
				if (Helper.IsValidIndex(Index))
				{
					Count += CountNonFinite(MapProperty->ValueProp, Helper.GetValuePtr(Index));
				}
			}
			return Count;
		}
		return 0;
	}

	// Every text anywhere under a struct, by reflection: the names and sentences that ComputeStateHash leaves out.
	// Maps are not walked; the state keeps no text in a map.
	void AppendTexts(const FProperty* Property, const void* Value, FString& Out);

	void AppendTextsInStruct(const UStruct* Struct, const void* Data, FString& Out)
	{
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			for (int32 Index = 0; Index < It->ArrayDim; ++Index)
			{
				AppendTexts(*It, It->ContainerPtrToValuePtr<void>(Data, Index), Out);
			}
		}
	}

	void AppendTexts(const FProperty* Property, const void* Value, FString& Out)
	{
		if (const FTextProperty* TextProperty = CastField<FTextProperty>(Property))
		{
			Out += TextProperty->GetPropertyValue(Value).ToString();
			Out += TEXT("|");
		}
		else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			AppendTextsInStruct(StructProperty->Struct, Value, Out);
		}
		else if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			FScriptArrayHelper Helper(ArrayProperty, Value);
			for (int32 Index = 0; Index < Helper.Num(); ++Index)
			{
				AppendTexts(ArrayProperty->Inner, Helper.GetRawPtr(Index), Out);
			}
		}
	}

	FString CollectText(const FGalaxyState& State)
	{
		FString Out;
		AppendTextsInStruct(FGalaxyState::StaticStruct(), &State, Out);
		return Out;
	}

	// Binary that holds the simulation code. Its timestamp tells a rebuilt game from a restarted one.
	FString SimBinaryPath()
	{
#if IS_MONOLITHIC
		return FPlatformProcess::ExecutablePath();
#else
		return FModuleManager::Get().GetModuleFilename(FName(TEXT("Cosmic_Ascension")));
#endif
	}

	// The definition tables are simulation input too: regenerated assets start the restart test over, like a rebuilt binary.
	// Every table the loader holds, read from its members by reflection, so a table added to FCosmicDataTables joins the
	// stamp without an edit here. A table injected by a test lives in the transient package and is left out (its package
	// name still converts to a file name under /Engine, one that does not exist).
	FString DataStamp(const FCosmicDataTables& Tables)
	{
		FString Stamp;
		for (TFieldIterator<FObjectPropertyBase> It(FCosmicDataTables::StaticStruct()); It; ++It)
		{
			const UObject* Table = It->GetObjectPropertyValue_InContainer(&Tables);
			FString File;
			if (Table && Table->GetPackage() != GetTransientPackage() && FPackageName::TryConvertLongPackageNameToFilename(Table->GetPackage()->GetName(), File, FPackageName::GetAssetPackageExtension()))
			{
				Stamp += TEXT("_") + It->GetName() + TEXT("@") + IFileManager::Get().GetTimeStamp(*File).ToIso8601();
			}
		}
		return Stamp;
	}

	// CosmicSeed::Derive for a name held as text, the way a caller that holds an FName has to pass it: lower-cased, as
	// ANSI bytes (section 9.2). Evaluated at run time, unlike the static_asserts of CosmicSeed.h.
	uint32 DeriveLowerCased(int32 Seed, const TCHAR* Name, uint32 Purpose, uint32 Index)
	{
		const FString Lower = FString(Name).ToLower();
		const auto Ansi = StringCast<ANSICHAR>(*Lower);
		return CosmicSeed::Derive(Seed, FAnsiStringView(Ansi.Get(), Ansi.Length()), Purpose, Index);
	}

	// Puts one value into every container of the state that a plain run leaves empty, so that the save and the hash see
	// tag containers, maps, notables, transcendents, log entries, the knowing and a crossing. Of the 25 members reserved
	// in v0.7 (section 9.1), 24 leave their initializer here: no game code moves them off it yet, and a member that
	// reloads as its initializer looks the same whether it was saved or not. The 25th, TellRngState, is not touched:
	// NewGalaxy stamps it and DrawTell moves it. Needs three civs or more.
	void FillEmptyContainers(FGalaxyState& State)
	{
		const FName TestRow(TEXT("evt_test"));
		const FName HomeGalaxy(TEXT("gal_milkyway"));

		State.Flags.AddTag(Tag(TEXT("Flag.Grace.Spent")));
		State.Civs[0].Traits.AddTag(Tag(TEXT("Trait.Faith.Seeking")));
		State.Player.StatusTags.AddTag(Tag(TEXT("Status.Abdicated")));
		State.Calling.Powers.AddTag(Tag(TEXT("Way.Received")));
		State.EventCooldownUntilTick.Add(TestRow, 5);
		State.Civs[0].Relations.Add(State.Civs[1].CivID, 0.5f);

		FNotableRecord& Notable = State.Civs[0].Notables.AddDefaulted_GetRef();
		Notable.Role = Tag(TEXT("Notable.Rebel"));
		Notable.NameIndex = 3;
		Notable.FirstTick = 10;
		Notable.SourceRow = TestRow;

		FTranscendentState& Transcendent = State.Transcendents.AddDefaulted_GetRef();
		Transcendent.TranscendentID = FName(TEXT("tr_test"));
		Transcendent.Doctrine = Tag(TEXT("Doctrine.Silence"));
		Transcendent.InfluencedSystems.Add(State.Systems.Last().SystemID);

		FCosmicLogEntry Entry;
		Entry.Tick = 10;
		Entry.Category = Tag(TEXT("Log.Civ"));
		Entry.Message = FText::AsCultureInvariant(TEXT("x"));
		Entry.FocusSystemID = State.Systems[0].SystemID;
		Entry.FocusCivID = State.Civs[1].CivID;
		Entry.bUntold = true;
		Entry.SourceRow = TestRow;
		State.Log.Add(Entry);
		State.Chronicle.Add(Entry);

		State.NextCivSerial = State.Civs.Num();
		for (FStarSystemState& System : State.Systems)
		{
			System.GalaxyRow = HomeGalaxy;
		}
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.GalaxyRow = HomeGalaxy;
		}
		State.Civs[1].OriginGalaxyRow = FName(TEXT("gal_andromeda"));

		FKnownMark& KnownSystem = State.Knowing.KnownSystems.AddDefaulted_GetRef();
		KnownSystem.ID = State.Systems[0].SystemID;
		KnownSystem.Tick = 11;
		FKnownMark& MetCiv = State.Knowing.MetCivs.AddDefaulted_GetRef();
		MetCiv.ID = State.Civs[1].CivID;
		MetCiv.Tick = 12;
		FKnownMark& MetGalaxy = State.Knowing.MetGalaxies.AddDefaulted_GetRef();
		MetGalaxy.ID = FName(TEXT("gal_lmc"));
		MetGalaxy.Tick = 13;
		State.Knowing.PendingContactCivID = State.Civs[2].CivID;
		State.Knowing.FirstContactCivID = State.Civs[1].CivID;
		State.Knowing.FarLineUntilTick = 14;

		FTransitRecord& Transit = State.Transits.AddDefaulted_GetRef();
		// Transit.People is declared in Phase 6a (section 7.2). Until then a declared tag stands in: the member only has
		// to leave its initializer.
		Transit.Kind = Tag(TEXT("Succession.Installed"));
		Transit.FromGalaxyRow = HomeGalaxy;
		Transit.ToGalaxyRow = FName(TEXT("gal_lmc"));
		Transit.DepartTick = 15;
		Transit.ArriveTick = 16;
		Transit.SourceCivID = State.Civs[1].CivID;
		Transit.ArchetypeRow = FName(TEXT("arch_test"));
		Transit.ResultCivID = FName(TEXT("civ_test"));
	}

	// How many of the 25 members reserved in v0.7 (section 9.1) are off their initializer. A member of an array element
	// counts when it is set in the last element of every array that holds it; OriginGalaxyRow is read in Civs[1], the
	// one civ FillEmptyContainers makes a newcomer (the others stay native, NAME_None).
	int32 CountReservedMembersSet(const FGalaxyState& State)
	{
		auto MarkSet = [](const TArray<FKnownMark>& Marks, bool bID)
		{
			return !Marks.IsEmpty() && (bID ? !Marks.Last().ID.IsNone() : Marks.Last().Tick != 0);
		};
		const FKnowingState& Knowing = State.Knowing;
		const bool bLogs = !State.Log.IsEmpty() && !State.Chronicle.IsEmpty();
		const bool bCivs = State.Civs.Num() > 1;

		int32 Count = 0;
		// FGalaxyState: TellRngState, NextCivSerial, Knowing, Transits.
		Count += (State.TellRngState != 0) ? 1 : 0;
		Count += (State.NextCivSerial != 0) ? 1 : 0;
		Count += (!Knowing.KnownSystems.IsEmpty() || !Knowing.MetCivs.IsEmpty() || !Knowing.MetGalaxies.IsEmpty()) ? 1 : 0;
		Count += !State.Transits.IsEmpty() ? 1 : 0;
		// FKnowingState: three arrays and three scalars.
		Count += !Knowing.KnownSystems.IsEmpty() ? 1 : 0;
		Count += !Knowing.MetCivs.IsEmpty() ? 1 : 0;
		Count += !Knowing.MetGalaxies.IsEmpty() ? 1 : 0;
		Count += !Knowing.PendingContactCivID.IsNone() ? 1 : 0;
		Count += !Knowing.FirstContactCivID.IsNone() ? 1 : 0;
		Count += (Knowing.FarLineUntilTick != 0) ? 1 : 0;
		// FKnownMark: ID, Tick.
		Count += (MarkSet(Knowing.KnownSystems, true) && MarkSet(Knowing.MetCivs, true) && MarkSet(Knowing.MetGalaxies, true)) ? 1 : 0;
		Count += (MarkSet(Knowing.KnownSystems, false) && MarkSet(Knowing.MetCivs, false) && MarkSet(Knowing.MetGalaxies, false)) ? 1 : 0;
		// FTransitRecord: eight members.
		if (!State.Transits.IsEmpty())
		{
			const FTransitRecord& Transit = State.Transits.Last();
			Count += Transit.Kind.IsValid() ? 1 : 0;
			Count += !Transit.FromGalaxyRow.IsNone() ? 1 : 0;
			Count += !Transit.ToGalaxyRow.IsNone() ? 1 : 0;
			Count += (Transit.DepartTick != 0) ? 1 : 0;
			Count += (Transit.ArriveTick != 0) ? 1 : 0;
			Count += !Transit.SourceCivID.IsNone() ? 1 : 0;
			Count += !Transit.ArchetypeRow.IsNone() ? 1 : 0;
			Count += !Transit.ResultCivID.IsNone() ? 1 : 0;
		}
		// FStarSystemState: GalaxyRow. FCivilizationState: GalaxyRow, OriginGalaxyRow.
		Count += (!State.Systems.IsEmpty() && !State.Systems.Last().GalaxyRow.IsNone()) ? 1 : 0;
		Count += (bCivs && !State.Civs.Last().GalaxyRow.IsNone()) ? 1 : 0;
		Count += (bCivs && !State.Civs[1].OriginGalaxyRow.IsNone()) ? 1 : 0;
		// FCosmicLogEntry: FocusCivID, bUntold.
		Count += (bLogs && !State.Log.Last().FocusCivID.IsNone() && !State.Chronicle.Last().FocusCivID.IsNone()) ? 1 : 0;
		Count += (bLogs && State.Log.Last().bUntold && State.Chronicle.Last().bUntold) ? 1 : 0;
		return Count;
	}

	bool SameMarks(const TArray<FKnownMark>& A, const TArray<FKnownMark>& B)
	{
		bool bSame = A.Num() == B.Num();
		for (int32 Index = 0; bSame && Index < A.Num(); ++Index)
		{
			bSame = A[Index].ID == B[Index].ID && A[Index].Tick == B[Index].Tick;
		}
		return bSame;
	}

	bool SameLogSubjects(const TArray<FCosmicLogEntry>& A, const TArray<FCosmicLogEntry>& B)
	{
		bool bSame = A.Num() == B.Num();
		for (int32 Index = 0; bSame && Index < A.Num(); ++Index)
		{
			bSame = A[Index].FocusCivID == B[Index].FocusCivID && A[Index].bUntold == B[Index].bUntold;
		}
		return bSame;
	}

	// The members reserved in v0.7, compared one by one. ComputeStateHash walks reflection, so it cannot see a member that
	// lost its UPROPERTY: such a member is neither saved nor hashed, and "same hash after load" still holds.
	bool SameReservedMembers(const FGalaxyState& A, const FGalaxyState& B)
	{
		bool bSame = A.TellRngState == B.TellRngState && A.NextCivSerial == B.NextCivSerial
			&& SameMarks(A.Knowing.KnownSystems, B.Knowing.KnownSystems)
			&& SameMarks(A.Knowing.MetCivs, B.Knowing.MetCivs)
			&& SameMarks(A.Knowing.MetGalaxies, B.Knowing.MetGalaxies)
			&& A.Knowing.PendingContactCivID == B.Knowing.PendingContactCivID
			&& A.Knowing.FirstContactCivID == B.Knowing.FirstContactCivID
			&& A.Knowing.FarLineUntilTick == B.Knowing.FarLineUntilTick
			&& SameLogSubjects(A.Log, B.Log) && SameLogSubjects(A.Chronicle, B.Chronicle)
			&& A.Transits.Num() == B.Transits.Num() && A.Systems.Num() == B.Systems.Num() && A.Civs.Num() == B.Civs.Num();
		for (int32 Index = 0; bSame && Index < A.Transits.Num(); ++Index)
		{
			const FTransitRecord& Left = A.Transits[Index];
			const FTransitRecord& Right = B.Transits[Index];
			bSame = Left.Kind == Right.Kind && Left.FromGalaxyRow == Right.FromGalaxyRow && Left.ToGalaxyRow == Right.ToGalaxyRow
				&& Left.DepartTick == Right.DepartTick && Left.ArriveTick == Right.ArriveTick && Left.SourceCivID == Right.SourceCivID
				&& Left.ArchetypeRow == Right.ArchetypeRow && Left.ResultCivID == Right.ResultCivID;
		}
		for (int32 Index = 0; bSame && Index < A.Systems.Num(); ++Index)
		{
			bSame = A.Systems[Index].GalaxyRow == B.Systems[Index].GalaxyRow;
		}
		for (int32 Index = 0; bSame && Index < A.Civs.Num(); ++Index)
		{
			bSame = A.Civs[Index].GalaxyRow == B.Civs[Index].GalaxyRow && A.Civs[Index].OriginGalaxyRow == B.Civs[Index].OriginGalaxyRow;
		}
		return bSame;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimDeterminismTest, "Cosmic_Ascension.Sim.Determinism", CosmicTests::Flags)
bool FCosmicSimDeterminismTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	const uint32 First = CosmicSimTests::HashAfter(*Sim, 42, 3000);
	const uint32 Second = CosmicSimTests::HashAfter(*Sim, 42, 3000);
	TestEqual(TEXT("Seed 42, 3,000 ticks, twice: same hash"), static_cast<int64>(Second), static_cast<int64>(First));
	TestEqual(TEXT("Tick counter after 3,000 ticks"), Sim->GetState().Tick, static_cast<int64>(3000));

	// The hash must react to each kind of field; GalaxyID and FText must stay out of it.
	FGalaxyState& State = Sim->GetMutableState();
	const uint32 Base = Sim->ComputeStateHash();

	float& Matter = State.Civs.Last().Stockpile.Matter;
	Matter += 1.f;
	TestTrue(TEXT("Hash sees a civ stockpile"), Sim->ComputeStateHash() != Base);
	Matter -= 1.f;

	float& Society = State.Civs[0].Research.FindChecked(FName(TEXT("Society")));
	const float OldSociety = Society;
	Society = 0.123f;
	TestTrue(TEXT("Hash sees a map value"), Sim->ComputeStateHash() != Base);
	Society = OldSociety;

	++State.Calling.Lives[0].ReignTicks[0];
	TestTrue(TEXT("Hash sees a life record array"), Sim->ComputeStateHash() != Base);
	--State.Calling.Lives[0].ReignTicks[0];

	FName& Owner = State.Systems.Last().OwnerCivID;
	const FName OldOwner = Owner;
	Owner = FName(TEXT("civ_xx"));
	TestTrue(TEXT("Hash sees a system name field"), Sim->ComputeStateHash() != Base);
	Owner = OldOwner;

	TestEqual(TEXT("Hash is back after undoing the changes"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Base));

	State.GalaxyID = FGuid::NewGuid();
	State.Civs[0].CivName = FText::AsCultureInvariant(TEXT("x"));
	TestEqual(TEXT("GalaxyID and FText stay out of the hash"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Base));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimNoNaNTest, "Cosmic_Ascension.Sim.NoNaN", CosmicTests::Flags)
bool FCosmicSimNoNaNTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	Sim->NewGalaxy(7, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(10000);
	TestEqual(TEXT("Non-finite floats in the state after 10,000 ticks"), CosmicSimTests::CountNonFiniteInStruct(FGalaxyState::StaticStruct(), &Sim->GetState()), 0);

	// The walker itself: it has to find a NaN planted deep in the state.
	FGalaxyState Poisoned = Sim->GetState();
	Poisoned.Civs.Last().Stockpile.Energy = std::numeric_limits<float>::quiet_NaN();
	TestEqual(TEXT("The walker sees a planted NaN"), CosmicSimTests::CountNonFiniteInStruct(FGalaxyState::StaticStruct(), &Poisoned), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimFirstLifeTest, "Cosmic_Ascension.Sim.FirstLife", CosmicTests::Flags)
bool FCosmicSimFirstLifeTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	// The Phase 1 prototype size: five systems.
	Sim->NewGalaxy(42, 5, 3);
	const FGalaxyState& State = Sim->GetState();
	const FName Sol(TEXT("sol"));

	const FCivArchetypeDef* Archetype = Sim->GetTables().FindCiv(Sim->GetRules().PlayerArchetypeRow);
	if (!TestNotNull(TEXT("DT_Civs holds the player archetype row (run Tools/make_data_tables.py)"), Archetype))
	{
		return false;
	}
	if (!TestEqual(TEXT("Systems"), State.Systems.Num(), 5) || !TestEqual(TEXT("Civs"), State.Civs.Num(), 3) || !TestEqual(TEXT("Lives"), State.Calling.Lives.Num(), 1))
	{
		return false;
	}

	const FStarSystemState& Home = State.Systems[0];
	const FCivilizationState& Civ = State.Civs[0];
	TestEqual(TEXT("Home system ID"), Home.SystemID, Sol);
	TestEqual(TEXT("Home system owner"), Home.OwnerCivID, Civ.CivID);
	TestEqual(TEXT("Home richness is ordinary"), Home.ResourceRichness, 1.f);
	TestTrue(TEXT("Home star class comes from the archetype"), Home.StarClass == Archetype->HomeStarClass);
	TestFalse(TEXT("Home system has a display name"), Home.DisplayName.IsEmpty());

	TestEqual(TEXT("Player civ archetype"), Civ.ArchetypeRow, Sim->GetRules().PlayerArchetypeRow);
	// The exit criterion names the row itself, not whatever DT_Rules points at.
	TestEqual(TEXT("Player civ archetype is arch_terran"), Civ.ArchetypeRow, FName(TEXT("arch_terran")));
	TestEqual(TEXT("Player civ home"), Civ.HomeSystemID, Sol);
	TestEqual(TEXT("Player civ owns one system"), Civ.OwnedSystems.Num(), 1);
	TestTrue(TEXT("Player civ is player ruled"), Civ.bIsPlayerRuled);
	TestFalse(TEXT("Player civ has a name"), Civ.CivName.IsEmpty());
	TestTrue(TEXT("Player civ traits come from the archetype"), Civ.Traits == Archetype->Traits);
	TestEqual(TEXT("Player civ openness comes from the archetype"), Civ.Ethics.Openness, Archetype->EthicsPreset.Openness);
	for (int32 Index = 1; Index < State.Civs.Num(); ++Index)
	{
		TestFalse(TEXT("Only the player civ is player ruled"), State.Civs[Index].bIsPlayerRuled);
		TestTrue(TEXT("AI civ has an AI profile"), State.Civs[Index].AIProfile.IsValid());
	}

	TestTrue(TEXT("Vessel is a ruler"), State.Player.Role == EPlayerRole::Ruler);
	TestEqual(TEXT("Vessel rules the player civ"), State.Player.RuledCivID, Civ.CivID);
	TestEqual(TEXT("Vessel is at home"), State.Player.CurrentSystemID, Sol);
	TestEqual(TEXT("First life index"), State.Calling.LifeIndex, 1);
	TestTrue(TEXT("Calling is embodied"), State.Calling.LifeState == ELifeState::Embodied);

	const FLifeRecord& Life = State.Calling.Lives[0];
	TestTrue(TEXT("Succession kind"), Life.SuccessionKind == CosmicSimTests::Tag(TEXT("Succession.Installed")));
	TestEqual(TEXT("Birth system"), Life.BirthSystemID, Sol);
	TestEqual(TEXT("Birth civ"), Life.BirthCivID, Civ.CivID);
	TestEqual(TEXT("Birth archetype row"), Life.BirthArchetypeRow, Civ.ArchetypeRow);
	TestTrue(TEXT("Reign arrays hold one open reign"),
		Life.RuledCivIDs.Num() == 1 && Life.ReignEndReasons.Num() == 1 && Life.ReignTicks.Num() == 1
		&& Life.RuledCivIDs[0] == Civ.CivID && !Life.ReignEndReasons[0].IsValid() && Life.ReignTicks[0] == 0);

	// The first line of the run calls the home system by name.
	if (TestTrue(TEXT("The log holds the first line"), State.Log.Num() >= 1))
	{
		const FCosmicLogEntry& First = State.Log[0];
		const FString Message = First.Message.ToString();
		TestEqual(TEXT("First line row"), First.SourceRow, CosmicRows::LogFirstLife);
		TestTrue(TEXT("First line category"), First.Category == CosmicSimTests::Tag(TEXT("Log.Calling")));
		TestEqual(TEXT("First line severity"), static_cast<int32>(First.Severity), 2);
		TestEqual(TEXT("First line tick"), First.Tick, static_cast<int64>(0));
		TestEqual(TEXT("First line focus"), First.FocusSystemID, Sol);
		TestTrue(TEXT("First line starts with the home system name"), Message.StartsWith(Home.DisplayName.ToString(), ESearchCase::CaseSensitive));
		TestFalse(TEXT("First line has no unfilled slot"), Message.Contains(TEXT("{")));

		const FLogTemplateDef* FirstRow = Sim->GetTables().FindLogTemplate(CosmicRows::LogFirstLife);
		if (TestNotNull(TEXT("DT_LogTemplates holds log_first_life"), FirstRow))
		{
			TestEqual(TEXT("log_first_life is a fixed line: exactly one variant"), FirstRow->Templates.Num(), 1);
		}
	}

	Sim->RunHeadless(100);
	TestEqual(TEXT("The open reign counts every tick"), Sim->GetState().Calling.Lives[0].ReignTicks[0], static_cast<int64>(100));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimPopulationBandTest, "Cosmic_Ascension.Sim.PopulationBand", CosmicTests::Flags)
bool FCosmicSimPopulationBandTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	Sim->NewGalaxy(42, 5, 3);
	const TArray<float>& Thresholds = Sim->GetRules().PopulationBandThresholds;
	if (!TestEqual(TEXT("PopulationBandThresholds has three entries"), Thresholds.Num(), 3))
	{
		return false;
	}

	FCivilizationState& Civ = Sim->GetMutableState().Civs[0];
	auto Band = [Sim, &Civ](EAscensionTier Tier, float Population)
	{
		Civ.Tier = Tier;
		Civ.Population = Population;
		return static_cast<int32>(Sim->GetPopulationBand(Civ.CivID));
	};

	TestEqual(TEXT("Tier 1 baseline"), Band(EAscensionTier::Tier1_Planetary, 1.f), static_cast<int32>(EPopulationBand::Billions));
	TestEqual(TEXT("Tier 2 baseline"), Band(EAscensionTier::Tier2_Stellar, 1.f), static_cast<int32>(EPopulationBand::HundredBillions));
	TestEqual(TEXT("Tier 3 baseline"), Band(EAscensionTier::Tier3_Causal, 1.f), static_cast<int32>(EPopulationBand::Trillions));
	TestEqual(TEXT("Tier 5 baseline"), Band(EAscensionTier::Tier5_EternalOrder, 1.f), static_cast<int32>(EPopulationBand::Countless));
	TestEqual(TEXT("Below the middle threshold: one band down"), Band(EAscensionTier::Tier1_Planetary, (Thresholds[0] + Thresholds[1]) * 0.5f), static_cast<int32>(EPopulationBand::Millions));
	TestEqual(TEXT("Below the lowest threshold: few"), Band(EAscensionTier::Tier3_Causal, Thresholds[0] * 0.5f), static_cast<int32>(EPopulationBand::Few));
	TestEqual(TEXT("Above the highest threshold: one band up"), Band(EAscensionTier::Tier1_Planetary, Thresholds[2] + 1.f), static_cast<int32>(EPopulationBand::HundredBillions));
	TestEqual(TEXT("One band up is capped at countless"), Band(EAscensionTier::Tier4_Transcendent, Thresholds[2] + 1.f), static_cast<int32>(EPopulationBand::Countless));
	TestEqual(TEXT("Tier 4 baseline"), Band(EAscensionTier::Tier4_Transcendent, 1.f), static_cast<int32>(EPopulationBand::Countless));
	TestEqual(TEXT("Exactly the lowest threshold: one band down, not few"), Band(EAscensionTier::Tier1_Planetary, Thresholds[0]), static_cast<int32>(EPopulationBand::Millions));
	TestEqual(TEXT("Exactly the middle threshold: baseline"), Band(EAscensionTier::Tier1_Planetary, Thresholds[1]), static_cast<int32>(EPopulationBand::Billions));
	TestEqual(TEXT("Exactly the highest threshold: baseline"), Band(EAscensionTier::Tier1_Planetary, Thresholds[2]), static_cast<int32>(EPopulationBand::Billions));
	TestEqual(TEXT("Unknown civ"), static_cast<int32>(Sim->GetPopulationBand(FName(TEXT("civ_none")))), static_cast<int32>(EPopulationBand::Few));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimFrameStepTest, "Cosmic_Ascension.Sim.FrameStep", CosmicTests::Flags)
bool FCosmicSimFrameStepTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	Sim->NewGalaxy(42, 5, 3);
	const FGalaxyState& State = Sim->GetState();

	Sim->Tick(5.f);
	TestEqual(TEXT("Paused: no tick"), State.Tick, static_cast<int64>(0));

	Sim->SetTimeScale(10.f);
	Sim->Tick(0.35f);
	TestEqual(TEXT("10x for 0.35 s: three ticks"), State.Tick, static_cast<int64>(3));

	Sim->SetTimeScale(50.f);
	TestEqual(TEXT("Time scale is capped at 10"), Sim->GetTimeScale(), 10.f);
	Sim->Tick(1.f);
	TestEqual(TEXT("At most ten ticks per frame"), State.Tick, static_cast<int64>(13));

	Sim->Tick(100.f);
	TestEqual(TEXT("A long frame still runs ten ticks"), State.Tick, static_cast<int64>(23));
	Sim->Tick(0.f);
	TestEqual(TEXT("The backlog carried over is ten ticks at most"), State.Tick, static_cast<int64>(33));
	Sim->Tick(0.f);
	TestEqual(TEXT("Nothing is left after the carried ticks"), State.Tick, static_cast<int64>(33));

	// One second at 1x is one tick: the part of a tick left by a frame carries over.
	Sim->SetTimeScale(1.f);
	Sim->Tick(0.25f);
	Sim->Tick(0.25f);
	Sim->Tick(0.25f);
	TestEqual(TEXT("1x: three quarter-second frames are not a tick yet"), State.Tick, static_cast<int64>(33));
	Sim->Tick(0.25f);
	TestEqual(TEXT("1x: the fourth frame completes one tick"), State.Tick, static_cast<int64>(34));

	// Ticks driven by frames, with the broadcast gate open, end in the same state as headless ticks. Far enough for the
	// first breakthrough lines: OnLogEntry then fires from inside a tick stage.
	Sim->SetTimeScale(10.f);
	for (int32 Frame = 0; Frame < 70; ++Frame)
	{
		Sim->Tick(1.f);
	}
	TestEqual(TEXT("Seventy full frames at 10x"), State.Tick, static_cast<int64>(734));
	TestTrue(TEXT("The framed run pushed log lines from inside a tick"), State.Log.Num() > 1);
	const uint32 FramedHash = Sim->ComputeStateHash();
	Sim->NewGalaxy(42, 5, 3);
	Sim->RunHeadless(734);
	TestEqual(TEXT("Frame-driven ticks equal headless ticks"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(FramedHash));

	Sim->SetTimeScale(0.f);
	Sim->Tick(5.f);
	TestEqual(TEXT("Paused again: no tick"), State.Tick, static_cast<int64>(734));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimGenerationRulesTest, "Cosmic_Ascension.Sim.GenerationRules", CosmicTests::Flags)
bool FCosmicSimGenerationRulesTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UPlayerRoleSubsystem* PlayerRole = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	const FGalaxyState& State = Sim->GetState();

	// Every case below injects its own tables; the tables of the running game come back at the end.
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };

	const FName PlayerRow = Sim->GetRules().PlayerArchetypeRow;
	const FName Unweighted(TEXT("arch_test_a"));
	const FName Light(TEXT("arch_test_b"));
	const FName Heavy(TEXT("arch_test_c"));

	// DT_Civs with the player's row (as many person names as asked) and three AI rows of weight 0, 1 and 3.
	// The rules and the log templates stay those of the running game.
	auto MakeTables = [&Original, PlayerRow, Unweighted, Light, Heavy](int32 PersonNames, bool bReverseRows = false)
	{
		UDataTable* Civs = NewObject<UDataTable>(GetTransientPackage());
		Civs->RowStruct = FCivArchetypeDef::StaticStruct();

		FCivArchetypeDef PlayerDef;
		PlayerDef.Weight = 0.f;
		for (int32 Index = 0; Index < PersonNames; ++Index)
		{
			PlayerDef.PersonNameBank.Add(FText::AsCultureInvariant(FString::Printf(TEXT("name %d"), Index)));
		}
		FCivArchetypeDef UnweightedDef;
		UnweightedDef.Weight = 0.f;
		FCivArchetypeDef LightDef;
		LightDef.Weight = 1.f;
		FCivArchetypeDef HeavyDef;
		HeavyDef.Weight = 3.f;
		if (bReverseRows)
		{
			Civs->AddRow(Heavy, HeavyDef);
			Civs->AddRow(Light, LightDef);
			Civs->AddRow(Unweighted, UnweightedDef);
			Civs->AddRow(PlayerRow, PlayerDef);
		}
		else
		{
			Civs->AddRow(PlayerRow, PlayerDef);
			Civs->AddRow(Unweighted, UnweightedDef);
			Civs->AddRow(Light, LightDef);
			Civs->AddRow(Heavy, HeavyDef);
		}

		FCosmicDataTables Tables = Original;
		Tables.Civs = Civs;
		return Tables;
	};

	// The weighted archetype draw: a row of weight 0 is never drawn, every weighted row is, and the seed decides who gets which.
	Sim->SetTables(MakeTables(3));
	// Counted over four more seeds: one galaxy of eleven AI civs misses the weight 1 row in 4% of the streams.
	int32 LightTotal = 0;
	int32 HeavyTotal = 0;
	for (int32 Seed = 43; Seed <= 46; ++Seed)
	{
		Sim->NewGalaxy(Seed, CosmicTests::BudgetSystems, CosmicTests::BudgetCivs);
		for (int32 Index = 1; Index < State.Civs.Num(); ++Index)
		{
			LightTotal += (State.Civs[Index].ArchetypeRow == Light) ? 1 : 0;
			HeavyTotal += (State.Civs[Index].ArchetypeRow == Heavy) ? 1 : 0;
		}
	}

	Sim->NewGalaxy(42, CosmicTests::BudgetSystems, CosmicTests::BudgetCivs);
	TArray<FName> Assigned;
	int32 LightCount = 0;
	int32 HeavyCount = 0;
	for (int32 Index = 1; Index < State.Civs.Num(); ++Index)
	{
		const FName Row = State.Civs[Index].ArchetypeRow;
		Assigned.Add(Row);
		LightCount += (Row == Light) ? 1 : 0;
		HeavyCount += (Row == Heavy) ? 1 : 0;
	}
	TestEqual(TEXT("The player civ keeps the player archetype row"), State.Civs[0].ArchetypeRow, PlayerRow);
	TestEqual(TEXT("AI civs draw weighted rows only"), LightCount + HeavyCount, CosmicTests::BudgetCivs - 1);
	TestTrue(TEXT("Every weighted row is drawn"), LightTotal + LightCount > 0 && HeavyTotal + HeavyCount > 0);
	const uint32 HashFirst = Sim->ComputeStateHash();

	Sim->NewGalaxy(42, CosmicTests::BudgetSystems, CosmicTests::BudgetCivs);
	bool bSameAssignment = State.Civs.Num() == Assigned.Num() + 1;
	for (int32 Index = 1; bSameAssignment && Index < State.Civs.Num(); ++Index)
	{
		bSameAssignment = State.Civs[Index].ArchetypeRow == Assigned[Index - 1];
	}
	TestTrue(TEXT("The same seed assigns the same archetypes"), bSameAssignment);
	TestEqual(TEXT("The same seed gives the same hash"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(HashFirst));

	// The draw walks the rows in lexical order, not in the table's own order (section 9.2).
	Sim->SetTables(MakeTables(3, true));
	Sim->NewGalaxy(42, CosmicTests::BudgetSystems, CosmicTests::BudgetCivs);
	TestEqual(TEXT("The row order of the table does not change the galaxy"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(HashFirst));

	// The vessel's name: inside the bank, and a new life in the same civ never repeats the name before it.
	const FName HomeSystemID = State.Systems[0].SystemID;
	const FName PlayerCivID = State.Civs[0].CivID;
	bool bNamesInBank = State.Calling.Lives.Num() == 1 && State.Calling.Lives[0].NameIndex >= 0 && State.Calling.Lives[0].NameIndex < 3;
	bool bNeverRepeated = true;
	for (int32 LifeNumber = 0; LifeNumber < 20; ++LifeNumber)
	{
		const int32 Previous = State.Calling.Lives.Last().NameIndex;
		PlayerRole->Install(HomeSystemID, PlayerCivID, EPlayerRole::Ruler);
		const int32 Drawn = State.Calling.Lives.Last().NameIndex;
		bNamesInBank &= Drawn >= 0 && Drawn < 3;
		bNeverRepeated &= Drawn != Previous;
	}
	TestTrue(TEXT("Every name index lies inside the three-name bank"), bNamesInBank);
	TestTrue(TEXT("A new life in the same civ never repeats the previous name"), bNeverRepeated);
	TestEqual(TEXT("Each Install opens one life"), State.Calling.Lives.Num(), 21);

	// A bank of one name: every life carries it.
	Sim->SetTables(MakeTables(1));
	Sim->NewGalaxy(42, 5, 3);
	PlayerRole->Install(State.Systems[0].SystemID, State.Civs[0].CivID, EPlayerRole::Ruler);
	TestTrue(TEXT("A one-name bank gives index 0 to every life"),
		State.Calling.Lives.Num() == 2 && State.Calling.Lives[0].NameIndex == 0 && State.Calling.Lives[1].NameIndex == 0);

	// A life knows where it was born, once, without a line, and the people it is born into is no first contact.
	TestTrue(TEXT("The first life wrote its line, so a line written by a later life would be seen"),
		State.Log.ContainsByPredicate([](const FCosmicLogEntry& Entry) { return Entry.SourceRow == CosmicRows::LogFirstLife; }));
	if (TestTrue(TEXT("The galaxy of the knowing checks has three peoples"), State.Civs.Num() >= 3))
	{
		const FKnowingState& Knowing = State.Knowing;
		TestEqual(TEXT("Lives born in one system write it once"), Knowing.KnownSystems.Num(), 1);
		TestTrue(TEXT("The known system is the birth system"), Knowing.KnownSystems.Num() == 1 && Knowing.KnownSystems[0].ID == State.Systems[0].SystemID);
		TestEqual(TEXT("Lives born into one people write it once"), Knowing.MetCivs.Num(), 1);
		TestTrue(TEXT("The met people is the birth people"), Knowing.MetCivs.Num() == 1 && Knowing.MetCivs[0].ID == State.Civs[0].CivID);
		TestTrue(TEXT("Install writes no first contact"), Knowing.FirstContactCivID.IsNone());

		// Some ticks on, a life born into another people: two more marks with the tick of the birth, no first contact and
		// no line.
		Sim->RunHeadless(5);
		const int32 LinesBefore = State.Log.Num();
		PlayerRole->Install(State.Civs[1].HomeSystemID, State.Civs[1].CivID, EPlayerRole::Ruler);
		TestEqual(TEXT("A life born elsewhere adds its birth system"), Knowing.KnownSystems.Num(), 2);
		TestTrue(TEXT("The added system is the new birth system"), Knowing.KnownSystems.Num() == 2 && Knowing.KnownSystems[1].ID == State.Civs[1].HomeSystemID);
		TestTrue(TEXT("The added system carries the tick of the birth"), Knowing.KnownSystems.Num() == 2 && Knowing.KnownSystems[1].Tick == State.Tick);
		TestEqual(TEXT("A life born into another people adds that people"), Knowing.MetCivs.Num(), 2);
		TestTrue(TEXT("The added people is the new birth people"), Knowing.MetCivs.Num() == 2 && Knowing.MetCivs[1].ID == State.Civs[1].CivID);
		TestTrue(TEXT("The added people carries the tick of the birth"), Knowing.MetCivs.Num() == 2 && Knowing.MetCivs[1].Tick == State.Tick);
		TestTrue(TEXT("Being born into another people is no first contact"), Knowing.FirstContactCivID.IsNone());
		TestEqual(TEXT("Install writes knowing without a line"), State.Log.Num(), LinesBefore);

		// A life without a birth system writes no system, and still its people.
		PlayerRole->Install(NAME_None, State.Civs[2].CivID, EPlayerRole::Wanderer);
		TestEqual(TEXT("A life without a birth system adds no system"), Knowing.KnownSystems.Num(), 2);
		TestEqual(TEXT("A life without a birth system still adds its people"), Knowing.MetCivs.Num(), 3);
	}

	// ---- The generation rules of v0.7 (section 7.4) ----
	using namespace CosmicSimTests;
	const FName HomeRow = Sim->GetRules().HomeGalaxyRow;
	const FCosmicRulesDef GameRules = Original.FindRules() ? *Original.FindRules() : FCosmicRulesDef();
	auto RulesTableOf = [](const FCosmicRulesDef& Row)
	{
		UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
		Table->RowStruct = FCosmicRulesDef::StaticStruct();
		Table->AddRow(CosmicRows::Rules, Row);
		return Table;
	};
	auto WithGalaxy = [&MakeTables](UDataTable* Galaxies, UDataTable* Transcendents = nullptr)
	{
		FCosmicDataTables Tables = MakeTables(3);
		Tables.Galaxies = Galaxies;
		Tables.Transcendents = Transcendents;
		return Tables;
	};

	// Counts of 0 are the home row's; without the table or the row they are the C++ defaults, and a table without the
	// row says so.
	Sim->SetTables(WithGalaxy(GalaxyTable(HomeRow, 17, 5, 5, 2, 6)));
	Sim->NewGalaxy(42, 0, 0);
	TestEqual(TEXT("A count of 0 systems is read from the home row"), State.Systems.Num(), 17);
	TestEqual(TEXT("A count of 0 peoples is read from the home row"), State.Civs.Num(), 5);
	Sim->SetTables(WithGalaxy(nullptr));
	Sim->NewGalaxy(42, 0, 0);
	TestEqual(TEXT("Without DT_Galaxies the count of systems is the C++ default"), State.Systems.Num(), FGalaxyDef().LivingWorlds);
	TestEqual(TEXT("Without DT_Galaxies the count of peoples is the C++ default"), State.Civs.Num(), FGalaxyDef().Peoples);
	AddExpectedErrorPlain(TEXT("DT_Galaxies has no row"), EAutomationExpectedErrorFlags::Contains, 1);
	Sim->SetTables(WithGalaxy(GalaxyTable(FName(TEXT("gal_test_other")), 17, 5, 5, 2, 6)));
	Sim->NewGalaxy(42, 0, 0);
	TestEqual(TEXT("A table without the home row gives the C++ default of systems"), State.Systems.Num(), FGalaxyDef().LivingWorlds);
	TestEqual(TEXT("A table without the home row gives the C++ default of peoples"), State.Civs.Num(), FGalaxyDef().Peoples);
	TestTrue(TEXT("The stamp of the systems is the knob's name even when the table lacks the row"),
		State.Systems.Num() > 0 && State.Systems.Last().GalaxyRow == HomeRow);
	TestTrue(TEXT("The stamp of the peoples is the knob's name even when the table lacks the row"),
		State.Civs.Num() > 0 && State.Civs.Last().GalaxyRow == HomeRow);
	Sim->NewGalaxy(42, 400, 3);
	TestEqual(TEXT("No upper bound on the number of systems in the simulation"), State.Systems.Num(), 400);
	Sim->NewGalaxy(42, 4, 9);
	TestEqual(TEXT("No more peoples than systems"), State.Civs.Num(), 4);

	// Knobs at values other than their C++ defaults (another home row, shares that leave G alone, sol at a quarter of the
	// radius): the generator reads them.
	{
		const FName TestHome(TEXT("gal_test_home"));
		UDataTable* Galaxies = GalaxyTable(TestHome, 20, 4, 20, 2, 10);
		Galaxies->FindRow<FGalaxyDef>(TestHome, TEXT("Sim.GenerationRules"))->HomeRadiusShare = 0.25f;
		FCosmicDataTables Tables = WithGalaxy(Galaxies);
		FCosmicRulesDef Knobs = GameRules;
		Knobs.HomeGalaxyRow = TestHome;
		Knobs.LivingWorldStarShares = { 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f };
		Tables.Rules = RulesTableOf(Knobs);
		Sim->SetTables(Tables);
		Sim->NewGalaxy(42, 0, 0);
		TestEqual(TEXT("The home row the knob names gives the number of systems"), State.Systems.Num(), 20);
		TestEqual(TEXT("The home row the knob names gives the number of peoples"), State.Civs.Num(), 4);
		bool bSystemsStamped = true;
		bool bOnlyG = true;
		for (const FStarSystemState& System : State.Systems)
		{
			bSystemsStamped &= System.GalaxyRow == TestHome;
			bOnlyG &= System.StarClass == Tag(TEXT("Star.Class.G"));
		}
		bool bCivsStamped = true;
		for (const FCivilizationState& Civ : State.Civs)
		{
			bCivsStamped &= Civ.GalaxyRow == TestHome;
		}
		TestTrue(TEXT("Every system carries the home row the knob names"), bSystemsStamped);
		TestTrue(TEXT("Every people carries the home row the knob names"), bCivsStamped);
		TestTrue(TEXT("sol lies at the share of the home row the knob names"),
			State.Systems.Num() > 0 && FMath::IsNearlyEqual(State.Systems[0].Position.X, 0.25 * 1000.0 * FMath::Sqrt(20.0), 1.0));
		TestTrue(TEXT("Shares that leave G alone give only G"), bOnlyG);
	}

	// Edges of the counts, and the draws a second people adds (section 7.4: one neighbour draw, five draws of archetype,
	// ethics and profile, two name draws, and one catalogue draw less).
	{
		Sim->SetTables(WithGalaxy(GalaxyTable(HomeRow, 17, 5, 17, 2, 6)));
		Sim->NewGalaxy(42, -1, -1);
		TestEqual(TEXT("A negative count of systems is read from the home row"), State.Systems.Num(), 17);
		TestEqual(TEXT("A negative count of peoples is read from the home row"), State.Civs.Num(), 5);
		Sim->NewGalaxy(42, 1, 1);
		TestEqual(TEXT("A galaxy asked for one system has one system"), State.Systems.Num(), 1);
		TestEqual(TEXT("A galaxy asked for one system has one people"), State.Civs.Num(), 1);
		Sim->NewGalaxy(42, 12, 1);
		FRandomStream Probe(Sim->GetRng().GetCurrentSeed());
		for (int32 Draw = 0; Draw < 7; ++Draw)
		{
			Probe.FRand();
		}
		Sim->NewGalaxy(42, 12, 2);
		TestEqual(TEXT("A second people makes exactly seven draws more"), Sim->GetRng().GetCurrentSeed(), Probe.GetCurrentSeed());
		Sim->NewGalaxy(42, 12, 12);
		int32 Owned = 0;
		for (const FStarSystemState& System : State.Systems)
		{
			Owned += System.OwnerCivID.IsNone() ? 0 : 1;
		}
		TestEqual(TEXT("As many peoples as systems: every system is a home"), Owned, 12);
	}

	// IDs without padding, from NextCivSerial.
	Sim->SetTables(WithGalaxy(GalaxyTable(HomeRow, 12, 11, 12, 2, 6)));
	Sim->NewGalaxy(42, 0, 0);
	{
		bool bSystemIDs = State.Systems[0].SystemID == FName(TEXT("sol"));
		for (int32 Index = 1; Index < State.Systems.Num(); ++Index)
		{
			bSystemIDs &= State.Systems[Index].SystemID.ToString().Equals(FString::Printf(TEXT("sys_%d"), Index), ESearchCase::CaseSensitive);
		}
		bool bCivIDs = true;
		for (int32 Index = 0; Index < State.Civs.Num(); ++Index)
		{
			bCivIDs &= State.Civs[Index].CivID.ToString().Equals(FString::Printf(TEXT("civ_%d"), Index), ESearchCase::CaseSensitive);
		}
		TestTrue(TEXT("Systems are sol and sys_<n> without padding"), bSystemIDs);
		TestTrue(TEXT("Civilizations are civ_<n> without padding"), bCivIDs);
		TestEqual(TEXT("NextCivSerial has handed out one number per civilization"), State.NextCivSerial, State.Civs.Num());
	}

	// Names without replacement. Twelve star names for eleven peoples and twelve pairs for the one unowned system:
	// no name twice. With fewer names than takers the rest stay empty, never repeated.
	{
		TSet<FString> Names;
		bool bAllNamed = true;
		for (const FStarSystemState& System : State.Systems)
		{
			bAllNamed &= !System.DisplayName.IsEmpty();
			Names.Add(System.DisplayName.ToString());
		}
		TestTrue(TEXT("Every system has a name"), bAllNamed);
		TestEqual(TEXT("No system name twice"), Names.Num(), State.Systems.Num());
		Sim->SetTables(WithGalaxy(GalaxyTable(HomeRow, 12, 6, 3, 1, 2)));
		Sim->NewGalaxy(42, 0, 0);
		int32 Empty = 0;
		TSet<FString> Short;
		for (const FStarSystemState& System : State.Systems)
		{
			Empty += System.DisplayName.IsEmpty() ? 1 : 0;
			if (!System.DisplayName.IsEmpty())
			{
				Short.Add(System.DisplayName.ToString());
			}
		}
		// Six homes: the player's from its own bank (none here), five from three star names; six unowned from two pairs.
		TestEqual(TEXT("A short pool leaves the takers after it without a name"), Empty, 12 - 3 - 2);
		TestEqual(TEXT("A short pool never repeats a name"), Short.Num(), 3 + 2);
	}

	// A people's name calls its own home, and a home name from the archetype's bank goes to that people alone.
	{
		FCosmicDataTables Tables = WithGalaxy(GalaxyTable(HomeRow, 20, 8, 20, 3, 5));
		UDataTable* Civs = NewObject<UDataTable>(GetTransientPackage());
		Civs->RowStruct = FCivArchetypeDef::StaticStruct();
		FCivArchetypeDef PlayerDef;
		PlayerDef.Weight = 0.f;
		PlayerDef.HomeSystemNames.Add(FText::AsCultureInvariant(TEXT("home of the player")));
		PlayerDef.HomeStarClass = Tag(TEXT("Star.Class.G"));
		FCivArchetypeDef Named;
		Named.Weight = 1.f;
		Named.NameBank.Add(FText::AsCultureInvariant(TEXT("{Star} league")));
		Civs->AddRow(PlayerRow, PlayerDef);
		Civs->AddRow(Light, Named);
		Tables.Civs = Civs;
		Sim->SetTables(Tables);
		Sim->NewGalaxy(42, 0, 0);
		const FStarSystemState* PlayerHome = State.Systems.FindByPredicate([&State](const FStarSystemState& System) { return System.SystemID == State.Civs[0].HomeSystemID; });
		TestTrue(TEXT("A home name from the archetype's bank goes to its people"), PlayerHome && PlayerHome->DisplayName.ToString() == TEXT("home of the player"));
		TestTrue(TEXT("A fixed star class replaces the drawn one on that home"), PlayerHome && PlayerHome->StarClass == Tag(TEXT("Star.Class.G")));
		bool bCallsHome = State.Civs.Num() == 8;
		for (int32 Index = 1; Index < State.Civs.Num(); ++Index)
		{
			const FStarSystemState* Home = State.Systems.FindByPredicate([&](const FStarSystemState& System) { return System.SystemID == State.Civs[Index].HomeSystemID; });
			bCallsHome &= Home && !Home->DisplayName.IsEmpty() && State.Civs[Index].CivName.ToString() == Home->DisplayName.ToString() + TEXT(" league");
		}
		TestTrue(TEXT("Every AI people is named after its own home"), bCallsHome);
	}

	// The first AI people lives one lane from sol, in every galaxy of two peoples or more.
	{
		Sim->SetTables(WithGalaxy(GalaxyTable(HomeRow, 30, 6, 30, 4, 8)));
		bool bNeighbour = true;
		for (int32 Seed = 1; Seed <= 30; ++Seed)
		{
			Sim->NewGalaxy(Seed, 0, 0);
			bNeighbour &= State.Civs.Num() >= 2 && State.Systems[0].Lanes.Contains(State.Civs[1].HomeSystemID);
		}
		TestTrue(TEXT("The first AI people's home is a lane neighbour of sol"), bNeighbour);
	}

	// The starting cores (section 5.7), against the rule walked by the test itself.
	{
		// Every core in roster order: a candidate among the systems no earlier core took, or without a candidate the
		// farthest of those systems from sol. Counts the cores that fell back.
		auto CoresHoldTheRule = [&State](int32 MinLanes, int32& FellBack)
		{
			const TArray<int32> Distance = LanesFrom(State, 0);
			TSet<FName> Taken;
			bool bHolds = !State.Transcendents.IsEmpty();
			for (const FTranscendentState& Transcendent : State.Transcendents)
			{
				if (Transcendent.InfluencedSystems.Num() != 1)
				{
					bHolds = false;
					continue;
				}
				const FName Core = Transcendent.InfluencedSystems[0];
				int32 CoreIndex = INDEX_NONE;
				TArray<int32> Candidates;
				int32 Farthest = INDEX_NONE;
				for (int32 Index = 0; Index < State.Systems.Num(); ++Index)
				{
					const FStarSystemState& System = State.Systems[Index];
					CoreIndex = (System.SystemID == Core) ? Index : CoreIndex;
					if (!System.OwnerCivID.IsNone() || Taken.Contains(System.SystemID))
					{
						continue;
					}
					bool bNextToAIHome = false;
					for (int32 CivIndex = 1; CivIndex < State.Civs.Num(); ++CivIndex)
					{
						bNextToAIHome |= System.Lanes.Contains(State.Civs[CivIndex].HomeSystemID);
					}
					if (Distance[Index] >= MinLanes && bNextToAIHome)
					{
						Candidates.Add(Index);
					}
					if (Farthest == INDEX_NONE || Distance[Index] > Distance[Farthest]
						|| (Distance[Index] == Distance[Farthest] && FNameLexicalLess()(System.SystemID, State.Systems[Farthest].SystemID)))
					{
						Farthest = Index;
					}
				}
				FellBack += Candidates.IsEmpty() ? 1 : 0;
				bHolds &= CoreIndex != INDEX_NONE && (Candidates.IsEmpty() ? CoreIndex == Farthest : Candidates.Contains(CoreIndex));
				Taken.Add(Core);
			}
			return bHolds;
		};
		// The number of different systems in all spheres.
		auto CoreSystems = [&State]()
		{
			TSet<FName> Cores;
			for (const FTranscendentState& Transcendent : State.Transcendents)
			{
				for (const FName System : Transcendent.InfluencedSystems)
				{
					Cores.Add(System);
				}
			}
			return Cores.Num();
		};
		auto RulesWith = [&WithGalaxy, &RulesTableOf, &GameRules, HomeRow](int32 Places, int32 MinLanes, bool bCores, int32 Extra = 0)
		{
			FCosmicDataTables Tables = WithGalaxy(GalaxyTable(HomeRow, 40, 8, 40, 5, 8), TranscendentTable(bCores, Extra));
			FCosmicRulesDef RulesRow = GameRules;
			RulesRow.FallenTranscendentCount = Places;
			RulesRow.FallenCoreMinLanesFromHome = MinLanes;
			Tables.Rules = RulesTableOf(RulesRow);
			return Tables;
		};
		bool bRule = true;
		int32 FellBack = 0;
		Sim->SetTables(RulesWith(1, 3, true));
		for (int32 Seed = 1; Seed <= 30; ++Seed)
		{
			Sim->NewGalaxy(Seed, 0, 0);
			bRule &= CoresHoldTheRule(3, FellBack);
		}
		TestTrue(TEXT("The core is a candidate of the rule in every galaxy"), bRule);
		TestTrue(TEXT("With three lanes the candidates were there in some galaxy"), FellBack < 30);
		bRule = true;
		FellBack = 0;
		Sim->SetTables(RulesWith(1, 99, true));
		for (int32 Seed = 1; Seed <= 30; ++Seed)
		{
			Sim->NewGalaxy(Seed, 0, 0);
			bRule &= CoresHoldTheRule(99, FellBack);
		}
		TestTrue(TEXT("Without a candidate the core is the unowned system farthest from sol"), bRule);
		TestEqual(TEXT("Without a candidate every core falls back"), FellBack, 30);
		Sim->SetTables(RulesWith(1, 3, false));
		Sim->NewGalaxy(3, 0, 0);
		TestEqual(TEXT("A row without a core still gets its instance"), State.Transcendents.Num(), 1);
		TestEqual(TEXT("A row without a core starts with an empty sphere"), CoreSystems(), 0);
		for (const int32 MinLanes : { 3, 99 })
		{
			Sim->SetTables(RulesWith(3, MinLanes, true, 2));
			Sim->NewGalaxy(3, 0, 0);
			int32 Unused = 0;
			TestEqual(*FString::Printf(TEXT("Three fallen get three instances (%d lanes)"), MinLanes), State.Transcendents.Num(), 3);
			TestEqual(*FString::Printf(TEXT("Three fallen get three different cores (%d lanes)"), MinLanes), CoreSystems(), 3);
			TestTrue(*FString::Printf(TEXT("Each of the three cores holds the rule among the systems left to it (%d lanes)"), MinLanes), CoresHoldTheRule(MinLanes, Unused));
		}
		Sim->SetTables(RulesWith(1, 3, true));
		Sim->NewGalaxy(3, 6, 6);
		TestEqual(TEXT("Without an unowned system the fallen still gets its instance"), State.Transcendents.Num(), 1);
		TestEqual(TEXT("Without an unowned system the sphere starts empty"), CoreSystems(), 0);
	}

	// The number of draws follows the seed and the two counts, never the tables: other weights, other banks, a home row
	// with or without pools, transcendents or none, leave every system and every home where it was.
	{
		auto Snapshot = [&State, Sim]()
		{
			TArray<FString> Shape;
			for (const FStarSystemState& System : State.Systems)
			{
				FString Lanes;
				for (const FName Lane : System.Lanes)
				{
					Lanes += Lane.ToString() + TEXT(",");
				}
				Shape.Add(FString::Printf(TEXT("%s %.6f %.6f %s"), *System.SystemID.ToString(), System.Position.X, System.Position.Y, *Lanes));
			}
			for (const FCivilizationState& Civ : State.Civs)
			{
				Shape.Add(Civ.CivID.ToString() + TEXT(" ") + Civ.HomeSystemID.ToString());
			}
			Shape.Add(FString::Printf(TEXT("stream %d"), Sim->GetRng().GetCurrentSeed()));
			return Shape;
		};
		Sim->SetTables(WithGalaxy(GalaxyTable(HomeRow, 40, 8, 40, 5, 8), TranscendentTable(true)));
		Sim->NewGalaxy(42, 0, 0);
		const TArray<FString> Before = Snapshot();
		const int32 StreamBefore = Sim->GetRng().GetCurrentSeed();

		// Two other sets of tables, neither with name pools nor transcendents. In the first no row is weighted, so every
		// AI people falls back to arch_default and its ethics are used; in the second one weighted row fixes the profile
		// and the home name and has a name pattern. Each draw that is used in one set is thrown away in another.
		auto CivTable = [&PlayerRow](bool bWeighted, FName AIRow)
		{
			UDataTable* Civs = NewObject<UDataTable>(GetTransientPackage());
			Civs->RowStruct = FCivArchetypeDef::StaticStruct();
			FCivArchetypeDef PlayerDef;
			PlayerDef.Weight = 0.f;
			PlayerDef.NameBank.Add(FText::AsCultureInvariant(TEXT("the player's people")));
			FCivArchetypeDef Fixed;
			Fixed.Weight = bWeighted ? 7.f : 0.f;
			Fixed.AIProfile = Tag(TEXT("AI.Profile.Scholarly"));
			Fixed.HomeSystemNames.Add(FText::AsCultureInvariant(TEXT("fixed home")));
			Fixed.NameBank.Add(FText::AsCultureInvariant(TEXT("{Star} folk")));
			Civs->AddRow(PlayerRow, PlayerDef);
			Civs->AddRow(AIRow, Fixed);
			return Civs;
		};
		FCosmicDataTables Fallback = WithGalaxy(GalaxyTable(HomeRow, 40, 8, 0, 0, 0), nullptr);
		Fallback.Civs = CivTable(false, Heavy);
		Sim->SetTables(Fallback);
		Sim->NewGalaxy(42, 0, 0);
		TestTrue(TEXT("Without a weighted row the AI peoples are arch_default"), State.Civs.Num() == 8 && State.Civs[1].ArchetypeRow == CosmicRows::ArchDefault);
		TestEqual(TEXT("Tables whose AI peoples fall back to arch_default give the same systems, lanes and homes"), FString::Join(Snapshot(), TEXT("\n")), FString::Join(Before, TEXT("\n")));
		FCosmicDataTables Fixed = WithGalaxy(GalaxyTable(HomeRow, 40, 8, 0, 0, 0), nullptr);
		Fixed.Civs = CivTable(true, Heavy);
		Sim->SetTables(Fixed);
		Sim->NewGalaxy(42, 0, 0);
		TestTrue(TEXT("The weighted row fixes the profile"), State.Civs.Num() == 8 && State.Civs[1].AIProfile == Tag(TEXT("AI.Profile.Scholarly")));
		TestEqual(TEXT("Tables with fixed profiles, home names and patterns give the same systems, lanes and homes"), FString::Join(Snapshot(), TEXT("\n")), FString::Join(Before, TEXT("\n")));

		// The draws of the stream follow the counts: a galaxy of other counts stands elsewhere.
		Sim->NewGalaxy(42, 40, 7);
		TestTrue(TEXT("Another number of peoples moves the stream"), Sim->GetRng().GetCurrentSeed() != StreamBefore);
	}

	// The two guards of NewGalaxy, each with one error line: a share table of another length (the C++ shares are used)
	// and a place of sol outside 0..1 (the C++ share is used). The short table is the test's own: read as it is, padded
	// or merged with the C++ shares it gives only G, which the C++ shares do not.
	{
		AddExpectedErrorPlain(TEXT("LivingWorldStarShares has 6 entries"), EAutomationExpectedErrorFlags::Contains, 1);
		AddExpectedErrorPlain(TEXT("HomeRadiusShare 1.5"), EAutomationExpectedErrorFlags::Contains, 1);
		UDataTable* Galaxies = GalaxyTable(HomeRow, 20, 4, 20, 2, 10);
		FGalaxyDef* HomeOfTheTest = Galaxies->FindRow<FGalaxyDef>(HomeRow, TEXT("Sim.GenerationRules"));
		HomeOfTheTest->HomeRadiusShare = 1.5f;
		FCosmicDataTables Tables = WithGalaxy(Galaxies);
		FCosmicRulesDef Shortened = GameRules;
		Shortened.LivingWorldStarShares = { 0.f, 0.f, 0.f, 0.f, 1.f, 1.f };
		Tables.Rules = RulesTableOf(Shortened);
		Sim->SetTables(Tables);
		Sim->NewGalaxy(42, 0, 0);
		TArray<FGameplayTag> Classes;
		for (const FStarSystemState& System : State.Systems)
		{
			Classes.Add(System.StarClass);
		}
		const double Radius = 1000.0 * FMath::Sqrt(20.0);
		TestEqual(TEXT("The galaxy of the two guards is generated"), State.Systems.Num(), 20);
		TestTrue(TEXT("A place of sol outside the disc is read as the C++ share"),
			State.Systems.Num() > 0 && FMath::IsNearlyEqual(State.Systems[0].Position.X, static_cast<double>(FGalaxyDef().HomeRadiusShare) * Radius, 1.0));

		// The same seed under the C++ shares given outright gives the same classes: the guard used that table.
		FCosmicRulesDef Explicit = GameRules;
		Explicit.LivingWorldStarShares = FCosmicRulesDef().LivingWorldStarShares;
		Tables.Rules = RulesTableOf(Explicit);
		HomeOfTheTest->HomeRadiusShare = FGalaxyDef().HomeRadiusShare;
		Sim->SetTables(Tables);
		Sim->NewGalaxy(42, 0, 0);
		TArray<FGameplayTag> ExplicitClasses;
		for (const FStarSystemState& System : State.Systems)
		{
			ExplicitClasses.Add(System.StarClass);
		}
		TestTrue(TEXT("A share table of another length is read as the C++ shares"), Classes == ExplicitClasses);
		TestTrue(TEXT("The C++ shares give a class other than G (the control of the short table)"),
			ExplicitClasses.ContainsByPredicate([](const FGameplayTag& Class) { return Class != FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.G"))); }));
	}

	// A galaxy of the Early Access size, ten times, timed once for section 9.4 (budget: below 50 ms each).
	{
		Sim->SetTables(Original);
		const double Start = FPlatformTime::Seconds();
		for (int32 Seed = 1; Seed <= 10; ++Seed)
		{
			Sim->NewGalaxy(Seed, CosmicTests::BudgetSystems, CosmicTests::BudgetCivs);
		}
		const double Each = (FPlatformTime::Seconds() - Start) * 100.0;
		AddInfo(FString::Printf(TEXT("NewGalaxy at %d systems and %d peoples: %.1f ms each"), CosmicTests::BudgetSystems, CosmicTests::BudgetCivs, Each));
		TestEqual(TEXT("The Early Access size is generated as asked"), State.Systems.Num(), CosmicTests::BudgetSystems);
	}

	// An injected rules row is read, and without a table the rules go back to the C++ defaults.
	UDataTable* MarkedRules = NewObject<UDataTable>(GetTransientPackage());
	MarkedRules->RowStruct = FCosmicRulesDef::StaticStruct();
	FCosmicRulesDef Marked;
	Marked.MaxNotablesPerCiv = FCosmicRulesDef().MaxNotablesPerCiv + 1;
	MarkedRules->AddRow(CosmicRows::Rules, Marked);
	FCosmicDataTables OnlyRules;
	OnlyRules.Rules = MarkedRules;
	Sim->SetTables(OnlyRules);
	TestEqual(TEXT("An injected rules row is read"), Sim->GetRules().MaxNotablesPerCiv, Marked.MaxNotablesPerCiv);

	// No tables at all: the galaxy is still generated, on the C++ defaults and without a log line.
	Sim->SetTables(FCosmicDataTables());
	Sim->NewGalaxy(42, 5, 3);
	TestEqual(TEXT("Without tables the rules are the C++ defaults"), Sim->GetRules().MaxNotablesPerCiv, FCosmicRulesDef().MaxNotablesPerCiv);
	TestTrue(TEXT("Without tables the galaxy is still generated"), State.Systems.Num() == 5 && State.Civs.Num() == 3 && State.Calling.Lives.Num() == 1);
	TestTrue(TEXT("Without tables nothing is logged"), State.Log.IsEmpty());
	TestTrue(TEXT("Without tables AI civs fall back to arch_default"),
		State.Civs.Num() == 3 && State.Civs[1].ArchetypeRow == CosmicRows::ArchDefault && State.Civs[2].ArchetypeRow == CosmicRows::ArchDefault);
	TestTrue(TEXT("Without tables no name is drawn"), State.Calling.Lives.Num() == 1 && State.Calling.Lives[0].NameIndex == 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimStateIndexTest, "Cosmic_Ascension.Sim.StateIndex", CosmicTests::Flags)
bool FCosmicSimStateIndexTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	// The one error line of the stale lookup below.
	AddExpectedError(TEXT("State index is stale for civ"), EAutomationExpectedErrorFlags::Contains, 1);

	// Every ID of the live state resolves to its own element.
	auto AllResolve = [Sim]()
	{
		const FGalaxyState& State = Sim->GetState();
		const FCosmicStateIndex& Index = Sim->GetStateIndex();
		bool bAll = true;
		for (int32 Civ = 0; Civ < State.Civs.Num(); ++Civ)
		{
			bAll &= Index.FindCiv(State, State.Civs[Civ].CivID) == Civ;
		}
		for (int32 System = 0; System < State.Systems.Num(); ++System)
		{
			bAll &= Index.FindSystem(State, State.Systems[System].SystemID) == System;
		}
		return bAll;
	};

	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	TestTrue(TEXT("After NewGalaxy every ID resolves to its own element"), AllResolve());
	TestEqual(TEXT("NAME_None is no civ"), Index.FindCiv(Sim->GetState(), NAME_None), static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("NAME_None is no system"), Index.FindSystem(Sim->GetState(), NAME_None), static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("An unknown civ is not found"), Index.FindCiv(Sim->GetState(), FName(TEXT("civ_none"))), static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("An unknown system is not found"), Index.FindSystem(Sim->GetState(), FName(TEXT("sys_none"))), static_cast<int32>(INDEX_NONE));

	// A state of another size put in place by LoadState.
	const FGalaxyState Larger = Sim->GetState();
	Sim->NewGalaxy(43, 5, 3);
	TestTrue(TEXT("After a smaller NewGalaxy every ID resolves"), AllResolve());
	Sim->LoadState(Larger);
	TestTrue(TEXT("After LoadState every ID resolves to its own element"),
		Sim->GetState().Systems.Num() == Larger.Systems.Num() && Sim->GetState().Civs.Num() == Larger.Civs.Num() && AllResolve());

	// An array changed by hand: the index is stale, says so once and never answers with another element.
	FGalaxyState& Mutable = Sim->GetMutableState();
	const FName Shifted = Mutable.Civs[2].CivID;
	Mutable.Civs.RemoveAt(1);
	TestEqual(TEXT("A stale index answers INDEX_NONE, not a wrong element"), Index.FindCiv(Mutable, Shifted), static_cast<int32>(INDEX_NONE));
	Sim->RebuildStateIndex();
	TestEqual(TEXT("After RebuildStateIndex the shifted civ is found where it is"), Index.FindCiv(Mutable, Shifted), 1);
	TestTrue(TEXT("After RebuildStateIndex every ID resolves"), AllResolve());

	// The lane walk (CosmicState::LaneDistancesFrom) over a topology the test makes: the chain 0-1-...-9, 4-10, 10-11.
	Sim->NewGalaxy(42, 14, 3);
	FGalaxyState& Wired = Sim->GetMutableState();
	if (TestTrue(TEXT("The bench galaxy can be wired"), CosmicTests::WireChain(Wired)))
	{
		const TArray<int32> FromZero = CosmicState::LaneDistancesFrom(Wired, Index, 0);
		TestEqual(TEXT("The walk has one entry per system"), FromZero.Num(), Wired.Systems.Num());
		TestEqual(TEXT("A system is 0 lanes from itself"), FromZero.IsValidIndex(0) ? FromZero[0] : -2, 0);
		TestEqual(TEXT("Along the chain a lane is a step"), FromZero.IsValidIndex(9) ? FromZero[9] : -2, 9);
		TestEqual(TEXT("A branch counts from where it leaves the chain"), FromZero.IsValidIndex(11) ? FromZero[11] : -2, 6);
		TestEqual(TEXT("A system with no lane cannot be reached"), FromZero.IsValidIndex(12) ? FromZero[12] : -2, static_cast<int32>(INDEX_NONE));
		const TArray<int32> Near = CosmicState::LaneDistancesFrom(Wired, Index, 0, 3);
		TestEqual(TEXT("MaxLanes keeps a system at that many lanes"), Near.IsValidIndex(3) ? Near[3] : -2, 3);
		TestEqual(TEXT("MaxLanes leaves a farther system out"), Near.IsValidIndex(4) ? Near[4] : -2, static_cast<int32>(INDEX_NONE));
		const TArray<int32> Here = CosmicState::LaneDistancesFrom(Wired, Index, 0, 0);
		TestEqual(TEXT("MaxLanes 0 keeps the system itself"), Here.IsValidIndex(0) ? Here[0] : -2, 0);
		TestEqual(TEXT("MaxLanes 0 reaches no neighbour"), Here.IsValidIndex(1) ? Here[1] : -2, static_cast<int32>(INDEX_NONE));
		const TArray<int32> Nowhere = CosmicState::LaneDistancesFrom(Wired, Index, 0, -1);
		TestEqual(TEXT("A negative MaxLanes still gives one entry per system"), Nowhere.Num(), Wired.Systems.Num());
		TestFalse(TEXT("A negative MaxLanes reaches nothing, not even the system itself"), Nowhere.Contains(0));
		// A lane into another galaxy is not followed, and a lane to an ID the state does not hold is skipped.
		Wired.Systems[5].GalaxyRow = FName(TEXT("gal_test_elsewhere"));
		Wired.Systems[0].Lanes.Add(FName(TEXT("sys_nowhere")));
		const TArray<int32> Split = CosmicState::LaneDistancesFrom(Wired, Index, 0);
		TestEqual(TEXT("A system of another GalaxyRow is not reached"), Split.IsValidIndex(5) ? Split[5] : -2, static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("Nor is what lies behind it"), Split.IsValidIndex(6) ? Split[6] : -2, static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("The walk still reaches the near side"), Split.IsValidIndex(4) ? Split[4] : -2, 4);
		const TArray<int32> Outside = CosmicState::LaneDistancesFrom(Wired, Index, Wired.Systems.Num());
		TestTrue(TEXT("An index out of range reaches nothing"), Outside.Num() == Wired.Systems.Num() && !Outside.Contains(0));
	}

	// Leave a consistent galaxy behind.
	Sim->NewGalaxy(42, 5, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSeedDeriveTest, "Cosmic_Ascension.Sim.SeedDerive", CosmicTests::Flags)
bool FCosmicSimSeedDeriveTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}

	// The reference values of section 9.2: the first golden values of the project. CosmicSeed.h checks the same rows at
	// compile time; here they are evaluated at run time, the names going through the lower-case ANSI conversion.
	struct FReferenceRow
	{
		int32 Seed;
		const TCHAR* Name;
		uint32 Purpose;
		uint32 Index;
		uint32 Value;
	};
	const FReferenceRow Rows[] = {
		{ 42, TEXT(""), CosmicSeed::Purpose::Tell, 0, 272114134u },
		{ 42, TEXT("gal_andromeda"), CosmicSeed::Purpose::Summary, 0, 569404729u },
		{ 7, TEXT("sol"), CosmicSeed::Purpose::Life, 3, 2833998181u },
		{ -1, TEXT("gal_lmc"), CosmicSeed::Purpose::Depart, 2, 668976756u },
		{ 0, TEXT(""), CosmicSeed::Purpose::Tell, 0, 4196651879u },
	};
	for (const FReferenceRow& Row : Rows)
	{
		TestEqual(*FString::Printf(TEXT("Derive(%d, '%s', %u, %u)"), Row.Seed, Row.Name, Row.Purpose, Row.Index),
			static_cast<int64>(CosmicSimTests::DeriveLowerCased(Row.Seed, Row.Name, Row.Purpose, Row.Index)), static_cast<int64>(Row.Value));
	}
	TestEqual(TEXT("Index defaults to 0"), static_cast<int64>(CosmicSeed::Derive(42, "", CosmicSeed::Purpose::Tell)), static_cast<int64>(272114134u));
	TestEqual(TEXT("A name in another casing gives the same value once the caller lower-cases it"),
		static_cast<int64>(CosmicSimTests::DeriveLowerCased(7, TEXT("SOL"), CosmicSeed::Purpose::Life, 3)), static_cast<int64>(2833998181u));
	TestTrue(TEXT("Derive itself folds every byte in as it is"), CosmicSeed::Derive(7, "SOL", CosmicSeed::Purpose::Life, 3) != 2833998181u);

	// Purpose numbers are fixed: never renumbered, new ones appended.
	const uint32 Purposes[] = {
		CosmicSeed::Purpose::Tell, CosmicSeed::Purpose::Summary, CosmicSeed::Purpose::Name, CosmicSeed::Purpose::Life,
		CosmicSeed::Purpose::Backdrop, CosmicSeed::Purpose::Scenery, CosmicSeed::Purpose::Depart, CosmicSeed::Purpose::Archetype,
		CosmicSeed::Purpose::EventSlot
	};
	bool bPurposesFixed = true;
	for (int32 Index = 0; Index < static_cast<int32>(UE_ARRAY_COUNT(Purposes)); ++Index)
	{
		bPurposesFixed &= Purposes[Index] == static_cast<uint32>(Index + 1);
	}
	TestTrue(TEXT("The nine purposes are numbered 1 to 9 in their fixed order"), bPurposesFixed);

	// The helper for a caller that holds an FName lower-cases for it, whatever casing the name was registered in.
	TestEqual(TEXT("DeriveForName gives the reference value of the lower-case name"),
		static_cast<int64>(CosmicSeed::DeriveForName(42, FName(TEXT("gal_andromeda")), CosmicSeed::Purpose::Summary)), static_cast<int64>(569404729u));
	TestEqual(TEXT("DeriveForName does not depend on the casing of the FName"),
		static_cast<int64>(CosmicSeed::DeriveForName(7, FName(TEXT("SOL")), CosmicSeed::Purpose::Life, 3)), static_cast<int64>(2833998181u));
	TestEqual(TEXT("DeriveForName reads NAME_None as the empty name"),
		static_cast<int64>(CosmicSeed::DeriveForName(42, NAME_None, CosmicSeed::Purpose::Tell)), static_cast<int64>(272114134u));

	// NewGalaxy stamps the tell stream with the derived value. Without tables no line is told, so the stamp is still there.
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); };
	const FGalaxyState& State = Sim->GetState();

	Sim->SetTables(FCosmicDataTables());
	Sim->NewGalaxy(42, 5, 3);
	TestTrue(TEXT("Without tables nothing is told"), State.Log.IsEmpty());
	TestEqual(TEXT("NewGalaxy(42) stamps the tell stream with the derived value"), State.TellRngState, static_cast<int32>(272114134u));
	Sim->NewGalaxy(0, 5, 3);
	TestEqual(TEXT("NewGalaxy(0) stamps the tell stream with the derived value"), State.TellRngState, static_cast<int32>(4196651879u));

	// With the tables of the game, Install tells the first line: one tell draw, taken from the stamp.
	Sim->SetTables(Original);
	Sim->NewGalaxy(42, 5, 3);
	FRandomStream Probe(static_cast<int32>(272114134u));
	Probe.RandHelper(1);
	TestEqual(TEXT("NewGalaxy tells the first line and nothing else"), State.Log.Num(), 1);
	TestEqual(TEXT("The first line took one tell draw from the stamp"), State.TellRngState, Probe.GetCurrentSeed());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimTellStreamSeparateTest, "Cosmic_Ascension.Sim.TellStreamSeparate", CosmicTests::Flags)
bool FCosmicSimTellStreamSeparateTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();

	// The one error line of the DrawTell(0) call below.
	AddExpectedError(TEXT("DrawTell: Count 0 is not a variant count"), EAutomationExpectedErrorFlags::Contains, 1);

	Sim->NewGalaxy(42, 5, 3);
	const FGalaxyState& State = Sim->GetState();
	const FName RowName = CosmicRows::LogBreakthrough[0];
	const FLogTemplateDef* Row = Sim->GetTables().FindLogTemplate(RowName);
	if (!TestTrue(TEXT("DT_LogTemplates holds the breakthrough row (run Tools/make_data_tables.py)"), Row && !Row->Templates.IsEmpty()))
	{
		return false;
	}

	// Telling moves the tell stream by one step per line and nothing of the world (section 9.2).
	const int32 SimSeed = Sim->GetRng().GetCurrentSeed();
	const uint32 WorldBefore = Sim->ComputeWorldHash();
	const uint32 FullBefore = Sim->ComputeStateHash();
	FFormatNamedArguments Args;
	Args.Add(TEXT("Civ"), FText::AsCultureInvariant(TEXT("X")));
	bool bOneTellStepPerLine = true;
	for (int32 Call = 0; Call < 50; ++Call)
	{
		FRandomStream Probe(State.TellRngState);
		Log->PushRow(RowName, 0, Args, NAME_None);
		Probe.RandHelper(Row->Templates.Num());
		bOneTellStepPerLine &= State.TellRngState == Probe.GetCurrentSeed();
	}
	TestTrue(TEXT("Each told line advances the tell stream by exactly one step"), bOneTellStepPerLine);
	TestEqual(TEXT("Telling leaves the sim stream where it was"), Sim->GetRng().GetCurrentSeed(), SimSeed);
	TestEqual(TEXT("Telling leaves State.RngState where it was"), State.RngState, SimSeed);
	TestEqual(TEXT("Telling leaves the world hash unchanged"), static_cast<int64>(Sim->ComputeWorldHash()), static_cast<int64>(WorldBefore));
	TestTrue(TEXT("Telling changes the full hash"), Sim->ComputeStateHash() != FullBefore);

	// DrawTell itself: the number a probe stream gives, inside the range, one step of the tell stream, no sim draw.
	bool bDrawMatchesProbe = true;
	for (const int32 Count : { 1, 2, 3, 7 })
	{
		for (int32 Round = 0; Round < 50; ++Round)
		{
			FRandomStream Probe(State.TellRngState);
			const uint32 Got = Sim->DrawTell(Count);
			const int32 Want = Probe.RandHelper(Count);
			bDrawMatchesProbe &= Got == static_cast<uint32>(Want) && Got < static_cast<uint32>(Count) && State.TellRngState == Probe.GetCurrentSeed();
		}
	}
	TestTrue(TEXT("DrawTell returns the number of the stream, below Count, and writes the state back"), bDrawMatchesProbe);
	TestEqual(TEXT("DrawTell never draws from the sim stream"), Sim->GetRng().GetCurrentSeed(), SimSeed);

	// Not a variant count: one error line, no draw.
	const int32 TellBefore = State.TellRngState;
	TestEqual(TEXT("DrawTell(0) returns 0"), static_cast<int64>(Sim->DrawTell(0)), static_cast<int64>(0));
	TestEqual(TEXT("DrawTell(0) draws nothing"), State.TellRngState, TellBefore);

	// Which top-level member of FGalaxyState belongs to which hash (section 9.2). Every member is cleared in turn on a
	// state in which each of them holds something: a world hash that forgot an exclusion, or excluded too much, fails here.
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(1000);
	FGalaxyState& Mutable = Sim->GetMutableState();
	CosmicSimTests::FillEmptyContainers(Mutable);
	const uint32 World = Sim->ComputeWorldHash();
	const uint32 Full = Sim->ComputeStateHash();
	const FGalaxyState Backup = Mutable;

	const TSet<FName> NotWorld = {
		GET_MEMBER_NAME_CHECKED(FGalaxyState, Knowing), GET_MEMBER_NAME_CHECKED(FGalaxyState, Log),
		GET_MEMBER_NAME_CHECKED(FGalaxyState, Chronicle), GET_MEMBER_NAME_CHECKED(FGalaxyState, TellRngState)
	};
	// The slot identity is in no hash, and the live sim stream stands in for RngState in both.
	const TSet<FName> InNeither = { GET_MEMBER_NAME_CHECKED(FGalaxyState, GalaxyID), GET_MEMBER_NAME_CHECKED(FGalaxyState, RngState) };
	int32 Seen = 0;
	for (TFieldIterator<FProperty> It(FGalaxyState::StaticStruct()); It; ++It)
	{
		const FName Name = It->GetFName();
		It->ClearValue_InContainer(&Mutable);
		const bool bWorldMoved = Sim->ComputeWorldHash() != World;
		const bool bFullMoved = Sim->ComputeStateHash() != Full;
		Mutable = Backup;

		const bool bInFull = !InNeither.Contains(Name);
		const bool bInWorld = bInFull && !NotWorld.Contains(Name);
		TestTrue(*FString::Printf(TEXT("%s is %s the world hash"), *Name.ToString(), bInWorld ? TEXT("in") : TEXT("out of")), bWorldMoved == bInWorld);
		TestTrue(*FString::Printf(TEXT("%s is %s the full hash"), *Name.ToString(), bInFull ? TEXT("in") : TEXT("out of")), bFullMoved == bInFull);
		++Seen;
	}
	TestEqual(TEXT("Top-level members of FGalaxyState classified"), Seen, 18);
	TestTrue(TEXT("Both hashes are back after restoring the state"), Sim->ComputeWorldHash() == World && Sim->ComputeStateHash() == Full);
	Sim->GetRng().GetUnsignedInt();
	TestTrue(TEXT("A sim draw moves both hashes"), Sim->ComputeWorldHash() != World && Sim->ComputeStateHash() != Full);

	// A whole run that tells more. Run B tells one extra line at every hundredth tick boundary, so every line told from
	// inside a tick stage afterwards draws from another position of the tell stream. (Renaming an AI civ changes no line
	// count: TickResearch tells the breakthroughs of every people, named or not. The boundary lines carry the block.)
	// The world does not notice.
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(3000);
	const uint32 QuietWorld = Sim->ComputeWorldHash();
	const uint32 QuietFull = Sim->ComputeStateHash();
	const int32 QuietSeed = Sim->GetRng().GetCurrentSeed();
	const int32 QuietTell = State.TellRngState;

	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->GetMutableState().Civs[1].CivName = FText::AsCultureInvariant(TEXT("X"));
	for (int32 Slice = 0; Slice < 30; ++Slice)
	{
		Log->PushRow(RowName, 0, Args, NAME_None);
		Sim->RunHeadless(100);
	}
	TestEqual(TEXT("Both runs are 3,000 ticks long"), State.Tick, static_cast<int64>(3000));
	TestTrue(TEXT("More telling leaves the tell stream somewhere else"), State.TellRngState != QuietTell);
	TestEqual(TEXT("More telling, same world hash"), static_cast<int64>(Sim->ComputeWorldHash()), static_cast<int64>(QuietWorld));
	TestEqual(TEXT("More telling, same sim stream"), Sim->GetRng().GetCurrentSeed(), QuietSeed);
	TestTrue(TEXT("More telling, another full hash"), Sim->ComputeStateHash() != QuietFull);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveRoundTripTest, "Cosmic_Ascension.Sim.SaveRoundTrip", CosmicTests::Flags)
bool FCosmicSimSaveRoundTripTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicSaveSubsystem* Save = Sim->GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>();

	// The refusals below each log one LogCosmic warning.
	AddExpectedMessage(TEXT("LoadGalaxy: slot .* is (truncated or holds another galaxy|missing or unreadable)"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 3);
	AddExpectedMessage(TEXT("LoadGalaxy: slot .* has save version"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);
	// The slot this test cuts ends at the end of the engine's save header: no property is read, so the engine reports
	// nothing (a cut inside a property would, or would stop the process - guideline 16 #70).

	// The budget size, so that names with a number part (sys_100, civ_10) go through the save and the hash.
	Sim->NewGalaxy(42, CosmicTests::BudgetSystems, CosmicTests::BudgetCivs);
	Sim->RunHeadless(1000);
	CosmicSimTests::FillEmptyContainers(Sim->GetMutableState());
	// TickResearch re-adds the field: into the freed slot of the running map, at the end of the loaded one (section 9.2).
	Sim->GetMutableState().Civs[0].Research.Remove(FName(TEXT("Physics")));
	// The same for the cooldown keys of the event slots ("<row>@<CivID>"): one removed and inserted again, both far
	// enough ahead that the sweep at tick 1,200 keeps them.
	{
		TMap<FName, int64>& Cooldowns = Sim->GetMutableState().EventCooldownUntilTick;
		const FName First = UCosmicEventSubsystem::CooldownKey(FName(TEXT("evt_test")), Sim->GetState().Civs[1].CivID);
		const FName Second = UCosmicEventSubsystem::CooldownKey(FName(TEXT("evt_test")), Sim->GetState().Civs[2].CivID);
		Cooldowns.Add(First, 100000);
		Cooldowns.Add(Second, 100001);
		Cooldowns.Remove(First);
		Cooldowns.Add(First, 100000);
	}
	// A draw outside a tick: the save itself has to capture the live stream position.
	Sim->GetRng().GetUnsignedInt();
	// A stay in a fallen sphere under way: the counter is checked on its own after the load, not only through the hash.
	Sim->GetMutableState().Civs[1].FallenSphereTicks = 123;
	// An answer under way, a completion on the record and a feat of the calling (section 5.7, 4.4): checked on their own
	// after the load too.
	{
		FStarSystemState& Projects = Sim->GetMutableState().Systems[0];
		Projects.ResponseMegaproject = FName(TEXT("project_test_answer"));
		Projects.ResponseProgress = 3.5f;
		FMegaprojectRecord& Record = Projects.CompletedMegaprojects.AddDefaulted_GetRef();
		Record.Row = FName(TEXT("project_test_done"));
		Record.Tick = 7;
		Sim->GetMutableState().Calling.CompletedFeats.Add(FName(TEXT("feat_test_done")));
	}

	const FGuid GalaxyID = Sim->GetState().GalaxyID;
	const uint32 HashAtSave = Sim->ComputeStateHash();
	// The members reserved in v0.7 are compared one by one after the load: the hash cannot see a member without reflection.
	const FGalaxyState AtSave = Sim->GetState();
	TestEqual(TEXT("All 25 reserved members are off their initializer at save"), CosmicSimTests::CountReservedMembersSet(AtSave), 25);
	// Names and sentences are outside the hash, so they are compared on their own.
	const FString TextAtSave = CosmicSimTests::CollectText(Sim->GetState());
	if (!TestTrue(TEXT("Save succeeds"), Save->SaveCurrentGalaxy()))
	{
		return false;
	}

	// The uninterrupted run: 500 more ticks, then four draws from each stream.
	Sim->RunHeadless(500);
	const uint32 HashContinued = Sim->ComputeStateHash();
	uint32 DrawsContinued[4];
	for (uint32& Draw : DrawsContinued)
	{
		Draw = Sim->GetRng().GetUnsignedInt();
	}
	uint32 TellsContinued[4];
	for (uint32& Tell : TellsContinued)
	{
		Tell = Sim->DrawTell(1000);
	}

	// The same from the save.
	if (TestTrue(TEXT("Load succeeds"), Save->LoadGalaxy(GalaxyID)))
	{
		TestEqual(TEXT("Hash right after load equals hash at save"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(HashAtSave));
		TestEqual(TEXT("Tick after load"), Sim->GetState().Tick, static_cast<int64>(1000));
		TestEqual(TEXT("A stay in a fallen sphere survives the round trip"),
			Sim->GetState().Civs.IsValidIndex(1) ? Sim->GetState().Civs[1].FallenSphereTicks : static_cast<int32>(INDEX_NONE), 123);
		if (TestTrue(TEXT("The system of the projects is there after load"), Sim->GetState().Systems.IsValidIndex(0)))
		{
			const FStarSystemState& Projects = Sim->GetState().Systems[0];
			TestEqual(TEXT("An answer under way survives the round trip"), Projects.ResponseMegaproject, FName(TEXT("project_test_answer")));
			TestEqual(TEXT("with its progress"), Projects.ResponseProgress, 3.5f);
			const FMegaprojectRecord* Record = Projects.CompletedMegaprojects.FindByPredicate([](const FMegaprojectRecord& Candidate) { return Candidate.Row == FName(TEXT("project_test_done")); });
			if (TestNotNull(TEXT("A completion record survives the round trip"), Record))
			{
				TestEqual(TEXT("with its tick"), Record->Tick, static_cast<int64>(7));
			}
		}
		TestTrue(TEXT("The feats of the calling survive the round trip"), Sim->GetState().Calling.CompletedFeats.Contains(FName(TEXT("feat_test_done"))));
		TestTrue(TEXT("GalaxyID survives the round trip"), Sim->GetState().GalaxyID == GalaxyID);
		TestEqual(TEXT("Names and log text survive the round trip"), CosmicSimTests::CollectText(Sim->GetState()), TextAtSave);
		TestTrue(TEXT("Loaded log text is still culture-invariant"), !Sim->GetState().Log.IsEmpty() && Sim->GetState().Log[0].Message.IsCultureInvariant());
		TestEqual(TEXT("All 25 reserved members are off their initializer after load"), CosmicSimTests::CountReservedMembersSet(Sim->GetState()), 25);
		TestTrue(TEXT("Every reserved member survives the round trip"), CosmicSimTests::SameReservedMembers(Sim->GetState(), AtSave));

		Sim->RunHeadless(500);
		TestEqual(TEXT("Hash after load + 500 ticks equals the uninterrupted run"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(HashContinued));
		for (const uint32 Expected : DrawsContinued)
		{
			TestEqual(TEXT("Random stream continues identically after load"), static_cast<int64>(Sim->GetRng().GetUnsignedInt()), static_cast<int64>(Expected));
		}
		for (const uint32 Expected : TellsContinued)
		{
			TestEqual(TEXT("Tell stream continues identically after load"), static_cast<int64>(Sim->DrawTell(1000)), static_cast<int64>(Expected));
		}
		TestEqual(TEXT("The cooldown keys of the event slots survive the round trip and the sweep"),
			Sim->GetState().EventCooldownUntilTick.FindRef(UCosmicEventSubsystem::CooldownKey(FName(TEXT("evt_test")), Sim->GetState().Civs[1].CivID)), static_cast<int64>(100000));
	}

	// Refused loads leave the running galaxy untouched.
	const uint32 HashBeforeRefusals = Sim->ComputeStateHash();
	const FString SlotName = UCosmicSaveSubsystem::MakeSlotName(GalaxyID);

	// A slot cut short, as a write interrupted by a crash would leave it.
	TArray<uint8> Bytes;
	if (TestTrue(TEXT("Slot bytes can be read"), UGameplayStatics::LoadDataFromSlot(Bytes, SlotName, 0)))
	{
		// The intact bytes under another galaxy's slot name, as a copied or renamed file would be.
		const FGuid ForeignID = FGuid::NewGuid();
		TestTrue(TEXT("Foreign slot written"), UGameplayStatics::SaveDataToSlot(Bytes, UCosmicSaveSubsystem::MakeSlotName(ForeignID), 0));
		TestFalse(TEXT("A slot that holds another galaxy is refused"), Save->LoadGalaxy(ForeignID));
		TestTrue(TEXT("Foreign slot removed"), Save->DeleteSave(ForeignID));

		// Cut at the end of the engine's save header, which ends with the class path of the save object and its
		// terminator: no tagged property is read, so the cut lands in the same place whatever the state holds. A cut
		// inside a property can reach a fatal check of the engine in a build with checks (guideline 16 #70).
		const FString ClassPath = UCosmicSaveGame::StaticClass()->GetPathName();
		const auto Path = StringCast<ANSICHAR>(*ClassPath);
		int32 HeaderEnd = INDEX_NONE;
		for (int32 At = 0; At + Path.Length() < Bytes.Num(); ++At)
		{
			if (FMemory::Memcmp(Bytes.GetData() + At, Path.Get(), Path.Length()) == 0 && Bytes[At + Path.Length()] == 0)
			{
				HeaderEnd = At + Path.Length() + 1;
				break;
			}
		}
		if (TestTrue(TEXT("The slot holds the class path of the save object"), HeaderEnd != INDEX_NONE))
		{
			Bytes.SetNum(HeaderEnd);
			TestTrue(TEXT("Truncated slot written"), UGameplayStatics::SaveDataToSlot(Bytes, SlotName, 0));
			TestFalse(TEXT("A slot cut at the end of the engine's header is refused"), Save->LoadGalaxy(GalaxyID));
		}
	}
	TestTrue(TEXT("Test slot removed"), Save->DeleteSave(GalaxyID));
	TestFalse(TEXT("A missing slot is refused"), Save->LoadGalaxy(GalaxyID));
	TestEqual(TEXT("Refused loads leave the galaxy untouched"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(HashBeforeRefusals));

	// A save newer than this build.
	Sim->NewGalaxy(43, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	const FGuid NewerID = Sim->GetState().GalaxyID;
	Sim->GetMutableState().SaveVersion = FGalaxyState::CurrentSaveVersion + 1;
	TestTrue(TEXT("Save of a newer-version state succeeds"), Save->SaveCurrentGalaxy());
	Sim->NewGalaxy(44, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	const uint32 HashBeforeNewer = Sim->ComputeStateHash();
	TestFalse(TEXT("A newer save is refused"), Save->LoadGalaxy(NewerID));
	TestEqual(TEXT("A refused newer save leaves the galaxy untouched"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(HashBeforeNewer));
	TestTrue(TEXT("Newer-version slot removed"), Save->DeleteSave(NewerID));
	return true;
}

// The Phase 1 exit criterion needs a restart in the middle, so this test spans two processes: the first run saves and
// records the hash, the next run (a new process of the same build) loads and compares, and also simulates the same
// seed afresh. A rebuilt binary or regenerated data tables start over. After a failed verification the save and the
// record stay, so every later process of that build and data fails the same way.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveRestartTest, "Cosmic_Ascension.Sim.SaveRestart", CosmicTests::Flags)
bool FCosmicSimSaveRestartTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicSaveSubsystem* Save = Sim->GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>();

	const FString RecordPath = FPaths::ProjectSavedDir() / TEXT("Automation/SaveRestart.txt");
	const FString BinaryStamp = IFileManager::Get().GetTimeStamp(*CosmicSimTests::SimBinaryPath()).ToIso8601() + CosmicSimTests::DataStamp(Sim->GetTables());
	const uint32 ProcessId = FPlatformProcess::GetCurrentProcessId();

	FString Record;
	FString RecordedStamp;
	FString RecordedGalaxy;
	FString RecordedHash;
	uint32 RecordedProcessId = 0;
	FGuid RecordedGalaxyID;
	const bool bHasRecord = FFileHelper::LoadFileToString(Record, *RecordPath)
		&& FParse::Value(*Record, TEXT("Binary="), RecordedStamp)
		&& FParse::Value(*Record, TEXT("Galaxy="), RecordedGalaxy)
		&& FParse::Value(*Record, TEXT("Hash="), RecordedHash)
		&& FParse::Value(*Record, TEXT("Process="), RecordedProcessId)
		&& FGuid::Parse(RecordedGalaxy, RecordedGalaxyID);
	// Parsed outside bHasRecord: a record of the format before this value still gets its slot deleted below.
	uint32 RecordedDerive = 0;
	const bool bHasDerive = FParse::Value(*Record, TEXT("Derive="), RecordedDerive);
	// Decided by the seed, the name and the frozen expression alone (section 9.2). The second half only runs on the binary
	// that wrote the record, so this comparison is equal by construction: it carries out the record entry of section 9.3
	// and guards nothing by itself. The value is held by Sim.SeedDerive (same call, golden constant) and CosmicSeed.h.
	const uint32 DeriveNow = CosmicSimTests::DeriveLowerCased(42, TEXT("gal_andromeda"), CosmicSeed::Purpose::Summary, 0);

	if (bHasRecord && RecordedStamp == BinaryStamp && RecordedProcessId != ProcessId)
	{
		// Second half: a save written by an earlier process of this same build.
		bool bVerified = false;
		if (TestTrue(TEXT("Load of the save written by the previous process succeeds"), Save->LoadGalaxy(RecordedGalaxyID)))
		{
			const bool bHashOk = TestEqual(TEXT("Hash after restart and load equals the hash recorded at save"),
				FString::Printf(TEXT("%u"), Sim->ComputeStateHash()), RecordedHash);
			const bool bTickOk = TestEqual(TEXT("Tick after restart and load"), Sim->GetState().Tick, static_cast<int64>(CosmicSimTests::RestartTicks));
			// Simulation determinism across processes, not only the hash of a loaded state.
			const bool bFreshOk = TestEqual(TEXT("A fresh run of seed 42 in this process equals the hash recorded by the previous process"),
				FString::Printf(TEXT("%u"), CosmicSimTests::HashAfter(*Sim, 42, CosmicSimTests::RestartTicks)), RecordedHash);
			const bool bDeriveOk = TestTrue(TEXT("CosmicSeed::Derive gives the value recorded by the previous process"), bHasDerive && RecordedDerive == DeriveNow);
			bVerified = bHashOk && bTickOk && bFreshOk && bDeriveOk;
		}
		// A slot that is gone cannot be verified again: start over.
		if (bVerified || !Save->DoesSaveExist(RecordedGalaxyID))
		{
			Save->DeleteSave(RecordedGalaxyID);
			IFileManager::Get().Delete(*RecordPath);
		}
		if (bVerified)
		{
			AddInfo(TEXT("Restart half verified against the save of an earlier process."));
		}
		return true;
	}

	// First half (or a stale record from another build): leave a save and its hash for the next process.
	if (bHasRecord)
	{
		Save->DeleteSave(RecordedGalaxyID);
	}
	const uint32 Hash = CosmicSimTests::HashAfter(*Sim, 42, CosmicSimTests::RestartTicks);
	if (!TestTrue(TEXT("Save succeeds"), Save->SaveCurrentGalaxy()))
	{
		return false;
	}
	Record = FString::Printf(TEXT("Binary=%s Galaxy=%s Hash=%u Process=%u Derive=%u"),
		*BinaryStamp, *Sim->GetState().GalaxyID.ToString(EGuidFormats::Digits), Hash, ProcessId, DeriveNow);
	TestTrue(TEXT("Record written"), FFileHelper::SaveStringToFile(Record, *RecordPath));
	AddInfo(TEXT("Saved and recorded the hash. Run the tests once more in a new process to verify the restart half."));
	return true;
}

// The layers of the home galaxy as the game's tables make them (section 3.1, 7.4): the numbers of the home row, the star
// classes of DT_Rules.LivingWorldStarShares, the place and class of sol, names that never repeat, lanes inside the one
// galaxy, and every system of a people stamped like its people. The lanes still stay inside after 20,000 ticks.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimLayersInRangeTest, "Cosmic_Ascension.Sim.LayersInRange", CosmicTests::Flags)
bool FCosmicSimLayersInRangeTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicRulesDef& Rules = Sim->GetRules();
	const FGalaxyDef& Home = Sim->GetHomeGalaxy();
	if (!TestNotNull(TEXT("DT_Galaxies holds the home row (run Tools/make_data_tables.py)"), Sim->GetTables().FindGalaxy(Rules.HomeGalaxyRow))
		|| !TestTrue(TEXT("DT_Rules has the seven star shares"), Rules.LivingWorldStarShares.Num() == 7))
	{
		return false;
	}
	const FCivArchetypeDef* PlayerArchetype = Sim->GetTables().FindCiv(Rules.PlayerArchetypeRow);
	const FGameplayTag HomeClass = PlayerArchetype ? PlayerArchetype->HomeStarClass : FGameplayTag();
	const TCHAR* ClassNames[] = { TEXT("Star.Class.O"), TEXT("Star.Class.B"), TEXT("Star.Class.A"), TEXT("Star.Class.F"), TEXT("Star.Class.G"), TEXT("Star.Class.K"), TEXT("Star.Class.M") };

	const FGalaxyState& State = Sim->GetState();
	bool bSystemCount = true;
	bool bPeopleCount = true;
	bool bSolID = true;
	bool bSolPlace = true;
	bool bSolAngle = true;
	bool bSolClass = true;
	bool bInDisc = true;
	bool bNamed = true;
	bool bNamesOnce = true;
	bool bPeopleNamed = true;
	bool bPeopleNamesOnce = true;
	bool bLanesHome = true;
	bool bCivIDsOnce = true;
	bool bSystemsStamped = true;
	bool bCivsStamped = true;
	bool bOwnedLikeOwner = true;
	TArray<int32> Classes;
	Classes.Init(0, UE_ARRAY_COUNT(ClassNames));
	int32 Drawn = 0;
	for (int32 Seed = 1; Seed <= 20; ++Seed)
	{
		Sim->NewGalaxy(Seed, 0, 0);
		bSystemCount &= State.Systems.Num() == Home.LivingWorlds;
		bPeopleCount &= State.Civs.Num() == Home.Peoples;
		if (State.Systems.IsEmpty())
		{
			continue;
		}
		const double Radius = 1000.0 * FMath::Sqrt(static_cast<double>(State.Systems.Num()));
		const FStarSystemState& Sol = State.Systems[0];
		bSolID &= Sol.SystemID == FName(TEXT("sol"));
		bSolPlace &= FMath::IsNearlyEqual(Sol.Position.X, Home.HomeRadiusShare * Radius, 0.01 * Radius);
		bSolAngle &= Sol.Position.Y == 0.0;
		bSolClass &= !HomeClass.IsValid() || Sol.StarClass == HomeClass;

		TSet<FString> Names;
		TSet<FName> Homes;
		for (const FCivilizationState& Civ : State.Civs)
		{
			Homes.Add(Civ.HomeSystemID);
		}
		for (const FStarSystemState& System : State.Systems)
		{
			bInDisc &= System.Position.X * System.Position.X + System.Position.Y * System.Position.Y <= Radius * Radius * 1.0001;
			bNamed &= !System.DisplayName.IsEmpty();
			bNamesOnce &= !Names.Contains(System.DisplayName.ToString());
			Names.Add(System.DisplayName.ToString());
			bSystemsStamped &= System.GalaxyRow == Rules.HomeGalaxyRow;
			for (const FName Lane : System.Lanes)
			{
				const FStarSystemState* Other = State.Systems.FindByPredicate([Lane](const FStarSystemState& Each) { return Each.SystemID == Lane; });
				bLanesHome &= Other && Other->GalaxyRow == System.GalaxyRow;
			}
			// Homes keep a class of their archetype when it has one; every other system shows the drawn shares.
			if (!Homes.Contains(System.SystemID))
			{
				for (int32 Class = 0; Class < Classes.Num(); ++Class)
				{
					Classes[Class] += (System.StarClass.GetTagName() == FName(ClassNames[Class])) ? 1 : 0;
				}
				++Drawn;
			}
		}
		TSet<FName> CivIDs;
		TSet<FString> PeopleNames;
		for (const FCivilizationState& Civ : State.Civs)
		{
			bCivIDsOnce &= !CivIDs.Contains(Civ.CivID);
			CivIDs.Add(Civ.CivID);
			bCivsStamped &= Civ.GalaxyRow == Rules.HomeGalaxyRow;
			bPeopleNamed &= !Civ.CivName.IsEmpty();
			bPeopleNamesOnce &= !PeopleNames.Contains(Civ.CivName.ToString());
			PeopleNames.Add(Civ.CivName.ToString());
			for (const FName Owned : Civ.OwnedSystems)
			{
				const FStarSystemState* System = State.Systems.FindByPredicate([Owned](const FStarSystemState& Each) { return Each.SystemID == Owned; });
				bOwnedLikeOwner &= System && System->GalaxyRow == Civ.GalaxyRow;
			}
		}
	}
	TestTrue(TEXT("The number of systems is that of the home row"), bSystemCount);
	TestTrue(TEXT("The number of peoples is that of the home row"), bPeopleCount);
	TestTrue(TEXT("The first system is sol"), bSolID);
	TestTrue(TEXT("sol lies at its share of the radius"), bSolPlace);
	TestTrue(TEXT("sol lies at angle 0"), bSolAngle);
	TestTrue(TEXT("sol has the star class of the player's archetype"), bSolClass);
	TestTrue(TEXT("Every system lies in the disc"), bInDisc);
	TestTrue(TEXT("Every system has a name"), bNamed);
	TestTrue(TEXT("No system name comes twice"), bNamesOnce);
	TestTrue(TEXT("Every people has a name"), bPeopleNamed);
	TestTrue(TEXT("No people name comes twice"), bPeopleNamesOnce);
	TestTrue(TEXT("Every lane joins two systems of one GalaxyRow"), bLanesHome);
	TestTrue(TEXT("No CivID comes twice"), bCivIDsOnce);
	TestTrue(TEXT("Every system carries the home row"), bSystemsStamped);
	TestTrue(TEXT("Every people carries the home row"), bCivsStamped);
	TestTrue(TEXT("Every owned system carries the GalaxyRow of its owner"), bOwnedLikeOwner);

	// The shares: about a thousand drawn systems against DT_Rules.LivingWorldStarShares. Each class is drawn at least
	// once, and lies within four standard deviations of its share (plus half a point); the seeds are fixed.
	float Below = 0.f;
	for (int32 Class = 0; Class < Classes.Num(); ++Class)
	{
		const float Expected = Rules.LivingWorldStarShares[Class] - Below;
		Below = Rules.LivingWorldStarShares[Class];
		const float Seen = Drawn > 0 ? static_cast<float>(Classes[Class]) / static_cast<float>(Drawn) : -1.f;
		const float Bound = Drawn > 0 ? 4.f * FMath::Sqrt(Expected * (1.f - Expected) / static_cast<float>(Drawn)) + 0.005f : 0.f;
		if (Expected > 0.f)
		{
			TestTrue(*FString::Printf(TEXT("%s is drawn at least once"), ClassNames[Class]), Classes[Class] > 0);
		}
		TestTrue(*FString::Printf(TEXT("%s: %.3f of the drawn systems, the shares say %.3f (bound %.3f)"), ClassNames[Class], Seen, Expected, Bound), FMath::Abs(Seen - Expected) <= Bound);
	}

	// Twenty thousand ticks later the lanes still stay inside the galaxy, and the breakthroughs of AI peoples are told
	// with their people (the CivName gate is gone, section 8.1).
	Sim->NewGalaxy(42, 0, 0);
	// Without their names the AI peoples are still told of: whether a line is told never reads a text (section 8.1).
	// FText is in neither hash, so the run is the same world.
	// The peoples of the start, by ID: whether the player rules it. A people may leave Civs later (the crossing stage), and
	// the log keeps 2,000 lines, so the lines are read every 1,000 ticks against the peoples of the start.
	TMap<FName, bool> RuledAtStart;
	for (FCivilizationState& Civ : Sim->GetMutableState().Civs)
	{
		RuledAtStart.Add(Civ.CivID, Civ.bIsPlayerRuled);
		if (!Civ.bIsPlayerRuled)
		{
			Civ.CivName = FText::GetEmpty();
		}
	}
	bool bToldOfAIPeople = false;
	bool bFocusOfBreakthrough = true;
	for (int32 Chunk = 0; Chunk < 20; ++Chunk)
	{
		Sim->RunHeadless(1000);
		for (const FCosmicLogEntry& Entry : State.Log)
		{
			if (Entry.SourceRow.ToString().StartsWith(TEXT("log_breakthrough_")))
			{
				const bool* bRuled = RuledAtStart.Find(Entry.FocusCivID);
				bFocusOfBreakthrough &= bRuled != nullptr;
				bToldOfAIPeople |= bRuled && !*bRuled;
			}
		}
	}
	bool bOneRow = true;
	bool bStillHome = true;
	for (const FStarSystemState& System : State.Systems)
	{
		bStillHome &= System.GalaxyRow == Rules.HomeGalaxyRow;
		for (const FName Lane : System.Lanes)
		{
			const FStarSystemState* Other = State.Systems.FindByPredicate([Lane](const FStarSystemState& Each) { return Each.SystemID == Lane; });
			bOneRow &= Other && Other->GalaxyRow == System.GalaxyRow;
		}
	}
	TestTrue(TEXT("After 20,000 ticks every lane still joins two systems of one galaxy"), bOneRow);
	TestTrue(TEXT("After 20,000 ticks every system is still of the home galaxy"), bStillHome);
	TestTrue(TEXT("Every breakthrough line carries the people it tells of"), bFocusOfBreakthrough);
	TestTrue(TEXT("Breakthroughs of AI peoples are told, nameless ones too"), bToldOfAIPeople);
	return true;
}

// The exit conditions of Phase 2 on the game's data, input-free (guideline 11, Phase 2): hazards are placed, run out and are
// answered, each ending with its own line; a claim stands next to Silence, runs out and joins its sphere (seed 42, 60
// systems and 12 peoples, 20,000 ticks); and a neglected ruler is deposed by a rebellion, with the fallen monarch's line
// (seed 7). The seeds are measured, not chosen by a rule: a change of the world that loses a condition on its seed has to
// show it on another one and say so in guideline 9.3. The stepped run steps as a game's frames do (StepOnce, the gate
// open), and it is the same world as the whole run at once with the gate closed.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimPhaseTwoExitTest, "Cosmic_Ascension.Sim.PhaseTwoExit", CosmicTests::Flags)
bool FCosmicSimPhaseTwoExitTest::RunTest(const FString& Parameters)
{
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	Sim->SetTables(Original);
	static const FGameplayTag ClaimTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Claim")));
	static const FGameplayTag RebellionTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Rebellion")));
	constexpr int64 Ticks = 20000;

	// The deposition of condition 5 rests on the game's rebellion row: the evidence of seed 7 below is only as good as this.
	FName RebellionRow;
	const FHazardDef* Rebellion = Original.FindHazardByTag(RebellionTag, &RebellionRow);
	TestTrue(TEXT("The game's rebellion row deposes the ruler when it runs out"),
		Rebellion && Rebellion->OnExpireEffects.ContainsByPredicate([](const FCosmicEffect& Effect) { return Effect.Type == EEffectType::DeposeRuler; }));

	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	const FGalaxyState& State = Sim->GetState();
	auto Silence = [&State]() { return State.Transcendents.FindByPredicate([](const FTranscendentState& Each) { return Each.TranscendentID == CosmicRows::TransSilence; }); };
	TestTrue(TEXT("Silence is in the roster (seed 42)"), Silence() != nullptr);
	const int32 SphereStart = Silence() ? Silence()->InfluencedSystems.Num() : 0;
	int32 Placed = 0;
	int32 RanOut = 0;
	int32 RanOutTold = 0;
	int32 Answered = 0;
	int32 AnsweredTold = 0;
	int32 Claims = 0;
	int32 ClaimsJoined = 0;
	struct FSlot
	{
		FGameplayTag Tag;
		int32 Remaining = 0;
		FName Answer;
	};
	TArray<FSlot> Before;
	for (int64 Step = 0; Step < Ticks; ++Step)
	{
		Before.Reset();
		for (const FStarSystemState& System : State.Systems)
		{
			Before.Add({ System.ActiveHazard, System.HazardTicksRemaining, System.ResponseMegaproject });
		}
		const int64 TickBefore = State.Tick;
		Sim->StepOnce();
		// Whether a line of Row on SystemID was stored in this tick (every line is stored: tell-all).
		const auto ToldHere = [&State, TickBefore](FName Row, FName SystemID)
		{
			for (int32 Index = State.Log.Num() - 1; Index >= 0 && State.Log[Index].Tick > TickBefore; --Index)
			{
				if (State.Log[Index].SourceRow == Row && State.Log[Index].FocusSystemID == SystemID)
				{
					return true;
				}
			}
			return false;
		};
		for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num() && SystemIndex < Before.Num(); ++SystemIndex)
		{
			const FStarSystemState& System = State.Systems[SystemIndex];
			const FSlot& Slot = Before[SystemIndex];
			const bool bAgain = System.ActiveHazard == Slot.Tag && System.HazardTicksRemaining >= Slot.Remaining;
			if (Slot.Tag.IsValid() && (System.ActiveHazard != Slot.Tag || bAgain))
			{
				FName Row;
				Original.FindHazardByTag(Slot.Tag, &Row);
				// Answered: before its last count, the answer in its response slot completed in this tick (its record moved
				// to this tick) - only the resolution of that completion ends a hazard there. On the last count an expiry
				// ends it in the same tick whatever the answer did, so such an end is counted as neither. Ran out: not
				// answered, on its last count, and not kept by the grace of the last system.
				const bool bCompleted = !Slot.Answer.IsNone() && CosmicMegaprojects::HasCompleted(System, Slot.Answer, State.Tick);
				if (bCompleted && Slot.Remaining > 1)
				{
					++Answered;
					AnsweredTold += ToldHere(Row, System.SystemID) ? 1 : 0;
				}
				else if (!bCompleted && Slot.Remaining == 1 && !ToldHere(CosmicRows::EventGraceRemnant, System.SystemID))
				{
					++RanOut;
					RanOutTold += ToldHere(Row, System.SystemID) ? 1 : 0;
					const FTranscendentState* Sphere = Silence();
					ClaimsJoined += (Slot.Tag == ClaimTag && Sphere && Sphere->InfluencedSystems.Contains(System.SystemID)) ? 1 : 0;
				}
			}
			if (System.ActiveHazard.IsValid() && (System.ActiveHazard != Slot.Tag || bAgain))
			{
				++Placed;
				Claims += System.ActiveHazard == ClaimTag ? 1 : 0;
			}
		}
	}
	TestTrue(TEXT("Hazards are placed in an input-free run of the game's data"), Placed > 0);
	TestTrue(TEXT("some run out"), RanOut > 0);
	TestEqual(TEXT("and every one that runs out is told by its row on its system"), RanOutTold, RanOut);
	TestTrue(TEXT("some are answered: an answer of the hazard's row completes before it runs out"), Answered > 0);
	TestEqual(TEXT("and every answered one is told by its row on its system"), AnsweredTold, Answered);
	TestTrue(TEXT("A claim stands (seed 42)"), Claims > 0);
	TestTrue(TEXT("A claim runs out and its system joins the sphere of Silence (seed 42)"), ClaimsJoined > 0);
	TestTrue(TEXT("and the sphere of Silence grows (seed 42)"), Silence() && Silence()->InfluencedSystems.Num() > SphereStart);
	const uint32 Stepped = Sim->ComputeWorldHash();
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(Ticks);
	TestEqual(TEXT("Stepped with the gate open, one tick at a time is the same world as the whole run at once with it closed"), Sim->ComputeWorldHash(), Stepped);

	// Neglect: nothing the player does keeps the ruled people's stability up, and a rebellion deposes the ruler.
	Sim->NewGalaxy(7, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(Ticks);
	TestTrue(TEXT("A neglected ruler is deposed (seed 7)"), State.Player.Role == EPlayerRole::Wanderer);
	TestTrue(TEXT("and carries the status of a fallen monarch"), State.Player.StatusTags.HasTagExact(FGameplayTag::RequestGameplayTag(FName(TEXT("Status.FallenMonarch")))));
	const auto Fallen = [](const FCosmicLogEntry& Entry) { return Entry.SourceRow == CosmicRows::LogFallenMonarch && Entry.Severity == 2; };
	const FCosmicLogEntry* FallenLine = State.Log.FindByPredicate(Fallen);
	FallenLine = FallenLine ? FallenLine : State.Chronicle.FindByPredicate(Fallen);
	TestTrue(TEXT("and the fallen monarch's line is told"), FallenLine != nullptr);
	const bool bReignDeposed = !State.Calling.Lives.IsEmpty()
		&& State.Calling.Lives[0].ReignEndReasons.Contains(FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Deposed"))));
	TestTrue(TEXT("The first reign ended as a deposition"), bReignDeposed);
	const int64 FallenTick = FallenLine ? FallenLine->Tick : -1;
	const auto RisingEnds = [FallenTick, RebellionRow](const FCosmicLogEntry& Entry) { return Entry.Tick == FallenTick && Entry.SourceRow == RebellionRow; };
	TestTrue(TEXT("A rebellion ran out in the tick of the deposition"), State.Log.ContainsByPredicate(RisingEnds) || State.Chronicle.ContainsByPredicate(RisingEnds));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

// Cosmic Ascension - automation tests of the simulation core. Design: guideline.md section 9.2, 9.3.
// Run with the automation command in CLAUDE.md, twice in a row after a build (see Sim.SaveRestart).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Containers/StringConv.h"
#include "Containers/StringView.h"
#include "Core/CosmicSeed.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
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
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/TextProperty.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"

#include <limits>

namespace CosmicSimTests
{
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
	FString DataStamp()
	{
		FString Stamp;
		for (const TCHAR* Package : { TEXT("/Game/CA/Data/DT_Rules"), TEXT("/Game/CA/Data/DT_Civs"), TEXT("/Game/CA/Data/DT_LogTemplates") })
		{
			FString File;
			if (FPackageName::TryConvertLongPackageNameToFilename(FString(Package), File, FPackageName::GetAssetPackageExtension()))
			{
				Stamp += TEXT("_") + IFileManager::Get().GetTimeStamp(*File).ToIso8601();
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
		Sim->GetState().Systems.Num() == CosmicTests::SliceSystems && Sim->GetState().Civs.Num() == CosmicTests::SliceCivs && AllResolve());

	// An array changed by hand: the index is stale, says so once and never answers with another element.
	FGalaxyState& Mutable = Sim->GetMutableState();
	const FName Shifted = Mutable.Civs[2].CivID;
	Mutable.Civs.RemoveAt(1);
	TestEqual(TEXT("A stale index answers INDEX_NONE, not a wrong element"), Index.FindCiv(Mutable, Shifted), static_cast<int32>(INDEX_NONE));
	Sim->RebuildStateIndex();
	TestEqual(TEXT("After RebuildStateIndex the shifted civ is found where it is"), Index.FindCiv(Mutable, Shifted), 1);
	TestTrue(TEXT("After RebuildStateIndex every ID resolves"), AllResolve());

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
		CosmicSeed::Purpose::Backdrop, CosmicSeed::Purpose::Scenery, CosmicSeed::Purpose::Depart, CosmicSeed::Purpose::Archetype
	};
	bool bPurposesFixed = true;
	for (int32 Index = 0; Index < static_cast<int32>(UE_ARRAY_COUNT(Purposes)); ++Index)
	{
		bPurposesFixed &= Purposes[Index] == static_cast<uint32>(Index + 1);
	}
	TestTrue(TEXT("The eight purposes are numbered 1 to 8 in their fixed order"), bPurposesFixed);

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
	// inside a tick stage afterwards draws from another position of the tell stream. While TickResearch still gates the
	// breakthrough lines on CivName (until the Phase 2 generation change), naming an AI civ also adds lines told from
	// inside the stages; once the gate is gone that line does nothing and the boundary lines carry the block.
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
	// The engine reports the slot that this test cuts in half: one or more errors, and a warning depending on where the cut falls.
	AddExpectedError(TEXT("Failed loading tagged"), EAutomationExpectedErrorFlags::Contains, 0);
	AddExpectedMessage(TEXT("SerializeFromMismatchedTag failed"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

	// The budget size, so that names with a number part (sys_100, civ_10) go through the save and the hash.
	Sim->NewGalaxy(42, CosmicTests::BudgetSystems, CosmicTests::BudgetCivs);
	Sim->RunHeadless(1000);
	CosmicSimTests::FillEmptyContainers(Sim->GetMutableState());
	// TickResearch re-adds the field: into the freed slot of the running map, at the end of the loaded one (section 9.2).
	Sim->GetMutableState().Civs[0].Research.Remove(FName(TEXT("Physics")));
	// A draw outside a tick: the save itself has to capture the live stream position.
	Sim->GetRng().GetUnsignedInt();

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
	}

	// Refused loads leave the running galaxy untouched.
	const uint32 HashBeforeRefusals = Sim->ComputeStateHash();
	const FString SlotName = UCosmicSaveSubsystem::MakeSlotName(GalaxyID);

	// A slot cut in half, as a write interrupted by a crash would leave it.
	TArray<uint8> Bytes;
	if (TestTrue(TEXT("Slot bytes can be read"), UGameplayStatics::LoadDataFromSlot(Bytes, SlotName, 0)))
	{
		// The intact bytes under another galaxy's slot name, as a copied or renamed file would be.
		const FGuid ForeignID = FGuid::NewGuid();
		TestTrue(TEXT("Foreign slot written"), UGameplayStatics::SaveDataToSlot(Bytes, UCosmicSaveSubsystem::MakeSlotName(ForeignID), 0));
		TestFalse(TEXT("A slot that holds another galaxy is refused"), Save->LoadGalaxy(ForeignID));
		TestTrue(TEXT("Foreign slot removed"), Save->DeleteSave(ForeignID));

		Bytes.SetNum(Bytes.Num() / 2);
		TestTrue(TEXT("Truncated slot written"), UGameplayStatics::SaveDataToSlot(Bytes, SlotName, 0));
		TestFalse(TEXT("A truncated slot is refused"), Save->LoadGalaxy(GalaxyID));
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
	const FString BinaryStamp = IFileManager::Get().GetTimeStamp(*CosmicSimTests::SimBinaryPath()).ToIso8601() + CosmicSimTests::DataStamp();
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
			const bool bTickOk = TestEqual(TEXT("Tick after restart and load"), Sim->GetState().Tick, static_cast<int64>(1000));
			// Simulation determinism across processes, not only the hash of a loaded state.
			const bool bFreshOk = TestEqual(TEXT("A fresh run of seed 42 in this process equals the hash recorded by the previous process"),
				FString::Printf(TEXT("%u"), CosmicSimTests::HashAfter(*Sim, 42, 1000)), RecordedHash);
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
	const uint32 Hash = CosmicSimTests::HashAfter(*Sim, 42, 1000);
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

#endif // WITH_DEV_AUTOMATION_TESTS

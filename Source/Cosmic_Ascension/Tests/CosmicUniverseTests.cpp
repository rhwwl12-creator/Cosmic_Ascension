// Cosmic Ascension - automation tests of the universe screen (Presentation). Design: guideline.md section 5.11, 6.4, 9.3,
// 11 Phase 3 (item 9).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/LineBatchComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "Core/CosmicSeed.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "InputCoreTypes.h"
#include "Internationalization/StringTableCore.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameters.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicAscensionOverlayWidget.h"
#include "UI/CosmicGalaxyPanelWidget.h"
#include "UI/CosmicPalette.h"
#include "UI/CosmicSystemLabelWidget.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicUILabels.h"
#include "UObject/NameTypes.h"
#include "World/CosmicCameraPawn.h"
#include "World/CosmicGalaxyMapActor.h"
#include "World/CosmicGameMode.h"
#include "World/CosmicMapLook.h"
#include "World/CosmicMapPick.h"
#include "World/CosmicStarSystemActor.h"
#include "World/CosmicUniverseLook.h"
#include "World/CosmicZoomLevels.h"

#include <limits>

namespace CosmicUniverseTests
{
	const float NaN = std::numeric_limits<float>::quiet_NaN();

	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name), false);
	}

	ACosmicGalaxyMapActor* FindMap(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}
		for (TActorIterator<ACosmicGalaxyMapActor> It(World); It; ++It)
		{
			return *It;
		}
		return nullptr;
	}

	FString Words(const TCHAR* Key)
	{
		return CosmicUI::Label(Key).ToString();
	}

	bool HasAsciiDigit(const FString& Text)
	{
		for (const TCHAR Char : Text)
		{
			if (Char >= TEXT('0') && Char <= TEXT('9'))
			{
				return true;
			}
		}
		return false;
	}

	// A scalar parameter of a material instance as it holds it now; -1 when it holds none.
	float ScalarOf(const UMaterialInterface* Material, const TCHAR* Name)
	{
		float Value = -1.f;
		if (!Material || !Material->GetScalarParameterValue(FHashedMaterialParameterInfo(FName(Name)), Value))
		{
			return -1.f;
		}
		return Value;
	}

	// The glow an instance carries now; -1 for an instance it does not have.
	float GlowOf(const UInstancedStaticMeshComponent& Mesh, int32 Instance)
	{
		const int32 At = Instance * Mesh.NumCustomDataFloats;
		return (Mesh.NumCustomDataFloats > 0 && Mesh.PerInstanceSMCustomData.IsValidIndex(At)) ? Mesh.PerInstanceSMCustomData[At] : -1.f;
	}

	// The verdict words of the schema (the one list, words.verdict).
	TArray<FString> VerdictWords()
	{
		TArray<FString> Out;
		FString Text;
		if (!FFileHelper::LoadFileToString(Text, *(FPaths::ProjectDir() / TEXT("Tools/data/schema.json"))))
		{
			return Out;
		}
		TSharedPtr<FJsonObject> Schema;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
		const TSharedPtr<FJsonObject>* WordLists = nullptr;
		const TArray<TSharedPtr<FJsonValue>>* Verdict = nullptr;
		if (FJsonSerializer::Deserialize(Reader, Schema) && Schema.IsValid() && Schema->TryGetObjectField(TEXT("words"), WordLists)
			&& WordLists && (*WordLists)->TryGetArrayField(TEXT("verdict"), Verdict) && Verdict)
		{
			for (const TSharedPtr<FJsonValue>& Value : *Verdict)
			{
				FString Word;
				if (Value.IsValid() && Value->TryGetString(Word))
				{
					Out.Add(Word);
				}
			}
		}
		return Out;
	}

	// Whether Text holds Word as a word of its own: a one-syllable word with no Hangul syllable on either side, a longer one
	// with none before it (as the checker's hedges read a word).
	bool HoldsWord(const FString& Text, const FString& Word)
	{
		auto IsHangul = [](TCHAR Char) { return Char >= 0xAC00 && Char <= 0xD7A3; };
		int32 From = 0;
		while (true)
		{
			const int32 At = Text.Find(Word, ESearchCase::CaseSensitive, ESearchDir::FromStart, From);
			if (At == INDEX_NONE)
			{
				return false;
			}
			const bool bBefore = At > 0 && IsHangul(Text[At - 1]);
			const int32 End = At + Word.Len();
			const bool bAfter = Word.Len() == 1 && End < Text.Len() && IsHangul(Text[End]);
			if (!bBefore && !bAfter)
			{
				return true;
			}
			From = At + 1;
		}
	}

	// The test's own camera of the start view (the pawn's FrameAll: pitch -59, yaw 0; the camera at the arm's end), its
	// projection of a world point to a 1280x720 screen (the horizontal field of view kept, as the engine's default aspect
	// constraint does) and the disc point under a screen point.
	struct FTestCamera
	{
		FVector Focus = FVector::ZeroVector;
		double Arm = 1.0;
		double Fov = 90.0;
		double Pitch = -59.0;
		double Width = 1280.0;
		double Height = 720.0;

		FVector Forward() const { return FVector(FMath::Cos(FMath::DegreesToRadians(Pitch)), 0.0, FMath::Sin(FMath::DegreesToRadians(Pitch))); }
		FVector Up() const { return FVector(-FMath::Sin(FMath::DegreesToRadians(Pitch)), 0.0, FMath::Cos(FMath::DegreesToRadians(Pitch))); }
		FVector Eye() const { return Focus - Forward() * Arm; }

		bool Project(const FVector& World, FVector2D& Out) const
		{
			const FVector V = World - Eye();
			const double Z = FVector::DotProduct(V, Forward());
			if (!(Z > 0.0))
			{
				return false;
			}
			const double T = FMath::Tan(FMath::DegreesToRadians(Fov * 0.5));
			Out.X = Width * 0.5 + (V.Y / Z) / T * (Width * 0.5);
			Out.Y = Height * 0.5 - (FVector::DotProduct(V, Up()) / Z) / T * (Width * 0.5);
			return true;
		}

		bool Ground(const FVector2D& Screen, FVector& Out) const
		{
			const double T = FMath::Tan(FMath::DegreesToRadians(Fov * 0.5));
			const double Nx = (Screen.X - Width * 0.5) / (Width * 0.5) * T;
			const double Ny = -(Screen.Y - Height * 0.5) / (Width * 0.5) * T;
			const FVector Direction = Forward() + Up() * Ny + FVector(0.0, Nx, 0.0);
			const FVector Eyes = Eye();
			if (!(Direction.Z < 0.0))
			{
				return false;
			}
			const double K = -Eyes.Z / Direction.Z;
			Out = Eyes + Direction * K;
			return true;
		}
	};
}

// ======================================================================================================================
// UI.ZoomLevels: the two levels of the one map and the wheel (pure), then on the game's camera pawn
// ======================================================================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIZoomLevelsTest, "Cosmic_Ascension.UI.ZoomLevels", CosmicTests::Flags)
bool FCosmicUIZoomLevelsTest::RunTest(const FString& Parameters)
{
	using namespace CosmicUniverseTests;

	const FCosmicZoomLevels L = CosmicZoom::LevelsOf(FVector(100.f, -50.f, 7.f), 10000.f, 80000.f);
	const FVector Centre(100.f, -50.f, 0.f);
	const FVector Away(30100.f, -50.f, 0.f);
	const float Near = 30000.f / 0.85f;

	// ---- 1 The region ----
	TestEqual(TEXT("The region's far stop is three disc radii"), L.RegionFar, 30000.f, 0.01f);
	TestEqual(TEXT("Its pan bound is 1.1 radii"), L.RegionBound, 11000.f, 0.01f);
	TestEqual(TEXT("Its near stop is 1,500"), L.RegionNear, 1500.f);
	TestEqual(TEXT("Its centre stands on the disc plane"), L.Centre.Z, 0.0);
	TestTrue(TEXT("on the disc's centre"), L.Centre.Equals(Centre, 0.001));
	TestEqual(TEXT("The region frames its radius"), L.RegionRadius, 10000.f);
	TestEqual(TEXT("It is entered over the home galaxy's light"), L.RegionEntry, 16000.f, 0.01f);
	TestEqual(TEXT("A disc smaller than 1,000 is framed as 1,000"), CosmicZoom::LevelsOf(Centre, 500.f, 80000.f).RegionFar, 3000.f, 0.01f);
	TestEqual(TEXT("A radius that is not a number frames the smallest disc"), CosmicZoom::LevelsOf(Centre, NaN, 80000.f).RegionFar, 3000.f, 0.01f);

	// ---- 2 The universe ----
	TestTrue(TEXT("(the hand levels open the universe)"), L.HasUniverse());
	TestEqual(TEXT("The universe begins one wheel step past the region's far stop"), L.UniverseNear, Near, 0.01f);
	TestEqual(TEXT("Its far stop is 1.25 times the reach"), L.UniverseFar, 100000.f, 0.01f);
	TestEqual(TEXT("Its pan bound is the reach"), L.UniverseBound, 80000.f, 0.01f);
	TestFalse(TEXT("A reach whose far stop stays short of the near stop opens no universe"), CosmicZoom::LevelsOf(Centre, 10000.f, 20000.f).HasUniverse());
	TestFalse(TEXT("A reach that is not a number opens none"), CosmicZoom::LevelsOf(Centre, 10000.f, NaN).HasUniverse());
	TestFalse(TEXT("No reach, no universe"), CosmicZoom::LevelsOf(Centre, 10000.f, 0.f).HasUniverse());
	const FCosmicZoomLevels Closed = CosmicZoom::LevelsOf(Centre, 10000.f, 0.f);

	// ---- 3 Levels and bounds ----
	TestTrue(TEXT("An arm at the region's far stop is the region"), CosmicZoom::LevelOf(30000.f, L) == ECosmicZoomLevel::Region);
	TestTrue(TEXT("One just past it is the universe"), CosmicZoom::LevelOf(30001.f, L) == ECosmicZoomLevel::Universe);
	TestTrue(TEXT("While the universe is closed, every arm is the region"), CosmicZoom::LevelOf(200000.f, Closed) == ECosmicZoomLevel::Region);
	TestTrue(TEXT("An arm that is not a number is the region"), CosmicZoom::LevelOf(NaN, L) == ECosmicZoomLevel::Region);
	TestEqual(TEXT("Each level has its own bound: the region's"), CosmicZoom::BoundOf(ECosmicZoomLevel::Region, L), 11000.f, 0.01f);
	TestEqual(TEXT("Each level has its own bound: the universe's"), CosmicZoom::BoundOf(ECosmicZoomLevel::Universe, L), 80000.f, 0.01f);
	TestEqual(TEXT("A closed universe has the region's bound"), CosmicZoom::BoundOf(ECosmicZoomLevel::Universe, Closed), 11000.f, 0.01f);
	TestTrue(TEXT("A focus on the centre enters the region"), CosmicZoom::EntersRegion(Centre, L));
	TestTrue(TEXT("and one just within the home's light"), CosmicZoom::EntersRegion(Centre + FVector(15999.f, 0.f, 0.f), L));
	TestFalse(TEXT("One past it does not"), CosmicZoom::EntersRegion(Centre + FVector(16001.f, 0.f, 0.f), L));
	TestFalse(TEXT("nor one that is not a number"), CosmicZoom::EntersRegion(FVector(NaN, 0.f, 0.f), L));
	// The expectation table walks every level (an enumerator added at the end is no level without an entry here).
	{
		const float Expected[] = { 11000.f, 80000.f };
		static_assert(UE_ARRAY_COUNT(Expected) == static_cast<int32>(ECosmicZoomLevel::Count), "one bound per zoom level");
		for (int32 At = 0; At < static_cast<int32>(ECosmicZoomLevel::Count); ++At)
		{
			TestEqual(*FString::Printf(TEXT("Level %d has its bound"), At), CosmicZoom::BoundOf(static_cast<ECosmicZoomLevel>(At), L), Expected[At], 0.01f);
		}
	}

	// ---- 4 The wheel ----
	TestEqual(TEXT("One step in comes nearer by x0.85"), CosmicZoom::StepArm(20000.f, 1, L, Centre), 17000.f, 0.01f);
	TestEqual(TEXT("One step out goes away by the inverse"), CosmicZoom::StepArm(20000.f, -1, L, Centre), 20000.f / 0.85f, 0.01f);
	TestEqual(TEXT("Steps in stop at the near stop"), CosmicZoom::ArmAfter(2000.f, 5.f, L, Centre), 1500.f);
	TestEqual(TEXT("With the universe closed, steps out stop at the region's far stop"), CosmicZoom::ArmAfter(20000.f, -100.f, Closed, Centre), 30000.f, 0.01f);
	TestEqual(TEXT("From the region's far stop one step out comes to the universe's near stop"), CosmicZoom::StepArm(30000.f, -1, L, Centre), Near, 0.01f);
	TestEqual(TEXT("and one step in from there comes back over the home galaxy"), CosmicZoom::StepArm(Near, 1, L, Centre), 30000.f, 0.01f);
	TestEqual(TEXT("Away from the home galaxy a step in stays at the universe's near stop"), CosmicZoom::StepArm(Near, 1, L, Away), Near, 0.01f);
	TestEqual(TEXT("Steps out stop at the universe's far stop"), CosmicZoom::ArmAfter(30000.f, -100.f, L, Centre), 100000.f, 0.01f);
	TestEqual(TEXT("Many steps in from there end at the region's near stop"), CosmicZoom::ArmAfter(100000.f, 100.f, L, Centre), 1500.f);
	TestEqual(TEXT("Many steps in away from the home galaxy end at the universe's near stop"), CosmicZoom::ArmAfter(100000.f, 100.f, L, Away), Near, 0.01f);
	TestEqual(TEXT("An arm between the levels steps nearer to the region's far stop"), CosmicZoom::StepArm(32000.f, 1, L, Centre), 30000.f, 0.01f);
	TestTrue(TEXT("and away to the universe"), CosmicZoom::StepArm(32000.f, -1, L, Centre) >= Near - 0.01f);
	{
		// The sweep: no step comes to rest strictly between the levels - from either level, near the home or away.
		int32 Between = 0;
		int32 FromRegion = 0;
		int32 FromUniverse = 0;
		int32 Crossings = 0;
		TArray<float> Arms;
		const float Low = L.RegionNear;
		const float High = L.UniverseFar;
		for (int32 At = 0; At < 200; ++At)
		{
			Arms.Add(Low * FMath::Pow(High / Low, At / 199.f));
		}
		Arms.Append({ L.RegionFar - 1.f, L.RegionFar + 0.01f, L.UniverseNear - 0.1f });
		for (const float Arm : Arms)
		{
			const bool bRegion = CosmicZoom::LevelOf(Arm, L) == ECosmicZoomLevel::Region;
			FromRegion += bRegion ? 1 : 0;
			FromUniverse += bRegion ? 0 : 1;
			for (const int32 Direction : { 1, -1 })
			{
				for (const FVector& Focus : { Centre, Away })
				{
					const float Next = CosmicZoom::StepArm(Arm, Direction, L, Focus);
					Between += (Next > L.RegionFar && Next < L.UniverseNear) ? 1 : 0;
					Crossings += (CosmicZoom::LevelOf(Next, L) == ECosmicZoomLevel::Region) != bRegion ? 1 : 0;
				}
			}
		}
		TestTrue(TEXT("(the sweep starts from the region)"), FromRegion >= 1);
		TestTrue(TEXT("(and from the universe)"), FromUniverse >= 1);
		TestTrue(TEXT("(some steps cross between the levels)"), Crossings >= 2);
		TestEqual(TEXT("No step comes to rest between the levels"), Between, 0);
	}
	TestEqual(TEXT("Half a step and less moves nothing"), CosmicZoom::ArmAfter(20000.f, 0.4f, L, Centre), 20000.f);
	TestEqual(TEXT("A step that is not a number moves nothing"), CosmicZoom::ArmAfter(20000.f, NaN, L, Centre), 20000.f);
	TestEqual(TEXT("A huge number of steps ends at the far stop"), CosmicZoom::ArmAfter(1500.f, -1.0e9f, L, Centre), 100000.f, 0.01f);
	TestEqual(TEXT("An arm that is not a positive number stays"), CosmicZoom::StepArm(-5.f, 1, L, Centre), -5.f);

	// ---- 5 The share ----
	TestEqual(TEXT("In the region the share is 0"), CosmicZoom::UniverseShareOf(30000.f, L), 0.f);
	TestEqual(TEXT("At the universe's near stop it is 1"), CosmicZoom::UniverseShareOf(Near, L), 1.f);
	TestEqual(TEXT("Half way, a half"), CosmicZoom::UniverseShareOf(0.5f * (30000.f + Near), L), 0.5f, 0.01f);
	TestEqual(TEXT("Past the near stop it stays 1"), CosmicZoom::UniverseShareOf(100000.f, L), 1.f);
	TestEqual(TEXT("A closed universe has no share"), CosmicZoom::UniverseShareOf(200000.f, Closed), 0.f);
	TestEqual(TEXT("An arm that is not a number has none"), CosmicZoom::UniverseShareOf(NaN, L), 0.f);
	TestTrue(TEXT("The region's detail is drawn below the half"), CosmicZoom::ShowsRegionDetail(0.49f));
	TestFalse(TEXT("and not from the half on"), CosmicZoom::ShowsRegionDetail(0.5f));
	TestFalse(TEXT("nor at the universe"), CosmicZoom::ShowsRegionDetail(1.f));
	TestTrue(TEXT("A share that is not a number draws it"), CosmicZoom::ShowsRegionDetail(NaN));

	// ---- 6 The game's pawn ----
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UWorld* World = Sim->GetGameInstance()->GetWorld();
	ACosmicGalaxyMapActor* Map = FindMap(World);
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(World ? World->GetFirstPlayerController() : nullptr);
	ACosmicCameraPawn* Pawn = Controller ? Cast<ACosmicCameraPawn>(Controller->GetPawn()) : nullptr;
	if (!TestNotNull(TEXT("(the game mode spawned the map)"), Map) || !TestNotNull(TEXT("(and a camera pawn)"), Pawn))
	{
		return false;
	}
	ON_SCOPE_EXIT
	{
		Sim->NewGalaxy(42, 5, 3);
		Map->RefreshNow();
	};
	Sim->NewGalaxy(42, 0, 0);
	Map->RefreshNow();
	const FCosmicZoomLevels& Mine = Map->GetZoomLevels();
	const FCosmicZoomLevels& Its = Pawn->GetZoomLevels();
	TestTrue(TEXT("(a rebuild opens the universe)"), Its.HasUniverse());
	TestTrue(TEXT("The camera's levels are the map's: the centre"), Its.Centre.Equals(Mine.Centre, 0.01));
	TestEqual(TEXT("The camera's levels are the map's: the region's radius"), Its.RegionRadius, Mine.RegionRadius, 0.01f);
	TestEqual(TEXT("The camera's levels are the map's: the region's near stop"), Its.RegionNear, Mine.RegionNear, 0.01f);
	TestEqual(TEXT("The camera's levels are the map's: the region's far stop"), Its.RegionFar, Mine.RegionFar, 0.01f);
	TestEqual(TEXT("The camera's levels are the map's: the region's bound"), Its.RegionBound, Mine.RegionBound, 0.01f);
	TestEqual(TEXT("The camera's levels are the map's: the universe's near stop"), Its.UniverseNear, Mine.UniverseNear, 0.01f);
	TestEqual(TEXT("The camera's levels are the map's: the universe's far stop"), Its.UniverseFar, Mine.UniverseFar, 0.01f);
	TestEqual(TEXT("The camera's levels are the map's: the universe's bound"), Its.UniverseBound, Mine.UniverseBound, 0.01f);
	if (!Its.HasUniverse())
	{
		return false;
	}
	const FVector Home = Its.Centre;
	const float R = Its.RegionRadius;
	Pawn->Zoom(-2.f);
	TestEqual(TEXT("Two steps out from the framing reach the region's far stop"), Pawn->GetTargetArmLength(), Its.RegionFar, 0.5f);
	Pawn->Zoom(-1.f);
	TestEqual(TEXT("One step more comes to the universe's near stop"), Pawn->GetTargetArmLength(), Its.UniverseNear, 0.5f);
	Controller->FollowZoomLevel();
	TestTrue(TEXT("(the controller's level follows)"), Controller->GetZoomLevel() == ECosmicZoomLevel::Universe);
	Pawn->Pan(FVector(100.f * R, 0.f, 0.f));
	TestEqual(TEXT("At the universe level a far pan stops at the universe's bound"), FVector::Dist2D(Pawn->GetTargetFocus(), Home), static_cast<double>(Its.UniverseBound), 0.5);
	Pawn->Zoom(1.f);
	TestEqual(TEXT("Away from the home galaxy one step in stays at the universe's near stop"), Pawn->GetTargetArmLength(), Its.UniverseNear, 0.5f);
	// A focus within the home's light but past the region's bound: one step in enters and pulls the focus home.
	Pawn->Pan(Home + FVector(1.3 * R, 0.0, 0.0) - Pawn->GetTargetFocus());
	TestEqual(TEXT("(the focus stands within the home's light, past the region's bound)"), FVector::Dist2D(Pawn->GetTargetFocus(), Home), 1.3 * R, 1.0);
	Pawn->Zoom(1.f);
	TestEqual(TEXT("One step in over the home's light returns to the region's far stop"), Pawn->GetTargetArmLength(), Its.RegionFar, 0.5f);
	TestTrue(TEXT("and pulls the focus within the region's bound"), FVector::Dist2D(Pawn->GetTargetFocus(), Home) <= Its.RegionBound + 0.5);
	// A galaxy's portrait: the wheel in keeps it.
	const FVector Galaxy = Home + FVector(5.0 * R, 0.0, 0.0);
	TestTrue(TEXT("Focusing a galaxy is taken while the universe is open"), Pawn->FocusOnUniverse(Galaxy, 7.f * R));
	TestEqual(TEXT("Focusing a galaxy comes to its arm"), Pawn->GetTargetArmLength(), 7.f * R, 0.5f);
	TestTrue(TEXT("and aims at it"), Pawn->GetTargetFocus().Equals(Galaxy, 0.5));
	Pawn->Zoom(10.f);
	TestEqual(TEXT("From a galaxy's portrait the wheel in stays at the universe's near stop"), Pawn->GetTargetArmLength(), Its.UniverseNear, 0.5f);
	TestTrue(TEXT("with the focus on the galaxy"), Pawn->GetTargetFocus().Equals(Galaxy, 0.5));
	Pawn->FocusOnUniverse(Galaxy, 1.0e9f);
	TestEqual(TEXT("A galaxy's arm keeps to the universe's far stop"), Pawn->GetTargetArmLength(), Its.UniverseFar, 0.5f);
	Pawn->FocusOnUniverse(Galaxy, 1.f);
	TestEqual(TEXT("and to its near stop"), Pawn->GetTargetArmLength(), Its.UniverseNear, 0.5f);
	const int32 SolIndex = Sim->GetState().Systems.IndexOfByPredicate([](const FStarSystemState& System) { return System.SystemID == FName(TEXT("sol")); });
	if (TestTrue(TEXT("(the game's galaxy holds sol)"), SolIndex != INDEX_NONE))
	{
		const FVector Sol = Sim->GetState().Systems[SolIndex].Position;
		Pawn->FocusOn(Sol);
		TestEqual(TEXT("Focusing a star from the universe comes to arm 3,000"), Pawn->GetTargetArmLength(), 3000.f);
		TestTrue(TEXT("and into the region"), Pawn->GetTargetLevel() == ECosmicZoomLevel::Region);
	}
	Pawn->Zoom(-100.f);
	Pawn->FocusOn(Home + FVector(3.0 * R, 0.0, 0.0));
	TestTrue(TEXT("A focus from the universe is held within the region's bound"), FVector::Dist2D(Pawn->GetTargetFocus(), Home) <= Its.RegionBound + 0.5);
	{
		ACosmicCameraPawn* Fresh = World->SpawnActor<ACosmicCameraPawn>();
		if (TestNotNull(TEXT("(a fresh camera pawn)"), Fresh))
		{
			const float Arm = Fresh->GetTargetArmLength();
			TestFalse(TEXT("A closed universe refuses a galaxy's focus"), Fresh->FocusOnUniverse(Galaxy, 7.f * R));
			TestEqual(TEXT("and changes nothing"), Fresh->GetTargetArmLength(), Arm);
			Fresh->Destroy();
		}
	}
	return true;
}

// ======================================================================================================================
// UI.UniverseLook: what the universe draws and what the right side shows (pure; tables and frames built by the test)
// ======================================================================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIUniverseLookTest, "Cosmic_Ascension.UI.UniverseLook", CosmicTests::Flags)
bool FCosmicUIUniverseLookTest::RunTest(const FString& Parameters)
{
	using namespace CosmicUniverseTests;
	using namespace CosmicUniverseLook;

	const FCosmicUniverseFrame F = FrameOf(FVector(300.f, -200.f, 9.f), 5000.f);
	const float LawRadius = static_cast<float>(HomeLawRadiusLy);
	const int32 BigLy = static_cast<int32>(FMath::RoundToInt(2.25f * LawRadius));

	// The board's table, rows added out of lexical order on purpose.
	UDataTable* Table = CosmicTests::NewTable(FGalaxyDef::StaticStruct());
	Table->AddRow(FName(TEXT("gal_test_small")), CosmicTests::GalaxyRowOf(TEXT("Small"), TEXT("Galaxy.Kind.Dwarf"), 0, FVector2D(0.0, -2.0)));
	Table->AddRow(FName(TEXT("gal_test_home")), CosmicTests::GalaxyRowOf(TEXT("Home"), TEXT("Galaxy.Kind.Spiral"), 50000, FVector2D(3.0, 4.0)));
	Table->AddRow(FName(TEXT("gal_test_big")), CosmicTests::GalaxyRowOf(TEXT("BiG Marker"), TEXT("Galaxy.Kind.GrandSpiral"), BigLy, FVector2D(6.0, 0.0)));
	Table->AddRow(FName(TEXT("gal_test_unplaced")), CosmicTests::GalaxyRowOf(TEXT("Unplaced"), TEXT("Galaxy.Kind.Spiral"), 0, FVector2D(0.0, 0.0)));
	Table->AddRow(FName(TEXT("gal_test_nan")), CosmicTests::GalaxyRowOf(TEXT("Nan"), TEXT("Galaxy.Kind.Spiral"), 0, FVector2D(NaN, 1.0)));
	Table->AddRow(FName(TEXT("gal_test_far")), CosmicTests::GalaxyRowOf(TEXT("Far"), TEXT("Galaxy.Kind.Spiral"), 0, FVector2D(100.0, 0.0)));
	Table->AddRow(FName(TEXT("gal_test_inside")), CosmicTests::GalaxyRowOf(TEXT("Inside"), TEXT("Galaxy.Kind.Spiral"), 0, FVector2D(0.5, 0.0)));
	Table->AddRow(FName(TEXT("gal_test_odd")), CosmicTests::GalaxyRowOf(TEXT("Odd"), TEXT("Hazard.Plague"), 0, FVector2D(-5.0, 0.0)));
	FCosmicDataTables T;
	T.Galaxies = Table;
	const FName HomeRow(TEXT("gal_test_home"));

	// ---- 1 The frame ----
	TestTrue(TEXT("The universe's centre is the disc's"), FVector2D(F.Centre.X, F.Centre.Y).Equals(FVector2D(300.0, -200.0), 0.001));
	TestEqual(TEXT("on the disc plane"), F.Centre.Z, 0.0);
	TestEqual(TEXT("Its unit is the disc radius"), F.Unit, 5000.f);
	TestEqual(TEXT("A disc smaller than 1,000 is taken as 1,000"), FrameOf(FVector::ZeroVector, 400.f).Unit, 1000.f);
	TestEqual(TEXT("and one that is not a number"), FrameOf(FVector::ZeroVector, NaN).Unit, 1000.f);

	// ---- 2 Places ----
	TestTrue(TEXT("A place is MapPosition home radii from the centre"), PlaceOf(F, FVector2D(6.0, 0.0)).Equals(FVector(30300.0, -200.0, 0.0), 0.01));
	TestTrue(TEXT("in the map's own X and Y"), PlaceOf(F, FVector2D(0.0, -2.0)).Equals(FVector(300.0, -10200.0, 0.0), 0.01));
	TestTrue(TEXT("The home always has its place"), HasPlace(FVector2D(3.0, 4.0), true));
	TestTrue(TEXT("The home has its place at (0,0) too"), HasPlace(FVector2D(0.0, 0.0), true));
	TestFalse(TEXT("A row left at (0,0) has no place yet"), HasPlace(FVector2D(0.0, 0.0), false));
	TestFalse(TEXT("A place that is not a number is none"), HasPlace(FVector2D(NaN, 1.0), false));
	TestFalse(TEXT("A place beyond the universe is none"), HasPlace(FVector2D(100.0, 0.0), false));
	TestFalse(TEXT("A place inside the home galaxy is none"), HasPlace(FVector2D(0.5, 0.0), false));
	TestTrue(TEXT("A place just outside the home galaxy is one"), HasPlace(FVector2D(1.5, 0.0), false));
	TestTrue(TEXT("and one at the universe's edge"), HasPlace(FVector2D(20.0, 0.0), false));

	// ---- 3 The law of every galaxy ----
	{
		FGalaxyDef Row = CosmicTests::GalaxyRowOf(TEXT("x"), TEXT("Galaxy.Kind.Spiral"), HomeLawRadiusLy, FVector2D(2.0, 0.0));
		TestEqual(TEXT("A galaxy of the home's own radius is drawn at the home sprite's share"), SpriteShareOf(Row), HomeSpriteShare, 0.001f);
		Row.RadiusLy = BigLy;
		TestEqual(TEXT("A galaxy 2.25 times the home's radius is drawn 1.5 times as large"), SpriteShareOf(Row), 1.5f * HomeSpriteShare, 0.001f);
		Row.RadiusLy = 5000000;
		TestEqual(TEXT("A huge one stops at the largest sprite"), SpriteShareOf(Row), MaxSpriteShare, 0.001f);
		Row.RadiusLy = 10;
		TestEqual(TEXT("A tiny one at the smallest"), SpriteShareOf(Row), MinSpriteShare, 0.001f);
		const FGalaxyDef Dwarf = CosmicTests::GalaxyRowOf(TEXT("x"), TEXT("Galaxy.Kind.Dwarf"), 0, FVector2D(2.0, 0.0));
		TestEqual(TEXT("A row without a radius takes its kind's"), SpriteShareOf(Dwarf), KindLookOf(Tag(TEXT("Galaxy.Kind.Dwarf"))).DefaultShare, 0.001f);
		const FGalaxyDef Odd = CosmicTests::GalaxyRowOf(TEXT("x"), TEXT("Hazard.Plague"), 0, FVector2D(2.0, 0.0));
		TestEqual(TEXT("A kind the table does not know takes the fallback"), SpriteShareOf(Odd), 0.45f, 0.001f);
		TestFalse(TEXT("and is not a known kind"), KindLookOf(Tag(TEXT("Hazard.Plague"))).bKnown);
		int32 Drops = 0;
		float Last = 0.f;
		for (int32 Step = 1; Step <= 12; ++Step)
		{
			Row.RadiusLy = Step * 15000;
			const float Share = SpriteShareOf(Row);
			Drops += Share < Last ? 1 : 0;
			Last = Share;
		}
		TestEqual(TEXT("Sizes grow with the radius"), Drops, 0);
	}
	{
		UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		const FGameplayTagContainer Children = Manager.RequestGameplayTagChildren(Tag(TEXT("Galaxy.Kind")));
		TArray<FGameplayTag> Kinds;
		Children.GetGameplayTagArray(Kinds);
		TestTrue(TEXT("(the tags hold the galaxy kinds)"), Kinds.Num() >= 4);
		for (const FGameplayTag& Kind : Kinds)
		{
			TestTrue(*FString::Printf(TEXT("Every galaxy kind of the tags has its look: %s"), *Kind.ToString()), KindLookOf(Kind).bKnown);
		}
		for (int32 A = 0; A < Kinds.Num(); ++A)
		{
			for (int32 B = A + 1; B < Kinds.Num(); ++B)
			{
				const FCosmicKindLook LookA = KindLookOf(Kinds[A]);
				const FCosmicKindLook LookB = KindLookOf(Kinds[B]);
				TestTrue(*FString::Printf(TEXT("Kinds %s and %s have different looks"), *Kinds[A].ToString(), *Kinds[B].ToString()),
					LookA.Aspect != LookB.Aspect || LookA.DefaultShare != LookB.DefaultShare);
			}
		}
		TestTrue(TEXT("An elliptical is drawn rounder than a spiral"), KindLookOf(Tag(TEXT("Galaxy.Kind.Elliptical"))).Aspect > KindLookOf(Tag(TEXT("Galaxy.Kind.Spiral"))).Aspect);
	}
	{
		const FName Big(TEXT("gal_test_big"));
		const float Expected = static_cast<float>(CosmicSeed::DeriveForName(0, Big, CosmicSeed::Purpose::Scenery, 1) >> 8) * (360.f / 16777216.f);
		TestEqual(TEXT("A named galaxy's turn is its own derive"), TurnDegreesOf(Big), Expected, 0.0001f);
		TestTrue(TEXT("Two galaxies turn apart"), TurnDegreesOf(Big) != TurnDegreesOf(FName(TEXT("gal_test_small"))));
		TestTrue(TEXT("and the turn lies in the circle"), TurnDegreesOf(Big) >= 0.f && TurnDegreesOf(Big) < 360.f);
	}

	// ---- 4 The marks ----
	const TArray<FCosmicGalaxyMark> Marks = MarksOf(T, HomeRow, F);
	TestEqual(TEXT("Every placed row is drawn, the home among them"), Marks.Num(), 4);
	{
		TArray<FName> Rows;
		for (const FCosmicGalaxyMark& Mark : Marks)
		{
			Rows.Add(Mark.Row);
		}
		TArray<FName> Sorted = Rows;
		Sorted.Sort(FNameLexicalLess());
		bool bSame = Sorted.Num() == Rows.Num();
		for (int32 At = 0; bSame && At < Rows.Num(); ++At)
		{
			bSame = Rows[At].ToString().Equals(Sorted[At].ToString(), ESearchCase::CaseSensitive);
		}
		TestTrue(TEXT("in lexical order of their rows"), bSame);
	}
	const FCosmicGalaxyMark* HomeMark = Marks.FindByPredicate([](const FCosmicGalaxyMark& Mark) { return Mark.bHome; });
	const FCosmicGalaxyMark* BigMark = Marks.FindByPredicate([](const FCosmicGalaxyMark& Mark) { return Mark.Row == FName(TEXT("gal_test_big")); });
	if (TestNotNull(TEXT("(the home is a mark)"), HomeMark) && TestNotNull(TEXT("(the big row is a mark)"), BigMark))
	{
		TestTrue(TEXT("The home stands at the centre, whatever its MapPosition holds"), HomeMark->Position.Equals(F.Centre, 0.01));
		TestEqual(TEXT("and is drawn round"), HomeMark->Aspect, 1.f);
		TestEqual(TEXT("and unturned"), HomeMark->TurnDegrees, 0.f);
		TestEqual(TEXT("at the home sprite's share"), HomeMark->Radius, HomeSpriteShare * F.Unit, 0.01f);
		TestTrue(TEXT("A row stands at its place"), BigMark->Position.Equals(PlaceOf(F, FVector2D(6.0, 0.0)), 0.01));
		TestEqual(TEXT("at its size"), BigMark->Radius, 1.5f * HomeSpriteShare * F.Unit, 0.5f);
		TestEqual(TEXT("with its kind's aspect"), BigMark->Aspect, KindLookOf(Tag(TEXT("Galaxy.Kind.GrandSpiral"))).Aspect);
		TestTrue(TEXT("A mark's name is its DisplayName as written"), BigMark->Name.ToString().Equals(TEXT("BiG Marker"), ESearchCase::CaseSensitive));
		bool bOneGlow = true;
		for (const FCosmicGalaxyMark& Mark : Marks)
		{
			bOneGlow &= Mark.Glow == NamedGlow;
		}
		TestTrue(TEXT("Every named galaxy has the same glow"), bOneGlow);
	}
	{
		const TArray<FName> Unplaced = UnplacedRowsOf(T, HomeRow);
		TArray<FName> Want = { FName(TEXT("gal_test_far")), FName(TEXT("gal_test_inside")), FName(TEXT("gal_test_nan")), FName(TEXT("gal_test_unplaced")) };
		Want.Sort(FNameLexicalLess());
		TestTrue(TEXT("UnplacedRowsOf names the rows without a place"), Unplaced == Want);
		TestFalse(TEXT("and never the home"), Unplaced.Contains(HomeRow));
	}
	{
		const TArray<FCosmicGalaxyMark> Missing = MarksOf(T, FName(TEXT("gal_test_missing")), F);
		const FCosmicGalaxyMark* Lone = Missing.FindByPredicate([](const FCosmicGalaxyMark& Mark) { return Mark.bHome; });
		TestTrue(TEXT("Without its row, the home still stands at the centre"), Lone && Lone->Position.Equals(F.Centre, 0.01) && Lone->Row == FName(TEXT("gal_test_missing")));
		TestTrue(TEXT("at the home's size"), Lone && FMath::IsNearlyEqual(Lone->Radius, HomeSpriteShare * F.Unit, 0.01f));
		TestTrue(TEXT("with no name"), Lone && Lone->Name.IsEmpty());
		UDataTable* Empty = CosmicTests::NewTable(FGalaxyDef::StaticStruct());
		FCosmicDataTables None;
		None.Galaxies = Empty;
		TestEqual(TEXT("A table without rows draws the home alone"), MarksOf(None, HomeRow, F).Num(), 1);
		TestEqual(TEXT("No table draws the home alone"), MarksOf(FCosmicDataTables(), HomeRow, F).Num(), 1);
	}

	// ---- 5 The scenery ----
	{
		FCosmicRulesDef Rules;
		Rules.SceneryGalaxyCount = 5000;
		TestEqual(TEXT("SceneryCountOf reads a count"), SceneryCountOf(Rules), 5000);
		Rules.SceneryGalaxyCount = 0;
		TestEqual(TEXT("SceneryCountOf reads no count as none"), SceneryCountOf(Rules), 0);
		Rules.SceneryGalaxyCount = -1;
		TestEqual(TEXT("SceneryCountOf reads a negative count as none"), SceneryCountOf(Rules), 0);
		Rules.SceneryGalaxyCount = 1000000;
		TestEqual(TEXT("A count above the most reads as the most"), SceneryCountOf(Rules), MaxSceneryPoints);
	}
	const TArray<FCosmicSceneryPoint> Points = SceneryPointsOf(HomeRow, 2000, F);
	TestEqual(TEXT("2000 points"), Points.Num(), 2000);
	// The thin bands at the edges, on a longer list (the inner band holds about one point in 800).
	const TArray<FCosmicSceneryPoint> Many = SceneryPointsOf(HomeRow, 40000, F);
	{
		int32 Outside = 0;
		int32 OffPlane = 0;
		int32 InQuarter = 0;
		int32 InnerBand = 0;
		int32 OuterBand = 0;
		int32 InnerHigh = 0;
		int32 OuterHigh = 0;
		int32 GroupHigh = 0;
		int32 GroupBig = 0;
		int32 GroupBright = 0;
		int32 OutOfRange = 0;
		const double QuarterRadius = FMath::Sqrt(SceneryInnerUnits * SceneryInnerUnits + 0.25 * (SceneryOuterUnits * SceneryOuterUnits - SceneryInnerUnits * SceneryInnerUnits));
		// The fades by hand (the test does not ask the law it checks).
		auto InnerFade = [](double R) { return FMath::Clamp((R - SceneryInnerUnits) / SceneryInnerFadeUnits, 0.0, 1.0); };
		auto OuterFade = [](double R) { return FMath::Clamp((SceneryOuterUnits - R) / SceneryOuterFadeUnits, 0.0, 1.0); };
		for (const FCosmicSceneryPoint& Point : Points)
		{
			const double R = FVector::Dist2D(Point.Position, F.Centre) / F.Unit;
			Outside += (R < SceneryInnerUnits - 0.01 || R > SceneryOuterUnits + 0.01) ? 1 : 0;
			OffPlane += Point.Position.Z != 0.0 ? 1 : 0;
			InQuarter += R < QuarterRadius ? 1 : 0;
			OutOfRange += (Point.Glow < 0.f || Point.Glow > SceneryMaxGlow) ? 1 : 0;
		}
		for (const FCosmicSceneryPoint& Point : Many)
		{
			const double R = FVector::Dist2D(Point.Position, F.Centre) / F.Unit;
			if (R < SceneryInnerUnits + SceneryInnerFadeUnits)
			{
				++InnerBand;
				InnerHigh += Point.Glow > SceneryMaxGlow * InnerFade(R) * LocalGroupGlowShare + 1e-4 ? 1 : 0;
			}
			if (R > SceneryOuterUnits - SceneryOuterFadeUnits)
			{
				++OuterBand;
				OuterHigh += Point.Glow > SceneryMaxGlow * OuterFade(R) + 1e-4 ? 1 : 0;
			}
			if (R < LocalGroupUnits)
			{
				GroupHigh += Point.Glow > SceneryMaxGlow * LocalGroupGlowShare + 1e-4f ? 1 : 0;
				GroupBig += Point.Size > SceneryMaxSizeUnits * F.Unit * LocalGroupSizeShare + 0.01f ? 1 : 0;
				// Past the inner fade the band is dimmed by its share alone: some of its points glow above half of it.
				GroupBright += (R > SceneryInnerUnits + SceneryInnerFadeUnits && Point.Glow > 0.5f * SceneryMaxGlow * LocalGroupGlowShare) ? 1 : 0;
			}
		}
		TestEqual(TEXT("Every point lies in the scenery's ring"), Outside, 0);
		TestEqual(TEXT("on the disc plane"), OffPlane, 0);
		const double Share = InQuarter / 2000.0;
		TestTrue(TEXT("The points fill the scenery evenly"), Share > 0.21 && Share < 0.29);
		TestEqual(TEXT("Every glow lies within the most"), OutOfRange, 0);
		TestTrue(TEXT("(some points lie in the inner band)"), InnerBand > 0);
		TestTrue(TEXT("(some points lie in the outer band)"), OuterBand > 0);
		TestEqual(TEXT("No point inside the inner fade glows above its share"), InnerHigh, 0);
		TestEqual(TEXT("No point inside the outer fade glows above its share"), OuterHigh, 0);
		TestEqual(TEXT("The Local Group's band glows fainter"), GroupHigh, 0);
		TestEqual(TEXT("and smaller"), GroupBig, 0);
		TestTrue(TEXT("but not dark: some of the band's points glow above half its share"), GroupBright > 0);
	}
	{
		// The two laws at points computed by hand (the review of item 9 - the bands above bound them from one side only):
		// the fade in over 2 - 2.5 units and out over 36 - 42, the Local Group's shares to 6.5 units.
		TestEqual(TEXT("SceneryFadeOf at the inner edge is 0"), SceneryFadeOf(2.f), 0.f, 1e-5f);
		TestEqual(TEXT("SceneryFadeOf halfway into the inner fade (2.25) is 0.5"), SceneryFadeOf(2.25f), 0.5f, 1e-5f);
		TestEqual(TEXT("SceneryFadeOf past the inner fade (2.5) is 1"), SceneryFadeOf(2.5f), 1.f, 1e-5f);
		TestEqual(TEXT("SceneryFadeOf in the middle (10) is 1"), SceneryFadeOf(10.f), 1.f, 1e-5f);
		TestEqual(TEXT("SceneryFadeOf where the outer fade begins (36) is 1"), SceneryFadeOf(36.f), 1.f, 1e-5f);
		TestEqual(TEXT("SceneryFadeOf halfway into the outer fade (39) is 0.5"), SceneryFadeOf(39.f), 0.5f, 1e-5f);
		TestEqual(TEXT("SceneryFadeOf at the outer edge (42) is 0"), SceneryFadeOf(42.f), 0.f, 1e-5f);
		TestEqual(TEXT("LocalGroupShareOf inside the band (6.4) dims the glow to 0.4"), LocalGroupShareOf(6.4f, true), 0.4f, 1e-6f);
		TestEqual(TEXT("and the size to 0.6"), LocalGroupShareOf(6.4f, false), 0.6f, 1e-6f);
		TestEqual(TEXT("LocalGroupShareOf past the band (6.6) keeps the glow"), LocalGroupShareOf(6.6f, true), 1.f, 1e-6f);
		TestEqual(TEXT("and the size"), LocalGroupShareOf(6.6f, false), 1.f, 1e-6f);
	}
	{
		// The law itself, on a stream of the test: five draws a point.
		FRandomStream Stream(1234);
		TArray<float> Sizes;
		TArray<float> Glows;
		int32 Round = 0;
		int32 Flat = 0;
		int32 SizeOut = 0;
		int32 GlowOut = 0;
		int32 AspectOut = 0;
		int32 TurnOut = 0;
		for (int32 At = 0; At < 2000; ++At)
		{
			FCosmicSceneryPoint Point;
			SceneryLookOf(Stream, 1000.f, Point);
			Sizes.Add(Point.Size);
			Glows.Add(Point.Glow);
			SizeOut += (Point.Size < SceneryMinSizeUnits * 1000.f - 0.01f || Point.Size > SceneryMaxSizeUnits * 1000.f + 0.01f) ? 1 : 0;
			GlowOut += (Point.Glow < SceneryMinGlow - 1e-5f || Point.Glow > SceneryMaxGlow + 1e-5f) ? 1 : 0;
			const bool bRound = Point.Aspect >= SceneryRoundAspectMin;
			Round += bRound ? 1 : 0;
			Flat += bRound ? 0 : 1;
			AspectOut += (Point.Aspect < SceneryFlatAspectMin - 1e-5f || Point.Aspect > 1.f + 1e-5f || (Point.Aspect > SceneryFlatAspectMax + 1e-5f && Point.Aspect < SceneryRoundAspectMin - 1e-5f)) ? 1 : 0;
			TurnOut += (Point.TurnDegrees < 0.f || Point.TurnDegrees > 360.f) ? 1 : 0;
		}
		Sizes.Sort();
		Glows.Sort();
		TestEqual(TEXT("Every point's size lies in the law's range"), SizeOut, 0);
		TestEqual(TEXT("its glow"), GlowOut, 0);
		TestEqual(TEXT("its aspect"), AspectOut, 0);
		TestEqual(TEXT("its turn"), TurnOut, 0);
		TestTrue(TEXT("Most points are small"), Sizes[1000] < 1000.f * (SceneryMinSizeUnits + 0.4f * (SceneryMaxSizeUnits - SceneryMinSizeUnits)));
		TestTrue(TEXT("Most points are faint"), Glows[1000] < SceneryMinGlow + 0.3f * (SceneryMaxGlow - SceneryMinGlow));
		TestTrue(TEXT("Round points stand in the scenery"), Round >= 1);
		TestTrue(TEXT("and flat ones"), Flat >= 1);
	}
	{
		// Exactly five draws a look, whatever they give: 200 looks on streams of the test, each against a stream moved by five.
		int32 Drift = 0;
		for (int32 Seed = 1; Seed <= 200; ++Seed)
		{
			FRandomStream Looked(Seed * 7919);
			FRandomStream Counted(Seed * 7919);
			FCosmicSceneryPoint Point;
			SceneryLookOf(Looked, 1000.f, Point);
			for (int32 Draw = 0; Draw < 5; ++Draw)
			{
				Counted.GetFraction();
			}
			Drift += Looked.GetCurrentSeed() == Counted.GetCurrentSeed() ? 0 : 1;
		}
		TestEqual(TEXT("A scenery look takes exactly five draws"), Drift, 0);
	}
	{
		// The stream: the Scenery derive of the home row, no run seed - exactly the place's draws and five look draws a point.
		FRandomStream Stream;
		Stream.Initialize(static_cast<int32>(CosmicSeed::DeriveForName(0, HomeRow, CosmicSeed::Purpose::Scenery, 0)));
		bool bSame = Points.Num() >= 2;
		for (int32 At = 0; bSame && At < 2; ++At)
		{
			FCosmicSceneryPoint Point;
			const FVector2D Place = AnnulusPointOf(Stream, SceneryInnerUnits, SceneryOuterUnits);
			SceneryLookOf(Stream, F.Unit, Point);
			const float R = static_cast<float>(Place.Size());
			const FVector Where = F.Centre + FVector(Place.X * F.Unit, Place.Y * F.Unit, 0.0);
			bSame = Points[At].Position.Equals(Where, 0.01) && FMath::IsNearlyEqual(Points[At].Glow, Point.Glow * SceneryFadeOf(R) * LocalGroupShareOf(R, true), 1e-5f)
				&& FMath::IsNearlyEqual(Points[At].Size, Point.Size * LocalGroupShareOf(R, false), 0.01f) && Points[At].Aspect == Point.Aspect && Points[At].TurnDegrees == Point.TurnDegrees;
		}
		TestTrue(TEXT("The stream is the Scenery derive of the home row"), bSame);
	}
	{
		const TArray<FCosmicSceneryPoint> Again = SceneryPointsOf(HomeRow, 2000, F);
		bool bSame = Again.Num() == Points.Num();
		for (int32 At = 0; bSame && At < Points.Num(); ++At)
		{
			bSame = Again[At].Position == Points[At].Position && Again[At].Glow == Points[At].Glow && Again[At].Size == Points[At].Size;
		}
		TestTrue(TEXT("The same arguments draw the same points"), bSame);
		const TArray<FCosmicSceneryPoint> Other = SceneryPointsOf(FName(TEXT("gal_test_big")), 2000, F);
		int32 Moved = 0;
		for (int32 At = 0; At < FMath::Min(Other.Num(), Points.Num()); ++At)
		{
			Moved += Other[At].Position.Equals(Points[At].Position, 0.01) ? 0 : 1;
		}
		TestTrue(TEXT("Another home row draws other points"), Moved >= 1980);
		const TArray<FCosmicSceneryPoint> Longer = SceneryPointsOf(HomeRow, 2500, F);
		bool bPrefix = Longer.Num() == 2500;
		for (int32 At = 0; bPrefix && At < Points.Num(); ++At)
		{
			bPrefix = Longer[At].Position == Points[At].Position;
		}
		TestTrue(TEXT("A longer list keeps the shorter list's points"), bPrefix);
		TestEqual(TEXT("No count, no points"), SceneryPointsOf(HomeRow, 0, F).Num(), 0);
		FCosmicUniverseFrame Zero = F;
		Zero.Unit = 0.f;
		TestEqual(TEXT("No unit, no points"), SceneryPointsOf(HomeRow, 100, Zero).Num(), 0);
	}

	// ---- 6 Clearing ----
	{
		FCosmicGalaxyMark Mark;
		Mark.Position = F.Centre + FVector(20000.0, 0.0, 0.0);
		Mark.Radius = 1000.f;
		TArray<FCosmicSceneryPoint> Hand;
		for (const double Offset : { 0.0, 500.0, 3000.0 })
		{
			FCosmicSceneryPoint& Point = Hand.AddDefaulted_GetRef();
			Point.Position = Mark.Position + FVector(Offset, 0.0, 0.0);
			Point.Glow = 0.3f;
		}
		ClearAroundMarks(Hand, TArray<FCosmicGalaxyMark>{ Mark });
		TestEqual(TEXT("A point under a named galaxy is dark"), Hand[0].Glow, 0.f);
		TestEqual(TEXT("and one at half its radius"), Hand[1].Glow, 0.f);
		TestEqual(TEXT("One three radii away keeps its glow"), Hand[2].Glow, 0.3f);
		TArray<FCosmicSceneryPoint> Cleared = Points;
		ClearAroundMarks(Cleared, Marks);
		int32 Moved = 0;
		int32 Changed = 0;
		for (int32 At = 0; At < Points.Num(); ++At)
		{
			Moved += Cleared[At].Position == Points[At].Position ? 0 : 1;
			bool bNear = false;
			for (const FCosmicGalaxyMark& Each : Marks)
			{
				bNear |= FVector::Dist2D(Points[At].Position, Each.Position) < ClearShare * Each.Radius;
			}
			Changed += (!bNear && Cleared[At].Glow != Points[At].Glow) ? 1 : 0;
		}
		TestEqual(TEXT("Clearing moves no point"), Moved, 0);
		TestEqual(TEXT("Clearing leaves the points away from every galaxy as they were"), Changed, 0);
	}

	// ---- 7 Reach and sky ----
	{
		FCosmicGalaxyMark Far;
		Far.Position = F.Centre + FVector(6.0 * F.Unit, 0.0, 0.0);
		Far.Radius = 2.4f * F.Unit;
		FCosmicGalaxyMark Home;
		Home.Position = F.Centre;
		Home.Radius = HomeSpriteShare * F.Unit;
		TestEqual(TEXT("The reach is 1.1 times the farthest far side"), ReachOf(F, TArray<FCosmicGalaxyMark>{ Home, Far }), 1.1f * 8.4f * F.Unit, 1.f);
		TestEqual(TEXT("The scenery does not count in the reach"), ReachOf(F, TArray<FCosmicGalaxyMark>{ Home }), 1.1f * HomeSpriteShare * F.Unit, 1.f);
		const FCosmicUniverseFrame Wide = FrameOf(F.Centre, 15000.f);
		const TArray<FCosmicGalaxyMark> WideMarks = MarksOf(T, HomeRow, Wide);
		bool bScaled = WideMarks.Num() == Marks.Num();
		for (int32 At = 0; bScaled && At < Marks.Num(); ++At)
		{
			bScaled = (WideMarks[At].Position - Wide.Centre).Equals(3.0 * (Marks[At].Position - F.Centre), 0.1) && FMath::IsNearlyEqual(WideMarks[At].Radius, 3.f * Marks[At].Radius, 0.1f);
		}
		TestTrue(TEXT("Every length of the universe scales with the unit"), bScaled && FMath::IsNearlyEqual(ReachOf(Wide, WideMarks), 3.f * ReachOf(F, Marks), 1.f));
		const FCosmicZoomLevels Levels = CosmicZoom::LevelsOf(F.Centre, F.Unit, ReachOf(F, Marks));
		const float Sky = SkyRadiusOf(Levels, F, F.Centre);
		TestTrue(TEXT("The sky holds every camera place"), Sky >= Levels.UniverseBound + Levels.UniverseFar);
		TestTrue(TEXT("and the scenery's outer edge"), Sky >= SceneryOuterUnits * F.Unit);
		TestEqual(TEXT("and adds the disc centre's offset"), SkyRadiusOf(Levels, F, F.Centre + FVector(1000.0, 0.0, 0.0)) - Sky, 1500.f, 1.f);
		TestEqual(TEXT("and is never smaller than today's"), SkyRadiusOf(CosmicZoom::LevelsOf(FVector::ZeroVector, 1000.f, 0.f), FrameOf(FVector::ZeroVector, 1000.f), FVector::ZeroVector), MinSkyRadius);
		TestEqual(TEXT("In the region the sight's intensity is the view's"), RecededIntensityOf(1.f, 0.f), 1.f);
		TestEqual(TEXT("At the universe level the sight dims nothing"), RecededIntensityOf(1.f, 1.f), 0.f);
		TestEqual(TEXT("A share that is not a number keeps the view's"), RecededIntensityOf(1.f, NaN), 1.f);
	}

	// ---- 8 What the screen shows ----
	{
		FCosmicGalaxyMark Mark;
		Mark.Radius = 1000.f;
		const float D = 1000.f / LabelRadiusShare;
		TestTrue(TEXT("A galaxy as large as the share of the view shows its label"), ShowsGalaxyLabel(Mark, D, false));
		TestFalse(TEXT("a smaller one does not"), ShowsGalaxyLabel(Mark, D * 1.01f, false));
		TestTrue(TEXT("a selected one always does"), ShowsGalaxyLabel(Mark, D * 100.f, true));
		FCosmicGalaxyMark HomeLabel = Mark;
		HomeLabel.bHome = true;
		TestTrue(TEXT("the home always does"), ShowsGalaxyLabel(HomeLabel, D * 100.f, false));
		TestFalse(TEXT("a distance that is not a number shows only the selected and the home"), ShowsGalaxyLabel(Mark, NaN, false));
		TestEqual(TEXT("The ring stands at its share of the sprite"), RingRadiusOf(Mark), RingShare * 1000.f);
		const FCosmicZoomLevels Levels = CosmicZoom::LevelsOf(FVector::ZeroVector, 10000.f, 80000.f);
		FCosmicGalaxyMark Large = Mark;
		Large.Radius = 15000.f;
		TestEqual(TEXT("A large galaxy is focused from its share of radii"), FocusArmOf(Large, Levels), GalaxyFocusFactor * 15000.f, 0.5f);
		TestEqual(TEXT("a small one from the near stop"), FocusArmOf(Mark, Levels), Levels.UniverseNear, 0.5f);
		Large.Radius = 1.0e7f;
		TestEqual(TEXT("never past the far stop"), FocusArmOf(Large, Levels), Levels.UniverseFar, 0.5f);
		TestEqual(TEXT("and none while the universe is closed"), FocusArmOf(Mark, CosmicZoom::LevelsOf(FVector::ZeroVector, 10000.f, 0.f)), 0.f);
		FCosmicGalaxyMark Turned;
		Turned.Position = FVector(10.0, 20.0, 0.0);
		Turned.Radius = 1000.f;
		Turned.Aspect = 0.5f;
		Turned.TurnDegrees = 30.f;
		const FTransform Instance = InstanceOf(Turned);
		TestTrue(TEXT("A mark's instance stands at its centre"), Instance.GetLocation().Equals(Turned.Position, 0.001));
		TestEqual(TEXT("twice its radius across"), Instance.GetScale3D().X, 20.0, 0.001);
		TestEqual(TEXT("flattened by its aspect"), Instance.GetScale3D().Y, 10.0, 0.001);
		TestEqual(TEXT("turned by its turn"), Instance.Rotator().Yaw, 30.0, 0.01);
		FCosmicSceneryPoint Point;
		Point.Size = 300.f;
		TestEqual(TEXT("A point's instance is its size across"), InstanceOf(Point).GetScale3D().X, 3.0, 0.001);
	}
	{
		const FName System(TEXT("sys_3"));
		const FName Galaxy(TEXT("gal_test_big"));
		auto Is = [](const FCosmicPanelChoice& Choice, bool bSystem, FName Shown) { return Choice.bSystemPanel == bSystem && Choice.Shown == Shown; };
		TestTrue(TEXT("Region, a system: that system's panel"), Is(PanelOf(ECosmicZoomLevel::Region, System, NAME_None, HomeRow), true, System));
		TestTrue(TEXT("Region, no system: the home's galaxy panel"), Is(PanelOf(ECosmicZoomLevel::Region, NAME_None, NAME_None, HomeRow), false, HomeRow));
		TestTrue(TEXT("Region, a galaxy selected: the home's galaxy panel, not the selected galaxy's"), Is(PanelOf(ECosmicZoomLevel::Region, NAME_None, Galaxy, HomeRow), false, HomeRow));
		TestTrue(TEXT("Region, a system and a galaxy: the system panel"), Is(PanelOf(ECosmicZoomLevel::Region, System, Galaxy, HomeRow), true, System));
		TestTrue(TEXT("Universe, a galaxy: that galaxy's panel"), Is(PanelOf(ECosmicZoomLevel::Universe, NAME_None, Galaxy, HomeRow), false, Galaxy));
		TestTrue(TEXT("Universe, no galaxy: the home's galaxy panel"), Is(PanelOf(ECosmicZoomLevel::Universe, NAME_None, NAME_None, HomeRow), false, HomeRow));
		TestTrue(TEXT("Universe, a system selected: the system steps aside"), Is(PanelOf(ECosmicZoomLevel::Universe, System, NAME_None, HomeRow), false, HomeRow));
	}
	{
		FCosmicHintProgress Progress;
		TestTrue(TEXT("The region's hint before the first system selection is item 8's"), CosmicUI::HintKeyOf(ECosmicZoomLevel::Region, Progress) == CosmicUI::HintPickStarKey);
		Progress.bPickedSystem = true;
		TestTrue(TEXT("then the wheel's"), CosmicUI::HintKeyOf(ECosmicZoomLevel::Region, Progress) == CosmicUI::HintZoomOutKey);
		Progress.bReachedUniverse = true;
		TestTrue(TEXT("and none after the first visit"), CosmicUI::HintKeyOf(ECosmicZoomLevel::Region, Progress) == nullptr);
		TestTrue(TEXT("The universe's hint before the first galaxy selection"), CosmicUI::HintKeyOf(ECosmicZoomLevel::Universe, Progress) == CosmicUI::HintPickGalaxyKey);
		Progress.bPickedGalaxy = true;
		TestTrue(TEXT("then the way home's"), CosmicUI::HintKeyOf(ECosmicZoomLevel::Universe, Progress) == CosmicUI::HintHomeKey);
		Progress.bLeftUniverse = true;
		TestTrue(TEXT("and none after the first way back"), CosmicUI::HintKeyOf(ECosmicZoomLevel::Universe, Progress) == nullptr);
	}
	return true;
}

// ======================================================================================================================
// UI.Universe: the game's map, controller, pawn and HUD at the universe level
// ======================================================================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIUniverseTest, "Cosmic_Ascension.UI.Universe", CosmicTests::Flags)
bool FCosmicUIUniverseTest::RunTest(const FString& Parameters)
{
	using namespace CosmicUniverseTests;
	using namespace CosmicUniverseLook;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	if (!FSlateApplication::IsInitialized())
	{
		AddError(TEXT("(Slate is not initialized)"));
		return false;
	}
	UWorld* World = Sim->GetGameInstance()->GetWorld();
	ACosmicGalaxyMapActor* Map = FindMap(World);
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(World ? World->GetFirstPlayerController() : nullptr);
	ACosmicCameraPawn* Pawn = Controller ? Cast<ACosmicCameraPawn>(Controller->GetPawn()) : nullptr;
	ACosmicHUD* HUD = Controller ? Controller->GetHUD<ACosmicHUD>() : nullptr;
	if (!TestNotNull(TEXT("The game mode spawned the map"), Map) || !TestNotNull(TEXT("(a camera pawn)"), Pawn) || !TestNotNull(TEXT("(the HUD)"), HUD))
	{
		return false;
	}
	const FCosmicDataTables GameTables = Sim->GetTables();
	const FCosmicHintProgress KeptHints = Controller->GetHintProgress();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(GameTables);
		Controller->CosmicSpiritualSight(0);
		Controller->SelectSystem(NAME_None);
		Controller->SelectGalaxy(NAME_None);
		Controller->SetHintProgress(KeptHints);
		Sim->NewGalaxy(42, 5, 3);
		Map->RefreshNow();
		HUD->RefreshPanel();
	};
	Sim->NewGalaxy(42, 0, 0);
	CosmicTests::KnowAll(Sim->GetMutableState());
	Map->RefreshNow();
	const FGalaxyState Board = Sim->GetState();
	const FCosmicRulesDef Rules = Sim->GetRules();
	const FName HomeRow = Rules.HomeGalaxyRow;
	const UInstancedStaticMeshComponent* Universe = Map->GetUniverse();
	const FCosmicZoomLevels Levels = Map->GetZoomLevels();
	const FCosmicUniverseFrame Frame = Map->GetUniverseFrame();
	if (!TestNotNull(TEXT("(the map holds a universe)"), Universe) || !TestTrue(TEXT("(the universe is open)"), Levels.HasUniverse()))
	{
		return false;
	}
	auto Restore = [&]()
	{
		Sim->SetTables(GameTables);
		Sim->LoadState(Board);
		Map->RefreshNow();
		Controller->SelectSystem(NAME_None);
		Controller->SelectGalaxy(NAME_None);
	};

	// ---- 1 The build and the game's rows ----
	{
		TestTrue(TEXT("It is one instanced mesh of the engine plane"), Universe->GetStaticMesh() && Universe->GetStaticMesh()->GetPathName() == FString(CosmicMapLook::BackdropMeshPath));
		const UMaterialInstanceDynamic* Own = Map->GetUniverseMaterial();
		TestTrue(TEXT("drawn through its own instance of M_Backdrop"), Own && Own->Parent && Own->Parent->GetPathName() == FString(CosmicMapLook::BackdropMaterialPath));
		TestTrue(TEXT("not the backdrop's"), Own && Own != Map->GetBackdropMaterial());
		const TArray<FCosmicGalaxyMark>& Marks = Map->GetGalaxyMarks();
		const int32 Count = SceneryCountOf(Rules);
		TestEqual(TEXT("It holds the rules' scenery and one sprite per named galaxy"), Universe->GetInstanceCount(), Count + Marks.Num());
		TArray<FCosmicSceneryPoint> Points = SceneryPointsOf(HomeRow, Count, Frame);
		ClearAroundMarks(Points, Marks);
		if (TestEqual(TEXT("(the map's scenery count is the rules')"), Map->GetSceneryCount(), Points.Num()) && Points.Num() > 0)
		{
			FTransform First;
			FTransform Last;
			Universe->GetInstanceTransform(0, First, true);
			Universe->GetInstanceTransform(Points.Num() - 1, Last, true);
			TestTrue(TEXT("Its first instance is the first scenery point"), First.GetLocation().Equals(Points[0].Position, 0.01) && FMath::IsNearlyEqual(First.GetScale3D().X, Points[0].Size / 100.0, 1e-4));
			TestTrue(TEXT("its last scenery instance the last point"), Last.GetLocation().Equals(Points.Last().Position, 0.01) && FMath::IsNearlyEqual(Last.GetScale3D().X, Points.Last().Size / 100.0, 1e-4));
			int32 Glows = 0;
			for (int32 At = 0; At < Points.Num(); ++At)
			{
				Glows += GlowOf(*Universe, At) == Points[At].Glow ? 0 : 1;
			}
			TestEqual(TEXT("and their glows are the points' after clearing"), Glows, 0);
		}
		int32 Sprites = 0;
		for (int32 At = 0; At < Marks.Num(); ++At)
		{
			FTransform Instance;
			Universe->GetInstanceTransform(Map->GetSceneryCount() + At, Instance, true);
			const FTransform Want = InstanceOf(Marks[At]);
			Sprites += (Instance.GetLocation().Equals(Want.GetLocation(), 0.01) && Instance.GetScale3D().Equals(Want.GetScale3D(), 1e-4) && GlowOf(*Universe, Map->GetSceneryCount() + At) == Marks[At].Glow) ? 0 : 1;
		}
		TestEqual(TEXT("Every named galaxy's sprite is its mark"), Sprites, 0);
		const FCosmicGalaxyMark* Home = Marks.FindByPredicate([](const FCosmicGalaxyMark& Mark) { return Mark.bHome; });
		TestTrue(TEXT("The home's sprite stands at the disc's centre"), Home && Home->Position.Equals(FVector(Map->GetDiscCentre().X, Map->GetDiscCentre().Y, 0.0), 0.01));
		const int32 Triangles = Universe->GetStaticMesh() ? Universe->GetStaticMesh()->GetNumTriangles(0) : 0;
		TestEqual(TEXT("(the plane is two triangles)"), Triangles, 2);
		TestTrue(TEXT("The universe draws at most UniverseTriangleBudget triangles"), static_cast<int64>(Triangles) * Universe->GetInstanceCount() <= UniverseTriangleBudget);
		TestEqual(TEXT("The map holds one label per mark"), Map->GetGalaxyLabels().Num(), Marks.Num());
	}
	// The game's rows (the only block that reads them).
	{
		const TArray<FCosmicGalaxyMark>& Marks = Map->GetGalaxyMarks();
		TestTrue(TEXT("(the game has at least the six galaxy rows of v0.7)"), Marks.Num() >= 6);
		TestEqual(TEXT("Every galaxy row of the game has a place"), UnplacedRowsOf(GameTables, HomeRow).Num(), 0);
		for (int32 A = 0; A < Marks.Num(); ++A)
		{
			for (int32 B = A + 1; B < Marks.Num(); ++B)
			{
				const double Apart = FVector::Dist2D(Marks[A].Position, Marks[B].Position) / Frame.Unit;
				const double Need = 0.3 * (Marks[A].Radius + Marks[B].Radius) / Frame.Unit + 0.1;
				TestTrue(*FString::Printf(TEXT("Galaxies %s and %s keep their cores apart"), *Marks[A].Row.ToString(), *Marks[B].Row.ToString()), Apart >= Need);
			}
		}
		const TSet<FName> Stage = { FName(TEXT("gal_andromeda")), FName(TEXT("gal_triangulum")), FName(TEXT("gal_lmc")), FName(TEXT("gal_smc")) };
		for (const FName Row : GameTables.GetGalaxyRowNames())
		{
			const FGalaxyDef* Def = GameTables.FindGalaxy(Row);
			TestTrue(*FString::Printf(TEXT("Galaxy %s's kind has a word"), *Row.ToString()), Def && CosmicUI::GalaxyKindKey(Def->Kind) != nullptr);
			TestEqual(*FString::Printf(TEXT("No galaxy of the game has a bridge but the four stage galaxies: %s"), *Row.ToString()), Def && Def->BridgeTicks > 0, Stage.Contains(Row));
		}
		// The far stop at the start view, through the game camera's field of view on a 16:9 screen.
		const UCameraComponent* GameCamera = Pawn->FindComponentByClass<UCameraComponent>();
		FTestCamera Camera;
		Camera.Fov = GameCamera ? GameCamera->FieldOfView : 90.0;
		Camera.Arm = Levels.UniverseFar;
		Camera.Focus = Levels.Centre;
		TestTrue(TEXT("(the game camera has a field of view)"), GameCamera != nullptr);
		for (const FCosmicGalaxyMark& Mark : Marks)
		{
			FVector2D Screen;
			const bool bOn = Camera.Project(Mark.Position, Screen) && Screen.X >= 16.0 && Screen.X <= 1264.0 && Screen.Y >= 40.0 && Screen.Y <= 704.0;
			TestTrue(*FString::Printf(TEXT("The far stop holds galaxy %s on a 16:9 screen at the start pitch"), *Mark.Row.ToString()), bOn);
		}
		// The scenery reaches past the far stop's corners: no rim at rest - the focus on the centre and at the bound.
		const double Edge = (SceneryOuterUnits - SceneryOuterFadeUnits) * Frame.Unit;
		for (const FVector& Offset : { FVector::ZeroVector, FVector(Levels.UniverseBound, 0.0, 0.0), FVector(-Levels.UniverseBound, 0.0, 0.0), FVector(0.0, Levels.UniverseBound, 0.0), FVector(0.0, -Levels.UniverseBound, 0.0) })
		{
			Camera.Focus = Levels.Centre + Offset;
			for (const FVector2D& Corner : { FVector2D(0.0, 0.0), FVector2D(1280.0, 0.0), FVector2D(0.0, 720.0), FVector2D(1280.0, 720.0) })
			{
				FVector Ground;
				const bool bInside = Camera.Ground(Corner, Ground) && FVector::Dist2D(Ground, Levels.Centre) <= Edge;
				TestTrue(*FString::Printf(TEXT("The scenery covers the far stop's corner (%.0f, %.0f) with the focus %.0f, %.0f from the centre"), Corner.X, Corner.Y, Offset.X, Offset.Y), bInside);
			}
		}
	}

	// ---- 2 The key ----
	{
		Restore();
		const int32 Built = Map->GetUniverseBuildCount();
		Map->RequestRefresh();
		Map->RefreshNow();
		TestEqual(TEXT("A refresh builds no universe"), Map->GetUniverseBuildCount(), Built);
		Sim->NewGalaxy(42, 0, 0);
		Map->RefreshNow();
		TestEqual(TEXT("A new galaxy of the same seed and size builds none"), Map->GetUniverseBuildCount(), Built);
		Restore();
		TestEqual(TEXT("A load of a copy builds none"), Map->GetUniverseBuildCount(), Built);
		FGalaxyState Reseeded = Board;
		Reseeded.Seed = Board.Seed + 1;
		Sim->LoadState(Reseeded);
		Map->RefreshNow();
		TestEqual(TEXT("A copy with another seed and the same frame builds nothing new (one sky for every run)"), Map->GetUniverseBuildCount(), Built);
		// Another frame: the farthest system moved outward.
		{
			FGalaxyState Moved = Board;
			int32 Farthest = 0;
			for (int32 At = 0; At < Moved.Systems.Num(); ++At)
			{
				if (FVector::Dist2D(Moved.Systems[At].Position, Map->GetDiscCentre()) > FVector::Dist2D(Moved.Systems[Farthest].Position, Map->GetDiscCentre()))
				{
					Farthest = At;
				}
			}
			const FVector Out = (Moved.Systems[Farthest].Position - Map->GetDiscCentre()).GetSafeNormal2D();
			Moved.Systems[Farthest].Position += Out * 500.0;
			Sim->LoadState(Moved);
			Map->RefreshNow();
			TestEqual(TEXT("Another frame builds a new one"), Map->GetUniverseBuildCount(), Built + 1);
		}
		Restore();
		const int32 Before = Map->GetUniverseBuildCount();
		const int32 SceneryBefore = Map->GetSceneryCount();
		TArray<FVector> Places;
		TArray<float> GlowsBefore;
		for (int32 At = 0; At < SceneryBefore; ++At)
		{
			FTransform Instance;
			Universe->GetInstanceTransform(At, Instance, true);
			Places.Add(Instance.GetLocation());
			GlowsBefore.Add(GlowOf(*Universe, At));
		}
		// Another table: the game's rows and one more.
		UDataTable* More = CosmicTests::NewTable(FGalaxyDef::StaticStruct());
		for (const FName Row : GameTables.GetGalaxyRowNames())
		{
			if (const FGalaxyDef* Def = GameTables.FindGalaxy(Row))
			{
				More->AddRow(Row, *Def);
			}
		}
		const FVector2D ExtraPlace(-8.0, 3.0);
		More->AddRow(FName(TEXT("gal_test_extra")), CosmicTests::GalaxyRowOf(TEXT("Extra"), TEXT("Galaxy.Kind.Spiral"), HomeLawRadiusLy, ExtraPlace));
		const FVector ExtraWorld = PlaceOf(Frame, ExtraPlace);
		const float ExtraRadius = HomeSpriteShare * Frame.Unit;
		int32 Under = 0;
		for (int32 At = 0; At < SceneryBefore; ++At)
		{
			Under += (FVector::Dist2D(Places[At], ExtraWorld) < ClearShare * ExtraRadius && GlowOf(*Universe, At) > 0.f) ? 1 : 0;
		}
		TestTrue(TEXT("(some point lies under the new galaxy)"), Under > 0);
		Sim->SetTables(CosmicTests::WithGalaxies(GameTables, More));
		Sim->LoadState(Board);
		Map->RefreshNow();
		TestEqual(TEXT("Another table of galaxies builds a new one"), Map->GetUniverseBuildCount(), Before + 1);
		bool bKept = Map->GetSceneryCount() == SceneryBefore;
		int32 Dimmed = 0;
		for (int32 At = 0; bKept && At < SceneryBefore; ++At)
		{
			FTransform Instance;
			Universe->GetInstanceTransform(At, Instance, true);
			bKept = Instance.GetLocation().Equals(Places[At], 0.01);
			Dimmed += (FVector::Dist2D(Places[At], ExtraWorld) < ClearShare * ExtraRadius && GlowsBefore[At] > 0.f && GlowOf(*Universe, At) == 0.f) ? 1 : 0;
		}
		TestTrue(TEXT("and keeps every scenery point"), bKept);
		TestEqual(TEXT("and dims the points under the new galaxy"), Dimmed, Under);
		// The two streams, across that build: a load puts both back, and the build draws from neither.
		{
			const int32 BuildsNow = Map->GetUniverseBuildCount();
			Sim->SetTables(GameTables);
			FGalaxyState Copy = Board;
			Sim->LoadState(Copy);
			const uint32 StateHash = Sim->ComputeStateHash();
			const uint32 WorldHash = Sim->ComputeWorldHash();
			Map->RefreshNow();
			TestEqual(TEXT("(the load rebuilt the universe)"), Map->GetUniverseBuildCount(), BuildsNow + 1);
			TestEqual(TEXT("The universe never draws from the simulation's stream"), Sim->GetRng().GetCurrentSeed(), Copy.RngState);
			TestEqual(TEXT("nor from the tell stream"), Sim->GetState().TellRngState, Copy.TellRngState);
			// Fades, both selections and a refresh at the universe level: neither hash moves.
			Map->SetViewDistance(Levels.UniverseNear);
			Map->RefreshNow();
			Controller->SelectGalaxy(FName(TEXT("gal_andromeda")));
			Controller->SelectSystem(Board.Systems.IsValidIndex(2) ? Board.Systems[2].SystemID : NAME_None);
			Map->RefreshNow();
			Map->SetViewDistance(Levels.RegionFar);
			Map->RefreshNow();
			TestEqual(TEXT("Building and drawing the universe moves neither hash: the state"), Sim->ComputeStateHash(), StateHash);
			TestEqual(TEXT("Building and drawing the universe moves neither hash: the world"), Sim->ComputeWorldHash(), WorldHash);
		}
		// Another scenery count, then another home row.
		Restore();
		{
			const int32 BuildsNow = Map->GetUniverseBuildCount();
			FCosmicRulesDef Fewer = Rules;
			Fewer.SceneryGalaxyCount = 500;
			Sim->SetTables(CosmicTests::WithRules(GameTables, Fewer));
			Sim->LoadState(Board);
			Map->RefreshNow();
			TestEqual(TEXT("Another scenery count builds a new one"), Map->GetUniverseBuildCount(), BuildsNow + 1);
			TestEqual(TEXT("with that many points"), Universe->GetInstanceCount(), 500 + Map->GetGalaxyMarks().Num());
		}
		Restore();
		{
			const int32 BuildsNow = Map->GetUniverseBuildCount();
			FCosmicRulesDef OtherHome = Rules;
			OtherHome.HomeGalaxyRow = FName(TEXT("gal_andromeda"));
			Sim->SetTables(CosmicTests::WithRules(GameTables, OtherHome));
			Sim->LoadState(Board);
			Map->RefreshNow();
			TestEqual(TEXT("Another home row builds a new one"), Map->GetUniverseBuildCount(), BuildsNow + 1);
		}
		Restore();
	}

	// ---- 3 What fades with the distance ----
	auto LayerOf = [](const UMaterialInstanceDynamic* Material) { return ScalarOf(Material, CosmicMapLook::LayerFadeParam); };
	const UInstancedStaticMeshComponent* Dust = Map->GetBackdrop();
	const ULineBatchComponent* Batch = Map->FindComponentByClass<ULineBatchComponent>();
	if (!TestNotNull(TEXT("(the map draws its lines in one batch)"), Batch) || !TestNotNull(TEXT("(and its dust in one mesh)"), Dust))
	{
		return false;
	}
	auto StarsHidden = [Map]()
	{
		int32 Hidden = 0;
		for (const ACosmicStarSystemActor* Star : Map->GetStars())
		{
			Hidden += (Star && Star->IsHidden()) ? 1 : 0;
		}
		return Hidden;
	};
	auto LabelsShown = [Map]()
	{
		int32 Shown = 0;
		for (int32 At = 0; At < Map->GetGalaxyMarks().Num(); ++At)
		{
			Shown += Map->IsGalaxyLabelVisible(At) ? 1 : 0;
		}
		return Shown;
	};
	{
		Map->SetViewDistance(Levels.RegionFar);
		Map->RefreshNow();
		TestTrue(TEXT("(in the region the owners' rings are drawn)"), Map->GetDrawnRingCount() > 0);
		int32 StarLabels = 0;
		for (const ACosmicStarSystemActor* Star : Map->GetStars())
		{
			StarLabels += (Star && Star->IsLabelVisible()) ? 1 : 0;
		}
		TestTrue(TEXT("(some star label shows in the region)"), StarLabels > 0);
		TestEqual(TEXT("In the region every star actor is drawn"), StarsHidden(), 0);
		TestTrue(TEXT("the lanes are drawn"), Map->GetDrawnLaneCount() > 0);
		TestEqual(TEXT("the dust is whole"), LayerOf(Map->GetBackdropMaterial()), 1.f);
		TestEqual(TEXT("the universe is dark"), LayerOf(Map->GetUniverseMaterial()), 0.f);
		TestFalse(TEXT("and not drawn at all"), Universe->IsVisible());
		TestEqual(TEXT("no galaxy label shows"), LabelsShown(), 0);

		const float Step = Levels.UniverseNear - Levels.RegionFar;
		Map->SetViewDistance(Levels.RegionFar + 0.5f * Step);
		TestEqual(TEXT("Half way the dust is half"), LayerOf(Map->GetBackdropMaterial()), 0.5f, 0.01f);
		TestEqual(TEXT("and so is the universe"), LayerOf(Map->GetUniverseMaterial()), 0.5f, 0.01f);
		TestTrue(TEXT("Both are drawn while the camera stands between"), Universe->IsVisible() && Dust->IsVisible());

		Map->SetViewDistance(Levels.RegionFar + 0.3f * Step);
		Map->RefreshNow();
		const int32 Refreshes = Map->GetRefreshCount();
		const float Fade = LayerOf(Map->GetUniverseMaterial());
		Map->SetViewDistance(Levels.RegionFar + 0.35f * Step);
		Map->RefreshNow();
		TestEqual(TEXT("A fade alone redraws nothing"), Map->GetRefreshCount(), Refreshes);
		TestTrue(TEXT("but moves the fades"), LayerOf(Map->GetUniverseMaterial()) > Fade);

		// Just short of the universe, then at it: the end is written even within the slack.
		Map->SetViewDistance(Levels.RegionFar + 0.997f * Step);
		Map->SetViewDistance(Levels.UniverseNear);
		Map->RefreshNow();
		TestEqual(TEXT("At the universe the universe reads exactly whole"), LayerOf(Map->GetUniverseMaterial()), 1.f);
		TestFalse(TEXT("and the dust is not drawn"), Dust->IsVisible());
		TestEqual(TEXT("At the universe level every star actor is hidden"), StarsHidden(), Map->GetStars().Num());
		int32 StarLabelsThere = 0;
		for (const ACosmicStarSystemActor* Star : Map->GetStars())
		{
			StarLabelsThere += (Star && Star->IsLabelVisible()) ? 1 : 0;
		}
		TestEqual(TEXT("and every star label"), StarLabelsThere, 0);
		TestEqual(TEXT("no lane is drawn"), Map->GetDrawnLaneCount(), 0);
		TestEqual(TEXT("no ring of a system is drawn"), Map->GetDrawnRingCount() + Map->GetDrawnSealedRingCount(), 0);
		TestEqual(TEXT("the dust is gone"), LayerOf(Map->GetBackdropMaterial()), 0.f);
		TestTrue(TEXT("the universe is drawn"), Universe->IsVisible());

		const TArray<FCosmicGalaxyMark>& Marks = Map->GetGalaxyMarks();
		int32 Wrong = 0;
		int32 Named = 0;
		int32 Placed = 0;
		for (int32 At = 0; At < Marks.Num(); ++At)
		{
			Wrong += Map->IsGalaxyLabelVisible(At) == ShowsGalaxyLabel(Marks[At], Map->GetViewDistance(), false) ? 0 : 1;
			const UWidgetComponent* Label = Map->GetGalaxyLabels().IsValidIndex(At) ? Map->GetGalaxyLabels()[At].Get() : nullptr;
			const UCosmicSystemLabelWidget* Widget = Label ? Cast<UCosmicSystemLabelWidget>(Label->GetUserWidgetObject()) : nullptr;
			Named += (Widget && Widget->GetShownLook().Name.ToString().Equals(Marks[At].Name.ToString(), ESearchCase::CaseSensitive)
				&& Widget->GetShownLook().ConstructionShare == -1.f && Widget->GetShownLook().ResponseShare == -1.f) ? 1 : 0;
			Placed += (Label && Label->GetComponentLocation().Equals(Marks[At].Position + FVector(0.0, 0.0, LabelLiftShare * Marks[At].Radius), 0.5)) ? 1 : 0;
		}
		TestEqual(TEXT("Every galaxy label shows near the universe as the rule says"), Wrong, 0);
		TestEqual(TEXT("Every galaxy label is its galaxy's name alone"), Named, Marks.Num());
		TestEqual(TEXT("Every label stands over its galaxy"), Placed, Marks.Num());

		Map->SetViewDistance(Levels.UniverseFar);
		Map->RefreshNow();
		int32 Want = 0;
		for (const FCosmicGalaxyMark& Mark : Marks)
		{
			Want += ShowsGalaxyLabel(Mark, Map->GetViewDistance(), false) ? 1 : 0;
		}
		TestTrue(TEXT("(some galaxy is too small for a label from there)"), Want < Marks.Num());
		TestEqual(TEXT("From the far stop only the large galaxies and the home are labelled"), LabelsShown(), Want);
		// A label follows its own threshold even inside the redraw's slack (the review of item 9: the far stop's labels
		// otherwise hung on the frame the glide last redrew on). A galaxy whose threshold lies well inside the universe.
		{
			int32 Edge = INDEX_NONE;
			for (int32 At = 0; At < Marks.Num() && Edge == INDEX_NONE; ++At)
			{
				const float Threshold = Marks[At].Radius / LabelRadiusShare;
				Edge = (!Marks[At].bHome && Threshold > 1.2f * Levels.UniverseNear && Threshold < 0.8f * Levels.UniverseFar) ? At : INDEX_NONE;
			}
			if (TestTrue(TEXT("(a galaxy's label turns inside the universe)"), Edge != INDEX_NONE))
			{
				const float Threshold = Marks[Edge].Radius / LabelRadiusShare;
				Map->SetViewDistance(0.99f * Threshold);
				Map->RefreshNow();
				TestTrue(TEXT("(its label shows just inside its threshold)"), Map->IsGalaxyLabelVisible(Edge));
				Map->SetViewDistance(1.02f * Threshold);
				Map->RefreshNow();
				TestFalse(TEXT("A step past its threshold inside the redraw's slack hides its label"), Map->IsGalaxyLabelVisible(Edge));
				Map->SetViewDistance(0.99f * Threshold);
				Map->RefreshNow();
				TestTrue(TEXT("and a step back shows it again"), Map->IsGalaxyLabelVisible(Edge));
			}
			Map->SetViewDistance(Levels.UniverseFar);
			Map->RefreshNow();
		}
		TestEqual(TEXT("At the universe level the map draws no line without a selection"), Batch->BatchedLines.Num(), 0);
		Controller->SelectGalaxy(FName(TEXT("gal_andromeda")));
		Map->RefreshNow();
		int32 White = 0;
		for (const FBatchedLine& Line : Batch->BatchedLines)
		{
			White += Line.Color.Equals(CosmicPalette::White, 0.001f) ? 1 : 0;
		}
		TestEqual(TEXT("and with one selected only its ring: 32 segments"), Batch->BatchedLines.Num(), 32);
		TestEqual(TEXT("all white - no bridge is drawn"), White, 32);
		Controller->SelectGalaxy(NAME_None);

		Map->SetViewDistance(Levels.RegionFar);
		Map->RefreshNow();
		TestEqual(TEXT("Back in the region the stars come back"), StarsHidden(), 0);
		TestEqual(TEXT("and the dust"), LayerOf(Map->GetBackdropMaterial()), 1.f);
		TestEqual(TEXT("and the universe goes dark"), LayerOf(Map->GetUniverseMaterial()), 0.f);
		TestEqual(TEXT("and the galaxy labels go"), LabelsShown(), 0);

		// The sight at the universe (amendment 7): the sky and the universe take the view's value x (1 - the share).
		Controller->CosmicSpiritualSight(1);
		Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
		Map->RefreshNow();
		TestEqual(TEXT("In the region the sight's sky is the view's"), ScalarOf(Map->GetSkyMaterial(), CosmicMapLook::SkyIntensityParam), 1.f);
		TestEqual(TEXT("and so is the universe's"), ScalarOf(Map->GetUniverseMaterial(), CosmicMapLook::SkyIntensityParam), 1.f);
		Map->SetViewDistance(Levels.UniverseNear);
		Map->RefreshNow();
		TestEqual(TEXT("At the universe level the sight dims no sky"), ScalarOf(Map->GetSkyMaterial(), CosmicMapLook::SkyIntensityParam), 0.f);
		TestEqual(TEXT("nor the universe"), ScalarOf(Map->GetUniverseMaterial(), CosmicMapLook::SkyIntensityParam), 0.f);
		const int32 Base = Map->GetSceneryCount();
		bool bSameGlows = true;
		for (int32 At = 0; At < Map->GetGalaxyMarks().Num(); ++At)
		{
			bSameGlows &= GlowOf(*Universe, Base + At) == NamedGlow;
		}
		TestTrue(TEXT("The sight changes no galaxy's glow"), bSameGlows);
		Controller->CosmicSpiritualSight(0);
		Map->SetViewDistance(Levels.RegionFar);
		Map->RefreshNow();

		Map->SetViewDistance(Levels.UniverseFar);
		Sim->NewGalaxy(42, 0, 0);
		Map->RefreshNow();
		TestEqual(TEXT("A rebuild starts in the region: no share"), Map->GetUniverseShare(), 0.f);
		TestTrue(TEXT("the region's detail drawn"), Map->IsRegionDetailShown());
		TestEqual(TEXT("the dust whole"), LayerOf(Map->GetBackdropMaterial()), 1.f);
		Restore();
	}

	// ---- 4 The click and the selections ----
	auto ToPlane = [](const FVector& World, FVector2D& Screen)
	{
		Screen = FVector2D(World.X, World.Y);
		return true;
	};
	{
		Map->SetViewDistance(Levels.UniverseNear);
		Map->RefreshNow();
		const TArray<FCosmicGalaxyPickCandidate> Candidates = ACosmicPlayerController::GalaxyPickCandidatesOf(*Map, ToPlane, FVector(0.0, 1.0, 0.0));
		TestEqual(TEXT("Every named galaxy is a pick target"), Candidates.Num(), Map->GetGalaxyMarks().Num());
		bool bOnlyMarks = true;
		for (const FCosmicGalaxyPickCandidate& Candidate : Candidates)
		{
			bOnlyMarks &= Map->GetGalaxyMarks().ContainsByPredicate([&Candidate](const FCosmicGalaxyMark& Mark) { return Mark.Row == Candidate.Row; });
		}
		TestTrue(TEXT("and no scenery galaxy is"), bOnlyMarks);
		TArray<FCosmicScreenTarget> Targets;
		for (const FCosmicGalaxyPickCandidate& Candidate : Candidates)
		{
			Targets.Add(Candidate.Target);
		}
		const FCosmicGalaxyMark* Andromeda = Map->GetGalaxyMarks().FindByPredicate([](const FCosmicGalaxyMark& Mark) { return Mark.Row == FName(TEXT("gal_andromeda")); });
		if (TestNotNull(TEXT("(the universe draws Andromeda)"), Andromeda))
		{
			const int32 Picked = CosmicMapPick::NearestOnScreen(Targets, FVector2D(Andromeda->Position.X, Andromeda->Position.Y), 24.f);
			TestTrue(TEXT("A click on Andromeda picks it"), Candidates.IsValidIndex(Picked) && Candidates[Picked].Row == Andromeda->Row);
		}
		// A scenery point beyond every galaxy's reach: a click there is a click on empty space.
		int32 Lone = INDEX_NONE;
		for (int32 At = 0; At < Map->GetSceneryCount() && Lone == INDEX_NONE; ++At)
		{
			FTransform Instance;
			Universe->GetInstanceTransform(At, Instance, true);
			bool bFree = true;
			for (const FCosmicGalaxyPickCandidate& Candidate : Candidates)
			{
				bFree &= FVector2D::Distance(FVector2D(Instance.GetLocation().X, Instance.GetLocation().Y), Candidate.Target.Centre) > FMath::Max(Candidate.Target.Radius, 24.f) + 1.0;
			}
			Lone = bFree ? At : INDEX_NONE;
		}
		if (TestTrue(TEXT("(the point lies beyond every galaxy's reach)"), Lone != INDEX_NONE))
		{
			FTransform Instance;
			Universe->GetInstanceTransform(Lone, Instance, true);
			TestEqual(TEXT("A click on a scenery point picks nothing there"), CosmicMapPick::NearestOnScreen(Targets, FVector2D(Instance.GetLocation().X, Instance.GetLocation().Y), 24.f), static_cast<int32>(INDEX_NONE));
		}
		const FCosmicKnownView View = Controller->GetKnownView();
		TestEqual(TEXT("At the universe level no star is a pick target"), ACosmicPlayerController::PickCandidatesOf(*Map, View, ToPlane, FVector(0.0, 1.0, 0.0)).Num(), 0);
		Map->SetViewDistance(Levels.RegionFar);
		Map->RefreshNow();
		TestEqual(TEXT("In the region no galaxy is a pick target"), ACosmicPlayerController::GalaxyPickCandidatesOf(*Map, ToPlane, FVector(0.0, 1.0, 0.0)).Num(), 0);
		TestTrue(TEXT("In the region the stars are"), ACosmicPlayerController::PickCandidatesOf(*Map, View, ToPlane, FVector(0.0, 1.0, 0.0)).Num() > 0);
	}
	{
		int32 Announced = 0;
		const FDelegateHandle Handle = Controller->OnGalaxySelectionChanged.AddLambda([&Announced](FName) { ++Announced; });
		ON_SCOPE_EXIT { Controller->OnGalaxySelectionChanged.Remove(Handle); };
		const FName System = Board.Systems.IsValidIndex(2) ? Board.Systems[2].SystemID : NAME_None;
		Controller->SelectSystem(System);
		const FName Andromeda(TEXT("gal_andromeda"));
		Controller->SelectGalaxy(Andromeda);
		TestEqual(TEXT("A named galaxy is selected"), Controller->GetSelectedGalaxy(), Andromeda);
		TestEqual(TEXT("and announced once"), Announced, 1);
		Controller->SelectGalaxy(Andromeda);
		TestEqual(TEXT("Selecting it again announces nothing"), Announced, 1);
		Controller->SelectGalaxy(FName(TEXT("gal_nowhere")));
		TestEqual(TEXT("A row the universe does not draw is ignored"), Controller->GetSelectedGalaxy(), Andromeda);
		TestEqual(TEXT("and not announced"), Announced, 1);
		TestEqual(TEXT("A galaxy selection leaves the system selection as it was"), Controller->GetSelectedSystem(), System);
		Map->SetViewDistance(Levels.UniverseNear);
		TestEqual(TEXT("(the map waits for the next frame)"), Map->GetDrawnGalaxySelection(), FName(NAME_None));
		Map->RefreshNow();
		TestEqual(TEXT("The map rings the selected galaxy at the universe level"), Map->GetDrawnGalaxySelection(), Andromeda);
		Map->SetViewDistance(Levels.RegionFar);
		Map->RefreshNow();
		TestEqual(TEXT("and draws no galaxy ring in the region"), Map->GetDrawnGalaxySelection(), FName(NAME_None));
		Controller->SelectGalaxy(NAME_None);
		TestEqual(TEXT("None clears"), Controller->GetSelectedGalaxy(), FName(NAME_None));
		Controller->SelectGalaxy(Andromeda);
		Announced = 0;
		Sim->NewGalaxy(42, 0, 0);
		Map->RefreshNow();
		TestEqual(TEXT("A new galaxy clears the galaxy selection"), Controller->GetSelectedGalaxy(), FName(NAME_None));
		TestEqual(TEXT("and announces it"), Announced, 1);
		Restore();
		// The release that ends a double click is no click (amendment 4): at the universe level, for a galaxy.
		Pawn->FrameAll(Levels);
		Pawn->Zoom(-100.f);
		Pawn->SnapToTargets();
		Map->SetViewDistance(Pawn->GetArmLength());
		Map->RefreshNow();
		Controller->FollowZoomLevel();
		Controller->SelectGalaxy(Andromeda);
		Controller->HandleLeftPressed(EKeys::LeftMouseButton);
		Controller->HandleLeftDoubleClick(EKeys::LeftMouseButton);
		Controller->HandleLeftReleased(EKeys::LeftMouseButton);
		Map->SetViewDistance(Pawn->GetArmLength());
		Map->RefreshNow();
		TestEqual(TEXT("A double click's release keeps the galaxy selected"), Controller->GetSelectedGalaxy(), Andromeda);
		Controller->HandleLeftPressed(EKeys::LeftMouseButton);
		Controller->HandleLeftReleased(EKeys::LeftMouseButton);
		TestEqual(TEXT("A single click on empty space clears the galaxy"), Controller->GetSelectedGalaxy(), FName(NAME_None));
		Restore();
	}

	// ---- 5 The right side and the hints ----
	{
		UCosmicGalaxyPanelWidget* GalaxyPanel = HUD->GetGalaxyPanel();
		UCosmicSystemPanelWidget* SystemPanel = HUD->GetSystemPanel();
		if (!TestNotNull(TEXT("(the HUD has a galaxy panel)"), GalaxyPanel) || !TestNotNull(TEXT("(and a system panel)"), SystemPanel))
		{
			return false;
		}
		auto Shows = [](const UUserWidget* Widget) { return Widget->GetVisibility() != ESlateVisibility::Collapsed; };
		const FString Named = Words(TEXT("Galaxy.Named"));
		Pawn->FrameAll(Levels);
		Pawn->SnapToTargets();
		Controller->FollowZoomLevel();
		HUD->RefreshPanel();
		TestTrue(TEXT("Region, nothing selected: the HUD shows the home's galaxy panel"), Shows(GalaxyPanel) && GalaxyPanel->GetShownGalaxy() == HomeRow);
		TestFalse(TEXT("The two panels never stand together: region, nothing selected"), Shows(SystemPanel));
		const FName System = Board.Systems.IsValidIndex(2) ? Board.Systems[2].SystemID : NAME_None;
		Controller->SelectSystem(System);
		TestTrue(TEXT("(a system's panel shows in the region)"), Shows(SystemPanel));
		Pawn->Zoom(-100.f);
		Controller->FollowZoomLevel();
		TestFalse(TEXT("At the universe level the system panel steps aside"), Shows(SystemPanel));
		TestTrue(TEXT("and the home's galaxy panel shows"), Shows(GalaxyPanel) && GalaxyPanel->GetShownGalaxy() == HomeRow);
		TestTrue(TEXT("with the every-galaxy line (nothing selected)"), GalaxyPanel->GetLinesText() && GalaxyPanel->GetLinesText()->GetText().ToString().Contains(Named));
		Controller->SelectGalaxy(FName(TEXT("gal_andromeda")));
		const FGalaxyDef* Andromeda = GameTables.FindGalaxy(FName(TEXT("gal_andromeda")));
		TestEqual(TEXT("A selected galaxy's panel shows"), GalaxyPanel->GetShownGalaxy(), FName(TEXT("gal_andromeda")));
		TestTrue(TEXT("with its distance as written"), Andromeda && GalaxyPanel->GetLinesText() && GalaxyPanel->GetLinesText()->GetText().ToString().Contains(Andromeda->DistanceText.ToString()));
		TestTrue(TEXT("and the every-galaxy line last"), GalaxyPanel->GetLinesText() && GalaxyPanel->GetLinesText()->GetText().ToString().EndsWith(Named));
		TestFalse(TEXT("The two panels never stand together: universe, a galaxy"), Shows(SystemPanel));
		Pawn->Zoom(100.f);
		Controller->FollowZoomLevel();
		TestTrue(TEXT("Back in the region the system panel returns"), Shows(SystemPanel));
		TestFalse(TEXT("and the galaxy panel steps aside"), Shows(GalaxyPanel));
		Controller->SelectSystem(NAME_None);
		TestTrue(TEXT("Region, the system cleared: the home's galaxy panel shows, not the selected galaxy's"), Shows(GalaxyPanel) && GalaxyPanel->GetShownGalaxy() == HomeRow);
		TestFalse(TEXT("The two panels never stand together: region, a galaxy selected"), Shows(SystemPanel));
		// No galaxy panel names a people, nor has a people line.
		{
			bool bNamesPeople = false;
			for (const FName Row : GameTables.GetGalaxyRowNames())
			{
				GalaxyPanel->ShowGalaxy(Row);
				const FString Texts = CosmicTests::ReadTexts(*GalaxyPanel);
				for (const FCivilizationState& Civ : Sim->GetState().Civs)
				{
					bNamesPeople |= !Civ.CivName.IsEmpty() && Texts.Contains(Civ.CivName.ToString());
				}
			}
			TestFalse(TEXT("No galaxy panel names a people"), bNamesPeople);
			HUD->RefreshPanel();
		}
		// The hints, from none.
		Controller->SelectGalaxy(NAME_None);
		Controller->SetHintProgress(FCosmicHintProgress());
		HUD->RefreshPanel();
		TestTrue(TEXT("The region's hint is item 8's before the first star"), GalaxyPanel->GetHintKey() == CosmicUI::HintPickStarKey);
		Controller->SelectSystem(System);
		TestTrue(TEXT("A first star selection notes it"), Controller->GetHintProgress().bPickedSystem);
		Controller->SelectSystem(NAME_None);
		TestTrue(TEXT("and the hint turns to the wheel's"), GalaxyPanel->GetHintKey() == CosmicUI::HintZoomOutKey);
		Pawn->Zoom(-100.f);
		Controller->FollowZoomLevel();
		TestTrue(TEXT("A first visit of the universe notes it"), Controller->GetHintProgress().bReachedUniverse);
		TestTrue(TEXT("and the universe's hint shows"), GalaxyPanel->GetHintKey() == CosmicUI::HintPickGalaxyKey);
		Controller->SelectGalaxy(FName(TEXT("gal_andromeda")));
		TestTrue(TEXT("A first galaxy selection notes it"), Controller->GetHintProgress().bPickedGalaxy);
		TestTrue(TEXT("and the hint turns to the way home"), GalaxyPanel->GetHintKey() == CosmicUI::HintHomeKey);
		const FGalaxyDef* HomeDef = GameTables.FindGalaxy(HomeRow);
		TestTrue(TEXT("naming the home galaxy"), HomeDef && GalaxyPanel->GetHintText() && GalaxyPanel->GetHintText()->GetText().ToString().StartsWith(HomeDef->DisplayName.ToString(), ESearchCase::CaseSensitive));
		TestFalse(TEXT("with no slot left in it"), GalaxyPanel->GetHintText() && GalaxyPanel->GetHintText()->GetText().ToString().Contains(TEXT("{")));
		Controller->SelectAndFocusGalaxy(HomeRow);
		TestTrue(TEXT("A first way back notes it"), Controller->GetHintProgress().bLeftUniverse);
		TestTrue(TEXT("and the hints are done"), GalaxyPanel->GetHintKey() == nullptr);
		const FCosmicHintProgress Done = Controller->GetHintProgress();
		Sim->NewGalaxy(42, 0, 0);
		Map->RefreshNow();
		TestEqual(TEXT("A new galaxy keeps the progress (the star's hint)"), Controller->GetHintProgress().bPickedSystem, Done.bPickedSystem);
		TestEqual(TEXT("A new galaxy keeps the progress (the way home's hint)"), Controller->GetHintProgress().bLeftUniverse, Done.bLeftUniverse);
		// A rebuild made while the camera stands at the universe frames the region itself: that is not the player's way
		// home, so the hint that teaches it stays (the review of item 9).
		Controller->SetHintProgress(FCosmicHintProgress());
		Pawn->Zoom(-100.f);
		Controller->FollowZoomLevel();
		TestTrue(TEXT("(the camera stands at the universe before the rebuild)"), Controller->GetZoomLevel() == ECosmicZoomLevel::Universe);
		Sim->NewGalaxy(42, 0, 0);
		Map->RefreshNow();
		TestTrue(TEXT("(the rebuild framed the region)"), Controller->GetZoomLevel() == ECosmicZoomLevel::Region);
		TestFalse(TEXT("A rebuild from the universe is no way home the player took"), Controller->GetHintProgress().bLeftUniverse);
		Controller->FollowZoomLevel();
		TestFalse(TEXT("nor is it noted on the next frame"), Controller->GetHintProgress().bLeftUniverse);
		Controller->SetHintProgress(Done);
		Restore();
		// The words of the new keys.
		const FString& Placeholder = FStringTableEntry::GetPlaceholderSourceString();
		const TArray<FString> Verdict = VerdictWords();
		TestTrue(TEXT("(the schema's verdict words were read)"), Verdict.Num() > 0);
		// Forbidden here: the words of an encounter, a reach of sight, a voyage, a flight, a gate, a way and a return, and the
		// words that would set named galaxies apart from the others.
		const TArray<FString> Forbidden = { TEXT("\uC870\uC6B0"), TEXT("\uC2DC\uC57C"), TEXT("\uD56D\uD574"), TEXT("\uBE44\uD589"), TEXT("\uBB38"), TEXT("\uAD00\uBB38"), TEXT("\uAE38"),
			TEXT("\uB3CC\uC544\uAC00"), TEXT("\uB3CC\uC544\uC624"), TEXT("\uC774\uB984 \uC788\uB294 \uC740\uD558"), TEXT("\uC774\uB984 \uC5C6\uB294") };
		for (const TCHAR* Key : { CosmicUI::HintZoomOutKey, CosmicUI::HintPickGalaxyKey, CosmicUI::HintHomeKey })
		{
			const FString Text = Words(Key);
			TestFalse(*FString::Printf(TEXT("%s is no missing entry"), Key), Text.Contains(Placeholder));
			TestFalse(*FString::Printf(TEXT("%s has words"), Key), Text.IsEmpty());
			TestFalse(*FString::Printf(TEXT("%s holds no digit"), Key), HasAsciiDigit(Text));
			bool bVerdict = false;
			for (const FString& Word : Verdict)
			{
				bVerdict |= Text.Contains(Word);
			}
			TestFalse(*FString::Printf(TEXT("%s holds no verdict word"), Key), bVerdict);
			bool bForbidden = false;
			for (const FString& Word : Forbidden)
			{
				bForbidden |= HoldsWord(Text, Word);
			}
			TestFalse(*FString::Printf(TEXT("%s holds no forbidden word"), Key), bForbidden);
		}
		TestTrue(TEXT("(the word check finds a one-syllable word standing alone)"), HoldsWord(FString(TEXT("\uC774 \uAE38 \uC704")), TEXT("\uAE38")));
		TestFalse(TEXT("(and not inside another word)"), HoldsWord(FString(TEXT("\uAE38\uC774")), TEXT("\uAE38")));
	}

	// ---- 6 Double click, the feed, the reveal and the aids ----
	{
		Restore();
		Pawn->FrameAll(Levels);
		Pawn->SnapToTargets();
		const FCosmicGalaxyMark* Andromeda = Map->GetGalaxyMarks().FindByPredicate([](const FCosmicGalaxyMark& Mark) { return Mark.Row == FName(TEXT("gal_andromeda")); });
		if (TestNotNull(TEXT("(the universe draws Andromeda)"), Andromeda))
		{
			const FCosmicGalaxyMark Mark = *Andromeda;
			TestTrue(TEXT("A double click on a galaxy is taken"), Controller->SelectAndFocusGalaxy(Mark.Row));
			TestEqual(TEXT("selects it"), Controller->GetSelectedGalaxy(), Mark.Row);
			TestEqual(TEXT("and glides to it at its focus arm"), Pawn->GetTargetArmLength(), FocusArmOf(Mark, Levels), 0.5f);
			TestTrue(TEXT("on its place"), Pawn->GetTargetFocus().Equals(Mark.Position, 0.5));
		}
		TestTrue(TEXT("A double click on the home is taken"), Controller->SelectAndFocusGalaxy(HomeRow));
		TestTrue(TEXT("Double-clicking the home goes back to the region"), Controller->GetZoomLevel() == ECosmicZoomLevel::Region);
		TestEqual(TEXT("framed"), Pawn->GetTargetArmLength(), CosmicZoom::FrameArmFactor * Levels.RegionRadius, 0.5f);
		TestTrue(TEXT("on the centre"), Pawn->GetTargetFocus().Equals(Levels.Centre, 0.5));
		TestEqual(TEXT("with no galaxy selected"), Controller->GetSelectedGalaxy(), FName(NAME_None));
		Pawn->Zoom(-3.f);
		TestEqual(TEXT("and the wheel still reaches the universe"), Pawn->GetTargetArmLength(), Levels.UniverseNear, 0.5f);
		const float Arm = Pawn->GetTargetArmLength();
		TestFalse(TEXT("A row the universe does not draw is refused"), Controller->SelectAndFocusGalaxy(FName(TEXT("gal_nowhere"))));
		TestEqual(TEXT("and the camera stays"), Pawn->GetTargetArmLength(), Arm);
		const FName System = Board.Systems.IsValidIndex(2) ? Board.Systems[2].SystemID : NAME_None;
		TestTrue(TEXT("(a feed click is taken)"), Controller->SelectAndFocus(System));
		TestTrue(TEXT("A feed click from the universe comes to the region"), Controller->GetZoomLevel() == ECosmicZoomLevel::Region);
		TestEqual(TEXT("and keeps the selection it made"), Controller->GetSelectedSystem(), System);
		Controller->SelectSystem(NAME_None);

		// The third rung's reveal at the universe level (amendment 6).
		UCosmicAscensionOverlayWidget* Overlay = HUD->GetAscensionOverlay();
		const TArray<FName> Feats = Sim->GetTables().GetFeatRowNames();
		if (TestNotNull(TEXT("(the HUD has the overlay)"), Overlay) && TestTrue(TEXT("(the game has a feat row)"), Feats.Num() > 0))
		{
			Controller->CosmicSpiritualSight(1);
			Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
			Pawn->Zoom(-100.f);
			Pawn->SnapToTargets();
			Controller->FollowZoomLevel();
			TestTrue(TEXT("(the camera stands at the universe level)"), Controller->GetZoomLevel() == ECosmicZoomLevel::Universe);
			TestTrue(TEXT("(and its arm is there, not only its target)"), Pawn->GetArmLength() > Levels.RegionFar);
			Overlay->Show(EAscensionTier::Tier3_Causal, Feats[0]);
			Overlay->Close();
			TestTrue(TEXT("Closing the third rung's presentation at the universe level frames the region"), Controller->GetZoomLevel() == ECosmicZoomLevel::Region);
			TestTrue(TEXT("and starts the reveal"), Overlay->IsRevealing());
			TestEqual(TEXT("over the region's framing at once (no glide hides the stars)"), Pawn->GetArmLength(), Pawn->GetTargetArmLength(), 0.5f);
			Overlay->Advance(2.f * CosmicAscensionOverlay::RevealSeconds);
			Controller->CosmicSpiritualSight(0);
		}

		// The development aids.
		Pawn->FrameAll(Levels);
		Pawn->SnapToTargets();
		Controller->CosmicWheel(-100.f);
		TestEqual(TEXT("CosmicWheel -100 on a built map frames the universe's far stop"), Pawn->GetTargetArmLength(), Levels.UniverseFar, 0.5f);
		Sim->NewGalaxy(42, 0, 0);
		Controller->CosmicWheel(-100.f);
		TestEqual(TEXT("A CosmicWheel before the build waits"), Pawn->GetTargetArmLength(), Levels.UniverseFar, 0.5f);
		Map->RefreshNow();
		TestEqual(TEXT("and turns after it (from the framing)"), Pawn->GetTargetArmLength(), Map->GetZoomLevels().UniverseFar, 0.5f);
		Sim->NewGalaxy(42, 0, 0);
		Controller->CosmicSelectGalaxy(TEXT("gal_andromeda"));
		TestEqual(TEXT("A CosmicSelectGalaxy before the build waits"), Controller->GetSelectedGalaxy(), FName(NAME_None));
		Map->RefreshNow();
		TestEqual(TEXT("and selects after it"), Controller->GetSelectedGalaxy(), FName(TEXT("gal_andromeda")));
		Controller->CosmicSelectGalaxy(TEXT("gal_nowhere"));
		TestEqual(TEXT("On a built map it refuses a row the universe does not draw"), Controller->GetSelectedGalaxy(), FName(TEXT("gal_andromeda")));
		Restore();
	}

	// ---- 7 The sky ----
	{
		const UStaticMeshComponent* Sky = Map->GetSky();
		if (TestNotNull(TEXT("(the map has its sky)"), Sky))
		{
			const double Radius = Sky->GetComponentScale().X * SkySphereRadius;
			TestTrue(TEXT("The sky holds the universe"), Radius >= SkyRadiusOf(Levels, Frame, Map->GetActorLocation()) - 1.0);
			TestTrue(TEXT("and every camera place"), Radius >= Levels.UniverseBound + Levels.UniverseFar);
			TestTrue(TEXT("and the scenery's edge"), Radius >= SceneryOuterUnits * Frame.Unit);
			TestTrue(TEXT("The sky stays at the map's centre"), Sky->GetComponentLocation().Equals(Map->GetActorLocation(), 0.01));
		}
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

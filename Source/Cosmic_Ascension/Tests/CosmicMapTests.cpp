// Cosmic Ascension - automation tests of the galaxy map (Presentation). Design: guideline.md section 6.4, 9.3, 11 Phase 3.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/EngineTypes.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameplayTagContainer.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionNoise.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameters.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "World/CosmicGalaxyMapActor.h"
#include "World/CosmicMapLook.h"
#include "World/CosmicStarSystemActor.h"

#include <limits>

namespace CosmicMapTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	ACosmicGalaxyMapActor* FindMap(UWorld* World)
	{
		// The iterator checks its world as it is made, before any loop condition.
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
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIGalaxyMapTest, "Cosmic_Ascension.UI.GalaxyMap", CosmicTests::Flags)
bool FCosmicUIGalaxyMapTest::RunTest(const FString& Parameters)
{
	using namespace CosmicMapTests;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	Sim->SetTables(Original);
	ACosmicGalaxyMapActor* Map = FindMap(Sim->GetGameInstance()->GetWorld());
	if (!TestNotNull(TEXT("The game mode spawned the map"), Map))
	{
		return false;
	}

	// ---- The map of the game's galaxy: one star per system at its position, every lane once ----
	{
		Sim->NewGalaxy(42, 0, 0);
		// The whole board, as before Phase 3 item 8: every system known and every people met (UI.KnownFilter holds the
		// gate itself).
		CosmicTests::KnowAll(Sim->GetMutableState());
		Map->RefreshNow();
		const FGalaxyState& State = Sim->GetState();
		TestEqual(TEXT("One star actor per system"), Map->GetStars().Num(), State.Systems.Num());
		bool bPlaced = true;
		bool bBound = true;
		bool bFound = true;
		int32 Listed = 0;
		int32 Owned = 0;
		for (int32 Index = 0; Index < State.Systems.Num() && Index < Map->GetStars().Num(); ++Index)
		{
			const ACosmicStarSystemActor* Star = Map->GetStars()[Index];
			const FStarSystemState& System = State.Systems[Index];
			bPlaced &= Star && Star->GetActorLocation().Equals(System.Position, 0.01);
			bBound &= Star && Star->GetSystemID() == System.SystemID;
			bFound &= Map->FindStar(System.SystemID) == Star;
			Listed += System.Lanes.Num();
			Owned += State.Civs.ContainsByPredicate([&System](const FCivilizationState& Civ) { return Civ.CivID == System.OwnerCivID; }) ? 1 : 0;
		}
		TestTrue(TEXT("Every star stands at its system's position"), bPlaced);
		TestTrue(TEXT("and draws its system"), bBound);
		TestTrue(TEXT("and is found by its system"), bFound);
		TestTrue(TEXT("(the galaxy has lanes, each listed by both ends)"), Listed > 0 && Listed % 2 == 0);
		TestEqual(TEXT("Every lane is drawn once"), Map->GetDrawnLaneCount(), Listed / 2);
		TestNull(TEXT("No star draws a system the state does not hold"), Map->FindStar(FName(TEXT("sys_nowhere"))));
		TestTrue(TEXT("(the galaxy has owned systems and wild ones)"), Owned > 0 && Owned < State.Systems.Num());
		TestEqual(TEXT("One owner's ring per owned system"), Map->GetDrawnRingCount(), Owned);
	}

	// ---- A tick only marks the map; the next frame (RefreshNow here) applies it ----
	{
		FGalaxyState& Live = Sim->GetMutableState();
		const int32 Index = 1;
		if (TestTrue(TEXT("(the board has a second system and its star)"), Live.Systems.IsValidIndex(Index) && Map->GetStars().IsValidIndex(Index) && Map->GetStars()[Index]))
		{
			Live.Systems[Index].ActiveHazard = Tag(TEXT("Hazard.AIAwakening"));
			Live.Systems[Index].HazardTicksRemaining = 1000;
			TestFalse(TEXT("(the star shows no hazard yet)"), Map->GetStars()[Index]->GetAppliedLook().bHazard);
			Sim->StepOnce();
			TestTrue(TEXT("(the hazard still stands after the tick)"), Sim->GetState().Systems[Index].ActiveHazard.IsValid());
			TestFalse(TEXT("A tick does not redraw the map by itself"), Map->GetStars()[Index]->GetAppliedLook().bHazard);
			Map->RefreshNow();
			TestTrue(TEXT("The next frame shows the hazard"), Map->GetStars()[Index]->GetAppliedLook().bHazard);
		}
	}

	// ---- A new galaxy and a load rebuild the map ----
	{
		const FGalaxyState Saved = Sim->GetState();
		Sim->NewGalaxy(7, 5, 3);
		Map->RefreshNow();
		TestEqual(TEXT("A new galaxy rebuilds the map: its five systems and the sanctuary"), Map->GetStars().Num(), 6);
		Sim->LoadState(Saved);
		Map->RefreshNow();
		TestEqual(TEXT("A load rebuilds it as well"), Map->GetStars().Num(), Saved.Systems.Num());
	}

	// ---- LookOf on a hand-made board ----
	{
		Sim->NewGalaxy(42, 5, 3);
		FGalaxyState Board = Sim->GetState();
		const FCosmicDataTables& Tables = Sim->GetTables();
		if (!TestTrue(TEXT("(the board has five systems and a people)"), CosmicTests::LivingSystems(Board) == 5 && Board.Civs.Num() >= 1))
		{
			return false;
		}
		for (FStarSystemState& System : Board.Systems)
		{
			System.OwnerCivID = NAME_None;
			System.ActiveHazard = FGameplayTag();
			System.ActiveMegaproject = NAME_None;
			System.ResponseMegaproject = NAME_None;
			System.StarClass = Tag(TEXT("Star.Class.G"));
		}
		Board.Civs[0].Color = FLinearColor(0.2f, 0.4f, 0.6f);

		TestFalse(TEXT("A system without an owner is drawn without one"), CosmicMapLook::LookOf(Board, Tables, 0).bOwned);
		Board.Systems[1].OwnerCivID = Board.Civs[0].CivID;
		const FCosmicSystemLook Owned = CosmicMapLook::LookOf(Board, Tables, 1);
		TestTrue(TEXT("An owned system is drawn with its owner"), Owned.bOwned);
		// The drawn colour is the meeting order's (ShownLookOf): the world's look carries no colour of the state.
		TestEqual(TEXT("LookOf draws no colour of the state"), Owned.OwnerColor, FLinearColor::Black);
		TestTrue(TEXT("and with its name"), Owned.Name.EqualTo(Board.Systems[1].DisplayName));
		Board.Systems[2].OwnerCivID = FName(TEXT("civ_99"));
		TestFalse(TEXT("An owner the state does not hold draws no owner"), CosmicMapLook::LookOf(Board, Tables, 2).bOwned);

		// The two slots: a share of what the row needs, -1 for an empty slot or an unknown row, clamped, a non-number is 0.
		const FMegaprojectDef* Settle = Tables.FindMegaproject(CosmicRows::ProjectSettle);
		if (TestNotNull(TEXT("(the game's tables hold the settlement row)"), Settle))
		{
			Board.Systems[1].ActiveMegaproject = CosmicRows::ProjectSettle;
			Board.Systems[1].MegaprojectProgress = 0.5f * CosmicMegaprojects::RequiredProgressOf(*Settle);
			const FCosmicSystemLook Settling = CosmicMapLook::LookOf(Board, Tables, 1);
			TestTrue(TEXT("A settlement under way is drawn as one"), Settling.bSettling);
			TestEqual(TEXT("with the share of its progress"), Settling.ConstructionShare, 0.5f, 1e-4f);
			Board.Systems[1].MegaprojectProgress = 1.0e9f;
			TestEqual(TEXT("A progress past what the row needs shows as whole"), CosmicMapLook::LookOf(Board, Tables, 1).ConstructionShare, 1.f);
			Board.Systems[1].MegaprojectProgress = std::numeric_limits<float>::quiet_NaN();
			TestEqual(TEXT("A progress that is not a number shows as none"), CosmicMapLook::LookOf(Board, Tables, 1).ConstructionShare, 0.f);
		}
		TestEqual(TEXT("An empty construction slot shows no bar"), CosmicMapLook::LookOf(Board, Tables, 0).ConstructionShare, -1.f);
		TestEqual(TEXT("An empty response slot shows no bar"), CosmicMapLook::LookOf(Board, Tables, 1).ResponseShare, -1.f);
		Board.Systems[1].ResponseMegaproject = FName(TEXT("project_nowhere"));
		TestEqual(TEXT("A row the tables do not hold shows no bar"), CosmicMapLook::LookOf(Board, Tables, 1).ResponseShare, -1.f);
		TestFalse(TEXT("A system that is not being settled is not drawn as one"), CosmicMapLook::LookOf(Board, Tables, 0).bSettling);
		Board.Systems[2].ActiveMegaproject = FName(TEXT("project_nowhere"));
		TestFalse(TEXT("nor one that builds something else"), CosmicMapLook::LookOf(Board, Tables, 2).bSettling);

		Board.Systems[3].ActiveHazard = Tag(TEXT("Hazard.AIAwakening"));
		TestTrue(TEXT("A system with a hazard is drawn with it"), CosmicMapLook::LookOf(Board, Tables, 3).bHazard);
		TestFalse(TEXT("and one without, without"), CosmicMapLook::LookOf(Board, Tables, 0).bHazard);

		// A dead node: no owner and no hazard even when the slots still say so.
		Board.Systems[4].StarClass = FGameplayTag();
		Board.Systems[4].OwnerCivID = Board.Civs[0].CivID;
		Board.Systems[4].ActiveHazard = Tag(TEXT("Hazard.AIAwakening"));
		const FCosmicSystemLook Dead = CosmicMapLook::LookOf(Board, Tables, 4);
		TestTrue(TEXT("A destroyed system is drawn dead"), Dead.bDead);
		TestFalse(TEXT("without an owner"), Dead.bOwned);
		TestFalse(TEXT("and without a hazard"), Dead.bHazard);
		TestFalse(TEXT("A living system is not drawn dead"), CosmicMapLook::LookOf(Board, Tables, 0).bDead);

		// The star classes: hot and large to cool and small, each its own colour; a tag that is no class is white and 1.
		const TCHAR* Classes[] = { TEXT("Star.Class.O"), TEXT("Star.Class.B"), TEXT("Star.Class.A"), TEXT("Star.Class.F"), TEXT("Star.Class.G"), TEXT("Star.Class.K"), TEXT("Star.Class.M") };
		float LastScale = TNumericLimits<float>::Max();
		FLinearColor LastColor = FLinearColor::Transparent;
		for (const TCHAR* Class : Classes)
		{
			Board.Systems[0].StarClass = Tag(Class);
			const FCosmicSystemLook Look = CosmicMapLook::LookOf(Board, Tables, 0);
			TestTrue(*FString::Printf(TEXT("%s is smaller than the class before it"), Class), Look.StarScale < LastScale);
			TestFalse(*FString::Printf(TEXT("%s has a colour of its own"), Class), Look.StarColor.Equals(LastColor));
			LastScale = Look.StarScale;
			LastColor = Look.StarColor;
		}
		Board.Systems[0].StarClass = Tag(TEXT("Hazard.Plague"));
		TestEqual(TEXT("A tag that is no star class is drawn white"), CosmicMapLook::LookOf(Board, Tables, 0).StarColor, FLinearColor::White);
		TestEqual(TEXT("at scale 1"), CosmicMapLook::LookOf(Board, Tables, 0).StarScale, 1.f);
		TestTrue(TEXT("An index out of range is the default look"), CosmicMapLook::LookOf(Board, Tables, 99).Name.IsEmpty());

		// The lanes: one listed by both ends once, one listed by one end too, none to a dead node or to a stranger.
		for (FStarSystemState& System : Board.Systems)
		{
			System.Lanes.Reset();
		}
		Board.Systems[0].Lanes = { Board.Systems[1].SystemID };
		Board.Systems[1].Lanes = { Board.Systems[0].SystemID, Board.Systems[2].SystemID };
		Board.Systems[3].Lanes = { Board.Systems[4].SystemID, FName(TEXT("sys_nowhere")) };
		const TArray<TPair<int32, int32>> Lanes = CosmicMapLook::LanesOf(Board);
		TestEqual(TEXT("Two lanes are drawn"), Lanes.Num(), 2);
		TestTrue(TEXT("the lane both ends list, once"), Lanes.Contains(TPair<int32, int32>(0, 1)));
		TestTrue(TEXT("the lane one end lists"), Lanes.Contains(TPair<int32, int32>(1, 2)));
		TestFalse(TEXT("no lane to a dead node"), Lanes.Contains(TPair<int32, int32>(3, 4)));
	}
	return true;
}

// The names C++ sets on the generated materials are the materials' own (a dynamic instance takes any name silently, so a
// misspelt name failed nothing before), and the defaults of the star's values are the material's: a star given nothing
// looks as the material does. Asks the base assets (a dynamic instance answers for any name once it was set).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIMaterialParamsTest, "Cosmic_Ascension.UI.MaterialParams", CosmicTests::Flags)
bool FCosmicUIMaterialParamsTest::RunTest(const FString& Parameters)
{
	auto Scalar = [](const UMaterialInterface& Material, const TCHAR* Name, float& Out)
	{
		return Material.GetScalarParameterValue(FHashedMaterialParameterInfo(FName(Name)), Out);
	};
	auto Vector = [](const UMaterialInterface& Material, const TCHAR* Name, FLinearColor& Out)
	{
		return Material.GetVectorParameterValue(FHashedMaterialParameterInfo(FName(Name)), Out);
	};

	const UMaterialInterface* Star = LoadObject<UMaterialInterface>(nullptr, CosmicMapLook::StarMaterialPath, nullptr, LOAD_NoWarn);
	if (!TestNotNull(TEXT("(M_Star loads)"), Star))
	{
		return false;
	}
	for (const TCHAR* Name : CosmicMapLook::StarScalarParams)
	{
		float Value = 0.f;
		TestTrue(*FString::Printf(TEXT("M_Star has the scalar %s"), Name), Scalar(*Star, Name, Value));
	}
	for (const TCHAR* Name : CosmicMapLook::StarVectorParams)
	{
		FLinearColor Value;
		TestTrue(*FString::Printf(TEXT("M_Star has the vector %s"), Name), Vector(*Star, Name, Value));
	}

	// The defaults of FCosmicStarParams are the material's own.
	const FCosmicStarParams Defaults;
	const TPair<const TCHAR*, float> Scalars[] = {
		{ CosmicMapLook::StarParam::Brightness, Defaults.Brightness },
		{ CosmicMapLook::StarParam::HazardAmount, Defaults.HazardAmount },
		{ CosmicMapLook::StarParam::ViewMode, Defaults.ViewMode },
		{ CosmicMapLook::StarParam::Pressure, Defaults.Pressure },
		{ CosmicMapLook::StarParam::ClaimState, Defaults.ClaimState },
	};
	TestEqual(TEXT("(every scalar has its default here)"), static_cast<int32>(UE_ARRAY_COUNT(Scalars)), static_cast<int32>(UE_ARRAY_COUNT(CosmicMapLook::StarScalarParams)));
	for (const TPair<const TCHAR*, float>& Each : Scalars)
	{
		float Value = -1.f;
		TestTrue(*FString::Printf(TEXT("M_Star's %s starts at the default star's"), Each.Key), Scalar(*Star, Each.Key, Value) && FMath::IsNearlyEqual(Value, Each.Value, 1e-5f));
	}
	const TPair<const TCHAR*, FLinearColor> Vectors[] = {
		{ CosmicMapLook::StarParam::StarColor, Defaults.StarColor },
		{ CosmicMapLook::StarParam::WarningColor, Defaults.WarningColor },
		{ CosmicMapLook::StarParam::DominionColor, Defaults.DominionColor },
	};
	TestEqual(TEXT("(every vector has its default here)"), static_cast<int32>(UE_ARRAY_COUNT(Vectors)), static_cast<int32>(UE_ARRAY_COUNT(CosmicMapLook::StarVectorParams)));
	for (const TPair<const TCHAR*, FLinearColor>& Each : Vectors)
	{
		FLinearColor Value = FLinearColor::Transparent;
		TestTrue(*FString::Printf(TEXT("M_Star's %s starts at the default star's"), Each.Key), Vector(*Star, Each.Key, Value) && Value.Equals(Each.Value, 1e-5f));
	}

	const UMaterialInterface* Nebula = LoadObject<UMaterialInterface>(nullptr, CosmicMapLook::NebulaMaterialPath, nullptr, LOAD_NoWarn);
	if (!TestNotNull(TEXT("(M_Nebula loads)"), Nebula))
	{
		return false;
	}
	float Intensity = -1.f;
	TestTrue(TEXT("M_Nebula has the scalar SpiritualIntensity"), Scalar(*Nebula, CosmicMapLook::SkyIntensityParam, Intensity));
	TestEqual(TEXT("which starts at 0"), Intensity, 0.f);
	// Phase 3 item 9: the sky's two colours keep below the fallen's band (the purple of the sight is theirs alone), and
	// its noise reads the view's direction - a Position of its own at scale 1.
	for (const TCHAR* Name : { CosmicMapLook::NebulaLowParam, CosmicMapLook::NebulaHighParam })
	{
		FLinearColor Colour = FLinearColor::Transparent;
		TestTrue(*FString::Printf(TEXT("M_Nebula has the vector %s"), Name), Vector(*Nebula, Name, Colour));
		const float Hue = Colour.LinearRGBToHSV().R;
		TestTrue(*FString::Printf(TEXT("M_Nebula's %s keeps out of the fallen's band"), Name), Hue < CosmicMapLook::FallenHueFrom || Hue > CosmicMapLook::FallenHueTo);
		TestTrue(*FString::Printf(TEXT("M_Nebula's %s is a blue"), Name), Hue > 180.f && Hue < CosmicMapLook::FallenHueFrom);
	}
#if WITH_EDITOR
	{
		const UMaterial* Base = Nebula->GetMaterial();
		const UMaterialExpressionNoise* Noise = nullptr;
		int32 Noises = 0;
		if (Base)
		{
			for (const UMaterialExpression* Expression : Base->GetExpressions())
			{
				if (const UMaterialExpressionNoise* Each = Cast<UMaterialExpressionNoise>(Expression))
				{
					Noise = Each;
					++Noises;
				}
			}
		}
		TestEqual(TEXT("(M_Nebula holds one noise)"), Noises, 1);
		TestTrue(TEXT("M_Nebula's noise reads a Position of its own"), Noise && Noise->Position.GetTracedInput().Expression != nullptr);
		TestTrue(TEXT("at scale 1"), Noise && Noise->Scale == 1.f);
	}
#endif

	// The backdrop's dust (Phase 3 item 8): the sky's one parameter, the instanced usage the asset must carry (under -game
	// a material without it falls back to the engine's default - only the editor proper adds a missing usage), and an
	// additive blend that writes no depth over the lanes.
	UMaterialInterface* Dust = LoadObject<UMaterialInterface>(nullptr, CosmicMapLook::BackdropMaterialPath, nullptr, LOAD_NoWarn);
	if (!TestNotNull(TEXT("(M_Backdrop loads)"), Dust))
	{
		return false;
	}
	float DustIntensity = -1.f;
	TestTrue(TEXT("M_Backdrop has the scalar SpiritualIntensity"), Scalar(*Dust, CosmicMapLook::SkyIntensityParam, DustIntensity));
	TestEqual(TEXT("M_Backdrop's SpiritualIntensity starts at 0"), DustIntensity, 0.f);
	TestTrue(TEXT("M_Backdrop may draw instanced meshes"), Dust->GetUsageByFlag(MATUSAGE_InstancedStaticMeshes));
	TestTrue(TEXT("M_Backdrop is additive"), Dust->GetBlendMode() == BLEND_Additive);
	// Phase 3 item 9: the share of its light a layer shows (the dust and the universe), and the one tint of the dust and of
	// every galaxy - never the fallen's hue, never white (no galaxy is the holy order's, section 6.4 'colour').
	float Fade = -1.f;
	TestTrue(TEXT("M_Backdrop has the scalar LayerFade"), Scalar(*Dust, CosmicMapLook::LayerFadeParam, Fade));
	TestEqual(TEXT("which starts at 1"), Fade, 1.f);
	FLinearColor Tint = FLinearColor::Transparent;
	TestTrue(TEXT("M_Backdrop has the vector Tint"), Vector(*Dust, CosmicMapLook::BackdropTintParam, Tint));
	const FLinearColor TintHsv = Tint.LinearRGBToHSV();
	TestTrue(TEXT("whose hue keeps out of the fallen's band"), TintHsv.R < CosmicMapLook::FallenHueFrom || TintHsv.R > CosmicMapLook::FallenHueTo);
	TestTrue(TEXT("and is not white"), TintHsv.G >= 0.15f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

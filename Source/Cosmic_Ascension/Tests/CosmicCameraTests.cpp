// Cosmic Ascension - automation tests of the camera and the selection (Presentation). Design: guideline.md section 6.4,
// 9.3, 11 Phase 3.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Camera/CameraComponent.h"
#include "Core/GalaxyState.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "World/CosmicCameraPawn.h"
#include "World/CosmicGalaxyMapActor.h"
#include "World/CosmicGameMode.h"
#include "World/CosmicMapLook.h"
#include "World/CosmicMapPick.h"
#include "World/CosmicStarSystemActor.h"
#include "World/CosmicZoomLevels.h"

#include <limits>

namespace CosmicCameraTests
{
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

	int32 FindSystem(const FGalaxyState& State, FName SystemID)
	{
		return State.Systems.IndexOfByPredicate([SystemID](const FStarSystemState& System) { return System.SystemID == SystemID; });
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUICameraAndSelectionTest, "Cosmic_Ascension.UI.CameraAndSelection", CosmicTests::Flags)
bool FCosmicUICameraAndSelectionTest::RunTest(const FString& Parameters)
{
	using namespace CosmicCameraTests;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UWorld* World = Sim->GetGameInstance()->GetWorld();
	ACosmicGalaxyMapActor* Map = FindMap(World);
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(World ? World->GetFirstPlayerController() : nullptr);
	if (!TestNotNull(TEXT("The game mode spawned the map"), Map) || !TestNotNull(TEXT("The first player controller is the game's"), Controller))
	{
		return false;
	}
	ACosmicCameraPawn* Pawn = Cast<ACosmicCameraPawn>(Controller->GetPawn());
	if (!TestNotNull(TEXT("The first player controller possesses a camera pawn"), Pawn))
	{
		return false;
	}
	TestTrue(TEXT("and looks through it"), Controller->GetViewTarget() == Pawn);
	TestNull(TEXT("The map has no camera of its own"), Map->FindComponentByClass<UCameraComponent>());
	ON_SCOPE_EXIT
	{
		Controller->SelectSystem(NAME_None);
		Sim->NewGalaxy(42, 5, 3);
	};

	// ---- A rebuild frames the whole disc and puts the camera there at once ----
	{
		Sim->NewGalaxy(42, 0, 0);
		// The whole board known, as before Phase 3 item 8 (the selection takes known systems only - UI.KnownFilter).
		CosmicTests::KnowAll(Sim->GetMutableState());
		Pawn->Orbit(30.f, 10.f);
		Pawn->Zoom(3.f);
		Map->RefreshNow();
		const FGalaxyState& State = Sim->GetState();
		const float Radius = Map->GetDiscRadius();
		bool bCovered = true;
		for (const FStarSystemState& System : State.Systems)
		{
			bCovered &= FVector::Dist2D(System.Position, Map->GetDiscCentre()) <= Radius + 0.01;
		}
		TestTrue(TEXT("(the disc of the game's galaxy is larger than the smallest)"), Radius > 1000.f);
		TestTrue(TEXT("The disc radius reaches every system"), bCovered);
		TestTrue(TEXT("A rebuild frames the disc: the focus is its centre"), Pawn->GetTargetFocus().Equals(Map->GetDiscCentre(), 0.01));
		TestEqual(TEXT("the arm is 2.25 radii"), Pawn->GetTargetArmLength(), 2.25f * Radius, 0.5f);
		TestEqual(TEXT("the pitch is -59"), Pawn->GetTargetPitch(), -59.f);
		TestEqual(TEXT("the yaw is 0"), Pawn->GetTargetYaw(), 0.f);
		TestEqual(TEXT("and the camera is there at once"), Pawn->GetArmLength(), Pawn->GetTargetArmLength());
	}
	const float Radius = Map->GetDiscRadius();
	const FVector Centre = Map->GetDiscCentre();
	const int32 SolIndex = FindSystem(Sim->GetState(), FName(TEXT("sol")));
	if (!TestTrue(TEXT("(the game's galaxy holds sol)"), SolIndex != INDEX_NONE))
	{
		return false;
	}
	const FVector Sol = Sim->GetState().Systems[SolIndex].Position;

	// ---- Zoom, orbit, pan and focus keep to their bounds ----
	{
		const float Framed = Pawn->GetTargetArmLength();
		Pawn->Zoom(1.f);
		TestEqual(TEXT("One wheel step in comes nearer by x0.85"), Pawn->GetTargetArmLength(), Framed * 0.85f, 0.5f);
		Pawn->Zoom(100.f);
		TestEqual(TEXT("Many steps in stop at 1,500"), Pawn->GetTargetArmLength(), 1500.f);
		// At 1,500 the target arm is the region's: its bound holds a far pan.
		Pawn->Pan(FVector(100.f * Radius, 0.f, 0.f));
		TestEqual(TEXT("A far pan in the region stops 1.1 radii from the centre"), FVector::Dist2D(Pawn->GetTargetFocus(), Centre), 1.1 * Radius, 0.5);
		TestTrue(TEXT("A pan moves the camera at once"), Pawn->GetActorLocation().Equals(Pawn->GetTargetFocus(), 0.01));
		Pawn->Zoom(-100.f);
		TestEqual(TEXT("Many steps out stop at the universe's far stop"), Pawn->GetTargetArmLength(), Pawn->GetZoomLevels().UniverseFar, 0.5f);
		TestTrue(TEXT("(the far stop lies past the region)"), Pawn->GetTargetArmLength() > 3.f * Radius);
		Pawn->Pan(FVector(100.f * Radius, 0.f, 0.f));
		TestEqual(TEXT("At the universe level a far pan stops at the universe's bound"), FVector::Dist2D(Pawn->GetTargetFocus(), Centre), static_cast<double>(Pawn->GetZoomLevels().UniverseBound), 0.5);
		Pawn->Orbit(0.f, -100.f);
		TestEqual(TEXT("The pitch stops at -85"), Pawn->GetTargetPitch(), -85.f);
		Pawn->Orbit(0.f, 200.f);
		TestEqual(TEXT("and at -20"), Pawn->GetTargetPitch(), -20.f);
		Pawn->Orbit(400.f, 0.f);
		TestEqual(TEXT("The yaw goes round freely"), Pawn->GetTargetYaw(), 40.f, 0.01f);
		Pawn->FocusOn(Sol);
		TestTrue(TEXT("Focusing aims at the system"), Pawn->GetTargetFocus().Equals(FVector(Sol.X, Sol.Y, 0.0), 0.01));
		TestEqual(TEXT("and comes to arm 3,000"), Pawn->GetTargetArmLength(), 3000.f);
		TestTrue(TEXT("and into the region"), Pawn->GetTargetLevel() == ECosmicZoomLevel::Region);
		// A drag during a glide moves the camera by the drag, from where it is (Phase 3 item 9, amendment 13): no jump to the
		// target.
		const FVector Before = Pawn->GetActorLocation();
		const FVector Drag(250.f, -100.f, 0.f);
		TestFalse(TEXT("(the camera is still gliding toward the system)"), Before.Equals(Pawn->GetTargetFocus(), 1.0));
		Pawn->Pan(Drag);
		TestTrue(TEXT("A pan during a glide moves the camera by the pan"), Pawn->GetActorLocation().Equals(Before + Drag, 0.01));
		Pawn->Zoom(100.f);
		Pawn->FocusOn(Sol);
		TestEqual(TEXT("A nearer arm stays when focusing"), Pawn->GetTargetArmLength(), 1500.f);
		Pawn->SnapToTargets();
		TestEqual(TEXT("A snap puts the arm at its target"), Pawn->GetArmLength(), 1500.f);
	}

	// ---- The selection: announced once per change, ringed by the map, cleared by a new galaxy ----
	{
		int32 Announced = 0;
		FName LastAnnounced;
		const FDelegateHandle Handle = Controller->OnSelectionChanged.AddLambda([&Announced, &LastAnnounced](FName SystemID)
		{
			++Announced;
			LastAnnounced = SystemID;
		});
		ON_SCOPE_EXIT { Controller->OnSelectionChanged.Remove(Handle); };
		Controller->SelectSystem(NAME_None);
		Announced = 0;

		const FName Picked = Sim->GetState().Systems.IsValidIndex(2) ? Sim->GetState().Systems[2].SystemID : NAME_None;
		Controller->SelectSystem(Picked);
		TestEqual(TEXT("A system of the state is selected"), Controller->GetSelectedSystem(), Picked);
		TestEqual(TEXT("and announced once"), Announced, 1);
		TestEqual(TEXT("with its ID"), LastAnnounced, Picked);
		TestEqual(TEXT("(the map waits for the next frame)"), Map->GetDrawnSelection(), FName(NAME_None));
		Map->RefreshNow();
		TestEqual(TEXT("The map rings the selected system"), Map->GetDrawnSelection(), Picked);
		Controller->SelectSystem(Picked);
		TestEqual(TEXT("Selecting it again announces nothing"), Announced, 1);
		Controller->SelectSystem(FName(TEXT("sys_nowhere")));
		TestEqual(TEXT("An ID the state does not hold is ignored"), Controller->GetSelectedSystem(), Picked);
		TestEqual(TEXT("and not announced"), Announced, 1);
		Controller->SelectSystem(NAME_None);
		TestEqual(TEXT("None clears the selection"), Controller->GetSelectedSystem(), FName(NAME_None));
		TestEqual(TEXT("announced as None"), LastAnnounced, FName(NAME_None));
		TestEqual(TEXT("(a second announcement)"), Announced, 2);
		Map->RefreshNow();
		TestEqual(TEXT("and the ring goes"), Map->GetDrawnSelection(), FName(NAME_None));

		Controller->SelectSystem(Picked);
		Map->RefreshNow();
		TestEqual(TEXT("(selected and ringed again)"), Map->GetDrawnSelection(), Picked);
		Sim->NewGalaxy(7, 5, 3);
		Map->RefreshNow();
		TestEqual(TEXT("A new galaxy clears the selection"), Controller->GetSelectedSystem(), FName(NAME_None));
		TestEqual(TEXT("and announces it"), LastAnnounced, FName(NAME_None));
		TestEqual(TEXT("and its ring"), Map->GetDrawnSelection(), FName(NAME_None));
		TestTrue(TEXT("and frames the new disc"), Pawn->GetTargetFocus().Equals(Map->GetDiscCentre(), 0.01));
		TestEqual(TEXT("from its own distance"), Pawn->GetTargetArmLength(), 2.25f * Map->GetDiscRadius(), 0.5f);
		TestTrue(TEXT("and opens the universe"), Pawn->GetZoomLevels().HasUniverse());
	}

	// ---- CosmicSelect given before a build waits for it (the old galaxy's stars still stand) ----
	{
		TestTrue(TEXT("(the old galaxy has a star named sol)"), Map->FindStar(FName(TEXT("sol"))) != nullptr);
		Sim->NewGalaxy(42, 0, 0);
		CosmicTests::KnowAll(Sim->GetMutableState());
		Controller->CosmicSelect(TEXT("sol"));
		TestEqual(TEXT("A CosmicSelect before the build waits"), Controller->GetSelectedSystem(), FName(NAME_None));
		Map->RefreshNow();
		TestEqual(TEXT("and selects after it"), Controller->GetSelectedSystem(), FName(TEXT("sol")));
		TestTrue(TEXT("and focuses on the new galaxy's sol"), Pawn->GetTargetFocus().Equals(FVector(Sol.X, Sol.Y, 0.0), 0.01));
		TestEqual(TEXT("from arm 3,000"), Pawn->GetTargetArmLength(), 3000.f);
		const FName Other = Sim->GetState().Systems.IsValidIndex(3) ? Sim->GetState().Systems[3].SystemID : NAME_None;
		Controller->CosmicSelect(Other.ToString());
		TestEqual(TEXT("On a built map it selects at once"), Controller->GetSelectedSystem(), Other);
	}

	// ---- From afar labels thin out to the owned, the hazards and the selection ----
	{
		Sim->NewGalaxy(42, 0, 0);
		CosmicTests::KnowAll(Sim->GetMutableState());
		Map->RefreshNow();
		FGalaxyState& Live = Sim->GetMutableState();
		TArray<int32> Wild;
		int32 Owned = INDEX_NONE;
		for (int32 Index = 0; Index < Live.Systems.Num(); ++Index)
		{
			if (Live.Systems[Index].OwnerCivID.IsNone())
			{
				Wild.Add(Index);
			}
			else if (Owned == INDEX_NONE)
			{
				Owned = Index;
			}
		}
		if (!TestTrue(TEXT("(three wild systems and an owned one)"), Wild.Num() >= 3 && Owned != INDEX_NONE))
		{
			return false;
		}
		TestEqual(TEXT("(20,000 lies in the region on this disc)"), CosmicZoom::UniverseShareOf(20000.f, Map->GetZoomLevels()), 0.f);
		Live.Systems[Wild[1]].ActiveHazard = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.AIAwakening")));
		Live.Systems[Wild[1]].HazardTicksRemaining = 1000;
		Controller->SelectSystem(Live.Systems[Wild[2]].SystemID);
		Map->SetViewDistance(20000.f);
		Map->RefreshNow();
		auto LabelOf = [Map, &Live](int32 Index)
		{
			const ACosmicStarSystemActor* Star = Map->FindStar(Live.Systems[Index].SystemID);
			return Star && Star->IsLabelVisible();
		};
		TestTrue(TEXT("20,000 away is the far view"), Map->IsFarView());
		TestFalse(TEXT("From afar a wild system's label is hidden"), LabelOf(Wild[0]));
		TestTrue(TEXT("an owned system's shows"), LabelOf(Owned));
		TestTrue(TEXT("a system with a hazard shows"), LabelOf(Wild[1]));
		TestTrue(TEXT("the selected one shows"), LabelOf(Wild[2]));
		Map->SetViewDistance(3000.f);
		Map->RefreshNow();
		TestFalse(TEXT("3,000 away is near"), Map->IsFarView());
		TestTrue(TEXT("Near, a wild system's label shows"), LabelOf(Wild[0]));

		FCosmicSystemLook Look;
		TestFalse(TEXT("ShowsLabel: from afar a plain system has none"), CosmicMapLook::ShowsLabel(Look, false, true));
		TestTrue(TEXT("near it has one"), CosmicMapLook::ShowsLabel(Look, false, false));
		TestTrue(TEXT("selected, from afar too"), CosmicMapLook::ShowsLabel(Look, true, true));
		Look.bOwned = true;
		TestTrue(TEXT("owned, from afar too"), CosmicMapLook::ShowsLabel(Look, false, true));
		Look.bOwned = false;
		Look.bHazard = true;
		TestTrue(TEXT("with a hazard, from afar too"), CosmicMapLook::ShowsLabel(Look, false, true));
	}

	// ---- The release that ends a double click is no click (Phase 3 item 8, amendment 14) ----
	{
		// UPlayerInput turns the second press into a press and a double click; the button-up after it is an ordinary
		// release. Here the cursor picks nothing (no mouse in a test): a release taken as a click would clear the selection.
		const FName Picked = Sim->GetState().Systems.IsValidIndex(2) ? Sim->GetState().Systems[2].SystemID : NAME_None;
		Controller->SelectSystem(Picked);
		TestEqual(TEXT("(a known system is selected)"), Controller->GetSelectedSystem(), Picked);
		Controller->HandleLeftPressed(EKeys::LeftMouseButton);
		Controller->HandleLeftDoubleClick(EKeys::LeftMouseButton);
		Controller->HandleLeftReleased(EKeys::LeftMouseButton);
		TestEqual(TEXT("A double click's release keeps the selection"), Controller->GetSelectedSystem(), Picked);
		// The other order (the double click before the press): the press leaves the swallow alone.
		Controller->HandleLeftDoubleClick(EKeys::LeftMouseButton);
		Controller->HandleLeftPressed(EKeys::LeftMouseButton);
		Controller->HandleLeftReleased(EKeys::LeftMouseButton);
		TestEqual(TEXT("and so does it when the double click comes before the press"), Controller->GetSelectedSystem(), Picked);
		// The control: a single press and release with no double click is a click on empty space, which clears.
		Controller->HandleLeftPressed(EKeys::LeftMouseButton);
		Controller->HandleLeftReleased(EKeys::LeftMouseButton);
		TestEqual(TEXT("A single click on empty space clears the selection"), Controller->GetSelectedSystem(), FName(NAME_None));
	}

	// ---- Lines keep about a pixel's width ----
	{
		Map->SetViewDistance(20000.f);
		Map->RefreshNow();
		const float Far = Map->GetLaneThickness();
		Map->SetViewDistance(3000.f);
		Map->RefreshNow();
		const float Near = Map->GetLaneThickness();
		TestTrue(TEXT("Lanes are thicker from afar"), Far > Near);
		TestEqual(TEXT("in proportion to the distance"), Far / Near, 20000.f / 3000.f, 0.01f);
		Map->SetViewDistance(3000.f * 1.03f);
		Map->RefreshNow();
		TestEqual(TEXT("A distance moved by under 5% does not redraw"), Map->GetLaneThickness(), Near);
		Map->SetViewDistance(3000.f * 1.1f);
		Map->RefreshNow();
		TestTrue(TEXT("one moved by more does"), Map->GetLaneThickness() > Near);
		Map->SetViewDistance(8800.f);
		Map->RefreshNow();
		TestFalse(TEXT("(8,800 away is near)"), Map->IsFarView());
		Map->SetViewDistance(9100.f);
		TestTrue(TEXT("Crossing 9,000 turns to the far view even by under 5%"), Map->IsFarView());
		Map->SetViewDistance(20000.f);
		Map->RefreshNow();
		const float Before = Map->GetLaneThickness();
		Map->SetViewDistance(std::numeric_limits<float>::quiet_NaN());
		Map->RefreshNow();
		TestTrue(TEXT("A distance that is not a number keeps the far view"), Map->IsFarView());
		TestEqual(TEXT("and the lines"), Map->GetLaneThickness(), Before);
	}

	// ---- The pure pick and the disc ray ----
	{
		const FCosmicScreenTarget Targets[] = {
			{ FVector2D(100.0, 100.0), 5.f },
			{ FVector2D(130.0, 100.0), 5.f },
			{ FVector2D(400.0, 400.0), 200.f },
		};
		TestEqual(TEXT("The nearest target within reach is picked"), CosmicMapPick::NearestOnScreen(Targets, FVector2D(110.0, 100.0), 24.f), 0);
		TestEqual(TEXT("and the nearer of two"), CosmicMapPick::NearestOnScreen(Targets, FVector2D(122.0, 100.0), 24.f), 1);
		TestEqual(TEXT("A tie goes to the lower index"), CosmicMapPick::NearestOnScreen(Targets, FVector2D(115.0, 100.0), 24.f), 0);
		TestEqual(TEXT("Nothing beyond the reach"), CosmicMapPick::NearestOnScreen(Targets, FVector2D(100.0, 130.0), 24.f), INDEX_NONE);
		TestEqual(TEXT("A large drawn radius reaches past the minimum"), CosmicMapPick::NearestOnScreen(Targets, FVector2D(500.0, 500.0), 24.f), 2);
		TestEqual(TEXT("An empty list picks nothing"), CosmicMapPick::NearestOnScreen(TArrayView<const FCosmicScreenTarget>(), FVector2D(0.0, 0.0), 24.f), INDEX_NONE);

		FVector Point;
		TestTrue(TEXT("A ray straight down meets the disc"), CosmicMapPick::OntoDisc(FVector(10.0, 20.0, 1000.0), FVector(0.0, 0.0, -1.0), Point));
		TestTrue(TEXT("under its origin"), Point.Equals(FVector(10.0, 20.0, 0.0), 0.001));
		TestTrue(TEXT("A slanted ray meets it"), CosmicMapPick::OntoDisc(FVector(0.0, 0.0, 1000.0), FVector(1.0, 0.0, -1.0).GetSafeNormal(), Point));
		TestTrue(TEXT("as far out as it was high"), Point.Equals(FVector(1000.0, 0.0, 0.0), 0.01));
		TestFalse(TEXT("A ray along the plane does not"), CosmicMapPick::OntoDisc(FVector(0.0, 0.0, 1000.0), FVector(1.0, 0.0, 0.0), Point));
		TestFalse(TEXT("nor one all but along it (it would meet the plane far beyond the sky)"), CosmicMapPick::OntoDisc(FVector(0.0, 0.0, 1000.0), FVector(1.0, 0.0, -1.0e-9), Point));
		TestFalse(TEXT("nor one pointing away"), CosmicMapPick::OntoDisc(FVector(0.0, 0.0, 1000.0), FVector(0.0, 0.0, 1.0), Point));
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

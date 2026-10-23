// Cosmic Ascension - automation tests of the automatic pause (Presentation). Design: guideline.md section 2.1, 8.2 (MVP
// item 1).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "CoreGlobals.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/Text.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/ScopeExit.h"
#include "Events/CosmicPetitions.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicAscensionOverlayWidget.h"
#include "UI/CosmicPetitionCardWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicAutoPause.h"
#include "World/CosmicCameraPawn.h"
#include "World/CosmicGalaxyMapActor.h"
#include "World/CosmicGameMode.h"
#include "World/CosmicZoomLevels.h"

namespace CosmicAutoPauseTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	FString Words(const TCHAR* Key)
	{
		return CosmicUI::Label(Key).ToString();
	}

	// The words the top bar shows for why the time stands still; empty while the line is collapsed.
	FString WhyShown(const UCosmicTopBarWidget& Bar)
	{
		const UTextBlock* Why = Bar.GetAutoPauseText();
		return (Why && Why->GetVisibility() != ESlateVisibility::Collapsed) ? Why->GetText().ToString() : FString();
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

// The automatic pause (guideline 2.1, MVP item 1): a told mythic line, a hazard newly standing on a system of the ruled
// people and a card stop a running game while the viewer keeps it on; the reason shows on the top bar until the speed is
// set again; a paused game and the choice off are left alone; the ascension presentation and the pause agree; the
// choice is the viewer's (on by default, never written by the tests). MVP item 6: the pause of a crisis selects its system
// and sends the camera there - not as the viewer's pick, not from a game already still, not with the choice off, the first
// system in array order, from the universe without noting the way home, over a press in flight, before the card of the
// same boundary.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIAutoPauseTest, "Cosmic_Ascension.UI.AutoPause", CosmicTests::Flags)
bool FCosmicUIAutoPauseTest::RunTest(const FString& Parameters)
{
	using namespace CosmicAutoPauseTests;

	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	if (!FSlateApplication::IsInitialized())
	{
		AddError(TEXT("No Slate application: run the automation tests with -game (see CLAUDE.md)."));
		return false;
	}
	UWorld* World = Sim->GetGameInstance()->GetWorld();
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(World ? World->GetFirstPlayerController() : nullptr);
	ACosmicHUD* HUD = Controller ? Controller->GetHUD<ACosmicHUD>() : nullptr;
	UCosmicTopBarWidget* Bar = HUD ? HUD->GetTopBar() : nullptr;
	UCosmicAscensionOverlayWidget* Overlay = HUD ? HUD->GetAscensionOverlay() : nullptr;
	UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	ACosmicGalaxyMapActor* Map = FindMap(World);
	ACosmicCameraPawn* Pawn = Controller ? Cast<ACosmicCameraPawn>(Controller->GetPawn()) : nullptr;
	if (!TestNotNull(TEXT("The first player controller is the game's"), Controller) || !TestNotNull(TEXT("The HUD has its top bar"), Bar)
		|| !TestNotNull(TEXT("and its ascension overlay"), Overlay) || !TestNotNull(TEXT("(the log subsystem runs)"), Log)
		|| !TestNotNull(TEXT("(the map)"), Map) || !TestNotNull(TEXT("(the camera pawn)"), Pawn))
	{
		return false;
	}
	const bool bViewerChoice = Controller->IsAutoPauseOn();
	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	const FCosmicHintProgress SavedHints = Controller->GetHintProgress();
	ON_SCOPE_EXIT
	{
		Controller->SelectSystem(NAME_None);
		Controller->SetHintProgress(SavedHints);
		Sim->SetTimeScale(0.f);
		Overlay->Close();
		Sim->SetTimeScale(0.f);
		Controller->SetAutoPause(bViewerChoice);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Sim->SetBroadcastEnabled(true);
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	// The map of this galaxy now (its rebuild clears the selection and frames the region): the focus finds its stars.
	Map->RefreshNow();
	Controller->SetHintProgress(FCosmicHintProgress());
	Overlay->Close();
	Sim->SetTimeScale(0.f);
	Controller->SetAutoPause(true);
	FGalaxyState& Live = Sim->GetMutableState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const int32 Ruled = Index.FindCiv(Live, Live.Player.RuledCivID);
	const int32 Home = Live.Civs.IsValidIndex(Ruled) ? Index.FindSystem(Live, Live.Civs[Ruled].HomeSystemID) : INDEX_NONE;
	const int32 Other = Live.Civs.IsValidIndex(1) ? Index.FindSystem(Live, Live.Civs[1].HomeSystemID) : INDEX_NONE;
	if (!TestTrue(TEXT("(the vessel rules a people with a home, and another people has one)"), Live.Systems.IsValidIndex(Home) && Live.Systems.IsValidIndex(Other) && Other != Home))
	{
		return false;
	}
	const FName HomeID = Live.Systems[Home].SystemID;
	const FName OtherID = Live.Systems[Other].SystemID;
	const FGameplayTag Plague = Tag(TEXT("Hazard.Plague"));
	const FGameplayTag Awakening = Tag(TEXT("Hazard.AIAwakening"));
	const FString& Placeholder = FStringTableEntry::GetPlaceholderSourceString();
	auto TellMythic = [&](uint8 Severity)
	{
		Log->Push(Tag(TEXT("Log.Calling")), Severity, INVTEXT("<auto pause>"), FFormatNamedArguments(), NAME_None, NAME_None);
	};

	// ---- 1. The pure rules ----
	{
		FCosmicLogEntry Entry;
		Entry.Severity = 2;
		TestTrue(TEXT("A told mythic line stops the time"), CosmicAutoPause::StopsFor(Entry));
		Entry.bUntold = true;
		TestFalse(TEXT("An untold one does not"), CosmicAutoPause::StopsFor(Entry));
		Entry.bUntold = false;
		Entry.Severity = 1;
		TestFalse(TEXT("A notable line does not"), CosmicAutoPause::StopsFor(Entry));
		Entry.Severity = 0;
		TestFalse(TEXT("nor an ordinary one"), CosmicAutoPause::StopsFor(Entry));

		FGalaxyState Copy = Live;
		TestEqual(TEXT("(no hazard stands on the new people's systems)"), CosmicAutoPause::OwnHazardsOf(Copy, Index).Num(), 0);
		Copy.Systems[Home].ActiveHazard = Plague;
		Copy.Systems[Other].ActiveHazard = Awakening;
		const TMap<FName, FGameplayTag> Own = CosmicAutoPause::OwnHazardsOf(Copy, Index);
		TestTrue(TEXT("The hazard on the ruled people's home is listed"), Own.Num() == 1 && Own.FindRef(HomeID) == Plague);
		TestFalse(TEXT("another people's is not"), Own.Contains(OtherID));
		FGalaxyState Wandering = Copy;
		Wandering.Player.Role = EPlayerRole::Wanderer;
		TestEqual(TEXT("A wanderer rules nothing: no hazard is its own"), CosmicAutoPause::OwnHazardsOf(Wandering, Index).Num(), 0);
		FGalaxyState Listed = Copy;
		Listed.Civs[Ruled].OwnedSystems.Add(OtherID);
		TestFalse(TEXT("A system the people lists but does not own counts for nothing"), CosmicAutoPause::OwnHazardsOf(Listed, Index).Contains(OtherID));

		const TMap<FName, FGameplayTag> None;
		const TMap<FName, FGameplayTag> HomePlague = { { HomeID, Plague } };
		const TMap<FName, FGameplayTag> HomeAwakening = { { HomeID, Awakening } };
		TestTrue(TEXT("A hazard standing where none stood is new"), CosmicAutoPause::HasNewHazard(None, HomePlague));
		TestFalse(TEXT("The same hazard still standing is not"), CosmicAutoPause::HasNewHazard(HomePlague, HomePlague));
		TestTrue(TEXT("Another hazard on the same system is"), CosmicAutoPause::HasNewHazard(HomePlague, HomeAwakening));
		TestFalse(TEXT("A hazard that ended is not"), CosmicAutoPause::HasNewHazard(HomePlague, None));

		TestTrue(TEXT("No reason has no words"), CosmicAutoPause::WhyKeyOf(ECosmicAutoPause::None) == nullptr);
		for (const ECosmicAutoPause Reason : { ECosmicAutoPause::MythicLine, ECosmicAutoPause::OwnHazard, ECosmicAutoPause::Card, ECosmicAutoPause::RunEnd,
			ECosmicAutoPause::Continued, ECosmicAutoPause::ContinuedFromBackup, ECosmicAutoPause::SaveUnreadable, ECosmicAutoPause::SaveFailed })
		{
			const TCHAR* Key = CosmicAutoPause::WhyKeyOf(Reason);
			TestTrue(*FString::Printf(TEXT("Reason %d has words in the String Table"), static_cast<int32>(Reason)), Key && !Words(Key).IsEmpty() && !Words(Key).Contains(Placeholder));
		}
		// The end of the run (MVP item 4) says so with the chronicle's words; the save's four lines are its own (MVP item 5).
		TestEqual(TEXT("An ended run's line is Chronicle.Why"), FString(CosmicAutoPause::WhyKeyOf(ECosmicAutoPause::RunEnd)), FString(TEXT("Chronicle.Why")));
		TestTrue(TEXT("Each reason of the save has a line of its own"), CosmicAutoPause::WhyKeyOf(ECosmicAutoPause::Continued) == FString(TEXT("Save.Why.Continued"))
			&& CosmicAutoPause::WhyKeyOf(ECosmicAutoPause::ContinuedFromBackup) == FString(TEXT("Save.Why.Backup"))
			&& CosmicAutoPause::WhyKeyOf(ECosmicAutoPause::SaveUnreadable) == FString(TEXT("Save.Why.Unreadable"))
			&& CosmicAutoPause::WhyKeyOf(ECosmicAutoPause::SaveFailed) == FString(TEXT("Save.Why.Failed")));
	}

	// ---- 2. A told mythic line ----
	{
		Controller->SetSpeed(2.f);
		TellMythic(2);
		TestEqual(TEXT("A told mythic line stops a running game"), Sim->GetTimeScale(), 0.f);
		TestTrue(TEXT("for that reason"), Controller->GetAutoPauseReason() == ECosmicAutoPause::MythicLine);
		TestEqual(TEXT("The top bar says why"), WhyShown(*Bar), Words(TEXT("AutoPause.Why.Mythic")));
		Controller->PressSpeedKey(EKeys::SpaceBar);
		TestEqual(TEXT("Space goes on at the speed before the pause"), Sim->GetTimeScale(), 2.f);
		TestTrue(TEXT("and the reason goes"), Controller->GetAutoPauseReason() == ECosmicAutoPause::None);
		TestEqual(TEXT("with its line"), WhyShown(*Bar), FString());

		TellMythic(1);
		TestEqual(TEXT("A notable line leaves the game running"), Sim->GetTimeScale(), 2.f);

		Controller->SetSpeed(0.f);
		TellMythic(2);
		TestTrue(TEXT("A paused game is left alone: no reason is noted"), Controller->GetAutoPauseReason() == ECosmicAutoPause::None);
		TestEqual(TEXT("and no line shows"), WhyShown(*Bar), FString());

		// The button: on, then off, then on again - in memory only while the tests run.
		FString KeptBefore;
		const bool bKeptBefore = GConfig && GConfig->GetString(ACosmicPlayerController::TimeUnitSection, ACosmicPlayerController::AutoPauseKey, KeptBefore, GGameUserSettingsIni);
		TestEqual(TEXT("The button says the automatic pause is on"), Bar->GetAutoPauseButton() ? Bar->GetAutoPauseButton()->GetLabel().ToString() : FString(), Words(TEXT("AutoPause.On")));
		TestEqual(TEXT("and its tooltip what stops the time"), Bar->GetAutoPauseButton() ? Bar->GetAutoPauseButton()->GetToolTipText().ToString() : FString(), Words(TEXT("AutoPause.Tip")));
		Bar->PressAutoPause();
		TestFalse(TEXT("A press turns it off"), Controller->IsAutoPauseOn());
		TestEqual(TEXT("and the button says so"), Bar->GetAutoPauseButton() ? Bar->GetAutoPauseButton()->GetLabel().ToString() : FString(), Words(TEXT("AutoPause.Off")));
		Controller->SetSpeed(2.f);
		TellMythic(2);
		TestEqual(TEXT("Off, a mythic line leaves the game running"), Sim->GetTimeScale(), 2.f);
		Bar->PressAutoPause();
		TestTrue(TEXT("A second press turns it on again"), Controller->IsAutoPauseOn());
		FString KeptAfter;
		const bool bKeptAfter = GConfig && GConfig->GetString(ACosmicPlayerController::TimeUnitSection, ACosmicPlayerController::AutoPauseKey, KeptAfter, GGameUserSettingsIni);
		TestTrue(TEXT("The presses wrote nothing into the viewer's settings while the tests run"), bKeptBefore == bKeptAfter && KeptBefore == KeptAfter);
	}

	// ---- 3. A hazard on a system of the ruled people ----
	{
		Controller->SetSpeed(1.f);
		Controller->FollowOwnHazards();
		Live.Systems[Other].ActiveHazard = Awakening;
		Live.Systems[Other].HazardTicksRemaining = 100;
		Controller->FollowOwnHazards();
		TestEqual(TEXT("A hazard on another people's system leaves the game running"), Sim->GetTimeScale(), 1.f);
		TestTrue(TEXT("and the camera where it was"), Controller->GetSelectedSystem().IsNone());
		Live.Systems[Home].ActiveHazard = Plague;
		Live.Systems[Home].HazardTicksRemaining = 100;
		Controller->FollowOwnHazards();
		TestEqual(TEXT("A hazard newly standing on the ruled people's system stops the game"), Sim->GetTimeScale(), 0.f);
		TestTrue(TEXT("for that reason"), Controller->GetAutoPauseReason() == ECosmicAutoPause::OwnHazard);
		TestEqual(TEXT("The top bar says why"), WhyShown(*Bar), Words(TEXT("AutoPause.Why.Hazard")));
		TestEqual(TEXT("The crisis's system is selected"), Controller->GetSelectedSystem(), HomeID);
		TestTrue(TEXT("and the camera goes there"), Pawn->GetTargetFocus().Equals(FVector(Live.Systems[Home].Position.X, Live.Systems[Home].Position.Y, 0.0), 0.01));
		TestFalse(TEXT("not as the viewer's pick (the star hint stays)"), Controller->GetHintProgress().bPickedSystem);
		Controller->SetSpeed(1.f);
		Controller->FollowOwnHazards();
		TestEqual(TEXT("The same hazard still standing does not stop it again"), Sim->GetTimeScale(), 1.f);
		Live.Systems[Home].ActiveHazard = FGameplayTag();
		Controller->FollowOwnHazards();
		TestEqual(TEXT("nor does its end"), Sim->GetTimeScale(), 1.f);

		// Through a tick: the controller follows the hazards at the tick boundary.
		Live.Systems[Home].ActiveHazard = Awakening;
		Live.Systems[Home].HazardTicksRemaining = 100;
		Sim->StepOnce();
		TestEqual(TEXT("A hazard standing on the ruled people's system at a tick boundary stops the game"), Sim->GetTimeScale(), 0.f);
		TestTrue(TEXT("for that reason"), Controller->GetAutoPauseReason() == ECosmicAutoPause::OwnHazard);

		// A new galaxy is another game: no reason stays, and its standing hazards are only noted.
		Controller->SetSpeed(1.f);
		Controller->RequestAutoPause(ECosmicAutoPause::Card);
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		TestTrue(TEXT("A new galaxy clears the reason"), Controller->GetAutoPauseReason() == ECosmicAutoPause::None);
		Sim->GetMutableState().Systems[Home].ActiveHazard = Plague;
		Controller->SetSpeed(1.f);
		Controller->FollowOwnHazards();
		TestEqual(TEXT("and the next hazard of the new game is new to it"), Sim->GetTimeScale(), 0.f);
		Sim->GetMutableState().Systems[Home].ActiveHazard = FGameplayTag();

		// A load is another game too: a hazard its state already holds is only noted.
		Controller->SetSpeed(1.f);
		Controller->FollowOwnHazards();
		FGalaxyState WithHazard = Sim->GetState();
		WithHazard.Systems[Home].ActiveHazard = Awakening;
		WithHazard.Systems[Home].HazardTicksRemaining = 100;
		Sim->LoadState(WithHazard);
		TestEqual(TEXT("A hazard the loaded state already holds does not stop the game"), Sim->GetTimeScale(), 1.f);
		Controller->FollowOwnHazards();
		TestEqual(TEXT("(nor at the next look)"), Sim->GetTimeScale(), 1.f);
		Sim->GetMutableState().Systems[Home].ActiveHazard = FGameplayTag();
	}

	// ---- 3b. The crisis focus (MVP item 6) ----
	{
		FGalaxyState& Now = Sim->GetMutableState();
		auto Clear = [&]()
		{
			for (FStarSystemState& System : Now.Systems)
			{
				System.ActiveHazard = FGameplayTag();
			}
			Controller->SelectSystem(NAME_None);
			Controller->SetSpeed(1.f);
			Controller->FollowOwnHazards();
		};
		auto Stand = [&](int32 At, FGameplayTag Hazard)
		{
			Now.Systems[At].ActiveHazard = Hazard;
			Now.Systems[At].HazardTicksRemaining = 100;
		};
		Map->RefreshNow();

		// The choice off: the time runs and the camera stays.
		Clear();
		Controller->SetAutoPause(false);
		Stand(Home, Plague);
		Controller->FollowOwnHazards();
		TestTrue(TEXT("With the automatic pause off the camera stays"), Controller->GetSelectedSystem().IsNone() && Sim->GetTimeScale() == 1.f);
		Controller->SetAutoPause(true);

		// A game already still (a card, a presentation): no move.
		Clear();
		Controller->SetSpeed(0.f);
		Stand(Home, Plague);
		Controller->FollowOwnHazards();
		TestTrue(TEXT("A game already still keeps its camera"), Controller->GetSelectedSystem().IsNone());

		// Two new crises in one look: the first system in the array (the home is the first life's system 0).
		Clear();
		const int32 Second = Now.Systems.Num() - 2;
		const FName SecondOwner = Now.Systems[Second].OwnerCivID;
		Now.Systems[Second].OwnerCivID = Now.Player.RuledCivID;
		Now.Civs[Ruled].OwnedSystems.AddUnique(Now.Systems[Second].SystemID);
		Controller->FollowOwnHazards();
		Stand(Second, Awakening);
		Stand(Home, Plague);
		Controller->FollowOwnHazards();
		TestEqual(TEXT("Of two new crises the first in the array is focused"), Controller->GetSelectedSystem(), Now.Systems[FMath::Min(Home, Second)].SystemID);
		Now.Civs[Ruled].OwnedSystems.Remove(Now.Systems[Second].SystemID);
		Now.Systems[Second].OwnerCivID = SecondOwner;

		// The end of a crisis moves nothing.
		Controller->SelectSystem(NAME_None);
		Controller->SetSpeed(1.f);
		Now.Systems[Home].ActiveHazard = FGameplayTag();
		Controller->FollowOwnHazards();
		TestTrue(TEXT("The end of a crisis moves nothing"), Controller->GetSelectedSystem().IsNone() && Sim->GetTimeScale() == 1.f);

		// From the universe: back to the region, and the way-home hint is not noted.
		Clear();
		Controller->FrameRegion();
		Pawn->SnapToTargets();
		Pawn->Zoom(-3.f);
		Controller->FollowZoomLevel();
		if (TestTrue(TEXT("(the camera is at the universe level)"), Controller->GetZoomLevel() == ECosmicZoomLevel::Universe))
		{
			FCosmicHintProgress AtUniverse = Controller->GetHintProgress();
			AtUniverse.bLeftUniverse = false;
			Controller->SetHintProgress(AtUniverse);
			Stand(Home, Plague);
			Controller->FollowOwnHazards();
			TestTrue(TEXT("A crisis seen from the universe brings the camera to the region"), Controller->GetZoomLevel() == ECosmicZoomLevel::Region);
			Controller->FollowZoomLevel();
			TestFalse(TEXT("and the way home stays the viewer's to find"), Controller->GetHintProgress().bLeftUniverse);
		}

		// A press in flight: its release is no click on empty space (the crisis stays selected).
		Clear();
		Controller->HandleLeftPressed(EKeys::LeftMouseButton);
		Stand(Home, Plague);
		Controller->FollowOwnHazards();
		Controller->HandleLeftReleased(EKeys::LeftMouseButton);
		TestEqual(TEXT("A press in flight does not clear the crisis just selected"), Controller->GetSelectedSystem(), HomeID);

		// A crisis and a card at one boundary: the crisis stops the time and the camera goes; the card shows over it.
		Clear();
		const FName Row(TEXT("petition_wise_heart"));
		Now.Knowing.PendingPetitionRow = Row;
		Now.Knowing.PendingPetitionSystemID = NAME_None;
		Stand(Home, Plague);
		Sim->StepOnce();
		TestEqual(TEXT("At one boundary the crisis is focused"), Controller->GetSelectedSystem(), HomeID);
		TestTrue(TEXT("it is the crisis that stopped the time"), Controller->GetAutoPauseReason() == ECosmicAutoPause::OwnHazard);
		TestTrue(TEXT("and the card shows"), HUD->GetPetitionCard() && HUD->GetPetitionCard()->GetShownRow() == Row);
		CosmicPetitions::Clear(Now);
		Controller->FollowPetition(false);
		Clear();
		Controller->SetSpeed(0.f);
	}

	// ---- 4. A card ----
	{
		Controller->SetSpeed(5.f);
		Controller->RequestAutoPause(ECosmicAutoPause::None);
		TestEqual(TEXT("No reason stops nothing"), Sim->GetTimeScale(), 5.f);
		Controller->RequestAutoPause(ECosmicAutoPause::Card);
		TestEqual(TEXT("A card stops a running game"), Sim->GetTimeScale(), 0.f);
		TestEqual(TEXT("The top bar says why"), WhyShown(*Bar), Words(TEXT("AutoPause.Why.Card")));
		Controller->PressSpeedKey(EKeys::SpaceBar);
		TestEqual(TEXT("and Space goes on at x5"), Sim->GetTimeScale(), 5.f);
	}

	// ---- 5. The ascension presentation ----
	{
		// The line of an ascension comes before its presentation (UAscensionSubsystem tells, then broadcasts): on, it stops
		// the game first, the presentation finds it paused, and closing leaves it paused.
		Controller->SetSpeed(2.f);
		TellMythic(2);
		Overlay->Show(EAscensionTier::Tier2_Stellar, NAME_None);
		Overlay->Close();
		TestEqual(TEXT("On, the game stays paused after the presentation"), Sim->GetTimeScale(), 0.f);
		TestTrue(TEXT("for the line's reason"), Controller->GetAutoPauseReason() == ECosmicAutoPause::MythicLine);
		// Off, the presentation pauses and gives the speed back.
		Controller->SetAutoPause(false);
		Controller->SetSpeed(2.f);
		TellMythic(2);
		Overlay->Show(EAscensionTier::Tier2_Stellar, NAME_None);
		TestEqual(TEXT("Off, the presentation pauses"), Sim->GetTimeScale(), 0.f);
		Overlay->Close();
		TestEqual(TEXT("and gives the speed back"), Sim->GetTimeScale(), 2.f);
		Controller->SetAutoPause(true);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

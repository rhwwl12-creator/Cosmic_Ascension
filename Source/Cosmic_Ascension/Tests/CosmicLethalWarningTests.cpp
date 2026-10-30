// Cosmic Ascension - automation tests of the lethal hazard's warnings and pause, and of the lock reason of a new vessel
// (Presentation, B5 [B-5c]). Design: docs/60_DECISIONS/headers/B5_death_succession_two_books.md section 2.8, 3.4, 4.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/UserWidget.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/Tuple.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicAutoPause.h"
#include "World/CosmicGalaxyMapActor.h"
#include "World/CosmicGameMode.h"

namespace CosmicLethalWarningTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	FString Words(const FText& Text)
	{
		return Text.ToString();
	}

	// What a text block shows; empty while it is collapsed.
	FString Shown(const UTextBlock* Text)
	{
		return (Text && Text->GetVisibility() != ESlateVisibility::Collapsed) ? Text->GetText().ToString() : FString();
	}

	// The game's tables with a KillPlayer in the supernova precursor's expiry: the game's own data kills nobody until [B-5d2],
	// so the warnings are seen with a test row of this file's own.
	FCosmicDataTables WithLethalPrecursor(const FCosmicDataTables& Base)
	{
		FCosmicDataTables Tables = Base;
		if (UDataTable* Hazards = CosmicTests::CopyWithout<FHazardDef>(Base.Hazards, {}))
		{
			for (const TPair<FName, uint8*>& Row : Hazards->GetRowMap())
			{
				FHazardDef& Hazard = *reinterpret_cast<FHazardDef*>(Row.Value);
				if (Hazard.HazardTag == Tag(TEXT("Hazard.PreSupernova")))
				{
					FCosmicEffect Kill;
					Kill.Type = EEffectType::KillPlayer;
					Kill.Tag = Tag(TEXT("Death.Hazard"));
					Hazard.OnExpireEffects.Add(Kill);
				}
			}
			Tables.Hazards = Hazards;
		}
		return Tables;
	}

	// The board of the wanderer's screens (UI.WandererPanel's): the chain 0-1-...-9 with 4-10-11, people 0 at 0, people 1 at
	// 5 and 6, people 2 at 8, people 3 at 11, no hazard, every rung the first; the vessel rules people 0 from 0 and knows 0.
	// Null when the galaxy is too small.
	FGalaxyState* LayBoard(UGalaxySimulationSubsystem& Sim)
	{
		Sim.NewGalaxy(42, 0, 0);
		FGalaxyState& State = Sim.GetMutableState();
		if (!CosmicTests::WireChain(State) || State.Civs.Num() < 4 || State.Player.RuledCivID != State.Civs[0].CivID)
		{
			return nullptr;
		}
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { 0 });
		CosmicTests::Give(State, 1, { 5, 6 });
		CosmicTests::Give(State, 2, { 8 });
		CosmicTests::Give(State, 3, { 11 });
		State.Transcendents.Reset();
		State.Log.Reset();
		for (FStarSystemState& System : State.Systems)
		{
			CosmicHazards::EmptyHazardSlot(System);
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
		}
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.Tier = EAscensionTier::Tier1_Planetary;
			Civ.Stability = 0.8f;
			Civ.ReverenceToPlayer = 0.f;
			Civ.ThroneLockedUntilTick = 0;
		}
		State.Calling.PersonalTier = EAscensionTier::Tier1_Planetary;
		State.Player.CurrentSystemID = State.Systems[0].SystemID;
		State.Knowing = FKnowingState();
		CosmicTests::SetKnowing(State, { 0 }, { 0 });
		return &State;
	}

	// The vessel leaves its throne from SystemIndex (a deposed wanderer that knows where it stands and the lanes around).
	void Walk(UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, int32 SystemIndex)
	{
		Sim.GetMutableState().Player.CurrentSystemID = Sim.GetState().Systems[SystemIndex].SystemID;
		Role.BecomeWanderer(Tag(TEXT("AI.Profile.Conservative")), Tag(TEXT("Reign.Deposed")));
	}

	const UCosmicActionButton* ButtonOf(const UCosmicSystemPanelWidget& Panel, FName Action)
	{
		for (const UCosmicActionButton* Button : Panel.GetWandererButtons())
		{
			if (Button && Button->GetAction() == Action)
			{
				return Button;
			}
		}
		return nullptr;
	}
}

// The lethal hazard (B5 header section 2.8, 4): one answer (CosmicAutoPause::LethalPlaceOf over CosmicHazards::KillsAVesselOf
// and the shared reach, CosmicHazards::VesselAtRiskOf at the hazard's expiry) for where it stands - an embodied wanderer's
// place, under way the system it left; never a ruler, the interregnum or a hazard that kills nobody -; the words of the top
// bar, of the panel of that system and of [Travel]'s tooltip, which say that a vessel that set out stands where it left
// until it arrives; the pause when such a hazard newly stands there (a new hazard, an arrival), its reason and its words, not
// again for the same one, not with the choice off, only noted on a load, and through a tick boundary. The birth protection:
// a hazard that expires while the vessel is shielded is no line and no pause, and born one tick earlier it is all of them.
// The lock reason of a vessel whose life began in the lock (section 3.4).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUILethalWarningTest, "Cosmic_Ascension.UI.LethalWarning", CosmicTests::Flags)
bool FCosmicUILethalWarningTest::RunTest(const FString& Parameters)
{
	using namespace CosmicLethalWarningTests;
	using CosmicUI::Label;

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
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	UCosmicSystemPanelWidget* Panel = Controller ? CreateWidget<UCosmicSystemPanelWidget>(Controller) : nullptr;
	if (!TestNotNull(TEXT("The first player controller is the game's"), Controller) || !TestNotNull(TEXT("The HUD has its top bar"), Bar)
		|| !TestNotNull(TEXT("(a system panel of the test's own)"), Panel) || !TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const TSharedRef<SWidget> SlatePanel = Panel->TakeWidget();
	const FCosmicDataTables Original = Sim->GetTables();
	const bool bViewerChoice = Controller->IsAutoPauseOn();
	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	ON_SCOPE_EXIT
	{
		Controller->SelectSystem(NAME_None);
		Controller->SetAutoPause(bViewerChoice);
		Controller->SetSpeed(0.f);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Sim->SetTimeScale(0.f);
	const FGameplayTag Precursor = Tag(TEXT("Hazard.PreSupernova"));
	const FGameplayTag Plague = Tag(TEXT("Hazard.Plague"));
	const FCosmicDataTables Lethal = WithLethalPrecursor(Original);
	const FHazardDef* PrecursorRow = Lethal.FindHazardByTag(Precursor);
	if (!TestTrue(TEXT("(the game has the supernova precursor and the plague)"), PrecursorRow && Lethal.FindHazardByTag(Plague)))
	{
		return false;
	}
	TestFalse(TEXT("The game's own data kills nobody yet (the data is off until [B-5d2])"), CosmicHazards::KillsAVesselOf(Original, Precursor));
	TestTrue(TEXT("(the test row kills at its expiry)"), CosmicHazards::KillsAVesselOf(Lethal, Precursor) && !CosmicHazards::KillsAVesselOf(Lethal, Plague));
	Sim->SetTables(Lethal);
	// The birth protection (DT_Rules.BirthProtectionTicks): the board's hazards count down past it, so the vessel born at the
	// board's tick 0 is reached at their expiry; section 3b walks its edge.
	const int32 Protect = Sim->GetRules().BirthProtectionTicks;
	if (!TestTrue(TEXT("(the game shields a newborn vessel for a while)"), Protect > 2))
	{
		return false;
	}
	const int32 PastProtection = Protect + 200;
	// The galaxy's delegates on (the map, the controller and the HUD follow the board), as UI.AutoPause runs them.
	Sim->SetBroadcastEnabled(true);
	FGalaxyState* State = LayBoard(*Sim);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	for (const TCHAR* Key : { TEXT("Wander.Lethal"), TEXT("Wander.Bar.Lethal"), TEXT("Wander.Bar.LethalTravelling"), TEXT("Wander.TravelLethalTip"),
		TEXT("Wander.Block.LockedNew"), TEXT("AutoPause.Why.Lethal") })
	{
		TestFalse(*FString::Printf(TEXT("%s has words in the String Table"), Key), Words(Label(Key)).IsEmpty() || Words(Label(Key)).Contains(FStringTableEntry::GetPlaceholderSourceString()));
	}
	FFormatNamedArguments HazardArgs;
	HazardArgs.Add(TEXT("Hazard"), PrecursorRow->DisplayName);
	const FString BarStanding = Words(FText::Format(Label(TEXT("Wander.Bar.Lethal")), HazardArgs));
	const FString BarUnderWay = Words(FText::Format(Label(TEXT("Wander.Bar.LethalTravelling")), HazardArgs));
	const FString TravelWarning = Words(FText::Format(Label(TEXT("Wander.TravelLethalTip")), HazardArgs));
	const FString PanelWarning = Words(Label(TEXT("Wander.Lethal")));
	auto Place = [&](int32 SystemIndex, const FGameplayTag& Hazard)
	{
		State->Systems[SystemIndex].ActiveHazard = Hazard;
		State->Systems[SystemIndex].HazardTicksRemaining = PastProtection;
	};
	auto Clear = [&](int32 SystemIndex)
	{
		CosmicHazards::EmptyHazardSlot(State->Systems[SystemIndex]);
	};
	auto PlaceNow = [&]() { return CosmicAutoPause::LethalPlaceOf(*State, Index, Sim->GetTables(), Sim->GetRules()); };

	// ---- 1. Where a lethal hazard stands for the vessel: one answer ----
	Place(0, Precursor);
	TestEqual(TEXT("A ruler is warned of nothing (what happens to a system never kills a ruler)"), PlaceNow(), INDEX_NONE);
	Clear(0);
	Walk(*Sim, *Role, 4);
	CosmicTests::KnowAll(*State);
	// The map of this board now: the pause's focus finds its stars.
	if (ACosmicGalaxyMapActor* Map = Controller->GetMap())
	{
		Map->RefreshNow();
	}
	TestEqual(TEXT("(a wanderer at 4, no hazard there)"), PlaceNow(), INDEX_NONE);
	Place(4, Plague);
	TestEqual(TEXT("A hazard whose expiry kills nobody is no warning"), PlaceNow(), INDEX_NONE);
	Place(4, Precursor);
	TestEqual(TEXT("A lethal hazard where the wanderer stands is"), PlaceNow(), 4);
	TestEqual(TEXT("not with the game's own data"), CosmicAutoPause::LethalPlaceOf(*State, Index, Original, Sim->GetRules()), INDEX_NONE);
	Place(5, Precursor);
	TestEqual(TEXT("A lethal hazard on a neighbour is not the vessel's"), PlaceNow(), 4);
	Clear(5);
	State->Calling.LifeState = ELifeState::Interregnum;
	TestEqual(TEXT("The interregnum has no vessel to warn"), PlaceNow(), INDEX_NONE);
	Bar->Refresh();
	TestTrue(TEXT("and its bar no warning"), Shown(Bar->GetLethalText()).IsEmpty());
	State->Calling.LifeState = ELifeState::Embodied;

	// ---- 2. The words: the top bar, the panel of the system, the travel buttons ----
	Bar->Refresh();
	TestEqual(TEXT("The top bar warns where the wanderer stands, promising nothing"), Shown(Bar->GetLethalText()), BarStanding);
	TestEqual(TEXT("(LethalLineOf says the same)"), Words(UCosmicTopBarWidget::LethalLineOf(*State, Index, Sim->GetTables(), Sim->GetRules())), BarStanding);
	TestFalse(TEXT("beside the wanderer's line"), Shown(Bar->GetWandererText()).IsEmpty());
	Panel->ShowSystem(State->Systems[4].SystemID);
	TestTrue(TEXT("The panel of the system it stands in warns under the hazard's line"), Words(Panel->GetDetailText()->GetText()).Contains(PanelWarning));
	Panel->ShowSystem(State->Systems[5].SystemID);
	const UCosmicActionButton* Go = ButtonOf(*Panel, UCosmicSystemPanelWidget::TravelAction);
	TestTrue(TEXT("[Travel] says that setting out does not take the vessel away before it arrives"), Go && Words(Go->GetToolTipText()).Contains(TravelWarning));
	TestTrue(TEXT("after the travel's own words"), Go && Words(Go->GetToolTipText()).StartsWith(Words(Label(TEXT("Wander.TravelTip")))));
	TestFalse(TEXT("A neighbour's panel has no warning line of its own"), Words(Panel->GetDetailText()->GetText()).Contains(PanelWarning));
	Place(4, Plague);
	Bar->Refresh();
	TestTrue(TEXT("A plague is no warning on the bar"), Shown(Bar->GetLethalText()).IsEmpty());
	Panel->ShowSystem(State->Systems[4].SystemID);
	TestFalse(TEXT("nor on the panel"), Words(Panel->GetDetailText()->GetText()).Contains(PanelWarning));
	Panel->ShowSystem(State->Systems[5].SystemID);
	Go = ButtonOf(*Panel, UCosmicSystemPanelWidget::TravelAction);
	TestTrue(TEXT("nor in [Travel]'s tooltip"), Go && !Words(Go->GetToolTipText()).Contains(TravelWarning));
	Place(4, Precursor);
	// Under way the vessel still stands where it left (section 2.8): the system it left warns, the bar says 'before it arrives'.
	TestTrue(TEXT("(it sets off to 5)"), Role->TravelTo(State->Systems[5].SystemID) == CosmicWanderer::ETravelBlock::None);
	TestEqual(TEXT("Under way the lethal place is the system it left"), PlaceNow(), 4);
	Place(5, Precursor);
	TestEqual(TEXT("not the one it goes to"), PlaceNow(), 4);
	Clear(5);
	Bar->Refresh();
	TestEqual(TEXT("The bar says 'before it arrives'"), Shown(Bar->GetLethalText()), BarUnderWay);
	Panel->ShowSystem(State->Systems[4].SystemID);
	TestTrue(TEXT("and the panel of the system it left still warns"), Words(Panel->GetDetailText()->GetText()).Contains(PanelWarning));
	Panel->ShowSystem(State->Systems[3].SystemID);
	Go = ButtonOf(*Panel, UCosmicSystemPanelWidget::TravelAction);
	TestTrue(TEXT("[Travel], greyed under way, still says so"), Go && !Go->GetIsEnabled() && Words(Go->GetToolTipText()).Contains(TravelWarning));
	State->Player.TravelTargetSystemID = NAME_None;
	State->Player.TravelTicksRemaining = 0;

	// ---- 3. The pause ----
	Controller->SetAutoPause(true);
	Clear(4);
	Controller->SetSpeed(1.f);
	Controller->FollowLethalHazard();
	Place(4, Plague);
	Controller->FollowLethalHazard();
	TestEqual(TEXT("A hazard that kills nobody leaves the time running"), Sim->GetTimeScale(), 1.f);
	Place(4, Precursor);
	Controller->FollowLethalHazard();
	TestEqual(TEXT("A lethal hazard newly standing where the wanderer stands stops the time"), Sim->GetTimeScale(), 0.f);
	TestTrue(TEXT("for a reason of its own"), Controller->GetAutoPauseReason() == ECosmicAutoPause::LethalHazard);
	TestEqual(TEXT("whose words are the String Table's"), FString(CosmicAutoPause::WhyKeyOf(ECosmicAutoPause::LethalHazard)), FString(TEXT("AutoPause.Why.Lethal")));
	Bar->Refresh();
	TestEqual(TEXT("The top bar says why"), Shown(Bar->GetAutoPauseText()), Words(Label(TEXT("AutoPause.Why.Lethal"))));
	TestEqual(TEXT("The system is selected"), Controller->GetSelectedSystem(), State->Systems[4].SystemID);
	Controller->SetSpeed(1.f);
	Controller->FollowLethalHazard();
	TestEqual(TEXT("The same hazard still standing does not stop it again"), Sim->GetTimeScale(), 1.f);
	// Coming to stand where one stands is new to the vessel (the move stage's arrival, by hand).
	Place(5, Precursor);
	State->Player.CurrentSystemID = State->Systems[5].SystemID;
	Controller->FollowLethalHazard();
	TestEqual(TEXT("An arrival where a lethal hazard stands stops the time"), Sim->GetTimeScale(), 0.f);
	TestEqual(TEXT("(that system selected)"), Controller->GetSelectedSystem(), State->Systems[5].SystemID);
	// The automatic pause off: the time runs, and the hazard noted then is no news once it is on again.
	Controller->SetAutoPause(false);
	Clear(5);
	Controller->SetSpeed(1.f);
	Controller->FollowLethalHazard();
	Place(5, Precursor);
	Controller->FollowLethalHazard();
	TestEqual(TEXT("With the automatic pause off the time runs on"), Sim->GetTimeScale(), 1.f);
	Controller->SetAutoPause(true);
	Controller->FollowLethalHazard();
	TestEqual(TEXT("and a hazard noted then is no news once it is on"), Sim->GetTimeScale(), 1.f);
	// A load is another game: a lethal hazard its state already holds is only noted.
	Clear(5);
	Controller->FollowLethalHazard();
	{
		FGalaxyState WithHazard = Sim->GetState();
		WithHazard.Systems[5].ActiveHazard = Precursor;
		WithHazard.Systems[5].HazardTicksRemaining = PastProtection;
		Sim->LoadState(WithHazard);
	}
	TestEqual(TEXT("A lethal hazard the loaded state already holds does not stop the game"), Sim->GetTimeScale(), 1.f);
	Controller->FollowLethalHazard();
	TestEqual(TEXT("(nor at the next look)"), Sim->GetTimeScale(), 1.f);
	// Through a tick: the controller follows it at the tick boundary (FollowOwnHazards calls it).
	Clear(5);
	Controller->FollowLethalHazard();
	Place(5, Precursor);
	Sim->StepOnce();
	TestEqual(TEXT("A lethal hazard standing there at a tick boundary stops the game"), Sim->GetTimeScale(), 0.f);
	TestTrue(TEXT("for its reason"), Controller->GetAutoPauseReason() == ECosmicAutoPause::LethalHazard);
	Controller->SetSpeed(0.f);

	// ---- 3b. The birth protection (header section 4: the screens read the shared reach, CosmicHazards::VesselAtRiskOf, at the
	// hazard's expiry): a lethal hazard that expires while the vessel is shielded is no warning - on the bar, the panel or
	// [Travel]'s tooltip - and no pause; the vessel born one tick earlier is reached at that expiry, and all of them come ----
	{
		const int64 Now = 1000;
		const int32 Left = Protect / 2;
		auto BornAt = [&](int64 Born)
		{
			State->Calling.BornTick = Born;
			State->Calling.Lives.Last().BornTick = Born;
		};
		State->Tick = Now;
		Clear(4);
		Clear(5);
		State->Player.CurrentSystemID = State->Systems[4].SystemID;
		Controller->SetAutoPause(true);
		Controller->SetSpeed(1.f);
		Controller->FollowLethalHazard();
		// The expiry (Now + Left) falls on the protection's last tick.
		BornAt(Now + Left - Protect + 1);
		State->Systems[4].ActiveHazard = Precursor;
		State->Systems[4].HazardTicksRemaining = Left;
		TestFalse(TEXT("(the shared reach shields the vessel at the expiry)"), CosmicHazards::VesselAtRiskOf(State->Player, State->Calling, Sim->GetRules(), Now + Left,
			State->Systems[4].SystemID, NAME_None));
		TestEqual(TEXT("A lethal hazard that expires inside the birth protection is no lethal place"), PlaceNow(), INDEX_NONE);
		Bar->Refresh();
		TestTrue(TEXT("The bar does not warn a shielded vessel"), Shown(Bar->GetLethalText()).IsEmpty());
		Panel->ShowSystem(State->Systems[4].SystemID);
		TestFalse(TEXT("nor does the panel of its system"), Words(Panel->GetDetailText()->GetText()).Contains(PanelWarning));
		Panel->ShowSystem(State->Systems[5].SystemID);
		Go = ButtonOf(*Panel, UCosmicSystemPanelWidget::TravelAction);
		TestTrue(TEXT("nor [Travel]'s tooltip"), Go && !Words(Go->GetToolTipText()).Contains(TravelWarning));
		Controller->FollowLethalHazard();
		TestEqual(TEXT("and the time runs on"), Sim->GetTimeScale(), 1.f);
		// Born one tick earlier: the expiry is the first tick the kill reaches the vessel.
		BornAt(Now + Left - Protect);
		TestTrue(TEXT("(the shared reach reaches the vessel at the expiry)"), CosmicHazards::VesselAtRiskOf(State->Player, State->Calling, Sim->GetRules(), Now + Left,
			State->Systems[4].SystemID, NAME_None));
		TestEqual(TEXT("Born one tick earlier, the same hazard is the lethal place"), PlaceNow(), 4);
		Bar->Refresh();
		TestEqual(TEXT("The bar warns"), Shown(Bar->GetLethalText()), BarStanding);
		Panel->ShowSystem(State->Systems[4].SystemID);
		TestTrue(TEXT("the panel of its system warns"), Words(Panel->GetDetailText()->GetText()).Contains(PanelWarning));
		Panel->ShowSystem(State->Systems[5].SystemID);
		Go = ButtonOf(*Panel, UCosmicSystemPanelWidget::TravelAction);
		TestTrue(TEXT("[Travel]'s tooltip warns"), Go && Words(Go->GetToolTipText()).Contains(TravelWarning));
		Controller->FollowLethalHazard();
		TestEqual(TEXT("and the time stops"), Sim->GetTimeScale(), 0.f);
		TestTrue(TEXT("for the lethal hazard"), Controller->GetAutoPauseReason() == ECosmicAutoPause::LethalHazard);
		Controller->SetSpeed(0.f);
	}

	// ---- 4. The lock reason of a vessel whose life began in the lock (section 3.4) ----
	const FCosmicRulesDef& Rules = Sim->GetRules();
	const int64 Lock = FMath::Max(0, Rules.ThroneLockTicks);
	if (!TestTrue(TEXT("(the game locks the thrones for a while)"), Lock > 200))
	{
		return false;
	}
	{
		FGalaxyState Pure;
		Pure.Tick = 750;
		Pure.Calling.Lives.AddDefaulted();
		FCivilizationState Owner;
		// The lock began at 700.
		Owner.ThroneLockedUntilTick = 700 + Lock;
		Pure.Calling.BornTick = 720;
		TestTrue(TEXT("A vessel born in the lock that never ruled did not leave a throne"), UCosmicSystemPanelWidget::IsLockedBeforeLife(Pure, Owner, Rules));
		Pure.Calling.BornTick = 700;
		TestTrue(TEXT("nor one born on the lock's tick (a leap of 0)"), UCosmicSystemPanelWidget::IsLockedBeforeLife(Pure, Owner, Rules));
		Pure.Calling.BornTick = 699;
		TestFalse(TEXT("A life older than the lock is not told so"), UCosmicSystemPanelWidget::IsLockedBeforeLife(Pure, Owner, Rules));
		Pure.Calling.BornTick = 720;
		Pure.Calling.Lives[0].RuledCivIDs.Add(FName(TEXT("civ_test")));
		TestFalse(TEXT("A life that sat on a throne left one"), UCosmicSystemPanelWidget::IsLockedBeforeLife(Pure, Owner, Rules));
		Pure.Calling.Lives[0].RuledCivIDs.Reset();
		Pure.Tick = Owner.ThroneLockedUntilTick;
		TestFalse(TEXT("An open throne is no lock"), UCosmicSystemPanelWidget::IsLockedBeforeLife(Pure, Owner, Rules));
	}
	{
		// The panel: a deposed vessel at 5 (people 1's settled system, met), the thrones locked by its fall.
		State = LayBoard(*Sim);
		if (!TestNotNull(TEXT("The board is laid again"), State))
		{
			return false;
		}
		State->Tick = 100;
		Walk(*Sim, *Role, 5);
		CosmicTests::KnowAll(*State);
		for (FCivilizationState& Civ : State->Civs)
		{
			Civ.ThroneLockedUntilTick = State->Tick + Lock;
		}
		Panel->ShowSystem(State->Systems[5].SystemID);
		const UCosmicActionButton* Crown = ButtonOf(*Panel, UCosmicSystemPanelWidget::CrownAction);
		TestTrue(TEXT("(the throne is locked)"), Panel->GetShownCrownBlock() == TOptional<CosmicWanderer::ECrownBlock>(CosmicWanderer::ECrownBlock::ThroneLocked));
		TestTrue(TEXT("A vessel that left a throne is told it left one"), Crown && Words(Crown->GetToolTipText()).StartsWith(Words(Label(TEXT("Wander.Block.Locked")))));
		// A called vessel born on the lock's tick (the leap of 0 after a ruler's death) that never ruled.
		FLifeRecord& Life = State->Calling.Lives.Last();
		Life.RuledCivIDs.Reset();
		Life.ReignEndReasons.Reset();
		Life.ReignTicks.Reset();
		Life.BornTick = State->Tick;
		State->Calling.BornTick = State->Tick;
		Panel->Refresh();
		Crown = ButtonOf(*Panel, UCosmicSystemPanelWidget::CrownAction);
		TestTrue(TEXT("(still the lock)"), Panel->GetShownCrownBlock() == TOptional<CosmicWanderer::ECrownBlock>(CosmicWanderer::ECrownBlock::ThroneLocked));
		TestTrue(TEXT("A vessel whose life began in the lock is told it in words of its own"), Crown && Words(Crown->GetToolTipText()).StartsWith(Words(Label(TEXT("Wander.Block.LockedNew")))));
		TestTrue(TEXT("never that it left a throne"), Crown && !Words(Crown->GetToolTipText()).Contains(Words(Label(TEXT("Wander.Block.Locked")))));
		TestTrue(TEXT("and its short words stay"), Crown && Words(Crown->GetLabel()).Contains(Words(FText::Format(Label(TEXT("Wander.Suffix.Locked")),
			CosmicUI::TimeLeftLabel(Lock, Controller->GetTimeUnit())))));
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

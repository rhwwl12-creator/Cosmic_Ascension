// Cosmic Ascension - automation tests of the lethal hazard's warnings and pause, and of the lock reason of a new vessel
// (Presentation, B5 [B-5c], [B-5r]). Design: docs/60_DECISIONS/headers/B5_death_succession_two_books.md section 2.8, 3.4, 4.

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
#include "Events/CosmicPetitions.h"
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
#include "UI/CosmicPetitionCardWidget.h"
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

	// The game's tables with a KillPlayer in the supernova precursor's expiry as well: the game's own data kills at the AI
	// awakening's own expiry alone ([B-5d2], decision 6-ga), so the precursor's warnings are seen with a test row of this file's
	// own; the awakening's are the game's.
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
// [B-5r] (the [B-5d2] review's play-1, risk-1, goals-1, play-2): under way the system it left counts only while its hazard
// expires at or before the arrival; where the vessel would come to stand (CosmicAutoPause::LethalStayOf - a [Travel]'s arrival,
// the walk of an abdication, where it stands) the three bands, the reach of the placed vessel and the birth protection; the
// readers - [Travel]'s tooltip and mark on the destination's panel, [Travel] away when too late, the bar under way, [Abdicate],
// the card window under its choices and its outcome -, and the trapped vessel (B1 decision 21) warned where it stands.
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
	TestFalse(TEXT("The game's own data does not kill at the supernova precursor's expiry ([B-5d2] opens the AI awakening's alone)"), CosmicHazards::KillsAVesselOf(Original, Precursor));
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
	// [B-5r] One lane's time (CosmicWanderer::LaneTicksOf): the bands of where the vessel would come to stand are counted in it.
	const int32 Lane = CosmicWanderer::LaneTicksOf(Sim->GetRules());
	if (!TestTrue(TEXT("(a lane takes more than one tick, well inside the protection)"), Lane > 1 && 2 * Lane + 2 < PastProtection))
	{
		return false;
	}
	// The galaxy's delegates on (the map, the controller and the HUD follow the board), as UI.AutoPause runs them.
	Sim->SetBroadcastEnabled(true);
	FGalaxyState* State = LayBoard(*Sim);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	for (const TCHAR* Key : { TEXT("Wander.Lethal"), TEXT("Wander.Bar.Lethal"), TEXT("Wander.Bar.LethalTravelling"), TEXT("Wander.TravelLethalTip"),
		TEXT("Wander.Block.LockedNew"), TEXT("AutoPause.Why.Lethal"), TEXT("Wander.TravelAheadTip"), TEXT("Wander.TravelAheadTrapTip"),
		TEXT("Wander.TravelLethalLate"), TEXT("Wander.Suffix.Lethal"), TEXT("Wander.AbdicateLethalTip"), TEXT("Wander.AbdicateTrapTip"),
		TEXT("Wander.Bar.Ahead"), TEXT("Wander.Bar.AheadTrap"), TEXT("Wander.CardLethal"), TEXT("Wander.CardLethalTrap") })
	{
		TestFalse(*FString::Printf(TEXT("%s has words in the String Table"), Key), Words(Label(Key)).IsEmpty() || Words(Label(Key)).Contains(FStringTableEntry::GetPlaceholderSourceString()));
	}
	FFormatNamedArguments HazardArgs;
	HazardArgs.Add(TEXT("Hazard"), PrecursorRow->DisplayName);
	const FString BarStanding = Words(FText::Format(Label(TEXT("Wander.Bar.Lethal")), HazardArgs));
	const FString BarUnderWay = Words(FText::Format(Label(TEXT("Wander.Bar.LethalTravelling")), HazardArgs));
	const FString TravelWarning = Words(FText::Format(Label(TEXT("Wander.TravelLethalTip")), HazardArgs));
	const FString PanelWarning = Words(Label(TEXT("Wander.Lethal")));
	// [B-5r]: [Travel] away too late, a destination's two bands, their mark, the bar under way to a destination.
	const FString TravelLate = Words(FText::Format(Label(TEXT("Wander.TravelLethalLate")), HazardArgs));
	const FString AheadWords = Words(FText::Format(Label(TEXT("Wander.TravelAheadTip")), HazardArgs));
	const FString AheadTrapWords = Words(FText::Format(Label(TEXT("Wander.TravelAheadTrapTip")), HazardArgs));
	const FString LethalMark = Words(Label(TEXT("Wander.Suffix.Lethal")));
	const FString BarAhead = Words(FText::Format(Label(TEXT("Wander.Bar.Ahead")), HazardArgs));
	const FString BarAheadTrap = Words(FText::Format(Label(TEXT("Wander.Bar.AheadTrap")), HazardArgs));
	auto Place = [&](int32 SystemIndex, const FGameplayTag& Hazard)
	{
		State->Systems[SystemIndex].ActiveHazard = Hazard;
		State->Systems[SystemIndex].HazardTicksRemaining = PastProtection;
	};
	// A hazard with Ticks left ([B-5r]: the bands are counted from the tick it expires at).
	auto SetLeft = [&](int32 SystemIndex, const FGameplayTag& Hazard, int32 Ticks)
	{
		State->Systems[SystemIndex].ActiveHazard = Hazard;
		State->Systems[SystemIndex].HazardTicksRemaining = Ticks;
	};
	auto BornAt = [&](int64 Born)
	{
		State->Calling.BornTick = Born;
		State->Calling.Lives.Last().BornTick = Born;
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
	TestEqual(TEXT("not with the game's own data (its precursor kills nobody)"), CosmicAutoPause::LethalPlaceOf(*State, Index, Original, Sim->GetRules()), INDEX_NONE);
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
	// [B-5d2] The game's own lethal row - the AI awakening's own expiry (decision 6-ga) - warns as the test row does, in its name.
	{
		const FGameplayTag Awakening = Tag(TEXT("Hazard.AIAwakening"));
		const FHazardDef* AwakeningRow = Original.FindHazardByTag(Awakening);
		if (TestTrue(TEXT("(the game's AI awakening kills at its expiry)"), AwakeningRow && CosmicHazards::KillsAVesselOf(Original, Awakening)))
		{
			Place(4, Awakening);
			TestEqual(TEXT("The game's own lethal row is a lethal place where the wanderer stands"), CosmicAutoPause::LethalPlaceOf(*State, Index, Original, Sim->GetRules()), 4);
			FFormatNamedArguments AwakeningArgs;
			AwakeningArgs.Add(TEXT("Hazard"), AwakeningRow->DisplayName);
			Bar->Refresh();
			TestEqual(TEXT("The top bar warns of it in the awakening's own name"), Shown(Bar->GetLethalText()), Words(FText::Format(Label(TEXT("Wander.Bar.Lethal")), AwakeningArgs)));
			Panel->ShowSystem(State->Systems[4].SystemID);
			TestTrue(TEXT("and so does the panel of its system"), Words(Panel->GetDetailText()->GetText()).Contains(PanelWarning));
			Panel->ShowSystem(State->Systems[5].SystemID);
			Go = ButtonOf(*Panel, UCosmicSystemPanelWidget::TravelAction);
			TestTrue(TEXT("and [Travel]'s tooltip"), Go && Words(Go->GetToolTipText()).Contains(Words(FText::Format(Label(TEXT("Wander.TravelLethalTip")), AwakeningArgs))));
		}
	}
	Place(4, Precursor);
	Panel->ShowSystem(State->Systems[5].SystemID);
	Go = ButtonOf(*Panel, UCosmicSystemPanelWidget::TravelAction);
	TestTrue(TEXT("Standing, [Travel] away from a lethal hazard that expires after any arrival is not told it is too late"),
		Go && Words(Go->GetToolTipText()).Contains(TravelWarning) && !Words(Go->GetToolTipText()).Contains(TravelLate));
	// Under way the vessel still stands where it left until it arrives (section 2.8). [B-5r] (the [B-5d2] review's play-1): the
	// system it left is the lethal place only while its hazard expires at or before the arrival - the crisis stage comes before
	// the move stage -; one that expires after it is left behind. Born a protection's span before the board's tick, a
	// countdown shorter than a lane reaches the vessel.
	const int64 BoardBorn = State->Calling.BornTick;
	BornAt(State->Tick - Protect);
	TestTrue(TEXT("(it sets off to 5)"), Role->TravelTo(State->Systems[5].SystemID) == CosmicWanderer::ETravelBlock::None);
	TestEqual(TEXT("Under way a lethal hazard where it left that expires after the arrival is left behind"), PlaceNow(), INDEX_NONE);
	Bar->Refresh();
	TestTrue(TEXT("no warning of it on the bar"), Shown(Bar->GetLethalText()).IsEmpty());
	Panel->ShowSystem(State->Systems[4].SystemID);
	TestFalse(TEXT("nor on the panel of the system it left"), Words(Panel->GetDetailText()->GetText()).Contains(PanelWarning));
	SetLeft(4, Precursor, Lane);
	TestEqual(TEXT("One that expires on the arrival's tick is the system it left"), PlaceNow(), 4);
	Place(5, Precursor);
	TestEqual(TEXT("not the one it goes to"), PlaceNow(), 4);
	Bar->Refresh();
	TestEqual(TEXT("The bar says it expires before the arrival - the system it left comes first"), Shown(Bar->GetLethalText()), BarUnderWay);
	Clear(5);
	Panel->ShowSystem(State->Systems[4].SystemID);
	TestTrue(TEXT("and the panel of the system it left still warns"), Words(Panel->GetDetailText()->GetText()).Contains(PanelWarning));
	Panel->ShowSystem(State->Systems[3].SystemID);
	Go = ButtonOf(*Panel, UCosmicSystemPanelWidget::TravelAction);
	TestTrue(TEXT("[Travel], greyed under way, says plainly it is too late"), Go && !Go->GetIsEnabled() && Words(Go->GetToolTipText()).Contains(TravelLate));
	SetLeft(4, Precursor, Lane + 1);
	TestEqual(TEXT("One that expires a tick after the arrival is left behind"), PlaceNow(), INDEX_NONE);
	// Nearer the arrival the journey's own time counts, not a lane's: five ticks from it, a lane's countdown is left behind.
	State->Player.TravelTicksRemaining = 5;
	SetLeft(4, Precursor, Lane);
	TestEqual(TEXT("Five ticks from the arrival, a hazard that expires in a lane's time is left behind"), PlaceNow(), INDEX_NONE);
	SetLeft(4, Precursor, 5);
	TestEqual(TEXT("one that expires on the arrival's tick is not"), PlaceNow(), 4);
	State->Player.TravelTargetSystemID = NAME_None;
	State->Player.TravelTicksRemaining = 0;
	Place(4, Precursor);
	BornAt(BoardBorn);

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

	// ---- 5. [B-5r] Where the vessel would come to stand (the [B-5d2] review's play-1, risk-1, goals-1; header section 2.8, 4):
	// one answer, CosmicAutoPause::LethalStayOf, of a lethal hazard on the system the vessel would stand in from the end of a
	// tick on - its three bands against a lane's time (expired before the vessel stands there: none; within a lane's time
	// after: no way out; later: the time to leave), the reach asked of the vessel placed there, the birth protection at the
	// expiry - and its readers: [Travel]'s tooltip and label on the destination's panel, [Travel] away said plainly when too
	// late, the bar under way, [Abdicate] (the walk from the tenth stage of the next tick) and the card window ----
	{
		State = LayBoard(*Sim);
		if (!TestNotNull(TEXT("The board is laid for the arrivals"), State))
		{
			return false;
		}
		const int64 Now = 1000;
		State->Tick = Now;
		Walk(*Sim, *Role, 4);
		CosmicTests::KnowAll(*State);
		const int64 Born = State->Calling.BornTick;
		if (!TestTrue(TEXT("(a wanderer at 4, born long enough ago that every expiry here is past the protection)"),
			CosmicWanderer::IsEmbodiedWanderer(*State) && Now - Born >= Protect))
		{
			return false;
		}
		auto StayAt = [&](int32 SystemIndex, int64 From)
		{
			return CosmicAutoPause::LethalStayOf(*State, Sim->GetTables(), Rules, SystemIndex, From);
		};
		const int64 Arrival = Now + Lane;

		// 5a. The one answer: a [Travel]'s arrival, where the vessel stands, the walk of an abdication.
		SetLeft(5, Precursor, Lane);
		TestTrue(TEXT("A lethal hazard that expires on the arrival's tick is no warning (the crisis stage comes before the move stage)"),
			StayAt(5, Arrival) == ECosmicLethalStay::None);
		SetLeft(5, Precursor, Lane + 1);
		TestTrue(TEXT("One that expires a tick after the arrival leaves no way out"), StayAt(5, Arrival) == ECosmicLethalStay::NoWayOut);
		SetLeft(5, Precursor, 2 * Lane);
		TestTrue(TEXT("nor one that expires a lane's time after it (set out again then, the vessel still stands there at the expiry)"),
			StayAt(5, Arrival) == ECosmicLethalStay::NoWayOut);
		SetLeft(5, Precursor, 2 * Lane + 1);
		TestTrue(TEXT("One that expires a tick later leaves the time to leave"), StayAt(5, Arrival) == ECosmicLethalStay::CanLeave);
		TestTrue(TEXT("not with the game's own data (its precursor kills nobody)"),
			CosmicAutoPause::LethalStayOf(*State, Original, Rules, 5, Arrival) == ECosmicLethalStay::None);
		SetLeft(5, Plague, 2 * Lane + 1);
		TestTrue(TEXT("A hazard that kills nobody is no warning"), StayAt(5, Arrival) == ECosmicLethalStay::None);
		Clear(5);
		TestTrue(TEXT("nor a system without one"), StayAt(5, Arrival) == ECosmicLethalStay::None);
		TestTrue(TEXT("nor an index the state does not hold"), StayAt(INDEX_NONE, Arrival) == ECosmicLethalStay::None
			&& StayAt(State->Systems.Num(), Arrival) == ECosmicLethalStay::None);
		// Where it stands (from now): LethalPlaceOf's answer there.
		SetLeft(4, Precursor, Lane);
		TestTrue(TEXT("Where the vessel stands, a lethal hazard that expires within a lane's time leaves leaving no way out"),
			StayAt(4, Now) == ECosmicLethalStay::NoWayOut);
		TestEqual(TEXT("(it is the lethal place)"), PlaceNow(), 4);
		SetLeft(4, Precursor, Lane + 1);
		TestTrue(TEXT("one a tick later leaves the time to leave"), StayAt(4, Now) == ECosmicLethalStay::CanLeave);
		TestEqual(TEXT("(it is the lethal place too)"), PlaceNow(), 4);
		Clear(4);
		// The walk of an abdication, from the next tick on: the expiry at that tick finds the vessel still ruling.
		SetLeft(5, Precursor, 1);
		TestTrue(TEXT("Standing from the next tick on, a lethal hazard that expires at that tick is no warning"), StayAt(5, Now + 1) == ECosmicLethalStay::None);
		SetLeft(5, Precursor, 2);
		TestTrue(TEXT("one that expires a tick later leaves no way out"), StayAt(5, Now + 1) == ECosmicLethalStay::NoWayOut);
		SetLeft(5, Precursor, Lane + 1);
		TestTrue(TEXT("(and one a lane's time later)"), StayAt(5, Now + 1) == ECosmicLethalStay::NoWayOut);
		SetLeft(5, Precursor, Lane + 2);
		TestTrue(TEXT("one a tick later still leaves the time to leave"), StayAt(5, Now + 1) == ECosmicLethalStay::CanLeave);
		// The birth protection at the expiry (Arrival + 1): shielded on the protection's last tick, reached born a tick earlier.
		SetLeft(5, Precursor, Lane + 1);
		BornAt(Arrival + 1 - Protect + 1);
		TestTrue(TEXT("A lethal hazard that expires while the vessel would be shielded is no warning"), StayAt(5, Arrival) == ECosmicLethalStay::None);
		BornAt(Arrival + 1 - Protect);
		TestTrue(TEXT("born a tick earlier, it is"), StayAt(5, Arrival) == ECosmicLethalStay::NoWayOut);
		BornAt(Born);
		State->Calling.LifeState = ELifeState::Interregnum;
		TestTrue(TEXT("The interregnum is warned of nothing"), StayAt(5, Arrival) == ECosmicLethalStay::None);
		State->Calling.LifeState = ELifeState::Embodied;

		// 5b. [Travel] on the destination's panel: in its tooltip after the travel's own words, its mark after the label.
		auto TravelTip = [&](int32 To)
		{
			Panel->ShowSystem(State->Systems[To].SystemID);
			Panel->Refresh();
			const UCosmicActionButton* Button = ButtonOf(*Panel, UCosmicSystemPanelWidget::TravelAction);
			return Button ? Words(Button->GetToolTipText()) : FString();
		};
		auto TravelLabel = [&]()
		{
			const UCosmicActionButton* Button = ButtonOf(*Panel, UCosmicSystemPanelWidget::TravelAction);
			return Button ? Words(Button->GetLabel()) : FString();
		};
		SetLeft(5, Precursor, 2 * Lane + 1);
		FString Tip = TravelTip(5);
		TestTrue(TEXT("[Travel] to a system whose lethal hazard still stands after the arrival says so"), Tip.Contains(AheadWords) && !Tip.Contains(AheadTrapWords));
		TestTrue(TEXT("after the travel's own words"), Tip.StartsWith(Words(Label(TEXT("Wander.TravelTip")))));
		TestTrue(TEXT("with its mark after the label"), TravelLabel().EndsWith(LethalMark));
		TestTrue(TEXT("a warning, not a gate"), Panel->GetShownTravelBlock() == TOptional<CosmicWanderer::ETravelBlock>(CosmicWanderer::ETravelBlock::None));
		SetLeft(5, Precursor, 2 * Lane);
		Tip = TravelTip(5);
		TestTrue(TEXT("One that expires within a lane's time after the arrival says leaving it then is no way out"), Tip.Contains(AheadTrapWords) && !Tip.Contains(AheadWords));
		TestTrue(TEXT("(the same mark)"), TravelLabel().EndsWith(LethalMark));
		SetLeft(5, Precursor, Lane + 1);
		TestTrue(TEXT("(so does one that expires a tick after the arrival)"), TravelTip(5).Contains(AheadTrapWords));
		SetLeft(5, Precursor, Lane);
		Tip = TravelTip(5);
		TestTrue(TEXT("One that expires before the vessel would arrive is no warning"), !Tip.Contains(AheadTrapWords) && !Tip.Contains(AheadWords));
		TestFalse(TEXT("and no mark"), TravelLabel().Contains(LethalMark));
		SetLeft(5, Plague, 2 * Lane + 1);
		Tip = TravelTip(5);
		TestTrue(TEXT("A plague there is no warning"), !Tip.Contains(AheadTrapWords) && !Tip.Contains(AheadWords) && !TravelLabel().Contains(LethalMark));
		SetLeft(5, Precursor, Lane + 1);
		BornAt(Arrival + 1 - Protect + 1);
		TestFalse(TEXT("A vessel that would be shielded at the expiry is not warned"), TravelTip(5).Contains(AheadTrapWords));
		BornAt(Born);
		TestTrue(TEXT("(born earlier, it is)"), TravelTip(5).Contains(AheadTrapWords));
		Tip = TravelTip(3);
		TestFalse(TEXT("Another neighbour's [Travel] does not speak of it"), Tip.Contains(AheadTrapWords) || Tip.Contains(AheadWords));
		// A waiting card greys [Travel] with its own short words; the warning stays (the vessel would set out after the answer).
		State->Knowing.PendingPetitionRow = FName(TEXT("enc_awakening_talk"));
		Tip = TravelTip(5);
		TestTrue(TEXT("A waiting card's short words take the label"), TravelLabel().EndsWith(Words(Label(TEXT("Wander.Suffix.CardWaiting")))) && !TravelLabel().Contains(LethalMark));
		TestTrue(TEXT("and the warning stays in the tooltip"), Tip.Contains(AheadTrapWords));
		State->Knowing.PendingPetitionRow = NAME_None;

		// 5c. [Travel] away from where the vessel stands: said plainly when its lethal hazard expires before any arrival.
		Clear(5);
		SetLeft(4, Precursor, Lane);
		Tip = TravelTip(5);
		TestTrue(TEXT("Where a lethal hazard expires within a lane's time, [Travel] away says leaving now is too late"), Tip.Contains(TravelLate) && !Tip.Contains(TravelWarning));
		SetLeft(4, Precursor, Lane + 1);
		Tip = TravelTip(5);
		TestTrue(TEXT("a tick later, only that the vessel stands there until it arrives"), Tip.Contains(TravelWarning) && !Tip.Contains(TravelLate));
		Clear(4);

		// 5d. The bar under way: the lethal hazard where the vessel goes, as it will stand at the arrival (the journey's own time).
		SetLeft(5, Precursor, 2 * Lane + 1);
		TestTrue(TEXT("(it sets off to 5)"), Role->TravelTo(State->Systems[5].SystemID) == CosmicWanderer::ETravelBlock::None);
		Bar->Refresh();
		TestEqual(TEXT("Under way the bar warns of the lethal hazard where the vessel goes"), Shown(Bar->GetLethalText()), BarAhead);
		TestEqual(TEXT("(LethalLineOf says the same)"), Words(UCosmicTopBarWidget::LethalLineOf(*State, Index, Sim->GetTables(), Rules)), BarAhead);
		SetLeft(5, Precursor, 2 * Lane);
		Bar->Refresh();
		TestEqual(TEXT("in words of no way out when it expires within a lane's time after the arrival"), Shown(Bar->GetLethalText()), BarAheadTrap);
		SetLeft(5, Precursor, Lane);
		Bar->Refresh();
		TestTrue(TEXT("and of nothing when it expires before the arrival"), Shown(Bar->GetLethalText()).IsEmpty());
		State->Player.TravelTicksRemaining = 1;
		SetLeft(5, Precursor, Lane + 1);
		Bar->Refresh();
		TestEqual(TEXT("A tick from the arrival, one that expires a lane's time after it leaves no way out"), Shown(Bar->GetLethalText()), BarAheadTrap);
		SetLeft(5, Precursor, Lane + 2);
		Bar->Refresh();
		TestEqual(TEXT("one a tick later the time to leave"), Shown(Bar->GetLethalText()), BarAhead);
		State->Player.TravelTicksRemaining = Lane;
		SetLeft(4, Precursor, Lane);
		Bar->Refresh();
		TestEqual(TEXT("A lethal hazard where it left that expires first is said first"), Shown(Bar->GetLethalText()), BarUnderWay);
		Clear(4);
		Clear(5);
		State->Player.TravelTargetSystemID = NAME_None;
		State->Player.TravelTicksRemaining = 0;
		// The trapped vessel (B1 decision 21 - every lane leads to a destroyed system, a dead node with no star class): no
		// [Travel] to warn on; the bar's warning of where it stands and its wanderer line's 'no lane' are the words it has.
		SetLeft(4, Precursor, 2 * Lane + 1);
		for (const FName Lane4 : State->Systems[4].Lanes)
		{
			const int32 Neighbour = Index.FindSystem(*State, Lane4);
			if (State->Systems.IsValidIndex(Neighbour))
			{
				State->Systems[Neighbour].StarClass = FGameplayTag();
			}
		}
		Bar->Refresh();
		TestEqual(TEXT("A trapped vessel at a lethal place is warned where it stands"), Shown(Bar->GetLethalText()), BarStanding);
		TestTrue(TEXT("beside the words that no lane leads away"), Shown(Bar->GetWandererText()).Contains(Words(Label(TEXT("Wander.Bar.Stuck")))));

		// 5e. [Abdicate]: the vessel off the throne walks from where it stands, from the tenth stage of the next tick.
		State = LayBoard(*Sim);
		if (!TestNotNull(TEXT("The board is laid for the abdication"), State))
		{
			return false;
		}
		State->Tick = Now;
		CosmicTests::KnowAll(*State);
		const int64 RulerBorn = State->Calling.BornTick;
		FFormatNamedArguments CapitalArgs = HazardArgs;
		CapitalArgs.Add(TEXT("System"), State->Systems[0].DisplayName);
		const FString AbdicateCan = Words(FText::Format(Label(TEXT("Wander.AbdicateLethalTip")), CapitalArgs));
		const FString AbdicateTrap = Words(FText::Format(Label(TEXT("Wander.AbdicateTrapTip")), CapitalArgs));
		const FString AbdicatePlain = Words(Label(TEXT("Wander.AbdicateTip")));
		const UCosmicActionButton* AbdicateButton = Panel->GetAbdicateButton();
		auto AbdicateTip = [&]()
		{
			Panel->ShowSystem(State->Systems[0].SystemID);
			Panel->Refresh();
			return AbdicateButton ? Words(AbdicateButton->GetToolTipText()) : FString();
		};
		SetLeft(0, Precursor, 1);
		TestEqual(TEXT("[Abdicate] over a lethal hazard that expires at the next tick - the vessel still rules then - says only its own words"), AbdicateTip(), AbdicatePlain);
		TestTrue(TEXT("(on the capital's panel)"), AbdicateButton && AbdicateButton->GetVisibility() != ESlateVisibility::Collapsed);
		SetLeft(0, Precursor, 2);
		Tip = AbdicateTip();
		TestTrue(TEXT("One that expires a tick later says the vessel off the throne could not get away"), Tip.StartsWith(AbdicatePlain) && Tip.Contains(AbdicateTrap) && !Tip.Contains(AbdicateCan));
		TestTrue(TEXT("with the mark after its label"), AbdicateButton && Words(AbdicateButton->GetLabel()).EndsWith(LethalMark));
		SetLeft(0, Precursor, Lane + 1);
		TestTrue(TEXT("(so does one that expires a lane's time after the walk begins)"), AbdicateTip().Contains(AbdicateTrap));
		SetLeft(0, Precursor, Lane + 2);
		Tip = AbdicateTip();
		TestTrue(TEXT("One that expires later says only that it would stand there"), Tip.Contains(AbdicateCan) && !Tip.Contains(AbdicateTrap));
		BornAt(Now + Lane + 2 - Protect + 1);
		TestEqual(TEXT("A vessel that would be shielded at the expiry is not warned"), AbdicateTip(), AbdicatePlain);
		BornAt(RulerBorn);
		SetLeft(0, Plague, Lane + 2);
		TestEqual(TEXT("A plague on the capital is no warning"), AbdicateTip(), AbdicatePlain);
		TestFalse(TEXT("and no mark"), AbdicateButton && Words(AbdicateButton->GetLabel()).Contains(LethalMark));
		// [B-5r] review logic-2: the capital's first-ship line (Hint.FirstShip - lay down the throne and leave) waits while a
		// lethal hazard stands where the vessel off the throne would walk from (the [Abdicate] band), as it waits for a
		// rebellion: taken away in the viewing it came in, not shown nor spent on a fresh session's look, back once it is gone.
		const FCosmicHintProgress WasHints = Controller->GetHintProgress();
		auto ShipShown = [&]()
		{
			Panel->ShowSystem(State->Systems[0].SystemID);
			Panel->Refresh();
			return Panel->GetShipHintText() && Panel->GetShipHintText()->GetVisibility() != ESlateVisibility::Collapsed;
		};
		Controller->SetHintProgress(FCosmicHintProgress());
		Clear(0);
		TestTrue(TEXT("(inside the first ship's window)"), State->Tick >= CosmicUI::FirstShipFromTick && State->Tick < CosmicUI::FirstShipUntilTick);
		TestTrue(TEXT("(the capital's panel shows the first ship's line)"), ShipShown());
		SetLeft(0, Precursor, Lane + 2);
		TestFalse(TEXT("A lethal hazard where the vessel off the throne would stand takes the first ship's line away"), ShipShown());
		Controller->SetHintProgress(FCosmicHintProgress());
		TestFalse(TEXT("(nor does it show on a fresh session's look)"), ShipShown());
		TestFalse(TEXT("and it does not spend the session's line"), Controller->GetHintProgress().bShipHinted);
		Clear(0);
		TestTrue(TEXT("The hazard gone, the line comes back"), ShipShown());
		Controller->SetHintProgress(WasHints);
		// Recorded, the abdication still says it: the walk begins at the tenth stage of the next tick.
		SetLeft(0, Precursor, Lane + 2);
		if (TestTrue(TEXT("(the abdication is recorded)"), Role->Abdicate()))
		{
			Tip = AbdicateTip();
			TestTrue(TEXT("A recorded abdication still warns, after its own words"), Tip.StartsWith(Words(Label(TEXT("Wander.Block.Leaving")))) && Tip.Contains(AbdicateCan));
			TestTrue(TEXT("its label keeps the leaving words"), AbdicateButton && Words(AbdicateButton->GetLabel()).EndsWith(Words(Label(TEXT("Wander.Suffix.Leaving")))));
		}

		// 5f. The card window: a wanderer's card at the lethal place says no answer moves the vessel - under the outcome too,
		// until an answer ends the crisis.
		State = LayBoard(*Sim);
		if (!TestNotNull(TEXT("The board is laid for the card"), State))
		{
			return false;
		}
		State->Tick = Now;
		Walk(*Sim, *Role, 4);
		CosmicTests::KnowAll(*State);
		UCosmicPetitionCardWidget* Card = HUD->GetPetitionCard();
		const FName Talk(TEXT("enc_awakening_talk"));
		const FGameplayTag Awakening = Tag(TEXT("Hazard.AIAwakening"));
		const FEncounterDef* TalkRow = Sim->GetTables().FindEncounter(Talk);
		const FHazardDef* AwakeningRow = Sim->GetTables().FindHazardByTag(Awakening);
		if (TestNotNull(TEXT("(the HUD's card)"), Card) && TestTrue(TEXT("(the game's card of the AI awakening: a check first, a second choice)"),
			TalkRow && AwakeningRow && TalkRow->Choices.Num() >= 2 && CosmicHazards::KillsAVesselOf(Sim->GetTables(), Awakening)))
		{
			const FString CardWords = Words(FText::FormatNamed(Label(TEXT("Wander.CardLethal")), TEXT("Hazard"), AwakeningRow->DisplayName));
			const FString CardTrapWords = Words(FText::FormatNamed(Label(TEXT("Wander.CardLethalTrap")), TEXT("Hazard"), AwakeningRow->DisplayName));
			auto PutTalk = [&]()
			{
				State->Knowing.PendingPetitionRow = Talk;
				State->Knowing.PendingPetitionSystemID = State->Systems[4].SystemID;
				Controller->FollowPetition(false);
			};
			SetLeft(4, Awakening, PastProtection);
			PutTalk();
			TestEqual(TEXT("(the card shows)"), Card->GetShownRow(), Talk);
			TestEqual(TEXT("A wanderer's card at the lethal place says no answer moves the vessel"), Shown(Card->GetLethalText()), CardWords);
			TestEqual(TEXT("(LethalLineOf says the same)"), Words(UCosmicPetitionCardWidget::LethalLineOf(*State, Index, Sim->GetTables(), Rules, State->Systems[4].SystemID)), CardWords);
			TestTrue(TEXT("not of another system"), UCosmicPetitionCardWidget::LethalLineOf(*State, Index, Sim->GetTables(), Rules, State->Systems[5].SystemID).IsEmpty());
			// The second answer moves nothing and ends nothing: the line stays under its outcome.
			Card->PressChoice(1);
			TestTrue(TEXT("(the second choice was given and left the awakening standing)"), Card->IsShowingOutcome() && State->Systems[4].ActiveHazard == Awakening);
			TestEqual(TEXT("An answer that leaves the crisis standing keeps the line under the outcome"), Shown(Card->GetLethalText()), CardWords);
			// Under the outcome the line follows the state: a refresh with the crisis gone takes it away, and with it back brings it.
			Clear(4);
			Card->Refresh();
			TestTrue(TEXT("Under the outcome a refresh follows the state (the crisis gone, no line)"), Shown(Card->GetLethalText()).IsEmpty());
			SetLeft(4, Awakening, PastProtection);
			Card->Refresh();
			TestEqual(TEXT("(the crisis back, the line back)"), Shown(Card->GetLethalText()), CardWords);
			Card->PressClose();
			TestTrue(TEXT("(a closed card has no line)"), Shown(Card->GetLethalText()).IsEmpty());
			// An answer that ends the crisis takes it away (a Mind no roll fails).
			State->Player.Mind = FMath::Max(State->Player.Mind, TalkRow->Choices[0].Difficulty);
			PutTalk();
			TestEqual(TEXT("(shown again, with its line)"), Shown(Card->GetLethalText()), CardWords);
			Card->PressChoice(0);
			TestTrue(TEXT("(the first choice ended the awakening)"), Card->IsShowingOutcome() && !State->Systems[4].ActiveHazard.IsValid());
			TestTrue(TEXT("An answer that ended the crisis takes the line away"), Shown(Card->GetLethalText()).IsEmpty());
			Card->PressClose();
			// No line where nothing kills, nor for a vessel shielded at the expiry.
			SetLeft(4, Plague, PastProtection);
			PutTalk();
			TestTrue(TEXT("A wanderer's card where no hazard kills has no line"), Shown(Card->GetLethalText()).IsEmpty());
			const int64 CardBorn = State->Calling.BornTick;
			SetLeft(4, Awakening, Lane + 1);
			BornAt(Now + Lane + 1 - Protect + 1);
			Card->Refresh();
			TestTrue(TEXT("nor one at a lethal hazard that expires while the vessel is shielded"), Shown(Card->GetLethalText()).IsEmpty());
			BornAt(Now + Lane + 1 - Protect);
			Card->Refresh();
			TestEqual(TEXT("born a tick earlier, the line comes"), Shown(Card->GetLethalText()), CardWords);
			BornAt(CardBorn);
			// [B-5r] review logic-1: within one lane's time leaving after the answer is too late too - the trap's words (the band
			// of CosmicAutoPause::LethalStayOf from now, as [Travel] away's); a tick later the plain words again.
			SetLeft(4, Awakening, Lane);
			Card->Refresh();
			TestEqual(TEXT("A lethal hazard that expires within a lane's time says leaving after the answer is too late"), Shown(Card->GetLethalText()), CardTrapWords);
			TestEqual(TEXT("(LethalLineOf says the same)"), Words(UCosmicPetitionCardWidget::LethalLineOf(*State, Index, Sim->GetTables(), Rules, State->Systems[4].SystemID)), CardTrapWords);
			SetLeft(4, Awakening, Lane + 1);
			Card->Refresh();
			TestEqual(TEXT("one a tick later only that no answer moves the vessel"), Shown(Card->GetLethalText()), CardWords);
			CosmicPetitions::Clear(*State);
			Controller->FollowPetition(false);
			TestTrue(TEXT("(the card taken away)"), Card->GetShownRow().IsNone() && Shown(Card->GetLethalText()).IsEmpty());
		}
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

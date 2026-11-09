// Cosmic Ascension - automation tests of the wanderer's screens (Presentation, B1). Design: guideline.md section 5.9, 5.10,
// 6.3; docs/60_DECISIONS/headers/B1_wanderer_loop.md section 4 and 6.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Events/CosmicPetitions.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicPetitionCardWidget.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicAutoPause.h"
#include "World/CosmicGalaxyMapActor.h"
#include "World/CosmicGameMode.h"
#include "World/CosmicMapLook.h"

namespace CosmicWandererUITests
{
	FString Words(const FText& Text)
	{
		return Text.ToString();
	}

	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// The board of the wanderer's tests (Tests/CosmicWandererTests.cpp): the chain 0-1-...-9 with 4-10-11, people 0 at 0,
	// people 1 at 5 and 6, people 2 at 8, people 3 at 11, no hazard, every rung the first; the vessel rules people 0 from 0
	// and knows 0. Null when the galaxy is too small.
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

	// The vessel leaves its throne from SystemIndex through BecomeWanderer (a deposed wanderer that knows where it stands
	// and the lanes around).
	void Walk(UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, int32 SystemIndex)
	{
		Sim.GetMutableState().Player.CurrentSystemID = Sim.GetState().Systems[SystemIndex].SystemID;
		Role.BecomeWanderer(Tag(TEXT("AI.Profile.Conservative")), Tag(TEXT("Reign.Deposed")));
	}

	void Meet(FGalaxyState& State, int32 CivIndex)
	{
		const FName CivID = State.Civs[CivIndex].CivID;
		if (!State.Knowing.MetCivs.ContainsByPredicate([CivID](const FKnownMark& Mark) { return Mark.ID == CivID; }))
		{
			FKnownMark& Mark = State.Knowing.MetCivs.AddDefaulted_GetRef();
			Mark.ID = CivID;
			Mark.Tick = State.Tick;
		}
	}

	// The wanderer's buttons by action, in panel order.
	TArray<FName> ActionsOf(const UCosmicSystemPanelWidget& Panel)
	{
		TArray<FName> Actions;
		for (const UCosmicActionButton* Button : Panel.GetWandererButtons())
		{
			Actions.Add(Button ? Button->GetAction() : NAME_None);
		}
		return Actions;
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

	// Presses the wanderer's button of Action as a click does; false when the panel shows none.
	bool Press(const UCosmicSystemPanelWidget& Panel, FName Action)
	{
		for (const TObjectPtr<UCosmicActionButton>& Button : Panel.GetWandererButtons())
		{
			if (Button && Button->GetAction() == Action)
			{
				Button.Get()->Press();
				return true;
			}
		}
		return false;
	}
}

// The wanderer's panel (B1 header section 4): the three buttons on the systems they belong to and only there, greyed with
// the reason the command gives (no see-all), the reverence line, the ruler's [Abdicate] on the capital with its two
// presses, and the vessel's mark on the map.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIWandererPanelTest, "Cosmic_Ascension.UI.WandererPanel", CosmicTests::Flags)
bool FCosmicUIWandererPanelTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWandererUITests;

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
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	UCosmicSystemPanelWidget* Panel = Controller ? CreateWidget<UCosmicSystemPanelWidget>(Controller) : nullptr;
	if (!TestNotNull(TEXT("(a system panel of the test's own)"), Panel) || !TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const TSharedRef<SWidget> SlatePanel = Panel->TakeWidget();
	ON_SCOPE_EXIT
	{
		Controller->ClearRunEnd();
		Controller->SetSpeed(0.f);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Sim->SetTimeScale(0.f);
	FGalaxyState* State = LayBoard(*Sim);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FName Travel = UCosmicSystemPanelWidget::TravelAction;
	const FName Scan = UCosmicSystemPanelWidget::ScanAction;
	const FName Crown = UCosmicSystemPanelWidget::CrownAction;
	const FString Reverence0 = Words(FText::Format(CosmicUI::Label(TEXT("Wander.Reverence")), CosmicUI::Percent(0.f)));

	// 1. The ruler: no wanderer's button anywhere; [Abdicate] on the capital only.
	CosmicTests::KnowAll(*State);
	Panel->ShowSystem(State->Systems[0].SystemID);
	TestEqual(TEXT("A ruler's panel has no wanderer's button"), Panel->GetWandererButtons().Num(), 0);
	TestTrue(TEXT("The capital's panel shows [Abdicate]"), Panel->GetAbdicateButton() && Panel->GetAbdicateButton()->GetVisibility() != ESlateVisibility::Collapsed);
	TestTrue(TEXT("enabled"), Panel->GetAbdicateButton() && Panel->GetAbdicateButton()->GetIsEnabled());
	TestFalse(TEXT("A ruler's panel has no reverence line"), Words(Panel->GetDetailText()->GetText()).Contains(Reverence0));
	TestTrue(TEXT("Its tooltip says the rung stays until a throne"), Words(Panel->GetAbdicateButton()->GetToolTipText()).Equals(Words(CosmicUI::Label(TEXT("Wander.AbdicateTip")))));
	Panel->ShowSystem(State->Systems[1].SystemID);
	TestTrue(TEXT("Another system's panel does not"), Panel->GetAbdicateButton()->GetVisibility() == ESlateVisibility::Collapsed);

	// 2. Two presses: the first arms and relabels, a second within the window records the abdication; a second after it
	// only arms again.
	Panel->ShowSystem(State->Systems[0].SystemID);
	const double Now = FPlatformTime::Seconds();
	Panel->PressAbdicate(Now);
	TestFalse(TEXT("One press records nothing"), State->Calling.PendingEffect.IsValid());
	TestTrue(TEXT("and asks for a second"), Words(Panel->GetAbdicateButton()->GetLabel()).Equals(Words(CosmicUI::Label(TEXT("Wander.AbdicateAgain")))));
	Panel->PressAbdicate(Now + UCosmicSystemPanelWidget::AbdicateConfirmSeconds + 0.5);
	TestFalse(TEXT("A second press after the window only arms again"), State->Calling.PendingEffect.IsValid());
	Panel->PressAbdicate(Now + UCosmicSystemPanelWidget::AbdicateConfirmSeconds + 1.0);
	TestTrue(TEXT("Two presses in the window record the abdication"), State->Calling.PendingEffect.IsValid());
	TestFalse(TEXT("and the button greys while it waits for the tick"), Panel->GetAbdicateButton()->GetIsEnabled());
	TestTrue(TEXT("saying it leaves"), Words(Panel->GetAbdicateButton()->GetToolTipText()).StartsWith(Words(CosmicUI::Label(TEXT("Wander.Block.Leaving")))));
	State->Calling.PendingCause = Tag(TEXT("Reign.Deposed"));
	Panel->Refresh();
	TestEqual(TEXT("A fall waiting for the tick is told as a fall, not as an abdication"), Words(Panel->GetAbdicateButton()->GetToolTipText()),
		Words(CosmicUI::Label(TEXT("Wander.Block.Deposed"))));

	// 3. The wanderer at 4, knowing what it sees: [Scan] where it stands, [Travel] on its three neighbours, nothing beyond.
	State = LayBoard(*Sim);
	Walk(*Sim, *Role, 4);
	Panel->ShowSystem(State->Systems[4].SystemID);
	TestTrue(TEXT("Where it stands, a wild system: [Scan] alone"), ActionsOf(*Panel) == TArray<FName>({ Scan }));
	TestTrue(TEXT("enabled"), ButtonOf(*Panel, Scan) && ButtonOf(*Panel, Scan)->GetIsEnabled());
	{
		const FString Nothing = Words(CosmicUI::Label(TEXT("Wander.ScanNothing")));
		TestTrue(TEXT("(a first scan)"), Press(*Panel, Scan));
		TestFalse(TEXT("A scan that learned something says no 'nothing new'"), Words(Panel->GetDetailText()->GetText()).Contains(Nothing));
		TestTrue(TEXT("(a second scan)"), Press(*Panel, Scan));
		TestTrue(TEXT("A second scan says it learned nothing new"), Words(Panel->GetDetailText()->GetText()).Contains(Nothing));
		Panel->ShowSystem(State->Systems[3].SystemID);
		Panel->ShowSystem(State->Systems[4].SystemID);
		TestFalse(TEXT("and says it no more once another system was shown"), Words(Panel->GetDetailText()->GetText()).Contains(Nothing));
		State->Knowing = FKnowingState();
		CosmicTests::SetKnowing(*State, { 0, 3, 4, 5, 10 }, { 0 });
	}
	{
		// The bar's line while standing: where, and no 'no lane' - all three neighbours live.
		const FString Bar = Words(UCosmicTopBarWidget::WandererLineOf(*State, Index, ECosmicTimeUnit::Cycle));
		TestTrue(TEXT("The bar names where the vessel stands"), Bar.Contains(Words(State->Systems[4].DisplayName)));
		TestFalse(TEXT("with no 'no lane' while lanes lead on"), Bar.Contains(Words(CosmicUI::Label(TEXT("Wander.Bar.Stuck")))));
	}
	for (const int32 Neighbour : { 3, 5, 10 })
	{
		Panel->ShowSystem(State->Systems[Neighbour].SystemID);
		TestTrue(FString::Printf(TEXT("A lane neighbour (%d): [Travel]"), Neighbour), ActionsOf(*Panel) == TArray<FName>({ Travel }));
		TestTrue(TEXT("enabled"), ButtonOf(*Panel, Travel) && ButtonOf(*Panel, Travel)->GetIsEnabled());
		TestTrue(TEXT("(the panel's reason is the command's)"), Panel->GetShownTravelBlock() == TOptional<CosmicWanderer::ETravelBlock>(
			CosmicWanderer::TravelBlockOf(*State, Index, State->Systems[Neighbour].SystemID, false)));
	}
	TestTrue(TEXT("[Travel] names the lane's time"), Words(ButtonOf(*Panel, Travel)->GetLabel()).Contains(Words(CosmicUI::DurationLabel(Sim->GetRules().LaneTravelTicks, ECosmicTimeUnit::Cycle))));
	CosmicTests::KnowAll(*State);
	Panel->ShowSystem(State->Systems[6].SystemID);
	TestEqual(TEXT("Two lanes off: no button"), Panel->GetWandererButtons().Num(), 0);
	TestFalse(TEXT("(no travel reason shown)"), Panel->GetShownTravelBlock().IsSet());
	TestTrue(TEXT("The wanderer has no [Abdicate]"), Panel->GetAbdicateButton()->GetVisibility() == ESlateVisibility::Collapsed);
	TestEqual(TEXT("nor project buttons"), Panel->GetActionButtons().Num(), 0);

	// 4. A card waits: [Travel] greys with the command's reason; [Scan] does not wait for cards.
	State->Knowing.PendingPetitionRow = FName(TEXT("enc_test"));
	Panel->ShowSystem(State->Systems[5].SystemID);
	TestTrue(TEXT("A waiting card greys [Travel]"), ButtonOf(*Panel, Travel) && !ButtonOf(*Panel, Travel)->GetIsEnabled());
	TestTrue(TEXT("with the command's reason"), Panel->GetShownTravelBlock() == TOptional<CosmicWanderer::ETravelBlock>(CosmicWanderer::ETravelBlock::CardWaiting)
		&& CosmicWanderer::TravelBlockOf(*State, Index, State->Systems[5].SystemID, true) == CosmicWanderer::ETravelBlock::CardWaiting);
	TestTrue(TEXT("its words first in the tooltip"), Words(ButtonOf(*Panel, Travel)->GetToolTipText()).StartsWith(Words(CosmicUI::Label(TEXT("Wander.Block.CardWaiting")))));
	Panel->ShowSystem(State->Systems[6].SystemID);
	TestEqual(TEXT("While a card waits, a system off the lanes has no button"), Panel->GetWandererButtons().Num(), 0);
	Panel->ShowSystem(State->Systems[4].SystemID);
	TestTrue(TEXT("[Scan] stays open while a card waits"), ButtonOf(*Panel, Scan) && ButtonOf(*Panel, Scan)->GetIsEnabled());
	State->Knowing.PendingPetitionRow = NAME_None;

	// 5. Under way: every button greys with Travelling; the vessel's mark is on the lane.
	TestTrue(TEXT("(it sets off to 5)"), Role->TravelTo(State->Systems[5].SystemID) == CosmicWanderer::ETravelBlock::None);
	Panel->ShowSystem(State->Systems[3].SystemID);
	TestTrue(TEXT("Under way, [Travel] greys"), ButtonOf(*Panel, Travel) && !ButtonOf(*Panel, Travel)->GetIsEnabled());
	TestTrue(TEXT("as the command says"), Panel->GetShownTravelBlock() == TOptional<CosmicWanderer::ETravelBlock>(CosmicWanderer::ETravelBlock::Travelling));
	TestTrue(TEXT("in its words"), ButtonOf(*Panel, Travel) && Words(ButtonOf(*Panel, Travel)->GetToolTipText()).StartsWith(Words(CosmicUI::Label(TEXT("Wander.Block.Travelling")))));
	Panel->ShowSystem(State->Systems[6].SystemID);
	TestEqual(TEXT("Under way, a system off the lanes has no button"), Panel->GetWandererButtons().Num(), 0);
	{
		const FString Bar = Words(UCosmicTopBarWidget::WandererLineOf(*State, Index, ECosmicTimeUnit::Cycle));
		TestTrue(TEXT("The bar names the target and the time left"), Bar.Contains(Words(State->Systems[5].DisplayName))
			&& Bar.Contains(Words(CosmicUI::TimeLeftLabel(State->Player.TravelTicksRemaining, ECosmicTimeUnit::Cycle))));
		TestFalse(TEXT("not the system it left"), Bar.Contains(Words(State->Systems[4].DisplayName)));
	}
	Panel->ShowSystem(State->Systems[4].SystemID);
	TestTrue(TEXT("and [Scan] too"), ButtonOf(*Panel, Scan) && !ButtonOf(*Panel, Scan)->GetIsEnabled());
	{
		const FCosmicVesselLook Look = CosmicMapLook::VesselLookOf(*State, Index, Sim->GetRules());
		TestTrue(TEXT("The mark: from 4 to 5"), Look.FromIndex == 4 && Look.ToIndex == 5);
		TestEqual(TEXT("at the start of the lane"), Look.Share, 0.f);
		Sim->RunHeadless(Sim->GetRules().LaneTravelTicks / 2);
		const FCosmicVesselLook Half = CosmicMapLook::VesselLookOf(*State, Index, Sim->GetRules());
		TestTrue(TEXT("half way along it"), FMath::IsNearlyEqual(Half.Share, 0.5f, 0.05f));
	}
	Sim->RunHeadless(Sim->GetRules().LaneTravelTicks);
	TestTrue(TEXT("(arrived at 5)"), State->Player.CurrentSystemID == State->Systems[5].SystemID);
	{
		const FCosmicVesselLook Look = CosmicMapLook::VesselLookOf(*State, Index, Sim->GetRules());
		TestTrue(TEXT("Standing, the mark is on 5 alone"), Look.FromIndex == 5 && Look.ToIndex == INDEX_NONE);
	}

	// 6. At 5, people 1's settled system: [Scan] and [Crown]; unmet, then lacking, then crowned by reverence.
	State->Knowing.PendingPetitionRow = NAME_None;
	State->Knowing.MetCivs.RemoveAll([State](const FKnownMark& Mark) { return Mark.ID == State->Civs[1].CivID; });
	Panel->ShowSystem(State->Systems[5].SystemID);
	TestTrue(TEXT("At a people's settled system: [Scan] and [Crown]"), ActionsOf(*Panel) == TArray<FName>({ Scan, Crown }));
	TestTrue(TEXT("A people not met: greyed, Unmet"), Panel->GetShownCrownBlock() == TOptional<CosmicWanderer::ECrownBlock>(CosmicWanderer::ECrownBlock::Unmet));
	TestTrue(TEXT("in its words"), ButtonOf(*Panel, Crown) && Words(ButtonOf(*Panel, Crown)->GetToolTipText()).StartsWith(Words(CosmicUI::Label(TEXT("Wander.Block.Unmet")))));
	TestFalse(TEXT("No reverence line for a people not met"), Words(Panel->GetDetailText()->GetText()).Contains(Reverence0));
	Meet(*State, 1);
	Panel->Refresh();
	TestTrue(TEXT("Met, without reverence or a vacuum: Lacking"), Panel->GetShownCrownBlock() == TOptional<CosmicWanderer::ECrownBlock>(CosmicWanderer::ECrownBlock::Lacking));
	TestTrue(TEXT("the same as the command's gate"), CosmicWanderer::CrownBlockOf(*State, Index, Sim->GetRules(), State->Civs[1].CivID, true, false) == CosmicWanderer::ECrownBlock::Lacking);
	TestTrue(TEXT("in its words"), ButtonOf(*Panel, Crown) && Words(ButtonOf(*Panel, Crown)->GetToolTipText()).StartsWith(Words(CosmicUI::Label(TEXT("Wander.Block.Lacking")))));
	TestTrue(TEXT("Its tooltip tells the reverence and the thresholds"), Words(ButtonOf(*Panel, Crown)->GetToolTipText()).Contains(Words(CosmicUI::Percent(Sim->GetRules().CoronationReverence))));
	{
		// The people the vessel left: [Return], open at no reverence once the lock has passed.
		const FName Former = State->Player.FormerCivID;
		State->Player.FormerCivID = State->Civs[1].CivID;
		Panel->Refresh();
		TestTrue(TEXT("The people it left: [Return], open at reverence 0"), ButtonOf(*Panel, Crown) && ButtonOf(*Panel, Crown)->GetIsEnabled()
			&& Words(ButtonOf(*Panel, Crown)->GetLabel()).StartsWith(Words(CosmicUI::Label(TEXT("Wander.Return")))));
		State->Player.FormerCivID = Former;
		Panel->Refresh();
	}
	{
		// B6b [B-6b3a] (decision 41): a people three rungs or more above the vessel gives no throne - and the panel shows no
		// [Crown] or [Return] for it at all, not a greyed one (a 'too high' would reveal a people the map hides; the mutation
		// that drops the panel's filter and gives FarAbove the words of Unmet shows one; the one that drops the filter alone
		// shows one under way). Two rungs up, the button as before.
		State->Civs[1].Tier = EAscensionTier::Tier4_Transcendent;
		Panel->Refresh();
		TestTrue(TEXT("A people far above the vessel: [Scan] alone"), ActionsOf(*Panel) == TArray<FName>({ Scan }));
		TestFalse(TEXT("(no crown reason shown)"), Panel->GetShownCrownBlock().IsSet());
		TestTrue(TEXT("(the command's gate says FarAbove)"), CosmicWanderer::CrownBlockOf(*State, Index, Sim->GetRules(), State->Civs[1].CivID, true, false)
			== CosmicWanderer::ECrownBlock::FarAbove);
		const FName Former = State->Player.FormerCivID;
		State->Player.FormerCivID = State->Civs[1].CivID;
		Panel->Refresh();
		TestTrue(TEXT("The people it left, far above it: no [Return] either"), ActionsOf(*Panel) == TArray<FName>({ Scan }));
		// Nor under way from it or with a card waiting: the gate asks those first, and a greyed [Return] there would promise
		// a throne refused on arrival (the panel leaves out a met people far above before it asks the gate).
		State->Player.TravelTargetSystemID = State->Systems[4].SystemID;
		State->Player.TravelTicksRemaining = Sim->GetRules().LaneTravelTicks;
		Panel->Refresh();
		TestTrue(TEXT("Under way from a people far above it: [Scan] alone"), ActionsOf(*Panel) == TArray<FName>({ Scan }));
		TestFalse(TEXT("(no crown reason shown under way)"), Panel->GetShownCrownBlock().IsSet());
		State->Player.TravelTargetSystemID = NAME_None;
		State->Player.TravelTicksRemaining = 0;
		State->Knowing.PendingPetitionRow = FName(TEXT("enc_test"));
		Panel->Refresh();
		TestTrue(TEXT("With a card waiting, at a people far above it: [Scan] alone"), ActionsOf(*Panel) == TArray<FName>({ Scan }));
		TestFalse(TEXT("(no crown reason shown with a card waiting)"), Panel->GetShownCrownBlock().IsSet());
		State->Knowing.PendingPetitionRow = NAME_None;
		State->Player.FormerCivID = Former;
		// Not met, the meeting comes first as in the gate: the greyed button of any people not met - Unmet standing,
		// Travelling under way - so that no missing button tells a rung the map hides (the mutation that asks the rung
		// without the meeting shows [Scan] alone).
		const TArray<FKnownMark> WasMet = State->Knowing.MetCivs;
		State->Knowing.MetCivs.RemoveAll([State](const FKnownMark& Mark) { return Mark.ID == State->Civs[1].CivID; });
		Panel->Refresh();
		TestTrue(TEXT("A people far above, not met: the greyed Unmet of any people not met"), ActionsOf(*Panel) == TArray<FName>({ Scan, Crown })
			&& Panel->GetShownCrownBlock() == TOptional<CosmicWanderer::ECrownBlock>(CosmicWanderer::ECrownBlock::Unmet));
		State->Player.TravelTargetSystemID = State->Systems[4].SystemID;
		State->Player.TravelTicksRemaining = Sim->GetRules().LaneTravelTicks;
		Panel->Refresh();
		TestTrue(TEXT("and under way, the greyed Travelling of any people not met"), ActionsOf(*Panel) == TArray<FName>({ Scan, Crown })
			&& Panel->GetShownCrownBlock() == TOptional<CosmicWanderer::ECrownBlock>(CosmicWanderer::ECrownBlock::Travelling));
		State->Player.TravelTargetSystemID = NAME_None;
		State->Player.TravelTicksRemaining = 0;
		State->Knowing.MetCivs = WasMet;
		State->Civs[1].Tier = EAscensionTier::Tier3_Causal;
		Panel->Refresh();
		TestTrue(TEXT("Two rungs up: [Crown] again, greyed for the reverence"), ActionsOf(*Panel) == TArray<FName>({ Scan, Crown })
			&& Panel->GetShownCrownBlock() == TOptional<CosmicWanderer::ECrownBlock>(CosmicWanderer::ECrownBlock::Lacking));
		State->Civs[1].Tier = EAscensionTier::Tier1_Planetary;
		Panel->Refresh();
	}
	TestTrue(TEXT("The reverence line shows for the met people"), Words(Panel->GetDetailText()->GetText()).Contains(
		Words(FText::Format(CosmicUI::Label(TEXT("Wander.Reverence")), CosmicUI::Percent(0.f)))));
	State->Civs[1].ReverenceToPlayer = Sim->GetRules().CoronationReverence;
	Panel->Refresh();
	TestTrue(TEXT("Reverence at the threshold: enabled"), ButtonOf(*Panel, Crown) && ButtonOf(*Panel, Crown)->GetIsEnabled());
	TestTrue(TEXT("labelled [Crown], not [Return]"), Words(ButtonOf(*Panel, Crown)->GetLabel()).StartsWith(Words(CosmicUI::Label(TEXT("Wander.Crown")))));
	State->Civs[1].ThroneLockedUntilTick = State->Tick + 40;
	Panel->Refresh();
	TestTrue(TEXT("A locked throne: greyed, ThroneLocked"), Panel->GetShownCrownBlock() == TOptional<CosmicWanderer::ECrownBlock>(CosmicWanderer::ECrownBlock::ThroneLocked));
	TestTrue(TEXT("with the time left on its label"), Words(ButtonOf(*Panel, Crown)->GetLabel()).Contains(Words(CosmicUI::TimeLeftLabel(40, ECosmicTimeUnit::Cycle))));
	{
		// The tooltip keeps still while the lock counts down (an open tooltip set again would close).
		const FString Before = Words(ButtonOf(*Panel, Crown)->GetToolTipText());
		State->Tick += 1;
		Panel->Refresh();
		TestEqual(TEXT("and a tooltip that keeps still as the lock counts down"), Words(ButtonOf(*Panel, Crown)->GetToolTipText()), Before);
		State->Tick -= 1;
		Panel->Refresh();
	}
	State->Civs[1].ThroneLockedUntilTick = 0;
	Panel->Refresh();
	Controller->ClearRunEnd();
	TestTrue(TEXT("The controller crowns through the same gate"), Controller->TakeThrone(State->Civs[1].CivID));
	TestTrue(TEXT("(the vessel rules people 1)"), State->Player.Role == EPlayerRole::Ruler && State->Player.RuledCivID == State->Civs[1].CivID);
	Panel->Refresh();
	TestEqual(TEXT("A ruler again: no wanderer's button"), Panel->GetWandererButtons().Num(), 0);

	// 7. The map draws the mark where the vessel stands, and nothing for a ruler.
	if (ACosmicGalaxyMapActor* Map = Controller->GetMap())
	{
		State = LayBoard(*Sim);
		Walk(*Sim, *Role, 4);
		Map->RefreshNow();
		TestEqual(TEXT("The map marks the system the vessel stands in"), Map->GetDrawnVesselSystem(), State->Systems[4].SystemID);
		TestFalse(TEXT("no lane lit while it stands"), Map->IsVesselLaneDrawn());
		// Through the controller, which asks the map to redraw (a paused simulation sends no tick).
		TestTrue(TEXT("The controller sets off towards 3"), Controller->TravelTo(State->Systems[3].SystemID));
		const FCosmicVesselLook Look = CosmicMapLook::VesselLookOf(*State, Sim->GetStateIndex(), Sim->GetRules());
		TestTrue(TEXT("(the look goes from 4 to 3)"), Look.FromIndex == 4 && Look.ToIndex == 3);
		Map->RefreshNow();
		TestTrue(TEXT("Under way, the lane is lit"), Map->IsVesselLaneDrawn());
		State = LayBoard(*Sim);
		Map->RefreshNow();
		TestTrue(TEXT("A ruler has no mark"), Map->GetDrawnVesselSystem().IsNone());
	}
	return true;
}

// The wanderer's card on the screen (B1 header section 4): its choices can be given (no cost) and their passing rolls are
// the vessel's stat; a withdrawn card says so in its own words; a wanderer's card stops the time with its own reason; a
// coronation over a people with a standing hazard does not stop the time for 'my crisis' on the next tick.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIWandererCardTest, "Cosmic_Ascension.UI.WandererCard", CosmicTests::Flags)
bool FCosmicUIWandererCardTest::RunTest(const FString& Parameters)
{
	using namespace CosmicWandererUITests;

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
	UCosmicPetitionCardWidget* Card = HUD ? HUD->GetPetitionCard() : nullptr;
	UPlayerRoleSubsystem* Role = Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	if (!TestNotNull(TEXT("The HUD has its card"), Card) || !TestNotNull(TEXT("(the role subsystem)"), Role))
	{
		return false;
	}
	const bool bViewerChoice = Controller->IsAutoPauseOn();
	ON_SCOPE_EXIT
	{
		Sim->SetTimeScale(0.f);
		Controller->SetAutoPause(bViewerChoice);
		Controller->ClearRunEnd();
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
		Controller->FollowOwnHazards();
		Controller->FollowPetition(false);
	};
	Sim->SetTimeScale(0.f);
	Controller->SetAutoPause(true);
	FGalaxyState* State = LayBoard(*Sim);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const FName Plague = CosmicRows::EncPlagueTend;
	const FEncounterDef* Row = Sim->GetTables().FindEncounter(Plague);
	if (!TestNotNull(TEXT("(the game has the card enc_plague_tend)"), Row) || !TestTrue(TEXT("(with a checked choice)"), Row->Choices.Num() >= 1))
	{
		return false;
	}

	// 1. A wanderer at 5 beside a plague, its card waiting: shown, every choice can be given, the rolls are the vessel's.
	Walk(*Sim, *Role, 4);
	State->Player.CurrentSystemID = State->Systems[5].SystemID;
	State->Systems[5].ActiveHazard = Tag(TEXT("Hazard.Plague"));
	State->Systems[5].HazardTicksRemaining = 150;
	Meet(*State, 1);
	State->Player.Body = 2;
	State->Player.Mind = 4;
	State->Knowing.PendingPetitionRow = Plague;
	State->Knowing.PendingPetitionSystemID = State->Systems[5].SystemID;
	Sim->SetTimeScale(1.f);
	Controller->FollowPetition(true);
	TestTrue(TEXT("The wanderer's card shows"), Card->GetShownRow() == Plague);
	// The top bar's line of the wanderer, with its stats in the tooltip.
	if (UCosmicTopBarWidget* Bar = HUD->GetTopBar())
	{
		Bar->Refresh();
		const UTextBlock* Line = Bar->GetWandererText();
		TestTrue(TEXT("The top bar shows the wanderer's line"), Line && Line->GetVisibility() != ESlateVisibility::Collapsed
			&& Words(Line->GetText()) == Words(UCosmicTopBarWidget::WandererLineOf(*State, Sim->GetStateIndex(), Controller->GetTimeUnit())));
		TestTrue(TEXT("naming where it stands"), Line && Words(Line->GetText()).Contains(Words(State->Systems[5].DisplayName)));
		TestTrue(TEXT("Its tooltip carries the vessel's body"), Bar->GetWandererTip().ToString().Contains(Words(FText::Format(INVTEXT("{0} {1}"),
			CosmicUI::Label(TEXT("Petition.Stat.Body")), FText::AsNumber(2)))));
		TestTrue(TEXT("The resources stay empty"), Bar->GetStatusText()->GetText().IsEmpty());
	}
	TestTrue(TEXT("and stops the time with its own reason"), !(Sim->GetTimeScale() > 0.f) && Controller->GetAutoPauseReason() == ECosmicAutoPause::Encounter);
	TestTrue(TEXT("whose words are the card's, not a petition's"), FString(CosmicAutoPause::WhyKeyOf(ECosmicAutoPause::Encounter)) == TEXT("AutoPause.Why.Encounter"));
	bool bAllGiven = Card->GetChoiceCount() == Row->Choices.Num();
	for (int32 At = 0; At < Card->GetChoiceCount(); ++At)
	{
		bAllGiven &= Card->GetChoiceButton(At) && Card->GetChoiceButton(At)->GetIsEnabled();
	}
	TestTrue(TEXT("Every choice can be given: a wanderer pays nothing"), bAllGiven);
	for (int32 At = 0; At < Row->Choices.Num(); ++At)
	{
		if (Row->Choices[At].CheckStat == ECosmicCheckStat::NoCheck)
		{
			continue;
		}
		const int32 Rolls = CosmicPetitions::PassingRollsOf(*State, INDEX_NONE, Row->Choices[At]);
		TestTrue(FString::Printf(TEXT("Choice %d's line names the vessel's passing rolls (%d)"), At, Rolls),
			Card->GetChoiceLine(At) && Words(Card->GetChoiceLine(At)->GetText()).Contains(FString::Printf(TEXT("%d/6"), Rolls)));
	}

	// 2. The vessel was carried away: the card still speaks of its own system (no people named); the answer withdraws it in
	// the wanderer's words; the close gives the time back.
	State->Player.CurrentSystemID = State->Systems[6].SystemID;
	Card->ShowCard(Plague, State->Systems[5].SystemID);
	TestTrue(TEXT("A card shown again after the vessel moved still names its own system"), Words(Card->GetBodyText()->GetText()).Contains(Words(State->Systems[5].DisplayName))
		&& !Words(Card->GetBodyText()->GetText()).Contains(Words(State->Systems[6].DisplayName)));
	Card->PressChoice(0);
	TestTrue(TEXT("A withdrawn wanderer's card says the matter has passed"), Words(Card->GetOutcomeText()->GetText()).Equals(Words(CosmicUI::Label(TEXT("Wander.Withdrawn")))));
	Card->PressClose();
	TestTrue(TEXT("Closing the wanderer's card gives the time back"), Sim->GetTimeScale() > 0.f && Controller->GetAutoPauseReason() == ECosmicAutoPause::None);
	Sim->SetTimeScale(0.f);

	// 2b. The arrival card on a wild system: every choice can be given, the text names the place.
	{
		const FName Weave(TEXT("enc_lane_weave"));
		State->Player.CurrentSystemID = State->Systems[4].SystemID;
		State->Knowing.PendingPetitionRow = Weave;
		State->Knowing.PendingPetitionSystemID = State->Systems[4].SystemID;
		Controller->FollowPetition(false);
		TestTrue(TEXT("The arrival card shows on a wild system"), Card->GetShownRow() == Weave);
		bool bAllOpen = Card->GetChoiceCount() > 0;
		for (int32 At = 0; At < Card->GetChoiceCount(); ++At)
		{
			bAllOpen &= Card->GetChoiceButton(At) && Card->GetChoiceButton(At)->GetIsEnabled();
		}
		TestTrue(TEXT("with every choice open (no people pays for the wanderer)"), bAllOpen);
		TestTrue(TEXT("and its text names the place"), Words(Card->GetBodyText()->GetText()).Contains(Words(State->Systems[4].DisplayName)));
		State->Knowing.PendingPetitionRow = NAME_None;
		State->Knowing.PendingPetitionSystemID = NAME_None;
		Controller->FollowPetition(false);
	}

	// 3. Crowned over a people with a standing hazard: no 'my crisis' stop on the next tick; a crisis that rises on that
	// very tick still stops the time.
	auto CrownBeside = [&]()
	{
		State = LayBoard(*Sim);
		Controller->FollowOwnHazards();
		Walk(*Sim, *Role, 4);
		Controller->FollowOwnHazards();
		State->Player.CurrentSystemID = State->Systems[5].SystemID;
		State->Systems[6].ActiveHazard = Tag(TEXT("Hazard.Plague"));
		State->Systems[6].HazardTicksRemaining = 300;
		Meet(*State, 1);
		State->Civs[1].ReverenceToPlayer = 1.f;
		State->Knowing.PendingPetitionRow = NAME_None;
		// Taken while the time runs (a click on a running game).
		Sim->SetTimeScale(1.f);
		return Controller->TakeThrone(State->Civs[1].CivID);
	};
	// The coronation's own line may stop the time (a mythic line); 'my crisis' does not, and the time runs on from there.
	TestTrue(TEXT("(crowned over people 1, at once a crisis on its first tick)"), CrownBeside());
	TestTrue(TEXT("Taking a throne beside a standing crisis is no 'my crisis' stop"), Controller->GetAutoPauseReason() != ECosmicAutoPause::OwnHazard);
	Controller->SetSpeed(1.f);
	State->Systems[5].ActiveHazard = Tag(TEXT("Hazard.Plague"));
	State->Systems[5].HazardTicksRemaining = 300;
	Controller->FollowOwnHazards();
	TestTrue(TEXT("A crisis that rises on the reign's first tick stops the time"), !(Sim->GetTimeScale() > 0.f) && Controller->GetAutoPauseReason() == ECosmicAutoPause::OwnHazard);
	Controller->SetSpeed(0.f);
	TestTrue(TEXT("(crowned over people 1)"), CrownBeside());
	TestTrue(TEXT("The coronation itself is no 'my crisis' stop"), Controller->GetAutoPauseReason() != ECosmicAutoPause::OwnHazard);
	if (UCosmicTopBarWidget* Bar = HUD->GetTopBar())
	{
		Bar->Refresh();
		TestTrue(TEXT("A ruler's top bar has no wanderer's line"), Bar->GetWandererText() && Bar->GetWandererText()->GetVisibility() == ESlateVisibility::Collapsed);
	}
	Controller->SetSpeed(1.f);
	Controller->FollowOwnHazards();
	TestTrue(TEXT("A crisis that stood before the throne stops nothing"), Sim->GetTimeScale() > 0.f);
	TestTrue(TEXT("(no 'my crisis' reason)"), Controller->GetAutoPauseReason() != ECosmicAutoPause::OwnHazard);
	State->Systems[5].ActiveHazard = Tag(TEXT("Hazard.Plague"));
	State->Systems[5].HazardTicksRemaining = 300;
	Controller->FollowOwnHazards();
	TestTrue(TEXT("A crisis that rises afterwards still stops the time"), !(Sim->GetTimeScale() > 0.f) && Controller->GetAutoPauseReason() == ECosmicAutoPause::OwnHazard);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

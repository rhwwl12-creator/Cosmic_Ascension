// Cosmic Ascension - automation tests of the research focus on the screen (MVP item 7): the system panel's block and the
// top bar's research tooltip. Design: docs/60_DECISIONS/headers/A7_research_focus.md section 3 and 6.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Events/CosmicEventSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicResearch.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "UI/CosmicUILabels.h"
#include "Widgets/IToolTip.h"
#include "Widgets/SWidget.h"
#include "World/CosmicGameMode.h"

namespace CosmicResearchFocusUITests
{
	const FName Physics(TEXT("Physics"));
	const FName Society(TEXT("Society"));
	const FName Engineering(TEXT("Engineering"));
	const FName Transcendence(TEXT("Transcendence"));

	FString Words(const TCHAR* Key)
	{
		return CosmicUI::Label(Key).ToString();
	}

	// The panel's focus button for Field (NAME_None: the even one), or null.
	UCosmicActionButton* FocusButtonOf(const UCosmicSystemPanelWidget& Panel, FName Field)
	{
		for (UCosmicActionButton* Button : Panel.GetFocusButtons())
		{
			if (Button && Button->GetAction() == Field)
			{
				return Button;
			}
		}
		return nullptr;
	}

	// How many times Needle stands in Hay.
	int32 CountOf(const FString& Hay, const FString& Needle)
	{
		int32 Count = 0;
		for (int32 At = Hay.Find(Needle); At != INDEX_NONE; At = Hay.Find(Needle, ESearchCase::CaseSensitive, ESearchDir::FromStart, At + Needle.Len()))
		{
			++Count;
		}
		return Count;
	}

	void StepBroadcast(UGalaxySimulationSubsystem& Sim)
	{
		const bool bWasBroadcasting = Sim.IsBroadcastEnabled();
		Sim.SetBroadcastEnabled(true);
		Sim.StepOnce();
		Sim.SetBroadcastEnabled(bWasBroadcasting);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIResearchFocusTest, "Cosmic_Ascension.UI.ResearchFocus", CosmicTests::Flags)
bool FCosmicUIResearchFocusTest::RunTest(const FString& Parameters)
{
	using namespace CosmicResearchFocusUITests;
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
	UCosmicSystemPanelWidget* Panel = HUD ? HUD->GetSystemPanel() : nullptr;
	UCosmicTopBarWidget* Bar = HUD ? HUD->GetTopBar() : nullptr;
	if (!TestNotNull(TEXT("(the HUD's system panel)"), Panel) || !TestNotNull(TEXT("(and its top bar)"), Bar))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef Rules = Sim->GetRules();
	ON_SCOPE_EXIT
	{
		Controller->ClearRunEnd();
		Controller->SelectSystem(NAME_None);
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->SetTimeScale(0.f);
	FGalaxyState& Live = Sim->GetMutableState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const int32 Ruled = Index.FindCiv(Live, Live.Player.RuledCivID);
	const int32 Home = Live.Civs.IsValidIndex(Ruled) ? Index.FindSystem(Live, Live.Civs[Ruled].HomeSystemID) : INDEX_NONE;
	const int32 Other = Live.Civs.IsValidIndex(1) ? Index.FindSystem(Live, Live.Civs[Ruled == 1 ? 0 : 1].HomeSystemID) : INDEX_NONE;
	if (!TestTrue(TEXT("(the ruled people has a home, and another people too)"), Live.Systems.IsValidIndex(Home) && Live.Systems.IsValidIndex(Other)))
	{
		return false;
	}
	CosmicTests::KnowAll(Sim->GetMutableState());
	const FName HomeID = Live.Systems[Home].SystemID;
	Controller->SelectSystem(HomeID);
	HUD->RefreshPanel();
	TestEqual(TEXT("(the home is selected)"), Controller->GetSelectedSystem(), HomeID);

	// The block: under the hint line, a heading and four buttons in order.
	if (!TestTrue(TEXT("The ruled home's panel shows the research focus"), Panel->IsFocusShown()))
	{
		return false;
	}
	const TArray<TObjectPtr<UCosmicActionButton>>& Buttons = Panel->GetFocusButtons();
	if (!TestEqual(TEXT("with four buttons"), Buttons.Num(), 4))
	{
		return false;
	}
	TestTrue(TEXT("Even, Physics, Society, Engineering in that order"), Buttons[0]->GetAction().IsNone() && Buttons[1]->GetAction() == Physics && Buttons[2]->GetAction() == Society && Buttons[3]->GetAction() == Engineering);
	TestTrue(TEXT("named as the top bar names the fields"), Buttons[0]->GetLabel().ToString() == Words(TEXT("Focus.Even")) && Buttons[1]->GetLabel().ToString() == Words(TEXT("Research.Physics"))
		&& Buttons[2]->GetLabel().ToString() == Words(TEXT("Research.Society")) && Buttons[3]->GetLabel().ToString() == Words(TEXT("Research.Engineering")));
	{
		const UVerticalBox* Column = Panel->GetColumn();
		const int32 HintAt = Column ? Column->GetChildIndex(Panel->GetHintText()) : INDEX_NONE;
		const int32 HeadingAt = Column && Panel->GetFocusHeading() ? Column->GetChildIndex(Panel->GetFocusHeading()->GetParent()) : INDEX_NONE;
		TestTrue(TEXT("The block stands under the hint line"), HintAt != INDEX_NONE && HeadingAt == HintAt + 1);
	}
	const FString EvenHeading = FText::Format(CosmicUI::Label(TEXT("Focus.Heading")), CosmicUI::Label(TEXT("Focus.Even"))).ToString();
	TestEqual(TEXT("A new run's heading says even"), Panel->GetFocusHeading()->GetText().ToString(), EvenHeading);
	TestFalse(TEXT("and the even button is greyed out"), Buttons[0]->GetIsEnabled());
	TestTrue(TEXT("its tooltip says it is the focus now"), Buttons[0]->GetToolTipText().ToString().StartsWith(Words(TEXT("Focus.TipNowEven")).Left(5)));
	TestTrue(TEXT("The three fields are enabled"), Buttons[1]->GetIsEnabled() && Buttons[2]->GetIsEnabled() && Buttons[3]->GetIsEnabled());
	{
		const FString Tip = Buttons[1]->GetToolTipText().ToString();
		TestEqual(TEXT("The physics tooltip shows 55% once"), CountOf(Tip, TEXT("55%")), 1);
		TestEqual(TEXT("and 15% three times"), CountOf(Tip, TEXT("15%")), 3);
		TestFalse(TEXT("with no slot left"), Tip.Contains(TEXT("{")));
		const int32 AtPhysics = Tip.Find(Words(TEXT("Research.Physics")));
		const int32 AtSociety = Tip.Find(Words(TEXT("Research.Society")));
		const int32 AtEngineering = Tip.Find(Words(TEXT("Research.Engineering")));
		const int32 AtTranscendence = Tip.Find(Words(TEXT("Research.Transcendence")));
		TestTrue(TEXT("in the top bar's order: physics, society, engineering, transcendence"), AtPhysics != INDEX_NONE && AtPhysics < AtSociety && AtSociety < AtEngineering && AtEngineering < AtTranscendence);
		TestTrue(TEXT("The first rung reads physics: its button says so"), Tip.Contains(Words(TEXT("Focus.TipNeeded"))));
		TestFalse(TEXT("society's does not"), Buttons[2]->GetToolTipText().ToString().Contains(Words(TEXT("Focus.TipNeeded"))));
		TestFalse(TEXT("nor engineering's on the first rung"), Buttons[3]->GetToolTipText().ToString().Contains(Words(TEXT("Focus.TipNeeded"))));
	}
	TestFalse(TEXT("The heading has a tooltip"), Panel->GetFocusHeading()->GetToolTipText().IsEmpty());

	// The objects stay across a refresh and a tick; an unchanged tooltip keeps its Slate object.
	{
		const TSharedRef<SWidget> SlatePanel = Panel->TakeWidget();
		const TArray<UCosmicActionButton*> Before = { Buttons[0], Buttons[1], Buttons[2], Buttons[3] };
		const TSharedPtr<SWidget> SlatePhysics = Buttons[1]->GetCachedWidget();
		const IToolTip* TipBefore = SlatePhysics.IsValid() ? SlatePhysics->GetToolTip().Get() : nullptr;
		Panel->Refresh();
		StepBroadcast(*Sim);
		bool bSame = Panel->GetFocusButtons().Num() == 4;
		for (int32 At = 0; bSame && At < 4; ++At)
		{
			bSame = Panel->GetFocusButtons()[At] == Before[At];
		}
		TestTrue(TEXT("The four buttons keep their objects across a refresh and a tick"), bSame);
		TestTrue(TEXT("and an unchanged tooltip keeps its Slate object"), TipBefore != nullptr && SlatePhysics->GetToolTip().Get() == TipBefore);
	}

	// A press through the controller: the HUD's panel shows it at once, paused.
	TestTrue(TEXT("The controller takes a focus on physics"), Controller->SetResearchFocus(Physics));
	TestEqual(TEXT("(still paused)"), Sim->GetTimeScale(), 0.f);
	TestFalse(TEXT("The physics button is greyed out at once"), Buttons[1]->GetIsEnabled());
	TestTrue(TEXT("and the even one is enabled again"), Buttons[0]->GetIsEnabled());
	TestEqual(TEXT("The heading says physics"), Panel->GetFocusHeading()->GetText().ToString(), FText::Format(CosmicUI::Label(TEXT("Focus.Heading")), CosmicUI::Label(TEXT("Research.Physics"))).ToString());
	TestTrue(TEXT("The top bar's research tooltip names the focus first"), Bar->GetResearchTip().ToString().StartsWith(FText::Format(CosmicUI::Label(TEXT("Focus.BarLine")), CosmicUI::Label(TEXT("Research.Physics")), CosmicUI::FocusSharesOf(Live.Civs[Ruled].ResearchFocus)).ToString()));
	TestTrue(TEXT("then the next rung"), Bar->GetResearchTip().ToString().Contains(Bar->GetNextRungTip().ToString()) && !Bar->GetNextRungTip().IsEmpty());
	// A press of a button: the even focus again.
	Buttons[0]->Press();
	TestEqual(TEXT("A press of 'even' gives the even focus"), CosmicResearch::ShapeOf(Live.Civs[Ruled].ResearchFocus), ECosmicFocusShape::Even);
	TestFalse(TEXT("and greys it"), Buttons[0]->GetIsEnabled());

	// The second rung reads engineering.
	Live.Civs[Ruled].Tier = EAscensionTier::Tier2_Stellar;
	Panel->Refresh();
	TestTrue(TEXT("On the second rung engineering's button says it is read"), Buttons[3]->GetToolTipText().ToString().Contains(Words(TEXT("Focus.TipNeeded"))));
	TestFalse(TEXT("and physics' no more"), Buttons[1]->GetToolTipText().ToString().Contains(Words(TEXT("Focus.TipNeeded"))));
	Live.Civs[Ruled].Tier = EAscensionTier::Tier1_Planetary;

	// The petition's answer leans to society: its button greys, the heading says society.
	{
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = Ruled;
		FCosmicEffect Effect;
		Effect.Type = EEffectType::FocusResearch;
		Effect.Key = Society;
		Effect.Value = 0.55f;
		TestTrue(TEXT("(the petition's way)"), UCosmicEventSubsystem::Apply(Effect, Ctx));
		Panel->Refresh();
		TestFalse(TEXT("After the petition's society answer its button is greyed"), Buttons[2]->GetIsEnabled());
		TestEqual(TEXT("and the heading says society"), Panel->GetFocusHeading()->GetText().ToString(), FText::Format(CosmicUI::Label(TEXT("Focus.Heading")), CosmicUI::Label(TEXT("Research.Society"))).ToString());
	}

	// A focus no button gives (transcendence, or a mixed map): all four enabled, the heading names it.
	Controller->SetResearchFocus(Transcendence);
	Panel->Refresh();
	TestTrue(TEXT("A transcendence focus leaves the four enabled"), Buttons[0]->GetIsEnabled() && Buttons[1]->GetIsEnabled() && Buttons[2]->GetIsEnabled() && Buttons[3]->GetIsEnabled());
	TestEqual(TEXT("and the heading names it"), Panel->GetFocusHeading()->GetText().ToString(), FText::Format(CosmicUI::Label(TEXT("Focus.Heading")), CosmicUI::Label(TEXT("Research.Transcendence"))).ToString());
	Live.Civs[Ruled].ResearchFocus = { { Physics, 0.4f }, { Society, 0.3f }, { Engineering, 0.2f }, { Transcendence, 0.1f } };
	Panel->Refresh();
	TestTrue(TEXT("A mixed map leaves the four enabled"), Buttons[0]->GetIsEnabled() && Buttons[1]->GetIsEnabled() && Buttons[2]->GetIsEnabled() && Buttons[3]->GetIsEnabled());
	TestEqual(TEXT("and the heading says so"), Panel->GetFocusHeading()->GetText().ToString(), FText::Format(CosmicUI::Label(TEXT("Focus.Heading")), CosmicUI::Label(TEXT("Focus.Mixed"))).ToString());
	CosmicResearch::Even(Live.Civs[Ruled].ResearchFocus);

	// The shares follow the knob.
	{
		FCosmicRulesDef Steep = Rules;
		Steep.ResearchFocusShare = 0.7f;
		Sim->SetTables(CosmicTests::WithRules(Original, Steep));
		Panel->Refresh();
		const FString Tip = Buttons[1]->GetToolTipText().ToString();
		TestTrue(TEXT("A knob of 0.7: 70% and 10% in the physics tooltip"), CountOf(Tip, TEXT("70%")) == 1 && CountOf(Tip, TEXT("10%")) == 3);
		Sim->SetTables(Original);
		Panel->Refresh();
	}

	// No block where the project buttons have none: another people's system, a settling system, a destroyed one, a wanderer.
	Controller->SelectSystem(Live.Systems[Other].SystemID);
	HUD->RefreshPanel();
	TestFalse(TEXT("Another people's system has no focus block"), Panel->IsFocusShown());
	Controller->SelectSystem(HomeID);
	const FName WasActive = Live.Systems[Home].ActiveMegaproject;
	Live.Systems[Home].ActiveMegaproject = CosmicRows::ProjectSettle;
	Panel->Refresh();
	TestFalse(TEXT("nor a settling system of the people"), Panel->IsFocusShown());
	Live.Systems[Home].ActiveMegaproject = WasActive;
	// A destroyed system has no star class (CosmicHazards::IsDeadNode).
	const FGameplayTag WasClass = Live.Systems[Home].StarClass;
	Live.Systems[Home].StarClass = FGameplayTag();
	Panel->Refresh();
	TestFalse(TEXT("nor a destroyed system"), Panel->IsFocusShown());
	Live.Systems[Home].StarClass = WasClass;
	Panel->Refresh();
	TestTrue(TEXT("(settled again, it stands)"), Panel->IsFocusShown());
	const EPlayerRole WasRole = Live.Player.Role;
	Live.Player.Role = EPlayerRole::Wanderer;
	Panel->Refresh();
	TestFalse(TEXT("A wanderer has no focus block"), Panel->IsFocusShown());
	Live.Player.Role = WasRole;
	Panel->Refresh();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

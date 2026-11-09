// Cosmic Ascension - automation tests of the ascension presentation (Presentation). Design: guideline.md section 6.5,
// 9.3, 9.4, 11 Phase 3.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Ascension/AscensionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Internationalization/StringTableCore.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicAscensionOverlayWidget.h"
#include "UI/CosmicPalette.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicGameMode.h"
#include "World/CosmicMapLook.h"

namespace CosmicAscensionOverlayTests
{
	// Everything the text blocks of a widget's own tree show (shared - Tests/CosmicTestHelpers.h).
	using CosmicTests::ReadTexts;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIAscensionOverlayTest, "Cosmic_Ascension.UI.AscensionOverlay", CosmicTests::Flags)
bool FCosmicUIAscensionOverlayTest::RunTest(const FString& Parameters)
{
	using namespace CosmicAscensionOverlayTests;

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
	UAscensionSubsystem* Ascension = Sim->GetGameInstance()->GetSubsystem<UAscensionSubsystem>();
	if (!TestNotNull(TEXT("The first player controller is the game's"), Controller) || !TestNotNull(TEXT("The ascension subsystem runs"), Ascension))
	{
		return false;
	}
	ON_SCOPE_EXIT
	{
		Sim->SetTimeScale(0.f);
		Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
		Sim->NewGalaxy(42, 5, 3);
	};
	Sim->NewGalaxy(42, 0, 0);

	// The game's first feat row (T1 to T2), as the delegate names it.
	TArray<FName> FeatRows = Sim->GetTables().GetFeatRowNames();
	const FAscensionFeatDef* Feat = FeatRows.IsEmpty() ? nullptr : Sim->GetTables().FindFeat(FeatRows[0]);
	if (!TestNotNull(TEXT("(the game has a feat row)"), Feat))
	{
		return false;
	}

	// The HUD's own overlay: a second one would answer the same delegate and pause or give back the speed in its place.
	ACosmicHUD* HUD = Controller->GetHUD<ACosmicHUD>();
	UCosmicAscensionOverlayWidget* Overlay = HUD ? HUD->GetAscensionOverlay() : nullptr;
	if (!TestNotNull(TEXT("The HUD has its ascension overlay"), Overlay))
	{
		return false;
	}
	Overlay->Close();
	TestFalse(TEXT("The overlay is hidden"), Overlay->IsShowing());
	TestTrue(TEXT("and collapsed"), Overlay->GetVisibility() == ESlateVisibility::Collapsed);
	auto Broadcast = [&](EAscensionTier Tier, FName Row)
	{
		const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
		Sim->SetBroadcastEnabled(true);
		Ascension->OnTierAscended.Broadcast(Tier, Row, EPlayerRole::Ruler);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
	};

	// ---- An ascension shows the presentation and pauses a running simulation; five seconds later it gives the speed back ----
	{
		Controller->SetSpeed(2.f);
		Broadcast(EAscensionTier::Tier2_Stellar, FeatRows[0]);
		TestTrue(TEXT("An ascension shows the overlay"), Overlay->IsShowing());
		TestTrue(TEXT("and makes it visible"), Overlay->GetVisibility() != ESlateVisibility::Collapsed);
		const FString Shown = ReadTexts(*Overlay);
		TestTrue(TEXT("with the feat's title"), Shown.Contains(Feat->Title.ToString()));
		TestTrue(TEXT("its narrative"), Shown.Contains(Feat->MythicNarrative.ToString()));
		TestTrue(TEXT("and the new rung"), Shown.Contains(FString::Printf(TEXT("%s 2"), *CosmicUI::Label(TEXT("Panel.Tier")).ToString())));
		TestEqual(TEXT("The simulation is paused"), Sim->GetTimeScale(), 0.f);
		Overlay->Advance(CosmicAscensionOverlay::Seconds - 0.1f);
		TestTrue(TEXT("It still shows just before five seconds"), Overlay->IsShowing());
		Overlay->Advance(0.2f);
		TestFalse(TEXT("and closes after them"), Overlay->IsShowing());
		TestTrue(TEXT("collapsed"), Overlay->GetVisibility() == ESlateVisibility::Collapsed);
		TestEqual(TEXT("giving the speed back"), Sim->GetTimeScale(), 2.f);
	}

	// ---- Shown while paused it leaves the pause; a speed the player set meanwhile stays ----
	{
		Controller->SetSpeed(0.f);
		Broadcast(EAscensionTier::Tier3_Causal, FeatRows[0]);
		TestTrue(TEXT("(shown while paused)"), Overlay->IsShowing());
		Overlay->Close();
		TestEqual(TEXT("Shown while paused, closing leaves the pause"), Sim->GetTimeScale(), 0.f);

		Controller->SetSpeed(2.f);
		Broadcast(EAscensionTier::Tier3_Causal, FeatRows[0]);
		TestEqual(TEXT("(paused again)"), Sim->GetTimeScale(), 0.f);
		Controller->SetSpeed(5.f);
		Overlay->Close();
		TestEqual(TEXT("A speed the player set meanwhile stays"), Sim->GetTimeScale(), 5.f);
		Controller->SetSpeed(0.f);
	}

	// ---- A second ascension while showing replaces the text; an unknown row shows the rung alone ----
	{
		Overlay->Show(EAscensionTier::Tier2_Stellar, FeatRows[0]);
		Overlay->Advance(3.f);
		Overlay->Show(EAscensionTier::Tier4_Transcendent, FName(TEXT("feat_nowhere")));
		const FString Shown = ReadTexts(*Overlay);
		TestTrue(TEXT("A second ascension shows its rung"), Shown.Contains(FString::Printf(TEXT("%s 4"), *CosmicUI::Label(TEXT("Panel.Tier")).ToString())));
		TestFalse(TEXT("and an unknown row no title"), Shown.Contains(Feat->Title.ToString()));
		Overlay->Advance(3.f);
		TestTrue(TEXT("and starts the five seconds again"), Overlay->IsShowing());
		Overlay->Close();
	}

	// ---- The third rung's presentation is followed by the reveal "this is seen now" over a map that shows the sight ----
	{
		FGalaxyState& Live = Sim->GetMutableState();
		const int32 Ruled = Sim->GetStateIndex().FindCiv(Live, Live.Player.RuledCivID);
		if (TestTrue(TEXT("(the player rules a people)"), Live.Civs.IsValidIndex(Ruled)))
		{
			// The ruled people on the third rung: the sight is open and the view drawn is spiritual.
			Live.Civs[Ruled].Tier = EAscensionTier::Tier3_Causal;
			Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
			Overlay->Show(EAscensionTier::Tier3_Causal, FeatRows[0]);
			TestFalse(TEXT("(no reveal while the presentation shows)"), Overlay->IsRevealing());
			Overlay->Close();
			TestTrue(TEXT("The third rung's presentation is followed by the reveal"), Overlay->IsRevealing());
			const UTextBlock* Reveal = Overlay->GetRevealText();
			if (TestNotNull(TEXT("(the overlay has its reveal line)"), Reveal))
			{
				TestTrue(TEXT("Its line shows"), Reveal->GetVisibility() != ESlateVisibility::Collapsed);
				TestEqual(TEXT("saying it is seen now"), Reveal->GetText().ToString(), CosmicUI::Label(TEXT("Sight.Opened")).ToString());
				TestTrue(TEXT("The reveal takes no click"), Overlay->GetVisibility() == ESlateVisibility::HitTestInvisible);
				TestFalse(TEXT("(the presentation itself is over)"), Overlay->IsShowing());
				// What the eye sees is the overlay's opacity: it fades in, stands at full strength, and fades out.
				TestTrue(TEXT("The reveal fades in"), Overlay->GetRenderOpacity() < 1.f);
				Overlay->Advance(0.5f * CosmicAscensionOverlay::RevealSeconds);
				TestEqual(TEXT("and stands at full strength halfway through its seconds"), Overlay->GetRenderOpacity(), 1.f);
				Overlay->Advance(0.5f * CosmicAscensionOverlay::RevealSeconds - 0.1f);
				TestTrue(TEXT("It still shows just before its seconds"), Overlay->IsRevealing());
				TestTrue(TEXT("fading out"), Overlay->GetRenderOpacity() > 0.f && Overlay->GetRenderOpacity() < 1.f);
				Overlay->Advance(0.2f);
				TestFalse(TEXT("and ends after them"), Overlay->IsRevealing());
				TestTrue(TEXT("Its line collapses"), Reveal->GetVisibility() == ESlateVisibility::Collapsed);
				TestTrue(TEXT("and a collapsed line carries no text"), Reveal->GetText().IsEmpty());
				TestTrue(TEXT("and the overlay is collapsed"), Overlay->GetVisibility() == ESlateVisibility::Collapsed);
			}
			// The other rungs have no reveal.
			Overlay->Show(EAscensionTier::Tier2_Stellar, FeatRows[0]);
			Overlay->Close();
			TestFalse(TEXT("A second rung's presentation has no reveal"), Overlay->IsRevealing());
			Overlay->Show(EAscensionTier::Tier4_Transcendent, FeatRows[0]);
			Overlay->Close();
			TestFalse(TEXT("nor a fourth rung's"), Overlay->IsRevealing());
			// Over a physical map the line would say what the map does not show.
			Controller->SetWantedViewMode(ECosmicViewMode::Physical);
			Overlay->Show(EAscensionTier::Tier3_Causal, FeatRows[0]);
			Overlay->Close();
			TestFalse(TEXT("Over a physical map the third rung's presentation has no reveal"), Overlay->IsRevealing());
			Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
			// A presentation ends a reveal still showing.
			Overlay->Show(EAscensionTier::Tier3_Causal, FeatRows[0]);
			Overlay->Close();
			TestTrue(TEXT("(revealing again)"), Overlay->IsRevealing());
			Overlay->Show(EAscensionTier::Tier2_Stellar, FeatRows[0]);
			TestFalse(TEXT("A presentation ends a reveal"), Overlay->IsRevealing());
			Overlay->Close();
			TestFalse(TEXT("Sight.Opened is no missing entry"), CosmicUI::Label(TEXT("Sight.Opened")).ToString().Contains(FStringTableEntry::GetPlaceholderSourceString()));
			Live.Civs[Ruled].Tier = EAscensionTier::Tier1_Planetary;
		}
	}

	// ---- B6b decisions 25, 26: what each rung's presentation wears - the fifth no glow, no gold, no chime ----
	{
		using CosmicAscensionOverlay::LookOf;
		for (const EAscensionTier Tier : { EAscensionTier::Tier2_Stellar, EAscensionTier::Tier3_Causal, EAscensionTier::Tier4_Transcendent })
		{
			const FCosmicAscensionLook Look = LookOf(Tier);
			const FString Rung = FString::Printf(TEXT("(rung %d)"), static_cast<int32>(Tier) + 1);
			TestTrue(TEXT("The second to fourth rungs wear the glow ") + Rung, Look.bGlow);
			TestTrue(TEXT("the chime ") + Rung, Look.bChime);
			TestTrue(TEXT("and the gold title ") + Rung, Look.TitleColor.Equals(CosmicPalette::Gold, 1e-4f));
			TestEqual(TEXT("Only the third rung's opens the sight ") + Rung, Look.bOpensSight, Tier == CosmicMapLook::SpiritualSightTier);
		}
		const FCosmicAscensionLook Fifth = LookOf(EAscensionTier::Tier5_EternalOrder);
		TestFalse(TEXT("The fifth rung's wears no glow"), Fifth.bGlow);
		TestFalse(TEXT("no chime"), Fifth.bChime);
		TestTrue(TEXT("and a white title"), Fifth.TitleColor.Equals(CosmicPalette::White, 1e-4f));
		TestFalse(TEXT("and opens no sight"), Fifth.bOpensSight);
		TestFalse(TEXT("(nor does the first rung's)"), LookOf(EAscensionTier::Tier1_Planetary).bOpensSight);

		// The fifth on the widget: the veil, the white title, the rung, the narrative and the pause stay; the glow and the
		// chime do not.
		const UTextBlock* Title = Overlay->GetTitleText();
		const UImage* Glow = Overlay->GetGlow();
		if (TestNotNull(TEXT("(the overlay has its title)"), Title) && TestNotNull(TEXT("(and its glow)"), Glow))
		{
			Controller->SetSpeed(2.f);
			const int32 ChimesBefore = Overlay->GetChimeCount();
			Overlay->Show(EAscensionTier::Tier5_EternalOrder, FeatRows[0]);
			TestTrue(TEXT("The fifth rung's presentation shows"), Overlay->IsShowing());
			TestEqual(TEXT("with the row's title"), Title->GetText().ToString(), Feat->Title.ToString());
			const FString Shown = ReadTexts(*Overlay);
			TestTrue(TEXT("its narrative"), Shown.Contains(Feat->MythicNarrative.ToString()));
			TestTrue(TEXT("and the fifth rung"), Shown.Contains(FString::Printf(TEXT("%s 5"), *CosmicUI::Label(TEXT("Panel.Tier")).ToString())));
			TestTrue(TEXT("The title is white"), Title->GetColorAndOpacity().GetSpecifiedColor().Equals(CosmicPalette::White, 1e-4f));
			TestTrue(TEXT("the glow collapsed"), Glow->GetVisibility() == ESlateVisibility::Collapsed);
			TestEqual(TEXT("and no chime played"), Overlay->GetChimeCount(), ChimesBefore);
			TestEqual(TEXT("The fifth rung's presentation pauses too"), Sim->GetTimeScale(), 0.f);
			Overlay->Advance(CosmicAscensionOverlay::Seconds + 0.1f);
			TestFalse(TEXT("and closes after five seconds"), Overlay->IsShowing());
			TestEqual(TEXT("giving the speed back"), Sim->GetTimeScale(), 2.f);
			Controller->SetSpeed(0.f);

			// The look comes back for the next rung's presentation.
			Overlay->Show(EAscensionTier::Tier5_EternalOrder, FeatRows[0]);
			Overlay->Show(EAscensionTier::Tier2_Stellar, FeatRows[0]);
			TestTrue(TEXT("A second rung's presentation after the fifth's has the gold title again"),
				Title->GetColorAndOpacity().GetSpecifiedColor().Equals(CosmicPalette::Gold, 1e-4f));
			TestTrue(TEXT("its glow"), Glow->GetVisibility() != ESlateVisibility::Collapsed);
			TestEqual(TEXT("and its chime"), Overlay->GetChimeCount(), ChimesBefore + 1);
			Overlay->Close();
		}

		// No reveal follows the fifth rung's presentation, over a spiritual map of a people on the fifth rung.
		FGalaxyState& Live = Sim->GetMutableState();
		const int32 Ruled = Sim->GetStateIndex().FindCiv(Live, Live.Player.RuledCivID);
		if (TestTrue(TEXT("(the player rules a people)"), Live.Civs.IsValidIndex(Ruled)))
		{
			Live.Civs[Ruled].Tier = EAscensionTier::Tier5_EternalOrder;
			Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
			TestTrue(TEXT("(the view drawn is spiritual)"), ACosmicPlayerController::ShownViewModeFor(*Overlay) == ECosmicViewMode::Spiritual);
			Overlay->Show(EAscensionTier::Tier5_EternalOrder, FeatRows[0]);
			Overlay->Close();
			TestFalse(TEXT("No reveal follows the fifth rung's presentation"), Overlay->IsRevealing());
			Live.Civs[Ruled].Tier = EAscensionTier::Tier1_Planetary;
		}
	}

	// ---- The glow's envelope and the chime ----
	{
		using CosmicAscensionOverlay::GlowAt;
		TestEqual(TEXT("The glow starts at 0"), GlowAt(0.f), 0.f);
		TestEqual(TEXT("is half lit at 0.4 s"), GlowAt(0.4f), 0.5f, 1e-4f);
		TestEqual(TEXT("full at 0.8 s"), GlowAt(0.8f), 1.f, 1e-4f);
		TestEqual(TEXT("full at 2.5 s"), GlowAt(2.5f), 1.f);
		TestEqual(TEXT("half gone at 4.6 s"), GlowAt(4.6f), 0.5f, 1e-4f);
		TestEqual(TEXT("out at 5 s"), GlowAt(5.f), 0.f);
		TestEqual(TEXT("and after"), GlowAt(6.f), 0.f);

		const TArray<int16> Chime = CosmicAscensionSound::MakeChime(44100, 3.f);
		TestEqual(TEXT("The chime is three seconds long"), Chime.Num(), 132300);
		int32 Peak = 0;
		double Early = 0.0;
		double Late = 0.0;
		for (int32 Index = 0; Index < Chime.Num(); ++Index)
		{
			Peak = FMath::Max(Peak, FMath::Abs(static_cast<int32>(Chime[Index])));
			if (Index < 22050)
			{
				Early += FMath::Square(static_cast<double>(Chime[Index]));
			}
			else if (Index >= Chime.Num() - 11025)
			{
				Late += FMath::Square(static_cast<double>(Chime[Index]));
			}
		}
		TestTrue(TEXT("leaves headroom"), Peak <= 0.25 * 32767 + 1);
		TestTrue(TEXT("is not silent"), Peak > 1000);
		int32 FirstMillisecond = 0;
		for (int32 Index = 0; Index < 44 && Index < Chime.Num(); ++Index)
		{
			FirstMillisecond = FMath::Max(FirstMillisecond, FMath::Abs(static_cast<int32>(Chime[Index])));
		}
		TestTrue(TEXT("rises from silence: its first millisecond stays under a tenth of its peak"), FirstMillisecond * 10 < Peak);
		TestTrue(TEXT("and fades: its last quarter second is far quieter than its first half second"), Late / 11025.0 < (Early / 22050.0) / 10.0);
		TestEqual(TEXT("No samples for no time"), CosmicAscensionSound::MakeChime(44100, 0.f).Num(), 0);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

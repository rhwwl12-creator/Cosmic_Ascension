// Cosmic Ascension - automation tests of the calling's pages (B5, commit [B-5c]): the three pages of a death (the epitaph, the
// digest, the inauguration) through the HUD's page widget and the controller's flow - the epitaph's 'next' runs the leap -, the
// last answers and the one of the death set apart, the second death's 'skip', a loaded interregnum's epitaph, an end inside
// the interregnum and on the death's tick, and the reign end of an abdication and of a deposition (no epitaph's word); no
// speed, no wheel and no give-back under a page, the silence after a leap, a succession on the end's tick, a leap of 0, a
// load on a succession's tick, and the cards the epitaph leaves out.
// Design: docs/60_DECISIONS/headers/B5_death_succession_two_books.md section 4, 2.6, 2.10, 3.6, 6 (UI.Epitaph).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/PanelWidget.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Core/CosmicIDs.h"
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Dom/JsonObject.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Events/CosmicPetitions.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/Text.h"
#include "Log/CosmicChronicle.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicAscensionOverlayWidget.h"
#include "UI/CosmicCallingPagesWidget.h"
#include "UI/CosmicChronicleWidget.h"
#include "UI/CosmicPetitionCardWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicAutoPause.h"
#include "World/CosmicCameraPawn.h"
#include "World/CosmicDigest.h"
#include "World/CosmicGalaxyMapActor.h"
#include "World/CosmicGameMode.h"
#include "World/CosmicMapLook.h"
#include "World/CosmicZoomLevels.h"

namespace CosmicCallingPagesTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	FString Words(const TCHAR* Key)
	{
		return CosmicUI::Label(Key).ToString();
	}

	FString Shown(const UTextBlock* Text)
	{
		return (Text && Text->GetVisibility() != ESlateVisibility::Collapsed) ? Text->GetText().ToString() : FString();
	}

	bool IsShownWidget(const UWidget* Widget)
	{
		const ESlateVisibility Visibility = Widget ? Widget->GetVisibility() : ESlateVisibility::Collapsed;
		return Visibility != ESlateVisibility::Collapsed && Visibility != ESlateVisibility::Hidden;
	}

	// The source row of a test's death: a hazard row of the game (no row kills yet - data off), and the cards of the answers.
	const FName DeathRow(TEXT("crisis_pre_supernova"));
	const FName ClaimCard(TEXT("petition_pressure_claim"));
	const FName WatchCard(TEXT("enc_star_watch"));
	const FName WallsCard(TEXT("petition_walls"));

	// The game's rules with a leap of exactly Ticks (one number either way - decision 3).
	FCosmicRulesDef LeapOf(const FCosmicRulesDef& Base, int32 Ticks)
	{
		FCosmicRulesDef Rules = Base;
		Rules.LeapTicksMin = Ticks;
		Rules.LeapTicksMax = Ticks;
		return Rules;
	}

	// A death recorded on the live state as Apply records one: the tenth stage of the next tick carries it out.
	void RecordDeath(FGalaxyState& State)
	{
		State.Calling.PendingEffect = CosmicTags::EffectKillPlayer();
		State.Calling.PendingCause = Tag(TEXT("Death.Hazard"));
		State.Calling.PendingSourceRow = DeathRow;
	}

	// A new galaxy of the home row (seed 42; its vessel rules), paused, at tick At - 1: the next StepOnce is the tick At.
	FGalaxyState& LayAt(UGalaxySimulationSubsystem& Sim, int64 At)
	{
		Sim.NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		Sim.SetTimeScale(0.f);
		FGalaxyState& State = Sim.GetMutableState();
		State.Tick = At - 1;
		Sim.SyncRngState();
		return State;
	}

	// An answer's outcome line as UPlayerRoleSubsystem::AnswerPetition stores it: Log.Civ, Severity 1, the card its source.
	void AddAnswerLine(FGalaxyState& State, int64 Tick, FName Card, FName SystemID, FName CivID)
	{
		FCosmicLogEntry& Entry = State.Log.AddDefaulted_GetRef();
		Entry.Tick = Tick;
		Entry.Category = Tag(TEXT("Log.Civ"));
		Entry.Severity = 1;
		Entry.Message = FText::FromString(FString::Printf(TEXT("(the answer to %s)"), *Card.ToString()));
		Entry.FocusSystemID = SystemID;
		Entry.FocusCivID = CivID;
		Entry.SourceRow = Card;
	}

	// The fixed words of a label: its text cut at every slot, the pieces of two characters or more, trimmed.
	TArray<FString> FixedWordsOf(const TCHAR* Key)
	{
		TArray<FString> Pieces;
		FString Piece;
		int32 Depth = 0;
		for (const TCHAR Char : Words(Key))
		{
			if (Char == TEXT('{'))
			{
				++Depth;
			}
			if (Depth == 0)
			{
				Piece.AppendChar(Char);
			}
			if (Char == TEXT('}') && Depth > 0)
			{
				--Depth;
			}
			if (Depth > 0 && !Piece.IsEmpty())
			{
				Pieces.Add(Piece.TrimStartAndEnd());
				Piece.Reset();
			}
		}
		Pieces.Add(Piece.TrimStartAndEnd());
		return Pieces.FilterByPredicate([](const FString& Each) { return Each.Len() >= 2; });
	}

	struct FScreen
	{
		UGalaxySimulationSubsystem* Sim = nullptr;
		ACosmicPlayerController* Controller = nullptr;
		ACosmicHUD* HUD = nullptr;
		UCosmicCallingPagesWidget* Pages = nullptr;
		UCosmicChronicleWidget* Chronicle = nullptr;
		UCosmicTopBarWidget* Bar = nullptr;
		UCosmicPetitionCardWidget* Card = nullptr;
		UCosmicAscensionOverlayWidget* Overlay = nullptr;
		ACosmicGalaxyMapActor* Map = nullptr;
	};

	// The live controller, HUD and widgets (UI.RunEnd's form); false (with a test error) when one is missing.
	bool Acquire(FAutomationTestBase& Test, FScreen& Out)
	{
		Out.Sim = CosmicTests::AcquireSim(Test);
		if (!Out.Sim)
		{
			return false;
		}
		if (!FSlateApplication::IsInitialized())
		{
			Test.AddError(TEXT("No Slate application: run the automation tests with -game (see CLAUDE.md)."));
			return false;
		}
		UWorld* World = Out.Sim->GetGameInstance()->GetWorld();
		Out.Controller = Cast<ACosmicPlayerController>(World ? World->GetFirstPlayerController() : nullptr);
		Out.HUD = Out.Controller ? Out.Controller->GetHUD<ACosmicHUD>() : nullptr;
		Out.Pages = Out.HUD ? Out.HUD->GetCallingPages() : nullptr;
		Out.Chronicle = Out.HUD ? Out.HUD->GetChronicle() : nullptr;
		Out.Bar = Out.HUD ? Out.HUD->GetTopBar() : nullptr;
		Out.Card = Out.HUD ? Out.HUD->GetPetitionCard() : nullptr;
		Out.Overlay = Out.HUD ? Out.HUD->GetAscensionOverlay() : nullptr;
		if (World)
		{
			for (TActorIterator<ACosmicGalaxyMapActor> It(World); It; ++It)
			{
				Out.Map = *It;
			}
		}
		return Test.TestNotNull(TEXT("The HUD has its calling pages"), Out.Pages) && Test.TestNotNull(TEXT("and its chronicle"), Out.Chronicle)
			&& Test.TestNotNull(TEXT("and its top bar"), Out.Bar) && Test.TestNotNull(TEXT("and its petition card"), Out.Card)
			&& Test.TestNotNull(TEXT("and its ascension presentation"), Out.Overlay) && Test.TestNotNull(TEXT("The map stands"), Out.Map);
	}

	// Two lists of lines say the same, in order.
	bool SameLines(const TArray<FText>& A, const TArray<FText>& B)
	{
		bool bSame = A.Num() == B.Num();
		for (int32 At = 0; bSame && At < A.Num(); ++At)
		{
			bSame = A[At].ToString() == B[At].ToString();
		}
		return bSame;
	}

	// The silence ahead (UI.LeapBoundary's form - Tests/CosmicLeapTests.cpp): every system but the vessel's and the sanctuary
	// leaves the vessel's galaxy row, so that no candidate stands at the leap's end.
	void LeaveNoCandidate(FGalaxyState& State, const FCosmicStateIndex& Index)
	{
		const int32 Here = Index.FindSystem(State, State.Player.CurrentSystemID);
		for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num(); ++SystemIndex)
		{
			if (SystemIndex != Here && !CosmicState::IsSanctuary(State.Systems[SystemIndex].SystemID))
			{
				State.Systems[SystemIndex].GalaxyRow = FName(TEXT("galaxy_test_apart"));
			}
		}
	}

	// The calling waits in the silence: in the interregnum with no leap left.
	bool IsSilent(const FGalaxyState& State)
	{
		return State.Calling.LifeState == ELifeState::Interregnum && State.Calling.PendingLeapTicks == 0;
	}

	// Tools/data/schema.json's epitaph_leaves_out, sorted (empty when it cannot be read).
	TArray<FString> SchemaLeftOut()
	{
		TArray<FString> Listed;
		FString Text;
		TSharedPtr<FJsonObject> Schema;
		if (FFileHelper::LoadFileToString(Text, *(FPaths::ProjectDir() / TEXT("Tools/data/schema.json"))))
		{
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
			if (FJsonSerializer::Deserialize(Reader, Schema) && Schema.IsValid())
			{
				Schema->TryGetStringArrayField(TEXT("epitaph_leaves_out"), Listed);
			}
		}
		Listed.Sort();
		return Listed;
	}

	// The pages' row of buttons shows (the parent of 'next' - shown once every line shows).
	bool ButtonsShown(const UCosmicCallingPagesWidget& Pages)
	{
		return Pages.GetNextButton() && IsShownWidget(Pages.GetNextButton()->GetParent());
	}

	// Every shown speed button of the bar is greyed out (after a redraw).
	bool AllSpeedsGrey(UCosmicTopBarWidget& Bar)
	{
		Bar.Refresh();
		bool bAny = false;
		bool bAllGrey = true;
		for (const UCosmicActionButton* Button : Bar.GetSpeedButtons())
		{
			if (Button && IsShownWidget(Button))
			{
				bAny = true;
				bAllGrey &= !Button->GetIsEnabled();
			}
		}
		return bAny && bAllGrey;
	}
}

// The calling's pages (header section 4, 2.6, 2.10, 3.6; section 6 UI.Epitaph): a ruler's death opens the epitaph at its
// boundary - the card on the screen folded, the time stopped, the speed buttons greyed and the keys refused - with its title,
// cause and source, cycles lived, rung and last answers (the first contact card's title, whose {OtherCiv} its line cannot say,
// left out; the card of the very hazard that killed set apart below the others, never with the cause); its 'next' runs the
// leap to the succession (the same state as the controller's AdvanceLeap) and opens the digest (CosmicDigest's lines), whose
// 'next' opens the inauguration (the new vessel's facts and the fixed line), whose close puts the camera on the new vessel's
// system with the time stopped and its line. A second death has 'skip', which goes straight through. A load in the
// interregnum shows its epitaph again at once. An end inside the interregnum: epitaph -> digest -> chronicle, the chronicle
// waiting for the pages, its cause note saying the end's tick. A death on the end's tick: epitaph -> chronicle. An abdication
// and a deposition open the reign end - no Epitaph. key, none of the epitaph's words, the time stopped with its own line; a
// role set by hand (no reign closed) opens nothing, a reign end opens once, and a reign taken between ticks and left before the
// next boundary opens it too. Under a page no speed is set, the pause key keeps the page's reason and the wheel does not move
// the camera; a showing ascension presentation folds at a death or a reign end with no give-back. The silence after a leap
// runs on to the end (switch on) or gives the time back as the digest folds (switch off). A succession on the end's tick
// opens no inauguration ('next' and 'skip'). A leap of 0 goes epitaph -> one line -> inauguration. An outcome on the card
// folds at a death. A load on a succession's tick shows its pages. The epitaph leaves out the cards that speak to the reader
// (the schema's list), sets apart only the killing hazard's card answered where the vessel died, and names a card's death by
// its hazard, never by the card's title.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIEpitaphTest, "Cosmic_Ascension.UI.Epitaph", CosmicTests::Flags)
bool FCosmicUIEpitaphTest::RunTest(const FString& Parameters)
{
	using namespace CosmicCallingPagesTests;
	using CosmicUI::Label;

	FScreen Screen;
	if (!Acquire(*this, Screen))
	{
		return false;
	}
	UGalaxySimulationSubsystem* Sim = Screen.Sim;
	ACosmicPlayerController* Controller = Screen.Controller;
	UCosmicCallingPagesWidget* Pages = Screen.Pages;
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef GameRules = Sim->GetRules();
	const bool bViewerChoice = Controller->IsAutoPauseOn();
	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	ON_SCOPE_EXIT
	{
		Controller->CosmicSeeAll(0);
		Controller->SetAutoPause(bViewerChoice);
		Controller->SetProvisionalEndEnabled(true);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Controller->SetAutoPause(false);
	Sim->SetBroadcastEnabled(true);
	Sim->SetTables(CosmicTests::WithRules(Original, LeapOf(GameRules, 40)));
	const FCosmicDataTables& Tables = Sim->GetTables();
	const FEncounterDef* FirstContactRow = Tables.FindEncounter(CosmicRows::PetitionFirstContact);
	const FEncounterDef* ClaimRow = Tables.FindEncounter(ClaimCard);
	const FEncounterDef* WatchRow = Tables.FindEncounter(WatchCard);
	const FHazardDef* Nova = Tables.FindHazard(DeathRow);
	if (!TestTrue(TEXT("(the game has the three cards and the hazard of the test's death)"), FirstContactRow && ClaimRow && WatchRow && Nova))
	{
		return false;
	}
	TestTrue(TEXT("(the star watch is the card of the hazard of the test's death)"), CosmicCallingPages::IsCardOfHazard(*WatchRow, *Nova));
	TestFalse(TEXT("(the claim's card is not)"), CosmicCallingPages::IsCardOfHazard(*ClaimRow, *Nova));
	{
		// The cards the epitaph leaves out are the schema's list, which the data checker holds every card title that speaks to the
		// reader to (Tools/cosmic_data_schema.py - header section 4, decision 12-da).
		TArray<FString> Code;
		for (const FName& Row : CosmicRows::EpitaphLeavesOut)
		{
			Code.Add(Row.ToString());
		}
		Code.Sort();
		const TArray<FString> Listed = SchemaLeftOut();
		TestTrue(TEXT("The epitaph's left-out cards are Tools/data/schema.json's epitaph_leaves_out"), !Listed.IsEmpty() && Listed == Code);
		TestTrue(TEXT("(the exaltation card and the throne card's question among them)"), Code.Contains(CosmicRows::PetitionExalt.ToString())
			&& Code.Contains(CosmicRows::PetitionKeepThrone.ToString()) && Tables.FindEncounter(CosmicRows::PetitionExalt) && Tables.FindEncounter(CosmicRows::PetitionKeepThrone));
	}
	{
		const TCHAR* SuccessionKey = CosmicAutoPause::WhyKeyOf(ECosmicAutoPause::Succession);
		const TCHAR* ReignKey = CosmicAutoPause::WhyKeyOf(ECosmicAutoPause::ReignEnd);
		TestTrue(TEXT("The pages' two reasons of a stopped time have their own lines"), SuccessionKey && ReignKey
			&& FString(SuccessionKey) == TEXT("Succession.Stopped") && FString(ReignKey) == TEXT("Reign.Stopped"));
		const FString& Placeholder = FStringTableEntry::GetPlaceholderSourceString();
		TestTrue(TEXT("with words in the String Table"), !Words(TEXT("Succession.Stopped")).IsEmpty() && !Words(TEXT("Reign.Stopped")).IsEmpty()
			&& !Words(TEXT("Succession.Stopped")).Contains(Placeholder) && !Words(TEXT("Reign.Stopped")).Contains(Placeholder));
	}

	// A ruler's death at tick 400 (its leap 40): three answered cards before it, and a card on the screen as it comes.
	bool bCardBefore = false;
	FText ClaimSystem;
	auto LayDeath = [&]() -> FGalaxyState&
	{
		FGalaxyState& Live = LayAt(*Sim, 400);
		const FCosmicStateIndex& Index = Sim->GetStateIndex();
		const int32 Ruled = CosmicPetitions::RuledCivIndexOf(Live, Index);
		const int32 Home = Live.Civs.IsValidIndex(Ruled) ? Index.FindSystem(Live, Live.Civs[Ruled].HomeSystemID) : INDEX_NONE;
		if (Live.Systems.IsValidIndex(Home))
		{
			const FName CivID = Live.Civs[Ruled].CivID;
			Live.Calling.Lives[0].LastChoiceRows = { CosmicRows::PetitionFirstContact, ClaimCard, WatchCard };
			AddAnswerLine(Live, 390, CosmicRows::PetitionFirstContact, NAME_None, CivID);
			AddAnswerLine(Live, 395, ClaimCard, Live.Systems[Home].SystemID, CivID);
			ClaimSystem = Live.Systems[Home].DisplayName;
			AddAnswerLine(Live, 398, WatchCard, Live.Player.CurrentSystemID, CivID);
		}
		Live.Knowing.PendingPetitionRow = WallsCard;
		Live.Knowing.PendingPetitionSystemID = NAME_None;
		Controller->FollowPetition(false);
		bCardBefore = Screen.Card->GetShownRow() == WallsCard;
		RecordDeath(Live);
		Controller->SetSpeed(1.f);
		Sim->StepOnce();
		return Live;
	};

	// ---- The leap the pages run is the controller's AdvanceLeap ----
	LayDeath();
	TestTrue(TEXT("(the controller's leap runs)"), Controller->AdvanceLeap());
	const uint32 LeapHash = Sim->ComputeStateHash();
	const uint32 LeapWorld = Sim->ComputeWorldHash();

	// ---- (1) The epitaph ----
	FGalaxyState& Live = LayDeath();
	Screen.Map->RefreshNow();
	if (!TestTrue(TEXT("(the vessel died at tick 400 with its leap of 40 ahead)"), Live.Tick == 400 && Live.Calling.LifeState == ELifeState::Interregnum
		&& Live.Calling.PendingLeapTicks == 40 && Live.Calling.Lives.Num() == 1))
	{
		return false;
	}
	TestTrue(TEXT("(a card showed before the death)"), bCardBefore);
	TestTrue(TEXT("The death's boundary opens the epitaph of the life that closed"), Pages->GetPage() == ECosmicCallingPage::Epitaph && Pages->GetLifeAt() == 0);
	TestTrue(TEXT("with its reveal"), Pages->IsRevealing());
	TestTrue(TEXT("The card on the screen folds"), Screen.Card->GetShownRow().IsNone() && Screen.Card->GetVisibility() == ESlateVisibility::Collapsed);
	TestEqual(TEXT("The time stands still"), Sim->GetTimeScale(), 0.f);
	TestTrue(TEXT("for the death, the automatic pause off"), Controller->GetAutoPauseReason() == ECosmicAutoPause::Death);
	TestTrue(TEXT("While a page shows every speed button is greyed out"), AllSpeedsGrey(*Screen.Bar));
	Controller->PressSpeedKey(EKeys::One);
	TestEqual(TEXT("and a number key does nothing"), Sim->GetTimeScale(), 0.f);
	Controller->PressSpeedKey(EKeys::SpaceBar);
	TestEqual(TEXT("nor does Space"), Sim->GetTimeScale(), 0.f);
	Controller->PressSpeedKey(EKeys::Zero);
	TestTrue(TEXT("and the pause key leaves the page's reason"), Controller->GetAutoPauseReason() == ECosmicAutoPause::Death);
	// Every give-back goes through the controller's one speed path: under a page it sets no speed (the presentation's, the
	// card's and the silence's give-back wait for the pages - header section 4, decision 16).
	Controller->SetSpeed(1.f);
	TestTrue(TEXT("A speed set while a page shows is refused, the page's reason kept"), Sim->GetTimeScale() == 0.f
		&& Controller->GetAutoPauseReason() == ECosmicAutoPause::Death);
	Controller->SetSpeed(0.f);
	TestTrue(TEXT("and so is a plain stop's empty reason"), Controller->GetAutoPauseReason() == ECosmicAutoPause::Death);
	ACosmicCameraPawn* CameraPawn = Cast<ACosmicCameraPawn>(Controller->GetPawn());
	if (TestNotNull(TEXT("(the camera pawn)"), CameraPawn))
	{
		const float Arm = CameraPawn->GetTargetArmLength();
		const ECosmicZoomLevel Level = Controller->GetZoomLevel();
		Controller->HandleWheel(EKeys::MouseScrollUp);
		const bool bUpStill = CameraPawn->GetTargetArmLength() == Arm;
		Controller->HandleWheel(EKeys::MouseScrollDown);
		TestTrue(TEXT("The wheel does not move the camera under a page"), bUpStill && CameraPawn->GetTargetArmLength() == Arm && Controller->GetZoomLevel() == Level);
	}
	Pages->PressNext();
	TestEqual(TEXT("'Next' takes no press while the lines show"), Live.Tick, static_cast<int64>(400));
	TestTrue(TEXT("The buttons wait for the lines"), Pages->IsRevealing() && !ButtonsShown(*Pages));
	Pages->PressVeil();
	TestTrue(TEXT("A press on the veil shows every line and the buttons"), !Pages->IsRevealing() && ButtonsShown(*Pages));
	TestFalse(TEXT("A first death has no 'skip'"), IsShownWidget(Pages->GetSkipButton()) || Pages->GetPageKeys().Contains(TEXT("Epitaph.Skip")));
	{
		const FLifeRecord& Dead = Live.Calling.Lives[0];
		const TArray<FText> Texts = Pages->GetPageTexts();
		const FText Nth = FText::Format(Label(TEXT("Chronicle.Life")), FText::AsNumber(1));
		const FText Name = CosmicChronicle::VesselNameOf(Dead, Tables);
		const int32 Cause = Pages->GetCauseRow();
		if (TestTrue(TEXT("(the page has its title, its facts and its answers)"), Texts.Num() >= 6 && Texts.IsValidIndex(Cause + 2)))
		{
			TestEqual(TEXT("The title is 'the Nth' with the vessel's name"), Texts[0].ToString(), (Name.IsEmpty() ? Nth : FText::Format(Label(TEXT("List.Pair")), Nth, Name)).ToString());
			FFormatNamedArguments CauseArgs;
			CauseArgs.Add(TEXT("Cause"), Label(TEXT("Epitaph.Cause.Hazard")));
			CauseArgs.Add(TEXT("Source"), Nova->DisplayName);
			TestEqual(TEXT("then the cause with the hazard's name"), Texts[Cause].ToString(), FText::Format(Label(TEXT("Epitaph.CauseFrom")), CauseArgs).ToString());
			TestEqual(TEXT("the cycles lived"), Texts[Cause + 1].ToString(), FText::Format(Label(TEXT("Epitaph.Lived")), FText::AsNumber(400)).ToString());
			TestEqual(TEXT("the highest rung"), Texts[Cause + 2].ToString(), FText::Format(Label(TEXT("Epitaph.Peak")), FText::AsNumber(static_cast<int32>(Dead.PeakTier) + 1)).ToString());
		}
		// The last answers: the first contact card's title is left out ({OtherCiv} - its line does not say the other people).
		const TArray<FCosmicAnswerTitle>& Answers = Pages->GetAnswers();
		if (TestEqual(TEXT("Two of the three answers have a title the page can say"), Answers.Num(), 2))
		{
			FFormatNamedArguments ClaimArgs;
			ClaimArgs.Add(TEXT("System"), ClaimSystem);
			TestTrue(TEXT("in time order: the claim's card first"), Answers[0].Row == ClaimCard && !Answers[0].bOfTheDeath);
			TestEqual(TEXT("its {System} filled from its line"), Answers[0].Title.ToString(), FText::Format(ClaimRow->Title, ClaimArgs).ToString());
			TestTrue(TEXT("then the star watch, the card of the hazard that killed"), Answers[1].Row == WatchCard && Answers[1].bOfTheDeath);
			TestEqual(TEXT("whose title is the card's"), Answers[1].Title.ToString(), WatchRow->Title.ToString());
		}
		TestFalse(TEXT("No answer is the first contact card's"), Answers.ContainsByPredicate([](const FCosmicAnswerTitle& Each) { return Each.Row == CosmicRows::PetitionFirstContact; }));
		// Set apart: below every fact and below the other answers, a line of its own - never in the cause's line.
		const int32 Apart = Pages->GetApartRow();
		if (TestTrue(TEXT("The answer of the death stands on a row of its own"), Texts.IsValidIndex(Apart)))
		{
			TestTrue(TEXT("below the facts"), Apart > Pages->GetFactsEndRow() && Pages->GetFactsEndRow() >= Cause);
			TestEqual(TEXT("last on the page"), Apart, Texts.Num() - 1);
			TestEqual(TEXT("its title alone"), Texts[Apart].ToString(), WatchRow->Title.ToString());
			TestFalse(TEXT("and the cause's line does not hold it"), Texts.IsValidIndex(Cause) && Texts[Cause].ToString().Contains(WatchRow->Title.ToString()));
			TestTrue(TEXT("The claim's title stands with the answers above it"), !Answers.IsEmpty() && Texts.IsValidIndex(Apart - 1)
				&& Texts[Apart - 1].ToString() == Answers[0].Title.ToString());
		}
		TestTrue(TEXT("The epitaph's words are its keys"), Pages->GetPageKeys().Contains(TEXT("Chronicle.Life")) && Pages->GetPageKeys().Contains(TEXT("Epitaph.Next")));
		// The page reads and writes nothing: built again, both hashes stay.
		const uint32 Before = Sim->ComputeStateHash();
		const uint32 BeforeWorld = Sim->ComputeWorldHash();
		Pages->OpenEpitaph(0, false, true);
		TestTrue(TEXT("Building the epitaph again moves neither hash"), Sim->ComputeStateHash() == Before && Sim->ComputeWorldHash() == BeforeWorld);
		TestTrue(TEXT("and gives the same page"), Pages->GetPageTexts().Num() == Texts.Num() && !Pages->IsRevealing());
	}

	// ---- (2) 'Next': the leap, then the digest ----
	Pages->PressNext();
	TestEqual(TEXT("The epitaph's 'next' runs the leap: the tick moves to the succession"), Live.Tick, static_cast<int64>(440));
	TestTrue(TEXT("where a new vessel stands"), Live.Calling.LifeState == ELifeState::Embodied && Live.Calling.Lives.Num() == 2 && Live.Calling.Lives[1].BornTick == 440);
	TestTrue(TEXT("The pages' leap is the controller's (the same state)"), Sim->ComputeStateHash() == LeapHash && Sim->ComputeWorldHash() == LeapWorld);
	// The state at the succession's boundary, for the load on that tick (case 14).
	const FGalaxyState AtSuccession = Sim->GetState();
	TestTrue(TEXT("Then the digest opens"), Pages->GetPage() == ECosmicCallingPage::Digest);
	TestTrue(TEXT("with the digest's lines of the life that closed"), SameLines(Pages->GetPageTexts(),
		CosmicDigest::LinesOf(*Sim, CosmicDigest::LastClosedLifeOf(Live), Controller->IsSeeingAll())));
	{
		// The page asks the digest with the controller's see-all (the development aid CosmicSeeAll): a people the view does not
		// show rose a rung in the span - under CosmicSeeAll an item, without it a change out of knowledge.
		const FCosmicKnownView View = Controller->GetKnownView();
		int32 Unshown = INDEX_NONE;
		for (int32 CivIndex = 0; CivIndex < Live.Civs.Num() && Unshown == INDEX_NONE; ++CivIndex)
		{
			Unshown = View.ShownOrderOf(CivIndex) == INDEX_NONE ? CivIndex : INDEX_NONE;
		}
		const TArray<FName> Feats = Tables.GetFeatRowNames();
		const int32 Closed = CosmicDigest::LastClosedLifeOf(Live);
		if (TestTrue(TEXT("(a people the viewer is not shown, and a feat row)"), Unshown != INDEX_NONE && !Feats.IsEmpty()))
		{
			FCosmicLogEntry& Rose = Live.Log.AddDefaulted_GetRef();
			Rose.Tick = 439;
			Rose.Category = Tag(TEXT("Log.Civ"));
			Rose.Severity = 1;
			Rose.Message = FText::FromString(TEXT("(a rung far away)"));
			Rose.FocusCivID = Live.Civs[Unshown].CivID;
			Rose.SourceRow = Feats[0];
			const TArray<FText> SeeAll = CosmicDigest::LinesOf(*Sim, Closed, true);
			const TArray<FText> Viewer = CosmicDigest::LinesOf(*Sim, Closed, false);
			TestFalse(TEXT("(the digest of the view that sees all differs from the viewer's)"), SameLines(SeeAll, Viewer));
			Controller->CosmicSeeAll(1);
			Pages->OpenDigest();
			TestTrue(TEXT("Under CosmicSeeAll the digest page is the digest of the view that sees all"), SameLines(Pages->GetPageTexts(), SeeAll));
			Controller->CosmicSeeAll(0);
			Pages->OpenDigest();
			TestTrue(TEXT("and without it the viewer's"), SameLines(Pages->GetPageTexts(), Viewer));
			Live.Log.Pop();
			Pages->OpenDigest();
		}
	}
	TestTrue(TEXT("The time still stands"), Sim->GetTimeScale() == 0.f && AllSpeedsGrey(*Screen.Bar));

	// ---- (3) The inauguration, and its close ----
	Pages->PressVeil();
	Pages->PressNext();
	if (TestTrue(TEXT("The digest's 'next' opens the inauguration of the new life"), Pages->GetPage() == ECosmicCallingPage::Inauguration && Pages->GetLifeAt() == 1))
	{
		const FLifeRecord& New = Live.Calling.Lives[1];
		const TArray<FText> Texts = Pages->GetPageTexts();
		TestTrue(TEXT("Its title is 'the Nth' of the new life"), !Texts.IsEmpty() && Texts[0].ToString() == FText::Format(Label(TEXT("Chronicle.Life")), FText::AsNumber(2)).ToString());
		const FString Facts = Texts.IsValidIndex(1) ? Texts[1].ToString() : FString();
		const FString NewName = CosmicChronicle::VesselNameOf(New, Tables).ToString();
		const int32 Born = Sim->GetStateIndex().FindSystem(Live, New.BirthSystemID);
		TestTrue(TEXT("its facts name the new vessel (third person)"), !NewName.IsEmpty() && Facts.Contains(NewName));
		TestTrue(TEXT("and its system"), Live.Systems.IsValidIndex(Born) && Facts.Contains(Live.Systems[Born].DisplayName.ToString()));
		TestTrue(TEXT("and its body"), Facts.Contains(FText::Format(INVTEXT("{0} {1}"), Label(TEXT("Petition.Stat.Body")), FText::AsNumber(Live.Player.Body)).ToString()));
		const int32 BornCiv = Sim->GetStateIndex().FindCiv(Live, New.BirthCivID);
		FFormatNamedArguments BornArgs;
		BornArgs.Add(TEXT("Civ"), Live.Civs.IsValidIndex(BornCiv) ? Live.Civs[BornCiv].CivName : FText::GetEmpty());
		TestTrue(TEXT("and the people it was born among, in the chronicle's words for a life"), Live.Civs.IsValidIndex(BornCiv)
			&& Facts.Contains(FText::Format(Label(TEXT("Chronicle.BornAmong")), BornArgs).ToString()) && Pages->GetPageKeys().Contains(TEXT("Chronicle.BornAmong")));
		TestTrue(TEXT("The fixed line closes the page"), !Texts.IsEmpty() && Texts.Last().ToString() == Words(TEXT("Succession.Kept")));
		TestFalse(TEXT("The inauguration says nothing of the epitaph"), Pages->GetPageKeys().ContainsByPredicate([](const FString& Key) { return Key.StartsWith(TEXT("Epitaph.")); }));
	}
	Pages->PressVeil();
	Pages->PressNext();
	TestFalse(TEXT("'To the map' folds the pages"), Pages->IsOpen());
	TestEqual(TEXT("The camera is on the new vessel's system"), Controller->GetSelectedSystem(), Live.Player.CurrentSystemID);
	TestEqual(TEXT("and the time still stands"), Sim->GetTimeScale(), 0.f);
	TestTrue(TEXT("for the succession"), Controller->GetAutoPauseReason() == ECosmicAutoPause::Succession);
	Screen.Bar->Refresh();
	TestEqual(TEXT("whose line the bar shows"), Shown(Screen.Bar->GetAutoPauseText()), Words(TEXT("Succession.Stopped")));
	Controller->PressSpeedKey(EKeys::One);
	TestEqual(TEXT("With the pages folded a key sets the speed again"), Sim->GetTimeScale(), 1.f);
	Controller->SetSpeed(0.f);
	if (CameraPawn)
	{
		// The control of case (1)'s wheel: with the pages folded a step of the wheel moves the camera (at a stop, the other way).
		const float Arm = CameraPawn->GetTargetArmLength();
		Controller->HandleWheel(EKeys::MouseScrollUp);
		const bool bMoved = CameraPawn->GetTargetArmLength() != Arm;
		if (!bMoved)
		{
			Controller->HandleWheel(EKeys::MouseScrollDown);
		}
		TestTrue(TEXT("(with the pages folded the wheel moves the camera)"), bMoved || CameraPawn->GetTargetArmLength() != Arm);
	}

	// ---- (4) A second death: 'skip' goes straight through ----
	RecordDeath(Live);
	Controller->SetSpeed(1.f);
	Sim->StepOnce();
	const FGalaxyState AtSecondDeath = Sim->GetState();
	if (TestTrue(TEXT("A second death opens its epitaph"), Pages->GetPage() == ECosmicCallingPage::Epitaph && Pages->GetLifeAt() == 1))
	{
		TestTrue(TEXT("with 'skip'"), Pages->GetPageKeys().Contains(TEXT("Epitaph.Skip")));
		Pages->PressSkip();
		TestEqual(TEXT("'Skip' takes no press while the lines show"), Live.Tick, static_cast<int64>(441));
		Pages->PressVeil();
		TestTrue(TEXT("(shown once the lines show)"), IsShownWidget(Pages->GetSkipButton()) && ButtonsShown(*Pages));
		Pages->PressSkip();
		TestEqual(TEXT("'Skip' runs the leap"), Live.Tick, static_cast<int64>(481));
		TestTrue(TEXT("to the succession"), Live.Calling.LifeState == ELifeState::Embodied && Live.Calling.Lives.Num() == 3);
		TestFalse(TEXT("and folds every page"), Pages->IsOpen());
		TestTrue(TEXT("as the inauguration's close does"), Controller->GetAutoPauseReason() == ECosmicAutoPause::Succession && Sim->GetTimeScale() == 0.f
			&& Controller->GetSelectedSystem() == Live.Player.CurrentSystemID);
	}

	// ---- (5) A load in the interregnum shows its epitaph again, at once, and stops a running time ----
	Controller->SetSpeed(1.f);
	TestEqual(TEXT("(the pages folded, the time runs before the load)"), Sim->GetTimeScale(), 1.f);
	Sim->LoadState(AtSecondDeath);
	TestTrue(TEXT("A load in the interregnum opens its epitaph"), Pages->GetPage() == ECosmicCallingPage::Epitaph && Pages->GetLifeAt() == 1);
	TestFalse(TEXT("fully shown"), Pages->IsRevealing());
	TestTrue(TEXT("with its 'skip'"), IsShownWidget(Pages->GetSkipButton()));
	TestEqual(TEXT("and stops the running time"), Sim->GetTimeScale(), 0.f);
	TestTrue(TEXT("for the death"), Controller->GetAutoPauseReason() == ECosmicAutoPause::Death);

	// ---- (6) An end inside the interregnum: epitaph -> digest -> chronicle ----
	const int64 EndTick = Sim->GetRules().ProvisionalEndTick;
	{
		FGalaxyState& Late = LayAt(*Sim, EndTick - 20);
		RecordDeath(Late);
		Controller->SetSpeed(1.f);
		Sim->StepOnce();
		TestTrue(TEXT("(a death twenty cycles before the end opens its epitaph)"), Pages->GetPage() == ECosmicCallingPage::Epitaph && !Controller->IsRunEnded());
		Pages->PressVeil();
		Pages->PressNext();
		TestEqual(TEXT("The leap stops at the end's tick"), Late.Tick, EndTick);
		TestTrue(TEXT("where the end latches"), Controller->GetRunEnd() == ECosmicProvisionalEnd::Clock);
		TestTrue(TEXT("and the digest opens"), Pages->GetPage() == ECosmicCallingPage::Digest);
		Controller->AdvanceRunEnd(CosmicChronicleScreen::HoldSeconds + 1.f);
		TestFalse(TEXT("The chronicle waits while a page shows"), Screen.Chronicle->IsOpen());
		Pages->PressVeil();
		Pages->PressNext();
		TestFalse(TEXT("The digest's 'next' folds the pages - no inauguration"), Pages->IsOpen());
		TestTrue(TEXT("(nobody was inaugurated: the calling still waits)"), Late.Calling.LifeState == ELifeState::Interregnum && Late.Calling.Lives.Num() == 1);
		TestTrue(TEXT("and opens the chronicle"), Screen.Chronicle->IsOpen());
		TestEqual(TEXT("whose cause note says the end's tick"), Shown(Screen.Chronicle->GetNoteText()),
			FText::Format(Label(TEXT("Chronicle.Note.Clock")), FText::AsNumber(EndTick)).ToString());
		Controller->LookAtMap();
	}

	// ---- (7) A death on the end's tick: epitaph -> chronicle ----
	{
		FGalaxyState& Last = LayAt(*Sim, EndTick);
		RecordDeath(Last);
		Controller->SetSpeed(1.f);
		Sim->StepOnce();
		TestTrue(TEXT("A death on the end's tick opens its epitaph"), Pages->GetPage() == ECosmicCallingPage::Epitaph && Controller->IsRunEnded());
		Controller->AdvanceRunEnd(CosmicChronicleScreen::HoldSeconds + 1.f);
		TestFalse(TEXT("The chronicle waits for it"), Screen.Chronicle->IsOpen());
		Pages->PressVeil();
		Pages->PressNext();
		TestFalse(TEXT("Its 'next' folds the pages - no digest"), Pages->IsOpen());
		TestTrue(TEXT("and opens the chronicle"), Screen.Chronicle->IsOpen());
		TestEqual(TEXT("with no leap run"), Last.Tick, EndTick);
		Controller->LookAtMap();
	}

	// ---- (8) The reign end: an abdication and a deposition, no epitaph's word ----
	TArray<FString> EpitaphWords;
	for (const TCHAR* Key : { TEXT("Epitaph.Cause"), TEXT("Epitaph.CauseFrom"), TEXT("Epitaph.Lived"), TEXT("Epitaph.Peak"), TEXT("Epitaph.Answers"),
		TEXT("Epitaph.Cause.Hazard"), TEXT("AutoPause.Why.Death") })
	{
		EpitaphWords.Append(FixedWordsOf(Key));
	}
	TestTrue(TEXT("(the epitaph's fixed words are read)"), EpitaphWords.Num() >= 5);
	auto NoDeathOnPage = [&](const TCHAR* Case)
	{
		TestFalse(*FString::Printf(TEXT("%s: no Epitaph. key on the page"), Case),
			Pages->GetPageKeys().ContainsByPredicate([](const FString& Key) { return Key.StartsWith(TEXT("Epitaph.")) || Key == TEXT("Chronicle.Life"); }));
		bool bDeathWord = false;
		for (const FText& Text : Pages->GetPageTexts())
		{
			for (const FString& Word : EpitaphWords)
			{
				bDeathWord |= Text.ToString().Contains(Word);
			}
		}
		TestFalse(*FString::Printf(TEXT("%s: none of the epitaph's words"), Case), bDeathWord);
	};
	{
		FGalaxyState& Ruling = LayAt(*Sim, 400);
		const int32 Ruled = CosmicPetitions::RuledCivIndexOf(Ruling, Sim->GetStateIndex());
		if (TestTrue(TEXT("(the vessel rules and its abdication is recorded)"), Ruled != INDEX_NONE && Controller->Abdicate()))
		{
			const FText CivName = Ruling.Civs[Ruled].CivName;
			Controller->SetSpeed(1.f);
			Sim->StepOnce();
			TestTrue(TEXT("(the vessel walks in the same life)"), Ruling.Player.Role == EPlayerRole::Wanderer && Ruling.Calling.Lives.Num() == 1
				&& Ruling.Calling.LifeState == ELifeState::Embodied);
			TestTrue(TEXT("An abdication opens the reign end's page"), Pages->GetPage() == ECosmicCallingPage::ReignEnd && Pages->GetLifeAt() == 0);
			const TArray<FText> Texts = Pages->GetPageTexts();
			if (TestEqual(TEXT("(the title, the reign's cycles and why it ended)"), Texts.Num(), 3))
			{
				const FLifeRecord& Life = Ruling.Calling.Lives[0];
				TestEqual(TEXT("The title"), Texts[0].ToString(), Words(TEXT("Reign.Title")));
				TestEqual(TEXT("the people and the cycles of its reign"), Texts[1].ToString(),
					CosmicUI::JoinList({ CivName, FText::Format(Label(TEXT("Reign.Ticks")), FText::AsNumber(Life.ReignTicks.IsEmpty() ? 0 : Life.ReignTicks.Last())) }).ToString());
				TestEqual(TEXT("and the abdication"), Texts[2].ToString(), Words(TEXT("Reign.End.Abdicated")));
			}
			NoDeathOnPage(TEXT("The abdication"));
			TestTrue(TEXT("The time stands still with the reign end's line"), Sim->GetTimeScale() == 0.f && Controller->GetAutoPauseReason() == ECosmicAutoPause::ReignEnd);
			TestTrue(TEXT("and the speed buttons are greyed out"), AllSpeedsGrey(*Screen.Bar));
			Pages->PressVeil();
			Pages->PressNext();
			TestFalse(TEXT("'To the map' folds it"), Pages->IsOpen());
			TestTrue(TEXT("over the stopped time"), Sim->GetTimeScale() == 0.f && Controller->GetAutoPauseReason() == ECosmicAutoPause::ReignEnd);
			Screen.Bar->Refresh();
			TestEqual(TEXT("whose line the bar shows"), Shown(Screen.Bar->GetAutoPauseText()), Words(TEXT("Reign.Stopped")));
			// The page opens once: the walking vessel's next boundary compares with the walk it saw (FollowReign's bWasRuling).
			Controller->SetSpeed(1.f);
			Sim->StepOnce();
			TestFalse(TEXT("A reign end's page opens once: the next boundary of the walking vessel opens nothing"), Pages->IsOpen());
			Controller->SetSpeed(0.f);
			// A reign taken between ticks is noted (TakeThrone): left again before any boundary, its end still opens the page.
			for (FCivilizationState& Civ : Ruling.Civs)
			{
				Civ.ThroneLockedUntilTick = 0;
			}
			Ruling.Knowing.PendingPetitionRow = NAME_None;
			if (TestTrue(TEXT("(the vessel returns to the throne it left, between ticks)"), Controller->TakeThrone(Ruling.Player.FormerCivID)
				&& Ruling.Player.Role == EPlayerRole::Ruler))
			{
				TestTrue(TEXT("(and abdicates again before any boundary)"), Controller->Abdicate());
				Controller->SetSpeed(1.f);
				Sim->StepOnce();
				TestTrue(TEXT("A reign taken and left between two boundaries opens the reign end's page"), Pages->GetPage() == ECosmicCallingPage::ReignEnd
					&& Controller->GetAutoPauseReason() == ECosmicAutoPause::ReignEnd && Sim->GetTimeScale() == 0.f);
				Pages->PressVeil();
				Pages->PressNext();
			}
		}
	}
	{
		FGalaxyState& Ruling = LayAt(*Sim, 400);
		Ruling.Calling.PendingEffect = Tag(TEXT("Effect.DeposeRuler"));
		Ruling.Calling.PendingCause = Tag(TEXT("Reign.Deposed"));
		Controller->SetSpeed(1.f);
		Sim->StepOnce();
		TestTrue(TEXT("A deposition opens the reign end's page"), Pages->GetPage() == ECosmicCallingPage::ReignEnd && Ruling.Player.Role == EPlayerRole::Wanderer);
		TestTrue(TEXT("saying the deposition"), Pages->GetPageTexts().Num() == 3 && Pages->GetPageTexts()[2].ToString() == Words(TEXT("Reign.End.Deposed")));
		NoDeathOnPage(TEXT("The deposition"));
		Pages->PressVeil();
		Pages->PressNext();
	}
	{
		FGalaxyState& Ruling = LayAt(*Sim, 400);
		Ruling.Player.Role = EPlayerRole::Wanderer;
		Sim->StepOnce();
		TestFalse(TEXT("A role set by hand closes no reign: no page"), Pages->IsOpen());
	}

	// ---- (9) A succession on the end's tick: the end wins - no inauguration ('next', then 'skip' on a second death) ----
	{
		FGalaxyState& Edge = LayAt(*Sim, EndTick - 40);
		RecordDeath(Edge);
		Controller->SetSpeed(1.f);
		Sim->StepOnce();
		TestTrue(TEXT("(a death forty cycles before the end: its leap ends on the end's tick)"), Pages->GetPage() == ECosmicCallingPage::Epitaph
			&& Edge.Calling.PendingLeapTicks == 40 && !Controller->IsRunEnded());
		Pages->PressVeil();
		Pages->PressNext();
		TestTrue(TEXT("The leap succeeds on the end's tick"), Edge.Tick == EndTick && Edge.Calling.Lives.Num() == 2 && Edge.Calling.LifeState == ELifeState::Embodied);
		TestTrue(TEXT("where the end latches"), Controller->IsRunEnded() && Controller->GetRunEnd() == ECosmicProvisionalEnd::Clock);
		TestTrue(TEXT("and the digest opens"), Pages->GetPage() == ECosmicCallingPage::Digest);
		Pages->PressVeil();
		Pages->PressNext();
		TestFalse(TEXT("The digest's 'next' folds the pages: no inauguration for the vessel the end's tick installed"), Pages->IsOpen());
		TestTrue(TEXT("it opens the chronicle"), Screen.Chronicle->IsOpen());
		Controller->LookAtMap();
	}
	{
		FGalaxyState& Edge = LayAt(*Sim, EndTick - 81);
		Screen.Map->RefreshNow();
		RecordDeath(Edge);
		Controller->SetSpeed(1.f);
		Sim->StepOnce();
		for (int32 Turn = 0; Turn < 3 && Pages->IsOpen(); ++Turn)
		{
			Pages->PressVeil();
			Pages->PressNext();
		}
		if (TestTrue(TEXT("(a first death, its pages turned: the second vessel stands forty-one cycles before the end)"), !Pages->IsOpen()
			&& Edge.Tick == EndTick - 41 && Edge.Calling.Lives.Num() == 2 && Edge.Calling.LifeState == ELifeState::Embodied))
		{
			RecordDeath(Edge);
			Controller->SetSpeed(1.f);
			Sim->StepOnce();
			TestTrue(TEXT("(its death opens an epitaph with 'skip')"), Pages->GetPage() == ECosmicCallingPage::Epitaph && Pages->GetLifeAt() == 1);
			Pages->PressVeil();
			Pages->PressSkip();
			TestTrue(TEXT("'Skip' runs the leap to a succession on the end's tick"), Edge.Tick == EndTick && Edge.Calling.Lives.Num() == 3
				&& Edge.Calling.LifeState == ELifeState::Embodied && Controller->IsRunEnded());
			TestFalse(TEXT("and folds every page: no inauguration"), Pages->IsOpen());
			TestTrue(TEXT("the chronicle opens"), Screen.Chronicle->IsOpen());
			TestTrue(TEXT("over the stopped time of the end"), Sim->GetTimeScale() == 0.f && Controller->GetAutoPauseReason() == ECosmicAutoPause::RunEnd);
		}
		Controller->LookAtMap();
	}

	// ---- (10) A leap of 0 (LeapTotalCap 0): epitaph -> the digest's one line -> inauguration ----
	{
		FCosmicRulesDef Capped = LeapOf(GameRules, 40);
		Capped.LeapTotalCap = 0;
		Sim->SetTables(CosmicTests::WithRules(Original, Capped));
		FGalaxyState& Zero = LayAt(*Sim, 400);
		Screen.Map->RefreshNow();
		RecordDeath(Zero);
		Controller->SetSpeed(1.f);
		Sim->StepOnce();
		if (TestTrue(TEXT("(a leap of 0: the next vessel stands on the death's tick)"), Zero.Calling.LifeState == ELifeState::Embodied
			&& Zero.Calling.Lives.Num() == 2 && Zero.Calling.Lives[1].BornTick == 400))
		{
			TestTrue(TEXT("The boundary of a leap of 0 opens the epitaph of the life that closed"), Pages->GetPage() == ECosmicCallingPage::Epitaph && Pages->GetLifeAt() == 0);
			Pages->PressVeil();
			Pages->PressNext();
			TestEqual(TEXT("Its 'next' runs no leap"), Zero.Tick, static_cast<int64>(400));
			const TArray<FText> Texts = Pages->GetPageTexts();
			TestTrue(TEXT("and opens the digest of a leap of 0: its one line"), Pages->GetPage() == ECosmicCallingPage::Digest && Texts.Num() == 1
				&& Texts[0].ToString() == Words(TEXT("Digest.NoLeap")));
			Pages->PressVeil();
			Pages->PressNext();
			TestTrue(TEXT("whose 'next' opens the inauguration of the new life"), Pages->GetPage() == ECosmicCallingPage::Inauguration && Pages->GetLifeAt() == 1);
			Pages->PressVeil();
			Pages->PressNext();
			TestFalse(TEXT("(its close folds the pages)"), Pages->IsOpen());
			TestEqual(TEXT("The camera is on the new vessel's system"), Controller->GetSelectedSystem(), Zero.Player.CurrentSystemID);
			TestTrue(TEXT("and the time stands for the succession"), Sim->GetTimeScale() == 0.f && Controller->GetAutoPauseReason() == ECosmicAutoPause::Succession);
		}
		Sim->SetTables(CosmicTests::WithRules(Original, LeapOf(GameRules, 40)));
	}

	// ---- (11) The silence after a leap: on to the end (the switch on), or the time back as the digest folds (off) ----
	auto LaySilentDeath = [&](int64 At) -> FGalaxyState&
	{
		FGalaxyState& Quiet = LayAt(*Sim, At);
		LeaveNoCandidate(Quiet, Sim->GetStateIndex());
		RecordDeath(Quiet);
		Controller->SetSpeed(1.f);
		Sim->StepOnce();
		return Quiet;
	};
	{
		FGalaxyState& Quiet = LaySilentDeath(EndTick - 100);
		if (TestTrue(TEXT("(a death with no candidate left in its galaxy, the end a hundred cycles ahead)"), Pages->GetPage() == ECosmicCallingPage::Epitaph
			&& Quiet.Calling.PendingLeapTicks == 40))
		{
			Pages->PressVeil();
			Pages->PressNext();
			TestTrue(TEXT("A leap that ends in the silence runs on to the end"), Quiet.Tick == EndTick && IsSilent(Quiet) && Quiet.Calling.Lives.Num() == 1);
			TestTrue(TEXT("where the end latches"), Controller->IsRunEnded() && Controller->GetRunEnd() == ECosmicProvisionalEnd::Clock);
			TestTrue(TEXT("then the digest opens"), Pages->GetPage() == ECosmicCallingPage::Digest);
			TestTrue(TEXT("spanning up to the end"), SameLines(Pages->GetPageTexts(), CosmicDigest::LinesOf(*Sim, 0, Controller->IsSeeingAll())));
			Pages->PressVeil();
			Pages->PressNext();
			TestFalse(TEXT("Its 'next' folds the pages"), Pages->IsOpen());
			TestTrue(TEXT("and opens the chronicle"), Screen.Chronicle->IsOpen());
			TestEqual(TEXT("whose cause note says the end's tick"), Shown(Screen.Chronicle->GetNoteText()),
				FText::Format(Label(TEXT("Chronicle.Note.Clock")), FText::AsNumber(EndTick)).ToString());
		}
		Controller->LookAtMap();
	}
	{
		Controller->SetProvisionalEndEnabled(false);
		FGalaxyState& Quiet = LaySilentDeath(400);
		if (TestTrue(TEXT("(a death with no candidate left, the end switched off)"), Pages->GetPage() == ECosmicCallingPage::Epitaph))
		{
			Pages->PressVeil();
			Pages->PressNext();
			TestTrue(TEXT("With the end switched off the leap ends in the silence"), Quiet.Tick == 440 && IsSilent(Quiet) && Quiet.Calling.Lives.Num() == 1);
			TestTrue(TEXT("and the digest of the leap opens"), Pages->GetPage() == ECosmicCallingPage::Digest);
			TestTrue(TEXT("over the stopped time: no page stands over running time"), Sim->GetTimeScale() == 0.f
				&& Controller->GetAutoPauseReason() == ECosmicAutoPause::Death);
			Pages->PressVeil();
			Pages->PressNext();
			TestFalse(TEXT("The digest's 'next' folds the pages"), Pages->IsOpen());
			TestEqual(TEXT("and gives the time back at the speed before the death"), Sim->GetTimeScale(), 1.f);
			TestTrue(TEXT("with no stop's line"), Controller->GetAutoPauseReason() == ECosmicAutoPause::None);
		}
		Controller->SetSpeed(0.f);
		Controller->SetProvisionalEndEnabled(true);
	}

	// ---- (12) An outcome on the card folds at the death (header section 2.10) ----
	{
		FGalaxyState& Answered = LayAt(*Sim, 400);
		const int32 Ruled = CosmicPetitions::RuledCivIndexOf(Answered, Sim->GetStateIndex());
		if (TestTrue(TEXT("(the vessel rules)"), Answered.Civs.IsValidIndex(Ruled)))
		{
			Answered.Civs[Ruled].Stockpile.Matter = 1000.f;
			Answered.Knowing.PendingPetitionRow = WallsCard;
			Answered.Knowing.PendingPetitionSystemID = NAME_None;
			Controller->FollowPetition(false);
			Screen.Card->PressChoice(0);
			if (TestTrue(TEXT("(an answer's outcome shows on the card)"), Screen.Card->IsShowingOutcome()))
			{
				RecordDeath(Answered);
				Controller->SetSpeed(1.f);
				Sim->StepOnce();
				TestTrue(TEXT("(the death opens its epitaph)"), Pages->GetPage() == ECosmicCallingPage::Epitaph);
				TestTrue(TEXT("A card showing an outcome folds at the death"), Screen.Card->GetShownRow().IsNone()
					&& Screen.Card->GetVisibility() == ESlateVisibility::Collapsed && !Screen.Card->IsShowingOutcome());
			}
		}
	}

	// ---- (13) The epitaph's words: the cards it leaves out, the answer it sets apart, the cause of a card's death ----
	{
		FGalaxyState& Ruling = LayAt(*Sim, 400);
		const FCosmicStateIndex& Index = Sim->GetStateIndex();
		const int32 Ruled = CosmicPetitions::RuledCivIndexOf(Ruling, Index);
		const int32 Home = Ruling.Civs.IsValidIndex(Ruled) ? Index.FindSystem(Ruling, Ruling.Civs[Ruled].HomeSystemID) : INDEX_NONE;
		const FEncounterDef* Exalt = Tables.FindEncounter(CosmicRows::PetitionExalt);
		const FEncounterDef* Throne = Tables.FindEncounter(CosmicRows::PetitionKeepThrone);
		if (TestTrue(TEXT("(the vessel rules from its home, and the game has the two cards)"), Ruling.Systems.IsValidIndex(Home) && Exalt && Throne))
		{
			const FName CivID = Ruling.Civs[Ruled].CivID;
			const FName HomeID = Ruling.Systems[Home].SystemID;
			// A ruler who answered the exaltation card and the throne card's question, then the claim's card where it stands (where
			// it will die - the newest answer is no card of the killing hazard, on the death's place), is deposed and dies.
			Ruling.Calling.Lives[0].LastChoiceRows = { CosmicRows::PetitionExalt, CosmicRows::PetitionKeepThrone, ClaimCard };
			AddAnswerLine(Ruling, 390, CosmicRows::PetitionExalt, NAME_None, CivID);
			AddAnswerLine(Ruling, 392, CosmicRows::PetitionKeepThrone, HomeID, CivID);
			AddAnswerLine(Ruling, 395, ClaimCard, Ruling.Player.CurrentSystemID, CivID);
			Ruling.Calling.PendingEffect = Tag(TEXT("Effect.DeposeRuler"));
			Ruling.Calling.PendingCause = Tag(TEXT("Reign.Deposed"));
			Controller->SetSpeed(1.f);
			Sim->StepOnce();
			TestTrue(TEXT("(deposed: the reign end's page)"), Pages->GetPage() == ECosmicCallingPage::ReignEnd && Ruling.Player.Role == EPlayerRole::Wanderer);
			Pages->PressVeil();
			Pages->PressNext();
			RecordDeath(Ruling);
			Controller->SetSpeed(1.f);
			Sim->StepOnce();
			if (TestTrue(TEXT("(then the walking vessel dies: its epitaph)"), Pages->GetPage() == ECosmicCallingPage::Epitaph && Pages->GetLifeAt() == 0))
			{
				const TArray<FCosmicAnswerTitle>& Answers = Pages->GetAnswers();
				TestTrue(TEXT("The epitaph leaves out the titles that speak to the reader or exalt: the claim's card alone stays"),
					Answers.Num() == 1 && Answers[0].Row == ClaimCard);
				bool bSaid = false;
				for (const FText& Text : Pages->GetPageTexts())
				{
					bSaid |= Text.ToString() == Exalt->Title.ToString() || Text.ToString() == Throne->Title.ToString();
				}
				TestFalse(TEXT("no line of the page says them"), bSaid);
				TestEqual(TEXT("The newest answer is no card of the killing hazard: nothing stands apart"), Pages->GetApartRow(), static_cast<int32>(INDEX_NONE));
				TestFalse(TEXT("(and no answer is the death's)"), Answers.ContainsByPredicate([](const FCosmicAnswerTitle& Each) { return Each.bOfTheDeath; }));
			}

			// The answer set apart is the killing hazard's card answered where the vessel died, of a life the hazard closed.
			FGalaxyState Apart = Sim->GetState();
			const FName DeathPlace = Apart.Player.CurrentSystemID;
			const int32 Elsewhere = Apart.Systems.IndexOfByPredicate([DeathPlace](const FStarSystemState& Each) { return Each.SystemID != DeathPlace; });
			if (TestTrue(TEXT("(the calling waits where the vessel died, and another system stands)"), Apart.Calling.LifeState == ELifeState::Interregnum
				&& !DeathPlace.IsNone() && Apart.Systems.IsValidIndex(Elsewhere)))
			{
				FLifeRecord& Dead = Apart.Calling.Lives[0];
				Dead.LastChoiceRows = { ClaimCard, WatchCard };
				AddAnswerLine(Apart, 398, WatchCard, DeathPlace, CivID);
				FCosmicLogEntry& WatchLine = Apart.Log.Last();
				TArray<FCosmicAnswerTitle> Titles = CosmicCallingPages::LastAnswersOf(Apart, Tables, 0);
				TestTrue(TEXT("(control: the killing hazard's card answered where the vessel died stands apart)"), Titles.Num() == 2 && Titles[1].bOfTheDeath
					&& !Titles[0].bOfTheDeath);
				WatchLine.FocusSystemID = Apart.Systems[Elsewhere].SystemID;
				Titles = CosmicCallingPages::LastAnswersOf(Apart, Tables, 0);
				TestTrue(TEXT("Answered on another system, it does not"), Titles.Num() == 2 && !Titles[1].bOfTheDeath);
				WatchLine.FocusSystemID = DeathPlace;
				Dead.CauseOfDeath = Tag(TEXT("Death.Encounter"));
				Titles = CosmicCallingPages::LastAnswersOf(Apart, Tables, 0);
				TestTrue(TEXT("Nor for a life a hazard did not close (the source row still the hazard's)"), Titles.Num() == 2 && !Titles[1].bOfTheDeath);
			}

			// The cause line of a card's death: the hazard the card belongs to, or no source - never the card's title.
			FLifeRecord& Dead = Ruling.Calling.Lives[0];
			const FGameplayTag Cause = Dead.CauseOfDeath;
			const FName Source = Dead.CauseSourceRow;
			Dead.CauseOfDeath = Tag(TEXT("Death.Encounter"));
			Dead.CauseSourceRow = WatchCard;
			Pages->OpenEpitaph(0, false, true);
			FFormatNamedArguments CauseArgs;
			CauseArgs.Add(TEXT("Cause"), Label(TEXT("Epitaph.Cause.Encounter")));
			CauseArgs.Add(TEXT("Source"), Nova->DisplayName);
			TestTrue(TEXT("A death by a hazard's card names that hazard"), Pages->GetPageTexts().IsValidIndex(Pages->GetCauseRow())
				&& Pages->GetPageTexts()[Pages->GetCauseRow()].ToString() == FText::Format(Label(TEXT("Epitaph.CauseFrom")), CauseArgs).ToString());
			Dead.CauseSourceRow = FName(TEXT("enc_lane_weave"));
			Pages->OpenEpitaph(0, false, true);
			FFormatNamedArguments PlainArgs;
			PlainArgs.Add(TEXT("Cause"), Label(TEXT("Epitaph.Cause.Encounter")));
			TestTrue(TEXT("and a death by a card of no hazard names no source - never the card's title"), Tables.FindEncounter(Dead.CauseSourceRow)
				&& Pages->GetPageTexts().IsValidIndex(Pages->GetCauseRow())
				&& Pages->GetPageTexts()[Pages->GetCauseRow()].ToString() == FText::Format(Label(TEXT("Epitaph.Cause")), PlainArgs).ToString());
			Dead.CauseOfDeath = Cause;
			Dead.CauseSourceRow = Source;
		}
	}

	// ---- (14) A load on a succession's tick shows its pages: epitaph -> digest -> inauguration ----
	{
		// (13) left an epitaph up, under which no speed is taken: fold it, so that the load comes over running time.
		Pages->Close();
		Controller->SetSpeed(1.f);
		TestTrue(TEXT("(the time runs before the load)"), Sim->GetTimeScale() > 0.f && !Pages->IsOpen());
		Sim->LoadState(AtSuccession);
		Screen.Map->RefreshNow();
		const FGalaxyState& Loaded = Sim->GetState();
		if (TestTrue(TEXT("A load on a succession's tick opens the epitaph of the life that closed"), Pages->GetPage() == ECosmicCallingPage::Epitaph
			&& Pages->GetLifeAt() == 0))
		{
			TestFalse(TEXT("fully shown"), Pages->IsRevealing());
			TestTrue(TEXT("and stops the running time"), Sim->GetTimeScale() == 0.f && Controller->GetAutoPauseReason() == ECosmicAutoPause::Death);
			Pages->PressNext();
			TestEqual(TEXT("Its 'next' runs no leap"), Loaded.Tick, static_cast<int64>(440));
			TestTrue(TEXT("and opens the digest of the leap"), Pages->GetPage() == ECosmicCallingPage::Digest
				&& SameLines(Pages->GetPageTexts(), CosmicDigest::LinesOf(*Sim, 0, Controller->IsSeeingAll())));
			Pages->PressVeil();
			Pages->PressNext();
			TestTrue(TEXT("then the inauguration of the new life"), Pages->GetPage() == ECosmicCallingPage::Inauguration && Pages->GetLifeAt() == 1);
			Pages->PressVeil();
			Pages->PressNext();
			TestEqual(TEXT("whose close puts the camera on the new vessel's system"), Controller->GetSelectedSystem(), Loaded.Player.CurrentSystemID);
			TestTrue(TEXT("the time stopped for the succession"), Sim->GetTimeScale() == 0.f && Controller->GetAutoPauseReason() == ECosmicAutoPause::Succession);
		}
		// The same load over a time already stopped with no reason: the bar still says why it stands still.
		Pages->Close();
		Controller->SetSpeed(0.f);
		TestTrue(TEXT("(the time stopped, no reason, before the load)"), Sim->GetTimeScale() == 0.f
			&& Controller->GetAutoPauseReason() == ECosmicAutoPause::None);
		Sim->LoadState(AtSuccession);
		TestTrue(TEXT("A load over a stopped time opens the epitaph with the death's reason"), Pages->GetPage() == ECosmicCallingPage::Epitaph
			&& Controller->GetAutoPauseReason() == ECosmicAutoPause::Death);
	}

	// ---- (15) The ascension presentation under a death and a reign end: folded at once, its give-back forgotten ----
	UCosmicAscensionOverlayWidget* Overlay = Screen.Overlay;
	{
		FGalaxyState& Rising = LayAt(*Sim, 400);
		RecordDeath(Rising);
		Controller->SetSpeed(1.f);
		Overlay->Show(EAscensionTier::Tier2_Stellar, NAME_None);
		TestTrue(TEXT("(the presentation shows, holding the speed it stopped)"), Overlay->IsShowing() && Overlay->GetPausedSpeed() == 1.f && Sim->GetTimeScale() == 0.f);
		Sim->StepOnce();
		if (TestTrue(TEXT("(the death's boundary opens the epitaph)"), Pages->GetPage() == ECosmicCallingPage::Epitaph))
		{
			TestTrue(TEXT("A death page folds the presentation at once, its give-back forgotten"), !Overlay->IsShowing() && Overlay->GetPausedSpeed() == 0.f);
			Overlay->Advance(CosmicAscensionOverlay::Seconds + 1.f);
			TestTrue(TEXT("Past the presentation's time the time still stands for the death"), Sim->GetTimeScale() == 0.f
				&& Controller->GetAutoPauseReason() == ECosmicAutoPause::Death);
		}
	}
	{
		LayAt(*Sim, 400);
		if (TestTrue(TEXT("(an abdication is recorded)"), Controller->Abdicate()))
		{
			Controller->SetSpeed(1.f);
			Overlay->Show(EAscensionTier::Tier2_Stellar, NAME_None);
			Sim->StepOnce();
			if (TestTrue(TEXT("(the abdication's boundary opens the reign end)"), Pages->GetPage() == ECosmicCallingPage::ReignEnd))
			{
				TestTrue(TEXT("The reign end folds the presentation at once, its give-back forgotten"), !Overlay->IsShowing() && Overlay->GetPausedSpeed() == 0.f);
				Overlay->Advance(CosmicAscensionOverlay::Seconds + 1.f);
				TestTrue(TEXT("and the time still stands for the reign end"), Sim->GetTimeScale() == 0.f && Controller->GetAutoPauseReason() == ECosmicAutoPause::ReignEnd);
			}
			Pages->PressVeil();
			Pages->PressNext();
		}
		Overlay->Close();
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

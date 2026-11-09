// Cosmic Ascension - automation tests of the third rung's spiritual sight (Presentation). Design: guideline.md section
// 3.2, 4.2, 6.3, 6.4, 8.2, 9.3, 11 Phase 3.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/LineBatchComponent.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "Internationalization/StringTableCore.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameters.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicSeal.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicLogFeedWidget.h"
#include "UI/CosmicPalette.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "UI/CosmicUILabels.h"
#include "UObject/Class.h"
#include "Widgets/IToolTip.h"
#include "Widgets/SWidget.h"
#include "World/CosmicGalaxyMapActor.h"
#include "World/CosmicGameMode.h"
#include "World/CosmicMapLook.h"
#include "World/CosmicStarSystemActor.h"

#include <limits>

namespace CosmicSpiritualSightTests
{
	// The test-table helpers are shared (Tests/CosmicTestHelpers.h).
	using CosmicTests::AddInstance;
	using CosmicTests::NewTable;
	using CosmicTests::WithRules;
	using CosmicTests::WithTranscendents;

	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	const FName Quiet(TEXT("trans_test_quiet"));
	const FName Zeal(TEXT("trans_test_zeal"));
	const FName Still(TEXT("trans_test_still"));
	const FName Covenant(TEXT("trans_test_covenant"));
	const FName Untitled(TEXT("trans_test_untitled"));
	// Hangul titles (the particle modifier |hpp adds a particle only after a Hangul syllable or a digit): one ends in a
	// vowel and takes the second form, one ends in a consonant and takes the first.
	const TCHAR* const QuietTitle = TEXT("\uACE0\uC694\uD55C \uC790");
	const TCHAR* const ZealTitle = TEXT("\uD0C0\uB294 \uC785");
	const TCHAR* const ParticleAfterVowel = TEXT("\uAC00");
	const TCHAR* const ParticleAfterConsonant = TEXT("\uC774");
	// The last word of every phrase of the sight: "place".
	const TCHAR* const PlaceWord = TEXT("\uACF3");

	FTranscendentDef RowOf(const TCHAR* Allegiance, const TCHAR* Doctrine, const TCHAR* Title, float Depth)
	{
		FTranscendentDef Row;
		Row.Allegiance = Tag(Allegiance);
		Row.Doctrine = Tag(Doctrine);
		Row.Title = FText::AsCultureInvariant(Title);
		Row.Tier = EAscensionTier::Tier4_Transcendent;
		Row.Depth = Depth;
		return Row;
	}

	// Two fallen ones of the fourth rung and of full depth, an independent one, a celestial one (the schema keeps it out
	// of a roster; the test makes an instance anyway) and a fallen one without a title (a slip the schema refuses).
	UDataTable* TestRows()
	{
		UDataTable* Table = NewTable(FTranscendentDef::StaticStruct());
		Table->AddRow(Quiet, RowOf(TEXT("Realm.Abyssal"), TEXT("Doctrine.Silence"), QuietTitle, 1.f));
		Table->AddRow(Zeal, RowOf(TEXT("Realm.Abyssal"), TEXT("Doctrine.Zeal"), ZealTitle, 1.f));
		Table->AddRow(Still, RowOf(TEXT("Realm.Independent"), TEXT("Doctrine.Stillness"), TEXT(""), 0.5f));
		Table->AddRow(Covenant, RowOf(TEXT("Realm.Celestial"), TEXT("Doctrine.Covenant"), TEXT(""), 0.5f));
		Table->AddRow(Untitled, RowOf(TEXT("Realm.Abyssal"), TEXT("Doctrine.Assimilation"), TEXT(""), 1.f));
		return Table;
	}

	// The knobs of the test, not of the game: the values below are compared exactly.
	FCosmicRulesDef TestRules(const FCosmicRulesDef& Base)
	{
		FCosmicRulesDef Rules = Base;
		Rules.PressureBySourceTier = { 0.f, 0.f, 0.25f, 1.f, 2.f };
		Rules.PressureAdjacencyFalloff = 0.5f;
		Rules.ClaimAdjacencyLanes = 1;
		return Rules;
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

	// How far a colour is from grey: its largest channel less its smallest.
	float SaturationOf(const FLinearColor& Color)
	{
		return FMath::Max3(Color.R, Color.G, Color.B) - FMath::Min3(Color.R, Color.G, Color.B);
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

	// The scalar and vector values of a material by name, as the base asset or an instance answers them.
	bool ScalarOf(const UMaterialInterface& Material, const TCHAR* Name, float& Out)
	{
		return Material.GetScalarParameterValue(FHashedMaterialParameterInfo(FName(Name)), Out);
	}

	bool VectorOf(const UMaterialInterface& Material, const TCHAR* Name, FLinearColor& Out)
	{
		return Material.GetVectorParameterValue(FHashedMaterialParameterInfo(FName(Name)), Out);
	}

	// Whether two sets of star values are the same, field by field.
	bool SameParams(const FCosmicStarParams& A, const FCosmicStarParams& B)
	{
		return A.StarColor.Equals(B.StarColor, 1e-5f) && FMath::IsNearlyEqual(A.Brightness, B.Brightness, 1e-5f) && FMath::IsNearlyEqual(A.HazardAmount, B.HazardAmount, 1e-5f)
			&& A.WarningColor.Equals(B.WarningColor, 1e-5f) && FMath::IsNearlyEqual(A.ViewMode, B.ViewMode, 1e-5f) && FMath::IsNearlyEqual(A.Pressure, B.Pressure, 1e-5f)
			&& A.DominionColor.Equals(B.DominionColor, 1e-5f) && FMath::IsNearlyEqual(A.ClaimState, B.ClaimState, 1e-5f);
	}

	// Whether a colour is a grey: no hue left.
	bool IsGrey(const FLinearColor& Color)
	{
		return SaturationOf(Color) < 1e-5f;
	}

	// The fallen's band of hues as the guideline gives it (section 6.4: violet, purple, magenta, pink), written here and
	// not read from the code under test.
	constexpr float FallenBandFrom = 250.f;
	constexpr float FallenBandTo = 330.f;

	// Whether a colour has a hue in the fallen's band (a grey has none).
	bool InFallenBand(const FLinearColor& Color)
	{
		const FLinearColor Hsv = Color.LinearRGBToHSV();
		return Hsv.G > 1e-3f && Hsv.R >= FallenBandFrom && Hsv.R <= FallenBandTo;
	}

	// The lines of the batch on a circle around Centre, further out than Inner and within Inner + Reach: a ring's segments
	// start and end at the same distance from their star, a lane starts at a star's centre.
	TArray<const FBatchedLine*> RingLinesAround(const ULineBatchComponent& Batch, const FVector& Centre, double Inner, double Reach)
	{
		TArray<const FBatchedLine*> Found;
		for (const FBatchedLine& Line : Batch.BatchedLines)
		{
			const double From = FVector::Dist2D(Line.Start, Centre);
			const double To = FVector::Dist2D(Line.End, Centre);
			if (From > Inner && From < Inner + Reach && FMath::Abs(From - To) < 0.5)
			{
				Found.Add(&Line);
			}
		}
		return Found;
	}

	// How many of the lines are drawn in Color.
	int32 CountInColor(const TArray<const FBatchedLine*>& Lines, const FLinearColor& Color)
	{
		int32 Count = 0;
		for (const FBatchedLine* Line : Lines)
		{
			Count += Line && Line->Color.Equals(Color, 1e-5f) ? 1 : 0;
		}
		return Count;
	}

	// Whether a widget shows and lets a click reach it: visible, or self-hit-test-invisible over its hit-testable button
	// (collapsed, hidden or hit-test-invisible does not).
	bool ShowsForClicks(ESlateVisibility Visibility)
	{
		return Visibility == ESlateVisibility::Visible || Visibility == ESlateVisibility::SelfHitTestInvisible;
	}

	// The line of the batch between two points, either way round; null when none.
	const FBatchedLine* FindLine(const ULineBatchComponent& Batch, const FVector& A, const FVector& B)
	{
		for (const FBatchedLine& Line : Batch.BatchedLines)
		{
			if ((Line.Start.Equals(A, 1.0) && Line.End.Equals(B, 1.0)) || (Line.Start.Equals(B, 1.0) && Line.End.Equals(A, 1.0)))
			{
				return &Line;
			}
		}
		return nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUISpiritualSightTest, "Cosmic_Ascension.UI.SpiritualSight", CosmicTests::Flags)
bool FCosmicUISpiritualSightTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSpiritualSightTests;
	using CosmicMapLook::LaneColorOf;
	using CosmicMapLook::PressureShareOf;
	using CosmicMapLook::ShownViewMode;
	using CosmicMapLook::StarParamsOf;

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
	ACosmicGalaxyMapActor* Map = FindMap(World);
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(World ? World->GetFirstPlayerController() : nullptr);
	ACosmicHUD* HUD = Controller ? Controller->GetHUD<ACosmicHUD>() : nullptr;
	if (!TestNotNull(TEXT("The game mode spawned the map"), Map) || !TestNotNull(TEXT("The first player controller is the game's"), Controller)
		|| !TestNotNull(TEXT("The controller has the HUD"), HUD))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(Original);
		Controller->CosmicSpiritualSight(0);
		Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
		Controller->SelectSystem(NAME_None);
		Sim->NewGalaxy(42, 5, 3);
		Map->RefreshNow();
	};
	const FString& Placeholder = FStringTableEntry::GetPlaceholderSourceString();
	const FCosmicRulesDef Rules = TestRules(Sim->GetRules());
	const FCosmicDataTables Tables = CosmicTests::WithoutProjects(WithRules(WithTranscendents(Original, TestRows()), Rules));
	const FTranscendentDef* QuietRow = Tables.FindTranscendent(Quiet);
	const FTranscendentDef* ZealRow = Tables.FindTranscendent(Zeal);
	const FTranscendentDef* StillRow = Tables.FindTranscendent(Still);
	const FTranscendentDef* CovenantRow = Tables.FindTranscendent(Covenant);
	if (!TestTrue(TEXT("(the test's rows are held)"), QuietRow && ZealRow && StillRow && CovenantRow))
	{
		return false;
	}

	// ---- 1. The rung opens the sight; the view drawn ----
	{
		const UEnum* Tiers = StaticEnum<EAscensionTier>();
		const int32 Count = Tiers->NumEnums() - 1;
		TestEqual(TEXT("(five rungs)"), Count, 5);
		for (int32 At = 0; At < Count; ++At)
		{
			const EAscensionTier Tier = static_cast<EAscensionTier>(Tiers->GetValueByIndex(At));
			const int32 Rung = static_cast<int32>(Tier) + 1;
			if (Rung < 3)
			{
				TestFalse(*FString::Printf(TEXT("Tier %d keeps the sight closed"), Rung), CosmicMapLook::SpiritualSightOpens(Tier));
			}
			else
			{
				TestTrue(*FString::Printf(TEXT("Tier %d opens the sight"), Rung), CosmicMapLook::SpiritualSightOpens(Tier));
			}
		}
		TestTrue(TEXT("Wanted but closed draws the physical view"), ShownViewMode(ECosmicViewMode::Spiritual, false, EAscensionTier::Tier2_Stellar) == ECosmicViewMode::Physical);
		TestTrue(TEXT("Wanted and open draws the spiritual view"), ShownViewMode(ECosmicViewMode::Spiritual, false, EAscensionTier::Tier3_Causal) == ECosmicViewMode::Spiritual);
		TestTrue(TEXT("Not wanted draws the physical view even at the fifth rung"), ShownViewMode(ECosmicViewMode::Physical, false, EAscensionTier::Tier5_EternalOrder) == ECosmicViewMode::Physical);
		TestTrue(TEXT("Forced open draws it at the first rung"), ShownViewMode(ECosmicViewMode::Spiritual, true, EAscensionTier::Tier1_Planetary) == ECosmicViewMode::Spiritual);
		TestTrue(TEXT("Forced open draws nothing that is not wanted"), ShownViewMode(ECosmicViewMode::Physical, true, EAscensionTier::Tier1_Planetary) == ECosmicViewMode::Physical);
	}

	// ---- 2. The pressure share ----
	{
		TestEqual(TEXT("Inside the deepest sphere pulses fully"), PressureShareOf(2.f, Rules), 1.f);
		TestEqual(TEXT("One lane out pulses at half"), PressureShareOf(1.f, Rules), 0.5f);
		TestEqual(TEXT("More than full pulses fully"), PressureShareOf(5.f, Rules), 1.f);
		TestEqual(TEXT("No pressure, no pulse"), PressureShareOf(0.f, Rules), 0.f);
		TestEqual(TEXT("A pressure that is not a number does not pulse"), PressureShareOf(std::numeric_limits<float>::quiet_NaN(), Rules), 0.f);
		TestEqual(TEXT("A negative pressure does not pulse"), PressureShareOf(-1.f, Rules), 0.f);
		FCosmicRulesDef Half = Rules;
		Half.PressureBySourceTier = { 0.f, 0.f, 0.25f, 0.5f, 2.f };
		TestEqual(TEXT("The share follows the rules' fourth-rung weight"), PressureShareOf(1.f, Half), 1.f);
		FCosmicRulesDef Zero = Rules;
		Zero.PressureBySourceTier = { 0.f, 0.f, 0.25f, 0.f, 2.f };
		TestEqual(TEXT("A fourth-rung weight of 0 gives no pulse"), PressureShareOf(1.f, Zero), 0.f);
		FCosmicRulesDef Short = Rules;
		Short.PressureBySourceTier = { 0.f, 0.f, 0.25f };
		TestEqual(TEXT("A weight list without a fourth rung gives no pulse"), PressureShareOf(1.f, Short), 0.f);
		FCosmicRulesDef NotANumber = Rules;
		NotANumber.PressureBySourceTier = { 0.f, 0.f, 0.25f, std::numeric_limits<float>::quiet_NaN(), 2.f };
		TestEqual(TEXT("A fourth-rung weight that is not a number gives no pulse"), PressureShareOf(1.f, NotANumber), 0.f);
	}

	// ---- 3. One purple, by the allegiance of the row ----
	{
		using CosmicMapLook::DominionColorOf;
		TestEqual(TEXT("The fallen one's colour is the one purple"), DominionColorOf(QuietRow), CosmicPalette::Purple);
		TestEqual(TEXT("which is the feed's purple"), DominionColorOf(QuietRow), CosmicLogFeed::ColorOf(1, Tag(TEXT("Log.Claim"))));
		TestEqual(TEXT("A second fallen doctrine has the same purple"), DominionColorOf(ZealRow), CosmicPalette::Purple);
		TestEqual(TEXT("A celestial row has no colour"), DominionColorOf(CovenantRow), FLinearColor::Black);
		TestEqual(TEXT("An independent row has no colour"), DominionColorOf(StillRow), FLinearColor::Black);
		TestEqual(TEXT("No row has no colour"), DominionColorOf(nullptr), FLinearColor::Black);
	}

	// ---- 4. The half-lanes ----
	{
		const FLinearColor Base = LaneColorOf(0.f, ECosmicViewMode::Physical);
		TestEqual(TEXT("The physical view keeps the lane colour under full pressure"), LaneColorOf(1.f, ECosmicViewMode::Physical), Base);
		TestEqual(TEXT("An end without pressure keeps its colour in the sight"), LaneColorOf(0.f, ECosmicViewMode::Spiritual), Base);
		const FLinearColor Full = LaneColorOf(1.f, ECosmicViewMode::Spiritual);
		TestEqual(TEXT("A fully pressed end is grey: red is green"), Full.R, Full.G, 1e-4f);
		TestEqual(TEXT("A fully pressed end is grey: green is blue"), Full.G, Full.B, 1e-4f);
		TestEqual(TEXT("and keeps it drawn"), Full.A, 1.f);
		const FLinearColor Half = LaneColorOf(0.5f, ECosmicViewMode::Spiritual);
		TestTrue(TEXT("Half the pressure takes some of the colour"), SaturationOf(Half) < SaturationOf(Base) - 1e-3f);
		TestTrue(TEXT("but not all of it"), SaturationOf(Half) > 1e-3f);
		TestEqual(TEXT("A pressure that is not a number keeps the colour"), LaneColorOf(std::numeric_limits<float>::quiet_NaN(), ECosmicViewMode::Spiritual), Base);
		TestTrue(TEXT("More than full is the same grey"), LaneColorOf(4.f, ECosmicViewMode::Spiritual).Equals(Full, 1e-5f));
	}

	// ---- 4b. A people's ring keeps out of the fallen's purple in the sight ----
	{
		using CosmicMapLook::OwnerRingColorOf;
		// Colours of the owners' wheel of a twelve-people galaxy (FCivilizationState::Color): hues about 269, 298, 328,
		// 239 (just outside the band) and 121.
		const FLinearColor Violet(0.63f, 0.294f, 1.f);
		const FLinearColor Magenta(0.978f, 0.294f, 1.f);
		const FLinearColor Pink(1.f, 0.294f, 0.675f);
		const FLinearColor Blue(0.294f, 0.305f, 1.f);
		const FLinearColor Green(0.3f, 1.f, 0.294f);
		TestEqual(TEXT("The physical view keeps a violet ring"), OwnerRingColorOf(Violet, ECosmicViewMode::Physical), Violet);
		const FLinearColor InSight = OwnerRingColorOf(Violet, ECosmicViewMode::Spiritual);
		TestEqual(TEXT("In the sight a violet ring is grey: red is green"), InSight.R, InSight.G, 1e-5f);
		TestEqual(TEXT("In the sight a violet ring is grey: green is blue"), InSight.G, InSight.B, 1e-5f);
		TestEqual(TEXT("and stays drawn"), InSight.A, 1.f);
		TestEqual(TEXT("at the violet's own brightness"), InSight.R, Violet.GetLuminance(), 1e-5f);
		TestTrue(TEXT("A magenta ring is grey in the sight"), IsGrey(OwnerRingColorOf(Magenta, ECosmicViewMode::Spiritual)));
		TestTrue(TEXT("A pink ring is grey in the sight"), IsGrey(OwnerRingColorOf(Pink, ECosmicViewMode::Spiritual)));
		TestTrue(TEXT("The fallen's own purple is no people's ring in the sight"), IsGrey(OwnerRingColorOf(CosmicPalette::Purple, ECosmicViewMode::Spiritual)));
		TestEqual(TEXT("A blue ring just outside the band keeps its colour in the sight"), OwnerRingColorOf(Blue, ECosmicViewMode::Spiritual), Blue);
		TestEqual(TEXT("A green ring keeps its colour in the sight"), OwnerRingColorOf(Green, ECosmicViewMode::Spiritual), Green);

		// Every people of the home galaxy (twelve, the wheel's colours).
		Sim->NewGalaxy(42, 0, 0);
		int32 InBand = 0;
		int32 PurpleInSight = 0;
		for (const FCivilizationState& Civ : Sim->GetState().Civs)
		{
			InBand += InFallenBand(Civ.Color) ? 1 : 0;
			PurpleInSight += InFallenBand(OwnerRingColorOf(Civ.Color, ECosmicViewMode::Spiritual)) ? 1 : 0;
		}
		TestTrue(TEXT("(the home galaxy has peoples whose colour lies in the fallen's band)"), InBand > 0);
		TestEqual(TEXT("No people's ring of the home galaxy is in the fallen's band in the sight"), PurpleInSight, 0);
	}

	// ---- 5. The sky ----
	TestEqual(TEXT("The physical sky is 0"), CosmicMapLook::SkyIntensityOf(ECosmicViewMode::Physical), 0.f);
	TestEqual(TEXT("The sight's sky is 1"), CosmicMapLook::SkyIntensityOf(ECosmicViewMode::Spiritual), 1.f);

	// ---- 6. The values of a star ----
	{
		FCosmicSystemLook Living;
		Living.StarColor = FLinearColor(0.2f, 0.4f, 0.6f);
		Living.bHazard = true;
		FCosmicSpiritLook Held;
		Held.Pressure = 0.5f;
		Held.DominionColor = CosmicPalette::Purple;
		Held.ClaimState = ECosmicClaimState::Held;
		Held.MarkRow = Quiet;

		const FCosmicStarParams Physical = StarParamsOf(Living, Held, ECosmicViewMode::Physical);
		TestEqual(TEXT("Physical: ViewMode 0"), Physical.ViewMode, 0.f);
		TestEqual(TEXT("Physical: no pressure"), Physical.Pressure, 0.f);
		TestEqual(TEXT("Physical: no claim state"), Physical.ClaimState, 0.f);
		TestEqual(TEXT("Physical: no dominion colour"), Physical.DominionColor, FLinearColor::Black);
		TestEqual(TEXT("Physical: the class colour"), Physical.StarColor, Living.StarColor);
		TestEqual(TEXT("Physical: brightness 1.3"), Physical.Brightness, 1.3f);
		TestEqual(TEXT("Physical: the hazard's warning"), Physical.HazardAmount, 1.f);
		TestEqual(TEXT("Physical: the warning is orange"), Physical.WarningColor, FLinearColor(1.f, 0.25f, 0.05f));

		const FCosmicStarParams Spiritual = StarParamsOf(Living, Held, ECosmicViewMode::Spiritual);
		TestEqual(TEXT("Spiritual: ViewMode 1"), Spiritual.ViewMode, 1.f);
		TestEqual(TEXT("Spiritual: the spirit's pressure"), Spiritual.Pressure, 0.5f);
		TestEqual(TEXT("Spiritual: the spirit's colour"), Spiritual.DominionColor, CosmicPalette::Purple);
		TestEqual(TEXT("Spiritual: the claim state"), Spiritual.ClaimState, 2.f);
		TestTrue(TEXT("Spiritual: the star's own colour recedes"), SaturationOf(Spiritual.StarColor) < SaturationOf(Living.StarColor) - 1e-3f);
		TestEqual(TEXT("Spiritual: and keeps its alpha"), Spiritual.StarColor.A, 1.f);
		TestTrue(TEXT("Spiritual: and dims"), Spiritual.Brightness < 1.3f);
		TestEqual(TEXT("Spiritual: the warning stays for a mark that is no claim"), Spiritual.HazardAmount, 1.f);
		TestFalse(TEXT("Spiritual: the warning is no longer orange"), Spiritual.WarningColor.Equals(Physical.WarningColor, 1e-3f));
		const FLinearColor& Warning = Spiritual.WarningColor;
		const float Scale = Warning.R / CosmicPalette::Grey.R;
		TestTrue(TEXT("Spiritual: the warning is a shade of the palette's grey"), FMath::IsNearlyEqual(Warning.G, Scale * CosmicPalette::Grey.G, 1e-4f) && FMath::IsNearlyEqual(Warning.B, Scale * CosmicPalette::Grey.B, 1e-4f));
		const float Peak = 3.f * FMath::Max3(Warning.R, Warning.G, Warning.B);
		TestTrue(TEXT("Spiritual: the warning's peak is about 1"), Peak > 0.8f && Peak < 1.2f);

		FCosmicSpiritLook Claimed = Held;
		Claimed.ClaimState = ECosmicClaimState::Claimed;
		const FCosmicStarParams Claim = StarParamsOf(Living, Claimed, ECosmicViewMode::Spiritual);
		TestEqual(TEXT("A claim replaces the warning"), Claim.HazardAmount, 0.f);
		TestEqual(TEXT("and is claim state 1"), Claim.ClaimState, 1.f);

		FCosmicSystemLook Dead = Living;
		Dead.bDead = true;
		Dead.bHazard = false;
		const FCosmicStarParams Gone = StarParamsOf(Dead, Held, ECosmicViewMode::Spiritual);
		TestEqual(TEXT("A destroyed star draws nothing spiritual"), Gone.ViewMode, 0.f);
		TestTrue(TEXT("and is dim"), Gone.Brightness < 1.f);
		TestEqual(TEXT("and has no dominion colour"), Gone.DominionColor, FLinearColor::Black);

		const FCosmicStarParams Default = StarParamsOf(FCosmicSystemLook(), FCosmicSpiritLook(), ECosmicViewMode::Physical);
		TestTrue(TEXT("A default star is the base material's look"), SameParams(Default, FCosmicStarParams()));
	}

	// ---- 7. What the sight draws of a board (SpiritOf) ----
	{
		Sim->NewGalaxy(42, 12, 3);
		FGalaxyState Board = Sim->GetState();
		Board.Transcendents.Reset();
		const bool bWired = Board.Civs.Num() == 3 && CosmicTests::WireChain(Board);
		if (!TestTrue(TEXT("(the board has twelve systems and three peoples to wire)"), bWired))
		{
			return false;
		}
		CosmicTests::ClearOwners(Board);
		CosmicTests::Give(Board, 0, { 7 });
		CosmicTests::Give(Board, 1, { 9 });
		CosmicTests::Give(Board, 2, { 5 });
		Board.Civs[0].Tier = EAscensionTier::Tier1_Planetary;
		Board.Civs[1].Tier = EAscensionTier::Tier3_Causal;
		Board.Civs[2].Tier = EAscensionTier::Tier4_Transcendent;
		for (FStarSystemState& System : Board.Systems)
		{
			System.ActiveHazard = FGameplayTag();
		}
		FCosmicStateIndex Index;
		Index.Rebuild(Board);
		auto IdOf = [&Board](int32 At) { return Board.Systems[At].SystemID; };
		auto SpiritAt = [&](const FGalaxyState& State, int32 At)
		{
			FCosmicStateIndex Own;
			Own.Rebuild(State);
			return CosmicMapLook::SpiritOf(State, Own, Tables, Rules, At);
		};
		const FGameplayTag Claim = Tag(TEXT("Hazard.Claim"));

		// The quiet one holds 6.
		FGalaxyState Held = Board;
		AddInstance(Held, Quiet, { IdOf(6) });
		{
			const FCosmicSpiritLook Six = SpiritAt(Held, 6);
			TestTrue(TEXT("System 6 in the quiet one's sphere is held"), Six.ClaimState == ECosmicClaimState::Held);
			TestEqual(TEXT("and pulses fully"), Six.Pressure, 1.f);
			TestEqual(TEXT("in the purple"), Six.DominionColor, CosmicPalette::Purple);
			TestEqual(TEXT("named after its holder"), Six.MarkRow, Quiet);
			TestFalse(TEXT("with no sealed ring"), Six.bSealedRing);
			TestFalse(TEXT("and holds rather than presses"), Six.bMarkPresses);
			const FCosmicSpiritLook Seven = SpiritAt(Held, 7);
			TestTrue(TEXT("One lane out, a first-rung people's system is no mark"), Seven.ClaimState == ECosmicClaimState::None);
			TestEqual(TEXT("but pulses at half"), Seven.Pressure, 0.5f);
			TestEqual(TEXT("in the purple of its presser"), Seven.DominionColor, CosmicPalette::Purple);
			TestEqual(TEXT("named after its presser"), Seven.MarkRow, Quiet);
			TestTrue(TEXT("as the one that presses"), Seven.bMarkPresses);
			const FCosmicSpiritLook Five = SpiritAt(Held, 5);
			TestEqual(TEXT("A people of the fallen's own rung does not pulse"), Five.Pressure, 0.f);
			TestTrue(TEXT("and nobody is named there"), Five.MarkRow.IsNone());
			TestEqual(TEXT("and it has no colour"), Five.DominionColor, FLinearColor::Black);
			const FCosmicSpiritLook Eight = SpiritAt(Held, 8);
			// The reach is the pressure function's (Sim.RealmPressureMonotonic holds it): checks of the board.
			TestEqual(TEXT("(two lanes out nothing pulses)"), Eight.Pressure, 0.f);
			TestTrue(TEXT("(two lanes out nothing marks)"), Eight.ClaimState == ECosmicClaimState::None && Eight.MarkRow.IsNone());
			TestEqual(TEXT("A third-rung people's own system does not pulse"), SpiritAt(Held, 9).Pressure, 0.f);
		}
		// A wanderer of the fourth rung on a wild system out of every sphere: its own term is the whole pressure there.
		{
			FGalaxyState Walking = Held;
			Walking.Player.Role = EPlayerRole::Wanderer;
			Walking.Player.RuledCivID = NAME_None;
			Walking.Calling.LifeState = ELifeState::Embodied;
			Walking.Calling.PersonalTier = EAscensionTier::Tier4_Transcendent;
			Walking.Player.CurrentSystemID = IdOf(1);
			FCosmicStateIndex Own;
			Own.Rebuild(Walking);
			const FCosmicPressure Reading = UGalaxySimulationSubsystem::ComputePressureIn(Walking, Own, Tables, Rules, 1);
			TestTrue(TEXT("(the wanderer presses where it stands)"), Reading.DominantSource == ECosmicPressureSource::Wanderer && Reading.P > 0.f);
			const FCosmicSpiritLook One = SpiritAt(Walking, 1);
			TestEqual(TEXT("A wanderer's own term does not pulse"), One.Pressure, 0.f);
		}
		// A claim on 5, whose fifth-rung owner outweighs every other term there: the words can name only the claimant.
		{
			FGalaxyState Owned = Held;
			Owned.Civs[2].Tier = EAscensionTier::Tier5_EternalOrder;
			Owned.Systems[5].ActiveHazard = Claim;
			const FCosmicSpiritLook Five = SpiritAt(Owned, 5);
			TestTrue(TEXT("A standing claim is Claimed"), Five.ClaimState == ECosmicClaimState::Claimed);
			TestEqual(TEXT("named after its claimant"), Five.MarkRow, Quiet);
			TestEqual(TEXT("in its purple"), Five.DominionColor, CosmicPalette::Purple);
			TestEqual(TEXT("(no fallen one leads the pressure there)"), Five.Pressure, 0.f);
		}
		// A claim on 7.
		{
			FGalaxyState Claiming = Held;
			Claiming.Systems[7].ActiveHazard = Claim;
			TestTrue(TEXT("(the claim on 7 has its claimant)"), SpiritAt(Claiming, 7).ClaimState == ECosmicClaimState::Claimed);

			// The quiet one sealed: its sphere is bound and its claim has no claimant now.
			FGalaxyState Sealed = Claiming;
			Sealed.Transcendents[0].bSealed = true;
			const FCosmicSpiritLook Six = SpiritAt(Sealed, 6);
			TestTrue(TEXT("A sealed sphere's system is Bound"), Six.ClaimState == ECosmicClaimState::Bound);
			TestTrue(TEXT("with the sealed ring"), Six.bSealedRing);
			TestEqual(TEXT("(and no pulse: a sealed one presses nothing)"), Six.Pressure, 0.f);
			TestEqual(TEXT("and names its sealed holder"), Six.MarkRow, Quiet);
			TestEqual(TEXT("whose colour it carries (no pulse, no hold: the material draws nothing of it)"), Six.DominionColor, CosmicPalette::Purple);
			const FCosmicSpiritLook Orphan = SpiritAt(Sealed, 7);
			TestTrue(TEXT("A claim with no claimant now is no claim"), Orphan.ClaimState == ECosmicClaimState::None);
		}
		// A sealed sphere pressed by another fallen one: the words name the presser, whose purple is drawn.
		{
			FGalaxyState Pressed = Board;
			AddInstance(Pressed, Quiet, { IdOf(6) }).bSealed = true;
			AddInstance(Pressed, Zeal, { IdOf(7) });
			const FCosmicSpiritLook Six = SpiritAt(Pressed, 6);
			TestTrue(TEXT("(a sealed sphere is still Bound under another's pressure)"), Six.ClaimState == ECosmicClaimState::Bound);
			TestEqual(TEXT("(and pulses with the other's pressure)"), Six.Pressure, 0.5f);
			TestEqual(TEXT("A sealed sphere pressed by another fallen names its presser"), Six.MarkRow, Zeal);
			TestTrue(TEXT("as the one that presses"), Six.bMarkPresses);
		}
		// An independent sphere and a celestial instance mark nothing and do not pulse.
		{
			FGalaxyState Independent = Board;
			AddInstance(Independent, Still, { IdOf(2) });
			TestTrue(TEXT("(the independent one presses inside its sphere)"), UGalaxySimulationSubsystem::ComputePressureIn(Independent, Index, Tables, Rules, 2).P > 0.f);
			TestTrue(TEXT("An independent sphere marks nothing"), SpiritAt(Independent, 2).ClaimState == ECosmicClaimState::None);
			TestTrue(TEXT("and names no one"), SpiritAt(Independent, 2).MarkRow.IsNone());
			TestEqual(TEXT("An independent sphere does not pulse inside"), SpiritAt(Independent, 2).Pressure, 0.f);
			TestEqual(TEXT("nor one lane out on one side"), SpiritAt(Independent, 1).Pressure, 0.f);
			TestEqual(TEXT("nor on the other"), SpiritAt(Independent, 3).Pressure, 0.f);
			FGalaxyState Messenger = Board;
			AddInstance(Messenger, Covenant, { IdOf(3) });
			TestTrue(TEXT("A holder that is not fallen marks nothing"), SpiritAt(Messenger, 3).ClaimState == ECosmicClaimState::None);
			TestEqual(TEXT("and does not pulse"), SpiritAt(Messenger, 3).Pressure, 0.f);
		}
		// Precedence: a claim over a hold, a live hold over a bound one.
		{
			FGalaxyState Both = Board;
			AddInstance(Both, Zeal, { IdOf(11) });
			AddInstance(Both, Quiet, { IdOf(6), IdOf(10) });
			Both.Systems[10].ActiveHazard = Claim;
			const FCosmicSpiritLook Ten = SpiritAt(Both, 10);
			TestTrue(TEXT("A claim outranks a hold"), Ten.ClaimState == ECosmicClaimState::Claimed);
			TestEqual(TEXT("named after the claimant, not the holder"), Ten.MarkRow, Zeal);

			FGalaxyState LiveAndSealed = Board;
			AddInstance(LiveAndSealed, Quiet, { IdOf(6) });
			AddInstance(LiveAndSealed, Zeal, { IdOf(6) }).bSealed = true;
			const FCosmicSpiritLook Six = SpiritAt(LiveAndSealed, 6);
			TestTrue(TEXT("In a live and a sealed sphere: Held"), Six.ClaimState == ECosmicClaimState::Held);
			TestTrue(TEXT("with the sealed ring too"), Six.bSealedRing);
			TestEqual(TEXT("named after the live holder"), Six.MarkRow, Quiet);
		}
		// A destroyed system, and an index out of range.
		{
			FGalaxyState Destroyed = Held;
			Destroyed.Systems[6].StarClass = FGameplayTag();
			const FCosmicSpiritLook Six = SpiritAt(Destroyed, 6);
			TestTrue(TEXT("A destroyed system has no mark"), Six.ClaimState == ECosmicClaimState::None);
			TestEqual(TEXT("and no pulse"), Six.Pressure, 0.f);
			TestTrue(TEXT("(an index out of range has no mark)"), SpiritAt(Held, 99).ClaimState == ECosmicClaimState::None);
		}
	}

	// ---- 8. The map: closed, opened, set, forced, deposed, opened again, rebuilt ----
	Sim->SetTables(Tables);
	Sim->NewGalaxy(42, 12, 3);
	FGalaxyState& Live = Sim->GetMutableState();
	const int32 RuledIndex = Sim->GetStateIndex().FindCiv(Live, Live.Player.RuledCivID);
	if (!TestTrue(TEXT("(the board has twelve systems, three peoples and a ruled one)"), CosmicTests::LivingSystems(Live) == 12 && Live.Civs.Num() == 3 && Live.Civs.IsValidIndex(RuledIndex)))
	{
		return false;
	}
	const int32 Other = RuledIndex == 1 ? 2 : 1;
	// No tick runs below: the crossing stage would take away the ruled people, which owns nothing here.
	Live.Transcendents.Reset();
	CosmicTests::WireChain(Live);
	CosmicTests::ClearOwners(Live);
	CosmicTests::Give(Live, Other, { 0, 1 });
	Live.Civs[Other].Tier = EAscensionTier::Tier1_Planetary;
	// The owners' wheel's violet (a hue of the fallen's band) as the state's colour: the map draws it nowhere (item 8 -
	// a ring takes the meeting colour), in either view.
	const FLinearColor OtherViolet(0.63f, 0.294f, 1.f);
	Live.Civs[Other].Color = OtherViolet;
	for (FStarSystemState& System : Live.Systems)
	{
		System.ActiveHazard = FGameplayTag();
	}
	AddInstance(Live, Quiet, { Live.Systems[6].SystemID });
	// Sealed through the game's one writer (B6b: the binding - CosmicSeal::Bind).
	AddInstance(Live, Zeal, { Live.Systems[2].SystemID, Live.Systems[3].SystemID });
	TestTrue(TEXT("(the binding's writer seals the zeal)"), CosmicSeal::Bind(Live, Live.Transcendents.Num() - 1, Live.Tick));
	Live.Civs[RuledIndex].Tier = EAscensionTier::Tier1_Planetary;
	// The whole board known (Phase 3 item 8): the sight draws only what the viewer knows (UI.KnownFilter holds that).
	CosmicTests::KnowAll(Live);
	// The meeting colour of the other people: its place in the meeting order (the ruled people met first, by Install).
	const int32 OtherOrder = Live.Knowing.MetCivs.IndexOfByPredicate([&Live, Other](const FKnownMark& Mark) { return Mark.ID == Live.Civs[Other].CivID; });
	const FLinearColor OtherColour = CosmicPalette::OwnerColorOf(OtherOrder);
	Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
	Map->RefreshNow();
	auto Refreshed = [&]()
	{
		Map->RequestRefresh();
		Map->RefreshNow();
	};
	if (!TestEqual(TEXT("(one star per system)"), Map->GetStars().Num(), Live.Systems.Num()))
	{
		return false;
	}
	ULineBatchComponent* Batch = Map->FindComponentByClass<ULineBatchComponent>();
	// The sky by its own getter: the backdrop is a static mesh component too (item 8), so a lookup by class may find it.
	const UStaticMeshComponent* SkyMesh = Map->GetSky();
	const UMaterialInterface* SkyMaterial = SkyMesh ? SkyMesh->GetMaterial(0) : nullptr;
	if (!TestNotNull(TEXT("(the map draws its lines in one batch)"), Batch) || !TestNotNull(TEXT("(the sky draws through a material)"), SkyMaterial))
	{
		return false;
	}
	TestNotNull(TEXT("The sky draws through its own instance"), Cast<UMaterialInstanceDynamic>(SkyMaterial));
	auto SkyValue = [SkyMaterial]()
	{
		float Value = -1.f;
		ScalarOf(*SkyMaterial, CosmicMapLook::SkyIntensityParam, Value);
		return Value;
	};

	{
		const uint32 StateHash = Sim->ComputeStateHash();
		const uint32 WorldHash = Sim->ComputeWorldHash();

		// The first rung: closed.
		Refreshed();
		TestTrue(TEXT("Closed, the map draws the physical view"), Map->GetDrawnViewMode() == ECosmicViewMode::Physical);
		int32 Spiritual = 0;
		for (const ACosmicStarSystemActor* Star : Map->GetStars())
		{
			Spiritual += Star && Star->GetAppliedParams().ViewMode != 0.f ? 1 : 0;
		}
		TestEqual(TEXT("Closed, every star draws the physical values"), Spiritual, 0);
		TestEqual(TEXT("Closed, no sealed ring"), Map->GetDrawnSealedRingCount(), 0);
		TestEqual(TEXT("Closed, no greyed half"), Map->GetDrawnGreyedHalfCount(), 0);
		TestEqual(TEXT("Closed, the sky is the physical one"), Map->GetSkyIntensity(), 0.f);
		TestEqual(TEXT("(two owner's rings)"), Map->GetDrawnRingCount(), 2);
		TestEqual(TEXT("Closed, every lane is one line"), Batch->BatchedLines.Num(), Map->GetDrawnLaneCount() + 2 * 24);
		TestNotNull(TEXT("(closed, the lane 4-5 is one line)"), FindLine(*Batch, Live.Systems[4].Position, Live.Systems[5].Position));
		const ACosmicStarSystemActor* Zero = Map->GetStars()[0].Get();
		const TArray<const FBatchedLine*> ZeroRing = Zero ? RingLinesAround(*Batch, Zero->GetActorLocation(), Zero->GetStarRadius(), 1000.0) : TArray<const FBatchedLine*>();
		TestEqual(TEXT("(closed, star 0 has its owner's ring of 24)"), ZeroRing.Num(), 24);
		TestTrue(TEXT("(the other people is met, after the ruled one)"), OtherOrder > 0);
		TestEqual(TEXT("Closed, a people's ring is drawn in its meeting colour"), CountInColor(ZeroRing, OtherColour), ZeroRing.Num());
		TestEqual(TEXT("and never in the state's violet"), CountInColor(ZeroRing, OtherViolet), 0);

		// The development force: open at the first rung, then the rung has its say again.
		Controller->CosmicSpiritualSight(1);
		Map->RefreshNow();
		TestTrue(TEXT("Forced open, the map draws the sight at the first rung"), Map->GetDrawnViewMode() == ECosmicViewMode::Spiritual);
		Controller->CosmicSpiritualSight(0);
		Map->RefreshNow();
		TestTrue(TEXT("Given back to the rung, the map draws the physical view"), Map->GetDrawnViewMode() == ECosmicViewMode::Physical);
		TestEqual(TEXT("The force changes no state"), Sim->ComputeStateHash(), StateHash);
		TestEqual(TEXT("The force changes no world"), Sim->ComputeWorldHash(), WorldHash);
	}

	// The third rung: open.
	Live.Civs[RuledIndex].Tier = EAscensionTier::Tier3_Causal;
	const uint32 StateHash = Sim->ComputeStateHash();
	const uint32 WorldHash = Sim->ComputeWorldHash();
	const TArray<TPair<int32, int32>> Lanes = CosmicMapLook::LanesOf(Live);
	{
		Refreshed();
		TestTrue(TEXT("Open, the wanted view is drawn"), Map->GetDrawnViewMode() == ECosmicViewMode::Spiritual);
		int32 Mismatched = 0;
		for (int32 At = 0; At < Map->GetStars().Num() && At < Live.Systems.Num(); ++At)
		{
			const ACosmicStarSystemActor* Star = Map->GetStars()[At];
			const FCosmicStarParams Wanted = StarParamsOf(CosmicMapLook::LookOf(Live, Sim->GetTables(), At),
				CosmicMapLook::SpiritOf(Live, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), At), ECosmicViewMode::Spiritual);
			Mismatched += Star && SameParams(Star->GetAppliedParams(), Wanted) ? 0 : 1;
		}
		TestEqual(TEXT("Every star's values are StarParamsOf its looks"), Mismatched, 0);
		const ACosmicStarSystemActor* Six = Map->GetStars().IsValidIndex(6) ? Map->GetStars()[6].Get() : nullptr;
		const UStaticMeshComponent* SixMesh = Six ? Six->FindComponentByClass<UStaticMeshComponent>() : nullptr;
		const UMaterialInstanceDynamic* SixMaterial = SixMesh ? Cast<UMaterialInstanceDynamic>(SixMesh->GetMaterial(0)) : nullptr;
		if (TestNotNull(TEXT("(star 6 draws through its own instance)"), SixMaterial))
		{
			float Value = -1.f;
			FLinearColor Colour = FLinearColor::Transparent;
			TestTrue(TEXT("Star 6's instance holds ViewMode 1"), ScalarOf(*SixMaterial, CosmicMapLook::StarParam::ViewMode, Value) && Value == 1.f);
			TestTrue(TEXT("Star 6's instance holds Pressure 1"), ScalarOf(*SixMaterial, CosmicMapLook::StarParam::Pressure, Value) && Value == 1.f);
			TestTrue(TEXT("Star 6's instance holds ClaimState 2"), ScalarOf(*SixMaterial, CosmicMapLook::StarParam::ClaimState, Value) && Value == 2.f);
			TestTrue(TEXT("Star 6's instance holds the purple"), VectorOf(*SixMaterial, CosmicMapLook::StarParam::DominionColor, Colour) && Colour.Equals(CosmicPalette::Purple, 1e-5f));
		}
		TestEqual(TEXT("Every lane is still drawn once"), Map->GetDrawnLaneCount(), Lanes.Num());
		int32 PressedEnds = 0;
		for (const TPair<int32, int32>& Lane : Lanes)
		{
			for (const int32 End : { Lane.Key, Lane.Value })
			{
				PressedEnds += CosmicMapLook::SpiritOf(Live, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), End).Pressure > 0.f ? 1 : 0;
			}
		}
		TestEqual(TEXT("(the chain has six ends under pressure: 5, 6 and 7)"), PressedEnds, 6);
		TestEqual(TEXT("The greyed halves are the ends under pressure"), Map->GetDrawnGreyedHalfCount(), PressedEnds);
		const FVector Four = Live.Systems[4].Position;
		const FVector Five = Live.Systems[5].Position;
		const FVector Middle = 0.5 * (Four + Five);
		const FBatchedLine* FarHalf = FindLine(*Batch, Four, Middle);
		const FBatchedLine* NearHalf = FindLine(*Batch, Five, Middle);
		if (TestNotNull(TEXT("(the half from 4 to the middle of 4-5 is drawn)"), FarHalf) && TestNotNull(TEXT("(the half from 5 to the middle of 4-5 is drawn)"), NearHalf))
		{
			TestTrue(TEXT("The far half keeps its colour"), FarHalf->Color.Equals(LaneColorOf(0.f, ECosmicViewMode::Spiritual), 1e-5f));
			TestTrue(TEXT("The near half is greyed"), NearHalf->Color.Equals(LaneColorOf(0.5f, ECosmicViewMode::Spiritual), 1e-5f));
		}
		TestEqual(TEXT("One sealed ring per system of the sealed sphere"), Map->GetDrawnSealedRingCount(), 2);
		TestEqual(TEXT("One owner's ring per owned system still"), Map->GetDrawnRingCount(), 2);
		// The lines themselves (nothing is selected on this board): every lane two halves and none whole, every ring 24.
		TestEqual(TEXT("In the sight the batch holds two halves per lane and 24 segments per ring"), Batch->BatchedLines.Num(),
			2 * Lanes.Num() + 24 * (Map->GetDrawnRingCount() + Map->GetDrawnSealedRingCount()));
		TestNull(TEXT("No lane is drawn whole under its halves"), FindLine(*Batch, Four, Five));
		const ACosmicStarSystemActor* Zero = Map->GetStars()[0].Get();
		const ACosmicStarSystemActor* Two = Map->GetStars().IsValidIndex(2) ? Map->GetStars()[2].Get() : nullptr;
		const TArray<const FBatchedLine*> ZeroRing = Zero ? RingLinesAround(*Batch, Zero->GetActorLocation(), Zero->GetStarRadius(), 1000.0) : TArray<const FBatchedLine*>();
		const TArray<const FBatchedLine*> TwoRing = Two ? RingLinesAround(*Batch, Two->GetActorLocation(), Two->GetStarRadius(), 1000.0) : TArray<const FBatchedLine*>();
		const bool bZeroRing = TestEqual(TEXT("(star 0 has its owner's ring of 24 in the sight)"), ZeroRing.Num(), 24);
		const bool bTwoRing = TestEqual(TEXT("The sealed ring around star 2 has 24 segments"), TwoRing.Num(), 24);
		if (bZeroRing && bTwoRing && Zero && Two)
		{
			// The palette leaves the fallen's band out (item 8): in the sight a people's ring keeps its meeting colour.
			TestEqual(TEXT("In the sight a people's ring keeps its meeting colour"), CountInColor(ZeroRing, OtherColour), ZeroRing.Num());
			TestEqual(TEXT("The sealed ring is the Severity-0 grey, every segment"), CountInColor(TwoRing, CosmicPalette::Grey), TwoRing.Num());
			// Two ring widths outside where the owner's ring stands (the owner's ring of star 0 gives the gap and the width).
			const double OwnerGap = FVector::Dist2D(ZeroRing[0]->Start, Zero->GetActorLocation()) - Zero->GetStarRadius();
			const double SealedGap = FVector::Dist2D(TwoRing[0]->Start, Two->GetActorLocation()) - Two->GetStarRadius();
			TestEqual(TEXT("The sealed ring stands two ring widths outside the owner's ring"), SealedGap, OwnerGap + 2.0 * ZeroRing[0]->Thickness, 1.0);
			TestEqual(TEXT("and is as thick as the owner's ring"), TwoRing[0]->Thickness, ZeroRing[0]->Thickness, 1e-3f);
		}
		TestEqual(TEXT("The sight's sky is quiet"), Map->GetSkyIntensity(), 1.f);
		TestEqual(TEXT("and the sky's instance holds SpiritualIntensity 1"), SkyValue(), 1.f);

		// B6b decision 32: a bound system's words are what the fallen one held before the binding - never the word of a seal
		// or of binding on the panel.
		if (UCosmicSystemPanelWidget* HudPanel = HUD->GetSystemPanel(); TestNotNull(TEXT("(the HUD has its system panel)"), HudPanel) && ZealRow)
		{
			Controller->SelectSystem(Live.Systems[2].SystemID);
			HudPanel->Refresh();
			const UTextBlock* Mark = HudPanel->GetMarkText();
			TestEqual(TEXT("A bound system's mark says what the fallen one held before the binding"), Mark ? Mark->GetText().ToString() : FString(),
				FText::FormatNamed(CosmicUI::Label(TEXT("Sight.Bound")), TEXT("Transcendent"), ZealRow->Title).ToString());
			const FString PanelWords = CosmicTests::ReadTexts(*HudPanel) + (Mark ? Mark->GetToolTipText().ToString() : FString())
				+ (HudPanel->GetDetailText() ? HudPanel->GetDetailText()->GetToolTipText().ToString() : FString());
			TestFalse(TEXT("No word of a seal on the panel"), PanelWords.Contains(TEXT("\uBD09\uC778")));
			TestFalse(TEXT("nor of binding"), PanelWords.Contains(TEXT("\uACB0\uBC15")));
			Controller->SelectSystem(NAME_None);
		}

		// A setter marks the map; the next frame draws.
		int32 Announced = 0;
		const FDelegateHandle Probe = Controller->OnViewModeChanged.AddLambda([&Announced]() { ++Announced; });
		Controller->SetWantedViewMode(ECosmicViewMode::Physical);
		TestTrue(TEXT("A setter only marks the map"), Map->GetDrawnViewMode() == ECosmicViewMode::Spiritual);
		Map->RefreshNow();
		TestTrue(TEXT("and the next frame draws the physical view"), Map->GetDrawnViewMode() == ECosmicViewMode::Physical);
		TestEqual(TEXT("and the sky follows back"), SkyValue(), 0.f);
		TestEqual(TEXT("and no sealed ring is drawn in it"), Map->GetDrawnSealedRingCount(), 0);
		Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
		Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
		Controller->OnViewModeChanged.Remove(Probe);
		TestEqual(TEXT("Each change is announced once, and setting the same view again announces nothing"), Announced, 2);
		Map->RefreshNow();
		TestEqual(TEXT("Drawing and turning the view changes no state"), Sim->ComputeStateHash(), StateHash);
		TestEqual(TEXT("Drawing and turning the view changes no world"), Sim->ComputeWorldHash(), WorldHash);
	}

	// ---- 9. The top bar's view button ----
	{
		UCosmicTopBarWidget* Bar = HUD->GetTopBar();
		if (!TestNotNull(TEXT("The HUD has its top bar"), Bar) || !TestNotNull(TEXT("(the bar has its view button)"), Bar->GetViewButton()))
		{
			return false;
		}
		int32 Announced = 0;
		const FDelegateHandle Probe = Controller->OnViewModeChanged.AddLambda([&Announced]() { ++Announced; });
		Live.Civs[RuledIndex].Tier = EAscensionTier::Tier1_Planetary;
		Bar->Refresh();
		TestTrue(TEXT("Closed, the view button is hidden"), Bar->GetViewButton()->GetVisibility() == ESlateVisibility::Collapsed);
		Bar->PressViewButton();
		TestTrue(TEXT("A press while closed changes nothing"), Controller->GetWantedViewMode() == ECosmicViewMode::Spiritual);
		Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
		Announced = 0;
		Controller->ToggleViewMode();
		TestTrue(TEXT("The controller refuses a toggle while the sight is closed"), Controller->GetWantedViewMode() == ECosmicViewMode::Spiritual);
		TestEqual(TEXT("and announces nothing"), Announced, 0);
		Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
		Announced = 0;

		Live.Civs[RuledIndex].Tier = EAscensionTier::Tier3_Causal;
		Bar->Refresh();
		TestTrue(TEXT("Open, the view button shows and takes clicks"), ShowsForClicks(Bar->GetViewButton()->GetVisibility()));
		TestEqual(TEXT("Its label names the view drawn"), Bar->GetViewButton()->GetLabel().ToString(), CosmicUI::Label(TEXT("Sight.Spiritual")).ToString());
		Bar->PressViewButton();
		TestTrue(TEXT("A press turns the view"), Controller->GetWantedViewMode() == ECosmicViewMode::Physical);
		TestEqual(TEXT("A press is announced once"), Announced, 1);
		TestEqual(TEXT("and the label follows"), Bar->GetViewButton()->GetLabel().ToString(), CosmicUI::Label(TEXT("Sight.Physical")).ToString());
		Map->RefreshNow();
		TestTrue(TEXT("and the map draws the physical view on the next frame"), Map->GetDrawnViewMode() == ECosmicViewMode::Physical);
		Bar->PressViewButton();
		TestTrue(TEXT("A second press turns it back"), Controller->GetWantedViewMode() == ECosmicViewMode::Spiritual);
		Controller->OnViewModeChanged.Remove(Probe);
		TestEqual(TEXT("The view button's tooltip says what the sight is"), Bar->GetViewButton()->GetToolTipText().ToString(), CosmicUI::Label(TEXT("Sight.Tip")).ToString());
		for (const TCHAR* Key : { TEXT("Sight.Spiritual"), TEXT("Sight.Physical"), TEXT("Sight.Tip") })
		{
			TestFalse(*FString::Printf(TEXT("%s is no missing entry"), Key), CosmicUI::Label(Key).ToString().Contains(Placeholder));
		}
		TestEqual(TEXT("The view button is no speed button: one speed button per step"), Bar->GetSpeedButtons().Num(), static_cast<int32>(UE_ARRAY_COUNT(CosmicUI::SpeedSteps)));
	}

	// ---- 10. The panel's words: a line of their own and its tooltip, in the sight only ----
	{
		UCosmicSystemPanelWidget* Panel = HUD->GetSystemPanel();
		if (!TestNotNull(TEXT("The HUD has its system panel"), Panel) || !TestNotNull(TEXT("(the panel has its detail block)"), Panel->GetDetailText())
			|| !TestNotNull(TEXT("(the panel has its line of the sight's words)"), Panel->GetMarkText()))
		{
			return false;
		}
		const UTextBlock* Detail = Panel->GetDetailText();
		const UTextBlock* Mark = Panel->GetMarkText();
		Live.Systems[7].ActiveHazard = Tag(TEXT("Hazard.Claim"));
		Live.Systems[7].HazardTicksRemaining = 100;
		Controller->SelectSystem(Live.Systems[7].SystemID);
		Panel->Refresh();
		const FString ClaimWords = UCosmicSystemPanelWidget::SpiritTipOf(CosmicMapLook::SpiritOf(Live, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 7), Sim->GetTables()).ToString();
		TestFalse(TEXT("(the claim has its words)"), ClaimWords.IsEmpty());
		const TSharedPtr<SWidget> SlateMark = Mark->GetCachedWidget();
		const TSharedPtr<SWidget> SlateDetail = Detail->GetCachedWidget();
		TestTrue(TEXT("(the spiritual tooltip exists)"), SlateMark.IsValid() && SlateMark->GetToolTip().IsValid());
		TestEqual(TEXT("A claim's tooltip is SpiritTipOf its look"), Mark->GetToolTipText().ToString(), ClaimWords);
		TestEqual(TEXT("and the panel says the same words in a line of their own"), Mark->GetText().ToString(), ClaimWords);
		TestTrue(TEXT("which shows and takes the mouse for its tooltip"), Mark->GetVisibility() == ESlateVisibility::Visible);
		TestFalse(TEXT("The detail lines do not hold the words"), Detail->GetText().ToString().Contains(ClaimWords));
		TestTrue(TEXT("and carry no tooltip: the words never cover the people's band"), Detail->GetToolTipText().IsEmpty());
		TestFalse(TEXT("(nor a Slate tooltip)"), SlateDetail.IsValid() && SlateDetail->GetToolTip().IsValid());
		TestTrue(TEXT("The words name the title with its particle"), ClaimWords.Contains(FString(QuietTitle) + ParticleAfterVowel));
		TestFalse(TEXT("and leave no slot or modifier"), ClaimWords.Contains(TEXT("{")) || ClaimWords.Contains(TEXT("|")));
		TestFalse(TEXT("and hold no digit"), HasAsciiDigit(ClaimWords));

		// The selection ring around 7 (no owner, no sealed sphere there): the palette's white.
		Map->RefreshNow();
		const ACosmicStarSystemActor* Seven = Map->GetStars().IsValidIndex(7) ? Map->GetStars()[7].Get() : nullptr;
		const TArray<const FBatchedLine*> SelectionRing = Seven ? RingLinesAround(*Batch, Seven->GetActorLocation(), Seven->GetStarRadius(), 1000.0) : TArray<const FBatchedLine*>();
		TestEqual(TEXT("(the selection ring around 7 has 32 segments)"), SelectionRing.Num(), 32);
		TestEqual(TEXT("The selection ring is the palette's white"), CountInColor(SelectionRing, CosmicPalette::White), SelectionRing.Num());

		Controller->SelectSystem(Live.Systems[6].SystemID);
		Panel->Refresh();
		const FString HeldWords = Mark->GetToolTipText().ToString();
		TestFalse(TEXT("The holder's words differ from the claimant's"), HeldWords.Equals(ClaimWords, ESearchCase::CaseSensitive));
		TestTrue(TEXT("and name the title too"), HeldWords.Contains(QuietTitle));

		// 5 is only pressed (one lane from the quiet one's sphere, nobody's).
		Controller->SelectSystem(Live.Systems[5].SystemID);
		Panel->Refresh();
		const FString PressedWords = Mark->GetText().ToString();
		TestEqual(TEXT("A system that is only pressed says who presses"), PressedWords,
			UCosmicSystemPanelWidget::SpiritTipOf(CosmicMapLook::SpiritOf(Live, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 5), Sim->GetTables()).ToString());
		TestTrue(TEXT("naming the title with its particle"), PressedWords.Contains(FString(QuietTitle) + ParticleAfterVowel));
		TestFalse(TEXT("in words of its own"), PressedWords.Equals(HeldWords, ESearchCase::CaseSensitive) || PressedWords.Equals(ClaimWords, ESearchCase::CaseSensitive));
		TestEqual(TEXT("and as its tooltip"), Mark->GetToolTipText().ToString(), PressedWords);

		// The physical view: the HUD redraws the panel through the controller's announcement (no refresh by hand).
		Controller->SelectSystem(Live.Systems[7].SystemID);
		Panel->Refresh();
		Controller->SetWantedViewMode(ECosmicViewMode::Physical);
		TestTrue(TEXT("The physical view shows no tooltip"), Mark->GetToolTipText().IsEmpty());
		TestFalse(TEXT("and no Slate tooltip"), SlateMark.IsValid() && SlateMark->GetToolTip().IsValid());
		TestTrue(TEXT("nor the line"), Mark->GetVisibility() == ESlateVisibility::Collapsed);
		TestTrue(TEXT("which carries no words"), Mark->GetText().IsEmpty());
		Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
		const IToolTip* TipBefore = SlateMark.IsValid() ? SlateMark->GetToolTip().Get() : nullptr;
		TestNotNull(TEXT("(the spiritual tooltip is back)"), TipBefore);
		Panel->Refresh();
		TestTrue(TEXT("A refresh keeps the tooltip object"), SlateMark.IsValid() && SlateMark->GetToolTip().Get() == TipBefore);

		// SpiritTipOf by itself.
		const FCosmicDataTables& Held = Sim->GetTables();
		FCosmicSpiritLook Look;
		TestTrue(TEXT("(no mark has no words)"), UCosmicSystemPanelWidget::SpiritTipOf(Look, Held).IsEmpty());
		Look.MarkRow = Untitled;
		Look.ClaimState = ECosmicClaimState::Held;
		TestTrue(TEXT("An empty title gives no words"), UCosmicSystemPanelWidget::SpiritTipOf(Look, Held).IsEmpty());
		Look.MarkRow = FName(TEXT("trans_test_nobody"));
		TestTrue(TEXT("(a row the tables do not hold gives no words)"), UCosmicSystemPanelWidget::SpiritTipOf(Look, Held).IsEmpty());
		Look.MarkRow = Quiet;
		const FString HeldTip = UCosmicSystemPanelWidget::SpiritTipOf(Look, Held).ToString();
		Look.ClaimState = ECosmicClaimState::Bound;
		const FString BoundTip = UCosmicSystemPanelWidget::SpiritTipOf(Look, Held).ToString();
		TestTrue(TEXT("The bound words name the title with its particle"), BoundTip.Contains(FString(QuietTitle) + ParticleAfterVowel));
		TestFalse(TEXT("Bound differs from Held"), BoundTip.Equals(HeldTip, ESearchCase::CaseSensitive));
		Look.ClaimState = ECosmicClaimState::None;
		Look.bMarkPresses = true;
		const FString PressedTip = UCosmicSystemPanelWidget::SpiritTipOf(Look, Held).ToString();
		TestTrue(TEXT("The pressed words name the title with its particle"), PressedTip.Contains(FString(QuietTitle) + ParticleAfterVowel));
		TestFalse(TEXT("Pressure alone differs from Held"), PressedTip.Equals(HeldTip, ESearchCase::CaseSensitive));
		Look.ClaimState = ECosmicClaimState::Bound;
		TestEqual(TEXT("A bound system under another's pressure says the presser presses"), UCosmicSystemPanelWidget::SpiritTipOf(Look, Held).ToString(), PressedTip);
		Look.MarkRow = Zeal;
		Look.ClaimState = ECosmicClaimState::Claimed;
		Look.bMarkPresses = false;
		const FString ZealClaim = UCosmicSystemPanelWidget::SpiritTipOf(Look, Held).ToString();
		TestTrue(TEXT("A consonant-final title takes the other particle"), ZealClaim.Contains(FString(ZealTitle) + ParticleAfterConsonant));
		for (const TCHAR* Key : { TEXT("Sight.Claimed"), TEXT("Sight.Held"), TEXT("Sight.Bound"), TEXT("Sight.Pressed") })
		{
			const FString Words = CosmicUI::Label(Key).ToString();
			TestFalse(*FString::Printf(TEXT("%s is no missing entry"), Key), Words.Contains(Placeholder));
			TestTrue(*FString::Printf(TEXT("%s has the title's slot"), Key), Words.Contains(TEXT("{Transcendent}")));
			// Each phrase ends on the place (section 8.4: a claim is never told with a size - no phrase leaves its object to
			// the people's band above it).
			TestTrue(*FString::Printf(TEXT("%s names a place"), Key), Words.EndsWith(PlaceWord, ESearchCase::CaseSensitive));
		}
	}

	// ---- The deposition, the opening again, and a load ----
	{
		// Deposed: a first-rung wanderer sees from the first rung again.
		Live.Player.Role = EPlayerRole::Wanderer;
		Live.Player.RuledCivID = NAME_None;
		Live.Calling.PersonalTier = EAscensionTier::Tier1_Planetary;
		Controller->SetWantedViewMode(ECosmicViewMode::Spiritual);
		// A frame of the controller (its PlayerTick follows the sight; public on the engine's class).
		APlayerController* const Frame = Controller;
		Frame->PlayerTick(0.f);
		TestFalse(TEXT("A deposed ruler's sight closes"), Controller->IsSpiritualSightOpen());
		Refreshed();
		TestTrue(TEXT("and the map falls back to the physical view"), Map->GetDrawnViewMode() == ECosmicViewMode::Physical);

		// The wanderer climbs back: the wanted view returns to the sight.
		Controller->SetWantedViewMode(ECosmicViewMode::Physical);
		Frame->PlayerTick(0.f);
		TestTrue(TEXT("(while closed the wanted view stays as set)"), Controller->GetWantedViewMode() == ECosmicViewMode::Physical);
		Live.Calling.PersonalTier = EAscensionTier::Tier3_Causal;
		Frame->PlayerTick(0.f);
		TestTrue(TEXT("When the sight opens again, the controller's next frame returns the wanted view to Spiritual"), Controller->GetWantedViewMode() == ECosmicViewMode::Spiritual);

		// A third-rung load with the wanted view physical draws the sight on its first refresh.
		Live.Player.Role = EPlayerRole::Ruler;
		Live.Player.RuledCivID = Live.Civs[RuledIndex].CivID;
		const FGalaxyState Saved = Live;
		Controller->SetWantedViewMode(ECosmicViewMode::Physical);
		Map->RefreshNow();
		Sim->LoadState(Saved);
		Map->RefreshNow();
		TestTrue(TEXT("A third-rung load draws the sight on its first refresh"), Map->GetDrawnViewMode() == ECosmicViewMode::Spiritual);
		TestTrue(TEXT("and the wanted view is Spiritual again"), Controller->GetWantedViewMode() == ECosmicViewMode::Spiritual);

		// A new galaxy returns the wanted view to the default.
		Controller->SetWantedViewMode(ECosmicViewMode::Physical);
		Sim->NewGalaxy(42, 12, 3);
		Map->RefreshNow();
		TestTrue(TEXT("A new galaxy returns the wanted view to Spiritual"), Controller->GetWantedViewMode() == ECosmicViewMode::Spiritual);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

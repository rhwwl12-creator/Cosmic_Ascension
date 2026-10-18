// Cosmic Ascension - automation tests of the display of knowing, the backdrop and the galaxy panel (Presentation).
// Design: guideline.md section 3.1, 5.7, 5.11, 6.3, 6.4, 8.1, 8.3, 9.3, 11 Phase 3 (item 8).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/LineBatchComponent.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Core/CosmicSeed.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "HAL/FileManager.h"
#include "Internationalization/StringTableCore.h"
#include "Log/CosmicLogSubsystem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameters.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicGalaxyPanelWidget.h"
#include "UI/CosmicLogFeedWidget.h"
#include "UI/CosmicPalette.h"
#include "UI/CosmicSystemPanelWidget.h"
#include "UI/CosmicTopBarWidget.h"
#include "UI/CosmicUILabels.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "World/CosmicCameraPawn.h"
#include "World/CosmicGalaxyMapActor.h"
#include "World/CosmicGameMode.h"
#include "World/CosmicMapLook.h"
#include "World/CosmicMapPick.h"
#include "World/CosmicStarSystemActor.h"

#include <limits>

namespace CosmicKnownFilterTests
{
	using CosmicTests::ReadTexts;

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

	FString Words(const TCHAR* Key)
	{
		return CosmicUI::Label(Key).ToString();
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

	// ---- The test board ----

	// The markers the board gives the peoples' names (FText is outside both hashes): a name search is exact.
	const TCHAR* const RuledMarker = TEXT("KF-ruled");
	const TCHAR* const MetMarker = TEXT("KF-met");
	const TCHAR* const UnmetMarker = TEXT("KF-unmet");
	const TCHAR* const FarMarker = TEXT("KF-far");
	// A colour in no palette: every people's FCivilizationState::Color on the board (drawn nowhere).
	const FLinearColor StateColour(0.123f, 0.456f, 0.789f);

	const FName Seen(TEXT("trans_test_seen"));
	const FName Bound(TEXT("trans_test_bound"));

	// Two fallen ones of the fourth rung and of full depth, with Hangul titles (escaped: the source files are ASCII).
	UDataTable* TestRows()
	{
		UDataTable* Table = CosmicTests::NewTable(FTranscendentDef::StaticStruct());
		auto Row = [](const TCHAR* Doctrine, const TCHAR* Title)
		{
			FTranscendentDef Def;
			Def.Allegiance = Tag(TEXT("Realm.Abyssal"));
			Def.Doctrine = Tag(Doctrine);
			Def.Title = FText::AsCultureInvariant(Title);
			Def.Tier = EAscensionTier::Tier4_Transcendent;
			Def.Depth = 1.f;
			return Def;
		};
		Table->AddRow(Seen, Row(TEXT("Doctrine.Silence"), TEXT("\uACE0\uC694\uD55C \uC790")));
		Table->AddRow(Bound, Row(TEXT("Doctrine.Zeal"), TEXT("\uD0C0\uB294 \uC785")));
		return Table;
	}

	// The peoples of the board by role: R the ruled one, A met (third), U not met, F met (second) and far above.
	struct FBoardPeoples
	{
		int32 R = INDEX_NONE;
		int32 A = INDEX_NONE;
		int32 U = INDEX_NONE;
		int32 F = INDEX_NONE;
	};

	// The board of UI.KnownFilter (section 6.4): twelve systems wired as CosmicTests::WireChain, four peoples, two test
	// fallen ones; known 0, 2, 3, 5, 6, 8, 9 and met R, F, A in that order. False when the state does not have the shape.
	bool MakeBoard(FGalaxyState& Live, const FCosmicStateIndex& Index, FBoardPeoples& Out)
	{
		if (Live.Systems.Num() != 12 || Live.Civs.Num() != 4)
		{
			return false;
		}
		Live.Transcendents.Reset();
		CosmicTests::WireChain(Live);
		CosmicTests::ClearOwners(Live);
		for (FStarSystemState& System : Live.Systems)
		{
			System.ActiveHazard = FGameplayTag();
			System.HazardTicksRemaining = 0;
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
			System.ResponseMegaproject = NAME_None;
			System.ResponseProgress = 0.f;
			System.StarClass = Tag(TEXT("Star.Class.G"));
		}
		Live.Systems[5].StarClass = Tag(TEXT("Star.Class.O"));
		Live.Systems[4].StarClass = Tag(TEXT("Star.Class.K"));
		Out.R = Index.FindCiv(Live, Live.Player.RuledCivID);
		if (!Live.Civs.IsValidIndex(Out.R))
		{
			return false;
		}
		TArray<int32> Others;
		for (int32 At = 0; At < Live.Civs.Num(); ++At)
		{
			if (At != Out.R)
			{
				Others.Add(At);
			}
		}
		Out.A = Others[0];
		Out.U = Others[1];
		Out.F = Others[2];
		Live.Civs[Out.R].CivName = FText::AsCultureInvariant(RuledMarker);
		Live.Civs[Out.A].CivName = FText::AsCultureInvariant(MetMarker);
		Live.Civs[Out.U].CivName = FText::AsCultureInvariant(UnmetMarker);
		Live.Civs[Out.F].CivName = FText::AsCultureInvariant(FarMarker);
		for (FCivilizationState& Civ : Live.Civs)
		{
			Civ.Color = StateColour;
			Civ.Tier = EAscensionTier::Tier1_Planetary;
		}
		Live.Civs[Out.F].Tier = EAscensionTier::Tier4_Transcendent;
		Live.Calling.PersonalTier = EAscensionTier::Tier1_Planetary;
		CosmicTests::Give(Live, Out.R, { 0 });
		CosmicTests::Give(Live, Out.A, { 1, 2 });
		CosmicTests::Give(Live, Out.U, { 3, 4 });
		CosmicTests::Give(Live, Out.F, { 8 });
		// Give keeps a generated capital that happens to be among the systems it hands over: every capital by hand.
		Live.Civs[Out.R].HomeSystemID = Live.Systems[0].SystemID;
		Live.Civs[Out.A].HomeSystemID = Live.Systems[1].SystemID;
		Live.Civs[Out.U].HomeSystemID = Live.Systems[3].SystemID;
		Live.Civs[Out.F].HomeSystemID = Live.Systems[8].SystemID;
		// The works and the hazards.
		const FName Dyson(TEXT("project_dyson"));
		for (const int32 At : { 2, 3, 4 })
		{
			Live.Systems[At].ActiveMegaproject = Dyson;
			Live.Systems[At].MegaprojectProgress = 120.f;
		}
		for (const int32 At : { 3, 4 })
		{
			Live.Systems[At].ActiveHazard = Tag(TEXT("Hazard.AIAwakening"));
			Live.Systems[At].HazardTicksRemaining = 1000;
		}
		// Destroyed: 9 known, 11 unknown (their lanes stay listed; LanesOf drops dead ends).
		Live.Systems[9].StarClass = FGameplayTag();
		Live.Systems[11].StarClass = FGameplayTag();
		auto IdOf = [&Live](int32 At) { return Live.Systems[At].SystemID; };
		CosmicTests::AddInstance(Live, Seen, { IdOf(3), IdOf(4), IdOf(5) });
		CosmicTests::AddInstance(Live, Bound, { IdOf(6), IdOf(7) }).bSealed = true;
		CosmicTests::SetKnowing(Live, { 0, 2, 3, 5, 6, 8, 9 }, { Out.R, Out.F, Out.A });
		return true;
	}

	const int32 KnownOnBoard[] = { 0, 2, 3, 5, 6, 8, 9 };
	const int32 UnknownOnBoard[] = { 1, 4, 7, 10, 11 };

	// ---- Lines of the map ----

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

	int32 CountInColor(const TArray<const FBatchedLine*>& Lines, const FLinearColor& Color)
	{
		int32 Count = 0;
		for (const FBatchedLine* Line : Lines)
		{
			Count += Line && Line->Color.Equals(Color, 1e-5f) ? 1 : 0;
		}
		return Count;
	}

	// The owner's ring of a star (the 24 segments just outside it), as the batch holds it now.
	TArray<const FBatchedLine*> RingOf(const ACosmicGalaxyMapActor& Map, int32 StarIndex)
	{
		const ULineBatchComponent* Batch = Map.FindComponentByClass<ULineBatchComponent>();
		const ACosmicStarSystemActor* Star = Map.GetStars().IsValidIndex(StarIndex) ? Map.GetStars()[StarIndex].Get() : nullptr;
		if (!Batch || !Star)
		{
			return TArray<const FBatchedLine*>();
		}
		// The owner's ring stands RingGap (110) outside the star; the sealed and selection rings further out are not it.
		const TArray<const FBatchedLine*> Around = RingLinesAround(*Batch, Star->GetActorLocation(), Star->GetStarRadius(), 1000.0);
		TArray<const FBatchedLine*> Ring;
		for (const FBatchedLine* Line : Around)
		{
			if (FMath::Abs(FVector::Dist2D(Line->Start, Star->GetActorLocation()) - (Star->GetStarRadius() + 110.0)) < 1.0)
			{
				Ring.Add(Line);
			}
		}
		return Ring;
	}

	bool SphereShows(const ACosmicGalaxyMapActor& Map, int32 StarIndex)
	{
		const ACosmicStarSystemActor* Star = Map.GetStars().IsValidIndex(StarIndex) ? Map.GetStars()[StarIndex].Get() : nullptr;
		const UStaticMeshComponent* Sphere = Star ? Star->FindComponentByClass<UStaticMeshComponent>() : nullptr;
		return Sphere && Sphere->IsVisible();
	}

	// The glow a backdrop instance carries now; -1 for an instance it does not have.
	float GlowOf(const UInstancedStaticMeshComponent& Backdrop, int32 Instance)
	{
		const int32 At = Instance * Backdrop.NumCustomDataFloats;
		return (Backdrop.NumCustomDataFloats > 0 && Backdrop.PerInstanceSMCustomData.IsValidIndex(At)) ? Backdrop.PerInstanceSMCustomData[At] : -1.f;
	}

	// ---- Colour arithmetic of the test (outside every hash: the cube root is allowed here) ----

	// OKLab of a linear colour.
	FVector OkLab(const FLinearColor& C)
	{
		const double L = 0.4122214708 * C.R + 0.5363325363 * C.G + 0.0514459929 * C.B;
		const double M = 0.2119034982 * C.R + 0.6806995451 * C.G + 0.1073969566 * C.B;
		const double S = 0.0883024619 * C.R + 0.2817188376 * C.G + 0.6299787005 * C.B;
		const double Lr = FMath::Pow(FMath::Max(L, 0.0), 1.0 / 3.0);
		const double Mr = FMath::Pow(FMath::Max(M, 0.0), 1.0 / 3.0);
		const double Sr = FMath::Pow(FMath::Max(S, 0.0), 1.0 / 3.0);
		return FVector(0.2104542553 * Lr + 0.7936177850 * Mr - 0.0040720468 * Sr,
			1.9779984951 * Lr - 2.4285922050 * Mr + 0.4505937099 * Sr,
			0.0259040371 * Lr + 0.7827717662 * Mr - 0.8086757660 * Sr);
	}

	double DeltaE(const FLinearColor& A, const FLinearColor& B)
	{
		return FVector::Dist(OkLab(A), OkLab(B));
	}

	// The light a line colour gives on the screen. The map's line batch writes a colour's numbers as sRGB bytes (measured on
	// the off-screen screenshots: teal 0.10/0.80/0.70 shows as 25/204/178), so the owners' and the lanes' numbers are
	// display values; decoded here (the sRGB curve) to linear light before OKLab. The material and widget colours (the
	// four, the warnings, the dead grey) are linear already.
	FLinearColor LineLight(const FLinearColor& C)
	{
		auto Decode = [](float V) { return V <= 0.04045f ? V / 12.92f : FMath::Pow((V + 0.055f) / 1.055f, 2.4f); };
		return FLinearColor(Decode(C.R), Decode(C.G), Decode(C.B), C.A);
	}

	// Machado, Oliveira and Fernandes (2009), severity 1, applied to linear light and clamped to 0..1.
	const double Deuteranopia[3][3] = { { 0.367322, 0.860646, -0.227968 }, { 0.280085, 0.672501, 0.047413 }, { -0.011820, 0.042940, 0.968881 } };
	const double Protanopia[3][3] = { { 0.152286, 1.052583, -0.204868 }, { 0.114503, 0.786281, 0.099216 }, { -0.003882, -0.048116, 1.051998 } };

	FLinearColor SeenThrough(const double (&M)[3][3], const FLinearColor& C)
	{
		auto Row = [&](int32 I) { return FMath::Clamp(M[I][0] * C.R + M[I][1] * C.G + M[I][2] * C.B, 0.0, 1.0); };
		return FLinearColor(static_cast<float>(Row(0)), static_cast<float>(Row(1)), static_cast<float>(Row(2)));
	}

	// The verdict words of the schema (words.verdict of Tools/data/schema.json, the list the checker holds the texts to).
	// bFileRead says whether the file was read at all; a file that was read but holds no list gives an empty one, which the
	// caller asserts against.
	TArray<FString> VerdictWords(bool& bFileRead)
	{
		TArray<FString> Out;
		FString Text;
		bFileRead = FFileHelper::LoadFileToString(Text, *(FPaths::ProjectDir() / TEXT("Tools/data/schema.json")));
		if (!bFileRead)
		{
			return Out;
		}
		TSharedPtr<FJsonObject> Schema;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
		const TSharedPtr<FJsonObject>* WordLists = nullptr;
		const TArray<TSharedPtr<FJsonValue>>* Verdict = nullptr;
		if (FJsonSerializer::Deserialize(Reader, Schema) && Schema.IsValid() && Schema->TryGetObjectField(TEXT("words"), WordLists)
			&& WordLists && (*WordLists)->TryGetArrayField(TEXT("verdict"), Verdict) && Verdict)
		{
			for (const TSharedPtr<FJsonValue>& Value : *Verdict)
			{
				FString Word;
				if (Value.IsValid() && Value->TryGetString(Word))
				{
					Out.Add(Word);
				}
			}
		}
		return Out;
	}

	bool ContainsAny(const FString& Text, const TArray<FString>& Needles)
	{
		for (const FString& Needle : Needles)
		{
			if (!Needle.IsEmpty() && Text.Contains(Needle, ESearchCase::CaseSensitive))
			{
				return true;
			}
		}
		return false;
	}

	// The panel's detail lines, one per entry.
	TArray<FString> DetailLines(const UCosmicSystemPanelWidget& Panel)
	{
		TArray<FString> Lines;
		if (Panel.GetDetailText())
		{
			Panel.GetDetailText()->GetText().ToString().ParseIntoArray(Lines, TEXT("\n"));
		}
		return Lines;
	}

	// Every tooltip the panel carries now (its buttons and its mark line).
	FString PanelTooltips(const UCosmicSystemPanelWidget& Panel)
	{
		FString Tips;
		for (const UCosmicActionButton* Button : Panel.GetActionButtons())
		{
			if (Button)
			{
				Tips += Button->GetToolTipText().ToString();
				Tips += TEXT("\n");
			}
		}
		if (Panel.GetMarkText())
		{
			Tips += Panel.GetMarkText()->GetToolTipText().ToString();
		}
		return Tips;
	}

	// Whether any people line (any band) stands among Lines.
	bool AnyPeopleLine(const TArray<FString>& Lines)
	{
		const UEnum* Bands = StaticEnum<EPopulationBand>();
		for (int32 At = 0; At < Bands->NumEnums() - 1; ++At)
		{
			if (Lines.Contains(CosmicUI::PeopleLine(static_cast<EPopulationBand>(Bands->GetValueByIndex(At))).ToString()))
			{
				return true;
			}
		}
		return false;
	}

	// The keys item 8 added to ST_CA (section 6.4): the panel's two, the twelve life words, the four kinds, the galaxy's
	// line and the hint.
	TArray<const TCHAR*> NewKeys()
	{
		TArray<const TCHAR*> Keys = { TEXT("Panel.Life"), TEXT("Panel.OtherOwner"), TEXT("Galaxy.Kind.Dwarf"), TEXT("Galaxy.Kind.Spiral"),
			TEXT("Galaxy.Kind.GrandSpiral"), TEXT("Galaxy.Kind.Elliptical"), TEXT("Galaxy.Named"), TEXT("Galaxy.Hint") };
		for (const TCHAR* Key : CosmicUI::LifeWordKeys)
		{
			Keys.Add(Key);
		}
		return Keys;
	}
}

// The display of knowing (section 6.4, 8.1): one reading of Knowing for the map, the panels and the selection; a system
// the viewer does not know is a backdrop point; an owner the viewer is not shown has no colour, name or works; lanes only
// between known ends; the meeting-order palette; the parity of the map and the scope of telling; the feed and the top
// bar; and the development aid CosmicSeeAll.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIKnownFilterTest, "Cosmic_Ascension.UI.KnownFilter", CosmicTests::Flags)
bool FCosmicUIKnownFilterTest::RunTest(const FString& Parameters)
{
	using namespace CosmicKnownFilterTests;
	using CosmicMapLook::KnownViewOf;
	using CosmicMapLook::LookOf;
	using CosmicMapLook::ShownLookOf;

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
	UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	if (!TestNotNull(TEXT("The game mode spawned the map"), Map) || !TestNotNull(TEXT("The first player controller is the game's"), Controller)
		|| !TestNotNull(TEXT("The controller has the HUD"), HUD) || !TestNotNull(TEXT("(the log subsystem runs)"), Log))
	{
		return false;
	}
	const FCosmicDataTables GameTables = Sim->GetTables();
	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(GameTables);
		Controller->CosmicSeeAll(0);
		Controller->CosmicSpiritualSight(0);
		Controller->SelectSystem(NAME_None);
		Log->SetTellAll(true);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Sim->NewGalaxy(42, 5, 3);
		Map->RefreshNow();
	};
	const FString& Placeholder = FStringTableEntry::GetPlaceholderSourceString();

	// ---- 1.1 The game's galaxy at tick 0: the birth system and the ruled people alone ----
	{
		Sim->NewGalaxy(42, 0, 0);
		const FGalaxyState& State = Sim->GetState();
		const FCosmicKnownView View = KnownViewOf(State, Sim->GetStateIndex(), Sim->GetViewerTier(), false);
		const int32 Ruled = Sim->GetStateIndex().FindCiv(State, State.Player.RuledCivID);
		const int32 Home = State.Civs.IsValidIndex(Ruled) ? Sim->GetStateIndex().FindSystem(State, State.Civs[Ruled].HomeSystemID) : INDEX_NONE;
		int32 KnownCount = 0;
		for (int32 At = 0; At < State.Systems.Num(); ++At)
		{
			KnownCount += View.IsKnown(At) ? 1 : 0;
		}
		TestEqual(TEXT("At the start only the birth system is known"), KnownCount, 1);
		TestTrue(TEXT("and it is the ruled people's capital"), View.IsKnown(Home));
		TestEqual(TEXT("The ruled people is shown first at the start"), View.ShownOrderOf(Ruled), 0);
		int32 OthersShown = 0;
		for (int32 At = 0; At < State.Civs.Num(); ++At)
		{
			OthersShown += (At != Ruled && View.ShownOrderOf(At) != INDEX_NONE) ? 1 : 0;
		}
		TestEqual(TEXT("and no other people is shown at the start"), OthersShown, 0);
	}

	// ---- The board ----
	Sim->SetTables(CosmicTests::WithTranscendents(GameTables, TestRows()));
	Sim->NewGalaxy(42, 12, 4);
	FGalaxyState& Live = Sim->GetMutableState();
	FBoardPeoples P;
	if (!TestTrue(TEXT("(the board has twelve systems and four peoples)"), MakeBoard(Live, Sim->GetStateIndex(), P)))
	{
		return false;
	}
	Sim->RebuildStateIndex();
	// The board never ticks (a sight stage would rewrite Knowing); every live section starts from this snapshot.
	const FGalaxyState Board = Live;
	auto Restore = [&]()
	{
		Sim->LoadState(Board);
		Map->RefreshNow();
	};
	auto Refreshed = [&]()
	{
		Map->RequestRefresh();
		Map->RefreshNow();
	};
	const FCosmicDataTables& Tables = Sim->GetTables();
	auto IdOf = [&Board](int32 At) { return Board.Systems[At].SystemID; };

	// A pure copy with its own index.
	FGalaxyState Pure = Board;
	FCosmicStateIndex PureIndex;
	PureIndex.Rebuild(Pure);
	const FCosmicKnownView View = KnownViewOf(Pure, PureIndex, EAscensionTier::Tier1_Planetary, false);
	const FCosmicKnownView SeeAll = KnownViewOf(Pure, PureIndex, EAscensionTier::Tier1_Planetary, true);
	auto Shown = [&](const FGalaxyState& State, const FCosmicKnownView& InView, int32 At)
	{
		return ShownLookOf(LookOf(State, Tables, At), State, InView, At);
	};

	// ---- 1.2 The view on the board ----
	TestTrue(TEXT("A system Knowing holds is known"), View.IsKnown(2));
	TestFalse(TEXT("A system Knowing does not hold is not known"), View.IsKnown(1));
	TestFalse(TEXT("An index below the range is not known"), View.IsKnown(-1));
	TestFalse(TEXT("An index past the range is not known"), View.IsKnown(99));
	TestTrue(TEXT("See-all knows a system Knowing does not hold"), SeeAll.IsKnown(1));

	// ---- 1.3 Meeting order and the gap ----
	TestEqual(TEXT("The ruled people is shown first"), View.ShownOrderOf(P.R), 0);
	TestEqual(TEXT("A people met third takes the third place, not its place in the state"), View.ShownOrderOf(P.A), 2);
	TestEqual(TEXT("A people not met is not shown"), View.ShownOrderOf(P.U), static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("A met people three rungs above the viewer is not shown"), View.ShownOrderOf(P.F), static_cast<int32>(INDEX_NONE));
	{
		FGalaxyState Closer = Pure;
		Closer.Civs[P.F].Tier = EAscensionTier::Tier3_Causal;
		TestEqual(TEXT("A met people two rungs above is shown"), KnownViewOf(Closer, PureIndex, EAscensionTier::Tier1_Planetary, false).ShownOrderOf(P.F), 1);
	}
	TestEqual(TEXT("Under see-all a people not met follows the met ones"), SeeAll.ShownOrderOf(P.U), 3);
	TestEqual(TEXT("Under see-all a people far above is shown"), SeeAll.ShownOrderOf(P.F), 1);

	// ---- 1.4 A gone people keeps its place ----
	{
		FGalaxyState Gone = Pure;
		const FName AID = Gone.Civs[P.A].CivID;
		Gone.Civs.RemoveAt(P.F);
		FCosmicStateIndex GoneIndex;
		GoneIndex.Rebuild(Gone);
		const int32 AAfter = GoneIndex.FindCiv(Gone, AID);
		TestEqual(TEXT("A people gone keeps its place, so no colour moves"), KnownViewOf(Gone, GoneIndex, EAscensionTier::Tier1_Planetary, false).ShownOrderOf(AAfter), 2);
	}

	// ---- 1.5 IsShownPeople by hand ----
	{
		using CosmicMapLook::IsShownPeople;
		TestTrue(TEXT("IsShownPeople: the ruled people is shown"), IsShownPeople(true, Pure.Civs[P.R], EAscensionTier::Tier1_Planetary));
		TestFalse(TEXT("IsShownPeople: a people not met is not"), IsShownPeople(false, Pure.Civs[P.U], EAscensionTier::Tier1_Planetary));
		TestFalse(TEXT("IsShownPeople: a met people three rungs above is not"), IsShownPeople(true, Pure.Civs[P.F], EAscensionTier::Tier1_Planetary));
		FCivilizationState Two = Pure.Civs[P.F];
		Two.Tier = EAscensionTier::Tier3_Causal;
		TestTrue(TEXT("IsShownPeople: a met people two rungs above is"), IsShownPeople(true, Two, EAscensionTier::Tier1_Planetary));
	}

	// ---- 1.6 OwnerShownOf ----
	{
		using CosmicMapLook::OwnerShownOf;
		TestTrue(TEXT("A shown owner's known system: Shown"), OwnerShownOf(Pure, View, 2) == ECosmicOwnerShown::Shown);
		TestTrue(TEXT("A system of a people not met: Hidden"), OwnerShownOf(Pure, View, 3) == ECosmicOwnerShown::Hidden);
		TestTrue(TEXT("A system of a people far above: Hidden"), OwnerShownOf(Pure, View, 8) == ECosmicOwnerShown::Hidden);
		TestTrue(TEXT("A wild system: None"), OwnerShownOf(Pure, View, 5) == ECosmicOwnerShown::None);
		FGalaxyState Stranger = Pure;
		Stranger.Systems[6].OwnerCivID = FName(TEXT("civ_99"));
		TestTrue(TEXT("An owner the state does not hold is no owner"), OwnerShownOf(Stranger, View, 6) == ECosmicOwnerShown::None);
		FGalaxyState DeadOwned = Pure;
		DeadOwned.Systems[9].OwnerCivID = DeadOwned.Civs[P.A].CivID;
		TestTrue(TEXT("A destroyed system shows no owner, even one its slot names"), OwnerShownOf(DeadOwned, View, 9) == ECosmicOwnerShown::None);
		TestTrue(TEXT("A system the viewer does not know shows no owner"), OwnerShownOf(Pure, View, 1) == ECosmicOwnerShown::None);
	}

	// ---- 2 The shown look ----
	{
		const FCosmicSystemLook World4 = LookOf(Pure, Tables, 4);
		TestTrue(TEXT("(the world's look of 4 has a name)"), !World4.Name.IsEmpty());
		TestTrue(TEXT("(the world's look of 4 has an owner)"), World4.bOwned);
		TestTrue(TEXT("(the world's look of 4 has a hazard)"), World4.bHazard);
		TestTrue(TEXT("(the world's look of 4 has a bar)"), World4.ConstructionShare > 0.f);
		TestTrue(TEXT("(the world's look of 4 is a K star)"), World4.StarScale < 1.f);
		const FCosmicSystemLook Four = Shown(Pure, View, 4);
		TestFalse(TEXT("An unknown system's look is not known"), Four.bKnown);
		TestTrue(TEXT("and has no name"), Four.Name.IsEmpty());
		TestFalse(TEXT("no owner"), Four.bOwned);
		TestFalse(TEXT("no hazard"), Four.bHazard);
		TestEqual(TEXT("no construction bar"), Four.ConstructionShare, -1.f);
		TestEqual(TEXT("no response bar"), Four.ResponseShare, -1.f);
		TestEqual(TEXT("and the default star's colour"), Four.StarColor, FLinearColor::White);
		TestEqual(TEXT("and the default star's size"), Four.StarScale, 1.f);

		const FCosmicSystemLook Eleven = Shown(Pure, View, 11);
		TestTrue(TEXT("(the world's look of 11 is dead)"), LookOf(Pure, Tables, 11).bDead);
		TestFalse(TEXT("An unknown destroyed system's look is not known"), Eleven.bKnown);
		TestFalse(TEXT("An unknown destroyed system is a backdrop point, not a dead star"), Eleven.bDead);

		const FCosmicSystemLook Nine = Shown(Pure, View, 9);
		TestTrue(TEXT("A known destroyed system stays a dead star"), Nine.bDead);
		TestFalse(TEXT("with its name"), Nine.Name.IsEmpty());

		const FCosmicSystemLook Five = Shown(Pure, View, 5);
		TestTrue(TEXT("A known system keeps its name"), Five.Name.EqualTo(Pure.Systems[5].DisplayName));
		TestEqual(TEXT("and its class"), Five.StarScale, 2.f);

		const FCosmicSystemLook Two = Shown(Pure, View, 2);
		const FMegaprojectDef* Dyson = Tables.FindMegaproject(FName(TEXT("project_dyson")));
		TestTrue(TEXT("A shown owner draws its ring"), Two.bOwned);
		TestEqual(TEXT("in its meeting colour"), Two.OwnerColor, CosmicPalette::OwnerColorOf(2));
		TestFalse(TEXT("and not the state's colour"), Two.OwnerColor.Equals(StateColour, 1e-4f));
		if (TestNotNull(TEXT("(the game has the Dyson swarm row)"), Dyson))
		{
			TestEqual(TEXT("with its works"), Two.ConstructionShare, 120.f / CosmicMegaprojects::RequiredProgressOf(*Dyson), 1e-5f);
		}
		TestEqual(TEXT("The first life's people takes the first colour"), Shown(Pure, View, 0).OwnerColor, CosmicPalette::OwnerColorOf(0));

		const FCosmicSystemLook World3 = LookOf(Pure, Tables, 3);
		TestTrue(TEXT("(the world's look of 3 holds an owner)"), World3.bOwned);
		TestTrue(TEXT("(and a bar)"), World3.ConstructionShare > 0.f);
		const FCosmicSystemLook Three = Shown(Pure, View, 3);
		TestFalse(TEXT("A hidden owner draws no ring"), Three.bOwned);
		TestEqual(TEXT("no colour"), Three.OwnerColor, FLinearColor::Black);
		TestEqual(TEXT("and no bars of its works"), Three.ConstructionShare, -1.f);
		TestTrue(TEXT("but its name stays"), Three.Name.EqualTo(Pure.Systems[3].DisplayName));
		TestEqual(TEXT("its class stays"), Three.StarColor, World3.StarColor);
		TestTrue(TEXT("its hazard stays"), Three.bHazard);

		FGalaxyState Settling = Pure;
		Settling.Systems[3].ActiveMegaproject = CosmicRows::ProjectSettle;
		TestTrue(TEXT("(the world's look of 3 there is settling)"), LookOf(Settling, Tables, 3).bSettling);
		TestFalse(TEXT("A hidden owner's settlement draws no dashed ring"), Shown(Settling, View, 3).bSettling);

		TestEqual(TEXT("LookOf draws no colour of the state"), LookOf(Pure, Tables, 2).OwnerColor, FLinearColor::Black);
		TestFalse(TEXT("A look out of range is not known"), Shown(Pure, View, 99).bKnown);
	}

	// ---- 3 Lanes and labels ----
	{
		using CosmicMapLook::KnownLanesOf;
		using CosmicMapLook::LanesOf;
		using CosmicMapLook::ShowsLabel;
		const TArray<TPair<int32, int32>> Drawn = KnownLanesOf(LanesOf(Pure), View);
		TestEqual(TEXT("Two lanes are drawn between known living systems"), Drawn.Num(), 2);
		TestTrue(TEXT("the lane 2-3"), Drawn.Contains(TPair<int32, int32>(2, 3)));
		TestTrue(TEXT("the lane 5-6"), Drawn.Contains(TPair<int32, int32>(5, 6)));
		int32 UnknownEnds = 0;
		for (const TPair<int32, int32>& Lane : Drawn)
		{
			UnknownEnds += (!View.IsKnown(Lane.Key) || !View.IsKnown(Lane.Value)) ? 1 : 0;
		}
		TestEqual(TEXT("No lane has an unknown end"), UnknownEnds, 0);
		TestEqual(TEXT("A lane whose second end is unknown is dropped"), KnownLanesOf({ TPair<int32, int32>(2, 4) }, View).Num(), 0);
		TestEqual(TEXT("A lane whose first end is unknown is dropped"), KnownLanesOf({ TPair<int32, int32>(4, 2) }, View).Num(), 0);
		TestTrue(TEXT("See-all draws every lane of the world"), KnownLanesOf(LanesOf(Pure), SeeAll) == LanesOf(Pure));

		FCosmicSystemLook Point;
		Point.bKnown = false;
		Point.bOwned = true;
		Point.bHazard = true;
		TestFalse(TEXT("A backdrop point shows no label near"), ShowsLabel(Point, false, false));
		TestFalse(TEXT("nor from afar"), ShowsLabel(Point, false, true));
		TestFalse(TEXT("nor selected"), ShowsLabel(Point, true, true));

		FGalaxyState HiddenOwned = Pure;
		HiddenOwned.Systems[6].OwnerCivID = HiddenOwned.Civs[P.U].CivID;
		TestTrue(TEXT("(the world's look of 6 there is owned)"), LookOf(HiddenOwned, Tables, 6).bOwned);
		TestFalse(TEXT("A hidden owner lights no label from afar"), ShowsLabel(Shown(HiddenOwned, View, 6), false, true));
		TestTrue(TEXT("A hazard does"), ShowsLabel(Shown(Pure, View, 3), false, true));
		TestTrue(TEXT("A shown owner does"), ShowsLabel(Shown(Pure, View, 2), false, true));
		TestTrue(TEXT("(a known plain system's label shows near)"), ShowsLabel(Shown(Pure, View, 5), false, false));
	}

	// ---- 4 The palette ----
	{
		using CosmicPalette::OwnerColorOf;
		using CosmicPalette::Owners;
		TestEqual(TEXT("The first people met takes Owners[0]"), OwnerColorOf(0), Owners[0]);
		TestEqual(TEXT("The thirteenth people met takes the first colour again"), OwnerColorOf(12), Owners[0]);
		TestEqual(TEXT("and the fourteenth the second"), OwnerColorOf(13), Owners[1]);
		TestEqual(TEXT("A people not shown has no colour"), OwnerColorOf(-1), FLinearColor::Black);

		FCosmicSystemLook Hazardous;
		Hazardous.bHazard = true;
		FCosmicSystemLook Dead;
		Dead.bDead = true;
		// The owners as the screen shows them (LineLight), and the fixed colours as each is drawn.
		TArray<FLinearColor> OnScreen;
		for (const FLinearColor& Each : Owners)
		{
			OnScreen.Add(LineLight(Each));
		}
		const TPair<const TCHAR*, FLinearColor> Fixed[] = {
			{ TEXT("grey"), CosmicPalette::Grey },
			{ TEXT("white"), CosmicPalette::White },
			{ TEXT("gold"), CosmicPalette::Gold },
			{ TEXT("purple"), CosmicPalette::Purple },
			{ TEXT("lane blue"), LineLight(CosmicMapLook::LaneColorOf(0.f, ECosmicViewMode::Physical)) },
			{ TEXT("physical warning"), FCosmicStarParams().WarningColor },
			{ TEXT("spiritual warning"), CosmicMapLook::StarParamsOf(Hazardous, FCosmicSpiritLook(), ECosmicViewMode::Spiritual).WarningColor },
			{ TEXT("dead grey"), CosmicMapLook::StarParamsOf(Dead, FCosmicSpiritLook(), ECosmicViewMode::Physical).StarColor },
			{ TEXT("black"), FLinearColor::Black },
		};
		const int32 Count = CosmicPalette::OwnerCount;
		for (int32 I = 0; I < Count; ++I)
		{
			for (int32 J = I + 1; J < Count; ++J)
			{
				TestTrue(*FString::Printf(TEXT("Owner colours %d and %d differ (OKLab at least 0.07)"), I, J), DeltaE(OnScreen[I], OnScreen[J]) >= 0.07);
			}
			for (const TPair<const TCHAR*, FLinearColor>& Each : Fixed)
			{
				TestTrue(*FString::Printf(TEXT("Owner colour %d is not the %s (OKLab at least 0.07)"), I, Each.Key), DeltaE(OnScreen[I], Each.Value) >= 0.07);
			}
			const FLinearColor Hsv = Owners[I].LinearRGBToHSV();
			TestFalse(*FString::Printf(TEXT("Owner colour %d keeps out of the fallen's band"), I), Hsv.R >= CosmicMapLook::FallenHueFrom && Hsv.R <= CosmicMapLook::FallenHueTo);
			TestEqual(*FString::Printf(TEXT("so the sight draws owner colour %d as it is"), I), CosmicMapLook::OwnerRingColorOf(Owners[I], ECosmicViewMode::Spiritual), Owners[I]);
			TestEqual(*FString::Printf(TEXT("Owner colour %d is opaque"), I), Owners[I].A, 1.f);
			TestTrue(*FString::Printf(TEXT("Under deuteranopia owner colour %d stays apart from the purple (OKLab at least 0.04)"), I),
				DeltaE(SeenThrough(Deuteranopia, OnScreen[I]), SeenThrough(Deuteranopia, CosmicPalette::Purple)) >= 0.04);
			TestTrue(*FString::Printf(TEXT("Under protanopia owner colour %d stays apart from the purple (OKLab at least 0.04)"), I),
				DeltaE(SeenThrough(Protanopia, OnScreen[I]), SeenThrough(Protanopia, CosmicPalette::Purple)) >= 0.04);
		}
		// The first life's own colour against the first neighbours met, for every eye.
		for (int32 I = 1; I <= 3; ++I)
		{
			TestTrue(*FString::Printf(TEXT("Under deuteranopia the first life's colour stays apart from the colour met %d (OKLab at least 0.1)"), I),
				DeltaE(SeenThrough(Deuteranopia, OnScreen[0]), SeenThrough(Deuteranopia, OnScreen[I])) >= 0.1);
			TestTrue(*FString::Printf(TEXT("Under protanopia the first life's colour stays apart from the colour met %d (OKLab at least 0.1)"), I),
				DeltaE(SeenThrough(Protanopia, OnScreen[0]), SeenThrough(Protanopia, OnScreen[I])) >= 0.1);
		}
	}

	// ---- 5 The live map ----
	Restore();
	Map->SetViewDistance(3000.f);
	Refreshed();
	ULineBatchComponent* Batch = Map->FindComponentByClass<ULineBatchComponent>();
	if (!TestEqual(TEXT("(one star per system)"), Map->GetStars().Num(), Board.Systems.Num()) || !TestNotNull(TEXT("(the map draws its lines in one batch)"), Batch))
	{
		return false;
	}
	{
		const uint32 StateHash = Sim->ComputeStateHash();
		const uint32 WorldHash = Sim->ComputeWorldHash();
		TestEqual(TEXT("The map draws only the lanes between known living systems"), Map->GetDrawnLaneCount(), 2);
		TestEqual(TEXT("One ring per shown owner's known system"), Map->GetDrawnRingCount(), 2);
		TestEqual(TEXT("The ruled people's ring is drawn in the first colour"), CountInColor(RingOf(*Map, 0), CosmicPalette::OwnerColorOf(0)), 24);
		TestEqual(TEXT("A's ring in the third"), CountInColor(RingOf(*Map, 2), CosmicPalette::OwnerColorOf(2)), 24);
		int32 StateColoured = 0;
		for (const FBatchedLine& Line : Batch->BatchedLines)
		{
			StateColoured += Line.Color.Equals(StateColour, 1e-4f) ? 1 : 0;
		}
		TestEqual(TEXT("No line is drawn in a colour of the state"), StateColoured, 0);
		TestEqual(TEXT("A known system of an unmet people has no ring"), RingOf(*Map, 3).Num(), 0);

		// Labels near, spheres, applied looks.
		for (int32 At = 0; At < Board.Systems.Num(); ++At)
		{
			const bool bKnown = View.IsKnown(At);
			const ACosmicStarSystemActor* Star = Map->GetStars()[At].Get();
			TestEqual(*FString::Printf(TEXT("Star %d's label shows near as the viewer knows it"), At), Star && Star->IsLabelVisible(), bKnown);
			if (bKnown)
			{
				TestTrue(*FString::Printf(TEXT("Star %d's sphere shows"), At), SphereShows(*Map, At));
			}
			else
			{
				TestFalse(*FString::Printf(TEXT("Star %d's sphere is hidden"), At), SphereShows(*Map, At));
				const FCosmicSystemLook Applied = Star ? Star->GetAppliedLook() : FCosmicSystemLook();
				TestTrue(*FString::Printf(TEXT("Star %d's applied look has no name"), At), Applied.Name.IsEmpty());
				TestFalse(*FString::Printf(TEXT("Star %d's applied look has no hazard"), At), Applied.bHazard);
				TestFalse(*FString::Printf(TEXT("Star %d's applied look has no owner"), At), Applied.bOwned);
				TestFalse(*FString::Printf(TEXT("Star %d's applied look is not known"), At), Applied.bKnown);
			}
		}

		// The selection ring, told directly to the map.
		Map->SetSelectedSystem(IdOf(4));
		Map->RefreshNow();
		TestEqual(TEXT("The map rings no unknown system, even when told to"), Map->GetDrawnSelection(), FName(NAME_None));
		Map->SetSelectedSystem(IdOf(2));
		Map->RefreshNow();
		TestEqual(TEXT("and rings a known one"), Map->GetDrawnSelection(), IdOf(2));
		Map->SetSelectedSystem(NAME_None);

		// The forced sight.
		Controller->CosmicSpiritualSight(1);
		Map->RefreshNow();
		const ACosmicStarSystemActor* FourStar = Map->GetStars()[4].Get();
		const FCosmicSpiritLook FourSpirit = FourStar ? FourStar->GetAppliedSpirit() : FCosmicSpiritLook();
		TestTrue(TEXT("(the sight is drawn)"), Map->GetDrawnViewMode() == ECosmicViewMode::Spiritual);
		TestTrue(TEXT("An unknown star is handed no claim state"), FourSpirit.ClaimState == ECosmicClaimState::None);
		TestEqual(TEXT("An unknown star is handed no pressure"), FourSpirit.Pressure, 0.f);
		TestTrue(TEXT("An unknown star is handed no mark"), FourSpirit.MarkRow.IsNone());
		const ACosmicStarSystemActor* ThreeStar = Map->GetStars()[3].Get();
		TestTrue(TEXT("A known star in the sphere is held, whoever owns it"), ThreeStar && ThreeStar->GetAppliedSpirit().ClaimState == ECosmicClaimState::Held);
		TestEqual(TEXT("One sealed ring, around the known system of the sealed sphere"), Map->GetDrawnSealedRingCount(), 1);
		// The greyed halves, with a control: system 1 known for this check only, so the lanes 0-1 and 1-2 are drawn too
		// and reach ends no fallen one presses (a sphere presses inside and one lane out: 0 and 1 lie off both spheres).
		FKnownMark& One = Live.Knowing.KnownSystems.AddDefaulted_GetRef();
		One.ID = IdOf(1);
		Refreshed();
		const FCosmicKnownView WithOne = KnownViewOf(Live, Sim->GetStateIndex(), Sim->GetViewerTier(), false);
		int32 DrawnHalves = 0;
		int32 PressedEnds = 0;
		for (const TPair<int32, int32>& Lane : CosmicMapLook::KnownLanesOf(CosmicMapLook::LanesOf(Live), WithOne))
		{
			for (const int32 End : { Lane.Key, Lane.Value })
			{
				++DrawnHalves;
				PressedEnds += CosmicMapLook::SpiritOf(Live, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), End).Pressure > 0.f ? 1 : 0;
			}
		}
		TestTrue(TEXT("(at least one half is greyed)"), PressedEnds > 0);
		TestTrue(TEXT("(and at least one drawn half is not)"), PressedEnds < DrawnHalves);
		TestEqual(TEXT("Greyed halves only on drawn lanes, as their ends are pressed"), Map->GetDrawnGreyedHalfCount(), PressedEnds);
		Live.Knowing.KnownSystems.RemoveAll([&IdOf](const FKnownMark& Mark) { return Mark.ID == IdOf(1); });
		Controller->CosmicSpiritualSight(0);
		Map->RefreshNow();

		TestEqual(TEXT("Drawing changes no state"), Sim->ComputeStateHash(), StateHash);
		TestEqual(TEXT("Drawing changes no world"), Sim->ComputeWorldHash(), WorldHash);
	}
	{
		// Knowledge shows at the next refresh.
		FKnownMark& Four = Live.Knowing.KnownSystems.AddDefaulted_GetRef();
		Four.ID = IdOf(4);
		TestFalse(TEXT("(not before the next refresh)"), SphereShows(*Map, 4));
		Refreshed();
		TestTrue(TEXT("Knowledge shows at the next refresh"), SphereShows(*Map, 4));
		const UInstancedStaticMeshComponent* Backdrop = Map->GetBackdrop();
		TestEqual(TEXT("and its slot goes dark"), Backdrop ? GlowOf(*Backdrop, Map->GetBackdropPointCount() + 4) : -1.f, 0.f);

		// Meeting a people: its ring, in its meeting colour (R 0, F 1, A 2, U 3).
		FKnownMark& Met = Live.Knowing.MetCivs.AddDefaulted_GetRef();
		Met.ID = Live.Civs[P.U].CivID;
		Refreshed();
		TestEqual(TEXT("A people met gets its ring"), RingOf(*Map, 3).Num(), 24);
		TestEqual(TEXT("in its meeting colour"), CountInColor(RingOf(*Map, 3), CosmicPalette::OwnerColorOf(3)), 24);

		// The gap closes.
		Live.Civs[P.F].Tier = EAscensionTier::Tier3_Causal;
		Refreshed();
		TestEqual(TEXT("A people no longer far above is ringed in its own colour"), CountInColor(RingOf(*Map, 8), CosmicPalette::OwnerColorOf(1)), 24);
	}
	{
		// The viewer's tier is the ruled people's while the vessel rules: a ruler on the second rung whose person stands on
		// the first sees a people of the fourth rung (a gap of two).
		Restore();
		Live.Civs[P.R].Tier = EAscensionTier::Tier2_Stellar;
		Live.Calling.PersonalTier = EAscensionTier::Tier1_Planetary;
		TestTrue(TEXT("(the viewer's tier is the ruled people's)"), Sim->GetViewerTier() == EAscensionTier::Tier2_Stellar);
		Refreshed();
		TestEqual(TEXT("Under a second-rung ruler a people of the fourth rung is ringed in its colour"), CountInColor(RingOf(*Map, 8), CosmicPalette::OwnerColorOf(1)), 24);
	}

	// ---- 6 The selection and the pick ----
	Restore();
	{
		ACosmicCameraPawn* Pawn = Cast<ACosmicCameraPawn>(Controller->GetPawn());
		if (!TestNotNull(TEXT("(the controller possesses the camera pawn)"), Pawn))
		{
			return false;
		}
		const uint32 StateHash = Sim->ComputeStateHash();
		const uint32 WorldHash = Sim->ComputeWorldHash();
		Controller->SelectSystem(NAME_None);
		int32 Announced = 0;
		const FDelegateHandle Probe = Controller->OnSelectionChanged.AddLambda([&Announced](FName) { ++Announced; });
		Controller->SelectSystem(IdOf(4));
		TestEqual(TEXT("An unknown system is not selected"), Controller->GetSelectedSystem(), FName(NAME_None));
		TestEqual(TEXT("and nothing is announced"), Announced, 0);
		Controller->SelectSystem(IdOf(3));
		Controller->OnSelectionChanged.Remove(Probe);
		TestEqual(TEXT("A known system of a hidden people is selected"), Controller->GetSelectedSystem(), IdOf(3));
		TestEqual(TEXT("and announced once (the probe hears a selection)"), Announced, 1);
		const FVector FocusBefore = Pawn->GetTargetFocus();
		TestFalse(TEXT("SelectAndFocus refuses an unknown system"), Controller->SelectAndFocus(IdOf(4)));
		TestTrue(TEXT("and the camera does not go there"), Pawn->GetTargetFocus().Equals(FocusBefore, 0.01));
		TestEqual(TEXT("and the selection stays"), Controller->GetSelectedSystem(), IdOf(3));

		const FCosmicKnownView LiveView = Controller->GetKnownView();
		auto Flat = [](const FVector& WorldPoint, FVector2D& Screen)
		{
			Screen = FVector2D(WorldPoint.X, WorldPoint.Y);
			return true;
		};
		const TArray<FCosmicPickCandidate> Candidates = ACosmicPlayerController::PickCandidatesOf(*Map, LiveView, Flat, FVector(0.0, 1.0, 0.0));
		auto IsCandidate = [&Candidates](FName ID) { return Candidates.ContainsByPredicate([ID](const FCosmicPickCandidate& Each) { return Each.SystemID == ID; }); };
		for (const int32 At : UnknownOnBoard)
		{
			TestFalse(*FString::Printf(TEXT("Star %d, which the viewer does not know, is no pick target"), At), IsCandidate(IdOf(At)));
		}
		for (const int32 At : KnownOnBoard)
		{
			TestTrue(*FString::Printf(TEXT("Star %d, which the viewer knows, is a pick target"), At), IsCandidate(IdOf(At)));
		}
		const FVector2D FourAt(Board.Systems[4].Position.X, Board.Systems[4].Position.Y);
		TArray<FCosmicScreenTarget> Targets;
		for (const FCosmicPickCandidate& Candidate : Candidates)
		{
			Targets.Add(Candidate.Target);
		}
		// The control: the same click among every star, the gate of knowledge left out, picks star 4.
		const FCosmicKnownView AllView = KnownViewOf(Sim->GetState(), Sim->GetStateIndex(), Sim->GetViewerTier(), true);
		const TArray<FCosmicPickCandidate> Ungated = ACosmicPlayerController::PickCandidatesOf(*Map, AllView, Flat, FVector(0.0, 1.0, 0.0));
		TArray<FCosmicScreenTarget> UngatedTargets;
		for (const FCosmicPickCandidate& Candidate : Ungated)
		{
			UngatedTargets.Add(Candidate.Target);
		}
		const int32 UngatedPick = CosmicMapPick::NearestOnScreen(UngatedTargets, FourAt, 24.f);
		if (TestTrue(TEXT("(without the gate a click at 4's place picks a star)"), Ungated.IsValidIndex(UngatedPick)))
		{
			TestEqual(TEXT("(and that star is 4)"), Ungated[UngatedPick].SystemID, IdOf(4));
		}
		TestEqual(TEXT("A click on an unknown star picks nothing there"), CosmicMapPick::NearestOnScreen(Targets, FourAt, 24.f), static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("Selecting changes no state"), Sim->ComputeStateHash(), StateHash);
		TestEqual(TEXT("Selecting changes no world"), Sim->ComputeWorldHash(), WorldHash);

		// CosmicSelect on a built map refuses an unknown system and leaves nothing pending.
		Controller->SelectSystem(NAME_None);
		AddExpectedMessagePlain(FString::Printf(TEXT("CosmicSelect: %s is not a system the viewer knows"), *IdOf(4).ToString()), ELogVerbosity::Display, EAutomationExpectedMessageFlags::Contains, 1);
		Controller->CosmicSelect(IdOf(4).ToString());
		TestEqual(TEXT("A CosmicSelect of an unknown system selects nothing"), Controller->GetSelectedSystem(), FName(NAME_None));
		// An ID the galaxy does not hold is told as such (CosmicSeeAll would not help).
		AddExpectedMessagePlain(TEXT("CosmicSelect: sys_nowhere is no system of this galaxy"), ELogVerbosity::Display, EAutomationExpectedMessageFlags::Contains, 1);
		Controller->CosmicSelect(TEXT("sys_nowhere"));
		TestEqual(TEXT("A CosmicSelect of an ID the galaxy does not hold selects nothing"), Controller->GetSelectedSystem(), FName(NAME_None));
		FGalaxyState Everything = Board;
		CosmicTests::KnowAll(Everything);
		Sim->LoadState(Everything);
		Map->RefreshNow();
		TestEqual(TEXT("and leaves nothing pending"), Controller->GetSelectedSystem(), FName(NAME_None));
		Restore();

		// See-all selects what it draws; without it an unknown selection goes.
		Controller->CosmicSeeAll(1);
		Controller->SelectSystem(IdOf(4));
		TestEqual(TEXT("Under see-all an unknown system is selected"), Controller->GetSelectedSystem(), IdOf(4));
		Controller->CosmicSeeAll(0);
		TestEqual(TEXT("Without see-all an unknown selection is dropped"), Controller->GetSelectedSystem(), FName(NAME_None));
	}

	// ---- 7 The system panel ----
	Restore();
	{
		UCosmicSystemPanelWidget* Panel = CreateWidget<UCosmicSystemPanelWidget>(Controller);
		if (!TestNotNull(TEXT("The system panel can be created"), Panel))
		{
			return false;
		}
		const TSharedRef<SWidget> SlatePanel = Panel->TakeWidget();
		for (const int32 At : { 4, 1, 7 })
		{
			Panel->ShowSystem(IdOf(At));
			TestTrue(*FString::Printf(TEXT("System %d, which the viewer does not know, shows no panel"), At), Panel->GetVisibility() == ESlateVisibility::Collapsed);
		}
		const FString OtherOwnerLine = FString::Printf(TEXT("%s  %s"), *Words(TEXT("Panel.Owner")), *Words(TEXT("Panel.OtherOwner")));
		const TArray<FString> Hidden = { UnmetMarker, FarMarker };

		// 3: a known system of a people not met.
		Panel->ShowSystem(IdOf(3));
		{
			TestTrue(TEXT("(the panel shows the known system)"), Panel->GetVisibility() != ESlateVisibility::Collapsed);
			const FString Texts = ReadTexts(*Panel);
			const TArray<FString> Lines = DetailLines(*Panel);
			TestTrue(TEXT("The panel names the system"), Texts.Contains(Board.Systems[3].DisplayName.ToString()));
			TestTrue(TEXT("and its star class"), Lines.Contains(FString::Printf(TEXT("%s  G"), *Words(TEXT("Panel.StarClass")))));
			TestTrue(TEXT("The owner of a people not met is another people"), Lines.Contains(OtherOwnerLine));
			TestFalse(TEXT("The panel names no unmet people in its texts"), Texts.Contains(UnmetMarker));
			TestFalse(TEXT("nor in its tooltips"), PanelTooltips(*Panel).Contains(UnmetMarker));
			TestFalse(TEXT("A hidden owner has no people line"), AnyPeopleLine(Lines));
			TestFalse(TEXT("A hidden owner's works have no lines"), Lines.ContainsByPredicate([](const FString& Line) { return Line.StartsWith(Words(TEXT("Panel.Construction"))); }));
			const FHazardDef* Awakening = Tables.FindHazardByTag(Tag(TEXT("Hazard.AIAwakening")));
			if (TestNotNull(TEXT("(the game has the AI awakening row)"), Awakening))
			{
				TestTrue(TEXT("The hazard of a known system shows"), Texts.Contains(Awakening->DisplayName.ToString()));
			}
			TestTrue(TEXT("The settle button says another people lives there"), Panel->GetShownSettleBlock() == CosmicMegaprojects::ECosmicSettleBlock::Owned);
			const UCosmicActionButton* Settle = Panel->GetActionButtons().IsEmpty() ? nullptr : Panel->GetActionButtons()[0].Get();
			TestTrue(TEXT("and names nobody"), Settle && !ContainsAny(Settle->GetToolTipText().ToString(), Hidden));
		}

		// 8: a people met and far above.
		Panel->ShowSystem(IdOf(8));
		TestFalse(TEXT("A people far above is named nowhere"), ReadTexts(*Panel).Contains(FarMarker));
		TestTrue(TEXT("its system says another people lives there"), DetailLines(*Panel).Contains(OtherOwnerLine));

		// 2: a shown owner.
		Panel->ShowSystem(IdOf(2));
		{
			const TArray<FString> Lines = DetailLines(*Panel);
			const int32 OwnerAt = Lines.IndexOfByKey(FString::Printf(TEXT("%s  %s"), *Words(TEXT("Panel.Owner")), MetMarker));
			const int32 PeopleAt = Lines.IndexOfByKey(CosmicUI::PeopleLine(Sim->GetPopulationBand(Board.Civs[P.A].CivID)).ToString());
			const int32 LifeAt = Lines.IndexOfByKey(CosmicUI::LifeLine(Board.Seed, IdOf(2)).ToString());
			TestTrue(TEXT("A shown owner is named"), OwnerAt != INDEX_NONE);
			TestTrue(TEXT("with its people line"), PeopleAt != INDEX_NONE && PeopleAt == OwnerAt + 1);
			TestTrue(TEXT("The life line comes right after the people line"), LifeAt != INDEX_NONE && LifeAt == PeopleAt + 1);
			const FMegaprojectDef* Dyson = Tables.FindMegaproject(FName(TEXT("project_dyson")));
			if (TestNotNull(TEXT("(the game has the Dyson swarm row)"), Dyson))
			{
				const FString Works = FString::Printf(TEXT("%s  %s 120/%s"), *Words(TEXT("Panel.Construction")), *Dyson->DisplayName.ToString(),
					*FText::AsNumber(FMath::FloorToInt32(CosmicMegaprojects::RequiredProgressOf(*Dyson))).ToString());
				TestTrue(TEXT("and its works"), Lines.Contains(Works));
			}
		}

		// 5: a wild system; 9: a destroyed one.
		Panel->ShowSystem(IdOf(5));
		{
			const TArray<FString> Lines = DetailLines(*Panel);
			const int32 OwnerAt = Lines.IndexOfByKey(Words(TEXT("Panel.NoOwner")));
			const int32 LifeAt = Lines.IndexOfByKey(CosmicUI::LifeLine(Board.Seed, IdOf(5)).ToString());
			TestTrue(TEXT("A wild system has no owner"), OwnerAt != INDEX_NONE);
			TestTrue(TEXT("A known living system shows its life line"), LifeAt != INDEX_NONE);
			TestEqual(TEXT("right after its owner line"), LifeAt, OwnerAt + 1);
		}
		Panel->ShowSystem(IdOf(9));
		{
			const TArray<FString> Lines = DetailLines(*Panel);
			TestTrue(TEXT("A destroyed system says so"), Lines.Contains(Words(TEXT("Panel.Destroyed"))));
			TestFalse(TEXT("and has no life line"), Lines.ContainsByPredicate([](const FString& Line) { return Line.StartsWith(Words(TEXT("Panel.Life"))); }));
		}

		// The life words on the panel: the run's seed's, per system (a wrong seed may hit one word by chance, not five).
		auto LifeWordLine = [](int32 Seed, FName ID)
		{
			const int32 Word = static_cast<int32>(CosmicSeed::DeriveForName(Seed, ID, CosmicSeed::Purpose::Life, 0) % 12u);
			return FString::Printf(TEXT("%s  %s"), *Words(TEXT("Panel.Life")), *Words(CosmicUI::LifeWordKeys[Word]));
		};
		for (const int32 At : { 2, 3, 5, 6, 8 })
		{
			Panel->ShowSystem(IdOf(At));
			TestTrue(*FString::Printf(TEXT("The panel's word is the run's seed's on system %d"), At), DetailLines(*Panel).Contains(LifeWordLine(Board.Seed, IdOf(At))));
		}
		TestEqual(TEXT("(system 0 is sol)"), IdOf(0), FName(TEXT("sol")));
		Panel->ShowSystem(IdOf(0));
		TestFalse(TEXT("(interim until v0.8 step 4) sol shows no life line"), DetailLines(*Panel).ContainsByPredicate([](const FString& Line) { return Line.StartsWith(Words(TEXT("Panel.Life"))); }));
		{
			FGalaxyState Later = Board;
			Later.Tick += 7;
			Sim->LoadState(Later);
			Map->RefreshNow();
			Panel->ShowSystem(IdOf(2));
			TestTrue(TEXT("The same word after a load"), DetailLines(*Panel).Contains(LifeWordLine(Board.Seed, IdOf(2))));
			Restore();
		}

		// OwnerLineOf by itself.
		TestEqual(TEXT("OwnerLineOf names a shown owner"), UCosmicSystemPanelWidget::OwnerLineOf(Pure, View, 2).ToString(), FString::Printf(TEXT("%s  %s"), *Words(TEXT("Panel.Owner")), MetMarker));
		TestEqual(TEXT("OwnerLineOf says another people for a hidden owner"), UCosmicSystemPanelWidget::OwnerLineOf(Pure, View, 3).ToString(), OtherOwnerLine);
		TestEqual(TEXT("OwnerLineOf says no owner for a wild system"), UCosmicSystemPanelWidget::OwnerLineOf(Pure, View, 5).ToString(), Words(TEXT("Panel.NoOwner")));
		TestEqual(TEXT("OwnerLineOf says destroyed for a destroyed system"), UCosmicSystemPanelWidget::OwnerLineOf(Pure, View, 9).ToString(), Words(TEXT("Panel.Destroyed")));
		TestTrue(TEXT("OwnerLineOf is empty for an unknown system"), UCosmicSystemPanelWidget::OwnerLineOf(Pure, View, 4).IsEmpty());
		TestTrue(TEXT("OwnerLineOf is empty for an index out of range"), UCosmicSystemPanelWidget::OwnerLineOf(Pure, View, 99).IsEmpty());

		// No panel of a known system names a people the viewer is not shown.
		for (const int32 At : KnownOnBoard)
		{
			Panel->ShowSystem(IdOf(At));
			TestFalse(*FString::Printf(TEXT("The panel of system %d names no people the viewer is not shown"), At), ContainsAny(ReadTexts(*Panel) + PanelTooltips(*Panel), Hidden));
		}
	}

	// ---- 7.9 The life words themselves ----
	{
		TArray<int32> Seen12;
		Seen12.Init(0, 12);
		int32 Mismatches = 0;
		for (int32 At = 0; At < 1000; ++At)
		{
			const FName ID(*FString::Printf(TEXT("sys_%d"), At));
			const int32 Expected = static_cast<int32>(CosmicSeed::DeriveForName(42, ID, CosmicSeed::Purpose::Life, 0) % 12u);
			const int32 Got = CosmicUI::LifeWordIndexOf(42, ID);
			Mismatches += Got == Expected ? 0 : 1;
			if (Seen12.IsValidIndex(Got))
			{
				++Seen12[Got];
			}
		}
		TestEqual(TEXT("The life word is the system's Life derive"), Mismatches, 0);
		TestFalse(TEXT("All twelve words are drawn"), Seen12.Contains(0));
		TSet<FString> Keys;
		for (const TCHAR* Key : CosmicUI::LifeWordKeys)
		{
			Keys.Add(Key);
		}
		TestEqual(TEXT("Twelve life words, each its own key"), Keys.Num(), 12);
	}

	// ---- 7.10 The words ----
	{
		for (const TCHAR* Key : NewKeys())
		{
			const FString Text = Words(Key);
			TestFalse(*FString::Printf(TEXT("%s is no missing entry"), Key), Text.Contains(Placeholder));
			TestFalse(*FString::Printf(TEXT("%s has words"), Key), Text.IsEmpty());
		}
		bool bSchemaRead = false;
		const TArray<FString> Verdict = VerdictWords(bSchemaRead);
		if (!bSchemaRead)
		{
			AddInfo(TEXT("Tools/data/schema.json could not be read next to this build: the verdict words are not checked."));
		}
		else
		{
			TestFalse(TEXT("(the schema's words.verdict list is read)"), Verdict.IsEmpty());
		}
		// Harvest words, the spirit's four pictures and the beast, in the String Table's language (escaped: ASCII sources).
		const TArray<FString> Harvest = { TEXT("\uAC70\uB450"), TEXT("\uC218\uD655"), TEXT("\uCD94\uC218"), TEXT("\uC5F4\uB9E4") };
		const TArray<FString> Spirit = { TEXT("\uBC14\uB78C"), TEXT("\uC228"), TEXT("\uBE5B"), TEXT("\uC74C\uC131") };
		const TArray<FString> Beast = { TEXT("\uC9D0\uC2B9") };
		for (const TCHAR* Key : CosmicUI::LifeWordKeys)
		{
			const FString Text = Words(Key);
			TestFalse(*FString::Printf(TEXT("Life word %s has words"), Key), Text.IsEmpty());
			TestFalse(*FString::Printf(TEXT("Life word %s holds no digit"), Key), HasAsciiDigit(Text));
			TestFalse(*FString::Printf(TEXT("Life word %s holds no verdict word"), Key), ContainsAny(Text, Verdict));
			TestFalse(*FString::Printf(TEXT("Life word %s holds no harvest word"), Key), ContainsAny(Text, Harvest));
			TestFalse(*FString::Printf(TEXT("Life word %s holds none of the spirit's four pictures"), Key), ContainsAny(Text, Spirit));
			TestFalse(*FString::Printf(TEXT("Life word %s holds no beast"), Key), ContainsAny(Text, Beast));
		}
		const FString OtherOwner = Words(TEXT("Panel.OtherOwner"));
		TestFalse(TEXT("The other owner's words hold no digit"), HasAsciiDigit(OtherOwner));
		TestFalse(TEXT("The other owner's words hold no verdict word"), ContainsAny(OtherOwner, Verdict));
	}

	// ---- 8.1 The feed's click ----
	Restore();
	{
		UCosmicLogFeedWidget* Feed = HUD->GetLogFeed();
		ACosmicCameraPawn* Pawn = Cast<ACosmicCameraPawn>(Controller->GetPawn());
		if (TestNotNull(TEXT("(the HUD has its feed)"), Feed) && TestNotNull(TEXT("(and the camera pawn)"), Pawn))
		{
			UCosmicLogItem* Known = NewObject<UCosmicLogItem>(Feed);
			Known->FocusSystemID = IdOf(2);
			Feed->OpenItem(Known);
			TestEqual(TEXT("A click on a line of a known system selects it"), Controller->GetSelectedSystem(), IdOf(2));
			const FVector FocusBefore = Pawn->GetTargetFocus();
			UCosmicLogItem* Unknown = NewObject<UCosmicLogItem>(Feed);
			Unknown->FocusSystemID = IdOf(4);
			Feed->OpenItem(Unknown);
			TestEqual(TEXT("A click on a line of an unknown system keeps the selection"), Controller->GetSelectedSystem(), IdOf(2));
			TestTrue(TEXT("and the camera"), Pawn->GetTargetFocus().Equals(FocusBefore, 0.01));
			UCosmicLogItem* UnknownCapital = NewObject<UCosmicLogItem>(Feed);
			UnknownCapital->FocusCivID = Board.Civs[P.A].CivID;
			Feed->OpenItem(UnknownCapital);
			TestEqual(TEXT("A click on a line whose people's capital is unknown changes nothing"), Controller->GetSelectedSystem(), IdOf(2));
			UCosmicLogItem* KnownCapital = NewObject<UCosmicLogItem>(Feed);
			KnownCapital->FocusCivID = Board.Civs[P.R].CivID;
			Feed->OpenItem(KnownCapital);
			TestEqual(TEXT("(a click on a line of a people whose capital is known goes there)"), Controller->GetSelectedSystem(), IdOf(0));
		}
	}

	// ---- 8.2 The parity of the map and the scope of telling (tell-all off; Severity 0: no draw, no far line) ----
	Restore();
	{
		Log->SetTellAll(false);
		Sim->SetBroadcastEnabled(false);
		const FGameplayTag CivLine = Tag(TEXT("Log.Civ"));
		auto Told = [&](FName FocusSystem, FName FocusCiv)
		{
			const int32 Before = Live.Log.Num();
			Log->Push(CivLine, 0, FText::AsCultureInvariant(TEXT("<parity>")), FFormatNamedArguments(), FocusSystem, NAME_None, FocusCiv);
			const bool bTold = Live.Log.Num() > Before && !Live.Log.Last().bUntold;
			Live.Log.SetNum(Before);
			return bTold;
		};
		// A people is shown on the map when its ring is drawn around its known system.
		auto Case = [&](const TCHAR* Name, int32 Civ, int32 SystemOfCiv)
		{
			Refreshed();
			const bool bShown = RingOf(*Map, SystemOfCiv).Num() == 24;
			TestEqual(*FString::Printf(TEXT("For %s, the map shows the people exactly when a line about it is told"), Name), bShown, Told(NAME_None, Live.Civs[Civ].CivID));
		};
		Case(TEXT("the ruled people"), P.R, 0);
		Case(TEXT("A"), P.A, 2);
		Case(TEXT("U"), P.U, 3);
		Case(TEXT("F at the fourth rung"), P.F, 8);
		Live.Civs[P.F].Tier = EAscensionTier::Tier3_Causal;
		Case(TEXT("F at the third rung"), P.F, 8);
		Live.Civs[P.F].Tier = EAscensionTier::Tier4_Transcendent;
		// The viewer's tier: a second-rung ruler whose person stands on the first sees F (and is told of it).
		Live.Civs[P.R].Tier = EAscensionTier::Tier2_Stellar;
		Refreshed();
		const bool bFarShown = RingOf(*Map, 8).Num() == 24;
		const bool bFarTold = Told(NAME_None, Live.Civs[P.F].CivID);
		TestTrue(TEXT("Under a second-rung ruler a people at the fourth rung is told"), bFarTold);
		TestEqual(TEXT("and shown exactly as it is told"), bFarShown, bFarTold);
		Live.Civs[P.R].Tier = EAscensionTier::Tier1_Planetary;
		Refreshed();
		for (int32 At = 0; At < Live.Systems.Num(); ++At)
		{
			TestEqual(*FString::Printf(TEXT("For system %d, the map knows it exactly when a line about it is told"), At), SphereShows(*Map, At), Told(IdOf(At), NAME_None));
		}
		Log->SetTellAll(true);
		Sim->SetBroadcastEnabled(bWasBroadcasting);
	}

	// ---- 9 The top bar and the aid ----
	Restore();
	{
		UCosmicTopBarWidget* Bar = CreateWidget<UCosmicTopBarWidget>(Controller);
		if (TestNotNull(TEXT("The top bar can be created"), Bar))
		{
			const TSharedRef<SWidget> SlateBar = Bar->TakeWidget();
			Bar->Refresh();
			const FString Texts = ReadTexts(*Bar);
			TestTrue(TEXT("(the top bar names the ruled people)"), Texts.Contains(RuledMarker));
			TestFalse(TEXT("The top bar names no people but the ruled one: not A"), Texts.Contains(MetMarker));
			TestFalse(TEXT("not U"), Texts.Contains(UnmetMarker));
			TestFalse(TEXT("not F"), Texts.Contains(FarMarker));
		}

		const uint32 StateHash = Sim->ComputeStateHash();
		const uint32 WorldHash = Sim->ComputeWorldHash();
		const int32 KnownMarks = Live.Knowing.KnownSystems.Num();
		const int32 MetMarks = Live.Knowing.MetCivs.Num();
		Controller->CosmicSeeAll(1);
		TestEqual(TEXT("A setter only marks the map"), Map->GetDrawnLaneCount(), 2);
		Map->RefreshNow();
		TestEqual(TEXT("See-all draws every lane"), Map->GetDrawnLaneCount(), CosmicMapLook::LanesOf(Live).Num());
		TestEqual(TEXT("every owner's ring"), Map->GetDrawnRingCount(), 6);
		int32 HiddenSpheres = 0;
		int32 LitSlots = 0;
		const UInstancedStaticMeshComponent* Backdrop = Map->GetBackdrop();
		for (int32 At = 0; At < Live.Systems.Num(); ++At)
		{
			HiddenSpheres += SphereShows(*Map, At) ? 0 : 1;
			LitSlots += (Backdrop && GlowOf(*Backdrop, Map->GetBackdropPointCount() + At) != 0.f) ? 1 : 0;
		}
		TestEqual(TEXT("every star"), HiddenSpheres, 0);
		TestTrue(TEXT("(the map has its backdrop)"), Backdrop != nullptr);
		TestEqual(TEXT("and darkens every slot"), LitSlots, 0);
		Controller->CosmicSpiritualSight(1);
		Map->RefreshNow();
		TestEqual(TEXT("and the sealed sphere's rings"), Map->GetDrawnSealedRingCount(), 2);
		Controller->CosmicSpiritualSight(0);
		Controller->CosmicSeeAll(0);
		Map->RefreshNow();
		TestEqual(TEXT("and gives knowledge its say back"), Map->GetDrawnLaneCount(), 2);
		TestEqual(TEXT("See-all writes no state"), Sim->ComputeStateHash(), StateHash);
		TestEqual(TEXT("See-all writes no world"), Sim->ComputeWorldHash(), WorldHash);
		TestEqual(TEXT("See-all knows no new system"), Live.Knowing.KnownSystems.Num(), KnownMarks);
		TestEqual(TEXT("See-all meets no new people"), Live.Knowing.MetCivs.Num(), MetMarks);
	}

	// ---- 8.3 The feed scan, last: the game's galaxy with the game's tables, told as the game tells ----
	Sim->SetTables(GameTables);
	Sim->NewGalaxy(42, 0, 0);
	Map->RefreshNow();
	Log->SetTellAll(false);
	{
		UCosmicLogFeedWidget* Feed = CreateWidget<UCosmicLogFeedWidget>(Controller);
		if (!TestNotNull(TEXT("A feed can be created"), Feed))
		{
			return false;
		}
		const TSharedRef<SWidget> SlateFeed = Feed->TakeWidget();
		const FGalaxyState& Game = Sim->GetState();
		// The peoples the viewer never met (a line is told by the scope when it is pushed, and being met is the part of that
		// scope that holds for good - a people met and shown then may rise out of sight later, its old lines rightly kept),
		// whose name no met people's name and no known system's name contains (a line that names those would contain it by
		// chance).
		auto IsMet = [&](int32 Civ)
		{
			const FName ID = Game.Civs[Civ].CivID;
			return Game.Knowing.MetCivs.ContainsByPredicate([ID](const FKnownMark& Mark) { return Mark.ID == ID; });
		};
		auto Unshown = [&]()
		{
			const FCosmicKnownView GameView = KnownViewOf(Game, Sim->GetStateIndex(), Sim->GetViewerTier(), false);
			TArray<int32> Out;
			for (int32 Civ = 0; Civ < Game.Civs.Num(); ++Civ)
			{
				if (IsMet(Civ))
				{
					continue;
				}
				const FString Name = Game.Civs[Civ].CivName.ToString();
				bool bContained = Name.IsEmpty();
				for (int32 Other = 0; Other < Game.Civs.Num(); ++Other)
				{
					bContained |= IsMet(Other) && Game.Civs[Other].CivName.ToString().Contains(Name, ESearchCase::CaseSensitive);
				}
				for (int32 At = 0; At < Game.Systems.Num(); ++At)
				{
					bContained |= GameView.IsKnown(At) && Game.Systems[At].DisplayName.ToString().Contains(Name, ESearchCase::CaseSensitive);
				}
				if (!bContained)
				{
					Out.Add(Civ);
				}
			}
			return Out;
		};
		auto ControlHolds = [&]()
		{
			for (const int32 Civ : Unshown())
			{
				const FString Name = Game.Civs[Civ].CivName.ToString();
				if (Game.Log.ContainsByPredicate([&Name](const FCosmicLogEntry& Entry) { return Entry.bUntold && Entry.Message.ToString().Contains(Name, ESearchCase::CaseSensitive); }))
				{
					return true;
				}
			}
			return false;
		};
		for (int32 Chunk = 0; Chunk < 12 && !ControlHolds(); ++Chunk)
		{
			Sim->RunHeadless(500);
		}
		Feed->Refresh();
		TArray<FString> Items;
		for (const UCosmicLogItem* Item : Feed->GetItems())
		{
			Items.Add(Item ? Item->Message.ToString() : FString());
		}
		TestTrue(TEXT("(the log keeps an untold entry that names a people the viewer never met)"), ControlHolds());
		const TArray<int32> Checked = Unshown();
		TestTrue(TEXT("(at least one people the viewer never met is checked)"), !Checked.IsEmpty());
		for (const int32 Civ : Checked)
		{
			const FString Name = Game.Civs[Civ].CivName.ToString();
			TestFalse(*FString::Printf(TEXT("No line of the feed names %s, a people the viewer never met"), *Game.Civs[Civ].CivID.ToString()),
				Items.ContainsByPredicate([&Name](const FString& Message) { return Message.Contains(Name, ESearchCase::CaseSensitive); }));
		}
		const FCosmicKnownView GameView = KnownViewOf(Game, Sim->GetStateIndex(), Sim->GetViewerTier(), false);
		bool bNamesShown = false;
		for (int32 Civ = 0; Civ < Game.Civs.Num(); ++Civ)
		{
			if (Game.Civs[Civ].CivID == Game.Player.RuledCivID || GameView.ShownOrderOf(Civ) == INDEX_NONE)
			{
				continue;
			}
			const FString Name = Game.Civs[Civ].CivName.ToString();
			bNamesShown |= Items.ContainsByPredicate([&Name](const FString& Message) { return Message.Contains(Name, ESearchCase::CaseSensitive); });
		}
		TestTrue(TEXT("The feed names a people the viewer is shown"), bNamesShown);
	}
	Log->SetTellAll(true);
	return true;
}

// The backdrop (section 3.1 layer 1, 6.4): the law of a point, the points of a galaxy, the slot of an unknown system, the
// one instanced mesh on the map and when it is built, the sight, and the hashes.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIBackdropTest, "Cosmic_Ascension.UI.Backdrop", CosmicTests::Flags)
bool FCosmicUIBackdropTest::RunTest(const FString& Parameters)
{
	using namespace CosmicKnownFilterTests;
	using namespace CosmicMapLook;

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
	const FCosmicDataTables GameTables = Sim->GetTables();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(GameTables);
		Controller->CosmicSpiritualSight(0);
		Sim->NewGalaxy(42, 5, 3);
		Map->RefreshNow();
	};

	// ---- 1 The points ----
	{
		FCosmicRulesDef Rules = Sim->GetRules();
		TestEqual(TEXT("The game's rules ask for 20,000 points"), BackdropCountOf(Rules), 20000);
		Rules.BackgroundStarCount = 0;
		TestEqual(TEXT("No count reads as none"), BackdropCountOf(Rules), 0);
		Rules.BackgroundStarCount = -1;
		TestEqual(TEXT("A count below 0 reads as none"), BackdropCountOf(Rules), 0);
		Rules.BackgroundStarCount = 1000000;
		TestEqual(TEXT("A count above the most reads as the most"), BackdropCountOf(Rules), MaxBackdropPoints);

		const FVector Centre(300.0, -200.0, 0.0);
		const float Radius = 5000.f;
		const FName Home(TEXT("gal_milkyway"));
		const TArray<FCosmicBackdropPoint> Points = BackdropPointsOf(42, Home, 1000, Centre, Radius);
		if (!TestEqual(TEXT("1000 points"), Points.Num(), 1000))
		{
			return false;
		}
		const double Reach = Radius * BackdropRimShare;
		int32 OffDisc = 0;
		int32 OffPlane = 0;
		int32 BadSize = 0;
		int32 BadGlowInside = 0;
		int32 BadGlow = 0;
		int32 Inner = 0;
		int32 PastEdge = 0;
		int32 RimMisses = 0;
		TArray<float> Sizes;
		TArray<float> InsideGlows;
		for (const FCosmicBackdropPoint& Point : Points)
		{
			const double Distance = FVector::Dist2D(Point.Position, Centre);
			OffDisc += Distance <= Reach + 0.01 ? 0 : 1;
			OffPlane += Point.Position.Z == Centre.Z ? 0 : 1;
			BadSize += (Point.Size >= Radius * BackdropMinSizeShare - 1e-3f && Point.Size <= Radius * BackdropMaxSizeShare + 1e-3f) ? 0 : 1;
			BadGlow += (Point.Glow >= 0.f && Point.Glow <= BackdropMaxGlow + 1e-6f) ? 0 : 1;
			Inner += Distance <= Reach / UE_DOUBLE_SQRT_2 ? 1 : 0;
			Sizes.Add(Point.Size);
			if (Distance <= Radius)
			{
				BadGlowInside += (Point.Glow >= BackdropMinGlow - 1e-6f && Point.Glow <= BackdropMaxGlow + 1e-6f) ? 0 : 1;
				InsideGlows.Add(Point.Glow);
			}
			else
			{
				++PastEdge;
				RimMisses += Point.Glow <= BackdropMaxGlow * (Reach - Distance) / (Reach - Radius) + 1e-4 ? 0 : 1;
			}
		}
		TestEqual(TEXT("Every point lies on the disc"), OffDisc, 0);
		TestEqual(TEXT("in the plane of the systems"), OffPlane, 0);
		TestEqual(TEXT("Every point's size lies in the law's range"), BadSize, 0);
		TestEqual(TEXT("Every point inside the disc glows within the law's range"), BadGlowInside, 0);
		TestEqual(TEXT("and no point glows below nothing or above the most"), BadGlow, 0);
		Sizes.Sort();
		InsideGlows.Sort();
		const float SizeFloor = Radius * BackdropMinSizeShare;
		const float SizeRange = Radius * (BackdropMaxSizeShare - BackdropMinSizeShare);
		TestTrue(TEXT("Most points are small (median size below the low 40% of the range)"), !Sizes.IsEmpty() && Sizes[Sizes.Num() / 2] < SizeFloor + 0.4f * SizeRange);
		TestTrue(TEXT("Most points are faint (median glow below the low 30% of the range)"), !InsideGlows.IsEmpty()
			&& InsideGlows[InsideGlows.Num() / 2] < BackdropMinGlow + 0.3f * (BackdropMaxGlow - BackdropMinGlow));
		const float InnerShare = static_cast<float>(Inner) / Points.Num();
		TestTrue(TEXT("The points fill the disc evenly (half within the inner circle of half the area)"), InnerShare >= 0.42f && InnerShare <= 0.58f);
		TestTrue(TEXT("(some points lie past the disc's edge)"), PastEdge > 0);
		TestEqual(TEXT("No point past the disc glows above its rim's share"), RimMisses, 0);

		const TArray<FCosmicBackdropPoint> Again = BackdropPointsOf(42, Home, 1000, Centre, Radius);
		int32 Different = 0;
		for (int32 At = 0; At < Points.Num() && At < Again.Num(); ++At)
		{
			Different += (Points[At].Position.Equals(Again[At].Position, 0.0) && Points[At].Size == Again[At].Size && Points[At].Glow == Again[At].Glow) ? 0 : 1;
		}
		TestEqual(TEXT("(the same seed draws as many points)"), Again.Num(), Points.Num());
		TestEqual(TEXT("The same seed draws the same points"), Different, 0);
		const TArray<FCosmicBackdropPoint> OtherSeed = BackdropPointsOf(43, Home, 1000, Centre, Radius);
		TestTrue(TEXT("Another seed draws other points"), !OtherSeed.IsEmpty() && !OtherSeed[0].Position.Equals(Points[0].Position, 0.01));
		const TArray<FCosmicBackdropPoint> OtherRow = BackdropPointsOf(42, FName(TEXT("gal_andromeda")), 1000, Centre, Radius);
		TestTrue(TEXT("Another galaxy row draws other points"), !OtherRow.IsEmpty() && !OtherRow[0].Position.Equals(Points[0].Position, 0.01));
		TestEqual(TEXT("No count, no points"), BackdropPointsOf(42, Home, 0, Centre, Radius).Num(), 0);
		TestEqual(TEXT("No radius, no points"), BackdropPointsOf(42, Home, 1000, Centre, 0.f).Num(), 0);
		TestEqual(TEXT("A radius that is not a number gives no points"), BackdropPointsOf(42, Home, 1000, Centre, std::numeric_limits<float>::quiet_NaN()).Num(), 0);
	}

	// ---- 2 The slots ----
	{
		const FName ID(TEXT("sys_3"));
		const FVector Place(1234.0, -567.0, 0.0);
		const float Radius = 7000.f;
		const FCosmicBackdropPoint Slot = BackdropSlotOf(42, ID, Place, Radius);
		FCosmicBackdropPoint Expected;
		FRandomStream Stream;
		Stream.Initialize(static_cast<int32>(CosmicSeed::DeriveForName(42, ID, CosmicSeed::Purpose::Backdrop, 0)));
		BackdropLookOf(Stream, Radius, Expected);
		TestTrue(TEXT("A slot stands at its system's place"), Slot.Position.Equals(Place, 0.0));
		TestEqual(TEXT("A slot's size is the law of its own derive"), Slot.Size, Expected.Size);
		TestEqual(TEXT("A slot's glow is the law of its own derive"), Slot.Glow, Expected.Glow);
		TestTrue(TEXT("and its glow lies in the law's range"), Slot.Glow >= BackdropMinGlow && Slot.Glow <= BackdropMaxGlow);
		TestTrue(TEXT("and its size lies in the law's range"), Slot.Size >= Radius * BackdropMinSizeShare - 1e-3f && Slot.Size <= Radius * BackdropMaxSizeShare + 1e-3f);
		TSet<float> Glows;
		for (int32 At = 0; At < 12; ++At)
		{
			Glows.Add(BackdropSlotOf(42, FName(*FString::Printf(TEXT("sys_%d"), At)), Place, Radius).Glow);
		}
		TestTrue(TEXT("Systems draw different slots"), Glows.Num() >= 2);
	}

	// ---- 3 The live backdrop on the game's galaxy at tick 0 ----
	Sim->NewGalaxy(42, 0, 0);
	Map->RefreshNow();
	const UInstancedStaticMeshComponent* Backdrop = Map->GetBackdrop();
	if (!TestNotNull(TEXT("(the map holds a backdrop)"), Backdrop))
	{
		return false;
	}
	{
		const FGalaxyState& State = Sim->GetState();
		const UStaticMesh* Plane = Backdrop->GetStaticMesh();
		TestTrue(TEXT("It is one instanced mesh of the engine plane"), Plane && Plane->GetPathName().Equals(BackdropMeshPath));
		const UMaterialInterface* DustBase = LoadObject<UMaterialInterface>(nullptr, BackdropMaterialPath, nullptr, LOAD_NoWarn);
		const UMaterialInstanceDynamic* Dust = Cast<UMaterialInstanceDynamic>(Backdrop->GetMaterial(0));
		TestTrue(TEXT("drawn through its own instance of M_Backdrop"), Dust && DustBase && Dust->Parent.Get() == DustBase);
		TestTrue(TEXT("The backdrop shows when its material is right"), Backdrop->IsVisible());
		const int32 Triangles = Plane ? Plane->GetNumTriangles(0) : 0;
		TestTrue(TEXT("(the backdrop mesh has triangles)"), Triangles > 0);
		TestEqual(TEXT("(the plane is two triangles)"), Triangles, 2);
		TestTrue(TEXT("The backdrop draws at most BackdropTriangleBudget triangles"), static_cast<int64>(Triangles) * Backdrop->GetInstanceCount() <= BackdropTriangleBudget);
		const int32 Count = BackdropCountOf(Sim->GetRules());
		TestEqual(TEXT("It holds the rules' points and one slot per system"), Backdrop->GetInstanceCount(), Count + State.Systems.Num());
		TestEqual(TEXT("(the map counts its points)"), Map->GetBackdropPointCount(), Count);
		AddInfo(FString::Printf(TEXT("The backdrop's last build took %.2f ms"), Map->GetBackdropBuildMs()));
		const TArray<FCosmicBackdropPoint> Points = BackdropPointsOf(State.Seed, Sim->GetRules().HomeGalaxyRow, Count, Map->GetDiscCentre(), Map->GetDiscRadius());
		if (TestTrue(TEXT("(the game's rules give backdrop points)"), Count > 0 && Points.Num() == Count))
		{
			for (const int32 At : { 0, Count - 1 })
			{
				FTransform Transform;
				const bool bGot = Backdrop->GetInstanceTransform(At, Transform, true);
				TestTrue(*FString::Printf(TEXT("Instance %d stands at point %d"), At, At), bGot && Transform.GetLocation().Equals(Points[At].Position, 0.01));
				TestEqual(*FString::Printf(TEXT("Instance %d is point %d's size"), At, At), bGot ? static_cast<float>(Transform.GetScale3D().X) : -1.f, Points[At].Size / 100.f, 1e-4f);
				TestEqual(*FString::Printf(TEXT("Instance %d glows as point %d"), At, At), GlowOf(*Backdrop, At), Points[At].Glow);
			}
		}
		int32 Misplaced = 0;
		int32 MisLit = 0;
		int32 MisHidden = 0;
		int32 KnownCount = 0;
		const FCosmicKnownView View = Controller->GetKnownView();
		for (int32 At = 0; At < State.Systems.Num(); ++At)
		{
			FTransform Transform;
			Misplaced += (Backdrop->GetInstanceTransform(Count + At, Transform, true) && Transform.GetLocation().Equals(State.Systems[At].Position, 0.01)) ? 0 : 1;
			if (View.IsKnown(At))
			{
				++KnownCount;
				TestEqual(TEXT("The known system's slot is dark"), GlowOf(*Backdrop, Count + At), 0.f);
				TestTrue(TEXT("and the known one shows"), SphereShows(*Map, At));
			}
			else
			{
				MisLit += GlowOf(*Backdrop, Count + At) == BackdropSlotOf(State.Seed, State.Systems[At].SystemID, State.Systems[At].Position, Map->GetDiscRadius()).Glow ? 0 : 1;
				MisHidden += SphereShows(*Map, At) ? 1 : 0;
			}
		}
		TestEqual(TEXT("Every slot stands at its system's place"), Misplaced, 0);
		TestEqual(TEXT("(the game's galaxy knows one system at the start)"), KnownCount, 1);
		TestEqual(TEXT("Every unknown system's slot glows its own law"), MisLit, 0);
		TestEqual(TEXT("Every unknown system's sphere is hidden"), MisHidden, 0);

		// The refreshes write only what flipped.
		const int32 WritesBefore = Map->GetSlotWrites();
		Map->RequestRefresh();
		Map->RefreshNow();
		TestEqual(TEXT("A refresh without a change writes no slot"), Map->GetSlotWrites(), WritesBefore);
		CosmicTests::KnowAll(Sim->GetMutableState());
		Map->RequestRefresh();
		Map->RefreshNow();
		TestEqual(TEXT("(the refresh wrote each flipped slot once)"), Map->GetSlotWrites(), WritesBefore + State.Systems.Num() - KnownCount);
		int32 StillLit = 0;
		int32 StillHidden = 0;
		for (int32 At = 0; At < State.Systems.Num(); ++At)
		{
			StillLit += GlowOf(*Backdrop, Count + At) != 0.f ? 1 : 0;
			StillHidden += SphereShows(*Map, At) ? 0 : 1;
		}
		TestEqual(TEXT("A system that becomes known darkens its slot"), StillLit, 0);
		TestEqual(TEXT("and shows its star"), StillHidden, 0);
	}

	// ---- 4 The build ----
	{
		int32 Rebuilds = 0;
		const FDelegateHandle Counter = Map->OnRebuilt.AddLambda([&Rebuilds]() { ++Rebuilds; });
		ON_SCOPE_EXIT { Map->OnRebuilt.Remove(Counter); };
		int32 Builds = Map->GetBackdropBuildCount();

		Sim->NewGalaxy(42, 0, 0);
		Map->RefreshNow();
		TestEqual(TEXT("(the new galaxy rebuilt the map)"), Rebuilds, 1);
		TestEqual(TEXT("A new galaxy of the same seed and size builds no backdrop"), Map->GetBackdropBuildCount(), Builds);
		Sim->StepOnce();
		Map->RequestRefresh();
		Map->RefreshNow();
		TestEqual(TEXT("A refresh builds none"), Map->GetBackdropBuildCount(), Builds);
		const FGalaxyState Copy = Sim->GetState();
		Sim->LoadState(Copy);
		Map->RefreshNow();
		TestEqual(TEXT("(the load rebuilt the map)"), Rebuilds, 2);
		TestEqual(TEXT("A load of the same galaxy builds none"), Map->GetBackdropBuildCount(), Builds);

		// Each part of the key, with everything else held.
		FGalaxyState Reseeded = Copy;
		Reseeded.Seed += 1;
		Sim->LoadState(Reseeded);
		Map->RefreshNow();
		TestEqual(TEXT("Another seed on the same disc builds a new one"), Map->GetBackdropBuildCount(), ++Builds);
		Sim->LoadState(Copy);
		Map->RefreshNow();
		++Builds;
		FCosmicRulesDef OtherRow = Sim->GetRules();
		OtherRow.HomeGalaxyRow = FName(TEXT("gal_andromeda"));
		Sim->SetTables(CosmicTests::WithRules(GameTables, OtherRow));
		Sim->LoadState(Copy);
		Map->RefreshNow();
		TestEqual(TEXT("Another row builds a new one"), Map->GetBackdropBuildCount(), ++Builds);
		// Back to the game's row first, so the count below is the only part that changes.
		Sim->SetTables(GameTables);
		Sim->LoadState(Copy);
		Map->RefreshNow();
		TestEqual(TEXT("(the game's row builds again)"), Map->GetBackdropBuildCount(), ++Builds);
		FCosmicRulesDef Fewer = GameTables.FindRules() ? *GameTables.FindRules() : FCosmicRulesDef();
		Fewer.BackgroundStarCount = 500;
		Sim->SetTables(CosmicTests::WithRules(GameTables, Fewer));
		Sim->LoadState(Copy);
		Map->RefreshNow();
		TestEqual(TEXT("Another count builds a new one"), Map->GetBackdropBuildCount(), ++Builds);
		TestEqual(TEXT("with that many points"), Map->GetBackdrop() ? Map->GetBackdrop()->GetInstanceCount() : -1, 500 + Copy.Systems.Num());
		Sim->SetTables(GameTables);
		Sim->NewGalaxy(7, 0, 0);
		Map->RefreshNow();
		TestEqual(TEXT("(a new galaxy of another seed builds a new one)"), Map->GetBackdropBuildCount(), ++Builds);

		// The slots follow the systems.
		Sim->NewGalaxy(42, 0, 0);
		Map->RefreshNow();
		++Builds;
		const FVector DiscCentre = Map->GetDiscCentre();
		const float DiscRadius = Map->GetDiscRadius();
		FGalaxyState Moved = Sim->GetState();
		int32 Nearest = INDEX_NONE;
		for (int32 At = 0; At < Moved.Systems.Num(); ++At)
		{
			if (Nearest == INDEX_NONE || FVector::Dist2D(Moved.Systems[At].Position, DiscCentre) < FVector::Dist2D(Moved.Systems[Nearest].Position, DiscCentre))
			{
				Nearest = At;
			}
		}
		if (TestTrue(TEXT("(the galaxy has a system near the disc's centre)"), Nearest != INDEX_NONE))
		{
			Moved.Systems[Nearest].Position.X += 50.0;
			Sim->LoadState(Moved);
			Map->RefreshNow();
			TestTrue(TEXT("(the disc is as it was)"), Map->GetDiscCentre().Equals(DiscCentre, 0.0) && Map->GetDiscRadius() == DiscRadius);
			FTransform Transform;
			const bool bGot = Map->GetBackdrop() && Map->GetBackdrop()->GetInstanceTransform(Map->GetBackdropPointCount() + Nearest, Transform, true);
			TestTrue(TEXT("the moved system's slot stands at its new place"), bGot && Transform.GetLocation().Equals(Moved.Systems[Nearest].Position, 0.01));
			TestEqual(TEXT("(and no backdrop was built for it)"), Map->GetBackdropBuildCount(), Builds);
		}
	}

	// ---- 5 The sight and the hashes ----
	{
		Sim->NewGalaxy(42, 0, 0);
		Map->RefreshNow();
		const uint32 StateHash = Sim->ComputeStateHash();
		const uint32 WorldHash = Sim->ComputeWorldHash();
		const UMaterialInterface* Dust = Map->GetBackdrop() ? Map->GetBackdrop()->GetMaterial(0) : nullptr;
		auto DustIntensity = [Dust]()
		{
			float Value = -1.f;
			if (Dust)
			{
				Dust->GetScalarParameterValue(FHashedMaterialParameterInfo(FName(SkyIntensityParam)), Value);
			}
			return Value;
		};
		Controller->CosmicSpiritualSight(1);
		Map->RefreshNow();
		TestEqual(TEXT("In the sight the backdrop recedes"), DustIntensity(), 1.f);
		TestEqual(TEXT("(the map reports it)"), Map->GetBackdropIntensity(), 1.f);
		Controller->CosmicSpiritualSight(0);
		Map->RefreshNow();
		TestEqual(TEXT("and comes back in the physical view"), DustIntensity(), 0.f);
		TestEqual(TEXT("The backdrop moves no state"), Sim->ComputeStateHash(), StateHash);
		TestEqual(TEXT("The backdrop moves no world"), Sim->ComputeWorldHash(), WorldHash);
	}
	return true;
}

// The galaxy panel (section 5.11, 6.4): its lines, the four kinds, every galaxy of the game, its place on the HUD in the
// system panel's place, and the hint.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIGalaxyPanelTest, "Cosmic_Ascension.UI.GalaxyPanel", CosmicTests::Flags)
bool FCosmicUIGalaxyPanelTest::RunTest(const FString& Parameters)
{
	using namespace CosmicKnownFilterTests;

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
	ACosmicGalaxyMapActor* Map = FindMap(World);
	if (!TestNotNull(TEXT("The first player controller is the game's"), Controller) || !TestNotNull(TEXT("The controller has the HUD"), HUD)
		|| !TestNotNull(TEXT("The game mode spawned the map"), Map))
	{
		return false;
	}
	ON_SCOPE_EXIT
	{
		Controller->SelectSystem(NAME_None);
		Sim->NewGalaxy(42, 5, 3);
		Map->RefreshNow();
	};
	const FString& Placeholder = FStringTableEntry::GetPlaceholderSourceString();
	const FCosmicDataTables& Tables = Sim->GetTables();
	auto Joined = [](const TArray<FText>& Lines) { return FText::Join(INVTEXT("|"), Lines).ToString(); };

	// ---- 1 Lines ----
	{
		FGalaxyDef Full;
		Full.Kind = Tag(TEXT("Galaxy.Kind.Spiral"));
		Full.StarCountText = FText::AsCultureInvariant(TEXT("<count>"));
		Full.DistanceText = FText::AsCultureInvariant(TEXT("<distance>"));
		Full.FeatureText = FText::AsCultureInvariant(TEXT("<feature>"));
		const FString Expected = FString::Printf(TEXT("%s|<count>|<distance>|<feature>|%s"), *Words(TEXT("Galaxy.Kind.Spiral")), *Words(TEXT("Galaxy.Named")));
		TestTrue(TEXT("The lines are the kind, the three texts and the line, in order"), Joined(UCosmicGalaxyPanelWidget::LinesOf(Full)).Equals(Expected, ESearchCase::CaseSensitive));

		FGalaxyDef NoCount = Full;
		NoCount.StarCountText = FText::GetEmpty();
		TestTrue(TEXT("An empty star count leaves no line"), Joined(UCosmicGalaxyPanelWidget::LinesOf(NoCount)).Equals(
			FString::Printf(TEXT("%s|<distance>|<feature>|%s"), *Words(TEXT("Galaxy.Kind.Spiral")), *Words(TEXT("Galaxy.Named"))), ESearchCase::CaseSensitive));
		TestTrue(TEXT("The feature line shows"), UCosmicGalaxyPanelWidget::LinesOf(NoCount).ContainsByPredicate([](const FText& Line) { return Line.ToString().Equals(TEXT("<feature>")); }));
		FGalaxyDef Bare;
		Bare.Kind = Full.Kind;
		TestTrue(TEXT("A row with empty texts still carries the line"), Joined(UCosmicGalaxyPanelWidget::LinesOf(Bare)).Equals(
			FString::Printf(TEXT("%s|%s"), *Words(TEXT("Galaxy.Kind.Spiral")), *Words(TEXT("Galaxy.Named"))), ESearchCase::CaseSensitive));
		FGalaxyDef Strange = Full;
		Strange.Kind = Tag(TEXT("Hazard.Plague"));
		TestTrue(TEXT("A kind the table does not know shows no kind line"), Joined(UCosmicGalaxyPanelWidget::LinesOf(Strange)).Equals(
			FString::Printf(TEXT("<count>|<distance>|<feature>|%s"), *Words(TEXT("Galaxy.Named"))), ESearchCase::CaseSensitive));
		bool bDigit = HasAsciiDigit(Words(TEXT("Galaxy.Named"))) || HasAsciiDigit(Words(TEXT("Galaxy.Hint")));
		for (const TCHAR* Key : { TEXT("Galaxy.Kind.Dwarf"), TEXT("Galaxy.Kind.Spiral"), TEXT("Galaxy.Kind.GrandSpiral"), TEXT("Galaxy.Kind.Elliptical") })
		{
			bDigit |= HasAsciiDigit(Words(Key));
		}
		TestFalse(TEXT("The panel adds no number of its own"), bDigit);
	}

	// ---- 2 Kinds ----
	{
		UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		const FGameplayTagContainer Children = Manager.RequestGameplayTagChildren(FGameplayTag::RequestGameplayTag(FName(TEXT("Galaxy.Kind")), false));
		TArray<FGameplayTag> Kinds;
		Children.GetGameplayTagArray(Kinds);
		TestEqual(TEXT("(four kinds)"), Kinds.Num(), 4);
		TSet<FString> KindWords;
		for (const FGameplayTag& Kind : Kinds)
		{
			const FString Name = Kind.GetTagName().ToString();
			const TCHAR* Key = CosmicUI::GalaxyKindKey(Kind);
			TestTrue(*FString::Printf(TEXT("Kind %s has its key"), *Name), Key && FString(Key).Equals(Name, ESearchCase::IgnoreCase));
			const FString Text = Key ? Words(Key) : FString();
			TestFalse(*FString::Printf(TEXT("Kind %s's words are no missing entry"), *Name), Text.Contains(Placeholder));
			TestFalse(*FString::Printf(TEXT("Kind %s has words"), *Name), Text.IsEmpty());
			KindWords.Add(Text);
		}
		TestEqual(TEXT("The four kinds have four words"), KindWords.Num(), 4);
		TestTrue(TEXT("No other tag has a kind"), CosmicUI::GalaxyKindKey(Tag(TEXT("Hazard.Plague"))) == nullptr);
	}

	// ---- 3 Every galaxy of the game ----
	{
		TArray<FName> Rows = Tables.Galaxies ? Tables.Galaxies->GetRowNames() : TArray<FName>();
		Rows.Sort(FNameLexicalLess());
		TestEqual(TEXT("(the game has its six galaxy rows)"), Rows.Num(), 6);
		const FString Named = Words(TEXT("Galaxy.Named"));
		// The other galaxies' distances are told from the home galaxy ("from our galaxy", escaped: ASCII sources).
		const FString FromHome(TEXT("\uC6B0\uB9AC\uC740\uD558\uC5D0\uC11C"));
		for (const FName Row : Rows)
		{
			const FGalaxyDef* Galaxy = Tables.FindGalaxy(Row);
			const TArray<FText> Lines = Galaxy ? UCosmicGalaxyPanelWidget::LinesOf(*Galaxy) : TArray<FText>();
			TestTrue(*FString::Printf(TEXT("Galaxy %s carries the every-galaxy line"), *Row.ToString()), !Lines.IsEmpty() && Lines.Last().ToString().Equals(Named, ESearchCase::CaseSensitive));
			TestFalse(*FString::Printf(TEXT("Galaxy %s shows no empty line"), *Row.ToString()), Lines.ContainsByPredicate([](const FText& Line) { return Line.IsEmpty(); }));
			if (Galaxy && Row != Sim->GetRules().HomeGalaxyRow)
			{
				TestTrue(*FString::Printf(TEXT("Galaxy %s's distance is told from the home galaxy"), *Row.ToString()), Galaxy->DistanceText.ToString().StartsWith(FromHome, ESearchCase::CaseSensitive));
			}
		}
	}

	// ---- 4 The HUD ----
	Sim->NewGalaxy(42, 0, 0);
	Map->RefreshNow();
	{
		UCosmicGalaxyPanelWidget* Galaxy = HUD->GetGalaxyPanel();
		UCosmicSystemPanelWidget* System = HUD->GetSystemPanel();
		if (!TestNotNull(TEXT("(the HUD has a galaxy panel)"), Galaxy) || !TestNotNull(TEXT("(and a system panel)"), System))
		{
			return false;
		}
		const FName HomeRow = Sim->GetRules().HomeGalaxyRow;
		const FGalaxyDef* Home = Tables.FindGalaxy(HomeRow);
		Controller->SelectSystem(NAME_None);
		// The hint lasts the session and an earlier test may have selected already: arm it again for this section.
		Galaxy->RestoreHintForTests();
		HUD->RefreshPanel();
		TestTrue(TEXT("With no selection the HUD shows the galaxy panel"), Galaxy->GetVisibility() != ESlateVisibility::Collapsed);
		TestTrue(TEXT("with the hint while nothing has been selected"), Galaxy->IsHintShown());
		TestEqual(TEXT("of the home galaxy"), Galaxy->GetShownGalaxy(), HomeRow);
		TestTrue(TEXT("naming it"), Home && Galaxy->GetNameText() && Galaxy->GetNameText()->GetText().EqualTo(Home->DisplayName));
		TestTrue(TEXT("with the every-galaxy line"), Galaxy->GetLinesText() && Galaxy->GetLinesText()->GetText().ToString().Contains(Words(TEXT("Galaxy.Named"))));
		TestFalse(TEXT("The two panels never stand together: with no selection"), System->GetVisibility() != ESlateVisibility::Collapsed);
		const FString GalaxyTexts = ReadTexts(*Galaxy);
		bool bNamesPeople = false;
		for (const FCivilizationState& Civ : Sim->GetState().Civs)
		{
			bNamesPeople |= !Civ.CivName.IsEmpty() && GalaxyTexts.Contains(Civ.CivName.ToString());
		}
		TestFalse(TEXT("The galaxy panel names no people"), bNamesPeople);
		TArray<FString> GalaxyLines;
		GalaxyTexts.ParseIntoArray(GalaxyLines, TEXT("\n"));
		TestFalse(TEXT("and has no people line"), AnyPeopleLine(GalaxyLines));

		const FName HomeSystem = Sim->GetState().Civs.IsValidIndex(0) ? Sim->GetState().Civs[0].HomeSystemID : NAME_None;
		const int32 Ruled = Sim->GetStateIndex().FindCiv(Sim->GetState(), Sim->GetState().Player.RuledCivID);
		const FName RuledHome = Sim->GetState().Civs.IsValidIndex(Ruled) ? Sim->GetState().Civs[Ruled].HomeSystemID : HomeSystem;
		Controller->SelectSystem(RuledHome);
		TestEqual(TEXT("(the ruled people's home is selected)"), Controller->GetSelectedSystem(), RuledHome);
		TestTrue(TEXT("A selection hides the galaxy panel"), Galaxy->GetVisibility() == ESlateVisibility::Collapsed);
		TestTrue(TEXT("and shows the system panel"), System->GetVisibility() != ESlateVisibility::Collapsed);
		Controller->SelectSystem(NAME_None);
		TestTrue(TEXT("Clearing the selection shows it again"), Galaxy->GetVisibility() != ESlateVisibility::Collapsed);
		TestFalse(TEXT("The two panels never stand together: after the selection is cleared"), System->GetVisibility() != ESlateVisibility::Collapsed);
		// Read with the panel shown again (a collapsed panel shows no hint whatever its state).
		TestFalse(TEXT("The HUD's first selection took the hint away for good"), Galaxy->IsHintShown());
	}

	// ---- 5 The hint, on a panel of the test's own ----
	{
		UCosmicGalaxyPanelWidget* Panel = CreateWidget<UCosmicGalaxyPanelWidget>(Controller);
		if (TestNotNull(TEXT("A galaxy panel can be created"), Panel))
		{
			const TSharedRef<SWidget> SlatePanel = Panel->TakeWidget();
			Panel->ShowGalaxy(Sim->GetRules().HomeGalaxyRow);
			TestTrue(TEXT("The hint shows until the first selection"), Panel->IsHintShown());
			TestTrue(TEXT("with its words"), Panel->GetHintText() && Panel->GetHintText()->GetText().ToString().Equals(Words(TEXT("Galaxy.Hint")), ESearchCase::CaseSensitive));
			Panel->DropHint();
			TestFalse(TEXT("DropHint takes it away"), Panel->IsHintShown());
			Panel->ShowGalaxy(Sim->GetRules().HomeGalaxyRow);
			TestFalse(TEXT("for good"), Panel->IsHintShown());
			Panel->ShowGalaxy(NAME_None);
			TestTrue(TEXT("None hides the panel"), Panel->GetVisibility() == ESlateVisibility::Collapsed);
			Panel->ShowGalaxy(FName(TEXT("gal_nowhere")));
			TestTrue(TEXT("and so does a row the tables do not hold"), Panel->GetVisibility() == ESlateVisibility::Collapsed);
		}
		for (const TCHAR* Key : { TEXT("Galaxy.Named"), TEXT("Galaxy.Hint") })
		{
			TestFalse(*FString::Printf(TEXT("%s is no missing entry"), Key), Words(Key).Contains(Placeholder));
			TestFalse(*FString::Printf(TEXT("%s has words"), Key), Words(Key).IsEmpty());
		}
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

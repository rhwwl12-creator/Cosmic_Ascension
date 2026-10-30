// Cosmic Ascension - automation test of the interregnum's digest (B5, commit [B-5c]): World/CosmicDigest - the span after a
// closed life, the items the vertical slice has the material for, knowledge read through CosmicMapLook::KnownViewOf alone,
// no first-person, Log.Record or Log.Calling line read, the closing line of what changed out of knowledge, the quiet line
// that never says nothing happened, the knowledge from before the next life (KnownViewOf's cut at its BornTick), a leap
// of 0, the cap and the throne items' bound, the seat a rebellion took with no record before it, the span's end where the
// next life opens (the chronicle's own cut, CosmicChronicle::OpensCalledChapter - its inauguration line, [B-5d1]), and the sentence variants of CosmicSeed's
// Purpose::Digest (12). A pure function over the state: the boards are laid by hand and no tick runs. Design:
// docs/60_DECISIONS/headers/B5_death_succession_two_books.md section 2.7 and 6.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicSeed.h"
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/Text.h"
#include "Log/CosmicChronicle.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicDigest.h"
#include "World/CosmicMapLook.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicDigestTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// The rows and names of this test: a feat row of its own (an ascension's line names it), a transcendent instance with no
	// row (a sphere the state holds), and a people no state holds (a gone one).
	const FName RiseRow(TEXT("feat_test_digest_rise"));
	const FName FallenID(TEXT("fallen_test_digest"));
	const FName GoneCivID(TEXT("civ_test_digest_gone"));

	// The game's hazard rows the lines name, by their tags (row names are frozen - they land in saves), and the rebellion's
	// outbreak row (the event that names its rebel).
	struct FRows
	{
		FName Plague;
		FName Supernova;
		FName Claim;
		FName Rebellion;
		FName Outbreak;
	};

	// One stored line as the log keeps it (the Message is a marker no generated name holds).
	FCosmicLogEntry LineOf(int64 Tick, const TCHAR* Category, uint8 Severity, FName SystemID, FName CivID, FName SourceRow, const TCHAR* Message)
	{
		FCosmicLogEntry Entry;
		Entry.Tick = Tick;
		Entry.Category = Tag(Category);
		Entry.Severity = Severity;
		Entry.FocusSystemID = SystemID;
		Entry.FocusCivID = CivID;
		Entry.SourceRow = SourceRow;
		Entry.Message = FText::AsCultureInvariant(Message);
		return Entry;
	}

	FLifeRecord LifeOf(int32 LifeIndex, int64 Born, int64 Died, FName SystemID, FName CivID)
	{
		FLifeRecord Life;
		Life.LifeIndex = LifeIndex;
		Life.BornTick = Born;
		Life.DiedTick = Died;
		Life.BirthSystemID = SystemID;
		Life.BirthCivID = CivID;
		return Life;
	}

	FNotableRecord RecordOf(const TCHAR* Role, int32 NameIndex, int64 First, int64 Last)
	{
		FNotableRecord Record;
		Record.Role = Tag(Role);
		Record.NameIndex = NameIndex;
		Record.FirstTick = First;
		Record.LastTick = Last;
		return Record;
	}

	// A destroyed system as CosmicHazards::Destroy leaves one: no star class, no owner, no lane.
	void Kill(FGalaxyState& State, int32 SystemIndex)
	{
		FStarSystemState& System = State.Systems[SystemIndex];
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.OwnedSystems.Remove(System.SystemID);
		}
		System.OwnerCivID = NAME_None;
		System.StarClass = FGameplayTag();
		System.Lanes.Reset();
	}

	/**
	 * The board: the game's galaxy of seed 42 with twelve systems and four peoples. Now people 0 holds system 4, people 1
	 * holds 5, 6, 0 and 3, people 2 holds 8 and people 3 holds 11; every people on the first rung but people 2 on the
	 * fourth - met, and three rungs above the viewer, so the view does not show it (the gap of the scope of telling). The
	 * calling: its first life died at 1000 and the second stood up at 1800, a wanderer in system 5 born among people 1, on
	 * the first rung; it knows systems 0 to 6 and 8 and has met peoples 1, 0 and 2 (not 3; 7, 9, 10 and 11 are unknown),
	 * all of it since tick 500 - before the death, so the knowledge from before the next life holds it. No line, notable or
	 * sphere; the tick is 1800. Null when the galaxy does not give that.
	 */
	FGalaxyState* Lay(UGalaxySimulationSubsystem& Sim)
	{
		Sim.NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim.GetMutableState();
		if (State.Systems.Num() < 12 || State.Civs.Num() != 4)
		{
			return nullptr;
		}
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { 4 });
		CosmicTests::Give(State, 1, { 5, 6, 0, 3 });
		CosmicTests::Give(State, 2, { 8 });
		CosmicTests::Give(State, 3, { 11 });
		State.Transcendents.Reset();
		State.Log.Reset();
		State.Chronicle.Reset();
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.Tier = EAscensionTier::Tier1_Planetary;
			Civ.Notables.Reset();
			Civ.bIsPlayerRuled = false;
		}
		State.Civs[2].Tier = EAscensionTier::Tier4_Transcendent;
		State.Calling.Lives.Reset();
		State.Calling.Lives.Add(LifeOf(1, 0, 1000, State.Systems[0].SystemID, State.Civs[0].CivID));
		State.Calling.Lives.Add(LifeOf(2, 1800, 0, State.Systems[5].SystemID, State.Civs[1].CivID));
		State.Calling.LifeIndex = 2;
		State.Calling.BornTick = 1800;
		State.Calling.LifeState = ELifeState::Embodied;
		State.Calling.PendingLeapTicks = 0;
		State.Calling.PersonalTier = EAscensionTier::Tier1_Planetary;
		State.Player.Role = EPlayerRole::Wanderer;
		State.Player.RuledCivID = NAME_None;
		State.Player.CurrentSystemID = State.Systems[5].SystemID;
		State.Tick = 1800;
		State.Knowing = FKnowingState();
		CosmicTests::SetKnowing(State, { 0, 1, 2, 3, 4, 5, 6, 8 }, { 1, 0, 2 });
		for (FKnownMark& Mark : State.Knowing.KnownSystems)
		{
			Mark.Tick = 500;
		}
		for (FKnownMark& Mark : State.Knowing.MetCivs)
		{
			Mark.Tick = 500;
		}
		Sim.RebuildStateIndex();
		return &State;
	}

	// A mark the succession's Install writes on the next life's BornTick (1800): its birth system, its birth people, the
	// systems around a wanderer.
	void MarkAtInstall(TArray<FKnownMark>& Marks, FName ID)
	{
		FKnownMark& Mark = Marks.AddDefaulted_GetRef();
		Mark.ID = ID;
		Mark.Tick = 1800;
	}

	bool HasItemAbout(const FCosmicDigest& Digest, FName SubjectID)
	{
		return Digest.Items.ContainsByPredicate([SubjectID](const FCosmicDigestItem& Item) { return Item.SubjectID == SubjectID; });
	}

	// The lines that are never the digest's: the death's tick and the tick after the succession (outside the span), the
	// vessel's line of the succession's tick (Log.Calling) - each about a known system and a shown people, so read they
	// would make an item.
	void AddOutsideLines(FGalaxyState& State)
	{
		State.Log.Add(LineOf(1000, TEXT("Log.Ascension"), 2, State.Systems[4].SystemID, State.Civs[0].CivID, RiseRow, TEXT("<rose at the death>")));
		State.Log.Add(LineOf(1800, TEXT("Log.Calling"), 2, State.Systems[5].SystemID, State.Civs[1].CivID, NAME_None, TEXT("<called>")));
		State.Log.Add(LineOf(1801, TEXT("Log.Ascension"), 2, State.Systems[4].SystemID, State.Civs[0].CivID, RiseRow, TEXT("<rose after>")));
	}

	FString Joined(const TArray<FText>& Lines)
	{
		FString Out;
		for (const FText& Line : Lines)
		{
			Out += Line.ToString();
			Out += TEXT("\n");
		}
		return Out;
	}

	// The line an item has to read: the key of the variant Purpose 12 picks (by the frozen number, not the constant), with
	// the names of the state now and the ruler's name given.
	FString ExpectedLineOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDigestItem& Item, const FText& Ruler)
	{
		const int32 Variant = static_cast<int32>(CosmicSeed::DeriveForName(State.Seed, Item.SubjectID, 12u, static_cast<uint32>(Item.Tick)) % 3u);
		const TCHAR* Key = CosmicDigest::KeyOf(Item.Kind, Variant);
		if (!Key)
		{
			return FString();
		}
		const int32 SystemIndex = Index.FindSystem(State, Item.SubjectID);
		const int32 CivIndex = Index.FindCiv(State, Item.CivID);
		FFormatNamedArguments Args;
		Args.Add(TEXT("System"), State.Systems.IsValidIndex(SystemIndex) ? State.Systems[SystemIndex].DisplayName : FText::GetEmpty());
		Args.Add(TEXT("Civ"), State.Civs.IsValidIndex(CivIndex) ? State.Civs[CivIndex].CivName : FText::GetEmpty());
		Args.Add(TEXT("Ruler"), Ruler);
		return FText::Format(CosmicUI::Label(Key), Args).ToString();
	}

	bool LinesHave(const TArray<FCosmicChronicleLine>& Lines, const TCHAR* Marker)
	{
		return Lines.ContainsByPredicate([Marker](const FCosmicChronicleLine& Line) { return Line.Entry.Message.ToString() == Marker; });
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicUIDigestTest, "Cosmic_Ascension.UI.Digest", CosmicTests::Flags)
bool FCosmicUIDigestTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDigestTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	// The game's tables with a feat row of the test's (no tick runs: nothing else of the tables is read but the hazard rows,
	// the name banks and this row).
	{
		FCosmicDataTables Tables = Original;
		UDataTable* Feats = CosmicTests::NewTable(FAscensionFeatDef::StaticStruct());
		FAscensionFeatDef Rise;
		Rise.Title = FText::AsCultureInvariant(TEXT("<rise>"));
		Feats->AddRow(RiseRow, Rise);
		Tables.Feats = Feats;
		Sim->SetTables(Tables);
	}
	FRows Rows;
	const FCosmicDataTables& Tables = Sim->GetTables();
	if (!TestNotNull(TEXT("The game has a plague row"), Tables.FindHazardByTag(Tag(TEXT("Hazard.Plague")), &Rows.Plague))
		|| !TestNotNull(TEXT("The game has a supernova row"), Tables.FindHazardByTag(Tag(TEXT("Hazard.PreSupernova")), &Rows.Supernova))
		|| !TestNotNull(TEXT("The game has a claim row"), Tables.FindHazardByTag(Tag(TEXT("Hazard.Claim")), &Rows.Claim)))
	{
		return false;
	}
	{
		const FHazardDef* Rebellion = Tables.FindHazardByTag(Tag(TEXT("Hazard.Rebellion")), &Rows.Rebellion);
		if (!TestNotNull(TEXT("The game has a rebellion row"), Rebellion)
			|| !TestFalse(TEXT("... with an outbreak row (the event that names its rebel)"), Rebellion->OutbreakRow.IsNone()))
		{
			return false;
		}
		Rows.Outbreak = Rebellion->OutbreakRow;
	}
	const FString& Placeholder = FStringTableEntry::GetPlaceholderSourceString();

	// ---- 0. The keys and the purpose ----
	{
		TestEqual(TEXT("The digest's purpose is 12, appended after Encounter (fixed numbers)"), CosmicSeed::Purpose::Digest, 12u);
		for (const TCHAR* Key : { CosmicDigest::HeadKey, CosmicDigest::QuietKey, CosmicDigest::ElsewhereKey, CosmicDigest::NoLeapKey, CosmicDigest::MoreKey })
		{
			TestFalse(*FString::Printf(TEXT("ST_CA holds %s"), Key), CosmicUI::Label(Key).ToString().Contains(Placeholder));
		}
		for (const ECosmicDigestItem Kind : { ECosmicDigestItem::LostPeople, ECosmicDigestItem::Rose, ECosmicDigestItem::Lost, ECosmicDigestItem::Annexed,
			ECosmicDigestItem::Owner, ECosmicDigestItem::Throne, ECosmicDigestItem::ThroneUnnamed })
		{
			for (int32 Variant = 0; Variant < CosmicDigest::VariantCount; ++Variant)
			{
				const TCHAR* Key = CosmicDigest::KeyOf(Kind, Variant);
				if (TestNotNull(TEXT("Every item has a key for each variant"), Key))
				{
					TestFalse(*FString::Printf(TEXT("ST_CA holds %s"), Key), CosmicUI::Label(Key).ToString().Contains(Placeholder));
				}
			}
		}
	}

	// ---- 1. Every item of a span, knowledge through the view, the same lines twice ----
	{
		FGalaxyState* Board = Lay(*Sim);
		if (!TestNotNull(TEXT("The board is laid"), Board))
		{
			return false;
		}
		FGalaxyState& State = *Board;
		const FCosmicStateIndex& Index = Sim->GetStateIndex();
		auto SysID = [&State](int32 At) { return State.Systems[At].SystemID; };
		auto PeopleID = [&State](int32 At) { return State.Civs[At].CivID; };
		Kill(State, 1);
		Kill(State, 2);
		CosmicTests::AddInstance(State, FallenID, { SysID(3) });
		// The rise of people 1 has moved to the chronicle (the digest reads the chronicle before the log).
		State.Chronicle.Add(LineOf(1100, TEXT("Log.Ascension"), 2, SysID(5), PeopleID(1), RiseRow, TEXT("<people 1 rose>")));
		AddOutsideLines(State);
		State.Log.Insert(LineOf(1050, TEXT("Log.Crisis"), 1, SysID(0), PeopleID(0), Rows.Plague, TEXT("<plague on 0>")), 1);
		State.Log.Add(LineOf(1150, TEXT("Log.Ascension"), 2, SysID(8), PeopleID(2), RiseRow, TEXT("<people 2 rose>")));
		State.Log.Add(LineOf(1200, TEXT("Log.Crisis"), 1, SysID(1), GoneCivID, Rows.Supernova, TEXT("<supernova on 1>")));
		State.Log.Add(LineOf(1250, TEXT("Log.Crisis"), 1, SysID(2), PeopleID(0), Rows.Supernova, TEXT("<supernova on 2>")));
		State.Log.Add(LineOf(1300, TEXT("Log.Crisis"), 1, SysID(3), PeopleID(1), Rows.Claim, TEXT("<claim on 3>")));
		State.Log.Add(LineOf(1350, TEXT("Log.Crisis"), 1, SysID(11), PeopleID(3), Rows.Plague, TEXT("<plague on 11>")));
		State.Log.Add(LineOf(1400, TEXT("Log.Claim"), 1, SysID(3), PeopleID(1), FallenID, TEXT("<mine>")));
		State.Log.Add(LineOf(1400, TEXT("Log.Record"), 1, SysID(1), NAME_None, NAME_None, TEXT("<recorded>")));
		// Lines never read, each of which would change an item if it were: a record about system 6 naming people 0 as its
		// owner (people 1 holds it - an owner change), a first-person line about system 0 (its item's tick would move to
		// 1600), and the vessel's line about system 4 naming people 1 (people 0 holds it - an owner change).
		State.Log.Add(LineOf(1500, TEXT("Log.Record"), 1, SysID(6), PeopleID(0), Rows.Plague, TEXT("<recorded on 6>")));
		State.Log.Add(LineOf(1600, TEXT("Log.Claim"), 1, SysID(0), PeopleID(1), Rows.Plague, TEXT("<mine on 0>")));
		State.Log.Add(LineOf(1700, TEXT("Log.Calling"), 1, SysID(4), PeopleID(1), Rows.Plague, TEXT("<called on 4>")));
		State.Log.StableSort([](const FCosmicLogEntry& A, const FCosmicLogEntry& B) { return A.Tick < B.Tick; });

		const FCosmicKnownView View = CosmicMapLook::KnownViewOf(State, Index, Sim->GetViewerTier(), false);
		TestTrue(TEXT("(people 2 is met)"), View.IsMet(2));
		TestEqual(TEXT("(and the view does not show it - three rungs above the viewer)"), View.ShownOrderOf(2), static_cast<int32>(INDEX_NONE));
		TestFalse(TEXT("(system 11 is unknown)"), View.IsKnown(11));

		const uint32 HashBefore = Sim->ComputeStateHash();
		const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
		const TArray<FText> Lines = CosmicDigest::LinesOf(*Sim, 0);
		const TArray<FText> Again = CosmicDigest::LinesOf(*Sim, 0);
		TestEqual(TEXT("The same state gives the same lines twice"), Joined(Again), Joined(Lines));
		TestEqual(TEXT("The page's lines are the digest's"), Joined(CosmicDigest::LinesOf(Digest)), Joined(Lines));
		TestEqual(TEXT("The digest writes nothing"), Sim->ComputeStateHash(), HashBefore);
		TestTrue(TEXT("The span after the first life is a digest"), Digest.bValid && !Digest.bNoLeap);
		TestEqual(TEXT("It starts after the death"), Digest.AfterTick, static_cast<int64>(1000));
		TestEqual(TEXT("It ends at the next life's birth"), Digest.ThroughTick, static_cast<int64>(1800));

		struct FWant
		{
			ECosmicDigestItem Kind;
			FName SubjectID;
			FName CivID;
			int64 Tick;
		};
		const FWant Wants[] = {
			{ ECosmicDigestItem::Owner, SysID(0), PeopleID(1), 1050 },
			{ ECosmicDigestItem::Rose, PeopleID(1), PeopleID(1), 1100 },
			{ ECosmicDigestItem::LostPeople, SysID(1), NAME_None, 1200 },
			{ ECosmicDigestItem::Lost, SysID(2), NAME_None, 1250 },
			{ ECosmicDigestItem::Annexed, SysID(3), NAME_None, 1300 },
		};
		if (TestEqual(TEXT("Five items: the owner, the rung, the people gone, the star lost, the claim joined"), Digest.Items.Num(), static_cast<int32>(UE_ARRAY_COUNT(Wants))))
		{
			for (int32 At = 0; At < Digest.Items.Num(); ++At)
			{
				const FCosmicDigestItem& Item = Digest.Items[At];
				TestEqual(*FString::Printf(TEXT("Item %d's kind"), At), static_cast<int32>(Item.Kind), static_cast<int32>(Wants[At].Kind));
				TestEqual(*FString::Printf(TEXT("Item %d's subject"), At), Item.SubjectID, Wants[At].SubjectID);
				TestEqual(*FString::Printf(TEXT("Item %d's people"), At), Item.CivID, Wants[At].CivID);
				TestEqual(*FString::Printf(TEXT("Item %d's tick"), At), Item.Tick, Wants[At].Tick);
				TestEqual(*FString::Printf(TEXT("Item %d's variant is Purpose 12's (name = its ID, index = its tick)"), At), Item.Variant,
					static_cast<int32>(CosmicSeed::DeriveForName(State.Seed, Item.SubjectID, 12u, static_cast<uint32>(Item.Tick)) % 3u));
				TestEqual(*FString::Printf(TEXT("Item %d reads its variant's line"), At), Item.Line.ToString(), ExpectedLineOf(State, Index, Item, FText::GetEmpty()));
			}
		}
		for (const FCosmicDigestItem& Item : Digest.Items)
		{
			const int32 SystemIndex = Index.FindSystem(State, Item.SubjectID);
			const int32 CivIndex = Index.FindCiv(State, Item.CivID);
			TestTrue(TEXT("A system an item names is known (IsKnown of the view)"), SystemIndex == INDEX_NONE || View.IsKnown(SystemIndex));
			TestTrue(TEXT("A people an item names is shown by the view"), Item.CivID.IsNone() || View.ShownOrderOf(CivIndex) != INDEX_NONE);
			TestTrue(TEXT("No item names the people the view does not show, the people not met or the unknown system"),
				Item.SubjectID != PeopleID(2) && Item.CivID != PeopleID(2) && Item.SubjectID != PeopleID(3) && Item.CivID != PeopleID(3) && Item.SubjectID != SysID(11));
		}
		TestTrue(TEXT("What changed out of knowledge closes the page"), Digest.bElsewhere);
		TestFalse(TEXT("Five items are within the cap"), Digest.bMore);
		if (TestEqual(TEXT("The head, five items, the closing line"), Lines.Num(), 7))
		{
			TestEqual(TEXT("The head counts the span's ticks"), Lines[0].ToString(), FText::Format(CosmicUI::Label(CosmicDigest::HeadKey), FText::AsNumber(800)).ToString());
			TestEqual(TEXT("The closing line is Digest.Elsewhere"), Lines.Last().ToString(), CosmicUI::Label(CosmicDigest::ElsewhereKey).ToString());
		}
		TestFalse(TEXT("No Log.Record line is read: system 6, whose record names another owner, has no item"), HasItemAbout(Digest, SysID(6)));
		TestTrue(TEXT("No first-person line is read: system 0's item keeps the tick of its last world line"),
			Digest.Items.ContainsByPredicate([&SysID](const FCosmicDigestItem& Item) { return Item.SubjectID == SysID(0) && Item.Tick == 1050; }));
		TestFalse(TEXT("No line of the vessel is read: system 4, whose vessel's line names another owner, has no item"), HasItemAbout(Digest, SysID(4)));
	}

	// ---- 2. The closing line: only what changed out of knowledge calls it ----
	{
		// A clean span: one rise the view shows, and the lines never read about the unknown system 11 - the first person, a
		// record and the vessel's line.
		auto Clean = [&]() -> FGalaxyState*
		{
			FGalaxyState* Board = Lay(*Sim);
			if (!Board)
			{
				return nullptr;
			}
			FGalaxyState& State = *Board;
			AddOutsideLines(State);
			State.Log.Add(LineOf(1100, TEXT("Log.Ascension"), 2, State.Systems[5].SystemID, State.Civs[1].CivID, RiseRow, TEXT("<people 1 rose>")));
			State.Log.Add(LineOf(1300, TEXT("Log.Claim"), 1, State.Systems[11].SystemID, State.Civs[3].CivID, FallenID, TEXT("<mine>")));
			State.Log.Add(LineOf(1300, TEXT("Log.Record"), 1, State.Systems[9].SystemID, NAME_None, NAME_None, TEXT("<recorded>")));
			State.Log.Add(LineOf(1400, TEXT("Log.Calling"), 2, State.Systems[11].SystemID, State.Civs[3].CivID, NAME_None, TEXT("<called>")));
			return Board;
		};
		auto Sorted = [](FGalaxyState& State)
		{
			State.Log.StableSort([](const FCosmicLogEntry& A, const FCosmicLogEntry& B) { return A.Tick < B.Tick; });
		};

		FGalaxyState* Board = Clean();
		if (!TestNotNull(TEXT("The clean board is laid"), Board))
		{
			return false;
		}
		Sorted(*Board);
		{
			const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
			const TArray<FText> Lines = CosmicDigest::LinesOf(Digest);
			TestFalse(TEXT("The first person, a record and the vessel's line about an unknown system are no change out of knowledge"), Digest.bElsewhere);
			TestEqual(TEXT("The clean span has the head and its one item"), Lines.Num(), 2);
			TestTrue(TEXT("Its item is the rise"), Digest.Items.Num() == 1 && Digest.Items[0].Kind == ECosmicDigestItem::Rose);
		}

		// A people that rose twice in the span: one item, at its last rise - the words name no count of rungs.
		Board = Clean();
		if (!Board)
		{
			return false;
		}
		Board->Log.Add(LineOf(1500, TEXT("Log.Ascension"), 2, Board->Systems[5].SystemID, Board->Civs[1].CivID, RiseRow, TEXT("<people 1 rose again>")));
		Sorted(*Board);
		{
			const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
			if (TestEqual(TEXT("Two rises of one people are one item"), Digest.Items.Num(), 1))
			{
				TestEqual(TEXT("... a rise"), static_cast<int32>(Digest.Items[0].Kind), static_cast<int32>(ECosmicDigestItem::Rose));
				TestEqual(TEXT("... told at the last rise"), Digest.Items[0].Tick, static_cast<int64>(1500));
			}
		}

		// An owner change to a people the view does not show: people 2 (met, three rungs above) holds the known system 8 now,
		// and a hazard line of the span named people 0 its owner. No Owner item (it would name people 2) - out of knowledge.
		Board = Clean();
		if (!Board)
		{
			return false;
		}
		Board->Log.Add(LineOf(1500, TEXT("Log.Crisis"), 1, Board->Systems[8].SystemID, Board->Civs[0].CivID, Rows.Plague, TEXT("<plague on 8 under people 0>")));
		Sorted(*Board);
		{
			const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
			TestFalse(TEXT("A known system passing to a people the view does not show makes no item"), HasItemAbout(Digest, Board->Systems[8].SystemID));
			TestEqual(TEXT("... only the rise stays"), Digest.Items.Num(), 1);
			TestTrue(TEXT("... and it is a change out of knowledge (its line names a known system and a shown people)"), Digest.bElsewhere);
		}

		// The change from a people the view does not show: a hazard line of the span named people 2 the owner of the known
		// system 6, which people 1 holds now.
		Board = Clean();
		if (!Board)
		{
			return false;
		}
		Board->Log.Add(LineOf(1500, TEXT("Log.Crisis"), 1, Board->Systems[6].SystemID, Board->Civs[2].CivID, Rows.Plague, TEXT("<plague on 6 under people 2>")));
		Sorted(*Board);
		{
			const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
			TestFalse(TEXT("A known system passing from a people the view does not show makes no item"), HasItemAbout(Digest, Board->Systems[6].SystemID));
			TestEqual(TEXT("... only the rise stays"), Digest.Items.Num(), 1);
			TestTrue(TEXT("... and it is a change out of knowledge"), Digest.bElsewhere);
		}

		// What the succession's Install made known (marked on the next life's BornTick) was out of knowledge while no vessel
		// stood: the digest reads the knowledge from before the next life. System 7 - people 1's now, a line of the span naming
		// people 0 its owner - and people 3 - risen in the span - are known only through it.
		Board = Clean();
		if (!Board)
		{
			return false;
		}
		CosmicTests::Give(*Board, 1, { 7 });
		MarkAtInstall(Board->Knowing.KnownSystems, Board->Systems[7].SystemID);
		MarkAtInstall(Board->Knowing.MetCivs, Board->Civs[3].CivID);
		Board->Log.Add(LineOf(1500, TEXT("Log.Crisis"), 1, Board->Systems[7].SystemID, Board->Civs[0].CivID, Rows.Plague, TEXT("<plague on 7 under people 0>")));
		Board->Log.Add(LineOf(1600, TEXT("Log.Ascension"), 2, NAME_None, Board->Civs[3].CivID, RiseRow, TEXT("<people 3 rose>")));
		Sorted(*Board);
		Sim->RebuildStateIndex();
		{
			const FCosmicKnownView Now = CosmicMapLook::KnownViewOf(*Board, Sim->GetStateIndex(), Sim->GetViewerTier(), false);
			TestTrue(TEXT("(the view of now knows system 7 and shows people 3)"), Now.IsKnown(7) && Now.ShownOrderOf(3) != INDEX_NONE);
			const FCosmicKnownView Before = CosmicMapLook::KnownViewOf(*Board, Sim->GetStateIndex(), Sim->GetViewerTier(), false, 1800);
			TestTrue(TEXT("The view cut at the next life's BornTick knows neither"), !Before.IsKnown(7) && Before.ShownOrderOf(3) == INDEX_NONE && !Before.IsMet(3));
			TestTrue(TEXT("... and keeps what was known before it, in its place"), Before.IsKnown(8) && Before.ShownOrderOf(1) == Now.ShownOrderOf(1) && Before.ShownOrderOf(0) == Now.ShownOrderOf(0));
			const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
			const TArray<FText> Lines = CosmicDigest::LinesOf(Digest);
			TestFalse(TEXT("A system known only through the Install makes no item"), HasItemAbout(Digest, Board->Systems[7].SystemID));
			TestFalse(TEXT("A people met only through the Install makes no item"), HasItemAbout(Digest, Board->Civs[3].CivID));
			TestEqual(TEXT("... only the rise of people 1 stays"), Digest.Items.Num(), 1);
			TestTrue(TEXT("... and they are changes out of knowledge, closing the page"), Digest.bElsewhere
				&& !Lines.IsEmpty() && Lines.Last().ToString() == CosmicUI::Label(CosmicDigest::ElsewhereKey).ToString());
			// The same span with no next life (the knowledge of now) tells both: what hides them is the cut alone.
			const FCosmicDigest Plain = CosmicDigest::DigestOver(*Sim, 1000, 1800);
			TestTrue(TEXT("Read with the knowledge of now, the same span tells them"), HasItemAbout(Plain, Board->Systems[7].SystemID) && HasItemAbout(Plain, Board->Civs[3].CivID));
		}

		Board = Clean();
		if (!Board)
		{
			return false;
		}
		Board->Log.Add(LineOf(1500, TEXT("Log.Crisis"), 1, Board->Systems[11].SystemID, Board->Civs[3].CivID, Rows.Plague, TEXT("<plague on 11>")));
		Sorted(*Board);
		{
			const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
			const TArray<FText> Lines = CosmicDigest::LinesOf(Digest);
			TestTrue(TEXT("A line about an unknown system is a change out of knowledge"), Digest.bElsewhere);
			TestEqual(TEXT("... and makes no item"), Digest.Items.Num(), 1);
			TestTrue(TEXT("... and closes the page with Digest.Elsewhere"), !Lines.IsEmpty() && Lines.Last().ToString() == CosmicUI::Label(CosmicDigest::ElsewhereKey).ToString());
			// The view, not Knowing: under the development aid CosmicSeeAll the view knows system 11 and shows people 3.
			TestFalse(TEXT("Under CosmicSeeAll the same line is no change out of knowledge (the digest asks the view)"), CosmicDigest::DigestOf(*Sim, 0, true).bElsewhere);
		}

		Board = Clean();
		if (!Board)
		{
			return false;
		}
		Board->Log.Add(LineOf(1500, TEXT("Log.Discovery"), 0, NAME_None, NAME_None, CosmicRows::LogFarEvent, TEXT("<far>")));
		Sorted(*Board);
		TestTrue(TEXT("A far line is a change out of knowledge"), CosmicDigest::DigestOf(*Sim, 0).bElsewhere);

		Board = Clean();
		if (!Board)
		{
			return false;
		}
		Board->Log.Add(LineOf(1500, TEXT("Log.Ascension"), 2, Board->Systems[8].SystemID, Board->Civs[2].CivID, RiseRow, TEXT("<people 2 rose>")));
		Sorted(*Board);
		{
			const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
			TestTrue(TEXT("The rise of a people the view does not show is a change out of knowledge"), Digest.bElsewhere);
			TestEqual(TEXT("... and no item"), Digest.Items.Num(), 1);
		}

		Board = Clean();
		if (!Board)
		{
			return false;
		}
		Board->Civs[3].Notables.Add(RecordOf(TEXT("Notable.Ruler"), 0, 200, 1500));
		Sorted(*Board);
		{
			const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
			TestTrue(TEXT("A seat left in a people not met is a change out of knowledge"), Digest.bElsewhere);
			TestEqual(TEXT("... and no item"), Digest.Items.Num(), 1);
		}

		Board = Lay(*Sim);
		if (!Board)
		{
			return false;
		}
		AddOutsideLines(*Board);
		{
			const TArray<FText> Lines = CosmicDigest::LinesOf(*Sim, 0);
			if (TestEqual(TEXT("A span with nothing to tell: the head and the quiet line"), Lines.Num(), 2))
			{
				TestEqual(TEXT("... Digest.Quiet"), Lines[1].ToString(), CosmicUI::Label(CosmicDigest::QuietKey).ToString());
			}
		}

		// No item where much happened: a plague came and went on the known system 0 (people 1's, before and after - no item
		// kind for it), the known system 6 passed from people 1 to people 0 with no line (nothing the digest reads shows it),
		// and a line about the unknown system 11 changed something out of knowledge. The page must not say that nothing
		// happened: the head and the one quiet line, which claims only that the world went on - and no closing line after it.
		Board = Lay(*Sim);
		if (!Board)
		{
			return false;
		}
		AddOutsideLines(*Board);
		Board->Systems[6].OwnerCivID = Board->Civs[0].CivID;
		Board->Civs[1].OwnedSystems.Remove(Board->Systems[6].SystemID);
		Board->Civs[0].OwnedSystems.Add(Board->Systems[6].SystemID);
		Board->Log.Add(LineOf(1300, TEXT("Log.Crisis"), 1, Board->Systems[0].SystemID, Board->Civs[1].CivID, Rows.Plague, TEXT("<plague passed on 0>")));
		Board->Log.Add(LineOf(1500, TEXT("Log.Crisis"), 1, Board->Systems[11].SystemID, Board->Civs[3].CivID, Rows.Plague, TEXT("<plague on 11>")));
		Sorted(*Board);
		Sim->RebuildStateIndex();
		{
			const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
			const TArray<FText> Lines = CosmicDigest::LinesOf(Digest);
			TestTrue(TEXT("(no item: a plague's passing and an unseen owner change make none)"), Digest.Items.IsEmpty());
			TestTrue(TEXT("(something changed out of knowledge)"), Digest.bElsewhere);
			if (TestEqual(TEXT("With no item the page is the head and the quiet line - no closing line after it"), Lines.Num(), 2))
			{
				TestEqual(TEXT("... the head"), Lines[0].ToString(), FText::Format(CosmicUI::Label(CosmicDigest::HeadKey), FText::AsNumber(800)).ToString());
				TestEqual(TEXT("... the quiet line"), Lines[1].ToString(), CosmicUI::Label(CosmicDigest::QuietKey).ToString());
			}
			// The quiet line says no more than the closing line does: it is the closing line's own last clause (the world went
			// on), with no claim that nothing happened.
			TestTrue(TEXT("The quiet line is the closing line's last clause"),
				CosmicUI::Label(CosmicDigest::ElsewhereKey).ToString().EndsWith(CosmicUI::Label(CosmicDigest::QuietKey).ToString(), ESearchCase::CaseSensitive));
		}
	}

	// ---- 3. The seats (at most two) and the cap ----
	{
		FGalaxyState* Board = Lay(*Sim);
		if (!TestNotNull(TEXT("The seats' board is laid"), Board))
		{
			return false;
		}
		FGalaxyState& State = *Board;
		const FCosmicStateIndex& Index = Sim->GetStateIndex();
		// People 3 met before the death too (a meeting at the next life's BornTick would be the Install's, out of knowledge).
		FKnownMark& Met = State.Knowing.MetCivs.AddDefaulted_GetRef();
		Met.ID = State.Civs[3].CivID;
		Met.Tick = 500;
		// People 1: a ruler left at 1400 and one sits (named); a regent left at the death's tick (outside the span).
		State.Civs[1].Notables = { RecordOf(TEXT("Notable.Ruler"), 1, 200, 1400), RecordOf(TEXT("Notable.Ruler"), 0, 1300, 0), RecordOf(TEXT("Notable.Regent"), 2, 900, 1000) };
		// People 0: a regent left at 1500 and nobody sits (no name to tell); a regent left after the succession (outside).
		State.Civs[0].Notables = { RecordOf(TEXT("Notable.Regent"), 0, 300, 1500), RecordOf(TEXT("Notable.Regent"), 1, 1600, 1801) };
		// People 3: a ruler left at 1300, the earliest seat - over the bound of two.
		State.Civs[3].Notables = { RecordOf(TEXT("Notable.Ruler"), 0, 200, 1300), RecordOf(TEXT("Notable.Ruler"), 1, 1300, 0) };
		// People 2 is on the fourth rung: no persons there (its records closed at its climb) - no seat, no closing line.
		State.Civs[2].Notables = { RecordOf(TEXT("Notable.Ruler"), 0, 200, 1450) };

		const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
		const FCivArchetypeDef* Archetype = Tables.FindCiv(State.Civs[1].ArchetypeRow.IsNone() ? CosmicRows::ArchDefault : State.Civs[1].ArchetypeRow);
		const FText Ruler = (Archetype && !Archetype->PersonNameBank.IsEmpty()) ? Archetype->PersonNameBank[0] : FText::GetEmpty();
		if (TestEqual(TEXT("Two seats at most: the latest two"), Digest.Items.Num(), 2))
		{
			TestEqual(TEXT("People 1's seat first (1400)"), Digest.Items[0].SubjectID, State.Civs[1].CivID);
			TestEqual(TEXT("... named by its open ruler when its bank has a name"), static_cast<int32>(Digest.Items[0].Kind),
				static_cast<int32>(Ruler.IsEmpty() ? ECosmicDigestItem::ThroneUnnamed : ECosmicDigestItem::Throne));
			TestEqual(TEXT("... at the tick the seat was left"), Digest.Items[0].Tick, static_cast<int64>(1400));
			TestEqual(TEXT("... reading its variant's line"), Digest.Items[0].Line.ToString(), ExpectedLineOf(State, Index, Digest.Items[0], Ruler));
			TestEqual(TEXT("People 0's seat next (1500)"), Digest.Items[1].SubjectID, State.Civs[0].CivID);
			TestEqual(TEXT("... with no name to tell"), static_cast<int32>(Digest.Items[1].Kind), static_cast<int32>(ECosmicDigestItem::ThroneUnnamed));
			TestEqual(TEXT("... reading its variant's line"), Digest.Items[1].Line.ToString(), ExpectedLineOf(State, Index, Digest.Items[1], FText::GetEmpty()));
		}
		TestFalse(TEXT("Every seat left was in a people the view shows (the fourth rung has none)"), Digest.bElsewhere);
		TestFalse(TEXT("The bound of the seats is not the cap's line"), Digest.bMore);
	}
	{
		FGalaxyState* Board = Lay(*Sim);
		if (!TestNotNull(TEXT("The cap's board is laid"), Board))
		{
			return false;
		}
		FGalaxyState& State = *Board;
		State.Log.Add(LineOf(1050, TEXT("Log.Ascension"), 2, State.Systems[5].SystemID, State.Civs[1].CivID, RiseRow, TEXT("<people 1 rose>")));
		const int32 Lost[] = { 0, 1, 2, 3, 4, 6 };
		for (int32 At = 0; At < static_cast<int32>(UE_ARRAY_COUNT(Lost)); ++At)
		{
			State.Log.Add(LineOf(1100 + 100 * At, TEXT("Log.Crisis"), 1, State.Systems[Lost[At]].SystemID, State.Civs[1].CivID, Rows.Supernova, TEXT("<supernova>")));
		}
		for (const int32 SystemIndex : Lost)
		{
			Kill(State, SystemIndex);
		}
		const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
		const TArray<FText> Lines = CosmicDigest::LinesOf(Digest);
		TestTrue(TEXT("Seven items are over the cap"), Digest.bMore);
		if (TestEqual(TEXT("The cap keeps five"), Digest.Items.Num(), CosmicDigest::MaxItems))
		{
			TestEqual(TEXT("The rise outranks the stars lost and stays, first in time"), static_cast<int32>(Digest.Items[0].Kind), static_cast<int32>(ECosmicDigestItem::Rose));
			TestEqual(TEXT("The two earliest stars lost go: the next is system 2's"), Digest.Items[1].SubjectID, State.Systems[2].SystemID);
			TestEqual(TEXT("... and the last is system 6's"), Digest.Items.Last().SubjectID, State.Systems[6].SystemID);
		}
		if (TestEqual(TEXT("The head, five items, the cap's line"), Lines.Num(), 7))
		{
			TestEqual(TEXT("The cap's line is the chronicle's, with no count"), Lines.Last().ToString(), CosmicUI::Label(CosmicDigest::MoreKey).ToString());
		}
		TestFalse(TEXT("Nothing out of knowledge"), Digest.bElsewhere);
	}

	// ---- 3b. A rebellion that took a throne with no record before it; a resolved one; a taking the log no longer proves ----
	{
		FGalaxyState* Board = Lay(*Sim);
		if (!TestNotNull(TEXT("The rebellions' board is laid"), Board))
		{
			return false;
		}
		FGalaxyState& State = *Board;
		const FCosmicStateIndex& Index = Sim->GetStateIndex();
		auto RebelOf = [&Rows](const TCHAR* Role, int32 NameIndex, int64 First, int64 Last)
		{
			FNotableRecord Record = RecordOf(Role, NameIndex, First, Last);
			Record.SourceRow = Rows.Outbreak;
			return Record;
		};
		const FName Home1 = State.Civs[1].HomeSystemID;
		const FName Home0 = State.Civs[0].HomeSystemID;
		AddOutsideLines(State);
		// People 1 had no ruler or regent record: its first rebel, named by the outbreak at 1100, took the throne at the
		// expiry at 1300 (ChangeThrone closed nothing and made the rebel the ruler).
		State.Civs[1].Notables = { RebelOf(TEXT("Notable.Ruler"), 1, 1100, 0) };
		State.Log.Add(LineOf(1100, TEXT("Log.Crisis"), 1, Home1, State.Civs[1].CivID, Rows.Outbreak, TEXT("<rebels rise in 1>")));
		State.Log.Add(LineOf(1300, TEXT("Log.Crisis"), 1, Home1, State.Civs[1].CivID, Rows.Rebellion, TEXT("<the rebels of 1 took the throne>")));
		// People 0: a rebellion broke out at 1150 and was resolved at 1350 (its rebel closed, still a rebel); and a ruler a
		// rebellion made long before, whose outbreak line at 900 the log no longer holds - so the line of 1350, the first of
		// the rebellion's row after that rebel was named that the log holds, proves nothing of its taking.
		State.Civs[0].Notables = { RebelOf(TEXT("Notable.Ruler"), 2, 900, 0), RebelOf(TEXT("Notable.Rebel"), 0, 1150, 1350) };
		State.Log.Add(LineOf(1150, TEXT("Log.Crisis"), 1, Home0, State.Civs[0].CivID, Rows.Outbreak, TEXT("<rebels rise in 0>")));
		State.Log.Add(LineOf(1350, TEXT("Log.Crisis"), 1, Home0, State.Civs[0].CivID, Rows.Rebellion, TEXT("<the rebels of 0 were calmed>")));
		State.Log.StableSort([](const FCosmicLogEntry& A, const FCosmicLogEntry& B) { return A.Tick < B.Tick; });

		const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
		const FCivArchetypeDef* Archetype = Tables.FindCiv(State.Civs[1].ArchetypeRow.IsNone() ? CosmicRows::ArchDefault : State.Civs[1].ArchetypeRow);
		const int32 Count = Archetype ? Archetype->PersonNameBank.Num() : 0;
		const FText Ruler = Count > 0 ? Archetype->PersonNameBank[1 % Count] : FText::GetEmpty();
		if (TestEqual(TEXT("One seat changed: the first rebel's taking (none for the resolved rebellion, none unproven)"), Digest.Items.Num(), 1))
		{
			const FCosmicDigestItem& Item = Digest.Items[0];
			TestEqual(TEXT("The seat is people 1's"), Item.SubjectID, State.Civs[1].CivID);
			TestEqual(TEXT("... told at the expiry that made the rebel the ruler"), Item.Tick, static_cast<int64>(1300));
			TestEqual(TEXT("... named by its open ruler, the rebel"), static_cast<int32>(Item.Kind),
				static_cast<int32>(Ruler.IsEmpty() ? ECosmicDigestItem::ThroneUnnamed : ECosmicDigestItem::Throne));
			TestEqual(TEXT("... reading its variant's line"), Item.Line.ToString(), ExpectedLineOf(State, Index, Item, Ruler));
		}
		TestFalse(TEXT("A resolved rebellion takes no throne: no item for people 0"), HasItemAbout(Digest, State.Civs[0].CivID));
		TestFalse(TEXT("Every line was about a known system and a shown people"), Digest.bElsewhere);
	}

	// ---- 4. The spans: the life that closed, the interregnum that lasts, a leap of 0 ----
	{
		FGalaxyState* Board = Lay(*Sim);
		if (!TestNotNull(TEXT("The spans' board is laid"), Board))
		{
			return false;
		}
		FGalaxyState& State = *Board;
		TestEqual(TEXT("After a succession the life that closed is the one before the last"), CosmicDigest::LastClosedLifeOf(State), 0);
		TestTrue(TEXT("No digest for the life that stands"), CosmicDigest::LinesOf(*Sim, 1).IsEmpty());
		TestTrue(TEXT("No digest for an index that is no life"), CosmicDigest::LinesOf(*Sim, 2).IsEmpty() && CosmicDigest::LinesOf(*Sim, INDEX_NONE).IsEmpty());

		// The interregnum that lasts (the run ended in it): up to the state's tick.
		State.Calling.Lives.SetNum(1);
		State.Calling.LifeState = ELifeState::Interregnum;
		State.Calling.PendingLeapTicks = 300;
		State.Tick = 1500;
		State.Log.Add(LineOf(1500, TEXT("Log.Ascension"), 2, State.Systems[5].SystemID, State.Civs[1].CivID, RiseRow, TEXT("<people 1 rose>")));
		TestEqual(TEXT("In the interregnum the life that closed is the last"), CosmicDigest::LastClosedLifeOf(State), 0);
		{
			const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
			const TArray<FText> Lines = CosmicDigest::LinesOf(Digest);
			TestTrue(TEXT("The interregnum that lasts is a digest"), Digest.bValid && !Digest.bNoLeap);
			TestEqual(TEXT("... up to the state's tick"), Digest.ThroughTick, static_cast<int64>(1500));
			TestTrue(TEXT("... its last tick read"), Digest.Items.Num() == 1 && Digest.Items[0].Kind == ECosmicDigestItem::Rose);
			TestTrue(TEXT("... its head counts the ticks so far"), !Lines.IsEmpty()
				&& Lines[0].ToString() == FText::Format(CosmicUI::Label(CosmicDigest::HeadKey), FText::AsNumber(500)).ToString());
		}
		// The boundary of the death itself: nothing has passed yet.
		State.Tick = 1000;
		TestFalse(TEXT("At the death's own tick there is no digest yet"), CosmicDigest::DigestOf(*Sim, 0).bValid);
		TestTrue(TEXT("... and no line"), CosmicDigest::LinesOf(*Sim, 0).IsEmpty());

		// A leap of 0: the next life stood up on the tick of the death.
		State.Calling.Lives.Add(LifeOf(2, 1000, 0, State.Systems[5].SystemID, State.Civs[1].CivID));
		State.Calling.LifeState = ELifeState::Embodied;
		State.Calling.PendingLeapTicks = 0;
		State.Tick = 1200;
		State.Log.Add(LineOf(1100, TEXT("Log.Ascension"), 2, State.Systems[5].SystemID, State.Civs[1].CivID, RiseRow, TEXT("<people 1 rose>")));
		{
			const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
			const TArray<FText> Lines = CosmicDigest::LinesOf(Digest);
			TestTrue(TEXT("A leap of 0 is a digest of its own"), Digest.bValid && Digest.bNoLeap);
			if (TestEqual(TEXT("A leap of 0 gives one line"), Lines.Num(), 1))
			{
				TestEqual(TEXT("... Digest.NoLeap"), Lines[0].ToString(), CosmicUI::Label(CosmicDigest::NoLeapKey).ToString());
			}
		}

		// No life closed: a first life, standing.
		State.Calling.Lives.SetNum(1);
		State.Calling.Lives[0].DiedTick = 0;
		TestEqual(TEXT("No life closed: none"), CosmicDigest::LastClosedLifeOf(State), static_cast<int32>(INDEX_NONE));
		TestTrue(TEXT("... and no digest"), CosmicDigest::LinesOf(*Sim, 0).IsEmpty());
	}

	// ---- 5. The span ends where the next life opens: the chronicle's own cut (CosmicChronicle::OpensCalledChapter) ----
	{
		FGalaxyState* Board = Lay(*Sim);
		if (!TestNotNull(TEXT("The cut's board is laid"), Board))
		{
			return false;
		}
		FGalaxyState& State = *Board;
		// A star lost on the succession's own tick (the world's stages run before the succession). With no inauguration line
		// in that tick (a called life whose line is missing - section 6 has one) no line of it opens the next life's chapter:
		// the whole tick is the span's.
		const TCHAR* const LostAtBirth = TEXT("<supernova on 2 at the succession>");
		AddOutsideLines(State);
		State.Log.Add(LineOf(1800, TEXT("Log.Crisis"), 1, State.Systems[2].SystemID, State.Civs[0].CivID, Rows.Supernova, LostAtBirth));
		State.Log.StableSort([](const FCosmicLogEntry& A, const FCosmicLogEntry& B) { return A.Tick < B.Tick; });
		Kill(State, 2);
		// Read past the birth (a life born on the state's tick is the end's and gets no chapter).
		State.Tick = 1850;
		const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
		TestTrue(TEXT("The digest reads the succession's tick: the star lost there is an item"), Digest.Items.Num() == 1
			&& Digest.Items[0].SubjectID == State.Systems[2].SystemID && Digest.Items[0].Tick == 1800);
		const TArray<FCosmicChronicleChapter> Chapters = CosmicChronicle::Build(State);
		if (TestEqual(TEXT("(two chapters)"), Chapters.Num(), 2))
		{
			const FCosmicChronicleNoVessel& Span = Chapters[1].Before;
			TestTrue(TEXT("The chronicle's span with no vessel is the digest's span"), Span.FromTick == Digest.AfterTick + 1 && Span.ToTick == Digest.ThroughTick);
			TestTrue(TEXT("... and holds the line the digest read on the succession's tick"), LinesHave(Span.Lines, LostAtBirth));
			TestFalse(TEXT("... which the next life's chapter does not"), LinesHave(Chapters[1].Lines, LostAtBirth));
		}
		// The succession on the end's tick: the life it stood up gets no chapter, and the span after the death - to the end -
		// is again the digest's.
		State.Tick = 1800;
		const TArray<FCosmicChronicleChapter> Ended = CosmicChronicle::Build(State);
		if (TestEqual(TEXT("(the end's tick: one chapter)"), Ended.Num(), 1))
		{
			TestTrue(TEXT("The span after the last chapter is the digest's span"), Ended[0].After.FromTick == Digest.AfterTick + 1 && Ended[0].After.ToTick == Digest.ThroughTick);
			TestTrue(TEXT("... and holds the same line"), LinesHave(Ended[0].After.Lines, LostAtBirth));
		}
	}

	// ---- 6. The inauguration line ([B-5d1]): the span - the digest's and the chronicle's - stops before it ----
	{
		FGalaxyState* Board = Lay(*Sim);
		if (!TestNotNull(TEXT("The inauguration's board is laid"), Board))
		{
			return false;
		}
		FGalaxyState& State = *Board;
		// The succession's tick: a star lost in the world's stages, then the inauguration line (the tenth stage, after
		// Install), then a rung of people 1 told after it before the next tick - read, it would be an item.
		const TCHAR* const LostAtBirth = TEXT("<supernova on 2 at the succession>");
		const TCHAR* const Inaugurated = TEXT("<inaugurated>");
		const TCHAR* const RoseAfter = TEXT("<people 1 rose after the inauguration>");
		State.Log.Add(LineOf(1800, TEXT("Log.Crisis"), 1, State.Systems[2].SystemID, State.Civs[0].CivID, Rows.Supernova, LostAtBirth));
		State.Log.Add(LineOf(1800, TEXT("Log.Calling"), 2, State.Systems[5].SystemID, State.Civs[1].CivID, CosmicRows::LogInauguration, Inaugurated));
		State.Log.Add(LineOf(1800, TEXT("Log.Ascension"), 2, State.Systems[6].SystemID, State.Civs[1].CivID, RiseRow, RoseAfter));
		Kill(State, 2);
		State.Tick = 1850;
		const FCosmicDigest Digest = CosmicDigest::DigestOf(*Sim, 0);
		TestEqual(TEXT("The digest's span still ends at the next life's birth"), Digest.ThroughTick, static_cast<int64>(1800));
		TestTrue(TEXT("It reads the succession's tick up to the inauguration line: the star lost before it is an item"),
			HasItemAbout(Digest, State.Systems[2].SystemID));
		TestFalse(TEXT("and stops there: the rung told after it on that tick is no item"), HasItemAbout(Digest, State.Civs[1].CivID));
		TestEqual(TEXT("(one item)"), Digest.Items.Num(), 1);
		const TArray<FCosmicChronicleChapter> Chapters = CosmicChronicle::Build(State);
		if (TestEqual(TEXT("(the inauguration's run: two chapters)"), Chapters.Num(), 2))
		{
			const FCosmicChronicleNoVessel& Span = Chapters[1].Before;
			TestTrue(TEXT("The chronicle's span with no vessel is the digest's span"), Span.FromTick == Digest.AfterTick + 1 && Span.ToTick == Digest.ThroughTick);
			TestTrue(TEXT("... with the line the digest read"), LinesHave(Span.Lines, LostAtBirth));
			TestFalse(TEXT("... and neither the inauguration line nor what follows it"), LinesHave(Span.Lines, Inaugurated) || LinesHave(Span.Lines, RoseAfter));
			TestTrue(TEXT("The next life's chapter opens with its inauguration line"), !Chapters[1].Lines.IsEmpty()
				&& Chapters[1].Lines[0].Entry.Message.ToString() == Inaugurated);
			TestTrue(TEXT("... and holds the rung told after it"), LinesHave(Chapters[1].Lines, RoseAfter));
		}

		// The succession on the end's tick (digest-5): the life it stood up gets no chapter; the last chapter's After - the span
		// to the end - stops before that life's inauguration line, as the digest does.
		State.Tick = 1800;
		const FCosmicDigest AtEnd = CosmicDigest::DigestOf(*Sim, 0);
		TestTrue(TEXT("At the end's tick the digest stops before the inauguration line too"), AtEnd.Items.Num() == 1 && HasItemAbout(AtEnd, State.Systems[2].SystemID)
			&& !HasItemAbout(AtEnd, State.Civs[1].CivID));
		const TArray<FCosmicChronicleChapter> Ended = CosmicChronicle::Build(State);
		if (TestEqual(TEXT("(the end's tick: one chapter)"), Ended.Num(), 1))
		{
			const FCosmicChronicleNoVessel& After = Ended[0].After;
			TestTrue(TEXT("The span after the last chapter is the digest's span"), After.FromTick == AtEnd.AfterTick + 1 && After.ToTick == AtEnd.ThroughTick);
			TestTrue(TEXT("... with the line before the inauguration"), LinesHave(After.Lines, LostAtBirth));
			TestFalse(TEXT("The inauguration line of a life born on the end's tick is not in the last chapter's After span"), LinesHave(After.Lines, Inaugurated));
			TestFalse(TEXT("... nor any Log.Calling line"), After.Lines.ContainsByPredicate([](const FCosmicChronicleLine& Line) { return Line.Entry.Category == CosmicTags::LogCalling(); }));
			TestFalse(TEXT("... nor the line told after it"), LinesHave(After.Lines, RoseAfter));
		}
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

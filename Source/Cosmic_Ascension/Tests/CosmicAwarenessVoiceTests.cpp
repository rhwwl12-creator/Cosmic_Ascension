// Cosmic Ascension - automation tests of the awareness line (milestone B item B6a, commit [B-6a5]): in the tick a claim
// first makes a fallen aware of the player, the end of that expiry tells one line of the claimant's AwarenessTemplates -
// Log.Transcendent, Severity 1, about the ruled people, one number of the tell stream - once per run.
// Design: docs/60_DECISIONS/headers/B6a_endings_and_the_run_past_the_third_rung.md section 2.11, 3.2, 3.6, 6 and decision 7.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Log/CosmicChronicle.h"
#include "Log/CosmicLogSubsystem.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sim/CosmicHazards.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/Class.h"
#include "UObject/NameTypes.h"
#include "World/CosmicAutoPause.h"

#include <initializer_list>

namespace CosmicAwarenessVoiceTests
{
	FGameplayTag VoiceTagOf(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// The rows of these tests (names of their own: the awareness tests' rows live in another file of the same unity build).
	const FName VoiceSparkRow(TEXT("crisis_test_voice_spark"));
	const FName VoiceClaimRow(TEXT("crisis_test_voice_claim"));
	const FName VoiceFirstFallen(TEXT("tr_test_voice_first"));
	const FName VoiceSecondFallen(TEXT("tr_test_voice_second"));

	// The systems of the board (LayVoiceBoard), by index into State.Systems - the awareness tests' board.
	constexpr int32 VoiceVesselSystem = 0;   // people 0, settled - the ruler stands here
	constexpr int32 VoiceClaimedSystem = 2;  // people 0, settled, one lane from the sphere - the first expiry runs out here
	constexpr int32 VoiceSphereSystem = 3;   // nobody's - every fallen of the board holds it in its sphere
	constexpr int32 VoiceLaterSystem = 4;    // people 0, settled, one lane from the sphere - a later expiry runs out here

	// The leap of the tests' rules: a death is followed by a succession this many ticks later.
	constexpr int32 VoiceLeap = 5;

	// Three culture-invariant lines "<Stem> <Moment> one/two/three".
	TArray<FText> VoiceLinesOf(const FString& Stem, const TCHAR* Moment)
	{
		const FString Head = FString::Printf(TEXT("%s %s"), *Stem, Moment);
		return TArray<FText>{ FText::AsCultureInvariant(Head + TEXT(" one")), FText::AsCultureInvariant(Head + TEXT(" two")), FText::AsCultureInvariant(Head + TEXT(" three")) };
	}

	// A hazard row of the tests: no spawn chance, Log.Crisis, Severity 1, three lines for each moment.
	FHazardDef VoiceHazardRowOf(const TCHAR* TagName, ECosmicHazardSpawn Rule, int32 Countdown)
	{
		FHazardDef Row;
		Row.HazardTag = VoiceTagOf(TagName);
		Row.DisplayName = FText::AsCultureInvariant(FString::Printf(TEXT("<%s>"), TagName));
		Row.CountdownTicks = Countdown;
		Row.SpawnRule = Rule;
		Row.SpawnChancePerTick = 0.f;
		Row.LogCategory = VoiceTagOf(TEXT("Log.Crisis"));
		Row.Severity = 1;
		Row.OutbreakTemplates = VoiceLinesOf(TagName, TEXT("outbreak"));
		Row.ExpireTemplates = VoiceLinesOf(TagName, TEXT("expiry"));
		Row.ResolveTemplates = VoiceLinesOf(TagName, TEXT("resolution"));
		return Row;
	}

	// The awareness lines of a fallen row of these tests (the AwarenessTemplates the row is given).
	TArray<FText> AwarenessLinesOf(FName Fallen)
	{
		return VoiceLinesOf(Fallen.ToString(), TEXT("aware"));
	}

	// A fallen row of tier 4 with a sphere of twelve; its weight 0, so the roster of NewGalaxy makes no instance of it. Its
	// three kinds of first person are told apart by their words: AwarenessTemplates "<row> aware ...", VoiceTemplates
	// "<row> voice ..." (the hedge's testimony - nothing may read them for the awareness), ClaimTemplates "<row> claim ...".
	FTranscendentDef VoiceFallenRowOf(FName ID, const TCHAR* Doctrine, float Depth)
	{
		FTranscendentDef Row;
		Row.Doctrine = VoiceTagOf(Doctrine);
		Row.Allegiance = VoiceTagOf(TEXT("Realm.Abyssal"));
		Row.Tier = EAscensionTier::Tier4_Transcendent;
		Row.Depth = Depth;
		Row.MaxInfluencedSystems = 12;
		Row.SpawnWeight = 0.f;
		Row.AwarenessTemplates = AwarenessLinesOf(ID);
		Row.VoiceTemplates = VoiceLinesOf(ID.ToString(), TEXT("voice"));
		Row.ClaimTemplates = VoiceLinesOf(ID.ToString(), TEXT("claim"));
		return Row;
	}

	// The awareness tests' quiet tables (Tests/CosmicAwarenessTests.cpp TablesOf without the deposing plague): no inherited
	// edict, no event slot, no first crisis, no periodic petition, no drift and no fleet, no pressure drain, a realm cap of
	// one, claims one lane out, a leap of exactly VoiceLeap ticks, no project or feat, no event or card row. The hazards: a
	// plague of the tests (one tick) and the claim row (300 ticks). The transcendents: two fallen rows with all three kinds
	// of first person, the second deeper (it claims over the first where both reach).
	FCosmicDataTables VoiceTablesOf(const FCosmicDataTables& Original, const FCosmicRulesDef& GameRules)
	{
		FCosmicRulesDef Rules = CosmicTests::WithoutStartingEdict(GameRules);
		Rules.EventChancePerSlot = 0.f;
		Rules.FirstCrisisTick = 0;
		Rules.PetitionPeriodTicks = 100000;
		Rules.RealmCapByTier = { 1, 1, 1, 1, 1 };
		Rules.RelationDriftRate = 0.f;
		Rules.FleetPerSettledByTier = { 0.f, 0.f, 0.f, 0.f, 0.f };
		Rules.SpiritualPressureStabilityDrain = 0.f;
		Rules.ClaimAdjacencyLanes = 1;
		Rules.LeapTicksMin = VoiceLeap;
		Rules.LeapTicksMax = VoiceLeap;
		Rules.LeapTotalCap = 1000000;
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));

		UDataTable* Hazards = CosmicTests::NewTable(FHazardDef::StaticStruct());
		Hazards->AddRow(VoiceSparkRow, VoiceHazardRowOf(TEXT("Hazard.Plague"), ECosmicHazardSpawn::Draw, 1));
		Hazards->AddRow(VoiceClaimRow, VoiceHazardRowOf(TEXT("Hazard.Claim"), ECosmicHazardSpawn::ClaimRule, 300));
		Tables.Hazards = Hazards;

		UDataTable* Transcendents = CosmicTests::NewTable(FTranscendentDef::StaticStruct());
		Transcendents->AddRow(VoiceFirstFallen, VoiceFallenRowOf(VoiceFirstFallen, TEXT("Doctrine.Silence"), 0.f));
		Transcendents->AddRow(VoiceSecondFallen, VoiceFallenRowOf(VoiceSecondFallen, TEXT("Doctrine.Zeal"), 1.f));
		Tables.Transcendents = Transcendents;

		Tables.Events = CosmicTests::NewTable(FCosmicEventDef::StaticStruct());
		Tables.Encounters = CosmicTests::NewTable(FEncounterDef::StaticStruct());
		return Tables;
	}

	// A hazard of the tests on System, running out at the next tick: the claim rule follows its expiry.
	void VoiceExpireOn(FGalaxyState& State, int32 System, const TCHAR* HazardTag)
	{
		State.Systems[System].ActiveHazard = VoiceTagOf(HazardTag);
		State.Systems[System].HazardTicksRemaining = 1;
	}

	// The awareness tests' board (LayAwareBoard) on the live state, the ruled people on the third rung: a galaxy of seed 42
	// with twelve living systems wired as CosmicTests::WireChain; people 0 - the vessel's - holds 0, 1, 2 and 4 with its
	// capital in 0; people 1 holds 6, people 2 holds 7, people 3 holds 11; every Stability 0.75; no hazard, project or
	// notable; the instances of Fallen (in that order, unaware, unsealed) hold VoiceSphereSystem; the vessel rules people 0
	// standing in VoiceVesselSystem; tick 400. ExpiringTag runs out on VoiceClaimedSystem at the next tick. Null when the
	// galaxy does not give that.
	FGalaxyState* LayVoiceBoard(UGalaxySimulationSubsystem& Sim, const TCHAR* ExpiringTag, std::initializer_list<FName> Fallen)
	{
		Sim.NewGalaxy(42, 12, 4);
		FGalaxyState& State = Sim.GetMutableState();
		if (CosmicTests::LivingSystems(State) < 12 || State.Civs.Num() < 4 || State.Player.RuledCivID != State.Civs[0].CivID
			|| State.Calling.LifeState != ELifeState::Embodied || !CosmicTests::WireChain(State))
		{
			return nullptr;
		}
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { VoiceVesselSystem, 1, VoiceClaimedSystem, VoiceLaterSystem });
		CosmicTests::Give(State, 1, { 6 });
		CosmicTests::Give(State, 2, { 7 });
		CosmicTests::Give(State, 3, { 11 });
		for (FStarSystemState& System : State.Systems)
		{
			CosmicHazards::EmptyHazardSlot(System);
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
		}
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.Stability = 0.75f;
			Civ.LowStabilityTicks = 0;
			Civ.Notables.Reset();
		}
		State.Civs[0].Tier = EAscensionTier::Tier3_Causal;
		State.Transcendents.Reset();
		for (const FName ID : Fallen)
		{
			const FTranscendentDef* Def = Sim.GetTables().FindTranscendent(ID);
			if (!Def)
			{
				return nullptr;
			}
			FTranscendentState& Instance = CosmicTests::AddInstance(State, ID, { State.Systems[VoiceSphereSystem].SystemID });
			Instance.Doctrine = Def->Doctrine;
		}
		State.Player.CurrentSystemID = State.Systems[VoiceVesselSystem].SystemID;
		State.Tick = 400;
		VoiceExpireOn(State, VoiceClaimedSystem, ExpiringTag);
		return &State;
	}

	// The indices into State.Log of the Log.Transcendent entries, in log order.
	TArray<int32> AwarenessIndicesOf(const FGalaxyState& State)
	{
		TArray<int32> Indices;
		for (int32 Index = 0; Index < State.Log.Num(); ++Index)
		{
			if (State.Log[Index].Category == CosmicTags::LogTranscendent())
			{
				Indices.Add(Index);
			}
		}
		return Indices;
	}

	// How many Log.Transcendent entries the run holds, in the log and in the chronicle.
	int32 AwarenessCountOf(const FGalaxyState& State)
	{
		int32 Count = AwarenessIndicesOf(State).Num();
		for (const FCosmicLogEntry& Entry : State.Chronicle)
		{
			Count += Entry.Category == CosmicTags::LogTranscendent() ? 1 : 0;
		}
		return Count;
	}

	// The index into State.Log of the first entry of Tick whose SourceRow is Row and whose message holds Word; INDEX_NONE
	// when there is none.
	int32 VoiceLineIndexOf(const FGalaxyState& State, int64 Tick, FName Row, const TCHAR* Word)
	{
		return State.Log.IndexOfByPredicate([Tick, Row, Word](const FCosmicLogEntry& Entry)
		{
			return Entry.Tick == Tick && Entry.SourceRow == Row && Entry.Message.ToString().Contains(Word);
		});
	}

	// The index into State.Log of the first entry of Tick of Category whose SourceRow is Row; INDEX_NONE when there is none.
	int32 CategoryIndexOf(const FGalaxyState& State, int64 Tick, const FGameplayTag& Category, FName Row)
	{
		return State.Log.IndexOfByPredicate([Tick, &Category, Row](const FCosmicLogEntry& Entry)
		{
			return Entry.Tick == Tick && Entry.Category == Category && Entry.SourceRow == Row;
		});
	}

	// Whether the message is one of Lines.
	bool IsOneOf(const FText& Message, const TArray<FText>& Lines)
	{
		const FString Said = Message.ToString();
		return Lines.ContainsByPredicate([&Said](const FText& Line) { return Line.ToString() == Said; });
	}

	// Whether an entry after At in State.Log, of At's tick, is a line of the expiry: a line of the tests' hazard rows (the
	// expiry's own, the new claim's outbreak), the record of an annexation, the claim's sign, or a line of a fallen row of
	// these tests (a first person, the awareness line).
	bool AnyExpiryLineAfter(const FGalaxyState& State, int32 At)
	{
		const FName ExpiryRows[] = { VoiceSparkRow, VoiceClaimRow, CosmicRows::LogRecordAnnexed, CosmicRows::LogSignClaim, VoiceFirstFallen, VoiceSecondFallen };
		for (int32 Index = At + 1; Index < State.Log.Num(); ++Index)
		{
			const FCosmicLogEntry& Entry = State.Log[Index];
			for (const FName Row : ExpiryRows)
			{
				if (Entry.Tick == State.Log[At].Tick && Entry.SourceRow == Row)
				{
					return true;
				}
			}
		}
		return false;
	}

	// Whether any line of the chapters (their bodies and both spans with no vessel) satisfies Pred.
	template <typename TPred>
	bool AnyChapterLine(const TArray<FCosmicChronicleChapter>& Chapters, TPred Pred)
	{
		for (const FCosmicChronicleChapter& Chapter : Chapters)
		{
			for (const TArray<FCosmicChronicleLine>* Lines : { &Chapter.Lines, &Chapter.Before.Lines, &Chapter.After.Lines })
			{
				if (Lines->ContainsByPredicate([&Pred](const FCosmicChronicleLine& Line) { return Pred(Line.Entry); }))
				{
					return true;
				}
			}
		}
		return false;
	}

	// Whether either part of the two books holds a Log.Transcendent entry.
	bool BooksHoldAwareness(const FCosmicTwoBooks& Books)
	{
		auto IsAwareness = [](const FCosmicChronicleLine& Line) { return Line.Entry.Category == CosmicTags::LogTranscendent(); };
		if (Books.Recorded.ContainsByPredicate(IsAwareness))
		{
			return true;
		}
		for (const FCosmicClaimedGroup& Group : Books.Claimed)
		{
			if (Group.Lines.ContainsByPredicate(IsAwareness))
			{
				return true;
			}
		}
		return false;
	}
}

// Pins the awareness line (B6a header section 2.11, 3.2, 3.6 (4), section 6 Log.AwarenessVoice, decision 7-ra): in the tick a
// claim first makes a fallen aware, exactly one Log.Transcendent line, Severity 1, about the ruled people and no system, out
// of the claimant's AwarenessTemplates (mutation: reading VoiceTemplates or ClaimTemplates breaks it); one number of the tell
// stream and none of the sim stream (the twin whose fallen was aware before the tick tells no line and ends in the same sim
// stream, world and Knowing, one tell draw behind - mutation: a write of Knowing in the line's block breaks it); told in
// scope with tell-all off and no auto pause (mutation: Severity 2 breaks it); not in the life chapter nor the two books
// (mutation: the category Log.Claim puts it in the two books), and dropped when it leaves the log - never moved into
// State.Chronicle (mutation: Admits without its Log.Transcendent clause moves it there); the last line
// of that expiry - after the expiry line, the record line, the joined fallen's first person and the new claim's outbreak
// line (mutation: telling it where Place is breaks it); after a succession the next first claim tells no second line
// (mutation: a succession that clears the flags breaks it); with two fallen rows the second transcendent's first claim
// sets its flag and tells no second line (mutation: gating the line on that transcendent's own flag breaks it).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicLogAwarenessVoiceTest, "Cosmic_Ascension.Log.AwarenessVoice", CosmicTests::Flags)
bool FCosmicLogAwarenessVoiceTest::RunTest(const FString& Parameters)
{
	using namespace CosmicAwarenessVoiceTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicLogSubsystem* Log = Sim->GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	if (!TestNotNull(TEXT("(the log subsystem)"), Log))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef GameRules = Sim->GetRules();
	const bool bWasTellAll = Log->IsTellAll();
	ON_SCOPE_EXIT
	{
		Log->SetTellAll(bWasTellAll);
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
		Sim->SetTimeScale(0.f);
	};
	Sim->SetTables(VoiceTablesOf(Original, GameRules));
	Log->SetTellAll(true);
	const FGalaxyState& State = Sim->GetState();
	const FGameplayTag Claim = VoiceTagOf(TEXT("Hazard.Claim"));
	const TArray<FText> FirstAwareness = AwarenessLinesOf(VoiceFirstFallen);

	// (a) The line, on the plague's board with one fallen: its expiry places the claim that makes the fallen aware.
	int32 TellBefore = 0;
	int32 MainTell = 0;
	int32 MainSim = 0;
	uint32 MainWorld = 0;
	FKnowingState MainKnowing;
	{
		FGalaxyState* Laid = LayVoiceBoard(*Sim, TEXT("Hazard.Plague"), { VoiceFirstFallen });
		if (!TestNotNull(TEXT("(the board of the line is laid)"), Laid))
		{
			return false;
		}
		const FName Ruled = Laid->Player.RuledCivID;
		TellBefore = State.TellRngState;
		Sim->RunHeadless(1);
		if (!TestTrue(TEXT("(the expiry placed a claim and made its claimant aware)"), State.Systems[VoiceClaimedSystem].ActiveHazard == Claim && State.Transcendents[0].bAwareOfPlayer))
		{
			return false;
		}
		MainKnowing = State.Knowing;
		const TArray<int32> Lines = AwarenessIndicesOf(State);
		if (TestEqual(TEXT("The tick the flag is written tells exactly one Log.Transcendent line"), Lines.Num(), 1))
		{
			const int32 At = Lines[0];
			const FCosmicLogEntry& Line = State.Log[At];
			TestEqual(TEXT("in that tick"), Line.Tick, State.Tick);
			TestEqual(TEXT("of Severity 1"), static_cast<int32>(Line.Severity), 1);
			TestTrue(TEXT("about the ruled people"), Line.FocusCivID == Ruled);
			TestTrue(TEXT("and about no system"), Line.FocusSystemID.IsNone());
			TestTrue(TEXT("out of the claimant's AwarenessTemplates (not its VoiceTemplates nor its ClaimTemplates)"), IsOneOf(Line.Message, FirstAwareness));
			TestTrue(TEXT("its row the claimant's"), Line.SourceRow == VoiceFirstFallen);
			TestFalse(TEXT("No auto pause stops for it"), CosmicAutoPause::StopsFor(Line));

			// The last line of that expiry: after the plague's expiry line and the new claim's outbreak line, and no line
			// of the expiry after it (mutation: telling it where Place is puts it before the expiry line).
			const int32 ExpiryAt = VoiceLineIndexOf(State, State.Tick, VoiceSparkRow, TEXT("expiry"));
			const int32 OutbreakAt = VoiceLineIndexOf(State, State.Tick, VoiceClaimRow, TEXT("outbreak"));
			if (TestTrue(TEXT("(the expiry line and the claim's outbreak line are told)"), ExpiryAt != INDEX_NONE && OutbreakAt != INDEX_NONE))
			{
				TestTrue(TEXT("The line follows the expiry line"), At > ExpiryAt);
				TestTrue(TEXT("and the new claim's outbreak line"), At > OutbreakAt);
				TestFalse(TEXT("and no line of the expiry follows it"), AnyExpiryLineAfter(State, At));
			}

			// Not in the life chapter nor the two books: heard in the feed alone.
			const FString ExpiryText = ExpiryAt != INDEX_NONE ? State.Log[ExpiryAt].Message.ToString() : FString();
			const TArray<FCosmicChronicleChapter> Chapters = CosmicChronicle::Build(State);
			TestTrue(TEXT("(the life chapter holds the expiry line of the tick)"), AnyChapterLine(Chapters, [&State, &ExpiryText](const FCosmicLogEntry& Entry)
			{
				return Entry.Tick == State.Tick && Entry.Message.ToString() == ExpiryText;
			}));
			TestFalse(TEXT("The awareness line is in no chapter of the chronicle"), AnyChapterLine(Chapters, [](const FCosmicLogEntry& Entry)
			{
				return Entry.Category == CosmicTags::LogTranscendent();
			}));
			const FCosmicTwoBooks Books = CosmicChronicle::BuildTwoBooks(State);
			TestFalse(TEXT("nor in the two books"), BooksHoldAwareness(Books));
			TestTrue(TEXT("which have no line of this board at all (mutation: the category Log.Claim gives part two a line)"), Books.IsEmpty());
		}
		MainTell = State.TellRngState;
		MainSim = Sim->GetRng().GetCurrentSeed();
		MainWorld = Sim->ComputeWorldHash();

		// The line leaves the log as every line the chronicle cannot show leaves it: dropped, never moved into State.Chronicle
		// (CosmicChronicle::Admits - nothing shows it there). MaxLogEntries lines with no subject and no row (Admits holds none
		// of them; Push draws nothing) push every entry of the board out of the log; the expiry line of the tick, which the
		// life chapter shows, moves into the chronicle beside it (mutation: Admits without its Log.Transcendent clause moves
		// the awareness line there too).
		const int32 LeavingExpiryAt = VoiceLineIndexOf(State, State.Tick, VoiceSparkRow, TEXT("expiry"));
		const FString LeavingExpiryText = LeavingExpiryAt != INDEX_NONE ? State.Log[LeavingExpiryAt].Message.ToString() : FString();
		const FGameplayTag Filler = VoiceTagOf(TEXT("Log.Crisis"));
		for (int32 Pushed = 0; Pushed < UCosmicLogSubsystem::MaxLogEntries; ++Pushed)
		{
			Log->Push(Filler, 1, FText::AsCultureInvariant(TEXT("filler")), FFormatNamedArguments(), NAME_None, NAME_None);
		}
		if (TestTrue(TEXT("(every line of the board left the log, and the chronicle took the tick's expiry line)"), AwarenessIndicesOf(State).IsEmpty()
			&& !LeavingExpiryText.IsEmpty() && State.Chronicle.ContainsByPredicate([&LeavingExpiryText](const FCosmicLogEntry& Entry)
			{
				return Entry.Message.ToString() == LeavingExpiryText;
			})))
		{
			TestEqual(TEXT("The awareness line that left the log is dropped: State.Chronicle holds no Log.Transcendent entry"), AwarenessCountOf(State), 0);
		}
	}

	// (b) The twin whose fallen was aware before the tick: no line, the same sim stream and world, one tell draw behind.
	{
		FGalaxyState* Laid = LayVoiceBoard(*Sim, TEXT("Hazard.Plague"), { VoiceFirstFallen });
		if (!TestNotNull(TEXT("(the board of the aware twin is laid)"), Laid))
		{
			return false;
		}
		Laid->Transcendents[0].bAwareOfPlayer = true;
		TestEqual(TEXT("(the twin starts from the same tell stream)"), State.TellRngState, TellBefore);
		Sim->RunHeadless(1);
		TestTrue(TEXT("(the twin's claim is placed)"), State.Systems[VoiceClaimedSystem].ActiveHazard == Claim);
		TestEqual(TEXT("A claim of a fallen aware already tells no awareness line"), AwarenessCountOf(State), 0);
		TestEqual(TEXT("The line draws nothing of the sim stream"), Sim->GetRng().GetCurrentSeed(), MainSim);
		TestTrue(TEXT("and writes nothing of the world"), Sim->ComputeWorldHash() == MainWorld);
		TestTrue(TEXT("nor anything of the knowing"), FKnowingState::StaticStruct()->CompareScriptStruct(&MainKnowing, &State.Knowing, 0));
		FRandomStream Probe(State.TellRngState);
		Probe.RandHelper(3);
		TestTrue(TEXT("The line is exactly one number of the tell stream"), Probe.GetCurrentSeed() == MainTell);
	}

	// (c) Judged by the scope of telling (tell-all off): the viewer of the tick is the ruled people on the third rung, so the
	// line is stored and told, and still stops nothing (mutation: Severity 2 makes StopsFor true).
	Log->SetTellAll(false);
	{
		FGalaxyState* Laid = LayVoiceBoard(*Sim, TEXT("Hazard.Plague"), { VoiceFirstFallen });
		if (!TestNotNull(TEXT("(the board of the scope is laid)"), Laid))
		{
			return false;
		}
		Sim->RunHeadless(1);
		const TArray<int32> Lines = AwarenessIndicesOf(State);
		if (TestEqual(TEXT("With tell-all off the line is stored once"), Lines.Num(), 1))
		{
			const FCosmicLogEntry& Line = State.Log[Lines[0]];
			TestFalse(TEXT("and told: the ruled people's third rung is inside its window"), Line.bUntold);
			TestFalse(TEXT("A told awareness line stops no running game"), CosmicAutoPause::StopsFor(Line));
		}
	}
	Log->SetTellAll(true);

	// (d) The order on the claim's board with two fallen: the claim on ClaimedSystem runs out, the deeper second fallen joins
	// it (the expiry line, the record line, its sign and first person), and the claim rule on the freed slot places the first
	// fallen's claim (its outbreak line) and makes it aware - the awareness line comes after all of them. Then (e) a later
	// expiry: the second fallen's first claim sets its own flag and tells no second line.
	{
		FGalaxyState* Laid = LayVoiceBoard(*Sim, TEXT("Hazard.Claim"), { VoiceFirstFallen, VoiceSecondFallen });
		if (!TestNotNull(TEXT("(the board of the claim's expiry is laid)"), Laid))
		{
			return false;
		}
		const FName ClaimedID = Laid->Systems[VoiceClaimedSystem].SystemID;
		Sim->RunHeadless(1);
		if (!TestTrue(TEXT("(the second fallen joined the system and the first claimed it again, aware)"),
			State.Transcendents[1].InfluencedSystems.Contains(ClaimedID) && State.Systems[VoiceClaimedSystem].ActiveHazard == Claim
			&& State.Transcendents[0].bAwareOfPlayer && !State.Transcendents[1].bAwareOfPlayer))
		{
			return false;
		}
		const TArray<int32> Lines = AwarenessIndicesOf(State);
		if (TestEqual(TEXT("The claim's expiry tells one awareness line"), Lines.Num(), 1))
		{
			const int32 At = Lines[0];
			TestTrue(TEXT("out of the aware claimant's lines"), IsOneOf(State.Log[At].Message, FirstAwareness) && State.Log[At].SourceRow == VoiceFirstFallen);
			const int32 ExpiryAt = VoiceLineIndexOf(State, State.Tick, VoiceClaimRow, TEXT("expiry"));
			const int32 RecordAt = CategoryIndexOf(State, State.Tick, CosmicTags::LogRecord(), CosmicRows::LogRecordAnnexed);
			const int32 FirstPersonAt = CategoryIndexOf(State, State.Tick, CosmicTags::LogClaim(), VoiceSecondFallen);
			const int32 OutbreakAt = VoiceLineIndexOf(State, State.Tick, VoiceClaimRow, TEXT("outbreak"));
			if (TestTrue(TEXT("(the expiry, record, first-person and outbreak lines are told, in that order)"),
				ExpiryAt != INDEX_NONE && RecordAt > ExpiryAt && FirstPersonAt > RecordAt && OutbreakAt > FirstPersonAt))
			{
				TestTrue(TEXT("The awareness line is the last line of the expiry: after the outbreak line"), At > OutbreakAt);
				TestFalse(TEXT("and no line of the expiry follows it"), AnyExpiryLineAfter(State, At));
			}
			const FCosmicTwoBooks Books = CosmicChronicle::BuildTwoBooks(State);
			TestTrue(TEXT("(the two books hold the record and the first person)"), Books.RecordedCandidates == 1 && Books.ClaimedCandidates == 1);
			TestFalse(TEXT("but not the awareness line"), BooksHoldAwareness(Books));
		}

		VoiceExpireOn(*Laid, VoiceLaterSystem, TEXT("Hazard.Plague"));
		Sim->RunHeadless(1);
		TestTrue(TEXT("(the later expiry placed the second fallen's claim)"), State.Systems[VoiceLaterSystem].ActiveHazard == Claim);
		TestTrue(TEXT("The second transcendent's first claim sets its own flag"), State.Transcendents[1].bAwareOfPlayer);
		TestEqual(TEXT("and tells no second awareness line: the second person is once per run"), AwarenessCountOf(State), 1);
	}

	// (f) A succession: the line was told to the first life; the next life's first claim tells none.
	{
		FGalaxyState* Laid = LayVoiceBoard(*Sim, TEXT("Hazard.Plague"), { VoiceFirstFallen });
		if (!TestNotNull(TEXT("(the board of the succession is laid)"), Laid))
		{
			return false;
		}
		Sim->RunHeadless(1);
		TestEqual(TEXT("(the first life is told the line)"), AwarenessCountOf(State), 1);
		Laid->Calling.PendingEffect = CosmicTags::EffectKillPlayer();
		Laid->Calling.PendingCause = VoiceTagOf(TEXT("Death.Hazard"));
		Laid->Calling.PendingSourceRow = VoiceSparkRow;
		Sim->RunHeadless(1);
		if (!TestTrue(TEXT("(the ruler died: the calling is in the interregnum)"), State.Calling.LifeState == ELifeState::Interregnum))
		{
			return false;
		}
		Sim->RunHeadless(VoiceLeap);
		if (!TestTrue(TEXT("(a new vessel is called at the leap's end)"), State.Calling.LifeState == ELifeState::Embodied && State.Calling.Lives.Num() == 2))
		{
			return false;
		}
		// Read before any claim of the new life: no claim has run since the succession (the gate needs an embodied ruler), so
		// nothing could have written the flag again (mutation: a succession that clears the flags breaks it).
		TestTrue(TEXT("The flag stays through the succession"), State.Transcendents[0].bAwareOfPlayer);
		// A called vessel is a wanderer (B5): it is put on a throne by hand, as a coronation would - the gate reads the role and
		// the ruled people only. The new life's ruled people holds LaterSystem on the third rung, and the plague runs out there.
		Laid->Player.Role = EPlayerRole::Ruler;
		Laid->Player.RuledCivID = Laid->Civs[0].CivID;
		const FName RuledID = State.Player.RuledCivID;
		const int32 Ruled = Laid->Civs.IndexOfByPredicate([RuledID](const FCivilizationState& Civ) { return Civ.CivID == RuledID; });
		if (!TestTrue(TEXT("(the new ruled people is in the state)"), Laid->Civs.IsValidIndex(Ruled)))
		{
			return false;
		}
		FStarSystemState& Later = Laid->Systems[VoiceLaterSystem];
		if (Later.OwnerCivID != RuledID)
		{
			for (FCivilizationState& Civ : Laid->Civs)
			{
				Civ.OwnedSystems.Remove(Later.SystemID);
			}
			Later.OwnerCivID = RuledID;
			Laid->Civs[Ruled].OwnedSystems.Add(Later.SystemID);
		}
		Laid->Civs[Ruled].Tier = EAscensionTier::Tier3_Causal;
		VoiceExpireOn(*Laid, VoiceLaterSystem, TEXT("Hazard.Plague"));
		Sim->RunHeadless(1);
		TestTrue(TEXT("(the new life's first claim is placed on its people's system)"), State.Systems[VoiceLaterSystem].ActiveHazard == Claim);
		TestEqual(TEXT("The new life's first claim tells no second awareness line"), AwarenessCountOf(State), 1);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

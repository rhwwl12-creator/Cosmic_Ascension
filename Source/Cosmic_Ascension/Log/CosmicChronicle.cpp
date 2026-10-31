// Cosmic Ascension - Log layer.

#include "Log/CosmicChronicle.h"

#include "Core/CosmicTags.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "GameplayTagContainer.h"

#include <limits>

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicChronicleImpl
{
	const FGameplayTag& CallingTag()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Calling")));
		return Tag;
	}

	const FGameplayTag& ClaimTag()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Claim")));
		return Tag;
	}

	const FGameplayTag& RecordTag()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Record")));
		return Tag;
	}

	// The four ranks of the cap, in the order they are kept (section 8.4 of docs/70; the header's table).
	enum ERank : int32
	{
		Calling = 0,
		OwnSeverity2 = 1,
		AdmittedSeverity1 = 2,
		OtherSeverity2 = 3,
		RankCount = 4,
	};

	struct FCandidate
	{
		const FCosmicLogEntry* Entry = nullptr;
		int32 Order = 0;
		ERank Rank = Calling;
	};

	// The indices kept of a rank of N lines when only K fit: Severity 1 keeps its first ceil(K/2) and its last floor(K/2), any
	// other rank (i * N) / K for i in 0..K-1. Integers only.
	TArray<int32> Thin(int32 N, int32 K, bool bFirstAndLast)
	{
		TArray<int32> Kept;
		if (K <= 0 || N <= 0)
		{
			return Kept;
		}
		if (K >= N)
		{
			for (int32 At = 0; At < N; ++At)
			{
				Kept.Add(At);
			}
			return Kept;
		}
		if (bFirstAndLast)
		{
			const int32 Head = (K + 1) / 2;
			const int32 Tail = K / 2;
			for (int32 At = 0; At < Head; ++At)
			{
				Kept.Add(At);
			}
			for (int32 At = N - Tail; At < N; ++At)
			{
				Kept.Add(At);
			}
			return Kept;
		}
		for (int32 I = 0; I < K; ++I)
		{
			Kept.Add(static_cast<int32>((static_cast<int64>(I) * N) / K));
		}
		return Kept;
	}

	// The four steps of a chapter's lines over Entries[Begin, End) for Peoples within Room: (1) gather what Admits holds but
	// the two books' entries and the fallen's awareness line (Log.Transcendent - the one second person stands in no book of a
	// life's deeds, B6a header section 2.11), (2) drop a repeat of the line just before (OutCandidates counts the rest), (3)
	// the cap by rank (bOutMore set when it leaves lines out), (4) back in time order, a repeat the thinning brought together
	// dropped once more.
	void FillLines(TConstArrayView<const FCosmicLogEntry*> Entries, int32 Begin, int32 End, TConstArrayView<FName> Peoples, int32 Room,
		TArray<FCosmicChronicleLine>& OutLines, int32& OutCandidates, bool& bOutMore)
	{
		// (1) Gather, and (2) drop a repeat of the line just before.
		TArray<FCandidate> Candidates;
		int32 Order = 0;
		for (int32 At = FMath::Max(0, Begin); At < End && At < Entries.Num(); ++At)
		{
			const FCosmicLogEntry& Entry = *Entries[At];
			if (Entry.Category == ClaimTag() || Entry.Category == RecordTag() || Entry.Category == CosmicTags::LogTranscendent()
				|| !CosmicChronicle::Admits(Entry, Peoples))
			{
				continue;
			}
			if (!Candidates.IsEmpty() && Candidates.Last().Entry->Message.ToString().Equals(Entry.Message.ToString(), ESearchCase::CaseSensitive))
			{
				continue;
			}
			const bool bOwnPeople = !Entry.FocusCivID.IsNone() && Peoples.Contains(Entry.FocusCivID);
			const ERank Rank = Entry.Category == CallingTag() ? Calling
				: Entry.Severity >= 2 ? (bOwnPeople ? OwnSeverity2 : OtherSeverity2)
				: AdmittedSeverity1;
			Candidates.Add({ &Entry, Order++, Rank });
		}
		OutCandidates = Candidates.Num();

		// (3) The cap, by rank: Log.Calling whole, then each rank while it fits; the first that does not is thinned, the later
		// ones are left out.
		TArray<FCandidate> Kept;
		if (Candidates.Num() <= Room)
		{
			Kept = Candidates;
		}
		else
		{
			int32 Left = Room;
			for (int32 Rank = Calling; Rank < RankCount; ++Rank)
			{
				TArray<FCandidate> OfRank;
				for (const FCandidate& Candidate : Candidates)
				{
					if (Candidate.Rank == Rank)
					{
						OfRank.Add(Candidate);
					}
				}
				if (Rank == Calling)
				{
					Kept.Append(OfRank);
					Left = FMath::Max(0, Left - OfRank.Num());
					continue;
				}
				if (OfRank.Num() <= Left)
				{
					Kept.Append(OfRank);
					Left -= OfRank.Num();
					continue;
				}
				for (const int32 At : Thin(OfRank.Num(), Left, Rank == AdmittedSeverity1))
				{
					Kept.Add(OfRank[At]);
				}
				Left = 0;
			}
			bOutMore = true;
		}

		// (4) Back in time order, a repeat the thinning brought together is dropped once more.
		Kept.Sort([](const FCandidate& A, const FCandidate& B) { return A.Order < B.Order; });
		for (const FCandidate& Candidate : Kept)
		{
			if (!OutLines.IsEmpty() && OutLines.Last().Entry.Message.ToString().Equals(Candidate.Entry->Message.ToString(), ESearchCase::CaseSensitive))
			{
				continue;
			}
			FCosmicChronicleLine& Line = OutLines.AddDefaulted_GetRef();
			Line.Entry = *Candidate.Entry;
			Line.bOwn = Candidate.Rank != OtherSeverity2;
		}
	}

	// A span with no vessel (B5): its lines over Entries[Begin, End) - its FromTick and ToTick already set - and its 'more'
	// when the saved chronicle's cap let lines of its ticks go (the body's rule 3b).
	void FillSpan(TConstArrayView<const FCosmicLogEntry*> Entries, int32 Begin, int32 End, TConstArrayView<FName> Peoples, int32 Room,
		const FChronicleEviction& Eviction, FCosmicChronicleNoVessel& OutSpan)
	{
		FillLines(Entries, Begin, End, Peoples, Room, OutSpan.Lines, OutSpan.Candidates, OutSpan.bMore);
		if (OutSpan.HasSpan() && Eviction.Count > 0 && OutSpan.FromTick <= Eviction.ThroughTick)
		{
			OutSpan.bMore = true;
		}
	}
}

TArray<FName> CosmicChronicle::PeoplesOf(const FLifeRecord& Life)
{
	TArray<FName> Peoples;
	if (!Life.BirthCivID.IsNone())
	{
		Peoples.Add(Life.BirthCivID);
	}
	for (const FName CivID : Life.RuledCivIDs)
	{
		if (!CivID.IsNone())
		{
			Peoples.AddUnique(CivID);
		}
	}
	return Peoples;
}

bool CosmicChronicle::Admits(const FCosmicLogEntry& Entry, TConstArrayView<FName> Peoples)
{
	using namespace CosmicChronicleImpl;

	// The fallen's awareness line (B6a header section 2.11) is heard in the feed alone: nothing of the chronicle shows it,
	// so it never moves there when it leaves the log (State.Chronicle holds what the chronicle can show and no other).
	if (Entry.Category == CosmicTags::LogTranscendent())
	{
		return false;
	}
	if (Entry.Category == CallingTag() || Entry.Category == ClaimTag() || Entry.Category == RecordTag() || Entry.Severity >= 2)
	{
		return true;
	}
	// The edict lines of the ruled people (B2) and its stance lines (B3) are the ruler's bookkeeping, not the life's story:
	// left out, so they never crowd a chapter. The treaty and siege lines are its story.
	if (Entry.SourceRow == CosmicRows::LogEdictEnacted || Entry.SourceRow == CosmicRows::LogEdictRevoked || Entry.SourceRow == CosmicRows::LogEdictLapsed
		|| Entry.SourceRow == CosmicRows::LogStanceChanged || Entry.SourceRow == CosmicRows::LogStanceLapsed)
	{
		return false;
	}
	// A wanderer's card is the life's own story wherever it was answered (B1): its rows carry the prefix enc_ - the schema
	// binds it - compared without case (a cooked build need not keep the case of a name).
	return Entry.Severity == 1 && ((!Entry.FocusCivID.IsNone() && Peoples.Contains(Entry.FocusCivID)) || Entry.SourceRow == CosmicRows::LogFirstContact
		|| Entry.SourceRow.ToString().StartsWith(TEXT("enc_"), ESearchCase::IgnoreCase));
}

TArray<FName> CosmicChronicle::PeoplesOfEveryLife(const FCallingState& Calling)
{
	TArray<FName> Peoples;
	for (const FLifeRecord& Life : Calling.Lives)
	{
		for (const FName& People : PeoplesOf(Life))
		{
			Peoples.AddUnique(People);
		}
	}
	return Peoples;
}

bool CosmicChronicle::MayLetGo(const FCosmicLogEntry& Entry)
{
	using namespace CosmicChronicleImpl;

	return Entry.Severity == 1 && Entry.Category != CallingTag() && Entry.Category != ClaimTag() && Entry.Category != RecordTag();
}

TArray<FCosmicChronicleChapter> CosmicChronicle::Build(const FGalaxyState& State)
{
	return BuildCapped(State, MaxLifeLines);
}

int32 CosmicChronicle::ShownLivesOf(const FGalaxyState& State)
{
	const TArray<FLifeRecord>& Lives = State.Calling.Lives;
	return (Lives.Num() > 1 && Lives.Last().BornTick == State.Tick) ? Lives.Num() - 1 : Lives.Num();
}

bool CosmicChronicle::OpensCalledChapter(const FCosmicLogEntry& Entry, const FLifeRecord& Called)
{
	// The inauguration line ([B-5d1] - UPlayerRoleSubsystem::Succeed tells it right after Install, at the tenth stage): the
	// line of Called's BornTick tick whose row is CosmicRows::LogInauguration, or its second row LogInaugurationUnnamed (told
	// in its place after a fallen's first person about the birth system). One succession a tick, so one such line.
	return Entry.Tick == Called.BornTick && (Entry.SourceRow == CosmicRows::LogInauguration || Entry.SourceRow == CosmicRows::LogInaugurationUnnamed);
}

bool CosmicChronicle::IsUnseenLifeLine(const FGalaxyState& State, const FCosmicLogEntry& Entry, bool bRunEnded)
{
	// The vessel the end's tick installed was never shown (ShownLivesOf): its inauguration line is that life's alone.
	const TArray<FLifeRecord>& Lives = State.Calling.Lives;
	return bRunEnded && ShownLivesOf(State) < Lives.Num() && OpensCalledChapter(Entry, Lives.Last());
}

TArray<FCosmicChronicleChapter> CosmicChronicle::BuildCapped(const FGalaxyState& State, int32 Cap)
{
	using namespace CosmicChronicleImpl;

	TArray<FCosmicChronicleChapter> Chapters;
	const TArray<FLifeRecord>& Lives = State.Calling.Lives;
	const int32 Shown = ShownLivesOf(State);
	const int32 Room = FMath::Max(0, Cap);
	const int32 NoVesselRoom = FMath::Min(Room, MaxNoVesselLines);

	// Every entry once, the chronicle before the log (the older entries moved there): the chapters and the spans with no
	// vessel cut this one sequence, so no entry stands in two of them.
	TArray<const FCosmicLogEntry*> Entries;
	Entries.Reserve(State.Chronicle.Num() + State.Log.Num());
	for (const FCosmicLogEntry& Entry : State.Chronicle)
	{
		Entries.Add(&Entry);
	}
	for (const FCosmicLogEntry& Entry : State.Log)
	{
		Entries.Add(&Entry);
	}
	// The first entry after Tick (the entries are in time order).
	auto FirstAfter = [&Entries](int64 Tick)
	{
		int32 At = 0;
		while (At < Entries.Num() && Entries[At]->Tick <= Tick)
		{
			++At;
		}
		return At;
	};
	// Where a life called after a death opens its chapter inside its BornTick tick (OpensCalledChapter, the rule the digest
	// reads too): the first entry of that tick that opens it, else the first entry after the tick.
	auto OpensAt = [&Entries, &FirstAfter](const FLifeRecord& Called)
	{
		const int32 TickEnd = FirstAfter(Called.BornTick);
		for (int32 At = FirstAfter(Called.BornTick - 1); At < TickEnd; ++At)
		{
			if (OpensCalledChapter(*Entries[At], Called))
			{
				return At;
			}
		}
		return TickEnd;
	};
	// The span with no vessel reads every shown life's peoples (PeoplesOfEveryLife's rule, without a life never shown).
	TArray<FName> EveryPeople;
	for (int32 LifeAt = 0; LifeAt < Shown; ++LifeAt)
	{
		for (const FName& People : PeoplesOf(Lives[LifeAt]))
		{
			EveryPeople.AddUnique(People);
		}
	}

	// The end of the last body, and whether that life died: the entries from there to the next body are its span with no
	// vessel.
	int32 PreviousEnd = 0;
	bool bPreviousDied = false;
	for (int32 LifeAt = 0; LifeAt < Shown; ++LifeAt)
	{
		const FLifeRecord& Life = Lives[LifeAt];
		FCosmicChronicleChapter& Chapter = Chapters.AddDefaulted_GetRef();
		Chapter.LifeAt = LifeAt;
		Chapter.FromTick = LifeAt == 0 ? std::numeric_limits<int64>::min() : Life.BornTick;
		Chapter.ToTick = Life.DiedTick > 0 ? Life.DiedTick : (Lives.IsValidIndex(LifeAt + 1) ? Lives[LifeAt + 1].BornTick - 1 : State.Tick);

		// The body: a life called after a death from where it opens in its BornTick tick (OpensAt - its inauguration line; with
		// none in that tick, the first entry after it), any other from its BornTick tick (every earlier tick for the first), never
		// before the end of the body before it. It ends with its ToTick - and, when a life called after its death opens on
		// that tick (a leap of 0), where that life opens: the tick of the death is the dead life's up to there.
		const int32 BodyBegin = LifeAt == 0 ? 0 : FMath::Max(bPreviousDied ? OpensAt(Life) : FirstAfter(Life.BornTick - 1), PreviousEnd);
		int32 BodyEnd = FMath::Max(FirstAfter(Chapter.ToTick), BodyBegin);
		if (Life.DiedTick > 0 && Lives.IsValidIndex(LifeAt + 1))
		{
			BodyEnd = FMath::Max(FMath::Min(BodyEnd, OpensAt(Lives[LifeAt + 1])), BodyBegin);
		}
		if (LifeAt > 0 && bPreviousDied)
		{
			// From the tick after the death through the BornTick tick (its lines before the life opens): no span for a leap of 0.
			Chapter.Before.FromTick = Lives[LifeAt - 1].DiedTick + 1;
			Chapter.Before.ToTick = Life.BornTick;
			FillSpan(Entries, PreviousEnd, BodyBegin, EveryPeople, NoVesselRoom, State.ChronicleEviction, Chapter.Before);
		}
		FillLines(Entries, BodyBegin, BodyEnd, PeoplesOf(Life), Room, Chapter.Lines, Chapter.Candidates, Chapter.bMore);

		// (3b) The saved chronicle's cap let older lines go (MVP item 5): a chapter whose range reaches them says so too. The
		// record does not say whose lines they were, so every chapter it reaches says it - more than the truth, never less.
		if (State.ChronicleEviction.Count > 0 && Chapter.FromTick <= State.ChronicleEviction.ThroughTick)
		{
			Chapter.bMore = true;
		}
		PreviousEnd = BodyEnd;
		bPreviousDied = Life.DiedTick > 0;
	}
	// The span after the last shown life's death, when no shown life follows: up to State.Tick - with a life born on the
	// end's tick (not shown, ShownLivesOf), up to where that life opens, so its inauguration line is in no span.
	if (!Chapters.IsEmpty() && bPreviousDied)
	{
		FCosmicChronicleChapter& Last = Chapters.Last();
		Last.After.FromTick = Lives[Last.LifeAt].DiedTick + 1;
		Last.After.ToTick = State.Tick;
		const int32 AfterEnd = Lives.IsValidIndex(Shown) ? OpensAt(Lives[Shown]) : FirstAfter(State.Tick);
		FillSpan(Entries, PreviousEnd, FMath::Max(AfterEnd, PreviousEnd), EveryPeople, NoVesselRoom, State.ChronicleEviction, Last.After);
	}
	return Chapters;
}

FText CosmicChronicle::VesselNameOf(const FLifeRecord& Life, const FCosmicDataTables& Tables)
{
	const FCivArchetypeDef* Archetype = Tables.FindCiv(Life.BirthArchetypeRow);
	if (!Archetype || Archetype->PersonNameBank.IsEmpty())
	{
		Archetype = Tables.FindCiv(CosmicRows::ArchDefault);
	}
	if (!Archetype || Archetype->PersonNameBank.IsEmpty())
	{
		return FText::GetEmpty();
	}
	const int32 Count = Archetype->PersonNameBank.Num();
	return Archetype->PersonNameBank[((Life.NameIndex % Count) + Count) % Count];
}

FCosmicTwoBooks CosmicChronicle::BuildTwoBooks(const FGalaxyState& State)
{
	using namespace CosmicChronicleImpl;

	// The two categories alone, in the one sequence the chapters read (the chronicle before the log - both in time order).
	// Every life and every span alike: the books are the run's, not a life's.
	TArray<const FCosmicLogEntry*> Records;
	TArray<FName> Claimants;
	TArray<TArray<const FCosmicLogEntry*>> Claims;
	auto Gather = [&](const TArray<FCosmicLogEntry>& Entries)
	{
		for (const FCosmicLogEntry& Entry : Entries)
		{
			if (Entry.Category == RecordTag())
			{
				Records.Add(&Entry);
			}
			else if (Entry.Category == ClaimTag())
			{
				int32 Group = Claimants.IndexOfByKey(Entry.SourceRow);
				if (Group == INDEX_NONE)
				{
					Group = Claimants.Add(Entry.SourceRow);
					Claims.AddDefaulted();
				}
				Claims[Group].Add(&Entry);
			}
		}
	};
	Gather(State.Chronicle);
	Gather(State.Log);

	auto LineOf = [](const FCosmicLogEntry& Entry)
	{
		FCosmicChronicleLine Line;
		Line.Entry = Entry;
		Line.bOwn = true;
		return Line;
	};

	FCosmicTwoBooks Books;
	Books.RecordedCandidates = Records.Num();
	for (const TArray<const FCosmicLogEntry*>& Group : Claims)
	{
		Books.ClaimedCandidates += Group.Num();
	}
	// Part one's rows follow from the two counts alone (its head, at most MaxBookItems lines, its 'more'), so part two is
	// dealt first and part one then chosen to hold what part two shows (below).
	const int32 PartOneLines = FMath::Min(Records.Num(), MaxBookItems);
	// Part one holds part two (decision 10-ga): a part two over an empty part one says that part one is not whole.
	Books.bRecordedMore = PartOneLines < Records.Num() || (Records.IsEmpty() && Books.ClaimedCandidates > 0);

	// Part two: what the chapter's rows leave after part one and part two's head (nothing to deal when it has no entry).
	const int32 PartOneRows = 1 + PartOneLines + (Books.bRecordedMore ? 1 : 0);
	int32 RowsLeft = MaxBookLines - PartOneRows - 1;
	TArray<int32> Quota;
	Quota.Init(0, Claims.Num());
	if (Books.ClaimedCandidates <= MaxBookItems && Claims.Num() + Books.ClaimedCandidates <= RowsLeft)
	{
		for (int32 Group = 0; Group < Claims.Num(); ++Group)
		{
			Quota[Group] = Claims[Group].Num();
		}
	}
	else
	{
		// Over a cap: 'more' takes a row, and the lines are dealt one a group in turn while both caps allow - a group's
		// first line costs its head too - so every claimant that fits stands before any has a second line.
		Books.bClaimedMore = true;
		RowsLeft -= 1;
		int32 ItemsLeft = MaxBookItems;
		bool bDealt = true;
		while (bDealt)
		{
			bDealt = false;
			for (int32 Group = 0; Group < Claims.Num(); ++Group)
			{
				const int32 Cost = Quota[Group] == 0 ? 2 : 1;
				if (Quota[Group] < Claims[Group].Num() && ItemsLeft > 0 && Cost <= RowsLeft)
				{
					++Quota[Group];
					--ItemsLeft;
					RowsLeft -= Cost;
					bDealt = true;
				}
			}
		}
	}
	for (int32 Group = 0; Group < Claims.Num(); ++Group)
	{
		if (Quota[Group] <= 0)
		{
			continue;
		}
		FCosmicClaimedGroup& Shown = Books.Claimed.AddDefaulted_GetRef();
		Shown.TranscendentID = Claimants[Group];
		Shown.Candidates = Claims[Group].Num();
		for (const int32 At : Thin(Claims[Group].Num(), Quota[Group], true))
		{
			Shown.Lines.Add(LineOf(*Claims[Group][At]));
		}
	}

	// Part one, in time order. Over MaxBookItems its slots go first to the record of each loss part two shows - a record and
	// its claim share their FocusSystemID and tick (ExpireHazard tells the record just before the claim's first person) - so
	// no shown claim stands over a place part one leaves out (decision 10-ga, header section 6); part two shows at most
	// MaxBookItems lines, so they always fit. The slots left go to the other records, their first ceil(K/2) and last floor(K/2).
	// A claim with no such record (a save from before [B-5d1]) takes no slot.
	TArray<bool> Kept;
	Kept.Init(PartOneLines == Records.Num(), Records.Num());
	if (PartOneLines < Records.Num())
	{
		int32 SlotsLeft = PartOneLines;
		for (const FCosmicClaimedGroup& Group : Books.Claimed)
		{
			for (const FCosmicChronicleLine& Line : Group.Lines)
			{
				for (int32 At = 0; At < Records.Num() && SlotsLeft > 0 && !Line.Entry.FocusSystemID.IsNone(); ++At)
				{
					if (!Kept[At] && Records[At]->FocusSystemID == Line.Entry.FocusSystemID && Records[At]->Tick == Line.Entry.Tick)
					{
						Kept[At] = true;
						--SlotsLeft;
						break;
					}
				}
			}
		}
		TArray<int32> Others;
		for (int32 At = 0; At < Records.Num(); ++At)
		{
			if (!Kept[At])
			{
				Others.Add(At);
			}
		}
		for (const int32 At : Thin(Others.Num(), SlotsLeft, true))
		{
			Kept[Others[At]] = true;
		}
	}
	for (int32 At = 0; At < Records.Num(); ++At)
	{
		if (Kept[At])
		{
			Books.Recorded.Add(LineOf(*Records[At]));
		}
	}
	return Books;
}

FText CosmicChronicle::ClaimantTitleOf(FName TranscendentID, const FCosmicDataTables& Tables)
{
	const FTranscendentDef* Claimant = TranscendentID.IsNone() ? nullptr : Tables.FindTranscendent(TranscendentID);
	return Claimant ? Claimant->Title : FText::GetEmpty();
}

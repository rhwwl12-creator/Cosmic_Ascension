// Cosmic Ascension - Log layer.

#include "Log/CosmicChronicle.h"

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

TArray<FCosmicChronicleChapter> CosmicChronicle::BuildCapped(const FGalaxyState& State, int32 Cap)
{
	using namespace CosmicChronicleImpl;

	TArray<FCosmicChronicleChapter> Chapters;
	const TArray<FLifeRecord>& Lives = State.Calling.Lives;
	const int32 Room = FMath::Max(0, Cap);
	for (int32 LifeAt = 0; LifeAt < Lives.Num(); ++LifeAt)
	{
		const FLifeRecord& Life = Lives[LifeAt];
		FCosmicChronicleChapter& Chapter = Chapters.AddDefaulted_GetRef();
		Chapter.LifeAt = LifeAt;
		Chapter.FromTick = LifeAt == 0 ? std::numeric_limits<int64>::min() : Life.BornTick;
		Chapter.ToTick = Life.DiedTick > 0 ? Life.DiedTick : (Lives.IsValidIndex(LifeAt + 1) ? Lives[LifeAt + 1].BornTick - 1 : State.Tick);
		const TArray<FName> Peoples = PeoplesOf(Life);

		// (1) Gather, the chronicle before the log (the older entries moved there), and (2) drop a repeat of the line just before.
		TArray<FCandidate> Candidates;
		int32 Order = 0;
		auto Gather = [&](const TArray<FCosmicLogEntry>& Entries)
		{
			for (const FCosmicLogEntry& Entry : Entries)
			{
				if (Entry.Tick < Chapter.FromTick || Entry.Tick > Chapter.ToTick || Entry.Category == ClaimTag() || Entry.Category == RecordTag() || !Admits(Entry, Peoples))
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
		};
		Gather(State.Chronicle);
		Gather(State.Log);
		Chapter.Candidates = Candidates.Num();

		// (3) The cap, by rank: Log.Calling whole, then each rank while it fits; the first that does not is thinned, the later ones
		// are left out.
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
			Chapter.bMore = true;
		}

		// (3b) The saved chronicle's cap let older lines go (MVP item 5): a chapter whose range reaches them says so too. The
		// record does not say whose lines they were, so every chapter it reaches says it - more than the truth, never less.
		if (State.ChronicleEviction.Count > 0 && Chapter.FromTick <= State.ChronicleEviction.ThroughTick)
		{
			Chapter.bMore = true;
		}

		// (4) Back in time order, a repeat the thinning brought together is dropped once more.
		Kept.Sort([](const FCandidate& A, const FCandidate& B) { return A.Order < B.Order; });
		for (const FCandidate& Candidate : Kept)
		{
			if (!Chapter.Lines.IsEmpty() && Chapter.Lines.Last().Entry.Message.ToString().Equals(Candidate.Entry->Message.ToString(), ESearchCase::CaseSensitive))
			{
				continue;
			}
			FCosmicChronicleLine& Line = Chapter.Lines.AddDefaulted_GetRef();
			Line.Entry = *Candidate.Entry;
			Line.bOwn = Candidate.Rank != OtherSeverity2;
		}
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

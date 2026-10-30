// Cosmic Ascension - Log layer (Core Logic). Design: docs/70_CONTENT_GUIDE.md section 8.4, headers/A4_provisional_end.md (MVP item 4).
// RULE: Core Logic - no UI/World include. The chronicle's minimal form: pure functions of the saved state - nothing saved, hashed,
// drawn or written; Knowing is never read (16 #49). Build reads; UCosmicLogSubsystem is the one writer of the move and the cap (MVP item 5).

#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "Core/GalaxyState.h"

struct FCosmicDataTables;

/** One line of a chapter: an entry of State.Chronicle or State.Log as stored (its Message already flattened). */
struct FCosmicChronicleLine
{
	FCosmicLogEntry Entry;
	bool bOwn = false;   // the life's own (Log.Calling, its peoples' Severity 2, its admitted Severity 1); false for another people's Severity 2 (dimmed)
};

/**
 * The lines of a span the calling spent with no vessel (B5 header section 2.7, 4): the entries after the death's tick up
 * to where the next life's chapter opens (CosmicChronicle::OpensCalledChapter), shown under ST_CA Chronicle.NoVessel - at
 * the head of the next life's chapter, or at the end of the last chapter when no shown life follows. No chapter of its own
 * and no page between lives (decision 14). Built at display time, never saved.
 */
struct FCosmicChronicleNoVessel
{
	int64 FromTick = 0;                   // the tick after the death
	int64 ToTick = -1;                    // the last tick of the span: the next life's BornTick (that tick's lines before its inauguration line - every line of
	                                      // it when the tick holds none), State.Tick for After; below FromTick: no span (a leap of 0)
	TArray<FCosmicChronicleLine> Lines;   // shown, in time order; chosen and capped as a chapter's (every life's peoples, MaxNoVesselLines)
	int32 Candidates = 0;                 // the lines left after the first repeat rule, before the cap
	bool bMore = false;                   // the cap left lines out, or the saved chronicle's cap let older lines of the span go

	/** Whether the calling spent ticks with no vessel here (a leap of 0 has none; the sub-head stands only over lines, the widget's rule). */
	bool HasSpan() const { return ToTick >= FromTick; }
};

/** One life's chapter ("the Nth", section 8.4). Built at display time, never saved. */
struct FCosmicChronicleChapter
{
	int32 LifeAt = INDEX_NONE;            // index into Calling.Lives ("the Nth" is Lives[LifeAt].LifeIndex)
	int64 FromTick = 0;                   // docs/21's BornTick/DiedTick range: from BornTick (the first life also every earlier tick) ...
	int64 ToTick = 0;                     // ... to DiedTick when set, else to the next life's BornTick - 1, else to State.Tick
	TArray<FCosmicChronicleLine> Lines;   // the body, shown in time order; copies, so the page outlives the next tick
	int32 Candidates = 0;                 // the body's lines left after the first repeat rule, before the cap (the commandlet's "of")
	bool bMore = false;                   // the cap left lines out, or the saved chronicle's cap let older lines go (ChronicleEviction): the chapter closes with ST_CA Chronicle.More (no count)
	FCosmicChronicleNoVessel Before;      // B5: a called life's head - the span with no vessel that led to it (no span for the first life)
	FCosmicChronicleNoVessel After;       // B5: the last chapter's tail - the span with no vessel after its death when no shown life follows
};

/** One claimant's lines in the second part of the two books (B5 header section 4, decision 10-na): the Log.Claim entries whose
 * SourceRow names one transcendent (the first-person line's SourceRow is its DT_Transcendents row - B4), under ST_CA
 * Chronicle.ClaimedBy with the row's Title (CosmicChronicle::ClaimantTitleOf). Grouped by transcendent, never by people: a
 * people's group would read as that people's forsaken. */
struct FCosmicClaimedGroup
{
	FName TranscendentID;                 // the lines' SourceRow (NAME_None groups the lines that name none)
	TArray<FCosmicChronicleLine> Lines;   // shown, in time order (bOwn true: one opacity for the whole chapter)
	int32 Candidates = 0;                 // the claimant's lines before the cap
};

/**
 * The two books' chapter (B5 header section 4, decisions 10 and 11; docs/70 section 8.4): after the life chapters, before
 * the epilogue's fixed lines. Part one (ST_CA Chronicle.BookRecorded): every Log.Record entry - the narrator's record of an
 * irreversible loss, a place and an event, never a person. Part two (ST_CA Chronicle.BookClaimed): every Log.Claim entry -
 * the fallen's first-person claim over the same loss - grouped by claimant. Part one holds part two (decision 10-ga): it stands
 * whenever the chapter does; part two stands only over a line. No line of any other category (a death, an inauguration, a
 * sign - Log.Sign is told apart by its category - or a vessel's line) is ever in it. Built at display time, never saved;
 * reads neither bUntold nor Knowing (the book is not the player's - 16 #49); no number but each line's cycle stamp.
 */
struct FCosmicTwoBooks
{
	TArray<FCosmicChronicleLine> Recorded;  // part one, in time order: at most MaxBookItems (over it the records of part two's shown lines first,
	                                        // then the others' first and last - BuildTwoBooks)
	int32 RecordedCandidates = 0;           // every Log.Record entry, before the cap
	bool bRecordedMore = false;             // ST_CA Chronicle.More closes part one: the cap left lines out, or part one has no line while part two
	                                        // has (a loss whose record is missing - a save from before [B-5d1]): said, never a bare head
	TArray<FCosmicClaimedGroup> Claimed;    // part two: the groups in the order of their first line, each in time order
	int32 ClaimedCandidates = 0;            // every Log.Claim entry, before the cap
	bool bClaimedMore = false;              // ST_CA Chronicle.More closes part two: the caps left lines (or whole groups) out

	/** No chapter: neither part has an entry. */
	bool IsEmpty() const { return RecordedCandidates == 0 && ClaimedCandidates == 0; }

	/** The chapter's rows with every group's head (a head whose claimant has no Title is left out by the page - one row
	 * fewer): part one's head, its lines and 'more'; part two's head, each group's head and lines, and 'more'. At most
	 * CosmicChronicle::MaxBookLines; 0 for no chapter. */
	int32 NumRows() const
	{
		if (IsEmpty())
		{
			return 0;
		}
		int32 Rows = 1 + Recorded.Num() + (bRecordedMore ? 1 : 0);
		if (!Claimed.IsEmpty())
		{
			Rows += 1 + (bClaimedMore ? 1 : 0);
			for (const FCosmicClaimedGroup& Group : Claimed)
			{
				Rows += 1 + Group.Lines.Num();
			}
		}
		return Rows;
	}
};

namespace CosmicChronicle
{
	/** Working cap of a life chapter's lines, set from the A-4a measurement. A display cap: nothing leaves the state. */
	inline constexpr int32 MaxLifeLines = 40;

	/** B5 ([B-5c] draft - the hand check of decision 22 tunes it): the cap of the lines of a span with no vessel - the world's
	 * news between two lives, not a life's story (the two books' block, docs/70 section 8.4, holds as many). A display cap. */
	inline constexpr int32 MaxNoVesselLines = 12;

	/** BirthCivID, then RuledCivIDs, without NAME_None and without repeats. */
	COSMIC_ASCENSION_API TArray<FName> PeoplesOf(const FLifeRecord& Life);

	/** Every life's peoples: PeoplesOf of each FLifeRecord of Calling.Lives, in life order, without repeats (MVP item 5's move filter, 16 #40 (7));
	 * empty when there is no life. */
	COSMIC_ASCENSION_API TArray<FName> PeoplesOfEveryLife(const FCallingState& Calling);

	/** 16 #31 (2) from the eight fields alone: Log.Calling, Log.Claim, Log.Record and every Severity 2; a Severity 1 entry when FocusCivID is one of Peoples,
	 * SourceRow is CosmicRows::LogFirstContact, or SourceRow starts with enc_ (a wanderer's card answered, B1 - compared without case) - but never the three
	 * edict lines of the ruled people, SourceRow CosmicRows::LogEdictEnacted, LogEdictRevoked or LogEdictLapsed (B2: the ruler's bookkeeping, Severity 1), nor its
 * two stance lines, LogStanceChanged and LogStanceLapsed (B3 - the treaty and siege lines are admitted).
	 * Reads neither bUntold nor a table. MVP item 5's Chronicle filter (16 #40 (7)) calls it with every life's peoples. */
	COSMIC_ASCENSION_API bool Admits(const FCosmicLogEntry& Entry, TConstArrayView<FName> Peoples);

	/** Whether the chronicle's cap may let Entry go (MVP item 5, 16 #40 (7)): a Severity 1 entry that is not Log.Calling, Log.Claim or Log.Record. Every other
	 * entry the chronicle holds is kept for good (Law.Judgment). The commandlet counts the kept ones; the cap of item 5 reads it. */
	COSMIC_ASCENSION_API bool MayLetGo(const FCosmicLogEntry& Entry);

	/** The lives that get a chapter (B5 header section 4): every life of Calling.Lives but a called one (not the first) born on State.Tick - the
	 * chronicle is read where the run ended, and a vessel the end's tick installed was never shown (the end wins over the inauguration page, section
	 * 2.6); its span with no vessel goes to the end of the chapter before. Lives.Num() otherwise. */
	COSMIC_ASCENSION_API int32 ShownLivesOf(const FGalaxyState& State);

	/** B5 (header section 2.7, 4): whether Entry opens Called's chapter - a life called after a death: true for its inauguration line alone, the entry of
	 * Called's BornTick tick whose SourceRow is CosmicRows::LogInauguration or its second row LogInaugurationUnnamed (told in its place after a fallen's first
	 * person about the birth system - docs/70 section 8.3 rule 5) (UPlayerRoleSubsystem::Succeed tells it after Install, [B-5d1]). The lines of the
	 * tick before it are the span with no vessel (a leap above 0) or the end of the chapter of the life that died on it (a leap of 0 - its death line
	 * among them), the lines from it on are Called's. The one rule of that cut: BuildCapped (the next chapter's Before and body; the death's chapter with
	 * a leap of 0; the last chapter's After when the life born on the end's tick is not shown - so the inauguration line never stands under
	 * Chronicle.NoVessel) and CosmicDigest::DigestOf (the end of its span) both read it, so the chronicle and the digest read the same lines. A called
	 * life whose BornTick tick holds no such line opens after that tick: the whole tick is the span's with a leap above 0, and with a leap of 0 the
	 * chapter of the life that died on it keeps it. */
	COSMIC_ASCENSION_API bool OpensCalledChapter(const FCosmicLogEntry& Entry, const FLifeRecord& Called);

	/** B5 ([B-5d1] - header section 4): whether Entry is a line of a life the player never saw - bRunEnded (the presentation's latch of the end:
	 * ACosmicPlayerController::IsRunEnded; the state does not know the end), a life ShownLivesOf leaves out (the called one the end's tick installed), and
	 * Entry its inauguration line (OpensCalledChapter of the last life: either row, on its BornTick tick). The screens that read where the run ended hold it
	 * back as they hold that vessel back (ACosmicPlayerController::IsVesselShown): the log feed skips it (UCosmicLogFeedWidget), as the chronicle's spans
	 * stop before it. Reads; false while the run goes on. */
	COSMIC_ASCENSION_API bool IsUnseenLifeLine(const FGalaxyState& State, const FCosmicLogEntry& Entry, bool bRunEnded);

	/** One chapter per shown life (ShownLivesOf - none for no life), over State.Chronicle then State.Log (already in time order). A body in four steps: gather
	 * what Admits holds for the life's peoples, but Log.Claim and Log.Record (the two books', B5); drop a repeat of the entry just before (Candidates counts the
	 * rest); over MaxLifeLines keep by rank - Log.Calling whole, own Severity 2, admitted Severity 1, other peoples' Severity 2 - thinning the first rank that
	 * does not fit (Severity 1: first ceil(k/2) and last floor(k/2); others: index (i * n) / k), the later ranks none, bMore set; in time order again, drop a
	 * repeat once more (not in bMore). B5 (header section 2.7, 4): a life that died closes its body with the entries of its DiedTick; the entries after them
	 * up to the next life's body are the span with no vessel - the same four steps over every shown life's peoples, Log.Claim and Log.Record left out as
	 * for a body, capped at MaxNoVesselLines - in the next chapter's Before, or in the last chapter's After when no shown life follows (with a life born on
	 * the end's tick, up to where that life would open). Where a life called after a death opens inside its BornTick tick is OpensCalledChapter's (the one
	 * rule the digest reads too): its body starts there, the span before ends there, and with a leap of 0 the death's chapter ends there. A life whose life
	 * before did not die starts at its BornTick tick's first entry. */
	COSMIC_ASCENSION_API TArray<FCosmicChronicleChapter> Build(const FGalaxyState& State);

	/** The same over a cap given (tests and the measurement; Build is BuildCapped(State, MaxLifeLines)). A cap below 0 reads as 0; a span with no vessel
	 * takes the smaller of Cap and MaxNoVesselLines. */
	COSMIC_ASCENSION_API TArray<FCosmicChronicleChapter> BuildCapped(const FGalaxyState& State, int32 Cap);

	/** {Player}: PersonNameBank[NameIndex % Num] of BirthArchetypeRow, else of arch_default (Install's rule); empty when neither resolves ("the Nth" alone). */
	COSMIC_ASCENSION_API FText VesselNameOf(const FLifeRecord& Life, const FCosmicDataTables& Tables);

	/** B5 (header section 4, docs/70 section 8.4): the most lines of each part of the two books, and of their whole chapter
	 * (every head and 'more' counted). Display caps: nothing leaves the state. */
	inline constexpr int32 MaxBookItems = 12;
	inline constexpr int32 MaxBookLines = 28;

	/** B5 (header section 4, decisions 10 and 11): the two books over State.Chronicle then State.Log (in time order), every
	 * life and every span alike. Part two: every Log.Claim entry grouped by SourceRow (the claimant), the groups in the order
	 * of their first line; while the whole of it fits MaxBookItems lines and the rows part one leaves of MaxBookLines, all of
	 * it; else bClaimedMore and its 'more' row, and the lines dealt one a group in turn (a group's first line costs its head
	 * too) while both caps allow, each group keeping its first ceil(K/2) and last floor(K/2). Part one: every Log.Record entry;
	 * over MaxBookItems (bRecordedMore set - and set as well when it has none while part two has some) its slots go first to
	 * the record of each shown claim (the same FocusSystemID and Tick - part one holds part two, decision 10-ga), then to the
	 * other records' first ceil(K/2) and last floor(K/2), the whole in time order. No repeat rule (each line is one loss); no
	 * other category; bUntold and Knowing are not read. */
	COSMIC_ASCENSION_API FCosmicTwoBooks BuildTwoBooks(const FGalaxyState& State);

	/** The head of a claimant's group: the Title of its DT_Transcendents row (a title, never a name - B4); empty when the row
	 * or its Title is missing (the page then shows the group's lines with no head). */
	COSMIC_ASCENSION_API FText ClaimantTitleOf(FName TranscendentID, const FCosmicDataTables& Tables);
}

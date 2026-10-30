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
	                                      // it until that line exists, [B-5d1]), State.Tick for After; below FromTick: no span (a leap of 0)
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

	/** B5 (header section 2.7, 4): whether Entry, a line of Called's BornTick tick, opens Called's chapter - a life called after a death. That line is its
	 * inauguration line ([B-5d1]): the lines of the tick before it are the span with no vessel (a leap above 0) or the end of the chapter of the life that
	 * died on it (a leap of 0), the lines from it on are Called's. The one rule of that cut: BuildCapped (the next chapter's Before and body; the death's
	 * chapter with a leap of 0; the last chapter's After when the life born on the end's tick is not shown - so the inauguration line never stands under
	 * Chronicle.NoVessel) and CosmicDigest::DigestOf (the end of its span) both read it, so the chronicle and the digest read the same lines. Until the
	 * inauguration line exists no line opens the chapter - false for every entry -: the whole BornTick tick is the span's with a leap above 0, and with a
	 * leap of 0 the chapter of the life that died on it keeps it. */
	COSMIC_ASCENSION_API bool OpensCalledChapter(const FCosmicLogEntry& Entry, const FLifeRecord& Called);

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
}

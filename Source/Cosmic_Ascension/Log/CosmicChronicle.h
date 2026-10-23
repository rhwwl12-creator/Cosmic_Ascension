// Cosmic Ascension - Log layer (Core Logic). Design: docs/70_CONTENT_GUIDE.md section 8.4, headers/A4_provisional_end.md (MVP item 4).
// RULE: Core Logic - no UI/World include. The chronicle's minimal form: pure functions of the saved state - nothing saved, hashed,
// drawn or written; Knowing is never read (16 #49).

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

/** One life's chapter ("the Nth", section 8.4). Built at display time, never saved. */
struct FCosmicChronicleChapter
{
	int32 LifeAt = INDEX_NONE;            // index into Calling.Lives ("the Nth" is Lives[LifeAt].LifeIndex)
	int64 FromTick = 0;                   // docs/21's BornTick/DiedTick range: from BornTick (the first life also every earlier tick) ...
	int64 ToTick = 0;                     // ... to DiedTick when set, else to the next life's BornTick - 1, else to State.Tick
	TArray<FCosmicChronicleLine> Lines;   // shown, in time order; copies, so the page outlives the next tick
	int32 Candidates = 0;                 // the lines left after the first repeat rule, before the cap (the commandlet's "of")
	bool bMore = false;                   // the cap left lines out: the chapter closes with ST_CA Chronicle.More (no count)
};

namespace CosmicChronicle
{
	/** Working cap of a life chapter's lines, set from the A-4a measurement. A display cap: nothing leaves the state. */
	inline constexpr int32 MaxLifeLines = 40;

	/** BirthCivID, then RuledCivIDs, without NAME_None and without repeats. */
	COSMIC_ASCENSION_API TArray<FName> PeoplesOf(const FLifeRecord& Life);

	/** 16 #31 (2) from the eight fields alone: Log.Calling, Log.Claim, Log.Record and every Severity 2; a Severity 1 entry when FocusCivID is one of Peoples or
	 * SourceRow is CosmicRows::LogFirstContact. Reads neither bUntold nor a table. MVP item 5's Chronicle filter (16 #40 (7)) calls it with every life's peoples. */
	COSMIC_ASCENSION_API bool Admits(const FCosmicLogEntry& Entry, TConstArrayView<FName> Peoples);

	/** One chapter per life (none for no life), over State.Chronicle then State.Log (already in time order), in four steps: gather what Admits holds for the
	 * life's peoples, but Log.Claim and Log.Record (the two books', B5); drop a repeat of the entry just before (Candidates counts the rest); over MaxLifeLines keep
	 * by rank - Log.Calling whole, own Severity 2, admitted Severity 1, other peoples' Severity 2 - thinning the first rank that does not fit (Severity 1: first
	 * ceil(k/2) and last floor(k/2); others: index (i * n) / k), the later ranks none, bMore set; in time order again, drop a repeat once more (not in bMore). */
	COSMIC_ASCENSION_API TArray<FCosmicChronicleChapter> Build(const FGalaxyState& State);

	/** The same over a cap given (tests and the measurement; Build is BuildCapped(State, MaxLifeLines)). A cap below 0 reads as 0. */
	COSMIC_ASCENSION_API TArray<FCosmicChronicleChapter> BuildCapped(const FGalaxyState& State, int32 Cap);

	/** {Player}: PersonNameBank[NameIndex % Num] of BirthArchetypeRow, else of arch_default (Install's rule); empty when neither resolves ("the Nth" alone). */
	COSMIC_ASCENSION_API FText VesselNameOf(const FLifeRecord& Life, const FCosmicDataTables& Tables);
}

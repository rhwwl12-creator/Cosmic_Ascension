// Cosmic Ascension - World layer (Presentation). Design: docs/60_DECISIONS/headers/B5_death_succession_two_books.md section
// 2.7 (decision 14), docs/21 section 7.5 'the digest', docs/70 section 8.4.
// The interregnum's digest: what changed in the world while no vessel stood, in a few lines - the middle page of the three
// pages of a death (epitaph, digest, inauguration). Computed when it is shown, from the state alone: no field is saved for
// it, nothing is written, no stream is drawn (the sentence variants come from CosmicSeed::Derive, Purpose::Digest), and
// the same state gives the same lines. Its inputs are the lines of the span (State.Chronicle, then State.Log) and the world
// as it is now; knowledge is read only through CosmicMapLook::KnownViewOf (invariant rule 14) - a known system is the view's
// IsKnown, a people is told only while the view shows it (ShownOrderOf), and the gap rule is not copied here. After a life
// the next one followed, the knowledge is the calling's from before that life (the view cut at its BornTick: what the
// succession's Install marked is out of knowledge), with the viewer tier of now - no tier of the past is kept. No
// first-person line (Log.Claim), no Log.Record line and no Log.Calling line is ever read: the digest is the world's news,
// told by the narrator (decision 14 - the first voice heard after a death is never a "mine").

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Internationalization/Text.h"
#include "UObject/NameTypes.h"

class UGalaxySimulationSubsystem;
struct FGalaxyState;
struct FLifeRecord;

/**
 * What one item of the digest says (header section 2.7 'items' - only what the vertical slice has the material for). Not
 * saved; the order is the rank of the cap (CosmicDigest::MaxItems), not a value anything stores.
 */
enum class ECosmicDigestItem : uint8
{
	// A people gone: a known system destroyed in the span (CosmicHazards::IsDeadNode now) whose people - the owner the last
	// hazard or project line of the span about it named - the state no longer holds. Told by the place: a gone people's name
	// no longer resolves.
	LostPeople,
	// A shown people that rose: a line of the span from a DT_Feats row about it (the ascension's line) - one item however
	// many rungs, so its words name no count. The screen never calls it a leap - the interregnum's word.
	Rose,
	// A known system destroyed in the span, its people still on the map.
	Lost,
	// A known living system that a claim of the span joined to a sphere: a line of a ClaimRule hazard row about it in the
	// span, and a transcendent's sphere holds it now. A narrator item - no first person, no claimant named.
	Annexed,
	// A known living system whose owner now - a people the view shows (CosmicMapLook::OwnerShownOf Shown) - is not the
	// owner the first hazard or project line of the span about it named, that one a people the view shows or a gone one. A
	// change with no such line before it is not seen (the siege lines are told to the ruled side only - none in the
	// interregnum).
	Owner,
	// A shown people whose seat changed hands in the span, told with the name of its open Notable.Ruler record. Two marks
	// only, each one that cannot stand without a change: a Notable.Ruler or Notable.Regent record closed in the span (a
	// rebellion that took the throne, a ruler named in its place), and a rebel that a rebellion made the ruler in the span -
	// a Notable.Ruler record of the rebellion's outbreak row whose expiry line falls in the span, read only when the log
	// still holds the outbreak's own told line (the first rebel to take a throne closes no record). A seat that changed
	// with neither mark left is not told; a resolved rebellion makes no ruler.
	Throne,
	// The same with no name to tell: no open Notable.Ruler record, or no name bank.
	ThroneUnnamed,
};

/** One item of the digest, rendered. */
struct COSMIC_ASCENSION_API FCosmicDigestItem
{
	ECosmicDigestItem Kind = ECosmicDigestItem::Lost;
	// The item's ID - the name of its variant's derive: the SystemID for LostPeople, Lost, Annexed and Owner; the CivID for
	// Rose, Throne and ThroneUnnamed.
	FName SubjectID;
	// The people the line names (Owner: the owner now; Rose and the throne items: the subject); None for LostPeople, Lost
	// and Annexed, which name no people.
	FName CivID;
	// The tick the item is told at - the index of its variant's derive and the order of the page: the last line of the span
	// about the system (the system items; Annexed: the last claim line), the last ascension line (Rose - one item however
	// many rungs: its words name no count), the last seat change (the throne items).
	int64 Tick = 0;
	// 0 .. CosmicDigest::VariantCount - 1: CosmicDigest::VariantOf(Seed, SubjectID, Tick).
	int32 Variant = 0;
	FText Line;
};

/** The digest of one span, as CosmicDigest::LinesOf renders it. Built when shown, never saved. */
struct COSMIC_ASCENSION_API FCosmicDigest
{
	// Whether there is a digest to show: a closed life with a span after it (or a leap of 0). False for an index that is no
	// closed life, and for the boundary of a death itself (the interregnum at its DiedTick: nothing has passed yet).
	bool bValid = false;
	// The span (AfterTick, ThroughTick]: the closed life's DiedTick, then the next life's BornTick - its tick read up to the
	// line that opens that life's chapter (CosmicChronicle::OpensCalledChapter, the chronicle's own cut: the inauguration
	// line, [B-5d1]; until it exists the whole tick) - or, while the interregnum lasts (the run ended in it), the state's
	// Tick.
	int64 AfterTick = 0;
	int64 ThroughTick = 0;
	// The next life stood up on the tick of the death (a leap of 0 - the cap of LeapTotalCap): one line, Digest.NoLeap.
	bool bNoLeap = false;
	// At most CosmicDigest::MaxItems, in tick order (then by rank, then systems before peoples in array order).
	TArray<FCosmicDigestItem> Items;
	// The cap left items out: the page closes the items with ST_CA Chronicle.More (no count).
	bool bMore = false;
	// Something changed out of the viewer's knowledge in the span (the closing line Digest.Elsewhere, docs/70 section 8.4):
	// a far line (log_far_event, log_far_ascent), a line whose subject the view does not show - a system it does not know
	// (one the succession's Install made known among them), a people in the state it does not show, a gone people told
	// about no known system -, a known system whose owner changed to or from a people it does not show (no Owner item), or
	// a seat changed in a people it does not show. The page shows it only after items (LinesOf).
	bool bElsewhere = false;
};

namespace CosmicDigest
{
	// The cap of the items (docs/21's budget of five to eight lines: the head, five items, the cap's line, the closing line).
	inline constexpr int32 MaxItems = 5;
	// The throne items, at most (docs/21 section 7.5): the latest seats left; competing with the others under MaxItems.
	inline constexpr int32 MaxThrones = 2;
	// The sentence variants of an item's key (ST_CA <key>.1 .. <key>.3).
	inline constexpr int32 VariantCount = 3;

	// The fixed lines (ST_CA): the head ({0} - the span's ticks, a UI number), the one line of a span with no item (true on
	// every path: the world went on - never that nothing happened, for the digest sees no change that left no line), the
	// closing line, the one line of a leap of 0, and the cap's line (the chronicle's own words - no count).
	inline constexpr const TCHAR* HeadKey = TEXT("Digest.Head");
	inline constexpr const TCHAR* QuietKey = TEXT("Digest.Quiet");
	inline constexpr const TCHAR* ElsewhereKey = TEXT("Digest.Elsewhere");
	inline constexpr const TCHAR* NoLeapKey = TEXT("Digest.NoLeap");
	inline constexpr const TCHAR* MoreKey = TEXT("Chronicle.More");

	/** The ST_CA key of Kind's variant (Digest.<Kind>.<Variant + 1>); a variant outside 0 .. VariantCount - 1 reads as the
	 * nearest. Slots: {System} (LostPeople, Lost, Annexed), {System} and {Civ} (Owner), {Civ} (Rose, ThroneUnnamed), {Civ}
	 * and {Ruler} (Throne). */
	COSMIC_ASCENSION_API const TCHAR* KeyOf(ECosmicDigestItem Kind, int32 Variant);

	/** The variant of an item: CosmicSeed::DeriveForName(Seed, SubjectID, Purpose::Digest, Tick as uint32) modulo
	 * VariantCount (header section 2.7 - neither the tell stream, a line's draw, nor the sim stream). */
	COSMIC_ASCENSION_API int32 VariantOf(int32 Seed, FName SubjectID, int64 Tick);

	/** The life that closed most recently, as an index into State.Calling.Lives: the last life while the calling is in the
	 * interregnum, the one before the last when a succession followed it (the last stands up embodied); INDEX_NONE when no
	 * life has closed (DiedTick 0) - a first life, a deposition or an abdication closes none. */
	COSMIC_ASCENSION_API int32 LastClosedLifeOf(const FGalaxyState& State);

	/** The digest of the interregnum after Calling.Lives[ClosedLifeIndex] closed (FCosmicDigest's span), over Sim's live
	 * state, its index, tables and viewer tier (UGalaxySimulationSubsystem::GetViewerTier - the tier of now); with a next
	 * life, the knowledge from before it (KnownViewOf cut at its BornTick) and its BornTick tick up to where its chapter
	 * opens; bSeeAll is the development aid CosmicSeeAll (the controller's IsSeeingAll), as KnownViewOf reads it. */
	COSMIC_ASCENSION_API FCosmicDigest DigestOf(const UGalaxySimulationSubsystem& Sim, int32 ClosedLifeIndex, bool bSeeAll = false);

	/** The same over a span given, (AfterTick, ThroughTick] - the reuse point of a later digest over another span (the
	 * return's, open question #152): with no next life, every line of ThroughTick and the knowledge as it is now. Not valid
	 * (nothing) for ThroughTick <= AfterTick. */
	COSMIC_ASCENSION_API FCosmicDigest DigestOver(const UGalaxySimulationSubsystem& Sim, int64 AfterTick, int64 ThroughTick, bool bSeeAll = false);

	/** The same with Next, the life called after the closed one (ThroughTick its BornTick), or null: Next's BornTick tick
	 * is read up to the line that opens its chapter (CosmicChronicle::OpensCalledChapter) and the knowledge is the view cut
	 * at its BornTick (KnownViewOf's cut - what its Install marked is out of knowledge). DigestOf calls it. */
	COSMIC_ASCENSION_API FCosmicDigest DigestOver(const UGalaxySimulationSubsystem& Sim, int64 AfterTick, int64 ThroughTick, bool bSeeAll, const FLifeRecord* Next);

	/** The page's lines: none for a digest that is not valid; Digest.NoLeap alone for a leap of 0; the head (Digest.Head
	 * with the span's ticks) and Digest.Quiet alone when there is no item; otherwise the head, the items' lines, then
	 * Chronicle.More when the cap left items out, then Digest.Elsewhere when something changed out of knowledge. */
	COSMIC_ASCENSION_API TArray<FText> LinesOf(const FCosmicDigest& Digest);

	/** The page's lines of the digest after Calling.Lives[ClosedLifeIndex] (DigestOf, then LinesOf): what the pages call. */
	COSMIC_ASCENSION_API TArray<FText> LinesOf(const UGalaxySimulationSubsystem& Sim, int32 ClosedLifeIndex, bool bSeeAll = false);
}

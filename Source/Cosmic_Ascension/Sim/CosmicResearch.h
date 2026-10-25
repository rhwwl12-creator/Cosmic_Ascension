// Cosmic Ascension - Sim layer (Core Logic). Design: docs/10_DESIGN/systems.md section 5.4, headers/A7_research_focus.md (MVP item 7).
// The single home of the research focus rule: how a people's ResearchFocus shares are written and what shape a map has. Pure
// functions of a map - no draw, no state of their own. The FocusResearch effect, the ruler's command SetResearchFocus, the
// panel, the top bar and the commandlet read them. The field list stays where the research stage is (ResearchFields).

#pragma once

#include "CoreMinimal.h"

/** What a ResearchFocus map is. Not a UENUM: never saved, hashed or a data column. */
enum class ECosmicFocusShape : uint8
{
	Even,      // two keys or more, every share equal (the generation's 1/4 each; the all-zero maps of old test boards too)
	Focused,   // one key strictly above every other key, and the others all equal to each other
	Mixed,     // anything else: fewer than two keys, or no such single key (the ruler's slider writes one - SharesOf, B2)
};

namespace CosmicResearch
{
	/** Field takes Share and every other key of the map the rest evenly, (1 - Share) / (keys - 1) - the expression and float
	 * order of the FocusResearch effect before item 7, so the petition's answer writes the same bits. False and nothing changed
	 * when Field is not a key, Share is not above 0 and at most 1 (NaN included), or the map has fewer than two keys. Each key
	 * is set from that key alone: no result depends on the map's iteration order (Core/GalaxyState.h, the TMap rule). */
	COSMIC_ASCENSION_API bool Focus(TMap<FName, float>& ResearchFocus, FName Field, float Share);

	/** Every key 1 / keys (the generation's shares: 1 / 4 is an exact float). False and nothing changed for an empty map. */
	COSMIC_ASCENSION_API bool Even(TMap<FName, float>& ResearchFocus);

	/** The shape of a map (see ECosmicFocusShape), with OutField set to the one key above the others when Focused and to
	 * NAME_None otherwise. Exact float comparisons: the writers above give the equal shares the very same float. Order-free: a
	 * strict unique maximum or none. */
	COSMIC_ASCENSION_API ECosmicFocusShape ShapeOf(const TMap<FName, float>& ResearchFocus, FName* OutField = nullptr);

	/** The transcendence field's name, "Transcendence" - the research stage's last field (its list stays there). The ruler's
	 * slider leaves this field alone until its own control comes (section 16 #119 (1)). */
	COSMIC_ASCENSION_API FName TranscendenceField();

	/** The largest weight of one field in SharesOf (one step is a share of about 0.05). */
	inline constexpr int32 MaxShareWeight = 20;

	/** The ruler's shares by hand (B2, section 16 #119 (3)): Weights must hold every key of the map except TranscendenceField,
	 * and no other key, each 0..MaxShareWeight with a sum above 0. The transcendence share T stays; every other key takes
	 * (1 - T) x weight / sum, the sum taken as an int32 (so no result depends on any order), a weight of 0 writing 0 and
	 * keeping its key. False and nothing changed when Weights does not fit. */
	COSMIC_ASCENSION_API bool SharesOf(TMap<FName, float>& ResearchFocus, const TMap<FName, int32>& Weights);
}

// Cosmic Ascension - Sim layer (Core Logic). Design: docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md section 2.6
// (decision 16); docs/21 section 7.4, the count of the fourth rung.
// The single home of the brake on the fourth rung (rule 13): whether a people the player does not rule may climb from the
// third rung to the fourth now. A pure function of the state and the tables - no draw, no field, no line, no knowing read
// (rule 2). The feat judgment asks it in one place (UAscensionSubsystem::JudgePeople, before it looks at the rows of a
// people the player does not rule standing on the third rung); the player's people and the climb to the fifth rung are never
// braked. The brake is a count against a cap, never a place a people takes: no text names it so.
// RULE (section 6.1): files under Sim/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"

struct FCosmicDataTables;
struct FGalaxyState;

namespace CosmicAscent
{
	/**
	 * Whether the fourth rung is open in the galaxy GalaxyRow (header section 2.6, docs/21 section 7.4 - the count is defined
	 * there once): the cap is the AIFourthRungCap of that DT_Galaxies row, and a row the tables do not hold or a cap of 0 or
	 * below is no brake (true). Otherwise T4Civs + Inbound < cap, where T4Civs counts the peoples of Civs whose GalaxyRow is
	 * GalaxyRow and whose Tier is the fourth rung or above - the people the player rules among them - and Inbound the records
	 * of Transits bound for GalaxyRow that have not landed (ArriveTick >= Tick: the judgment runs before the crossing stage;
	 * empty until Phase 6a, counted now so that 6a changes nothing here). The judgment sees each climb of the tick at once
	 * (UAscensionSubsystem::Ascend sets the Tier), so the people judged first passes first and a people that climbed earlier
	 * in the tick is counted. Reads only; draws nothing.
	 */
	COSMIC_ASCENSION_API bool FourthRungOpenOf(const FGalaxyState& State, const FCosmicDataTables& Tables, FName GalaxyRow);
}

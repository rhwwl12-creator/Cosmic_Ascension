// Cosmic Ascension - the names of rows the balance commandlet's bricks line reads by name (B7x [B-7x4m] - header section 5
// judgment 3: the Standing cards a waiting crossroads card may push off), for the measurement only. CLAUDE.md rule 8: a row
// the code names is named through a constant. They are not CosmicRows:: constants (Data/CosmicRowNames.h): the data checker
// holds every constant of that header to a required row the data holds (Tools/cosmic_data_schema.py, load_code_rows), so a
// constant there would say that the game calls its row - no game code does. The form is CosmicB6bRows'
// (Tests/CosmicB6bRowNames.h); B7x's own rows stay in CosmicTests::B7xRows (Tests/CosmicTestHelpers.h), which only the
// test build has.
// Not behind WITH_DEV_AUTOMATION_TESTS: the commandlet builds in every target.

#pragma once

#include "CoreMinimal.h"

namespace CosmicB7xRows
{
	// DT_Encounters: the two Standing petitions the bricks line counts (came, and lost their waiting slot).
	inline const FName PetitionWalls(TEXT("petition_walls"));
	inline const FName PetitionBread(TEXT("petition_bread"));
}

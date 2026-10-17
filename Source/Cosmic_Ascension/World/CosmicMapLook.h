// Cosmic Ascension - World layer (Presentation). Design: guideline.md section 6.4, 11 Phase 3.
// What the map draws of the state, computed from the state alone: no UObject, no draw, no write.

#pragma once

#include "CoreMinimal.h"
#include "Core/GalaxyState.h"
#include "Templates/Tuple.h"

struct FCosmicDataTables;

/** How one system is drawn. */
struct COSMIC_ASCENSION_API FCosmicSystemLook
{
	FText Name;
	// By star class (Star.Class.O .. M; white and 1 for an unknown class).
	FLinearColor StarColor = FLinearColor::White;
	float StarScale = 1.f;
	// The owner, settled or settling, and its colour (FCivilizationState::Color). No owner: false and black.
	bool bOwned = false;
	FLinearColor OwnerColor = FLinearColor::Black;
	// The construction slot holds project_settle.
	bool bSettling = false;
	// A hazard stands in the slot.
	bool bHazard = false;
	// Progress of the construction and the response slot as a share of RequiredProgressOf(row), 0..1; -1 for an empty
	// slot or a row the tables do not hold.
	float ConstructionShare = -1.f;
	float ResponseShare = -1.f;
	// Destroyed (CosmicHazards::IsDeadNode): drawn dim, no owner, no hazard, no bars.
	bool bDead = false;
};

namespace CosmicMapLook
{
	/** The look of State.Systems[SystemIndex]; the default look for an index out of range. */
	COSMIC_ASCENSION_API FCosmicSystemLook LookOf(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 SystemIndex);

	/** The lanes to draw, each once as (lower index, higher index): a lane listed by either end, both ends existing and not dead. */
	COSMIC_ASCENSION_API TArray<TPair<int32, int32>> LanesOf(const FGalaxyState& State);

	/** Whether a system's label shows: always when near; from afar only for a system with an owner or a hazard, or the
	 * selected one. */
	COSMIC_ASCENSION_API bool ShowsLabel(const FCosmicSystemLook& Look, bool bSelected, bool bFarView);
}

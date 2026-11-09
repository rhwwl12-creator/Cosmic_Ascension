// Cosmic Ascension - Core layer. Design: guideline.md section 3.2, 8.1; docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md
// section 2.9 (16 #107, decision 19). The gap rule, once: how many rungs one people stands above a viewer, and whether
// that is far enough to hide it. Pure functions of two tiers - no state, no knowing, no draw - so the scope of telling
// (UCosmicLogSubsystem::Tell), the meeting (UGalaxySimulationSubsystem::NoteMet), the presentation
// (CosmicMapLook::IsShownPeople), the crown gate (CosmicWanderer::CrownBlockOf - and the system panel's filter before it,
// UCosmicSystemPanelWidget, with the gate's viewer: the calling's PersonalTier), the nameless lines of a siege and a
// lapse, and a wanderer's card (CosmicPetitions::WithinWandererGap, its own threshold) give one answer.
// RULE (section 6.1): files under Core/ must never include Data, Sim, Events or presentation headers.

#pragma once

#include "CoreMinimal.h"
#include "Core/CosmicTypes.h"

namespace CosmicKnowing
{
	/** A people this many rungs or more above the viewer is far above it: seen, not met, its lines untold, not shown
	 * (section 3.2, 8.1). Measured upward only - a people below the viewer is never far. */
	inline constexpr int32 FarAboveRungs = 3;

	/** How many rungs Of stands above Viewer (negative when below): the tier numbers' difference, 1..5 as the gap rule
	 * counts them (EAscensionTier's values are the numbers less one, so the difference is the same). */
	inline constexpr int32 RungsAbove(EAscensionTier Of, EAscensionTier Viewer)
	{
		return static_cast<int32>(Of) - static_cast<int32>(Viewer);
	}

	/** Whether Of stands FarAboveRungs rungs or more above Viewer - the gap rule. The tiers are the world's; the viewer's
	 * is the caller's to choose (GetViewerTier for the telling and the meeting, the ruled people's for its nameless lines,
	 * the calling's PersonalTier for the crown gate). */
	inline constexpr bool IsFarAbove(EAscensionTier Of, EAscensionTier Viewer)
	{
		return RungsAbove(Of, Viewer) >= FarAboveRungs;
	}
}

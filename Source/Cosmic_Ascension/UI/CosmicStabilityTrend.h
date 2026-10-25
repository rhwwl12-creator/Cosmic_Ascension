// Cosmic Ascension - UI layer (Presentation). Design: docs/10_DESIGN/systems.md:189 (the HUD's stability trend),
// headers/A6_feedback_and_onboarding.md section 2 (MVP item 6).
// The stability trend of the top bar as pure functions: a screen value, never saved, hashed or drawn.

#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"

/** The direction the top bar shows next to the stability (not a UENUM - a screen value, never saved or hashed). */
enum class ECosmicTrend : uint8
{
	Unknown,   // fewer than CosmicUI::StabilityTrendTicks samples: no mark, the tooltip says when one comes
	Up,
	Down,
	Flat,
};

namespace CosmicUI
{
	// The window of the trend: docs/10_DESIGN/systems.md:189 - a screen rule, not a knob.
	inline constexpr int32 StabilityTrendTicks = 60;

	/** The shown whole percent of Now minus the shown whole percent of the mean of Samples (CosmicNextRung::WholePercent -
	 * the rounding the top bar's percent uses, so the mark moves with the number on the screen). Samples: the tick-boundary
	 * values, oldest first, consecutive ticks of one people; only the last StabilityTrendTicks count. Now: the value as it
	 * is drawn (an answer given while paused counts at once). 0 while there are fewer than StabilityTrendTicks samples. */
	COSMIC_ASCENSION_API int32 StabilityPointsOf(TConstArrayView<float> Samples, float Now);

	/** Unknown with fewer than StabilityTrendTicks samples; else Up at one point or more, Down at minus one or less, Flat
	 * between. */
	COSMIC_ASCENSION_API ECosmicTrend StabilityTrendOf(TConstArrayView<float> Samples, float Now);
}

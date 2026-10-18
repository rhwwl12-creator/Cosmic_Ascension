// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 4.5, 6.3, 6.4, 8.2.
// The four colours that carry meaning on the screens (section 8.2): Severity 0 grey, 1 white, 2 gold, and the one purple
// of the spiritual realm - the fallen's first person in the log feed and their marks in the third rung's spiritual sight.
// The feed and the map read them here, so there is one purple. The owners' colours (FCivilizationState::Color, item 8's
// palette) are not among them - in the sight no owner's ring takes the purple (CosmicMapLook::OwnerRingColorOf) - and the
// holy order has no colour (section 4.5).

#pragma once

#include "CoreMinimal.h"

namespace CosmicPalette
{
	inline constexpr FLinearColor Grey(0.6f, 0.6f, 0.65f);
	inline constexpr FLinearColor White(0.95f, 0.95f, 0.95f);
	inline constexpr FLinearColor Gold(1.f, 0.82f, 0.4f);
	inline constexpr FLinearColor Purple(0.78f, 0.6f, 1.f);
}

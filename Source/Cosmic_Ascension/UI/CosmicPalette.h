// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 4.5, 6.3, 6.4, 8.2.
// The four colours that carry meaning on the screens (section 8.2): Severity 0 grey, 1 white, 2 gold, and the one purple
// of the spiritual realm - the fallen's first person in the log feed and their marks in the third rung's spiritual sight.
// The feed and the map read them here, so there is one purple. The owners' colours follow them (Owners - the meeting
// order's, section 6.4); FCivilizationState::Color stays in the state and is drawn nowhere - and the holy order has no
// colour (section 4.5).

#pragma once

#include "CoreMinimal.h"

namespace CosmicPalette
{
	inline constexpr FLinearColor Grey(0.6f, 0.6f, 0.65f);
	inline constexpr FLinearColor White(0.95f, 0.95f, 0.95f);
	inline constexpr FLinearColor Gold(1.f, 0.82f, 0.4f);
	inline constexpr FLinearColor Purple(0.78f, 0.6f, 1.f);

	// The owners' colours on the map (section 6.4): one per people in the order the viewer met them (Knowing.MetCivs - the
	// first life's own people first, by Install; CosmicMapLook::KnownViewOf), from the thirteenth on from the first again
	// (EA looks at the wrap in Phase 6a). Colours of ownership: outside the four above and outside the spiritual view's
	// cap of four. None lies in the fallen's band of hues (CosmicMapLook::FallenHueFrom..FallenHueTo - so
	// CosmicMapLook::OwnerRingColorOf keeps every one in the sight). Measured in OKLab (UI.KnownFilter): every pair at least
	// 0.07 apart, every colour at least 0.07 from the four above, the lanes' blue, the warnings, the dead grey and black;
	// under deuteranopia and protanopia (Machado) the first life's own colour stays at least 0.1 from the next three and
	// every colour at least 0.04 from the purple. The order is the farthest-point order over the worst of normal,
	// deuteranopic and protanopic vision, from the first life's teal: the peoples met first are the neighbours. Starting
	// values; the screenshots and the developer's eye tune them (section 16). Colour alone cannot tell twelve peoples apart
	// for every eye: the panel's owner line names the people.
	inline constexpr FLinearColor Owners[] = {
		FLinearColor(0.10f, 0.80f, 0.70f), // teal - the first life's own people
		FLinearColor(0.00f, 0.25f, 1.00f), // cobalt
		FLinearColor(0.70f, 0.10f, 0.00f), // vermilion
		FLinearColor(0.75f, 0.95f, 0.15f), // lime
		FLinearColor(0.70f, 0.20f, 0.35f), // wine
		FLinearColor(0.60f, 0.55f, 0.10f), // olive
		FLinearColor(0.00f, 0.45f, 1.00f), // cerulean
		FLinearColor(0.55f, 1.00f, 0.75f), // mint
		FLinearColor(0.00f, 0.75f, 0.40f), // spring
		FLinearColor(0.35f, 0.85f, 0.20f), // green
		FLinearColor(0.20f, 0.60f, 0.20f), // forest
		FLinearColor(1.00f, 0.40f, 0.55f), // rose
	};
	inline constexpr int32 OwnerCount = UE_ARRAY_COUNT(Owners);
	static_assert(OwnerCount == 12, "Twelve owners' colours (section 6.4)");

	/** The colour of the people met at MetOrder (0 first, from the thirteenth the first again); black for a people not
	 * shown (below 0). */
	inline FLinearColor OwnerColorOf(int32 MetOrder)
	{
		return MetOrder >= 0 ? Owners[MetOrder % OwnerCount] : FLinearColor::Black;
	}
}

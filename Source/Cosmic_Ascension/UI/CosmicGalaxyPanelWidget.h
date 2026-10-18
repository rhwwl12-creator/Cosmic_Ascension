// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 3.4, 5.11, 6.3, 6.4, 8.3, 11 Phase 3.
// The UI reads the simulation's tables; it writes nothing.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CosmicGalaxyPanelWidget.generated.h"

class UTextBlock;
struct FGalaxyDef;

/**
 * The panel of a galaxy (section 6.4 'the display of knowing', 5.11): its name, the word of its kind (ST_CA by
 * CosmicUI::GalaxyKindKey), the real-figure texts of its DT_Galaxies row as written (StarCountText, DistanceText,
 * FeatureText - each carries its own hedge; an empty one is no line), and on every galaxy, the home galaxy too, the one
 * line ST_CA Galaxy.Named. A galaxy not met - every galaxy before Phase 6a - shows nothing more. No people line, no count
 * of living systems, peoples or known things, no allegiance or jurisdiction (section 8.3, 12.2). Under the lines, in grey,
 * one hint the HUD gives (ShowHint - CosmicUI::HintKeyOf of the camera's level and the session's progress: a star, the
 * wheel past the region, a galaxy, the way home; never saved). The HUD shows the galaxy the universe level selected, else
 * the home galaxy (CosmicUniverseLook::PanelOf), in the system panel's place. Built in C++ (no WBP).
 */
UCLASS(meta = (DisableNativeTick))
class COSMIC_ASCENSION_API UCosmicGalaxyPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Shows the DT_Galaxies row Row and redraws; None, and a row the tables do not hold, hide the panel. */
	void ShowGalaxy(FName Row);
	FName GetShownGalaxy() const { return ShownRow; }

	/** The lines under the name, in order: the kind's word (when CosmicUI::GalaxyKindKey knows the tag), the three texts
	 * that are not empty, then Galaxy.Named - on every galaxy. The panel shows them with an empty line before the last. */
	static TArray<FText> LinesOf(const FGalaxyDef& Galaxy);

	/** Shows the String Table entry Key as the grey hint line under the lines - {Home} filled with the home galaxy's
	 * DisplayName (CosmicUI::HintHomeKey) -, or collapses it for null. The HUD gives it CosmicUI::HintKeyOf. */
	void ShowHint(const TCHAR* Key);
	/** The key ShowHint was given last (null for none). */
	const TCHAR* GetHintKey() const { return HintKey; }
	/** Whether the hint line shows. */
	bool IsHintShown() const;

	/** The name, the block of lines and the hint (tests). */
	const UTextBlock* GetNameText() const { return NameText; }
	const UTextBlock* GetLinesText() const { return LinesText; }
	const UTextBlock* GetHintText() const { return HintText; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// A press on the panel stops here: it is not a click on the map under it (an unhandled press bubbles up to the viewport).
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	// The shown row drawn again (ShowGalaxy, and RebuildWidget for a row given before the tree was built).
	void Redraw();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LinesText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HintText;

	FName ShownRow;
	const TCHAR* HintKey = nullptr;
};

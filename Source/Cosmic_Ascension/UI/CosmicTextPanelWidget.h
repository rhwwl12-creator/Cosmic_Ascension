// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 6.1, 10.2, 11 Phase 1.
// The UI reads the simulation and listens to its delegates; it never changes the state.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/GalaxyState.h"
#include "CosmicTextPanelWidget.generated.h"

class UScrollBox;
class UTextBlock;

/**
 * The text panel of the Phase 1 prototype, cut down to the tail of the log at the bottom left (Phase 3 item 3 moved its
 * status lines to the top bar and the system panel; the log feed of item 4 replaces it). Built in C++ without a Widget
 * Blueprint. Labels come from the String Table ST_CA. Refreshed from the subsystem delegates, never from a tick.
 */
// DisableNativeTick: the meta of UUserWidget is not inherited, and Widget Blueprints made from this class should not tick.
UCLASS(meta = (DisableNativeTick))
class COSMIC_ASCENSION_API UCosmicTextPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Redraws the log from the current state.
	void Refresh();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// A press on the panel stops here: it is not a click on the map under it (an unhandled press bubbles up to the viewport).
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	UFUNCTION()
	void HandleGalaxyGenerated(int32 Seed);

	UFUNCTION()
	void HandleLogEntry(const FCosmicLogEntry& Entry);

	void RefreshLog();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LogText;

	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> LogScroll;
};

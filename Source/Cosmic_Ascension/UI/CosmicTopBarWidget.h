// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 2.1, 6.3, 8.3, 11 Phase 3.
// The UI reads the simulation and listens to its delegates; commands go through the player controller.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Misc/Optional.h"
#include "UI/CosmicUILabels.h"
#include "CosmicTopBarWidget.generated.h"

class UButton;
class UCosmicActionButton;
class UTextBlock;

/**
 * The band along the top of the screen: the people the player rules - its name and rung (RungText, a flat button that
 * lights while the mouse is on it and a next rung is told), its stability and five resources, and its four research
 * shares (ResearchText - the block that yields when the band runs short: clipped, with an ellipsis); the rung and the
 * research shares carry the 'next rung' tooltip (CosmicNextRung) -, the clock, and one speed button per
 * CosmicUI::SpeedSteps (pause, x1, x2, x5, x10 and, once the third rung's gate is open and the ascension presentation is
 * not showing, x25 - the current one greyed out, a step the gate does not open collapsed; x25's tooltip is set once). The
 * clock is a flat button: a press switches the clock's unit between cycles and years (the controller's ToggleTimeUnit),
 * the mouse on it shows both units, and the speed buttons follow the unit ("2 years a second"). From the third rung (the
 * controller's IsSpiritualSightOpen) a bordered view button stands between the clock and the speeds - its label names
 * the view drawn (ST_CA Sight.Spiritual or Sight.Physical), a press switches it (the controller's ToggleViewMode), its
 * tooltip says what the sight is (Sight.Tip); below the third rung it is collapsed, so its appearing is the opening. A
 * wanderer has no people: the band shows only the clock, the view and the speeds. Built in C++ (no WBP); redrawn from the
 * simulation's delegates, after a speed button and after the clock's and the view's press.
 */
UCLASS(meta = (DisableNativeTick))
class COSMIC_ASCENSION_API UCosmicTopBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Refresh();

	/** The speed buttons in the order of CosmicUI::SpeedSteps, pause first (a step the gate does not open is collapsed). */
	const TArray<TObjectPtr<UCosmicActionButton>>& GetSpeedButtons() const { return SpeedButtons; }

	/** The speed button that sets Value; null when there is none (tests find buttons by value, not by place). */
	UCosmicActionButton* FindSpeedButton(float Value) const;

	/** What a click on the clock does (tests call it in place of a click). */
	void PressClock();
	/** The mouse on the clock (true) or off it (tests call it in place of the mouse). */
	void SetClockHovered(bool bHovered);
	/** The clock's text block (tests). */
	const UTextBlock* GetClockText() const { return ClockText; }
	/** The clock's button (tests read its tooltip). */
	const UButton* GetClockButton() const { return ClockButton; }

	/** What a click on the view button does (tests call it in place of a click). */
	void PressViewButton();
	/** The view button (tests read its label, visibility and tooltip). */
	const UCosmicActionButton* GetViewButton() const { return ViewButton; }

	/** The people's name and rung, which carry the next-rung tooltip (tests read its text, colour and Slate tooltip). */
	const UTextBlock* GetRungText() const { return RungText; }
	/** The four research shares, which carry the same tooltip (tests). */
	const UTextBlock* GetResearchText() const { return ResearchText; }
	/** The mouse on the rung (true) or off it (tests call it in place of the mouse): the rung lights as the clock does, while
	 * GetNextRungTip has words. */
	void SetRungHovered(bool bHovered);

	/**
	 * What the next-rung tooltip says now: CosmicNextRung::TextOf over the live state; empty - no tooltip - without a next
	 * rung. Bound once to the ToolTipTextDelegate of RungText and ResearchText before their Slate widgets are built, so one
	 * Slate tooltip each reads it while the mouse is on it; SetToolTipText is never called on them (it would build a new
	 * tooltip at every change and close an open one).
	 */
	UFUNCTION()
	FText GetNextRungTip() const;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// A press on the panel stops here: it is not a click on the map under it (an unhandled press bubbles up to the viewport).
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	UFUNCTION()
	void HandleSimTick(int64 Tick);

	UFUNCTION()
	void HandleGalaxyGenerated(int32 Seed);

	void HandleSpeed(FName Action, float Value);

	void HandleView(FName Action, float Value);

	UFUNCTION()
	void HandleClockClicked();

	UFUNCTION()
	void HandleClockHovered();

	UFUNCTION()
	void HandleClockUnhovered();

	UFUNCTION()
	void HandleRungHovered();

	UFUNCTION()
	void HandleRungUnhovered();

	// "<people>   rung N": the head of the status, in a flat button of its own.
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RungText;

	// Stability and the five resources.
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	// The four research shares: the band's fill, clipped with an ellipsis when the band runs short.
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ResearchText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> RungButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ClockButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ClockText;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCosmicActionButton>> SpeedButtons;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicActionButton> ViewButton;

	bool bClockHovered = false;
	// The unit the speed buttons were last labelled with (relabelled only when it changes).
	TOptional<ECosmicTimeUnit> LabelledUnit;
};

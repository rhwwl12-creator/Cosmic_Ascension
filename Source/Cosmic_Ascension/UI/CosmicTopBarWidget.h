// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 2.1, 6.3, 8.3, 11 Phase 3.
// The UI reads the simulation and listens to its delegates; commands go through the player controller.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Misc/Optional.h"
#include "UI/CosmicStabilityTrend.h"
#include "UI/CosmicUILabels.h"
#include "CosmicTopBarWidget.generated.h"

class UButton;
class UCosmicActionButton;
class UHorizontalBox;
class UTextBlock;
struct FCosmicDataTables;
struct FCosmicRulesDef;
struct FCosmicStateIndex;
struct FGalaxyState;

/**
 * The band along the top of the screen: the people the player rules - its name and rung (RungText, a flat button that
 * lights while the mouse is on it and a next rung is told), its stability (a block of its own with the trend's mark and
 * tooltip - MVP item 6), its five resources, and its four research
 * shares (ResearchText - the block that yields when the band runs short: clipped, with an ellipsis); the rung and the
 * research shares carry the 'next rung' tooltip (CosmicNextRung) -, the clock, and one speed button per
 * CosmicUI::SpeedSteps (pause, x1, x2, x5, x10 and, once the third rung's gate is open and the ascension presentation is
 * not showing, x25 - the current one greyed out, a step the gate does not open collapsed; x25's tooltip is set once). The
 * clock is a flat button: a press switches the clock's unit between cycles and years (the controller's ToggleTimeUnit),
 * the mouse on it shows both units, and the speed buttons follow the unit ("2 years a second"). From the third rung (the
 * controller's IsSpiritualSightOpen) a bordered view button stands between the clock and the speeds - its label names
 * the view drawn (ST_CA Sight.Spiritual or Sight.Physical), a press switches it (the controller's ToggleViewMode), its
 * tooltip says what the sight is (Sight.Tip); below the third rung it is collapsed, so its appearing is the opening. After
 * the speeds the automatic pause's button - its label says whether it is on (ST_CA AutoPause.On / AutoPause.Off), a press
 * switches it (the controller's ToggleAutoPause), its tooltip says what stops the time (AutoPause.Tip) - and, while an
 * automatic pause holds the simulation still, one line that says why (CosmicAutoPause::WhyKeyOf of the controller's
 * reason); it goes when the speed is set again. A wanderer has no people: after the resources' empty place the band shows its
 * line (WandererLineOf - where it stands or where it goes and how long, and when no lane leads anywhere) with the vessel's
 * stats and the shown people that looks up to it most in its tooltip (GetWandererTip, B1) and, while a hazard that kills a
 * vessel at its expiry stands where it counts as standing, the warning after it (LethalLineOf, B5), then the clock, the view,
 * the speeds and the automatic pause. In the interregnum (B5) the band shows no vessel's line: no people, no wanderer's line;
 * nor does it show the wanderer's line and its warning while the controller does not show the vessel (B5 [B-5c] -
 * ACosmicPlayerController::IsVesselShownFor: a calling page open, the vessel an ended run's last tick installed). After an embodied ruler's resources, before the research block (which yields first), the
 * edict button (B2 - GetEdictButton) and, while the edict hint stands, its grey mark. Built in C++ (no WBP); redrawn from
 * the simulation's delegates, after a speed button and after the clock's, the view's and the automatic pause's press.
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
	/** The clock's tooltip: what the clock is, what a press does and, with the provisional end on (MVP item 4) and a clock end
	 * (EndTick above 0), the run's rule - Chronicle.Rule with EndTick in cycles. No rule line without a clock end: the clock is
	 * the one end play reaches in B6a (decision 13; Chronicle.RuleRung is not read). Built once (tests). */
	static FText ClockTipOf(int32 EndTick, bool bEndOn);

	/** What a click on the view button does (tests call it in place of a click). */
	void PressViewButton();

	/** What a click on the automatic pause's button does (tests call it in place of a click). */
	void PressAutoPause();
	/** The automatic pause's button (tests read its label and tooltip). */
	const UCosmicActionButton* GetAutoPauseButton() const { return AutoPauseButton; }
	/** The line that says why an automatic pause holds the time (tests read its text and visibility). */
	const UTextBlock* GetAutoPauseText() const { return AutoPauseText; }
	/** The view button (tests read its label, visibility and tooltip). */
	const UCosmicActionButton* GetViewButton() const { return ViewButton; }
	/** The provisional end (MVP item 4): the grey line beside the clock - Chronicle.Soon over the last CosmicChronicleScreen::EndSoonTicks before the clock end,
	 * else Chronicle.Until while the vessel is an embodied wanderer (B5: every one, a called vessel that never ruled too - none in the interregnum and none
	 * for a ruler, who is told the clock end by the clock tip's rule line - B6a header section 4); collapsed after the end, with the end switched off or
	 * without a clock end (tests read it). */
	const UTextBlock* GetEndSoonText() const { return EndSoonText; }
	/** The 'chronicle' button after the speeds: shown only while the run is over and its page is folded; a press reopens it (tests press it). */
	UCosmicActionButton* GetReopenButton() const { return ReopenButton; }

	/** The people's name and rung, which carry the next-rung tooltip (tests read its text, colour and Slate tooltip). */
	const UTextBlock* GetRungText() const { return RungText; }
	/** The four research shares, which carry the research tooltip (GetResearchTip - tests). */
	const UTextBlock* GetResearchText() const { return ResearchText; }
	/** The mouse on the rung (true) or off it (tests call it in place of the mouse): the rung lights as the clock does, while
	 * GetNextRungTip has words. */
	void SetRungHovered(bool bHovered);

	/**
	 * What the next-rung tooltip says now: CosmicNextRung::TextOfAll of AllOf over the live state (every way up); empty -
	 * no tooltip - without a next rung. Bound once to the ToolTipTextDelegate of RungText before its Slate widget is built
	 * (GetResearchTip, ResearchText's, reads it after the focus line), so the Slate tooltip reads it only while the mouse is
	 * on it; SetToolTipText is never called on them (it would build a new
	 * tooltip at every change and close an open one).
	 */
	UFUNCTION()
	FText GetNextRungTip() const;

	/**
	 * What the research block's tooltip says now (MVP item 7): ST_CA Focus.BarLine - the ruled people's focus in a word and
	 * its four shares (CosmicUI::FocusWordOf, FocusSharesOf) - then, after an empty line, GetNextRungTip when it has words;
	 * GetNextRungTip alone without a ruled people. Bound once to the ToolTipTextDelegate of ResearchText before its Slate
	 * widget is built (as GetNextRungTip on RungText).
	 */
	UFUNCTION()
	FText GetResearchTip() const;

	/**
	 * What the stability tooltip says now (MVP item 6): ST_CA Panel.StabilityUp / Down / Flat with the window and the
	 * difference in whole points, or Panel.StabilityWait with the time still missing; empty without a ruled people. Bound
	 * once to the ToolTipTextDelegate of StabilityText and TrendText before their Slate widgets are built (as GetNextRungTip).
	 */
	UFUNCTION()
	FText GetStabilityTip() const;

	/**
	 * The wanderer's line (B1 header section 4): ST_CA Wander.Bar.Standing with the system the vessel stands in, or
	 * Wander.Bar.Travelling with the system it goes to and the time left in Unit (CosmicUI::TimeLeftLabel); standing where no lane leads to a system
	 * TravelTo would take (every neighbour destroyed), Wander.Bar.Stuck after it. Empty but for an embodied wanderer.
	 */
	static FText WandererLineOf(const FGalaxyState& State, const FCosmicStateIndex& Index, ECosmicTimeUnit Unit);

	/**
	 * The lethal hazard's warning (B5 header section 2.8, 4): where an embodied wanderer counts as standing a hazard stands
	 * whose expiry kills a vessel there and would reach this one then (CosmicAutoPause::LethalPlaceOf - the birth protection
	 * among the rule) - ST_CA Wander.Bar.Lethal with the hazard's name, or under way Wander.Bar.LethalTravelling (the vessel
	 * stands where it left until it arrives, and the system it left counts only while its hazard expires before the arrival -
	 * [B-5r]); else, under way ([B-5r], the [B-5d2] review's play-1), the hazard of the system it goes to that will still
	 * stand when it arrives (CosmicAutoPause::LethalStayOf at the arrival's tick): Wander.Bar.Ahead, or Wander.Bar.AheadTrap
	 * when leaving it again would not be gone before the expiry. Words that promise nothing. Empty otherwise - a ruler, the
	 * interregnum, no such hazard, a shielded vessel. Shown after the wanderer's line (GetLethalText) while the vessel is shown.
	 */
	static FText LethalLineOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules);

	/**
	 * What the wanderer line's tooltip says now: the vessel's four stats (the card's stat words), then the people the
	 * viewer is shown that looks up to the vessel most (Wander.Bar.TipReverence - none while none does); empty but for an
	 * embodied wanderer. Bound once to the ToolTipTextDelegate of the wanderer's line (as GetNextRungTip): a ruler's resources
	 * keep no tooltip.
	 */
	UFUNCTION()
	FText GetWandererTip() const;
	/** The trend as the bar shows it now (CosmicUI::StabilityTrendOf over the samples and the live stability): Unknown
	 * without a ruled people, before StabilityTrendTicks samples of it, or when the samples are not of this tick (tests). */
	ECosmicTrend GetStabilityTrend() const;
	/** The stability block ("stability 72%"), its trend cell (the mark or nothing) and the resources (tests). */
	const UTextBlock* GetStabilityText() const { return StabilityText; }
	const UTextBlock* GetTrendText() const { return TrendText; }
	const UTextBlock* GetStatusText() const { return StatusText; }
	/** The wanderer's line (B1, tests): WandererLineOf, collapsed while it has no words or the vessel is not shown (B5). */
	const UTextBlock* GetWandererText() const { return WandererText; }
	/** The lethal hazard's warning after it (B5, tests): LethalLineOf, collapsed while it has no words or the vessel is not shown. */
	const UTextBlock* GetLethalText() const { return LethalText; }

	/** The edict button (B2 header section 4): Edict.Count with the held edicts and the slots (CosmicEdicts::HeldCountOf,
	 * SlotsOf), for an embodied ruler in a run that is not over - collapsed otherwise; a press opens or closes the edict
	 * window (ACosmicPlayerController::ToggleEdictWindow). Its tooltip is GetEdictTip, set when its words change (tests
	 * press it). */
	UCosmicActionButton* GetEdictButton() const { return EdictButton; }
	/** The grey mark after the edict button: HitTestInvisible while the edict hint stands (CosmicUI::ShowsEdictHint),
	 * Hidden otherwise - it keeps its width, so the research block never moves (tests). */
	const UTextBlock* GetEdictMark() const { return EdictMark; }
	/** What the edict button's tooltip says now: the edict hint first while it stands (Hint.Edict), the held edicts'
	 * upkeep (Edict.Bar.Upkeep), the next slot's Society mark (Edict.Bar.NextSlot, or Edict.Bar.AllSlots), that the
	 * window holds the peoples too (B3, Edict.Bar.Peoples) and what a press does (Edict.Bar.Press); empty without an
	 * embodied ruler. */
	FText GetEdictTip() const;

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

	void HandleAutoPause(FName Action, float Value);

	void HandleReopen(FName Action, float Value);

	void HandleEdicts(FName Action, float Value);

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

	// The stability and its trend cell (MVP item 6), then the five resources.
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StabilityText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TrendText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> WandererText;

	// The lethal hazard's warning (B5): after the wanderer's line, collapsed while it has no words.
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LethalText;

	// The edict button and its mark, in a block of their own (B2): collapsed together.
	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> EdictBlock;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicActionButton> EdictButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EdictMark;

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

	UPROPERTY(Transient)
	TObjectPtr<UCosmicActionButton> AutoPauseButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AutoPauseText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EndSoonText;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicActionButton> ReopenButton;

	// The ruled people's stability at the last tick boundaries (MVP item 6): oldest first, at most
	// CosmicUI::StabilityTrendTicks, consecutive ticks of one people - cleared on a new galaxy or a load, on another ruled
	// people, without one, and when a tick was skipped. Presentation state: never saved; a new widget starts empty.
	TArray<float> StabilitySamples;
	int64 LastSampleTick = -1;
	FName SampledCivID;
	void SampleStability();
	void ClearStabilitySamples();
	// The trend now and its difference in points (0 when Unknown); the samples count only when they are of the ruled
	// people at this tick.
	ECosmicTrend TrendNow(int32& OutPoints, int32& OutMissing) const;

	bool bClockHovered = false;
	// The unit the speed buttons were last labelled with (relabelled only when it changes).
	TOptional<ECosmicTimeUnit> LabelledUnit;
};

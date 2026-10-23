// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 5.7, 5.12, 6.3, 8.3, 11 Phase 3.
// The UI reads the simulation and listens to its delegates; commands go through the player controller.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Sim/CosmicMegaprojects.h"
#include "Templates/Tuple.h"
#include "UI/CosmicUILabels.h"
#include "CosmicSystemPanelWidget.generated.h"

class UCosmicActionButton;
class UGalaxySimulationSubsystem;
class UTextBlock;
class UVerticalBox;
struct FCosmicDataTables;
struct FCosmicKnownView;
struct FCosmicSpiritLook;
struct FGalaxyState;

/** What the system panel remembers of one project slot, to tell a slot that does not pay (guideline 6.3, Panel.Stalled). */
struct COSMIC_ASCENSION_API FCosmicSlotWatch
{
	// The slot as last seen, and the tick it was seen at (-1: not yet).
	CosmicMegaprojects::FCosmicSlotReading Seen;
	int64 Tick = -1;

	// How many observed ticks in a row the slot paid nothing, and how many it paid.
	int32 UnpaidRun = 0;
	int32 PaidRun = 0;

	// Whether its slot line shows the paused mark.
	bool bStalled = false;
};

namespace CosmicSlotWatch
{
	// Ticks in a row that turn the mark on, and off: a slot that pays every other cycle (the first life's Dyson swarm on an
	// income of 1 Energy against its 2) never shows it, and one that pays less than half its cycles shows it steadily.
	inline constexpr int32 StallAfter = 2;
	inline constexpr int32 ResumeAfter = 2;

	/**
	 * The watch after seeing Now at NowTick. At Watch.Tick + 1 and the same row: a slot that paid nothing
	 * (CosmicMegaprojects::PaidNothing) adds to the unpaid run, one whose progress grew adds to the paid run, and the mark
	 * comes on at StallAfter unpaid ticks in a row and goes at ResumeAfter paid ticks in a row. At Watch.Tick: the same
	 * reading changes nothing (a redraw between ticks), another one starts the watch again (a command). Anything else - an
	 * empty slot, another row, a lower progress, a tick that is not the next (a gap, a load) - starts the watch again from
	 * Now, without the mark. Pure.
	 */
	COSMIC_ASCENSION_API FCosmicSlotWatch Step(const FCosmicSlotWatch& Watch, const CosmicMegaprojects::FCosmicSlotReading& Now, int64 NowTick);
}

/**
 * The panel of the selected system, on the right of the screen (hidden with no selection and for a system the viewer
 * is not shown - FCosmicKnownView::IsShown: known, or the sanctuary by its name; the HUD shows the galaxy panel in its
 * place while nothing is selected): its name, star class, the
 * owner as OwnerLineOf says (a people the viewer is shown by name, any other as another people, or none, or destroyed)
 * and right after a shown owner that people's band (ST_CA Panel.People - section 8.3, never a number; none on a wild or
 * destroyed system or for an owner the viewer is not shown), the life of its world (ST_CA Panel.Life with one of twelve
 * words, CosmicUI::LifeLine - none on a destroyed system and none on the sanctuary, whose owner line says it is kept),
 * the hazard standing there with the time it has left (in the clock's unit - cycles
 * or years, the controller's choice; the HUD redraws the panel when it changes), the construction and response slots
 * with their progress against what they need (not for an owner the viewer is not shown: they are its works) - on a slot
 * of the people the player rules followed by the paused mark
 * (ST_CA Panel.Stalled) while its watch says it does not pay (FCosmicSlotWatch; another people's slot says nothing -
 * that would tell its stockpile -, nor does any slot while the vessel is not an embodied ruler); in the spiritual view (the controller's view drawn) a line of its own under the
 * details with the words of the sight's mark, which it also carries as its tooltip (SpiritTipOf - the fallen one's title,
 * no number; a block apart, so that the words never cover the people's band) - then, for the people the player rules,
 * the commands. On its settled system one button per project row that belongs there, in lexical order: enabled when
 * CosmicMegaprojects::StartBlockOf says None, greyed out with a short reason after its name and the reason first in its
 * tooltip when the row's rung, this answer running already, another answer in the response slot or this construction
 * running already stands in the way (StartReasonOf); a row that does not belong there (an answer to no hazard standing
 * there, project_settle, ...) has no button. On every living known system that is not the people's a settle button -
 * greyed out, with the reason first in its tooltip and as a line under it, when CosmicMegaprojects::SettleBlockOf
 * refuses (where another people lives, Settle.Block.Owned names no people, whether the viewer is shown it or not; on the
 * sanctuary, Settle.Block.Sanctuary).
 * Every command button's tooltip carries its cost after the reason (StartHintOf - none where another people lives, on the
 * sanctuary or where no settle row exists): the cost refuses nothing (section 5.7). No commands while the vessel is not an embodied ruler. The
 * buttons are built again only when the list of their actions changes; otherwise each keeps its object and takes its
 * label, state and tooltip in place - a click spans a press and a release, and a button built again between them drops
 * it. Built in C++ (no WBP).
 */
UCLASS(meta = (DisableNativeTick))
class COSMIC_ASCENSION_API UCosmicSystemPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Shows SystemID (None hides the panel) and redraws. */
	void ShowSystem(FName SystemID);
	FName GetShownSystem() const { return ShownSystemID; }
	void Refresh();

	/**
	 * The owner line of State.Systems[SystemIndex] (section 6.3, 6.4), as CosmicMapLook::OwnerShownOf says: Panel.Owner
	 * and the CivName of a shown owner; Panel.Owner and Panel.OtherOwner for a hidden one - a people lives there (the
	 * settle button's Settle.Block.Owned says so too) but no name; Panel.NoOwner for none; Panel.Destroyed for a destroyed
	 * system; empty for an index out of range and for a system the view does not show. The sanctuary's line first:
	 * Panel.Sanctuary, whatever else holds (section 4.5 (1), 6.3).
	 */
	static FText OwnerLineOf(const FGalaxyState& State, const FCosmicKnownView& View, int32 SystemIndex);

	/** The command buttons now shown (tests). */
	const TArray<TObjectPtr<UCosmicActionButton>>& GetActionButtons() const { return ActionButtons; }
	/** The reason the shown settle button is greyed out; None when it is enabled or there is no settle button. */
	CosmicMegaprojects::ECosmicSettleBlock GetShownSettleBlock() const { return ShownSettleBlock; }
	/** Whether the reason line under the settle button shows. */
	bool IsReasonShown() const;
	/**
	 * The words of a reason a settlement is refused (ST_CA Settle.Block.*, the rung read from the settle row and the
	 * sphere rule; Settle.Block.Sanctuary for the sanctuary); empty for None, NoPeople and Dead, which no settle button shows.
	 */
	static FText ReasonOf(CosmicMegaprojects::ECosmicSettleBlock Block, const FCosmicDataTables& Tables);

	/** The reason the panel shows on RowName's project button: None for an enabled button, Count when no project button of
	 * RowName shows (tests). */
	CosmicMegaprojects::ECosmicStartBlock GetShownStartBlock(FName RowName) const;

	/** Whether the slot line of the slot Which on SystemID shows the paused mark (tests). */
	bool IsSlotStalled(FName SystemID, CosmicMegaprojects::ECosmicProjectSlot Which) const;

	/**
	 * The words of a reason a project button is greyed out (ST_CA Start.Block.*; for RowTier the rung read from RowName's
	 * row, numbered 1 to 5 as the top bar numbers rungs); empty for None and for the reasons that show no button - NoRow,
	 * SettleRow, NoSystem, Dead, NotSettled, NoOwner, NoHazard and OtherHazard.
	 */
	static FText StartReasonOf(CosmicMegaprojects::ECosmicStartBlock Block, const FCosmicDataTables& Tables, FName RowName);

	/**
	 * The lines of a command button's tooltip after its reason, for RowName started on System: first - for a construction
	 * row other than project_settle while System's construction slot holds another row - Start.Replaces with that row's
	 * name (the one loss a start cannot undo), then the row's cost a cycle, or a year in the year unit
	 * (CosmicMegaprojects::CostPerTickOf, the members above 0 - a whole amount as a whole number, any other with up to two
	 * decimals - joined by CosmicUI::JoinList; Start.CostPerCycle, Start.CostPerYear, Start.Free for a row that costs
	 * nothing), then - for a construction row that costs something (project_settle included) - Start.AnswersFirst. Built
	 * from the row, the slot and the unit alone, never from the stockpile: the words stay while it moves. Empty for a row
	 * the tables do not hold.
	 */
	static FText StartHintOf(const FCosmicDataTables& Tables, FName RowName, const FStarSystemState& System, ECosmicTimeUnit Unit);

	/**
	 * The words of the spiritual sight for a system (section 4.2, 6.4): the Title of Spirit.MarkRow - a title, never a
	 * name, no number - in the words of its mark (ST_CA Sight.Claimed, Sight.Held, Sight.Pressed for the fallen one that
	 * presses, Sight.Bound for the sealed holder alone; the particle by |hpp); for the sanctuary (Spirit.bSanctuary) the
	 * words Sight.Sanctuary, that no pressure reaches it. Empty when nothing marks the system, when the tables do not hold
	 * the row and when its Title is empty (a slip the schema refuses for a fallen row).
	 */
	static FText SpiritTipOf(const FCosmicSpiritLook& Spirit, const FCosmicDataTables& Tables);
	/** The block of detail lines (tests read its text). */
	const UTextBlock* GetDetailText() const { return DetailText; }
	/** The line of the sight's words under the detail lines, with the same words as its tooltip (tests): shown in the
	 * spiritual view when SpiritTipOf has words, collapsed and empty otherwise. */
	const UTextBlock* GetMarkText() const { return MarkText; }
	/** The grey hint line at the bottom (MVP item 6, ST_CA Hint.Commands): shown while CosmicUI::ShowsCommandHint holds for
	 * the shown system - an enabled project of the ruler on it, the run not over, no project started this session (tests). */
	const UTextBlock* GetHintText() const { return HintText; }

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

	void HandleAction(FName Action, float Value);

	// Steps the watches of both slots of every system the people the player rules owns (settled or being settled), by
	// SystemID, and drops the others - only outside a Core Logic broadcast (UGalaxySimulationSubsystem::IsBroadcasting:
	// mid-tick a slot may not have been paid yet). Called first in every refresh, whatever the shown system.
	void WatchSlots(const UGalaxySimulationSubsystem& Sim);

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DetailText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> MarkText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ReasonText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HintText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> ButtonBox;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCosmicActionButton>> ActionButtons;

	FName ShownSystemID;
	CosmicMegaprojects::ECosmicSettleBlock ShownSettleBlock = CosmicMegaprojects::ECosmicSettleBlock::None;

	// By SystemID; emptied on a new galaxy or a load (HandleGalaxyGenerated). Presentation memory: never in FGalaxyState.
	TMap<FName, FCosmicSlotWatch> ConstructionWatches;
	TMap<FName, FCosmicSlotWatch> ResponseWatches;

	// The reasons of the project buttons now shown, in button order (the settle button has none here).
	TArray<TPair<FName, CosmicMegaprojects::ECosmicStartBlock>> ShownStartBlocks;
};

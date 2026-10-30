// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 5.7, 5.12, 6.3, 8.3, 11 Phase 3.
// The UI reads the simulation and listens to its delegates; commands go through the player controller.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TimerHandle.h"
#include "Misc/Optional.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicWanderer.h"
#include "Templates/Tuple.h"
#include "UI/CosmicUILabels.h"
#include "CosmicSystemPanelWidget.generated.h"

class UCosmicActionButton;
class UGalaxySimulationSubsystem;
class UTextBlock;
class UVerticalBox;
struct FCivilizationState;
struct FCosmicDataTables;
struct FCosmicKnownView;
struct FCosmicRulesDef;
struct FCosmicSpiritLook;
struct FGalaxyState;
struct FHazardDef;

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
 * Last, under the hint line, the research focus (MVP item 7), on the same systems as the project buttons: a heading that
 * names the people's focus now (ST_CA Focus.Heading with CosmicUI::FocusWordOf, its tooltip Focus.HeadingTip) and one row of
 * four buttons, [Even] [Physics] [Society] [Engineering] - the one that is the focus now greyed out (its tooltip says so
 * with the shares), the others with the shares a press would give (Focus.TipField, Focus.TipEven) and, on a field the
 * next rung reads (CosmicNextRung::ResearchFieldsOf), Focus.TipNeeded. Built once, kept across refreshes; a press goes
 * through ACosmicPlayerController::SetResearchFocus. In an ended run all four are greyed with Chronicle.Block.
 * The wanderer's row (B1 header section 4), in a box of its own under the project buttons and the fleet's block
 * (GetActionButtons stays the projects'): on the system an embodied wanderer stands in, [Scan] and - on a settled system of a people - [Crown], or
 * [Return] for the people it left; on each living lane neighbour of that system, [Travel] with the lane's time. Each is
 * greyed by the first reason of the command's own gate (CosmicWanderer::TravelBlockOf, CrownBlockOf - fed the meeting and
 * the waiting card from the view), the short words after its name and the reason first in its tooltip (a refused crown
 * of a shown people also says its reverence and the two thresholds). For a wanderer, a shown people's panel says how much
 * it looks up to the vessel (Wander.Reverence), and a scan that learned nothing says so there (Wander.ScanNothing) while
 * the vessel stands in it. A locked throne's time left (as the clock counts down) stays on the button's label; its tooltip
 * keeps still (B5: a vessel whose life began after the lock started is told the lock in words of its own - IsLockedBeforeLife).
 * B5 (header section 2.8, 4): while a hazard that kills a vessel at its expiry stands where the wanderer counts as standing
 * (CosmicAutoPause::LethalPlaceOf - under way, the system it left), that system's panel says so under the hazard's line
 * (Wander.Lethal) and every [Travel]'s tooltip that the vessel stands there until it arrives (Wander.TravelLethalTip) - words
 * that promise nothing, and not for a hazard that expires while the vessel is shielded at its birth. The wanderer's lines, its
 * commands and those warnings stand only while the controller shows the vessel (B5 [B-5c] -
 * ACosmicPlayerController::IsVesselShownFor: no calling page open, not the vessel an ended run's last tick installed). The ruler's [Abdicate] stands on the capital's panel only: a first press arms it for AbdicateConfirmSeconds
 * (Wander.AbdicateAgain), a second within them records the abdication (ACosmicPlayerController::Abdicate); greyed while a
 * role effect waits for the tick, with the words of its cause (a recorded abdication, or a fall - Wander.Block.Deposed). Under the hint line, on the capital's
 * panel, the first ship's line once a session (CosmicUI::ShowsShipHint, Hint.FirstShip). In an ended run the wanderer's
 * buttons and [Abdicate] are greyed with Chronicle.Block.
 * The edicts (B2 header section 4): under the hazard's lines on a settled system of the ruled people, the edict that answers
 * it (EdictAnswerLineOf); on the capital's panel, under [Abdicate] and above the first ship's line, the edict hint while it
 * stands (Hint.Edict, CosmicUI::ShowsEdictHint); in the research focus block, under the four buttons and at the right,
 * [Share by hand] (the edict window on its shares' page). The focus heading and buttons judge by preset (CosmicUI::FocusWordOf,
 * IsPresetNow - a button is greyed only when its press would change nothing), and [Society]'s tooltip says what Society
 * research opens (Focus.TipOpensSlot or Focus.TipOpensAll, and Focus.TipOpensRecovery while DT_Rules.SocietyRecoveryBonus
 * gives one).
 * The fleet (B3 header section 4), for the people the player rules: on its settled system, the line Guard.Holding while its
 * fleet guards it (and Guard.Delay while its delay, CosmicConflict::GuardDelayTicksOf at gap 0, is a whole tick - a new
 * hazard there gains time), else
 * [Guard here] greyed by CosmicConflict::GuardBlockOf (the first rung: no fleet yet - CosmicUI::FleetSuffix after
 * the label, the rung the fleet comes at); a siege on it in the line Siege.OnUs with the besieger's name - Siege.OnUsUnknown for a besieger the viewer is
 * not shown - the time left and our side's band (1 - the attacker's odds). On another people's settled system whose owner
 * the viewer is shown, [Send the fleet] greyed by the first reason of CosmicConflict::SiegeBlockOf (its short words after
 * the label, the long ones first in the tooltip; a NoFleet with a fleet is the owner standing too high - Siege.Block.Above),
 * the siege's cost and what follows it in the tooltip (Siege.SendTip) and, enabled, our side's band; or Siege.Ours while
 * our fleet besieges it. A button that would move the fleet from another target says so in its tooltip (Guard.Moves,
 * Siege.Lifts). The lines (GetFleetText) and the button (GetFleetButtons) are a block of their own under the settle
 * button's reason line - the button's place takes the lines a press brings, so nothing below it moves much under the
 * cursor -; the lines' tooltip says what the delay leaves alone and, inside a higher sphere, the attrition
 * (UGalaxySimulationSubsystem::ComputeAttritionPressureIn). Another people's siege of a third people has no line here.
 * A press goes through ACosmicPlayerController::AllocateFleet.
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
	/** The grey hint line under the commands, above the research focus (MVP item 6, ST_CA Hint.Commands): shown while CosmicUI::ShowsCommandHint holds for
	 * the shown system - an enabled project of the ruler on it, the run not over, no project started this session (tests). */
	const UTextBlock* GetHintText() const { return HintText; }
	/** The research focus block (MVP item 7, tests): whether it shows, its heading, and its four buttons in order - Even
	 * (Action NAME_None), Physics, Society, Engineering. */
	bool IsFocusShown() const;
	const UTextBlock* GetFocusHeading() const { return FocusHeading; }
	const TArray<TObjectPtr<UCosmicActionButton>>& GetFocusButtons() const { return FocusButtons; }
	/** The column of the panel's box, top to bottom (tests read the order of its children). */
	const UVerticalBox* GetColumn() const { return Column; }

	/** The wanderer's buttons now shown, in panel order (B1, tests): [Travel] (TravelAction) on a lane neighbour of the
	 * vessel's system; [Scan] (ScanAction) then [Crown] or [Return] (CrownAction) on the system it stands in. */
	const TArray<TObjectPtr<UCosmicActionButton>>& GetWandererButtons() const { return WandererButtons; }
	/** The ruler's [Abdicate] (AbdicateAction): built once, collapsed except on the capital's panel (tests). */
	UCosmicActionButton* GetAbdicateButton() const { return AbdicateButton; }
	/** The capital's first-ship line (tests). */
	const UTextBlock* GetShipHintText() const { return ShipHintText; }
	/** The capital's edict hint line (B2, tests). */
	const UTextBlock* GetEdictHintText() const { return EdictHintText; }
	/** The fleet's lines (B3, tests). */
	const UTextBlock* GetFleetText() const { return FleetText; }
	/** The fleet's button now shown (B3, tests): [Guard here] (GuardAction) or [Send the fleet] (SiegeAction); empty without. */
	const TArray<TObjectPtr<UCosmicActionButton>>& GetFleetButtons() const { return FleetButtons; }
	/** The gate's answer the panel shows on [Guard here] or [Send the fleet] (the guard's in the siege's names -
	 * CosmicConflict::SiegeNameOf); unset when neither button shows (tests: the same as the command's). */
	TOptional<CosmicConflict::ECosmicSiegeBlock> GetShownFleetBlock() const { return ShownFleetBlock; }
	/** The research focus block's [Share by hand] (B2): opens the edict window on its shares' page (tests press it). */
	UCosmicActionButton* GetDirectButton() const { return DirectButton; }
	/**
	 * The edict line under a hazard (B2 header section 4) for the ruled people Ruled, on whose settled system Hazard stands:
	 * the first edict of Hazard.RulerResolutions the people holds (CosmicEdicts::HeldAnswerOf) with Edict.Panel.Held - it
	 * stands, so the hazard resolves at its expiry -, else the first DT_Edicts row it lists with Edict.Panel.Can; empty when
	 * no edict answers it.
	 */
	static FText EdictAnswerLineOf(const FCivilizationState& Ruled, const FHazardDef& Hazard, const FCosmicDataTables& Tables);
	/** The gate's answer the panel shows on [Travel] and on [Crown] or [Return]; unset when the button is not shown (tests:
	 * the same as the command's). */
	TOptional<CosmicWanderer::ETravelBlock> GetShownTravelBlock() const { return ShownTravelBlock; }
	TOptional<CosmicWanderer::ECrownBlock> GetShownCrownBlock() const { return ShownCrownBlock; }
	/**
	 * B5 (header section 3.4, 4): whether the lock on Owner's throne began no later than the vessel's life and the vessel
	 * never sat on a throne in it - Calling.BornTick at or after the lock's start (ThroneLockedUntilTick -
	 * DT_Rules.ThroneLockTicks, 0 or more) and the life's RuledCivIDs empty: a called vessel born into the lock of the death
	 * before it, whom the crown's reason does not tell it left one - Wander.Block.LockedNew in place of Wander.Block.Locked
	 * (the short words stay). False while the throne is not locked.
	 */
	static bool IsLockedBeforeLife(const FGalaxyState& State, const FCivilizationState& Owner, const FCosmicRulesDef& Rules);
	/** What a press on [Abdicate] does at NowSeconds (FPlatformTime seconds - tests pass their own): within
	 * AbdicateConfirmSeconds of the press that armed it, it records the abdication; otherwise it arms it (and its label
	 * asks for the second press until the window closes - a timer redraws it then). */
	void PressAbdicate(double NowSeconds);
	static constexpr double AbdicateConfirmSeconds = 3.0;
	static const FName TravelAction;
	static const FName ScanAction;
	static const FName CrownAction;
	static const FName AbdicateAction;
	static const FName DirectAction;
	static const FName GuardAction;
	static const FName SiegeAction;

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

	// A press on a wanderer's button or on [Abdicate] (B1).
	void HandleWanderer(FName Action, float Value);

	// Whether a press at NowSeconds is the second of two (AbdicateArmedAt, within AbdicateConfirmSeconds).
	bool IsAbdicateArmed(double NowSeconds) const;

	// A focus button's press (Action = the field, NAME_None the even focus; Value unused - the form of OnAction).
	void HandleFocus(FName Field, float Value);

	// [Share by hand] (B2).
	void HandleDirect(FName Action, float Value);

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
	TObjectPtr<UTextBlock> FleetText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ReasonText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HintText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> ButtonBox;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCosmicActionButton>> ActionButtons;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> Column;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> FocusBox;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FocusHeading;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCosmicActionButton>> FocusButtons;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> WandererBox;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCosmicActionButton>> WandererButtons;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> FleetBox;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCosmicActionButton>> FleetButtons;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicActionButton> AbdicateButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ShipHintText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EdictHintText;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicActionButton> DirectButton;

	FName ShownSystemID;
	CosmicMegaprojects::ECosmicSettleBlock ShownSettleBlock = CosmicMegaprojects::ECosmicSettleBlock::None;

	// By SystemID; emptied on a new galaxy or a load (HandleGalaxyGenerated). Presentation memory: never in FGalaxyState.
	TMap<FName, FCosmicSlotWatch> ConstructionWatches;
	TMap<FName, FCosmicSlotWatch> ResponseWatches;

	// The reasons of the project buttons now shown, in button order (the settle button has none here).
	TArray<TPair<FName, CosmicMegaprojects::ECosmicStartBlock>> ShownStartBlocks;

	// The wanderer's gates as the panel shows them (B1); unset without the button.
	TOptional<CosmicWanderer::ETravelBlock> ShownTravelBlock;
	TOptional<CosmicWanderer::ECrownBlock> ShownCrownBlock;
	// The fleet's gate as the panel shows it (B3); unset without the button.
	TOptional<CosmicConflict::ECosmicSiegeBlock> ShownFleetBlock;
	// The FPlatformTime seconds of the press that armed [Abdicate]; below 0 when it is not armed.
	double AbdicateArmedAt = -1.0;
	FTimerHandle AbdicateTimer;
	// The capital whose panel shows the first-ship line in this viewing; None otherwise.
	FName ShipHintSystem;
	// The system where the vessel's last scan learned nothing (its panel says so while the vessel stands there); None
	// otherwise.
	FName ScanNothingAt;
};

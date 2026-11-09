// Cosmic Ascension - UI layer (Presentation). Design: docs/60_DECISIONS/headers/B2_edicts_and_research.md section 2.8, 4;
// the peoples' page: docs/60_DECISIONS/headers/B3_diplomacy_and_conflict.md section 4.
// The UI reads the simulation and listens to its delegates; commands go through the player controller.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Misc/Optional.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEdicts.h"
#include "UI/CosmicUILabels.h"
#include "CosmicEdictWidget.generated.h"

class UCosmicActionButton;
class UScrollBox;
class USlider;
class UTextBlock;
class UVerticalBox;
struct FCivilizationState;
struct FCosmicDataTables;
struct FCosmicKnownView;
struct FCosmicRulesDef;
struct FCosmicStateIndex;
struct FEdictDef;
struct FGalaxyState;
struct FResourcePool;

/** The pages of the edict window. Not a UENUM: presentation state, never saved. */
enum class ECosmicEdictTab : uint8
{
	Edicts,
	Shares,
	// B3: the peoples met - stances, treaties and the fleet.
	Peoples,
};

/**
 * The edict window (B2 header section 4): the ruler's edicts, the research shares and (B3) the peoples met, on the left
 * under the top bar and under the petition card, opened and closed by the top bar's edict button (the system panel's
 * [Share by hand] opens it on its second page). It does not stop the time. Three pages behind three tab buttons - the page
 * shown greyed out, the edicts' tab named with the held edicts and the slots (Edict.Count) - then [Close]:
 * - Edicts: one row per DT_Edicts row (CosmicDataTables::GetEdictRowNames, lexical) of the ruled people's rung or below,
 *   and any it holds: its name, the grey mark of the inherited edict (Edict.Inherited - while the people holds
 *   DT_Rules.StartingEdict, enacted again or not), the grey mark of a galaxy law (Edict.Wide - CosmicEdicts::IsGalaxyLaw,
 *   B6b), its Summary and one button - [Revoke] for a held edict, else [Enact]
 *   with the cost, greyed by the first reason of CosmicEdicts::EnactBlockOf (the command's own gate: the short words after
 *   the label, the long words first in the tooltip). The tooltip then says what the edict costs and does (TipOf). Only the
 *   ruled people's edicts are shown.
 * - Shares: one slider per field but transcendence (SliderFields, 0..CosmicResearch::MaxShareWeight in whole steps - the
 *   mouse follows the steps too; never focusable), the share each gives, the shares the sliders would give, the shares
 *   now, and [Share it so] - enabled only after a person moved a slider (the window's own settings of a slider are not a
 *   move) and for weights CosmicResearch::SharesOf takes; one ACosmicPlayerController::SetResearchShares per press. The
 *   window sets the sliders from the shares now when it opens, and on a refresh when the shares changed by another way
 *   (sliders that give the shares now stay - the weights a person applied stay where they were put); never while a person
 *   holds one or after a person moved one. Before a move the share beside each slider is the share now and the 'would'
 *   line is hidden; three sliders at 0 say that nothing can be shared (Edict.Shares.Zero).
 * - Peoples (B3 header section 4 - the tab Diplomacy.Tab): a head of the fleet (Siege.Fleet - its power and its cap,
 *   CosmicConflict::FleetCapOf; Guard.NoFleet without either - its tooltip how it grows and its upkeep a size), its
 *   target (Guard.Line, Siege.Line with the time left and our side's band, or Guard.Even) with [Withdraw the fleet]
 *   while it has one (its tooltip by the target's kind), and the friendly slots (Diplomacy.Slots - the rung that opens
 *   the next in the tooltip - Diplomacy.SlotsAll from the third rung: the fifth rung's slot comes after the run's end);
 *   then one row per people met and shown (FCosmicKnownView::IsMet and ShownOrderOf, in the meeting order - no
 *   transcendent): its name, its rung (RungWordsOf - B6b decision 31) and the band of the view it holds of us
 *   (CosmicDiplomacy::ViewOf, ViewBandOf - the terms in the tooltip, ViewTipOf); Diplomacy.TheyHostile on a line of its
 *   own when it is hostile toward us; our three stances (the one held greyed and marked with Edict.Mark,
 *   Diplomacy.TipNow; a refused one greyed with the first reason of CosmicDiplomacy::StanceBlockOf - the command's
 *   gate; the tooltips say what a stance costs and breaks - a trade treaty's betrayal, our siege of them lifted); the
 *   two treaties - [Trade] [Non-aggression] greyed by CosmicDiplomacy::TreatyBlockOf, the cost and the acceptance bar
 *   (TreatyBarOf) in the tooltip, or [Break ...] for one held - both tooltips end with Diplomacy.TreatyRungTip while
 *   the tables judge a third-rung row that asks a treaty (CosmicNextRung::HasJudgedRowAskingAt, B6b decision 28); and a
 *   line of each siege between us (Siege.Row.Ours, Siege.Row.Theirs). Diplomacy.None with nobody met. Every press is
 *   one command through the controller. The page is redrawn only while it is shown. The live view moves with the tick:
 *   one tooltip tells it, beside the treaties' bar, to a tenth (cut down, never below a bar it reaches - it agrees with
 *   the gate), so an open tooltip seldom closes; the band's tooltip tells the terms, which keep still.
 * Shown only while open for an embodied ruler in a run that is not over; it closes itself otherwise, and on a new galaxy
 * or a load. Built in C++ (no WBP); redrawn from the simulation's ticks and by the HUD; clicks on it do not reach the map.
 */
UCLASS(meta = (DisableNativeTick))
class COSMIC_ASCENSION_API UCosmicEdictWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Opens the window on Tab with the sliders set from the shares now - nothing without an embodied ruler or in an ended
	 * run. The controller notes the open (ACosmicPlayerController::NoteEdictWindowOpened - the edict hint ends). An open
	 * window only turns to Tab: a person's moves stay. */
	void Open(ECosmicEdictTab Tab);
	/** Closes it (a person's slider moves are forgotten). */
	void Close();
	/** The top bar's button: closes an open window, opens a closed one on the edicts' page. */
	void Toggle();
	bool IsOpen() const { return bOpen; }
	ECosmicEdictTab GetTab() const { return Tab; }

	/** Redraws the open window from the state (the HUD after a command, the simulation's ticks). */
	void Refresh();

	/** What a click on a row's button does (tests call it in place of a click): revokes a held edict, enacts another,
	 * through the controller. A greyed-out button's click never reaches here. */
	void PressRow(FName Row);
	/** A person moves the slider of Field to Weight (tests call it in place of a drag; it takes the drag's path). */
	void MoveSlider(FName Field, int32 Weight);
	/** What a click on [Share it so] does (tests call it in place of a click). Nothing while it is greyed out. */
	void PressApply();
	/** The peoples' page (B3, tests call these in place of a click): a stance button of the people CivID (StanceAt 0
	 * friendly, 1 neutral, 2 hostile) - ACosmicPlayerController::SetStance; a treaty button (TreatyAt 0 trade, 1
	 * non-aggression) - ProposeTreaty, or BreakTreaty for one held; [Withdraw the fleet] - AllocateFleet(None). Nothing for
	 * a greyed-out button or a people not shown. */
	void PressStance(FName CivID, int32 StanceAt);
	void PressTreaty(FName CivID, int32 TreatyAt);
	void PressWithdraw();

	// What tests read.
	/** The rows now shown, in order. */
	const TArray<FName>& GetShownRows() const { return ShownRows; }
	UCosmicActionButton* GetRowButton(FName Row) const;
	const UTextBlock* GetRowMark(FName Row) const;
	/** A row's galaxy-law mark (Edict.Wide - B6b): shown on a CosmicEdicts::IsGalaxyLaw row, collapsed on any other. */
	const UTextBlock* GetRowWideMark(FName Row) const;
	/** The gate's answer a row's [Enact] shows (None for an enabled one); unset for a held row and a row not shown. */
	TOptional<CosmicEdicts::ECosmicEdictBlock> GetShownBlock(FName Row) const;
	UCosmicActionButton* GetApplyButton() const { return ApplyButton; }
	UCosmicActionButton* GetEdictsTabButton() const { return EdictsTabButton; }
	UCosmicActionButton* GetSharesTabButton() const { return SharesTabButton; }
	UCosmicActionButton* GetCloseButton() const { return CloseButton; }
	UCosmicActionButton* GetPeoplesTabButton() const { return PeoplesTabButton; }
	/** The peoples' page (B3, tests): the peoples shown, in order; a row's widgets; the gates' answers its buttons show (a
	 * stance button's - Same for the one held -, a treaty's - unset for a treaty held, whose button breaks it). */
	const TArray<FName>& GetShownPeoples() const { return ShownPeoples; }
	const UWidget* GetPeopleRow(FName CivID) const;
	const UTextBlock* GetPeopleBand(FName CivID) const;
	/** A row's rung beside the people's name (RungWordsOf - B6b decision 31). */
	const UTextBlock* GetPeopleRung(FName CivID) const;
	const UTextBlock* GetPeopleHostility(FName CivID) const;
	const UTextBlock* GetPeopleSiege(FName CivID) const;
	UCosmicActionButton* GetStanceButton(FName CivID, int32 StanceAt) const;
	UCosmicActionButton* GetTreatyButton(FName CivID, int32 TreatyAt) const;
	TOptional<CosmicDiplomacy::ECosmicStanceBlock> GetShownStanceBlock(FName CivID, int32 StanceAt) const;
	TOptional<CosmicDiplomacy::ECosmicTreatyBlock> GetShownTreatyBlock(FName CivID, int32 TreatyAt) const;
	const UTextBlock* GetFleetText() const { return FleetText; }
	const UTextBlock* GetTargetText() const { return TargetText; }
	const UTextBlock* GetSlotsText() const { return SlotsText; }
	const UTextBlock* GetNoPeoplesText() const { return NoPeoplesText; }
	UCosmicActionButton* GetWithdrawButton() const { return WithdrawButton; }
	/** The tab line (tests measure its width). */
	const UWidget* GetTabLine() const { return TabLine; }
	/** The three pages (tests read which shows). */
	const UWidget* GetEdictsPage() const;
	const UWidget* GetSharesPage() const;
	const UWidget* GetPeoplesPage() const;
	/** The weight the slider of Field stands at (INDEX_NONE for a field without one). */
	int32 GetSliderWeight(FName Field) const;
	const UTextBlock* GetWouldText() const { return WouldText; }
	const UTextBlock* GetNowText() const { return NowText; }

	/** The three fields the sliders share out, in the top bar's order (physics, society, engineering). */
	static const FName SliderFields[3];

	/**
	 * The words of an edict's tooltip after its reason: the enactment's cost (Edict.TipCost, or Edict.TipCostNone), the
	 * upkeep (Edict.TipUpkeep with UpkeepWordsOf), one line per effect a tick (an income in percent, Stability in points
	 * over one hundred cycles or years, an ethics axis in words), the fleet's growth and the treaties' acceptance (B3) and
	 * the pressure's drain (B4 - Edict.Effect.PressureDrain) where they change something, the spawn factors and countdown
	 * bonuses by hazard (in
	 * tag order, Edict.TipNewOnly after the bonuses), for a galaxy law (CosmicEdicts::IsGalaxyLaw - B6b) with a spawn or
	 * countdown line Edict.TipWide under them - what reaches the whole home galaxy and what stays the people's -, and for
	 * each DT_Hazards row that lists the row among its RulerResolutions, Edict.TipAnswers. Built from the row, the tables
	 * and the unit alone: the words stay while the stockpile moves. Empty for a row the tables do not hold. The petition
	 * card's edict tooltip reads it too, so a card choice that enacts a galaxy law carries Edict.TipWide.
	 */
	static FText TipOf(const FCosmicDataTables& Tables, FName Row, ECosmicTimeUnit Unit);

	/** The long words of a reason [Enact] is greyed for (ST_CA Edict.Block.*; Tier with the row's rung, 1 to 5; Slots in
	 * two forms - with Society research to grow the slots while a mark is ahead (bSlotsCanGrow), Edict.Block.SlotsAll
	 * once every slot is open); empty for None and for the command's own reasons (NotRuler, Handler), which no button
	 * shows. */
	static FText ReasonOf(CosmicEdicts::ECosmicEdictBlock Block, const FEdictDef* Row, bool bSlotsCanGrow = true);
	/** The short words after a greyed [Enact] (Edict.Suffix.*): for Tier, Conditions, Slots and Cost; empty otherwise. */
	static FText SuffixOf(CosmicEdicts::ECosmicEdictBlock Block, const FEdictDef* Row);

	/** A cost as the window says it: the members above 0 as the stockpile pays them (CosmicEdicts::CostOf), each its label and
	 * amount (a whole amount as a whole number, any other with up to two decimals), joined by CosmicUI::JoinList; empty when
	 * nothing is paid. */
	static FText CostWordsOf(const FResourcePool& Cost);
	/** An upkeep in the viewer's unit: Edict.PerCycle or Edict.PerYear with CostWordsOf (a cycle is a year), or
	 * Edict.NoUpkeep. */
	static FText UpkeepWordsOf(const FResourcePool& Upkeep, ECosmicTimeUnit Unit);
	/** The Society research at which the people's next slot opens: the smallest DT_Rules.EdictSlotSocietyMarks mark it has
	 * not reached, as CosmicEdicts::SlotsOf reads them (reached at Society >= mark; a mark that is not a finite number never
	 * counts); unset when every slot is open. */
	static TOptional<float> NextSlotMarkOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules);
	/** The upkeep a tick of the edicts the people holds and the table knows, each as the stockpile pays it (CosmicEdicts::CostOf),
	 * summed member by member in ActiveEdicts order (the top bar's tooltip). */
	static FResourcePool HeldUpkeepOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables);
	/** The edict hint's words (Hint.Edict with the DisplayName of DT_Rules.StartingEdict); empty when no row is named. */
	static FText HintOf(const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules);

	/** The stances and treaties in the order of the buttons (B3). */
	static FGameplayTag StanceAt(int32 At);
	static FGameplayTag TreatyAt(int32 At);
	/** The word of the view a people holds (ST_CA Diplomacy.View.0..4 - CosmicDiplomacy::ViewBandOf). */
	static FText ViewWordOf(float View);
	/** The terms of the view the people Viewer holds of the people Viewed, a tooltip (B3 section 4): for a bordering pair the
	 * target the view moves to and its ethics, stance (the term of the stance Viewed holds toward Viewer, read from the knob)
	 * and treaty terms; for a pair that does not border the ethics and treaty terms and Diplomacy.TipFar. The view itself
	 * is told beside the treaties' bar only (it moves with the tick). Numbers of the screen only. */
	static FText ViewTipOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, int32 Viewer, int32 Viewed);
	/** The long words of a refused stance (Diplomacy.Block.*, Diplomacy.TipNow for Same); empty for None and the command's
	 * own reasons. */
	static FText StanceReasonOf(CosmicDiplomacy::ECosmicStanceBlock Block);
	/** The long words of a refused treaty (Diplomacy.Treaty.Block.*); empty for None, Held and the command's own reasons. */
	static FText TreatyReasonOf(CosmicDiplomacy::ECosmicTreatyBlock Block);
	/** The long words of a refused siege (Siege.Block.*) - the system panel's [Send the fleet] too; empty for None and the
	 * reasons no button shows (the system is the people's own, wild, dead, being settled, the sanctuary). bAbove: a NoFleet
	 * while the people has a fleet - the owner stands too high for it (Siege.Block.Above). */
	static FText SiegeReasonOf(CosmicConflict::ECosmicSiegeBlock Block, bool bAbove = false);
	/** The short words after a greyed [Send the fleet] (Siege.Suffix.*), for the same reasons; empty otherwise. */
	static FText SiegeSuffixOf(CosmicConflict::ECosmicSiegeBlock Block, bool bAbove = false);
	/** A number as the window says it: up to two decimals, a plain 0 for anything nearer 0 than 0.005 (never '-0'). */
	static FText AmountOf(float Value);
	/** The rung (EAscensionTier's value, 0..3) at which the people's next friendly slot opens: the first above its Tier
	 * and below the fifth whose DT_Rules.FriendlySlotsByTier entry is more than CosmicDiplomacy::FriendlySlotsOf now; unset
	 * when none is (the fifth rung's slot comes after the run's end - B6b decision 27). */
	static TOptional<int32> NextFriendlySlotRungOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules);
	/** The rung words beside a people's name on the peoples' page (B6b decision 31): Panel.Tier and the rung (1 to 5) of
	 * the people CivIndex while the viewer knows it - met (FCosmicKnownView::IsMet) and shown (ShownOrderOf: not one far
	 * above, rule 14); empty otherwise. Reads the view, never writes Knowing. */
	static FText RungWordsOf(const FCosmicKnownView& View, const FGalaxyState& State, int32 CivIndex);
	/** Our side's band of a siege (Siege.Band.* - CosmicConflict::OddsBandOf of the odds read from our side). */
	static FText OddsWordOf(float OursOdds);
	/** The time a siege of the people Attacker has left, in ticks (never below 0). */
	static int64 SiegeTicksLeftOf(const FCivilizationState& Attacker, int64 Tick, const FCosmicRulesDef& Rules);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// A press on the window stops here: it is not a click on the map under it.
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	UFUNCTION()
	void HandleSimTick(int64 Tick);

	UFUNCTION()
	void HandleGalaxyGenerated(int32 Seed);

	// Any slider's value changed: a person's move unless the window is setting the sliders itself.
	UFUNCTION()
	void HandleSliderChanged(float Value);

	UFUNCTION()
	void HandleSliderCaptureBegin();

	UFUNCTION()
	void HandleSliderCaptureEnd();

	// A row's button (Action = the row) and the window's own buttons (the tabs, [Close], [Share it so]).
	void HandleRow(FName Row, float Value);
	void HandleAction(FName Action, float Value);
	// A people's button (Action = the people's CivID, Value = 0..2 a stance, 3..4 a treaty).
	void HandlePeople(FName CivID, float Value);

	// The ruled people of an embodied ruler, in a run that is not over; INDEX_NONE otherwise (the window then closes).
	int32 RuledIndexNow() const;
	// The rows: built again only when the list of shown rows changes (a click spans a press and a release); otherwise each
	// keeps its objects and takes its words and state in place.
	void RefreshEdicts(int32 RuledIndex);
	void RefreshShares(int32 RuledIndex);
	void RefreshPeoples(int32 RuledIndex);
	// Sets the sliders from the shares now - the window's own setting, not a person's move.
	void SetSlidersFromShares(int32 RuledIndex);
	// The weights the sliders stand at, by field.
	TMap<FName, int32> WeightsNow() const;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicActionButton> EdictsTabButton;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicActionButton> SharesTabButton;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicActionButton> CloseButton;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicActionButton> PeoplesTabButton;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> TabLine;

	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> EdictsPage;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> RowBox;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> RowNames;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> RowMarks;

	// The galaxy-law marks (Edict.Wide - B6b), one a row.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> RowWideMarks;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> RowSummaries;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCosmicActionButton>> RowButtons;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> SharesPage;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USlider>> Sliders;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> SliderShares;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> WouldText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NowText;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicActionButton> ApplyButton;

	// The peoples' page (B3).
	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> PeoplesPage;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FleetText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TargetText;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicActionButton> WithdrawButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SlotsText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NoPeoplesText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> PeopleBox;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UVerticalBox>> PeopleRows;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> PeopleNames;

	// The rung beside each name (B6b decision 31).
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> PeopleRungs;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> PeopleBands;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> PeopleHostilities;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> PeopleSieges;

	// Three a people (friendly, neutral, hostile), then two a people (trade, non-aggression).
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCosmicActionButton>> StanceButtons;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCosmicActionButton>> TreatyButtons;

	bool bOpen = false;
	ECosmicEdictTab Tab = ECosmicEdictTab::Edicts;
	// The rows the row widgets were built for, and the reasons of their [Enact] (unset: held).
	TArray<FName> ShownRows;
	TArray<TOptional<CosmicEdicts::ECosmicEdictBlock>> ShownBlocks;
	// The peoples the rows were built for, and the gates' answers their buttons show (three, then two, a people).
	TArray<FName> ShownPeoples;
	TArray<CosmicDiplomacy::ECosmicStanceBlock> ShownStanceBlocks;
	TArray<TOptional<CosmicDiplomacy::ECosmicTreatyBlock>> ShownTreatyBlocks;
	// The window is setting a slider itself; a person moved a slider since the window last set them; a slider is held by
	// the mouse.
	bool bSettingSliders = false;
	bool bMoved = false;
	bool bDragging = false;
};

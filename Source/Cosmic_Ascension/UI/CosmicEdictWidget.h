// Cosmic Ascension - UI layer (Presentation). Design: docs/60_DECISIONS/headers/B2_edicts_and_research.md section 2.8, 4.
// The UI reads the simulation and listens to its delegates; commands go through the player controller.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Misc/Optional.h"
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
struct FCosmicRulesDef;
struct FEdictDef;
struct FResourcePool;

/** The two pages of the edict window. Not a UENUM: presentation state, never saved. */
enum class ECosmicEdictTab : uint8
{
	Edicts,
	Shares,
};

/**
 * The edict window (B2 header section 4): the ruler's edicts and the research shares, on the left under the top bar and
 * under the petition card, opened and closed by the top bar's edict button (the system panel's [Share by hand] opens it
 * on its second page). It does not stop the time. Two pages behind two tab buttons - the page shown greyed out, the
 * edicts' tab named with the held edicts and the slots (Edict.Count) - then [Close]:
 * - Edicts: one row per DT_Edicts row (CosmicDataTables::GetEdictRowNames, lexical) of the ruled people's rung or below,
 *   and any it holds: its name, the grey mark of the inherited edict (Edict.Inherited - while the people holds
 *   DT_Rules.StartingEdict, enacted again or not), its Summary and one button - [Revoke] for a held edict, else [Enact]
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

	// What tests read.
	/** The rows now shown, in order. */
	const TArray<FName>& GetShownRows() const { return ShownRows; }
	UCosmicActionButton* GetRowButton(FName Row) const;
	const UTextBlock* GetRowMark(FName Row) const;
	/** The gate's answer a row's [Enact] shows (None for an enabled one); unset for a held row and a row not shown. */
	TOptional<CosmicEdicts::ECosmicEdictBlock> GetShownBlock(FName Row) const;
	UCosmicActionButton* GetApplyButton() const { return ApplyButton; }
	UCosmicActionButton* GetEdictsTabButton() const { return EdictsTabButton; }
	UCosmicActionButton* GetSharesTabButton() const { return SharesTabButton; }
	UCosmicActionButton* GetCloseButton() const { return CloseButton; }
	/** The weight the slider of Field stands at (INDEX_NONE for a field without one). */
	int32 GetSliderWeight(FName Field) const;
	const UTextBlock* GetWouldText() const { return WouldText; }
	const UTextBlock* GetNowText() const { return NowText; }

	/** The three fields the sliders share out, in the top bar's order (physics, society, engineering). */
	static const FName SliderFields[3];

	/**
	 * The words of an edict's tooltip after its reason: the enactment's cost (Edict.TipCost, or Edict.TipCostNone), the
	 * upkeep (Edict.TipUpkeep with UpkeepWordsOf), one line per effect a tick (an income in percent, Stability in points
	 * over one hundred cycles or years, an ethics axis in words), the spawn factors and countdown bonuses by hazard (in
	 * tag order, Edict.TipNewOnly after the bonuses), and for each DT_Hazards row that lists the row among its
	 * RulerResolutions, Edict.TipAnswers. Built from the row, the tables and the unit alone: the words stay while the
	 * stockpile moves. Empty for a row the tables do not hold.
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

	// The ruled people of an embodied ruler, in a run that is not over; INDEX_NONE otherwise (the window then closes).
	int32 RuledIndexNow() const;
	// The rows: built again only when the list of shown rows changes (a click spans a press and a release); otherwise each
	// keeps its objects and takes its words and state in place.
	void RefreshEdicts(int32 RuledIndex);
	void RefreshShares(int32 RuledIndex);
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
	TObjectPtr<UScrollBox> EdictsPage;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> RowBox;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> RowNames;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> RowMarks;

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

	bool bOpen = false;
	ECosmicEdictTab Tab = ECosmicEdictTab::Edicts;
	// The rows the row widgets were built for, and the reasons of their [Enact] (unset: held).
	TArray<FName> ShownRows;
	TArray<TOptional<CosmicEdicts::ECosmicEdictBlock>> ShownBlocks;
	// The window is setting a slider itself; a person moved a slider since the window last set them; a slider is held by
	// the mouse.
	bool bSettingSliders = false;
	bool bMoved = false;
	bool bDragging = false;
};

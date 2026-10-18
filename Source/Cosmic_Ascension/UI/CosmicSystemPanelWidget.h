// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 5.7, 5.12, 6.3, 8.3, 11 Phase 3.
// The UI reads the simulation and listens to its delegates; commands go through the player controller.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Sim/CosmicMegaprojects.h"
#include "CosmicSystemPanelWidget.generated.h"

class UCosmicActionButton;
class UTextBlock;
class UVerticalBox;
struct FCosmicDataTables;
struct FCosmicSpiritLook;

/**
 * The panel of the selected system, on the right of the screen (hidden with no selection): its name, star class, owner
 * (or none, or destroyed) and right after it the owner people's band (ST_CA Panel.People - section 8.3, never a number;
 * none on a wild or destroyed system), the hazard standing there with the time it has left (in the clock's unit - cycles
 * or years, the controller's choice; the HUD redraws the panel when it changes), the construction and response slots
 * with their progress against what they need; in the spiritual view (the controller's view drawn) a line of its own under
 * the details with the words of the sight's mark, which it also carries as its tooltip (SpiritTipOf - the fallen one's
 * title, no number; a block apart, so that the words never cover the people's band) - then, for the people the player
 * rules, the commands: one button per
 * project CosmicMegaprojects::CanStart takes on its settled system (rows in lexical order), and on every living system
 * that is not the people's a settle button - greyed out, with the reason as its tooltip and as a line under it, when
 * CosmicMegaprojects::SettleBlockOf refuses (another people lives there, no lane, the settle row's rung, the realm cap of
 * section 5.12, ...). No commands while the vessel is not an embodied ruler. The buttons are built again only when the
 * list of their actions changes; otherwise each keeps its object and takes its label, state and tooltip in place - a
 * click spans a press and a release, and a button built again between them drops it. Built in C++ (no WBP).
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

	/** The command buttons now shown (tests). */
	const TArray<TObjectPtr<UCosmicActionButton>>& GetActionButtons() const { return ActionButtons; }
	/** The reason the shown settle button is greyed out; None when it is enabled or there is no settle button. */
	CosmicMegaprojects::ECosmicSettleBlock GetShownSettleBlock() const { return ShownSettleBlock; }
	/** Whether the reason line under the settle button shows. */
	bool IsReasonShown() const;
	/**
	 * The words of a reason a settlement is refused (ST_CA Settle.Block.*, the rung read from the settle row and the
	 * sphere rule); empty for None, NoPeople and Dead, which no settle button shows.
	 */
	static FText ReasonOf(CosmicMegaprojects::ECosmicSettleBlock Block, const FCosmicDataTables& Tables);

	/**
	 * The words of the spiritual sight for a system (section 4.2, 6.4): the Title of Spirit.MarkRow - a title, never a
	 * name, no number - in the words of its mark (ST_CA Sight.Claimed, Sight.Held, Sight.Pressed for the fallen one that
	 * presses, Sight.Bound for the sealed holder alone; the particle by |hpp). Empty when nothing marks the system, when
	 * the tables do not hold the row and when its Title is empty (a slip the schema refuses for a fallen row).
	 */
	static FText SpiritTipOf(const FCosmicSpiritLook& Spirit, const FCosmicDataTables& Tables);
	/** The block of detail lines (tests read its text). */
	const UTextBlock* GetDetailText() const { return DetailText; }
	/** The line of the sight's words under the detail lines, with the same words as its tooltip (tests): shown in the
	 * spiritual view when SpiritTipOf has words, collapsed and empty otherwise. */
	const UTextBlock* GetMarkText() const { return MarkText; }

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

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DetailText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> MarkText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ReasonText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> ButtonBox;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCosmicActionButton>> ActionButtons;

	FName ShownSystemID;
	CosmicMegaprojects::ECosmicSettleBlock ShownSettleBlock = CosmicMegaprojects::ECosmicSettleBlock::None;
};

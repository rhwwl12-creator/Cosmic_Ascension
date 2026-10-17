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

/**
 * The panel of the selected system, on the right of the screen (hidden with no selection): its name, star class, owner
 * (or none, or destroyed), the hazard standing there with the time it has left (in the clock's unit - cycles or years,
 * the controller's choice; the HUD redraws the panel when it changes), the construction and response slots
 * with their progress against what they need - then, for the people the player rules, the commands: one button per
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
	TObjectPtr<UTextBlock> ReasonText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> ButtonBox;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCosmicActionButton>> ActionButtons;

	FName ShownSystemID;
	CosmicMegaprojects::ECosmicSettleBlock ShownSettleBlock = CosmicMegaprojects::ECosmicSettleBlock::None;
};

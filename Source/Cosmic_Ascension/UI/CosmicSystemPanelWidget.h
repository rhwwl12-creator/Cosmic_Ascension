// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 5.7, 5.12, 6.3, 8.3, 11 Phase 3.
// The UI reads the simulation and listens to its delegates; commands go through the player controller.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CosmicSystemPanelWidget.generated.h"

class UCosmicActionButton;
class UTextBlock;
class UVerticalBox;

/**
 * The panel of the selected system, on the right of the screen (hidden with no selection): its name, star class, owner
 * (or none, or destroyed), the hazard standing there with the cycles it has left, the construction and response slots
 * with their progress against what they need - then, for the people the player rules, the commands: one button per
 * project CosmicMegaprojects::CanStart takes on its settled system (rows in lexical order), a settle button on a wild
 * system it CanSettle, and on a wild system one lane from it a line when its realm cap stops the settlement (section
 * 5.12). No commands while the vessel is not an embodied ruler. Built in C++ (no WBP).
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
	/** Whether the realm-cap line shows. */
	bool IsRealmCapShown() const;

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
	TObjectPtr<UTextBlock> RealmCapText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> ButtonBox;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCosmicActionButton>> ActionButtons;

	FName ShownSystemID;
};

// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 6.3, 11 Phase 3.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CosmicActionButton.generated.h"

class UButton;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCosmicAction, FName /* Action */, float /* Value */);

/**
 * A button with its label that names an action when pressed: a project row, project_settle, or a speed (with its value).
 * One handler serves every button of a panel. Built in C++ (no WBP); never takes the keyboard focus, so Space and the
 * number keys keep reaching the controller after a click. A double-click's second press is no press: SButton takes an
 * unhandled double-click as one more press, and a button that turns in place under the cursor ([Trade] to [Break trade],
 * [Enact] to [Revoke]) would undo its own command at once - the Slate button handles the double-click itself (as the
 * system's double-click setting tells it). A press from code (Press) is never held back.
 */
UCLASS(meta = (DisableNativeTick))
class COSMIC_ASCENSION_API UCosmicActionButton : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(FName InAction, float InValue, const FText& Label);
	FName GetAction() const { return Action; }
	float GetValue() const { return Value; }
	const FText& GetLabel() const { return ShownLabel; }

	/** What a click does: announces the action (tests call it in place of a click). A greyed-out button announces nothing. */
	void Press();

	FOnCosmicAction OnAction;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UFUNCTION()
	void HandleClicked();

	UPROPERTY(Transient)
	TObjectPtr<UButton> Button;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> Text;

	FName Action;
	float Value = 0.f;
	// Kept for a tree built after Setup.
	FText ShownLabel;
};

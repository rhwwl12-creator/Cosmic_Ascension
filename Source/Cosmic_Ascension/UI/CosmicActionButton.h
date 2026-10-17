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
 * number keys keep reaching the controller after a click.
 */
UCLASS(meta = (DisableNativeTick))
class COSMIC_ASCENSION_API UCosmicActionButton : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(FName InAction, float InValue, const FText& Label);
	FName GetAction() const { return Action; }
	float GetValue() const { return Value; }

	/** What a click does: announces the action (tests call it in place of a click). */
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

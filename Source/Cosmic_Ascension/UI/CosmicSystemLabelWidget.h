// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 6.4, 11 Phase 3.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "World/CosmicMapLook.h"
#include "CosmicSystemLabelWidget.generated.h"

class UProgressBar;
class UTextBlock;

/** The label of one system on the map: its name, and a bar for each slot that holds a project - or, at the universe level,
 * of one named galaxy: its name alone (a look whose two shares are -1). Built in C++ (no WBP). */
UCLASS(meta = (DisableNativeTick))
class COSMIC_ASCENSION_API UCosmicSystemLabelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Show(const FCosmicSystemLook& Look);
	/** The look shown last (tests read a galaxy label's name from it). */
	const FCosmicSystemLook& GetShownLook() const { return Shown; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> ConstructionBar;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> ResponseBar;

	// The look shown last: a look given before the tree is built is shown when it is.
	FCosmicSystemLook Shown;
};

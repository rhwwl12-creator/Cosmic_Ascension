// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 6.1, 10.2, 11 Phase 1.
// The UI reads the simulation and listens to its delegates; it never changes the state.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/GalaxyState.h"
#include "CosmicTextPanelWidget.generated.h"

class UScrollBox;
class UTextBlock;

/**
 * The one text panel of the Phase 1 prototype: cycle and speed, then the tier, stability, resources and research of the civ
 * the player rules and the projects of its systems (with the prototype's project key), then the tail of the log. Built in C++ without a Widget Blueprint; the WBP layouts of Phase 3 replace it.
 * Labels come from the String Table ST_CA. Refreshed from the subsystem delegates, never from a tick.
 */
// DisableNativeTick: the meta of UUserWidget is not inherited, and Widget Blueprints made from this class should not tick.
UCLASS(meta = (DisableNativeTick))
class COSMIC_ASCENSION_API UCosmicTextPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Redraws everything from the current state. For changes no delegate announces, such as a new time scale while paused.
	void Refresh();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleSimTick(int64 Tick);

	UFUNCTION()
	void HandleGalaxyGenerated(int32 Seed);

	UFUNCTION()
	void HandleLogEntry(const FCosmicLogEntry& Entry);

	void RefreshStatus();
	void RefreshLog();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LogText;

	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> LogScroll;
};

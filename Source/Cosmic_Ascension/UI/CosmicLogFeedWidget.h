// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 6.3, 8.1, 8.2, 9.4, 11 Phase 3.
// The UI reads the simulation and listens to its delegates; a click goes through the player controller.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "UI/CosmicUILabels.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "CosmicLogFeedWidget.generated.h"

class UListView;
class UTextBlock;
struct FCosmicLogEntry;

/** One line of the feed: a copy of a told log entry. */
UCLASS()
class COSMIC_ASCENSION_API UCosmicLogItem : public UObject
{
	GENERATED_BODY()

public:
	int64 Tick = 0;
	FText Message;
	uint8 Severity = 0;
	FGameplayTag Category;
	FName FocusSystemID;
	FName FocusCivID;
};

/** The widget of one line; the list makes and reuses these. Built in C++ (no WBP). The stamp follows the clock's unit
 * of the owning controller. */
UCLASS(meta = (DisableNativeTick))
class COSMIC_ASCENSION_API UCosmicLogEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	/** Shows Item (what the list does through NativeOnListItemObjectSet; tests call it on a widget of their own). */
	void ShowItem(const UCosmicLogItem* Item);
	/** Draws the shown item again (the clock's unit changed). */
	void Redraw();
	/** The line's text block (tests). */
	const UTextBlock* GetText() const { return Text; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> Text;

	TWeakObjectPtr<const UCosmicLogItem> ShownItem;
};

namespace CosmicLogFeed
{
	/** The colour of a line (section 8.2): Severity 0 grey, 1 white, 2 gold; the spirit's own voice (Log.Transcendent,
	 * Log.Claim) purple - four colours at most. */
	COSMIC_ASCENSION_API FLinearColor ColorOf(uint8 Severity, FGameplayTag Category);

	/** The lines the feed holds at most (section 9.4). */
	inline constexpr int32 MaxItems = 500;

	/** The text of a line: its stamp in Unit, then the message ("[1,204] ..." or the year form). */
	COSMIC_ASCENSION_API FText LineText(const UCosmicLogItem& Item, ECosmicTimeUnit Unit);
}

/**
 * The log feed at the bottom left of the screen: the last told entries of the log (section 8.1 - an entry marked bUntold
 * never shows) in a virtualized list (section 6.3), oldest first, coloured by severity. A new galaxy or a load rebuilds it
 * from the state; a new line arrives through OnLogEntry, and the list follows it when it showed the end. A click on a line
 * goes to its place - the focus system, or else the capital of its focus people - when the viewer knows that place (the
 * controller's SelectAndFocus refuses any other, section 8.1). Built in C++ (no WBP).
 */
UCLASS(meta = (DisableNativeTick))
class COSMIC_ASCENSION_API UCosmicLogFeedWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Rebuilds the items from the state. */
	void Refresh();
	/** The items now held, oldest first (tests). */
	const TArray<TObjectPtr<UCosmicLogItem>>& GetItems() const { return Items; }
	/** What a click on Item does: goes to its place as the class comment says (tests call it in place of a click). */
	void OpenItem(UCosmicLogItem* Item);
	/** Draws the shown lines again, their stamps in the clock's unit (the HUD calls it when the unit changes). */
	void RedrawLines();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// A press on the feed stops here: it is not a click on the map under it.
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	UFUNCTION()
	void HandleGalaxyGenerated(int32 Seed);

	UFUNCTION()
	void HandleLogEntry(const FCosmicLogEntry& Entry);

	void HandleItemClicked(UObject* Item);
	// A new item at the end, the oldest dropped past MaxItems.
	void Append(const FCosmicLogEntry& Entry);

	UPROPERTY(Transient)
	TObjectPtr<UListView> List;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCosmicLogItem>> Items;
};

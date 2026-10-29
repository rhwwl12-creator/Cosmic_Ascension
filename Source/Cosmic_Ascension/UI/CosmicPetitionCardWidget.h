// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 5.9, 5.10, 6.3; docs/60_DECISIONS/headers/A2_petition_cards.md section 6.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Sim/CosmicConflict.h"
#include "CosmicPetitionCardWidget.generated.h"

class UBorder;
class UCosmicActionButton;
class UTextBlock;
class UVerticalBox;

/**
 * The petition card (MVP item 2): the card that waits for the ruler's answer (Knowing.PendingPetitionRow, read through
 * the controller's known view), in the middle of the screen over the panels and under the ascension presentation. From
 * the top: the title; the voice that brings it - a voiced card only: the council or the elders of the congregation,
 * picked when the card shows by the ruled people's Spiritualism (above 0 the elders) and kept while it shows -; its text;
 * one button per choice with a line under it: the cost, and how many of the six rolls pass the check. A choice the
 * stockpile cannot pay is greyed out, the reason its tooltip. Nothing on the card says which choice is right.
 * A click answers through the controller (ACosmicPlayerController::AnswerPetition); the card then shows the outcome line
 * and one button - Close, or Next petition when the answer chained a card - until it is closed. The controller shows
 * the waiting card (FollowPetition) and redraws it every tick. Built in C++ (no WBP); clicks on it do not reach the map.
 * A wanderer's card (B1, CosmicPetitions::IsWandererCard) is about the system the vessel stands in and its people
 * (CosmicPetitions::VesselSubjectOf - the answer's own subject): no voice, every choice can be given (a wanderer pays
 * nothing), its rolls are the vessel's stats, and a withdrawn one says so in its own words (Wander.Withdrawn).
 * The first contact card (B3 header section 4) names the other people ({OtherCiv} - the view's PendingContactCivID), and
 * a choice that turns a people friendly says under it what the friendly stance holds: its upkeep and the friendly slots
 * (Petition.FriendlyUpkeep).
 * A choice the card's gate refuses (B4 header section 4 - CosmicPetitions::ChoiceGateOf, the gate the answer reads: a
 * stance, an edict it cannot enact, the fleet's time it cannot buy) is greyed out while the card stands, the first reason
 * its tooltip - after the stockpile's - and the short words of an edict's reason or of a fleet with no cap yet after its
 * label (the edict window's and the panel's form; the fleet's rung from CosmicUI::FleetSuffix); an edict the people already
 * holds carries the held mark Edict.Mark in front of its label instead (B4 [B-4d2]). A choice that enacts an edict says under it the edict's cost and the
 * slots (Petition.EdictLine), and its tooltip what the edict costs and does (UCosmicEdictWidget::TipOf); one that buys a
 * standing hazard time with the fleet, the fleet now and after, the hazard and the time added (Petition.ResistLine).
 */
UCLASS(meta = (DisableNativeTick))
class COSMIC_ASCENSION_API UCosmicPetitionCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Shows the card of Row about SystemID (and, for the first contact card, the people OtherCivID - B3) with the voice
	 * picked now, its choices built again; None collapses the card. */
	void ShowCard(FName Row, FName SystemID, FName OtherCivID = NAME_None);

	/** Redraws the choices of the card that shows: payable or not, whether their gate is open (CosmicPetitions::ChoiceGateOf -
	 * an edict revoked in the edict window opens a greyed choice while the card waits), the rolls that pass and the lines
	 * under them. Nothing while it shows an outcome. */
	void Refresh();

	/** What a click on a choice does (tests call it in place of a click). Nothing while an outcome shows or no card shows. */
	void PressChoice(int32 ChoiceIndex);

	/** What a click on the button under an outcome does (tests call it in place of a click): the card closes and the
	 * controller shows the next card or lets the time run again (ACosmicPlayerController::ClosePetitionCard). */
	void PressClose();

	/** The row and the system of the card that shows; None when the card is collapsed. */
	FName GetShownRow() const { return ShownRow; }
	FName GetShownSystem() const { return ShownSystem; }
	/** Whether an answer's outcome shows (until the card is closed). */
	bool IsShowingOutcome() const { return bShowingOutcome; }
	/** The voice that brings the card: 0 the council, 1 the elders of the congregation, INDEX_NONE without voices. */
	int32 GetVoice() const { return Voice; }

	// What tests read.
	const UTextBlock* GetTitleText() const { return TitleText; }
	const UTextBlock* GetVoiceText() const { return VoiceText; }
	const UTextBlock* GetBodyText() const { return BodyText; }
	const UTextBlock* GetOutcomeText() const { return OutcomeText; }
	const UCosmicActionButton* GetCloseButton() const { return CloseButton; }
	int32 GetChoiceCount() const { return ChoiceButtons.Num(); }
	const UCosmicActionButton* GetChoiceButton(int32 ChoiceIndex) const { return ChoiceButtons.IsValidIndex(ChoiceIndex) ? ChoiceButtons[ChoiceIndex].Get() : nullptr; }
	const UTextBlock* GetChoiceLine(int32 ChoiceIndex) const { return ChoiceLines.IsValidIndex(ChoiceIndex) ? ChoiceLines[ChoiceIndex].Get() : nullptr; }

	/** The long words of a refused fleet's time (B4): Petition.Resist.Block.NoHazard and .Capital with the hazard's name
	 * Hazard, Guard.Block.NoFleet for a fleet with no cap (the first rung's), Siege.Block.NoFleet for a spent fleet (the
	 * panel's words for the same fleet); empty for None and NotOwn, which no card shows (a card whose system is no longer
	 * the people's settled one no longer stands). */
	static FText ResistReasonOf(CosmicConflict::ECosmicResistBlock Block, const FText& Hazard);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void HandleAction(FName Action, float Value);

	UPROPERTY(Transient)
	TObjectPtr<UBorder> Box;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> VoiceText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BodyText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> ChoiceBox;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCosmicActionButton>> ChoiceButtons;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> ChoiceLines;

	// The choices' labels as the card was shown (the gate's short words go after them).
	TArray<FText> ChoiceLabels;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OutcomeText;

	UPROPERTY(Transient)
	TObjectPtr<UCosmicActionButton> CloseButton;

	FName ShownRow;
	FName ShownSystem;
	// The other people of the card that shows (B3: the first contact card's).
	FName ShownOther;
	int32 Voice = INDEX_NONE;
	bool bShowingOutcome = false;
	// The card that shows is a wanderer's (B1).
	bool bShownWanderer = false;
};

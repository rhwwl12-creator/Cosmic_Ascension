// Cosmic Ascension - UI layer (Presentation). Design: docs/60_DECISIONS/headers/B5_death_succession_two_books.md section 4, 2.6,
// 2.7, 2.10, 3.6 (B5 [B-5c]).
// The calling's pages: the three pages of a death (the epitaph, the digest, the inauguration) and the reign end of an abdication
// or a deposition (in place of an epitaph). The chronicle's veil and one centred column, line by line (UI/CosmicChronicleWidget.h:
// the veil in VeilSeconds, a line every LineSeconds, the page within RevealCapSeconds) - no glow, no chime, no rising motion: never
// the ascension presentation, and every death in the same form and colours. Reads the state, writes nothing, reads no Knowing; its
// buttons go through the player controller, which runs the leap (ACosmicPlayerController::NextCallingPage).

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CosmicCallingPagesWidget.generated.h"

class UBorder;
class UCosmicActionButton;
class UScrollBox;
class UWidget;
struct FCosmicDataTables;
struct FEncounterDef;
struct FGalaxyState;
struct FHazardDef;

/** Which of the calling's pages shows (presentation state: never saved, outside both hashes). */
enum class ECosmicCallingPage : uint8
{
	None,
	// A life closed: its cause, its source, the cycles it lived, the highest rung its people reached (none for the first - B6b
	// decision 30), its feats and its last answers.
	Epitaph,
	// What the world did while no vessel stood (CosmicDigest::LinesOf).
	Digest,
	// The new vessel: its facts and the fixed line of decision 12-na.
	Inauguration,
	// An abdication or a deposition within one life: the reign's cycles and why it ended - no death word, no 'the Nth'.
	ReignEnd,
};

/** One of a life's last answers as its epitaph shows it. */
struct FCosmicAnswerTitle
{
	// The card's DT_Encounters row (FLifeRecord.LastChoiceRows).
	FName Row;
	// The card's title, its slots filled from the answer's line.
	FText Title;
	// The newest answer, on the card of the very hazard that closed the life (on the system it closed it in, when the state still
	// says where): the epitaph sets it apart from the cause line (header section 3.6 1 - no reading of a sacrifice).
	bool bOfTheDeath = false;
};

namespace CosmicCallingPages
{
	/** Whether Card is the card of Hazard: one of its conditions names the hazard's tag (enc_star_watch's PlayerInSystemWithHazard
	 * Hazard.PreSupernova, petition_unstable_star's SystemHasHazard). */
	COSMIC_ASCENSION_API bool IsCardOfHazard(const FEncounterDef& Card, const FHazardDef& Hazard);

	/**
	 * The titles of the last answers of Lives[LifeAt] (LastChoiceRows - the last three, oldest first), in time order. Each row is
	 * paired with its answer's line - the newest entry of State.Chronicle and State.Log within the life (BornTick to DiedTick, or
	 * to State.Tick while it is open) whose SourceRow is the card's row (UPlayerRoleSubsystem::AnswerPetition's outcome line),
	 * from the newest answer back, each line used once. A title's slots are filled from that line: {System} from its
	 * FocusSystemID, {Civ} from its FocusCivID (a people the state still holds). A title with a slot the line cannot fill - any
	 * other slot ({OtherCiv} of petition_first_contact, {Hazard}, {Rebel}), a slot without a line, a system or a people the
	 * state no longer holds - is left out; so is a row the tables do not hold, and a card whose title speaks to the reader or
	 * exalts the vessel (CosmicRows::EpitaphLeavesOut - decision 12-da: no 'you' across a death; section 4: nothing rises at a
	 * death). bOfTheDeath on the newest answer only (see FCosmicAnswerTitle), and only for a life closed by Death.Hazard. Reads
	 * the state and the tables; no Knowing.
	 */
	COSMIC_ASCENSION_API TArray<FCosmicAnswerTitle> LastAnswersOf(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 LifeAt);
}

/**
 * The calling's pages over the stopped map (B5 header section 4): the veil, then one column of lines revealed one after another,
 * and under it the page's buttons - shown once every line shows (a click on the veil shows them all at once). The epitaph has
 * 'next' and, from a second death on, 'skip'; the digest 'next'; the inauguration and the reign end 'to the map'. A press goes
 * to the owning controller (NextCallingPage, SkipCallingPages); the controller opens each page and folds them. A click on the
 * pages is no click on the map. Over the card and the ascension presentation, under the chronicle (ACosmicHUD). Built in C++.
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicCallingPagesWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** The epitaph of Lives[InLifeAt]: the title 'the Nth' with the vessel's name (third person - decision 12-da), the cause and
	 * its source (a hazard's name; a cause whose source is a card names the hazard the card belongs to - IsCardOfHazard - or
	 * nothing: never a card's title, an answer never shares the cause's line), the cycles lived (DiedTick - BornTick), the
	 * highest rung the people reached while the vessel ruled (Epitaph.Peak - no line for the first rung, B6b decision 30), the
	 * feats, the last answers
	 * (LastAnswersOf - the one of the death set apart below the others); no epithet. bSkippable shows 'skip' (a second or later
	 * death). bAtOnce: no fade, no reveal (a continued run). */
	void OpenEpitaph(int32 InLifeAt, bool bSkippable, bool bAtOnce);
	/** The digest of the interregnum that followed the last closed life: the lines of CosmicDigest::LinesOf(Sim,
	 * CosmicDigest::LastClosedLifeOf, the owning controller's see-all), one row each - its head (Digest.Head) as the title, a
	 * leap of 0's one line as it is. The veil stays up from the epitaph. */
	void OpenDigest();
	/** The inauguration of Lives[InLifeAt]: 'the Nth', the new vessel's facts (its name, the people it was born among - the
	 * chronicle's words, Chronicle.BornAmong -, its system, its body and mind) and the fixed line Succession.Kept. */
	void OpenInauguration(int32 InLifeAt);
	/** The reign end of the last reign of Lives[InLifeAt]: its people with the cycles it was ruled (ReignTicks) and why it ended -
	 * Reign.* keys only, no Epitaph. key, no 'the Nth', no death word, no cycles lived. */
	void OpenReignEnd(int32 InLifeAt);
	/** Folds the pages (the controller's CloseCallingPages). */
	void Close();

	bool IsOpen() const { return Shown != ECosmicCallingPage::None; }
	ECosmicCallingPage GetPage() const { return Shown; }
	// The life the page is about (INDEX_NONE while folded): the closed life of the epitaph and the digest, the new one of the
	// inauguration, the walking one of the reign end.
	int32 GetLifeAt() const { return LifeAt; }
	bool IsRevealing() const { return IsOpen() && !bRevealed; }   // lines still to show (the buttons wait)
	void Advance(float DeltaSeconds);                             // the fade and the reveal (its tick; tests)
	void PressVeil();                                             // tests in place of a click: every line at once, then the buttons
	void PressNext();                                             // the page's 'next' (or 'to the map'): the controller's NextCallingPage; nothing while revealing
	void PressSkip();                                             // the epitaph's 'skip': the controller's SkipCallingPages; nothing while revealing or when not shown

	TArray<FText> GetPageTexts() const { return RowTexts; }       // tests: the page's lines in order
	TArray<FString> GetPageKeys() const { return Keys; }          // tests: every ST_CA key the page's words came from (its buttons too)
	int32 GetApartRow() const { return ApartRow; }                // tests: the row of the answer set apart, INDEX_NONE for none
	int32 GetCauseRow() const { return CauseRow; }                // tests: the epitaph's cause row, INDEX_NONE on another page
	int32 GetFactsEndRow() const { return FactsEndRow; }          // tests: the epitaph's last fact row (the cause, the cycles, the rung above the first, the feats)
	const TArray<FCosmicAnswerTitle>& GetAnswers() const { return Answers; }   // tests: the epitaph's answers as shown
	UCosmicActionButton* GetNextButton() const { return NextButton; }
	UCosmicActionButton* GetSkipButton() const { return SkipButton; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	// A press on the pages stops here (PressVeil while the reveal runs): it is not a click on the map under them.
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void HandleAction(FName Action, float Value);
	// The common start of a page: the rows cleared and the veil from 0 - or kept up when a page already shows.
	void Begin(ECosmicCallingPage Page, int32 InLifeAt);
	// The common end: the buttons labelled (NextKey; 'skip' with bSkippable), shown, and the reveal (bAtOnce: every line at once).
	void Finish(const TCHAR* NextKey, bool bSkippable, bool bAtOnce);
	// One row of words (a line of the column) with its size, colour and gap above.
	void AddRow(const FText& Words, int32 Size, const FLinearColor& Color, float GapAbove);
	// A label of the String Table (CosmicUI::Label), noted in Keys.
	FText LabelOf(const TCHAR* Key);
	// The veil's colour, the rows' opacities and the buttons for the time Elapsed since the page began.
	void ShowProgress();

	UPROPERTY(Transient) TObjectPtr<UBorder> Veil;
	UPROPERTY(Transient) TObjectPtr<UScrollBox> Column;
	UPROPERTY(Transient) TArray<TObjectPtr<UWidget>> LineRows;
	UPROPERTY(Transient) TObjectPtr<UWidget> ButtonRow;
	UPROPERTY(Transient) TObjectPtr<UCosmicActionButton> NextButton;
	UPROPERTY(Transient) TObjectPtr<UCosmicActionButton> SkipButton;

	TArray<FText> RowTexts;
	TArray<FString> Keys;
	TArray<FCosmicAnswerTitle> Answers;
	ECosmicCallingPage Shown = ECosmicCallingPage::None;
	int32 LifeAt = INDEX_NONE;
	int32 ApartRow = INDEX_NONE;
	int32 CauseRow = INDEX_NONE;
	int32 FactsEndRow = INDEX_NONE;
	bool bSkipShown = false;
	bool bRevealed = false;
	// Real seconds since the page began (a page after the first starts with the veil already up).
	float Elapsed = 0.f;
};

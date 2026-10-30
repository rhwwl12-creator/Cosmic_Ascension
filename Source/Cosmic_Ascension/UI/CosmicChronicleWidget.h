// Cosmic Ascension - UI layer (Presentation). Design: docs/70_CONTENT_GUIDE.md section 8.4, headers/A4_provisional_end.md section 5 (MVP item 4).
// The chronicle screen: reads the state through UCosmicLogSubsystem::BuildChronicle (the book, never Knowing - 16 #49) and
// writes nothing; its buttons go through the player controller.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Misc/Optional.h"
#include "Sim/CosmicEndings.h"
#include "CosmicChronicleWidget.generated.h"

class UBorder;
class UCosmicActionButton;
class UScrollBox;
class UTextBlock;
class UWidget;

namespace CosmicChronicleScreen
{
	// Real seconds the stopped map stays in view before the veil (after the third rung's reveal, or at the end tick).
	inline constexpr float HoldSeconds = 1.5f;
	// The veil darkens over this many real seconds.
	inline constexpr float VeilSeconds = 1.f;
	// The lines show one after another LineSeconds apart, the whole page within RevealCapSeconds.
	inline constexpr float LineSeconds = 0.25f;
	inline constexpr float RevealCapSeconds = 6.f;
	// The opacity of another people's lines, against 1 for the life's own (inside the four colours of section 8.2).
	inline constexpr float WorldLineOpacity = 0.7f;
	// A second press of 'new run' within these real seconds starts it.
	inline constexpr float ConfirmSeconds = 3.f;
	// The top bar's quiet notice stands over the last EndSoonTicks ticks before the clock end (one minute at x10).
	inline constexpr int64 EndSoonTicks = 600;
}

/** The chronicle screen (MVP item 4, section 8.4's minimal form): a dark veil, one centred column in a scroll box (life chapters, the epilogue's two fixed
 * lines, the horizon line) and, outside the scroll, the cause note, the provisional notice and two buttons. A click on the page is not a map click.
 * A chapter is its title, its facts (the peoples ruled, or for a life that never ruled ST_CA Chronicle.BornAmong with its birth people, then 'wandering';
 * the rung; the cycles), then - B5 - the span with no vessel that led to it under ST_CA Chronicle.NoVessel, its body, and on the last chapter the span
 * after its death when no shown life follows (FCosmicChronicleChapter::Before, After; a span without a line shows nothing). */
UCLASS()
class COSMIC_ASCENSION_API UCosmicChronicleWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void Open(ECosmicProvisionalEnd Cause, bool bAtOnce);   // Cause picks the note (None = the development preview: no note, no provisional notice, no 'new run'); bAtOnce: no fade, no reveal
	void Close();                                           // a 'new run' half pressed is forgotten
	bool IsOpen() const { return bOpen; }
	bool IsRevealing() const { return bOpen && !bRevealed; }   // lines still to show (the buttons wait)
	void Advance(float DeltaSeconds);                       // the fade, the reveal and the confirm window (its tick; tests)
	void PressVeil();                                       // tests in place of clicks: every line at once, then the buttons
	void PressToMap();
	void PressNewRun();                                     // the first press changes the words; a second within ConfirmSeconds calls StartNewRun
	TArray<FText> GetPageTexts() const;                     // tests: the page's texts in order (a line's message without its stamp)
	TArray<FText> GetLineStamps() const;                    // tests: the stamps of the lines, in order
	TArray<float> GetLineOpacities() const;
	const UTextBlock* GetNoteText() const { return NoteText; }
	const UTextBlock* GetProvisionalText() const { return ProvisionalText; }
	UCosmicActionButton* GetToMapButton() const { return ToMapButton; }
	UCosmicActionButton* GetNewRunButton() const { return NewRunButton; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	// A press on the page stops here (PressVeil while the reveal runs): it is not a click on the map under it.
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void HandleAction(FName Action, float Value);
	// The page's rows again from the state.
	void BuildPage();
	// The veil's colour, the rows' opacities and the buttons for the time Elapsed since Open.
	void ShowProgress();
	// The cause line of an end (empty for a preview).
	FText NoteOf(ECosmicProvisionalEnd Cause) const;

	UPROPERTY(Transient) TObjectPtr<UBorder> Veil;
	UPROPERTY(Transient) TObjectPtr<UScrollBox> Page;
	UPROPERTY(Transient) TArray<TObjectPtr<UWidget>> LineRows;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> NoteText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ProvisionalText;
	UPROPERTY(Transient) TObjectPtr<UWidget> ButtonRow;
	UPROPERTY(Transient) TObjectPtr<UCosmicActionButton> ToMapButton;
	UPROPERTY(Transient) TObjectPtr<UCosmicActionButton> NewRunButton;

	// What the page shows, row by row (GetPageTexts), the stamps and opacities of its lines.
	TArray<FText> RowTexts;
	TArray<float> RowOpacities;
	TArray<FText> Stamps;
	TArray<float> LineOpacities;

	ECosmicProvisionalEnd ShownCause = ECosmicProvisionalEnd::None;
	bool bOpen = false;
	bool bRevealed = false;
	float Elapsed = 0.f;
	// Real seconds left for the second press of 'new run'; unset while none was half pressed.
	TOptional<float> ConfirmLeft;
};

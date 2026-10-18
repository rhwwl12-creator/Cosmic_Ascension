// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 3.2, 6.5, 8.4, 9.4, 11 Phase 3.
// The UI reads the simulation and listens to its delegates; the pause goes through the player controller.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/CosmicTypes.h"
#include "CosmicAscensionOverlayWidget.generated.h"

class UImage;
class UMaterialInstanceDynamic;
class USoundWaveProcedural;
class UTextBlock;
class UWidget;

namespace CosmicAscensionOverlay
{
	/** The length of the presentation, in seconds of real time. */
	inline constexpr float Seconds = 5.f;

	/** The length of the reveal that follows the third rung's presentation ("this is seen now"), in seconds of real time. */
	inline constexpr float RevealSeconds = 3.f;

	/** The glow's strength at Seconds into the presentation: 0 -> 1 over 0.8 s, 1 until 4.2 s, 1 -> 0 at 5 s, 0 after. */
	COSMIC_ASCENSION_API float GlowAt(float Seconds);
}

namespace CosmicAscensionSound
{
	/** The chime of an ascension as 16-bit mono PCM at SampleRate for Seconds: a soft C major triad that fades out. */
	COSMIC_ASCENSION_API TArray<int16> MakeChime(int32 SampleRate, float Seconds);
}

/**
 * The ascension presentation (the C++ stand-in for WBP_AscensionOverlay): over the whole screen a dim veil, a gold glow
 * in the centre (the UI material M_AscensionGlow), the feat's title, the new rung and its MythicNarrative, with a chime.
 * About five seconds of real time, then it closes; a click closes it at once. It answers UAscensionSubsystem::
 * OnTierAscended (the player's ascensions) and pauses a running simulation while it shows, giving the speed back when it
 * closes unless the player changed it meanwhile. When the presentation of the rung that opens the spiritual sight
 * (CosmicMapLook::SpiritualSightTier) closes and the view drawn is spiritual, a second phase shows ST_CA Sight.Opened
 * over the map - one plain white line, no veil, no glow, no chime, taking no click, for RevealSeconds (guideline 6.4:
 * the opening is received, so it stands apart from the feat's prize). NS_Ascension (Niagara) is deferred (guideline
 * 6.5). Built in C++.
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicAscensionOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Shows the ascension to NewTier through FeatRow (its title and narrative; an unknown row shows the rung alone). */
	void Show(EAscensionTier NewTier, FName FeatRow);
	/** Closes now and gives back the speed it paused (unless the player changed it); the third rung's presentation then
	 * turns into the reveal when the view drawn is spiritual. */
	void Close();
	/** Moves the presentation, or the reveal, on by DeltaSeconds of real time (its tick; tests call it). */
	void Advance(float DeltaSeconds);
	bool IsShowing() const { return bShowing; }
	/** Whether the reveal ("this is seen now") shows. */
	bool IsRevealing() const { return bRevealing; }
	/** The reveal's line (tests): ST_CA Sight.Opened while the reveal shows; collapsed and empty otherwise. */
	const UTextBlock* GetRevealText() const { return RevealText; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	// A click anywhere closes the presentation (and is not a click on the map).
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	UFUNCTION()
	void HandleTierAscended(EAscensionTier NewTier, FName FeatRow, EPlayerRole Path);

	void PlayChime();
	// The second phase after the third rung's presentation, and its end.
	void StartReveal();
	void EndReveal();

	// The veil, the glow and the texts of the presentation, hidden together while the reveal shows.
	UPROPERTY(Transient)
	TObjectPtr<UWidget> PresentationLayer;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RevealText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RungText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NarrativeText;

	UPROPERTY(Transient)
	TObjectPtr<UImage> Glow;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> GlowMaterial;

	UPROPERTY(Transient)
	TObjectPtr<USoundWaveProcedural> Chime;

	float Elapsed = 0.f;
	bool bShowing = false;
	// The speed the overlay paused; 0 when it found the simulation paused.
	float PausedSpeed = 0.f;
	// The rung the presentation shows (the reveal follows the third rung's).
	EAscensionTier PresentedTier = EAscensionTier::Tier1_Planetary;
	bool bRevealing = false;
	float RevealElapsed = 0.f;
};

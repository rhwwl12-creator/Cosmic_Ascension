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

namespace CosmicAscensionOverlay
{
	/** The length of the presentation, in seconds of real time. */
	inline constexpr float Seconds = 5.f;

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
 * closes unless the player changed it meanwhile. NS_Ascension (Niagara) is deferred (guideline 6.5). Built in C++.
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicAscensionOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Shows the ascension to NewTier through FeatRow (its title and narrative; an unknown row shows the rung alone). */
	void Show(EAscensionTier NewTier, FName FeatRow);
	/** Closes now and gives back the speed it paused (unless the player changed it). */
	void Close();
	/** Moves the presentation on by DeltaSeconds of real time (its tick; tests call it). */
	void Advance(float DeltaSeconds);
	bool IsShowing() const { return bShowing; }

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
};

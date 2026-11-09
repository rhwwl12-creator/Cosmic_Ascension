// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicAscensionOverlayWidget.h"

#include "Ascension/AscensionSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Cosmic_Ascension.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Layout/Margin.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Sound/SoundWaveProcedural.h"
#include "UI/CosmicPalette.h"
#include "UI/CosmicUILabels.h"
#include "UObject/UObjectGlobals.h"
#include "World/CosmicCameraPawn.h"
#include "World/CosmicGameMode.h"
#include "World/CosmicMapLook.h"
#include "World/CosmicZoomLevels.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicAscensionOverlayImpl
{
	const TCHAR* const GlowMaterialPath = TEXT("/Game/CA/Materials/M_AscensionGlow.M_AscensionGlow");
	const FName GlowParam(TEXT("Glow"));
	const FLinearColor VeilColor(0.f, 0.f, 0.02f, 0.6f);
	constexpr float FadeIn = 0.8f;
	constexpr float FadeOutFrom = 4.2f;
	// The glow grows from this scale to the next over the presentation.
	constexpr float GlowScaleFrom = 0.6f;
	constexpr float GlowScaleTo = 1.3f;
	constexpr float GlowSize = 900.f;
	// The glow's opacity at full strength: the text in front of it stays readable.
	constexpr float GlowPeak = 0.8f;

	// A soft dark shadow under the text over the glow.
	void AddShadow(UTextBlock& Text)
	{
		Text.SetShadowOffset(FVector2D(1.5f, 1.5f));
		Text.SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.9f));
	}
	constexpr int32 ChimeSampleRate = 44100;
	constexpr float ChimeSeconds = 3.f;
	// The reveal fades in and out over this much of its seconds.
	constexpr float RevealFadeSeconds = 0.4f;
}

float CosmicAscensionOverlay::GlowAt(float At)
{
	using namespace CosmicAscensionOverlayImpl;

	if (!(At > 0.f) || At >= CosmicAscensionOverlay::Seconds)
	{
		return 0.f;
	}
	if (At < FadeIn)
	{
		return At / FadeIn;
	}
	if (At <= FadeOutFrom)
	{
		return 1.f;
	}
	return (CosmicAscensionOverlay::Seconds - At) / (CosmicAscensionOverlay::Seconds - FadeOutFrom);
}

FCosmicAscensionLook CosmicAscensionOverlay::LookOf(EAscensionTier NewTier)
{
	FCosmicAscensionLook Look;
	if (NewTier == EAscensionTier::Tier5_EternalOrder)
	{
		// The top of the ladder: no light, no gold, no sound (B6b decision 26).
		Look.bGlow = false;
		Look.bChime = false;
		Look.TitleColor = CosmicPalette::White;
		return Look;
	}
	Look.bOpensSight = NewTier == CosmicMapLook::SpiritualSightTier;
	return Look;
}

TArray<int16> CosmicAscensionSound::MakeChime(int32 SampleRate, float Seconds)
{
	TArray<int16> Samples;
	const int32 Count = (SampleRate > 0 && Seconds > 0.f) ? FMath::FloorToInt32(SampleRate * Seconds) : 0;
	Samples.SetNumUninitialized(Count);
	// C5, E5, G5 with a short attack and an exponential decay; the amplitude leaves headroom.
	const float Notes[] = { 523.25f, 659.25f, 783.99f };
	constexpr float NoteCount = 3.f;
	constexpr float Amplitude = 0.25f;
	constexpr float AttackSeconds = 0.02f;
	constexpr float DecayPerSecond = 1.6f;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const float Time = static_cast<float>(Index) / SampleRate;
		const float Envelope = FMath::Min(Time / AttackSeconds, 1.f) * FMath::Exp(-DecayPerSecond * Time);
		float Value = 0.f;
		for (const float Note : Notes)
		{
			Value += FMath::Sin(UE_TWO_PI * Note * Time);
		}
		Samples[Index] = static_cast<int16>(FMath::Clamp(Amplitude * Envelope * Value / NoteCount, -1.f, 1.f) * 32767.f);
	}
	return Samples;
}

TSharedRef<SWidget> UCosmicAscensionOverlayWidget::RebuildWidget()
{
	using namespace CosmicAscensionOverlayImpl;

	// RebuildWidget can run again after the Slate widgets are released; the tree is built once.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>();
		WidgetTree->RootWidget = Root;

		// The presentation's layer: veil, glow and texts, hidden together while the reveal shows.
		UOverlay* Layer = WidgetTree->ConstructWidget<UOverlay>();
		Layer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		UOverlaySlot* LayerSlot = Root->AddChildToOverlay(Layer);
		LayerSlot->SetHorizontalAlignment(HAlign_Fill);
		LayerSlot->SetVerticalAlignment(VAlign_Fill);
		PresentationLayer = Layer;

		UBorder* Veil = WidgetTree->ConstructWidget<UBorder>();
		Veil->SetBrushColor(VeilColor);
		UOverlaySlot* VeilSlot = Layer->AddChildToOverlay(Veil);
		VeilSlot->SetHorizontalAlignment(HAlign_Fill);
		VeilSlot->SetVerticalAlignment(VAlign_Fill);

		USizeBox* GlowBox = WidgetTree->ConstructWidget<USizeBox>();
		GlowBox->SetWidthOverride(GlowSize);
		GlowBox->SetHeightOverride(GlowSize);
		Glow = WidgetTree->ConstructWidget<UImage>();
		GlowBox->AddChild(Glow);
		UOverlaySlot* GlowSlot = Layer->AddChildToOverlay(GlowBox);
		GlowSlot->SetHorizontalAlignment(HAlign_Center);
		GlowSlot->SetVerticalAlignment(VAlign_Center);

		UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
		UOverlaySlot* ColumnSlot = Layer->AddChildToOverlay(Column);
		ColumnSlot->SetHorizontalAlignment(HAlign_Center);
		ColumnSlot->SetVerticalAlignment(VAlign_Center);
		TitleText = CosmicUI::MakeText(*WidgetTree, 34, false);
		TitleText->SetColorAndOpacity(FSlateColor(CosmicPalette::Gold));
		TitleText->SetJustification(ETextJustify::Center);
		AddShadow(*TitleText);
		Column->AddChildToVerticalBox(TitleText)->SetHorizontalAlignment(HAlign_Center);
		RungText = CosmicUI::MakeText(*WidgetTree, 18, false);
		RungText->SetJustification(ETextJustify::Center);
		AddShadow(*RungText);
		Column->AddChildToVerticalBox(RungText)->SetHorizontalAlignment(HAlign_Center);
		USizeBox* NarrativeBox = WidgetTree->ConstructWidget<USizeBox>();
		NarrativeBox->SetMaxDesiredWidth(760.f);
		NarrativeText = CosmicUI::MakeText(*WidgetTree, 20);
		NarrativeText->SetJustification(ETextJustify::Center);
		AddShadow(*NarrativeText);
		NarrativeBox->AddChild(NarrativeText);
		UVerticalBoxSlot* NarrativeSlot = Column->AddChildToVerticalBox(NarrativeBox);
		NarrativeSlot->SetHorizontalAlignment(HAlign_Center);
		NarrativeSlot->SetPadding(FMargin(0.f, 18.f, 0.f, 0.f));

		// The reveal: one plain white line over the map, collapsed and empty until the third rung's presentation closes.
		RevealText = CosmicUI::MakeText(*WidgetTree, 24, false);
		RevealText->SetColorAndOpacity(FSlateColor(CosmicPalette::White));
		RevealText->SetJustification(ETextJustify::Center);
		AddShadow(*RevealText);
		RevealText->SetVisibility(ESlateVisibility::Collapsed);
		UOverlaySlot* RevealSlot = Root->AddChildToOverlay(RevealText);
		RevealSlot->SetHorizontalAlignment(HAlign_Center);
		RevealSlot->SetVerticalAlignment(VAlign_Center);

		SetVisibility(ESlateVisibility::Collapsed);
	}
	return Super::RebuildWidget();
}

void UCosmicAscensionOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// Loaded here, not by a constructor finder: the script refills the material in place (see ACosmicStarSystemActor).
	if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, CosmicAscensionOverlayImpl::GlowMaterialPath, nullptr, LOAD_NoWarn))
	{
		GlowMaterial = UMaterialInstanceDynamic::Create(Base, this);
		if (Glow)
		{
			Glow->SetBrushFromMaterial(GlowMaterial);
		}
	}
	else
	{
		UE_LOG(LogCosmic, Error, TEXT("%s is missing: run Tools/make_materials.py"), CosmicAscensionOverlayImpl::GlowMaterialPath);
	}
	if (UAscensionSubsystem* Ascension = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAscensionSubsystem>() : nullptr)
	{
		Ascension->OnTierAscended.AddUniqueDynamic(this, &UCosmicAscensionOverlayWidget::HandleTierAscended);
	}
}

void UCosmicAscensionOverlayWidget::NativeDestruct()
{
	if (UAscensionSubsystem* Ascension = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAscensionSubsystem>() : nullptr)
	{
		Ascension->OnTierAscended.RemoveDynamic(this, &UCosmicAscensionOverlayWidget::HandleTierAscended);
	}
	Super::NativeDestruct();
}

void UCosmicAscensionOverlayWidget::HandleTierAscended(EAscensionTier NewTier, FName FeatRow, EPlayerRole Path)
{
	// Inside the AI stage: the presentation reads the tables and pauses (SetTimeScale is allowed in a handler).
	Show(NewTier, FeatRow);
}

void UCosmicAscensionOverlayWidget::Show(EAscensionTier NewTier, FName FeatRow)
{
	// A presentation ends a reveal still showing.
	if (bRevealing)
	{
		EndReveal();
	}
	PresentedTier = NewTier;
	// Fully qualified: CosmicMapLook::LookOf exists too.
	const FCosmicAscensionLook Look = CosmicAscensionOverlay::LookOf(NewTier);
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	const FAscensionFeatDef* Feat = (Sim && !FeatRow.IsNone()) ? Sim->GetTables().FindFeat(FeatRow) : nullptr;
	if (TitleText)
	{
		TitleText->SetText(Feat ? Feat->Title : FText::GetEmpty());
		RungText->SetText(FText::Format(INVTEXT("{0} {1}"), CosmicUI::Label(TEXT("Panel.Tier")), FText::AsNumber(static_cast<int32>(NewTier) + 1)));
		NarrativeText->SetText(Feat ? Feat->MythicNarrative : FText::GetEmpty());
		// Set on every show: the fifth rung's white and missing glow do not stay for the next presentation.
		TitleText->SetColorAndOpacity(FSlateColor(Look.TitleColor));
	}
	if (Glow)
	{
		Glow->SetVisibility(Look.bGlow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// Showing before the pause: the pause redraws the HUD, and the top bar keeps x25 hidden while the presentation that
	// opens the spiritual sight shows.
	bShowing = true;
	Elapsed = 0.f;
	// Pause a running simulation (a second ascension while showing pauses again if the player resumed meanwhile).
	if (Sim && Sim->GetTimeScale() > 0.f)
	{
		PausedSpeed = Sim->GetTimeScale();
		if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
		{
			Controller->SetSpeed(0.f);
		}
	}
	SetVisibility(ESlateVisibility::Visible);
	Advance(0.f);
	if (Look.bChime)
	{
		PlayChime();
	}
	else if (Chime)
	{
		// The fifth rung is silent: a chime of an earlier presentation still sounding stops.
		Chime->ResetAudio();
	}
}

bool UCosmicAscensionOverlayWidget::IsShowingSightOpening() const
{
	// PresentedTier outlives the presentation (Close and FoldSilently leave it): bShowing first.
	return bShowing && CosmicAscensionOverlay::LookOf(PresentedTier).bOpensSight;
}

void UCosmicAscensionOverlayWidget::Close()
{
	if (!bShowing)
	{
		return;
	}
	bShowing = false;
	SetVisibility(ESlateVisibility::Collapsed);
	// Give the paused speed back, unless the player set a speed meanwhile (through the controller's SetSpeed: a x25 the
	// rung no longer opens comes back as x10). The HUD is redrawn either way - what waited for the presentation to close
	// (the third rung's x25) shows now, also when the simulation stays paused.
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer());
	if (PausedSpeed > 0.f && Sim && !(Sim->GetTimeScale() > 0.f) && Controller)
	{
		Controller->SetSpeed(PausedSpeed);
	}
	else if (ACosmicHUD* HUD = Controller ? Controller->GetHUD<ACosmicHUD>() : nullptr)
	{
		HUD->RefreshPanel();
	}
	PausedSpeed = 0.f;
	// The first reward of the third rung (guideline 6.4): after the presentation, never inside it, and only over a map
	// that shows what it says.
	if (CosmicAscensionOverlay::LookOf(PresentedTier).bOpensSight && ACosmicPlayerController::ShownViewModeFor(*this) == ECosmicViewMode::Spiritual)
	{
		// At the universe level the map shows no star the reveal speaks of: the camera comes back to the region first
		// (Phase 3 item 9), at once - a glide would keep the stars hidden for the reveal's first frames (the map hides the
		// region's detail while the arm's share is past half). The cut falls as the presentation goes.
		if (Controller && Controller->GetZoomLevel() == ECosmicZoomLevel::Universe)
		{
			Controller->FrameRegion();
			if (ACosmicCameraPawn* CameraPawn = Cast<ACosmicCameraPawn>(Controller->GetPawn()))
			{
				CameraPawn->SnapToTargets();
			}
			Controller->FollowZoomLevel();
		}
		StartReveal();
	}
}

void UCosmicAscensionOverlayWidget::FoldSilently()
{
	if (bRevealing)
	{
		EndReveal();
	}
	if (!bShowing)
	{
		return;
	}
	bShowing = false;
	PausedSpeed = 0.f;
	SetVisibility(ESlateVisibility::Collapsed);
	// The chime still sounding stops: the procedural wave plays only what is queued, and the queue empties.
	if (Chime)
	{
		Chime->ResetAudio();
	}
}

void UCosmicAscensionOverlayWidget::StartReveal()
{
	bRevealing = true;
	RevealElapsed = 0.f;
	if (PresentationLayer)
	{
		PresentationLayer->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (RevealText)
	{
		RevealText->SetText(CosmicUI::Label(TEXT("Sight.Opened")));
		RevealText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	// Over the map, taking no click: the map stays under the cursor.
	SetVisibility(ESlateVisibility::HitTestInvisible);
	Advance(0.f);
}

void UCosmicAscensionOverlayWidget::EndReveal()
{
	bRevealing = false;
	// A collapsed line carries no text.
	if (RevealText)
	{
		RevealText->SetText(FText::GetEmpty());
		RevealText->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (PresentationLayer)
	{
		PresentationLayer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCosmicAscensionOverlayWidget::Advance(float DeltaSeconds)
{
	using namespace CosmicAscensionOverlayImpl;

	if (bRevealing)
	{
		RevealElapsed += FMath::Max(DeltaSeconds, 0.f);
		if (RevealElapsed >= CosmicAscensionOverlay::RevealSeconds)
		{
			EndReveal();
			return;
		}
		// A short fade in and out.
		const float Fade = FMath::Min(RevealElapsed, CosmicAscensionOverlay::RevealSeconds - RevealElapsed) / RevealFadeSeconds;
		SetRenderOpacity(Fade < 1.f ? Fade : 1.f);
		return;
	}
	if (!bShowing)
	{
		return;
	}
	Elapsed += FMath::Max(DeltaSeconds, 0.f);
	if (Elapsed >= CosmicAscensionOverlay::Seconds)
	{
		Close();
		return;
	}
	const float Strength = CosmicAscensionOverlay::GlowAt(Elapsed);
	SetRenderOpacity(FMath::Min(1.f, Strength * 1.5f));
	if (GlowMaterial)
	{
		GlowMaterial->SetScalarParameterValue(GlowParam, Strength * GlowPeak);
	}
	if (Glow)
	{
		Glow->SetRenderScale(FVector2D(FMath::Lerp(GlowScaleFrom, GlowScaleTo, Elapsed / CosmicAscensionOverlay::Seconds)));
	}
}

void UCosmicAscensionOverlayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Advance(InDeltaTime);
}

FReply UCosmicAscensionOverlayWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Close();
	return FReply::Handled();
}

void UCosmicAscensionOverlayWidget::PlayChime()
{
	using namespace CosmicAscensionOverlayImpl;

	if (!Chime)
	{
		Chime = NewObject<USoundWaveProcedural>(this);
		Chime->SetSampleRate(ChimeSampleRate);
		Chime->NumChannels = 1;
		Chime->Duration = ChimeSeconds;
		Chime->bLooping = false;
	}
	const TArray<int16> Samples = CosmicAscensionSound::MakeChime(ChimeSampleRate, ChimeSeconds);
	Chime->ResetAudio();
	Chime->QueueAudio(reinterpret_cast<const uint8*>(Samples.GetData()), Samples.Num() * sizeof(int16));
	UGameplayStatics::PlaySound2D(this, Chime);
	++ChimeCount;
}

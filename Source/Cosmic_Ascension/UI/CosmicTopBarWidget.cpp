// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicTopBarWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateNoResource.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Core/GalaxyState.h"
#include "Engine/GameInstance.h"
#include "Layout/Clipping.h"
#include "Layout/Margin.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicAscensionOverlayWidget.h"
#include "UI/CosmicChronicleWidget.h"
#include "UI/CosmicNextRung.h"
#include "UI/CosmicUILabels.h"
#include "UObject/UnrealType.h"
#include "World/CosmicAutoPause.h"
#include "World/CosmicGameMode.h"
#include "World/CosmicMapLook.h"

// x25 is a time scale the simulation takes: a higher step would set the cap and show as the current speed.
static_assert(CosmicUI::ThirdRungTopSpeed <= UGalaxySimulationSubsystem::MaxTimeScale, "x25 is a time scale the simulation takes");

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicTopBarImpl
{
	const FName SpeedAction(TEXT("speed"));
	const FName ViewAction(TEXT("view"));
	const FName AutoPauseAction(TEXT("autopause"));
	const FName ReopenAction(TEXT("reopen"));
	// The provisional end's grey line beside the clock (MVP item 4): quieter than the line of a pause.
	const FLinearColor EndSoonColor(0.6f, 0.6f, 0.65f);
	// The line of an automatic pause: the warm colour of a flat button under the mouse, so the eye finds why time stopped.
	const FLinearColor WhyColor(1.f, 0.82f, 0.4f);
	const FLinearColor BandColor(0.01f, 0.01f, 0.03f, 0.8f);
	// The text of a flat button (the clock, the rung), and its colour while the mouse is on it (a press or a tooltip waits there).
	const FLinearColor FlatColor = FLinearColor::White;
	const FLinearColor FlatHoverColor(1.f, 0.82f, 0.4f);

	// A flat button around Content (no brush in any state, no padding shift when pressed) that never takes the focus.
	UButton* MakeFlatButton(UWidgetTree& Tree, UTextBlock& Content)
	{
		UButton* Button = Tree.ConstructWidget<UButton>();
		// Before the Slate button exists: a focusable button keeps the keyboard focus after a click and Space presses it.
		// UButton keeps its initializer of IsFocusable protected (5.8), so the property is set through reflection.
		if (FBoolProperty* Focusable = FindFProperty<FBoolProperty>(UButton::StaticClass(), TEXT("IsFocusable")))
		{
			Focusable->SetPropertyValue_InContainer(Button, false);
		}
		FButtonStyle Flat = Button->GetStyle();
		const FSlateNoResource NoBrush;
		Flat.SetNormal(NoBrush).SetHovered(NoBrush).SetPressed(NoBrush).SetDisabled(NoBrush);
		Flat.SetNormalPadding(FMargin(0.f)).SetPressedPadding(FMargin(0.f));
		Button->SetStyle(Flat);
		Content.SetColorAndOpacity(FSlateColor(FlatColor));
		Button->AddChild(&Content);
		return Button;
	}

	// The third rung's gate as this bar sees it: its controller's (the development force of CosmicSpiritualSight included),
	// the viewer tier's alone for a bar without one.
	bool ThirdRungOpen(const UUserWidget& Bar, const UGalaxySimulationSubsystem& Sim)
	{
		const ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(Bar.GetOwningPlayer());
		return Controller ? Controller->IsSpiritualSightOpen() : CosmicMapLook::SpiritualSightOpens(Sim.GetViewerTier());
	}

	// Whether the HUD's ascension presentation shows now: x25 waits for it to close (guideline 6.3 - after the
	// presentation, at the same moment as the sight's reveal).
	bool Presenting(const UUserWidget& Bar)
	{
		const APlayerController* Owner = Bar.GetOwningPlayer();
		const ACosmicHUD* HUD = Owner ? Owner->GetHUD<ACosmicHUD>() : nullptr;
		const UCosmicAscensionOverlayWidget* Overlay = HUD ? HUD->GetAscensionOverlay() : nullptr;
		return Overlay && Overlay->IsShowing();
	}
}

TSharedRef<SWidget> UCosmicTopBarWidget::RebuildWidget()
{
	using namespace CosmicTopBarImpl;

	// RebuildWidget can run again after the Slate widgets are released; the tree is built once.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UBorder* Band = WidgetTree->ConstructWidget<UBorder>();
		Band->SetBrushColor(BandColor);
		Band->SetPadding(FMargin(16.f, 6.f));
		WidgetTree->RootWidget = Band;
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
		Band->SetContent(Row);

		// The status in three blocks: the name and rung (a flat button that lights on hover), stability and resources, and
		// the research shares. The rung and the research carry the next-rung tooltip, bound before their Slate widgets are
		// built (the first TakeWidget synchronises the properties, and the binding is read there): one Slate tooltip each
		// that pulls its words only while the mouse is on it. The research block fills the band's free width but is drawn
		// at its own width (left-aligned), so its tooltip covers the shares, not the empty band.
		RungText = CosmicUI::MakeText(*WidgetTree, 14, false);
		RungText->ToolTipTextDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UCosmicTopBarWidget, GetNextRungTip));
		RungButton = MakeFlatButton(*WidgetTree, *RungText);
		RungButton->OnHovered.AddUniqueDynamic(this, &UCosmicTopBarWidget::HandleRungHovered);
		RungButton->OnUnhovered.AddUniqueDynamic(this, &UCosmicTopBarWidget::HandleRungUnhovered);
		UHorizontalBoxSlot* RungSlot = Row->AddChildToHorizontalBox(RungButton);
		RungSlot->SetVerticalAlignment(VAlign_Center);
		RungSlot->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));

		StatusText = CosmicUI::MakeText(*WidgetTree, 14, false);
		UHorizontalBoxSlot* StatusSlot = Row->AddChildToHorizontalBox(StatusText);
		StatusSlot->SetVerticalAlignment(VAlign_Center);
		StatusSlot->SetPadding(FMargin(0.f, 0.f, 32.f, 0.f));

		ResearchText = CosmicUI::MakeText(*WidgetTree, 14, false);
		ResearchText->ToolTipTextDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UCosmicTopBarWidget, GetNextRungTip));
		// The one block that yields when the band runs short (the third rung in years, the mouse on the clock with both
		// units): clipped to its slot and ended with an ellipsis, never drawn over the clock beside it.
		ResearchText->SetClipping(EWidgetClipping::ClipToBounds);
		ResearchText->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
		UHorizontalBoxSlot* ResearchSlot = Row->AddChildToHorizontalBox(ResearchText);
		ResearchSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		ResearchSlot->SetHorizontalAlignment(HAlign_Left);
		ResearchSlot->SetVerticalAlignment(VAlign_Center);

		// The clock: a flat button around the text.
		ClockText = CosmicUI::MakeText(*WidgetTree, 14, false);
		ClockButton = MakeFlatButton(*WidgetTree, *ClockText);
		ClockButton->OnClicked.AddUniqueDynamic(this, &UCosmicTopBarWidget::HandleClockClicked);
		ClockButton->OnHovered.AddUniqueDynamic(this, &UCosmicTopBarWidget::HandleClockHovered);
		ClockButton->OnUnhovered.AddUniqueDynamic(this, &UCosmicTopBarWidget::HandleClockUnhovered);
		// Set once: SetToolTipText builds a new Slate tooltip each time. What a year is, then what a press does (a flat
		// button does not look pressable).
		// A third line states the provisional end as a rule of the run (MVP item 4): the tooltip is built once.
		{
			const ACosmicPlayerController* Owner = Cast<ACosmicPlayerController>(GetOwningPlayer());
			const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
			ClockButton->SetToolTipText(ClockTipOf(Sim ? Sim->GetRules().ProvisionalEndTick : 0, Sim && (!Owner || Owner->IsProvisionalEndEnabled())));
		}
		UHorizontalBoxSlot* ClockSlot = Row->AddChildToHorizontalBox(ClockButton);
		ClockSlot->SetVerticalAlignment(VAlign_Center);
		ClockSlot->SetPadding(FMargin(16.f, 0.f, 4.f, 0.f));
		EndSoonText = CosmicUI::MakeText(*WidgetTree, 12, false);
		EndSoonText->SetColorAndOpacity(FSlateColor(EndSoonColor));
		EndSoonText->SetVisibility(ESlateVisibility::Collapsed);
		UHorizontalBoxSlot* EndSoonSlot = Row->AddChildToHorizontalBox(EndSoonText);
		EndSoonSlot->SetVerticalAlignment(VAlign_Center);
		EndSoonSlot->SetPadding(FMargin(4.f, 0.f, 12.f, 0.f));

		// The view: a bordered button between the clock and the speeds (its slot never moves when the clock's hover text
		// grows - the research block takes that), outside SpeedButtons. Its tooltip is set once; Refresh shows it from the
		// third rung and labels it with the view drawn.
		ViewButton = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		ViewButton->Setup(ViewAction, 0.f, CosmicUI::Label(TEXT("Sight.Spiritual")));
		ViewButton->OnAction.AddUObject(this, &UCosmicTopBarWidget::HandleView);
		ViewButton->SetToolTipText(CosmicUI::Label(TEXT("Sight.Tip")));
		ViewButton->SetVisibility(ESlateVisibility::Collapsed);
		UHorizontalBoxSlot* ViewSlot = Row->AddChildToHorizontalBox(ViewButton);
		ViewSlot->SetVerticalAlignment(VAlign_Center);
		ViewSlot->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));

		// One button per speed step, labelled in the cycle form here (Refresh relabels them in the controller's unit). A
		// step above the calling's top starts collapsed and takes its tooltip once: what the third rung's speed does to the
		// crises.
		for (const float Speed : CosmicUI::SpeedSteps)
		{
			UCosmicActionButton* Button = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
			Button->Setup(SpeedAction, Speed, CosmicUI::SpeedLabel(Speed, ECosmicTimeUnit::Cycle));
			Button->OnAction.AddUObject(this, &UCosmicTopBarWidget::HandleSpeed);
			if (Speed > CosmicUI::CallingTopSpeed)
			{
				Button->SetToolTipText(CosmicUI::Label(TEXT("Speed.ThirdRungTip")));
				Button->SetVisibility(ESlateVisibility::Collapsed);
			}
			UHorizontalBoxSlot* ButtonSlot = Row->AddChildToHorizontalBox(Button);
			ButtonSlot->SetVerticalAlignment(VAlign_Center);
			ButtonSlot->SetPadding(FMargin(2.f, 0.f));
			SpeedButtons.Add(Button);
		}
		LabelledUnit = ECosmicTimeUnit::Cycle;

		// The chronicle again (MVP item 4): after the speeds, collapsed until the run is over and its page folded.
		ReopenButton = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		ReopenButton->Setup(ReopenAction, 0.f, CosmicUI::Label(TEXT("Chronicle.Reopen")));
		ReopenButton->OnAction.AddUObject(this, &UCosmicTopBarWidget::HandleReopen);
		ReopenButton->SetVisibility(ESlateVisibility::Collapsed);
		UHorizontalBoxSlot* ReopenSlot = Row->AddChildToHorizontalBox(ReopenButton);
		ReopenSlot->SetVerticalAlignment(VAlign_Center);
		ReopenSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 0.f));

		// The automatic pause after the speeds: its button (the tooltip set once - SetToolTipText builds a new Slate tooltip
		// each time; Refresh labels it on or off), then the line of why it stopped, collapsed until it holds the time.
		AutoPauseButton = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		AutoPauseButton->Setup(AutoPauseAction, 0.f, CosmicUI::Label(TEXT("AutoPause.On")));
		AutoPauseButton->OnAction.AddUObject(this, &UCosmicTopBarWidget::HandleAutoPause);
		AutoPauseButton->SetToolTipText(CosmicUI::Label(TEXT("AutoPause.Tip")));
		UHorizontalBoxSlot* AutoPauseSlot = Row->AddChildToHorizontalBox(AutoPauseButton);
		AutoPauseSlot->SetVerticalAlignment(VAlign_Center);
		AutoPauseSlot->SetPadding(FMargin(12.f, 0.f, 0.f, 0.f));
		AutoPauseText = CosmicUI::MakeText(*WidgetTree, 13, false);
		AutoPauseText->SetColorAndOpacity(FSlateColor(WhyColor));
		AutoPauseText->SetVisibility(ESlateVisibility::Collapsed);
		UHorizontalBoxSlot* WhySlot = Row->AddChildToHorizontalBox(AutoPauseText);
		WhySlot->SetVerticalAlignment(VAlign_Center);
		WhySlot->SetPadding(FMargin(8.f, 0.f, 0.f, 0.f));
	}
	return Super::RebuildWidget();
}

void UCosmicTopBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr)
	{
		Sim->OnSimTick.AddUniqueDynamic(this, &UCosmicTopBarWidget::HandleSimTick);
		Sim->OnGalaxyGenerated.AddUniqueDynamic(this, &UCosmicTopBarWidget::HandleGalaxyGenerated);
	}
	Refresh();
}

void UCosmicTopBarWidget::NativeDestruct()
{
	if (UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr)
	{
		Sim->OnSimTick.RemoveDynamic(this, &UCosmicTopBarWidget::HandleSimTick);
		Sim->OnGalaxyGenerated.RemoveDynamic(this, &UCosmicTopBarWidget::HandleGalaxyGenerated);
	}
	Super::NativeDestruct();
}

void UCosmicTopBarWidget::HandleSimTick(int64 Tick)
{
	Refresh();
}

void UCosmicTopBarWidget::HandleGalaxyGenerated(int32 Seed)
{
	Refresh();
}

UCosmicActionButton* UCosmicTopBarWidget::FindSpeedButton(float Value) const
{
	for (UCosmicActionButton* Button : SpeedButtons)
	{
		if (Button && FMath::IsNearlyEqual(Button->GetValue(), Value))
		{
			return Button;
		}
	}
	return nullptr;
}

void UCosmicTopBarWidget::HandleSpeed(FName Action, float Value)
{
	// Through the controller, which keeps the speed to return to after a pause (the keys share it) and gives a speed the
	// gate does not open as x10.
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->SetSpeed(Value);
	}
	else if (UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr)
	{
		// A bar without a controller keeps the same rule.
		Sim->SetTimeScale(CosmicUI::OpenSpeed(Value, CosmicTopBarImpl::ThirdRungOpen(*this, *Sim)));
	}
	Refresh();
}

FText UCosmicTopBarWidget::ClockTipOf(int32 EndTick, bool bEndOn)
{
	const FText Tip = FText::Format(INVTEXT("{0}\n{1}"), CosmicUI::Label(TEXT("Time.Tip")), CosmicUI::Label(TEXT("Time.TipPress")));
	if (!bEndOn)
	{
		return Tip;
	}
	// Always in cycles: the rule is the run's, not the viewer's unit.
	const FText Rule = EndTick > 0 ? FText::Format(CosmicUI::Label(TEXT("Chronicle.Rule")), CosmicUI::ClockLabel(EndTick, ECosmicTimeUnit::Cycle))
		: CosmicUI::Label(TEXT("Chronicle.RuleRung"));
	return FText::Format(INVTEXT("{0}\n{1}"), Tip, Rule);
}

void UCosmicTopBarWidget::HandleReopen(FName Action, float Value)
{
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->ReopenChronicle();
	}
	Refresh();
}

void UCosmicTopBarWidget::HandleView(FName Action, float Value)
{
	PressViewButton();
}

void UCosmicTopBarWidget::PressViewButton()
{
	// The controller refuses while the sight is closed, hands the view to the map and announces it (the HUD redraws its
	// panels); this bar may not be the HUD's (tests).
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->ToggleViewMode();
	}
	Refresh();
}

void UCosmicTopBarWidget::HandleAutoPause(FName Action, float Value)
{
	PressAutoPause();
}

void UCosmicTopBarWidget::PressAutoPause()
{
	// The controller keeps the choice for the next session and redraws the HUD; this bar may not be the HUD's (tests).
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->ToggleAutoPause();
	}
	Refresh();
}

void UCosmicTopBarWidget::HandleClockClicked()
{
	PressClock();
}

void UCosmicTopBarWidget::HandleClockHovered()
{
	SetClockHovered(true);
}

void UCosmicTopBarWidget::HandleClockUnhovered()
{
	SetClockHovered(false);
}

void UCosmicTopBarWidget::HandleRungHovered()
{
	SetRungHovered(true);
}

void UCosmicTopBarWidget::HandleRungUnhovered()
{
	SetRungHovered(false);
}

void UCosmicTopBarWidget::PressClock()
{
	// The controller announces the change and the HUD redraws its panels; this bar may not be the HUD's (tests).
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->ToggleTimeUnit();
	}
	Refresh();
}

void UCosmicTopBarWidget::SetClockHovered(bool bHovered)
{
	bClockHovered = bHovered;
	if (ClockText)
	{
		ClockText->SetColorAndOpacity(FSlateColor(bHovered ? CosmicTopBarImpl::FlatHoverColor : CosmicTopBarImpl::FlatColor));
	}
	Refresh();
}

void UCosmicTopBarWidget::SetRungHovered(bool bHovered)
{
	// Only the colour: the words are the tooltip's, pulled while the mouse stays. The gold says a tooltip waits there, so
	// the rung lights only while there is a next rung to tell (none on the third rung of the slice, the fifth, for a
	// wanderer or in the interregnum).
	if (RungText)
	{
		const bool bLit = bHovered && !GetNextRungTip().IsEmpty();
		RungText->SetColorAndOpacity(FSlateColor(bLit ? CosmicTopBarImpl::FlatHoverColor : CosmicTopBarImpl::FlatColor));
	}
}

FText UCosmicTopBarWidget::GetNextRungTip() const
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim)
	{
		return FText::GetEmpty();
	}
	return CosmicNextRung::TextOfAll(CosmicNextRung::AllOf(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules()), Sim->GetTables());
}

void UCosmicTopBarWidget::Refresh()
{
	using namespace CosmicUI;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || !StatusText || !RungText || !ResearchText)
	{
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	const FName RuledCivID = State.Player.RuledCivID;
	const FCivilizationState* Civ = RuledCivID.IsNone() ? nullptr
		: State.Civs.FindByPredicate([RuledCivID](const FCivilizationState& Candidate) { return Candidate.CivID == RuledCivID; });
	if (Civ)
	{
		RungText->SetText(FText::Format(INVTEXT("{0}   {1} {2}"), Civ->CivName, Label(TEXT("Panel.Tier")), FText::AsNumber(static_cast<int32>(Civ->Tier) + 1)));
		StatusText->SetText(FText::Format(INVTEXT("{0} {1}        {2} {3}  {4} {5}  {6} {7}  {8} {9}  {10} {11}"),
				Label(TEXT("Panel.Stability")), Percent(Civ->Stability),
				Label(TEXT("Resource.Energy")), WholeNumber(Civ->Stockpile.Energy),
				Label(TEXT("Resource.Matter")), WholeNumber(Civ->Stockpile.Matter),
				Label(TEXT("Resource.Insight")), WholeNumber(Civ->Stockpile.Insight),
				Label(TEXT("Resource.Influence")), WholeNumber(Civ->Stockpile.Influence),
				Label(TEXT("Resource.Causality")), WholeNumber(Civ->Stockpile.Causality)));
		ResearchText->SetText(FText::Format(INVTEXT("{0} {1}  {2} {3}  {4} {5}  {6} {7}"),
				Label(TEXT("Research.Physics")), Percent(Civ->Research.FindRef(FName(TEXT("Physics")))),
				Label(TEXT("Research.Society")), Percent(Civ->Research.FindRef(FName(TEXT("Society")))),
				Label(TEXT("Research.Engineering")), Percent(Civ->Research.FindRef(FName(TEXT("Engineering")))),
				Label(TEXT("Research.Transcendence")), Percent(Civ->Research.FindRef(FName(TEXT("Transcendence"))))));
	}
	else
	{
		RungText->SetText(FText::GetEmpty());
		StatusText->SetText(FText::GetEmpty());
		ResearchText->SetText(FText::GetEmpty());
	}
	// The viewer's unit; the mouse on the clock shows both.
	const ECosmicTimeUnit Unit = ACosmicPlayerController::TimeUnitFor(*this);
	if (ClockText)
	{
		ClockText->SetText(bClockHovered ? ClockBothLabel(State.Tick) : ClockLabel(State.Tick, Unit));
	}

	// The view button: shown from the third rung (collapsed, not greyed, below it - its appearing is the opening), labelled
	// with the view drawn even while hidden. Both set only when they change.
	if (ViewButton)
	{
		const ESlateVisibility ViewVisibility = ACosmicPlayerController::IsSpiritualSightOpenFor(*this) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
		if (ViewButton->GetVisibility() != ViewVisibility)
		{
			ViewButton->SetVisibility(ViewVisibility);
		}
		const FText ViewLabel = Label(ACosmicPlayerController::ShownViewModeFor(*this) == ECosmicViewMode::Spiritual ? TEXT("Sight.Spiritual") : TEXT("Sight.Physical"));
		if (!ViewButton->GetLabel().ToString().Equals(ViewLabel.ToString(), ESearchCase::CaseSensitive))
		{
			ViewButton->Setup(CosmicTopBarImpl::ViewAction, 0.f, ViewLabel);
		}
	}

	// The speed buttons: a step the gate does not open is collapsed (x25 also while the ascension presentation shows - it
	// appears when the presentation closes), the current speed's button is greyed out (a click on it would change
	// nothing), and every label, the collapsed ones too, follows the unit. Visibility is set only when it changes.
	const bool bThirdRungShown = CosmicTopBarImpl::ThirdRungOpen(*this, *Sim) && !CosmicTopBarImpl::Presenting(*this);
	// The run is over (MVP item 4): every speed button is greyed out (x25 stays where the gate shows it).
	const bool bEnded = ACosmicPlayerController::IsRunEndedFor(*this);
	const bool bRelabel = !LabelledUnit.IsSet() || LabelledUnit.GetValue() != Unit;
	const float Current = Sim->GetTimeScale();
	for (UCosmicActionButton* Button : SpeedButtons)
	{
		if (Button)
		{
			if (bRelabel)
			{
				Button->Setup(Button->GetAction(), Button->GetValue(), SpeedLabel(Button->GetValue(), Unit));
			}
			const ESlateVisibility Shown = IsSpeedOpen(Button->GetValue(), bThirdRungShown) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
			if (Button->GetVisibility() != Shown)
			{
				Button->SetVisibility(Shown);
			}
			Button->SetIsEnabled(!bEnded && !FMath::IsNearlyEqual(Button->GetValue(), Current));
		}
	}
	LabelledUnit = Unit;

	// The automatic pause: its button says on or off; the line says why the time stands still while an automatic pause
	// holds it (the reason goes when the speed is set again). Both set only when they change.
	const ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer());

	// The provisional end (MVP item 4): the chronicle's button while the run is over and the page folded, and before the end
	// the grey line - the last minutes of the clock, or the clock end once the vessel was deposed. Set only when they change.
	if (ReopenButton)
	{
		const ACosmicHUD* HUD = Controller ? Controller->GetHUD<ACosmicHUD>() : nullptr;
		const UCosmicChronicleWidget* Page = HUD ? HUD->GetChronicle() : nullptr;
		const ESlateVisibility ReopenVisibility = (bEnded && Page && !Page->IsOpen()) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
		if (ReopenButton->GetVisibility() != ReopenVisibility)
		{
			ReopenButton->SetVisibility(ReopenVisibility);
		}
	}
	if (EndSoonText)
	{
		FText Soon;
		const int32 EndTick = Sim->GetRules().ProvisionalEndTick;
		if (!bEnded && Controller && Controller->IsProvisionalEndEnabled() && EndTick > 0)
		{
			const int64 Left = static_cast<int64>(EndTick) - State.Tick;
			const TArray<FLifeRecord>& Lives = State.Calling.Lives;
			const bool bDeposed = !Lives.IsEmpty() && !Lives.Last().ReignEndReasons.IsEmpty() && Lives.Last().ReignEndReasons.Last().IsValid();
			if (Left > 0 && Left <= CosmicChronicleScreen::EndSoonTicks)
			{
				Soon = FText::Format(Label(TEXT("Chronicle.Soon")), TimeLeftLabel(Left, Unit));
			}
			else if (Left > 0 && bDeposed)
			{
				Soon = FText::Format(Label(TEXT("Chronicle.Until")), ClockLabel(EndTick, Unit));
			}
		}
		if (!EndSoonText->GetText().ToString().Equals(Soon.ToString(), ESearchCase::CaseSensitive))
		{
			EndSoonText->SetText(Soon);
		}
		const ESlateVisibility SoonVisibility = Soon.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible;
		if (EndSoonText->GetVisibility() != SoonVisibility)
		{
			EndSoonText->SetVisibility(SoonVisibility);
		}
	}
	if (AutoPauseButton)
	{
		const FText PauseLabel = Label((!Controller || Controller->IsAutoPauseOn()) ? TEXT("AutoPause.On") : TEXT("AutoPause.Off"));
		if (!AutoPauseButton->GetLabel().ToString().Equals(PauseLabel.ToString(), ESearchCase::CaseSensitive))
		{
			AutoPauseButton->Setup(CosmicTopBarImpl::AutoPauseAction, 0.f, PauseLabel);
		}
	}
	if (AutoPauseText)
	{
		const TCHAR* WhyKey = (Controller && !(Current > 0.f)) ? CosmicAutoPause::WhyKeyOf(Controller->GetAutoPauseReason()) : nullptr;
		const FText Why = WhyKey ? Label(WhyKey) : FText::GetEmpty();
		if (!AutoPauseText->GetText().ToString().Equals(Why.ToString(), ESearchCase::CaseSensitive))
		{
			AutoPauseText->SetText(Why);
		}
		const ESlateVisibility WhyVisibility = Why.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible;
		if (AutoPauseText->GetVisibility() != WhyVisibility)
		{
			AutoPauseText->SetVisibility(WhyVisibility);
		}
	}
}

FReply UCosmicTopBarWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Only a hit-testable part of the panel reaches here (the widget and its root are self-hit-test-invisible).
	return FReply::Handled();
}

FReply UCosmicTopBarWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

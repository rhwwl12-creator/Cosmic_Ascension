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
#include "Layout/Margin.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicUILabels.h"
#include "UObject/UnrealType.h"
#include "World/CosmicGameMode.h"
#include "World/CosmicMapLook.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicTopBarImpl
{
	const FName SpeedAction(TEXT("speed"));
	const FName ViewAction(TEXT("view"));
	// Pause first; the keys 0 to 4 set the same speeds.
	constexpr float Speeds[] = { 0.f, 1.f, 2.f, 5.f, 10.f };
	const FLinearColor BandColor(0.01f, 0.01f, 0.03f, 0.8f);
	// The clock's text, and its colour while the mouse is on it (a press does something there).
	const FLinearColor ClockColor = FLinearColor::White;
	const FLinearColor ClockHoverColor(1.f, 0.82f, 0.4f);
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

		StatusText = CosmicUI::MakeText(*WidgetTree, 14, false);
		UHorizontalBoxSlot* StatusSlot = Row->AddChildToHorizontalBox(StatusText);
		StatusSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		StatusSlot->SetVerticalAlignment(VAlign_Center);

		// The clock: a flat button around the text (no brush in any state, no padding shift when pressed).
		ClockButton = WidgetTree->ConstructWidget<UButton>();
		// Before the Slate button exists: a focusable button keeps the keyboard focus after a click and Space presses it.
		// UButton keeps its initializer of IsFocusable protected (5.8), so the property is set through reflection.
		if (FBoolProperty* Focusable = FindFProperty<FBoolProperty>(UButton::StaticClass(), TEXT("IsFocusable")))
		{
			Focusable->SetPropertyValue_InContainer(ClockButton, false);
		}
		FButtonStyle Flat = ClockButton->GetStyle();
		const FSlateNoResource NoBrush;
		Flat.SetNormal(NoBrush).SetHovered(NoBrush).SetPressed(NoBrush).SetDisabled(NoBrush);
		Flat.SetNormalPadding(FMargin(0.f)).SetPressedPadding(FMargin(0.f));
		ClockButton->SetStyle(Flat);
		ClockButton->OnClicked.AddUniqueDynamic(this, &UCosmicTopBarWidget::HandleClockClicked);
		ClockButton->OnHovered.AddUniqueDynamic(this, &UCosmicTopBarWidget::HandleClockHovered);
		ClockButton->OnUnhovered.AddUniqueDynamic(this, &UCosmicTopBarWidget::HandleClockUnhovered);
		// Set once: SetToolTipText builds a new Slate tooltip each time. What a year is, then what a press does (a flat
		// button does not look pressable).
		ClockButton->SetToolTipText(FText::Format(INVTEXT("{0}\n{1}"), CosmicUI::Label(TEXT("Time.Tip")), CosmicUI::Label(TEXT("Time.TipPress"))));
		ClockText = CosmicUI::MakeText(*WidgetTree, 14, false);
		ClockText->SetColorAndOpacity(FSlateColor(ClockColor));
		ClockButton->AddChild(ClockText);
		UHorizontalBoxSlot* ClockSlot = Row->AddChildToHorizontalBox(ClockButton);
		ClockSlot->SetVerticalAlignment(VAlign_Center);
		ClockSlot->SetPadding(FMargin(16.f, 0.f, 12.f, 0.f));

		// The view: a bordered button between the clock and the speeds (its slot never moves when the clock's hover text
		// grows - the status text takes that), outside SpeedButtons. Its tooltip is set once; Refresh shows it from the
		// third rung and labels it with the view drawn.
		ViewButton = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		ViewButton->Setup(ViewAction, 0.f, CosmicUI::Label(TEXT("Sight.Spiritual")));
		ViewButton->OnAction.AddUObject(this, &UCosmicTopBarWidget::HandleView);
		ViewButton->SetToolTipText(CosmicUI::Label(TEXT("Sight.Tip")));
		ViewButton->SetVisibility(ESlateVisibility::Collapsed);
		UHorizontalBoxSlot* ViewSlot = Row->AddChildToHorizontalBox(ViewButton);
		ViewSlot->SetVerticalAlignment(VAlign_Center);
		ViewSlot->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));

		// Labelled in the cycle form here; Refresh relabels them in the controller's unit.
		for (const float Speed : Speeds)
		{
			UCosmicActionButton* Button = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
			Button->Setup(SpeedAction, Speed, CosmicUI::SpeedLabel(Speed, ECosmicTimeUnit::Cycle));
			Button->OnAction.AddUObject(this, &UCosmicTopBarWidget::HandleSpeed);
			UHorizontalBoxSlot* ButtonSlot = Row->AddChildToHorizontalBox(Button);
			ButtonSlot->SetVerticalAlignment(VAlign_Center);
			ButtonSlot->SetPadding(FMargin(2.f, 0.f));
			SpeedButtons.Add(Button);
		}
		LabelledUnit = ECosmicTimeUnit::Cycle;
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

void UCosmicTopBarWidget::HandleSpeed(FName Action, float Value)
{
	// Through the controller, which keeps the speed to return to after a pause (the keys share it).
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->SetSpeed(Value);
	}
	else if (UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr)
	{
		Sim->SetTimeScale(Value);
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
		ClockText->SetColorAndOpacity(FSlateColor(bHovered ? CosmicTopBarImpl::ClockHoverColor : CosmicTopBarImpl::ClockColor));
	}
	Refresh();
}

void UCosmicTopBarWidget::Refresh()
{
	using namespace CosmicUI;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || !StatusText)
	{
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	const FName RuledCivID = State.Player.RuledCivID;
	const FCivilizationState* Civ = RuledCivID.IsNone() ? nullptr
		: State.Civs.FindByPredicate([RuledCivID](const FCivilizationState& Candidate) { return Candidate.CivID == RuledCivID; });
	if (Civ)
	{
		StatusText->SetText(FText::Format(INVTEXT("{0}   {1} {2}   {3} {4}        {5} {6}  {7} {8}  {9} {10}  {11} {12}  {13} {14}        {15} {16}  {17} {18}  {19} {20}  {21} {22}"),
				Civ->CivName,
				Label(TEXT("Panel.Tier")), FText::AsNumber(static_cast<int32>(Civ->Tier) + 1),
				Label(TEXT("Panel.Stability")), Percent(Civ->Stability),
				Label(TEXT("Resource.Energy")), WholeNumber(Civ->Stockpile.Energy),
				Label(TEXT("Resource.Matter")), WholeNumber(Civ->Stockpile.Matter),
				Label(TEXT("Resource.Insight")), WholeNumber(Civ->Stockpile.Insight),
				Label(TEXT("Resource.Influence")), WholeNumber(Civ->Stockpile.Influence),
				Label(TEXT("Resource.Causality")), WholeNumber(Civ->Stockpile.Causality),
				Label(TEXT("Research.Physics")), Percent(Civ->Research.FindRef(FName(TEXT("Physics")))),
				Label(TEXT("Research.Society")), Percent(Civ->Research.FindRef(FName(TEXT("Society")))),
				Label(TEXT("Research.Engineering")), Percent(Civ->Research.FindRef(FName(TEXT("Engineering")))),
				Label(TEXT("Research.Transcendence")), Percent(Civ->Research.FindRef(FName(TEXT("Transcendence"))))));
	}
	else
	{
		StatusText->SetText(FText::GetEmpty());
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

	// The current speed's button is greyed out (and a click on it would change nothing). The labels follow the unit.
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
			Button->SetIsEnabled(!FMath::IsNearlyEqual(Button->GetValue(), Current));
		}
	}
	LabelledUnit = Unit;
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

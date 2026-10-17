// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicTopBarWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Core/GalaxyState.h"
#include "Engine/GameInstance.h"
#include "Layout/Margin.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicGameMode.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicTopBarImpl
{
	const FName SpeedAction(TEXT("speed"));
	// Pause first; the keys 0 to 4 set the same speeds.
	constexpr float Speeds[] = { 0.f, 1.f, 2.f, 5.f, 10.f };
	const FLinearColor BandColor(0.01f, 0.01f, 0.03f, 0.8f);
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

		CycleText = CosmicUI::MakeText(*WidgetTree, 14, false);
		UHorizontalBoxSlot* CycleSlot = Row->AddChildToHorizontalBox(CycleText);
		CycleSlot->SetVerticalAlignment(VAlign_Center);
		CycleSlot->SetPadding(FMargin(16.f, 0.f, 12.f, 0.f));

		for (const float Speed : Speeds)
		{
			UCosmicActionButton* Button = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
			const FText Label = Speed > 0.f ? FText::Format(INVTEXT("x{0}"), CosmicUI::WholeNumber(Speed)) : CosmicUI::Label(TEXT("Speed.Pause"));
			Button->Setup(SpeedAction, Speed, Label);
			Button->OnAction.AddUObject(this, &UCosmicTopBarWidget::HandleSpeed);
			UHorizontalBoxSlot* ButtonSlot = Row->AddChildToHorizontalBox(Button);
			ButtonSlot->SetVerticalAlignment(VAlign_Center);
			ButtonSlot->SetPadding(FMargin(2.f, 0.f));
			SpeedButtons.Add(Button);
		}
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
	CycleText->SetText(FText::Format(INVTEXT("{0} {1}"), Label(TEXT("Panel.Cycle")), FText::AsNumber(State.Tick)));

	// The current speed's button is greyed out (and a click on it would change nothing).
	const float Current = Sim->GetTimeScale();
	for (UCosmicActionButton* Button : SpeedButtons)
	{
		if (Button)
		{
			Button->SetIsEnabled(!FMath::IsNearlyEqual(Button->GetValue(), Current));
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

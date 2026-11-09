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
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/GameInstance.h"
#include "Layout/Clipping.h"
#include "Layout/Margin.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicAscensionOverlayWidget.h"
#include "UI/CosmicChronicleWidget.h"
#include "UI/CosmicEdictWidget.h"
#include "UI/CosmicNextRung.h"
#include "UI/CosmicStabilityTrend.h"
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
	const FName EdictAction(TEXT("edicts"));
	// The provisional end's grey line beside the clock (MVP item 4): quieter than the line of a pause.
	const FLinearColor EndSoonColor(0.6f, 0.6f, 0.65f);
	// The line of an automatic pause: the warm colour of a flat button under the mouse, so the eye finds why time stopped.
	const FLinearColor WhyColor(1.f, 0.82f, 0.4f);
	const FLinearColor BandColor(0.01f, 0.01f, 0.03f, 0.8f);
	// The text of a flat button (the clock, the rung), and its colour while the mouse is on it (a press or a tooltip waits there).
	const FLinearColor FlatColor = FLinearColor::White;
	const FLinearColor FlatHoverColor(1.f, 0.82f, 0.4f);
	// The trend's cell after the stability (MVP item 6): wide enough for its mark, so the resources do not move with it.
	constexpr float TrendCellWidth = 16.f;

	// The people the vessel rules, as the bar shows it: none for a wanderer, in the interregnum (B5 - no vessel, no line of
	// one: the dead ruler's ReleaseThrone clears the ID, and the bar asks the life state as well) or for an ID the state does
	// not hold.
	const FCivilizationState* RuledCivOf(const FGalaxyState& State)
	{
		const FName RuledCivID = State.Player.RuledCivID;
		return (RuledCivID.IsNone() || State.Calling.LifeState != ELifeState::Embodied) ? nullptr
			: State.Civs.FindByPredicate([RuledCivID](const FCivilizationState& Candidate) { return Candidate.CivID == RuledCivID; });
	}

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

	// Whether the HUD shows the ascension presentation that opens the spiritual sight now: x25 waits for it alone to close
	// (guideline 6.3 - after the presentation, at the same moment as the sight's reveal; B6b decision 25 - a presentation
	// of another rung leaves an open x25 in place).
	bool Presenting(const UUserWidget& Bar)
	{
		const APlayerController* Owner = Bar.GetOwningPlayer();
		const ACosmicHUD* HUD = Owner ? Owner->GetHUD<ACosmicHUD>() : nullptr;
		const UCosmicAscensionOverlayWidget* Overlay = HUD ? HUD->GetAscensionOverlay() : nullptr;
		return Overlay && Overlay->IsShowingSightOpening();
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
		// the research shares. The rung carries the next-rung tooltip and the research its own (GetResearchTip: the focus,
		// then the next rung), each bound before its Slate widget is
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

		// The stability in a block of its own (MVP item 6): its number, then a cell of a fixed width for the trend's mark,
		// so the resources after it never move when the mark comes and goes. Both carry the stability tooltip, bound as
		// the next-rung one; the resources carry none.
		StabilityText = CosmicUI::MakeText(*WidgetTree, 14, false);
		StabilityText->ToolTipTextDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UCosmicTopBarWidget, GetStabilityTip));
		UHorizontalBoxSlot* StabilitySlot = Row->AddChildToHorizontalBox(StabilityText);
		StabilitySlot->SetVerticalAlignment(VAlign_Center);
		StabilitySlot->SetPadding(FMargin(0.f, 0.f, 4.f, 0.f));
		TrendText = CosmicUI::MakeText(*WidgetTree, 14, false);
		TrendText->SetMinDesiredWidth(TrendCellWidth);
		TrendText->ToolTipTextDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UCosmicTopBarWidget, GetStabilityTip));
		UHorizontalBoxSlot* TrendSlot = Row->AddChildToHorizontalBox(TrendText);
		TrendSlot->SetVerticalAlignment(VAlign_Center);
		TrendSlot->SetPadding(FMargin(0.f, 0.f, 20.f, 0.f));

		StatusText = CosmicUI::MakeText(*WidgetTree, 14, false);
		UHorizontalBoxSlot* StatusSlot = Row->AddChildToHorizontalBox(StatusText);
		StatusSlot->SetVerticalAlignment(VAlign_Center);
		StatusSlot->SetPadding(FMargin(0.f, 0.f, 32.f, 0.f));

		// The wanderer's line (B1), a block of its own beside the resources' empty place, with its tooltip bound as the
		// next-rung one; collapsed while a people is ruled.
		WandererText = CosmicUI::MakeText(*WidgetTree, 14, false);
		WandererText->ToolTipTextDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UCosmicTopBarWidget, GetWandererTip));
		WandererText->SetVisibility(ESlateVisibility::Collapsed);
		UHorizontalBoxSlot* WandererSlot = Row->AddChildToHorizontalBox(WandererText);
		WandererSlot->SetVerticalAlignment(VAlign_Center);
		WandererSlot->SetPadding(FMargin(0.f, 0.f, 32.f, 0.f));
		// The lethal hazard's warning (B5 header section 2.8, 4), right after the wanderer's line in the warm colour of a pause's
		// line; collapsed while LethalLineOf has no words.
		LethalText = CosmicUI::MakeText(*WidgetTree, 14, false);
		LethalText->SetColorAndOpacity(FSlateColor(WhyColor));
		LethalText->SetVisibility(ESlateVisibility::Collapsed);
		UHorizontalBoxSlot* LethalSlot = Row->AddChildToHorizontalBox(LethalText);
		LethalSlot->SetVerticalAlignment(VAlign_Center);
		LethalSlot->SetPadding(FMargin(0.f, 0.f, 32.f, 0.f));

		// The edict button (B2) and its mark: a block of its own after the resources, before the research block that
		// yields first; Refresh shows it for an embodied ruler, labels it and sets its tooltip when the words change.
		EdictBlock = WidgetTree->ConstructWidget<UHorizontalBox>();
		EdictBlock->SetVisibility(ESlateVisibility::Collapsed);
		UHorizontalBoxSlot* EdictBlockSlot = Row->AddChildToHorizontalBox(EdictBlock);
		EdictBlockSlot->SetVerticalAlignment(VAlign_Center);
		EdictBlockSlot->SetPadding(FMargin(0.f, 0.f, 24.f, 0.f));
		EdictButton = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		EdictButton->Setup(EdictAction, 0.f, FText::GetEmpty());
		EdictButton->OnAction.AddUObject(this, &UCosmicTopBarWidget::HandleEdicts);
		EdictBlock->AddChildToHorizontalBox(EdictButton)->SetVerticalAlignment(VAlign_Center);
		EdictMark = CosmicUI::MakeText(*WidgetTree, 12, false);
		EdictMark->SetColorAndOpacity(FSlateColor(EndSoonColor));
		EdictMark->SetText(CosmicUI::Label(TEXT("Edict.Mark")));
		// Hidden, never collapsed, while the block shows: the research block after it does not move when the hint comes.
		EdictMark->SetVisibility(ESlateVisibility::Hidden);
		UHorizontalBoxSlot* MarkSlot = EdictBlock->AddChildToHorizontalBox(EdictMark);
		MarkSlot->SetVerticalAlignment(VAlign_Center);
		MarkSlot->SetPadding(FMargin(4.f, 0.f, 0.f, 0.f));

		ResearchText = CosmicUI::MakeText(*WidgetTree, 14, false);
		ResearchText->ToolTipTextDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UCosmicTopBarWidget, GetResearchTip));
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
		// Then the run's rules (MVP item 4; B6b decision 27): the fifth rung's while the tables judge a row up to it, the
		// clock end's while there is one - the tooltip is built once.
		{
			const ACosmicPlayerController* Owner = Cast<ACosmicPlayerController>(GetOwningPlayer());
			const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
			ClockButton->SetToolTipText(ClockTipOf(Sim ? Sim->GetRules().ProvisionalEndTick : 0, Sim && (!Owner || Owner->IsProvisionalEndEnabled()),
				Sim && CosmicNextRung::HasJudgedRowAt(Sim->GetTables(), EAscensionTier::Tier4_Transcendent)));
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

		// One button per speed step, labelled in the cycle form here (Refresh relabels them in the controller's unit). Each
		// takes its tooltip once: its number key (MVP item 6, the first hints - the step's place is its key, 0 to 5; the
		// pause names Space too). A step above the calling's top starts collapsed and adds what the third rung's speed does
		// to the crises.
		for (int32 Step = 0; Step < static_cast<int32>(UE_ARRAY_COUNT(CosmicUI::SpeedSteps)); ++Step)
		{
			const float Speed = CosmicUI::SpeedSteps[Step];
			UCosmicActionButton* Button = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
			Button->Setup(SpeedAction, Speed, CosmicUI::SpeedLabel(Speed, ECosmicTimeUnit::Cycle));
			Button->OnAction.AddUObject(this, &UCosmicTopBarWidget::HandleSpeed);
			const FText KeyTip = FText::Format(CosmicUI::Label(Speed > 0.f ? TEXT("Hint.SpeedKey") : TEXT("Hint.PauseKey")), FText::AsNumber(Step));
			if (Speed > CosmicUI::CallingTopSpeed)
			{
				Button->SetToolTipText(CosmicUI::JoinLines({ KeyTip, CosmicUI::Label(TEXT("Speed.ThirdRungTip")) }));
				Button->SetVisibility(ESlateVisibility::Collapsed);
			}
			else
			{
				Button->SetToolTipText(KeyTip);
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
	SampleStability();
	Refresh();
}

void UCosmicTopBarWidget::HandleGalaxyGenerated(int32 Seed)
{
	// Another game (a new galaxy or a load): what the samples saw belongs to the old one.
	ClearStabilitySamples();
	Refresh();
}

void UCosmicTopBarWidget::ClearStabilitySamples()
{
	StabilitySamples.Reset();
	LastSampleTick = -1;
	SampledCivID = NAME_None;
}

void UCosmicTopBarWidget::SampleStability()
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	const FCivilizationState* Civ = Sim ? CosmicTopBarImpl::RuledCivOf(Sim->GetState()) : nullptr;
	if (!Civ)
	{
		ClearStabilitySamples();
		return;
	}
	const int64 Tick = Sim->GetState().Tick;
	// Consecutive ticks of one people only: a deposition, an enthronement or a skipped tick (a headless leap) starts again.
	if (Civ->CivID != SampledCivID || LastSampleTick < 0 || Tick != LastSampleTick + 1)
	{
		StabilitySamples.Reset();
	}
	SampledCivID = Civ->CivID;
	LastSampleTick = Tick;
	StabilitySamples.Add(Civ->Stability);
	if (StabilitySamples.Num() > CosmicUI::StabilityTrendTicks)
	{
		StabilitySamples.RemoveAt(0, StabilitySamples.Num() - CosmicUI::StabilityTrendTicks);
	}
}

ECosmicTrend UCosmicTopBarWidget::TrendNow(int32& OutPoints, int32& OutMissing) const
{
	OutPoints = 0;
	OutMissing = CosmicUI::StabilityTrendTicks;
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	const FCivilizationState* Civ = Sim ? CosmicTopBarImpl::RuledCivOf(Sim->GetState()) : nullptr;
	// The samples are this people's up to this tick: a redraw while paused takes the live stability (an answer counts at once).
	if (!Civ || Civ->CivID != SampledCivID || LastSampleTick != Sim->GetState().Tick)
	{
		return ECosmicTrend::Unknown;
	}
	OutMissing = FMath::Max(0, CosmicUI::StabilityTrendTicks - StabilitySamples.Num());
	OutPoints = CosmicUI::StabilityPointsOf(StabilitySamples, Civ->Stability);
	return CosmicUI::StabilityTrendOf(StabilitySamples, Civ->Stability);
}

ECosmicTrend UCosmicTopBarWidget::GetStabilityTrend() const
{
	int32 Points = 0;
	int32 Missing = 0;
	return TrendNow(Points, Missing);
}

FText UCosmicTopBarWidget::WandererLineOf(const FGalaxyState& State, const FCosmicStateIndex& Index, ECosmicTimeUnit Unit)
{
	using CosmicUI::Label;

	const int32 Here = CosmicWanderer::IsEmbodiedWanderer(State) ? Index.FindSystem(State, State.Player.CurrentSystemID) : INDEX_NONE;
	if (!State.Systems.IsValidIndex(Here))
	{
		return FText::GetEmpty();
	}
	FFormatNamedArguments Args;
	if (CosmicWanderer::IsTravelling(State))
	{
		const int32 To = Index.FindSystem(State, State.Player.TravelTargetSystemID);
		Args.Add(TEXT("System"), State.Systems.IsValidIndex(To) ? State.Systems[To].DisplayName : FText::GetEmpty());
		Args.Add(TEXT("Time"), CosmicUI::TimeLeftLabel(State.Player.TravelTicksRemaining, Unit));
		return FText::Format(Label(TEXT("Wander.Bar.Travelling")), Args);
	}
	Args.Add(TEXT("System"), State.Systems[Here].DisplayName);
	const FText Line = FText::Format(Label(TEXT("Wander.Bar.Standing")), Args);
	// Nowhere to go: no lane of the system leads where TravelTo would take the vessel.
	bool bStuck = true;
	for (const FName Lane : State.Systems[Here].Lanes)
	{
		bStuck = bStuck && CosmicWanderer::TravelBlockOf(State, Index, Lane, false) != CosmicWanderer::ETravelBlock::None;
	}
	return bStuck ? CosmicUI::JoinList({ Line, Label(TEXT("Wander.Bar.Stuck")) }) : Line;
}

FText UCosmicTopBarWidget::LethalLineOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
{
	auto HazardName = [&Tables](const FGameplayTag& Standing)
	{
		const FHazardDef* Hazard = Tables.FindHazardByTag(Standing);
		return Hazard ? Hazard->DisplayName : FText::AsCultureInvariant(Standing.ToString());
	};
	const int32 Place = CosmicAutoPause::LethalPlaceOf(State, Index, Tables, Rules);
	if (State.Systems.IsValidIndex(Place))
	{
		// Under way the vessel still stands where it left until it arrives (section 2.8), and the system it left is the lethal
		// place then only while its hazard expires first ([B-5r]): the words say it expires before the arrival.
		return FText::FormatNamed(CosmicUI::Label(CosmicWanderer::IsTravelling(State) ? TEXT("Wander.Bar.LethalTravelling") : TEXT("Wander.Bar.Lethal")),
			TEXT("Hazard"), HazardName(State.Systems[Place].ActiveHazard));
	}
	// [B-5r] (the [B-5d2] review's play-1): under way to a system whose lethal hazard will still stand when the vessel arrives
	// (CosmicAutoPause::LethalStayOf at the arrival's tick), in the words of whether leaving it again would be gone before it.
	if (!CosmicWanderer::IsEmbodiedWanderer(State) || !CosmicWanderer::IsTravelling(State) || State.Player.TravelTicksRemaining <= 0)
	{
		return FText::GetEmpty();
	}
	const int32 To = Index.FindSystem(State, State.Player.TravelTargetSystemID);
	const ECosmicLethalStay Ahead = CosmicAutoPause::LethalStayOf(State, Tables, Rules, To, State.Tick + State.Player.TravelTicksRemaining);
	if (Ahead == ECosmicLethalStay::None)
	{
		return FText::GetEmpty();
	}
	return FText::FormatNamed(CosmicUI::Label(Ahead == ECosmicLethalStay::NoWayOut ? TEXT("Wander.Bar.AheadTrap") : TEXT("Wander.Bar.Ahead")),
		TEXT("Hazard"), HazardName(State.Systems[To].ActiveHazard));
}

FText UCosmicTopBarWidget::GetWandererTip() const
{
	using CosmicUI::Label;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || !CosmicWanderer::IsEmbodiedWanderer(Sim->GetState()))
	{
		return FText::GetEmpty();
	}
	const FGalaxyState& State = Sim->GetState();
	const FPlayerEntityState& Vessel = State.Player;
	auto Stat = [](const TCHAR* Key, int32 Value) { return FText::Format(INVTEXT("{0} {1}"), Label(Key), FText::AsNumber(Value)); };
	const FText Stats = CosmicUI::JoinList({ Stat(TEXT("Petition.Stat.Body"), Vessel.Body), Stat(TEXT("Petition.Stat.Mind"), Vessel.Mind),
		Stat(TEXT("Petition.Stat.Causality"), Vessel.Causality), Stat(TEXT("Petition.Stat.Exaltation"), Vessel.Exaltation) });
	// The people that looks up to the vessel most, among those the viewer is shown (the first in Civs order on a tie).
	const FCosmicKnownView View = ACosmicPlayerController::KnownViewFor(*this);
	int32 Most = INDEX_NONE;
	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		const float Reverence = State.Civs[CivIndex].ReverenceToPlayer;
		if (View.ShownOrderOf(CivIndex) != INDEX_NONE && Reverence > 0.f && (Most == INDEX_NONE || Reverence > State.Civs[Most].ReverenceToPlayer))
		{
			Most = CivIndex;
		}
	}
	FText Looked;
	if (Most != INDEX_NONE)
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Civ"), State.Civs[Most].CivName);
		Args.Add(TEXT("Reverence"), CosmicUI::Percent(State.Civs[Most].ReverenceToPlayer));
		Looked = FText::Format(Label(TEXT("Wander.Bar.TipReverence")), Args);
	}
	return CosmicUI::JoinLines({ Stats, Looked });
}

FText UCosmicTopBarWidget::GetStabilityTip() const
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || !CosmicTopBarImpl::RuledCivOf(Sim->GetState()))
	{
		return FText::GetEmpty();
	}
	const ECosmicTimeUnit Unit = ACosmicPlayerController::TimeUnitFor(*this);
	int32 Points = 0;
	int32 Missing = 0;
	switch (TrendNow(Points, Missing))
	{
	case ECosmicTrend::Up:
		return FText::Format(CosmicUI::Label(TEXT("Panel.StabilityUp")), CosmicUI::DurationLabel(CosmicUI::StabilityTrendTicks, Unit), FText::AsNumber(Points));
	case ECosmicTrend::Down:
		return FText::Format(CosmicUI::Label(TEXT("Panel.StabilityDown")), CosmicUI::DurationLabel(CosmicUI::StabilityTrendTicks, Unit), FText::AsNumber(-Points));
	case ECosmicTrend::Flat:
		return FText::Format(CosmicUI::Label(TEXT("Panel.StabilityFlat")), CosmicUI::DurationLabel(CosmicUI::StabilityTrendTicks, Unit));
	case ECosmicTrend::Unknown:
		break;
	}
	return FText::Format(CosmicUI::Label(TEXT("Panel.StabilityWait")), CosmicUI::DurationLabel(Missing, Unit));
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

FText UCosmicTopBarWidget::ClockTipOf(int32 EndTick, bool bEndOn, bool bRungRule)
{
	FText Tip = FText::Format(INVTEXT("{0}\n{1}"), CosmicUI::Label(TEXT("Time.Tip")), CosmicUI::Label(TEXT("Time.TipPress")));
	// With the end switched off nothing ends (the game mode's run end returns first): no rule line.
	if (!bEndOn)
	{
		return Tip;
	}
	// The fifth rung's rule while a row can reach it (B6b decision 27: the ruled people's end), with or without a clock.
	if (bRungRule)
	{
		Tip = FText::Format(INVTEXT("{0}\n{1}"), Tip, CosmicUI::Label(TEXT("Chronicle.RuleRung")));
	}
	// The clock end's rule, always in cycles: the rule is the run's, not the viewer's unit.
	if (EndTick > 0)
	{
		const FText Rule = FText::Format(CosmicUI::Label(TEXT("Chronicle.Rule")), CosmicUI::ClockLabel(EndTick, ECosmicTimeUnit::Cycle));
		Tip = FText::Format(INVTEXT("{0}\n{1}"), Tip, Rule);
	}
	return Tip;
}

void UCosmicTopBarWidget::HandleReopen(FName Action, float Value)
{
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->ReopenChronicle();
	}
	Refresh();
}

void UCosmicTopBarWidget::HandleEdicts(FName Action, float Value)
{
	// The controller hands the press to the HUD's window; this bar may not be the HUD's (tests).
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->ToggleEdictWindow();
	}
	Refresh();
}

FText UCosmicTopBarWidget::GetEdictTip() const
{
	using CosmicUI::Label;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	const FCivilizationState* Civ = Sim ? CosmicTopBarImpl::RuledCivOf(Sim->GetState()) : nullptr;
	if (!Civ || Sim->GetState().Calling.LifeState != ELifeState::Embodied || Sim->GetState().Player.Role != EPlayerRole::Ruler)
	{
		return FText::GetEmpty();
	}
	const ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer());
	const FText Hint = (Controller && CosmicUI::ShowsEdictHint(Controller->GetHintProgress())) ? UCosmicEdictWidget::HintOf(Sim->GetTables(), Sim->GetRules()) : FText::GetEmpty();
	const FText Upkeep = FText::Format(Label(TEXT("Edict.Bar.Upkeep")),
		UCosmicEdictWidget::UpkeepWordsOf(UCosmicEdictWidget::HeldUpkeepOf(*Civ, Sim->GetTables()), ACosmicPlayerController::TimeUnitFor(*this)));
	const TOptional<float> Next = UCosmicEdictWidget::NextSlotMarkOf(*Civ, Sim->GetRules());
	const FText SlotWords = Next.IsSet() ? FText::Format(Label(TEXT("Edict.Bar.NextSlot")), CosmicUI::Percent(Next.GetValue())) : Label(TEXT("Edict.Bar.AllSlots"));
	return CosmicUI::JoinLines({ Hint, Upkeep, SlotWords, Label(TEXT("Edict.Bar.Peoples")), Label(TEXT("Edict.Bar.Press")) });
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
	// the rung lights only while there is a next rung to tell (none on the fifth or on a rung with no judged row, for a
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
	return CosmicNextRung::TextOfAll(CosmicNextRung::AllOf(Sim->GetState(), Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules()), Sim->GetTables(),
		ACosmicPlayerController::TimeUnitFor(*this));
}

FText UCosmicTopBarWidget::GetResearchTip() const
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	const FCivilizationState* Civ = Sim ? CosmicTopBarImpl::RuledCivOf(Sim->GetState()) : nullptr;
	const FText Rung = GetNextRungTip();
	if (!Civ)
	{
		return Rung;
	}
	// The focus first: after another system is selected the panel's block is gone, and the bar still says it.
	const FText Focus = FText::Format(CosmicUI::Label(TEXT("Focus.BarLine")), CosmicUI::FocusWordOf(Civ->ResearchFocus, Sim->GetRules().ResearchFocusShare),
		CosmicUI::FocusSharesOf(Civ->ResearchFocus));
	return Rung.IsEmpty() ? Focus : FText::Format(INVTEXT("{0}\n\n{1}"), Focus, Rung);
}

void UCosmicTopBarWidget::Refresh()
{
	using namespace CosmicUI;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || !StatusText || !StabilityText || !TrendText || !RungText || !ResearchText || !WandererText)
	{
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	const FCivilizationState* Civ = CosmicTopBarImpl::RuledCivOf(State);
	if (Civ)
	{
		RungText->SetText(FText::Format(INVTEXT("{0}   {1} {2}"), Civ->CivName, Label(TEXT("Panel.Tier")), FText::AsNumber(static_cast<int32>(Civ->Tier) + 1)));
		StabilityText->SetText(FText::Format(INVTEXT("{0} {1}"), Label(TEXT("Panel.Stability")), Percent(Civ->Stability)));
		int32 Points = 0;
		int32 Missing = 0;
		const ECosmicTrend Trend = TrendNow(Points, Missing);
		TrendText->SetText(Trend == ECosmicTrend::Up ? Label(TEXT("Panel.TrendUp")) : (Trend == ECosmicTrend::Down ? Label(TEXT("Panel.TrendDown")) : FText::GetEmpty()));
		StatusText->SetText(FText::Format(INVTEXT("{0} {1}  {2} {3}  {4} {5}  {6} {7}  {8} {9}"),
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
		StabilityText->SetText(FText::GetEmpty());
		TrendText->SetText(FText::GetEmpty());
		StatusText->SetText(FText::GetEmpty());
		ResearchText->SetText(FText::GetEmpty());
	}
	// Whether the controller shows the vessel (B5 [B-5c]): not under a calling page - the digest and the inauguration stand over
	// a new vessel that the inauguration introduces - nor the vessel an ended run's last tick installed. The wanderer's line and
	// its warning read it; the fold of the pages redraws the bar.
	const bool bVesselShown = ACosmicPlayerController::IsVesselShownFor(*this);
	// The wanderer's line (B1): shown while it has words - an embodied wanderer -, collapsed otherwise.
	const FText Wandering = (Civ || !bVesselShown) ? FText::GetEmpty() : WandererLineOf(State, Sim->GetStateIndex(), ACosmicPlayerController::TimeUnitFor(*this));
	if (!WandererText->GetText().ToString().Equals(Wandering.ToString(), ESearchCase::CaseSensitive))
	{
		WandererText->SetText(Wandering);
	}
	const ESlateVisibility WanderingVisibility = Wandering.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
	if (WandererText->GetVisibility() != WanderingVisibility)
	{
		WandererText->SetVisibility(WanderingVisibility);
	}
	// The lethal hazard where the wanderer stands (B5): its warning while it has words. Set only when it changes.
	if (LethalText)
	{
		const FText Lethal = bVesselShown ? LethalLineOf(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules()) : FText::GetEmpty();
		if (!LethalText->GetText().ToString().Equals(Lethal.ToString(), ESearchCase::CaseSensitive))
		{
			LethalText->SetText(Lethal);
		}
		const ESlateVisibility LethalVisibility = Lethal.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible;
		if (LethalText->GetVisibility() != LethalVisibility)
		{
			LethalText->SetVisibility(LethalVisibility);
		}
	}
	// The edict button (B2): an embodied ruler's, in a run that is not over. The edict hint is stepped first (the system
	// panel steps it too - whichever redraws first). Words and visibility set only when they change.
	if (EdictBlock && EdictButton && EdictMark)
	{
		ACosmicPlayerController* EdictOwner = Cast<ACosmicPlayerController>(GetOwningPlayer());
		if (EdictOwner)
		{
			EdictOwner->FollowEdictHint();
		}
		const bool bRuler = Civ && State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler;
		const bool bShowEdicts = bRuler && !ACosmicPlayerController::IsRunEndedFor(*this);
		const ESlateVisibility EdictVisibility = bShowEdicts ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
		if (EdictBlock->GetVisibility() != EdictVisibility)
		{
			EdictBlock->SetVisibility(EdictVisibility);
		}
		if (bShowEdicts)
		{
			const FText Count = FText::Format(Label(TEXT("Edict.Count")), FText::AsNumber(CosmicEdicts::HeldCountOf(*Civ, Sim->GetTables())),
				FText::AsNumber(CosmicEdicts::SlotsOf(*Civ, Sim->GetRules())));
			if (!EdictButton->GetLabel().ToString().Equals(Count.ToString(), ESearchCase::CaseSensitive))
			{
				EdictButton->Setup(CosmicTopBarImpl::EdictAction, 0.f, Count);
			}
			const FText Tip = GetEdictTip();
			if (!EdictButton->GetToolTipText().ToString().Equals(Tip.ToString(), ESearchCase::CaseSensitive))
			{
				EdictButton->SetToolTipText(Tip);
			}
		}
		const ESlateVisibility MarkVisibility = (bShowEdicts && EdictOwner && CosmicUI::ShowsEdictHint(EdictOwner->GetHintProgress()))
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
		if (EdictMark->GetVisibility() != MarkVisibility)
		{
			EdictMark->SetVisibility(MarkVisibility);
		}
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

	// The speed buttons: a step the gate does not open is collapsed (x25 also while the presentation that opens the
	// spiritual sight shows - it appears when that presentation closes; B6b decision 25), the current speed's button is greyed out (a click on it would change
	// nothing), and every label, the collapsed ones too, follows the unit. Visibility is set only when it changes.
	const bool bThirdRungShown = CosmicTopBarImpl::ThirdRungOpen(*this, *Sim) && !CosmicTopBarImpl::Presenting(*this);
	// The run is over (MVP item 4): every speed button is greyed out (x25 stays where the gate shows it).
	const bool bEnded = ACosmicPlayerController::IsRunEndedFor(*this);
	const bool bCallingPage = ACosmicPlayerController::IsCallingPageOpenFor(*this);
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
			// So is every one while a calling page shows (B5 [B-5c]): the page's own buttons turn it.
			Button->SetIsEnabled(!bEnded && !bCallingPage && !FMath::IsNearlyEqual(Button->GetValue(), Current));
		}
	}
	LabelledUnit = Unit;

	// The automatic pause: its button says on or off; the line says why the time stands still while an automatic pause
	// holds it (the reason goes when the speed is set again). Both set only when they change.
	const ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer());

	// The provisional end (MVP item 4): the chronicle's button while the run is over and the page folded, and before the end
	// the grey line - the last minutes of the clock, or the clock end while the vessel is an embodied wanderer (B5: every one -
	// a called vessel never ruled; a ruler is told the end by the clock tip's rule line - B6a header section 4). Set only when
	// they change.
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
			if (Left > 0 && Left <= CosmicChronicleScreen::EndSoonTicks)
			{
				Soon = FText::Format(Label(TEXT("Chronicle.Soon")), TimeLeftLabel(Left, Unit));
			}
			else if (Left > 0 && CosmicWanderer::IsEmbodiedWanderer(State))
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

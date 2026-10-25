// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicEdictWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/GameInstance.h"
#include "Framework/Text/TextLayout.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Layout/Margin.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicResearch.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicGameMode.h"

const FName UCosmicEdictWidget::SliderFields[3] = { FName(TEXT("Physics")), FName(TEXT("Society")), FName(TEXT("Engineering")) };

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicEdictWindowImpl
{
	// The panels' box, a little less see-through: the window is read, not looked through.
	const FLinearColor BoxColor(0.01f, 0.01f, 0.03f, 0.9f);
	// The rows' page grows with its rows up to this height and scrolls past it (the window's area is 760 high - the HUD).
	constexpr float RowsMaxHeight = 680.f;
	// The inherited mark: the grey of the first hints.
	const FLinearColor QuietColor(0.6f, 0.6f, 0.65f);
	const FLinearColor SummaryColor(0.75f, 0.75f, 0.82f);
	const FName EdictsTabAction(TEXT("tab_edicts"));
	const FName SharesTabAction(TEXT("tab_shares"));
	const FName CloseAction(TEXT("close"));
	const FName ApplyAction(TEXT("apply"));
	// One hundred ticks: the span Stability points are told over (a tick's change is a fraction of a point).
	constexpr int64 StabilitySpan = 100;

	bool SameWords(const FText& A, const FText& B)
	{
		return A.ToString().Equals(B.ToString(), ESearchCase::CaseSensitive);
	}

	FText Amount(float Value)
	{
		FNumberFormattingOptions Options;
		Options.MinimumFractionalDigits = 0;
		Options.MaximumFractionalDigits = 2;
		return FText::AsNumber(Value, &Options);
	}

	// "+20%" or "-20%": a factor told as the change it makes.
	FText SignedPercent(float Factor)
	{
		const float Change = Factor - 1.f;
		return Change > 0.f ? FText::Format(INVTEXT("+{0}"), CosmicUI::Percent(Change)) : CosmicUI::Percent(Change);
	}

	// "+5" or "-15": Stability points over StabilitySpan ticks (up to one decimal).
	FText SignedPoints(float PerTick)
	{
		FNumberFormattingOptions Options;
		Options.MinimumFractionalDigits = 0;
		Options.MaximumFractionalDigits = 1;
		const float Points = PerTick * static_cast<float>(StabilitySpan) * 100.f;
		const FText Number = FText::AsNumber(Points, &Options);
		return Points > 0.f ? FText::Format(INVTEXT("+{0}"), Number) : Number;
	}

	// The label key of a resource named by an effect's Key; null for anything else.
	const TCHAR* ResourceKeyOf(FName Key)
	{
		static const TPair<const TCHAR*, const TCHAR*> Resources[] = {
			{ TEXT("Energy"), TEXT("Resource.Energy") },
			{ TEXT("Matter"), TEXT("Resource.Matter") },
			{ TEXT("Insight"), TEXT("Resource.Insight") },
			{ TEXT("Influence"), TEXT("Resource.Influence") },
			{ TEXT("Causality"), TEXT("Resource.Causality") },
		};
		for (const TPair<const TCHAR*, const TCHAR*>& Resource : Resources)
		{
			if (Key == FName(Resource.Key))
			{
				return Resource.Value;
			}
		}
		return nullptr;
	}

	// The words of an ethics axis moved up (bUp) or down; null for a key that is no axis.
	const TCHAR* EthicsKeyOf(FName Key, bool bUp)
	{
		if (Key == FName(TEXT("Pacifism")))
		{
			return bUp ? TEXT("Edict.Effect.PacifismUp") : TEXT("Edict.Effect.PacifismDown");
		}
		if (Key == FName(TEXT("Spiritualism")))
		{
			return bUp ? TEXT("Edict.Effect.SpiritualismUp") : TEXT("Edict.Effect.SpiritualismDown");
		}
		if (Key == FName(TEXT("Openness")))
		{
			return bUp ? TEXT("Edict.Effect.OpennessUp") : TEXT("Edict.Effect.OpennessDown");
		}
		return nullptr;
	}

	// One effect a tick in words; empty for an effect that changes nothing, and for an effect the window has no words of -
	// that one sets bOther (TipOf then adds Edict.Effect.Other once).
	FText EffectLineOf(const FCosmicEffect& Effect, ECosmicTimeUnit Unit, bool& bOther)
	{
		using CosmicUI::Label;

		if (!(Effect.Value >= -TNumericLimits<float>::Max() && Effect.Value <= TNumericLimits<float>::Max()))
		{
			return FText::GetEmpty();
		}
		switch (Effect.Type)
		{
		case EEffectType::MultiplyIncome:
		{
			if (Effect.Value == 1.f || Effect.Value < 0.f)
			{
				return FText::GetEmpty();
			}
			if (Effect.Key.IsNone())
			{
				return FText::Format(Label(TEXT("Edict.Effect.AllIncome")), SignedPercent(Effect.Value));
			}
			const TCHAR* const ResourceKey = ResourceKeyOf(Effect.Key);
			return ResourceKey ? FText::Format(Label(TEXT("Edict.Effect.Income")), Label(ResourceKey), SignedPercent(Effect.Value)) : FText::GetEmpty();
		}
		case EEffectType::ChangeStability:
			return Effect.Value == 0.f ? FText::GetEmpty()
				: FText::Format(Label(TEXT("Edict.Effect.Stability")), SignedPoints(Effect.Value), CosmicUI::DurationLabel(StabilitySpan, Unit));
		case EEffectType::ShiftEthics:
		{
			const TCHAR* const Key = Effect.Value == 0.f ? nullptr : EthicsKeyOf(Effect.Key, Effect.Value > 0.f);
			return Key ? Label(Key) : FText::GetEmpty();
		}
		default:
			break;
		}
		bOther = bOther || Effect.Value != 0.f;
		return FText::GetEmpty();
	}

	// The name of the hazard of Tag (its row's DisplayName), or the tag itself when no row holds it.
	FText HazardNameOf(const FCosmicDataTables& Tables, FGameplayTag Tag)
	{
		const FHazardDef* Hazard = Tables.FindHazardByTag(Tag);
		return Hazard ? Hazard->DisplayName : FText::AsCultureInvariant(Tag.ToString());
	}

	// The tags of a map, in their names' lexical order (the window lists them the same way every time).
	template <typename FValue>
	TArray<FGameplayTag> SortedTags(const TMap<FGameplayTag, FValue>& Map)
	{
		TArray<FGameplayTag> Tags;
		Map.GetKeys(Tags);
		Tags.Sort([](const FGameplayTag& A, const FGameplayTag& B) { return FNameLexicalLess()(A.GetTagName(), B.GetTagName()); });
		return Tags;
	}

	// The research shares Weights would give the people (Current when they do not fit, with bFits false).
	TMap<FName, float> WouldOf(const TMap<FName, float>& Current, const TMap<FName, int32>& Weights, bool& bFits)
	{
		TMap<FName, float> Would = Current;
		bFits = CosmicResearch::SharesOf(Would, Weights);
		return bFits ? Would : Current;
	}
}

FText UCosmicEdictWidget::CostWordsOf(const FResourcePool& Cost)
{
	using namespace CosmicEdictWindowImpl;

	const FResourcePool Paid = CosmicEdicts::CostOf(Cost);
	const TPair<const TCHAR*, float> Members[] = {
		{ TEXT("Resource.Energy"), Paid.Energy },
		{ TEXT("Resource.Matter"), Paid.Matter },
		{ TEXT("Resource.Insight"), Paid.Insight },
		{ TEXT("Resource.Influence"), Paid.Influence },
		{ TEXT("Resource.Causality"), Paid.Causality },
	};
	TArray<FText> Words;
	for (const TPair<const TCHAR*, float>& Member : Members)
	{
		if (Member.Value > 0.f)
		{
			Words.Add(FText::Format(INVTEXT("{0} {1}"), CosmicUI::Label(Member.Key), Amount(Member.Value)));
		}
	}
	return CosmicUI::JoinList(Words);
}

FText UCosmicEdictWidget::UpkeepWordsOf(const FResourcePool& Upkeep, ECosmicTimeUnit Unit)
{
	const FText Cost = CostWordsOf(Upkeep);
	return Cost.IsEmpty() ? CosmicUI::Label(TEXT("Edict.NoUpkeep"))
		: FText::Format(CosmicUI::Label(Unit == ECosmicTimeUnit::Year ? TEXT("Edict.PerYear") : TEXT("Edict.PerCycle")), Cost);
}

TOptional<float> UCosmicEdictWidget::NextSlotMarkOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules)
{
	// The marks as CosmicEdicts::SlotsOf reads them: reached at Society >= Mark, a mark that is not a number never.
	const float Society = Civ.Research.FindRef(FName(TEXT("Society")));
	TOptional<float> Next;
	for (const float Mark : Rules.EdictSlotSocietyMarks)
	{
		if (Mark >= -TNumericLimits<float>::Max() && Mark <= TNumericLimits<float>::Max() && !(Society >= Mark) && (!Next.IsSet() || Mark < Next.GetValue()))
		{
			Next = Mark;
		}
	}
	return Next;
}

FResourcePool UCosmicEdictWidget::HeldUpkeepOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables)
{
	FResourcePool Sum;
	for (const FName Row : Civ.ActiveEdicts)
	{
		if (const FEdictDef* Def = Tables.FindEdict(Row))
		{
			const FResourcePool Paid = CosmicEdicts::CostOf(Def->UpkeepPerTick);
			Sum.Energy += Paid.Energy;
			Sum.Matter += Paid.Matter;
			Sum.Insight += Paid.Insight;
			Sum.Influence += Paid.Influence;
			Sum.Causality += Paid.Causality;
		}
	}
	return Sum;
}

FText UCosmicEdictWidget::HintOf(const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
{
	const FEdictDef* Def = Rules.StartingEdict.IsNone() ? nullptr : Tables.FindEdict(Rules.StartingEdict);
	return Def ? FText::FormatNamed(CosmicUI::Label(TEXT("Hint.Edict")), TEXT("Edict"), Def->DisplayName) : FText::GetEmpty();
}

FText UCosmicEdictWidget::TipOf(const FCosmicDataTables& Tables, FName Row, ECosmicTimeUnit Unit)
{
	using namespace CosmicEdictWindowImpl;
	using CosmicUI::Label;

	const FEdictDef* Def = Row.IsNone() ? nullptr : Tables.FindEdict(Row);
	if (!Def)
	{
		return FText::GetEmpty();
	}
	TArray<FText> Lines;
	const FText Cost = CostWordsOf(Def->EnactCost);
	Lines.Add(Cost.IsEmpty() ? Label(TEXT("Edict.TipCostNone")) : FText::Format(Label(TEXT("Edict.TipCost")), Cost));
	Lines.Add(FText::Format(Label(TEXT("Edict.TipUpkeep")), UpkeepWordsOf(Def->UpkeepPerTick, Unit)));
	bool bOther = false;
	for (const FCosmicEffect& Effect : Def->EffectsPerTick)
	{
		Lines.Add(EffectLineOf(Effect, Unit, bOther));
	}
	if (bOther)
	{
		Lines.Add(Label(TEXT("Edict.Effect.Other")));
	}
	for (const FGameplayTag& Tag : SortedTags(Def->HazardSpawnMultipliers))
	{
		const float Factor = Def->HazardSpawnMultipliers.FindRef(Tag);
		if (Factor >= 0.f && Factor <= TNumericLimits<float>::Max() && Factor != 1.f)
		{
			FFormatNamedArguments Args;
			Args.Add(TEXT("Hazard"), HazardNameOf(Tables, Tag));
			Args.Add(TEXT("Factor"), Amount(Factor));
			Lines.Add(FText::Format(Label(TEXT("Edict.Effect.Spawn")), Args));
		}
	}
	bool bCountdown = false;
	for (const FGameplayTag& Tag : SortedTags(Def->HazardCountdownBonus))
	{
		const int32 Bonus = Def->HazardCountdownBonus.FindRef(Tag);
		if (Bonus > 0)
		{
			FFormatNamedArguments Args;
			Args.Add(TEXT("Hazard"), HazardNameOf(Tables, Tag));
			Args.Add(TEXT("Time"), CosmicUI::DurationLabel(Bonus, Unit));
			Lines.Add(FText::Format(Label(TEXT("Edict.Effect.Countdown")), Args));
			bCountdown = true;
		}
	}
	if (bCountdown)
	{
		Lines.Add(Label(TEXT("Edict.TipNewOnly")));
	}
	// The hazards it answers at their expiry, by their rows' order.
	for (const FName HazardRow : Tables.GetHazardRowNames())
	{
		const FHazardDef* Hazard = Tables.FindHazard(HazardRow);
		if (Hazard && Hazard->RulerResolutions.Contains(Row))
		{
			Lines.Add(FText::FormatNamed(Label(TEXT("Edict.TipAnswers")), TEXT("Hazard"), Hazard->DisplayName));
		}
	}
	return CosmicUI::JoinLines(Lines);
}

FText UCosmicEdictWidget::ReasonOf(CosmicEdicts::ECosmicEdictBlock Block, const FEdictDef* Row, bool bSlotsCanGrow)
{
	using CosmicEdicts::ECosmicEdictBlock;
	using CosmicUI::Label;

	switch (Block)
	{
	case ECosmicEdictBlock::NoRow:
		return Label(TEXT("Edict.Block.NoRow"));
	case ECosmicEdictBlock::Held:
		return Label(TEXT("Edict.Block.Held"));
	case ECosmicEdictBlock::Tier:
		return FText::Format(Label(TEXT("Edict.Block.Tier")), FText::AsNumber(Row ? static_cast<int32>(Row->RequiredTier) + 1 : 0));
	case ECosmicEdictBlock::Conditions:
		return Label(TEXT("Edict.Block.Conditions"));
	case ECosmicEdictBlock::Slots:
		return Label(bSlotsCanGrow ? TEXT("Edict.Block.Slots") : TEXT("Edict.Block.SlotsAll"));
	case ECosmicEdictBlock::Cost:
		return Label(TEXT("Edict.Block.Cost"));
	case ECosmicEdictBlock::None:
	case ECosmicEdictBlock::NotRuler:
	case ECosmicEdictBlock::Handler:
		break;
	}
	return FText::GetEmpty();
}

FText UCosmicEdictWidget::SuffixOf(CosmicEdicts::ECosmicEdictBlock Block, const FEdictDef* Row)
{
	using CosmicEdicts::ECosmicEdictBlock;
	using CosmicUI::Label;

	switch (Block)
	{
	case ECosmicEdictBlock::Tier:
		return FText::Format(Label(TEXT("Edict.Suffix.Tier")), FText::AsNumber(Row ? static_cast<int32>(Row->RequiredTier) + 1 : 0));
	case ECosmicEdictBlock::Conditions:
		return Label(TEXT("Edict.Suffix.Conditions"));
	case ECosmicEdictBlock::Slots:
		return Label(TEXT("Edict.Suffix.Slots"));
	case ECosmicEdictBlock::Cost:
		return Label(TEXT("Edict.Suffix.Cost"));
	case ECosmicEdictBlock::None:
	case ECosmicEdictBlock::NotRuler:
	case ECosmicEdictBlock::NoRow:
	case ECosmicEdictBlock::Held:
	case ECosmicEdictBlock::Handler:
		break;
	}
	return FText::GetEmpty();
}

TSharedRef<SWidget> UCosmicEdictWidget::RebuildWidget()
{
	using namespace CosmicEdictWindowImpl;

	// RebuildWidget can run again after the Slate widgets are released; the tree is built once.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>();
		Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		WidgetTree->RootWidget = Root;
		// The box sits at the top of the window's area as tall as its page; the rest of the area lets clicks through to the map.
		UBorder* Box = WidgetTree->ConstructWidget<UBorder>();
		Box->SetBrushColor(BoxColor);
		Box->SetPadding(FMargin(14.f, 10.f));
		Root->AddChildToVerticalBox(Box);
		UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
		Box->SetContent(Column);

		// The tabs, then [Close] at the right end.
		UHorizontalBox* Tabs = WidgetTree->ConstructWidget<UHorizontalBox>();
		Column->AddChildToVerticalBox(Tabs)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		auto AddControl = [this, Tabs](FName Action, const FText& Label, float LeftPad) -> UCosmicActionButton*
		{
			UCosmicActionButton* Button = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
			Button->Setup(Action, 0.f, Label);
			Button->OnAction.AddUObject(this, &UCosmicEdictWidget::HandleAction);
			Tabs->AddChildToHorizontalBox(Button)->SetPadding(FMargin(LeftPad, 0.f, 0.f, 0.f));
			return Button;
		};
		EdictsTabButton = AddControl(EdictsTabAction, FText::Format(CosmicUI::Label(TEXT("Edict.Count")), FText::AsNumber(0), FText::AsNumber(1)), 0.f);
		SharesTabButton = AddControl(SharesTabAction, CosmicUI::Label(TEXT("Edict.Tab.Shares")), 4.f);
		UHorizontalBoxSlot* Gap = Tabs->AddChildToHorizontalBox(WidgetTree->ConstructWidget<UTextBlock>());
		Gap->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		CloseButton = AddControl(CloseAction, CosmicUI::Label(TEXT("Petition.Close")), 4.f);

		// The edicts' page: the rows scroll past RowsMaxHeight (the edicts of later milestones will not all fit).
		USizeBox* RowsHeight = WidgetTree->ConstructWidget<USizeBox>();
		RowsHeight->SetMaxDesiredHeight(RowsMaxHeight);
		Column->AddChildToVerticalBox(RowsHeight);
		EdictsPage = WidgetTree->ConstructWidget<UScrollBox>();
		RowsHeight->AddChild(EdictsPage);
		RowBox = WidgetTree->ConstructWidget<UVerticalBox>();
		EdictsPage->AddChild(RowBox);

		// The shares' page: a slider per field (whole steps 0..MaxShareWeight; the mouse follows the steps too; never takes the
		// keyboard focus, so Space and the number keys keep reaching the controller), the share it gives, then the lines and
		// [Share it so].
		SharesPage = WidgetTree->ConstructWidget<UVerticalBox>();
		SharesPage->SetVisibility(ESlateVisibility::Collapsed);
		Column->AddChildToVerticalBox(SharesPage);
		for (const FName Field : SliderFields)
		{
			UHorizontalBox* Line = WidgetTree->ConstructWidget<UHorizontalBox>();
			SharesPage->AddChildToVerticalBox(Line)->SetPadding(FMargin(0.f, 4.f));
			UTextBlock* Name = CosmicUI::MakeText(*WidgetTree, 13, false);
			Name->SetText(CosmicUI::Label(CosmicUI::ResearchLabelKey(Field)));
			Name->SetMinDesiredWidth(48.f);
			UHorizontalBoxSlot* NameSlot = Line->AddChildToHorizontalBox(Name);
			NameSlot->SetVerticalAlignment(VAlign_Center);
			USlider* Slider = WidgetTree->ConstructWidget<USlider>();
			// Before the first TakeWidget: SynchronizeProperties hands these to the Slate slider.
			Slider->MouseUsesStep = true;
			Slider->IsFocusable = false;
			Slider->SetMinValue(0.f);
			Slider->SetMaxValue(static_cast<float>(CosmicResearch::MaxShareWeight));
			Slider->SetStepSize(1.f);
			Slider->OnValueChanged.AddUniqueDynamic(this, &UCosmicEdictWidget::HandleSliderChanged);
			Slider->OnMouseCaptureBegin.AddUniqueDynamic(this, &UCosmicEdictWidget::HandleSliderCaptureBegin);
			Slider->OnMouseCaptureEnd.AddUniqueDynamic(this, &UCosmicEdictWidget::HandleSliderCaptureEnd);
			UHorizontalBoxSlot* SliderSlot = Line->AddChildToHorizontalBox(Slider);
			SliderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			SliderSlot->SetVerticalAlignment(VAlign_Center);
			SliderSlot->SetPadding(FMargin(6.f, 0.f));
			UTextBlock* Share = CosmicUI::MakeText(*WidgetTree, 13, false);
			Share->SetMinDesiredWidth(40.f);
			Share->SetJustification(ETextJustify::Right);
			UHorizontalBoxSlot* ShareSlot = Line->AddChildToHorizontalBox(Share);
			ShareSlot->SetVerticalAlignment(VAlign_Center);
			Sliders.Add(Slider);
			SliderShares.Add(Share);
		}
		WouldText = CosmicUI::MakeText(*WidgetTree, 13);
		SharesPage->AddChildToVerticalBox(WouldText)->SetPadding(FMargin(0.f, 10.f, 0.f, 2.f));
		NowText = CosmicUI::MakeText(*WidgetTree, 12);
		NowText->SetColorAndOpacity(FSlateColor(SummaryColor));
		SharesPage->AddChildToVerticalBox(NowText)->SetPadding(FMargin(0.f, 0.f, 0.f, 10.f));
		ApplyButton = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		ApplyButton->Setup(ApplyAction, 0.f, CosmicUI::Label(TEXT("Edict.Shares.Apply")));
		ApplyButton->OnAction.AddUObject(this, &UCosmicEdictWidget::HandleAction);
		ApplyButton->SetToolTipText(CosmicUI::Label(TEXT("Edict.Shares.Tip")));
		ApplyButton->SetIsEnabled(false);
		SharesPage->AddChildToVerticalBox(ApplyButton);
		SetVisibility(ESlateVisibility::Collapsed);
	}
	return Super::RebuildWidget();
}

void UCosmicEdictWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr)
	{
		Sim->OnSimTick.AddUniqueDynamic(this, &UCosmicEdictWidget::HandleSimTick);
		Sim->OnGalaxyGenerated.AddUniqueDynamic(this, &UCosmicEdictWidget::HandleGalaxyGenerated);
	}
}

void UCosmicEdictWidget::NativeDestruct()
{
	if (UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr)
	{
		Sim->OnSimTick.RemoveDynamic(this, &UCosmicEdictWidget::HandleSimTick);
		Sim->OnGalaxyGenerated.RemoveDynamic(this, &UCosmicEdictWidget::HandleGalaxyGenerated);
	}
	Super::NativeDestruct();
}

void UCosmicEdictWidget::HandleSimTick(int64 Tick)
{
	Refresh();
}

void UCosmicEdictWidget::HandleGalaxyGenerated(int32 Seed)
{
	// Another game: the window does not stay open over it.
	Close();
}

int32 UCosmicEdictWidget::RuledIndexNow() const
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || ACosmicPlayerController::IsRunEndedFor(*this))
	{
		return INDEX_NONE;
	}
	const FGalaxyState& State = Sim->GetState();
	const bool bRuler = State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler && !State.Player.RuledCivID.IsNone();
	return bRuler ? Sim->GetStateIndex().FindCiv(State, State.Player.RuledCivID) : INDEX_NONE;
}

void UCosmicEdictWidget::Open(ECosmicEdictTab InTab)
{
	if (!WidgetTree || !WidgetTree->RootWidget)
	{
		// The tree first: the window may be asked before the viewport built it.
		TakeWidget();
	}
	const int32 RuledIndex = RuledIndexNow();
	if (RuledIndex == INDEX_NONE)
	{
		return;
	}
	// An open window only turns to the page: a person's slider moves stay.
	if (bOpen)
	{
		Tab = InTab;
		Refresh();
		return;
	}
	bOpen = true;
	Tab = InTab;
	bMoved = false;
	bDragging = false;
	SetSlidersFromShares(RuledIndex);
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	// The first open ends the edict hint (the controller redraws the HUD, this window with it).
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->NoteEdictWindowOpened();
	}
	Refresh();
}

void UCosmicEdictWidget::Close()
{
	bOpen = false;
	bMoved = false;
	bDragging = false;
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCosmicEdictWidget::Toggle()
{
	if (bOpen)
	{
		Close();
	}
	else
	{
		Open(ECosmicEdictTab::Edicts);
	}
}

void UCosmicEdictWidget::Refresh()
{
	if (!bOpen || !RowBox)
	{
		return;
	}
	const int32 RuledIndex = RuledIndexNow();
	if (RuledIndex == INDEX_NONE)
	{
		// A deposition, an abdication or the end: no ruler's window stays.
		Close();
		return;
	}
	const UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	const FCivilizationState& Civ = Sim->GetState().Civs[RuledIndex];
	// The tabs: the page shown is greyed out; the edicts' tab counts the held edicts and the slots.
	const FText Count = FText::Format(CosmicUI::Label(TEXT("Edict.Count")), FText::AsNumber(CosmicEdicts::HeldCountOf(Civ, Sim->GetTables())),
		FText::AsNumber(CosmicEdicts::SlotsOf(Civ, Sim->GetRules())));
	if (!CosmicEdictWindowImpl::SameWords(EdictsTabButton->GetLabel(), Count))
	{
		EdictsTabButton->Setup(CosmicEdictWindowImpl::EdictsTabAction, 0.f, Count);
	}
	EdictsTabButton->SetIsEnabled(Tab != ECosmicEdictTab::Edicts);
	SharesTabButton->SetIsEnabled(Tab != ECosmicEdictTab::Shares);
	EdictsPage->SetVisibility(Tab == ECosmicEdictTab::Edicts ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	SharesPage->SetVisibility(Tab == ECosmicEdictTab::Shares ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	RefreshEdicts(RuledIndex);
	RefreshShares(RuledIndex);
}

void UCosmicEdictWidget::RefreshEdicts(int32 RuledIndex)
{
	using namespace CosmicEdictWindowImpl;
	using CosmicUI::Label;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	const FGalaxyState& State = Sim->GetState();
	const FCosmicDataTables& Tables = Sim->GetTables();
	const FCosmicRulesDef& Rules = Sim->GetRules();
	const FCivilizationState& Civ = State.Civs[RuledIndex];
	const ECosmicTimeUnit Unit = ACosmicPlayerController::TimeUnitFor(*this);
	// Whether Society research can still open a slot (the slots' reason then says so).
	const bool bSlotsCanGrow = NextSlotMarkOf(Civ, Rules).IsSet();

	// The rows the people may hold: those of its rung and below, and any it holds.
	TArray<FName> Wanted;
	for (const FName Row : Tables.GetEdictRowNames())
	{
		const FEdictDef* Def = Tables.FindEdict(Row);
		if (Def && (Civ.ActiveEdicts.Contains(Row) || static_cast<int32>(Def->RequiredTier) <= static_cast<int32>(Civ.Tier)))
		{
			Wanted.Add(Row);
		}
	}
	if (Wanted != ShownRows)
	{
		RowBox->ClearChildren();
		RowButtons.Reset();
		RowMarks.Reset();
		RowNames.Reset();
		RowSummaries.Reset();
		for (const FName Row : Wanted)
		{
			UHorizontalBox* Head = WidgetTree->ConstructWidget<UHorizontalBox>();
			RowBox->AddChildToVerticalBox(Head)->SetPadding(FMargin(0.f, RowNames.IsEmpty() ? 0.f : 10.f, 0.f, 2.f));
			UTextBlock* Name = CosmicUI::MakeText(*WidgetTree, 15, false);
			UHorizontalBoxSlot* NameSlot = Head->AddChildToHorizontalBox(Name);
			NameSlot->SetVerticalAlignment(VAlign_Bottom);
			UTextBlock* Mark = CosmicUI::MakeText(*WidgetTree, 12, false);
			Mark->SetColorAndOpacity(FSlateColor(QuietColor));
			Mark->SetText(Label(TEXT("Edict.Inherited")));
			Mark->SetVisibility(ESlateVisibility::Collapsed);
			UHorizontalBoxSlot* MarkSlot = Head->AddChildToHorizontalBox(Mark);
			MarkSlot->SetVerticalAlignment(VAlign_Bottom);
			MarkSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 1.f));
			UTextBlock* Summary = CosmicUI::MakeText(*WidgetTree, 12);
			Summary->SetColorAndOpacity(FSlateColor(SummaryColor));
			RowBox->AddChildToVerticalBox(Summary)->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));
			UCosmicActionButton* Button = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
			Button->Setup(Row, 0.f, FText::GetEmpty());
			Button->OnAction.AddUObject(this, &UCosmicEdictWidget::HandleRow);
			RowBox->AddChildToVerticalBox(Button);
			RowNames.Add(Name);
			RowMarks.Add(Mark);
			RowSummaries.Add(Summary);
			RowButtons.Add(Button);
		}
		ShownRows = Wanted;
	}
	ShownBlocks.SetNum(ShownRows.Num());
	for (int32 At = 0; At < ShownRows.Num(); ++At)
	{
		const FName Row = ShownRows[At];
		const FEdictDef* Def = Tables.FindEdict(Row);
		const bool bHeld = Civ.ActiveEdicts.Contains(Row);
		if (!SameWords(RowNames[At]->GetText(), Def->DisplayName))
		{
			RowNames[At]->SetText(Def->DisplayName);
		}
		if (!SameWords(RowSummaries[At]->GetText(), Def->Summary))
		{
			RowSummaries[At]->SetText(Def->Summary);
		}
		// The inherited edict's mark: while the people holds the row, enacted again or not.
		RowMarks[At]->SetVisibility((bHeld && Row == Rules.StartingEdict) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		FText ButtonLabel;
		FText Tip;
		if (bHeld)
		{
			ShownBlocks[At].Reset();
			ButtonLabel = Label(TEXT("Edict.Revoke"));
			Tip = CosmicUI::JoinLines({ TipOf(Tables, Row, Unit), Label(TEXT("Edict.TipRevoke")) });
			RowButtons[At]->SetIsEnabled(true);
		}
		else
		{
			// The command's own gate (the same function EnactEdict asks): the first reason greys the button.
			const CosmicEdicts::ECosmicEdictBlock Block = CosmicEdicts::EnactBlockOf(State, Sim->GetStateIndex(), Tables, Rules, RuledIndex, Row);
			ShownBlocks[At] = Block;
			const FText Cost = CostWordsOf(Def->EnactCost);
			const FText Enact = Cost.IsEmpty() ? Label(TEXT("Edict.EnactFree")) : FText::Format(Label(TEXT("Edict.Enact")), Cost);
			const FText Suffix = SuffixOf(Block, Def);
			ButtonLabel = Suffix.IsEmpty() ? Enact : FText::Format(INVTEXT("{0}  {1}"), Enact, Suffix);
			Tip = CosmicUI::JoinLines({ ReasonOf(Block, Def, bSlotsCanGrow), TipOf(Tables, Row, Unit) });
			RowButtons[At]->SetIsEnabled(Block == CosmicEdicts::ECosmicEdictBlock::None);
		}
		if (!SameWords(RowButtons[At]->GetLabel(), ButtonLabel))
		{
			RowButtons[At]->Setup(Row, 0.f, ButtonLabel);
		}
		// A tooltip set again is a new tooltip, and an open one would close: set only when the words change.
		if (!SameWords(RowButtons[At]->GetToolTipText(), Tip))
		{
			RowButtons[At]->SetToolTipText(Tip);
		}
	}
}

void UCosmicEdictWidget::SetSlidersFromShares(int32 RuledIndex)
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || !Sim->GetState().Civs.IsValidIndex(RuledIndex))
	{
		return;
	}
	const TMap<FName, float>& Focus = Sim->GetState().Civs[RuledIndex].ResearchFocus;
	// The weights of the shares now: share / (1 - transcendence) x MaxShareWeight, rounded (B2 section 2.8).
	const float Rest = 1.f - Focus.FindRef(CosmicResearch::TranscendenceField());
	bSettingSliders = true;
	for (int32 At = 0; At < Sliders.Num(); ++At)
	{
		const float Share = Focus.FindRef(SliderFields[At]);
		const int32 Weight = Rest > 0.f ? FMath::Clamp(FMath::RoundToInt32(Share / Rest * static_cast<float>(CosmicResearch::MaxShareWeight)), 0, CosmicResearch::MaxShareWeight) : 0;
		if (FMath::RoundToInt32(Sliders[At]->GetValue()) != Weight)
		{
			Sliders[At]->SetValue(static_cast<float>(Weight));
		}
	}
	bSettingSliders = false;
}

TMap<FName, int32> UCosmicEdictWidget::WeightsNow() const
{
	TMap<FName, int32> Weights;
	for (int32 At = 0; At < Sliders.Num(); ++At)
	{
		Weights.Add(SliderFields[At], FMath::Clamp(FMath::RoundToInt32(Sliders[At]->GetValue()), 0, CosmicResearch::MaxShareWeight));
	}
	return Weights;
}

int32 UCosmicEdictWidget::GetSliderWeight(FName Field) const
{
	for (int32 At = 0; At < Sliders.Num(); ++At)
	{
		if (SliderFields[At] == Field)
		{
			return FMath::RoundToInt32(Sliders[At]->GetValue());
		}
	}
	return INDEX_NONE;
}

void UCosmicEdictWidget::RefreshShares(int32 RuledIndex)
{
	using namespace CosmicEdictWindowImpl;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	const TMap<FName, float>& Current = Sim->GetState().Civs[RuledIndex].ResearchFocus;
	// A slider a person holds or has moved is never set from here; nor are sliders whose weights give the shares now (the
	// weights a person just applied stay where they were put - the same weights give the same bits). Shares changed by
	// another way (a focus button, a card's answer) set them again.
	if (!bMoved && !bDragging)
	{
		bool bTaken = false;
		const bool bGivesNow = WouldOf(Current, WeightsNow(), bTaken).OrderIndependentCompareEqual(Current) && bTaken;
		if (!bGivesNow)
		{
			SetSlidersFromShares(RuledIndex);
		}
	}
	bool bFits = false;
	const TMap<FName, float> Would = WouldOf(Current, WeightsNow(), bFits);
	// Beside each slider: the share now until a person moves one, then the share the sliders would give (0 for three
	// sliders at 0, which share nothing).
	for (int32 At = 0; At < SliderShares.Num(); ++At)
	{
		const float Value = !bMoved ? Current.FindRef(SliderFields[At]) : (bFits ? Would.FindRef(SliderFields[At]) : 0.f);
		const FText Share = CosmicUI::Percent(Value);
		if (!SameWords(SliderShares[At]->GetText(), Share))
		{
			SliderShares[At]->SetText(Share);
		}
	}
	// The 'would' line after a person's move only (hidden, not collapsed: [Share it so] does not move on the first drag);
	// weights that share nothing say so.
	const FText WouldLine = !bMoved ? FText::GetEmpty()
		: (bFits ? FText::Format(CosmicUI::Label(TEXT("Edict.Shares.Would")), CosmicUI::FocusSharesOf(Would)) : CosmicUI::Label(TEXT("Edict.Shares.Zero")));
	if (!SameWords(WouldText->GetText(), WouldLine))
	{
		WouldText->SetText(WouldLine);
	}
	WouldText->SetVisibility(bMoved ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	const FText NowLine = FText::Format(CosmicUI::Label(TEXT("Edict.Shares.Now")), CosmicUI::FocusSharesOf(Current));
	if (!SameWords(NowText->GetText(), NowLine))
	{
		NowText->SetText(NowLine);
	}
	// Only after a person's move, and only for weights the command takes.
	ApplyButton->SetIsEnabled(bMoved && bFits);
}

void UCosmicEdictWidget::HandleSliderChanged(float Value)
{
	if (bSettingSliders)
	{
		return;
	}
	bMoved = true;
	const int32 RuledIndex = RuledIndexNow();
	if (bOpen && RuledIndex != INDEX_NONE)
	{
		RefreshShares(RuledIndex);
	}
}

void UCosmicEdictWidget::HandleSliderCaptureBegin()
{
	bDragging = true;
}

void UCosmicEdictWidget::HandleSliderCaptureEnd()
{
	bDragging = false;
}

void UCosmicEdictWidget::MoveSlider(FName Field, int32 Weight)
{
	for (int32 At = 0; At < Sliders.Num(); ++At)
	{
		if (SliderFields[At] == Field)
		{
			// The path of a drag: the slider's value changes and its OnValueChanged reaches HandleSliderChanged.
			Sliders[At]->SetValue(static_cast<float>(FMath::Clamp(Weight, 0, CosmicResearch::MaxShareWeight)));
		}
	}
}

void UCosmicEdictWidget::PressApply()
{
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer());
	if (!Controller || !ApplyButton || !ApplyButton->GetIsEnabled())
	{
		return;
	}
	// One command per press; the sliders then follow the shares again.
	Controller->SetResearchShares(WeightsNow());
	bMoved = false;
	Refresh();
}

void UCosmicEdictWidget::PressRow(FName Row)
{
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer());
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	const int32 RuledIndex = RuledIndexNow();
	if (!Controller || !Sim || RuledIndex == INDEX_NONE)
	{
		return;
	}
	if (Sim->GetState().Civs[RuledIndex].ActiveEdicts.Contains(Row))
	{
		Controller->RevokeEdict(Row);
	}
	else
	{
		Controller->EnactEdict(Row);
	}
	Refresh();
}

UCosmicActionButton* UCosmicEdictWidget::GetRowButton(FName Row) const
{
	const int32 At = ShownRows.IndexOfByKey(Row);
	return RowButtons.IsValidIndex(At) ? RowButtons[At].Get() : nullptr;
}

const UTextBlock* UCosmicEdictWidget::GetRowMark(FName Row) const
{
	const int32 At = ShownRows.IndexOfByKey(Row);
	return RowMarks.IsValidIndex(At) ? RowMarks[At].Get() : nullptr;
}

TOptional<CosmicEdicts::ECosmicEdictBlock> UCosmicEdictWidget::GetShownBlock(FName Row) const
{
	const int32 At = ShownRows.IndexOfByKey(Row);
	return ShownBlocks.IsValidIndex(At) ? ShownBlocks[At] : TOptional<CosmicEdicts::ECosmicEdictBlock>();
}

void UCosmicEdictWidget::HandleRow(FName Row, float Value)
{
	PressRow(Row);
}

void UCosmicEdictWidget::HandleAction(FName Action, float Value)
{
	using namespace CosmicEdictWindowImpl;

	if (Action == CloseAction)
	{
		Close();
	}
	else if (Action == ApplyAction)
	{
		PressApply();
	}
	else if (Action == EdictsTabAction || Action == SharesTabAction)
	{
		Tab = Action == EdictsTabAction ? ECosmicEdictTab::Edicts : ECosmicEdictTab::Shares;
		Refresh();
	}
}

FReply UCosmicEdictWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Only the window's box reaches here (the widget and its root are self-hit-test-invisible).
	return FReply::Handled();
}

FReply UCosmicEdictWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

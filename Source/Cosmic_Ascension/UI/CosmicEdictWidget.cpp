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
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/GameInstance.h"
#include "Framework/Text/TextLayout.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Layout/Margin.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicExistenceBricks.h"
#include "Sim/CosmicResearch.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Data/CosmicConditionTypes.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicNextRung.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicGameMode.h"
#include "World/CosmicMapLook.h"

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
	const FName PeoplesTabAction(TEXT("tab_peoples"));
	const FName WithdrawAction(TEXT("withdraw"));
	// A people's hostility toward us: a muted red, read beside the grey band.
	const FLinearColor HostileColor(0.9f, 0.45f, 0.4f);
	// The value of a people's button: a stance at 0..2, a treaty at TreatyBase + 0..1.
	constexpr float TreatyBase = 3.f;
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
		// Nearer 0 than the last digit shown: a plain 0 (the formatter keeps the sign of -0.001, and of -0.0, as '-0').
		return FText::AsNumber((Value >= -0.005f && Value <= 0.005f) ? 0.f : Value, &Options);
	}

	// "+20%" or "-20%": a factor told as the change it makes.
	FText SignedPercent(float Factor)
	{
		const float Change = Factor - 1.f;
		return Change > 0.f ? FText::Format(INVTEXT("+{0}"), CosmicUI::Percent(Change)) : CosmicUI::Percent(Change);
	}

	// "+0.24" or "-0.12": a term of a view, signed (up to two decimals; a plain 0 for 0.005 or less either way).
	FText Signed(float Value)
	{
		return Value > 0.005f ? FText::Format(INVTEXT("+{0}"), Amount(Value)) : Amount(Value);
	}

	// The live view beside a treaty's bar - the one place a number of it is told (the band's tooltip tells its terms): it
	// moves with the tick, so it is told to a tenth (it changes seldom, and an open tooltip seldom closes), cut down - a view
	// below the bar never reads at it -, and never below a bar it reaches (a bar that is no tenth).
	FText ViewAgainstBarOf(float View, float Bar)
	{
		const float Tenth = FMath::FloorToFloat(View * 10.f) / 10.f;
		return Signed(View >= Bar ? FMath::Max(Tenth, Bar) : Tenth);
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
	// The fleet's growth and the treaties' acceptance (B3), read as the rules read them.
	if (Def->FleetGrowthMultiplier > 0.f && Def->FleetGrowthMultiplier <= TNumericLimits<float>::Max() && Def->FleetGrowthMultiplier != 1.f)
	{
		Lines.Add(FText::Format(Label(TEXT("Edict.Effect.FleetGrowth")), Amount(Def->FleetGrowthMultiplier)));
	}
	if (Def->TreatyAcceptBonus > 0.f && Def->TreatyAcceptBonus <= TNumericLimits<float>::Max())
	{
		Lines.Add(FText::Format(Label(TEXT("Edict.Effect.TreatyAccept")), Amount(Def->TreatyAcceptBonus)));
	}
	// Whether a hazard's spawn line stands (B6b: a galaxy law's Edict.TipWide follows it).
	bool bSpawn = false;
	// The pressure's drain on Stability (B4), read as CosmicEdicts::PressureDrainMultiplierOf reads it.
	if (Def->PressureDrainMultiplier > 0.f && Def->PressureDrainMultiplier <= TNumericLimits<float>::Max() && Def->PressureDrainMultiplier != 1.f)
	{
		Lines.Add(FText::Format(Label(TEXT("Edict.Effect.PressureDrain")), Amount(Def->PressureDrainMultiplier)));
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
			bSpawn = true;
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
	// A galaxy law's hazard lines reach the whole home galaxy (B6b): said under them; its cost and other effects stay the
	// people's.
	if (CosmicEdicts::IsGalaxyLaw(Tables, Row) && (bSpawn || bCountdown))
	{
		Lines.Add(Label(TEXT("Edict.TipWide")));
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
		PeoplesTabButton = AddControl(PeoplesTabAction, CosmicUI::Label(TEXT("Diplomacy.Tab")), 4.f);
		TabLine = Tabs;
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

		// The peoples' page (B3): the fleet and the friendly slots, then a row per people met; it scrolls as the edicts' does.
		USizeBox* PeoplesHeight = WidgetTree->ConstructWidget<USizeBox>();
		PeoplesHeight->SetMaxDesiredHeight(RowsMaxHeight);
		Column->AddChildToVerticalBox(PeoplesHeight);
		PeoplesPage = WidgetTree->ConstructWidget<UScrollBox>();
		PeoplesPage->SetVisibility(ESlateVisibility::Collapsed);
		PeoplesHeight->AddChild(PeoplesPage);
		UVerticalBox* PeoplesColumn = WidgetTree->ConstructWidget<UVerticalBox>();
		PeoplesPage->AddChild(PeoplesColumn);
		FleetText = CosmicUI::MakeText(*WidgetTree, 13);
		PeoplesColumn->AddChildToVerticalBox(FleetText);
		TargetText = CosmicUI::MakeText(*WidgetTree, 12);
		TargetText->SetColorAndOpacity(FSlateColor(SummaryColor));
		PeoplesColumn->AddChildToVerticalBox(TargetText)->SetPadding(FMargin(0.f, 2.f, 0.f, 0.f));
		WithdrawButton = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		WithdrawButton->Setup(WithdrawAction, 0.f, CosmicUI::Label(TEXT("Guard.Withdraw")));
		WithdrawButton->OnAction.AddUObject(this, &UCosmicEdictWidget::HandleAction);
		WithdrawButton->SetVisibility(ESlateVisibility::Collapsed);
		UVerticalBoxSlot* WithdrawSlot = PeoplesColumn->AddChildToVerticalBox(WithdrawButton);
		WithdrawSlot->SetHorizontalAlignment(HAlign_Left);
		WithdrawSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));
		SlotsText = CosmicUI::MakeText(*WidgetTree, 13);
		PeoplesColumn->AddChildToVerticalBox(SlotsText)->SetPadding(FMargin(0.f, 6.f, 0.f, 4.f));
		NoPeoplesText = CosmicUI::MakeText(*WidgetTree, 12);
		NoPeoplesText->SetColorAndOpacity(FSlateColor(QuietColor));
		NoPeoplesText->SetText(CosmicUI::Label(TEXT("Diplomacy.None")));
		NoPeoplesText->SetVisibility(ESlateVisibility::Collapsed);
		PeoplesColumn->AddChildToVerticalBox(NoPeoplesText)->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
		PeopleBox = WidgetTree->ConstructWidget<UVerticalBox>();
		PeoplesColumn->AddChildToVerticalBox(PeopleBox);
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
	PeoplesTabButton->SetIsEnabled(Tab != ECosmicEdictTab::Peoples);
	EdictsPage->SetVisibility(Tab == ECosmicEdictTab::Edicts ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	SharesPage->SetVisibility(Tab == ECosmicEdictTab::Shares ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	PeoplesPage->SetVisibility(Tab == ECosmicEdictTab::Peoples ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	RefreshEdicts(RuledIndex);
	RefreshShares(RuledIndex);
	// The peoples' page only while it shows: its rows read the borders and the views of every people met (the tab's press
	// and Open redraw it).
	if (Tab == ECosmicEdictTab::Peoples)
	{
		RefreshPeoples(RuledIndex);
	}
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
		RowWideMarks.Reset();
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
			// The galaxy law's mark (B6b): the same grey, after the inherited one.
			UTextBlock* WideMark = CosmicUI::MakeText(*WidgetTree, 12, false);
			WideMark->SetColorAndOpacity(FSlateColor(QuietColor));
			WideMark->SetText(Label(TEXT("Edict.Wide")));
			WideMark->SetVisibility(ESlateVisibility::Collapsed);
			UHorizontalBoxSlot* WideMarkSlot = Head->AddChildToHorizontalBox(WideMark);
			WideMarkSlot->SetVerticalAlignment(VAlign_Bottom);
			WideMarkSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 1.f));
			UTextBlock* Summary = CosmicUI::MakeText(*WidgetTree, 12);
			Summary->SetColorAndOpacity(FSlateColor(SummaryColor));
			RowBox->AddChildToVerticalBox(Summary)->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));
			UCosmicActionButton* Button = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
			Button->Setup(Row, 0.f, FText::GetEmpty());
			Button->OnAction.AddUObject(this, &UCosmicEdictWidget::HandleRow);
			RowBox->AddChildToVerticalBox(Button);
			RowNames.Add(Name);
			RowMarks.Add(Mark);
			RowWideMarks.Add(WideMark);
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
		// The galaxy law's mark: on every row of Scope HomeGalaxy, held or not.
		RowWideMarks[At]->SetVisibility(CosmicEdicts::IsGalaxyLaw(Tables, Row) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
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

const UTextBlock* UCosmicEdictWidget::GetRowWideMark(FName Row) const
{
	const int32 At = ShownRows.IndexOfByKey(Row);
	return RowWideMarks.IsValidIndex(At) ? RowWideMarks[At].Get() : nullptr;
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
	else if (Action == WithdrawAction)
	{
		PressWithdraw();
	}
	else if (Action == EdictsTabAction || Action == SharesTabAction || Action == PeoplesTabAction)
	{
		Tab = Action == EdictsTabAction ? ECosmicEdictTab::Edicts : (Action == SharesTabAction ? ECosmicEdictTab::Shares : ECosmicEdictTab::Peoples);
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

// ---- The peoples' page (B3 header section 4) ----

FGameplayTag UCosmicEdictWidget::StanceAt(int32 At)
{
	switch (At)
	{
	case 0: return CosmicTags::StanceFriendly();
	case 2: return CosmicTags::StanceHostile();
	default: return FGameplayTag();
	}
}

FGameplayTag UCosmicEdictWidget::TreatyAt(int32 At)
{
	return At == 0 ? CosmicTags::TreatyTrade() : CosmicTags::TreatyNonAggression();
}

FText UCosmicEdictWidget::ViewWordOf(float View)
{
	static const TCHAR* const Keys[] = { TEXT("Diplomacy.View.0"), TEXT("Diplomacy.View.1"), TEXT("Diplomacy.View.2"), TEXT("Diplomacy.View.3"), TEXT("Diplomacy.View.4") };
	const int32 Band = FMath::Clamp(CosmicDiplomacy::ViewBandOf(View), 0, static_cast<int32>(UE_ARRAY_COUNT(Keys)) - 1);
	return CosmicUI::Label(Keys[Band]);
}

FText UCosmicEdictWidget::ViewTipOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, int32 Viewer, int32 Viewed)
{
	using namespace CosmicEdictWindowImpl;
	using CosmicUI::Label;

	if (!State.Civs.IsValidIndex(Viewer) || !State.Civs.IsValidIndex(Viewed) || Viewer == Viewed)
	{
		return FText::GetEmpty();
	}
	const FCivilizationState& Them = State.Civs[Viewer];
	const FCivilizationState& Us = State.Civs[Viewed];
	// The terms as CosmicDiplomacy::TargetRelationOf sums them: the ethics, the stance we hold toward them, the treaties, and the
	// bricks we carry (B7x - CosmicBricks::RelationTargetTermOf; shown only when not 0).
	const float Ethics = -Rules.RelationEthicsWeight * CosmicDiplomacy::EthicsDistanceOf(Them.Ethics, Us.Ethics);
	const float Treaties = Rules.RelationTreatyTerm * static_cast<float>(CosmicDiplomacy::TreatyCountOf(Them, Us.CivID));
	TArray<FText> Lines;
	if (CosmicDiplomacy::Borders(State, Index, Viewer, Viewed))
	{
		// The stance's term read where the rule reads it (not the target less the others: their rounding would show as +-0).
		const FGameplayTag Ours = CosmicDiplomacy::StanceOf(Us, Them.CivID);
		const int32 TermAt = Ours == CosmicTags::StanceFriendly() ? 0 : (Ours == CosmicTags::StanceHostile() ? 1 : INDEX_NONE);
		const float Stance = (Rules.RelationStanceTerms.IsValidIndex(TermAt) && FMath::IsFinite(Rules.RelationStanceTerms[TermAt])) ? Rules.RelationStanceTerms[TermAt] : 0.f;
		Lines.Add(FText::Format(Label(TEXT("Diplomacy.TipTarget")), Signed(CosmicDiplomacy::TargetRelationOf(State, Rules, Viewer, Viewed))));
		Lines.Add(FText::Format(Label(TEXT("Diplomacy.TipEthics")), Signed(Ethics)));
		Lines.Add(FText::Format(Label(TEXT("Diplomacy.TipStance")), Signed(Stance)));
		Lines.Add(FText::Format(Label(TEXT("Diplomacy.TipTreaties")), Signed(Treaties)));
		const float Bricks = CosmicBricks::RelationTargetTermOf(Us, Rules);
		if (Bricks != 0.f)
		{
			Lines.Add(FText::Format(Label(TEXT("Diplomacy.TipBricks")), Signed(Bricks)));
		}
	}
	else
	{
		Lines.Add(FText::Format(Label(TEXT("Diplomacy.TipEthics")), Signed(Ethics)));
		Lines.Add(FText::Format(Label(TEXT("Diplomacy.TipTreaties")), Signed(Treaties)));
		Lines.Add(Label(TEXT("Diplomacy.TipFar")));
	}
	return CosmicUI::JoinLines(Lines);
}

FText UCosmicEdictWidget::StanceReasonOf(CosmicDiplomacy::ECosmicStanceBlock Block)
{
	using CosmicDiplomacy::ECosmicStanceBlock;
	using CosmicUI::Label;

	switch (Block)
	{
	case ECosmicStanceBlock::NotMet:
		return Label(TEXT("Diplomacy.Block.NotMet"));
	case ECosmicStanceBlock::Same:
		return Label(TEXT("Diplomacy.TipNow"));
	case ECosmicStanceBlock::FriendlySlots:
		return Label(TEXT("Diplomacy.Block.FriendlySlots"));
	case ECosmicStanceBlock::Tier:
		return Label(TEXT("Diplomacy.Block.Tier"));
	case ECosmicStanceBlock::NonAggression:
		return Label(TEXT("Diplomacy.Block.NonAggression"));
	case ECosmicStanceBlock::None:
	case ECosmicStanceBlock::NotRuler:
	case ECosmicStanceBlock::NoCiv:
	case ECosmicStanceBlock::Self:
	case ECosmicStanceBlock::Handler:
		break;
	}
	return FText::GetEmpty();
}

FText UCosmicEdictWidget::TreatyReasonOf(CosmicDiplomacy::ECosmicTreatyBlock Block)
{
	using CosmicDiplomacy::ECosmicTreatyBlock;
	using CosmicUI::Label;

	switch (Block)
	{
	case ECosmicTreatyBlock::NotMet:
		return Label(TEXT("Diplomacy.Treaty.Block.NotMet"));
	case ECosmicTreatyBlock::Tier:
		return Label(TEXT("Diplomacy.Treaty.Block.Tier"));
	case ECosmicTreatyBlock::Hostile:
		return Label(TEXT("Diplomacy.Treaty.Block.Hostile"));
	case ECosmicTreatyBlock::Refused:
		return Label(TEXT("Diplomacy.Treaty.Block.Refused"));
	case ECosmicTreatyBlock::Cost:
		return Label(TEXT("Diplomacy.Treaty.Block.Cost"));
	case ECosmicTreatyBlock::None:
	case ECosmicTreatyBlock::NotRuler:
	case ECosmicTreatyBlock::NoCiv:
	case ECosmicTreatyBlock::Self:
	case ECosmicTreatyBlock::Held:
	case ECosmicTreatyBlock::Handler:
		break;
	}
	return FText::GetEmpty();
}

FText UCosmicEdictWidget::SiegeReasonOf(CosmicConflict::ECosmicSiegeBlock Block, bool bAbove)
{
	using CosmicConflict::ECosmicSiegeBlock;
	using CosmicUI::Label;

	if (bAbove && Block == ECosmicSiegeBlock::NoFleet)
	{
		return Label(TEXT("Siege.Block.Above"));
	}
	switch (Block)
	{
	case ECosmicSiegeBlock::Capital:
		return Label(TEXT("Siege.Block.Capital"));
	case ECosmicSiegeBlock::LastSettled:
		return Label(TEXT("Siege.Block.LastSettled"));
	case ECosmicSiegeBlock::NotAdjacent:
		return Label(TEXT("Siege.Block.NotAdjacent"));
	case ECosmicSiegeBlock::Besieged:
		return Label(TEXT("Siege.Block.Besieged"));
	case ECosmicSiegeBlock::RealmCap:
		return Label(TEXT("Siege.Block.RealmCap"));
	case ECosmicSiegeBlock::NoFleet:
		return Label(TEXT("Siege.Block.NoFleet"));
	case ECosmicSiegeBlock::NonAggression:
		return Label(TEXT("Siege.Block.NonAggression"));
	case ECosmicSiegeBlock::NotHostile:
		return Label(TEXT("Siege.Block.NotHostile"));
	case ECosmicSiegeBlock::None:
	case ECosmicSiegeBlock::NotRuler:
	case ECosmicSiegeBlock::Sanctuary:
	case ECosmicSiegeBlock::NoSystem:
	case ECosmicSiegeBlock::Dead:
	case ECosmicSiegeBlock::Unowned:
	case ECosmicSiegeBlock::Own:
	case ECosmicSiegeBlock::NotSettled:
	case ECosmicSiegeBlock::Handler:
		break;
	}
	return FText::GetEmpty();
}

FText UCosmicEdictWidget::SiegeSuffixOf(CosmicConflict::ECosmicSiegeBlock Block, bool bAbove)
{
	using CosmicConflict::ECosmicSiegeBlock;
	using CosmicUI::Label;

	if (bAbove && Block == ECosmicSiegeBlock::NoFleet)
	{
		return Label(TEXT("Siege.Suffix.Above"));
	}
	switch (Block)
	{
	case ECosmicSiegeBlock::Capital:
		return Label(TEXT("Siege.Suffix.Capital"));
	case ECosmicSiegeBlock::LastSettled:
		return Label(TEXT("Siege.Suffix.LastSettled"));
	case ECosmicSiegeBlock::NotAdjacent:
		return Label(TEXT("Siege.Suffix.NotAdjacent"));
	case ECosmicSiegeBlock::Besieged:
		return Label(TEXT("Siege.Suffix.Besieged"));
	case ECosmicSiegeBlock::RealmCap:
		return Label(TEXT("Siege.Suffix.RealmCap"));
	case ECosmicSiegeBlock::NoFleet:
		return Label(TEXT("Siege.Suffix.NoFleet"));
	case ECosmicSiegeBlock::NonAggression:
		return Label(TEXT("Siege.Suffix.NonAggression"));
	case ECosmicSiegeBlock::NotHostile:
		return Label(TEXT("Siege.Suffix.NotHostile"));
	case ECosmicSiegeBlock::None:
	case ECosmicSiegeBlock::NotRuler:
	case ECosmicSiegeBlock::Sanctuary:
	case ECosmicSiegeBlock::NoSystem:
	case ECosmicSiegeBlock::Dead:
	case ECosmicSiegeBlock::Unowned:
	case ECosmicSiegeBlock::Own:
	case ECosmicSiegeBlock::NotSettled:
	case ECosmicSiegeBlock::Handler:
		break;
	}
	return FText::GetEmpty();
}

FText UCosmicEdictWidget::AmountOf(float Value)
{
	return CosmicEdictWindowImpl::Amount(Value);
}

TOptional<int32> UCosmicEdictWidget::NextFriendlySlotRungOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules)
{
	const int32 Now = CosmicDiplomacy::FriendlySlotsOf(Civ, Rules);
	// The fifth rung's slot comes after the run's end (B6b decision 27): it is never told.
	for (int32 At = static_cast<int32>(Civ.Tier) + 1; At < static_cast<int32>(EAscensionTier::Tier5_EternalOrder) && Rules.FriendlySlotsByTier.IsValidIndex(At); ++At)
	{
		if (FMath::Max(0, Rules.FriendlySlotsByTier[At]) > Now)
		{
			return At;
		}
	}
	return TOptional<int32>();
}

FText UCosmicEdictWidget::RungWordsOf(const FCosmicKnownView& View, const FGalaxyState& State, int32 CivIndex)
{
	// Known: met, and shown - a people three rungs or more above the viewer is not (rule 14).
	if (!State.Civs.IsValidIndex(CivIndex) || !View.IsMet(CivIndex) || View.ShownOrderOf(CivIndex) == INDEX_NONE)
	{
		return FText::GetEmpty();
	}
	return FText::Format(INVTEXT("{0} {1}"), CosmicUI::Label(TEXT("Panel.Tier")), FText::AsNumber(static_cast<int32>(State.Civs[CivIndex].Tier) + 1));
}

const UWidget* UCosmicEdictWidget::GetEdictsPage() const
{
	return EdictsPage;
}

const UWidget* UCosmicEdictWidget::GetSharesPage() const
{
	return SharesPage;
}

const UWidget* UCosmicEdictWidget::GetPeoplesPage() const
{
	return PeoplesPage;
}

FText UCosmicEdictWidget::OddsWordOf(float OursOdds)
{
	switch (CosmicConflict::OddsBandOf(OursOdds))
	{
	case 2: return CosmicUI::Label(TEXT("Siege.Band.Ahead"));
	case 0: return CosmicUI::Label(TEXT("Siege.Band.Behind"));
	default: return CosmicUI::Label(TEXT("Siege.Band.Even"));
	}
}

int64 UCosmicEdictWidget::SiegeTicksLeftOf(const FCivilizationState& Attacker, int64 Tick, const FCosmicRulesDef& Rules)
{
	return FMath::Max<int64>(0, CosmicConflict::SiegeEndTickOf(Attacker, Rules) - Tick);
}

void UCosmicEdictWidget::RefreshPeoples(int32 RuledIndex)
{
	using namespace CosmicEdictWindowImpl;
	using CosmicUI::Label;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	const FGalaxyState& State = Sim->GetState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicDataTables& Tables = Sim->GetTables();
	const FCosmicRulesDef& Rules = Sim->GetRules();
	const FCivilizationState& Civ = State.Civs[RuledIndex];
	const ECosmicTimeUnit Unit = ACosmicPlayerController::TimeUnitFor(*this);
	// What the viewer knows (rule 14): the peoples met and shown, in the meeting order.
	const FCosmicKnownView View = ACosmicPlayerController::KnownViewFor(*this);

	auto SetWords = [](UTextBlock* Text, const FText& Words)
	{
		if (!SameWords(Text->GetText(), Words))
		{
			Text->SetText(Words);
		}
	};
	auto SetTip = [](UWidget* Widget, const FText& Tip)
	{
		// A tooltip set again is a new tooltip, and an open one would close: set only when the words change.
		if (!SameWords(Widget->GetToolTipText(), Tip))
		{
			Widget->SetToolTipText(Tip);
		}
	};
	// A siege line: the system, the time it has left (a length: the line says 'time left'), our side's band.
	auto SiegeWords = [&](const TCHAR* Key, int32 Attacker, int32 SystemIndex, bool bOurs)
	{
		const float Odds = CosmicConflict::SiegeOddsOf(State, Index, Rules, Attacker, SystemIndex);
		FFormatNamedArguments Args;
		Args.Add(TEXT("System"), State.Systems[SystemIndex].DisplayName);
		Args.Add(TEXT("Time"), CosmicUI::DurationLabel(SiegeTicksLeftOf(State.Civs[Attacker], State.Tick, Rules), Unit));
		Args.Add(TEXT("Band"), OddsWordOf(bOurs ? Odds : 1.f - Odds));
		return FText::Format(Label(Key), Args);
	};

	const FText PerTick = CosmicUI::DurationLabel(1, Unit);

	// The head: the fleet, its target and the friendly slots.
	const float Cap = CosmicConflict::FleetCapOf(State, Index, RuledIndex, Rules);
	SetWords(FleetText, (Cap > 0.f || Civ.FleetPower > 0.f) ? FText::Format(Label(TEXT("Siege.Fleet")), Amount(Civ.FleetPower), Amount(Cap)) : Label(TEXT("Guard.NoFleet")));
	// How the fleet grows, and its upkeep a size (the rate, not the sum: the sum moves with the fleet every tick).
	const float UpkeepPerPower = (Rules.FleetUpkeepEnergyPerPower > 0.f && Rules.FleetUpkeepEnergyPerPower <= TNumericLimits<float>::Max()) ? Rules.FleetUpkeepEnergyPerPower : 0.f;
	SetTip(FleetText, CosmicUI::JoinLines({ Label(TEXT("Siege.FleetTip")), FText::Format(Label(TEXT("Siege.FleetUpkeep")), Amount(UpkeepPerPower), PerTick) }));
	const int32 Target = Civ.FleetTargetSystemID.IsNone() ? INDEX_NONE : Index.FindSystem(State, Civ.FleetTargetSystemID);
	FText TargetWords;
	if (State.Systems.IsValidIndex(Target) && State.Systems[Target].OwnerCivID == Civ.CivID)
	{
		TargetWords = FText::FormatNamed(Label(TEXT("Guard.Line")), TEXT("System"), State.Systems[Target].DisplayName);
	}
	else if (State.Systems.IsValidIndex(Target))
	{
		TargetWords = SiegeWords(TEXT("Siege.Line"), RuledIndex, Target, true);
	}
	else if (Cap > 0.f || Civ.FleetPower > 0.f)
	{
		TargetWords = Label(TEXT("Guard.Even"));
	}
	SetWords(TargetText, TargetWords);
	TargetText->SetVisibility(TargetWords.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	WithdrawButton->SetVisibility(State.Systems.IsValidIndex(Target) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	// A withdrawal ends a guard, or a siege with its count.
	const bool bBesieging = State.Systems.IsValidIndex(Target) && State.Systems[Target].OwnerCivID != Civ.CivID;
	SetTip(WithdrawButton, Label(bBesieging ? TEXT("Siege.WithdrawTip") : TEXT("Guard.WithdrawTip")));
	SetWords(SlotsText, FText::Format(Label(TEXT("Diplomacy.Slots")), FText::AsNumber(CosmicDiplomacy::FriendlyCountOf(Civ)),
		FText::AsNumber(CosmicDiplomacy::FriendlySlotsOf(Civ, Rules))));
	const TOptional<int32> NextSlotRung = NextFriendlySlotRungOf(Civ, Rules);
	SetTip(SlotsText, CosmicUI::JoinLines({ Label(TEXT("Diplomacy.SlotsTip")), NextSlotRung.IsSet()
		? FText::Format(Label(TEXT("Diplomacy.SlotsNext")), FText::AsNumber(*NextSlotRung + 1)) : Label(TEXT("Diplomacy.SlotsAll")) }));

	// The peoples met and shown, by their meeting order.
	TArray<int32> Wanted;
	for (int32 Other = 0; Other < State.Civs.Num(); ++Other)
	{
		if (Other != RuledIndex && View.IsMet(Other) && View.ShownOrderOf(Other) != INDEX_NONE)
		{
			Wanted.Add(Other);
		}
	}
	Wanted.Sort([&View](int32 A, int32 B) { return View.ShownOrderOf(A) < View.ShownOrderOf(B); });
	TArray<FName> WantedIDs;
	for (const int32 Other : Wanted)
	{
		WantedIDs.Add(State.Civs[Other].CivID);
	}
	NoPeoplesText->SetVisibility(Wanted.IsEmpty() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (WantedIDs != ShownPeoples)
	{
		PeopleBox->ClearChildren();
		PeopleRows.Reset();
		PeopleNames.Reset();
		PeopleRungs.Reset();
		PeopleBands.Reset();
		PeopleHostilities.Reset();
		PeopleSieges.Reset();
		StanceButtons.Reset();
		TreatyButtons.Reset();
		for (const FName CivID : WantedIDs)
		{
			UVerticalBox* Row = WidgetTree->ConstructWidget<UVerticalBox>();
			PeopleBox->AddChildToVerticalBox(Row)->SetPadding(FMargin(0.f, 10.f, 0.f, 0.f));
			// The first line: the name and the band of their view of us; their hostility on a line of its own under it (a long
			// name, the band and the mark would not fit one line).
			UHorizontalBox* Head = WidgetTree->ConstructWidget<UHorizontalBox>();
			Row->AddChildToVerticalBox(Head)->SetPadding(FMargin(0.f, 0.f, 0.f, 3.f));
			UTextBlock* Name = CosmicUI::MakeText(*WidgetTree, 15, false);
			Head->AddChildToHorizontalBox(Name)->SetVerticalAlignment(VAlign_Bottom);
			// The people's rung beside its name (B6b decision 31): set on every refresh, a climb keeps the row.
			UTextBlock* Rung = CosmicUI::MakeText(*WidgetTree, 12, false);
			Rung->SetColorAndOpacity(FSlateColor(SummaryColor));
			UHorizontalBoxSlot* RungSlot = Head->AddChildToHorizontalBox(Rung);
			RungSlot->SetVerticalAlignment(VAlign_Bottom);
			RungSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 1.f));
			UTextBlock* Band = CosmicUI::MakeText(*WidgetTree, 12, false);
			Band->SetColorAndOpacity(FSlateColor(SummaryColor));
			UHorizontalBoxSlot* BandSlot = Head->AddChildToHorizontalBox(Band);
			BandSlot->SetVerticalAlignment(VAlign_Bottom);
			BandSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 1.f));
			UTextBlock* Hostility = CosmicUI::MakeText(*WidgetTree, 12, false);
			Hostility->SetColorAndOpacity(FSlateColor(HostileColor));
			Hostility->SetText(Label(TEXT("Diplomacy.TheyHostile")));
			Hostility->SetVisibility(ESlateVisibility::Collapsed);
			Row->AddChildToVerticalBox(Hostility)->SetPadding(FMargin(0.f, 0.f, 0.f, 3.f));
			// The second line: our three stances; the third: the two treaties - each button an even share of the line.
			auto AddButtons = [&](int32 Count, float Base, TArray<TObjectPtr<UCosmicActionButton>>& Into)
			{
				UHorizontalBox* Line = WidgetTree->ConstructWidget<UHorizontalBox>();
				Row->AddChildToVerticalBox(Line)->SetPadding(FMargin(0.f, 0.f, 0.f, 3.f));
				for (int32 At = 0; At < Count; ++At)
				{
					UCosmicActionButton* Button = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
					Button->Setup(CivID, Base + static_cast<float>(At), FText::GetEmpty());
					Button->OnAction.AddUObject(this, &UCosmicEdictWidget::HandlePeople);
					UHorizontalBoxSlot* ButtonSlot = Line->AddChildToHorizontalBox(Button);
					ButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
					ButtonSlot->SetPadding(FMargin(At == 0 ? 0.f : 4.f, 0.f, 0.f, 0.f));
					Into.Add(Button);
				}
			};
			AddButtons(3, 0.f, StanceButtons);
			AddButtons(2, TreatyBase, TreatyButtons);
			UTextBlock* Siege = CosmicUI::MakeText(*WidgetTree, 12);
			Siege->SetColorAndOpacity(FSlateColor(SummaryColor));
			Siege->SetVisibility(ESlateVisibility::Collapsed);
			Row->AddChildToVerticalBox(Siege);
			PeopleRows.Add(Row);
			PeopleNames.Add(Name);
			PeopleRungs.Add(Rung);
			PeopleBands.Add(Band);
			PeopleHostilities.Add(Hostility);
			PeopleSieges.Add(Siege);
		}
		ShownPeoples = WantedIDs;
	}

	ShownStanceBlocks.SetNum(ShownPeoples.Num() * 3);
	ShownTreatyBlocks.SetNum(ShownPeoples.Num() * 2);
	const FText Upkeep = Amount(Rules.FriendlyUpkeepInfluence);
	// A treaty is a part of the next rung while a judged third-rung row asks one (B6b decision 28): definitions only.
	const FText TreatyRungTip = CosmicNextRung::HasJudgedRowAskingAt(Tables, EAscensionTier::Tier3_Causal, EConditionType::CivHasTreaty)
		? Label(TEXT("Diplomacy.TreatyRungTip")) : FText::GetEmpty();
	for (int32 At = 0; At < Wanted.Num(); ++At)
	{
		const int32 Other = Wanted[At];
		const FCivilizationState& Them = State.Civs[Other];
		const bool bMet = View.IsMet(Other);
		SetWords(PeopleNames[At], Them.CivName);
		SetWords(PeopleRungs[At], RungWordsOf(View, State, Other));
		const float TheirView = CosmicDiplomacy::ViewOf(State, Index, Rules, Other, RuledIndex);
		SetWords(PeopleBands[At], ViewWordOf(TheirView));
		SetTip(PeopleBands[At], ViewTipOf(State, Index, Rules, Other, RuledIndex));
		PeopleBands[At]->SetVisibility(ESlateVisibility::Visible);
		PeopleHostilities[At]->SetVisibility(CosmicDiplomacy::StanceOf(Them, Civ.CivID) == CosmicTags::StanceHostile() ? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Collapsed);
		const bool bBorders = CosmicDiplomacy::Borders(State, Index, Other, RuledIndex);
		// The view their acceptance of our treaties asks (the gate's own).
		const float Bar = CosmicDiplomacy::TreatyBarOf(Civ, Tables, Rules);
		// Our fleet besieges one of their systems: a stance that is not hostile lifts it (the conflict stage's gate).
		const bool bOurSiege = State.Systems.IsValidIndex(Target) && State.Systems[Target].OwnerCivID == Them.CivID;
		const FText Lifts = bOurSiege ? FText::FormatNamed(Label(TEXT("Siege.Lifts")), TEXT("System"), State.Systems[Target].DisplayName) : FText::GetEmpty();
		// What the end of a treaty takes from their view: the betrayal's fall where the stored view is read (the pair
		// borders), else the treaty's own share of the view (a pair that does not border reads no stored view).
		auto Fall = [&](const TCHAR* NearKey, const TCHAR* FarKey)
		{
			return bBorders ? FText::Format(Label(NearKey), Amount(Rules.BetrayalRelationPenalty)) : FText::Format(Label(FarKey), Amount(Rules.RelationTreatyTerm));
		};

		// Our stances: the command's gate greys a button; the stance held is greyed and marked, with Diplomacy.TipNow.
		for (int32 StanceIndex = 0; StanceIndex < 3; ++StanceIndex)
		{
			const FGameplayTag Stance = StanceAt(StanceIndex);
			const CosmicDiplomacy::ECosmicStanceBlock Block = CosmicDiplomacy::StanceBlockOf(State, Index, Rules, RuledIndex, Them.CivID, Stance, bMet);
			ShownStanceBlocks[At * 3 + StanceIndex] = Block;
			UCosmicActionButton* Button = StanceButtons[At * 3 + StanceIndex];
			const bool bHeld = Block == CosmicDiplomacy::ECosmicStanceBlock::Same;
			const FText ButtonLabel = bHeld ? FText::Format(INVTEXT("{0} {1}"), Label(TEXT("Edict.Mark")), CosmicDiplomacy::StanceNameOf(Stance))
				: CosmicDiplomacy::StanceNameOf(Stance);
			if (!SameWords(Button->GetLabel(), ButtonLabel))
			{
				Button->Setup(Them.CivID, static_cast<float>(StanceIndex), ButtonLabel);
			}
			Button->SetIsEnabled(Block == CosmicDiplomacy::ECosmicStanceBlock::None);
			TArray<FText> What;
			if (StanceIndex == 0)
			{
				What.Add(FText::Format(Label(TEXT("Diplomacy.TipFriendly")), Upkeep, PerTick, FText::AsNumber(CosmicDiplomacy::FriendlyCountOf(Civ)),
					FText::AsNumber(CosmicDiplomacy::FriendlySlotsOf(Civ, Rules))));
			}
			else
			{
				What.Add(Label(StanceIndex == 1 ? TEXT("Diplomacy.TipNeutral") : TEXT("Diplomacy.TipHostile")));
			}
			if (StanceIndex == 2 && CosmicDiplomacy::HasTreaty(Civ, Them.CivID, CosmicTags::TreatyTrade()))
			{
				What.Add(Fall(TEXT("Diplomacy.TipBetray"), TEXT("Diplomacy.TipBetrayFar")));
			}
			else if (StanceIndex != 2)
			{
				What.Add(Lifts);
			}
			// The stance held says only that it is held - but the friendly one keeps its upkeep in sight while it is paid.
			TArray<FText> Tip = { StanceReasonOf(Block) };
			if (!bHeld || StanceIndex == 0)
			{
				Tip.Append(What);
			}
			SetTip(Button, CosmicUI::JoinLines(Tip));
		}

		// The treaties: one held is broken by its button; another is proposed, greyed by the command's gate.
		for (int32 TreatyIndex = 0; TreatyIndex < 2; ++TreatyIndex)
		{
			const FGameplayTag Treaty = TreatyAt(TreatyIndex);
			UCosmicActionButton* Button = TreatyButtons[At * 2 + TreatyIndex];
			FText ButtonLabel;
			FText Tip;
			const FText What = TreatyIndex == 0 ? FText::Format(Label(TEXT("Diplomacy.TipTrade")), Amount(Rules.TradeInfluencePerTreaty), PerTick)
				: Label(TEXT("Diplomacy.TipNonAggression"));
			if (CosmicDiplomacy::HasTreaty(Civ, Them.CivID, Treaty))
			{
				ShownTreatyBlocks[At * 2 + TreatyIndex].Reset();
				ButtonLabel = Label(TreatyIndex == 0 ? TEXT("Diplomacy.Break.Trade") : TEXT("Diplomacy.Break.NonAggression"));
				Tip = CosmicUI::JoinLines({ What, Fall(TEXT("Diplomacy.TipBreak"), TEXT("Diplomacy.TipBreakFar")), TreatyRungTip });
				Button->SetIsEnabled(true);
			}
			else
			{
				const CosmicDiplomacy::ECosmicTreatyBlock Block = CosmicDiplomacy::TreatyBlockOf(State, Index, Tables, Rules, RuledIndex, Them.CivID, Treaty, bMet);
				ShownTreatyBlocks[At * 2 + TreatyIndex] = Block;
				ButtonLabel = Label(TreatyIndex == 0 ? TEXT("Diplomacy.Treaty.Trade") : TEXT("Diplomacy.Treaty.NonAggression"));
				Tip = CosmicUI::JoinLines({ TreatyReasonOf(Block), What, FText::Format(Label(TEXT("Diplomacy.TipCost")), Amount(CosmicDiplomacy::TreatyCostOf(Rules))),
					FText::Format(Label(TEXT("Diplomacy.TipBar")), ViewAgainstBarOf(TheirView, Bar), Signed(Bar)), TreatyRungTip });
				Button->SetIsEnabled(Block == CosmicDiplomacy::ECosmicTreatyBlock::None);
			}
			if (!SameWords(Button->GetLabel(), ButtonLabel))
			{
				Button->Setup(Them.CivID, TreatyBase + static_cast<float>(TreatyIndex), ButtonLabel);
			}
			SetTip(Button, Tip);
		}

		// The sieges between us: ours on one of their systems, theirs on one of ours.
		TArray<FText> Sieges;
		if (State.Systems.IsValidIndex(Target) && State.Systems[Target].OwnerCivID == Them.CivID)
		{
			Sieges.Add(SiegeWords(TEXT("Siege.Row.Ours"), RuledIndex, Target, true));
		}
		const int32 TheirTarget = Them.FleetTargetSystemID.IsNone() ? INDEX_NONE : Index.FindSystem(State, Them.FleetTargetSystemID);
		if (State.Systems.IsValidIndex(TheirTarget) && State.Systems[TheirTarget].OwnerCivID == Civ.CivID)
		{
			Sieges.Add(SiegeWords(TEXT("Siege.Row.Theirs"), Other, TheirTarget, false));
		}
		const FText SiegeLines = CosmicUI::JoinLines(Sieges);
		SetWords(PeopleSieges[At], SiegeLines);
		PeopleSieges[At]->SetVisibility(SiegeLines.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
}

void UCosmicEdictWidget::PressStance(FName CivID, int32 StanceIndex)
{
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer());
	UCosmicActionButton* Button = GetStanceButton(CivID, StanceIndex);
	if (!Controller || !Button || !Button->GetIsEnabled())
	{
		return;
	}
	Controller->SetStance(CivID, StanceAt(StanceIndex));
	Refresh();
}

void UCosmicEdictWidget::PressTreaty(FName CivID, int32 TreatyIndex)
{
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer());
	UCosmicActionButton* Button = GetTreatyButton(CivID, TreatyIndex);
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	const int32 RuledIndex = RuledIndexNow();
	if (!Controller || !Button || !Button->GetIsEnabled() || !Sim || RuledIndex == INDEX_NONE)
	{
		return;
	}
	const FGameplayTag Treaty = TreatyAt(TreatyIndex);
	if (CosmicDiplomacy::HasTreaty(Sim->GetState().Civs[RuledIndex], CivID, Treaty))
	{
		Controller->BreakTreaty(CivID, Treaty);
	}
	else
	{
		Controller->ProposeTreaty(CivID, Treaty);
	}
	Refresh();
}

void UCosmicEdictWidget::PressWithdraw()
{
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer());
	if (!Controller || !WithdrawButton || WithdrawButton->GetVisibility() == ESlateVisibility::Collapsed || !WithdrawButton->GetIsEnabled())
	{
		return;
	}
	Controller->AllocateFleet(NAME_None);
	Refresh();
}

void UCosmicEdictWidget::HandlePeople(FName CivID, float Value)
{
	const int32 Code = FMath::RoundToInt32(Value);
	if (Code < static_cast<int32>(CosmicEdictWindowImpl::TreatyBase))
	{
		PressStance(CivID, Code);
	}
	else
	{
		PressTreaty(CivID, Code - static_cast<int32>(CosmicEdictWindowImpl::TreatyBase));
	}
}

const UWidget* UCosmicEdictWidget::GetPeopleRow(FName CivID) const
{
	const int32 At = ShownPeoples.IndexOfByKey(CivID);
	return PeopleRows.IsValidIndex(At) ? PeopleRows[At].Get() : nullptr;
}

const UTextBlock* UCosmicEdictWidget::GetPeopleBand(FName CivID) const
{
	const int32 At = ShownPeoples.IndexOfByKey(CivID);
	return PeopleBands.IsValidIndex(At) ? PeopleBands[At].Get() : nullptr;
}

const UTextBlock* UCosmicEdictWidget::GetPeopleRung(FName CivID) const
{
	const int32 At = ShownPeoples.IndexOfByKey(CivID);
	return PeopleRungs.IsValidIndex(At) ? PeopleRungs[At].Get() : nullptr;
}

const UTextBlock* UCosmicEdictWidget::GetPeopleHostility(FName CivID) const
{
	const int32 At = ShownPeoples.IndexOfByKey(CivID);
	return PeopleHostilities.IsValidIndex(At) ? PeopleHostilities[At].Get() : nullptr;
}

const UTextBlock* UCosmicEdictWidget::GetPeopleSiege(FName CivID) const
{
	const int32 At = ShownPeoples.IndexOfByKey(CivID);
	return PeopleSieges.IsValidIndex(At) ? PeopleSieges[At].Get() : nullptr;
}

UCosmicActionButton* UCosmicEdictWidget::GetStanceButton(FName CivID, int32 StanceIndex) const
{
	const int32 At = ShownPeoples.IndexOfByKey(CivID);
	return (At != INDEX_NONE && StanceIndex >= 0 && StanceIndex < 3 && StanceButtons.IsValidIndex(At * 3 + StanceIndex)) ? StanceButtons[At * 3 + StanceIndex].Get() : nullptr;
}

UCosmicActionButton* UCosmicEdictWidget::GetTreatyButton(FName CivID, int32 TreatyIndex) const
{
	const int32 At = ShownPeoples.IndexOfByKey(CivID);
	return (At != INDEX_NONE && TreatyIndex >= 0 && TreatyIndex < 2 && TreatyButtons.IsValidIndex(At * 2 + TreatyIndex)) ? TreatyButtons[At * 2 + TreatyIndex].Get() : nullptr;
}

TOptional<CosmicDiplomacy::ECosmicStanceBlock> UCosmicEdictWidget::GetShownStanceBlock(FName CivID, int32 StanceIndex) const
{
	const int32 At = ShownPeoples.IndexOfByKey(CivID);
	return (At != INDEX_NONE && StanceIndex >= 0 && StanceIndex < 3 && ShownStanceBlocks.IsValidIndex(At * 3 + StanceIndex))
		? TOptional<CosmicDiplomacy::ECosmicStanceBlock>(ShownStanceBlocks[At * 3 + StanceIndex]) : TOptional<CosmicDiplomacy::ECosmicStanceBlock>();
}

TOptional<CosmicDiplomacy::ECosmicTreatyBlock> UCosmicEdictWidget::GetShownTreatyBlock(FName CivID, int32 TreatyIndex) const
{
	const int32 At = ShownPeoples.IndexOfByKey(CivID);
	return (At != INDEX_NONE && TreatyIndex >= 0 && TreatyIndex < 2 && ShownTreatyBlocks.IsValidIndex(At * 2 + TreatyIndex))
		? ShownTreatyBlocks[At * 2 + TreatyIndex] : TOptional<CosmicDiplomacy::ECosmicTreatyBlock>();
}

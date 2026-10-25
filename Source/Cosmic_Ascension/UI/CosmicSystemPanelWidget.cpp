// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicSystemPanelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Core/CosmicIDs.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "HAL/PlatformTime.h"
#include "Layout/Margin.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicResearch.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "TimerManager.h"
#include "UI/CosmicActionButton.h"
#include "UI/CosmicEdictWidget.h"
#include "UI/CosmicNextRung.h"
#include "UI/CosmicUILabels.h"
#include "UObject/NameTypes.h"
#include "World/CosmicGameMode.h"
#include "World/CosmicMapLook.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicSystemPanelImpl
{
	const FLinearColor BoxColor(0.01f, 0.01f, 0.03f, 0.85f);
	const TCHAR* const StarClassPrefix = TEXT("Star.Class.");

	// "G" of Star.Class.G; empty for a tag that is no star class.
	FString StarClassLetter(FGameplayTag StarClass)
	{
		const FString Name = StarClass.GetTagName().ToString();
		return Name.StartsWith(StarClassPrefix) ? Name.RightChop(FCString::Strlen(StarClassPrefix)) : FString();
	}

	// "<slot> <project> <progress>/<needed>", or empty for an empty slot or a row the tables do not hold.
	FText SlotLine(const TCHAR* SlotKey, const FCosmicDataTables& Tables, FName RowName, float Progress)
	{
		const FMegaprojectDef* Row = RowName.IsNone() ? nullptr : Tables.FindMegaproject(RowName);
		if (!Row)
		{
			return FText::GetEmpty();
		}
		return FText::Format(INVTEXT("{0}  {1} {2}/{3}"), CosmicUI::Label(SlotKey), Row->DisplayName,
			CosmicUI::WholeNumber(Progress), CosmicUI::WholeNumber(CosmicMegaprojects::RequiredProgressOf(*Row)));
	}

	// The String Table key of each reason, in the order of ECosmicSettleBlock (None, NoPeople and Dead show no button).
	const TCHAR* const SettleBlockKeys[] = {
		nullptr,
		TEXT("Settle.Block.Sanctuary"),
		nullptr,
		TEXT("Settle.Block.Owned"),
		nullptr,
		TEXT("Settle.Block.NoLane"),
		TEXT("Settle.Block.NoRow"),
		TEXT("Settle.Block.RowTier"),
		TEXT("Settle.Block.RealmCap"),
		TEXT("Settle.Block.Settling"),
		TEXT("Settle.Block.Sphere"),
	};
	static_assert(static_cast<int32>(UE_ARRAY_COUNT(SettleBlockKeys)) == static_cast<int32>(CosmicMegaprojects::ECosmicSettleBlock::Count), "One key per reason, in the order of the enum");

	// The String Table key of each reason a project button is greyed for, in the order of ECosmicStartBlock (null: None,
	// and the reasons that show no button - the row does not belong on that system).
	const TCHAR* const StartBlockKeys[] = {
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		TEXT("Start.Block.RowTier"),
		TEXT("Start.Block.Answering"),
		TEXT("Start.Block.ResponseBusy"),
		TEXT("Start.Block.Running"),
	};
	static_assert(static_cast<int32>(UE_ARRAY_COUNT(StartBlockKeys)) == static_cast<int32>(CosmicMegaprojects::ECosmicStartBlock::Count), "One key per reason, in the order of the enum");

	// The short reason after a greyed project button's name, in the same order ("(rung 2 on)", "(building)", ...).
	const TCHAR* const StartSuffixKeys[] = {
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		TEXT("Start.Suffix.RowTier"),
		TEXT("Start.Suffix.Answering"),
		TEXT("Start.Suffix.ResponseBusy"),
		TEXT("Start.Suffix.Running"),
	};
	static_assert(static_cast<int32>(UE_ARRAY_COUNT(StartSuffixKeys)) == static_cast<int32>(CosmicMegaprojects::ECosmicStartBlock::Count), "One suffix per reason, in the order of the enum");

	// The key of Block in Keys (one per enumerator); null out of range.
	const TCHAR* StartKeyOf(const TCHAR* const (&Keys)[static_cast<int32>(CosmicMegaprojects::ECosmicStartBlock::Count)], CosmicMegaprojects::ECosmicStartBlock Block)
	{
		const int32 At = static_cast<int32>(Block);
		return (At >= 0 && At < static_cast<int32>(CosmicMegaprojects::ECosmicStartBlock::Count)) ? Keys[At] : nullptr;
	}

	// The rung a RowTier reason names: the row's RequiredTier, numbered as the top bar shows rungs (1 to 5).
	FText RungOfRow(const FCosmicDataTables& Tables, FName RowName)
	{
		const FMegaprojectDef* Row = Tables.FindMegaproject(RowName);
		return FText::AsNumber(Row ? static_cast<int32>(Row->RequiredTier) + 1 : 0);
	}

	// The short reason after a greyed project button's name; empty for an enabled button.
	FText StartSuffixOf(CosmicMegaprojects::ECosmicStartBlock Block, const FCosmicDataTables& Tables, FName RowName)
	{
		const TCHAR* const Key = StartKeyOf(StartSuffixKeys, Block);
		if (!Key)
		{
			return FText::GetEmpty();
		}
		return Block == CosmicMegaprojects::ECosmicStartBlock::RowTier ? FText::Format(CosmicUI::Label(Key), RungOfRow(Tables, RowName)) : CosmicUI::Label(Key);
	}

	// One member of a cost: its label and its amount - a whole amount as a whole number, any other with up to two decimals
	// (no trailing zeros: the top bar shows the stockpile in whole numbers).
	FText CostMember(const TCHAR* LabelKey, float Amount)
	{
		FNumberFormattingOptions Options;
		Options.MinimumFractionalDigits = 0;
		Options.MaximumFractionalDigits = 2;
		return FText::Format(INVTEXT("{0} {1}"), CosmicUI::Label(LabelKey), FText::AsNumber(Amount, &Options));
	}

	// A command button the panel shows: what it does, its label, why it is greyed out (empty: enabled) and its tooltip.
	struct FWantedButton
	{
		FName Action;
		FText Label;
		FText Reason;
		FText Tip;
	};

	// Whether two texts say the same words (the panel leaves a text alone when its words stay).
	bool SameWords(const FText& A, const FText& B)
	{
		return A.ToString().Equals(B.ToString(), ESearchCase::CaseSensitive);
	}

	// The words of a greyed travel button (B1): the short ones after its name (bShort), the long ones first in its tooltip;
	// null for None and for the reasons that show no button (the target is no lane neighbour, the vessel no wanderer, a
	// handler - and the sanctuary, which has no lane).
	const TCHAR* TravelKeyOf(CosmicWanderer::ETravelBlock Block, bool bShort)
	{
		switch (Block)
		{
		case CosmicWanderer::ETravelBlock::Travelling:
			return bShort ? TEXT("Wander.Suffix.Travelling") : TEXT("Wander.Block.Travelling");
		case CosmicWanderer::ETravelBlock::CardWaiting:
			return bShort ? TEXT("Wander.Suffix.CardWaiting") : TEXT("Wander.Block.CardWaiting");
		case CosmicWanderer::ETravelBlock::None:
		case CosmicWanderer::ETravelBlock::NotWanderer:
		case CosmicWanderer::ETravelBlock::Sanctuary:
		case CosmicWanderer::ETravelBlock::NotNeighbour:
		case CosmicWanderer::ETravelBlock::Handler:
			break;
		}
		return nullptr;
	}

	// The words of a greyed crown button, in the same two lengths (ThroneLocked's take the time left as {0}); null for
	// None and for the reasons that show no button (the vessel stands elsewhere or in a system being settled).
	const TCHAR* CrownKeyOf(CosmicWanderer::ECrownBlock Block, bool bShort)
	{
		switch (Block)
		{
		case CosmicWanderer::ECrownBlock::Travelling:
			return bShort ? TEXT("Wander.Suffix.Travelling") : TEXT("Wander.Block.Travelling");
		case CosmicWanderer::ECrownBlock::CardWaiting:
			return bShort ? TEXT("Wander.Suffix.CardWaiting") : TEXT("Wander.Block.CardWaiting");
		case CosmicWanderer::ECrownBlock::Unmet:
			return bShort ? TEXT("Wander.Suffix.Unmet") : TEXT("Wander.Block.Unmet");
		case CosmicWanderer::ECrownBlock::ThroneLocked:
			return bShort ? TEXT("Wander.Suffix.Locked") : TEXT("Wander.Block.Locked");
		case CosmicWanderer::ECrownBlock::Lacking:
			return bShort ? TEXT("Wander.Suffix.Lacking") : TEXT("Wander.Block.Lacking");
		case CosmicWanderer::ECrownBlock::None:
		case CosmicWanderer::ECrownBlock::NotWanderer:
		case CosmicWanderer::ECrownBlock::NotStanding:
		case CosmicWanderer::ECrownBlock::NotSettled:
		case CosmicWanderer::ECrownBlock::Handler:
			break;
		}
		return nullptr;
	}

	// Builds Buttons again in Box only when the list of their actions changes (a click spans a press and a release, a
	// button built again between them drops it, and the simulation ticks up to 25 times a second); otherwise each button
	// keeps its object and takes its label, state and tooltip in place. Bind hooks a new button to its handler.
	template <typename FBind>
	void SyncButtons(UWidgetTree& Tree, UVerticalBox& Box, TArray<TObjectPtr<UCosmicActionButton>>& Buttons, const TArray<FWantedButton>& Wanted, FBind&& Bind)
	{
		bool bSameActions = Buttons.Num() == Wanted.Num();
		for (int32 At = 0; bSameActions && At < Wanted.Num(); ++At)
		{
			bSameActions = Buttons[At] && Buttons[At]->GetAction() == Wanted[At].Action;
		}
		if (!bSameActions)
		{
			Box.ClearChildren();
			Buttons.Reset();
			for (const FWantedButton& Want : Wanted)
			{
				UCosmicActionButton* Button = Tree.ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
				Button->Setup(Want.Action, 0.f, Want.Label);
				Bind(*Button);
				Box.AddChildToVerticalBox(Button)->SetPadding(FMargin(0.f, 2.f));
				Buttons.Add(Button);
			}
		}
		for (int32 At = 0; At < Wanted.Num(); ++At)
		{
			UCosmicActionButton* Button = Buttons[At];
			if (!SameWords(Button->GetLabel(), Wanted[At].Label))
			{
				Button->Setup(Wanted[At].Action, 0.f, Wanted[At].Label);
			}
			Button->SetIsEnabled(Wanted[At].Reason.IsEmpty());
			// A tooltip set again is a new tooltip (SWidget::SetToolTipText makes one), and an open one would close.
			if (!SameWords(Button->GetToolTipText(), Wanted[At].Tip))
			{
				Button->SetToolTipText(Wanted[At].Tip);
			}
		}
	}
}

const FName UCosmicSystemPanelWidget::TravelAction(TEXT("travel"));
const FName UCosmicSystemPanelWidget::ScanAction(TEXT("scan"));
const FName UCosmicSystemPanelWidget::CrownAction(TEXT("crown"));
const FName UCosmicSystemPanelWidget::AbdicateAction(TEXT("abdicate"));
const FName UCosmicSystemPanelWidget::DirectAction(TEXT("direct"));

TSharedRef<SWidget> UCosmicSystemPanelWidget::RebuildWidget()
{
	using namespace CosmicSystemPanelImpl;

	// RebuildWidget can run again after the Slate widgets are released; the tree is built once.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		// The box sits at the top of the panel's area; the rest of the area lets clicks through to the map.
		UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>();
		Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		WidgetTree->RootWidget = Root;
		UBorder* Box = WidgetTree->ConstructWidget<UBorder>();
		Box->SetBrushColor(BoxColor);
		Box->SetPadding(FMargin(16.f, 12.f));
		Root->AddChildToVerticalBox(Box);

		Column = WidgetTree->ConstructWidget<UVerticalBox>();
		Box->SetContent(Column);

		NameText = CosmicUI::MakeText(*WidgetTree, 18);
		Column->AddChildToVerticalBox(NameText)->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
		DetailText = CosmicUI::MakeText(*WidgetTree, 13);
		Column->AddChildToVerticalBox(DetailText)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		// The sight's words, a block apart from the details: their tooltip covers only them (Refresh).
		MarkText = CosmicUI::MakeText(*WidgetTree, 13);
		MarkText->SetVisibility(ESlateVisibility::Collapsed);
		Column->AddChildToVerticalBox(MarkText)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		ButtonBox = WidgetTree->ConstructWidget<UVerticalBox>();
		Column->AddChildToVerticalBox(ButtonBox);
		// The wanderer's row (B1): a box of its own under the project buttons, so GetActionButtons stays the projects'.
		WandererBox = WidgetTree->ConstructWidget<UVerticalBox>();
		Column->AddChildToVerticalBox(WandererBox);
		// The ruler's [Abdicate] (B1): built once, shown on the capital's panel only.
		AbdicateButton = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		AbdicateButton->Setup(AbdicateAction, 0.f, CosmicUI::Label(TEXT("Wander.Abdicate")));
		AbdicateButton->OnAction.AddUObject(this, &UCosmicSystemPanelWidget::HandleWanderer);
		AbdicateButton->SetVisibility(ESlateVisibility::Collapsed);
		Column->AddChildToVerticalBox(AbdicateButton)->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
		// The edict hint (B2): grey and quiet like the first ship's line and above it - the first of the three hints when
		// they stand together.
		EdictHintText = CosmicUI::MakeText(*WidgetTree, 13);
		EdictHintText->SetColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.65f)));
		EdictHintText->SetVisibility(ESlateVisibility::Collapsed);
		Column->AddChildToVerticalBox(EdictHintText)->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
		// The first ship's line (B1): grey and quiet, under [Abdicate] on the capital's panel, once a session (the hint line
		// and the research focus below keep their order).
		ShipHintText = CosmicUI::MakeText(*WidgetTree, 13);
		ShipHintText->SetColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.65f)));
		ShipHintText->SetText(CosmicUI::Label(TEXT("Hint.FirstShip")));
		ShipHintText->SetVisibility(ESlateVisibility::Collapsed);
		Column->AddChildToVerticalBox(ShipHintText)->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
		ReasonText = CosmicUI::MakeText(*WidgetTree, 13);
		ReasonText->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.9f)));
		ReasonText->SetVisibility(ESlateVisibility::Collapsed);
		Column->AddChildToVerticalBox(ReasonText)->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
		// The first hints (MVP item 6): what the buttons do, under them, quieter than a reason - until the first project.
		HintText = CosmicUI::MakeText(*WidgetTree, 13);
		HintText->SetColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.65f)));
		HintText->SetText(CosmicUI::Label(TEXT("Hint.Commands")));
		HintText->SetVisibility(ESlateVisibility::Collapsed);
		Column->AddChildToVerticalBox(HintText)->SetPadding(FMargin(0.f, 8.f, 0.f, 0.f));
		// The research focus (MVP item 7), last: the hint line above stays under the project buttons it speaks of. The
		// heading names the focus now; the four buttons are built once and keep their objects.
		FocusBox = WidgetTree->ConstructWidget<UVerticalBox>();
		FocusBox->SetVisibility(ESlateVisibility::Collapsed);
		Column->AddChildToVerticalBox(FocusBox)->SetPadding(FMargin(0.f, 8.f, 0.f, 0.f));
		FocusHeading = CosmicUI::MakeText(*WidgetTree, 13);
		FocusHeading->SetToolTipText(CosmicUI::Label(TEXT("Focus.HeadingTip")));
		FocusBox->AddChildToVerticalBox(FocusHeading)->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));
		UHorizontalBox* FocusRow = WidgetTree->ConstructWidget<UHorizontalBox>();
		FocusBox->AddChildToVerticalBox(FocusRow);
		const TPair<FName, const TCHAR*> Choices[] = {
			{ NAME_None, TEXT("Focus.Even") },
			{ FName(TEXT("Physics")), TEXT("Research.Physics") },
			{ FName(TEXT("Society")), TEXT("Research.Society") },
			{ FName(TEXT("Engineering")), TEXT("Research.Engineering") },
		};
		for (const TPair<FName, const TCHAR*>& Choice : Choices)
		{
			UCosmicActionButton* Button = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
			Button->Setup(Choice.Key, 0.f, CosmicUI::Label(Choice.Value));
			Button->OnAction.AddUObject(this, &UCosmicSystemPanelWidget::HandleFocus);
			UHorizontalBoxSlot* ChoiceSlot = FocusRow->AddChildToHorizontalBox(Button);
			ChoiceSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			ChoiceSlot->SetPadding(FMargin(FocusButtons.IsEmpty() ? 0.f : 4.f, 0.f, 0.f, 0.f));
			FocusButtons.Add(Button);
		}
		// [Share by hand] (B2): a line of its own under the four buttons, at the right - the row of four stays as it is.
		DirectButton = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		DirectButton->Setup(DirectAction, 0.f, CosmicUI::Label(TEXT("Focus.Direct")));
		DirectButton->OnAction.AddUObject(this, &UCosmicSystemPanelWidget::HandleDirect);
		UVerticalBoxSlot* DirectSlot = FocusBox->AddChildToVerticalBox(DirectButton);
		DirectSlot->SetHorizontalAlignment(HAlign_Right);
		DirectSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));
	}
	return Super::RebuildWidget();
}

void UCosmicSystemPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr)
	{
		Sim->OnSimTick.AddUniqueDynamic(this, &UCosmicSystemPanelWidget::HandleSimTick);
		Sim->OnGalaxyGenerated.AddUniqueDynamic(this, &UCosmicSystemPanelWidget::HandleGalaxyGenerated);
	}
	Refresh();
}

void UCosmicSystemPanelWidget::NativeDestruct()
{
	if (UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr)
	{
		Sim->OnSimTick.RemoveDynamic(this, &UCosmicSystemPanelWidget::HandleSimTick);
		Sim->OnGalaxyGenerated.RemoveDynamic(this, &UCosmicSystemPanelWidget::HandleGalaxyGenerated);
	}
	Super::NativeDestruct();
}

void UCosmicSystemPanelWidget::HandleSimTick(int64 Tick)
{
	Refresh();
}

void UCosmicSystemPanelWidget::HandleGalaxyGenerated(int32 Seed)
{
	// A new galaxy or a load is another game: what the watches saw belongs to the old one.
	ConstructionWatches.Reset();
	ResponseWatches.Reset();
	Refresh();
}

FCosmicSlotWatch CosmicSlotWatch::Step(const FCosmicSlotWatch& Watch, const CosmicMegaprojects::FCosmicSlotReading& Now, int64 NowTick)
{
	FCosmicSlotWatch Fresh;
	Fresh.Seen = Now;
	Fresh.Tick = NowTick;
	// An empty slot shows nothing and has nothing to watch.
	if (Now.Row.IsNone())
	{
		return Fresh;
	}
	// The same tick again: a redraw between ticks changes nothing; a new reading (a command) starts the watch again.
	if (Watch.Tick == NowTick)
	{
		const bool bSame = Watch.Seen.Row == Now.Row && Watch.Seen.Progress == Now.Progress;
		return bSame ? Watch : Fresh;
	}
	// Judged on the next tick only, the same row only.
	if (Watch.Tick >= 0 && NowTick == Watch.Tick + 1 && Now.Row == Watch.Seen.Row)
	{
		FCosmicSlotWatch Next = Watch;
		if (CosmicMegaprojects::PaidNothing(Watch.Seen, Now))
		{
			++Next.UnpaidRun;
			Next.PaidRun = 0;
			if (Next.UnpaidRun >= StallAfter)
			{
				Next.bStalled = true;
			}
		}
		else if (Now.Progress > Watch.Seen.Progress)
		{
			++Next.PaidRun;
			Next.UnpaidRun = 0;
			if (Next.PaidRun >= ResumeAfter)
			{
				Next.bStalled = false;
			}
		}
		else
		{
			// A lower progress: the row started again.
			return Fresh;
		}
		Next.Seen = Now;
		Next.Tick = NowTick;
		return Next;
	}
	// A gap, another row, a load: start again.
	return Fresh;
}

void UCosmicSystemPanelWidget::WatchSlots(const UGalaxySimulationSubsystem& Sim)
{
	// Mid-tick (a handler of a Core Logic delegate redraws the HUD from inside a stage) a slot may not have been paid yet.
	if (Sim.IsBroadcasting())
	{
		return;
	}
	const FGalaxyState& State = Sim.GetState();
	const FCosmicStateIndex& Index = Sim.GetStateIndex();
	const bool bRules = State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler && !State.Player.RuledCivID.IsNone();
	const int32 RuledIndex = bRules ? Index.FindCiv(State, State.Player.RuledCivID) : INDEX_NONE;
	TSet<FName> Watched;
	if (State.Civs.IsValidIndex(RuledIndex))
	{
		const FCivilizationState& Civ = State.Civs[RuledIndex];
		for (const FName SystemID : Civ.OwnedSystems)
		{
			const int32 SystemIndex = Index.FindSystem(State, SystemID);
			if (!State.Systems.IsValidIndex(SystemIndex) || State.Systems[SystemIndex].OwnerCivID != Civ.CivID)
			{
				continue;
			}
			const FStarSystemState& System = State.Systems[SystemIndex];
			ConstructionWatches.Add(SystemID, CosmicSlotWatch::Step(ConstructionWatches.FindRef(SystemID),
				CosmicMegaprojects::ReadSlot(System, CosmicMegaprojects::ECosmicProjectSlot::Construction), State.Tick));
			ResponseWatches.Add(SystemID, CosmicSlotWatch::Step(ResponseWatches.FindRef(SystemID),
				CosmicMegaprojects::ReadSlot(System, CosmicMegaprojects::ECosmicProjectSlot::Response), State.Tick));
			Watched.Add(SystemID);
		}
	}
	// A system the people no longer owns is no longer watched (and none at all without an embodied ruler).
	for (auto It = ConstructionWatches.CreateIterator(); It; ++It)
	{
		if (!Watched.Contains(It.Key()))
		{
			It.RemoveCurrent();
		}
	}
	for (auto It = ResponseWatches.CreateIterator(); It; ++It)
	{
		if (!Watched.Contains(It.Key()))
		{
			It.RemoveCurrent();
		}
	}
}

bool UCosmicSystemPanelWidget::IsSlotStalled(FName SystemID, CosmicMegaprojects::ECosmicProjectSlot Which) const
{
	const FCosmicSlotWatch* Watch = (Which == CosmicMegaprojects::ECosmicProjectSlot::Construction ? ConstructionWatches : ResponseWatches).Find(SystemID);
	return Watch && Watch->bStalled;
}

CosmicMegaprojects::ECosmicStartBlock UCosmicSystemPanelWidget::GetShownStartBlock(FName RowName) const
{
	for (const TPair<FName, CosmicMegaprojects::ECosmicStartBlock>& Shown : ShownStartBlocks)
	{
		if (Shown.Key == RowName)
		{
			return Shown.Value;
		}
	}
	return CosmicMegaprojects::ECosmicStartBlock::Count;
}

FText UCosmicSystemPanelWidget::StartReasonOf(CosmicMegaprojects::ECosmicStartBlock Block, const FCosmicDataTables& Tables, FName RowName)
{
	using namespace CosmicSystemPanelImpl;

	const TCHAR* const Key = StartKeyOf(StartBlockKeys, Block);
	if (!Key)
	{
		return FText::GetEmpty();
	}
	// The rung the words name is read where the rule keeps it: the row's own RequiredTier.
	return Block == CosmicMegaprojects::ECosmicStartBlock::RowTier ? FText::Format(CosmicUI::Label(Key), RungOfRow(Tables, RowName)) : CosmicUI::Label(Key);
}

FText UCosmicSystemPanelWidget::StartHintOf(const FCosmicDataTables& Tables, FName RowName, const FStarSystemState& System, ECosmicTimeUnit Unit)
{
	using namespace CosmicSystemPanelImpl;
	using CosmicUI::Label;

	const FMegaprojectDef* Row = RowName.IsNone() ? nullptr : Tables.FindMegaproject(RowName);
	if (!Row)
	{
		return FText::GetEmpty();
	}
	// The cost as Advance pays it: the members above 0.
	const FResourcePool Cost = CosmicMegaprojects::CostPerTickOf(*Row);
	const TPair<const TCHAR*, float> Members[] = {
		{ TEXT("Resource.Energy"), Cost.Energy },
		{ TEXT("Resource.Matter"), Cost.Matter },
		{ TEXT("Resource.Insight"), Cost.Insight },
		{ TEXT("Resource.Influence"), Cost.Influence },
		{ TEXT("Resource.Causality"), Cost.Causality },
	};
	TArray<FText> Paid;
	for (const TPair<const TCHAR*, float>& Member : Members)
	{
		if (Member.Value > 0.f)
		{
			Paid.Add(CostMember(Member.Key, Member.Value));
		}
	}
	TArray<FText> Lines;
	// Another construction in the slot is taken over (section 5.7): said, not refused - first, as the one loss a start
	// cannot undo.
	const bool bConstruction = !Tables.IsHazardAnswer(RowName);
	if (bConstruction && RowName != CosmicRows::ProjectSettle && !System.ActiveMegaproject.IsNone() && System.ActiveMegaproject != RowName)
	{
		if (const FMegaprojectDef* Held = Tables.FindMegaproject(System.ActiveMegaproject))
		{
			Lines.Add(FText::Format(Label(TEXT("Start.Replaces")), Held->DisplayName));
		}
	}
	Lines.Add(Paid.IsEmpty() ? Label(TEXT("Start.Free"))
		: FText::Format(Label(Unit == ECosmicTimeUnit::Year ? TEXT("Start.CostPerYear") : TEXT("Start.CostPerCycle")), CosmicUI::JoinList(Paid)));
	// A construction that pays waits while an answer of its people waits for its cost (section 5.7): the mark of a slot
	// may then stand while the stockpile looks full.
	if (bConstruction && !Paid.IsEmpty())
	{
		Lines.Add(Label(TEXT("Start.AnswersFirst")));
	}
	return CosmicUI::JoinLines(Lines);
}

void UCosmicSystemPanelWidget::ShowSystem(FName SystemID)
{
	// Another system: [Abdicate] is no longer armed, and the scan's 'nothing new' goes.
	if (SystemID != ShownSystemID)
	{
		AbdicateArmedAt = -1.0;
		ScanNothingAt = NAME_None;
	}
	ShownSystemID = SystemID;
	Refresh();
}

FText UCosmicSystemPanelWidget::OwnerLineOf(const FGalaxyState& State, const FCosmicKnownView& View, int32 SystemIndex)
{
	using CosmicUI::Label;

	if (!State.Systems.IsValidIndex(SystemIndex) || !View.IsShown(SystemIndex))
	{
		return FText::GetEmpty();
	}
	// The sanctuary's line first: it is kept, and belongs to no people (section 4.5 (1), 6.3).
	if (CosmicState::IsSanctuary(State.Systems[SystemIndex].SystemID))
	{
		return Label(TEXT("Panel.Sanctuary"));
	}
	if (!View.IsKnown(SystemIndex))
	{
		return FText::GetEmpty();
	}
	if (CosmicHazards::IsDeadNode(State.Systems[SystemIndex]))
	{
		return Label(TEXT("Panel.Destroyed"));
	}
	switch (CosmicMapLook::OwnerShownOf(State, View, SystemIndex))
	{
	case ECosmicOwnerShown::Shown:
	{
		const FName OwnerID = State.Systems[SystemIndex].OwnerCivID;
		const FCivilizationState* Owner = State.Civs.FindByPredicate([OwnerID](const FCivilizationState& Civ) { return Civ.CivID == OwnerID; });
		return Owner ? FText::Format(INVTEXT("{0}  {1}"), Label(TEXT("Panel.Owner")), Owner->CivName) : Label(TEXT("Panel.NoOwner"));
	}
	case ECosmicOwnerShown::Hidden:
		// A people lives there, whom the viewer is not shown: true in both hidden cases (not met, and met but three rungs
		// above), and it names no one - the words of the settle button's reason (Settle.Block.Owned).
		return FText::Format(INVTEXT("{0}  {1}"), Label(TEXT("Panel.Owner")), Label(TEXT("Panel.OtherOwner")));
	case ECosmicOwnerShown::None:
		break;
	}
	return Label(TEXT("Panel.NoOwner"));
}

FText UCosmicSystemPanelWidget::ReasonOf(CosmicMegaprojects::ECosmicSettleBlock Block, const FCosmicDataTables& Tables)
{
	using namespace CosmicSystemPanelImpl;
	using CosmicMegaprojects::ECosmicSettleBlock;

	const int32 At = static_cast<int32>(Block);
	const TCHAR* const Key = (At >= 0 && At < static_cast<int32>(UE_ARRAY_COUNT(SettleBlockKeys))) ? SettleBlockKeys[At] : nullptr;
	if (!Key)
	{
		return FText::GetEmpty();
	}
	// The rung the words name is read where the rule keeps it, numbered as the top bar shows rungs (1 to 5): the settle
	// row's RequiredTier, and the rung that lifts a sphere.
	if (Block == ECosmicSettleBlock::RowTier)
	{
		const FMegaprojectDef* Row = Tables.FindMegaproject(CosmicRows::ProjectSettle);
		return FText::Format(CosmicUI::Label(Key), FText::AsNumber(Row ? static_cast<int32>(Row->RequiredTier) + 1 : 0));
	}
	if (Block == ECosmicSettleBlock::Sphere)
	{
		return FText::Format(CosmicUI::Label(Key), FText::AsNumber(static_cast<int32>(CosmicMegaprojects::SphereLiftTier) + 1));
	}
	return CosmicUI::Label(Key);
}

FText UCosmicSystemPanelWidget::SpiritTipOf(const FCosmicSpiritLook& Spirit, const FCosmicDataTables& Tables)
{
	// No pressure reaches the sanctuary (Law.Hedge, section 4.5 (1)).
	if (Spirit.bSanctuary)
	{
		return CosmicUI::Label(TEXT("Sight.Sanctuary"));
	}
	const FTranscendentDef* Row = Spirit.MarkRow.IsNone() ? nullptr : Tables.FindTranscendent(Spirit.MarkRow);
	if (!Row || Row->Title.IsEmpty())
	{
		return FText::GetEmpty();
	}
	// The verbs of the counterfeit principle (section 4.2): it claims, it holds, it presses; a sealed sphere is what it
	// held before the binding.
	const TCHAR* Key = Spirit.ClaimState == ECosmicClaimState::Claimed ? TEXT("Sight.Claimed")
		: Spirit.ClaimState == ECosmicClaimState::Held ? TEXT("Sight.Held")
		: Spirit.bMarkPresses ? TEXT("Sight.Pressed")
		: Spirit.ClaimState == ECosmicClaimState::Bound ? TEXT("Sight.Bound") : nullptr;
	return Key ? FText::FormatNamed(CosmicUI::Label(Key), TEXT("Transcendent"), Row->Title) : FText::GetEmpty();
}

bool UCosmicSystemPanelWidget::IsFocusShown() const
{
	return FocusBox && FocusBox->GetVisibility() != ESlateVisibility::Collapsed;
}

bool UCosmicSystemPanelWidget::IsReasonShown() const
{
	return ReasonText && ReasonText->GetVisibility() != ESlateVisibility::Collapsed;
}

void UCosmicSystemPanelWidget::HandleAction(FName Action, float Value)
{
	// Through the controller, which redraws the HUD and the map after a command (a paused simulation sends no tick).
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->StartProject(ShownSystemID, Action);
	}
	Refresh();
}

void UCosmicSystemPanelWidget::HandleWanderer(FName Action, float Value)
{
	if (Action == AbdicateAction)
	{
		PressAbdicate(FPlatformTime::Seconds());
		return;
	}
	// Through the controller, which redraws the HUD and the map after a command (a paused simulation sends no tick).
	ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer());
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (Controller && Sim)
	{
		if (Action == TravelAction)
		{
			Controller->TravelTo(ShownSystemID);
		}
		else if (Action == ScanAction)
		{
			// A scan that learned nothing says so on the panel (header section 2.3), while the vessel stands there.
			const int32 Learned = Controller->ScanHere();
			ScanNothingAt = Learned == 0 ? Sim->GetState().Player.CurrentSystemID : NAME_None;
		}
		else if (Action == CrownAction)
		{
			const int32 SystemIndex = Sim->GetStateIndex().FindSystem(Sim->GetState(), ShownSystemID);
			if (Sim->GetState().Systems.IsValidIndex(SystemIndex))
			{
				Controller->TakeThrone(Sim->GetState().Systems[SystemIndex].OwnerCivID);
			}
		}
	}
	Refresh();
}

bool UCosmicSystemPanelWidget::IsAbdicateArmed(double NowSeconds) const
{
	return AbdicateArmedAt >= 0.0 && NowSeconds >= AbdicateArmedAt && NowSeconds - AbdicateArmedAt <= AbdicateConfirmSeconds;
}

void UCosmicSystemPanelWidget::PressAbdicate(double NowSeconds)
{
	if (IsAbdicateArmed(NowSeconds))
	{
		AbdicateArmedAt = -1.0;
		if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
		{
			Controller->Abdicate();
		}
	}
	else
	{
		AbdicateArmedAt = NowSeconds;
		// The label goes back by itself when the window closes: a paused simulation sends no tick to redraw it.
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(AbdicateTimer, FTimerDelegate::CreateUObject(this, &UCosmicSystemPanelWidget::Refresh),
				static_cast<float>(AbdicateConfirmSeconds) + 0.05f, false);
		}
	}
	Refresh();
}

void UCosmicSystemPanelWidget::HandleFocus(FName Field, float Value)
{
	// Through the controller, which redraws the HUD after a change (a paused simulation sends no tick).
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->SetResearchFocus(Field);
	}
	Refresh();
}

void UCosmicSystemPanelWidget::HandleDirect(FName Action, float Value)
{
	// The HUD's edict window on its shares' page, through the controller.
	if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GetOwningPlayer()))
	{
		Controller->OpenEdictWindow(ECosmicEdictTab::Shares);
	}
}

FText UCosmicSystemPanelWidget::EdictAnswerLineOf(const FCivilizationState& Ruled, const FHazardDef& Hazard, const FCosmicDataTables& Tables)
{
	const FName Held = CosmicEdicts::HeldAnswerOf(Ruled, Hazard, Tables);
	FName Answer = Held;
	for (int32 At = 0; Answer.IsNone() && At < Hazard.RulerResolutions.Num(); ++At)
	{
		Answer = Tables.FindEdict(Hazard.RulerResolutions[At]) ? Hazard.RulerResolutions[At] : NAME_None;
	}
	const FEdictDef* Edict = Answer.IsNone() ? nullptr : Tables.FindEdict(Answer);
	return Edict ? FText::FormatNamed(CosmicUI::Label(Held.IsNone() ? TEXT("Edict.Panel.Can") : TEXT("Edict.Panel.Held")), TEXT("Edict"), Edict->DisplayName)
		: FText::GetEmpty();
}

void UCosmicSystemPanelWidget::Refresh()
{
	using namespace CosmicSystemPanelImpl;
	using namespace CosmicUI;

	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || !NameText)
	{
		return;
	}
	// First, whatever the panel shows: a system selected later already has its watch.
	WatchSlots(*Sim);
	const FGalaxyState& State = Sim->GetState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicDataTables& Tables = Sim->GetTables();
	const int32 SystemIndex = ShownSystemID.IsNone() ? INDEX_NONE : Index.FindSystem(State, ShownSystemID);
	const ECosmicTimeUnit Unit = ACosmicPlayerController::TimeUnitFor(*this);
	// What the viewer knows (section 6.4): a system the viewer does not know shows no panel, as if nothing were selected
	// (a guard - the controller selects only known systems), and an owner the viewer is not shown has no name, no people
	// line and no works here.
	const FCosmicKnownView View = ACosmicPlayerController::KnownViewFor(*this);

	TArray<FWantedButton> Wanted;
	ShownSettleBlock = CosmicMegaprojects::ECosmicSettleBlock::None;
	// The run is over (MVP item 4): every command button is greyed out with the end's reason - the settle button and its line
	// under it too, with no cost line and no short reason of its own.
	const bool bEnded = ACosmicPlayerController::IsRunEndedFor(*this);
	const FText EndBlock = Label(TEXT("Chronicle.Block"));
	bool bSettleButton = false;
	// Whether the panel offers the ruler a project to start now (the hint line's condition - not the settle button).
	bool bEnabledProject = false;
	ShownStartBlocks.Reset();
	// The spiritual sight's words for the shown system (section 6.4): a line of their own and its tooltip.
	FText SpiritWords;
	// The people whose research focus the panel shows (MVP item 7): the ruled people, on a settled system of its own.
	const FCivilizationState* FocusCiv = nullptr;
	// The wanderer's row (B1), and whether the panel shows the ruled people's capital ([Abdicate], the first-ship line).
	TArray<FWantedButton> WandererWanted;
	ShownTravelBlock.Reset();
	ShownCrownBlock.Reset();
	bool bCapital = false;
	// The panel's gate of knowledge: a known system, or the sanctuary by its name (the view's ShownByName - its own
	// panel, section 6.3).
	if (State.Systems.IsValidIndex(SystemIndex) && View.IsShown(SystemIndex))
	{
		// The box takes the clicks on it (NativeOnMouseButtonDown); the panel's empty area does not.
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		const FStarSystemState& System = State.Systems[SystemIndex];
		const bool bDead = CosmicHazards::IsDeadNode(System);
		const int32 OwnerIndex = System.OwnerCivID.IsNone() ? INDEX_NONE : Index.FindCiv(State, System.OwnerCivID);
		// Whether the vessel is an embodied ruler, and of which people: the paused mark and the commands read it.
		const int32 RuledIndex = State.Player.RuledCivID.IsNone() ? INDEX_NONE : Index.FindCiv(State, State.Player.RuledCivID);
		const bool bRules = State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler && State.Civs.IsValidIndex(RuledIndex);
		NameText->SetText(System.DisplayName);

		TArray<FText> Lines;
		if (bDead)
		{
			Lines.Add(Label(TEXT("Panel.Destroyed")));
		}
		else
		{
			const FString Letter = StarClassLetter(System.StarClass);
			if (!Letter.IsEmpty())
			{
				Lines.Add(FText::Format(INVTEXT("{0}  {1}"), Label(TEXT("Panel.StarClass")), FText::AsCultureInvariant(Letter)));
			}
			Lines.Add(OwnerLineOf(State, View, SystemIndex));
			// The owner people's size, as its band only (section 8.3), for a people the viewer is shown: never asked of a
			// wild system or an owner the state does not hold - GetPopulationBand reads those as Few.
			const ECosmicOwnerShown OwnerShown = CosmicMapLook::OwnerShownOf(State, View, SystemIndex);
			if (OwnerShown == ECosmicOwnerShown::Shown && State.Civs.IsValidIndex(OwnerIndex))
			{
				Lines.Add(PeopleLine(Sim->GetPopulationBand(State.Civs[OwnerIndex].CivID)));
			}
			// How much a shown people looks up to the vessel, for a wanderer (B1 - what a coronation reads; a number of the
			// screen only).
			if (OwnerShown == ECosmicOwnerShown::Shown && State.Civs.IsValidIndex(OwnerIndex) && CosmicWanderer::IsEmbodiedWanderer(State))
			{
				Lines.Add(FText::Format(Label(TEXT("Wander.Reverence")), Percent(State.Civs[OwnerIndex].ReverenceToPlayer)));
			}
			// The scan that learned nothing here (B1), while the vessel still stands here.
			if (!ScanNothingAt.IsNone() && System.SystemID == ScanNothingAt && CosmicWanderer::IsEmbodiedWanderer(State)
				&& State.Player.CurrentSystemID == ScanNothingAt && !CosmicWanderer::IsTravelling(State))
			{
				Lines.Add(Label(TEXT("Wander.ScanNothing")));
			}
			// The life of the world (section 5.7), after the people: the world's other life (none on the sanctuary, whose
			// owner line says it is kept).
			const FText Life = LifeLine(State.Seed, System.SystemID);
			if (!Life.IsEmpty())
			{
				Lines.Add(Life);
			}
			if (System.ActiveHazard.IsValid())
			{
				const FHazardDef* Hazard = Tables.FindHazardByTag(System.ActiveHazard);
				const FText HazardName = Hazard ? Hazard->DisplayName : FText::AsCultureInvariant(System.ActiveHazard.ToString());
				// The countdown in the clock's unit (the viewer's choice, guideline 6.3).
				Lines.Add(FText::Format(INVTEXT("{0}  {1}   {2}"), Label(TEXT("Panel.Hazard")), HazardName,
					TimeLeftLabel(System.HazardTicksRemaining, Unit)));
			}
			// A slot line takes the paused mark while its watch says the slot does not pay - on a system of the people the
			// vessel rules only: on another people's slot the mark would tell its stockpile. The watches keep to that people
			// too (WatchSlots), but a refresh inside a Core Logic broadcast does not step them, so the line asks the owner
			// itself.
			const bool bMarksSlots = bRules && OwnerIndex == RuledIndex;
			auto Marked = [&](const FText& SlotText, CosmicMegaprojects::ECosmicProjectSlot Which)
			{
				return (bMarksSlots && !SlotText.IsEmpty() && IsSlotStalled(System.SystemID, Which)) ? JoinList({ SlotText, Label(TEXT("Panel.Stalled")) }) : SlotText;
			};
			const FText SlotLines[] = {
				Marked(SlotLine(TEXT("Panel.Construction"), Tables, System.ActiveMegaproject, System.MegaprojectProgress), CosmicMegaprojects::ECosmicProjectSlot::Construction),
				Marked(SlotLine(TEXT("Panel.Response"), Tables, System.ResponseMegaproject, System.ResponseProgress), CosmicMegaprojects::ECosmicProjectSlot::Response),
			};
			// The slots are the owner's works: none of them for a people the viewer is not shown.
			for (const FText& SlotText : SlotLines)
			{
				if (!SlotText.IsEmpty() && OwnerShown != ECosmicOwnerShown::Hidden)
				{
					Lines.Add(SlotText);
				}
			}
			// The edict that answers the hazard standing here (B2): on a settled system of the ruled people only - another
			// people's edicts are never shown.
			if (System.ActiveHazard.IsValid() && bRules && OwnerIndex == RuledIndex && CosmicState::IsSettled(System))
			{
				// In an ended run only the held form stays (a fact); the other one invites a command no longer taken.
				const FHazardDef* Standing = Tables.FindHazardByTag(System.ActiveHazard);
				if (Standing && (!bEnded || !CosmicEdicts::HeldAnswerOf(State.Civs[RuledIndex], *Standing, Tables).IsNone()))
				{
					const FText EdictLine = EdictAnswerLineOf(State.Civs[RuledIndex], *Standing, Tables);
					if (!EdictLine.IsEmpty())
					{
						Lines.Add(EdictLine);
					}
				}
			}
			// The sight names the fallen one that marks the system by its title, in the spiritual view only.
			if (ACosmicPlayerController::ShownViewModeFor(*this) == ECosmicViewMode::Spiritual)
			{
				SpiritWords = SpiritTipOf(CosmicMapLook::SpiritOf(State, Index, Tables, Sim->GetRules(), SystemIndex), Tables);
			}
		}
		DetailText->SetText(FText::Join(INVTEXT("\n"), Lines));

		// The commands of an embodied ruler (section 5.7): the same rules as StartMegaproject applies when pressed.
		if (bRules && !bDead && OwnerIndex == RuledIndex && CosmicState::IsSettled(System))
		{
			FocusCiv = &State.Civs[RuledIndex];
			// Every row that belongs on this system, in lexical order, so a button that greys does not move under the
			// cursor: enabled where the rule takes it, greyed with a short reason and its words where the rung or a slot
			// stands in the way. A row whose reason has no words does not belong here (an answer to no hazard standing
			// here, project_settle - the settle button is separate).
			TArray<FName> Rows = Tables.Megaprojects ? Tables.Megaprojects->GetRowNames() : TArray<FName>();
			Rows.Sort(FNameLexicalLess());
			for (const FName Row : Rows)
			{
				const FMegaprojectDef* Def = Tables.FindMegaproject(Row);
				const CosmicMegaprojects::ECosmicStartBlock Block = CosmicMegaprojects::StartBlockOf(State, Index, Tables, SystemIndex, Row);
				const FText Reason = StartReasonOf(Block, Tables, Row);
				if (!Def || (Block != CosmicMegaprojects::ECosmicStartBlock::None && Reason.IsEmpty()))
				{
					continue;
				}
				const FText Name = FText::Format(INVTEXT("{0}  {1}"), Label(TEXT("Panel.Start")), Def->DisplayName);
				const FText Suffix = bEnded ? Label(TEXT("Chronicle.Suffix")) : StartSuffixOf(Block, Tables, Row);
				const FText ButtonLabel = Suffix.IsEmpty() ? Name : FText::Format(INVTEXT("{0}  {1}"), Name, Suffix);
				Wanted.Add({ Row, ButtonLabel, bEnded ? EndBlock : Reason, bEnded ? EndBlock : JoinLines({ Reason, StartHintOf(Tables, Row, System, Unit) }) });
				bEnabledProject |= !bEnded && Block == CosmicMegaprojects::ECosmicStartBlock::None;
				ShownStartBlocks.Emplace(Row, Block);
			}
		}
		else if (bRules && !bDead && OwnerIndex != RuledIndex)
		{
			// Every living system that is not the people's shows the settle button; refused, it is greyed out and says why
			// (the player sees what to do next - rise a rung, wait for the settlement under way, settle one lane from home).
			// Its cost follows the reason, except where no settlement can start whatever the player does (another people
			// lives there, no settle row).
			ShownSettleBlock = CosmicMegaprojects::SettleBlockOf(State, Index, Tables, Sim->GetRules(), RuledIndex, SystemIndex);
			const FText Reason = ReasonOf(ShownSettleBlock, Tables);
			// A reason added to ECosmicSettleBlock decides here whether its button shows the cost: the sanctuary's shows
			// none, as Owned and NoRow (guideline 6.3).
			static_assert(static_cast<int32>(CosmicMegaprojects::ECosmicSettleBlock::Count) == 11, "A new settle reason: decide whether its settle button shows the cost line");
			const bool bHint = ShownSettleBlock != CosmicMegaprojects::ECosmicSettleBlock::Owned && ShownSettleBlock != CosmicMegaprojects::ECosmicSettleBlock::NoRow
				&& ShownSettleBlock != CosmicMegaprojects::ECosmicSettleBlock::Sanctuary;
			Wanted.Add({ CosmicRows::ProjectSettle, Label(TEXT("Panel.Settle")), bEnded ? EndBlock : Reason,
				bEnded ? EndBlock : JoinLines({ Reason, bHint ? StartHintOf(Tables, CosmicRows::ProjectSettle, System, Unit) : FText::GetEmpty() }) });
			bSettleButton = true;
		}
		bCapital = bRules && !bDead && OwnerIndex == RuledIndex && System.SystemID == State.Civs[RuledIndex].HomeSystemID;

		// The wanderer's commands (B1 header section 4): the commands' own gates, fed the knowing they need from the view
		// (rule 14); the first reason greys the button, its short words after the name, its long words first in the tooltip.
		if (!bDead && CosmicWanderer::IsEmbodiedWanderer(State))
		{
			const bool bCardWaiting = !View.PendingPetitionRow.IsNone();
			const int32 Here = Index.FindSystem(State, State.Player.CurrentSystemID);
			auto Want = [&](FName Action, const FText& Name, const FText& Short, const FText& Reason, const TArray<FText>& TipLines)
			{
				const FText Suffix = bEnded ? Label(TEXT("Chronicle.Suffix")) : Short;
				TArray<FText> Tip = { Reason };
				Tip.Append(TipLines);
				WandererWanted.Add({ Action, Suffix.IsEmpty() ? Name : FText::Format(INVTEXT("{0}  {1}"), Name, Suffix), bEnded ? EndBlock : Reason,
					bEnded ? EndBlock : JoinLines(Tip) });
			};
			if (SystemIndex == Here)
			{
				// [Scan]: refused only under way (no card waits for it).
				const bool bUnderWay = CosmicWanderer::IsTravelling(State);
				Want(ScanAction, Label(TEXT("Wander.Scan")), bUnderWay ? Label(TEXT("Wander.Suffix.Travelling")) : FText::GetEmpty(),
					bUnderWay ? Label(TEXT("Wander.Block.Travelling")) : FText::GetEmpty(), { Label(TEXT("Wander.ScanTip")) });
				// [Crown] or [Return]: on a settled system of a people.
				if (State.Civs.IsValidIndex(OwnerIndex) && CosmicState::IsSettled(System))
				{
					const FCivilizationState& Owner = State.Civs[OwnerIndex];
					const CosmicWanderer::ECrownBlock Block = CosmicWanderer::CrownBlockOf(State, Index, Sim->GetRules(), Owner.CivID, View.IsMet(OwnerIndex), bCardWaiting);
					const TCHAR* const LongKey = CrownKeyOf(Block, false);
					if (Block == CosmicWanderer::ECrownBlock::None || LongKey)
					{
						ShownCrownBlock = Block;
						const bool bReturn = CosmicWanderer::IsReturn(State, Owner.CivID);
						// The lock's time left, counted down as the clock counts: on the label only - a tooltip that changed every
						// tick would close every tick.
						const FText Left = TimeLeftLabel(FMath::Max<int64>(0, Owner.ThroneLockedUntilTick - State.Tick), Unit);
						auto Words = [&](const TCHAR* Key, bool bShort)
						{
							return !Key ? FText::GetEmpty() : ((bShort && Block == CosmicWanderer::ECrownBlock::ThroneLocked) ? FText::Format(Label(Key), Left) : Label(Key));
						};
						TArray<FText> TipLines;
						// What a refused crown of a shown people lacks: its reverence and the two thresholds (numbers of the screen).
						if (Block == CosmicWanderer::ECrownBlock::Lacking && CosmicMapLook::OwnerShownOf(State, View, SystemIndex) == ECosmicOwnerShown::Shown)
						{
							TipLines.Add(FText::Format(Label(TEXT("Wander.Block.LackingTip")), Percent(Owner.ReverenceToPlayer),
								Percent(Sim->GetRules().CoronationReverence), Percent(Sim->GetRules().CoronationStabilityBelow)));
						}
						TipLines.Add(Label(bReturn ? TEXT("Wander.ReturnTip") : TEXT("Wander.CrownTip")));
						Want(CrownAction, Label(bReturn ? TEXT("Wander.Return") : TEXT("Wander.Crown")), Words(CrownKeyOf(Block, true), true), Words(LongKey, false), TipLines);
					}
				}
			}
			else if (State.Systems.IsValidIndex(Here) && State.Systems[Here].Lanes.Contains(System.SystemID) && System.GalaxyRow == State.Systems[Here].GalaxyRow)
			{
				// [Travel]: on a living lane neighbour, with the lane's time.
				const CosmicWanderer::ETravelBlock Block = CosmicWanderer::TravelBlockOf(State, Index, System.SystemID, bCardWaiting);
				const TCHAR* const LongKey = TravelKeyOf(Block, false);
				if (Block == CosmicWanderer::ETravelBlock::None || LongKey)
				{
					ShownTravelBlock = Block;
					const TCHAR* const ShortKey = TravelKeyOf(Block, true);
					Want(TravelAction, FText::Format(Label(TEXT("Wander.Travel")), DurationLabel(CosmicWanderer::LaneTicksOf(Sim->GetRules()), Unit)),
						ShortKey ? Label(ShortKey) : FText::GetEmpty(), LongKey ? Label(LongKey) : FText::GetEmpty(), { Label(TEXT("Wander.TravelTip")) });
				}
			}
		}
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}

	// The project buttons, then the wanderer's row (B1): each list built again only when its actions change.
	SyncButtons(*WidgetTree, *ButtonBox, ActionButtons, Wanted, [this](UCosmicActionButton& Button)
	{
		Button.OnAction.AddUObject(this, &UCosmicSystemPanelWidget::HandleAction);
	});
	SyncButtons(*WidgetTree, *WandererBox, WandererButtons, WandererWanted, [this](UCosmicActionButton& Button)
	{
		Button.OnAction.AddUObject(this, &UCosmicSystemPanelWidget::HandleWanderer);
	});

	// The ruler's [Abdicate] on the capital (B1): armed by a first press, greyed while a role effect waits for the tick (the
	// abdication itself, or a deposition), greyed with the end's reason in an ended run.
	AbdicateButton->SetVisibility(bCapital ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (bCapital)
	{
		// A role effect waits for the tick: the abdication itself, or a fall (the throne card's failed check) - each in its
		// own words.
		static const FGameplayTag AbdicatedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Abdicated")));
		const bool bLeaving = State.Calling.PendingEffect.IsValid();
		const bool bAbdicating = bLeaving && State.Calling.PendingCause == AbdicatedTag;
		const bool bArmed = !bLeaving && IsAbdicateArmed(FPlatformTime::Seconds());
		const FText Name = Label(bArmed ? TEXT("Wander.AbdicateAgain") : TEXT("Wander.Abdicate"));
		const FText Suffix = bEnded ? Label(TEXT("Chronicle.Suffix")) : (bLeaving ? Label(TEXT("Wander.Suffix.Leaving")) : FText::GetEmpty());
		const FText AbdicateLabel = Suffix.IsEmpty() ? Name : FText::Format(INVTEXT("{0}  {1}"), Name, Suffix);
		if (!SameWords(AbdicateButton->GetLabel(), AbdicateLabel))
		{
			AbdicateButton->Setup(AbdicateAction, 0.f, AbdicateLabel);
		}
		AbdicateButton->SetIsEnabled(!bEnded && !bLeaving);
		const FText Tip = bEnded ? EndBlock
			: (bAbdicating ? JoinLines({ Label(TEXT("Wander.Block.Leaving")), Label(TEXT("Wander.AbdicateTip")) })
				: (bLeaving ? Label(TEXT("Wander.Block.Deposed")) : Label(TEXT("Wander.AbdicateTip"))));
		if (!SameWords(AbdicateButton->GetToolTipText(), Tip))
		{
			AbdicateButton->SetToolTipText(Tip);
		}
	}
	else
	{
		AbdicateArmedAt = -1.0;
	}

	// The reason line under a greyed-out settle button.
	const FText Reason = (bEnded && bSettleButton) ? EndBlock : ReasonOf(ShownSettleBlock, Tables);
	if (!SameWords(ReasonText->GetText(), Reason))
	{
		ReasonText->SetText(Reason);
	}
	ReasonText->SetVisibility(Reason.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);

	// The hint line (MVP item 6): what the panel shows decides it, not who selected the system.
	ACosmicPlayerController* Owner = Cast<ACosmicPlayerController>(GetOwningPlayer());
	const bool bHint = Owner && ShowsCommandHint(bEnabledProject, bEnded, Owner->GetHintProgress());
	HintText->SetVisibility(bHint ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	// The first ship's line (B1): on the capital's panel, once a session - it stays for the viewing it came in.
	const bool bThisViewing = !ShipHintSystem.IsNone() && ShipHintSystem == ShownSystemID;
	const bool bShip = Owner && ShowsShipHint(bCapital, State.Tick, bEnded, !State.Player.FormerCivID.IsNone(), Owner->GetHintProgress(), bThisViewing);
	if (bShip && !Owner->GetHintProgress().bShipHinted)
	{
		Owner->NoteShipHinted();
	}
	ShipHintSystem = bShip ? ShownSystemID : NAME_None;
	ShipHintText->SetVisibility(bShip ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	// The edict hint (B2): on the capital's panel while it stands. The controller steps it first (the top bar steps it too -
	// whichever redraws first).
	if (Owner)
	{
		Owner->FollowEdictHint();
	}
	const FText EdictHint = (Owner && bCapital && !bEnded && ShowsEdictHint(Owner->GetHintProgress())) ? UCosmicEdictWidget::HintOf(Tables, Sim->GetRules()) : FText::GetEmpty();
	if (!SameWords(EdictHintText->GetText(), EdictHint))
	{
		EdictHintText->SetText(EdictHint);
	}
	EdictHintText->SetVisibility(EdictHint.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);

	// The research focus (MVP item 7): the heading names the focus now; the button that is it is greyed out, the others
	// say the shares a press would give, a field the next rung reads says so. Words only when they change (a tooltip set
	// again is a new tooltip and closes an open one); the shares move only by a command, the rung only by an ascent.
	FocusBox->SetVisibility(FocusCiv ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (FocusCiv)
	{
		const TMap<FName, float>& Focus = FocusCiv->ResearchFocus;
		const float Share = Sim->GetRules().ResearchFocusShare;
		// By preset (B2 section 2.8): the heading names the preset the shares are, and a button is greyed only when its press
		// would change nothing - shares set by hand leave the four pressable.
		const FText Heading = FText::Format(Label(TEXT("Focus.Heading")), FocusWordOf(Focus, Share));
		if (!SameWords(FocusHeading->GetText(), Heading))
		{
			FocusHeading->SetText(Heading);
		}
		const TArray<FCosmicRungPart> Needed = CosmicNextRung::ResearchFieldsOf(CosmicNextRung::AllOf(State, Index, Tables, Sim->GetRules()));
		for (UCosmicActionButton* Button : FocusButtons)
		{
			const FName Field = Button->GetAction();
			const bool bNow = IsPresetNow(Focus, Field, Share);
			FText Tip;
			if (bEnded)
			{
				Tip = EndBlock;
			}
			else if (bNow)
			{
				Tip = FText::Format(Label(Field.IsNone() ? TEXT("Focus.TipNowEven") : TEXT("Focus.TipNow")), FocusSharesOf(Focus));
			}
			else
			{
				// The shares a press would give, by the rule the command applies.
				TMap<FName, float> Would = Focus;
				if (Field.IsNone())
				{
					CosmicResearch::Even(Would);
				}
				else
				{
					CosmicResearch::Focus(Would, Field, Share);
				}
				Tip = FText::Format(Label(Field.IsNone() ? TEXT("Focus.TipEven") : TEXT("Focus.TipField")), FocusSharesOf(Would));
			}
			const bool bNeeded = !Field.IsNone() && Needed.ContainsByPredicate([Field](const FCosmicRungPart& Part) { return Part.Key == Field; });
			if (!bEnded && bNeeded)
			{
				Tip = JoinLines({ Tip, Label(TEXT("Focus.TipNeeded")) });
			}
			// What Society research opens (B2): the edicts' next slot, and the faster recovery while the knob gives one.
			if (!bEnded && Field == FName(TEXT("Society")))
			{
				const TOptional<float> NextMark = UCosmicEdictWidget::NextSlotMarkOf(*FocusCiv, Sim->GetRules());
				const float Bonus = Sim->GetRules().SocietyRecoveryBonus;
				Tip = JoinLines({ Tip, NextMark.IsSet() ? FText::Format(Label(TEXT("Focus.TipOpensSlot")), Percent(NextMark.GetValue())) : Label(TEXT("Focus.TipOpensAll")),
					(Bonus > 0.f && Bonus <= TNumericLimits<float>::Max()) ? Label(TEXT("Focus.TipOpensRecovery")) : FText::GetEmpty() });
			}
			Button->SetIsEnabled(!bEnded && !bNow);
			if (!SameWords(Button->GetToolTipText(), Tip))
			{
				Button->SetToolTipText(Tip);
			}
		}
		// [Share by hand]: greyed with the end's reason in an ended run.
		DirectButton->SetIsEnabled(!bEnded);
		const FText DirectTip = bEnded ? EndBlock : Label(TEXT("Focus.DirectTip"));
		if (!SameWords(DirectButton->GetToolTipText(), DirectTip))
		{
			DirectButton->SetToolTipText(DirectTip);
		}
	}

	// The sight's words (none in the physical view): a line of their own under the details, with the same words as its
	// tooltip - a block apart, so that neither the line nor the tooltip covers the people's band (a claim is never told
	// with a size, section 8.4). Set only when the words change: the panel redraws every tick, and a new tooltip closes
	// an open one.
	if (!SameWords(MarkText->GetText(), SpiritWords))
	{
		MarkText->SetText(SpiritWords);
	}
	if (!SameWords(MarkText->GetToolTipText(), SpiritWords))
	{
		MarkText->SetToolTipText(SpiritWords);
	}
	MarkText->SetVisibility(SpiritWords.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}

FReply UCosmicSystemPanelWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Only a hit-testable part of the panel reaches here (the widget and its root are self-hit-test-invisible).
	return FReply::Handled();
}

FReply UCosmicSystemPanelWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

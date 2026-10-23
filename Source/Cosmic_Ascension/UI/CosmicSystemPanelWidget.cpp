// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicSystemPanelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
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
#include "Layout/Margin.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UI/CosmicActionButton.h"
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
}

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

		UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
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
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}

	// The buttons are built again only when the list of their actions changes. A click spans a press and a release, a
	// button built again between them drops it, and the simulation ticks up to 25 times a second (x25): otherwise each button
	// keeps its object and takes its label, state and tooltip in place.
	bool bSameActions = ActionButtons.Num() == Wanted.Num();
	for (int32 At = 0; bSameActions && At < Wanted.Num(); ++At)
	{
		bSameActions = ActionButtons[At] && ActionButtons[At]->GetAction() == Wanted[At].Action;
	}
	if (!bSameActions)
	{
		ButtonBox->ClearChildren();
		ActionButtons.Reset();
		for (const FWantedButton& Want : Wanted)
		{
			UCosmicActionButton* Button = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
			Button->Setup(Want.Action, 0.f, Want.Label);
			Button->OnAction.AddUObject(this, &UCosmicSystemPanelWidget::HandleAction);
			ButtonBox->AddChildToVerticalBox(Button)->SetPadding(FMargin(0.f, 2.f));
			ActionButtons.Add(Button);
		}
	}
	for (int32 At = 0; At < Wanted.Num(); ++At)
	{
		UCosmicActionButton* Button = ActionButtons[At];
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

	// The reason line under a greyed-out settle button.
	const FText Reason = (bEnded && bSettleButton) ? EndBlock : ReasonOf(ShownSettleBlock, Tables);
	if (!SameWords(ReasonText->GetText(), Reason))
	{
		ReasonText->SetText(Reason);
	}
	ReasonText->SetVisibility(Reason.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);

	// The hint line (MVP item 6): what the panel shows decides it, not who selected the system.
	const ACosmicPlayerController* Owner = Cast<ACosmicPlayerController>(GetOwningPlayer());
	const bool bHint = Owner && ShowsCommandHint(bEnabledProject, bEnded, Owner->GetHintProgress());
	HintText->SetVisibility(bHint ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

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

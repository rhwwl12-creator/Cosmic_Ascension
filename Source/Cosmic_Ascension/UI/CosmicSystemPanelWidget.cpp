// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicSystemPanelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
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

	// A command button the panel shows: what it does, its label, and why it is greyed out (empty: enabled).
	struct FWantedButton
	{
		FName Action;
		FText Label;
		FText Reason;
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
	Refresh();
}

void UCosmicSystemPanelWidget::ShowSystem(FName SystemID)
{
	ShownSystemID = SystemID;
	Refresh();
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
	const FGalaxyState& State = Sim->GetState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicDataTables& Tables = Sim->GetTables();
	const int32 SystemIndex = ShownSystemID.IsNone() ? INDEX_NONE : Index.FindSystem(State, ShownSystemID);

	TArray<FWantedButton> Wanted;
	ShownSettleBlock = CosmicMegaprojects::ECosmicSettleBlock::None;
	// The spiritual sight's words for the shown system (section 6.4): a line of their own and its tooltip.
	FText SpiritWords;
	if (State.Systems.IsValidIndex(SystemIndex))
	{
		// The box takes the clicks on it (NativeOnMouseButtonDown); the panel's empty area does not.
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		const FStarSystemState& System = State.Systems[SystemIndex];
		const bool bDead = CosmicHazards::IsDeadNode(System);
		const int32 OwnerIndex = System.OwnerCivID.IsNone() ? INDEX_NONE : Index.FindCiv(State, System.OwnerCivID);
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
			Lines.Add(State.Civs.IsValidIndex(OwnerIndex) ? FText::Format(INVTEXT("{0}  {1}"), Label(TEXT("Panel.Owner")), State.Civs[OwnerIndex].CivName) : Label(TEXT("Panel.NoOwner")));
			// The owner people's size, as its band only (section 8.3): never asked of a wild system or an owner the state
			// does not hold - GetPopulationBand reads those as Few.
			if (State.Civs.IsValidIndex(OwnerIndex))
			{
				Lines.Add(PeopleLine(Sim->GetPopulationBand(State.Civs[OwnerIndex].CivID)));
			}
			if (System.ActiveHazard.IsValid())
			{
				const FHazardDef* Hazard = Tables.FindHazardByTag(System.ActiveHazard);
				const FText HazardName = Hazard ? Hazard->DisplayName : FText::AsCultureInvariant(System.ActiveHazard.ToString());
				// The countdown in the clock's unit (the viewer's choice, guideline 6.3).
				Lines.Add(FText::Format(INVTEXT("{0}  {1}   {2}"), Label(TEXT("Panel.Hazard")), HazardName,
					TimeLeftLabel(System.HazardTicksRemaining, ACosmicPlayerController::TimeUnitFor(*this))));
			}
			const FText SlotLines[] = {
				SlotLine(TEXT("Panel.Construction"), Tables, System.ActiveMegaproject, System.MegaprojectProgress),
				SlotLine(TEXT("Panel.Response"), Tables, System.ResponseMegaproject, System.ResponseProgress),
			};
			for (const FText& SlotText : SlotLines)
			{
				if (!SlotText.IsEmpty())
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
		const int32 RuledIndex = State.Player.RuledCivID.IsNone() ? INDEX_NONE : Index.FindCiv(State, State.Player.RuledCivID);
		const bool bRules = State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler && State.Civs.IsValidIndex(RuledIndex);
		if (bRules && !bDead && OwnerIndex == RuledIndex && CosmicState::IsSettled(System))
		{
			TArray<FName> Rows = Tables.Megaprojects ? Tables.Megaprojects->GetRowNames() : TArray<FName>();
			Rows.Sort(FNameLexicalLess());
			for (const FName Row : Rows)
			{
				const FMegaprojectDef* Def = Tables.FindMegaproject(Row);
				// CanStart never takes project_settle: a settlement starts on the system to settle (the settle button).
				if (Def && CosmicMegaprojects::CanStart(State, Index, Tables, SystemIndex, Row))
				{
					Wanted.Add({ Row, FText::Format(INVTEXT("{0}  {1}"), Label(TEXT("Panel.Start")), Def->DisplayName), FText::GetEmpty() });
				}
			}
		}
		else if (bRules && !bDead && OwnerIndex != RuledIndex)
		{
			// Every living system that is not the people's shows the settle button; refused, it is greyed out and says why
			// (the player sees what to do next - rise a rung, wait for the settlement under way, settle one lane from home).
			ShownSettleBlock = CosmicMegaprojects::SettleBlockOf(State, Index, Tables, Sim->GetRules(), RuledIndex, SystemIndex);
			Wanted.Add({ CosmicRows::ProjectSettle, Label(TEXT("Panel.Settle")), ReasonOf(ShownSettleBlock, Tables) });
		}
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}

	// The buttons are built again only when the list of their actions changes. A click spans a press and a release, a
	// button built again between them drops it, and the simulation ticks up to ten times a second: otherwise each button
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
		if (!SameWords(Button->GetToolTipText(), Wanted[At].Reason))
		{
			Button->SetToolTipText(Wanted[At].Reason);
		}
	}

	// The reason line under a greyed-out settle button.
	const FText Reason = ReasonOf(ShownSettleBlock, Tables);
	if (!SameWords(ReasonText->GetText(), Reason))
	{
		ReasonText->SetText(Reason);
	}
	ReasonText->SetVisibility(Reason.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);

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

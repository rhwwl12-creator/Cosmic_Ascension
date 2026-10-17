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

	// The systems the people CivIndex owns, as MaySettle counts them (an entry whose owner is another people counts for nothing).
	int32 OwnedCount(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex)
	{
		int32 Owned = 0;
		for (const FName SystemID : State.Civs[CivIndex].OwnedSystems)
		{
			const int32 SystemIndex = Index.FindSystem(State, SystemID);
			Owned += (State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == State.Civs[CivIndex].CivID) ? 1 : 0;
		}
		return Owned;
	}

	// Whether System lies one lane from a settled system of the people CivIndex.
	bool NextToSettled(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex, const FStarSystemState& System)
	{
		for (const FName Lane : System.Lanes)
		{
			const int32 Other = Index.FindSystem(State, Lane);
			if (State.Systems.IsValidIndex(Other) && State.Systems[Other].OwnerCivID == State.Civs[CivIndex].CivID && CosmicState::IsSettled(State.Systems[Other]))
			{
				return true;
			}
		}
		return false;
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
		ButtonBox = WidgetTree->ConstructWidget<UVerticalBox>();
		Column->AddChildToVerticalBox(ButtonBox);
		RealmCapText = CosmicUI::MakeText(*WidgetTree, 13);
		RealmCapText->SetText(CosmicUI::Label(TEXT("Panel.RealmCap")));
		RealmCapText->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.9f)));
		RealmCapText->SetVisibility(ESlateVisibility::Collapsed);
		Column->AddChildToVerticalBox(RealmCapText)->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
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

bool UCosmicSystemPanelWidget::IsRealmCapShown() const
{
	return RealmCapText && RealmCapText->GetVisibility() != ESlateVisibility::Collapsed;
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

	ButtonBox->ClearChildren();
	ActionButtons.Reset();
	RealmCapText->SetVisibility(ESlateVisibility::Collapsed);
	if (!State.Systems.IsValidIndex(SystemIndex))
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
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
		if (System.ActiveHazard.IsValid())
		{
			const FHazardDef* Hazard = Tables.FindHazardByTag(System.ActiveHazard);
			const FText HazardName = Hazard ? Hazard->DisplayName : FText::AsCultureInvariant(System.ActiveHazard.ToString());
			Lines.Add(FText::Format(INVTEXT("{0}  {1}   {2} {3}"), Label(TEXT("Panel.Hazard")), HazardName, Label(TEXT("Panel.CyclesLeft")), FText::AsNumber(System.HazardTicksRemaining)));
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
	}
	DetailText->SetText(FText::Join(INVTEXT("\n"), Lines));

	// The commands of an embodied ruler (section 5.7): the same rules as StartMegaproject applies when pressed.
	const int32 RuledIndex = State.Player.RuledCivID.IsNone() ? INDEX_NONE : Index.FindCiv(State, State.Player.RuledCivID);
	const bool bRules = State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler && State.Civs.IsValidIndex(RuledIndex);
	if (!bRules || bDead)
	{
		return;
	}
	auto AddButton = [this](FName Action, const FText& ButtonLabel)
	{
		UCosmicActionButton* Button = WidgetTree->ConstructWidget<UCosmicActionButton>(UCosmicActionButton::StaticClass());
		Button->Setup(Action, 0.f, ButtonLabel);
		Button->OnAction.AddUObject(this, &UCosmicSystemPanelWidget::HandleAction);
		ButtonBox->AddChildToVerticalBox(Button)->SetPadding(FMargin(0.f, 2.f));
		ActionButtons.Add(Button);
	};
	if (OwnerIndex == RuledIndex && CosmicState::IsSettled(System))
	{
		TArray<FName> Rows = Tables.Megaprojects ? Tables.Megaprojects->GetRowNames() : TArray<FName>();
		Rows.Sort(FNameLexicalLess());
		for (const FName Row : Rows)
		{
			const FMegaprojectDef* Def = Tables.FindMegaproject(Row);
			// CanStart never takes project_settle: a settlement starts on the wild system (the settle button).
			if (Def && CosmicMegaprojects::CanStart(State, Index, Tables, SystemIndex, Row))
			{
				AddButton(Row, FText::Format(INVTEXT("{0}  {1}"), Label(TEXT("Panel.Start")), Def->DisplayName));
			}
		}
	}
	else if (System.OwnerCivID.IsNone())
	{
		if (CosmicMegaprojects::CanSettle(State, Index, Tables, Sim->GetRules(), RuledIndex, SystemIndex))
		{
			AddButton(CosmicRows::ProjectSettle, Label(TEXT("Panel.Settle")));
		}
		else if (NextToSettled(State, Index, RuledIndex, System)
			&& OwnedCount(State, Index, RuledIndex) >= CosmicMegaprojects::RealmCapOf(Sim->GetRules(), State.Civs[RuledIndex].Tier))
		{
			// Section 5.12: wider only by rising.
			RealmCapText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
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

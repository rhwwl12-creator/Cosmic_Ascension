// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicTextPanelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "Fonts/SlateFontInfo.h"
#include "Internationalization/StringTableCoreFwd.h"
#include "Internationalization/Text.h"
#include "Layout/Margin.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Log/CosmicLogSubsystem.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/GalaxySimulationSubsystem.h"

namespace
{
	// String Table with the UI labels (section 8.5).
	const FName LabelTableId(TEXT("/Game/CA/Data/ST_CA.ST_CA"));

	// Log lines shown at once. The virtualized list of Phase 3 lifts this.
	constexpr int32 MaxShownLogLines = 60;

	// A blocking load on purpose. The default policy loads the table asynchronously while anything else is loading, and
	// the lines formatted meanwhile would keep the placeholder of a missing entry.
	FText Label(const TCHAR* Key)
	{
		return FText::FromStringTable(LabelTableId, Key, EStringTableLoadingPolicy::FindOrFullyLoad);
	}

	FText WholeNumber(float Value)
	{
		return FText::AsNumber(FMath::FloorToInt32(Value));
	}

	FText Percent(float Ratio)
	{
		FNumberFormattingOptions Options;
		Options.MaximumFractionalDigits = 0;
		return FText::AsPercent(Ratio, &Options);
	}

	UTextBlock* MakeText(UWidgetTree& Tree, int32 FontSize)
	{
		UTextBlock* Text = Tree.ConstructWidget<UTextBlock>();
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = FontSize;
		Font.TypefaceFontName = FName(TEXT("Regular"));
		Text->SetFont(Font);
		Text->SetAutoWrapText(true);
		return Text;
	}
}

TSharedRef<SWidget> UCosmicTextPanelWidget::RebuildWidget()
{
	// RebuildWidget can run again after the Slate widgets are released; the tree is built once.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>();
		WidgetTree->RootWidget = Root;

		StatusText = MakeText(*WidgetTree, 16);
		Root->AddChildToVerticalBox(StatusText)->SetPadding(FMargin(32.f, 24.f, 32.f, 12.f));

		LogText = MakeText(*WidgetTree, 14);
		LogScroll = WidgetTree->ConstructWidget<UScrollBox>();
		LogScroll->AddChild(LogText);
		UVerticalBoxSlot* LogSlot = Root->AddChildToVerticalBox(LogScroll);
		LogSlot->SetPadding(FMargin(32.f, 0.f, 32.f, 24.f));
		LogSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	return Super::RebuildWidget();
}

void UCosmicTextPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGalaxySimulationSubsystem* Sim = GameInstance->GetSubsystem<UGalaxySimulationSubsystem>())
		{
			Sim->OnSimTick.AddUniqueDynamic(this, &UCosmicTextPanelWidget::HandleSimTick);
			Sim->OnGalaxyGenerated.AddUniqueDynamic(this, &UCosmicTextPanelWidget::HandleGalaxyGenerated);
		}
		if (UCosmicLogSubsystem* Log = GameInstance->GetSubsystem<UCosmicLogSubsystem>())
		{
			Log->OnLogEntry.AddUniqueDynamic(this, &UCosmicTextPanelWidget::HandleLogEntry);
		}
	}
	Refresh();
}

void UCosmicTextPanelWidget::Refresh()
{
	RefreshStatus();
	RefreshLog();
}

void UCosmicTextPanelWidget::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGalaxySimulationSubsystem* Sim = GameInstance->GetSubsystem<UGalaxySimulationSubsystem>())
		{
			Sim->OnSimTick.RemoveDynamic(this, &UCosmicTextPanelWidget::HandleSimTick);
			Sim->OnGalaxyGenerated.RemoveDynamic(this, &UCosmicTextPanelWidget::HandleGalaxyGenerated);
		}
		if (UCosmicLogSubsystem* Log = GameInstance->GetSubsystem<UCosmicLogSubsystem>())
		{
			Log->OnLogEntry.RemoveDynamic(this, &UCosmicTextPanelWidget::HandleLogEntry);
		}
	}
	Super::NativeDestruct();
}

void UCosmicTextPanelWidget::HandleSimTick(int64 Tick)
{
	RefreshStatus();
}

void UCosmicTextPanelWidget::HandleGalaxyGenerated(int32 Seed)
{
	Refresh();
}

void UCosmicTextPanelWidget::HandleLogEntry(const FCosmicLogEntry& Entry)
{
	RefreshLog();
}

void UCosmicTextPanelWidget::RefreshStatus()
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UGalaxySimulationSubsystem* Sim = GameInstance ? GameInstance->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || !StatusText)
	{
		return;
	}
	const FGalaxyState& State = Sim->GetState();

	TArray<FText> Lines;
	const float Speed = Sim->GetTimeScale();
	Lines.Add(FText::Format(INVTEXT("{0} {1}      {2} {3}"),
		Label(TEXT("Panel.Cycle")), FText::AsNumber(State.Tick),
		Label(TEXT("Panel.Speed")), Speed > 0.f ? FText::Format(INVTEXT("x{0}"), WholeNumber(Speed)) : Label(TEXT("Panel.Paused"))));

	// The civ the player rules. A wanderer has none; the wanderer panels come with Phase 4.
	const FName RuledCivID = State.Player.RuledCivID;
	const FCivilizationState* Civ = State.Civs.FindByPredicate([RuledCivID](const FCivilizationState& Candidate) { return Candidate.CivID == RuledCivID; });
	if (Civ)
	{
		Lines.Add(FText::Format(INVTEXT("{0}      {1} {2}      {3} {4}"), Civ->CivName, Label(TEXT("Panel.Tier")), FText::AsNumber(static_cast<int32>(Civ->Tier) + 1),
			Label(TEXT("Panel.Stability")), Percent(Civ->Stability)));
		Lines.Add(FText::Format(INVTEXT("{0}      {1} {2}   {3} {4}   {5} {6}   {7} {8}   {9} {10}"),
			Label(TEXT("Panel.Resources")),
			Label(TEXT("Resource.Energy")), WholeNumber(Civ->Stockpile.Energy),
			Label(TEXT("Resource.Matter")), WholeNumber(Civ->Stockpile.Matter),
			Label(TEXT("Resource.Insight")), WholeNumber(Civ->Stockpile.Insight),
			Label(TEXT("Resource.Influence")), WholeNumber(Civ->Stockpile.Influence),
			Label(TEXT("Resource.Causality")), WholeNumber(Civ->Stockpile.Causality)));
		Lines.Add(FText::Format(INVTEXT("{0}      {1} {2}   {3} {4}   {5} {6}   {7} {8}"),
			Label(TEXT("Panel.Research")),
			Label(TEXT("Research.Physics")), Percent(Civ->Research.FindRef(FName(TEXT("Physics")))),
			Label(TEXT("Research.Society")), Percent(Civ->Research.FindRef(FName(TEXT("Society")))),
			Label(TEXT("Research.Engineering")), Percent(Civ->Research.FindRef(FName(TEXT("Engineering")))),
			Label(TEXT("Research.Transcendence")), Percent(Civ->Research.FindRef(FName(TEXT("Transcendence"))))));

		// The projects of the ruled people, both slots of every system, and the prototype's key (section 5.7).
		TArray<FText> Projects;
		const FCosmicDataTables& Tables = Sim->GetTables();
		for (const FStarSystemState& System : State.Systems)
		{
			if (System.OwnerCivID != RuledCivID)
			{
				continue;
			}
			const FName SlotRows[] = { System.ActiveMegaproject, System.ResponseMegaproject };
			const float SlotProgress[] = { System.MegaprojectProgress, System.ResponseProgress };
			for (int32 SlotIndex = 0; SlotIndex < 2; ++SlotIndex)
			{
				if (const FMegaprojectDef* Row = SlotRows[SlotIndex].IsNone() ? nullptr : Tables.FindMegaproject(SlotRows[SlotIndex]))
				{
					Projects.Add(FText::Format(INVTEXT("{0} {1} {2}/{3}"), System.DisplayName, Row->DisplayName, WholeNumber(SlotProgress[SlotIndex]), WholeNumber(CosmicMegaprojects::RequiredProgressOf(*Row))));
				}
			}
		}
		Lines.Add(FText::Format(INVTEXT("{0}      {1}      ({2})"), Label(TEXT("Panel.Projects")),
			Projects.IsEmpty() ? Label(TEXT("Panel.None")) : FText::Join(INVTEXT(", "), Projects), Label(TEXT("Panel.ProjectKey"))));
	}
	Lines.Add(FText::GetEmpty());
	Lines.Add(Label(TEXT("Panel.Log")));

	StatusText->SetText(FText::Join(INVTEXT("\n"), Lines));
}

void UCosmicTextPanelWidget::RefreshLog()
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UGalaxySimulationSubsystem* Sim = GameInstance ? GameInstance->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || !LogText)
	{
		return;
	}
	const TArray<FCosmicLogEntry>& Log = Sim->GetState().Log;

	TArray<FText> Lines;
	for (int32 Index = FMath::Max(0, Log.Num() - MaxShownLogLines); Index < Log.Num(); ++Index)
	{
		Lines.Add(FText::Format(INVTEXT("[{0}] {1}"), FText::AsNumber(Log[Index].Tick), Log[Index].Message));
	}
	LogText->SetText(FText::Join(INVTEXT("\n"), Lines));

	if (LogScroll)
	{
		LogScroll->ScrollToEnd();
	}
}

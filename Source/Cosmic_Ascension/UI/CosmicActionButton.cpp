// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicActionButton.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Fonts/SlateFontInfo.h"
#include "UObject/UnrealType.h"

void UCosmicActionButton::Setup(FName InAction, float InValue, const FText& Label)
{
	Action = InAction;
	Value = InValue;
	ShownLabel = Label;
	if (Text)
	{
		Text->SetText(ShownLabel);
	}
}

void UCosmicActionButton::Press()
{
	// A click never reaches a disabled button; a press from code is refused the same way.
	if (!GetIsEnabled())
	{
		return;
	}
	OnAction.Broadcast(Action, Value);
}

void UCosmicActionButton::HandleClicked()
{
	Press();
}

TSharedRef<SWidget> UCosmicActionButton::RebuildWidget()
{
	// RebuildWidget can run again after the Slate widgets are released; the tree is built once.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		Button = WidgetTree->ConstructWidget<UButton>();
		// Before the Slate button exists: a focusable button keeps the keyboard focus after a click and Space presses it.
		// UButton keeps its initializer of IsFocusable protected (5.8), so the property is set through reflection.
		if (FBoolProperty* Focusable = FindFProperty<FBoolProperty>(UButton::StaticClass(), TEXT("IsFocusable")))
		{
			Focusable->SetPropertyValue_InContainer(Button, false);
		}
		Button->OnClicked.AddUniqueDynamic(this, &UCosmicActionButton::HandleClicked);
		WidgetTree->RootWidget = Button;

		Text = WidgetTree->ConstructWidget<UTextBlock>();
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = 13;
		Font.TypefaceFontName = FName(TEXT("Regular"));
		Text->SetFont(Font);
		Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.05f, 0.05f, 0.08f)));
		Text->SetText(ShownLabel);
		Button->AddChild(Text);
	}
	return Super::RebuildWidget();
}

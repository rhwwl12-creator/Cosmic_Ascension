// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicActionButton.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Fonts/SlateFontInfo.h"
#include "Input/Reply.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"
#include "Types/WidgetMouseEventsDelegate.h"
#include "UObject/UnrealType.h"
#include "Widgets/SWidget.h"

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
		// The label takes the face's foreground. Slate draws a greyed-out widget at 45% opacity, face and label alike: the
		// default face (no disabled brush - a pale box) under the dark label left a greyed button hardly legible, and the
		// short reason of a greyed project button must be read (item 12). A greyed button is a near-black face under a
		// white label - at 45% the label still stands about 4 to 1 over the face; an enabled one keeps the light face under
		// the dark label.
		FButtonStyle Style = Button->GetStyle();
		const FSlateColor Ink(FLinearColor(0.05f, 0.05f, 0.08f));
		Style.SetNormalForeground(Ink).SetHoveredForeground(Ink).SetPressedForeground(Ink);
		Style.SetDisabledForeground(FSlateColor(FLinearColor::White));
		FSlateBrush Off = Style.Normal;
		Off.TintColor = FSlateColor(FLinearColor(0.02f, 0.02f, 0.025f));
		Style.SetDisabled(Off);
		Button->SetStyle(Style);
		WidgetTree->RootWidget = Button;

		Text = WidgetTree->ConstructWidget<UTextBlock>();
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = 13;
		Font.TypefaceFontName = FName(TEXT("Regular"));
		Text->SetFont(Font);
		Text->SetColorAndOpacity(FSlateColor::UseForeground());
		Text->SetText(ShownLabel);
		Button->AddChild(Text);
	}
	TSharedRef<SWidget> Built = Super::RebuildWidget();
	// A double-click's second press, handled here, is no press (the class comment). The Slate button is built again with the
	// tree's Slate widgets, so the handler is bound on every build.
	if (Button)
	{
		if (TSharedPtr<SWidget> Slate = Button->GetCachedWidget())
		{
			Slate->SetOnMouseDoubleClick(FPointerEventHandler::CreateLambda([](const FGeometry&, const FPointerEvent&) { return FReply::Handled(); }));
		}
	}
	return Built;
}

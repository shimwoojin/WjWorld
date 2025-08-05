// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Interact/InteractionWidget.h"
#include "Components/TextBlock.h"

void UInteractionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (KeyPrompt)
    {
        KeyPrompt->SetText(FText::FromString(TEXT("[F]")));
    }
}

void UInteractionWidget::SetInteractionText(const FString& Text)
{
    if (InteractionText)
    {
        InteractionText->SetText(FText::FromString(Text));
    }
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Ability/AbilityPromptWidget.h"

#include "Components/TextBlock.h"

void UAbilityPromptWidget::SetPromptInfo(const FText& ConfirmKeyName, const FText& CancelKeyName, const FText& Description)
{
	if (ConfirmText)
	{
		ConfirmText->SetText(FText::Format(
			NSLOCTEXT("AbilityPrompt", "ConfirmFormat", "{0}: 확인"),
			ConfirmKeyName
		));
	}

	if (CancelText)
	{
		CancelText->SetText(FText::Format(
			NSLOCTEXT("AbilityPrompt", "CancelFormat", "{0}: 취소"),
			CancelKeyName
		));
	}

	if (DescriptionText)
	{
		if (Description.IsEmpty())
		{
			DescriptionText->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			DescriptionText->SetText(Description);
			DescriptionText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
}

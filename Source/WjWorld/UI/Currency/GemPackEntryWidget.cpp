// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Currency/GemPackEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UGemPackEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BuyButton)
	{
		BuyButton->OnClicked.RemoveDynamic(this, &UGemPackEntryWidget::OnBuyButtonClicked);
		BuyButton->OnClicked.AddDynamic(this, &UGemPackEntryWidget::OnBuyButtonClicked);
	}
}

void UGemPackEntryWidget::SetPackDefinition(const FGemPackDefinition& InDef)
{
	CachedDefId = InDef.SteamItemDefId;

	if (PackNameText)
	{
		PackNameText->SetText(InDef.DisplayName);
	}

	if (GemAmountText)
	{
		GemAmountText->SetText(FText::Format(
			NSLOCTEXT("Currency", "GemAmountFormat", "{0} Gems"),
			FText::AsNumber(InDef.GemAmount)));
	}

	if (PriceText)
	{
		PriceText->SetText(InDef.PriceText);
	}
}

void UGemPackEntryWidget::OnBuyButtonClicked()
{
	OnBuyClicked.Broadcast(CachedDefId);
}

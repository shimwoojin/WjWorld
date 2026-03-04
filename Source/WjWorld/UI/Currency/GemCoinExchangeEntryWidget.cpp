// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Currency/GemCoinExchangeEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UGemCoinExchangeEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ExchangeButton)
	{
		ExchangeButton->OnClicked.RemoveDynamic(this, &UGemCoinExchangeEntryWidget::OnExchangeButtonClicked);
		ExchangeButton->OnClicked.AddDynamic(this, &UGemCoinExchangeEntryWidget::OnExchangeButtonClicked);
	}
}

void UGemCoinExchangeEntryWidget::SetExchangeDefinition(const FGemCoinExchangeDefinition& InDef)
{
	CachedDefId = InDef.SteamItemDefId;
	CachedGemCost = InDef.GemCost;

	if (DisplayNameText)
	{
		DisplayNameText->SetText(InDef.DisplayName);
	}

	if (GemCostText)
	{
		GemCostText->SetText(FText::Format(
			NSLOCTEXT("Currency", "GemCostFormat", "{0} Gems"),
			FText::AsNumber(InDef.GemCost)));
	}

	// 보너스 비율 계산 (기본 비율: 1 Gem = 10 Coin)
	if (BonusText && InDef.GemCost > 0)
	{
		float BaseCoins = static_cast<float>(InDef.GemCost) * 10.0f;
		float BonusPercent = ((static_cast<float>(InDef.CoinAmount) / BaseCoins) - 1.0f) * 100.0f;

		if (BonusPercent > 0.5f)
		{
			BonusText->SetText(FText::Format(
				NSLOCTEXT("Currency", "BonusFormat", "+{0}%"),
				FText::AsNumber(FMath::RoundToInt(BonusPercent))));
			BonusText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			BonusText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UGemCoinExchangeEntryWidget::UpdateButtonState(int32 CurrentGemBalance)
{
	if (ExchangeButton)
	{
		ExchangeButton->SetIsEnabled(CurrentGemBalance >= CachedGemCost);
	}
}

void UGemCoinExchangeEntryWidget::OnExchangeButtonClicked()
{
	OnExchangeClicked.Broadcast(CachedDefId);
}

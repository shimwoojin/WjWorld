// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Currency/CurrencyBalanceWidget.h"
#include "Currency/WjWorldCurrencySubsystem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"

void UCurrencyBalanceWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		if (UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>())
		{
			CurrencySub->OnCurrencyBalanceChanged.RemoveDynamic(this, &UCurrencyBalanceWidget::OnCurrencyBalanceChanged);
			CurrencySub->OnCurrencyBalanceChanged.AddDynamic(this, &UCurrencyBalanceWidget::OnCurrencyBalanceChanged);
		}
	}

	if (GemAreaButton)
	{
		GemAreaButton->OnClicked.RemoveDynamic(this, &UCurrencyBalanceWidget::OnGemAreaButtonClicked);
		GemAreaButton->OnClicked.AddDynamic(this, &UCurrencyBalanceWidget::OnGemAreaButtonClicked);
	}

	if (CoinIcon)
	{
		CoinIcon->SetToolTipText(FText::FromString(TEXT("Coin")));
	}
	if (GemIcon)
	{
		GemIcon->SetToolTipText(FText::FromString(TEXT("Gem")));
	}

	UpdateDisplay();
}

void UCurrencyBalanceWidget::NativeDestruct()
{
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		if (UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>())
		{
			CurrencySub->OnCurrencyBalanceChanged.RemoveDynamic(this, &UCurrencyBalanceWidget::OnCurrencyBalanceChanged);
		}
	}

	if (GemAreaButton)
	{
		GemAreaButton->OnClicked.RemoveDynamic(this, &UCurrencyBalanceWidget::OnGemAreaButtonClicked);
	}

	Super::NativeDestruct();
}

void UCurrencyBalanceWidget::OnGemAreaButtonClicked()
{
	OnGemAreaClicked.Broadcast();
}

void UCurrencyBalanceWidget::OnCurrencyBalanceChanged(ECurrencyType CurrencyType, int32 NewBalance)
{
	switch (CurrencyType)
	{
	case ECurrencyType::Coin:
		if (CoinAmountText)
		{
			CoinAmountText->SetText(FText::AsNumber(NewBalance));
		}
		break;
	case ECurrencyType::Gem:
		if (GemAmountText)
		{
			GemAmountText->SetText(FText::AsNumber(NewBalance));
		}
		break;
	}
}

void UCurrencyBalanceWidget::UpdateDisplay()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
	if (!CurrencySub)
	{
		return;
	}

	if (CoinAmountText)
	{
		CoinAmountText->SetText(FText::AsNumber(CurrencySub->GetBalance(ECurrencyType::Coin)));
	}

	if (GemAmountText)
	{
		GemAmountText->SetText(FText::AsNumber(CurrencySub->GetBalance(ECurrencyType::Gem)));
	}
}

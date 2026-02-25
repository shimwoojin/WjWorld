// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/CoinGainNotificationWidget.h"
#include "Currency/WjWorldCurrencySubsystem.h"
#include "Components/TextBlock.h"
#include "WjWorldLogCategories.h"

void UCoinGainNotificationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기 잔액 캐시
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		if (UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>())
		{
			CachedCoinBalance = CurrencySub->GetBalance(ECurrencyType::Coin);
			CachedGemBalance = CurrencySub->GetBalance(ECurrencyType::Gem);

			CurrencySub->OnCurrencyBalanceChanged.RemoveDynamic(this, &UCoinGainNotificationWidget::OnCurrencyBalanceChanged);
			CurrencySub->OnCurrencyBalanceChanged.AddDynamic(this, &UCoinGainNotificationWidget::OnCurrencyBalanceChanged);
		}
	}

	// 초기 상태: 숨김
	if (NotificationText)
	{
		NotificationText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UCoinGainNotificationWidget::NativeDestruct()
{
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		if (UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>())
		{
			CurrencySub->OnCurrencyBalanceChanged.RemoveDynamic(this, &UCoinGainNotificationWidget::OnCurrencyBalanceChanged);
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
	}

	Super::NativeDestruct();
}

void UCoinGainNotificationWidget::OnCurrencyBalanceChanged(ECurrencyType CurrencyType, int32 NewBalance)
{
	int32 Delta = 0;

	switch (CurrencyType)
	{
	case ECurrencyType::Coin:
		Delta = NewBalance - CachedCoinBalance;
		CachedCoinBalance = NewBalance;
		break;
	case ECurrencyType::Gem:
		Delta = NewBalance - CachedGemBalance;
		CachedGemBalance = NewBalance;
		break;
	}

	if (Delta > 0)
	{
		FString CurrencyName = (CurrencyType == ECurrencyType::Coin) ? TEXT("Coin") : TEXT("Gem");
		ShowNotification(FString::Printf(TEXT("+%d %s"), Delta, *CurrencyName));
	}
}

void UCoinGainNotificationWidget::ShowNotification(const FString& Message)
{
	if (!NotificationText)
	{
		return;
	}

	NotificationText->SetText(FText::FromString(Message));
	NotificationText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.85f, 0.0f, 1.0f))); // 금색
	NotificationText->SetVisibility(ESlateVisibility::HitTestInvisible);

	UE_LOG(LogWjWorldCurrency, Log, TEXT("CoinGainNotification: %s"), *Message);

	// 기존 타이머 리셋 후 새로 시작
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
		World->GetTimerManager().SetTimer(
			HideTimerHandle,
			this,
			&UCoinGainNotificationWidget::HideNotification,
			DisplayDuration,
			false
		);
	}
}

void UCoinGainNotificationWidget::HideNotification()
{
	if (NotificationText)
	{
		NotificationText->SetVisibility(ESlateVisibility::Hidden);
	}
}

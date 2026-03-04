// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/MatchRewardCounterWidget.h"
#include "Currency/WjWorldCurrencySubsystem.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Components/TextBlock.h"
#include "WjWorldLogCategories.h"

void UMatchRewardCounterWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();

	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		if (UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>())
		{
			// 초기 카운트 표시
			int32 WinCount = CurrencySub->GetTodayWinRewardCount();
			int32 LossCount = CurrencySub->GetTodayLossRewardCount();
			int32 MaxWin = Settings ? Settings->MaxWinRewardsPerDay : 5;
			int32 MaxLoss = Settings ? Settings->MaxLossRewardsPerDay : 10;

			UpdateDisplay(WinCount, MaxWin, LossCount, MaxLoss);

			// 델리게이트 구독
			CurrencySub->OnMatchRewardCountChanged.RemoveDynamic(this, &UMatchRewardCounterWidget::OnMatchRewardCountChanged);
			CurrencySub->OnMatchRewardCountChanged.AddDynamic(this, &UMatchRewardCounterWidget::OnMatchRewardCountChanged);
		}
	}

	// 보상 금액 표시 (옵션)
	if (Settings)
	{
		if (WinRewardText)
		{
			WinRewardText->SetText(FText::FromString(FString::Printf(TEXT("승리 보상(%d Coin)"), Settings->WinRewardCoinAmount)));
		}
		if (LossRewardText)
		{
			LossRewardText->SetText(FText::FromString(FString::Printf(TEXT("패배 보상(%d Coin)"), Settings->LossRewardCoinAmount)));
		}
	}
}

void UMatchRewardCounterWidget::NativeDestruct()
{
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		if (UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>())
		{
			CurrencySub->OnMatchRewardCountChanged.RemoveDynamic(this, &UMatchRewardCounterWidget::OnMatchRewardCountChanged);
		}
	}

	Super::NativeDestruct();
}

void UMatchRewardCounterWidget::OnMatchRewardCountChanged(int32 WinCount, int32 MaxWin, int32 LossCount, int32 MaxLoss)
{
	UpdateDisplay(WinCount, MaxWin, LossCount, MaxLoss);
}

void UMatchRewardCounterWidget::UpdateDisplay(int32 WinCount, int32 MaxWin, int32 LossCount, int32 MaxLoss)
{
	// 남은 횟수 = Max - Count
	int32 WinRemaining = FMath::Max(0, MaxWin - WinCount);
	int32 LossRemaining = FMath::Max(0, MaxLoss - LossCount);

	// 상한 도달 시 회색, 아니면 기본 흰색
	FSlateColor NormalColor = FSlateColor(FLinearColor::White);
	FSlateColor ExhaustedColor = FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));

	if (WinCountText)
	{
		WinCountText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), WinCount, MaxWin)));
		WinCountText->SetColorAndOpacity(WinRemaining <= 0 ? ExhaustedColor : NormalColor);
	}

	if (LossCountText)
	{
		LossCountText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), LossCount, MaxLoss)));
		LossCountText->SetColorAndOpacity(LossRemaining <= 0 ? ExhaustedColor : NormalColor);
	}

	// 보상 금액 텍스트도 회색 처리
	if (WinRewardText)
	{
		WinRewardText->SetColorAndOpacity(WinRemaining <= 0 ? ExhaustedColor : NormalColor);
	}
	if (LossRewardText)
	{
		LossRewardText->SetColorAndOpacity(LossRemaining <= 0 ? ExhaustedColor : NormalColor);
	}
}

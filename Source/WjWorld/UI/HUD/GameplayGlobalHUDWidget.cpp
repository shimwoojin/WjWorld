// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/GameplayGlobalHUDWidget.h"
#include "Components/TextBlock.h"

void UGameplayGlobalHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameStartCountText->SetText(FText::GetEmpty());
	GameStartCountText->SetVisibility(ESlateVisibility::Collapsed);
}

void UGameplayGlobalHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if(CountDownTime > 0.0f)
	{
		CountDownTime -= InDeltaTime;
		int32 DisplayTime = FMath::CeilToInt(CountDownTime);
		GameStartCountText->SetText(FText::AsNumber(DisplayTime));
		if(CountDownTime <= 0.0f)
		{
			GameStartCountText->SetText(FText::GetEmpty());
			GameStartCountText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UGameplayGlobalHUDWidget::StartCountDown(float CountDown)
{
	CountDownTime = CountDown;

	if(CountDownTime < 0.0f)
	{
		CountDownTime = 0.0f;
	}

	if(CountDownTime > 0.0f)
	{
		int32 DisplayTime = FMath::CeilToInt(CountDownTime);
		GameStartCountText->SetText(FText::AsNumber(DisplayTime));
		GameStartCountText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		GameStartCountText->SetText(FText::GetEmpty());
		GameStartCountText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

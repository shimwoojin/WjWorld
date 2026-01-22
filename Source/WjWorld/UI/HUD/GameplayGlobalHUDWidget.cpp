// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/GameplayGlobalHUDWidget.h"
#include "Components/TextBlock.h"

void UGameplayGlobalHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameStartCountText->SetText(FText::GetEmpty());
	GameStartCountText->SetVisibility(ESlateVisibility::Collapsed);

	GameEndCountText->SetText(FText::GetEmpty());
	GameEndCountText->SetVisibility(ESlateVisibility::Collapsed);

	GameResultText->SetText(FText::GetEmpty());
	GameResultText->SetVisibility(ESlateVisibility::Collapsed);
}

void UGameplayGlobalHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if(GameStartCountDownTime > 0.0f)
	{
		GameStartCountDownTime -= InDeltaTime;
		int32 DisplayTime = FMath::CeilToInt(GameStartCountDownTime);
		GameStartCountText->SetText(FText::AsNumber(DisplayTime));
		if(GameStartCountDownTime <= 0.0f)
		{
			GameStartCountText->SetText(FText::GetEmpty());
			GameStartCountText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (GameEndCountDownTime > 0.0f)
	{
		GameEndCountDownTime -= InDeltaTime;
		int32 DisplayTime = FMath::CeilToInt(GameEndCountDownTime);
		GameEndCountText->SetText(FText::AsNumber(DisplayTime));
		if (GameEndCountDownTime <= 0.0f)
		{
			GameEndCountText->SetText(FText::GetEmpty());
			GameEndCountText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (ResultTextDisplayTime > 0.0f)
	{
		ResultTextDisplayTime -= InDeltaTime;
		if(ResultTextDisplayTime <= 0.0f)
		{
			GameResultText->SetText(FText::GetEmpty());
			GameResultText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UGameplayGlobalHUDWidget::StartCountDown(float CountDown)
{
	GameStartCountDownTime = CountDown;

	if(GameStartCountDownTime < 0.0f)
	{
		GameStartCountDownTime = 0.0f;
	}

	if(GameStartCountDownTime > 0.0f)
	{
		int32 DisplayTime = FMath::CeilToInt(GameStartCountDownTime);
		GameStartCountText->SetText(FText::AsNumber(DisplayTime));
		GameStartCountText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		GameStartCountText->SetText(FText::GetEmpty());
		GameStartCountText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UGameplayGlobalHUDWidget::EndCountDown(float CountDown)
{
	GameEndCountDownTime = CountDown;

	if(GameEndCountDownTime < 0.0f)
	{
		GameEndCountDownTime = 0.0f;
	}

	if(GameEndCountDownTime > 0.0f)
	{
		int32 DisplayTime = FMath::CeilToInt(GameEndCountDownTime);
		GameEndCountText->SetText(FText::AsNumber(DisplayTime));
		GameEndCountText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		GameEndCountText->SetText(FText::GetEmpty());
		GameEndCountText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UGameplayGlobalHUDWidget::ShowGameResultText(const FString& ResultText, float Duration)
{
	ResultTextDisplayTime = Duration;

	GameResultText->SetText(FText::FromString(ResultText));
	GameResultText->SetVisibility(Duration > 0.0f ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

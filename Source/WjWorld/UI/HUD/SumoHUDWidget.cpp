// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/SumoHUDWidget.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/GameData/SumoGameDataComponent.h"
#include "Components/TextBlock.h"

void USumoHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (AlivePlayerText)
	{
		AlivePlayerText->SetText(FText::FromString(TEXT("Alive: 0 / 0")));
	}

	if (KillFeedText)
	{
		KillFeedText->SetVisibility(ESlateVisibility::Hidden);
	}

	if (RoundText)
	{
		RoundText->SetText(FText::GetEmpty());
	}

	// 킬피드 델리게이트 구독
	UWorld* World = GetWorld();
	if (World)
	{
		AWjWorldGameStatePlay* GameState = World->GetGameState<AWjWorldGameStatePlay>();
		if (GameState)
		{
			USumoGameDataComponent* GameData = GameState->GetGameData<USumoGameDataComponent>();
			if (GameData)
			{
				GameData->OnKillFeedUpdated.AddDynamic(this, &USumoHUDWidget::OnKillFeedReceived);
			}
		}
	}
}

void USumoHUDWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(KillFeedTimerHandle);
	}

	Super::NativeDestruct();
}

void USumoHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UWorld* World = GetWorld();
	if (!World) return;

	AWjWorldGameStatePlay* GameState = World->GetGameState<AWjWorldGameStatePlay>();
	if (!GameState) return;

	USumoGameDataComponent* GameData = GameState->GetGameData<USumoGameDataComponent>();
	if (!GameData) return;

	// 생존자 수 갱신
	if (AlivePlayerText)
	{
		AlivePlayerText->SetText(FText::FromString(FString::Printf(TEXT("Alive: %d / %d"),
			GameData->GetAlivePlayerCount(), GameData->GetTotalPlayerCount())));
	}

	// 라운드 정보 갱신
	if (RoundText)
	{
		int32 CurrentRound = GameData->GetCurrentRound();
		int32 MaxRounds = GameData->GetMaxRounds();
		if (CurrentRound > 0)
		{
			RoundText->SetText(FText::FromString(FString::Printf(TEXT("Round %d / %d"), CurrentRound, MaxRounds)));
		}
	}
}

void USumoHUDWidget::OnKillFeedReceived(const FString& InText)
{
	if (!KillFeedText) return;

	KillFeedText->SetText(FText::FromString(InText));
	KillFeedText->SetVisibility(ESlateVisibility::Visible);

	// 3초 후 숨기기
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(KillFeedTimerHandle);
		World->GetTimerManager().SetTimer(KillFeedTimerHandle, FTimerDelegate::CreateUObject(this, &USumoHUDWidget::HideKillFeed),
			KillFeedDisplayDuration, false);
	}
}

void USumoHUDWidget::HideKillFeed()
{
	if (KillFeedText)
	{
		KillFeedText->SetVisibility(ESlateVisibility::Hidden);
	}
}

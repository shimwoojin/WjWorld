// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/JumpMapHUDWidget.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "Core/GameData/JumpMapGameDataComponent.h"
#include "Core/GameData/JumpMapPlayerDataComponent.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

void UJumpMapHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TimerText)
	{
		TimerText->SetText(FText::FromString(TEXT("00:00.00")));
	}

	if (TimeLimitText)
	{
		TimeLimitText->SetText(FText::GetEmpty());
	}

	if (CheckpointText)
	{
		CheckpointText->SetText(FText::FromString(TEXT("Checkpoint: 0")));
	}

	if (DeathCountText)
	{
		DeathCountText->SetText(FText::FromString(TEXT("Deaths: 0")));
	}

	// 완주 델리게이트 구독
	UWorld* World = GetWorld();
	if (World)
	{
		AWjWorldGameStatePlay* GameState = World->GetGameState<AWjWorldGameStatePlay>();
		if (GameState)
		{
			UJumpMapGameDataComponent* GameData = GameState->GetGameData<UJumpMapGameDataComponent>();
			if (GameData)
			{
				GameData->OnPlayerFinished.AddDynamic(this, &UJumpMapHUDWidget::OnPlayerFinished);
			}
		}
	}
}

void UJumpMapHUDWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UJumpMapHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UWorld* World = GetWorld();
	if (!World) return;

	AWjWorldGameStatePlay* GameState = World->GetGameState<AWjWorldGameStatePlay>();
	if (!GameState) return;

	UJumpMapGameDataComponent* GameData = GameState->GetGameData<UJumpMapGameDataComponent>();
	if (!GameData) return;

	// 경과 시간 갱신 (MM:SS.ms)
	if (TimerText)
	{
		float Elapsed = GameData->GetElapsedTime();
		int32 Minutes = FMath::FloorToInt(Elapsed / 60.f);
		int32 Seconds = FMath::FloorToInt(FMath::Fmod(Elapsed, 60.f));
		int32 Centiseconds = FMath::FloorToInt(FMath::Fmod(Elapsed * 100.f, 100.f));

		TimerText->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d.%02d"), Minutes, Seconds, Centiseconds)));
	}

	// 남은 시간 갱신
	if (TimeLimitText)
	{
		float TimeLimit = GameData->GetTimeLimit();
		float Remaining = FMath::Max(0.f, TimeLimit - GameData->GetElapsedTime());
		int32 Minutes = FMath::FloorToInt(Remaining / 60.f);
		int32 Seconds = FMath::CeilToInt(FMath::Fmod(Remaining, 60.f));
		if (Seconds >= 60)
		{
			Minutes += 1;
			Seconds = 0;
		}

		TimeLimitText->SetText(FText::FromString(FString::Printf(TEXT("Remaining: %02d:%02d"), Minutes, Seconds)));
	}

	// 로컬 플레이어 데이터 갱신
	APlayerController* PC = World->GetFirstPlayerController();
	if (PC)
	{
		AWjWorldPlayerStatePlay* PS = PC->GetPlayerState<AWjWorldPlayerStatePlay>();
		if (PS)
		{
			UJumpMapPlayerDataComponent* PlayerData = PS->GetGameData<UJumpMapPlayerDataComponent>();
			if (PlayerData)
			{
				if (CheckpointText)
				{
					CheckpointText->SetText(FText::FromString(FString::Printf(TEXT("Checkpoint: %d"),
						PlayerData->GetCurrentCheckpointIndex() + 1)));
				}

				if (DeathCountText)
				{
					DeathCountText->SetText(FText::FromString(FString::Printf(TEXT("Deaths: %d"),
						PlayerData->GetDeathCount())));
				}
			}
		}
	}
}

void UJumpMapHUDWidget::OnPlayerFinished(const FJumpMapPlayerFinish& FinishInfo)
{
	UpdateLeaderboard();
}

void UJumpMapHUDWidget::UpdateLeaderboard()
{
	if (!LeaderboardBox) return;

	LeaderboardBox->ClearChildren();

	UWorld* World = GetWorld();
	if (!World) return;

	AWjWorldGameStatePlay* GameState = World->GetGameState<AWjWorldGameStatePlay>();
	if (!GameState) return;

	UJumpMapGameDataComponent* GameData = GameState->GetGameData<UJumpMapGameDataComponent>();
	if (!GameData) return;

	const TArray<FJumpMapPlayerFinish>& FinishOrder = GameData->GetPlayerFinishOrder();
	for (int32 i = 0; i < FinishOrder.Num(); ++i)
	{
		const FJumpMapPlayerFinish& Finish = FinishOrder[i];

		int32 Minutes = FMath::FloorToInt(Finish.FinishTime / 60.f);
		int32 Seconds = FMath::FloorToInt(FMath::Fmod(Finish.FinishTime, 60.f));
		int32 Centiseconds = FMath::FloorToInt(FMath::Fmod(Finish.FinishTime * 100.f, 100.f));

		FString EntryStr = FString::Printf(TEXT("#%d  %s  %02d:%02d.%02d"),
			i + 1, *Finish.PlayerName, Minutes, Seconds, Centiseconds);

		UTextBlock* EntryText = NewObject<UTextBlock>(this);
		if (EntryText)
		{
			EntryText->SetText(FText::FromString(EntryStr));
			LeaderboardBox->AddChild(EntryText);
		}
	}
}

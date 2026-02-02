// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameRule/WjWorldGameRuleBase.h"
#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "Core/GameData/WjWorldGameDataComponent.h"
#include "Core/WjWorldGameInstance.h"

#include "WjWorldLogCategories.h"

void UWjWorldGameRuleBase::Initialize(AWjWorldGameModePlay* InGameMode)
{
	if (InGameMode)
	{
		GameMode = InGameMode;
		//GameState = InGameMode->GetGameState<AWjWorldGameStatePlay>();
		//
		//UE_LOG(LogWjWorld, Log, TEXT("GameState Valid ? : %d"), (int32)GameState.IsValid());
		//
		//if (GameState.IsValid() && GameDataComponentClass)
		//{
		//	GameState->AddGameDataComponent(GameDataComponentClass);
		//}
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: Initialized"));
}

void UWjWorldGameRuleBase::OnGameReady()
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(DelayStartHandle, FTimerDelegate::CreateLambda([this]() {
			if (GameMode.IsValid())
			{
				GameMode->StartGame(SecondsForGameStartCount);
			}
			}), StartDelay, false);

		ChangeGamePhase(EGamePhase::Ready);
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: Game Ready"));
}

void UWjWorldGameRuleBase::OnGameStart()
{
	if (!HasAuthority()) return;

	ChangeGamePhase(EGamePhase::Playing);
	UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: Game Started"));
}

void UWjWorldGameRuleBase::OnGameEndPredict(float Seconds)
{
	if (!HasAuthority()) return;
	if (GameMode.IsValid())
	{
		GameMode->EndGamePredict(Seconds);
	}
}

void UWjWorldGameRuleBase::OnGameEnd()
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (World)
	{
		// 세션 종료 (InProgress → Ended) - WaitingRoom 복귀 후 다시 StartSession 가능하게
		if (UWjWorldGameInstance* GI = Cast<UWjWorldGameInstance>(World->GetGameInstance()))
		{
			GI->EndGame();
		}

		World->GetTimerManager().SetTimer(GotoWaitingRoomHandle, FTimerDelegate::CreateLambda([this]() {
			GetWorld()->ServerTravel(TEXT("/Game/Map/02-2_WaitingRoom"));
			}), SecondsForGotoWaitingRoom, false);

		ChangeGamePhase(EGamePhase::Finished);
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: Game Ended"));
}

void UWjWorldGameRuleBase::OnPlayerJoined(AWjWorldPlayerStatePlay* Player)
{
	if (!HasAuthority()) return;

	if (Player && PlayerDataComponentClass)
	{
		Player->AddGameDataComponent(PlayerDataComponentClass);
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: Player Joined - %s"), *Player->GetName());
}

void UWjWorldGameRuleBase::OnPlayerLeft(AWjWorldPlayerStatePlay* Player)
{
	if (!HasAuthority()) return;
}

bool UWjWorldGameRuleBase::CheckWinCondition() const
{
	return false;
}

AWjWorldPlayerStatePlay* UWjWorldGameRuleBase::GetWinner() const
{
	return nullptr;
}

void UWjWorldGameRuleBase::Tick(float DeltaTime)
{

}

UWorld* UWjWorldGameRuleBase::GetTickableGameObjectWorld() const
{
	if (GameMode.IsValid())
	{
		return GameMode->GetWorld();
	}
	return nullptr;
}

TStatId UWjWorldGameRuleBase::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMassComponentHitSubsystem, STATGROUP_Tickables);
}

void UWjWorldGameRuleBase::BeginDestroy()
{
	Super::BeginDestroy();
	
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(DelayStartHandle);
		World->GetTimerManager().ClearTimer(GotoWaitingRoomHandle);
	}
}

AWjWorldGameModePlay* UWjWorldGameRuleBase::GetGameModePlay() const
{
	UWorld* World = GetWorld();
	if (World)
	{
		return World->GetAuthGameMode<AWjWorldGameModePlay>();
	}
	return nullptr;
}

AWjWorldGameStatePlay* UWjWorldGameRuleBase::GetGameStatePlay() const
{
	UWorld* World = GetWorld();
	if (World)
	{
		return World->GetGameState<AWjWorldGameStatePlay>();
	}
	return nullptr;
}

void UWjWorldGameRuleBase::GameLevelUp(int32 NewLevel)
{
	if (!HasAuthority()) return;

	AWjWorldGameModePlay* GameModePlay = GetGameModePlay();
	if (GameModePlay)
	{
		GameModePlay->OnGameLevelUp(NewLevel);
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: Game Level Up to %d"), NewLevel);
}

void UWjWorldGameRuleBase::ChangeGamePhase(EGamePhase GamePhase)
{
	if (!HasAuthority()) return;

	if (AWjWorldGameStatePlay* GameStatePlay = GetGameStatePlay())
	{
		GameStatePlay->SetGamePhase(GamePhase);
		UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: Game Phase changed to %d"), static_cast<int32>(GamePhase));
	}
}

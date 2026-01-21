// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameRule/WjWorldGameRuleBase.h"
#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "Core/GameData/WjWorldGameDataComponent.h"

#include "WjWorldLogCategories.h"

void UWjWorldGameRuleBase::Initialize(AWjWorldGameModePlay* InGameMode)
{
	if (InGameMode)
	{
		GameMode = InGameMode;
		GameState = InGameMode->GetGameState<AWjWorldGameStatePlay>();

		UE_LOG(LogWjWorld, Log, TEXT("GameState Valid ? : %d"), (int32)GameState.IsValid());

		if (GameState.IsValid() && GameDataComponentClass)
		{
			GameState->AddGameDataComponent(GameDataComponentClass);
		}
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: Initialized"));
}

void UWjWorldGameRuleBase::OnGameReady()
{
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
	ChangeGamePhase(EGamePhase::Playing);
	UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: Game Started"));
}

void UWjWorldGameRuleBase::OnGameEnd()
{
	ChangeGamePhase(EGamePhase::Finished);
	UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: Game Ended"));
}

void UWjWorldGameRuleBase::OnPlayerJoined(AWjWorldPlayerStatePlay* Player)
{
	if (Player && PlayerDataComponentClass)
	{
		Player->AddGameDataComponent(PlayerDataComponentClass);
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: Player Joined - %s"), *Player->GetName());
}

void UWjWorldGameRuleBase::OnPlayerLeft(AWjWorldPlayerStatePlay* Player)
{
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
	}
}

void UWjWorldGameRuleBase::ChangeGamePhase(EGamePhase GamePhase)
{
	UWorld* World = GetWorld();
	if (World)
	{
		if (AWjWorldGameStatePlay* GameStatePlay = World->GetGameState<AWjWorldGameStatePlay>())
		{
			GameStatePlay->CurrentPhase = GamePhase;
			UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: Game Phase changed to %d"), static_cast<int32>(GamePhase));
		}
	}
}

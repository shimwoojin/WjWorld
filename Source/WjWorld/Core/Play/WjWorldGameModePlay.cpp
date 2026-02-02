// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/Play/WjWorldHUDPlay.h"
#include "Core/Play/WjWorldPlayerControllerPlay.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"

#include "Core/GameRule/WjWorldGameRuleBase.h"
#include "Core/WjWorldGameInstance.h"

#include "WjWorldLogCategories.h"

AWjWorldGameModePlay::AWjWorldGameModePlay()
{
	PlayerStateClass = AWjWorldPlayerStatePlay::StaticClass();
	PlayerControllerClass = AWjWorldPlayerControllerPlay::StaticClass();
	HUDClass = AWjWorldHUDPlay::StaticClass();
	GameStateClass = AWjWorldGameStatePlay::StaticClass();
	DefaultPawnClass = AWjWorldCharacterPlay::StaticClass();
}

void AWjWorldGameModePlay::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	if (GameRuleClass)
	{
		CurrentGameRule = NewObject<UWjWorldGameRuleBase>(this, GameRuleClass);
		CurrentGameRule->Initialize(this);
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameModePlay: InitGame completed"));
}

void AWjWorldGameModePlay::StartPlay()
{
	Super::StartPlay();

	if (CurrentGameRule)
	{
		CurrentGameRule->OnGameReady();
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameModePlay: StartPlay completed"));
}

void AWjWorldGameModePlay::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (CurrentGameRule)
	{
		AWjWorldPlayerStatePlay* NewPlayerState = Cast<AWjWorldPlayerStatePlay>(NewPlayer->PlayerState);
		if (NewPlayerState)
		{
			CurrentGameRule->OnPlayerJoined(NewPlayerState);
		}
	}

	// Play 중 플레이어 목록 캐시 (호스트 마이그레이션용)
	if (UWjWorldGameInstance* GI = Cast<UWjWorldGameInstance>(GetGameInstance()))
	{
		AGameStateBase* GS = GetGameState<AGameStateBase>();
		if (GS)
		{
			TArray<FPlayerDisplayInfo> PlayerList;
			bool bFirstPlayer = true;
			for (APlayerState* PS : GS->PlayerArray)
			{
				if (PS && !PS->GetPlayerName().IsEmpty())
				{
					FPlayerDisplayInfo Info;
					Info.PlayerName = PS->GetPlayerName();
					Info.PlayerID = PS->GetPlayerId();
					Info.bIsHost = bFirstPlayer; // 서버의 첫 번째 플레이어 = 호스트
					Info.bIsReady = true;
					PlayerList.Add(Info);
					bFirstPlayer = false;
				}
			}
			GI->CachePlayerList(PlayerList);
		}
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameModePlay: PostLogin completed for %s"), *NewPlayer->GetName());
}

void AWjWorldGameModePlay::StartGame(float SecondsForStartCount)
{
	GetGameState<AWjWorldGameStatePlay>()->GameStartWithCountdown(SecondsForStartCount);
	GetGameState<AWjWorldGameStatePlay>()->SetGameRuleClass(CurrentGameRule->GetClass());

	if(GetWorld() && CurrentGameRule)
	{
		GetWorld()->GetTimerManager().SetTimer(StartGameHandle, FTimerDelegate::CreateLambda([this, SecondsForStartCount]()
		{
			CurrentGameRule->OnGameStart();
			OnGameStart.Broadcast();
		}), SecondsForStartCount, false);
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameModePlay: StartGame scheduled in %f seconds"), SecondsForStartCount);
}

void AWjWorldGameModePlay::EndGamePredict(float SecondsForEndCount)
{
	GetGameState<AWjWorldGameStatePlay>()->GameEndWithCountdown(SecondsForEndCount);
	UE_LOG(LogWjWorld, Log, TEXT("GameModePlay: EndGame scheduled in %f seconds"), SecondsForEndCount);
}

void AWjWorldGameModePlay::OnGameLevelUp(int32 NewLevel)
{
	OnGameLevelChange.Broadcast(NewLevel);
	UE_LOG(LogWjWorld, Log, TEXT("GameModePlay: Game Level Up to %d"), NewLevel);
}

void AWjWorldGameModePlay::BeginDestroy()
{
	Super::BeginDestroy();

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(StartGameHandle);
	}
}
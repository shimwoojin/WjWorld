// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/Play/WjWorldHUDPlay.h"
#include "Core/Play/WjWorldPlayerControllerPlay.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"

#include "Core/GameRule/WjWorldGameRuleBase.h"

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

	UE_LOG(LogWjWorld, Log, TEXT("GameModePlay: PostLogin completed for %s"), *NewPlayer->GetName());
}

void AWjWorldGameModePlay::StartGame(float SecondsForStartCount)
{
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

void AWjWorldGameModePlay::BeginDestroy()
{
	Super::BeginDestroy();

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(StartGameHandle);
	}
}
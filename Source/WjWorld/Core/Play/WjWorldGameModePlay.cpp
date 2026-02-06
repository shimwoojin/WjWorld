// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/Play/WjWorldHUDPlay.h"
#include "Core/Play/WjWorldPlayerControllerPlay.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"

#include "Core/GameRule/WjWorldGameRuleBase.h"
#include "Core/WjWorldGameInstance.h"
#include "DataAsset/WjWorldMinigameDataAsset.h"
#include "Setting/WjWorldDeveloperSettings.h"

#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

AWjWorldGameModePlay::AWjWorldGameModePlay()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	PlayerStateClass = AWjWorldPlayerStatePlay::StaticClass();
	PlayerControllerClass = AWjWorldPlayerControllerPlay::StaticClass();
	HUDClass = AWjWorldHUDPlay::StaticClass();
	GameStateClass = AWjWorldGameStatePlay::StaticClass();
	DefaultPawnClass = AWjWorldCharacterPlay::StaticClass();
}

void AWjWorldGameModePlay::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// URL Options에서 GameModeId, MapOption 파싱
	GameModeId = UGameplayStatics::ParseOption(Options, TEXT("GameModeId"));
	MapOption = UGameplayStatics::ParseOption(Options, TEXT("MapOption"));
	UE_LOG(LogWjWorld, Log, TEXT("GameModePlay: GameModeId='%s', MapOption='%s'"), *GameModeId, *MapOption);

	// 카탈로그에서 GameRuleClass 조회
	TSubclassOf<UWjWorldGameRuleBase> ResolvedGameRuleClass = nullptr;

	const UWjWorldDeveloperSettings* DevSettings = GetDefault<UWjWorldDeveloperSettings>();
	if (DevSettings && !DevSettings->MinigameCatalog.IsNull())
	{
		UWjWorldMinigameDataAsset* Catalog = DevSettings->MinigameCatalog.LoadSynchronous();
		if (Catalog && !GameModeId.IsEmpty())
		{
			const FWjWorldMinigameDefinition* Def = Catalog->FindByGameModeId(FName(*GameModeId));
			if (Def && Def->GameRuleClass)
			{
				ResolvedGameRuleClass = Def->GameRuleClass;
				UE_LOG(LogWjWorld, Log, TEXT("GameModePlay: GameRuleClass resolved from catalog - %s"),
					*ResolvedGameRuleClass->GetName());
			}
		}
	}

	// 카탈로그에서 못 찾으면 폴백 (Deprecated)
	if (!ResolvedGameRuleClass && GameRuleClass)
	{
		ResolvedGameRuleClass = GameRuleClass;
		UE_LOG(LogWjWorld, Warning, TEXT("GameModePlay: Using fallback GameRuleClass (deprecated) - %s"),
			*ResolvedGameRuleClass->GetName());
	}

	// GameRule 생성
	if (ResolvedGameRuleClass)
	{
		CurrentGameRule = NewObject<UWjWorldGameRuleBase>(this, ResolvedGameRuleClass);
		CurrentGameRule->Initialize(this);
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("GameModePlay: No GameRuleClass found!"));
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

void AWjWorldGameModePlay::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// GameRule 틱 호출
	if (CurrentGameRule)
	{
		CurrentGameRule->TickGameRule(DeltaSeconds);
	}
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

void AWjWorldGameModePlay::Logout(AController* Exiting)
{
	// GameRule에 플레이어 이탈 알림 (Super::Logout 전에 호출해야 PlayerState가 유효함)
	if (CurrentGameRule && Exiting)
	{
		AWjWorldPlayerStatePlay* ExitingPlayerState = Cast<AWjWorldPlayerStatePlay>(Exiting->PlayerState);
		if (ExitingPlayerState)
		{
			UE_LOG(LogWjWorld, Log, TEXT("GameModePlay: Logout - Notifying GameRule for player %s"), *ExitingPlayerState->GetPlayerName());
			CurrentGameRule->OnPlayerLeft(ExitingPlayerState);
		}
	}

	Super::Logout(Exiting);

	UE_LOG(LogWjWorld, Log, TEXT("GameModePlay: Logout completed for %s"), Exiting ? *Exiting->GetName() : TEXT("Unknown"));
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
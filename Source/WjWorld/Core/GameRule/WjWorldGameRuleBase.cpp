// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameRule/WjWorldGameRuleBase.h"
#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "Core/GameData/WjWorldGameDataComponent.h"
#include "Core/WjWorldGameInstance.h"
#include "DataAsset/WjWorldMinigameDataAsset.h"
#include "Setting/WjWorldDeveloperSettings.h"

#include "WjWorldLogCategories.h"

void UWjWorldGameRuleBase::Initialize(AWjWorldGameModePlay* InGameMode)
{
	if (InGameMode)
	{
		GameMode = InGameMode;
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: Initialized"));
}

void UWjWorldGameRuleBase::OnGameReady()
{
	if (!HasAuthority()) return;

	// GameState는 InitGameState() (PreInitializeComponents) 이후에 생성되므로
	// Initialize() 시점에는 없고, OnGameReady() (StartPlay) 시점에 사용 가능
	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (GameState && GameDataComponentClass)
	{
		GameState->AddGameDataComponent(GameDataComponentClass);
		UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: GameDataComponent added to GameState"));
	}

	// MinigameCatalog에서 AllowedAbilityTags, StatNamespace 설정
	if (GameState && GameMode.IsValid())
	{
		const UWjWorldDeveloperSettings* DevSettings = GetDefault<UWjWorldDeveloperSettings>();
		if (DevSettings && !DevSettings->MinigameCatalog.IsNull())
		{
			UWjWorldMinigameDataAsset* Catalog = DevSettings->MinigameCatalog.LoadSynchronous();
			FString CurrentGameModeId = GameMode->GetMapOption();
			// GameModeId는 URL Options에서 직접 가져오기
			const FWjWorldMinigameDefinition* Def = nullptr;
			if (Catalog)
			{
				// GameModePlay의 InitGame에서 파싱된 GameModeId를 사용해야 하므로
				// 카탈로그에서 현재 GameRule 클래스를 기준으로 찾기
				for (const FWjWorldMinigameDefinition& MiniDef : Catalog->Minigames)
				{
					if (MiniDef.GameRuleClass == GetClass())
					{
						Def = &MiniDef;
						break;
					}
				}
			}

			if (Def)
			{
				GameState->SetAllowedAbilityTags(Def->AllowedAbilityTags);
				GameState->SetStatNamespace(Def->StatNamespace);
				GameState->SetDefaultCameraMode(Def->DefaultCameraMode);
				UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: AllowedAbilityTags set (%d tags), StatNamespace='%s', CameraMode=%d"),
					Def->AllowedAbilityTags.Num(), *Def->StatNamespace.ToString(), static_cast<int32>(Def->DefaultCameraMode));
			}
		}
	}

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

	// 게임 시작 시점 참여 인원 기록 (1인 플레이 스탯/보상 제외용)
	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (GameState)
	{
		GameState->SetInitialPlayerCount(GameState->PlayerArray.Num());
	}

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

		// ServerTravel URL 미리 계산 (타이머 콜백에서 this 접근 문제 방지)
		const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
		FString TravelURL = Settings->GetWaitingRoomServerTravelURL();

		UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase::OnGameEnd - Will travel to: %s in %f seconds"), *TravelURL, SecondsForGotoWaitingRoom);

		// Lobby 맵을 사용하되 GameMode는 WaitingRoom으로 오버라이드
		// World를 WeakPtr로 캡처하여 타이머 콜백에서 안전하게 접근
		TWeakObjectPtr<UWorld> WeakWorld = World;
		World->GetTimerManager().SetTimer(GotoWaitingRoomHandle, FTimerDelegate::CreateLambda([WeakWorld, TravelURL]() {
			if (UWorld* ValidWorld = WeakWorld.Get())
			{
				UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase - Executing ServerTravel to: %s"), *TravelURL);
				ValidWorld->ServerTravel(TravelURL);
			}
			else
			{
				UE_LOG(LogWjWorld, Error, TEXT("GameRuleBase - ServerTravel failed: World is invalid"));
			}
		}), SecondsForGotoWaitingRoom, false);

		ChangeGamePhase(EGamePhase::Finished);
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleBase: Game Ended"));
}

void UWjWorldGameRuleBase::OnPlayerJoined(AWjWorldPlayerStatePlay* Player)
{
	if (!HasAuthority()) return;
	if (!Player) return;

	if (PlayerDataComponentClass)
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

void UWjWorldGameRuleBase::TickGameRule(float DeltaTime)
{
	// 서브클래스에서 오버라이드
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

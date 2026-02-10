// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameRule/WjWorldGameRuleJumpMap.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameData/JumpMapGameDataComponent.h"
#include "Core/GameData/JumpMapPlayerDataComponent.h"
#include "GamePlay/JumpMap/JumpMapLayoutDataAsset.h"
#include "GamePlay/JumpMap/JumpMapCheckpointActor.h"
#include "GamePlay/JumpMap/JumpMapEndActor.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Stats/WjWorldStatsSubsystem.h"
#include "Stats/WjWorldStatTypes.h"
#include "WjWorldLogCategories.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"

void UWjWorldGameRuleJumpMap::Initialize(AWjWorldGameModePlay* InGameMode)
{
	Super::Initialize(InGameMode);

	GameDataComponentClass = UJumpMapGameDataComponent::StaticClass();
	PlayerDataComponentClass = UJumpMapPlayerDataComponent::StaticClass();

	// MapOption에서 레이아웃 이름 파싱
	if (InGameMode)
	{
		const FString& MapOpt = InGameMode->GetMapOption();
		UE_LOG(LogWjWorld, Log, TEXT("GameRuleJumpMap: MapOption = '%s'"), *MapOpt);
	}

	// 레이아웃 로드 및 게임플레이 액터 스폰
	LoadLayoutAndSpawnActors();

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleJumpMap: Initialized (TimeLimit=%.1f, FallThreshold=%.1f)"),
		TimeLimit, FallThresholdZ);
}

void UWjWorldGameRuleJumpMap::LoadLayoutAndSpawnActors()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const UWjWorldDeveloperSettings* DevSettings = GetDefault<UWjWorldDeveloperSettings>();
	if (!DevSettings) return;

	// JumpMap 배치 카탈로그에서 레이아웃 로드
	// 맵에 이미 배치된 체크포인트/엔드 액터를 수집
	for (TActorIterator<AJumpMapCheckpointActor> It(World); It; ++It)
	{
		AJumpMapCheckpointActor* Checkpoint = *It;
		if (Checkpoint)
		{
			int32 Order = Checkpoint->CheckpointOrder;
			// 배열 크기 확보
			if (CheckpointLocations.Num() <= Order)
			{
				CheckpointLocations.SetNum(Order + 1);
			}
			CheckpointLocations[Order] = Checkpoint->GetActorLocation();

			UE_LOG(LogWjWorld, Log, TEXT("GameRuleJumpMap: Checkpoint %d at %s"),
				Order, *Checkpoint->GetActorLocation().ToString());
		}
	}

	// 엔드 액터에서 스타트 위치 확인
	for (TActorIterator<AJumpMapEndActor> It(World); It; ++It)
	{
		AJumpMapEndActor* EndActor = *It;
		if (EndActor)
		{
			UE_LOG(LogWjWorld, Log, TEXT("GameRuleJumpMap: End actor at %s"),
				*EndActor->GetActorLocation().ToString());
		}
	}

	// 플레이어 스타트에서 시작 위치 가져오기
	AActor* PlayerStart = UGameplayStatics::GetActorOfClass(World, APlayerStart::StaticClass());
	if (PlayerStart)
	{
		StartLocation = PlayerStart->GetActorLocation();
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleJumpMap: Layout loaded - Start: %s, Checkpoints: %d"),
		*StartLocation.ToString(), CheckpointLocations.Num());
}

void UWjWorldGameRuleJumpMap::OnGameReady()
{
	if (!HasAuthority()) return;

	Super::OnGameReady();

	// GameData에 초기 정보 설정
	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (GameState)
	{
		UJumpMapGameDataComponent* GameData = GameState->GetGameData<UJumpMapGameDataComponent>();
		if (GameData)
		{
			GameData->SetTimeLimit(TimeLimit);
			GameData->SetTotalPlayerCount(TotalPlayerCount);
		}
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleJumpMap: Game Ready - Players: %d, Checkpoints: %d"),
		TotalPlayerCount, CheckpointLocations.Num());
}

void UWjWorldGameRuleJumpMap::OnGameStart()
{
	if (!HasAuthority()) return;

	Super::OnGameStart();

	bIsGameStarted = true;
	ElapsedTime = 0.f;

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleJumpMap: Game Started - Players: %d, TimeLimit: %.1f"),
		TotalPlayerCount, TimeLimit);

	// 엣지 케이스: 플레이어 없음
	if (TotalPlayerCount <= 0)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("GameRuleJumpMap: No players, ending game"));
		bGameOverConditionMet = true;
		WinnerPlayer = nullptr;
		OnGameEnd();
		return;
	}
}

void UWjWorldGameRuleJumpMap::OnGameEnd()
{
	if (!HasAuthority()) return;

	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (GameState)
	{
		// 승자 결정: 가장 빠른 완주 시간
		if (!WinnerPlayer.IsValid())
		{
			float BestTime = MAX_FLT;
			for (const auto& WeakPS : AllPlayers)
			{
				if (!WeakPS.IsValid()) continue;
				AWjWorldPlayerStatePlay* PS = WeakPS.Get();
				UJumpMapPlayerDataComponent* PlayerData = PS->GetGameData<UJumpMapPlayerDataComponent>();
				if (PlayerData && PlayerData->HasFinished() && PlayerData->GetFinishTime() < BestTime)
				{
					BestTime = PlayerData->GetFinishTime();
					WinnerPlayer = PS;
				}
			}
		}

		if (WinnerPlayer.IsValid())
		{
			GameState->SetGameResult(WinnerPlayer->GetPlayerName(), true);
			UE_LOG(LogWjWorld, Log, TEXT("GameRuleJumpMap: Winner: %s"), *WinnerPlayer->GetPlayerName());
		}
		else
		{
			GameState->SetGameResult(TEXT(""), false);
			UE_LOG(LogWjWorld, Log, TEXT("GameRuleJumpMap: No winner (time expired)"));
		}
	}

	// 스폰된 게임플레이 액터 정리
	for (const auto& WeakActor : SpawnedGameplayActors)
	{
		if (WeakActor.IsValid())
		{
			WeakActor->Destroy();
		}
	}
	SpawnedGameplayActors.Empty();

	Super::OnGameEnd();
}

void UWjWorldGameRuleJumpMap::OnPlayerJoined(AWjWorldPlayerStatePlay* Player)
{
	Super::OnPlayerJoined(Player);

	if (!HasAuthority() || !Player) return;

	AllPlayers.Add(Player);
	TotalPlayerCount++;

	UpdateGameData();

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleJumpMap: Player Joined - %s, Total: %d"),
		*Player->GetPlayerName(), TotalPlayerCount);
}

void UWjWorldGameRuleJumpMap::OnPlayerLeft(AWjWorldPlayerStatePlay* Player)
{
	Super::OnPlayerLeft(Player);

	if (!HasAuthority() || !Player) return;

	AllPlayers.Remove(Player);
	TotalPlayerCount--;

	UpdateGameData();

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleJumpMap: Player Left - %s, Total: %d"),
		*Player->GetPlayerName(), TotalPlayerCount);

	// 모든 플레이어 이탈
	if (TotalPlayerCount <= 0)
	{
		bGameOverConditionMet = true;
		WinnerPlayer = nullptr;
		OnGameEnd();
		return;
	}

	// 남은 플레이어가 모두 완주했는지 체크
	int32 ActivePlayers = 0;
	for (const auto& WeakPS : AllPlayers)
	{
		if (!WeakPS.IsValid()) continue;
		UJumpMapPlayerDataComponent* PlayerData = WeakPS->GetGameData<UJumpMapPlayerDataComponent>();
		if (!PlayerData || !PlayerData->HasFinished())
		{
			ActivePlayers++;
		}
	}

	if (ActivePlayers == 0 && FinishedPlayerCount > 0)
	{
		bGameOverConditionMet = true;
		OnGameEnd();
	}
}

void UWjWorldGameRuleJumpMap::OnPlayerDied(AWjWorldCharacterPlay* Character)
{
	if (!HasAuthority() || !Character) return;

	AWjWorldPlayerStatePlay* PS = Character->GetPlayerState<AWjWorldPlayerStatePlay>();
	if (!PS) return;

	// 이미 완주한 플레이어는 무시
	UJumpMapPlayerDataComponent* PlayerData = PS->GetGameData<UJumpMapPlayerDataComponent>();
	if (!PlayerData || PlayerData->HasFinished()) return;

	// 사망 횟수 증가
	PlayerData->IncrementDeathCount();

	int32 CheckpointIdx = PlayerData->GetCurrentCheckpointIndex();

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleJumpMap: Player died: %s (Deaths: %d, Checkpoint: %d)"),
		*PS->GetPlayerName(), PlayerData->GetDeathCount(), CheckpointIdx);

	// 리스폰
	RespawnPlayerAtCheckpoint(Character, CheckpointIdx);
}

void UWjWorldGameRuleJumpMap::RespawnPlayerAtCheckpoint(AWjWorldCharacterPlay* Character, int32 CheckpointIndex)
{
	if (!Character) return;

	FVector RespawnLocation;
	if (CheckpointIndex >= 0 && CheckpointIndex < CheckpointLocations.Num())
	{
		RespawnLocation = CheckpointLocations[CheckpointIndex];
	}
	else
	{
		RespawnLocation = StartLocation;
	}

	// 약간 위에 스폰 (바닥 충돌 방지)
	RespawnLocation.Z += 100.f;

	Character->SetActorLocation(RespawnLocation);

	// 속도 초기화
	if (UCharacterMovementComponent* MC = Character->GetCharacterMovement())
	{
		MC->StopMovementImmediately();
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleJumpMap: Respawned at checkpoint %d (%s)"),
		CheckpointIndex, *RespawnLocation.ToString());
}

void UWjWorldGameRuleJumpMap::OnPlayerFinished(AWjWorldCharacterPlay* Character)
{
	if (!HasAuthority() || !Character) return;

	AWjWorldPlayerStatePlay* PS = Character->GetPlayerState<AWjWorldPlayerStatePlay>();
	if (!PS) return;

	UJumpMapPlayerDataComponent* PlayerData = PS->GetGameData<UJumpMapPlayerDataComponent>();
	if (!PlayerData || PlayerData->HasFinished()) return;

	// 완주 기록
	PlayerData->SetFinished(ElapsedTime);
	FinishedPlayerCount++;

	// GameData에 완주 순서 추가
	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (GameState)
	{
		UJumpMapGameDataComponent* GameData = GameState->GetGameData<UJumpMapGameDataComponent>();
		if (GameData)
		{
			FJumpMapPlayerFinish FinishInfo;
			FinishInfo.PlayerName = PS->GetPlayerName();
			FinishInfo.FinishTime = ElapsedTime;
			GameData->AddPlayerFinish(FinishInfo);
			GameData->SetFinishedPlayerCount(FinishedPlayerCount);
		}
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleJumpMap: Player finished: %s (Time: %.2f, Order: %d/%d)"),
		*PS->GetPlayerName(), ElapsedTime, FinishedPlayerCount, TotalPlayerCount);

	// 첫 번째 완주자가 승자
	if (!WinnerPlayer.IsValid())
	{
		WinnerPlayer = PS;
	}

	// 모든 플레이어가 완주했는지 체크
	int32 ActivePlayers = 0;
	for (const auto& WeakPS : AllPlayers)
	{
		if (!WeakPS.IsValid()) continue;
		UJumpMapPlayerDataComponent* PD = WeakPS->GetGameData<UJumpMapPlayerDataComponent>();
		if (!PD || !PD->HasFinished())
		{
			ActivePlayers++;
		}
	}

	if (ActivePlayers == 0)
	{
		bGameOverConditionMet = true;
		OnGameEnd();
	}
}

bool UWjWorldGameRuleJumpMap::CheckWinCondition() const
{
	if (!bIsGameStarted) return false;
	return bGameOverConditionMet;
}

AWjWorldPlayerStatePlay* UWjWorldGameRuleJumpMap::GetWinner() const
{
	return WinnerPlayer.Get();
}

void UWjWorldGameRuleJumpMap::TickGameRule(float DeltaTime)
{
	if (!HasAuthority()) return;
	if (!bIsGameStarted || bGameOverConditionMet) return;

	// 경과 시간 업데이트
	ElapsedTime += DeltaTime;

	// GameData 업데이트
	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (GameState)
	{
		UJumpMapGameDataComponent* GameData = GameState->GetGameData<UJumpMapGameDataComponent>();
		if (GameData)
		{
			GameData->SetElapsedTime(ElapsedTime);
		}
	}

	// 낙하 플레이어 체크
	for (const auto& WeakPS : AllPlayers)
	{
		if (!WeakPS.IsValid()) continue;

		APawn* Pawn = WeakPS->GetPawn();
		AWjWorldCharacterPlay* Character = Cast<AWjWorldCharacterPlay>(Pawn);
		if (!Character) continue;

		UJumpMapPlayerDataComponent* PlayerData = WeakPS->GetGameData<UJumpMapPlayerDataComponent>();
		if (PlayerData && PlayerData->HasFinished()) continue;

		if (Character->GetActorLocation().Z < FallThresholdZ)
		{
			OnPlayerDied(Character);
		}
	}

	// 시간 제한 체크
	if (TimeLimit > 0.f && ElapsedTime >= TimeLimit)
	{
		UE_LOG(LogWjWorld, Log, TEXT("GameRuleJumpMap: Time limit reached (%.1f)"), TimeLimit);
		bGameOverConditionMet = true;
		OnGameEnd();
	}
}

void UWjWorldGameRuleJumpMap::UpdateGameData()
{
	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (!GameState) return;

	UJumpMapGameDataComponent* GameData = GameState->GetGameData<UJumpMapGameDataComponent>();
	if (!GameData) return;

	GameData->SetTotalPlayerCount(TotalPlayerCount);
	GameData->SetFinishedPlayerCount(FinishedPlayerCount);
}

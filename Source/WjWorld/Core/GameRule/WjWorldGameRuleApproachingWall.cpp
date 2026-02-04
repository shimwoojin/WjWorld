// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameData/ApproachingWallPlayerDataComponent.h"
#include "Core/GameData/ApproachingWallGameDataComponent.h"
#include "GamePlay/Wall/WjWorldBrickSpawner.h"
#include "GamePlay/Wall/WjWorldWallManager.h"
#include "Setting/WjWorldDeveloperSettings.h"

#include "WjWorldLogCategories.h"

void UWjWorldGameRuleApproachingWall::Initialize(AWjWorldGameModePlay* InGameMode)
{
	Super::Initialize(InGameMode);

	// DeveloperSettings에서 WallDescriptionAsset 로드
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	TSoftObjectPtr<UWjWorldWallDescriptionDataAsset> WallDescAsset;
	if (Settings && !Settings->WallDescriptionAsset.IsNull())
	{
		WallDescAsset = Settings->WallDescriptionAsset;
	}

	BrickSpawner = UWjWorldBrickSpawner::CreateBrickSpawner(this, WallDescAsset);
	if (!BrickSpawner)
	{
		UE_LOG(LogWjWorld, Error, TEXT("Failed to create BrickSpawner in GameRuleApporachingWall"));
	}

	WallManager = UWjWorldWallManager::CreateWallManager(this);
	if (!WallManager)
	{
		UE_LOG(LogWjWorld, Error, TEXT("Failed to create WallManager in GameRuleApporachingWall"));
	}
	else
	{
		WallManager->SetGameRule(this);
	}
}

void UWjWorldGameRuleApproachingWall::OnGameReady()
{
	if (!HasAuthority()) return;

	Super::OnGameReady();

	bIsGameReady = true;

	if(BrickSpawner)
	{
		BrickSpawner->OnWallSpawnFinished.AddUObject(this, &UWjWorldGameRuleApproachingWall::OnWallSpawnFinished);

		// MapOption에 따라 Random 또는 특정 WallName 스폰
		AWjWorldGameModePlay* GameModePlay = GetGameModePlay();
		FString CurrentMapOption = GameModePlay ? GameModePlay->GetMapOption() : FString();

		if (CurrentMapOption.IsEmpty() || CurrentMapOption.Equals(TEXT("Random"), ESearchCase::IgnoreCase))
		{
			BrickSpawner->SpawnBricksRandomMapAsync();
			UE_LOG(LogWjWorld, Log, TEXT("UWjWorldGameRuleApproachingWall::OnGameReady >>> SpawnBricksRandomMapAsync"));
		}
		else
		{
			BrickSpawner->SpawnBricksFromWallNameAsync(CurrentMapOption);
			UE_LOG(LogWjWorld, Log, TEXT("UWjWorldGameRuleApproachingWall::OnGameReady >>> SpawnBricksFromWallNameAsync('%s')"), *CurrentMapOption);
		}
	}

	InternalGameReadyProcess();

	UE_LOG(LogWjWorld, Log, TEXT("UWjWorldGameRuleApproachingWall::OnGameReady()"));
}

void UWjWorldGameRuleApproachingWall::OnGameStart()
{
	if (!HasAuthority()) return;

	Super::OnGameStart();

	UE_LOG(LogWjWorld, Log, TEXT("UWjWorldGameRuleApproachingWall::OnGameStart()"));
	bIsGameStarted = true;
	InternalGameStartProcess();
}

void UWjWorldGameRuleApproachingWall::OnGameEnd()
{
	if (!HasAuthority()) return;

	// 게임 결과 설정
	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (GameState)
	{
		if (WinnerPlayer.IsValid())
		{
			GameState->SetGameResult(WinnerPlayer->GetPlayerName(), true);
			UE_LOG(LogWjWorld, Log, TEXT("UWjWorldGameRuleApproachingWall::OnGameEnd - Winner: %s"), *WinnerPlayer->GetPlayerName());
		}
		else
		{
			// 승자 없음 (무승부 또는 모두 탈락)
			GameState->SetGameResult(TEXT(""), false);
			UE_LOG(LogWjWorld, Log, TEXT("UWjWorldGameRuleApproachingWall::OnGameEnd - No winner (Draw)"));
		}
	}

	// 부모 클래스의 OnGameEnd 호출 (대기실 복귀 타이머 등)
	Super::OnGameEnd();
}

void UWjWorldGameRuleApproachingWall::OnPlayerJoined(AWjWorldPlayerStatePlay* Player)
{
	Super::OnPlayerJoined(Player);

	if (!HasAuthority() || !Player) return;

	// 플레이어를 생존자 목록에 추가
	AlivePlayers.Add(Player);
	AlivePlayerCount++;
	TotalPlayerCount++;

	UpdateGameData();

	UE_LOG(LogWjWorld, Log, TEXT("UWjWorldGameRuleApproachingWall::OnPlayerJoined - Player: %s, Alive: %d, Total: %d"),
		*Player->GetPlayerName(), AlivePlayerCount, TotalPlayerCount);
}

void UWjWorldGameRuleApproachingWall::OnPlayerLeft(AWjWorldPlayerStatePlay* Player)
{
	Super::OnPlayerLeft(Player);

	if (!HasAuthority() || !Player) return;

	// 플레이어가 나가면 생존자 목록에서 제거
	if (AlivePlayers.Remove(Player) > 0)
	{
		AlivePlayerCount--;
	}
	TotalPlayerCount--;

	UpdateGameData();

	UE_LOG(LogWjWorld, Log, TEXT("UWjWorldGameRuleApproachingWall::OnPlayerLeft - Player: %s, Alive: %d, Total: %d"),
		*Player->GetPlayerName(), AlivePlayerCount, TotalPlayerCount);

	// 남은 플레이어 수 확인하여 승리 조건 체크
	if (bIsGameStarted && CheckWinCondition())
	{
		bGameOverConditionMet = true;
		OnGameEnd();
	}
}

void UWjWorldGameRuleApproachingWall::OnPlayerEliminated(AWjWorldCharacterPlay* EliminatedCharacter)
{
	if (!HasAuthority() || !EliminatedCharacter) return;

	// 이미 제거된 캐릭터면 무시
	if (EliminatedCharacter->IsEliminated()) return;

	// 캐릭터 사망 처리
	EliminatedCharacter->OnEliminated();

	// PlayerState 가져오기
	AWjWorldPlayerStatePlay* PlayerState = EliminatedCharacter->GetPlayerState<AWjWorldPlayerStatePlay>();
	if (PlayerState)
	{
		// 생존자 목록에서 제거
		if (AlivePlayers.Remove(PlayerState) > 0)
		{
			AlivePlayerCount--;
		}

		UE_LOG(LogWjWorld, Log, TEXT("UWjWorldGameRuleApproachingWall::OnPlayerEliminated - Player: %s eliminated. Alive: %d"),
			*PlayerState->GetPlayerName(), AlivePlayerCount);
	}

	UpdateGameData();

	// 승리 조건 체크
	if (bIsGameStarted && CheckWinCondition())
	{
		bGameOverConditionMet = true;

		// 마지막 생존자를 승자로 설정
		if (AlivePlayerCount == 1 && AlivePlayers.Num() > 0)
		{
			for (const auto& AlivePlayer : AlivePlayers)
			{
				if (AlivePlayer.IsValid())
				{
					WinnerPlayer = AlivePlayer;
					break;
				}
			}
		}

		UE_LOG(LogWjWorld, Log, TEXT("Win condition met! Winner: %s"),
			WinnerPlayer.IsValid() ? *WinnerPlayer->GetPlayerName() : TEXT("None"));

		OnGameEnd();
	}
}

bool UWjWorldGameRuleApproachingWall::CheckWinCondition() const
{
	// 게임이 시작되지 않았으면 승리 조건 미충족
	if (!bIsGameStarted) return false;

	// 생존자가 1명 이하면 게임 종료
	return AlivePlayerCount <= 1;
}

AWjWorldPlayerStatePlay* UWjWorldGameRuleApproachingWall::GetWinner() const
{
	return WinnerPlayer.Get();
}

void UWjWorldGameRuleApproachingWall::TickGameRule(float DeltaTime)
{
	if (!HasAuthority()) return;
	if (!bGameStartInternalProcessDone) return;

	TimeSinceLastBrickMoveSignal += DeltaTime;

	if (TimeSinceLastBrickMoveSignal >= BrickMoveSignalInterval
		&& bGameOverConditionMet == false)
	{
		// 오차 누적 방지
		TimeSinceLastBrickMoveSignal -= BrickMoveSignalInterval;

		++BrickMoveSignalCount;

		bool bAnySafeZoneExist = false;
		ShrinkSafeZones(bAnySafeZoneExist);

		if (bAnySafeZoneExist)
		{
			if (WallManager)
			{
				WallManager->WallMoveStart(UWjWorldWallManager::GetBrickMovementAllowedTime(BrickMoveSignalCount));
			}

			GameLevelUp(BrickMoveSignalCount);
			UpdateGameData();

			if (PredictNextLevelIsLast())
			{
				OnGameEndPredict(BrickMoveSignalInterval);
			}
		}
		else
		{
			bGameOverConditionMet = true;
			UE_LOG(LogWjWorld, Log, TEXT("No Safe Zone Exist. Game Over Condition Met."));

			// 안전 구역이 없어져서 게임이 끝날 때, 마지막 생존자가 승자
			if (AlivePlayerCount == 1 && AlivePlayers.Num() > 0)
			{
				for (const auto& AlivePlayer : AlivePlayers)
				{
					if (AlivePlayer.IsValid())
					{
						WinnerPlayer = AlivePlayer;
						break;
					}
				}
				UE_LOG(LogWjWorld, Log, TEXT("Last player standing wins: %s"),
					WinnerPlayer.IsValid() ? *WinnerPlayer->GetPlayerName() : TEXT("None"));
			}

			OnGameEnd();
		}
	}

	if (WallManager)
	{
		WallManager->Tick(DeltaTime);
	}
}

void UWjWorldGameRuleApproachingWall::InternalGameReadyProcess()
{
	if (!HasAuthority()) return;

	if (bIsWallSpawned && bIsGameReady)
	{
		UE_LOG(LogWjWorld, Log, TEXT("Ready Game after Wall Spawned"));

		if (GetWorld())
		{
			int32 SafeZoneIndex = 0;
			for (auto Iter = GetWorld()->GetPlayerControllerIterator(); Iter; ++Iter)
			{
				APlayerController* PC = Iter->Get();
				if (PC)
				{
					PC->ClientMessage(TEXT("Game Ready! Wall has spawned."));
					FVector SpawnLocation = SpawnSafeZones.IsValidIndex(SafeZoneIndex) ? SpawnSafeZones[SafeZoneIndex] : FVector::ZeroVector;
					SafeZoneIndex++;
					PC->GetPawn()->SetActorLocation(SpawnLocation);
				}
			}
		}
	}

	InternalGameStartProcess();
}

void UWjWorldGameRuleApproachingWall::InternalGameStartProcess()
{
	if (!HasAuthority()) return;

	if(bIsWallSpawned && bIsGameReady && bIsGameStarted && !bGameStartInternalProcessDone)
	{
		bGameStartInternalProcessDone = true;
		TimeSinceLastBrickMoveSignal = 0.0f;
		BrickMoveSignalCount = 0;

		UE_LOG(LogWjWorld, Log, TEXT("Starting Game after Wall Spawned"));
	}
}

void UWjWorldGameRuleApproachingWall::OnWallSpawnFinished(const TArray<FVector>& InSafeZones, const FWjWorldWallDescription& Desc)
{
	if (!HasAuthority()) return;

	bIsWallSpawned = true;
	SpawnSafeZones.Empty();
	SpawnSafeZones.Append(InSafeZones);

	WallDesc = Desc;

	InternalGameReadyProcess();

	if (BrickSpawner)
	{
		CurrentSafeZonePoints.Empty();
		CurrentSafeZonePoints.Append(BrickSpawner->GetStartSafeZonePoints());
		BrickSpawner->OnWallSpawnFinished.RemoveAll(this);
	}
}

void UWjWorldGameRuleApproachingWall::UpdateGameData()
{
	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (!GameState) return;

	UApproachingWallGameDataComponent* GameData = GameState->GetGameData<UApproachingWallGameDataComponent>();
	if (!GameData) return;

	GameData->SetCurrentLevel(BrickMoveSignalCount);
	GameData->SetAlivePlayerCount(AlivePlayerCount);
	GameData->SetTotalPlayerCount(TotalPlayerCount);
}

void UWjWorldGameRuleApproachingWall::ShrinkSafeZones(bool& bAnySafeZoneExist)
{
	FloodFillPoints.Empty();

	for (auto Iter = CurrentSafeZonePoints.CreateIterator(); Iter; ++Iter)
	{
		FIntPoint& SafeZonePoint = *Iter;
		TArray<FIntPoint> AdjacentPoints = {
			FIntPoint(SafeZonePoint.X + 1, SafeZonePoint.Y),
			FIntPoint(SafeZonePoint.X - 1, SafeZonePoint.Y),
			FIntPoint(SafeZonePoint.X, SafeZonePoint.Y + 1),
			FIntPoint(SafeZonePoint.X, SafeZonePoint.Y - 1)
		};

		for (const FIntPoint& AdjacentPoint : AdjacentPoints)
		{
			if (CurrentSafeZonePoints.Contains(AdjacentPoint) == false)
			{
				FloodFillPoints.Add(SafeZonePoint);
				break;
			}
		}
	}

	for(const FIntPoint& PointToRemove : FloodFillPoints)
	{
		CurrentSafeZonePoints.Remove(PointToRemove);
	}

	bAnySafeZoneExist = CurrentSafeZonePoints.Num() > 1;
}

bool UWjWorldGameRuleApproachingWall::PredictNextLevelIsLast()
{
	TSet<FIntPoint> SimulatedSafeZones = CurrentSafeZonePoints;
	TSet<FIntPoint> SimulatedFloodFillPoints;

	for (auto Iter = SimulatedSafeZones.CreateIterator(); Iter; ++Iter)
	{
		FIntPoint& SafeZonePoint = *Iter;
		TArray<FIntPoint> AdjacentPoints = {
			FIntPoint(SafeZonePoint.X + 1, SafeZonePoint.Y),
			FIntPoint(SafeZonePoint.X - 1, SafeZonePoint.Y),
			FIntPoint(SafeZonePoint.X, SafeZonePoint.Y + 1),
			FIntPoint(SafeZonePoint.X, SafeZonePoint.Y - 1)
		};

		for (const FIntPoint& AdjacentPoint : AdjacentPoints)
		{
			if (SimulatedSafeZones.Contains(AdjacentPoint) == false)
			{
				SimulatedFloodFillPoints.Add(SafeZonePoint);
				break;
			}
		}
	}

	for (const FIntPoint& PointToRemove : SimulatedFloodFillPoints)
	{
		SimulatedSafeZones.Remove(PointToRemove);
	}

	return SimulatedSafeZones.Num() <= 1;
}

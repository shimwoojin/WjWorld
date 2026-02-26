// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameRule/WjWorldGameRuleBase.h"
#include "GamePlay/Wall/WjWorldWallDescriptionDataAsset.h"
#include "WjWorldGameRuleApproachingWall.generated.h"

class UWjWorldBrickSpawner;
class UWjWorldWallManager;
class AWjWorldCharacterPlay;

/**
 * 
 */
UCLASS(Blueprintable)
class WJWORLD_API UWjWorldGameRuleApproachingWall : public UWjWorldGameRuleBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(AWjWorldGameModePlay* InGameMode) override;
	virtual void OnGameReady() override;
	virtual void OnGameStart() override;
	virtual void OnGameEnd() override;
	virtual void OnPlayerJoined(AWjWorldPlayerStatePlay* Player) override;
	virtual void OnPlayerLeft(AWjWorldPlayerStatePlay* Player) override;

	// 승리 조건
	virtual bool CheckWinCondition() const override;
	virtual AWjWorldPlayerStatePlay* GetWinner() const override;

	// 플레이어 제거 처리
	void OnPlayerEliminated(AWjWorldCharacterPlay* EliminatedCharacter);

	// 생존자 수 반환
	int32 GetAlivePlayerCount() const { return AlivePlayerCount; }

	// 최소 플레이어 수 (게임 시작 조건)
	int32 GetMinimumPlayerCount() const { return MinimumPlayerCount; }

	const TSet<FIntPoint>& GetFloodFillPoints() const { return FloodFillPoints; }
	const TSet<FIntPoint>& GetCurrentSafeZonePoints() const { return CurrentSafeZonePoints; }
	TObjectPtr<UWjWorldWallManager> GetWallManager() const { return WallManager; }

	const FWjWorldWallDescription& GetWallDesc() const { return WallDesc; }

protected:
	virtual void TickGameRule(float DeltaTime) override;

private:
	void InternalGameReadyProcess();
	void InternalGameStartProcess();

	UFUNCTION()
	void OnWallSpawnFinished(const TArray<FVector>& InSafeZones, const FWjWorldWallDescription& Desc);

	void ShrinkSafeZones(bool& bAnySafeZoneExist);
	virtual bool PredictNextLevelIsLast() override;

	/** 중앙 할당: 각 FloodFillPoint에 가장 가까운 벽돌을 배정 */
	void AssignBrickTargets();

	void UpdateGameData();

protected:
	UPROPERTY()
	TObjectPtr<UWjWorldWallManager> WallManager;

	UPROPERTY()
	TObjectPtr<UWjWorldBrickSpawner> BrickSpawner;

	UPROPERTY()
	FWjWorldWallDescription WallDesc;

	TSet<FIntPoint> CurrentSafeZonePoints;
	TSet<FIntPoint> FloodFillPoints;
	TArray<FVector> SpawnSafeZones;
	bool bIsWallSpawned = false;
	bool bIsGameStarted = false;
	bool bIsGameReady = false;
	bool bGameStartInternalProcessDone = false;
	bool bGameOverConditionMet = false;
	int32 BrickMoveSignalCount = 0;
	float TimeSinceLastBrickMoveSignal = 0.0f;
	float BrickMoveSignalInterval = 12.0f;

	// 플레이어 추적
	UPROPERTY()
	TArray<TWeakObjectPtr<AWjWorldPlayerStatePlay>> AlivePlayers;

	/** 게임 참여자 (관전자 제외, OnPlayerLeft에서 참여자 판별용) */
	UPROPERTY()
	TArray<TWeakObjectPtr<AWjWorldPlayerStatePlay>> AllParticipants;

	UPROPERTY()
	TWeakObjectPtr<AWjWorldPlayerStatePlay> WinnerPlayer;

	int32 AlivePlayerCount = 0;
	int32 TotalPlayerCount = 0;

	// 최소 플레이어 수 (기본값: 1, 솔로 테스트 허용)
	UPROPERTY(EditDefaultsOnly, Category = "GameRule")
	int32 MinimumPlayerCount = 1;

	// 킬 스탯 기록 (공격자가 있는 경우)
	void RecordKillStat(AWjWorldPlayerStatePlay* KillerPlayerState);
};

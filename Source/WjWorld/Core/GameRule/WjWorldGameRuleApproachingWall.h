// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameRule/WjWorldGameRuleBase.h"
#include "WjWorldGameRuleApproachingWall.generated.h"

class UWjWorldBrickSpawner;
class UWjWorldWallManager;
class AWjWorldCharacterPlay;

/**
 * 
 */
UCLASS()
class WJWORLD_API UWjWorldGameRuleApproachingWall : public UWjWorldGameRuleBase
{
	GENERATED_BODY()
	
	const FSoftObjectPath WallDescPath = FSoftObjectPath("/Game/GamePlay/Wall/DA_WallDescription");

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

	const TSet<FIntPoint>& GetFloodFillPoints() const { return FloodFillPoints; }
	TObjectPtr<UWjWorldWallManager> GetWallManager() const { return WallManager; }


protected:
	virtual void Tick(float DeltaTime) override;

private:
	void InternalGameReadyProcess();
	void InternalGameStartProcess();

	UFUNCTION()
	void OnWallSpawnFinished(const TArray<FVector>& InSafeZones);

	void ShrinkSafeZones(bool& bAnySafeZoneExist);
	virtual bool PredictNextLevelIsLast() override;;

protected:
	UPROPERTY()
	TObjectPtr<UWjWorldWallManager> WallManager;
	UPROPERTY()
	TObjectPtr<UWjWorldBrickSpawner> BrickSpawner;
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

	UPROPERTY()
	TWeakObjectPtr<AWjWorldPlayerStatePlay> WinnerPlayer;

	int32 AlivePlayerCount = 0;
	int32 TotalPlayerCount = 0;
};

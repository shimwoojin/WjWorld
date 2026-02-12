// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameRule/WjWorldGameRuleBase.h"
#include "WjWorldGameRuleJumpMap.generated.h"

class AWjWorldCharacterPlay;
class AWjWorldPlayerStatePlay;
class AJumpMapCheckpointActor;
class AJumpMapEndActor;
struct FPlacedObjectSaveEntry;
class AJumpMapActorBase;
class UJumpMapLayoutDataAsset;
struct FJumpMapLayout;

/**
 * JumpMap 미니게임 규칙
 * - 장애물 코스를 통과하여 결승점에 도달하는 레이스
 * - 사망 시 체크포인트에서 리스폰
 * - 시간 제한 + 완주 순서 기반 승리
 * - 배치 에디터 레이아웃 기반 맵 구성
 */
UCLASS(Blueprintable)
class WJWORLD_API UWjWorldGameRuleJumpMap : public UWjWorldGameRuleBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(AWjWorldGameModePlay* InGameMode) override;
	virtual void OnGameReady() override;
	virtual void OnGameStart() override;
	virtual void OnGameEnd() override;
	virtual void OnPlayerJoined(AWjWorldPlayerStatePlay* Player) override;
	virtual void OnPlayerLeft(AWjWorldPlayerStatePlay* Player) override;

	virtual bool CheckWinCondition() const override;
	virtual AWjWorldPlayerStatePlay* GetWinner() const override;

	/** 플레이어 사망 → 체크포인트에서 리스폰 */
	void OnPlayerDied(AWjWorldCharacterPlay* Character);

	/** 플레이어 완주 */
	void OnPlayerFinished(AWjWorldCharacterPlay* Character);

protected:
	virtual void TickGameRule(float DeltaTime) override;

private:
	void LoadLayoutAndSpawnActors();
	void SpawnActorsFromLayout(const FJumpMapLayout& Layout);
	void RespawnPlayerAtCheckpoint(AWjWorldCharacterPlay* Character, int32 CheckpointIndex);
	void UpdateGameData();

protected:
	/** 제한 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "GameRule")
	float TimeLimit = 120.f;

	/** 리스폰 대기 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "GameRule")
	float RespawnDelay = 1.f;

	/** 낙하 판정 Z 좌표 (이 미만이면 사망) */
	UPROPERTY(EditDefaultsOnly, Category = "GameRule")
	float FallThresholdZ = -500.f;

	/** ObjectId → 게임플레이 액터 클래스 매핑 (BP에서 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "GameRule|Layout")
	TMap<FName, TSubclassOf<AActor>> ObjectIdToActorClassMap;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> SpawnedGameplayActors;

	UPROPERTY()
	TArray<TWeakObjectPtr<AWjWorldPlayerStatePlay>> AllPlayers;

	UPROPERTY()
	TWeakObjectPtr<AWjWorldPlayerStatePlay> WinnerPlayer;

	FVector StartLocation = FVector::ZeroVector;
	TArray<FVector> CheckpointLocations;

	float ElapsedTime = 0.f;
	int32 TotalPlayerCount = 0;
	int32 FinishedPlayerCount = 0;
	bool bIsGameStarted = false;
	bool bGameOverConditionMet = false;
};

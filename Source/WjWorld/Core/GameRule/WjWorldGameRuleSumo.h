// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameRule/WjWorldGameRuleBase.h"
#include "WjWorldGameRuleSumo.generated.h"

class AWjWorldCharacterPlay;
class AWjWorldPlayerStatePlay;
class ASumoFloorRingActor;
class ASumoPowerUpActor;

/**
 * Sumo Knockoff 미니게임 규칙
 * - 원형 플랫폼 위에서 상대를 밀어 떨어뜨리는 PvP 서바이벌
 * - Z 위치 기반 낙하 감지 → 제거
 * - 3라운드 시스템 (순위 점수 → 총점 승자)
 * - 축소 플랫폼 (외곽 링 순차 파괴)
 * - 파워업 (스피드/슈퍼푸시/실드)
 * - 맵 변형 지원
 */
UCLASS(Blueprintable)
class WJWORLD_API UWjWorldGameRuleSumo : public UWjWorldGameRuleBase
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

	void OnPlayerEliminated(AWjWorldCharacterPlay* EliminatedCharacter);

	int32 GetAlivePlayerCount() const { return AlivePlayerCount; }

protected:
	virtual void TickGameRule(float DeltaTime) override;

private:
	// 플레이어 체크
	void CheckFallenPlayers();
	void UpdateGameData();
	void RecordKillStat(AWjWorldPlayerStatePlay* KillerPlayerState);

	// 킬피드
	void BroadcastKillFeed(const FString& KillerName, const FString& VictimName);

	// 라운드 시스템
	void OnRoundEnd();
	void ResetRound();
	void OnRoundStart();
	void AwardRoundScores();
	void UpdatePlayerScores();
	AWjWorldPlayerStatePlay* GetFinalWinner() const;

	// 축소 플랫폼
	void CollectFloorRings();
	void TickShrinkPlatform(float DeltaTime);

	// 파워업
	void TickPowerUpSpawn(float DeltaTime);
	void SpawnPowerUp();
	FVector GetRandomSpawnLocationOnActiveRings() const;
	void CleanupPowerUps();

	// 버프 정리
	void RemoveAllPlayerBuffs();

protected:
	/** 낙하 판정 Z 좌표 (이 미만이면 제거) */
	UPROPERTY(EditDefaultsOnly, Category = "GameRule")
	float FallThresholdZ = -500.f;

	/** 최대 라운드 수 */
	UPROPERTY(EditDefaultsOnly, Category = "GameRule|Round")
	int32 MaxRounds = 3;

	/** 라운드 간 대기 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "GameRule|Round")
	float RoundResetDelay = 3.f;

	/** 축소 주기 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "GameRule|Shrink")
	float ShrinkInterval = 10.f;

	/** 경고 표시 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "GameRule|Shrink")
	float WarningDuration = 3.f;

	/** 파워업 스폰 주기 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "GameRule|PowerUp")
	float PowerUpSpawnInterval = 8.f;

	/** 최대 동시 활성 파워업 수 */
	UPROPERTY(EditDefaultsOnly, Category = "GameRule|PowerUp")
	int32 MaxActivePowerUps = 3;

	/** 파워업 액터 클래스 (BP에서 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "GameRule|PowerUp")
	TSubclassOf<ASumoPowerUpActor> PowerUpActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "GameRule")
	int32 MinimumPlayerCount = 1;

private:
	// 플레이어 추적
	UPROPERTY()
	TArray<TWeakObjectPtr<AWjWorldPlayerStatePlay>> AlivePlayers;

	/** 모든 참여 플레이어 (라운드 리셋 시 사용) */
	UPROPERTY()
	TArray<TWeakObjectPtr<AWjWorldPlayerStatePlay>> AllPlayers;

	UPROPERTY()
	TWeakObjectPtr<AWjWorldPlayerStatePlay> WinnerPlayer;

	int32 AlivePlayerCount = 0;
	int32 TotalPlayerCount = 0;
	bool bIsGameStarted = false;
	bool bGameOverConditionMet = false;

	// 라운드
	int32 CurrentRound = 0;
	bool bIsRoundActive = false;
	bool bIsResettingRound = false;

	/** 탈락 순서 기록 (역순으로 점수 부여) */
	UPROPERTY()
	TArray<TWeakObjectPtr<AWjWorldPlayerStatePlay>> EliminationOrder;

	// 축소 플랫폼
	UPROPERTY()
	TArray<ASumoFloorRingActor*> FloorRings;

	int32 CurrentShrinkLevel = 0;
	float TimeSinceLastShrink = 0.f;
	bool bIsWarningActive = false;
	float WarningElapsed = 0.f;

	// 파워업
	UPROPERTY()
	TArray<TWeakObjectPtr<ASumoPowerUpActor>> ActivePowerUps;

	float TimeSinceLastPowerUp = 0.f;

	FTimerHandle RoundResetTimerHandle;
};

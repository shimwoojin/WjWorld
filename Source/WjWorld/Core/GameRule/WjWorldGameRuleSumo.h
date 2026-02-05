// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameRule/WjWorldGameRuleBase.h"
#include "WjWorldGameRuleSumo.generated.h"

class AWjWorldCharacterPlay;

/**
 * Sumo Knockoff 미니게임 규칙
 * - 원형 플랫폼 위에서 상대를 밀어 떨어뜨리는 PvP 서바이벌
 * - Z 위치 기반 낙하 감지 → 제거
 * - 최후 1인 생존 시 승리
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
	void CheckFallenPlayers();
	void UpdateGameData();
	void RecordKillStat(AWjWorldPlayerStatePlay* KillerPlayerState);

protected:
	/** 낙하 판정 Z 좌표 (이 미만이면 제거) */
	UPROPERTY(EditDefaultsOnly, Category = "GameRule")
	float FallThresholdZ = -500.f;

	// 플레이어 추적
	UPROPERTY()
	TArray<TWeakObjectPtr<AWjWorldPlayerStatePlay>> AlivePlayers;

	UPROPERTY()
	TWeakObjectPtr<AWjWorldPlayerStatePlay> WinnerPlayer;

	int32 AlivePlayerCount = 0;
	int32 TotalPlayerCount = 0;

	bool bIsGameStarted = false;
	bool bGameOverConditionMet = false;

	UPROPERTY(EditDefaultsOnly, Category = "GameRule")
	int32 MinimumPlayerCount = 1;
};

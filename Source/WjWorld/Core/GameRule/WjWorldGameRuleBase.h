// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Core/WjWorldCoreTypes.h"
#include "WjWorldGameRuleBase.generated.h"

class AWjWorldGameModePlay;
class AWjWorldPlayerStatePlay;
class AWjWorldGameStatePlay;
class UWjWorldGameDataComponent;

/**
 * 게임 규칙 베이스 클래스
 * GameMode에서 TickGameRule()을 통해 틱 호출
 */
UCLASS(Blueprintable)
class WJWORLD_API UWjWorldGameRuleBase : public UObject
{
	GENERATED_BODY()

public:
	// 라이프사이클
	virtual void Initialize(AWjWorldGameModePlay* InGameMode);
	virtual void OnGameReady();
	virtual void OnGameStart();
	virtual void OnGameEndPredict(float Seconds);
	virtual void OnGameEnd();

	// 이벤트
	virtual void OnPlayerJoined(AWjWorldPlayerStatePlay* Player);
	virtual void OnPlayerLeft(AWjWorldPlayerStatePlay* Player);

	// 승리 조건
	virtual bool CheckWinCondition() const;
	virtual AWjWorldPlayerStatePlay* GetWinner() const;

	// 틱 (GameMode에서 호출)
	virtual void TickGameRule(float DeltaTime);

	virtual void BeginDestroy() override;

	bool HasAuthority() const
	{
		UWorld* World = GetWorld();
		return World && World->GetAuthGameMode() != nullptr;
	}

    AWjWorldGameModePlay* GetGameModePlay() const;
    AWjWorldGameStatePlay* GetGameStatePlay() const;

    /** 게임이 Playing 또는 Finished 상태인지 (중간 입장자 판별용) */
    bool IsGameInProgress() const;

    virtual bool PredictNextLevelIsLast() { return false; }

protected:
	void GameLevelUp(int32 NewLevel);

private:
    void ChangeGamePhase(EGamePhase GamePhase);

protected:
    UPROPERTY()
    TWeakObjectPtr<AWjWorldGameModePlay> GameMode;

    UPROPERTY(EditDefaultsOnly, Category = "DataComponent")
	TSubclassOf<UWjWorldGameDataComponent> GameDataComponentClass;

    UPROPERTY(EditDefaultsOnly, Category = "DataComponent")
    TSubclassOf<UWjWorldGameDataComponent> PlayerDataComponentClass;

    UPROPERTY(EditDefaultsOnly, Category = "GameRule")
	float StartDelay = 2.0f;

    /** OnGameEnd 중복 호출 방지 */
    bool bGameEnded = false;

    FTimerHandle DelayStartHandle;
    FTimerHandle GotoWaitingRoomHandle;

    UPROPERTY(EditDefaultsOnly, Category = "GameRule")
    float SecondsForGameStartCount = 3.0f;

    UPROPERTY(EditDefaultsOnly, Category = "GameRule")
    float SecondsForGotoWaitingRoom = 3.0f;
};

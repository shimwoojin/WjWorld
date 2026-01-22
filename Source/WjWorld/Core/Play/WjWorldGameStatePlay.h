// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldGameStateBase.h"
#include "Core/WjWorldCoreTypes.h"
#include "WjWorldGameStatePlay.generated.h"

class UWjWorldGameDataComponent;

/**
 * 
 */
UCLASS()
class WJWORLD_API AWjWorldGameStatePlay : public AWjWorldGameStateBase
{
	GENERATED_BODY()
	
	friend class UWjWorldGameRuleBase;

public:
    AWjWorldGameStatePlay();

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

    virtual bool HasMatchStarted() const override;
    virtual bool HasMatchEnded() const override;

	void GameStartWithCountdown(float CountdownTime);
	void GameEndWithCountdown(float CountdownTime);

	void SetGamePhase(EGamePhase NewPhase);

	// 게임 결과 설정 (서버에서 호출)
	void SetGameResult(const FString& WinnerName, bool bHasWinner);

	// 게임 결과 조회
	UFUNCTION(BlueprintCallable, Category = "Game Result")
	FString GetWinnerName() const { return WinnerPlayerName; }

	UFUNCTION(BlueprintCallable, Category = "Game Result")
	bool HasWinner() const { return bGameHasWinner; }

public:
    // 게임별 데이터는 컴포넌트로
    template<typename T>
    T* GetGameData() const { return FindComponentByClass<T>(); }

    UWjWorldGameDataComponent* AddGameDataComponent(TSubclassOf<UWjWorldGameDataComponent> InDataComponentClass);

    UFUNCTION()
    void OnRep_GamePhase();

    UFUNCTION()
    void OnRep_IsGameStartCountDownReady();

    UFUNCTION()
    void OnRep_IsGameEndCountDownReady();

    UFUNCTION()
    void OnRep_GameResult();

private:
    UPROPERTY()
    TObjectPtr<UWjWorldGameDataComponent> GameDataComponent;

    // 공통 데이터 (모든 미니게임)
    UPROPERTY(ReplicatedUsing = OnRep_GamePhase)
    EGamePhase CurrentPhase;  // Ready, Playing, Finished

    UPROPERTY(ReplicatedUsing = OnRep_IsGameStartCountDownReady)
    bool bIsGameStartCountDownReady;

    UPROPERTY(Replicated)
	float StartCountDownTime;

    UPROPERTY(ReplicatedUsing = OnRep_IsGameEndCountDownReady)
    bool bIsGameEndCountDownReady;

    UPROPERTY(Replicated)
    float EndCountDownTime;

    // 게임 결과 (승자 정보)
    UPROPERTY(ReplicatedUsing = OnRep_GameResult)
    FString WinnerPlayerName;

    UPROPERTY(ReplicatedUsing = OnRep_GameResult)
    bool bGameHasWinner = false;

    UPROPERTY(ReplicatedUsing = OnRep_GameResult)
    bool bGameResultReady = false;
};

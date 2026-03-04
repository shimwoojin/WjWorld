// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldGameStateBase.h"
#include "Core/WjWorldCoreTypes.h"
#include "GameplayTagContainer.h"
#include "GamePlay/Camera/WjCameraTypes.h"
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

    EGamePhase GetGamePhase() const { return CurrentPhase; }
	void SetGamePhase(EGamePhase NewPhase);

	// 게임 결과 설정 (서버에서 호출)
	void SetGameResult(const FString& WinnerName, bool bHasWinner);

    void SetGameRuleClass(TSubclassOf<UWjWorldGameRuleBase> InGameRuleClass);

	// 게임 결과 조회
	UFUNCTION(BlueprintCallable, Category = "Game Result")
	FString GetWinnerName() const { return WinnerPlayerName; }

	UFUNCTION(BlueprintCallable, Category = "Game Result")
	bool HasWinner() const { return bGameHasWinner; }

	// 어빌리티 제한 시스템
	void SetAllowedAbilityTags(const FGameplayTagContainer& InTags);
	const FGameplayTagContainer& GetAllowedAbilityTags() const { return AllowedAbilityTags; }

	// 스탯 네임스페이스
	void SetStatNamespace(FName InNamespace);
	FName GetStatNamespace() const { return StatNamespace; }

	// 초기 참여 인원 (1인 플레이 시 스탯/보상 제외용)
	void SetInitialPlayerCount(int32 InCount);
	int32 GetInitialPlayerCount() const { return InitialPlayerCount; }

	// 미니게임별 기본 카메라 모드
	void SetDefaultCameraMode(ECharacterCameraMode InMode);
	ECharacterCameraMode GetDefaultCameraMode() const { return DefaultCameraMode; }

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

    UFUNCTION()
    void OnRep_GameRuleClass();

    UFUNCTION()
    void OnRep_DefaultCameraMode();

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

    // 서버에서 카운트다운이 시작된 시각 (GetServerWorldTimeSeconds 기준)
    UPROPERTY(Replicated)
    double CountdownStartServerTime;

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

    UPROPERTY(ReplicatedUsing = OnRep_GameRuleClass)
	TSubclassOf<UWjWorldGameRuleBase> GameRuleClass;

    // 허용된 어빌리티 태그 (빈 컨테이너 = 전부 허용)
    UPROPERTY(Replicated)
    FGameplayTagContainer AllowedAbilityTags;

    // 스탯 네임스페이스 (미니게임별)
    UPROPERTY(Replicated)
    FName StatNamespace;

    // 게임 시작 시점 참여 인원 (1인 플레이 스탯/보상 제외용)
    UPROPERTY(Replicated)
    int32 InitialPlayerCount = 0;

    // 미니게임별 기본 카메라 모드
    UPROPERTY(ReplicatedUsing = OnRep_DefaultCameraMode)
    ECharacterCameraMode DefaultCameraMode = ECharacterCameraMode::TopDown;

    // OnRep_GameResult 중복 실행 방지 (3개 프로퍼티가 같은 OnRep 사용)
    bool bGameResultHandled = false;
};

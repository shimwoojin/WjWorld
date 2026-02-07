// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/OnlineReplStructs.h"

#if WITH_STEAM
#include "steam/steam_api.h"
#endif

#include "WjWorldStatsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLocalStatsReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUserStatsReceived, const FUniqueNetIdRepl&, UserId);

/**
 * Steam User Stats 래핑 서브시스템
 * - 로컬 플레이어 스탯 읽기/쓰기/저장
 * - 타 플레이어 스탯 비동기 조회
 * - WITHOUT_STEAM: TMap 폴백 (개발/테스트용, GConfig 저장)
 */
UCLASS()
class WJWORLD_API UWjWorldStatsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- 로컬 플레이어 스탯 ----

	/** Steam에서 현재 유저 스탯 요청 (Initialize에서 자동 호출) */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RequestCurrentStats();

	/** 로컬 스탯 값 반환 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	int32 GetLocalStat(FName StatName) const;

	/** 로컬 스탯 증가 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void IncrementLocalStat(FName StatName, int32 Delta = 1);

	/** Steam에 스탯 저장 (푸시) */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void StoreStats();

	// ---- 타 플레이어 스탯 (비동기) ----

	/** 타 플레이어 스탯 요청 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RequestUserStats(const FUniqueNetIdRepl& UserId);

	/** 타 플레이어 스탯 값 반환 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	int32 GetUserStat(const FUniqueNetIdRepl& UserId, FName StatName) const;

	/** 타 플레이어 스탯이 준비되었는지 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool IsUserStatsReady(const FUniqueNetIdRepl& UserId) const;

	// ---- 델리게이트 ----

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnLocalStatsReady OnLocalStatsReady;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnUserStatsReceived OnUserStatsReceived;

private:
	/** 로컬 스탯을 GConfig에 저장 (폴백) */
	void SaveLocalStatsToConfig();

	/** GConfig에서 로컬 스탯 로드 (폴백) */
	void LoadLocalStatsFromConfig();

#if WITH_STEAM
	/** Steam CCallResult 콜백 (RequestUserStats 결과) */
	void OnSteamUserStatsReceivedCallback(UserStatsReceived_t* pCallback, bool bIOFailure);

	bool bLocalStatsLoaded = false;

	/** 타 유저 스탯 요청 CCallResult 추적 */
	CCallResult<UWjWorldStatsSubsystem, UserStatsReceived_t> UserStatsCallResult;
#endif

	/** 로컬 스탯 캐시 (Steam이든 폴백이든 동일하게 사용) */
	TMap<FName, int32> LocalStats;

	/** 타 플레이어 스탯 캐시 */
	TMap<FString, TMap<FName, int32>> UserStatsCache;

	/** 스탯 요청이 완료된 유저 ID 목록 */
	TSet<FString> ReadyUserIds;

	static const FString StatsConfigSection;
};

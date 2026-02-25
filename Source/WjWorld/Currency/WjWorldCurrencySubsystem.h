// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Currency/WjWorldCurrencyTypes.h"
#include "WjWorldCurrencySubsystem.generated.h"

#if WITH_STEAM
#include "steam/steam_api.h"
#endif

class UWjWorldCosmeticSubsystem;
class UWjWorldPlaceableObjectDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrencyBalanceChanged, ECurrencyType, CurrencyType, int32, NewBalance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrencyPurchaseComplete, FName, ItemId, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlacementPurchaseComplete, FName, ObjectId, bool, bSuccess);

/**
 * 재화 관리 서브시스템
 * - Steam Inventory 기반 재화 (Coin/Gem) 잔액 조회
 * - 미니게임 종료 시 보상 지급 (TriggerItemDrop)
 * - 재화로 코스메틱 구매 (ExchangeItems)
 * - 유료 재화 팩 구매 (StartPurchase)
 * - 비Steam 폴백: GConfig 기반 로컬 잔액
 */
UCLASS()
class WJWORLD_API UWjWorldCurrencySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- 잔액 조회 ----

	/** 특정 재화의 잔액 반환 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	int32 GetBalance(ECurrencyType Type) const;

	/** 전체 잔액 반환 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	TArray<FCurrencyBalance> GetAllBalances() const;

	// ---- 미니게임 보상 ----

	/** 미니게임 종료 시 보상 지급 (각 클라이언트에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void TriggerMatchReward(bool bIsWinner);

	/** 오늘 남은 일일 보상 횟수 반환 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	int32 GetRemainingDailyRewards() const;

	// ---- 재화로 코스메틱 구매 ----

	/** 재화를 소비하여 코스메틱 아이템 구매 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool PurchaseItemWithCurrency(FName CosmeticItemId, ECurrencyType CurrencyType);

	// ---- 재화로 배치 오브젝트 구매 ----

	/** 코인을 소비하여 배치 오브젝트 구매 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool PurchasePlacementObject(FName ObjectId);

	// ---- 유료 재화 팩 구매 ----

	/** Steam 결제로 유료 재화 팩 구매 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool PurchaseGemPack(int32 PackDefId);

	/** 재화 교환(ExchangeItems)이 진행 중인지 여부 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool IsExchangePending() const { return bExchangePending; }

	// ---- 잔액 갱신 ----

	/** Steam Inventory에서 재화 잔액 갱신 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void RefreshBalancesFromInventory();

	// ---- 테스트/디버그 ----

	/** 로컬 재화 부여 (테스트용) */
	UFUNCTION(BlueprintCallable, Category = "Currency|Debug", meta = (DevelopmentOnly))
	void GrantCurrencyLocally(ECurrencyType Type, int32 Amount);

	/** 잔액을 특정 값으로 설정 (테스트용) */
	UFUNCTION(BlueprintCallable, Category = "Currency|Debug", meta = (DevelopmentOnly))
	void SetCurrencyLocally(ECurrencyType Type, int32 Amount);

	/** 현재 잔액을 로그로 출력 */
	UFUNCTION(BlueprintCallable, Category = "Currency|Debug")
	void DebugPrintBalances() const;

	/** Steam 인벤토리에서 모든 재화(Coin/Gem) 소비 (테스트용) */
	UFUNCTION(BlueprintCallable, Category = "Currency|Debug", meta = (DevelopmentOnly))
	void DebugConsumeAllSteamCurrency();

	/** Steam 인벤토리의 모든 아이템 소비 — 전체 초기화 (테스트용) */
	UFUNCTION(BlueprintCallable, Category = "Currency|Debug", meta = (DevelopmentOnly))
	void DebugConsumeAllSteamItems();

	// ---- 델리게이트 ----

	UPROPERTY(BlueprintAssignable, Category = "Currency")
	FOnCurrencyBalanceChanged OnCurrencyBalanceChanged;

	UPROPERTY(BlueprintAssignable, Category = "Currency")
	FOnCurrencyPurchaseComplete OnCurrencyPurchaseComplete;

	UPROPERTY(BlueprintAssignable, Category = "Currency")
	FOnPlacementPurchaseComplete OnPlacementPurchaseComplete;

private:
	/** 인벤토리 갱신 콜백 (CosmeticSubsystem.OnInventoryUpdated 구독) */
	UFUNCTION()
	void HandleInventoryUpdated();

	/** Exchange 결과 폴링 */
	void PollExchangeResult();

	/** Exchange 폴링 시작 */
	void StartExchangePolling();

	/** Exchange 폴링 중지 */
	void StopExchangePolling();

	/** GemPack 구매 결과 폴링 */
	void PollGemPurchaseResult();

	/** GemPack 폴링 시작 */
	void StartGemPurchasePolling();

	/** GemPack 폴링 중지 */
	void StopGemPurchasePolling();

	/** 로컬 잔액 저장 (비Steam 폴백) */
	void SaveBalancesToLocal();

	/** 로컬 잔액 로드 (비Steam 폴백) */
	void LoadBalancesFromLocal();

	/** 잔액 설정 및 델리게이트 브로드캐스트 */
	void SetBalance(ECurrencyType Type, int32 NewAmount);

	// 잔액 캐시
	int32 CoinBalance = 0;
	int32 GemBalance = 0;

	// 로컬 저장 섹션
	static const FString CurrencyConfigSection;

	// Exchange 관련
	FName PendingExchangeItemId;
	bool bPendingIsPlacement = false;

#if WITH_STEAM
	SteamInventoryResult_t ExchangeResultHandle = k_SteamInventoryResultInvalid;
	SteamInventoryResult_t GemPurchaseResultHandle = k_SteamInventoryResultInvalid;

	// 재화 인스턴스 ID 캐시 (ExchangeItems 호출에 필요)
	SteamItemInstanceID_t CachedCoinInstanceId = k_SteamItemInstanceIDInvalid;
	SteamItemInstanceID_t CachedGemInstanceId = k_SteamItemInstanceIDInvalid;
#endif

	FTimerHandle ExchangePollTimerHandle;
	FTimerHandle GemPurchasePollTimerHandle;

	bool bExchangePending = false;
	bool bGemPurchasePending = false;

	/** Gem 팩 구매 시작 시간 (타임아웃 판단용) */
	float GemPurchaseStartTime = 0.0f;

	// 일일 보상 제한 추적
	int32 TodayMatchRewardCount = 0;
	FDateTime LastRewardDate;

	/** 일일 보상 카운트 로드/저장 */
	void LoadDailyRewardData();
	void SaveDailyRewardData();

	/** 날짜 변경 시 카운트 리셋 */
	void CheckDailyReset();

	/** 인벤토리 갱신 지연 호출 (2.5초 + 5초 재시도) */
	void ScheduleInventoryRefresh();
};

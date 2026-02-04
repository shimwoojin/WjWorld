// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WjWorldPurchaseSubsystem.generated.h"

#if WITH_STEAM
#include "steam/steam_api.h"
#endif

class UWjWorldCosmeticSubsystem;

/**
 * 구매 상태
 */
UENUM(BlueprintType)
enum class EPurchaseState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Pending		UMETA(DisplayName = "Pending"),
	Completed	UMETA(DisplayName = "Completed"),
	Failed		UMETA(DisplayName = "Failed"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPurchaseComplete, FName, ItemId, bool, bSuccess);

/**
 * Steam 결제 처리 서브시스템
 * - Steam MicroTransaction API 호출 (#if WITH_STEAM)
 * - 구매 상태 관리
 * - 구매 완료 시 CosmeticSubsystem에 인벤토리 갱신 요청
 */
UCLASS()
class WJWORLD_API UWjWorldPurchaseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * 아이템 구매 요청
	 * @param ItemId 구매할 아이템의 플랫폼 독립 ID
	 * @return 구매 요청 성공 여부 (비동기, 결과는 OnPurchaseComplete로 수신)
	 */
	UFUNCTION(BlueprintCallable, Category = "Purchase")
	bool RequestPurchase(FName ItemId);

	/** 현재 구매 상태 */
	UFUNCTION(BlueprintCallable, Category = "Purchase")
	EPurchaseState GetPurchaseState() const { return CurrentState; }

	/** 현재 구매 진행 중 여부 */
	UFUNCTION(BlueprintCallable, Category = "Purchase")
	bool IsPurchasePending() const { return CurrentState == EPurchaseState::Pending; }

	/** 구매 완료 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Purchase")
	FOnPurchaseComplete OnPurchaseComplete;

private:
	/** Steam 구매 결과 처리 */
	void HandlePurchaseResult(bool bSuccess);

	/** 구매 완료 후 인벤토리 갱신 */
	void RefreshInventoryAfterPurchase();

	/** 구매 결과 폴링 */
	void PollPurchaseResult();

	/** 폴링 타이머 시작 */
	void StartPurchasePolling();

	/** 폴링 타이머 중지 */
	void StopPurchasePolling();

	EPurchaseState CurrentState = EPurchaseState::Idle;

	/** 현재 구매 중인 아이템 */
	FName PendingItemId;

#if WITH_STEAM
	/** 대기 중인 구매 결과 핸들 */
	SteamInventoryResult_t PurchaseResultHandle = k_SteamInventoryResultInvalid;
#endif

	/** 폴링 타이머 핸들 */
	FTimerHandle PurchasePollTimerHandle;

	/** 폴링 최대 대기 시간 (초) */
	float PurchaseTimeoutSeconds = 300.0f;

	/** 폴링 시작 시간 */
	float PurchaseStartTime = 0.0f;
};

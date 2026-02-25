// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Cosmetic/WjWorldCosmeticTypes.h"
#include "WjWorldCosmeticSubsystem.generated.h"

#if WITH_STEAM
#include "steam/steam_api.h"
#endif

class UWjWorldCosmeticCatalogDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadoutChanged, ECosmeticSlot, Slot, FName, ItemId);

/**
 * 코스메틱 인벤토리 및 로드아웃 관리 서브시스템
 * - Steam Inventory 조회 (#if WITH_STEAM)
 * - 인벤토리 캐시 (SteamItemDefId → ItemId 변환)
 * - 로드아웃 장착/해제
 * - 로컬 저장/로드 (GConfig)
 */
UCLASS()
class WJWORLD_API UWjWorldCosmeticSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- 카탈로그 ----

	/** 카탈로그 데이터 에셋 설정 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void SetCatalog(UWjWorldCosmeticCatalogDataAsset* InCatalog);

	/** 카탈로그 데이터 에셋 반환 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	UWjWorldCosmeticCatalogDataAsset* GetCatalog() const { return Catalog; }

	// ---- 인벤토리 ----

	/** Steam으로부터 인벤토리를 갱신 요청 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void RequestInventoryRefresh();

	/** 캐시된 인벤토리 반환 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	const TArray<FCosmeticItemInstance>& GetInventory() const { return CachedInventory; }

	/** 특정 아이템 보유 여부 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	bool HasItem(FName ItemId) const;

	/** 인벤토리에 아이템 추가 (테스트/오프라인용) */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void GrantItemLocally(FName ItemId, int32 Quantity = 1);

	// ---- Promo 아이템 ----

	/** Steam AddPromoItem을 사용하여 프로모 아이템 지급 (promo: "manual" 설정된 아이템) */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	bool AddPromoItem(int32 SteamItemDefId);

	/** 모든 프로모 아이템 지급 (promo 설정된 모든 아이템) */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	bool AddAllPromoItems();

	// ---- 전체 아이템 수량 조회 (배치 오브젝트 소유권 등) ----

	/** SteamItemDefId별 보유 수량 조회 (코스메틱 카탈로그 외 아이템 포함) */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	int32 GetItemQuantityByDefId(int32 SteamItemDefId) const;

	/** 전체 SteamItemDefId별 수량 캐시 (코스메틱 + 배치 오브젝트 등 모든 아이템) */
	TMap<int32, int32> AllItemQuantities;

	/** 전체 SteamItemDefId별 인스턴스 ID 캐시 (재화 ExchangeItems에 필요) */
	TMap<int32, uint64> AllItemInstanceIds;

	// ---- 테스트/디버그 ----

	/** Steam GenerateItems를 사용하여 테스트 아이템 생성 (개발 빌드 전용) */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic|Debug", meta = (DevelopmentOnly))
	bool GenerateTestItem(FName ItemId);

	/** 모든 카탈로그 아이템을 로컬 인벤토리에 추가 (개발 빌드 전용) */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic|Debug", meta = (DevelopmentOnly))
	void GrantAllItemsLocally();

	/** 로컬 인벤토리 초기화 (개발 빌드 전용) */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic|Debug", meta = (DevelopmentOnly))
	void ClearLocalInventory();

	/** 현재 인벤토리 상태를 로그로 출력 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic|Debug")
	void DebugPrintInventory() const;

	/** 현재 로드아웃 상태를 로그로 출력 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic|Debug")
	void DebugPrintLoadout() const;

	// ---- 로드아웃 ----

	/** 현재 로드아웃 반환 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	const FCosmeticLoadout& GetLoadout() const { return CurrentLoadout; }

	/** 슬롯에 아이템 장착 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	bool EquipItem(ECosmeticSlot Slot, FName ItemId);

	/** 슬롯 아이템 해제 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void UnequipSlot(ECosmeticSlot Slot);

	// ---- 로컬 저장 ----

	/** 로드아웃을 로컬에 저장 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void SaveLoadoutToLocal();

	/** 로컬에서 로드아웃 로드 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void LoadLoadoutFromLocal();

	// ---- 델리게이트 ----

	UPROPERTY(BlueprintAssignable, Category = "Cosmetic")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Cosmetic")
	FOnLoadoutChanged OnLoadoutChanged;

private:
	/** Steam Inventory 결과 처리 (폴링 방식) */
	void PollSteamInventoryResult();

	/** 인벤토리 결과 파싱 */
	void ParseInventoryResult();

	/** 폴링 타이머 시작 */
	void StartInventoryPolling();

	/** 폴링 타이머 중지 */
	void StopInventoryPolling();

	UPROPERTY()
	TObjectPtr<UWjWorldCosmeticCatalogDataAsset> Catalog;

	/** 캐시된 인벤토리 */
	TArray<FCosmeticItemInstance> CachedInventory;

	/** 현재 로드아웃 */
	FCosmeticLoadout CurrentLoadout;

	/** 배치 인벤토리 비Steam 폴백 로드 */
	void LoadPlacementInventoryFromLocal();

	/** 로드아웃 저장/로드용 설정 섹션 */
	static const FString LoadoutConfigSection;

#if WITH_STEAM
	/** 대기 중인 인벤토리 결과 핸들 */
	SteamInventoryResult_t PendingResultHandle = k_SteamInventoryResultInvalid;
#endif

	/** 폴링 타이머 핸들 */
	FTimerHandle InventoryPollTimerHandle;

	/** 인벤토리 요청 진행 중 여부 */
	bool bInventoryRequestPending = false;
};

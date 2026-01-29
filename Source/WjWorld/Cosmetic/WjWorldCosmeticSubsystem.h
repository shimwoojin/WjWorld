// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Cosmetic/WjWorldCosmeticTypes.h"
#include "WjWorldCosmeticSubsystem.generated.h"

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
	/** Steam Inventory 결과 처리 */
	void HandleSteamInventoryResult();

	UPROPERTY()
	TObjectPtr<UWjWorldCosmeticCatalogDataAsset> Catalog;

	/** 캐시된 인벤토리 */
	TArray<FCosmeticItemInstance> CachedInventory;

	/** 현재 로드아웃 */
	FCosmeticLoadout CurrentLoadout;

	/** 로드아웃 저장/로드용 설정 섹션 */
	static const FString LoadoutConfigSection;
};

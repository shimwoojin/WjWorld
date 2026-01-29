// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Cosmetic/WjWorldCosmeticTypes.h"
#include "WjWorldCosmeticDataAsset.generated.h"

/**
 * 개별 코스메틱 아이템 정의
 * ItemId(FName)를 중간 키로 사용하여 Steam ItemDefId와 비주얼 에셋을 매핑
 */
USTRUCT(BlueprintType)
struct WJWORLD_API FCosmeticItemDefinition
{
	GENERATED_BODY()

	/** 플랫폼 독립적 아이템 식별자 (중간 키) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemId;

	/** Steam Inventory Item Definition ID (플랫폼 종속) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SteamItemDefId = 0;

	/** 표시 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	/** 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	/** 장착 슬롯 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECosmeticSlot Slot = ECosmeticSlot::None;

	/** 희귀도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECosmeticRarity Rarity = ECosmeticRarity::Common;

	/** 메시 에셋 (비동기 로드용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	/** 스태틱 메시 에셋 (Back, Effect 등 슬롯용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	/** 아이콘 텍스처 (UI용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	/** 가격 (상점 표시용, 실제 결제는 Steam 처리) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Price = 0;

	bool IsValid() const { return !ItemId.IsNone(); }
};

/**
 * 코스메틱 카탈로그 데이터 에셋
 * 전체 코스메틱 아이템 목록을 관리하며, 양방향 조회(ItemId ↔ SteamItemDefId) 지원
 */
UCLASS(BlueprintType)
class WJWORLD_API UWjWorldCosmeticCatalogDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 전체 아이템 정의 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cosmetic")
	TArray<FCosmeticItemDefinition> Items;

	/** ItemId로 아이템 정의 조회 */
	const FCosmeticItemDefinition* FindByItemId(FName ItemId) const;

	/** SteamItemDefId로 아이템 정의 조회 */
	const FCosmeticItemDefinition* FindBySteamItemDefId(int32 SteamItemDefId) const;

	/** SteamItemDefId → ItemId 변환 */
	FName SteamItemDefIdToItemId(int32 SteamItemDefId) const;

	/** ItemId → SteamItemDefId 변환 */
	int32 ItemIdToSteamItemDefId(FName ItemId) const;

	/** 특정 슬롯의 모든 아이템 반환 */
	TArray<const FCosmeticItemDefinition*> GetItemsBySlot(ECosmeticSlot Slot) const;

//#if WITH_EDITOR
//	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
//#endif

protected:
	/** 내부 검색 최적화용 인덱스 (런타임에 빌드) */
	void BuildLookupMaps() const;

	mutable TMap<FName, int32> ItemIdToIndex;
	mutable TMap<int32, int32> SteamDefIdToIndex;
	mutable bool bLookupDirty = true;
};

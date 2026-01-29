// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WjWorldCosmeticTypes.generated.h"

/**
 * 코스메틱 슬롯 종류
 */
UENUM(BlueprintType)
enum class ECosmeticSlot : uint8
{
	None	UMETA(DisplayName = "None"),
	Head	UMETA(DisplayName = "Head"),
	Body	UMETA(DisplayName = "Body"),
	Back	UMETA(DisplayName = "Back"),
	Effect	UMETA(DisplayName = "Effect"),
};

/**
 * 코스메틱 희귀도
 */
UENUM(BlueprintType)
enum class ECosmeticRarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	Epic		UMETA(DisplayName = "Epic"),
	Legendary	UMETA(DisplayName = "Legendary"),
};

/**
 * 개별 코스메틱 아이템 인스턴스 (인벤토리에 보유한 아이템)
 * FName ItemId를 중간 키로 사용하여 플랫폼 독립적 식별
 */
USTRUCT(BlueprintType)
struct WJWORLD_API FCosmeticItemInstance
{
	GENERATED_BODY()

	FCosmeticItemInstance() = default;

	explicit FCosmeticItemInstance(FName InItemId)
		: ItemId(InItemId)
	{
	}

	/** 플랫폼 독립적 아이템 식별자 (DataAsset과 매핑) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemId;

	/** 보유 수량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	bool IsValid() const { return !ItemId.IsNone(); }

	bool operator==(const FCosmeticItemInstance& Other) const
	{
		return ItemId == Other.ItemId;
	}

	friend uint32 GetTypeHash(const FCosmeticItemInstance& Instance)
	{
		return GetTypeHash(Instance.ItemId);
	}
};

/**
 * 코스메틱 로드아웃 (장착 중인 슬롯 → 아이템 매핑)
 */
USTRUCT(BlueprintType)
struct WJWORLD_API FCosmeticLoadout
{
	GENERATED_BODY()

	/** 슬롯별 장착 아이템 */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//TODO:리플리케이션 사용을 위해 TMap 사용 불가, 추후 다른 방법으로 변경 필요
	TMap<ECosmeticSlot, FName> EquippedItems;

	/** 특정 슬롯에 아이템 장착 */
	void Equip(ECosmeticSlot Slot, FName ItemId)
	{
		if (Slot != ECosmeticSlot::None && !ItemId.IsNone())
		{
			EquippedItems.Add(Slot, ItemId);
		}
	}

	/** 특정 슬롯의 아이템 해제 */
	void Unequip(ECosmeticSlot Slot)
	{
		EquippedItems.Remove(Slot);
	}

	/** 특정 슬롯에 장착된 아이템 반환 */
	FName GetEquippedItem(ECosmeticSlot Slot) const
	{
		const FName* Found = EquippedItems.Find(Slot);
		return Found ? *Found : NAME_None;
	}

	/** 로드아웃이 비었는지 확인 */
	bool IsEmpty() const
	{
		return EquippedItems.Num() == 0;
	}

	bool operator==(const FCosmeticLoadout& Other) const
	{
		return EquippedItems.OrderIndependentCompareEqual(Other.EquippedItems);
	}

	bool operator!=(const FCosmeticLoadout& Other) const
	{
		return !(*this == Other);
	}
};

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
 * 코스메틱 슬롯-아이템 매핑 엔트리 (리플리케이션 가능)
 */
USTRUCT(BlueprintType)
struct WJWORLD_API FCosmeticSlotEntry
{
	GENERATED_BODY()

	FCosmeticSlotEntry() = default;

	FCosmeticSlotEntry(ECosmeticSlot InSlot, FName InItemId)
		: Slot(InSlot), ItemId(InItemId)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECosmeticSlot Slot = ECosmeticSlot::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemId;

	bool operator==(const FCosmeticSlotEntry& Other) const
	{
		return Slot == Other.Slot && ItemId == Other.ItemId;
	}

	bool operator!=(const FCosmeticSlotEntry& Other) const
	{
		return !(*this == Other);
	}
};

/**
 * 코스메틱 로드아웃 (장착 중인 슬롯 → 아이템 매핑)
 * TArray<FCosmeticSlotEntry> 기반으로 리플리케이션 지원
 */
USTRUCT(BlueprintType)
struct WJWORLD_API FCosmeticLoadout
{
	GENERATED_BODY()

	/** 슬롯별 장착 아이템 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCosmeticSlotEntry> Entries;

	/** 특정 슬롯에 아이템 장착 */
	void Equip(ECosmeticSlot Slot, FName ItemId)
	{
		if (Slot == ECosmeticSlot::None || ItemId.IsNone())
		{
			return;
		}

		// 이미 해당 슬롯에 엔트리가 있으면 교체
		for (FCosmeticSlotEntry& Entry : Entries)
		{
			if (Entry.Slot == Slot)
			{
				Entry.ItemId = ItemId;
				return;
			}
		}

		Entries.Emplace(Slot, ItemId);
	}

	/** 특정 슬롯의 아이템 해제 */
	void Unequip(ECosmeticSlot Slot)
	{
		Entries.RemoveAll([Slot](const FCosmeticSlotEntry& Entry)
		{
			return Entry.Slot == Slot;
		});
	}

	/** 특정 슬롯에 장착된 아이템 반환 */
	FName GetEquippedItem(ECosmeticSlot Slot) const
	{
		for (const FCosmeticSlotEntry& Entry : Entries)
		{
			if (Entry.Slot == Slot)
			{
				return Entry.ItemId;
			}
		}
		return NAME_None;
	}

	/** 로드아웃이 비었는지 확인 */
	bool IsEmpty() const
	{
		return Entries.Num() == 0;
	}

	/** 모든 슬롯의 키 배열 반환 */
	TArray<ECosmeticSlot> GetEquippedSlots() const
	{
		TArray<ECosmeticSlot> Slots;
		Slots.Reserve(Entries.Num());
		for (const FCosmeticSlotEntry& Entry : Entries)
		{
			Slots.Add(Entry.Slot);
		}
		return Slots;
	}

	/** 초기화 */
	void Reset()
	{
		Entries.Reset();
	}

	bool operator==(const FCosmeticLoadout& Other) const
	{
		if (Entries.Num() != Other.Entries.Num())
		{
			return false;
		}

		for (const FCosmeticSlotEntry& Entry : Entries)
		{
			if (Other.GetEquippedItem(Entry.Slot) != Entry.ItemId)
			{
				return false;
			}
		}
		return true;
	}

	bool operator!=(const FCosmeticLoadout& Other) const
	{
		return !(*this == Other);
	}
};

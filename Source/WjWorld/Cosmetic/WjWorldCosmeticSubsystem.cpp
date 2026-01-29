// Fill out your copyright notice in the Description page of Project Settings.

#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Cosmetic/WjWorldCosmeticDataAsset.h"
#include "WjWorldLogCategories.h"

#if WITH_STEAM
#include "steam/steam_api.h"
#endif

const FString UWjWorldCosmeticSubsystem::LoadoutConfigSection = TEXT("CosmeticLoadout");

void UWjWorldCosmeticSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadLoadoutFromLocal();

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticSubsystem 초기화 완료"));
}

void UWjWorldCosmeticSubsystem::Deinitialize()
{
	SaveLoadoutToLocal();

	Super::Deinitialize();
}

void UWjWorldCosmeticSubsystem::SetCatalog(UWjWorldCosmeticCatalogDataAsset* InCatalog)
{
	Catalog = InCatalog;
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("카탈로그 설정: %s"), InCatalog ? *InCatalog->GetName() : TEXT("null"));
}

void UWjWorldCosmeticSubsystem::RequestInventoryRefresh()
{
#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("Steam Inventory API를 사용할 수 없습니다."));
		return;
	}

	SteamInventoryResult_t ResultHandle = k_SteamInventoryResultInvalid;
	if (SteamInv->GetAllItems(&ResultHandle))
	{
		UE_LOG(LogWjWorldCosmetic, Log, TEXT("Steam 인벤토리 요청 성공 (ResultHandle: %d)"), ResultHandle);
		// Steam 콜백으로 결과 비동기 수신 → HandleSteamInventoryResult 에서 처리
	}
	else
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("Steam 인벤토리 요청 실패"));
	}
#else
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("Steam 미지원 환경 - 인벤토리 갱신 스킵"));
#endif
}

bool UWjWorldCosmeticSubsystem::HasItem(FName ItemId) const
{
	return CachedInventory.ContainsByPredicate([ItemId](const FCosmeticItemInstance& Inst)
	{
		return Inst.ItemId == ItemId && Inst.Quantity > 0;
	});
}

void UWjWorldCosmeticSubsystem::GrantItemLocally(FName ItemId, int32 Quantity)
{
	if (ItemId.IsNone() || Quantity <= 0)
	{
		return;
	}

	// 기존 아이템 수량 증가 또는 새로 추가
	FCosmeticItemInstance* Existing = CachedInventory.FindByPredicate([ItemId](const FCosmeticItemInstance& Inst)
	{
		return Inst.ItemId == ItemId;
	});

	if (Existing)
	{
		Existing->Quantity += Quantity;
	}
	else
	{
		FCosmeticItemInstance NewItem(ItemId);
		NewItem.Quantity = Quantity;
		CachedInventory.Add(NewItem);
	}

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("로컬 아이템 부여: %s x%d"), *ItemId.ToString(), Quantity);
	OnInventoryUpdated.Broadcast();
}

bool UWjWorldCosmeticSubsystem::EquipItem(ECosmeticSlot Slot, FName ItemId)
{
	if (Slot == ECosmeticSlot::None)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("None 슬롯에 장착할 수 없습니다."));
		return false;
	}

	if (!HasItem(ItemId))
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("아이템 '%s'을 보유하고 있지 않습니다."), *ItemId.ToString());
		return false;
	}

	// 카탈로그 검증: 슬롯 일치 확인
	if (Catalog)
	{
		const FCosmeticItemDefinition* Def = Catalog->FindByItemId(ItemId);
		if (Def && Def->Slot != Slot)
		{
			UE_LOG(LogWjWorldCosmetic, Warning, TEXT("아이템 '%s'은 슬롯 %d에 장착할 수 없습니다 (정의된 슬롯: %d)."),
				*ItemId.ToString(), static_cast<int32>(Slot), static_cast<int32>(Def->Slot));
			return false;
		}
	}

	CurrentLoadout.Equip(Slot, ItemId);
	OnLoadoutChanged.Broadcast(Slot, ItemId);

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("장착: 슬롯 %d ← %s"), static_cast<int32>(Slot), *ItemId.ToString());
	return true;
}

void UWjWorldCosmeticSubsystem::UnequipSlot(ECosmeticSlot Slot)
{
	CurrentLoadout.Unequip(Slot);
	OnLoadoutChanged.Broadcast(Slot, NAME_None);

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("해제: 슬롯 %d"), static_cast<int32>(Slot));
}

void UWjWorldCosmeticSubsystem::SaveLoadoutToLocal()
{
	FString ConfigFilePath = FPaths::GeneratedConfigDir() + TEXT("CosmeticLoadout.ini");

	for (const auto& Pair : CurrentLoadout.EquippedItems)
	{
		FString SlotKey = FString::Printf(TEXT("Slot_%d"), static_cast<int32>(Pair.Key));
		GConfig->SetString(*LoadoutConfigSection, *SlotKey, *Pair.Value.ToString(), ConfigFilePath);
	}

	GConfig->Flush(false, ConfigFilePath);
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("로드아웃 로컬 저장 완료"));
}

void UWjWorldCosmeticSubsystem::LoadLoadoutFromLocal()
{
	FString ConfigFilePath = FPaths::GeneratedConfigDir() + TEXT("CosmeticLoadout.ini");

	CurrentLoadout.EquippedItems.Reset();

	for (int32 SlotIdx = static_cast<int32>(ECosmeticSlot::Head);
		SlotIdx <= static_cast<int32>(ECosmeticSlot::Effect); ++SlotIdx)
	{
		FString SlotKey = FString::Printf(TEXT("Slot_%d"), SlotIdx);
		FString ItemIdStr;
		if (GConfig->GetString(*LoadoutConfigSection, *SlotKey, ItemIdStr, ConfigFilePath))
		{
			if (!ItemIdStr.IsEmpty())
			{
				CurrentLoadout.Equip(static_cast<ECosmeticSlot>(SlotIdx), FName(*ItemIdStr));
			}
		}
	}

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("로드아웃 로컬 로드 완료 (%d 슬롯)"), CurrentLoadout.EquippedItems.Num());
}

void UWjWorldCosmeticSubsystem::HandleSteamInventoryResult()
{
#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv || !Catalog)
	{
		return;
	}

	// 이 함수는 Steam 콜백에서 호출되어야 하며, 현재는 구조만 정의
	// 실제 콜백 등록은 Steam 통합 시 추가 구현 필요
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("Steam 인벤토리 결과 처리"));

	// 예시: ResultHandle에서 아이템 목록을 읽어 CachedInventory 갱신
	// SteamItemDetails_t* pDetails = ...;
	// for each item:
	//   FName ItemId = Catalog->SteamItemDefIdToItemId(pDetails[i].m_iDefinition);
	//   CachedInventory.Add(FCosmeticItemInstance(ItemId));

	OnInventoryUpdated.Broadcast();
#endif
}

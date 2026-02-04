// Fill out your copyright notice in the Description page of Project Settings.

#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Cosmetic/WjWorldCosmeticDataAsset.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "WjWorldLogCategories.h"
#include "Engine/World.h"
#include "TimerManager.h"

const FString UWjWorldCosmeticSubsystem::LoadoutConfigSection = TEXT("CosmeticLoadout");

void UWjWorldCosmeticSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// DeveloperSettings에서 카탈로그 로드
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (Settings && !Settings->CosmeticCatalog.IsNull())
	{
		Catalog = Settings->CosmeticCatalog.LoadSynchronous();
		UE_LOG(LogWjWorldCosmetic, Log, TEXT("카탈로그 로드: %s"), Catalog ? *Catalog->GetName() : TEXT("실패"));
	}
	else
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("DeveloperSettings에 CosmeticCatalog가 설정되지 않았습니다."));
	}

	LoadLoadoutFromLocal();

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticSubsystem 초기화 완료"));
}

void UWjWorldCosmeticSubsystem::Deinitialize()
{
	StopInventoryPolling();
	SaveLoadoutToLocal();

#if WITH_STEAM
	// 대기 중인 결과 핸들 정리
	if (PendingResultHandle != k_SteamInventoryResultInvalid)
	{
		ISteamInventory* SteamInv = SteamInventory();
		if (SteamInv)
		{
			SteamInv->DestroyResult(PendingResultHandle);
		}
		PendingResultHandle = k_SteamInventoryResultInvalid;
	}
#endif

	Super::Deinitialize();
}

void UWjWorldCosmeticSubsystem::SetCatalog(UWjWorldCosmeticCatalogDataAsset* InCatalog)
{
	Catalog = InCatalog;
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("카탈로그 설정: %s"), InCatalog ? *InCatalog->GetName() : TEXT("null"));
}

void UWjWorldCosmeticSubsystem::RequestInventoryRefresh()
{
	if (bInventoryRequestPending)
	{
		UE_LOG(LogWjWorldCosmetic, Log, TEXT("인벤토리 요청이 이미 진행 중입니다."));
		return;
	}

#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("Steam Inventory API를 사용할 수 없습니다."));
		return;
	}

	// 기존 핸들 정리
	if (PendingResultHandle != k_SteamInventoryResultInvalid)
	{
		SteamInv->DestroyResult(PendingResultHandle);
		PendingResultHandle = k_SteamInventoryResultInvalid;
	}

	if (SteamInv->GetAllItems(&PendingResultHandle))
	{
		bInventoryRequestPending = true;
		StartInventoryPolling();
		UE_LOG(LogWjWorldCosmetic, Log, TEXT("Steam 인벤토리 요청 성공 (ResultHandle: %d)"), PendingResultHandle);
	}
	else
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("Steam 인벤토리 요청 실패"));
	}
#else
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("Steam 미지원 환경 - 인벤토리 갱신 스킵"));
	OnInventoryUpdated.Broadcast();
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

	for (const FCosmeticSlotEntry& Entry : CurrentLoadout.Entries)
	{
		FString SlotKey = FString::Printf(TEXT("Slot_%d"), static_cast<int32>(Entry.Slot));
		GConfig->SetString(*LoadoutConfigSection, *SlotKey, *Entry.ItemId.ToString(), ConfigFilePath);
	}

	GConfig->Flush(false, ConfigFilePath);
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("로드아웃 로컬 저장 완료"));
}

void UWjWorldCosmeticSubsystem::LoadLoadoutFromLocal()
{
	FString ConfigFilePath = FPaths::GeneratedConfigDir() + TEXT("CosmeticLoadout.ini");

	CurrentLoadout.Reset();

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

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("로드아웃 로컬 로드 완료 (%d 슬롯)"), CurrentLoadout.Entries.Num());
}

void UWjWorldCosmeticSubsystem::StartInventoryPolling()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 100ms 간격으로 결과 폴링
	World->GetTimerManager().SetTimer(
		InventoryPollTimerHandle,
		this,
		&UWjWorldCosmeticSubsystem::PollSteamInventoryResult,
		0.1f,
		true
	);

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("인벤토리 폴링 시작"));
}

void UWjWorldCosmeticSubsystem::StopInventoryPolling()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(InventoryPollTimerHandle);
	}
	bInventoryRequestPending = false;

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("인벤토리 폴링 중지"));
}

void UWjWorldCosmeticSubsystem::PollSteamInventoryResult()
{
#if WITH_STEAM
	if (PendingResultHandle == k_SteamInventoryResultInvalid)
	{
		StopInventoryPolling();
		return;
	}

	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv)
	{
		StopInventoryPolling();
		return;
	}

	EResult Status = SteamInv->GetResultStatus(PendingResultHandle);

	if (Status == k_EResultPending)
	{
		// 아직 대기 중 - 계속 폴링
		return;
	}

	// 결과 처리
	if (Status == k_EResultOK)
	{
		UE_LOG(LogWjWorldCosmetic, Log, TEXT("Steam 인벤토리 결과 수신 성공"));
		ParseInventoryResult();
	}
	else
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("Steam 인벤토리 결과 실패 (Status: %d)"), static_cast<int32>(Status));
	}

	// 정리
	SteamInv->DestroyResult(PendingResultHandle);
	PendingResultHandle = k_SteamInventoryResultInvalid;
	StopInventoryPolling();

	// 델리게이트 브로드캐스트
	OnInventoryUpdated.Broadcast();
#else
	StopInventoryPolling();
#endif
}

void UWjWorldCosmeticSubsystem::ParseInventoryResult()
{
#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv || !Catalog)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("ParseInventoryResult: SteamInventory 또는 Catalog 없음"));
		return;
	}

	if (PendingResultHandle == k_SteamInventoryResultInvalid)
	{
		return;
	}

	// 아이템 개수 확인
	uint32 ItemCount = 0;
	if (!SteamInv->GetResultItems(PendingResultHandle, nullptr, &ItemCount) || ItemCount == 0)
	{
		UE_LOG(LogWjWorldCosmetic, Log, TEXT("Steam 인벤토리: 아이템 0개"));
		CachedInventory.Empty();
		return;
	}

	// 아이템 상세 정보 가져오기
	TArray<SteamItemDetails_t> ItemDetails;
	ItemDetails.SetNum(ItemCount);

	if (!SteamInv->GetResultItems(PendingResultHandle, ItemDetails.GetData(), &ItemCount))
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("Steam 인벤토리 아이템 상세 정보 가져오기 실패"));
		return;
	}

	// 캐시된 인벤토리 갱신
	CachedInventory.Empty(ItemCount);

	for (uint32 i = 0; i < ItemCount; ++i)
	{
		const SteamItemDetails_t& Details = ItemDetails[i];

		// SteamItemDefId → ItemId 변환
		FName ItemId = Catalog->SteamItemDefIdToItemId(Details.m_iDefinition);
		if (ItemId.IsNone())
		{
			UE_LOG(LogWjWorldCosmetic, Warning, TEXT("알 수 없는 SteamItemDefId: %d"), Details.m_iDefinition);
			continue;
		}

		// 수량 체크 (0이면 소비됨/만료됨)
		if (Details.m_unQuantity == 0)
		{
			continue;
		}

		// 기존 아이템 수량 증가 또는 새로 추가
		FCosmeticItemInstance* Existing = CachedInventory.FindByPredicate([ItemId](const FCosmeticItemInstance& Inst)
		{
			return Inst.ItemId == ItemId;
		});

		if (Existing)
		{
			Existing->Quantity += Details.m_unQuantity;
		}
		else
		{
			FCosmeticItemInstance NewItem(ItemId);
			NewItem.Quantity = Details.m_unQuantity;
			CachedInventory.Add(NewItem);
		}

		UE_LOG(LogWjWorldCosmetic, Verbose, TEXT("인벤토리 아이템: %s (DefId: %d, Qty: %d)"),
			*ItemId.ToString(), Details.m_iDefinition, Details.m_unQuantity);
	}

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("Steam 인벤토리 파싱 완료: %d개 아이템"), CachedInventory.Num());
#endif
}

bool UWjWorldCosmeticSubsystem::AddPromoItem(int32 SteamItemDefId)
{
#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("AddPromoItem: Steam Inventory API 사용 불가"));
		return false;
	}

	SteamItemDef_t ItemDef = static_cast<SteamItemDef_t>(SteamItemDefId);
	SteamInventoryResult_t ResultHandle = k_SteamInventoryResultInvalid;

	if (SteamInv->AddPromoItem(&ResultHandle, ItemDef))
	{
		UE_LOG(LogWjWorldCosmetic, Log, TEXT("AddPromoItem: 프로모 아이템 지급 요청 성공 (DefId: %d)"), SteamItemDefId);

		// 결과 핸들 정리 (비동기 결과는 나중에 인벤토리 갱신으로 확인)
		SteamInv->DestroyResult(ResultHandle);

		// 인벤토리 갱신 요청
		RequestInventoryRefresh();
		return true;
	}
	else
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("AddPromoItem: Steam AddPromoItem 실패 (DefId: %d)"), SteamItemDefId);
		return false;
	}
#else
	UE_LOG(LogWjWorldCosmetic, Warning, TEXT("AddPromoItem: Steam 미지원 환경"));
	return false;
#endif
}

bool UWjWorldCosmeticSubsystem::AddAllPromoItems()
{
#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("AddAllPromoItems: Steam Inventory API 사용 불가"));
		return false;
	}

	SteamInventoryResult_t ResultHandle = k_SteamInventoryResultInvalid;

	if (SteamInv->AddPromoItems(&ResultHandle, nullptr, 0))
	{
		UE_LOG(LogWjWorldCosmetic, Log, TEXT("AddAllPromoItems: 모든 프로모 아이템 지급 요청 성공"));

		SteamInv->DestroyResult(ResultHandle);
		RequestInventoryRefresh();
		return true;
	}
	else
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("AddAllPromoItems: Steam AddPromoItems 실패"));
		return false;
	}
#else
	UE_LOG(LogWjWorldCosmetic, Warning, TEXT("AddAllPromoItems: Steam 미지원 환경"));
	return false;
#endif
}

bool UWjWorldCosmeticSubsystem::GenerateTestItem(FName ItemId)
{
#if WITH_STEAM && !UE_BUILD_SHIPPING
	if (ItemId.IsNone())
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("GenerateTestItem: 유효하지 않은 ItemId"));
		return false;
	}

	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("GenerateTestItem: Steam Inventory API 사용 불가"));
		return false;
	}

	if (!Catalog)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("GenerateTestItem: 카탈로그 없음"));
		return false;
	}

	int32 SteamDefId = Catalog->ItemIdToSteamItemDefId(ItemId);
	if (SteamDefId == 0)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("GenerateTestItem: SteamItemDefId를 찾을 수 없음 (%s)"), *ItemId.ToString());
		return false;
	}

	SteamItemDef_t ItemDef = static_cast<SteamItemDef_t>(SteamDefId);
	uint32 Quantity = 1;
	SteamInventoryResult_t ResultHandle = k_SteamInventoryResultInvalid;

	if (SteamInv->GenerateItems(&ResultHandle, &ItemDef, &Quantity, 1))
	{
		UE_LOG(LogWjWorldCosmetic, Log, TEXT("GenerateTestItem: 테스트 아이템 생성 요청 성공 (%s, DefId: %d)"), *ItemId.ToString(), SteamDefId);

		// 결과 핸들 정리 (비동기이므로 즉시 파괴하면 안됨 - 실제 구현시 폴링 필요)
		// 간단한 테스트를 위해 로컬 인벤토리에도 추가
		GrantItemLocally(ItemId, 1);

		SteamInv->DestroyResult(ResultHandle);
		return true;
	}
	else
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("GenerateTestItem: Steam GenerateItems 실패"));
		return false;
	}
#else
	// 비 Steam 또는 Shipping 빌드: 로컬로 아이템 부여
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("GenerateTestItem: 로컬 아이템 부여 (비Steam 환경) - %s"), *ItemId.ToString());
	GrantItemLocally(ItemId, 1);
	return true;
#endif
}

void UWjWorldCosmeticSubsystem::GrantAllItemsLocally()
{
#if !UE_BUILD_SHIPPING
	if (!Catalog)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("GrantAllItemsLocally: 카탈로그 없음"));
		return;
	}

	int32 GrantedCount = 0;
	for (const FCosmeticItemDefinition& Def : Catalog->Items)
	{
		if (Def.IsValid() && !HasItem(Def.ItemId))
		{
			GrantItemLocally(Def.ItemId, 1);
			GrantedCount++;
		}
	}

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("GrantAllItemsLocally: %d개 아이템 부여 완료"), GrantedCount);
#endif
}

void UWjWorldCosmeticSubsystem::ClearLocalInventory()
{
#if !UE_BUILD_SHIPPING
	int32 PrevCount = CachedInventory.Num();
	CachedInventory.Empty();
	CurrentLoadout.Reset();

	OnInventoryUpdated.Broadcast();

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("ClearLocalInventory: 인벤토리 초기화 (%d개 아이템 제거)"), PrevCount);
#endif
}

void UWjWorldCosmeticSubsystem::DebugPrintInventory() const
{
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("========== 인벤토리 상태 =========="));
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("총 아이템: %d개"), CachedInventory.Num());

	for (const FCosmeticItemInstance& Item : CachedInventory)
	{
		FString DisplayName = TEXT("Unknown");
		if (Catalog)
		{
			if (const FCosmeticItemDefinition* Def = Catalog->FindByItemId(Item.ItemId))
			{
				DisplayName = Def->DisplayName.ToString();
			}
		}

		UE_LOG(LogWjWorldCosmetic, Log, TEXT("  - %s (%s) x%d"),
			*Item.ItemId.ToString(), *DisplayName, Item.Quantity);
	}
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("===================================="));
}

void UWjWorldCosmeticSubsystem::DebugPrintLoadout() const
{
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("========== 로드아웃 상태 =========="));
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("장착 슬롯: %d개"), CurrentLoadout.Entries.Num());

	auto SlotToString = [](ECosmeticSlot Slot) -> FString
	{
		switch (Slot)
		{
		case ECosmeticSlot::Head: return TEXT("Head");
		case ECosmeticSlot::Body: return TEXT("Body");
		case ECosmeticSlot::Back: return TEXT("Back");
		case ECosmeticSlot::Effect: return TEXT("Effect");
		default: return TEXT("None");
		}
	};

	for (const FCosmeticSlotEntry& Entry : CurrentLoadout.Entries)
	{
		FString DisplayName = TEXT("Unknown");
		if (Catalog)
		{
			if (const FCosmeticItemDefinition* Def = Catalog->FindByItemId(Entry.ItemId))
			{
				DisplayName = Def->DisplayName.ToString();
			}
		}

		UE_LOG(LogWjWorldCosmetic, Log, TEXT("  - [%s] %s (%s)"),
			*SlotToString(Entry.Slot), *Entry.ItemId.ToString(), *DisplayName);
	}
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("===================================="));
}

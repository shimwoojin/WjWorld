// Fill out your copyright notice in the Description page of Project Settings.

#include "Cosmetic/WjWorldPurchaseSubsystem.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Cosmetic/WjWorldCosmeticDataAsset.h"
#include "WjWorldLogCategories.h"

#if WITH_STEAM
#include "steam/steam_api.h"
#endif

void UWjWorldPurchaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// CosmeticSubsystem이 먼저 초기화되도록 의존성 보장
	Collection.InitializeDependency<UWjWorldCosmeticSubsystem>();

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("PurchaseSubsystem 초기화 완료"));
}

void UWjWorldPurchaseSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UWjWorldPurchaseSubsystem::RequestPurchase(FName ItemId)
{
	if (CurrentState == EPurchaseState::Pending)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("이미 구매가 진행 중입니다 (PendingItem: %s)."), *PendingItemId.ToString());
		return false;
	}

	if (ItemId.IsNone())
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("유효하지 않은 ItemId입니다."));
		return false;
	}

#if WITH_STEAM
	UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub || !CosmeticSub->GetCatalog())
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("CosmeticSubsystem 또는 카탈로그가 없습니다."));
		return false;
	}

	int32 SteamDefId = CosmeticSub->GetCatalog()->ItemIdToSteamItemDefId(ItemId);
	if (SteamDefId == 0)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("아이템 '%s'에 대한 SteamItemDefId를 찾을 수 없습니다."), *ItemId.ToString());
		return false;
	}

	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("Steam Inventory API를 사용할 수 없습니다."));
		return false;
	}

	// Steam StartPurchase를 통해 결제 오버레이 요청
	SteamItemDef_t ItemDef = static_cast<SteamItemDef_t>(SteamDefId);
	uint32 Quantity = 1;
	SteamInventoryResult_t ResultHandle = k_SteamInventoryResultInvalid;

	// Note: StartPurchase는 Steam 오버레이를 통해 비동기적으로 처리됨
	// 콜백 등록은 Steam 통합 확장 시 구현
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("Steam 구매 요청: ItemDefId=%d, ItemId=%s"), SteamDefId, *ItemId.ToString());

	CurrentState = EPurchaseState::Pending;
	PendingItemId = ItemId;
	return true;
#else
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("Steam 미지원 환경 - 구매 시뮬레이션: %s"), *ItemId.ToString());

	// 비 Steam 환경에서는 즉시 성공 처리 (테스트용)
	CurrentState = EPurchaseState::Pending;
	PendingItemId = ItemId;
	HandlePurchaseResult(true);
	return true;
#endif
}

void UWjWorldPurchaseSubsystem::HandlePurchaseResult(bool bSuccess)
{
	if (bSuccess)
	{
		CurrentState = EPurchaseState::Completed;
		UE_LOG(LogWjWorldCosmetic, Log, TEXT("구매 완료: %s"), *PendingItemId.ToString());

		RefreshInventoryAfterPurchase();
	}
	else
	{
		CurrentState = EPurchaseState::Failed;
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("구매 실패: %s"), *PendingItemId.ToString());
	}

	OnPurchaseComplete.Broadcast(PendingItemId, bSuccess);

	// 상태 리셋 (다음 구매를 위해)
	FName CompletedItemId = PendingItemId;
	PendingItemId = NAME_None;
	CurrentState = EPurchaseState::Idle;
}

void UWjWorldPurchaseSubsystem::RefreshInventoryAfterPurchase()
{
	UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (CosmeticSub)
	{
#if WITH_STEAM
		CosmeticSub->RequestInventoryRefresh();
#else
		// 비 Steam 환경: 로컬로 아이템 부여
		CosmeticSub->GrantItemLocally(PendingItemId, 1);
#endif
	}
}

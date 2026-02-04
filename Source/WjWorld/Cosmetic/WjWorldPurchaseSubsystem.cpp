// Fill out your copyright notice in the Description page of Project Settings.

#include "Cosmetic/WjWorldPurchaseSubsystem.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Cosmetic/WjWorldCosmeticDataAsset.h"
#include "WjWorldLogCategories.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UWjWorldPurchaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// CosmeticSubsystem이 먼저 초기화되도록 의존성 보장
	Collection.InitializeDependency<UWjWorldCosmeticSubsystem>();

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("PurchaseSubsystem 초기화 완료"));
}

void UWjWorldPurchaseSubsystem::Deinitialize()
{
	StopPurchasePolling();

#if WITH_STEAM
	// 대기 중인 결과 핸들 정리
	if (PurchaseResultHandle != k_SteamInventoryResultInvalid)
	{
		ISteamInventory* SteamInv = SteamInventory();
		if (SteamInv)
		{
			SteamInv->DestroyResult(PurchaseResultHandle);
		}
		PurchaseResultHandle = k_SteamInventoryResultInvalid;
	}
#endif

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

	// 기존 핸들 정리
	if (PurchaseResultHandle != k_SteamInventoryResultInvalid)
	{
		SteamInv->DestroyResult(PurchaseResultHandle);
		PurchaseResultHandle = k_SteamInventoryResultInvalid;
	}

	// Steam StartPurchase를 통해 결제 오버레이 요청
	SteamItemDef_t ItemDef = static_cast<SteamItemDef_t>(SteamDefId);
	uint32 Quantity = 1;

	if (SteamInv->StartPurchase(&ItemDef, &Quantity, 1))
	{
		CurrentState = EPurchaseState::Pending;
		PendingItemId = ItemId;
		PurchaseStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
		StartPurchasePolling();

		UE_LOG(LogWjWorldCosmetic, Log, TEXT("Steam 구매 요청 시작: ItemDefId=%d, ItemId=%s"), SteamDefId, *ItemId.ToString());
		return true;
	}
	else
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("Steam StartPurchase 실패: ItemDefId=%d"), SteamDefId);
		return false;
	}
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

void UWjWorldPurchaseSubsystem::StartPurchasePolling()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 1초 간격으로 구매 결과 폴링
	World->GetTimerManager().SetTimer(
		PurchasePollTimerHandle,
		this,
		&UWjWorldPurchaseSubsystem::PollPurchaseResult,
		1.0f,
		true
	);

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("구매 결과 폴링 시작"));
}

void UWjWorldPurchaseSubsystem::StopPurchasePolling()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(PurchasePollTimerHandle);
	}

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("구매 결과 폴링 중지"));
}

void UWjWorldPurchaseSubsystem::PollPurchaseResult()
{
#if WITH_STEAM
	if (CurrentState != EPurchaseState::Pending)
	{
		StopPurchasePolling();
		return;
	}

	// 타임아웃 체크
	UWorld* World = GetWorld();
	if (World)
	{
		float ElapsedTime = World->GetTimeSeconds() - PurchaseStartTime;
		if (ElapsedTime > PurchaseTimeoutSeconds)
		{
			UE_LOG(LogWjWorldCosmetic, Warning, TEXT("구매 타임아웃 (%.1f초 경과)"), ElapsedTime);
			HandlePurchaseResult(false);
			return;
		}
	}

	// Steam 콜백이 아닌 인벤토리 변경 체크 방식 사용
	// Steam은 구매 완료 시 자동으로 인벤토리에 아이템을 추가하므로,
	// 주기적으로 인벤토리를 확인하여 아이템이 추가되었는지 체크
	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv)
	{
		HandlePurchaseResult(false);
		return;
	}

	// 인벤토리 갱신 요청을 통해 구매 완료 여부 확인
	// (실제 콜백은 CosmeticSubsystem에서 처리)
	UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (CosmeticSub && CosmeticSub->HasItem(PendingItemId))
	{
		// 이미 아이템을 보유하고 있으면 구매 성공
		UE_LOG(LogWjWorldCosmetic, Log, TEXT("구매 확인: 아이템 '%s' 보유 감지"), *PendingItemId.ToString());
		HandlePurchaseResult(true);
	}
#else
	StopPurchasePolling();
#endif
}

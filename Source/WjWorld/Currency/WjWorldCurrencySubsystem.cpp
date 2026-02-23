// Fill out your copyright notice in the Description page of Project Settings.

#include "Currency/WjWorldCurrencySubsystem.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Cosmetic/WjWorldCosmeticDataAsset.h"
#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "WjWorldLogCategories.h"
#include "Engine/World.h"
#include "TimerManager.h"

const FString UWjWorldCurrencySubsystem::CurrencyConfigSection = TEXT("CurrencyBalance");

void UWjWorldCurrencySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// CosmeticSubsystem 의존성 보장
	Collection.InitializeDependency<UWjWorldCosmeticSubsystem>();

	// CosmeticSubsystem의 인벤토리 갱신 구독
	UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (CosmeticSub)
	{
		CosmeticSub->OnInventoryUpdated.AddDynamic(this, &UWjWorldCurrencySubsystem::HandleInventoryUpdated);
	}

	// 로컬 잔액 로드 (비Steam 폴백)
	LoadBalancesFromLocal();

	UE_LOG(LogWjWorldCurrency, Log, TEXT("CurrencySubsystem 초기화 완료 (Coin: %d, Gem: %d)"), CoinBalance, GemBalance);
}

void UWjWorldCurrencySubsystem::Deinitialize()
{
	StopExchangePolling();
	StopGemPurchasePolling();

	// 로컬 잔액 저장
	SaveBalancesToLocal();

	// 델리게이트 해제
	UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (CosmeticSub)
	{
		CosmeticSub->OnInventoryUpdated.RemoveDynamic(this, &UWjWorldCurrencySubsystem::HandleInventoryUpdated);
	}

#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (SteamInv)
	{
		if (ExchangeResultHandle != k_SteamInventoryResultInvalid)
		{
			SteamInv->DestroyResult(ExchangeResultHandle);
			ExchangeResultHandle = k_SteamInventoryResultInvalid;
		}
		if (GemPurchaseResultHandle != k_SteamInventoryResultInvalid)
		{
			SteamInv->DestroyResult(GemPurchaseResultHandle);
			GemPurchaseResultHandle = k_SteamInventoryResultInvalid;
		}
	}

	CachedCoinInstanceId = k_SteamItemInstanceIDInvalid;
	CachedGemInstanceId = k_SteamItemInstanceIDInvalid;
#endif

	Super::Deinitialize();
}

int32 UWjWorldCurrencySubsystem::GetBalance(ECurrencyType Type) const
{
	switch (Type)
	{
	case ECurrencyType::Coin: return CoinBalance;
	case ECurrencyType::Gem: return GemBalance;
	default: return 0;
	}
}

TArray<FCurrencyBalance> UWjWorldCurrencySubsystem::GetAllBalances() const
{
	TArray<FCurrencyBalance> Balances;
	Balances.Emplace(ECurrencyType::Coin, CoinBalance);
	Balances.Emplace(ECurrencyType::Gem, GemBalance);
	return Balances;
}

void UWjWorldCurrencySubsystem::TriggerMatchReward(bool bIsWinner)
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		return;
	}

	int32 GeneratorDefId = bIsWinner ? Settings->MatchWinRewardDefId : Settings->MatchLossRewardDefId;

	UE_LOG(LogWjWorldCurrency, Log, TEXT("매치 보상 트리거: %s (GeneratorDefId: %d)"),
		bIsWinner ? TEXT("승리") : TEXT("패배"), GeneratorDefId);

#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (SteamInv)
	{
		SteamInventoryResult_t ResultHandle = k_SteamInventoryResultInvalid;
		SteamItemDef_t DropDef = static_cast<SteamItemDef_t>(GeneratorDefId);

		if (SteamInv->TriggerItemDrop(&ResultHandle, DropDef))
		{
			UE_LOG(LogWjWorldCurrency, Log, TEXT("Steam TriggerItemDrop 요청 성공 (DefId: %d)"), GeneratorDefId);
			SteamInv->DestroyResult(ResultHandle);

			// 인벤토리 갱신으로 잔액 반영
			UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>();
			if (CosmeticSub)
			{
				CosmeticSub->RequestInventoryRefresh();
			}
		}
		else
		{
			UE_LOG(LogWjWorldCurrency, Warning, TEXT("Steam TriggerItemDrop 실패 (DefId: %d)"), GeneratorDefId);
		}

		return;
	}
#endif

	// 비Steam 폴백: 로컬 잔액 직접 증가
	int32 RewardAmount = bIsWinner ? 50 : 10;
	GrantCurrencyLocally(ECurrencyType::Coin, RewardAmount);
	UE_LOG(LogWjWorldCurrency, Log, TEXT("로컬 매치 보상 지급: %d Coin"), RewardAmount);
}

bool UWjWorldCurrencySubsystem::PurchaseItemWithCurrency(FName CosmeticItemId, ECurrencyType CurrencyType)
{
	if (bExchangePending)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("이미 교환이 진행 중입니다."));
		return false;
	}

	if (CosmeticItemId.IsNone())
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("유효하지 않은 CosmeticItemId"));
		return false;
	}

	// 카탈로그에서 아이템 정보 조회
	UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub || !CosmeticSub->GetCatalog())
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("CosmeticSubsystem 또는 카탈로그 없음"));
		return false;
	}

	const FCosmeticItemDefinition* ItemDef = CosmeticSub->GetCatalog()->FindByItemId(CosmeticItemId);
	if (!ItemDef)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("카탈로그에서 아이템 '%s' 찾을 수 없음"), *CosmeticItemId.ToString());
		return false;
	}

	// 가격 확인
	int32 ItemPrice = (CurrencyType == ECurrencyType::Coin) ? ItemDef->CoinPrice : ItemDef->GemPrice;
	if (ItemPrice <= 0)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("아이템 '%s'은 %s로 구매할 수 없습니다 (가격: %d)"),
			*CosmeticItemId.ToString(),
			(CurrencyType == ECurrencyType::Coin) ? TEXT("Coin") : TEXT("Gem"),
			ItemPrice);
		return false;
	}

	// 잔액 확인
	int32 CurrentBalance = GetBalance(CurrencyType);
	if (CurrentBalance < ItemPrice)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("잔액 부족: %s %d < %d"),
			(CurrencyType == ECurrencyType::Coin) ? TEXT("Coin") : TEXT("Gem"),
			CurrentBalance, ItemPrice);
		return false;
	}

	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		return false;
	}

	int32 CurrencyDefId = (CurrencyType == ECurrencyType::Coin) ? Settings->CoinSteamItemDefId : Settings->GemSteamItemDefId;
	int32 CosmeticDefId = ItemDef->SteamItemDefId;

	UE_LOG(LogWjWorldCurrency, Log, TEXT("재화 구매 요청: %s (%d %s → CosmeticDefId %d)"),
		*CosmeticItemId.ToString(), ItemPrice,
		(CurrencyType == ECurrencyType::Coin) ? TEXT("Coin") : TEXT("Gem"),
		CosmeticDefId);

#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (SteamInv && CosmeticDefId > 0)
	{
		// 캐시된 재화 인스턴스 ID 조회
		SteamItemInstanceID_t CurrencyInstanceId = (CurrencyType == ECurrencyType::Coin)
			? CachedCoinInstanceId
			: CachedGemInstanceId;

		if (CurrencyInstanceId == k_SteamItemInstanceIDInvalid)
		{
			UE_LOG(LogWjWorldCurrency, Warning, TEXT("재화 인스턴스 ID가 캐싱되지 않음 — 인벤토리 갱신 필요"));
			return false;
		}

		// 기존 핸들 정리
		if (ExchangeResultHandle != k_SteamInventoryResultInvalid)
		{
			SteamInv->DestroyResult(ExchangeResultHandle);
			ExchangeResultHandle = k_SteamInventoryResultInvalid;
		}

		// ExchangeItems: 재화 소비 → 코스메틱 아이템 획득
		SteamItemDef_t OutputItemDef = static_cast<SteamItemDef_t>(CosmeticDefId);
		uint32 OutputQuantity = 1;

		uint32 InputQuantity = static_cast<uint32>(ItemPrice);

		bool bExchangeResult = SteamInv->ExchangeItems(
			&ExchangeResultHandle,
			&OutputItemDef, &OutputQuantity, 1,       // 출력: 코스메틱 아이템 1개
			&CurrencyInstanceId, &InputQuantity, 1     // 입력: 재화 인스턴스 × 가격
		);

		if (bExchangeResult)
		{
			bExchangePending = true;
			bPendingIsPlacement = false;
			PendingExchangeItemId = CosmeticItemId;
			StartExchangePolling();

			UE_LOG(LogWjWorldCurrency, Log, TEXT("Steam ExchangeItems 요청 성공: %s (CurrencyInstanceId=%llu, Price=%d)"),
				*CosmeticItemId.ToString(), CurrencyInstanceId, ItemPrice);
			return true;
		}
		else
		{
			UE_LOG(LogWjWorldCurrency, Warning, TEXT("Steam ExchangeItems 호출 실패: %s"), *CosmeticItemId.ToString());
			ExchangeResultHandle = k_SteamInventoryResultInvalid;
			return false;
		}
	}
#endif

	// 비Steam 폴백: 로컬 잔액 차감 + 아이템 부여
	SetBalance(CurrencyType, CurrentBalance - ItemPrice);
	CosmeticSub->GrantItemLocally(CosmeticItemId, 1);

	OnCurrencyPurchaseComplete.Broadcast(CosmeticItemId, true);

	UE_LOG(LogWjWorldCurrency, Log, TEXT("로컬 재화 구매 완료: %s (%d %s 차감)"),
		*CosmeticItemId.ToString(), ItemPrice,
		(CurrencyType == ECurrencyType::Coin) ? TEXT("Coin") : TEXT("Gem"));

	return true;
}

bool UWjWorldCurrencySubsystem::PurchasePlacementObject(FName ObjectId)
{
	if (bExchangePending)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("이미 교환이 진행 중입니다."));
		return false;
	}

	if (ObjectId.IsNone())
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("유효하지 않은 ObjectId"));
		return false;
	}

	// DeveloperSettings에서 LobbyPlaceableCatalog 로드
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		return false;
	}

	UWjWorldPlaceableObjectDataAsset* PlacementCatalog = Settings->GetPlaceableCatalogForContext(EPlacementContext::Lobby);
	if (!PlacementCatalog)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("LobbyPlaceableCatalog 없음"));
		return false;
	}

	const FPlaceableObjectDefinition* Def = PlacementCatalog->FindByObjectId(ObjectId);
	if (!Def)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("카탈로그에서 오브젝트 '%s' 찾을 수 없음"), *ObjectId.ToString());
		return false;
	}

	if (Def->CoinPrice <= 0)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("오브젝트 '%s'은 무료입니다 (구매 불필요)"), *ObjectId.ToString());
		return false;
	}

	if (Def->SteamItemDefId <= 0)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("오브젝트 '%s'에 SteamItemDefId가 설정되지 않음"), *ObjectId.ToString());
		return false;
	}

	// 잔액 확인
	int32 CurrentBalance = GetBalance(ECurrencyType::Coin);
	if (CurrentBalance < Def->CoinPrice)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Coin 잔액 부족: %d < %d"), CurrentBalance, Def->CoinPrice);
		return false;
	}

	UE_LOG(LogWjWorldCurrency, Log, TEXT("배치 오브젝트 구매 요청: %s (%d Coin → DefId %d)"),
		*ObjectId.ToString(), Def->CoinPrice, Def->SteamItemDefId);

#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (SteamInv)
	{
		SteamItemInstanceID_t CoinInstanceId = CachedCoinInstanceId;
		if (CoinInstanceId == k_SteamItemInstanceIDInvalid)
		{
			UE_LOG(LogWjWorldCurrency, Warning, TEXT("Coin 인스턴스 ID가 캐싱되지 않음 — 인벤토리 갱신 필요"));
			return false;
		}

		// 기존 핸들 정리
		if (ExchangeResultHandle != k_SteamInventoryResultInvalid)
		{
			SteamInv->DestroyResult(ExchangeResultHandle);
			ExchangeResultHandle = k_SteamInventoryResultInvalid;
		}

		// ExchangeItems: Coin 소비 → 배치 오브젝트 아이템 획득
		SteamItemDef_t OutputItemDef = static_cast<SteamItemDef_t>(Def->SteamItemDefId);
		uint32 OutputQuantity = 1;
		uint32 InputQuantity = static_cast<uint32>(Def->CoinPrice);

		bool bExchangeResult = SteamInv->ExchangeItems(
			&ExchangeResultHandle,
			&OutputItemDef, &OutputQuantity, 1,
			&CoinInstanceId, &InputQuantity, 1
		);

		if (bExchangeResult)
		{
			bExchangePending = true;
			bPendingIsPlacement = true;
			PendingExchangeItemId = ObjectId;
			StartExchangePolling();

			UE_LOG(LogWjWorldCurrency, Log, TEXT("Steam ExchangeItems 요청 성공 (배치): %s (CoinInstanceId=%llu, Price=%d)"),
				*ObjectId.ToString(), CoinInstanceId, Def->CoinPrice);
			return true;
		}
		else
		{
			UE_LOG(LogWjWorldCurrency, Warning, TEXT("Steam ExchangeItems 호출 실패 (배치): %s"), *ObjectId.ToString());
			ExchangeResultHandle = k_SteamInventoryResultInvalid;
			return false;
		}
	}
#endif

	// 비Steam 폴백: 로컬 잔액 차감 + 로컬 소유 수량 증가
	SetBalance(ECurrencyType::Coin, CurrentBalance - Def->CoinPrice);

	// GConfig에 소유 수량 기록
	static const FString PlacementInventorySection = TEXT("PlacementInventory");
	int32 CurrentOwned = 0;
	GConfig->GetInt(*PlacementInventorySection, *ObjectId.ToString(), CurrentOwned, GGameUserSettingsIni);
	GConfig->SetInt(*PlacementInventorySection, *ObjectId.ToString(), CurrentOwned + 1, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);

	// CosmeticSubsystem의 AllItemQuantities도 갱신 (비Steam)
	UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (CosmeticSub)
	{
		CosmeticSub->AllItemQuantities.FindOrAdd(Def->SteamItemDefId) += 1;
	}

	OnPlacementPurchaseComplete.Broadcast(ObjectId, true);

	UE_LOG(LogWjWorldCurrency, Log, TEXT("로컬 배치 오브젝트 구매 완료: %s (%d Coin 차감)"),
		*ObjectId.ToString(), Def->CoinPrice);

	return true;
}

bool UWjWorldCurrencySubsystem::PurchaseGemPack(int32 PackDefId)
{
	if (bGemPurchasePending)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("이미 Gem 팩 구매가 진행 중입니다."));
		return false;
	}

#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Steam Inventory API 사용 불가"));
		return false;
	}

	// 기존 핸들 정리
	if (GemPurchaseResultHandle != k_SteamInventoryResultInvalid)
	{
		SteamInv->DestroyResult(GemPurchaseResultHandle);
		GemPurchaseResultHandle = k_SteamInventoryResultInvalid;
	}

	SteamItemDef_t ItemDef = static_cast<SteamItemDef_t>(PackDefId);
	uint32 Quantity = 1;

	if (SteamInv->StartPurchase(&ItemDef, &Quantity, 1))
	{
		bGemPurchasePending = true;
		StartGemPurchasePolling();
		UE_LOG(LogWjWorldCurrency, Log, TEXT("Steam Gem 팩 구매 요청 시작 (DefId: %d)"), PackDefId);
		return true;
	}
	else
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Steam StartPurchase 실패 (DefId: %d)"), PackDefId);
		return false;
	}
#else
	UE_LOG(LogWjWorldCurrency, Warning, TEXT("Gem 팩 구매는 Steam 환경에서만 가능합니다."));
	return false;
#endif
}

void UWjWorldCurrencySubsystem::RefreshBalancesFromInventory()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		return;
	}

#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (SteamInv)
	{
		// 단일 GetAllItems 호출로 잔액 + 인스턴스 ID 동시 캐싱
		SteamInventoryResult_t ResultHandle = k_SteamInventoryResultInvalid;
		if (!SteamInv->GetAllItems(&ResultHandle))
		{
			UE_LOG(LogWjWorldCurrency, Warning, TEXT("Steam GetAllItems 실패"));
			return;
		}

		int32 NewCoinBalance = 0;
		int32 NewGemBalance = 0;
		SteamItemInstanceID_t NewCoinInstanceId = k_SteamItemInstanceIDInvalid;
		SteamItemInstanceID_t NewGemInstanceId = k_SteamItemInstanceIDInvalid;

		EResult Status = SteamInv->GetResultStatus(ResultHandle);
		if (Status == k_EResultOK)
		{
			uint32 ItemCount = 0;
			if (SteamInv->GetResultItems(ResultHandle, nullptr, &ItemCount) && ItemCount > 0)
			{
				TArray<SteamItemDetails_t> ItemDetails;
				ItemDetails.SetNum(ItemCount);

				if (SteamInv->GetResultItems(ResultHandle, ItemDetails.GetData(), &ItemCount))
				{
					for (uint32 i = 0; i < ItemCount; ++i)
					{
						const SteamItemDetails_t& Detail = ItemDetails[i];
						if (Detail.m_unQuantity == 0)
						{
							continue;
						}

						if (Detail.m_iDefinition == Settings->CoinSteamItemDefId)
						{
							NewCoinBalance += Detail.m_unQuantity;
							NewCoinInstanceId = Detail.m_itemId;
						}
						else if (Detail.m_iDefinition == Settings->GemSteamItemDefId)
						{
							NewGemBalance += Detail.m_unQuantity;
							NewGemInstanceId = Detail.m_itemId;
						}
					}
				}
			}
		}

		SteamInv->DestroyResult(ResultHandle);

		// 인스턴스 ID 캐시 갱신
		CachedCoinInstanceId = NewCoinInstanceId;
		CachedGemInstanceId = NewGemInstanceId;

		// 잔액 갱신
		if (NewCoinBalance != CoinBalance)
		{
			SetBalance(ECurrencyType::Coin, NewCoinBalance);
		}
		if (NewGemBalance != GemBalance)
		{
			SetBalance(ECurrencyType::Gem, NewGemBalance);
		}

		UE_LOG(LogWjWorldCurrency, Log, TEXT("Steam 잔액 갱신: Coin=%d (InstanceId=%llu), Gem=%d (InstanceId=%llu)"),
			CoinBalance, CachedCoinInstanceId, GemBalance, CachedGemInstanceId);
		return;
	}
#endif

	// 비Steam: 로컬 값 유지
	UE_LOG(LogWjWorldCurrency, Log, TEXT("로컬 잔액: Coin=%d, Gem=%d"), CoinBalance, GemBalance);
}

void UWjWorldCurrencySubsystem::GrantCurrencyLocally(ECurrencyType Type, int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	int32 CurrentBalance = GetBalance(Type);
	SetBalance(Type, CurrentBalance + Amount);

	UE_LOG(LogWjWorldCurrency, Log, TEXT("로컬 재화 부여: %s +%d (잔액: %d)"),
		(Type == ECurrencyType::Coin) ? TEXT("Coin") : TEXT("Gem"),
		Amount, GetBalance(Type));
}

void UWjWorldCurrencySubsystem::SetCurrencyLocally(ECurrencyType Type, int32 Amount)
{
	SetBalance(Type, Amount);

	UE_LOG(LogWjWorldCurrency, Log, TEXT("로컬 재화 설정: %s = %d"),
		(Type == ECurrencyType::Coin) ? TEXT("Coin") : TEXT("Gem"),
		GetBalance(Type));
}

void UWjWorldCurrencySubsystem::DebugPrintBalances() const
{
	UE_LOG(LogWjWorldCurrency, Log, TEXT("========== 재화 잔액 =========="));
	UE_LOG(LogWjWorldCurrency, Log, TEXT("  Coin: %d"), CoinBalance);
	UE_LOG(LogWjWorldCurrency, Log, TEXT("  Gem:  %d"), GemBalance);
	UE_LOG(LogWjWorldCurrency, Log, TEXT("================================"));
}

void UWjWorldCurrencySubsystem::DebugConsumeAllSteamCurrency()
{
#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("DebugConsumeAllSteamCurrency: Steam Inventory API 없음"));
		return;
	}

	// 캐시된 인스턴스 ID로 소비
	if (CachedCoinInstanceId != k_SteamItemInstanceIDInvalid && CoinBalance > 0)
	{
		SteamInventoryResult_t ResultHandle = k_SteamInventoryResultInvalid;
		if (SteamInv->ConsumeItem(&ResultHandle, CachedCoinInstanceId, static_cast<uint32>(CoinBalance)))
		{
			UE_LOG(LogWjWorldCurrency, Log, TEXT("Steam Coin 소비 요청 성공: %d개 (InstanceId=%llu)"), CoinBalance, CachedCoinInstanceId);
			SteamInv->DestroyResult(ResultHandle);
		}
		else
		{
			UE_LOG(LogWjWorldCurrency, Warning, TEXT("Steam Coin 소비 요청 실패"));
		}
	}

	if (CachedGemInstanceId != k_SteamItemInstanceIDInvalid && GemBalance > 0)
	{
		SteamInventoryResult_t ResultHandle = k_SteamInventoryResultInvalid;
		if (SteamInv->ConsumeItem(&ResultHandle, CachedGemInstanceId, static_cast<uint32>(GemBalance)))
		{
			UE_LOG(LogWjWorldCurrency, Log, TEXT("Steam Gem 소비 요청 성공: %d개 (InstanceId=%llu)"), GemBalance, CachedGemInstanceId);
			SteamInv->DestroyResult(ResultHandle);
		}
		else
		{
			UE_LOG(LogWjWorldCurrency, Warning, TEXT("Steam Gem 소비 요청 실패"));
		}
	}

	// 인벤토리 갱신으로 잔액 반영
	UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (CosmeticSub)
	{
		CosmeticSub->RequestInventoryRefresh();
	}
#else
	UE_LOG(LogWjWorldCurrency, Warning, TEXT("DebugConsumeAllSteamCurrency: Steam 환경에서만 사용 가능"));
#endif
}

void UWjWorldCurrencySubsystem::DebugConsumeAllSteamItems()
{
#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("DebugConsumeAllSteamItems: Steam Inventory API 없음"));
		return;
	}

	SteamInventoryResult_t AllItemsHandle = k_SteamInventoryResultInvalid;
	if (!SteamInv->GetAllItems(&AllItemsHandle))
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("DebugConsumeAllSteamItems: GetAllItems 실패"));
		return;
	}

	EResult Status = SteamInv->GetResultStatus(AllItemsHandle);
	if (Status != k_EResultOK)
	{
		SteamInv->DestroyResult(AllItemsHandle);
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("DebugConsumeAllSteamItems: GetAllItems 상태 실패 (%d)"), static_cast<int32>(Status));
		return;
	}

	uint32 ItemCount = 0;
	if (!SteamInv->GetResultItems(AllItemsHandle, nullptr, &ItemCount) || ItemCount == 0)
	{
		SteamInv->DestroyResult(AllItemsHandle);
		UE_LOG(LogWjWorldCurrency, Log, TEXT("DebugConsumeAllSteamItems: 인벤토리가 이미 비어있음"));
		return;
	}

	TArray<SteamItemDetails_t> Items;
	Items.SetNum(ItemCount);
	SteamInv->GetResultItems(AllItemsHandle, Items.GetData(), &ItemCount);
	SteamInv->DestroyResult(AllItemsHandle);

	int32 ConsumedCount = 0;
	for (const SteamItemDetails_t& Detail : Items)
	{
		if (Detail.m_unQuantity == 0)
		{
			continue;
		}

		SteamInventoryResult_t ConsumeHandle = k_SteamInventoryResultInvalid;
		if (SteamInv->ConsumeItem(&ConsumeHandle, Detail.m_itemId, Detail.m_unQuantity))
		{
			UE_LOG(LogWjWorldCurrency, Log, TEXT("  소비: DefId=%d, Quantity=%d, InstanceId=%llu"),
				Detail.m_iDefinition, Detail.m_unQuantity, Detail.m_itemId);
			SteamInv->DestroyResult(ConsumeHandle);
			++ConsumedCount;
		}
	}

	// 캐시 초기화
	CachedCoinInstanceId = k_SteamItemInstanceIDInvalid;
	CachedGemInstanceId = k_SteamItemInstanceIDInvalid;

	// 인벤토리 갱신
	UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (CosmeticSub)
	{
		CosmeticSub->RequestInventoryRefresh();
	}

	UE_LOG(LogWjWorldCurrency, Log, TEXT("DebugConsumeAllSteamItems: %d/%d 아이템 소비 완료"), ConsumedCount, ItemCount);
#else
	UE_LOG(LogWjWorldCurrency, Warning, TEXT("DebugConsumeAllSteamItems: Steam 환경에서만 사용 가능"));
#endif
}

void UWjWorldCurrencySubsystem::HandleInventoryUpdated()
{
	RefreshBalancesFromInventory();
}

void UWjWorldCurrencySubsystem::PollExchangeResult()
{
#if WITH_STEAM
	if (!bExchangePending || ExchangeResultHandle == k_SteamInventoryResultInvalid)
	{
		StopExchangePolling();
		return;
	}

	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv)
	{
		StopExchangePolling();
		bExchangePending = false;
		if (bPendingIsPlacement)
		{
			OnPlacementPurchaseComplete.Broadcast(PendingExchangeItemId, false);
		}
		else
		{
			OnCurrencyPurchaseComplete.Broadcast(PendingExchangeItemId, false);
		}
		bPendingIsPlacement = false;
		PendingExchangeItemId = NAME_None;
		return;
	}

	EResult Status = SteamInv->GetResultStatus(ExchangeResultHandle);
	if (Status == k_EResultPending)
	{
		return;
	}

	bool bSuccess = (Status == k_EResultOK);
	if (bSuccess)
	{
		UE_LOG(LogWjWorldCurrency, Log, TEXT("Steam Exchange 성공: %s"), *PendingExchangeItemId.ToString());

		// 인벤토리 갱신
		UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>();
		if (CosmeticSub)
		{
			CosmeticSub->RequestInventoryRefresh();
		}
	}
	else
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Steam Exchange 실패: %s (Status: %d)"),
			*PendingExchangeItemId.ToString(), static_cast<int32>(Status));
	}

	SteamInv->DestroyResult(ExchangeResultHandle);
	ExchangeResultHandle = k_SteamInventoryResultInvalid;

	bExchangePending = false;

	// 배치 vs 코스메틱 분기
	if (bPendingIsPlacement)
	{
		OnPlacementPurchaseComplete.Broadcast(PendingExchangeItemId, bSuccess);
	}
	else
	{
		OnCurrencyPurchaseComplete.Broadcast(PendingExchangeItemId, bSuccess);
	}
	bPendingIsPlacement = false;
	PendingExchangeItemId = NAME_None;

	StopExchangePolling();
#else
	StopExchangePolling();
#endif
}

void UWjWorldCurrencySubsystem::StartExchangePolling()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		ExchangePollTimerHandle,
		this,
		&UWjWorldCurrencySubsystem::PollExchangeResult,
		0.5f,
		true
	);
}

void UWjWorldCurrencySubsystem::StopExchangePolling()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(ExchangePollTimerHandle);
	}
}

void UWjWorldCurrencySubsystem::PollGemPurchaseResult()
{
#if WITH_STEAM
	if (!bGemPurchasePending)
	{
		StopGemPurchasePolling();
		return;
	}

	// Gem 팩 구매는 Steam 오버레이를 통해 처리되므로
	// 인벤토리 변경을 감지하여 완료 판단
	// CosmeticSubsystem의 OnInventoryUpdated → HandleInventoryUpdated → RefreshBalancesFromInventory
	// 에서 Gem 잔액 변경이 감지되면 구매 완료로 판단

	// 간단한 타임아웃 처리 (300초)
	UWorld* World = GetWorld();
	if (World)
	{
		if (GemPurchaseStartTime == 0.0f)
		{
			GemPurchaseStartTime = World->GetTimeSeconds();
		}

		float ElapsedTime = World->GetTimeSeconds() - GemPurchaseStartTime;
		if (ElapsedTime > 300.0f)
		{
			UE_LOG(LogWjWorldCurrency, Warning, TEXT("Gem 팩 구매 타임아웃"));
			bGemPurchasePending = false;
			GemPurchaseStartTime = 0.0f;
			StopGemPurchasePolling();
		}
	}
#else
	StopGemPurchasePolling();
#endif
}

void UWjWorldCurrencySubsystem::StartGemPurchasePolling()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		GemPurchasePollTimerHandle,
		this,
		&UWjWorldCurrencySubsystem::PollGemPurchaseResult,
		1.0f,
		true
	);
}

void UWjWorldCurrencySubsystem::StopGemPurchasePolling()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(GemPurchasePollTimerHandle);
	}
}

void UWjWorldCurrencySubsystem::SaveBalancesToLocal()
{
	GConfig->SetInt(*CurrencyConfigSection, TEXT("CoinBalance"), CoinBalance, GGameUserSettingsIni);
	GConfig->SetInt(*CurrencyConfigSection, TEXT("GemBalance"), GemBalance, GGameUserSettingsIni);

	GConfig->Flush(false, GGameUserSettingsIni);
	UE_LOG(LogWjWorldCurrency, Log, TEXT("재화 잔액 로컬 저장 완료"));
}

void UWjWorldCurrencySubsystem::LoadBalancesFromLocal()
{
	int32 LoadedCoin = 0;
	int32 LoadedGem = 0;

	GConfig->GetInt(*CurrencyConfigSection, TEXT("CoinBalance"), LoadedCoin, GGameUserSettingsIni);
	GConfig->GetInt(*CurrencyConfigSection, TEXT("GemBalance"), LoadedGem, GGameUserSettingsIni);

	CoinBalance = FMath::Max(0, LoadedCoin);
	GemBalance = FMath::Max(0, LoadedGem);

	UE_LOG(LogWjWorldCurrency, Log, TEXT("재화 잔액 로컬 로드 완료 (Coin: %d, Gem: %d)"), CoinBalance, GemBalance);
}

void UWjWorldCurrencySubsystem::SetBalance(ECurrencyType Type, int32 NewAmount)
{
	NewAmount = FMath::Max(0, NewAmount);

	switch (Type)
	{
	case ECurrencyType::Coin:
		if (CoinBalance != NewAmount)
		{
			CoinBalance = NewAmount;
			OnCurrencyBalanceChanged.Broadcast(ECurrencyType::Coin, CoinBalance);
		}
		break;
	case ECurrencyType::Gem:
		if (GemBalance != NewAmount)
		{
			GemBalance = NewAmount;
			OnCurrencyBalanceChanged.Broadcast(ECurrencyType::Gem, GemBalance);
		}
		break;
	}
}

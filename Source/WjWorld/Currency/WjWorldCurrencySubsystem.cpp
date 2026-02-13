// Fill out your copyright notice in the Description page of Project Settings.

#include "Currency/WjWorldCurrencySubsystem.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Cosmetic/WjWorldCosmeticDataAsset.h"
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
		// 기존 핸들 정리
		if (ExchangeResultHandle != k_SteamInventoryResultInvalid)
		{
			SteamInv->DestroyResult(ExchangeResultHandle);
			ExchangeResultHandle = k_SteamInventoryResultInvalid;
		}

		// ExchangeItems: 재화 소비 → 코스메틱 아이템 획득
		SteamItemDef_t OutputItemDef = static_cast<SteamItemDef_t>(CosmeticDefId);
		uint32 OutputQuantity = 1;

		// 입력: 재화 아이템 (인벤토리에서 해당 수량 소비)
		// ExchangeItems는 인스턴스 ID가 필요하므로, 전체 인벤토리에서 재화 인스턴스를 찾아야 함
		// 여기서는 간단화를 위해 재화가 auto_stack이므로 단일 인스턴스라 가정
		// 실제 구현시에는 GetAllItems 결과에서 인스턴스 ID를 캐싱해야 함

		// Steam ExchangeItems는 SteamItemInstanceID_t가 필요
		// exchange 레시피는 서버 사이드에서 itemdef의 "exchange" 프로퍼티로 검증
		// 따라서 클라이언트에서는 재화 인스턴스 ID만 전달하면 됨

		// 재화 인스턴스 ID를 찾기 위해 인벤토리 조회 필요
		// CosmeticSubsystem의 캐시된 결과에서는 인스턴스 ID를 보관하지 않으므로
		// 별도의 GetAllItems 호출 후 처리해야 함
		// 이 부분은 추후 인스턴스 ID 캐싱이 구현되면 개선

		UE_LOG(LogWjWorldCurrency, Log, TEXT("Steam ExchangeItems는 인스턴스 ID 기반 - 인벤토리 갱신 후 재시도 필요"));

		// 현재는 로컬 차감 + CosmeticSubsystem으로 아이템 부여하는 방식으로 처리
		// Steam 서버 사이드 exchange는 추후 인스턴스 ID 캐싱 구현 시 활성화
		SetBalance(CurrencyType, CurrentBalance - ItemPrice);
		CosmeticSub->GrantItemLocally(CosmeticItemId, 1);
		CosmeticSub->RequestInventoryRefresh();

		OnCurrencyPurchaseComplete.Broadcast(CosmeticItemId, true);
		return true;
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
		// CosmeticSubsystem의 인벤토리 갱신을 구독하고 있으므로
		// 여기서는 캐시된 인벤토리에서 재화 수량을 파싱
		int32 NewCoinBalance = GetItemQuantityFromInventory(Settings->CoinSteamItemDefId);
		int32 NewGemBalance = GetItemQuantityFromInventory(Settings->GemSteamItemDefId);

		if (NewCoinBalance != CoinBalance)
		{
			SetBalance(ECurrencyType::Coin, NewCoinBalance);
		}
		if (NewGemBalance != GemBalance)
		{
			SetBalance(ECurrencyType::Gem, NewGemBalance);
		}

		UE_LOG(LogWjWorldCurrency, Log, TEXT("Steam 잔액 갱신: Coin=%d, Gem=%d"), CoinBalance, GemBalance);
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

void UWjWorldCurrencySubsystem::HandleInventoryUpdated()
{
	RefreshBalancesFromInventory();
}

int32 UWjWorldCurrencySubsystem::GetItemQuantityFromInventory(int32 SteamItemDefId) const
{
	// CosmeticSubsystem의 캐시된 인벤토리에서 해당 DefId의 수량을 찾음
	// 재화 아이템은 CosmeticCatalog에 등록되어 있지 않으므로
	// CosmeticSubsystem의 파싱에서 "알 수 없는 SteamItemDefId"로 스킵됨
	// 따라서 별도의 Steam Inventory 조회가 필요

	// 현재는 CosmeticSubsystem의 인벤토리 갱신 시점에 호출되므로
	// 직접 Steam API를 호출하여 확인
#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv)
	{
		return 0;
	}

	SteamInventoryResult_t ResultHandle = k_SteamInventoryResultInvalid;
	if (!SteamInv->GetAllItems(&ResultHandle))
	{
		return 0;
	}

	// 결과 대기 (동기적 폴링 - 이미 캐시된 경우 즉시 반환)
	// 참고: GetAllItems는 이전에 성공한 결과가 캐시되어 있으면 즉시 반환됨
	int32 TotalQuantity = 0;

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
					if (ItemDetails[i].m_iDefinition == SteamItemDefId && ItemDetails[i].m_unQuantity > 0)
					{
						TotalQuantity += ItemDetails[i].m_unQuantity;
					}
				}
			}
		}
	}

	SteamInv->DestroyResult(ResultHandle);
	return TotalQuantity;
#else
	return 0;
#endif
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
		OnCurrencyPurchaseComplete.Broadcast(PendingExchangeItemId, false);
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
	OnCurrencyPurchaseComplete.Broadcast(PendingExchangeItemId, bSuccess);
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
	static float GemPurchaseStartTime = 0.0f;
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
	FString ConfigFilePath = FPaths::GeneratedConfigDir() + TEXT("CurrencyBalance.ini");

	GConfig->SetInt(*CurrencyConfigSection, TEXT("CoinBalance"), CoinBalance, ConfigFilePath);
	GConfig->SetInt(*CurrencyConfigSection, TEXT("GemBalance"), GemBalance, ConfigFilePath);

	GConfig->Flush(false, ConfigFilePath);
	UE_LOG(LogWjWorldCurrency, Log, TEXT("재화 잔액 로컬 저장 완료"));
}

void UWjWorldCurrencySubsystem::LoadBalancesFromLocal()
{
	FString ConfigFilePath = FPaths::GeneratedConfigDir() + TEXT("CurrencyBalance.ini");

	int32 LoadedCoin = 0;
	int32 LoadedGem = 0;

	GConfig->GetInt(*CurrencyConfigSection, TEXT("CoinBalance"), LoadedCoin, ConfigFilePath);
	GConfig->GetInt(*CurrencyConfigSection, TEXT("GemBalance"), LoadedGem, ConfigFilePath);

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

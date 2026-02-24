// Fill out your copyright notice in the Description page of Project Settings.

#include "WjWorldPlayerControllerBase.h"
#include "WjWorldCharacterBase.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Cosmetic/WjWorldCosmeticTypes.h"
#include "Currency/WjWorldCurrencySubsystem.h"
#include "GamePlay/TreasureChest/WjWorldTreasureChestActor.h"
#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"
#include "GamePlay/Placement/WjWorldPlacementTypes.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "EngineUtils.h"
#include "WjWorldLogCategories.h"

AWjWorldPlayerControllerBase::AWjWorldPlayerControllerBase()
{
	
}

void AWjWorldPlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeController();
	InitializeUI();
}

void AWjWorldPlayerControllerBase::InitializeController()
{
	// Base implementation - override in derived classes
}

void AWjWorldPlayerControllerBase::InitializeUI()
{
	// Base implementation - override in derived classes
}

void AWjWorldPlayerControllerBase::CheckInputMode()
{
#if UE_ENABLE_DEBUG_DRAWING
	FString InputModeString = GetCurrentInputModeDebugString();
	UE_LOG(LogWjWorld, Log, TEXT("Current Input Mode: %s"), *InputModeString);
#endif
}

void AWjWorldPlayerControllerBase::ChangeCharacterViewMode(int32 InViewMode)
{
	AWjWorldCharacterBase* WjCharacter = Cast<AWjWorldCharacterBase>(GetPawn());
	if (WjCharacter == nullptr)
	{
		return;
	}

	switch (InViewMode)
	{
	case 0:
	{
		WjCharacter->SetCharacterViewMode(ECharacterCameraMode::TopDown);
		break;
	}
	case 1:
	{
		WjCharacter->SetCharacterViewMode(ECharacterCameraMode::ThirdPerson);
		break;
	}
	case 2:
	{
		WjCharacter->SetCharacterViewMode(ECharacterCameraMode::FirstPerson);
		break;
	}
	}
}

void AWjWorldPlayerControllerBase::ServerTravelWaitingRoom()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	GetWorld()->ServerTravel(Settings->GetWaitingRoomServerTravelURL());
}

void AWjWorldPlayerControllerBase::Cosmetic_GrantItem(FString ItemId)
{
#if !UE_BUILD_SHIPPING
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Cosmetic_GrantItem: CosmeticSubsystem 없음"));
		return;
	}

	FName ItemFName(*ItemId);
	CosmeticSub->GenerateTestItem(ItemFName);
	UE_LOG(LogWjWorld, Log, TEXT("Cosmetic_GrantItem: %s 부여 완료"), *ItemId);
#endif
}

void AWjWorldPlayerControllerBase::Cosmetic_GrantAll()
{
#if !UE_BUILD_SHIPPING
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Cosmetic_GrantAll: CosmeticSubsystem 없음"));
		return;
	}

	CosmeticSub->GrantAllItemsLocally();
	UE_LOG(LogWjWorld, Log, TEXT("Cosmetic_GrantAll: 모든 아이템 부여 완료"));
#endif
}

void AWjWorldPlayerControllerBase::Cosmetic_ClearInventory()
{
#if !UE_BUILD_SHIPPING
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Cosmetic_ClearInventory: CosmeticSubsystem 없음"));
		return;
	}

	CosmeticSub->ClearLocalInventory();
	UE_LOG(LogWjWorld, Log, TEXT("Cosmetic_ClearInventory: 인벤토리 초기화 완료"));
#endif
}

void AWjWorldPlayerControllerBase::Cosmetic_PrintInventory()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Cosmetic_PrintInventory: CosmeticSubsystem 없음"));
		return;
	}

	CosmeticSub->DebugPrintInventory();
}

void AWjWorldPlayerControllerBase::Cosmetic_PrintLoadout()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Cosmetic_PrintLoadout: CosmeticSubsystem 없음"));
		return;
	}

	CosmeticSub->DebugPrintLoadout();
}

void AWjWorldPlayerControllerBase::Cosmetic_Equip(int32 SlotIndex, FString ItemId)
{
#if !UE_BUILD_SHIPPING
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Cosmetic_Equip: CosmeticSubsystem 없음"));
		return;
	}

	ECosmeticSlot Slot = static_cast<ECosmeticSlot>(SlotIndex);
	if (Slot == ECosmeticSlot::None || SlotIndex < 1 || SlotIndex > 4)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Cosmetic_Equip: 유효하지 않은 슬롯 (1=Head, 2=Body, 3=Back, 4=Effect)"));
		return;
	}

	FName ItemFName(*ItemId);
	if (CosmeticSub->EquipItem(Slot, ItemFName))
	{
		CosmeticSub->SaveLoadoutToLocal();
		UE_LOG(LogWjWorld, Log, TEXT("Cosmetic_Equip: 슬롯 %d에 %s 장착 완료"), SlotIndex, *ItemId);
	}
#endif
}

void AWjWorldPlayerControllerBase::Cosmetic_Unequip(int32 SlotIndex)
{
#if !UE_BUILD_SHIPPING
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Cosmetic_Unequip: CosmeticSubsystem 없음"));
		return;
	}

	ECosmeticSlot Slot = static_cast<ECosmeticSlot>(SlotIndex);
	if (Slot == ECosmeticSlot::None || SlotIndex < 1 || SlotIndex > 4)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Cosmetic_Unequip: 유효하지 않은 슬롯 (1=Head, 2=Body, 3=Back, 4=Effect)"));
		return;
	}

	CosmeticSub->UnequipSlot(Slot);
	CosmeticSub->SaveLoadoutToLocal();
	UE_LOG(LogWjWorld, Log, TEXT("Cosmetic_Unequip: 슬롯 %d 해제 완료"), SlotIndex);
#endif
}

void AWjWorldPlayerControllerBase::Cosmetic_RefreshInventory()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Cosmetic_RefreshInventory: CosmeticSubsystem 없음"));
		return;
	}

	CosmeticSub->RequestInventoryRefresh();
	UE_LOG(LogWjWorld, Log, TEXT("Cosmetic_RefreshInventory: 인벤토리 갱신 요청"));
}

void AWjWorldPlayerControllerBase::Cosmetic_AddPromo(int32 SteamItemDefId)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Cosmetic_AddPromo: CosmeticSubsystem 없음"));
		return;
	}

	if (CosmeticSub->AddPromoItem(SteamItemDefId))
	{
		UE_LOG(LogWjWorld, Log, TEXT("Cosmetic_AddPromo: SteamItemDefId %d 프로모 아이템 지급 요청 완료"), SteamItemDefId);
	}
	else
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Cosmetic_AddPromo: 프로모 아이템 지급 실패 (DefId: %d)"), SteamItemDefId);
	}
}

void AWjWorldPlayerControllerBase::Cosmetic_AddAllPromos()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Cosmetic_AddAllPromos: CosmeticSubsystem 없음"));
		return;
	}

	if (CosmeticSub->AddAllPromoItems())
	{
		UE_LOG(LogWjWorld, Log, TEXT("Cosmetic_AddAllPromos: 모든 프로모 아이템 지급 요청 완료"));
	}
	else
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Cosmetic_AddAllPromos: 프로모 아이템 지급 실패"));
	}
}

void AWjWorldPlayerControllerBase::Cosmetic_OpenShop()
{
	// 로비 HUD에서 코스메틱 상점을 여는 로직
	// 실제 구현은 HUD 클래스에서 처리
	UE_LOG(LogWjWorld, Log, TEXT("Cosmetic_OpenShop: 콘솔에서 상점 열기는 로비에서만 가능합니다. LobbyHUD의 코스메틱 버튼을 사용하세요."));
}

// ---- 재화 테스트 콘솔 명령어 ----

void AWjWorldPlayerControllerBase::Currency_GrantCoin(int32 Amount)
{
#if !UE_BUILD_SHIPPING
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
	if (!CurrencySub)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Currency_GrantCoin: CurrencySubsystem 없음"));
		return;
	}

	CurrencySub->GrantCurrencyLocally(ECurrencyType::Coin, Amount);
#endif
}

void AWjWorldPlayerControllerBase::Currency_GrantGem(int32 Amount)
{
#if !UE_BUILD_SHIPPING
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
	if (!CurrencySub)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Currency_GrantGem: CurrencySubsystem 없음"));
		return;
	}

	CurrencySub->GrantCurrencyLocally(ECurrencyType::Gem, Amount);
#endif
}

void AWjWorldPlayerControllerBase::Currency_SetCoin(int32 Amount)
{
#if !UE_BUILD_SHIPPING
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
	if (!CurrencySub)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Currency_SetCoin: CurrencySubsystem 없음"));
		return;
	}

	CurrencySub->SetCurrencyLocally(ECurrencyType::Coin, Amount);
#endif
}

void AWjWorldPlayerControllerBase::Currency_SetGem(int32 Amount)
{
#if !UE_BUILD_SHIPPING
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
	if (!CurrencySub)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Currency_SetGem: CurrencySubsystem 없음"));
		return;
	}

	CurrencySub->SetCurrencyLocally(ECurrencyType::Gem, Amount);
#endif
}

void AWjWorldPlayerControllerBase::Currency_BuyGemPack(int32 PackDefId)
{
#if !UE_BUILD_SHIPPING
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
	if (!CurrencySub)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Currency_BuyGemPack: CurrencySubsystem 없음"));
		return;
	}

	if (CurrencySub->PurchaseGemPack(PackDefId))
	{
		UE_LOG(LogWjWorldCurrency, Log, TEXT("Currency_BuyGemPack: PackDefId %d 구매 요청 완료"), PackDefId);
	}
	else
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Currency_BuyGemPack: PackDefId %d 구매 요청 실패"), PackDefId);
	}
#endif
}

void AWjWorldPlayerControllerBase::Currency_SimulateReward(int32 bWin)
{
#if !UE_BUILD_SHIPPING
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
	if (!CurrencySub)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Currency_SimulateReward: CurrencySubsystem 없음"));
		return;
	}

	CurrencySub->TriggerMatchReward(bWin != 0);
#endif
}

void AWjWorldPlayerControllerBase::Currency_Print()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
	if (!CurrencySub)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Currency_Print: CurrencySubsystem 없음"));
		return;
	}

	CurrencySub->DebugPrintBalances();
}

void AWjWorldPlayerControllerBase::Currency_Refresh()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
	if (!CurrencySub)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Currency_Refresh: CurrencySubsystem 없음"));
		return;
	}

	CurrencySub->RefreshBalancesFromInventory();
	UE_LOG(LogWjWorldCurrency, Log, TEXT("Currency_Refresh: 잔액 갱신 요청"));
}

void AWjWorldPlayerControllerBase::Steam_ConsumeCurrency()
{
#if !UE_BUILD_SHIPPING
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
	if (!CurrencySub)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Steam_ConsumeCurrency: CurrencySubsystem 없음"));
		return;
	}

	CurrencySub->DebugConsumeAllSteamCurrency();
#endif
}

void AWjWorldPlayerControllerBase::Steam_ConsumeAllItems()
{
#if !UE_BUILD_SHIPPING
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
	if (!CurrencySub)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Steam_ConsumeAllItems: CurrencySubsystem 없음"));
		return;
	}

	CurrencySub->DebugConsumeAllSteamItems();
#endif
}

void AWjWorldPlayerControllerBase::Steam_GrantCoin(int32 Count)
{
#if !UE_BUILD_SHIPPING && WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (!SteamInv)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Steam_GrantCoin: Steam Inventory 사용 불가"));
		return;
	}

	if (Count < 1) Count = 1;

	// GenerateItems: 퍼블리셔 계정 전용 개발 API — 아이템 직접 생성
	SteamItemDef_t ItemDef = 1000; // WjCoin
	uint32 Quantity = static_cast<uint32>(Count);
	SteamInventoryResult_t ResultHandle = k_SteamInventoryResultInvalid;

	if (SteamInv->GenerateItems(&ResultHandle, &ItemDef, &Quantity, 1))
	{
		UE_LOG(LogWjWorldCurrency, Log, TEXT("Steam_GrantCoin: GenerateItems 성공 (%d코인)"), Count);
		SteamInv->DestroyResult(ResultHandle);

		// Steam 서버 반영 대기 후 인벤토리 갱신 (즉시 호출 시 아직 반영 안 됨)
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, [WeakThis = TWeakObjectPtr<AWjWorldPlayerControllerBase>(this)]()
		{
			if (AWjWorldPlayerControllerBase* PC = WeakThis.Get())
			{
				UWjWorldCosmeticSubsystem* CosmeticSub = PC->GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>();
				if (CosmeticSub)
				{
					CosmeticSub->RequestInventoryRefresh();
				}
				UE_LOG(LogWjWorldCurrency, Log, TEXT("Steam_GrantCoin: 지연 인벤토리 갱신 완료"));
			}
		}, 1.5f, false);
	}
	else
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("Steam_GrantCoin: GenerateItems 실패 (퍼블리셔 계정 필요)"));
	}
#else
	UE_LOG(LogWjWorldCurrency, Warning, TEXT("Steam_GrantCoin: Steam 빌드에서만 사용 가능"));
#endif
}

void AWjWorldPlayerControllerBase::TreasureChest_ClearCooldowns()
{
#if !UE_BUILD_SHIPPING
	UWorld* World = GetWorld();
	if (!World) return;

	int32 Count = 0;
	for (TActorIterator<AWjWorldTreasureChestActor> It(World); It; ++It)
	{
		It->ResetCooldown();
		++Count;
	}

	UE_LOG(LogWjWorld, Log, TEXT("TreasureChest_ClearCooldowns: %d개 보물상자 쿨타임 초기화 완료"), Count);
#endif
}

void AWjWorldPlayerControllerBase::Placement_Buy(FString ObjectId)
{
#if !UE_BUILD_SHIPPING
	UWjWorldCurrencySubsystem* CurrencySub = GetGameInstance()->GetSubsystem<UWjWorldCurrencySubsystem>();
	if (!CurrencySub)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Placement_Buy: CurrencySubsystem 없음"));
		return;
	}

	if (CurrencySub->PurchasePlacementObject(FName(*ObjectId)))
	{
		UE_LOG(LogWjWorld, Log, TEXT("Placement_Buy: %s 구매 요청 성공"), *ObjectId);
	}
	else
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Placement_Buy: %s 구매 요청 실패"), *ObjectId);
	}
#endif
}

void AWjWorldPlayerControllerBase::Placement_PrintInventory()
{
#if !UE_BUILD_SHIPPING
	UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Placement_PrintInventory: CosmeticSubsystem 없음"));
		return;
	}

	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		return;
	}

	UWjWorldPlaceableObjectDataAsset* Catalog = Settings->GetPlaceableCatalogForContext(EPlacementContext::Lobby);
	if (!Catalog)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Placement_PrintInventory: LobbyPlaceableCatalog 없음"));
		return;
	}

	UE_LOG(LogWjWorld, Log, TEXT("========== 배치 오브젝트 소유 현황 =========="));
	for (const FPlaceableObjectDefinition& Def : Catalog->Objects)
	{
		if (Def.SteamItemDefId > 0 && Def.CoinPrice > 0)
		{
			int32 OwnedQty = CosmeticSub->GetItemQuantityByDefId(Def.SteamItemDefId);
			UE_LOG(LogWjWorld, Log, TEXT("  %s (DefId:%d, Price:%d Coin) — 소유: %d"),
				*Def.ObjectId.ToString(), Def.SteamItemDefId, Def.CoinPrice, OwnedQty);
		}
		else
		{
			UE_LOG(LogWjWorld, Log, TEXT("  %s — 무료"), *Def.ObjectId.ToString());
		}
	}
	UE_LOG(LogWjWorld, Log, TEXT("============================================="));
#endif
}

void AWjWorldPlayerControllerBase::Placement_GrantItem(FString ObjectId, int32 Quantity)
{
#if !UE_BUILD_SHIPPING
	if (Quantity <= 0) Quantity = 1;

	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		return;
	}

	UWjWorldPlaceableObjectDataAsset* Catalog = Settings->GetPlaceableCatalogForContext(EPlacementContext::Lobby);
	if (!Catalog)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Placement_GrantItem: LobbyPlaceableCatalog 없음"));
		return;
	}

	const FPlaceableObjectDefinition* Def = Catalog->FindByObjectId(FName(*ObjectId));
	if (!Def)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Placement_GrantItem: '%s' 카탈로그에 없음"), *ObjectId);
		return;
	}

	if (Def->SteamItemDefId <= 0)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Placement_GrantItem: '%s'에 SteamItemDefId 없음"), *ObjectId);
		return;
	}

	UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Placement_GrantItem: CosmeticSubsystem 없음"));
		return;
	}

	// AllItemQuantities에 직접 추가 (로컬 테스트용)
	CosmeticSub->AllItemQuantities.FindOrAdd(Def->SteamItemDefId) += Quantity;

	// 비Steam: GConfig에도 기록
	static const FString PlacementInventorySection = TEXT("PlacementInventory");
	int32 CurrentOwned = 0;
	GConfig->GetInt(*PlacementInventorySection, *ObjectId, CurrentOwned, GGameUserSettingsIni);
	GConfig->SetInt(*PlacementInventorySection, *ObjectId, CurrentOwned + Quantity, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);

	CosmeticSub->OnInventoryUpdated.Broadcast();

	UE_LOG(LogWjWorld, Log, TEXT("Placement_GrantItem: %s x%d 부여 완료 (DefId: %d)"),
		*ObjectId, Quantity, Def->SteamItemDefId);
#endif
}

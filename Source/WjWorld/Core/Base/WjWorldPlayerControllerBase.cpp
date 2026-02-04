// Fill out your copyright notice in the Description page of Project Settings.

#include "WjWorldPlayerControllerBase.h"
#include "WjWorldCharacterBase.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Cosmetic/WjWorldCosmeticTypes.h"
#include "Setting/WjWorldDeveloperSettings.h"
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

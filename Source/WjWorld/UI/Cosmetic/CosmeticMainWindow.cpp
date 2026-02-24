// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Cosmetic/CosmeticMainWindow.h"
#include "UI/Cosmetic/CosmeticItemEntryWidget.h"
#include "UI/Cosmetic/CosmeticPreviewPanel.h"
#include "UI/Currency/CurrencyBalanceWidget.h"
#include "UI/Currency/GemPackStoreWidget.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Cosmetic/WjWorldCosmeticDataAsset.h"
#include "Cosmetic/WjWorldPurchaseSubsystem.h"
#include "Currency/WjWorldCurrencySubsystem.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/TextBlock.h"
#include "WjWorldLogCategories.h"
#include "TimerManager.h"

void UCosmeticMainWindow::NativeConstruct()
{
	Super::NativeConstruct();

	// 모드 탭 버튼 바인딩
	if (ShopTabButton)
	{
		ShopTabButton->OnClicked.RemoveDynamic(this, &UCosmeticMainWindow::OnShopTabClicked);
		ShopTabButton->OnClicked.AddDynamic(this, &UCosmeticMainWindow::OnShopTabClicked);
	}

	if (InventoryTabButton)
	{
		InventoryTabButton->OnClicked.RemoveDynamic(this, &UCosmeticMainWindow::OnInventoryTabClicked);
		InventoryTabButton->OnClicked.AddDynamic(this, &UCosmeticMainWindow::OnInventoryTabClicked);
	}

	// 슬롯 탭 버튼 바인딩
	if (TabHeadButton)
	{
		TabHeadButton->OnClicked.RemoveDynamic(this, &UCosmeticMainWindow::OnTabHeadClicked);
		TabHeadButton->OnClicked.AddDynamic(this, &UCosmeticMainWindow::OnTabHeadClicked);
	}

	if (TabBodyButton)
	{
		TabBodyButton->OnClicked.RemoveDynamic(this, &UCosmeticMainWindow::OnTabBodyClicked);
		TabBodyButton->OnClicked.AddDynamic(this, &UCosmeticMainWindow::OnTabBodyClicked);
	}

	if (TabBackButton)
	{
		TabBackButton->OnClicked.RemoveDynamic(this, &UCosmeticMainWindow::OnTabBackClicked);
		TabBackButton->OnClicked.AddDynamic(this, &UCosmeticMainWindow::OnTabBackClicked);
	}

	if (TabEffectButton)
	{
		TabEffectButton->OnClicked.RemoveDynamic(this, &UCosmeticMainWindow::OnTabEffectClicked);
		TabEffectButton->OnClicked.AddDynamic(this, &UCosmeticMainWindow::OnTabEffectClicked);
	}

	// 액션/닫기 버튼 바인딩
	if (ActionButton)
	{
		ActionButton->OnClicked.RemoveDynamic(this, &UCosmeticMainWindow::OnActionButtonClicked);
		ActionButton->OnClicked.AddDynamic(this, &UCosmeticMainWindow::OnActionButtonClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UCosmeticMainWindow::OnCloseClicked);
		CloseButton->OnClicked.AddDynamic(this, &UCosmeticMainWindow::OnCloseClicked);
	}

	// Gem 충전 버튼 바인딩
	if (GemChargeButton)
	{
		GemChargeButton->OnClicked.RemoveDynamic(this, &UCosmeticMainWindow::OnGemChargeClicked);
		GemChargeButton->OnClicked.AddDynamic(this, &UCosmeticMainWindow::OnGemChargeClicked);
	}

	// CurrencyBalanceWidget의 Gem 영역 클릭 구독
	if (CurrencyBalanceWidget)
	{
		CurrencyBalanceWidget->OnGemAreaClicked.RemoveDynamic(this, &UCosmeticMainWindow::OnGemAreaClicked);
		CurrencyBalanceWidget->OnGemAreaClicked.AddDynamic(this, &UCosmeticMainWindow::OnGemAreaClicked);
	}

	// 알림 텍스트 초기 숨김
	if (NotificationText)
	{
		NotificationText->SetVisibility(ESlateVisibility::Hidden);
	}

	// 서브시스템 델리게이트 바인딩
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		if (UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>())
		{
			CosmeticSub->OnInventoryUpdated.RemoveDynamic(this, &UCosmeticMainWindow::OnInventoryUpdated);
			CosmeticSub->OnInventoryUpdated.AddDynamic(this, &UCosmeticMainWindow::OnInventoryUpdated);
			CosmeticSub->OnLoadoutChanged.RemoveDynamic(this, &UCosmeticMainWindow::OnLoadoutChanged);
			CosmeticSub->OnLoadoutChanged.AddDynamic(this, &UCosmeticMainWindow::OnLoadoutChanged);
		}

		if (UWjWorldPurchaseSubsystem* PurchaseSub = GI->GetSubsystem<UWjWorldPurchaseSubsystem>())
		{
			PurchaseSub->OnPurchaseComplete.RemoveDynamic(this, &UCosmeticMainWindow::OnPurchaseComplete);
			PurchaseSub->OnPurchaseComplete.AddDynamic(this, &UCosmeticMainWindow::OnPurchaseComplete);
		}

		if (UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>())
		{
			CurrencySub->OnCurrencyPurchaseComplete.RemoveDynamic(this, &UCosmeticMainWindow::OnCurrencyPurchaseComplete);
			CurrencySub->OnCurrencyPurchaseComplete.AddDynamic(this, &UCosmeticMainWindow::OnCurrencyPurchaseComplete);
		}
	}

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: NativeConstruct completed"));
}

void UCosmeticMainWindow::NativeDestruct()
{
	// 알림 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(NotificationTimerHandle);
	}

	// Gem 관련 델리게이트 해제
	if (GemChargeButton)
	{
		GemChargeButton->OnClicked.RemoveDynamic(this, &UCosmeticMainWindow::OnGemChargeClicked);
	}

	if (CurrencyBalanceWidget)
	{
		CurrencyBalanceWidget->OnGemAreaClicked.RemoveDynamic(this, &UCosmeticMainWindow::OnGemAreaClicked);
	}

	// 서브시스템 델리게이트 바인딩 해제
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		if (UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>())
		{
			CosmeticSub->OnInventoryUpdated.RemoveDynamic(this, &UCosmeticMainWindow::OnInventoryUpdated);
			CosmeticSub->OnLoadoutChanged.RemoveDynamic(this, &UCosmeticMainWindow::OnLoadoutChanged);
		}

		if (UWjWorldPurchaseSubsystem* PurchaseSub = GI->GetSubsystem<UWjWorldPurchaseSubsystem>())
		{
			PurchaseSub->OnPurchaseComplete.RemoveDynamic(this, &UCosmeticMainWindow::OnPurchaseComplete);
		}

		if (UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>())
		{
			CurrencySub->OnCurrencyPurchaseComplete.RemoveDynamic(this, &UCosmeticMainWindow::OnCurrencyPurchaseComplete);
		}
	}

	ClearItemGrid();
	Super::NativeDestruct();
}

void UCosmeticMainWindow::ShowPopup()
{
	AddToViewport(100);

	// 입력 모드: UI 전용
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}

	// 초기 상태 설정 및 UI 갱신
	CurrentMode = ECosmeticWindowMode::Shop;
	CurrentSlot = ECosmeticSlot::Head;
	SelectedItemId = NAME_None;

	RefreshUI();

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Popup shown"));
}

void UCosmeticMainWindow::ClosePopup()
{
	// 시착 취소
	if (PreviewPanel)
	{
		PreviewPanel->ResetPreview();
	}

	RemoveFromParent();

	// 입력 모드 복원
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->SetInputMode(FInputModeGameAndUI());
		PC->bShowMouseCursor = true;
	}

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Popup closed"));
}

void UCosmeticMainWindow::OnShopTabClicked()
{
	SetMode(ECosmeticWindowMode::Shop);
}

void UCosmeticMainWindow::OnInventoryTabClicked()
{
	SetMode(ECosmeticWindowMode::Inventory);
}

void UCosmeticMainWindow::OnTabHeadClicked()
{
	SetSlot(ECosmeticSlot::Head);
}

void UCosmeticMainWindow::OnTabBodyClicked()
{
	SetSlot(ECosmeticSlot::Body);
}

void UCosmeticMainWindow::OnTabBackClicked()
{
	SetSlot(ECosmeticSlot::Back);
}

void UCosmeticMainWindow::OnTabEffectClicked()
{
	SetSlot(ECosmeticSlot::Effect);
}

void UCosmeticMainWindow::OnActionButtonClicked()
{
	if (SelectedItemId.IsNone())
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		return;
	}

	const FCosmeticLoadout& Loadout = CosmeticSub->GetLoadout();
	FName EquippedItem = Loadout.GetEquippedItem(CurrentSlot);

	if (CurrentMode == ECosmeticWindowMode::Shop)
	{
		// 상점 모드
		if (CosmeticSub->HasItem(SelectedItemId))
		{
			// 보유 중인 아이템 → 장착 또는 해제
			if (EquippedItem == SelectedItemId)
			{
				CosmeticSub->UnequipSlot(CurrentSlot);
				UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Unequipped slot %d (from Shop)"), static_cast<int32>(CurrentSlot));
			}
			else
			{
				CosmeticSub->EquipItem(CurrentSlot, SelectedItemId);
				UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Equipped %s on slot %d (from Shop)"), *SelectedItemId.ToString(), static_cast<int32>(CurrentSlot));
			}
			CosmeticSub->SaveLoadoutToLocal();
		}
		else
		{
			// 미보유 아이템 → 재화 구매 우선, 재화 가격 없는 경우만 Steam 실결제
			UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();

			// 재화 교환 진행 중이면 중복 요청 방지
			if (CurrencySub && CurrencySub->IsExchangePending())
			{
				UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Exchange already pending, ignoring click"));
				return;
			}

			const FCosmeticItemDefinition* ItemDef = CosmeticSub->GetCatalog() ? CosmeticSub->GetCatalog()->FindByItemId(SelectedItemId) : nullptr;

			bool bHasCurrencyPrice = ItemDef && (ItemDef->CoinPrice > 0 || ItemDef->GemPrice > 0);
			bool bCurrencyPurchased = false;

			if (ItemDef && CurrencySub)
			{
				// Coin 잔액 충분 시 Coin으로 구매
				if (ItemDef->CoinPrice > 0 && CurrencySub->GetBalance(ECurrencyType::Coin) >= ItemDef->CoinPrice)
				{
					bCurrencyPurchased = CurrencySub->PurchaseItemWithCurrency(SelectedItemId, ECurrencyType::Coin);
					if (bCurrencyPurchased)
					{
						UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Coin purchase requested for %s"), *SelectedItemId.ToString());
					}
				}

				// Coin 구매 실패 시 Gem 잔액 충분하면 Gem으로 구매
				if (!bCurrencyPurchased && ItemDef->GemPrice > 0 && CurrencySub->GetBalance(ECurrencyType::Gem) >= ItemDef->GemPrice)
				{
					bCurrencyPurchased = CurrencySub->PurchaseItemWithCurrency(SelectedItemId, ECurrencyType::Gem);
					if (bCurrencyPurchased)
					{
						UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Gem purchase requested for %s"), *SelectedItemId.ToString());
					}
				}
			}

			if (bCurrencyPurchased)
			{
				// 재화 구매 성공 → 알림은 OnCurrencyPurchaseComplete에서 처리
			}
			else if (bHasCurrencyPrice)
			{
				// 재화 가격이 있지만 잔액 부족 → Steam 실결제로 넘어가지 않음
				ShowNotification(NSLOCTEXT("Cosmetic", "InsufficientBalanceNotify", "잔액이 부족합니다"), false);
				UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Insufficient balance for %s"), *SelectedItemId.ToString());
			}
			else
			{
				// 재화 가격 없음 → Steam 실결제
				UWjWorldPurchaseSubsystem* PurchaseSub = GI->GetSubsystem<UWjWorldPurchaseSubsystem>();
				if (PurchaseSub && !PurchaseSub->IsPurchasePending())
				{
					PurchaseSub->RequestPurchase(SelectedItemId);
					UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Steam purchase requested for %s"), *SelectedItemId.ToString());
				}
			}
		}
	}
	else
	{
		// 인벤토리 모드: 장착 또는 해제
		if (EquippedItem == SelectedItemId)
		{
			CosmeticSub->UnequipSlot(CurrentSlot);
			UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Unequipped slot %d"), static_cast<int32>(CurrentSlot));
		}
		else
		{
			CosmeticSub->EquipItem(CurrentSlot, SelectedItemId);
			UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Equipped %s on slot %d"), *SelectedItemId.ToString(), static_cast<int32>(CurrentSlot));
		}
		CosmeticSub->SaveLoadoutToLocal();
	}
}

void UCosmeticMainWindow::OnCloseClicked()
{
	ClosePopup();
}

void UCosmeticMainWindow::OnGemChargeClicked()
{
	OpenGemPackStore();
}

void UCosmeticMainWindow::OnGemAreaClicked()
{
	OpenGemPackStore();
}

void UCosmeticMainWindow::OpenGemPackStore()
{
	if (!GemPackStoreWidgetClass)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("CosmeticMainWindow: GemPackStoreWidgetClass is not set"));
		return;
	}

	if (!GemPackStoreInstance)
	{
		GemPackStoreInstance = CreateWidget<UGemPackStoreWidget>(GetOwningPlayer(), GemPackStoreWidgetClass);
	}

	if (GemPackStoreInstance)
	{
		GemPackStoreInstance->ShowPopup();
	}
}

void UCosmeticMainWindow::OnItemClicked(FName ItemId)
{
	SelectItem(ItemId);
}

void UCosmeticMainWindow::OnInventoryUpdated()
{
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Inventory updated, refreshing UI"));
	RefreshUI();
}

void UCosmeticMainWindow::OnLoadoutChanged(ECosmeticSlot CosmeticSlot, FName ItemId)
{
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Loadout changed, refreshing UI"));
	RefreshUI();

	// 프리뷰 패널도 갱신
	if (PreviewPanel)
	{
		PreviewPanel->ShowCurrentLoadout();
	}
}

void UCosmeticMainWindow::OnPurchaseComplete(FName ItemId, bool bSuccess)
{
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Purchase %s for %s"), bSuccess ? TEXT("succeeded") : TEXT("failed"), *ItemId.ToString());

	if (bSuccess)
	{
		ShowNotification(NSLOCTEXT("Cosmetic", "PurchaseSuccess", "구매 성공!"), true);
	}
	else
	{
		ShowNotification(NSLOCTEXT("Cosmetic", "PurchaseFailed", "구매에 실패했습니다"), false);
	}

	RefreshUI();
}

void UCosmeticMainWindow::OnCurrencyPurchaseComplete(FName ItemId, bool bSuccess)
{
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Currency purchase %s for %s"), bSuccess ? TEXT("succeeded") : TEXT("failed"), *ItemId.ToString());

	if (bSuccess)
	{
		ShowNotification(NSLOCTEXT("Cosmetic", "CurrencyPurchaseSuccess", "구매 성공!"), true);
	}
	else
	{
		ShowNotification(NSLOCTEXT("Cosmetic", "CurrencyPurchaseFailed", "구매에 실패했습니다"), false);
	}

	RefreshUI();
}

void UCosmeticMainWindow::RefreshUI()
{
	UpdateTabStyles();
	RefreshItemGrid();
	RefreshDetailInfo();
	RefreshActionButton();
}

void UCosmeticMainWindow::RefreshItemGrid()
{
	ClearItemGrid();

	if (CurrentMode == ECosmeticWindowMode::Shop)
	{
		PopulateShopItems();
	}
	else
	{
		PopulateInventoryItems();
	}
}

void UCosmeticMainWindow::RefreshDetailInfo()
{
	if (SelectedItemId.IsNone())
	{
		if (SelectedItemNameText)
		{
			SelectedItemNameText->SetText(FText::GetEmpty());
		}
		if (SelectedItemDescriptionText)
		{
			SelectedItemDescriptionText->SetText(FText::GetEmpty());
		}
		if (SelectedItemPriceText)
		{
			SelectedItemPriceText->SetText(FText::GetEmpty());
		}
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub || !CosmeticSub->GetCatalog())
	{
		return;
	}

	const FCosmeticItemDefinition* ItemDef = CosmeticSub->GetCatalog()->FindByItemId(SelectedItemId);
	if (!ItemDef)
	{
		return;
	}

	if (SelectedItemNameText)
	{
		SelectedItemNameText->SetText(ItemDef->DisplayName);
	}

	if (SelectedItemDescriptionText)
	{
		SelectedItemDescriptionText->SetText(ItemDef->Description);
	}

	if (SelectedItemPriceText)
	{
		if (CosmeticSub->HasItem(SelectedItemId))
		{
			SelectedItemPriceText->SetText(NSLOCTEXT("Cosmetic", "Owned", "보유중"));
		}
		else if (ItemDef->CoinPrice > 0 && ItemDef->GemPrice > 0)
		{
			SelectedItemPriceText->SetText(FText::Format(
				NSLOCTEXT("Cosmetic", "CoinGemPriceFormat", "{0} Coin / {1} Gem"),
				FText::AsNumber(ItemDef->CoinPrice), FText::AsNumber(ItemDef->GemPrice)));
		}
		else if (ItemDef->CoinPrice > 0)
		{
			SelectedItemPriceText->SetText(FText::Format(
				NSLOCTEXT("Cosmetic", "CoinPriceFormat", "{0} Coin"),
				FText::AsNumber(ItemDef->CoinPrice)));
		}
		else if (ItemDef->GemPrice > 0)
		{
			SelectedItemPriceText->SetText(FText::Format(
				NSLOCTEXT("Cosmetic", "GemPriceFormat", "{0} Gem"),
				FText::AsNumber(ItemDef->GemPrice)));
		}
		else
		{
			SelectedItemPriceText->SetText(NSLOCTEXT("Cosmetic", "Free", "무료"));
		}
	}
}

void UCosmeticMainWindow::RefreshActionButton()
{
	if (!ActionButton || !ActionButtonText)
	{
		return;
	}

	if (SelectedItemId.IsNone())
	{
		ActionButton->SetIsEnabled(false);
		ActionButtonText->SetText(NSLOCTEXT("Cosmetic", "SelectItem", "아이템 선택"));
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		return;
	}

	const FCosmeticLoadout& Loadout = CosmeticSub->GetLoadout();
	FName EquippedItem = Loadout.GetEquippedItem(CurrentSlot);
	bool bIsEquipped = (EquippedItem == SelectedItemId);

	// 장착/해제 분기에서 기본 색상 리셋
	auto ResetButtonTextColor = [this]()
	{
		ActionButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	};

	if (CurrentMode == ECosmeticWindowMode::Shop)
	{
		// 상점 모드
		if (CosmeticSub->HasItem(SelectedItemId))
		{
			// 보유 중인 아이템 → 장착/해제 가능
			ActionButton->SetIsEnabled(true);
			ResetButtonTextColor();

			if (bIsEquipped)
			{
				ActionButtonText->SetText(NSLOCTEXT("Cosmetic", "Unequip", "해제"));
			}
			else
			{
				ActionButtonText->SetText(NSLOCTEXT("Cosmetic", "Equip", "장착"));
			}
		}
		else
		{
			// 미보유 아이템 → 구매
			UWjWorldPurchaseSubsystem* PurchaseSub = GI->GetSubsystem<UWjWorldPurchaseSubsystem>();
			UWjWorldCurrencySubsystem* CurrencySub2 = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
			bool bPending = (PurchaseSub && PurchaseSub->IsPurchasePending())
				|| (CurrencySub2 && CurrencySub2->IsExchangePending());

			const FCosmeticItemDefinition* ItemDef = nullptr;
			if (CosmeticSub->GetCatalog())
			{
				ItemDef = CosmeticSub->GetCatalog()->FindByItemId(SelectedItemId);
			}

			UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();

			bool bHasCoinPrice = ItemDef && ItemDef->CoinPrice > 0;
			bool bHasGemPrice = ItemDef && ItemDef->GemPrice > 0;
			bool bCoinAffordable = bHasCoinPrice && CurrencySub && CurrencySub->GetBalance(ECurrencyType::Coin) >= ItemDef->CoinPrice;
			bool bGemAffordable = bHasGemPrice && CurrencySub && CurrencySub->GetBalance(ECurrencyType::Gem) >= ItemDef->GemPrice;

			if (bPending)
			{
				ActionButton->SetIsEnabled(false);
				ResetButtonTextColor();
				ActionButtonText->SetText(NSLOCTEXT("Cosmetic", "Purchasing", "구매 중..."));
			}
			else if (bCoinAffordable)
			{
				ActionButton->SetIsEnabled(true);
				ResetButtonTextColor();
				ActionButtonText->SetText(FText::Format(
					NSLOCTEXT("Cosmetic", "PurchaseCoinAmount", "{0} Coin 구매"),
					FText::AsNumber(ItemDef->CoinPrice)));
			}
			else if (bGemAffordable)
			{
				ActionButton->SetIsEnabled(true);
				ResetButtonTextColor();
				ActionButtonText->SetText(FText::Format(
					NSLOCTEXT("Cosmetic", "PurchaseGemAmount", "{0} Gem 구매"),
					FText::AsNumber(ItemDef->GemPrice)));
			}
			else if (bHasCoinPrice || bHasGemPrice)
			{
				// 재화 가격이 있지만 잔액 부족
				ActionButton->SetIsEnabled(false);
				ActionButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
				ActionButtonText->SetText(NSLOCTEXT("Cosmetic", "InsufficientBalance", "잔액 부족"));
			}
			else
			{
				// 재화 가격 없음 → Steam 실결제
				ActionButton->SetIsEnabled(true);
				ResetButtonTextColor();
				ActionButtonText->SetText(NSLOCTEXT("Cosmetic", "Purchase", "구매"));
			}
		}
	}
	else
	{
		// 인벤토리 모드
		ActionButton->SetIsEnabled(true);
		ResetButtonTextColor();

		if (bIsEquipped)
		{
			ActionButtonText->SetText(NSLOCTEXT("Cosmetic", "Unequip", "해제"));
		}
		else
		{
			ActionButtonText->SetText(NSLOCTEXT("Cosmetic", "Equip", "장착"));
		}
	}
}

void UCosmeticMainWindow::UpdateTabStyles()
{
	// 모드 탭 스타일 (현재 모드에 따라 시각적 피드백)
	// Blueprint에서 버튼 스타일을 더 상세하게 조정할 수 있음
	// 여기서는 간단한 활성화/비활성화 표시만 수행

	// 슬롯 탭도 마찬가지로 Blueprint에서 스타일 조정 가능
}

void UCosmeticMainWindow::SetMode(ECosmeticWindowMode NewMode)
{
	if (CurrentMode == NewMode)
	{
		return;
	}

	CurrentMode = NewMode;
	SelectedItemId = NAME_None;

	// 시착 취소
	if (PreviewPanel)
	{
		PreviewPanel->ResetPreview();
	}

	RefreshUI();

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Mode changed to %s"),
		CurrentMode == ECosmeticWindowMode::Shop ? TEXT("Shop") : TEXT("Inventory"));
}

void UCosmeticMainWindow::SetSlot(ECosmeticSlot NewSlot)
{
	if (CurrentSlot == NewSlot)
	{
		return;
	}

	CurrentSlot = NewSlot;
	SelectedItemId = NAME_None;

	// 슬롯 전환 시 시착 유지 (다중 슬롯 시착 지원)
	// 모드 전환 시에만 ResetPreview() 호출

	RefreshUI();

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Slot changed to %d"), static_cast<int32>(CurrentSlot));
}

void UCosmeticMainWindow::SelectItem(FName ItemId)
{
	// 기존 선택 해제
	for (UCosmeticItemEntryWidget* Entry : ItemEntryWidgets)
	{
		if (Entry)
		{
			Entry->SetSelected(Entry->GetItemId() == ItemId);
		}
	}

	SelectedItemId = ItemId;
	RefreshDetailInfo();
	RefreshActionButton();

	// 시착 프리뷰
	if (PreviewPanel && !ItemId.IsNone())
	{
		PreviewPanel->PreviewItem(CurrentSlot, ItemId);
	}

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Item selected: %s"), *ItemId.ToString());
}

void UCosmeticMainWindow::PopulateShopItems()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub || !CosmeticSub->GetCatalog())
	{
		return;
	}

	UWjWorldCosmeticCatalogDataAsset* Catalog = CosmeticSub->GetCatalog();
	TArray<const FCosmeticItemDefinition*> Items = Catalog->GetItemsBySlot(CurrentSlot);

	const FCosmeticLoadout& Loadout = CosmeticSub->GetLoadout();

	int32 Index = 0;
	for (const FCosmeticItemDefinition* ItemDef : Items)
	{
		if (!ItemDef || !ItemDef->IsValid())
		{
			continue;
		}

		UCosmeticItemEntryWidget* EntryWidget = CreateItemEntry(*ItemDef);
		if (!EntryWidget)
		{
			continue;
		}

		// 보유/장착 상태 설정
		bool bOwned = CosmeticSub->HasItem(ItemDef->ItemId);
		bool bEquipped = Loadout.GetEquippedItem(CurrentSlot) == ItemDef->ItemId;

		EntryWidget->SetOwned(bOwned);
		EntryWidget->SetEquipped(bEquipped);
		EntryWidget->SetSelected(ItemDef->ItemId == SelectedItemId);

		// 그리드에 추가
		if (ItemGridPanel)
		{
			ItemGridPanel->AddChildToUniformGrid(EntryWidget, Index / GridColumnCount, Index % GridColumnCount);
		}

		ItemEntryWidgets.Add(EntryWidget);
		Index++;
	}

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Populated %d shop items"), Index);
}

void UCosmeticMainWindow::PopulateInventoryItems()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub || !CosmeticSub->GetCatalog())
	{
		return;
	}

	UWjWorldCosmeticCatalogDataAsset* Catalog = CosmeticSub->GetCatalog();
	const TArray<FCosmeticItemInstance>& Inventory = CosmeticSub->GetInventory();
	const FCosmeticLoadout& Loadout = CosmeticSub->GetLoadout();

	int32 Index = 0;
	for (const FCosmeticItemInstance& ItemInstance : Inventory)
	{
		const FCosmeticItemDefinition* ItemDef = Catalog->FindByItemId(ItemInstance.ItemId);
		if (!ItemDef || !ItemDef->IsValid())
		{
			continue;
		}

		// 현재 슬롯에 해당하는 아이템만 표시
		if (ItemDef->Slot != CurrentSlot)
		{
			continue;
		}

		UCosmeticItemEntryWidget* EntryWidget = CreateItemEntry(*ItemDef);
		if (!EntryWidget)
		{
			continue;
		}

		// 인벤토리 모드에서는 항상 보유 중
		bool bEquipped = Loadout.GetEquippedItem(CurrentSlot) == ItemDef->ItemId;

		EntryWidget->SetOwned(true);
		EntryWidget->SetEquipped(bEquipped);
		EntryWidget->SetSelected(ItemDef->ItemId == SelectedItemId);

		// 그리드에 추가
		if (ItemGridPanel)
		{
			ItemGridPanel->AddChildToUniformGrid(EntryWidget, Index / GridColumnCount, Index % GridColumnCount);
		}

		ItemEntryWidgets.Add(EntryWidget);
		Index++;
	}

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Populated %d inventory items"), Index);
}

UCosmeticItemEntryWidget* UCosmeticMainWindow::CreateItemEntry(const FCosmeticItemDefinition& Def)
{
	if (!ItemEntryWidgetClass)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("CosmeticMainWindow: ItemEntryWidgetClass is not set"));
		return nullptr;
	}

	UCosmeticItemEntryWidget* EntryWidget = CreateWidget<UCosmeticItemEntryWidget>(GetOwningPlayer(), ItemEntryWidgetClass);
	if (!EntryWidget)
	{
		return nullptr;
	}

	EntryWidget->SetItemDefinition(Def);
	EntryWidget->OnItemClicked.AddDynamic(this, &UCosmeticMainWindow::OnItemClicked);

	return EntryWidget;
}

void UCosmeticMainWindow::ClearItemGrid()
{
	// 델리게이트 바인딩 해제 및 위젯 제거
	for (UCosmeticItemEntryWidget* Entry : ItemEntryWidgets)
	{
		if (Entry)
		{
			Entry->OnItemClicked.RemoveDynamic(this, &UCosmeticMainWindow::OnItemClicked);
			Entry->RemoveFromParent();
		}
	}
	ItemEntryWidgets.Empty();

	// 그리드 클리어
	if (ItemGridPanel)
	{
		ItemGridPanel->ClearChildren();
	}
}

void UCosmeticMainWindow::ShowNotification(const FText& Message, bool bSuccess)
{
	if (!NotificationText)
	{
		return;
	}

	NotificationText->SetText(Message);
	NotificationText->SetColorAndOpacity(bSuccess
		? FSlateColor(FLinearColor::Green)
		: FSlateColor(FLinearColor::Red));
	NotificationText->SetVisibility(ESlateVisibility::Visible);

	// 기존 타이머 갱신
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(NotificationTimerHandle);
		World->GetTimerManager().SetTimer(
			NotificationTimerHandle,
			this,
			&UCosmeticMainWindow::HideNotification,
			NotificationDisplayDuration,
			false
		);
	}
}

void UCosmeticMainWindow::HideNotification()
{
	if (NotificationText)
	{
		NotificationText->SetVisibility(ESlateVisibility::Hidden);
	}
}

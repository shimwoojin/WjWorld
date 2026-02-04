// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Cosmetic/CosmeticMainWindow.h"
#include "UI/Cosmetic/CosmeticItemEntryWidget.h"
#include "UI/Cosmetic/CosmeticPreviewPanel.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Cosmetic/WjWorldCosmeticDataAsset.h"
#include "Cosmetic/WjWorldPurchaseSubsystem.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/TextBlock.h"
#include "WjWorldLogCategories.h"

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
	}

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: NativeConstruct completed"));
}

void UCosmeticMainWindow::NativeDestruct()
{
	// 델리게이트 바인딩 해제
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
			// 미보유 아이템 → 구매
			UWjWorldPurchaseSubsystem* PurchaseSub = GI->GetSubsystem<UWjWorldPurchaseSubsystem>();
			if (PurchaseSub && !PurchaseSub->IsPurchasePending())
			{
				PurchaseSub->RequestPurchase(SelectedItemId);
				UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticMainWindow: Purchase requested for %s"), *SelectedItemId.ToString());
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
		else if (ItemDef->Price > 0)
		{
			SelectedItemPriceText->SetText(FText::Format(NSLOCTEXT("Cosmetic", "PriceFormat", "{0}"), FText::AsNumber(ItemDef->Price)));
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

	if (CurrentMode == ECosmeticWindowMode::Shop)
	{
		// 상점 모드
		if (CosmeticSub->HasItem(SelectedItemId))
		{
			// 보유 중인 아이템 → 장착/해제 가능
			ActionButton->SetIsEnabled(true);

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
			bool bPending = PurchaseSub && PurchaseSub->IsPurchasePending();

			ActionButton->SetIsEnabled(!bPending);
			ActionButtonText->SetText(bPending
				? NSLOCTEXT("Cosmetic", "Purchasing", "구매 중...")
				: NSLOCTEXT("Cosmetic", "Purchase", "구매"));
		}
	}
	else
	{
		// 인벤토리 모드
		ActionButton->SetIsEnabled(true);

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

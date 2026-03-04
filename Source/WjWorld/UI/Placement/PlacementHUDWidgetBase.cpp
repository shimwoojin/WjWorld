// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Placement/PlacementHUDWidgetBase.h"
#include "UI/Placement/PlacementSaveDialogWidget.h"
#include "UI/Placement/PlacementLoadDialogWidget.h"
#include "UI/Placement/PlacementCatalogItemWidget.h"
#include "UI/Common/ConfirmDialogWidget.h"
#include "GamePlay/Placement/WjWorldPlacementComponent.h"
#include "GamePlay/Placement/IWjWorldPlacementDataProvider.h"
#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Currency/WjWorldCurrencySubsystem.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "WjWorldLogCategories.h"

void UPlacementHUDWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UPlacementHUDWidgetBase::OnExitClicked);
	}

	if (DeleteModeButton)
	{
		DeleteModeButton->OnClicked.AddDynamic(this, &UPlacementHUDWidgetBase::OnDeleteModeClicked);
	}

	if (SaveButton)
	{
		SaveButton->OnClicked.AddDynamic(this, &UPlacementHUDWidgetBase::OnSaveClicked);
	}

	if (LoadButton)
	{
		LoadButton->OnClicked.AddDynamic(this, &UPlacementHUDWidgetBase::OnLoadClicked);
	}

	if (ClearButton)
	{
		ClearButton->OnClicked.AddDynamic(this, &UPlacementHUDWidgetBase::OnClearClicked);
	}

	// 기본 조작 안내
	UpdateControlsHint();
}

void UPlacementHUDWidgetBase::SetPlacementComponent(UWjWorldPlacementComponent* InComponent)
{
	PlacementComponent = InComponent;

	if (PlacementComponent)
	{
		PopulateCatalog(PlacementComponent->GetCatalog());

		// 배치/삭제 시 카탈로그 리프레시 (배치 수 갱신)
		PlacementComponent->OnObjectPlaced.AddDynamic(this, &UPlacementHUDWidgetBase::RefreshCatalogUI);
		PlacementComponent->OnObjectDeleted.AddDynamic(this, &UPlacementHUDWidgetBase::RefreshCatalogUI);

		// 상태 변경 구독
		PlacementComponent->OnAirModeChanged.AddDynamic(this, &UPlacementHUDWidgetBase::OnAirModeChanged);
		PlacementComponent->OnSnapDegreesChanged.AddDynamic(this, &UPlacementHUDWidgetBase::OnSnapDegreesChanged);
		PlacementComponent->OnObjectSelected.AddDynamic(this, &UPlacementHUDWidgetBase::OnObjectSelected);

		// 현재 상태 반영 (위젯이 늦게 생성된 경우 대비)
		bCachedAirMode = PlacementComponent->IsAirPlacementMode();
		CachedSnapDegrees = PlacementComponent->GetCurrentSnapDegrees();
		UpdateControlsHint();

		// 로비 컨텍스트: 구매/인벤토리 델리게이트 구독
		if (GetCurrentContext() == EPlacementContext::Lobby)
		{
			UGameInstance* GI = GetGameInstance();
			if (GI)
			{
				UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
				if (CurrencySub)
				{
					CurrencySub->OnPlacementPurchaseComplete.AddDynamic(this, &UPlacementHUDWidgetBase::HandlePlacementPurchaseComplete);
				}

				UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
				if (CosmeticSub)
				{
					CosmeticSub->OnInventoryUpdated.AddDynamic(this, &UPlacementHUDWidgetBase::HandleInventoryUpdated);
				}
			}
		}
	}
}

EPlacementContext UPlacementHUDWidgetBase::GetCurrentContext() const
{
	if (PlacementComponent)
	{
		return PlacementComponent->GetCurrentContext();
	}
	return EPlacementContext::None;
}

void UPlacementHUDWidgetBase::PopulateCatalog(UWjWorldPlaceableObjectDataAsset* Catalog)
{
	if (!CatalogScrollBox || !Catalog)
	{
		return;
	}

	CatalogScrollBox->ClearChildren();
	CatalogItemWidgets.Empty();

	// 로비 컨텍스트일 때만 소유권/가격 정보 표시
	const bool bIsLobby = (GetCurrentContext() == EPlacementContext::Lobby);
	UWjWorldCosmeticSubsystem* CosmeticSub = nullptr;
	int32 CoinBalance = 0;
	int32 TotalPlacedCount = 0;
	int32 MaxTotalPlaced = 0;

	if (bIsLobby)
	{
		UGameInstance* GI = GetGameInstance();
		if (GI)
		{
			CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
			UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
			if (CurrencySub)
			{
				CoinBalance = CurrencySub->GetBalance(ECurrencyType::Coin);
			}
		}

		const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
		if (Settings)
		{
			MaxTotalPlaced = Settings->MaxTotalLobbyPlacedObjects;
		}

		if (PlacementComponent)
		{
			IWjWorldPlacementDataProvider* DataProvider = PlacementComponent->GetPlacementDataProvider();
			if (DataProvider)
			{
				TotalPlacedCount = DataProvider->GetPlacedObjects().Num();
			}
		}
	}

	const bool bTotalLimitReached = (MaxTotalPlaced > 0 && TotalPlacedCount >= MaxTotalPlaced);

	for (const FPlaceableObjectDefinition& Def : Catalog->Objects)
	{
		const bool bIsPaidItem = (bIsLobby && Def.CoinPrice > 0 && Def.SteamItemDefId > 0);
		int32 OwnedQty = (bIsPaidItem && CosmeticSub) ? CosmeticSub->GetItemQuantityByDefId(Def.SteamItemDefId) : 0;
		const bool bOwned = (!bIsPaidItem || OwnedQty > 0);

		// 아이템 위젯 생성
		UPlacementCatalogItemWidget* ItemWidget = nullptr;
		if (CatalogItemWidgetClass)
		{
			ItemWidget = CreateWidget<UPlacementCatalogItemWidget>(GetOwningPlayer(), CatalogItemWidgetClass);
		}

		if (!ItemWidget)
		{
			continue;
		}

		// 아이콘 로드 (동기 — 카탈로그 UI 생성 시점이므로 허용)
		UTexture2D* IconTexture = nullptr;
		if (!Def.Icon.IsNull())
		{
			IconTexture = Def.Icon.LoadSynchronous();
		}

		// 기본 데이터 설정
		FText DisplayName = Def.DisplayName.IsEmpty()
			? FText::FromName(Def.ObjectId)
			: Def.DisplayName;
		ItemWidget->SetItemData(Def.ObjectId, DisplayName, IconTexture);

		// 배치 수량 표시
		if (bIsLobby && PlacementComponent)
		{
			int32 PlacedCount = PlacementComponent->CountPlacedObjectsByType(Def.ObjectId);
			if (bIsPaidItem && Def.MaxPlacementCount > 0)
			{
				ItemWidget->SetPlacementCount(PlacedCount, OwnedQty);
			}
			else if (Def.MaxPlacementCount > 0)
			{
				ItemWidget->SetPlacementCount(PlacedCount, Def.MaxPlacementCount);
			}
		}

		// 소유 상태
		ItemWidget->SetOwned(bOwned);

		// 구매 버튼
		const bool bCanBuyMore = bIsPaidItem
			&& (Def.MaxPlacementCount > 0 ? OwnedQty < Def.MaxPlacementCount : !bOwned);
		bool bCanBuy = bCanBuyMore && (CoinBalance >= Def.CoinPrice) && !bTotalLimitReached;
		ItemWidget->SetBuyInfo(bCanBuyMore, Def.CoinPrice, bCanBuy);

		// 이벤트 바인딩
		ItemWidget->OnItemSelected.AddDynamic(this, &UPlacementHUDWidgetBase::HandleCatalogItemSelected);
		ItemWidget->OnBuyClicked.AddDynamic(this, &UPlacementHUDWidgetBase::HandleCatalogItemBuyClicked);

		CatalogScrollBox->AddChild(ItemWidget);
		CatalogItemWidgets.Add(ItemWidget);
	}

	UpdateTotalPlacementCountText();

	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Catalog populated with %d items"), Catalog->Objects.Num());
}

void UPlacementHUDWidgetBase::OnExitClicked()
{
	// 서브클래스에서 오버라이드
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Exit clicked (base implementation)"));
}

void UPlacementHUDWidgetBase::OnDeleteModeClicked()
{
	if (PlacementComponent)
	{
		PlacementComponent->ToggleDeleteMode();
	}
}

void UPlacementHUDWidgetBase::OnSaveClicked()
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Save clicked"));

	if (!SaveDialogClass)
	{
		// 다이얼로그 클래스가 없으면 기본 슬롯 이름으로 바로 저장
		FString DefaultSlotName = GetCurrentLoadedSlotName();
		if (DefaultSlotName.IsEmpty())
		{
			DefaultSlotName = GetSaveSlotNameForContext(GetCurrentContext());
		}
		ExecuteSave(DefaultSlotName);
		return;
	}

	// 이미 열려있으면 스킵
	if (SaveDialogInstance && SaveDialogInstance->IsInViewport())
	{
		return;
	}

	// 다이얼로그 생성
	SaveDialogInstance = CreateWidget<UPlacementSaveDialogWidget>(GetOwningPlayer(), SaveDialogClass);
	if (SaveDialogInstance)
	{
		// 로드된 슬롯 이름이 있으면 그것을 기본값으로, 없으면 컨텍스트 기본값 사용
		FString DefaultSlotName = GetCurrentLoadedSlotName();
		if (DefaultSlotName.IsEmpty())
		{
			DefaultSlotName = GetSaveSlotNameForContext(GetCurrentContext());
		}
		SaveDialogInstance->SetDefaultSlotName(DefaultSlotName);
		SaveDialogInstance->SetContext(GetCurrentContext());

		// 콜백 바인딩
		SaveDialogInstance->OnSaveConfirmed.AddDynamic(this, &UPlacementHUDWidgetBase::OnSaveConfirmed);
		SaveDialogInstance->ShowPopup();
	}
}

void UPlacementHUDWidgetBase::OnSaveConfirmed(const FString& SlotName)
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Save confirmed with slot name: %s"), *SlotName);
	ExecuteSave(SlotName);
}

void UPlacementHUDWidgetBase::ExecuteSave(const FString& SlotName)
{
	// 기본 구현: PlacementComponent의 SaveLayoutToSlot 호출
	if (PlacementComponent)
	{
		PlacementComponent->SaveLayoutToSlot(SlotName);
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Layout saved to slot '%s'"), *SlotName);
	}
}

void UPlacementHUDWidgetBase::HandleCatalogItemSelected(FName ObjectId)
{
	if (!PlacementComponent)
	{
		return;
	}

	// SelectObject 내부에서 소유권 검증 (미소유 시 차단됨)
	PlacementComponent->SelectObject(ObjectId);
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Selected object %s"), *ObjectId.ToString());
}

void UPlacementHUDWidgetBase::HandleCatalogItemBuyClicked(FName ObjectId)
{
	UGameInstance* GI = GetGameInstance();
	UWjWorldCurrencySubsystem* CurrencySub = GI ? GI->GetSubsystem<UWjWorldCurrencySubsystem>() : nullptr;
	if (CurrencySub)
	{
		CurrencySub->PurchasePlacementObject(ObjectId);
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: 구매 시도 %s"), *ObjectId.ToString());
	}
}

void UPlacementHUDWidgetBase::SetTitleText(const FText& Title)
{
	if (TitleText)
	{
		TitleText->SetText(Title);
	}
}

void UPlacementHUDWidgetBase::SetControlsHintText(const FText& Hint)
{
	if (ControlsHintText)
	{
		ControlsHintText->SetText(Hint);
	}
}

void UPlacementHUDWidgetBase::OnLoadClicked()
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Load clicked"));

	if (!LoadDialogClass)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementHUDWidgetBase: LoadDialogClass not set"));
		return;
	}

	// 이미 열려있으면 스킵
	if (LoadDialogInstance && LoadDialogInstance->IsInViewport())
	{
		return;
	}

	// 다이얼로그 생성
	LoadDialogInstance = CreateWidget<UPlacementLoadDialogWidget>(GetOwningPlayer(), LoadDialogClass);
	if (LoadDialogInstance)
	{
		// 저장된 슬롯 목록 가져오기
		TArray<FString> SlotNames;
		if (PlacementComponent)
		{
			SlotNames = PlacementComponent->GetSavedLayoutSlots();
		}

		LoadDialogInstance->SetSlotList(SlotNames);
		LoadDialogInstance->SetContext(GetCurrentContext());

		// 콜백 바인딩
		LoadDialogInstance->OnLoadConfirmed.AddDynamic(this, &UPlacementHUDWidgetBase::OnLoadConfirmed);
		LoadDialogInstance->OnSlotDeleteRequested.AddDynamic(this, &UPlacementHUDWidgetBase::OnSlotDeleteRequested);
		LoadDialogInstance->ShowPopup();
	}
}

void UPlacementHUDWidgetBase::OnLoadConfirmed(const FString& SlotName)
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Load confirmed with slot name: %s"), *SlotName);
	ExecuteLoad(SlotName);
}

void UPlacementHUDWidgetBase::ExecuteLoad(const FString& SlotName)
{
	// 기본 구현: PlacementComponent의 LoadLayoutFromSlot 호출
	if (PlacementComponent)
	{
		if (PlacementComponent->LoadLayoutFromSlot(SlotName))
		{
			UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Layout loaded from slot '%s'"), *SlotName);
		}
		else
		{
			UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementHUDWidgetBase: Failed to load layout from slot '%s'"), *SlotName);
		}
	}
}

void UPlacementHUDWidgetBase::OnSlotDeleteRequested(const FString& SlotName)
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Slot delete requested: %s"), *SlotName);

	if (PlacementComponent)
	{
		PlacementComponent->DeleteLayoutSlot(SlotName);

		// 다이얼로그 내 슬롯 목록 갱신 (닫지 않음)
		if (LoadDialogInstance)
		{
			TArray<FString> UpdatedSlots = PlacementComponent->GetSavedLayoutSlots();
			LoadDialogInstance->SetSlotList(UpdatedSlots);
		}
	}
}

void UPlacementHUDWidgetBase::UpdateTotalPlacementCountText()
{
	if (!TotalPlacementCountText || !PlacementComponent)
	{
		return;
	}

	if (GetCurrentContext() != EPlacementContext::Lobby)
	{
		TotalPlacementCountText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	IWjWorldPlacementDataProvider* DataProvider = PlacementComponent->GetPlacementDataProvider();
	if (!Settings || !DataProvider)
	{
		return;
	}

	int32 CurrentCount = DataProvider->GetPlacedObjects().Num();
	int32 MaxCount = Settings->MaxTotalLobbyPlacedObjects;

	if (MaxCount > 0)
	{
		TotalPlacementCountText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), CurrentCount, MaxCount)));
		TotalPlacementCountText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		TotalPlacementCountText->SetText(FText::FromString(FString::Printf(TEXT("%d"), CurrentCount)));
		TotalPlacementCountText->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPlacementHUDWidgetBase::RefreshCatalogUI()
{
	if (PlacementComponent)
	{
		PopulateCatalog(PlacementComponent->GetCatalog());
	}
}

void UPlacementHUDWidgetBase::HandlePlacementPurchaseComplete(FName ObjectId, bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: 구매 완료 — %s"), *ObjectId.ToString());
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementHUDWidgetBase: 구매 실패 — %s"), *ObjectId.ToString());
	}
	RefreshCatalogUI();
}

void UPlacementHUDWidgetBase::HandleInventoryUpdated()
{
	RefreshCatalogUI();
}

void UPlacementHUDWidgetBase::OnClearClicked()
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Clear clicked"));

	if (!ConfirmDialogClass)
	{
		// 다이얼로그 클래스가 없으면 바로 실행
		OnClearConfirmed();
		return;
	}

	// 이미 열려있으면 스킵
	if (ConfirmDialogInstance && ConfirmDialogInstance->IsInViewport())
	{
		return;
	}

	ConfirmDialogInstance = CreateWidget<UConfirmDialogWidget>(GetOwningPlayer(), ConfirmDialogClass);
	if (ConfirmDialogInstance)
	{
		ConfirmDialogInstance->SetMessage(FText::FromString(TEXT("배치된 모든 오브젝트를 삭제합니다.\n이 작업은 되돌릴 수 없습니다.")));
		ConfirmDialogInstance->OnConfirmed.AddDynamic(this, &UPlacementHUDWidgetBase::OnClearConfirmed);
		ConfirmDialogInstance->ShowPopup();
	}
}

void UPlacementHUDWidgetBase::OnClearConfirmed()
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Clear confirmed"));

	if (PlacementComponent)
	{
		PlacementComponent->ClearAllPlacedObjects();
	}
}

FString UPlacementHUDWidgetBase::GetCurrentLoadedSlotName() const
{
	if (PlacementComponent)
	{
		return PlacementComponent->GetLoadedSlotName();
	}
	return FString();
}

void UPlacementHUDWidgetBase::OnAirModeChanged(bool bIsAirMode)
{
	bCachedAirMode = bIsAirMode;
	UpdateControlsHint();
}

void UPlacementHUDWidgetBase::OnSnapDegreesChanged(float NewSnapDegrees)
{
	CachedSnapDegrees = NewSnapDegrees;
	UpdateControlsHint();
}

void UPlacementHUDWidgetBase::UpdateControlsHint()
{
	// 스냅 각도 표시 (정수면 소수점 생략)
	FString SnapStr = (FMath::Fmod(CachedSnapDegrees, 1.f) == 0.f)
		? FString::Printf(TEXT("%.0f"), CachedSnapDegrees)
		: FString::Printf(TEXT("%.1f"), CachedSnapDegrees);

	if (bCachedAirMode)
	{
		SetControlsHintText(FText::FromString(FString::Printf(
			TEXT("LMB: 배치 | R: 회전 | T: 축 전환 | G: 각도(%s°) | DEL: 삭제 | Wheel: 높이 조절 | F: 공중모드 OFF | ESC: 나가기"),
			*SnapStr)));
	}
	else
	{
		SetControlsHintText(FText::FromString(FString::Printf(
			TEXT("LMB: 배치 | R: 회전 | T: 축 전환 | G: 각도(%s°) | DEL: 삭제 | F: 공중모드 | ESC: 나가기"),
			*SnapStr)));
	}
}

void UPlacementHUDWidgetBase::OnObjectSelected(FName ObjectId)
{
	if (CatalogScrollBox)
	{
		if (ObjectId.IsNone())
		{
			CatalogScrollBox->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			CatalogScrollBox->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

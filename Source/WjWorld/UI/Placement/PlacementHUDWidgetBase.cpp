// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Placement/PlacementHUDWidgetBase.h"
#include "UI/Placement/PlacementSaveDialogWidget.h"
#include "UI/Placement/PlacementLoadDialogWidget.h"
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
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
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
	SetControlsHintText(FText::FromString(TEXT("LMB: 배치 | R: 회전 | T: 축 전환 | G: 각도 전환 | DEL: 삭제 | F: 공중모드 | ESC: 나가기")));
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
	ButtonToObjectIdMap.Empty();
	BuyButtonToObjectIdMap.Empty();

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

		// ---- 행 컨테이너 (HorizontalBox) ----
		UHorizontalBox* Row = NewObject<UHorizontalBox>(this);

		// ---- 아이템 선택 버튼 ----
		UButton* ItemButton = NewObject<UButton>(this);
		UTextBlock* ItemText = NewObject<UTextBlock>(this);

		FString DisplayStr = Def.DisplayName.IsEmpty() ? Def.ObjectId.ToString() : Def.DisplayName.ToString();

		// 배치 수량 표시
		if (bIsLobby && PlacementComponent)
		{
			int32 PlacedCount = PlacementComponent->CountPlacedObjectsByType(Def.ObjectId);
			if (bIsPaidItem && Def.MaxPlacementCount > 0)
			{
				// 유료 아이템: 구매 수량(OwnedQty)이 설치 상한
				DisplayStr += FString::Printf(TEXT("  [%d/%d]"), PlacedCount, OwnedQty);
			}
			else if (Def.MaxPlacementCount > 0)
			{
				// 무료 아이템: MaxPlacementCount가 상한
				DisplayStr += FString::Printf(TEXT("  [%d/%d]"), PlacedCount, Def.MaxPlacementCount);
			}
		}

		ItemText->SetText(FText::FromString(DisplayStr));

		FSlateFontInfo FontInfo = ItemText->GetFont();
		FontInfo.Size = 14;
		ItemText->SetFont(FontInfo);

		// 미소유 아이템: 회색 처리
		if (bIsPaidItem && !bOwned)
		{
			ItemText->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)));
		}

		ItemButton->AddChild(ItemText);
		Row->AddChild(ItemButton);

		// 아이템 버튼 Fill (나머지 공간 차지)
		if (UHorizontalBoxSlot* ItemSlot = Cast<UHorizontalBoxSlot>(ItemButton->Slot))
		{
			ItemSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}

		ButtonToObjectIdMap.Add(ItemButton, Def.ObjectId);
		ItemButton->OnClicked.AddDynamic(this, &UPlacementHUDWidgetBase::OnCatalogButtonClicked);

		// ---- 구매 버튼 (추가 구매 가능한 유료 아이템) ----
		// 유료 + 상한: 소유 수 < 상한이면 추가 구매 가능
		// 유료 + 무상한(0): 미소유일 때만 1회 구매 (기존 호환)
		const bool bCanBuyMore = bIsPaidItem
			&& (Def.MaxPlacementCount > 0 ? OwnedQty < Def.MaxPlacementCount : !bOwned);
		if (bCanBuyMore)
		{
			UButton* BuyButton = NewObject<UButton>(this);
			UTextBlock* BuyText = NewObject<UTextBlock>(this);

			BuyText->SetText(FText::FromString(FString::Printf(TEXT("%d Coin"), Def.CoinPrice)));

			FSlateFontInfo BuyFont = BuyText->GetFont();
			BuyFont.Size = 12;
			BuyText->SetFont(BuyFont);

			BuyButton->AddChild(BuyText);
			Row->AddChild(BuyButton);

			// 구매 가능 여부: 재화 충분 + 전체 상한 미달
			bool bCanBuy = (CoinBalance >= Def.CoinPrice) && !bTotalLimitReached;

			BuyButton->SetIsEnabled(bCanBuy);

			BuyButtonToObjectIdMap.Add(BuyButton, Def.ObjectId);
			BuyButton->OnClicked.AddDynamic(this, &UPlacementHUDWidgetBase::OnBuyButtonClicked);
		}

		CatalogScrollBox->AddChild(Row);
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

void UPlacementHUDWidgetBase::OnCatalogButtonClicked()
{
	// 호버 상태인 버튼을 찾아 ObjectId 매핑 조회
	for (const auto& Pair : ButtonToObjectIdMap)
	{
		if (Pair.Key && Pair.Key->IsHovered())
		{
			OnCatalogItemClicked(Pair.Value);
			return;
		}
	}
}

void UPlacementHUDWidgetBase::OnCatalogItemClicked(FName ObjectId)
{
	if (!PlacementComponent)
	{
		return;
	}

	// SelectObject 내부에서 소유권 검증 (미소유 시 차단됨)
	PlacementComponent->SelectObject(ObjectId);
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Selected object %s"), *ObjectId.ToString());
}

void UPlacementHUDWidgetBase::OnBuyButtonClicked()
{
	for (const auto& Pair : BuyButtonToObjectIdMap)
	{
		if (Pair.Key && Pair.Key->IsHovered())
		{
			UGameInstance* GI = GetGameInstance();
			UWjWorldCurrencySubsystem* CurrencySub = GI ? GI->GetSubsystem<UWjWorldCurrencySubsystem>() : nullptr;
			if (CurrencySub)
			{
				CurrencySub->PurchasePlacementObject(Pair.Value);
				UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: 구매 시도 %s"), *Pair.Value.ToString());
			}
			return;
		}
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

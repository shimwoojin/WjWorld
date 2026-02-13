// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "Cosmetic/WjWorldCosmeticTypes.h"
#include "CosmeticMainWindow.generated.h"

class UButton;
class UScrollBox;
class UUniformGridPanel;
class UTextBlock;
class UCosmeticPreviewPanel;
class UCosmeticItemEntryWidget;
class UCurrencyBalanceWidget;
class UGemPackStoreWidget;
struct FCosmeticItemDefinition;

/**
 * 윈도우 모드
 */
UENUM(BlueprintType)
enum class ECosmeticWindowMode : uint8
{
	Shop		UMETA(DisplayName = "Shop"),
	Inventory	UMETA(DisplayName = "Inventory"),
};

/**
 * 코스메틱 메인 윈도우
 *
 * 상점/인벤토리 통합 윈도우
 * - 탭으로 상점/인벤토리 전환
 * - 슬롯별 아이템 필터링
 * - 4열 그리드 레이아웃
 * - 3D 캐릭터 프리뷰
 * - 구매/장착/해제 기능
 */
UCLASS()
class WJWORLD_API UCosmeticMainWindow : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 팝업 표시 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void ShowPopup();

	/** 팝업 닫기 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void ClosePopup();

protected:
	//~ 모드 탭 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ShopTabButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> InventoryTabButton;

	//~ 슬롯 탭 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TabHeadButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TabBodyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TabBackButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TabEffectButton;

	//~ 아이템 그리드
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ItemScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> ItemGridPanel;

	//~ 상세 정보
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SelectedItemNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SelectedItemDescriptionText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SelectedItemPriceText;

	//~ 잔액 표시 위젯
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCurrencyBalanceWidget> CurrencyBalanceWidget;

	//~ 프리뷰 패널
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCosmeticPreviewPanel> PreviewPanel;

	//~ 액션 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ActionButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ActionButtonText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	//~ Gem 충전 버튼
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> GemChargeButton;

	//~ 엔트리 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCosmeticItemEntryWidget> ItemEntryWidgetClass;

	/** Gem 팩 상점 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGemPackStoreWidget> GemPackStoreWidgetClass;

protected:
	//~ 버튼 클릭 핸들러

	UFUNCTION()
	void OnShopTabClicked();

	UFUNCTION()
	void OnInventoryTabClicked();

	UFUNCTION()
	void OnTabHeadClicked();

	UFUNCTION()
	void OnTabBodyClicked();

	UFUNCTION()
	void OnTabBackClicked();

	UFUNCTION()
	void OnTabEffectClicked();

	UFUNCTION()
	void OnActionButtonClicked();

	UFUNCTION()
	void OnCloseClicked();

	UFUNCTION()
	void OnGemChargeClicked();

	UFUNCTION()
	void OnGemAreaClicked();

	//~ 아이템 클릭 핸들러
	UFUNCTION()
	void OnItemClicked(FName ItemId);

	//~ 서브시스템 델리게이트 핸들러
	UFUNCTION()
	void OnInventoryUpdated();

	UFUNCTION()
	void OnLoadoutChanged(ECosmeticSlot CosmeticSlot, FName ItemId);

	UFUNCTION()
	void OnPurchaseComplete(FName ItemId, bool bSuccess);

	UFUNCTION()
	void OnCurrencyPurchaseComplete(FName ItemId, bool bSuccess);

private:
	//~ UI 갱신 함수

	/** 현재 모드로 UI 갱신 */
	void RefreshUI();

	/** 아이템 그리드 갱신 */
	void RefreshItemGrid();

	/** 상세 정보 갱신 */
	void RefreshDetailInfo();

	/** 액션 버튼 갱신 */
	void RefreshActionButton();

	/** 탭 버튼 스타일 갱신 */
	void UpdateTabStyles();

	/** 모드 전환 */
	void SetMode(ECosmeticWindowMode NewMode);

	/** 슬롯 전환 */
	void SetSlot(ECosmeticSlot NewSlot);

	/** 아이템 선택 */
	void SelectItem(FName ItemId);

	/** 상점 모드: 아이템 그리드 채우기 */
	void PopulateShopItems();

	/** 인벤토리 모드: 아이템 그리드 채우기 */
	void PopulateInventoryItems();

	/** 아이템 엔트리 위젯 생성 */
	UCosmeticItemEntryWidget* CreateItemEntry(const FCosmeticItemDefinition& Def);

	/** 그리드 클리어 */
	void ClearItemGrid();

	/** Gem 팩 상점 팝업 열기 */
	void OpenGemPackStore();

	//~ 상태

	/** Gem 팩 상점 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<UGemPackStoreWidget> GemPackStoreInstance;

	/** 현재 모드 */
	ECosmeticWindowMode CurrentMode = ECosmeticWindowMode::Shop;

	/** 현재 슬롯 */
	ECosmeticSlot CurrentSlot = ECosmeticSlot::Head;

	/** 선택된 아이템 ID */
	FName SelectedItemId;

	/** 생성된 엔트리 위젯 목록 */
	UPROPERTY()
	TArray<TObjectPtr<UCosmeticItemEntryWidget>> ItemEntryWidgets;

	/** 그리드 열 수 */
	static constexpr int32 GridColumnCount = 4;
};

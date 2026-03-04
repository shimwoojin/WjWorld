// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "PlacementCatalogItemWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCatalogItemSelected, FName, ObjectId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCatalogItemBuyClicked, FName, ObjectId);

/**
 * 배치 카탈로그 아이템 위젯
 * ScrollBox에 추가되는 개별 아이템 엔트리
 */
UCLASS(Abstract)
class WJWORLD_API UPlacementCatalogItemWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** 아이템 데이터 설정 */
	void SetItemData(FName InObjectId, const FText& InDisplayName, UTexture2D* InIcon);

	/** 배치 수량 표시 (예: "2/5") */
	void SetPlacementCount(int32 PlacedCount, int32 MaxCount);

	/** 구매 버튼 표시/숨김 및 가격 설정 */
	void SetBuyInfo(bool bShowBuy, int32 CoinPrice, bool bCanAfford);

	/** 소유 상태 설정 (미소유 시 회색 처리) */
	void SetOwned(bool bOwned);

	FOnCatalogItemSelected OnItemSelected;
	FOnCatalogItemBuyClicked OnBuyClicked;

protected:
	/** 아이템 선택 버튼 (전체 행 클릭 영역) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ItemButton;

	/** 아이템 이름 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;

	/** 아이템 아이콘 (선택적) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ItemIcon;

	/** 배치 수량 텍스트 (선택적, 예: "2/5") */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlacementCountText;

	/** 구매 버튼 (선택적) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BuyButton;

	/** 구매 가격 텍스트 (선택적) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BuyPriceText;

private:
	UFUNCTION()
	void HandleItemClicked();

	UFUNCTION()
	void HandleBuyClicked();

	FName ObjectId;
};

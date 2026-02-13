// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "Cosmetic/WjWorldCosmeticTypes.h"
#include "Engine/StreamableManager.h"
#include "CosmeticItemEntryWidget.generated.h"

class UImage;
class UTextBlock;
class UBorder;
class UButton;
struct FCosmeticItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCosmeticItemClicked, FName, ItemId);

/**
 * 코스메틱 아이템 엔트리 위젯
 *
 * 상점/인벤토리에서 개별 아이템을 표시하는 재사용 위젯
 * - 아이콘, 이름, 희귀도 바, 가격 표시
 * - 보유/장착/선택 상태 시각화
 */
UCLASS()
class WJWORLD_API UCosmeticItemEntryWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 아이템 정의 설정 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void SetItemDefinition(const FCosmeticItemDefinition& Def);

	/** 보유 상태 설정 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void SetOwned(bool bOwned);

	/** 장착 상태 설정 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void SetEquipped(bool bEquipped);

	/** 선택 상태 설정 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void SetSelected(bool bSelected);

	/** 아이템 ID 반환 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	FName GetItemId() const { return CachedItemId; }

	/** 아이템 클릭 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Cosmetic")
	FOnCosmeticItemClicked OnItemClicked;

protected:
	/** 아이콘 이미지 (128x128) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	/** 아이템 이름 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;

	/** 희귀도 색상 바 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> RarityBar;

	/** 가격 또는 "보유중" 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PriceText;

	/** 선택 하이라이트 테두리 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> SelectionBorder;

	/** 장착 표시 아이콘 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> EquippedIcon;

	/** 클릭 감지 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ItemButton;

	UFUNCTION()
	void OnItemButtonClicked();

private:
	/** 희귀도별 색상 반환 */
	FLinearColor GetRarityColor(ECosmeticRarity Rarity) const;

	/** 아이콘 로드 완료 콜백 */
	void OnIconLoaded();

	/** 캐시된 아이템 ID */
	FName CachedItemId;

	/** 캐시된 Coin 가격 */
	int32 CachedCoinPrice = 0;

	/** 캐시된 Gem 가격 */
	int32 CachedGemPrice = 0;

	/** 보유 여부 */
	bool bIsOwned = false;

	/** 장착 여부 */
	bool bIsEquipped = false;

	/** 선택 여부 */
	bool bIsSelected = false;

	/** 비동기 로드 핸들 */
	FStreamableManager StreamableManager;
	TSharedPtr<FStreamableHandle> IconLoadHandle;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "Currency/WjWorldCurrencyTypes.h"
#include "GemPackEntryWidget.generated.h"

class UTextBlock;
class UButton;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGemPackBuyClicked, int32, PackDefId);

/**
 * Gem 팩 상점의 개별 팩 엔트리 위젯
 */
UCLASS()
class WJWORLD_API UGemPackEntryWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** 팩 정의 설정 */
	void SetPackDefinition(const FGemPackDefinition& InDef);

	/** 구매 버튼 클릭 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnGemPackBuyClicked OnBuyClicked;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PackNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GemAmountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PriceText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BuyButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> GemIcon;

private:
	UFUNCTION()
	void OnBuyButtonClicked();

	int32 CachedDefId = 0;
};

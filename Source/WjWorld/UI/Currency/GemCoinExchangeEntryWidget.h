// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "Currency/WjWorldCurrencyTypes.h"
#include "GemCoinExchangeEntryWidget.generated.h"

class UTextBlock;
class UButton;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGemCoinExchangeClicked, int32, ExchangeDefId);

/**
 * Gem -> Coin 교환 엔트리 위젯
 */
UCLASS()
class WJWORLD_API UGemCoinExchangeEntryWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** 교환 정의 설정 */
	void SetExchangeDefinition(const FGemCoinExchangeDefinition& InDef);

	/** Gem 잔액에 따라 버튼 활성화/비활성화 갱신 */
	void UpdateButtonState(int32 CurrentGemBalance);

	/** 교환 버튼 클릭 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnGemCoinExchangeClicked OnExchangeClicked;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DisplayNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GemCostText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExchangeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CoinIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> GemIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BonusText;

private:
	UFUNCTION()
	void OnExchangeButtonClicked();

	int32 CachedDefId = 0;
	int32 CachedGemCost = 0;
};

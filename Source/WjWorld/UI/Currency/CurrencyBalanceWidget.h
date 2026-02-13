// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "Currency/WjWorldCurrencyTypes.h"
#include "CurrencyBalanceWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGemAreaClicked);

/**
 * Coin/Gem 잔액을 표시하는 재사용 가능한 위젯
 */
UCLASS()
class WJWORLD_API UCurrencyBalanceWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Gem 영역 클릭 시 브로드캐스트 */
	UPROPERTY(BlueprintAssignable)
	FOnGemAreaClicked OnGemAreaClicked;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CoinAmountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GemAmountText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CoinIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> GemIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> GemAreaButton;

private:
	UFUNCTION()
	void OnCurrencyBalanceChanged(ECurrencyType CurrencyType, int32 NewBalance);

	UFUNCTION()
	void OnGemAreaButtonClicked();

	/** 양쪽 잔액 텍스트 갱신 */
	void UpdateDisplay();
};

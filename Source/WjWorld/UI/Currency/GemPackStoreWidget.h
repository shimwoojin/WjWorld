// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "GemPackStoreWidget.generated.h"

class UVerticalBox;
class UButton;
class UTextBlock;
class UCurrencyBalanceWidget;
class UGemPackEntryWidget;
class UGemCoinExchangeEntryWidget;

/**
 * Gem 팩 상점 팝업 위젯
 * DeveloperSettings의 GemPackDefinitions를 기반으로 팩 목록을 표시
 */
UCLASS()
class WJWORLD_API UGemPackStoreWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 팝업 표시 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void ShowPopup();

	/** 팝업 닫기 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void ClosePopup();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PackListBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCurrencyBalanceWidget> CurrencyBalanceWidget;

	/** 팩 엔트리 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGemPackEntryWidget> PackEntryWidgetClass;

	// ---- Gem → Coin 교환 섹션 ----

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> ExchangeListBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ExchangeSectionText;

	/** 교환 엔트리 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGemCoinExchangeEntryWidget> ExchangeEntryWidgetClass;

private:
	UFUNCTION()
	void OnCloseClicked();

	UFUNCTION()
	void OnPackPurchaseClicked(int32 PackDefId);

	UFUNCTION()
	void OnExchangeClicked(int32 ExchangeDefId);

	UFUNCTION()
	void OnGemBalanceChanged(ECurrencyType CurrencyType, int32 NewBalance);

	/** 팩 목록 생성 */
	void PopulatePackList();

	/** 팩 목록 클리어 */
	void ClearPackList();

	/** 교환 목록 생성 */
	void PopulateExchangeList();

	/** 교환 목록 클리어 */
	void ClearExchangeList();

	/** 교환 엔트리 버튼 상태 갱신 */
	void UpdateExchangeButtonStates();

	UPROPERTY()
	TArray<TObjectPtr<UGemPackEntryWidget>> PackEntryWidgets;

	UPROPERTY()
	TArray<TObjectPtr<UGemCoinExchangeEntryWidget>> ExchangeEntryWidgets;
};

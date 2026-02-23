// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "ConfirmDialogWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfirmDialogConfirmed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfirmDialogCancelled);

/**
 * 공용 확인/취소 다이얼로그 위젯
 * 메시지 + 확인/취소 버튼으로 구성된 범용 팝업
 */
UCLASS()
class WJWORLD_API UConfirmDialogWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	/** 팝업 표시 */
	void ShowPopup();

	/** 팝업 닫기 */
	void ClosePopup();

	/** 메시지 텍스트 설정 */
	void SetMessage(const FText& InMessage);

	/** 확인/취소 버튼 텍스트 커스텀 (기본: 확인/취소) */
	void SetButtonLabels(const FText& ConfirmLabel, const FText& CancelLabel);

	/** 확인 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnConfirmDialogConfirmed OnConfirmed;

	/** 취소 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnConfirmDialogCancelled OnCancelled;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MessageText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ConfirmButtonText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CancelButtonText;

private:
	UFUNCTION()
	void OnConfirmClicked();

	UFUNCTION()
	void OnCancelClicked();

	/** NativeConstruct 전 호출 대비 캐시 */
	FText CachedMessage;
	FText CachedConfirmLabel;
	FText CachedCancelLabel;
	bool bMessageSet = false;
	bool bLabelsSet = false;
};

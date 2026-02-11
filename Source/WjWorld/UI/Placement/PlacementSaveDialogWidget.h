// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "GamePlay/Placement/WjWorldPlacementTypes.h"
#include "PlacementSaveDialogWidget.generated.h"

class UButton;
class UTextBlock;
class UEditableTextBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlacementSaveConfirmed, const FString&, SlotName);

/**
 * 배치 저장 다이얼로그 위젯
 * 슬롯 이름 입력 및 유효성 검사 결과 표시
 */
UCLASS()
class WJWORLD_API UPlacementSaveDialogWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** 팝업 표시 */
	void ShowPopup();

	/** 팝업 닫기 */
	void ClosePopup();

	/** 기본 슬롯 이름 설정 */
	void SetDefaultSlotName(const FString& SlotName);

	/** 컨텍스트 설정 (AW의 경우 유효성 검사 표시) */
	void SetContext(EPlacementContext InContext);

	/** 유효성 검사 결과 설정 */
	void SetValidationResult(bool bIsValid, const FText& Message);

	/** 저장 확인 시 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Placement")
	FOnPlacementSaveConfirmed OnSaveConfirmed;

protected:
	/** 슬롯 이름 입력 텍스트박스 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> SlotNameTextBox;

	/** 확인 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButton;

	/** 취소 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;

	/** 타이틀 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	/** 유효성 검사 결과 텍스트 (AW용) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ValidationText;

	/** 유효성 검사 아이콘/상태 표시 (선택적) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ValidationStatusText;

private:
	UFUNCTION()
	void OnConfirmClicked();

	UFUNCTION()
	void OnCancelClicked();

	UFUNCTION()
	void OnSlotNameChanged(const FText& Text);

	/** 현재 컨텍스트 */
	EPlacementContext CurrentContext = EPlacementContext::Lobby;

	/** SetContext가 호출되었는지 */
	bool bContextSet = false;

	/** SetValidationResult가 호출되었는지 */
	bool bValidationResultSet = false;

	/** 현재 유효성 상태 */
	bool bIsCurrentlyValid = true;

	/** 캐시된 유효성 메시지 (NativeConstruct 이전 호출 대응) */
	FText CachedValidationMessage;
};

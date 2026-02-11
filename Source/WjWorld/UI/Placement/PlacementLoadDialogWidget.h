// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "GamePlay/Placement/WjWorldPlacementTypes.h"
#include "PlacementLoadDialogWidget.generated.h"

class UButton;
class UTextBlock;
class UScrollBox;
class UHorizontalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlacementLoadConfirmed, const FString&, SlotName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlacementSlotDeleteRequested, const FString&, SlotName);

/**
 * 배치 레이아웃 불러오기 다이얼로그 위젯
 * 저장된 슬롯 목록 표시 및 선택
 */
UCLASS()
class WJWORLD_API UPlacementLoadDialogWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** 팝업 표시 */
	void ShowPopup();

	/** 팝업 닫기 */
	void ClosePopup();

	/** 슬롯 목록 설정 */
	void SetSlotList(const TArray<FString>& SlotNames);

	/** 컨텍스트 설정 */
	void SetContext(EPlacementContext InContext);

	/** 불러오기 확인 시 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Placement")
	FOnPlacementLoadConfirmed OnLoadConfirmed;

	/** 슬롯 삭제 요청 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Placement")
	FOnPlacementSlotDeleteRequested OnSlotDeleteRequested;

protected:
	/** 슬롯 목록 스크롤박스 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> SlotListScrollBox;

	/** 취소 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;

	/** 타이틀 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	/** 안내 텍스트 (슬롯이 없을 때) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EmptyHintText;

private:
	UFUNCTION()
	void OnCancelClicked();

	UFUNCTION()
	void OnSlotButtonClicked();

	UFUNCTION()
	void OnDeleteButtonClicked();

	/** 현재 컨텍스트 */
	EPlacementContext CurrentContext = EPlacementContext::Lobby;

	/** 슬롯 버튼 → 슬롯 이름 매핑 */
	UPROPERTY()
	TMap<TObjectPtr<UButton>, FString> ButtonToSlotNameMap;

	/** 삭제 버튼 → 슬롯 이름 매핑 */
	UPROPERTY()
	TMap<TObjectPtr<UButton>, FString> DeleteButtonToSlotNameMap;
};

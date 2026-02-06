// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "GamePlay/Placement/WjWorldPlacementTypes.h"
#include "PlacementContextSelectWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlacementContextSelected, EPlacementContext, SelectedContext);

/**
 * 배치 컨텍스트 선택 팝업 위젯
 * 로비에서 Lobby / ApproachingWall / JumpMap 중 선택
 */
UCLASS()
class WJWORLD_API UPlacementContextSelectWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** 팝업 표시 */
	void ShowPopup();

	/** 팝업 닫기 */
	void ClosePopup();

	/** 컨텍스트 선택 시 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Placement")
	FOnPlacementContextSelected OnContextSelected;

protected:
	/** 로비 배치 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LobbyButton;

	/** Approaching Wall 에디터 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AWEditorButton;

	/** JumpMap 에디터 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JumpMapEditorButton;

	/** 닫기 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	/** 타이틀 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	/** 로비 버튼 설명 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LobbyDescText;

	/** AW 버튼 설명 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AWDescText;

	/** JumpMap 버튼 설명 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> JumpMapDescText;

private:
	UFUNCTION()
	void OnLobbyClicked();

	UFUNCTION()
	void OnAWEditorClicked();

	UFUNCTION()
	void OnJumpMapEditorClicked();

	UFUNCTION()
	void OnCloseClicked();

	/** 버튼 활성화 상태 업데이트 (에디터 맵 설정 여부에 따라) */
	void UpdateButtonStates();
};

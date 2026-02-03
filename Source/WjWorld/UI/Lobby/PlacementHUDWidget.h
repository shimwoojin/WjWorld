// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "PlacementHUDWidget.generated.h"

class UWjWorldPlacementComponent;
class UWjWorldPlaceableObjectDataAsset;
class UScrollBox;
class UButton;
class UTextBlock;

/**
 * 배치 모드 전용 HUD 위젯
 * 카탈로그 표시, 삭제 모드 토글, 조작 안내
 */
UCLASS()
class WJWORLD_API UPlacementHUDWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** PlacementComponent 참조 설정 및 카탈로그 표시 */
	void SetPlacementComponent(UWjWorldPlacementComponent* InComponent);

protected:
	/** 카탈로그 스크롤 박스 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> CatalogScrollBox;

	/** 나가기 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

	/** 삭제 모드 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DeleteModeButton;

	/** 조작 안내 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ControlsHintText;

private:
	/** 카탈로그 아이템 목록 채우기 */
	void PopulateCatalog(UWjWorldPlaceableObjectDataAsset* Catalog);

	UFUNCTION()
	void OnExitClicked();

	UFUNCTION()
	void OnDeleteModeClicked();

	/** 카탈로그 버튼 클릭 공통 핸들러 */
	UFUNCTION()
	void OnCatalogButtonClicked();

	/** 카탈로그 항목 클릭 시 호출 */
	void OnCatalogItemClicked(FName ObjectId);

	UPROPERTY()
	TObjectPtr<UWjWorldPlacementComponent> PlacementComponent;

	/** 버튼 → ObjectId 매핑 */
	UPROPERTY()
	TMap<TObjectPtr<UButton>, FName> ButtonToObjectIdMap;
};

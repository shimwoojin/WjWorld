// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Placement/PlacementHUDWidgetBase.h"
#include "PlacementHUDWidgetAWEditor.generated.h"

/**
 * Approaching Wall 에디터 배치 HUD 위젯
 * 나가기 시 로비로 복귀
 * 저장 시 배치된 오브젝트를 WallLayout CSV로 변환
 */
UCLASS()
class WJWORLD_API UPlacementHUDWidgetAWEditor : public UPlacementHUDWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** 벽돌 크기 (그리드 셀 크기) */
	UPROPERTY(EditDefaultsOnly, Category = "AW Editor")
	FVector BrickSize = FVector(100.0, 100.0, 100.0);

protected:
	/** 나가기 버튼 클릭 - 로비로 복귀 */
	virtual void OnExitClicked() override;

	/** 저장 버튼 클릭 - 유효성 검사 후 CSV 저장 */
	virtual void OnSaveClicked() override;

	/** 실제 저장 로직 - WallLayout CSV 파일로 저장 */
	virtual void ExecuteSave(const FString& SlotName) override;

private:
	/** 현재 변환된 WallLayout (저장 전 캐시) */
	TArray<TArray<int32>> CachedWallLayout;

	/** 현재 레이아웃의 유효성 */
	bool bIsCurrentLayoutValid = false;

	/** 현재 레이아웃의 행/열 수 */
	int32 CachedRowNum = 0;
	int32 CachedColumnNum = 0;

	/** 현재 레이아웃의 중심 오프셋 */
	FVector CachedCenterOffset = FVector::ZeroVector;

	/** 배치된 오브젝트들을 WallLayout으로 변환하고 유효성 검사 */
	bool ConvertAndValidateLayout(FText& OutMessage);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Placement/PlacementHUDWidgetBase.h"
#include "PlacementHUDWidgetJumpMapEditor.generated.h"

/**
 * JumpMap 에디터 배치 HUD 위젯
 * 나가기 시 로비로 복귀
 */
UCLASS()
class WJWORLD_API UPlacementHUDWidgetJumpMapEditor : public UPlacementHUDWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	/** 나가기 버튼 클릭 - 로비로 복귀 */
	virtual void OnExitClicked() override;

	/** 저장 시 JumpMap 레이아웃 검증 후 저장 */
	virtual void ExecuteSave(const FString& SlotName) override;
};

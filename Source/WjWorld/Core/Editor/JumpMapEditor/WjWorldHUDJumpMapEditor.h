// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldHUDBase.h"
#include "WjWorldHUDJumpMapEditor.generated.h"

class UPlacementHUDWidgetJumpMapEditor;
class UWjWorldPlacementComponent;

/**
 * JumpMap 에디터 HUD
 * 배치 모드 UI 표시
 */
UCLASS()
class WJWORLD_API AWjWorldHUDJumpMapEditor : public AWjWorldHUDBase
{
	GENERATED_BODY()

public:
	AWjWorldHUDJumpMapEditor();

protected:
	virtual void BeginPlay() override;

	/** HUD 위젯 클래스 (BP에서 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPlacementHUDWidgetJumpMapEditor> HUDWidgetClass;

	/** 생성된 HUD 위젯 */
	UPROPERTY()
	TObjectPtr<UPlacementHUDWidgetJumpMapEditor> HUDWidget;

	/** HUD 위젯 생성 및 PlacementComponent 연결 */
	void CreateHUDWidget();
};

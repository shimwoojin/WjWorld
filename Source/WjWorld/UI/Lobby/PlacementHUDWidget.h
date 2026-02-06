// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Placement/PlacementHUDWidgetBase.h"
#include "PlacementHUDWidget.generated.h"

/**
 * 로비 배치 모드 전용 HUD 위젯
 * PlacementHUDWidgetBase를 상속받아 로비 컨텍스트에 맞게 커스터마이징
 *
 * 기능:
 * - 카탈로그 표시, 삭제 모드 토글, 조작 안내
 * - 나가기 시 GameModeLobby로 배치 모드 종료 요청
 */
UCLASS()
class WJWORLD_API UPlacementHUDWidget : public UPlacementHUDWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	/** 나가기 버튼 클릭 - 로비 GameMode로 배치 모드 종료 요청 */
	virtual void OnExitClicked() override;
};

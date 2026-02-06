// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldGameModeBase.h"
#include "WjWorldGameModeAWEditor.generated.h"

/**
 * Approaching Wall 에디터 게임 모드
 * 벽 레이아웃 및 맵 데코레이션 배치 에디터
 * 싱글플레이어 전용
 */
UCLASS()
class WJWORLD_API AWjWorldGameModeAWEditor : public AWjWorldGameModeBase
{
	GENERATED_BODY()

public:
	AWjWorldGameModeAWEditor();

protected:
	virtual void BeginPlay() override;
};

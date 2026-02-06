// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldGameModeBase.h"
#include "WjWorldGameModeJumpMapEditor.generated.h"

/**
 * JumpMap 에디터 게임 모드
 * 플랫폼, 장애물, 함정 배치 에디터
 * 싱글플레이어 전용
 */
UCLASS()
class WJWORLD_API AWjWorldGameModeJumpMapEditor : public AWjWorldGameModeBase
{
	GENERATED_BODY()

public:
	AWjWorldGameModeJumpMapEditor();

protected:
	virtual void BeginPlay() override;
};

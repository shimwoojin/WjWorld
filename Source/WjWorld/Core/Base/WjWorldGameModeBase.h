// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WjWorldGameModeBase.generated.h"

/**
 * 기본 게임 모드 클래스
 * 모든 게임 모드의 Base 클래스
 */
UCLASS(abstract)
class AWjWorldGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AWjWorldGameModeBase();
};

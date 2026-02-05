// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_SumoSuperPush.generated.h"

/**
 * Sumo SuperPush GameplayEffect (참조/확장용)
 * - Infinite duration
 * - 실제 버프 적용은 SumoPowerUpActor에서 AddLooseGameplayTag()로 처리
 * - GA_Push에서 Buff.SuperPush 태그 체크 후 제거
 */
UCLASS()
class WJWORLD_API UGE_SumoSuperPush : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_SumoSuperPush();
};

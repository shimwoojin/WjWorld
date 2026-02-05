// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_SumoSpeedBoost.generated.h"

/**
 * Sumo SpeedBoost GameplayEffect (참조/확장용)
 * - HasDuration: 5초
 * - 실제 버프 적용은 SumoPowerUpActor에서 AddLooseGameplayTag()로 처리
 * - BP에서 이 GE를 확장하여 추가 Modifier 설정 가능
 */
UCLASS()
class WJWORLD_API UGE_SumoSpeedBoost : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_SumoSpeedBoost();
};

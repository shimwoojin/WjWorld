// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_SumoShield.generated.h"

/**
 * Sumo Shield GameplayEffect (참조/확장용)
 * - Infinite duration
 * - 실제 버프 적용은 SumoPowerUpActor에서 AddLooseGameplayTag()로 처리
 * - CharacterPlay::OnEliminated()에서 Buff.Shield 태그 체크 후 제거
 */
UCLASS()
class WJWORLD_API UGE_SumoShield : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_SumoShield();
};

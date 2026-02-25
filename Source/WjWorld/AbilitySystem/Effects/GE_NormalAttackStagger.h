// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_NormalAttackStagger.generated.h"

/**
 * NormalAttack 피격 시 1초 경직 GameplayEffect
 * - HasDuration (1초)
 * - State.Staggered 태그 부여 → 이동 + 어빌리티 차단
 */
UCLASS()
class WJWORLD_API UGE_NormalAttackStagger : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_NormalAttackStagger();
};

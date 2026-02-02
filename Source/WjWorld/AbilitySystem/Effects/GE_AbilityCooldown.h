// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_AbilityCooldown.generated.h"

/**
 * 범용 어빌리티 쿨다운 GameplayEffect
 * - Duration 기반 (HasDuration)
 * - SetByCaller로 쿨다운 시간 설정 (Data.Cooldown 태그)
 * - 쿨다운 태그는 DynamicGrantedTags로 런타임에 추가
 */
UCLASS()
class WJWORLD_API UGE_AbilityCooldown : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_AbilityCooldown();
};

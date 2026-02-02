// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WjWorldGameplayAbilityBase.h"
#include "GamePlay/Wall/WjWorldWallDescriptionDataAsset.h"
#include "GA_NormalAttack.generated.h"

/**
 * 바라보는 방향의 벽돌을 공격(파괴)하는 어빌리티
 * - Yaw 기반 4방향 스냅으로 타겟 위치 계산
 * - 벽돌 타입별 처리: Standard(파괴 불가), Explosive(폭발), Moving/Destructible(파괴)
 * - 단발성: ActivateAbility → 공격 처리 → 즉시 EndAbility
 */
UCLASS(Abstract)
class WJWORLD_API UGA_NormalAttack : public UWjWorldGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_NormalAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	FVector CalculateTargetLocation() const;

	FWjWorldWallDescription CachedWallDesc;
};

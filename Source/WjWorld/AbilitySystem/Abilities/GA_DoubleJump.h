// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GA_Jump.h"
#include "GA_DoubleJump.generated.h"

/**
 * 이중 점프 어빌리티 (JumpMap용)
 * - GA_Jump 확장
 * - 공중에서 한 번 더 점프 가능
 * - 착지 시 카운트 리셋
 */
UCLASS(Abstract)
class WJWORLD_API UGA_DoubleJump : public UGA_Jump
{
	GENERATED_BODY()

public:
	UGA_DoubleJump();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** 최대 점프 횟수 (1 = 일반 점프, 2 = 이중 점프) */
	UPROPERTY(EditDefaultsOnly, Category = "DoubleJump")
	int32 MaxJumpCount = 2;

private:
	mutable int32 CurrentJumpCount = 0;
};

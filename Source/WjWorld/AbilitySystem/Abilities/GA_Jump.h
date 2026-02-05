// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WjWorldGameplayAbilityBase.h"
#include "GA_Jump.generated.h"

/**
 * 점프 어빌리티
 * - UE 기본 ACharacter::Jump() 기반
 * - AllowedAbilityTags로 미니게임별 활성화 제어 (Sumo 전용)
 * - 입력 해제 시 StopJumping() 호출 (가변 점프 높이 지원)
 * - 쿨다운 지원 (BP에서 설정)
 */
UCLASS(Abstract)
class WJWORLD_API UGA_Jump : public UWjWorldGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_Jump();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
};

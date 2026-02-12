// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WjWorldGameplayAbilityBase.h"
#include "GA_ToggleCameraView.generated.h"

/**
 * 카메라 전환 어빌리티
 * - 3인칭 ↔ TopDown 즉시 전환
 * - 쿨다운 없음
 */
UCLASS(Abstract)
class WJWORLD_API UGA_ToggleCameraView : public UWjWorldGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_ToggleCameraView();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};

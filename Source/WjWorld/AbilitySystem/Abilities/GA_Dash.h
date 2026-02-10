// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WjWorldGameplayAbilityBase.h"
#include "GA_Dash.generated.h"

/**
 * 전방 대시 어빌리티 (JumpMap용)
 * - 전방으로 짧은 거리 빠르게 이동
 * - LaunchCharacter 기반
 * - 쿨다운 지원
 */
UCLASS(Abstract)
class WJWORLD_API UGA_Dash : public UWjWorldGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_Dash();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** 대시 거리 */
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashDistance = 600.f;

	/** 대시 지속 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashDuration = 0.2f;

	/** 대시 쿨다운 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashCooldownDuration = 2.f;

private:
	void OnDashEnd();

	// EndAbility 호출용 캐시
	FGameplayAbilitySpecHandle CachedHandle;
	const FGameplayAbilityActorInfo* CachedActorInfo = nullptr;
	FGameplayAbilityActivationInfo CachedActivationInfo;
	FTimerHandle DashTimerHandle;
};

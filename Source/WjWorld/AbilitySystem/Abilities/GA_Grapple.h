// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WjWorldGameplayAbilityBase.h"
#include "GA_Grapple.generated.h"

class AJumpMapGrapplePointActor;

/**
 * 그래플 어빌리티 (JumpMap용)
 * - 카메라 방향으로 라인 트레이스 → GrapplePoint 감지
 * - 히트 시 포인트까지 당김 이동
 * - 미스 시 쿨다운 없이 종료
 */
UCLASS(Abstract)
class WJWORLD_API UGA_Grapple : public UWjWorldGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_Grapple();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** 그래플 감지 범위 */
	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
	float GrappleRange = 2000.f;

	/** 당김 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
	float GrapplePullSpeed = 1500.f;

	/** 그래플 쿨다운 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
	float GrappleCooldownDuration = 3.f;

	/** 도착 판정 거리 */
	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
	float ArrivalThreshold = 100.f;

private:
	/** 도착 체크 (타이머 기반) */
	void CheckArrival();

	bool bIsPulling = false;
	FVector GrappleTargetLocation;
	FTimerHandle ArrivalCheckTimerHandle;

	// EndAbility 호출용 캐시
	FGameplayAbilitySpecHandle CachedHandle;
	const FGameplayAbilityActorInfo* CachedActorInfo = nullptr;
	FGameplayAbilityActivationInfo CachedActivationInfo;
};

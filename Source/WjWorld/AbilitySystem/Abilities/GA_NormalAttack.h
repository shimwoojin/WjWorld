// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WjWorldGameplayAbilityBase.h"
#include "GamePlay/Wall/WjWorldWallDescriptionDataAsset.h"
#include "GA_NormalAttack.generated.h"

class UAnimMontage;

/**
 * 바라보는 방향의 벽돌을 공격(파괴)하는 어빌리티
 * - Yaw 기반 4방향 스냅으로 타겟 위치 계산
 * - 벽돌 타입별 처리: Standard(파괴 불가), Explosive(폭발), Moving/Destructible(파괴)
 * - 공격 로직 즉시 실행 + Montage 시각 피드백 (Montage 없어도 동작)
 */
UCLASS(Abstract)
class WJWORLD_API UGA_NormalAttack : public UWjWorldGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_NormalAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** 공격 애니메이션 몽타주 (BP에서 설정, 없으면 몽타주 없이 즉시 공격) */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** 몽타주 재생 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float MontagePlayRate = 1.5f;

private:
	/** 실제 공격 처리 로직 (서버에서만 실행) */
	void ExecuteAttack(const FGameplayAbilityActivationInfo& ActivationInfo);

	FVector CalculateTargetLocation() const;

	// Montage 콜백
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();

	FWjWorldWallDescription CachedWallDesc;

	// EndAbility 호출용 캐시
	FGameplayAbilitySpecHandle CachedHandle;
	const FGameplayAbilityActorInfo* CachedActorInfo = nullptr;
	FGameplayAbilityActivationInfo CachedActivationInfo;
};

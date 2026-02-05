// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WjWorldGameplayAbilityBase.h"
#include "GA_Push.generated.h"

class UAnimMontage;

/**
 * 전방으로 밀치기 어빌리티 (Sumo 미니게임용)
 * - 전방 구형 오버랩으로 히트 캐릭터 감지
 * - LaunchCharacter()로 넉백 적용
 * - SetLastAttacker() 호출하여 킬 추적
 */
UCLASS(Abstract)
class WJWORLD_API UGA_Push : public UWjWorldGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_Push();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** 밀치기 힘 */
	UPROPERTY(EditDefaultsOnly, Category = "Push")
	float PushForce = 1200.f;

	/** 밀치기 감지 범위 (구형 오버랩 반지름) */
	UPROPERTY(EditDefaultsOnly, Category = "Push")
	float PushRange = 300.f;

	/** 위쪽으로 추가 힘 (공중에 살짝 띄우기) */
	UPROPERTY(EditDefaultsOnly, Category = "Push")
	float PushUpForce = 400.f;

	/** 공격 애니메이션 몽타주 (BP에서 설정, 없으면 즉시 실행) */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> PushMontage;

	/** 몽타주 재생 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float MontagePlayRate = 1.5f;

private:
	/** 서버에서 밀치기 실행 */
	void ExecutePush(const FGameplayAbilityActivationInfo& ActivationInfo);

	// Montage 콜백
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();

	// EndAbility 호출용 캐시
	FGameplayAbilitySpecHandle CachedHandle;
	const FGameplayAbilityActorInfo* CachedActorInfo = nullptr;
	FGameplayAbilityActivationInfo CachedActivationInfo;
};

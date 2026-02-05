// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WjWorldAnimInstance.generated.h"

class UAbilitySystemComponent;

/**
 * WjWorld 캐릭터 애니메이션 인스턴스
 * GameplayTag 기반 애니메이션 상태 관리
 */
UCLASS()
class WJWORLD_API UWjWorldAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	/** LiftBrick 블렌드 가중치 (0.0 ~ 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|LiftBrick")
	float LiftBrickBlendWeight = 0.0f;

	/** 블렌드 보간 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|LiftBrick")
	float LiftBrickBlendSpeed = 10.0f;

private:
	/** 캐시된 ASC 참조 */
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

	/** 목표 블렌드 값 (태그 유무에 따라 0 또는 1) */
	float TargetLiftBrickBlendWeight = 0.0f;
};

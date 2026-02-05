// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/WjWorldAnimInstance.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "WjWorldGameplayTag.h"

void UWjWorldAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Owner Pawn의 ASC 캐시
	if (AActor* OwnerActor = GetOwningActor())
	{
		CachedASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor);
	}
}

void UWjWorldAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// ASC가 없으면 다시 시도
	if (!CachedASC.IsValid())
	{
		if (AActor* OwnerActor = GetOwningActor())
		{
			CachedASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor);
		}
	}

	// GameplayTag 체크하여 목표 블렌드 값 설정
	if (CachedASC.IsValid())
	{
		bool bHasLiftBrickTag = CachedASC->HasMatchingGameplayTag(WjWorldGameplayTag::State_LiftBrickCarry());
		TargetLiftBrickBlendWeight = bHasLiftBrickTag ? 1.0f : 0.0f;
	}
	else
	{
		TargetLiftBrickBlendWeight = 0.0f;
	}

	// 부드러운 보간
	LiftBrickBlendWeight = FMath::FInterpTo(LiftBrickBlendWeight, TargetLiftBrickBlendWeight, DeltaSeconds, LiftBrickBlendSpeed);
}

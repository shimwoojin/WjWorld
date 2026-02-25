// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/GE_NormalAttackStagger.h"

UGE_NormalAttackStagger::UGE_NormalAttackStagger()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(1.0f));

	// State.Staggered 태그는 GA_NormalAttack에서 AddLooseGameplayTag()로 직접 부여/제거
	// GE는 지속 시간 관리용
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/GE_SumoSpeedBoost.h"

UGE_SumoSpeedBoost::UGE_SumoSpeedBoost()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(5.f));

	// Buff.SpeedBoost 태그는 SumoPowerUpActor에서 AddLooseGameplayTag()로 직접 부여
	// GE는 지속 시간 관리용 (BP에서 태그 설정 가능)
}

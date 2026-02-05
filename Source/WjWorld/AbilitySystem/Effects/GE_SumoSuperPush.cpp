// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/GE_SumoSuperPush.h"

UGE_SumoSuperPush::UGE_SumoSuperPush()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	// Buff.SuperPush 태그는 SumoPowerUpActor에서 AddLooseGameplayTag()로 직접 부여
	// GA_Push에서 태그 체크 후 RemoveLooseGameplayTag()로 제거
}

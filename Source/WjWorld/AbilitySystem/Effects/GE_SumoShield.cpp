// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/GE_SumoShield.h"

UGE_SumoShield::UGE_SumoShield()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	// Buff.Shield 태그는 SumoPowerUpActor에서 AddLooseGameplayTag()로 직접 부여
	// CharacterPlay::OnEliminated()에서 태그 체크 후 RemoveLooseGameplayTag()로 제거
}

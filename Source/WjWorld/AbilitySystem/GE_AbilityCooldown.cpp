// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GE_AbilityCooldown.h"
#include "WjWorldGameplayTag.h"

UGE_AbilityCooldown::UGE_AbilityCooldown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	// SetByCaller로 Duration 설정
	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = WjWorldGameplayTag::Data_Cooldown();
	DurationMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
}

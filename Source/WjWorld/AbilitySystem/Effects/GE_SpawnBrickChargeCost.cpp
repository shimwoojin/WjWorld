// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/GE_SpawnBrickChargeCost.h"
#include "AbilitySystem/AttributeSets/WjWorldCharacterAttributeSet.h"

UGE_SpawnBrickChargeCost::UGE_SpawnBrickChargeCost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UWjWorldCharacterAttributeSet::GetSpawnBrickChargesAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-1.f));
	Modifiers.Add(Modifier);
}

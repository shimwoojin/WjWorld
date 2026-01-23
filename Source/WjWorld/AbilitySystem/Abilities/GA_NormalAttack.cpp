// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_NormalAttack.h"

#include "WjWorldGameplayTag.h"

UGA_NormalAttack::UGA_NormalAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(WjWorldGameplayTag::Ability_NormalAttack());
	SetAssetTags(AssetTags);
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * 
 */
class WJWORLD_API WjWorldGameplayTag
{
public:
	WjWorldGameplayTag();
	~WjWorldGameplayTag();

	//Camera
	static FGameplayTag Camera_ThirdPerson();
	static FGameplayTag Camera_FirstPerson();
	static FGameplayTag Camera_TopDown();

	//Ability
	static FGameplayTag Ability_NormalAttack();
	static FGameplayTag Ability_LiftBrick();

	//State
	static FGameplayTag State_SpawnBrickPreview();
	static FGameplayTag State_LiftBrickCarry();
	static FGameplayTag State_Eliminated();

	//Cooldown
	static FGameplayTag Cooldown_NormalAttack();
	static FGameplayTag Cooldown_LiftBrick();

	//Data (SetByCaller용)
	static FGameplayTag Data_Cooldown();
};

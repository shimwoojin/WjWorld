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
	static FGameplayTag Ability_SpawnBrick();
	static FGameplayTag Ability_LiftBrick();
	static FGameplayTag Ability_Push();

	//State
	static FGameplayTag State_SpawnBrickPreview();
	static FGameplayTag State_LiftBrickCarry();
	static FGameplayTag State_Eliminated();

	//Cooldown
	static FGameplayTag Cooldown_NormalAttack();
	static FGameplayTag Cooldown_LiftBrick();
	static FGameplayTag Cooldown_Push();

	//Data (SetByCaller용)
	static FGameplayTag Data_Cooldown();

	//Buff (파워업)
	static FGameplayTag Buff_SpeedBoost();
	static FGameplayTag Buff_SuperPush();
	static FGameplayTag Buff_Shield();

	//GameplayCue (사운드/VFX)
	static FGameplayTag GameplayCue_Ability_NormalAttack();
	static FGameplayTag GameplayCue_Ability_SpawnBrick();
	static FGameplayTag GameplayCue_Ability_LiftBrick();
	static FGameplayTag GameplayCue_Ability_LiftBrick_Place();
	static FGameplayTag GameplayCue_Ability_Push();
	static FGameplayTag GameplayCue_Sumo_PowerUp_Pickup();
};

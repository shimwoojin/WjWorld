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

	//State
	static FGameplayTag State_SpawnBrickPreview();
};

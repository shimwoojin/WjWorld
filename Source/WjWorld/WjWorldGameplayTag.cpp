// Fill out your copyright notice in the Description page of Project Settings.


#include "WjWorldGameplayTag.h"

WjWorldGameplayTag::WjWorldGameplayTag()
{
}

WjWorldGameplayTag::~WjWorldGameplayTag()
{
}

FGameplayTag WjWorldGameplayTag::Camera_ThirdPerson()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Camera.ThirdPerson"));
}

FGameplayTag WjWorldGameplayTag::Camera_FirstPerson()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Camera.FirstPerson"));
}

FGameplayTag WjWorldGameplayTag::Camera_TopDown()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Camera.TopDown"));
}

FGameplayTag WjWorldGameplayTag::Ability_NormalAttack()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Ability.NormalAttack"));
}

FGameplayTag WjWorldGameplayTag::Ability_LiftBrick()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Ability.LiftBrick"));
}

FGameplayTag WjWorldGameplayTag::State_SpawnBrickPreview()
{
	return FGameplayTag::RequestGameplayTag(TEXT("State.SpawnBrickPreview"));
}

FGameplayTag WjWorldGameplayTag::State_LiftBrickCarry()
{
	return FGameplayTag::RequestGameplayTag(TEXT("State.LiftBrickCarry"));
}

FGameplayTag WjWorldGameplayTag::State_Eliminated()
{
	return FGameplayTag::RequestGameplayTag(TEXT("State.Eliminated"));
}

FGameplayTag WjWorldGameplayTag::Cooldown_NormalAttack()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Cooldown.NormalAttack"));
}

FGameplayTag WjWorldGameplayTag::Cooldown_LiftBrick()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Cooldown.LiftBrick"));
}

FGameplayTag WjWorldGameplayTag::Data_Cooldown()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Data.Cooldown"));
}

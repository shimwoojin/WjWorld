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

FGameplayTag WjWorldGameplayTag::Ability_SpawnBrick()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Ability.SpawnBrick"));
}

FGameplayTag WjWorldGameplayTag::Ability_LiftBrick()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Ability.LiftBrick"));
}

FGameplayTag WjWorldGameplayTag::Ability_Push()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Ability.Push"));
}

FGameplayTag WjWorldGameplayTag::Ability_Jump()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Ability.Jump"));
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

FGameplayTag WjWorldGameplayTag::State_Staggered()
{
	return FGameplayTag::RequestGameplayTag(TEXT("State.Staggered"));
}

FGameplayTag WjWorldGameplayTag::Cooldown_NormalAttack()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Cooldown.NormalAttack"));
}

FGameplayTag WjWorldGameplayTag::Cooldown_LiftBrick()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Cooldown.LiftBrick"));
}

FGameplayTag WjWorldGameplayTag::Cooldown_Push()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Push"));
}

FGameplayTag WjWorldGameplayTag::Cooldown_Jump()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Jump"));
}

FGameplayTag WjWorldGameplayTag::Data_Cooldown()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Data.Cooldown"));
}

FGameplayTag WjWorldGameplayTag::GameplayCue_Ability_NormalAttack()
{
	return FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Ability.NormalAttack"));
}

FGameplayTag WjWorldGameplayTag::GameplayCue_Ability_SpawnBrick()
{
	return FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Ability.SpawnBrick"));
}

FGameplayTag WjWorldGameplayTag::GameplayCue_Ability_LiftBrick()
{
	return FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Ability.LiftBrick"));
}

FGameplayTag WjWorldGameplayTag::GameplayCue_Ability_LiftBrick_Place()
{
	return FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Ability.LiftBrick.Place"));
}

FGameplayTag WjWorldGameplayTag::Buff_SpeedBoost()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Buff.SpeedBoost"));
}

FGameplayTag WjWorldGameplayTag::Buff_SuperPush()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Buff.SuperPush"));
}

FGameplayTag WjWorldGameplayTag::Buff_Shield()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Buff.Shield"));
}

FGameplayTag WjWorldGameplayTag::GameplayCue_Ability_Push()
{
	return FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Ability.Push"));
}

FGameplayTag WjWorldGameplayTag::GameplayCue_Sumo_PowerUp_Pickup()
{
	return FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Sumo.PowerUp.Pickup"));
}

FGameplayTag WjWorldGameplayTag::Ability_Dash()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Ability.Dash"));
}

FGameplayTag WjWorldGameplayTag::Ability_Grapple()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Ability.Grapple"));
}

FGameplayTag WjWorldGameplayTag::Ability_DoubleJump()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Ability.DoubleJump"));
}

FGameplayTag WjWorldGameplayTag::Cooldown_Dash()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Dash"));
}

FGameplayTag WjWorldGameplayTag::Cooldown_Grapple()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Grapple"));
}

FGameplayTag WjWorldGameplayTag::Ability_ToggleCameraView()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Ability.ToggleCameraView"));
}

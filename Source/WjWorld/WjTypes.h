// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

using tid = int32;

UENUM()
namespace EWjWorldDirection
{
	enum Type : uint8
	{
		Up = 0,
		Right,
		Down,
		Left,
		Max
	};
}

UENUM(BlueprintType)
enum class EWjWorldAbilityInputID : uint8
{
	None = 0,
	Confirm = 1,	// 좌클릭 - 확정
	Cancel = 2,		// 우클릭/ESC - 취소
	Ability3 = 3,	// 일반 공격
	Ability4 = 4,	// SpawnBrick
	Ability5 = 5,	// LiftBrick
	Ability6 = 6,	// Push
	Ability7 = 7,	// Jump
};
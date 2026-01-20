// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WJCameraTypes.generated.h"

UENUM()
enum class ECharacterCameraMode : uint8
{
	TopDown,
	ThirdPerson,
	FirstPerson
};
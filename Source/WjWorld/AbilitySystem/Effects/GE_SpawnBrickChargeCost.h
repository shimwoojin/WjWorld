// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_SpawnBrickChargeCost.generated.h"

/**
 * SpawnBrick 충전 소모 GameplayEffect
 * - Instant: 즉시 적용
 * - SpawnBrickCharges -1
 */
UCLASS()
class WJWORLD_API UGE_SpawnBrickChargeCost : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_SpawnBrickChargeCost();
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameData/WjWorldGameDataComponent.h"
#include "ApproachingWallGameDataComponent.generated.h"

/**
 * 
 */
UCLASS()
class WJWORLD_API UApproachingWallGameDataComponent : public UWjWorldGameDataComponent
{
	GENERATED_BODY()
	
public:
	int32 GetSecondsForNextWave(int32 WaveIndex) const;
	

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(Replicated)
	TArray<int32> SecondsForNextWaves;
};

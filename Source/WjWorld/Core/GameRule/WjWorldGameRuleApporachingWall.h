// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameRule/WjWorldGameRuleBase.h"
#include "WjWorldGameRuleApporachingWall.generated.h"

class UWjWorldBrickSpawner;

/**
 * 
 */
UCLASS()
class WJWORLD_API UWjWorldGameRuleApporachingWall : public UWjWorldGameRuleBase
{
	GENERATED_BODY()
	
	const FSoftObjectPath WallDescPath = FSoftObjectPath("/Game/GamePlay/Wall/DA_WallDescription");

public:
	virtual void Initialize(AWjWorldGameModePlay* InGameMode) override;
	virtual void OnGameReady() override;
	virtual void OnGameStart() override;

private:
	void InternalGameStartProcess();

	UFUNCTION()
	void OnWallSpawnFinished(const TArray<FVector>& InSafeZones);

protected:
	UPROPERTY()
	TObjectPtr<UWjWorldBrickSpawner> BrickSpawner;
	TArray<FVector> SafeZones;
	bool bIsWallSpawned = false;
	bool bIsGameStarted = false;
};

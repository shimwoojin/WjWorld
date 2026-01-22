// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WjWorldWallManager.generated.h"

/**
 * 
 */
UCLASS()
class WJWORLD_API UWjWorldWallManager : public UObject
{
	GENERATED_BODY()
	
	constexpr static int32 MaxGameLevel = 10;
	constexpr static float BrickMoveAllowedTimeMax = 5.0f;
	constexpr static float BrickMoveAllowedTimeMin = 1.0f;

public:
	static UWjWorldWallManager* CreateWallManager(UObject* Outer);
	static float GetBrickMovementAllowedTime(int32 BrickMoveSignalCount);
	void Tick(float DeltaTime);

	void WallMoveStart(float WallMoveSeconds);
	bool IsWallMoving() const { return bIsWallMoving; }

private:
	void OnWallMoveEnd();
	void PushCharactersInSafeZone();

private:
	float WallMoveDurationSeconds = 5.0f;
	float WallMoveElapsedSeconds = 0.0f;
	bool bIsWallMoving = false;
};

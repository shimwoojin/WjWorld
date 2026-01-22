// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldWallManager.h"

UWjWorldWallManager* UWjWorldWallManager::CreateWallManager(UObject* Outer)
{
	if (Outer)
	{
		UWjWorldWallManager* WallManager = NewObject<UWjWorldWallManager>(Outer, UWjWorldWallManager::StaticClass());
		if (WallManager)
		{
			return WallManager;
		}
	}
	return nullptr;
}

float UWjWorldWallManager::GetBrickMovementAllowedTime(int32 BrickMoveSignalCount)
{
	float DeltaTime = BrickMoveAllowedTimeMax;

	float LevelRatio = FMath::Clamp(static_cast<float>(BrickMoveSignalCount) / static_cast<float>(MaxGameLevel), 0.0f, 1.0f);
	float InverseRatio = 1.0f - LevelRatio;
	DeltaTime = FMath::Lerp(BrickMoveAllowedTimeMin, BrickMoveAllowedTimeMax, InverseRatio);

	return DeltaTime;
}

void UWjWorldWallManager::Tick(float DeltaTime)
{
	if (bIsWallMoving == false) return;

	WallMoveElapsedSeconds += DeltaTime;

	if(WallMoveElapsedSeconds >= WallMoveDurationSeconds)
	{
		// Wall Move End 처리
		OnWallMoveEnd();
		WallMoveElapsedSeconds = 0.0f;
	}
}

void UWjWorldWallManager::WallMoveStart(float WallMoveSeconds)
{
	WallMoveDurationSeconds = WallMoveSeconds;
	bIsWallMoving = true;
	WallMoveElapsedSeconds = 0.0f;
}

void UWjWorldWallManager::OnWallMoveEnd()
{
	bIsWallMoving = false;
	PushCharactersInSafeZone();
}

void UWjWorldWallManager::PushCharactersInSafeZone()
{
}

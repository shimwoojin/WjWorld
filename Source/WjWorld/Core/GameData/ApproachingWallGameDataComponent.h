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

	int32 GetCurrentLevel() const { return CurrentLevel; }
	void SetCurrentLevel(int32 InLevel) { CurrentLevel = InLevel; }

	int32 GetAlivePlayerCount() const { return AlivePlayerCount; }
	void SetAlivePlayerCount(int32 InCount) { AlivePlayerCount = InCount; }

	int32 GetTotalPlayerCount() const { return TotalPlayerCount; }
	void SetTotalPlayerCount(int32 InCount) { TotalPlayerCount = InCount; }

	const FString& GetCurrentWallName() const { return CurrentWallName; }
	void SetCurrentWallName(const FString& InWallName) { CurrentWallName = InWallName; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(Replicated)
	TArray<int32> SecondsForNextWaves;

	UPROPERTY(Replicated)
	int32 CurrentLevel = 0;

	UPROPERTY(Replicated)
	int32 AlivePlayerCount = 0;

	UPROPERTY(Replicated)
	int32 TotalPlayerCount = 0;

	UPROPERTY(Replicated)
	FString CurrentWallName;
};

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

	/** 리플리케이트된 WallDesc 그리드 속성 (클라이언트에서 CSV 없이 사용) */
	const FVector& GetWallBrickSize() const { return WallBrickSize; }
	void SetWallBrickSize(const FVector& InSize) { WallBrickSize = InSize; }

	const FVector& GetWallCenterOffset() const { return WallCenterOffset; }
	void SetWallCenterOffset(const FVector& InOffset) { WallCenterOffset = InOffset; }

	int32 GetWallColumnNum() const { return WallColumnNum; }
	void SetWallColumnNum(int32 InNum) { WallColumnNum = InNum; }

	int32 GetWallRowNum() const { return WallRowNum; }
	void SetWallRowNum(int32 InNum) { WallRowNum = InNum; }

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

	UPROPERTY(Replicated)
	FVector WallBrickSize = FVector::ZeroVector;

	UPROPERTY(Replicated)
	FVector WallCenterOffset = FVector::ZeroVector;

	UPROPERTY(Replicated)
	int32 WallColumnNum = 0;

	UPROPERTY(Replicated)
	int32 WallRowNum = 0;
};

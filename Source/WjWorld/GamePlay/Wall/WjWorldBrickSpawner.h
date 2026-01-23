// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WjWorldWallDescriptionDataAsset.h"
#include "WjWorldBrickSpawner.generated.h"

class UWjWorldWallDescriptionDataAsset;
class AWjWorldBrickActor;

struct FWjWorldWallDescription;
struct FWjWorldBrickProperties;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWallSpawnFinished, const TArray<FVector>&, const FWjWorldWallDescription&);

/**
 * 
 */
UCLASS()
class WJWORLD_API UWjWorldBrickSpawner : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	static UWjWorldBrickSpawner* CreateBrickSpawner(UObject* Outer, TSoftObjectPtr<UWjWorldWallDescriptionDataAsset> WallDescDataAsset);
	static AWjWorldBrickActor* SpawnBrickActor(UWorld* World, const FWjWorldBrickProperties& BrickProperties, int32 ColumnIndex, int32 RowIndex);

public:
	void SpawnBricksRandomMapAsync();
	void SpawnBricksFromWallNameAsync(const FString& WallName);
	static FVector CalculateBrickPosition(int32 BrickColIndex, int32 BrickRowIndex, int32 ColNum, int32 RowNum, const FVector& WallOrigin, const FVector& BrickSize);
	static FIntPoint CalculateBrickGridIndex(const FVector& WorldLocation, int32 ColNum, int32 RowNum, const FVector& WallOrigin, const FVector& BrickSize);

	const TArray<FIntPoint>& GetStartSafeZonePoints();

	// FTickableGameObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual UWorld* GetTickableGameObjectWorld() const;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	void SetTickable(bool bInTickable);
	// ~FTickableGameObject Interface

public:
	FOnWallSpawnFinished OnWallSpawnFinished;


private:
	UPROPERTY()
	TSoftObjectPtr<UWjWorldWallDescriptionDataAsset> WallDescriptionDataAsset;

	UPROPERTY()
	TObjectPtr<UWjWorldWallDescriptionDataAsset> LoadedWallDescriptionDataAsset;

	UPROPERTY()
	FWjWorldWallDescription TargetDesc;

	UPROPERTY()
	int32 CurrentLoadingBrickColIndex = 0;
	UPROPERTY()
	int32 CurrentLoadingBrickRowIndex = 0;

	FLoadSoftObjectPathAsyncDelegate OnWallDescriptionDataAssetLoaded;

	void SpawnBricksFromWallDescriptionAsync(FWjWorldWallDescription& WallDesc);
	FString GenerateRandomWallName();

	const int32 CheckCellCountPerTick = 8;

	UPROPERTY()
	bool bTickable = false;

	UPROPERTY()
	TArray<FIntPoint> StartSafeZonePoints;
};

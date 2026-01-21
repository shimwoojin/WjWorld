// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WjWorldLogCategories.h"
#include "WjWorldWallDescriptionDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FWjWorldWallDescription
{
	GENERATED_BODY()

	constexpr static int32 WallCell_Empty = -1;

	bool LoadWallLayoutFromFile();
	void GetRandomSafeZones(TArray<FIntPoint>& OutSafeZones, int32 MaxCount = 10) const;

	void Reset()
	{
		WallLayout.Empty();
		bIsLayoutValid = false;
		bSizeExactMatch = false;
		RowNum = 0;
		ColumnNum = 0;
	}

	bool IsWallClosed()
	{
		if(IsLayoutEmpty())
		{
			bool Result = LoadWallLayoutFromFile();
			if (Result == false)
			{
				return false;
			}
		}

		return IsLayoutValid();
	}

	bool IsLayoutValid()
	{
		return !IsLayoutEmpty() && bIsLayoutValid;
	}

	bool IsLayoutEmpty()
	{
		return WallLayout.IsEmpty();
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString WallName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector CenterOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector BrickSize = FVector(100.0, 100.0, 100.0);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FFilePath WallLayoutFilePath;

	TArray<TArray<int32>> WallLayout;
	bool bIsLayoutValid = false;
	bool bSizeExactMatch = false;

	int32 RowNum = 0;
	int32 ColumnNum = 0;

	TArray<FIntPoint> SafeZones;

private:
	bool ParseWallLayout(const FString& FileContent);
	bool FindStartingEmptyCell(int32& OutX, int32& OutY);
	bool IsAreaEnclosedByWalls(int32 StartX, int32 StartY);
};

/**
 * 
 */
UCLASS()
class WJWORLD_API UWjWorldWallDescriptionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	bool GetWallDescriptionByName(const FString& Name, FWjWorldWallDescription& OutDescription) const
	{
		for (const FWjWorldWallDescription& Description : WallDescriptions)
		{
			if (Description.WallName == Name)
			{
				OutDescription = Description;
				return true;
			}
		}
		return false;
	}

	FString GenerateRandomWallDescriptionKey()
	{
		int32 RandomIndex = FMath::RandRange(0, WallDescriptions.Num() - 1);
		if (WallDescriptions.IsValidIndex(RandomIndex))
		{
			return WallDescriptions[RandomIndex].WallName;
		}

		UE_LOG(LogWjWorld, Error, TEXT("No Wall Descriptions available to generate a random key."));
		return FString();
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Description")
	TArray<FWjWorldWallDescription> WallDescriptions;
};

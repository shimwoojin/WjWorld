// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WjWorldLogCategories.h"
#include "GamePlay/Placement/WjWorldPlacementTypes.h"
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
	TSet<FIntPoint> SafeZonesSet;

private:
	bool ParseWallLayout(const FString& FileContent);
	bool FindStartingEmptyCell(int32& OutX, int32& OutY);
	bool IsAreaEnclosedByWalls(int32 StartX, int32 StartY);

	/** 경계에서 Flood Fill로 외부 영역(경계에 연결된 빈 셀) 마킹 */
	void MarkExteriorCells(TSet<FIntPoint>& OutExteriorCells);
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

	/**
	 * 유저 벽 레이아웃 디렉토리를 스캔하여 사용 가능한 레이아웃 목록 반환
	 * @param OutUserDescriptions 발견된 유저 레이아웃 목록
	 * @return 발견된 레이아웃 수
	 */
	int32 ScanUserWallLayouts(TArray<FWjWorldWallDescription>& OutUserDescriptions) const;

	/**
	 * 내장 레이아웃 + 유저 레이아웃을 모두 포함한 이름 목록 반환
	 */
	TArray<FString> GetAllWallNames() const;

	/**
	 * 내장 또는 유저 레이아웃에서 이름으로 검색 (유저 레이아웃 포함)
	 */
	bool GetWallDescriptionByNameIncludingUser(const FString& Name, FWjWorldWallDescription& OutDescription) const;

	/**
	 * 랜덤 벽 이름 생성 (유저 레이아웃 포함)
	 */
	FString GenerateRandomWallNameIncludingUser() const;

private:
	/** 캐시된 유저 레이아웃 (런타임에 스캔) */
	mutable TArray<FWjWorldWallDescription> CachedUserDescriptions;
	mutable bool bUserDescriptionsCached = false;
};

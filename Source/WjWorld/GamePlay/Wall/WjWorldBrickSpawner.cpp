// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldBrickSpawner.h"
#include "GamePlay/Wall/WjWorldWallDescriptionDataAsset.h"
#include "GamePlay/Wall/WjWorldBrickActor.h"
#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/Wall/WjWorldTileActor.h"

#include "WjWorldLogCategories.h"

UWjWorldBrickSpawner* UWjWorldBrickSpawner::CreateBrickSpawner(UObject* Outer, TSoftObjectPtr<UWjWorldWallDescriptionDataAsset> WallDescDataAsset)
{
	if (Outer)
	{
		UWjWorldBrickSpawner* BrickSpawner = NewObject<UWjWorldBrickSpawner>(Outer, UWjWorldBrickSpawner::StaticClass());
		if (BrickSpawner)
		{
			BrickSpawner->WallDescriptionDataAsset = WallDescDataAsset;
			BrickSpawner->LoadedWallDescriptionDataAsset = LoadObject<UWjWorldWallDescriptionDataAsset>(Outer, *WallDescDataAsset.ToString());
			return BrickSpawner;
		}
	}

	return nullptr;
}

AWjWorldBrickActor* UWjWorldBrickSpawner::SpawnBrickActor(UWorld* World, const FWjWorldBrickProperties& BrickProperties, int32 ColumnIndex, int32 RowIndex)
{
	if (!World) return nullptr;

	AWjWorldBrickActor* BrickActor = World->SpawnActor<AWjWorldBrickActor>(AWjWorldBrickActor::StaticClass());
	if (BrickActor)
	{
		if (UWjWorldBrickComponent* BrickComponent = BrickActor->GetBrickComponent())
		{
			BrickComponent->InitializeBrick(BrickProperties);
		}

		FVector BrickPosition = CalculateBrickPosition(
			ColumnIndex,
			RowIndex,
			BrickProperties.ColumnNum,
			BrickProperties.RowNum,
			BrickProperties.CenterOffset,
			BrickProperties.Size);

		BrickActor->SetActorLocation(BrickPosition);

		UE_LOG(LogWjWorld, Log, TEXT("Spawned Brick at Row: %d, Col: %d, Type: %d, Position : %s"),
			RowIndex, ColumnIndex, (int32)BrickProperties.BrickType, *BrickPosition.ToString());
	}

	return BrickActor;
}

void UWjWorldBrickSpawner::SpawnBricksRandomMapAsync()
{
	FString RandomWallName = GenerateRandomWallName();
	UE_LOG(LogWjWorld, Log, TEXT("Spawning Bricks from Random Wall Name: %s"), *RandomWallName);

	SpawnBricksFromWallNameAsync(RandomWallName);
}

void UWjWorldBrickSpawner::SpawnBricksFromWallNameAsync(const FString& WallName)
{
	if (!LoadedWallDescriptionDataAsset) return;

	if (!LoadedWallDescriptionDataAsset->GetWallDescriptionByName(WallName, TargetDesc))
	{
		UE_LOG(LogWjWorld, Error, TEXT("Invalid Wall Desc Name : %s"), *WallName);
		return;
	}

	SpawnBricksFromWallDescriptionAsync(TargetDesc);
}

FVector UWjWorldBrickSpawner::CalculateBrickPosition(int32 BrickColIndex, int32 BrickRowIndex, int32 ColNum, int32 RowNum, const FVector& WallOrigin, const FVector& BrickSize)
{
	// 그리드 중심 인덱스 (정중앙 기준)
	const float CenterCol = (ColNum - 1) * 0.5f;
	const float CenterRow = (RowNum - 1) * 0.5f;

	// 중심으로부터의 오프셋 (그리드 좌표계)
	const float ColOffset = BrickColIndex - CenterCol;
	const float RowOffset = BrickRowIndex - CenterRow;

	// 월드 오프셋 (BrickSize 반영)
	const float WorldOffsetX = ColOffset * BrickSize.X;
	const float WorldOffsetY = RowOffset * BrickSize.Y;

	return WallOrigin + FVector(WorldOffsetX, WorldOffsetY, BrickSize.Z / 2.0);
}

FIntPoint UWjWorldBrickSpawner::CalculateBrickGridIndex(const FVector& WorldLocation, int32 ColNum, int32 RowNum, const FVector& WallOrigin, const FVector& BrickSize)
{
	// 그리드 중심 인덱스 (정중앙 기준)
	const float CenterCol = (ColNum - 1) * 0.5f;
	const float CenterRow = (RowNum - 1) * 0.5f;

	// 월드 오프셋 계산
	const float WorldOffsetX = WorldLocation.X - WallOrigin.X;
	const float WorldOffsetY = WorldLocation.Y - WallOrigin.Y;

	// 그리드 오프셋 계산
	const float ColOffset = WorldOffsetX / BrickSize.X;
	const float RowOffset = WorldOffsetY / BrickSize.Y;

	// 그리드 인덱스 계산 (반올림하여 가장 가까운 셀로)
	const int32 ColIndex = FMath::RoundToInt32(ColOffset + CenterCol);
	const int32 RowIndex = FMath::RoundToInt32(RowOffset + CenterRow);

	return FIntPoint(ColIndex, RowIndex);
}

const TArray<FIntPoint>& UWjWorldBrickSpawner::GetStartSafeZonePoints()
{
	return StartSafeZonePoints;
}

void UWjWorldBrickSpawner::Tick(float DeltaTime)
{
	//if (bTickable == false) return;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	bool bIsLayoutValid = TargetDesc.IsLayoutValid();
	if (bIsLayoutValid == false)
	{
		UE_LOG(LogWjWorld, Error, TEXT("Wall Description Layout is Invalid"));
		CurrentLoadingBrickColIndex = 0;
		CurrentLoadingBrickRowIndex = 0;
		SetTickable(false);
		return;
	}

	int32 ProcessedCellsCount = 0;
	bool bFinishedSpawning = false;

	while (ProcessedCellsCount < CheckCellCountPerTick
		&& bFinishedSpawning == false)
	{
		int32 BrickType = TargetDesc.WallLayout[CurrentLoadingBrickRowIndex][CurrentLoadingBrickColIndex];
		++ProcessedCellsCount;

		if (BrickType >= (int32)EWjWorldBrickType::Standard)
		{
			FWjWorldBrickProperties BrickProperties;
			BrickProperties.BrickType = static_cast<EWjWorldBrickType>(BrickType);
			BrickProperties.BrickMoveType = EWjWorldBrickMoveType::Standard;
			BrickProperties.Size = TargetDesc.BrickSize;
			BrickProperties.Color = FColor::White;
			BrickProperties.SpawnedGridPosition = FIntPoint(CurrentLoadingBrickColIndex, CurrentLoadingBrickRowIndex);
			BrickProperties.CenterOffset = TargetDesc.CenterOffset;
			BrickProperties.ColumnNum = TargetDesc.ColumnNum;
			BrickProperties.RowNum = TargetDesc.RowNum;

			SpawnBrickActor(World, BrickProperties, CurrentLoadingBrickColIndex, CurrentLoadingBrickRowIndex);
		}
		else if (BrickType < (int32)EWjWorldBrickType::Standard
			&& TargetDesc.SafeZonesSet.Contains(FIntPoint(CurrentLoadingBrickColIndex, CurrentLoadingBrickRowIndex)))
		{
			AWjWorldTileActor* TileActor = World->SpawnActor<AWjWorldTileActor>(AWjWorldTileActor::StaticClass());
			if (TileActor)
			{
				FVector TilePosition = CalculateBrickPosition(
					CurrentLoadingBrickColIndex,
					CurrentLoadingBrickRowIndex,
					TargetDesc.ColumnNum,
					TargetDesc.RowNum,
					TargetDesc.CenterOffset,
					TargetDesc.BrickSize);

				TileActor->InitializeTile(TargetDesc.BrickSize, TilePosition);
			}
		}

		CurrentLoadingBrickColIndex++;
		if (TargetDesc.WallLayout[CurrentLoadingBrickRowIndex].IsValidIndex(CurrentLoadingBrickColIndex) == false)
		{
			CurrentLoadingBrickColIndex = 0;
			CurrentLoadingBrickRowIndex++;

			if (TargetDesc.WallLayout.IsValidIndex(CurrentLoadingBrickRowIndex) == false)
			{
				CurrentLoadingBrickColIndex = 0;
				CurrentLoadingBrickRowIndex = 0;
				bFinishedSpawning = true;
				SetTickable(false);
				UE_LOG(LogWjWorld, Log, TEXT("Finished Spawning All Bricks"));
			}
		}
	}

	if (bFinishedSpawning)
	{
		TArray<FIntPoint> RandomSafeZonePoints;
		TargetDesc.GetRandomSafeZones(RandomSafeZonePoints);

		StartSafeZonePoints = TargetDesc.SafeZones;

		TArray<FVector> RandomSafeZoneLocations;
		for(const FIntPoint& SafeZonePoint : RandomSafeZonePoints)
		{
			FVector SafeZoneLocation = CalculateBrickPosition(
				SafeZonePoint.X,
				SafeZonePoint.Y,
				TargetDesc.ColumnNum,
				TargetDesc.RowNum,
				TargetDesc.CenterOffset,
				TargetDesc.BrickSize);
			RandomSafeZoneLocations.Add(SafeZoneLocation);
		}

		OnWallSpawnFinished.Broadcast(RandomSafeZoneLocations, TargetDesc);
		TargetDesc.Reset();
	}

	UE_LOG(LogWjWorld, Warning,
		TEXT("Tick Start: Row=%d Col=%d bTickable=%d"),
		CurrentLoadingBrickRowIndex,
		CurrentLoadingBrickColIndex,
		bTickable);
}

UWorld* UWjWorldBrickSpawner::GetTickableGameObjectWorld() const
{
	if (GetOuter())
	{
		return GetOuter()->GetWorld();
	}
	return nullptr;
}

TStatId UWjWorldBrickSpawner::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UWjWorldBrickSpawner, STATGROUP_Tickables);
}

bool UWjWorldBrickSpawner::IsTickable() const
{
	return bTickable;
}

void UWjWorldBrickSpawner::SetTickable(bool bInTickable)
{
	bTickable = bInTickable;
	UE_LOG(LogWjWorld, Log, TEXT("BrickSpawner Tickable set to %d"), bTickable);
}

void UWjWorldBrickSpawner::SpawnBricksFromWallDescriptionAsync(FWjWorldWallDescription& WallDesc)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if(bTickable)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Already Spawning Bricks, Ignoring new Spawn Request"));
		return;
	}

	bool bResult = WallDesc.LoadWallLayoutFromFile();
	SetTickable(bResult);
}

FString UWjWorldBrickSpawner::GenerateRandomWallName()
{
	if (!LoadedWallDescriptionDataAsset)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WallDescriptionDataAsset is null, %s"), *WallDescriptionDataAsset.GetLongPackageName());
		return FString();
	}

	return LoadedWallDescriptionDataAsset->GenerateRandomWallDescriptionKey();
}

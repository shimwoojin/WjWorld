// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameRule/WjWorldGameRuleApporachingWall.h"
#include "GamePlay/Wall/WjWorldBrickSpawner.h"

#include "WjWorldLogCategories.h"

void UWjWorldGameRuleApporachingWall::Initialize(AWjWorldGameModePlay* InGameMode)
{
	Super::Initialize(InGameMode);

	BrickSpawner = UWjWorldBrickSpawner::CreateBrickSpawner(this, TSoftObjectPtr<UWjWorldWallDescriptionDataAsset>(WallDescPath));
	if (!BrickSpawner)
	{
		UE_LOG(LogWjWorld, Error, TEXT("Failed to create BrickSpawner in GameRuleApporachingWall"));
	}

	BrickSpawner->OnWallSpawnFinished.AddLambda([this](const TArray<FVector>& SpawnedBrickPositions) {
		UE_LOG(LogWjWorld, Log, TEXT("Bricks Spawned. Total Bricks: %d"), SpawnedBrickPositions.Num());
		bIsWallSpawned = true;
		InternalGameStartProcess();
		});
}

void UWjWorldGameRuleApporachingWall::OnGameReady()
{
	Super::OnGameReady();

	if(BrickSpawner)
	{
		BrickSpawner->OnWallSpawnFinished.AddUObject(this, &UWjWorldGameRuleApporachingWall::OnWallSpawnFinished);
		BrickSpawner->SpawnBricksRandomMapAsync();

		UE_LOG(LogWjWorld, Log, TEXT("UWjWorldGameRuleApporachingWall::OnGameReady >>> SpawnBricksRandomMapAsync"));
	}

	UE_LOG(LogWjWorld, Log, TEXT("UWjWorldGameRuleApporachingWall::OnGameReady()"));
}

void UWjWorldGameRuleApporachingWall::OnGameStart()
{
	Super::OnGameStart();

	UE_LOG(LogWjWorld, Log, TEXT("UWjWorldGameRuleApporachingWall::OnGameStart()"));
	bIsGameStarted = true;
	InternalGameStartProcess();
}

void UWjWorldGameRuleApporachingWall::InternalGameStartProcess()
{
	if(bIsWallSpawned && bIsGameStarted)
	{
		UE_LOG(LogWjWorld, Log, TEXT("Starting Game after Wall Spawned"));

		if (GetWorld())
		{
			int32 SafeZoneIndex = 0;
			for (auto Iter = GetWorld()->GetPlayerControllerIterator(); Iter; ++Iter)
			{
				APlayerController* PC = Iter->Get();
				if (PC)
				{
					PC->ClientMessage(TEXT("Game Started! Wall has spawned."));
					PC->GetPawn()->SetActorLocation(SafeZones.IsValidIndex(SafeZoneIndex++) ? SafeZones[SafeZoneIndex] : FVector::ZeroVector);
				}
			}
		}
	}
}

void UWjWorldGameRuleApporachingWall::OnWallSpawnFinished(const TArray<FVector>& InSafeZones)
{
	bIsWallSpawned = true;
	SafeZones.Empty();
	SafeZones.Append(InSafeZones);

	InternalGameStartProcess();

	if (BrickSpawner)
	{
		BrickSpawner->OnWallSpawnFinished.RemoveAll(this);
	}
}

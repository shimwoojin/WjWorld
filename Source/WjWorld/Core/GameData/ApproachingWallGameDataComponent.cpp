// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameData/ApproachingWallGameDataComponent.h"
#include "Net/UnrealNetwork.h"

int32 UApproachingWallGameDataComponent::GetSecondsForNextWave(int32 WaveIndex) const
{
	if (SecondsForNextWaves.IsValidIndex(WaveIndex))
	{
		return SecondsForNextWaves[WaveIndex];
	}
	return -1;
}

void UApproachingWallGameDataComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UApproachingWallGameDataComponent, SecondsForNextWaves);
	DOREPLIFETIME(UApproachingWallGameDataComponent, CurrentLevel);
	DOREPLIFETIME(UApproachingWallGameDataComponent, AlivePlayerCount);
	DOREPLIFETIME(UApproachingWallGameDataComponent, TotalPlayerCount);
	DOREPLIFETIME(UApproachingWallGameDataComponent, CurrentWallName);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameData/JumpMapPlayerDataComponent.h"
#include "Net/UnrealNetwork.h"

UJumpMapPlayerDataComponent::UJumpMapPlayerDataComponent()
{
	bHasFinished = false;
	FinishTime = 0.f;
	CurrentCheckpointIndex = -1;
	DeathCount = 0;
}

void UJumpMapPlayerDataComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UJumpMapPlayerDataComponent, bHasFinished);
	DOREPLIFETIME(UJumpMapPlayerDataComponent, FinishTime);
	DOREPLIFETIME(UJumpMapPlayerDataComponent, CurrentCheckpointIndex);
	DOREPLIFETIME(UJumpMapPlayerDataComponent, DeathCount);
}

void UJumpMapPlayerDataComponent::SetFinished(float InFinishTime)
{
	bHasFinished = true;
	FinishTime = InFinishTime;
}

void UJumpMapPlayerDataComponent::SetCurrentCheckpointIndex(int32 Index)
{
	if (CurrentCheckpointIndex != Index)
	{
		CurrentCheckpointIndex = Index;
		OnCheckpointReached.Broadcast(CurrentCheckpointIndex);
	}
}

void UJumpMapPlayerDataComponent::IncrementDeathCount()
{
	DeathCount++;
}

void UJumpMapPlayerDataComponent::ResetForNewGame()
{
	bHasFinished = false;
	FinishTime = 0.f;
	CurrentCheckpointIndex = -1;
	DeathCount = 0;
}

void UJumpMapPlayerDataComponent::OnRep_CurrentCheckpointIndex()
{
	OnCheckpointReached.Broadcast(CurrentCheckpointIndex);
}

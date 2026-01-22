// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameData/ApproachingWallPlayerDataComponent.h"
#include "Net/UnrealNetwork.h"

UApproachingWallPlayerDataComponent::UApproachingWallPlayerDataComponent()
{
	bIsAlive = true;
}

void UApproachingWallPlayerDataComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UApproachingWallPlayerDataComponent, bIsAlive);
}

void UApproachingWallPlayerDataComponent::SetAlive(bool bNewAlive)
{
	if (bIsAlive != bNewAlive)
	{
		bIsAlive = bNewAlive;
		OnPlayerAliveChanged.Broadcast(bIsAlive);
	}
}

void UApproachingWallPlayerDataComponent::OnEliminated()
{
	SetAlive(false);
}

void UApproachingWallPlayerDataComponent::OnRep_IsAlive()
{
	OnPlayerAliveChanged.Broadcast(bIsAlive);
}

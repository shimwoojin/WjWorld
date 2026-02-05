// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameData/SumoPlayerDataComponent.h"
#include "Net/UnrealNetwork.h"

USumoPlayerDataComponent::USumoPlayerDataComponent()
{
	bIsAlive = true;
}

void USumoPlayerDataComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USumoPlayerDataComponent, bIsAlive);
	DOREPLIFETIME(USumoPlayerDataComponent, TotalScore);
}

void USumoPlayerDataComponent::SetAlive(bool bNewAlive)
{
	if (bIsAlive != bNewAlive)
	{
		bIsAlive = bNewAlive;
		OnPlayerAliveChanged.Broadcast(bIsAlive);
	}
}

void USumoPlayerDataComponent::OnEliminated()
{
	SetAlive(false);
}

void USumoPlayerDataComponent::ResetForNewRound()
{
	SetAlive(true);
}

void USumoPlayerDataComponent::AddScore(int32 InScore)
{
	TotalScore += InScore;
}

void USumoPlayerDataComponent::OnRep_IsAlive()
{
	OnPlayerAliveChanged.Broadcast(bIsAlive);
}

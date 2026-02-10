// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameData/JumpMapGameDataComponent.h"
#include "Net/UnrealNetwork.h"

void UJumpMapGameDataComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UJumpMapGameDataComponent, ElapsedTime);
	DOREPLIFETIME(UJumpMapGameDataComponent, TimeLimit);
	DOREPLIFETIME(UJumpMapGameDataComponent, FinishedPlayerCount);
	DOREPLIFETIME(UJumpMapGameDataComponent, TotalPlayerCount);
	DOREPLIFETIME(UJumpMapGameDataComponent, PlayerFinishOrder);
}

void UJumpMapGameDataComponent::AddPlayerFinish(const FJumpMapPlayerFinish& Finish)
{
	PlayerFinishOrder.Add(Finish);
	OnPlayerFinished.Broadcast(Finish);
}

void UJumpMapGameDataComponent::OnRep_PlayerFinishOrder()
{
	if (PlayerFinishOrder.Num() > 0)
	{
		OnPlayerFinished.Broadcast(PlayerFinishOrder.Last());
	}
}

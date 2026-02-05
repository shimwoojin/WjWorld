// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameData/SumoGameDataComponent.h"
#include "Net/UnrealNetwork.h"

void USumoGameDataComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USumoGameDataComponent, AlivePlayerCount);
	DOREPLIFETIME(USumoGameDataComponent, TotalPlayerCount);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Components/WjWorldGameplayActorComponent.h"

#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/Play/WjWorldGameStatePlay.h"

UWjWorldGameplayActorComponent::UWjWorldGameplayActorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

TObjectPtr<AWjWorldGameModePlay> UWjWorldGameplayActorComponent::GetGameModePlay()
{
	if (GameModePlay == nullptr)
	{
		GameModePlay = GetWorld()->GetAuthGameMode<AWjWorldGameModePlay>();
	}

	return GameModePlay;
}

TObjectPtr<AWjWorldGameStatePlay> UWjWorldGameplayActorComponent::GetGameStatePlay()
{
	if (GameStatePlay == nullptr)
	{
		GameStatePlay = GetWorld()->GetGameState<AWjWorldGameStatePlay>();
	}

	return GameStatePlay;
}

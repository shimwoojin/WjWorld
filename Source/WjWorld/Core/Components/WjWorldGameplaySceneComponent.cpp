// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Components/WjWorldGameplaySceneComponent.h"

#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/Play/WjWorldGameStatePlay.h"

UWjWorldGameplaySceneComponent::UWjWorldGameplaySceneComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

TObjectPtr<AWjWorldGameModePlay> UWjWorldGameplaySceneComponent::GetGameModePlay()
{
	return GetWorld()->GetAuthGameMode<AWjWorldGameModePlay>();
}

TObjectPtr<AWjWorldGameStatePlay> UWjWorldGameplaySceneComponent::GetGameStatePlay()
{
	return GetWorld()->GetGameState<AWjWorldGameStatePlay>();
}
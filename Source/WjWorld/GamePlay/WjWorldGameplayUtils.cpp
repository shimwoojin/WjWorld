// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/WjWorldGameplayUtils.h"

#include "Core/Play/WjWorldGameStatePlay.h"

bool WjWorldGameplayUtils::IsGameplayPhaseplaying(UWorld* InWorld)
{
	if (InWorld == nullptr) return false;

	AWjWorldGameStatePlay* GameStatePlay = InWorld->GetGameState<AWjWorldGameStatePlay>();
	if (GameStatePlay == nullptr) return false;

	if (GameStatePlay->GetGamePhase() == EGamePhase::Playing)
	{
		return true;
	}

	return false;
}

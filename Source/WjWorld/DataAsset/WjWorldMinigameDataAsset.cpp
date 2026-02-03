// Fill out your copyright notice in the Description page of Project Settings.

#include "DataAsset/WjWorldMinigameDataAsset.h"

const FWjWorldMinigameDefinition* UWjWorldMinigameDataAsset::FindByGameModeId(FName GameModeId) const
{
	for (const FWjWorldMinigameDefinition& Def : Minigames)
	{
		if (Def.GameModeId == GameModeId)
		{
			return &Def;
		}
	}
	return nullptr;
}

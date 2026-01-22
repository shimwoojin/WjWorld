// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WjWorldGameplaySceneComponent.generated.h"

class AWjWorldGameModePlay;
class AWjWorldGameStatePlay;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WJWORLD_API UWjWorldGameplaySceneComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UWjWorldGameplaySceneComponent();

public:
	TObjectPtr<AWjWorldGameModePlay> GetGameModePlay();
	TObjectPtr<AWjWorldGameStatePlay> GetGameStatePlay();

//private:
//	UPROPERTY()
//	TObjectPtr<AWjWorldGameModePlay> GameModePlay;
//
//	UPROPERTY()
//	TObjectPtr<AWjWorldGameStatePlay> GameStatePlay;
};

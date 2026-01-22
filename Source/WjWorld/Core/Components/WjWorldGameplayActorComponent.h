// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WjWorldGameplayActorComponent.generated.h"

class AWjWorldGameModePlay;
class AWjWorldGameStatePlay;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WJWORLD_API UWjWorldGameplayActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWjWorldGameplayActorComponent();

public:
	TObjectPtr<AWjWorldGameModePlay> GetGameModePlay();
	TObjectPtr<AWjWorldGameStatePlay> GetGameStatePlay();

private:
	UPROPERTY()
	TObjectPtr<AWjWorldGameModePlay> GameModePlay;

	UPROPERTY()
	TObjectPtr<AWjWorldGameStatePlay> GameStatePlay;
		
};

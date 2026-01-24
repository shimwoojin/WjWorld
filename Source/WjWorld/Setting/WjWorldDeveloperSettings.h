// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "WjWorldDeveloperSettings.generated.h"

class AWjWorldTileActor;

/**
 * 
 */
UCLASS(config = WjWorld, defaultconfig, meta = (DisplayName = "WjWorld"))
class WJWORLD_API UWjWorldDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()
	
public:
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Approaching Wall|Tile")
	TSoftClassPtr<AWjWorldTileActor> TileActorClass;
};

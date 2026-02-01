// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "WjWorldDeveloperSettings.generated.h"

class AWjWorldTileActor;
class UNiagaraSystem;
class UGeometryCollection;

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

	UPROPERTY(EditAnywhere, Category = "Approaching Wall|Brick")
	TSoftObjectPtr<UNiagaraSystem> BrickDestroyEffect;

	UPROPERTY(EditAnywhere, Category = "Approaching Wall|Brick")
	TSoftObjectPtr<UNiagaraSystem> BrickExplosionEffect;

	// Destructible 벽돌 기본 내구도
	UPROPERTY(EditAnywhere, Category = "Approaching Wall|Brick|Destructible", meta = (ClampMin = "1"))
	int32 DestructibleBrickDefaultHP = 3;

	// Destructible 벽돌 파쇄 GeometryCollection 에셋
	UPROPERTY(EditAnywhere, Category = "Approaching Wall|Brick|Destructible")
	TSoftObjectPtr<UGeometryCollection> DestructibleBrickFractureCollection;

	// 파쇄 파편 자동 제거까지의 수명 (초)
	UPROPERTY(EditAnywhere, Category = "Approaching Wall|Brick|Destructible", meta = (ClampMin = "0.1"))
	float DestructibleBrickFractureLifetime = 3.0f;
};

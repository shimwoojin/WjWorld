// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "WjWorldDeveloperSettings.generated.h"

class AWjWorldTileActor;
class UNiagaraSystem;
class UGeometryCollection;
class UWjWorldMinigameDataAsset;
class UWjWorldPlaceableObjectDataAsset;
class UInputMappingContext;
class UInputAction;

/**
 * WjWorld 프로젝트 개발자 설정
 * Project Settings > WjWorld 에서 확인 가능
 */
UCLASS(config = WjWorld, defaultconfig, meta = (DisplayName = "WjWorld"))
class WJWORLD_API UWjWorldDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	// Project Settings에서 "Project" 섹션에 표시
	virtual FName GetContainerName() const override { return TEXT("Project"); }
	virtual FName GetCategoryName() const override { return TEXT("WjWorld"); }
	virtual FName GetSectionName() const override { return TEXT("WjWorld"); }
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

	// 미니게임 카탈로그 데이터 에셋
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Minigame")
	TSoftObjectPtr<UWjWorldMinigameDataAsset> MinigameCatalog;

	// 배치 가능한 오브젝트 카탈로그
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Placement")
	TSoftObjectPtr<UWjWorldPlaceableObjectDataAsset> PlaceableObjectCatalog;

	// 배치 모드 입력 매핑 컨텍스트
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Placement|Input")
	TSoftObjectPtr<UInputMappingContext> PlacementMappingContext;

	// 배치 확정 입력 액션 (LMB)
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Placement|Input")
	TSoftObjectPtr<UInputAction> PlacementConfirmAction;

	// 배치 취소 입력 액션 (ESC)
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Placement|Input")
	TSoftObjectPtr<UInputAction> PlacementCancelAction;

	// 회전 입력 액션 (R)
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Placement|Input")
	TSoftObjectPtr<UInputAction> PlacementRotateAction;

	// 삭제 입력 액션 (DEL)
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Placement|Input")
	TSoftObjectPtr<UInputAction> PlacementDeleteAction;
};

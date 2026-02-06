// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "GamePlay/Placement/WjWorldPlacementTypes.h"
#include "WjWorldDeveloperSettings.generated.h"

class AWjWorldTileActor;
class UNiagaraSystem;
class UGeometryCollection;
class UWjWorldMinigameDataAsset;
class UWjWorldPlaceableObjectDataAsset;
class UWjWorldCosmeticCatalogDataAsset;
class UWjWorldWallDescriptionDataAsset;
class UInputMappingContext;
class UInputAction;
class USkeletalMesh;
class UStaticMesh;
class UAnimInstance;

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

	// ========== Maps ==========

	/** 로비 맵 경로 */
	UPROPERTY(config, EditAnywhere, Category = "Maps")
	FSoftObjectPath LobbyMapPath = FSoftObjectPath(TEXT("/Game/Map/02-1_Lobby"));

	/** 대기실 게임모드 클래스 */
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Maps")
	TSoftClassPtr<AGameModeBase> WaitingRoomGameModeClass;

	/** 플레이 게임모드 클래스 */
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Maps")
	TSoftClassPtr<AGameModeBase> PlayGameModeClass;

	// ========== Character Defaults ==========

	/** 캐릭터 기본 스켈레탈 메시 */
	UPROPERTY(config, EditAnywhere, Category = "Character|Default")
	TSoftObjectPtr<USkeletalMesh> DefaultCharacterMesh;

	/** 캐릭터 기본 애니메이션 블루프린트 클래스 */
	UPROPERTY(config, EditAnywhere, Category = "Character|Default")
	TSoftClassPtr<UAnimInstance> DefaultAnimBlueprintClass;

	/** 캐릭터 기본 입력 매핑 컨텍스트 */
	UPROPERTY(config, EditAnywhere, Category = "Character|Default")
	TSoftObjectPtr<UInputMappingContext> DefaultInputMappingContext;

	// ========== Approaching Wall ==========

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

	/** 벽돌 메시 */
	UPROPERTY(config, EditAnywhere, Category = "Approaching Wall|Mesh")
	TSoftObjectPtr<UStaticMesh> BrickMesh;

	/** 타일 메시 */
	UPROPERTY(config, EditAnywhere, Category = "Approaching Wall|Mesh")
	TSoftObjectPtr<UStaticMesh> TileMesh;

	/** 벽 레이아웃 데이터 에셋 */
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Approaching Wall")
	TSoftObjectPtr<UWjWorldWallDescriptionDataAsset> WallDescriptionAsset;

	// ========== Minigame ==========

	/** 미니게임 카탈로그 데이터 에셋 */
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Minigame")
	TSoftObjectPtr<UWjWorldMinigameDataAsset> MinigameCatalog;

	// ========== Cosmetic ==========

	/** 코스메틱 카탈로그 데이터 에셋 */
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Cosmetic")
	TSoftObjectPtr<UWjWorldCosmeticCatalogDataAsset> CosmeticCatalog;

	// ========== Placement ==========

	/** @deprecated 로비 배치 카탈로그 사용 권장 (LobbyPlaceableCatalog) */
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Placement", meta = (DisplayName = "Placeable Object Catalog (Legacy)"))
	TSoftObjectPtr<UWjWorldPlaceableObjectDataAsset> PlaceableObjectCatalog;

	// ========== Placement Editor Maps ==========

	/** Approaching Wall 에디터 맵 경로 */
	UPROPERTY(config, EditAnywhere, Category = "Placement|Editor Maps")
	FSoftObjectPath AWEditorMapPath;

	/** JumpMap 에디터 맵 경로 */
	UPROPERTY(config, EditAnywhere, Category = "Placement|Editor Maps")
	FSoftObjectPath JumpMapEditorMapPath;

	/** AW 에디터 게임모드 클래스 */
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Placement|Editor Maps")
	TSoftClassPtr<AGameModeBase> AWEditorGameModeClass;

	/** JumpMap 에디터 게임모드 클래스 */
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Placement|Editor Maps")
	TSoftClassPtr<AGameModeBase> JumpMapEditorGameModeClass;

	/** 로비 배치 카탈로그 */
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Placement|Context")
	TSoftObjectPtr<UWjWorldPlaceableObjectDataAsset> LobbyPlaceableCatalog;

	/** Approaching Wall 배치 카탈로그 */
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Placement|Context")
	TSoftObjectPtr<UWjWorldPlaceableObjectDataAsset> ApproachingWallPlaceableCatalog;

	/** JumpMap 배치 카탈로그 */
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Placement|Context")
	TSoftObjectPtr<UWjWorldPlaceableObjectDataAsset> JumpMapPlaceableCatalog;

	/** 배치 모드 입력 매핑 컨텍스트 */
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Placement|Input")
	TSoftObjectPtr<UInputMappingContext> PlacementMappingContext;

	// ========== Helper Functions ==========

	/** 로비 맵 경로 반환 (예: "/Game/Map/02-1_Lobby") */
	FString GetLobbyMapPath() const;

	/** 대기실 OpenLevel URL 반환 (예: "/Game/Map/02-1_Lobby?game=...?Listen") */
	FString GetWaitingRoomOpenLevelURL() const;

	/** 대기실 ServerTravel URL 반환 (예: "/Game/Map/02-1_Lobby?game=...") */
	FString GetWaitingRoomServerTravelURL() const;

	/** 플레이 ServerTravel URL 반환 */
	FString GetPlayServerTravelURL(const FString& LevelPath, const FString& GameModeId) const;

	/** 컨텍스트별 배치 카탈로그 반환 (폴백: PlaceableObjectCatalog) */
	UWjWorldPlaceableObjectDataAsset* GetPlaceableCatalogForContext(EPlacementContext Context) const;

	/** 컨텍스트별 에디터 맵 OpenLevel URL 반환 (빈 문자열 = 미설정) */
	FString GetEditorMapOpenLevelURL(EPlacementContext Context) const;

	/** 컨텍스트별 에디터 맵이 설정되어 있는지 확인 */
	bool HasEditorMapForContext(EPlacementContext Context) const;

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

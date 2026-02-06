// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldGameStateBase.h"
#include "GamePlay/Placement/IWjWorldPlacementDataProvider.h"
#include "Save/WjWorldLayoutSaveGame.h"
#include "WjWorldGameStateJumpMapEditor.generated.h"

class AWjWorldPlacedObjectActor;
class UWjWorldPlaceableObjectDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJumpMapEditorPlacementDataChanged);

/**
 * JumpMap 에디터 게임 상태
 * 플랫폼, 장애물, 함정 배치 에디터
 * 싱글플레이어 전용 (에디터 모드)
 */
UCLASS()
class WJWORLD_API AWjWorldGameStateJumpMapEditor : public AWjWorldGameStateBase, public IWjWorldPlacementDataProvider
{
	GENERATED_BODY()

public:
	AWjWorldGameStateJumpMapEditor();

	//~ IWjWorldPlacementDataProvider 인터페이스 구현
	virtual void AddPlacedObject(const FPlacedObjectSaveEntry& Entry) override;
	virtual void RemovePlacedObjectAt(int32 Index) override;
	virtual const TArray<FPlacedObjectSaveEntry>& GetPlacedObjects() const override { return PlacedObjects; }
	virtual void SetPlacedObjects(const TArray<FPlacedObjectSaveEntry>& InPlacedObjects) override;
	virtual void ClearPlacedObjects() override;
	virtual void SetPlacementCatalog(UWjWorldPlaceableObjectDataAsset* InCatalog) override { Catalog = InCatalog; }
	virtual UWjWorldPlaceableObjectDataAsset* GetPlacementCatalog() const override { return Catalog; }

	//~ Getter

	int32 GetPlacedObjectCount() const { return PlacedObjects.Num(); }

	//~ 델리게이트

	/** 배치 데이터 변경 시 */
	UPROPERTY(BlueprintAssignable, Category = "JumpMapEditor|Placement")
	FOnJumpMapEditorPlacementDataChanged OnPlacementDataChanged;

protected:
	virtual void BeginPlay() override;

	/** 배치 데이터 (로컬, 리플리케이션 불필요) */
	UPROPERTY()
	TArray<FPlacedObjectSaveEntry> PlacedObjects;

private:
	/** 카탈로그 참조 */
	UPROPERTY()
	TObjectPtr<UWjWorldPlaceableObjectDataAsset> Catalog;

	/** 현재 스폰된 오브젝트 액터들 */
	UPROPERTY()
	TArray<TObjectPtr<AWjWorldPlacedObjectActor>> SpawnedActors;

	/** 스폰된 오브젝트들 제거 후 재스폰 */
	void RespawnAllPlacedObjects();

	/** Catalog가 설정되지 않았으면 DeveloperSettings에서 로드 */
	void EnsureCatalogLoaded();

	/** SaveGame 로드 */
	void LoadLayoutFromSaveGame();
};

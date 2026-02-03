// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldGameStateBase.h"
#include "Save/WjWorldLayoutSaveGame.h"
#include "WjWorldGameStateLobby.generated.h"

class AWjWorldPlacedObjectActor;
class UWjWorldPlaceableObjectDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlacementDataChanged);

/**
 * 로비 게임 상태
 * 배치 데이터 리플리케이션을 담당
 */
UCLASS()
class WJWORLD_API AWjWorldGameStateLobby : public AWjWorldGameStateBase
{
	GENERATED_BODY()

public:
	AWjWorldGameStateLobby();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ 배치 데이터 관리 (서버)

	/** 배치 오브젝트 추가 (서버에서 호출) */
	void AddPlacedObject(const FPlacedObjectSaveEntry& Entry);

	/** 배치 오브젝트 제거 (서버에서 호출) */
	void RemovePlacedObjectAt(int32 Index);

	/** 전체 배치 데이터 설정 (서버에서 호출, 초기 로드 시) */
	void SetPlacedObjects(const TArray<FPlacedObjectSaveEntry>& InPlacedObjects);

	/** 배치 데이터 전체 클리어 (서버에서 호출) */
	void ClearPlacedObjects();

	//~ Getter

	const TArray<FPlacedObjectSaveEntry>& GetPlacedObjects() const { return PlacedObjects; }
	int32 GetPlacedObjectCount() const { return PlacedObjects.Num(); }

	//~ 델리게이트

	/** 배치 데이터 변경 시 (클라이언트에서 오브젝트 재스폰용) */
	UPROPERTY(BlueprintAssignable, Category = "Lobby|Placement")
	FOnPlacementDataChanged OnPlacementDataChanged;

	//~ 카탈로그 (오브젝트 스폰에 필요)

	void SetCatalog(UWjWorldPlaceableObjectDataAsset* InCatalog) { Catalog = InCatalog; }
	UWjWorldPlaceableObjectDataAsset* GetCatalog() const { return Catalog; }

protected:
	/** 리플리케이트되는 배치 데이터 */
	UPROPERTY(ReplicatedUsing = OnRep_PlacedObjects)
	TArray<FPlacedObjectSaveEntry> PlacedObjects;

	UFUNCTION()
	void OnRep_PlacedObjects();

private:
	/** 카탈로그 참조 (리플리케이트 안함, 각 클라이언트가 로컬 로드) */
	UPROPERTY()
	TObjectPtr<UWjWorldPlaceableObjectDataAsset> Catalog;

	/** 현재 스폰된 오브젝트 액터들 (클라이언트 로컬) */
	UPROPERTY()
	TArray<TObjectPtr<AWjWorldPlacedObjectActor>> SpawnedActors;

	/** 스폰된 오브젝트들 제거 후 재스폰 */
	void RespawnAllPlacedObjects();

	/** Catalog가 설정되지 않았으면 DeveloperSettings에서 로드 (폴백) */
	void EnsureCatalogLoaded();
};

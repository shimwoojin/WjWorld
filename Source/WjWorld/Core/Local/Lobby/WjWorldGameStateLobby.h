// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldGameStateBase.h"
#include "GamePlay/Placement/IWjWorldPlacementDataProvider.h"
#include "Save/WjWorldLayoutSaveGame.h"
#include "WjWorldGameStateLobby.generated.h"

class AWjWorldPlacedObjectActor;
class UWjWorldPlaceableObjectDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlacementDataChanged);

/**
 * 로비 게임 상태
 * 배치 데이터 리플리케이션을 담당
 * IWjWorldPlacementDataProvider 인터페이스 구현
 */
UCLASS()
class WJWORLD_API AWjWorldGameStateLobby : public AWjWorldGameStateBase, public IWjWorldPlacementDataProvider
{
	GENERATED_BODY()

public:
	AWjWorldGameStateLobby();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ IWjWorldPlacementDataProvider 인터페이스 구현
	virtual void AddPlacedObject(const FPlacedObjectSaveEntry& Entry) override;
	virtual void RemovePlacedObjectAt(int32 Index) override;
	virtual const TArray<FPlacedObjectSaveEntry>& GetPlacedObjects() const override { return PlacedObjects; }
	virtual void SetPlacedObjects(const TArray<FPlacedObjectSaveEntry>& InPlacedObjects) override;
	virtual void ClearPlacedObjects() override;
	virtual void SetPlacementCatalog(UWjWorldPlaceableObjectDataAsset* InCatalog) override { Catalog = InCatalog; }
	virtual UWjWorldPlaceableObjectDataAsset* GetPlacementCatalog() const override { return Catalog; }

	//~ Getter (하위 호환성 유지)

	int32 GetPlacedObjectCount() const { return PlacedObjects.Num(); }

	/** @deprecated SetPlacementCatalog 사용 권장 */
	void SetCatalog(UWjWorldPlaceableObjectDataAsset* InCatalog) { Catalog = InCatalog; }
	/** @deprecated GetPlacementCatalog 사용 권장 */
	UWjWorldPlaceableObjectDataAsset* GetCatalog() const { return Catalog; }

	//~ 델리게이트

	/** 배치 데이터 변경 시 (클라이언트에서 오브젝트 재스폰용) */
	UPROPERTY(BlueprintAssignable, Category = "Lobby|Placement")
	FOnPlacementDataChanged OnPlacementDataChanged;

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

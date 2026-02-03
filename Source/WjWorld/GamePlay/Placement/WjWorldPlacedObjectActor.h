// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StreamableManager.h"
#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"
#include "WjWorldPlacedObjectActor.generated.h"

class UStaticMeshComponent;

/**
 * 실제 배치된 오브젝트 액터
 */
UCLASS()
class WJWORLD_API AWjWorldPlacedObjectActor : public AActor
{
	GENERATED_BODY()

public:
	AWjWorldPlacedObjectActor();

	/** 오브젝트 정의로 초기화 (배치 시) */
	void InitializeFromDefinition(const FPlaceableObjectDefinition& Definition);

	/** 세이브 데이터로 복원 */
	void InitializeFromSaveData(FName InObjectId, const FTransform& InTransform, const FPlaceableObjectDefinition& Definition);

	/** 삭제 모드 호버 하이라이트 */
	void SetHighlighted(bool bHighlight);

	/** 오브젝트 ID */
	FName GetObjectId() const { return ObjectId; }

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

private:
	void LoadMesh(const FPlaceableObjectDefinition& Definition);
	void OnMeshLoaded();

	FStreamableManager StreamableManager;
	TSharedPtr<FStreamableHandle> MeshLoadHandle;
	FName ObjectId;
	FVector CachedScale = FVector::OneVector;
	bool bIsHighlighted = false;
};

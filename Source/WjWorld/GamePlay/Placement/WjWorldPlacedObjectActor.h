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

	/** 배치 모드 색상 구분 (배치된 오브젝트를 레벨 지오메트리와 구별) */
	void SetPlacementModeVisual(bool bEnable);

	/** 오브젝트 ID */
	FName GetObjectId() const { return ObjectId; }

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

private:
	void LoadMesh(const FPlaceableObjectDefinition& Definition);
	void OnMeshLoaded();

	void ApplyPlacementTint();
	void RemovePlacementTint();

	FStreamableManager StreamableManager;
	TSharedPtr<FStreamableHandle> MeshLoadHandle;
	FName ObjectId;
	FVector CachedScale = FVector::OneVector;
	bool bIsHighlighted = false;
	bool bShowPlacementVisual = false;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInterface>> OriginalMaterials;

	static const FLinearColor PlacementTintColor;
	static constexpr float PlacementTintOpacity = 0.7f;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WjWorldPlaceableObjectDataAsset.generated.h"

class AWjWorldPlacedObjectActor;

/**
 * 배치 가능한 오브젝트 정의
 */
USTRUCT(BlueprintType)
struct FPlaceableObjectDefinition
{
	GENERATED_BODY()

	/** 고유 식별자 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ObjectId;

	/** UI 표시명 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	/** 카테고리 (가구, 장식 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Category;

	/** 메시 (비동기 로드) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> Mesh;

	/** 카탈로그 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	/** 기본 스케일 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector DefaultScale = FVector::OneVector;

	/** 바닥 오프셋 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GroundOffset = 0.0f;

	/** 회전 단위 (도) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1.0"))
	float RotationSnapDegrees = 90.0f;

	/** 스폰할 액터 클래스 (nullptr이면 기본 AWjWorldPlacedObjectActor) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AWjWorldPlacedObjectActor> ActorClassOverride;
};

/**
 * 배치 가능한 오브젝트 카탈로그 데이터 에셋
 */
UCLASS(BlueprintType)
class WJWORLD_API UWjWorldPlaceableObjectDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 배치 가능한 오브젝트 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
	TArray<FPlaceableObjectDefinition> Objects;

	/** ObjectId로 오브젝트 정의 검색 */
	const FPlaceableObjectDefinition* FindByObjectId(FName ObjectId) const;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StreamableManager.h"
#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"
#include "WjWorldPlacementPreviewActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/** 배치 회전 축 */
UENUM()
enum class EPlacementRotationAxis : uint8
{
	Yaw,    // Z축 (기본)
	Pitch,  // Y축
	Roll    // X축
};

/**
 * 배치 프리뷰 액터
 * 마우스를 따라다니며 유효/무효 색상을 표시
 */
UCLASS()
class WJWORLD_API AWjWorldPlacementPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	AWjWorldPlacementPreviewActor();

	/** 오브젝트 정의로 프리뷰 초기화 (메시 비동기 로드) */
	void InitializePreview(const FPlaceableObjectDefinition& Definition);

	/** 프리뷰 위치/회전 갱신 */
	void UpdatePreviewTransform(const FVector& Location, const FRotator& Rotation);

	/** 유효/무효 색상 전환 */
	void SetPreviewValid(bool bInIsValid);

	/** 현재 축 기준 회전 누적 */
	void RotatePreview(float DeltaDegrees);

	/** 회전 축 전환 (Yaw→Pitch→Roll 사이클) */
	void CycleRotationAxis();

	/** 현재 회전 값 */
	FRotator GetCurrentRotation() const { return CurrentRotation; }

	/** 현재 회전 축 */
	EPlacementRotationAxis GetCurrentRotationAxis() const { return CurrentRotationAxis; }

	/** 유효 상태 */
	bool IsPreviewValid() const { return bIsValid; }

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> PreviewMeshComponent;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	FLinearColor ValidColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f);

	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	FLinearColor InvalidColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);

	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	float PreviewOpacity = 0.5f;

private:
	void OnMeshLoaded();

	FStreamableManager StreamableManager;
	TSharedPtr<FStreamableHandle> MeshLoadHandle;
	FRotator CurrentRotation = FRotator::ZeroRotator;
	EPlacementRotationAxis CurrentRotationAxis = EPlacementRotationAxis::Yaw;
	bool bIsValid = true;
	FVector CachedScale = FVector::OneVector;
};

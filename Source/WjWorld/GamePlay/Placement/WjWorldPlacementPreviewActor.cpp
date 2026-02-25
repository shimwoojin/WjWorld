// Fill out your copyright notice in the Description page of Project Settings.

#include "GamePlay/Placement/WjWorldPlacementPreviewActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "WjWorldLogCategories.h"

AWjWorldPlacementPreviewActor::AWjWorldPlacementPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	PreviewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMeshComponent"));
	RootComponent = PreviewMeshComponent;

	// 충돌 비활성화 (프리뷰는 물리 없음)
	PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMeshComponent->SetCastShadow(false);
}

void AWjWorldPlacementPreviewActor::InitializePreview(const FPlaceableObjectDefinition& Definition)
{
	CachedScale = Definition.DefaultScale;

	// 메시 비동기 로드
	if (!Definition.Mesh.IsNull())
	{
		MeshLoadHandle = StreamableManager.RequestAsyncLoad(
			Definition.Mesh.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &AWjWorldPlacementPreviewActor::OnMeshLoaded)
		);
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementPreviewActor: Mesh is null for object"));
	}
}

void AWjWorldPlacementPreviewActor::OnMeshLoaded()
{
	if (MeshLoadHandle.IsValid() && MeshLoadHandle->HasLoadCompleted())
	{
		UStaticMesh* LoadedMesh = Cast<UStaticMesh>(MeshLoadHandle->GetLoadedAsset());
		if (LoadedMesh && PreviewMeshComponent)
		{
			PreviewMeshComponent->SetStaticMesh(LoadedMesh);
			PreviewMeshComponent->SetWorldScale3D(CachedScale);

			// 동적 머티리얼 생성
			DynamicMaterial = PreviewMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
			if (DynamicMaterial)
			{
				DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), PreviewOpacity);
				SetPreviewValid(bIsValid);
			}

			UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementPreviewActor: Mesh loaded successfully"));
		}
	}
}

void AWjWorldPlacementPreviewActor::UpdatePreviewTransform(const FVector& Location, const FRotator& Rotation)
{
	SetActorLocation(Location);
	SetActorRotation(Rotation);
}

void AWjWorldPlacementPreviewActor::SetPreviewValid(bool bInIsValid)
{
	bIsValid = bInIsValid;

	if (DynamicMaterial)
	{
		FLinearColor Color = bIsValid ? ValidColor : InvalidColor;
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}
}

void AWjWorldPlacementPreviewActor::RotatePreview(float DeltaDegrees)
{
	switch (CurrentRotationAxis)
	{
	case EPlacementRotationAxis::Yaw:
		CurrentRotation.Yaw = FRotator::NormalizeAxis(CurrentRotation.Yaw + DeltaDegrees);
		break;
	case EPlacementRotationAxis::Pitch:
		CurrentRotation.Pitch = FRotator::NormalizeAxis(CurrentRotation.Pitch + DeltaDegrees);
		break;
	case EPlacementRotationAxis::Roll:
		CurrentRotation.Roll = FRotator::NormalizeAxis(CurrentRotation.Roll + DeltaDegrees);
		break;
	}
}

void AWjWorldPlacementPreviewActor::CycleRotationAxis()
{
	switch (CurrentRotationAxis)
	{
	case EPlacementRotationAxis::Yaw:   CurrentRotationAxis = EPlacementRotationAxis::Pitch; break;
	case EPlacementRotationAxis::Pitch:  CurrentRotationAxis = EPlacementRotationAxis::Roll;  break;
	case EPlacementRotationAxis::Roll:   CurrentRotationAxis = EPlacementRotationAxis::Yaw;   break;
	}
}

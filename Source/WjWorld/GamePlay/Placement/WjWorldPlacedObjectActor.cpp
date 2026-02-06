// Fill out your copyright notice in the Description page of Project Settings.

#include "GamePlay/Placement/WjWorldPlacedObjectActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "WjWorldLogCategories.h"

AWjWorldPlacedObjectActor::AWjWorldPlacedObjectActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// 물리/충돌 활성화
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AWjWorldPlacedObjectActor::InitializeFromDefinition(const FPlaceableObjectDefinition& Definition)
{
	ObjectId = Definition.ObjectId;
	CachedScale = Definition.DefaultScale;
	LoadMesh(Definition);
}

void AWjWorldPlacedObjectActor::InitializeFromSaveData(FName InObjectId, const FTransform& InTransform, const FPlaceableObjectDefinition& Definition)
{
	ObjectId = InObjectId;
	CachedScale = Definition.DefaultScale;
	SetActorTransform(InTransform);

	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacedObjectActor::InitializeFromSaveData - ObjectId=%s, Location=%s, Scale=%s, MeshPath=%s"),
		*ObjectId.ToString(),
		*InTransform.GetLocation().ToString(),
		*CachedScale.ToString(),
		*Definition.Mesh.ToString());

	LoadMesh(Definition);
}

void AWjWorldPlacedObjectActor::LoadMesh(const FPlaceableObjectDefinition& Definition)
{
	if (!Definition.Mesh.IsNull())
	{
		FSoftObjectPath MeshPath = Definition.Mesh.ToSoftObjectPath();
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacedObjectActor::LoadMesh - Starting async load for ObjectId=%s, Path=%s"),
			*ObjectId.ToString(), *MeshPath.ToString());

		MeshLoadHandle = StreamableManager.RequestAsyncLoad(
			MeshPath,
			FStreamableDelegate::CreateUObject(this, &AWjWorldPlacedObjectActor::OnMeshLoaded)
		);

		if (!MeshLoadHandle.IsValid())
		{
			UE_LOG(LogWjWorldPlacement, Error, TEXT("PlacedObjectActor::LoadMesh - RequestAsyncLoad returned invalid handle for ObjectId=%s"), *ObjectId.ToString());
		}
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacedObjectActor: Mesh is null for ObjectId=%s"), *ObjectId.ToString());
	}
}

void AWjWorldPlacedObjectActor::OnMeshLoaded()
{
	if (!MeshLoadHandle.IsValid())
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("PlacedObjectActor::OnMeshLoaded - MeshLoadHandle is invalid for ObjectId=%s"), *ObjectId.ToString());
		return;
	}

	if (!MeshLoadHandle->HasLoadCompleted())
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("PlacedObjectActor::OnMeshLoaded - Load not completed for ObjectId=%s"), *ObjectId.ToString());
		return;
	}

	UStaticMesh* LoadedMesh = Cast<UStaticMesh>(MeshLoadHandle->GetLoadedAsset());
	if (!LoadedMesh)
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("PlacedObjectActor::OnMeshLoaded - LoadedMesh is null for ObjectId=%s (Asset: %s)"),
			*ObjectId.ToString(),
			*MeshLoadHandle->GetDebugName());
		return;
	}

	if (!MeshComponent)
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("PlacedObjectActor::OnMeshLoaded - MeshComponent is null for ObjectId=%s"), *ObjectId.ToString());
		return;
	}

	MeshComponent->SetStaticMesh(LoadedMesh);
	MeshComponent->SetWorldScale3D(CachedScale);

	// 가시성 명시적으로 활성화
	MeshComponent->SetVisibility(true);
	MeshComponent->SetHiddenInGame(false);

	// 머티리얼 체크
	int32 NumMaterials = MeshComponent->GetNumMaterials();
	FString MaterialInfo = FString::Printf(TEXT("NumMaterials=%d"), NumMaterials);
	for (int32 i = 0; i < NumMaterials; ++i)
	{
		UMaterialInterface* Mat = MeshComponent->GetMaterial(i);
		MaterialInfo += FString::Printf(TEXT(", Mat[%d]=%s"), i, Mat ? *Mat->GetName() : TEXT("NULL"));
	}

	// 추가 디버깅: 가시성 및 바운드 확인
	FBoxSphereBounds Bounds = MeshComponent->Bounds;
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacedObjectActor::OnMeshLoaded - SUCCESS for ObjectId=%s, MeshName=%s, Scale=%s, BoundsExtent=%s, Visible=%d, HiddenInGame=%d, %s"),
		*ObjectId.ToString(),
		*LoadedMesh->GetName(),
		*CachedScale.ToString(),
		*Bounds.BoxExtent.ToString(),
		MeshComponent->IsVisible(),
		IsHidden(),
		*MaterialInfo);
}

void AWjWorldPlacedObjectActor::SetHighlighted(bool bHighlight)
{
	if (bIsHighlighted == bHighlight)
	{
		return;
	}

	bIsHighlighted = bHighlight;

	if (MeshComponent)
	{
		MeshComponent->SetRenderCustomDepth(bHighlight);
		MeshComponent->SetCustomDepthStencilValue(bHighlight ? 1 : 0);
	}
}

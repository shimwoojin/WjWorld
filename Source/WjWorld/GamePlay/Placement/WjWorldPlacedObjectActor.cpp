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
	LoadMesh(Definition);
}

void AWjWorldPlacedObjectActor::LoadMesh(const FPlaceableObjectDefinition& Definition)
{
	if (!Definition.Mesh.IsNull())
	{
		MeshLoadHandle = StreamableManager.RequestAsyncLoad(
			Definition.Mesh.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &AWjWorldPlacedObjectActor::OnMeshLoaded)
		);
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacedObjectActor: Mesh is null for ObjectId=%s"), *ObjectId.ToString());
	}
}

void AWjWorldPlacedObjectActor::OnMeshLoaded()
{
	if (MeshLoadHandle.IsValid() && MeshLoadHandle->HasLoadCompleted())
	{
		UStaticMesh* LoadedMesh = Cast<UStaticMesh>(MeshLoadHandle->GetLoadedAsset());
		if (LoadedMesh && MeshComponent)
		{
			MeshComponent->SetStaticMesh(LoadedMesh);
			MeshComponent->SetWorldScale3D(CachedScale);
			UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacedObjectActor: Mesh loaded for ObjectId=%s"), *ObjectId.ToString());
		}
	}
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

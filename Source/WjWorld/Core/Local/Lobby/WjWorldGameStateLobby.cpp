// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Local/Lobby/WjWorldGameStateLobby.h"
#include "GamePlay/Placement/WjWorldPlacedObjectActor.h"
#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Net/UnrealNetwork.h"
#include "WjWorldLogCategories.h"

AWjWorldGameStateLobby::AWjWorldGameStateLobby()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AWjWorldGameStateLobby::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWjWorldGameStateLobby, PlacedObjects);
}

void AWjWorldGameStateLobby::AddPlacedObject(const FPlacedObjectSaveEntry& Entry)
{
	if (!HasAuthority())
	{
		return;
	}

	PlacedObjects.Add(Entry);

	// 서버에서도 즉시 오브젝트 스폰 (클라이언트는 OnRep에서 처리)
	RespawnAllPlacedObjects();

	OnPlacementDataChanged.Broadcast();

	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateLobby: Added object %s (Total: %d)"),
		*Entry.ObjectId.ToString(), PlacedObjects.Num());
}

void AWjWorldGameStateLobby::RemovePlacedObjectAt(int32 Index)
{
	if (!HasAuthority())
	{
		return;
	}

	if (PlacedObjects.IsValidIndex(Index))
	{
		FName RemovedId = PlacedObjects[Index].ObjectId;
		PlacedObjects.RemoveAt(Index);

		// 서버에서도 즉시 오브젝트 재스폰 (클라이언트는 OnRep에서 처리)
		RespawnAllPlacedObjects();

		OnPlacementDataChanged.Broadcast();

		UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateLobby: Removed object %s at index %d (Total: %d)"),
			*RemovedId.ToString(), Index, PlacedObjects.Num());
	}
}

void AWjWorldGameStateLobby::SetPlacedObjects(const TArray<FPlacedObjectSaveEntry>& InPlacedObjects)
{
	if (!HasAuthority())
	{
		return;
	}

	PlacedObjects = InPlacedObjects;

	// 서버에서도 즉시 오브젝트 스폰
	RespawnAllPlacedObjects();

	OnPlacementDataChanged.Broadcast();

	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateLobby: Set %d placed objects"), PlacedObjects.Num());
}

void AWjWorldGameStateLobby::ClearPlacedObjects()
{
	if (!HasAuthority())
	{
		return;
	}

	PlacedObjects.Empty();

	// 서버에서도 즉시 오브젝트 제거
	RespawnAllPlacedObjects();

	OnPlacementDataChanged.Broadcast();

	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateLobby: Cleared all placed objects"));
}

void AWjWorldGameStateLobby::OnRep_PlacedObjects()
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateLobby: OnRep_PlacedObjects - Received %d objects"), PlacedObjects.Num());

	// 클라이언트에서 오브젝트 재스폰
	RespawnAllPlacedObjects();

	// 델리게이트 브로드캐스트
	OnPlacementDataChanged.Broadcast();
}

void AWjWorldGameStateLobby::RespawnAllPlacedObjects()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 기존 스폰된 오브젝트 제거
	for (AWjWorldPlacedObjectActor* Actor : SpawnedActors)
	{
		if (Actor && !Actor->IsPendingKillPending())
		{
			Actor->Destroy();
		}
	}
	SpawnedActors.Empty();

	// 카탈로그 폴백 로드 시도
	EnsureCatalogLoaded();

	// 카탈로그 없으면 스폰 불가
	if (!Catalog)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("GameStateLobby: Cannot respawn objects - no catalog set"));
		return;
	}

	// 새로 스폰
	for (const FPlacedObjectSaveEntry& Entry : PlacedObjects)
	{
		const FPlaceableObjectDefinition* Def = Catalog->FindByObjectId(Entry.ObjectId);
		if (!Def)
		{
			UE_LOG(LogWjWorldPlacement, Warning, TEXT("GameStateLobby: ObjectId %s not found in catalog"),
				*Entry.ObjectId.ToString());
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AWjWorldPlacedObjectActor* NewActor = World->SpawnActor<AWjWorldPlacedObjectActor>(
			AWjWorldPlacedObjectActor::StaticClass(),
			Entry.Transform.GetLocation(),
			Entry.Transform.GetRotation().Rotator(),
			SpawnParams
		);

		if (NewActor)
		{
			NewActor->InitializeFromSaveData(Entry.ObjectId, Entry.Transform, *Def);
			SpawnedActors.Add(NewActor);
		}
	}

	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateLobby: Respawned %d objects"), SpawnedActors.Num());
}

void AWjWorldGameStateLobby::EnsureCatalogLoaded()
{
	// 이미 Catalog가 설정되어 있으면 스킵
	if (Catalog)
	{
		return;
	}

	// DeveloperSettings에서 Catalog 로드 (폴백 메커니즘)
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings || Settings->PlaceableObjectCatalog.IsNull())
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("GameStateLobby: No PlaceableObjectCatalog configured in DeveloperSettings"));
		return;
	}

	Catalog = Settings->PlaceableObjectCatalog.LoadSynchronous();
	if (Catalog)
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateLobby: Catalog loaded from DeveloperSettings (fallback)"));
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("GameStateLobby: Failed to load PlaceableObjectCatalog"));
	}
}

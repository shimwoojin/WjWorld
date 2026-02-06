// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Editor/AWEditor/WjWorldGameStateAWEditor.h"
#include "GamePlay/Placement/WjWorldPlacedObjectActor.h"
#include "GamePlay/Placement/WjWorldPlacementTypes.h"
#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

AWjWorldGameStateAWEditor::AWjWorldGameStateAWEditor()
{
	// 싱글플레이어 전용이므로 리플리케이션 불필요
	bReplicates = false;
}

void AWjWorldGameStateAWEditor::BeginPlay()
{
	Super::BeginPlay();

	// 카탈로그 로드 후 SaveGame 로드
	EnsureCatalogLoaded();
	LoadLayoutFromSaveGame();
}

void AWjWorldGameStateAWEditor::AddPlacedObject(const FPlacedObjectSaveEntry& Entry)
{
	PlacedObjects.Add(Entry);
	RespawnAllPlacedObjects();
	OnPlacementDataChanged.Broadcast();

	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateAWEditor: Added object %s (Total: %d)"),
		*Entry.ObjectId.ToString(), PlacedObjects.Num());
}

void AWjWorldGameStateAWEditor::RemovePlacedObjectAt(int32 Index)
{
	if (PlacedObjects.IsValidIndex(Index))
	{
		FName RemovedId = PlacedObjects[Index].ObjectId;
		PlacedObjects.RemoveAt(Index);
		RespawnAllPlacedObjects();
		OnPlacementDataChanged.Broadcast();

		UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateAWEditor: Removed object %s at index %d (Total: %d)"),
			*RemovedId.ToString(), Index, PlacedObjects.Num());
	}
}

void AWjWorldGameStateAWEditor::SetPlacedObjects(const TArray<FPlacedObjectSaveEntry>& InPlacedObjects)
{
	PlacedObjects = InPlacedObjects;
	RespawnAllPlacedObjects();
	OnPlacementDataChanged.Broadcast();

	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateAWEditor: Set %d placed objects"), PlacedObjects.Num());
}

void AWjWorldGameStateAWEditor::ClearPlacedObjects()
{
	PlacedObjects.Empty();
	RespawnAllPlacedObjects();
	OnPlacementDataChanged.Broadcast();

	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateAWEditor: Cleared all placed objects"));
}

void AWjWorldGameStateAWEditor::RespawnAllPlacedObjects()
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
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("GameStateAWEditor: Cannot respawn objects - no catalog set"));
		return;
	}

	// 새로 스폰
	for (const FPlacedObjectSaveEntry& Entry : PlacedObjects)
	{
		const FPlaceableObjectDefinition* Def = Catalog->FindByObjectId(Entry.ObjectId);
		if (!Def)
		{
			UE_LOG(LogWjWorldPlacement, Warning, TEXT("GameStateAWEditor: ObjectId %s not found in catalog"),
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

	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateAWEditor: Respawned %d objects"), SpawnedActors.Num());
}

void AWjWorldGameStateAWEditor::EnsureCatalogLoaded()
{
	if (Catalog)
	{
		return;
	}

	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("GameStateAWEditor: DeveloperSettings not found"));
		return;
	}

	// ApproachingWall 컨텍스트 카탈로그 로드
	Catalog = Settings->GetPlaceableCatalogForContext(EPlacementContext::ApproachingWall);
	if (Catalog)
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateAWEditor: Catalog loaded for ApproachingWall context"));
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("GameStateAWEditor: Failed to load PlaceableObjectCatalog"));
	}
}

void AWjWorldGameStateAWEditor::LoadLayoutFromSaveGame()
{
	FString SlotName = GetSaveSlotNameForContext(EPlacementContext::ApproachingWall);

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateAWEditor: No save game found at slot '%s'"), *SlotName);
		return;
	}

	UWjWorldLayoutSaveGame* SaveGame = Cast<UWjWorldLayoutSaveGame>(
		UGameplayStatics::LoadGameFromSlot(SlotName, 0)
	);

	if (!SaveGame)
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("GameStateAWEditor: Failed to load save game from slot '%s'"), *SlotName);
		return;
	}

	SetPlacedObjects(SaveGame->PlacedObjects);
	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateAWEditor: Loaded %d objects from slot '%s'"),
		SaveGame->PlacedObjects.Num(), *SlotName);
}

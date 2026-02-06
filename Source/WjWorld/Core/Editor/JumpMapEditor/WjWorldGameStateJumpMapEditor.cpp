// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Editor/JumpMapEditor/WjWorldGameStateJumpMapEditor.h"
#include "GamePlay/Placement/WjWorldPlacedObjectActor.h"
#include "GamePlay/Placement/WjWorldPlacementTypes.h"
#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

AWjWorldGameStateJumpMapEditor::AWjWorldGameStateJumpMapEditor()
{
	// 싱글플레이어 전용이므로 리플리케이션 불필요
	bReplicates = false;
}

void AWjWorldGameStateJumpMapEditor::BeginPlay()
{
	Super::BeginPlay();

	// 카탈로그 로드 후 SaveGame 로드
	EnsureCatalogLoaded();
	LoadLayoutFromSaveGame();
}

void AWjWorldGameStateJumpMapEditor::AddPlacedObject(const FPlacedObjectSaveEntry& Entry)
{
	PlacedObjects.Add(Entry);
	RespawnAllPlacedObjects();
	OnPlacementDataChanged.Broadcast();

	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateJumpMapEditor: Added object %s (Total: %d)"),
		*Entry.ObjectId.ToString(), PlacedObjects.Num());
}

void AWjWorldGameStateJumpMapEditor::RemovePlacedObjectAt(int32 Index)
{
	if (PlacedObjects.IsValidIndex(Index))
	{
		FName RemovedId = PlacedObjects[Index].ObjectId;
		PlacedObjects.RemoveAt(Index);
		RespawnAllPlacedObjects();
		OnPlacementDataChanged.Broadcast();

		UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateJumpMapEditor: Removed object %s at index %d (Total: %d)"),
			*RemovedId.ToString(), Index, PlacedObjects.Num());
	}
}

void AWjWorldGameStateJumpMapEditor::SetPlacedObjects(const TArray<FPlacedObjectSaveEntry>& InPlacedObjects)
{
	PlacedObjects = InPlacedObjects;
	RespawnAllPlacedObjects();
	OnPlacementDataChanged.Broadcast();

	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateJumpMapEditor: Set %d placed objects"), PlacedObjects.Num());
}

void AWjWorldGameStateJumpMapEditor::ClearPlacedObjects()
{
	PlacedObjects.Empty();
	RespawnAllPlacedObjects();
	OnPlacementDataChanged.Broadcast();

	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateJumpMapEditor: Cleared all placed objects"));
}

void AWjWorldGameStateJumpMapEditor::RespawnAllPlacedObjects()
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
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("GameStateJumpMapEditor: Cannot respawn objects - no catalog set"));
		return;
	}

	// 새로 스폰
	for (const FPlacedObjectSaveEntry& Entry : PlacedObjects)
	{
		const FPlaceableObjectDefinition* Def = Catalog->FindByObjectId(Entry.ObjectId);
		if (!Def)
		{
			UE_LOG(LogWjWorldPlacement, Warning, TEXT("GameStateJumpMapEditor: ObjectId %s not found in catalog"),
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

	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateJumpMapEditor: Respawned %d objects"), SpawnedActors.Num());
}

void AWjWorldGameStateJumpMapEditor::EnsureCatalogLoaded()
{
	if (Catalog)
	{
		return;
	}

	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("GameStateJumpMapEditor: DeveloperSettings not found"));
		return;
	}

	// JumpMap 컨텍스트 카탈로그 로드
	Catalog = Settings->GetPlaceableCatalogForContext(EPlacementContext::JumpMap);
	if (Catalog)
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateJumpMapEditor: Catalog loaded for JumpMap context"));
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("GameStateJumpMapEditor: Failed to load PlaceableObjectCatalog"));
	}
}

void AWjWorldGameStateJumpMapEditor::LoadLayoutFromSaveGame()
{
	FString SlotName = GetSaveSlotNameForContext(EPlacementContext::JumpMap);

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateJumpMapEditor: No save game found at slot '%s'"), *SlotName);
		return;
	}

	UWjWorldLayoutSaveGame* SaveGame = Cast<UWjWorldLayoutSaveGame>(
		UGameplayStatics::LoadGameFromSlot(SlotName, 0)
	);

	if (!SaveGame)
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("GameStateJumpMapEditor: Failed to load save game from slot '%s'"), *SlotName);
		return;
	}

	SetPlacedObjects(SaveGame->PlacedObjects);
	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameStateJumpMapEditor: Loaded %d objects from slot '%s'"),
		SaveGame->PlacedObjects.Num(), *SlotName);
}

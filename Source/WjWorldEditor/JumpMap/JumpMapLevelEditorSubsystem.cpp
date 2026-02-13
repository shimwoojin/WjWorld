// Fill out your copyright notice in the Description page of Project Settings.

#include "JumpMap/JumpMapLevelEditorSubsystem.h"
#include "GamePlay/JumpMap/JumpMapActorBase.h"
#include "GamePlay/JumpMap/JumpMapLayoutDataAsset.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "EngineUtils.h"
#include "Editor.h"

bool UJumpMapLevelEditorSubsystem::SaveCurrentLevelLayout(const FString& LayoutName)
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("JumpMapLevelEditor: No editor world"));
		return false;
	}

	UJumpMapLayoutDataAsset* LayoutDA = GetLayoutDataAsset();
	if (!LayoutDA)
	{
		UE_LOG(LogTemp, Error, TEXT("JumpMapLevelEditor: JumpMapLayoutDataAsset not found in DeveloperSettings"));
		return false;
	}

	FJumpMapLayout Layout;
	Layout.LayoutName = LayoutName;

	// 현재 레벨의 모든 JumpMap 액터 수집
	for (TActorIterator<AJumpMapActorBase> It(World); It; ++It)
	{
		AJumpMapActorBase* Actor = *It;
		if (!Actor || Actor->GetJumpMapObjectId().IsNone() || Actor->bIsDefaultPlacement)
		{
			continue;
		}

		FJumpMapLayoutEntry Entry;
		Entry.ObjectId = Actor->GetJumpMapObjectId();
		Entry.Transform = Actor->GetActorTransform();
		Actor->GetSerializableProperties(Entry.CustomProperties);

		Layout.Objects.Add(Entry);
	}

	if (Layout.Objects.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("JumpMapLevelEditor: No JumpMap actors found in current level"));
		return false;
	}

	// 동일 이름의 기존 레이아웃이 있으면 갱신, 없으면 추가
	bool bUpdated = false;
	for (FJumpMapLayout& Existing : LayoutDA->BuiltInLayouts)
	{
		if (Existing.LayoutName == LayoutName)
		{
			Existing = Layout;
			bUpdated = true;
			break;
		}
	}

	if (!bUpdated)
	{
		LayoutDA->BuiltInLayouts.Add(Layout);
	}

	// DataAsset 패키지를 Dirty 마킹 (에디터에서 저장 가능하도록)
	LayoutDA->Modify();
	LayoutDA->MarkPackageDirty();

	UE_LOG(LogTemp, Log, TEXT("JumpMapLevelEditor: %s layout '%s' (%d actors) to DataAsset"),
		bUpdated ? TEXT("Updated") : TEXT("Added"), *LayoutName, Layout.Objects.Num());
	return true;
}

bool UJumpMapLevelEditorSubsystem::LoadLayoutToCurrentLevel(const FString& LayoutName)
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("JumpMapLevelEditor: No editor world"));
		return false;
	}

	UJumpMapLayoutDataAsset* LayoutDA = GetLayoutDataAsset();
	if (!LayoutDA)
	{
		UE_LOG(LogTemp, Error, TEXT("JumpMapLevelEditor: JumpMapLayoutDataAsset not found in DeveloperSettings"));
		return false;
	}

	// BuiltInLayouts에서 이름으로 검색
	const FJumpMapLayout* FoundLayout = nullptr;
	for (const FJumpMapLayout& Layout : LayoutDA->BuiltInLayouts)
	{
		if (Layout.LayoutName == LayoutName)
		{
			FoundLayout = &Layout;
			break;
		}
	}

	if (!FoundLayout)
	{
		UE_LOG(LogTemp, Warning, TEXT("JumpMapLevelEditor: Layout '%s' not found in DataAsset"), *LayoutName);
		return false;
	}

	if (FoundLayout->Objects.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("JumpMapLevelEditor: Layout '%s' has no objects"), *LayoutName);
		return false;
	}

	// 기존 액터 제거
	ClearAllJumpMapActors();

	// 에디터 트랜잭션으로 Undo 지원
	FScopedTransaction Transaction(FText::FromString(TEXT("Load JumpMap Layout")));

	int32 SpawnedCount = 0;
	for (const FJumpMapLayoutEntry& Entry : FoundLayout->Objects)
	{
		TSubclassOf<AJumpMapActorBase> ActorClass = GetActorClassForObjectId(Entry.ObjectId);
		if (!ActorClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("JumpMapLevelEditor: No class mapping for ObjectId '%s'"), *Entry.ObjectId.ToString());
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AJumpMapActorBase* NewActor = World->SpawnActor<AJumpMapActorBase>(ActorClass, Entry.Transform, SpawnParams);
		if (NewActor)
		{
			NewActor->ApplySerializedProperties(Entry.CustomProperties);
			SpawnedCount++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("JumpMapLevelEditor: Spawned %d/%d actors from layout '%s'"),
		SpawnedCount, FoundLayout->Objects.Num(), *LayoutName);
	return SpawnedCount > 0;
}

void UJumpMapLevelEditorSubsystem::ClearAllJumpMapActors()
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World) return;

	FScopedTransaction Transaction(FText::FromString(TEXT("Clear JumpMap Actors")));

	TArray<AJumpMapActorBase*> ActorsToDestroy;
	for (TActorIterator<AJumpMapActorBase> It(World); It; ++It)
	{
		if (!(*It)->bIsDefaultPlacement)
		{
			ActorsToDestroy.Add(*It);
		}
	}

	for (AJumpMapActorBase* Actor : ActorsToDestroy)
	{
		if (Actor)
		{
			Actor->Modify();
			World->DestroyActor(Actor);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("JumpMapLevelEditor: Cleared %d JumpMap actors"), ActorsToDestroy.Num());
}

TArray<FString> UJumpMapLevelEditorSubsystem::GetAvailableLayoutNames() const
{
	TArray<FString> LayoutNames;

	UJumpMapLayoutDataAsset* LayoutDA = GetLayoutDataAsset();
	if (!LayoutDA)
	{
		return LayoutNames;
	}

	for (const FJumpMapLayout& Layout : LayoutDA->BuiltInLayouts)
	{
		LayoutNames.Add(Layout.LayoutName);
	}

	return LayoutNames;
}

int32 UJumpMapLevelEditorSubsystem::GetJumpMapActorCount() const
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World) return 0;

	int32 Count = 0;
	for (TActorIterator<AJumpMapActorBase> It(World); It; ++It)
	{
		Count++;
	}
	return Count;
}

UJumpMapLayoutDataAsset* UJumpMapLevelEditorSubsystem::GetLayoutDataAsset() const
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings || Settings->JumpMapLayoutDataAsset.IsNull())
	{
		return nullptr;
	}

	return Settings->JumpMapLayoutDataAsset.LoadSynchronous();
}

TSubclassOf<AJumpMapActorBase> UJumpMapLevelEditorSubsystem::GetActorClassForObjectId(const FName& ObjectId) const
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings) return nullptr;

	const TSoftClassPtr<AJumpMapActorBase>* FoundClass = Settings->JumpMapObjectIdToClassMap.Find(ObjectId);
	if (!FoundClass || FoundClass->IsNull())
	{
		return nullptr;
	}

	return FoundClass->LoadSynchronous();
}

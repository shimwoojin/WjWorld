// Fill out your copyright notice in the Description page of Project Settings.

#include "JumpMap/JumpMapLevelEditorSubsystem.h"
#include "GamePlay/JumpMap/JumpMapActorBase.h"
#include "GamePlay/JumpMap/JumpMapLayoutDataAsset.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "EngineUtils.h"
#include "Editor.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

bool UJumpMapLevelEditorSubsystem::SaveCurrentLevelLayout(const FString& LayoutName)
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("JumpMapLevelEditor: No editor world"));
		return false;
	}

	FJumpMapLayout Layout;
	Layout.LayoutName = LayoutName;

	// 현재 레벨의 모든 JumpMap 액터 수집
	for (TActorIterator<AJumpMapActorBase> It(World); It; ++It)
	{
		AJumpMapActorBase* Actor = *It;
		if (!Actor || Actor->GetJumpMapObjectId().IsNone())
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

	// CSV 내보내기
	FString CSVContent = UJumpMapLayoutDataAsset::ExportLayoutToCSV(Layout);

	// 저장 디렉토리 생성
	FString SaveDir = GetUserLayoutDirectory();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*SaveDir))
	{
		PlatformFile.CreateDirectoryTree(*SaveDir);
	}

	FString FilePath = SaveDir / FString::Printf(TEXT("%s.csv"), *LayoutName);

	if (!FFileHelper::SaveStringToFile(CSVContent, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("JumpMapLevelEditor: Failed to save layout to: %s"), *FilePath);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("JumpMapLevelEditor: Saved %d actors to '%s'"), Layout.Objects.Num(), *FilePath);
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

	// CSV 파일 로드
	FString FilePath = GetUserLayoutDirectory() / FString::Printf(TEXT("%s.csv"), *LayoutName);
	FString FileContent;

	if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("JumpMapLevelEditor: Failed to load file: %s"), *FilePath);
		return false;
	}

	// 임시 DataAsset을 통해 CSV 파싱
	UJumpMapLayoutDataAsset* TempAsset = NewObject<UJumpMapLayoutDataAsset>();
	TArray<FJumpMapLayout> ParsedLayouts;
	// ParseLayoutCSV는 private이므로 ScanUserJumpMapLayouts 패턴 대신 직접 파싱
	// ExportLayoutToCSV의 역함수가 필요하므로 수동 파싱
	FJumpMapLayout Layout;
	{
		TArray<FString> Lines;
		FileContent.ParseIntoArrayLines(Lines);
		bool bHeaderParsed = false;

		for (const FString& Line : Lines)
		{
			if (Line.IsEmpty()) continue;

			if (Line.StartsWith(TEXT("#")))
			{
				if (Line.StartsWith(TEXT("#META:MapName:")))
				{
					Layout.LayoutName = Line.RightChop(14);
					Layout.LayoutName.TrimStartAndEndInline();
				}
				continue;
			}

			if (!bHeaderParsed && Line.Contains(TEXT("ObjectId")))
			{
				bHeaderParsed = true;
				continue;
			}

			TArray<FString> Values;
			Line.ParseIntoArray(Values, TEXT(","), true);

			if (Values.Num() < 10) continue;

			FJumpMapLayoutEntry Entry;
			Entry.ObjectId = FName(*Values[0].TrimStartAndEnd());

			FVector Location(FCString::Atof(*Values[1]), FCString::Atof(*Values[2]), FCString::Atof(*Values[3]));
			FRotator Rotation(FCString::Atof(*Values[4]), FCString::Atof(*Values[5]), FCString::Atof(*Values[6]));
			FVector Scale(FCString::Atof(*Values[7]), FCString::Atof(*Values[8]), FCString::Atof(*Values[9]));
			Entry.Transform = FTransform(Rotation, Location, Scale);

			if (Values.Num() >= 11)
			{
				FString PropertiesStr = Values[10].TrimStartAndEnd();
				if (!PropertiesStr.IsEmpty())
				{
					TArray<FString> PropertyPairs;
					PropertiesStr.ParseIntoArray(PropertyPairs, TEXT("|"), true);
					for (const FString& Pair : PropertyPairs)
					{
						FString Key, Value;
						if (Pair.Split(TEXT("="), &Key, &Value))
						{
							Entry.CustomProperties.Add(Key.TrimStartAndEnd(), Value.TrimStartAndEnd());
						}
					}
				}
			}

			Layout.Objects.Add(Entry);
		}
	}

	if (Layout.Objects.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("JumpMapLevelEditor: No objects parsed from layout '%s'"), *LayoutName);
		return false;
	}

	// 기존 액터 제거
	ClearAllJumpMapActors();

	// 에디터 트랜잭션으로 Undo 지원
	FScopedTransaction Transaction(FText::FromString(TEXT("Load JumpMap Layout")));

	int32 SpawnedCount = 0;
	for (const FJumpMapLayoutEntry& Entry : Layout.Objects)
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
		SpawnedCount, Layout.Objects.Num(), *LayoutName);
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
		ActorsToDestroy.Add(*It);
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

	FString UserLayoutDir = GetUserLayoutDirectory();
	IFileManager& FileManager = IFileManager::Get();

	if (!FileManager.DirectoryExists(*UserLayoutDir))
	{
		return LayoutNames;
	}

	TArray<FString> FoundFiles;
	FileManager.FindFiles(FoundFiles, *UserLayoutDir, TEXT("*.csv"));

	for (const FString& FileName : FoundFiles)
	{
		LayoutNames.Add(FPaths::GetBaseFilename(FileName));
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

FString UJumpMapLevelEditorSubsystem::GetUserLayoutDirectory()
{
	return FPaths::ProjectContentDir() / TEXT("JumpMapLayouts") / TEXT("User");
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

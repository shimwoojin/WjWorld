// Fill out your copyright notice in the Description page of Project Settings.

#include "GamePlay/Placement/WjWorldPlacementComponent.h"
#include "GamePlay/Placement/WjWorldPlacementPreviewActor.h"
#include "GamePlay/Placement/WjWorldPlacedObjectActor.h"
#include "GamePlay/Placement/IWjWorldPlacementDataProvider.h"
#include "Core/Local/Lobby/WjWorldGameStateLobby.h"
#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"
#include "Save/WjWorldLayoutSaveGame.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "WjWorldLogCategories.h"

UWjWorldPlacementComponent::UWjWorldPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UWjWorldPlacementComponent::EnterPlacementMode()
{
	// 기본 컨텍스트: Lobby
	EnterPlacementModeWithContext(EPlacementContext::Lobby);
}

void UWjWorldPlacementComponent::EnterPlacementModeWithContext(EPlacementContext Context)
{
	if (CurrentMode != EPlacementMode::None)
	{
		return;
	}

	CurrentContext = Context;

	// DeveloperSettings에서 입력 설정 및 Catalog 로드 (컨텍스트별)
	LoadInputSettings();
	EnsureCatalogLoaded();

	CurrentMode = EPlacementMode::Placing;
	SetComponentTickEnabled(true);
	BindInputActions();

	OnPlacementModeChanged.Broadcast(CurrentMode);
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Entered placement mode (Context: %s)"),
		*GetPlacementContextName(CurrentContext));
}

void UWjWorldPlacementComponent::ExitPlacementMode()
{
	if (CurrentMode == EPlacementMode::None)
	{
		return;
	}

	// 호버 하이라이트 해제
	if (HoveredObject)
	{
		HoveredObject->SetHighlighted(false);
		HoveredObject = nullptr;
	}

	DestroyPreview();
	UnbindInputActions();
	SetComponentTickEnabled(false);

	SelectedObjectId = NAME_None;
	CurrentMode = EPlacementMode::None;
	// 컨텍스트는 리셋하지 않음 (다음 EnterPlacementMode 시 설정)

	OnPlacementModeChanged.Broadcast(CurrentMode);
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Exited placement mode"));
}

void UWjWorldPlacementComponent::ToggleDeleteMode()
{
	if (CurrentMode == EPlacementMode::None)
	{
		return;
	}

	if (CurrentMode == EPlacementMode::Deleting)
	{
		// 삭제 모드 해제 → 배치 모드 복귀
		if (HoveredObject)
		{
			HoveredObject->SetHighlighted(false);
			HoveredObject = nullptr;
		}
		CurrentMode = EPlacementMode::Placing;
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Switched to placing mode"));
	}
	else
	{
		// 배치 모드 → 삭제 모드
		DestroyPreview();
		CurrentMode = EPlacementMode::Deleting;
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Switched to delete mode"));
	}

	OnPlacementModeChanged.Broadcast(CurrentMode);
}

void UWjWorldPlacementComponent::SelectObject(FName ObjectId)
{
	if (!Catalog)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementComponent: No catalog set"));
		return;
	}

	const FPlaceableObjectDefinition* Def = Catalog->FindByObjectId(ObjectId);
	if (!Def)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementComponent: ObjectId=%s not found in catalog"), *ObjectId.ToString());
		return;
	}

	// 삭제 모드에서 선택하면 배치 모드로 전환
	if (CurrentMode == EPlacementMode::Deleting)
	{
		if (HoveredObject)
		{
			HoveredObject->SetHighlighted(false);
			HoveredObject = nullptr;
		}
		CurrentMode = EPlacementMode::Placing;
		OnPlacementModeChanged.Broadcast(CurrentMode);
	}

	// 기존 프리뷰 제거 후 새로 생성
	DestroyPreview();
	SelectedObjectId = ObjectId;

	UWorld* World = GetWorld();
	if (World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PreviewActor = World->SpawnActor<AWjWorldPlacementPreviewActor>(AWjWorldPlacementPreviewActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (PreviewActor)
		{
			PreviewActor->InitializePreview(*Def);
			UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Preview created for ObjectId=%s"), *ObjectId.ToString());
		}
	}
}

void UWjWorldPlacementComponent::RotatePreview()
{
	if (PreviewActor && CurrentMode == EPlacementMode::Placing)
	{
		const FPlaceableObjectDefinition* Def = Catalog ? Catalog->FindByObjectId(SelectedObjectId) : nullptr;
		float SnapDegrees = Def ? Def->RotationSnapDegrees : 90.0f;
		PreviewActor->RotatePreview(SnapDegrees);
	}
}

void UWjWorldPlacementComponent::ConfirmPlacement()
{
	if (CurrentMode != EPlacementMode::Placing || !PreviewActor || !bCurrentPreviewValid)
	{
		return;
	}

	if (!Catalog)
	{
		return;
	}

	const FPlaceableObjectDefinition* Def = Catalog->FindByObjectId(SelectedObjectId);
	if (!Def)
	{
		return;
	}

	IWjWorldPlacementDataProvider* DataProvider = GetPlacementDataProvider();
	if (!DataProvider)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementComponent: PlacementDataProvider not found"));
		return;
	}

	// 배치 데이터 생성
	FVector SpawnLocation = PreviewActor->GetActorLocation();
	FRotator SpawnRotation = FRotator(0.0f, PreviewActor->GetCurrentYaw(), 0.0f);

	FPlacedObjectSaveEntry Entry;
	Entry.ObjectId = SelectedObjectId;
	Entry.Transform = FTransform(SpawnRotation, SpawnLocation, Def->DefaultScale);

	// DataProvider에 추가 (서버에서 스폰 + 클라이언트 리플리케이션)
	DataProvider->AddPlacedObject(Entry);

	// 로컬 SaveGame에도 저장 (호스트 영구 저장용)
	SaveLayout();

	OnObjectPlaced.Broadcast();
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Object placed - ObjectId=%s at %s (Context: %s)"),
		*SelectedObjectId.ToString(), *SpawnLocation.ToString(), *GetPlacementContextName(CurrentContext));
}

void UWjWorldPlacementComponent::DeleteHoveredObject()
{
	if (CurrentMode != EPlacementMode::Deleting || !HoveredObject)
	{
		return;
	}

	IWjWorldPlacementDataProvider* DataProvider = GetPlacementDataProvider();
	if (!DataProvider)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementComponent: PlacementDataProvider not found"));
		return;
	}

	// DataProvider에서 해당 오브젝트 인덱스 찾기
	int32 Index = FindPlacedObjectIndex(HoveredObject);
	if (Index == INDEX_NONE)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementComponent: Could not find object index for deletion"));
		return;
	}

	HoveredObject = nullptr;

	// DataProvider에서 제거 (서버에서 파괴 + 클라이언트 리플리케이션)
	DataProvider->RemovePlacedObjectAt(Index);

	// 로컬 SaveGame에도 저장
	SaveLayout();

	OnObjectDeleted.Broadcast();
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Object deleted at index %d"), Index);
}

void UWjWorldPlacementComponent::SetCatalog(UWjWorldPlaceableObjectDataAsset* InCatalog)
{
	Catalog = InCatalog;
}

void UWjWorldPlacementComponent::SaveLayout()
{
	// 호스트(ListenServer 또는 Standalone)인 경우에만 SaveGame 저장
	// 클라이언트가 저장하면 다른 세션의 배치물이 섞이는 문제 발생
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ENetMode NetMode = World->GetNetMode();
	if (NetMode != NM_Standalone && NetMode != NM_ListenServer)
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: SaveLayout skipped (not host)"));
		return;
	}

	// DataProvider에서 데이터를 가져와 로컬 SaveGame에 저장 (호스트 영구 저장용)
	IWjWorldPlacementDataProvider* DataProvider = GetPlacementDataProvider();
	if (!DataProvider)
	{
		return;
	}

	UWjWorldLayoutSaveGame* SaveGame = Cast<UWjWorldLayoutSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UWjWorldLayoutSaveGame::StaticClass())
	);

	if (!SaveGame)
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("PlacementComponent: Failed to create save game object"));
		return;
	}

	// DataProvider의 배치 데이터 복사
	SaveGame->PlacedObjects = DataProvider->GetPlacedObjects();

	// 컨텍스트별 SaveSlot 이름 사용
	FString SlotName = GetCurrentSaveSlotName();
	if (UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0))
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Layout saved to '%s' (%d objects)"),
			*SlotName, SaveGame->PlacedObjects.Num());
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("PlacementComponent: Failed to save layout to '%s'"), *SlotName);
	}
}

void UWjWorldPlacementComponent::LoadLayout()
{
	// 멀티플레이 모드에서는 GameModeLobby가 GameStateLobby로 로드함
	// 이 함수는 더 이상 직접 호출되지 않음 (하위 호환성 유지용)
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: LoadLayout called - handled by GameModeLobby in multiplayer"));
}

void UWjWorldPlacementComponent::SaveLayoutToSlot(const FString& SlotName)
{
	// 호스트(ListenServer 또는 Standalone)인 경우에만 SaveGame 저장
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ENetMode NetMode = World->GetNetMode();
	if (NetMode != NM_Standalone && NetMode != NM_ListenServer)
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: SaveLayoutToSlot skipped (not host)"));
		return;
	}

	IWjWorldPlacementDataProvider* DataProvider = GetPlacementDataProvider();
	if (!DataProvider)
	{
		return;
	}

	UWjWorldLayoutSaveGame* SaveGame = Cast<UWjWorldLayoutSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UWjWorldLayoutSaveGame::StaticClass())
	);

	if (!SaveGame)
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("PlacementComponent: Failed to create save game object"));
		return;
	}

	// DataProvider의 배치 데이터 복사
	SaveGame->PlacedObjects = DataProvider->GetPlacedObjects();
	SaveGame->SavedContext = CurrentContext;

	if (UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0))
	{
		LoadedSlotName = SlotName;
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Layout saved to '%s' (%d objects)"),
			*SlotName, SaveGame->PlacedObjects.Num());

		// AW 컨텍스트인 경우 CSV도 함께 내보내기
		if (CurrentContext == EPlacementContext::ApproachingWall)
		{
			ExportLayoutAsCSV(SlotName);
		}
		// JumpMap 컨텍스트인 경우 CSV도 함께 내보내기
		else if (CurrentContext == EPlacementContext::JumpMap)
		{
			ExportJumpMapLayoutAsCSV(SlotName);
		}
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("PlacementComponent: Failed to save layout to '%s'"), *SlotName);
	}
}

bool UWjWorldPlacementComponent::LoadLayoutFromSlot(const FString& SlotName)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// SaveGame 로드
	UWjWorldLayoutSaveGame* SaveGame = Cast<UWjWorldLayoutSaveGame>(
		UGameplayStatics::LoadGameFromSlot(SlotName, 0)
	);

	if (!SaveGame)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementComponent: Failed to load layout from '%s'"), *SlotName);
		return false;
	}

	// 컨텍스트 검증 (다른 컨텍스트의 레이아웃 로드 방지)
	if (SaveGame->SavedContext != EPlacementContext::None && SaveGame->SavedContext != CurrentContext)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementComponent: Context mismatch - slot '%s' is for '%s', current context is '%s'"),
			*SlotName, *GetPlacementContextName(SaveGame->SavedContext), *GetPlacementContextName(CurrentContext));
		return false;
	}

	// DataProvider가 있으면 데이터 설정
	IWjWorldPlacementDataProvider* DataProvider = GetPlacementDataProvider();
	if (DataProvider)
	{
		// 일괄 설정 (RespawnAllPlacedObjects가 한 번만 호출됨)
		DataProvider->SetPlacedObjects(SaveGame->PlacedObjects);
	}

	LoadedSlotName = SlotName;
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Layout loaded from '%s' (%d objects)"),
		*SlotName, SaveGame->PlacedObjects.Num());
	return true;
}

TArray<FString> UWjWorldPlacementComponent::GetSavedLayoutSlots() const
{
	TArray<FString> SlotNames;

	// SaveGame 디렉토리에서 모든 .sav 파일 검색
	FString SaveDirectory = FPaths::ProjectSavedDir() / TEXT("SaveGames");

	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> FoundFiles;
	FileManager.FindFiles(FoundFiles, *SaveDirectory, TEXT("*.sav"));

	for (const FString& FileName : FoundFiles)
	{
		// .sav 확장자 제거하여 슬롯 이름 추출
		FString SlotName = FPaths::GetBaseFilename(FileName);

		// SaveGame을 로드하여 컨텍스트 확인
		UWjWorldLayoutSaveGame* SaveGame = Cast<UWjWorldLayoutSaveGame>(
			UGameplayStatics::LoadGameFromSlot(SlotName, 0)
		);

		if (SaveGame && SaveGame->SavedContext == CurrentContext)
		{
			SlotNames.Add(SlotName);
		}
	}

	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Found %d saved layout slots for context '%s'"),
		SlotNames.Num(), *GetPlacementContextName(CurrentContext));
	return SlotNames;
}

bool UWjWorldPlacementComponent::DeleteLayoutSlot(const FString& SlotName)
{
	// SaveGame 삭제
	if (!UGameplayStatics::DeleteGameInSlot(SlotName, 0))
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementComponent: Failed to delete save slot '%s'"), *SlotName);
		return false;
	}

	// AW 컨텍스트: CSV 파일도 삭제
	if (CurrentContext == EPlacementContext::ApproachingWall)
	{
		FString CSVPath = GetUserWallLayoutDirectory() / SlotName + TEXT(".csv");
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (PlatformFile.FileExists(*CSVPath))
		{
			PlatformFile.DeleteFile(*CSVPath);
			UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Deleted CSV '%s'"), *CSVPath);
		}
	}
	// JumpMap 컨텍스트: CSV 파일도 삭제
	else if (CurrentContext == EPlacementContext::JumpMap)
	{
		FString CSVPath = GetUserJumpMapLayoutDirectory() / SlotName + TEXT(".csv");
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (PlatformFile.FileExists(*CSVPath))
		{
			PlatformFile.DeleteFile(*CSVPath);
			UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Deleted CSV '%s'"), *CSVPath);
		}
	}

	// 현재 로드된 슬롯이면 초기화
	if (LoadedSlotName == SlotName)
	{
		LoadedSlotName.Empty();
	}

	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Deleted layout slot '%s'"), *SlotName);
	return true;
}

bool UWjWorldPlacementComponent::ExportLayoutAsCSV(const FString& FileName)
{
	// AW 컨텍스트에서만 동작
	if (CurrentContext != EPlacementContext::ApproachingWall)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("ExportLayoutAsCSV: Only available in ApproachingWall context"));
		return false;
	}

	IWjWorldPlacementDataProvider* DataProvider = GetPlacementDataProvider();
	if (!DataProvider)
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("ExportLayoutAsCSV: No data provider"));
		return false;
	}

	const TArray<FPlacedObjectSaveEntry>& PlacedObjects = DataProvider->GetPlacedObjects();
	if (PlacedObjects.Num() == 0)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("ExportLayoutAsCSV: No objects to export"));
		return false;
	}

	// 그리드 설정 (기본값 사용, 추후 설정 가능하게 확장 가능)
	const FApproachingWallGridConfig GridConfig;
	const FVector& BrickSize = GridConfig.BrickSize;

	// 1. 배치된 오브젝트의 바운딩 박스 계산
	FVector MinBounds(FLT_MAX, FLT_MAX, FLT_MAX);
	FVector MaxBounds(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (const FPlacedObjectSaveEntry& Entry : PlacedObjects)
	{
		const FVector& Location = Entry.Transform.GetLocation();
		MinBounds.X = FMath::Min(MinBounds.X, Location.X);
		MinBounds.Y = FMath::Min(MinBounds.Y, Location.Y);
		MaxBounds.X = FMath::Max(MaxBounds.X, Location.X);
		MaxBounds.Y = FMath::Max(MaxBounds.Y, Location.Y);
	}

	// 2. 그리드 크기 계산 (+1 = 위치 개수, +2 = 양쪽 1셀 패딩)
	int32 GridColumns = FMath::CeilToInt((MaxBounds.X - MinBounds.X) / BrickSize.X) + 1 + 2;
	int32 GridRows = FMath::CeilToInt((MaxBounds.Y - MinBounds.Y) / BrickSize.Y) + 1 + 2;

	// 최소 크기 보장
	GridColumns = FMath::Max(GridColumns, 3);
	GridRows = FMath::Max(GridRows, 3);

	// 그리드 원점: MinBounds에서 1칸 앞 (패딩용, 오브젝트가 인덱스 1부터 매핑)
	FVector GridOrigin = MinBounds - FVector(BrickSize.X, BrickSize.Y, 0.0f);

	// 3. 그리드 초기화 (-1 = Empty)
	TArray<TArray<int32>> Grid;
	Grid.SetNum(GridRows);
	for (int32 Row = 0; Row < GridRows; ++Row)
	{
		Grid[Row].SetNum(GridColumns);
		for (int32 Col = 0; Col < GridColumns; ++Col)
		{
			Grid[Row][Col] = -1;
		}
	}

	// 4. 오브젝트를 그리드에 매핑 (RoundToInt로 CalculateBrickGridIndex와 일치)
	for (const FPlacedObjectSaveEntry& Entry : PlacedObjects)
	{
		const FVector& Location = Entry.Transform.GetLocation();

		// 월드 좌표 → 그리드 인덱스
		int32 Col = FMath::RoundToInt((Location.X - GridOrigin.X) / BrickSize.X);
		int32 Row = FMath::RoundToInt((Location.Y - GridOrigin.Y) / BrickSize.Y);

		// 범위 체크
		if (Row >= 0 && Row < GridRows && Col >= 0 && Col < GridColumns)
		{
			int32 BrickTypeValue = ObjectIdToBrickTypeValue(Entry.ObjectId);
			Grid[Row][Col] = BrickTypeValue;
		}
	}

	// 5. CSV 문자열 생성 (첫 줄에 메타데이터 포함)
	FString CSVContent;

	// ⭐ 메타데이터 헤더: CenterOffset 저장 (클라이언트 프리뷰 위치 동기화용)
	// CalculateBrickPosition은 CenterOffset을 그리드 중심으로 사용하므로, 그리드 중심 좌표를 저장
	FVector CenterOffset;
	CenterOffset.X = GridOrigin.X + (GridColumns - 1) * 0.5f * BrickSize.X;
	CenterOffset.Y = GridOrigin.Y + (GridRows - 1) * 0.5f * BrickSize.Y;
	CenterOffset.Z = 0.0f;
	CSVContent += FString::Printf(TEXT("#META:CenterOffset:%f,%f,%f\n"),
		CenterOffset.X, CenterOffset.Y, CenterOffset.Z);

	for (int32 Row = 0; Row < GridRows; ++Row)
	{
		for (int32 Col = 0; Col < GridColumns; ++Col)
		{
			CSVContent += FString::Printf(TEXT("%d"), Grid[Row][Col]);
			if (Col < GridColumns - 1)
			{
				CSVContent += TEXT(",");
			}
		}
		CSVContent += TEXT("\n");
	}

	// 6. 파일 저장
	FString UserLayoutDir = GetUserWallLayoutDirectory();

	// 디렉토리 생성
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*UserLayoutDir))
	{
		PlatformFile.CreateDirectoryTree(*UserLayoutDir);
	}

	FString FilePath = UserLayoutDir / FileName + TEXT(".csv");

	if (FFileHelper::SaveStringToFile(CSVContent, *FilePath))
	{
		LastExportedCSVPath = FilePath;
		UE_LOG(LogWjWorldPlacement, Log, TEXT("ExportLayoutAsCSV: Exported to '%s' (Grid: %dx%d)"),
			*FilePath, GridColumns, GridRows);
		return true;
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("ExportLayoutAsCSV: Failed to save file '%s'"), *FilePath);
		return false;
	}
}

bool UWjWorldPlacementComponent::ExportJumpMapLayoutAsCSV(const FString& FileName)
{
	if (CurrentContext != EPlacementContext::JumpMap)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("ExportJumpMapLayoutAsCSV: Only available in JumpMap context"));
		return false;
	}

	IWjWorldPlacementDataProvider* DataProvider = GetPlacementDataProvider();
	if (!DataProvider)
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("ExportJumpMapLayoutAsCSV: No data provider"));
		return false;
	}

	const TArray<FPlacedObjectSaveEntry>& PlacedObjects = DataProvider->GetPlacedObjects();
	if (PlacedObjects.Num() == 0)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("ExportJumpMapLayoutAsCSV: No objects to export"));
		return false;
	}

	// CSV 생성: 헤더 + 오브젝트 행
	FString CSVContent;
	CSVContent += FString::Printf(TEXT("#META:MapName:%s\n"), *FileName);
	CSVContent += TEXT("ObjectId,PosX,PosY,PosZ,RotPitch,RotYaw,RotRoll,ScaleX,ScaleY,ScaleZ\n");

	for (const FPlacedObjectSaveEntry& Entry : PlacedObjects)
	{
		const FVector& Loc = Entry.Transform.GetLocation();
		const FRotator Rot = Entry.Transform.GetRotation().Rotator();
		const FVector& Scale = Entry.Transform.GetScale3D();

		CSVContent += FString::Printf(TEXT("%s,%f,%f,%f,%f,%f,%f,%f,%f,%f\n"),
			*Entry.ObjectId.ToString(),
			Loc.X, Loc.Y, Loc.Z,
			Rot.Pitch, Rot.Yaw, Rot.Roll,
			Scale.X, Scale.Y, Scale.Z);
	}

	// 파일 저장
	FString UserLayoutDir = GetUserJumpMapLayoutDirectory();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*UserLayoutDir))
	{
		PlatformFile.CreateDirectoryTree(*UserLayoutDir);
	}

	FString FilePath = UserLayoutDir / FileName + TEXT(".csv");
	if (FFileHelper::SaveStringToFile(CSVContent, *FilePath))
	{
		LastExportedCSVPath = FilePath;
		UE_LOG(LogWjWorldPlacement, Log, TEXT("ExportJumpMapLayoutAsCSV: Exported to '%s' (%d objects)"),
			*FilePath, PlacedObjects.Num());
		return true;
	}

	UE_LOG(LogWjWorldPlacement, Error, TEXT("ExportJumpMapLayoutAsCSV: Failed to save '%s'"), *FilePath);
	return false;
}

void UWjWorldPlacementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentMode == EPlacementMode::Placing && PreviewActor)
	{
		// 프리뷰 위치 갱신
		FHitResult HitResult;
		if (TraceGroundUnderMouse(HitResult))
		{
			const FPlaceableObjectDefinition* Def = Catalog ? Catalog->FindByObjectId(SelectedObjectId) : nullptr;
			float GroundOffset = Def ? Def->GroundOffset : 0.0f;

			FVector PlaceLocation = HitResult.ImpactPoint + FVector(0.0f, 0.0f, GroundOffset);

			// AW 컨텍스트: 그리드 스냅 적용
			if (CurrentContext == EPlacementContext::ApproachingWall)
			{
				PlaceLocation = SnapToGrid(PlaceLocation);
			}

			FRotator PlaceRotation = FRotator(0.0f, PreviewActor->GetCurrentYaw(), 0.0f);
			PreviewActor->UpdatePreviewTransform(PlaceLocation, PlaceRotation);

			// 유효성 검사
			bCurrentPreviewValid = IsPlacementLocationValid(PlaceLocation);
			PreviewActor->SetPreviewValid(bCurrentPreviewValid);
		}
		else
		{
			bCurrentPreviewValid = false;
			PreviewActor->SetPreviewValid(false);
		}
	}
	else if (CurrentMode == EPlacementMode::Deleting)
	{
		// 삭제 모드: 호버 대상 갱신
		AWjWorldPlacedObjectActor* NewHovered = TraceForPlacedObject();

		if (NewHovered != HoveredObject)
		{
			if (HoveredObject)
			{
				HoveredObject->SetHighlighted(false);
			}
			HoveredObject = NewHovered;
			if (HoveredObject)
			{
				HoveredObject->SetHighlighted(true);
			}
		}
	}
}

bool UWjWorldPlacementComponent::TraceGroundUnderMouse(FHitResult& OutHit) const
{
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return false;
	}

	FVector WorldLocation, WorldDirection;
	if (!PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		return false;
	}

	FVector TraceEnd = WorldLocation + WorldDirection * TraceDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(GetOwner());
	if (PreviewActor)
	{
		QueryParams.AddIgnoredActor(PreviewActor);
	}

	return GetWorld()->LineTraceSingleByChannel(OutHit, WorldLocation, TraceEnd, ECC_Visibility, QueryParams);
}

bool UWjWorldPlacementComponent::IsPlacementLocationValid(const FVector& Location) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// AW 컨텍스트: 그리드 스냅 사용 시 더 작은 반경으로 정확히 같은 위치만 체크
	// 일반 컨텍스트: 기존 겹침 반경 사용
	float CheckRadius = (CurrentContext == EPlacementContext::ApproachingWall)
		? GridOverlapCheckRadius
		: OverlapCheckRadius;

	// 겹침 체크: 기존 배치된 오브젝트와 겹치는지
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	if (PreviewActor)
	{
		QueryParams.AddIgnoredActor(PreviewActor);
	}

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(CheckRadius);
	World->OverlapMultiByChannel(Overlaps, Location, FQuat::Identity, ECC_Visibility, SphereShape, QueryParams);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (Cast<AWjWorldPlacedObjectActor>(Overlap.GetActor()))
		{
			return false;
		}
	}

	return true;
}

FVector UWjWorldPlacementComponent::SnapToGrid(const FVector& Location) const
{
	// AW 그리드 설정 사용
	const FApproachingWallGridConfig GridConfig;
	const FVector& BrickSize = GridConfig.BrickSize;

	// 그리드 스냅: 각 축을 BrickSize 단위로 반올림
	FVector SnappedLocation;
	SnappedLocation.X = FMath::RoundToFloat(Location.X / BrickSize.X) * BrickSize.X;
	SnappedLocation.Y = FMath::RoundToFloat(Location.Y / BrickSize.Y) * BrickSize.Y;
	SnappedLocation.Z = Location.Z; // Z축은 스냅하지 않음 (바닥 높이 유지)

	return SnappedLocation;
}

AWjWorldPlacedObjectActor* UWjWorldPlacementComponent::TraceForPlacedObject() const
{
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return nullptr;
	}

	FVector WorldLocation, WorldDirection;
	if (!PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		return nullptr;
	}

	FVector TraceEnd = WorldLocation + WorldDirection * TraceDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ECC_Visibility, QueryParams))
	{
		return Cast<AWjWorldPlacedObjectActor>(HitResult.GetActor());
	}

	return nullptr;
}

void UWjWorldPlacementComponent::DestroyPreview()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
}

void UWjWorldPlacementComponent::LoadInputSettings()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementComponent: DeveloperSettings not found"));
		return;
	}

	// 이미 로드된 경우 스킵
	if (PlacementMappingContext)
	{
		return;
	}

	// MappingContext 로드
	if (!Settings->PlacementMappingContext.IsNull())
	{
		PlacementMappingContext = Settings->PlacementMappingContext.LoadSynchronous();
	}

	// InputAction 로드
	if (!Settings->PlacementConfirmAction.IsNull())
	{
		ConfirmAction = Settings->PlacementConfirmAction.LoadSynchronous();
	}
	if (!Settings->PlacementCancelAction.IsNull())
	{
		CancelAction = Settings->PlacementCancelAction.LoadSynchronous();
	}
	if (!Settings->PlacementRotateAction.IsNull())
	{
		RotateAction = Settings->PlacementRotateAction.LoadSynchronous();
	}
	if (!Settings->PlacementDeleteAction.IsNull())
	{
		DeleteAction = Settings->PlacementDeleteAction.LoadSynchronous();
	}

	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Input settings loaded from DeveloperSettings"));
}

void UWjWorldPlacementComponent::EnsureCatalogLoaded()
{
	// 이미 Catalog가 설정되어 있으면 스킵
	if (Catalog)
	{
		return;
	}

	// DeveloperSettings에서 컨텍스트별 Catalog 로드
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementComponent: DeveloperSettings not found"));
		return;
	}

	Catalog = Settings->GetPlaceableCatalogForContext(CurrentContext);
	if (Catalog)
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Catalog loaded for context '%s'"),
			*GetPlacementContextName(CurrentContext));
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("PlacementComponent: Failed to load PlaceableObjectCatalog for context '%s'"),
			*GetPlacementContextName(CurrentContext));
	}
}

void UWjWorldPlacementComponent::BindInputActions()
{
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return;
	}

	// MappingContext 추가
	if (PlacementMappingContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(PlacementMappingContext, 1);
		}
	}

	// InputAction 바인딩
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent))
	{
		if (ConfirmAction)
		{
			EIC->BindAction(ConfirmAction, ETriggerEvent::Triggered, this, &UWjWorldPlacementComponent::OnConfirmAction);
		}
		if (CancelAction)
		{
			EIC->BindAction(CancelAction, ETriggerEvent::Triggered, this, &UWjWorldPlacementComponent::OnCancelAction);
		}
		if (RotateAction)
		{
			EIC->BindAction(RotateAction, ETriggerEvent::Triggered, this, &UWjWorldPlacementComponent::OnRotateAction);
		}
		if (DeleteAction)
		{
			EIC->BindAction(DeleteAction, ETriggerEvent::Triggered, this, &UWjWorldPlacementComponent::OnDeleteAction);
		}
	}
}

void UWjWorldPlacementComponent::UnbindInputActions()
{
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return;
	}

	// MappingContext 제거
	if (PlacementMappingContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(PlacementMappingContext);
		}
	}

	// InputAction 바인딩 해제
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent))
	{
		EIC->ClearBindingsForObject(this);
	}
}

void UWjWorldPlacementComponent::OnConfirmAction(const FInputActionValue& Value)
{
	if (CurrentMode == EPlacementMode::Placing)
	{
		ConfirmPlacement();
	}
	else if (CurrentMode == EPlacementMode::Deleting)
	{
		DeleteHoveredObject();
	}
}

void UWjWorldPlacementComponent::OnCancelAction(const FInputActionValue& Value)
{
	ExitPlacementMode();
}

void UWjWorldPlacementComponent::OnRotateAction(const FInputActionValue& Value)
{
	RotatePreview();
}

void UWjWorldPlacementComponent::OnDeleteAction(const FInputActionValue& Value)
{
	if (CurrentMode == EPlacementMode::Deleting)
	{
		DeleteHoveredObject();
	}
	else if (CurrentMode == EPlacementMode::Placing)
	{
		ToggleDeleteMode();
	}
}

APlayerController* UWjWorldPlacementComponent::GetPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

IWjWorldPlacementDataProvider* UWjWorldPlacementComponent::GetPlacementDataProvider() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// GameState에서 인터페이스 구현체 검색
	AGameStateBase* GameState = World->GetGameState();
	if (!GameState)
	{
		return nullptr;
	}

	return Cast<IWjWorldPlacementDataProvider>(GameState);
}

AWjWorldGameStateLobby* UWjWorldPlacementComponent::GetLobbyGameState() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	return World->GetGameState<AWjWorldGameStateLobby>();
}

int32 UWjWorldPlacementComponent::FindPlacedObjectIndex(AWjWorldPlacedObjectActor* Actor) const
{
	if (!Actor)
	{
		return INDEX_NONE;
	}

	IWjWorldPlacementDataProvider* DataProvider = GetPlacementDataProvider();
	if (!DataProvider)
	{
		return INDEX_NONE;
	}

	const TArray<FPlacedObjectSaveEntry>& PlacedObjects = DataProvider->GetPlacedObjects();
	FName TargetId = Actor->GetObjectId();
	FVector TargetLocation = Actor->GetActorLocation();

	// ObjectId와 위치로 매칭 (동일 ObjectId가 여러 개 있을 수 있으므로 위치도 체크)
	for (int32 i = 0; i < PlacedObjects.Num(); ++i)
	{
		const FPlacedObjectSaveEntry& Entry = PlacedObjects[i];
		if (Entry.ObjectId == TargetId)
		{
			// 위치가 가까우면 매치
			if (FVector::DistSquared(Entry.Transform.GetLocation(), TargetLocation) < 100.0f)
			{
				return i;
			}
		}
	}

	return INDEX_NONE;
}

FString UWjWorldPlacementComponent::GetCurrentSaveSlotName() const
{
	return GetSaveSlotNameForContext(CurrentContext);
}

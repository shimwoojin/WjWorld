// Fill out your copyright notice in the Description page of Project Settings.

#include "GamePlay/Placement/WjWorldPlacementComponent.h"
#include "GamePlay/Placement/WjWorldPlacementPreviewActor.h"
#include "GamePlay/Placement/WjWorldPlacedObjectActor.h"
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
#include "WjWorldLogCategories.h"

const FString UWjWorldPlacementComponent::SaveSlotName = TEXT("LobbyLayout");

UWjWorldPlacementComponent::UWjWorldPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UWjWorldPlacementComponent::EnterPlacementMode()
{
	if (CurrentMode != EPlacementMode::None)
	{
		return;
	}

	// DeveloperSettings에서 입력 설정 및 Catalog 로드 (폴백)
	LoadInputSettings();
	EnsureCatalogLoaded();

	CurrentMode = EPlacementMode::Placing;
	SetComponentTickEnabled(true);
	BindInputActions();

	OnPlacementModeChanged.Broadcast(CurrentMode);
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Entered placement mode"));
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

	AWjWorldGameStateLobby* LobbyGameState = GetLobbyGameState();
	if (!LobbyGameState)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementComponent: GameStateLobby not found"));
		return;
	}

	// 배치 데이터 생성
	FVector SpawnLocation = PreviewActor->GetActorLocation();
	FRotator SpawnRotation = FRotator(0.0f, PreviewActor->GetCurrentYaw(), 0.0f);

	FPlacedObjectSaveEntry Entry;
	Entry.ObjectId = SelectedObjectId;
	Entry.Transform = FTransform(SpawnRotation, SpawnLocation, Def->DefaultScale);

	// GameStateLobby에 추가 (서버에서 스폰 + 클라이언트 리플리케이션)
	LobbyGameState->AddPlacedObject(Entry);

	// 로컬 SaveGame에도 저장 (호스트 영구 저장용)
	SaveLayout();

	OnObjectPlaced.Broadcast();
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Object placed - ObjectId=%s at %s"),
		*SelectedObjectId.ToString(), *SpawnLocation.ToString());
}

void UWjWorldPlacementComponent::DeleteHoveredObject()
{
	if (CurrentMode != EPlacementMode::Deleting || !HoveredObject)
	{
		return;
	}

	AWjWorldGameStateLobby* LobbyGameState = GetLobbyGameState();
	if (!LobbyGameState)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementComponent: GameStateLobby not found"));
		return;
	}

	// GameStateLobby에서 해당 오브젝트 인덱스 찾기
	int32 Index = FindPlacedObjectIndex(HoveredObject);
	if (Index == INDEX_NONE)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementComponent: Could not find object index for deletion"));
		return;
	}

	HoveredObject = nullptr;

	// GameStateLobby에서 제거 (서버에서 파괴 + 클라이언트 리플리케이션)
	LobbyGameState->RemovePlacedObjectAt(Index);

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

	// GameStateLobby에서 데이터를 가져와 로컬 SaveGame에 저장 (호스트 영구 저장용)
	AWjWorldGameStateLobby* LobbyGameState = GetLobbyGameState();
	if (!LobbyGameState)
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

	// GameStateLobby의 배치 데이터 복사
	SaveGame->PlacedObjects = LobbyGameState->GetPlacedObjects();

	if (UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0))
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Layout saved (%d objects)"), SaveGame->PlacedObjects.Num());
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("PlacementComponent: Failed to save layout"));
	}
}

void UWjWorldPlacementComponent::LoadLayout()
{
	// 멀티플레이 모드에서는 GameModeLobby가 GameStateLobby로 로드함
	// 이 함수는 더 이상 직접 호출되지 않음 (하위 호환성 유지용)
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: LoadLayout called - handled by GameModeLobby in multiplayer"));
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

	// 겹침 체크: 기존 배치된 오브젝트와 겹치는지
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	if (PreviewActor)
	{
		QueryParams.AddIgnoredActor(PreviewActor);
	}

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(OverlapCheckRadius);
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

	// DeveloperSettings에서 Catalog 로드 (폴백 메커니즘)
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings || Settings->PlaceableObjectCatalog.IsNull())
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementComponent: No PlaceableObjectCatalog configured in DeveloperSettings"));
		return;
	}

	Catalog = Settings->PlaceableObjectCatalog.LoadSynchronous();
	if (Catalog)
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementComponent: Catalog loaded from DeveloperSettings (fallback)"));
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("PlacementComponent: Failed to load PlaceableObjectCatalog"));
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

	AWjWorldGameStateLobby* LobbyGameState = GetLobbyGameState();
	if (!LobbyGameState)
	{
		return INDEX_NONE;
	}

	const TArray<FPlacedObjectSaveEntry>& PlacedObjects = LobbyGameState->GetPlacedObjects();
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

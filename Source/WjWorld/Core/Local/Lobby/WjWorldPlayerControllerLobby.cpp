// Fill out your copyright notice in the Description page of Project Settings.

#include "WjWorldPlayerControllerLobby.h"
#include "Core/Base/WjWorldCharacterBase.h"
#include "GamePlay/Placement/WjWorldPlacementComponent.h"
#include "GamePlay/Placement/WjWorldPlacementCameraPawn.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Camera/PlayerCameraManager.h"
#include "WjWorldLogCategories.h"

AWjWorldPlayerControllerLobby::AWjWorldPlayerControllerLobby()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	PlacementComponent = CreateDefaultSubobject<UWjWorldPlacementComponent>(TEXT("PlacementComponent"));
}

void AWjWorldPlayerControllerLobby::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeLobbyController();
}

void AWjWorldPlayerControllerLobby::InitializeController()
{
	Super::InitializeController();
	
	// 로비 컨트롤러 초기화
	//SetInputMode(FInputModeUIOnly());
}

void AWjWorldPlayerControllerLobby::InitializeUI()
{
	Super::InitializeUI();
	
	CreateLobbyUI();
}

void AWjWorldPlayerControllerLobby::OpenRoomList()
{
	// 방 목록 창 열기 로직
}

void AWjWorldPlayerControllerLobby::OpenCreateRoom()
{
	// 방 생성 창 열기 로직
}

void AWjWorldPlayerControllerLobby::InitializeLobbyController()
{
	// 로비 컨트롤러 전용 초기화 로직
}

void AWjWorldPlayerControllerLobby::CreateLobbyUI()
{
	// 로비 UI 생성 로직
}

void AWjWorldPlayerControllerLobby::SwitchToPlacementCamera()
{
	if (PlacementCameraPawn)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlayerControllerLobby: Already in placement camera mode"));
		return;
	}

	OriginalPawn = GetPawn();

	// 캐릭터의 GamePlayCamera 비활성화 (Possess 전 해제해야 재 Possess 시 Activate()가 정상 동작)
	if (AWjWorldCharacterBase* CharBase = Cast<AWjWorldCharacterBase>(GetPawn()))
	{
		if (UGameplayCameraComponent* GamePlayCamera = CharBase->GetGamePlayCamera())
		{
			GamePlayCamera->Deactivate();
		}
	}

	// 현재 카메라 위치/회전을 스폰 기준으로 사용
	FVector CameraSpawnLoc = FVector::ZeroVector;
	FRotator CameraSpawnRot = FRotator::ZeroRotator;
	if (PlayerCameraManager)
	{
		CameraSpawnLoc = PlayerCameraManager->GetCameraLocation();
		CameraSpawnRot = PlayerCameraManager->GetCameraRotation();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PlacementCameraPawn = GetWorld()->SpawnActor<AWjWorldPlacementCameraPawn>(
		AWjWorldPlacementCameraPawn::StaticClass(), CameraSpawnLoc, CameraSpawnRot, SpawnParams);

	if (!PlacementCameraPawn)
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("PlayerControllerLobby: Failed to spawn PlacementCameraPawn"));
		return;
	}

	Possess(PlacementCameraPawn);
	SetControlRotation(CameraSpawnRot);

	// 마우스 커서 유지
	SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));

	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlayerControllerLobby: Switched to placement camera"));
}

void AWjWorldPlayerControllerLobby::RestoreOriginalPawn()
{
	if (!PlacementCameraPawn)
	{
		return;
	}

	if (OriginalPawn.IsValid())
	{
		Possess(OriginalPawn.Get());
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlayerControllerLobby: OriginalPawn is no longer valid"));
	}

	OriginalPawn.Reset();

	PlacementCameraPawn->Destroy();
	PlacementCameraPawn = nullptr;

	SetInputMode(FInputModeGameAndUI());

	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlayerControllerLobby: Restored original pawn"));
}

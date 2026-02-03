// Fill out your copyright notice in the Description page of Project Settings.

#include "WjWorldPlayerControllerLobby.h"
#include "Core/Base/WjWorldCharacterBase.h"
#include "GamePlay/Placement/WjWorldPlacementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"

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

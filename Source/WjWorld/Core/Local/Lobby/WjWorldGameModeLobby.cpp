// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Local/Lobby/WjWorldGameModeLobby.h"
#include "Core/Local/Lobby/WjWorldHUDLobby.h"
#include "UI/Session/CreateRoomWindow.h"
#include "UI/Session/RoomListWindow.h"
#include "Blueprint/UserWidget.h"
#include "WjWorldLogCategories.h"

AWjWorldGameModeLobby::AWjWorldGameModeLobby()
{
	PrimaryActorTick.bCanEverTick = false;

	// 기본 HUD 클래스 설정
	HUDClass = AWjWorldHUDLobby::StaticClass();

	// CreateRoomWindow Blueprint 클래스 자동 로드
	static ConstructorHelpers::FClassFinder<UCreateRoomWindow> CreateRoomWindowBPClass(
		TEXT("/Game/UI/Blueprint/Session/BP_CreateRoomWindow")
	);

	if (CreateRoomWindowBPClass.Succeeded())
	{
		CreateRoomWindowClass = CreateRoomWindowBPClass.Class;
	}

	// RoomListWindow Blueprint 클래스 자동 로드
	static ConstructorHelpers::FClassFinder<URoomListWindow> RoomListWindowBPClass(
		TEXT("/Game/UI/Blueprint/Session/BP_RoomListWindow")
	);

	if (RoomListWindowBPClass.Succeeded())
	{
		RoomListWindowClass = RoomListWindowBPClass.Class;
	}
}

void AWjWorldGameModeLobby::BeginPlay()
{
	Super::BeginPlay();

	// 마우스 커서 표시 (로비에서는 UI 조작 필요)
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeGameAndUI());
	}

	UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeLobby: BeginPlay - Lobby loaded"));
}

void AWjWorldGameModeLobby::ShowCreateRoomWindow()
{
	if (!CreateRoomWindowClass)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeLobby: CreateRoomWindowClass is not set!"));
		return;
	}

	// 이미 인스턴스가 있으면 재사용
	if (CreateRoomWindowInstance && CreateRoomWindowInstance->IsInViewport())
	{
		UE_LOG(LogWjWorld, Warning, TEXT("WjWorldGameModeLobby: CreateRoomWindow is already open"));
		return;
	}

	// 새 인스턴스 생성
	CreateRoomWindowInstance = CreateWidget<UCreateRoomWindow>(GetWorld(), CreateRoomWindowClass);
	
	if (CreateRoomWindowInstance)
	{
		CreateRoomWindowInstance->ShowPopup();
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeLobby: CreateRoomWindow opened"));
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeLobby: Failed to create CreateRoomWindow widget"));
	}
}

void AWjWorldGameModeLobby::ShowRoomListWindow()
{
	if (!RoomListWindowClass)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeLobby: RoomListWindowClass is not set!"));
		return;
	}

	// 이미 인스턴스가 있으면 재사용
	if (RoomListWindowInstance && RoomListWindowInstance->IsInViewport())
	{
		UE_LOG(LogWjWorld, Warning, TEXT("WjWorldGameModeLobby: RoomListWindow is already open"));
		return;
	}

	// 새 인스턴스 생성
	RoomListWindowInstance = CreateWidget<URoomListWindow>(GetWorld(), RoomListWindowClass);
	
	if (RoomListWindowInstance)
	{
		RoomListWindowInstance->ShowPopup();
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeLobby: RoomListWindow opened"));
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeLobby: Failed to create RoomListWindow widget"));
	}
}

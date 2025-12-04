// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Local/Lobby/WjWorldHUDLobby.h"
#include "UI/Lobby/LobbyHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "WjWorldLogCategories.h"

AWjWorldHUDLobby::AWjWorldHUDLobby()
{
	// Blueprint 클래스 자동 로드
	static ConstructorHelpers::FClassFinder<ULobbyHUDWidget> LobbyHUDWidgetBPClass(
		TEXT("/Game/UI/Blueprint/Lobby/BP_LobbyHUDWidget")
	);

	if (LobbyHUDWidgetBPClass.Succeeded())
	{
		LobbyHUDWidgetClass = LobbyHUDWidgetBPClass.Class;
	}
}

void AWjWorldHUDLobby::BeginPlay()
{
	Super::BeginPlay();

	// 로비 HUD 자동 표시
	ShowLobbyHUD();

	UE_LOG(LogWjWorld, Log, TEXT("WjWorldHUDLobby: BeginPlay - Lobby HUD initialized"));
}

void AWjWorldHUDLobby::ShowLobbyHUD()
{
	if (!LobbyHUDWidgetClass)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldHUDLobby: LobbyHUDWidgetClass is not set!"));
		return;
	}

	// 이미 인스턴스가 있으면 재사용
	if (LobbyHUDWidgetInstance && LobbyHUDWidgetInstance->IsInViewport())
	{
		UE_LOG(LogWjWorld, Warning, TEXT("WjWorldHUDLobby: Lobby HUD is already visible"));
		return;
	}

	// 새 인스턴스 생성
	LobbyHUDWidgetInstance = CreateWidget<ULobbyHUDWidget>(GetWorld(), LobbyHUDWidgetClass);

	if (LobbyHUDWidgetInstance)
	{
		LobbyHUDWidgetInstance->AddToViewport(0); // Z-Order 0 (배경)
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldHUDLobby: Lobby HUD displayed"));
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldHUDLobby: Failed to create Lobby HUD widget"));
	}
}

void AWjWorldHUDLobby::HideLobbyHUD()
{
	if (LobbyHUDWidgetInstance && LobbyHUDWidgetInstance->IsInViewport())
	{
		LobbyHUDWidgetInstance->RemoveFromParent();
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldHUDLobby: Lobby HUD hidden"));
	}
}

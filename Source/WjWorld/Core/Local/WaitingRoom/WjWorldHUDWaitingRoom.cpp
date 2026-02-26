// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Local/WaitingRoom/WjWorldHUDWaitingRoom.h"
#include "UI/WaitingRoom/WaitingRoomHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "WjWorldLogCategories.h"

AWjWorldHUDWaitingRoom::AWjWorldHUDWaitingRoom()
{
	bCreateChatWidget = true;
}

void AWjWorldHUDWaitingRoom::BeginPlay()
{
	Super::BeginPlay();

	// 대기실 HUD 자동 표시
	ShowWaitingRoomHUD();

	UE_LOG(LogWjWorld, Log, TEXT("WjWorldHUDWaitingRoom: BeginPlay - WaitingRoom HUD initialized"));
}

bool AWjWorldHUDWaitingRoom::TryCloseTopPopup()
{
	if (WaitingRoomHUDWidgetInstance)
	{
		return WaitingRoomHUDWidgetInstance->TryCloseTopPopup();
	}
	return false;
}

void AWjWorldHUDWaitingRoom::ShowWaitingRoomHUD()
{
	if (!WaitingRoomHUDWidgetClass)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldHUDWaitingRoom: WaitingRoomHUDWidgetClass is not set!"));
		return;
	}

	// 이미 인스턴스가 있으면 재사용
	if (WaitingRoomHUDWidgetInstance && WaitingRoomHUDWidgetInstance->IsInViewport())
	{
		UE_LOG(LogWjWorld, Warning, TEXT("WjWorldHUDWaitingRoom: WaitingRoom HUD is already visible"));
		return;
	}

	// 새 인스턴스 생성
	WaitingRoomHUDWidgetInstance = CreateWidget<UWaitingRoomHUDWidget>(GetWorld(), WaitingRoomHUDWidgetClass);

	if (WaitingRoomHUDWidgetInstance)
	{
		WaitingRoomHUDWidgetInstance->AddToViewport(0); // Z-Order 0 (배경)
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldHUDWaitingRoom: WaitingRoom HUD displayed"));
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldHUDWaitingRoom: Failed to create WaitingRoom HUD widget"));
	}
}

void AWjWorldHUDWaitingRoom::HideWaitingRoomHUD()
{
	if (WaitingRoomHUDWidgetInstance && WaitingRoomHUDWidgetInstance->IsInViewport())
	{
		WaitingRoomHUDWidgetInstance->RemoveFromParent();
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldHUDWaitingRoom: WaitingRoom HUD hidden"));
	}
}

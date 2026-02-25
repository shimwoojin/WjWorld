// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Base/WjWorldHUDBase.h"
#include "UI/HUD/CoinGainNotificationWidget.h"
#include "UI/Chat/ChatWidget.h"
#include "Setting/WjWorldDeveloperSettings.h"

void AWjWorldHUDBase::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->IsLocalPlayerController())
	{
		return;
	}

	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		return;
	}

	// Coin 획득 알림 위젯 생성
	if (!Settings->CoinGainNotificationWidgetClass.IsNull())
	{
		UClass* WidgetClass = Settings->CoinGainNotificationWidgetClass.LoadSynchronous();
		if (WidgetClass)
		{
			CoinGainNotificationWidget = CreateWidget<UCoinGainNotificationWidget>(PC, WidgetClass);
			if (CoinGainNotificationWidget)
			{
				CoinGainNotificationWidget->AddToViewport(100);
			}
		}
	}

	// 채팅 위젯 생성
	if (!Settings->ChatWidgetClass.IsNull())
	{
		UClass* ChatClass = Settings->ChatWidgetClass.LoadSynchronous();
		if (ChatClass)
		{
			ChatWidget = CreateWidget<UChatWidget>(PC, ChatClass);
			if (ChatWidget)
			{
				ChatWidget->AddToViewport(50);
			}
		}
	}
}

bool AWjWorldHUDBase::TryCloseTopPopup()
{
	// Base: 닫을 팝업 없음. 서브클래스에서 override하여 구현.
	return false;
}

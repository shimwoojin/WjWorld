// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Base/WjWorldHUDBase.h"
#include "UI/HUD/CoinGainNotificationWidget.h"
#include "UI/HUD/MatchRewardCounterWidget.h"
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

	// 매치 보상 카운터 위젯 생성 (Lobby/WaitingRoom에서만)
	if (bCreateMatchRewardCounter && !Settings->MatchRewardCounterWidgetClass.IsNull())
	{
		UClass* RewardCounterClass = Settings->MatchRewardCounterWidgetClass.LoadSynchronous();
		if (RewardCounterClass)
		{
			MatchRewardCounterWidget = CreateWidget<UMatchRewardCounterWidget>(PC, RewardCounterClass);
			if (MatchRewardCounterWidget)
			{
				MatchRewardCounterWidget->AddToViewport(75);
			}
		}
	}

	// 채팅 위젯 생성 (멀티플레이어 컨텍스트에서만)
	if (bCreateChatWidget && !Settings->ChatWidgetClass.IsNull())
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

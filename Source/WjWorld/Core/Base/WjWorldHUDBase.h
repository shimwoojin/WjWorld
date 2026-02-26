// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WjWorldHUDBase.generated.h"

class UCoinGainNotificationWidget;
class UChatWidget;

/**
 * HUD 베이스 클래스
 * Coin 획득 알림, 채팅 등 모든 컨텍스트 공통 UI 관리
 */
UCLASS()
class WJWORLD_API AWjWorldHUDBase : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	/** 채팅 위젯 접근 */
	UChatWidget* GetChatWidget() const { return ChatWidget; }

	/** 가장 위에 열린 팝업 닫기 (서브클래스에서 override) */
	virtual bool TryCloseTopPopup();

protected:
	/** 채팅 위젯 생성 여부 (Intro/Login 등 비멀티플레이어 컨텍스트에서 false) */
	bool bCreateChatWidget = false;

private:
	UPROPERTY()
	TObjectPtr<UCoinGainNotificationWidget> CoinGainNotificationWidget;

	UPROPERTY()
	TObjectPtr<UChatWidget> ChatWidget;
};

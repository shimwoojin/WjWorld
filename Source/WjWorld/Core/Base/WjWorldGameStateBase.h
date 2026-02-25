// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "WjWorldGameStateBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatMessageReceived, const FString&, SenderName, const FString&, Message);

/**
 * 기본 게임 상태 클래스
 * 채팅 등 모든 컨텍스트 공통 기능 관리
 */
UCLASS()
class WJWORLD_API AWjWorldGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** 채팅 메시지 수신 델리게이트 (UI에서 바인딩) */
	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatMessageReceived OnChatMessageReceived;

	/** 서버에서 호출 - 모든 클라이언트에 채팅 메시지 브로드캐스트 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastReceiveChatMessage(const FString& SenderName, const FString& Message);
};

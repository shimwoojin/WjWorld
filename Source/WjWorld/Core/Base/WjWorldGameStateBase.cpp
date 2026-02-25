// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Base/WjWorldGameStateBase.h"

void AWjWorldGameStateBase::MulticastReceiveChatMessage_Implementation(const FString& SenderName, const FString& Message)
{
	OnChatMessageReceived.Broadcast(SenderName, Message);
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/WjWorldGameInstance.h"
#include "Core/Session/SessionManager.h"
#include "WjWorldLogCategories.h"

void UWjWorldGameInstance::Init()
{
	Super::Init();

	// SessionManager 생성 및 초기화
	SessionManager = NewObject<USessionManager>(this);
	if (SessionManager)
	{
		SessionManager->Initialize();
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameInstance: SessionManager created and initialized"));
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: Failed to create SessionManager"));
	}
}

void UWjWorldGameInstance::Shutdown()
{
	// SessionManager 정리
	if (SessionManager)
	{
		SessionManager->Shutdown();
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameInstance: SessionManager shutdown"));
	}

	Super::Shutdown();
}

bool UWjWorldGameInstance::CreateRoom(const FRoomSettings& Settings)
{
	if (SessionManager)
	{
		return SessionManager->CreateSession(Settings);
	}
	
	UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: SessionManager is null"));
	return false;
}

bool UWjWorldGameInstance::FindRooms(int32 MaxResults)
{
	if (SessionManager)
	{
		return SessionManager->FindSessions(MaxResults);
	}

	UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: SessionManager is null"));
	return false;
}

bool UWjWorldGameInstance::JoinRoom(int32 RoomIndex)
{
	if (SessionManager)
	{
		return SessionManager->JoinSession(RoomIndex);
	}

	UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: SessionManager is null"));
	return false;
}

bool UWjWorldGameInstance::StartGame()
{
	if (SessionManager)
	{
		return SessionManager->StartSession();
	}

	UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: SessionManager is null"));
	return false;
}

bool UWjWorldGameInstance::LeaveRoom()
{
	if (SessionManager)
	{
		return SessionManager->DestroySession();
	}

	UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: SessionManager is null"));
	return false;
}

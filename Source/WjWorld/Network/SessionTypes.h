// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SessionTypes.generated.h"

/**
 * 방 생성 설정
 */
USTRUCT(BlueprintType)
struct FRoomSettings
{
	GENERATED_BODY()

	/** 방 이름 */
	UPROPERTY(BlueprintReadWrite, Category = "Room")
	FString RoomName = TEXT("Default Room");

	/** 최대 플레이어 수 */
	UPROPERTY(BlueprintReadWrite, Category = "Room")
	int32 MaxPlayers = 8;

	/** 비공개 방 여부 */
	UPROPERTY(BlueprintReadWrite, Category = "Room")
	bool bIsPrivate = false;

	/** 게임 모드 (예: SpeedRace, ItemRace 등) */
	UPROPERTY(BlueprintReadWrite, Category = "Room")
	FString GameMode = TEXT("Default");

	/** 맵 이름 */
	UPROPERTY(BlueprintReadWrite, Category = "Room")
	FString MapName = TEXT("DefaultMap");

	/** 게임 중 입장 허용 여부 */
	UPROPERTY(BlueprintReadWrite, Category = "Room")
	bool bAllowJoinInProgress = true;

	/** 비밀번호 (비공개 방용) */
	UPROPERTY(BlueprintReadWrite, Category = "Room")
	FString Password;
};

/**
 * 검색된 방 정보 (UI 표시용)
 */
USTRUCT(BlueprintType)
struct FRoomInfo
{
	GENERATED_BODY()

	/** 방 이름 */
	UPROPERTY(BlueprintReadOnly, Category = "Room")
	FString RoomName;

	/** 현재 플레이어 수 */
	UPROPERTY(BlueprintReadOnly, Category = "Room")
	int32 CurrentPlayers = 0;

	/** 최대 플레이어 수 */
	UPROPERTY(BlueprintReadOnly, Category = "Room")
	int32 MaxPlayers = 0;

	/** 비공개 방 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Room")
	bool bIsPrivate = false;

	/** 게임 모드 */
	UPROPERTY(BlueprintReadOnly, Category = "Room")
	FString GameMode;

	/** 핑 */
	UPROPERTY(BlueprintReadOnly, Category = "Room")
	int32 Ping = 0;

	/** 게임 진행 중 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Room")
	bool bInProgress = false;

	/** 방장 이름 */
	UPROPERTY(BlueprintReadOnly, Category = "Room")
	FString HostName;

	/** 검색 결과 인덱스 (내부 사용) */
	int32 SearchResultIndex = -1;
};

/**
 * Session 델리게이트 타입
 * 주의: 이름에 "Room"을 붙여서 OnlineSubsystem 델리게이트와 충돌 방지
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomCreated, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoomsFound, bool, bWasSuccessful, const TArray<FRoomInfo>&, Rooms);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomJoined, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomDestroyed, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomStarted, bool, bWasSuccessful);

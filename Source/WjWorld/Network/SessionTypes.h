// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SessionTypes.generated.h"

/**
 * 네트워크 모드
 */
UENUM(BlueprintType)
enum class ENetworkMode : uint8
{
	LAN		UMETA(DisplayName = "LAN"),
	Steam	UMETA(DisplayName = "Steam")
};

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

	/** 네트워크 모드 (LAN 또는 Steam) */
	UPROPERTY(BlueprintReadWrite, Category = "Room")
	ENetworkMode NetworkMode = ENetworkMode::LAN;
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomEnded, bool, bWasSuccessful);

/**
 * 플레이어 정보 구조체 (UI 표시 및 캐싱용)
 */
USTRUCT(BlueprintType)
struct FPlayerDisplayInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bIsReady = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bIsHost = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	int32 PlayerID = -1;
};

/**
 * 호스트 마이그레이션 상태
 */
UENUM(BlueprintType)
enum class EHostMigrationState : uint8
{
	None			UMETA(DisplayName = "None"),
	Detected		UMETA(DisplayName = "Detected"),
	CreatingSession	UMETA(DisplayName = "Creating Session"),
	SearchingSession	UMETA(DisplayName = "Searching Session"),
	JoiningSession	UMETA(DisplayName = "Joining Session"),
	Complete		UMETA(DisplayName = "Complete"),
	Failed			UMETA(DisplayName = "Failed")
};

/**
 * 호스트 마이그레이션 컨텍스트
 * 마이그레이션 진행 중 필요한 모든 상태를 보관
 */
USTRUCT(BlueprintType)
struct FHostMigrationContext
{
	GENERATED_BODY()

	/** 마이그레이션 진행 중 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Migration")
	bool bIsMigrating = false;

	/** 이 클라이언트가 새 호스트인지 */
	UPROPERTY(BlueprintReadOnly, Category = "Migration")
	bool bIsNewHost = false;

	/** 캐시된 방 설정 */
	UPROPERTY(BlueprintReadOnly, Category = "Migration")
	FRoomSettings CachedRoomSettings;

	/** 마이그레이션 세션 식별 태그 (모든 클라이언트 동일 생성) */
	UPROPERTY(BlueprintReadOnly, Category = "Migration")
	FString MigrationSessionTag;

	/** 캐시된 플레이어 목록 */
	UPROPERTY(BlueprintReadOnly, Category = "Migration")
	TArray<FPlayerDisplayInfo> CachedPlayerList;

	/** 로컬 플레이어 ID */
	UPROPERTY(BlueprintReadOnly, Category = "Migration")
	int32 LocalPlayerID = -1;

	/** 마이그레이션 시작 시각 */
	double MigrationStartTime = 0.0;

	/** 재시도 횟수 */
	int32 RetryCount = 0;

	/** 최대 재시도 횟수 */
	static constexpr int32 MaxRetryCount = 10;

	/** 재시도 간격 (초) */
	static constexpr float RetryInterval = 2.0f;

	/** 마이그레이션 타임아웃 (초) */
	static constexpr float MigrationTimeout = 30.0f;

	/** 상태 초기화 */
	void Reset()
	{
		bIsMigrating = false;
		bIsNewHost = false;
		CachedRoomSettings = FRoomSettings();
		MigrationSessionTag.Empty();
		CachedPlayerList.Empty();
		LocalPlayerID = -1;
		MigrationStartTime = 0.0;
		RetryCount = 0;
	}
};

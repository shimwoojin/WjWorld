// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Network/SessionTypes.h"
#include "SessionManager.generated.h"

/**
 * Online Subsystem Session 관리 클래스
 * 
 * 역할:
 * - 세션(방) 생성, 검색, 참가, 종료
 * - Online Subsystem과의 통신 담당
 * - UI와 네트워크 로직 분리
 */
UCLASS()
class WJWORLD_API USessionManager : public UObject
{
	GENERATED_BODY()
	
public:
	USessionManager();

	/** 초기화 (GameInstance에서 호출) */
	void Initialize();

	/** 정리 */
	void Shutdown();

	//~ Session 생성
	/**
	 * 세션(방) 생성
	 * @param Settings 방 설정
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool CreateSession(const FRoomSettings& Settings);

	//~ Session 검색
	/**
	 * 세션(방) 검색
	 * @param NetworkMode 네트워크 모드 (LAN 또는 Steam)
	 * @param MaxSearchResults 최대 검색 결과 수
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool FindSessions(ENetworkMode NetworkMode = ENetworkMode::LAN, int32 MaxSearchResults = 100);

	//~ Session 참가
	/**
	 * 세션(방) 참가
	 * @param RoomIndex 검색 결과에서의 인덱스
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool JoinSession(int32 RoomIndex);

	//~ Session 시작
	/**
	 * 세션 시작 (방장만 가능)
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool StartSession();

	//~ Session 종료 (InProgress → Ended)
	/**
	 * 세션 종료 (InProgress 상태를 Ended로 변경)
	 * Play → WaitingRoom 복귀 시 호출하여 다시 StartSession 가능하게 함
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool EndSession();

	//~ Session 파괴
	/**
	 * 세션 나가기/방 파괴
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool DestroySession();

	//~ 유틸리티
	/**
	 * 현재 세션에 있는지 확인
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	bool IsInSession() const { return bIsInSession; }

	/**
	 * 호스트인지 확인
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	bool IsHost() const { return bIsHost; }

	/**
	 * 마지막 방 설정 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	const FRoomSettings& GetLastRoomSettings() const { return LastRoomSettings; }

	/**
	 * 마지막 방 설정 업데이트 (대기실에서 호스트가 설정 변경 시)
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void UpdateLastRoomSettings(const FRoomSettings& NewSettings) { LastRoomSettings = NewSettings; }

public:
	//~ 델리게이트 (UI에서 바인딩)
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnRoomCreated OnRoomCreatedEvent;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnRoomsFound OnRoomsFoundEvent;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnRoomJoined OnRoomJoinedEvent;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnRoomDestroyed OnRoomDestroyedEvent;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnRoomStarted OnRoomStartedEvent;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnRoomEnded OnRoomEndedEvent;

	//~ 호스트 마이그레이션 지원
	/**
	 * 마이그레이션 세션 생성 (MIGRATION_TAG 커스텀 세팅 포함)
	 * @param Settings 방 설정
	 * @param MigrationTag 마이그레이션 식별 태그
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool CreateMigrationSession(const FRoomSettings& Settings, const FString& MigrationTag);

	/**
	 * 마이그레이션 세션 검색 (MIGRATION_TAG로 필터링)
	 * @param MigrationTag 검색할 마이그레이션 태그
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool FindMigrationSession(const FString& MigrationTag);

	/**
	 * 네트워크 모드에 따라 GameNetDriver를 전환
	 * LAN → IpNetDriver, Steam → SteamNetDriver
	 * 세션 생성/참가 전에 호출하여 Listen Server 및 ClientTravel이 올바른 드라이버 사용
	 */
	static void ApplyNetDriverForMode(ENetworkMode Mode);

private:
	//~ Online Subsystem 콜백
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);

	//~ 헬퍼 함수
	/** SearchResult를 FRoomInfo로 변환 */
	FRoomInfo ConvertSearchResultToRoomInfo(const FOnlineSessionSearchResult& SearchResult, int32 Index);

private:
	/** Online Session Interface */
	IOnlineSessionPtr SessionInterface;

	/** 검색 설정 및 결과 */
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	/** 현재 세션 이름 */
	static constexpr const TCHAR* SESSION_NAME = TEXT("WjWorldSession");

	/** 상태 플래그 */
	bool bIsInSession = false;
	bool bIsHost = false;
	bool bIsSearchInProgress = false;

	/** 마지막 방 설정 (재생성용) */
	FRoomSettings LastRoomSettings;

	/** 마지막으로 참가한 방 인덱스 */
	int32 LastJoinedRoomIndex = -1;

	/** 마지막 검색 네트워크 모드 (JoinSession에서 NetDriver 전환용) */
	ENetworkMode LastSearchNetworkMode = ENetworkMode::LAN;

	/** 마이그레이션 검색 시 사용할 태그 */
	FString PendingMigrationTag;

	/** 대기 중인 검색 요청 (이전 검색 취소 후 실행) */
	TOptional<TPair<ENetworkMode, int32>> PendingSearchRequest;
};

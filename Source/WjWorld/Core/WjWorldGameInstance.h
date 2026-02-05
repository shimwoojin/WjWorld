// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/NetDriver.h"
#include "Network/SessionTypes.h"
#include "WjWorldGameInstance.generated.h"

class USessionManager;

/**
 * WjWorld 게임 인스턴스
 *
 * 역할:
 * - 게임 전역 데이터 관리
 * - SessionManager 소유 및 접근 제공
 * - 레벨 간 데이터 유지
 * - 호스트 마이그레이션 관리
 */
UCLASS()
class WJWORLD_API UWjWorldGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	//~ Session Manager 접근
	UFUNCTION(BlueprintPure, Category = "Session", meta = (DisplayName = "Get Session Manager"))
	USessionManager* GetSessionManager() const { return SessionManager; }

	//~ 편의 함수 (UI에서 직접 호출 가능)
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool CreateRoom(const FRoomSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Session")
	bool FindRooms(ENetworkMode NetworkMode = ENetworkMode::LAN, int32 MaxResults = 100);

	UFUNCTION(BlueprintCallable, Category = "Session")
	bool JoinRoom(int32 RoomIndex);

	UFUNCTION(BlueprintCallable, Category = "Session")
	bool StartGame();

	UFUNCTION(BlueprintCallable, Category = "Session")
	bool LeaveRoom();

	//~ 게임 종료 (세션 InProgress → Ended)
	/**
	 * 게임 종료 래퍼. Play → WaitingRoom 복귀 전에 호출.
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool EndGame();

	//~ 캐싱 (호스트 마이그레이션용)
	/** 플레이어 목록 캐시 */
	void CachePlayerList(const TArray<FPlayerDisplayInfo>& PlayerList);

	/** 방 설정 캐시 */
	void CacheRoomSettings(const FRoomSettings& Settings);

	//~ 호스트 마이그레이션
	/** 마이그레이션 진행 중 여부 */
	UFUNCTION(BlueprintPure, Category = "Migration")
	bool IsMigrating() const { return MigrationContext.bIsMigrating; }

	/** 현재 마이그레이션 상태 */
	UFUNCTION(BlueprintPure, Category = "Migration")
	EHostMigrationState GetMigrationState() const { return MigrationState; }

	/** 마이그레이션 컨텍스트 접근 */
	FHostMigrationContext& GetMigrationContext() { return MigrationContext; }
	const FHostMigrationContext& GetMigrationContext() const { return MigrationContext; }

private:
	//~ NetworkFailure 핸들러
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver,
		ENetworkFailure::Type FailureType, const FString& ErrorString);

	//~ 호스트 마이그레이션 내부 함수
	/** 마이그레이션 시작 */
	void BeginHostMigration();

	/** 새 호스트 결정 (결정론적) */
	bool DetermineIfNewHost() const;

	/** 마이그레이션 태그 생성 (결정론적) */
	FString GenerateMigrationTag() const;

	/** 새 호스트: 마이그레이션 세션 생성 */
	void MigrationCreateSession();

	/** 비호스트: 마이그레이션 세션 검색 */
	void MigrationSearchForNewHost();

	/** 마이그레이션 세션 생성 완료 콜백 */
	UFUNCTION()
	void OnMigrationSessionCreated(bool bWasSuccessful);

	/** 마이그레이션 세션 검색 실패 시 재시도 또는 실패 */
	UFUNCTION()
	void OnMigrationSearchResult(bool bWasSuccessful, const TArray<FRoomInfo>& Rooms);

	/** 재시도 또는 실패 처리 */
	void MigrationRetryOrFail();

	/** 마이그레이션 타임아웃 */
	void OnMigrationTimeout();

	/** 마이그레이션 실패 → 로비로 복귀 */
	void MigrationFailed();

	/** 마이그레이션 상태 전이 */
	void SetMigrationState(EHostMigrationState NewState);

private:
	/** Session 관리자 */
	UPROPERTY()
	TObjectPtr<USessionManager> SessionManager;

	/** NetworkFailure 델리게이트 핸들 */
	FDelegateHandle NetworkFailureHandle;

	/** 호스트 마이그레이션 컨텍스트 */
	FHostMigrationContext MigrationContext;

	/** 호스트 마이그레이션 상태 */
	EHostMigrationState MigrationState = EHostMigrationState::None;

	/** 마이그레이션 타임아웃 타이머 */
	FTimerHandle MigrationTimeoutHandle;

	/** 마이그레이션 재시도 타이머 */
	FTimerHandle MigrationRetryHandle;
};

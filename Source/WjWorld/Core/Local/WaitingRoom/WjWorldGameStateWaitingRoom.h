// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Local/Lobby/WjWorldGameStateLobby.h"
#include "Network/SessionTypes.h"
#include "WjWorldGameStateWaitingRoom.generated.h"

class AWjWorldPlayerStateBase;

/**
 * 게임 상태 델리게이트
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerListChanged, const TArray<FPlayerDisplayInfo>&, PlayerList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomInfoChanged, const FRoomSettings&, RoomSettings);

/**
 * 대기실 게임 상태 클래스
 *
 * 역할:
 * - GameStateLobby 상속: 배치 오브젝트 리플리케이션 기능
 * - 방 정보 동기화 (Replicated)
 * - 플레이어 목록 관리 (Replicated)
 * - 준비 상태 관리 (Replicated)
 * - 모든 클라이언트에서 접근 가능한 대기실 상태
 */
UCLASS()
class WJWORLD_API AWjWorldGameStateWaitingRoom : public AWjWorldGameStateLobby
{
	GENERATED_BODY()
	
public:
	AWjWorldGameStateWaitingRoom();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ Room Info
	/**
	 * 방 설정 정보 초기화 (Server Only)
	 * GameMode의 BeginPlay에서 호출
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|WaitingRoom")
	void InitializeRoomSettings(const FRoomSettings& Settings);

	/**
	 * 방 설정 정보 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "GameState|WaitingRoom")
	const FRoomSettings& GetRoomSettings() const { return RoomSettings; }

	/**
	 * 방 설정 정보 업데이트 (Server Only)
	 * 호스트가 대기실에서 설정을 변경할 때 사용
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|WaitingRoom")
	void UpdateRoomSettings(const FRoomSettings& NewSettings);

	//~ Player Management
	/**
	 * 플레이어 목록 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "GameState|WaitingRoom")
	TArray<FPlayerDisplayInfo> GetPlayerList() const;

	/**
	 * 플레이어 수 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "GameState|WaitingRoom")
	int32 GetPlayerCount() const;

	/**
	 * 준비된 플레이어 수 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "GameState|WaitingRoom")
	int32 GetReadyPlayerCount() const;

	/**
	 * 모든 플레이어가 준비되었는지 확인
	 */
	UFUNCTION(BlueprintPure, Category = "GameState|WaitingRoom")
	bool AreAllPlayersReady() const;

	//~ Events
	/**
	 * 플레이어 목록 변경 이벤트
	 */
	UPROPERTY(BlueprintAssignable, Category = "GameState|WaitingRoom|Events")
	FOnPlayerListChanged OnPlayerListChanged;

	/**
	 * 방 정보 변경 이벤트
	 */
	UPROPERTY(BlueprintAssignable, Category = "GameState|WaitingRoom|Events")
	FOnRoomInfoChanged OnRoomInfoChanged;

protected:
	//~ Replicated Properties
	/**
	 * 방 설정 정보 (Server → All Clients)
	 */
	UPROPERTY(ReplicatedUsing = OnRep_RoomSettings, BlueprintReadOnly, Category = "GameState|WaitingRoom")
	FRoomSettings RoomSettings;

	//~ Replication Callbacks
	UFUNCTION()
	void OnRep_RoomSettings();

	//~ Override GameStateBase
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	//~ PlayerState 이벤트 핸들러
	UFUNCTION()
	void OnPlayerNameUpdated(const FString& PlayerName);

	UFUNCTION()
	void OnPlayerReadyStateChanged(int32 PlayerID, bool bIsReady);

private:
	/** 플레이어 목록 변경 브로드캐스트 */
	void BroadcastPlayerListChanged();

	/** 호스트 PlayerID (첫 번째 플레이어) */
	int32 HostPlayerID = -1;
};

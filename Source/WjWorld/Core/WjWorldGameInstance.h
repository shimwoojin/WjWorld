// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
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
 */
UCLASS()
class WJWORLD_API UWjWorldGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void Shutdown() override;

	//~ Session Manager 접근
	/**
	 * SessionManager 가져오기 (블루프린트/C++ 모두 접근 가능)
	 */
	UFUNCTION(BlueprintPure, Category = "Session", meta = (DisplayName = "Get Session Manager"))
	USessionManager* GetSessionManager() const { return SessionManager; }

	//~ 편의 함수 (UI에서 직접 호출 가능)
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool CreateRoom(const FRoomSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Session")
	bool FindRooms(int32 MaxResults = 100);

	UFUNCTION(BlueprintCallable, Category = "Session")
	bool JoinRoom(int32 RoomIndex);

	UFUNCTION(BlueprintCallable, Category = "Session")
	bool StartGame();

	UFUNCTION(BlueprintCallable, Category = "Session")
	bool LeaveRoom();

private:
	/** Session 관리자 */
	UPROPERTY()
	TObjectPtr<USessionManager> SessionManager;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldPlayerControllerBase.h"
#include "WjWorldPlayerControllerWaitingRoom.generated.h"

/**
 * 대기실 플레이어 컨트롤러 클래스
 * 
 * 역할:
 * - 대기실 화면의 입력 처리
 * - 대기실 UI 관리
 * - 준비 상태 및 팀 선택 관리
 */
UCLASS()
class WJWORLD_API AWjWorldPlayerControllerWaitingRoom : public AWjWorldPlayerControllerBase
{
	GENERATED_BODY()

public:
	AWjWorldPlayerControllerWaitingRoom();

protected:
	virtual void BeginPlay() override;

	virtual void InitializeController() override;
	virtual void InitializeUI() override;

public:
	/**
	 * 준비 상태 토글
	 */
	UFUNCTION(BlueprintCallable, Category = "WaitingRoom")
	void ToggleReady();

	/**
	 * 팀 변경 요청
	 */
	UFUNCTION(BlueprintCallable, Category = "WaitingRoom")
	void RequestChangeTeam(int32 NewTeamID);

	/**
	 * 게임 시작 준비 완료 확인
	 */
	UFUNCTION(BlueprintCallable, Category = "WaitingRoom")
	bool CanStartGame() const;

protected:
	/**
	 * 대기실 전용 초기화
	 */
	void InitializeWaitingRoomController();

	/**
	 * 대기실 UI 생성
	 */
	void CreateWaitingRoomUI();

	/**
	 * 준비 상태 변경 처리
	 */
	UFUNCTION(Server, Reliable)
	void ServerToggleReady();

	/**
	 * 팀 변경 요청 처리
	 */
	UFUNCTION(Server, Reliable)
	void ServerRequestChangeTeam(int32 NewTeamID);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldPlayerControllerBase.h"
#include "WjWorldPlayerControllerLobby.generated.h"

class UWjWorldPlacementComponent;
class AWjWorldPlacementCameraPawn;

/**
 * 로비 플레이어 컨트롤러 클래스
 *
 * 역할:
 * - 로비 화면의 입력 처리
 * - 로비 UI 관리
 * - 매치메이킹 및 모드 선택 관리
 * - 오브젝트 배치 시스템 관리
 */
UCLASS()
class WJWORLD_API AWjWorldPlayerControllerLobby : public AWjWorldPlayerControllerBase
{
	GENERATED_BODY()

public:
	AWjWorldPlayerControllerLobby();

	/** 배치 컴포넌트 getter */
	UFUNCTION(BlueprintCallable, Category = "Lobby|Placement")
	UWjWorldPlacementComponent* GetPlacementComponent() const { return PlacementComponent; }

	/** 배치 카메라 Pawn으로 전환 */
	void SwitchToPlacementCamera();

	/** 원래 캐릭터로 복귀 */
	void RestoreOriginalPawn();

	/** 카메라 모드 중인지 확인 */
	bool IsInPlacementCameraMode() const { return PlacementCameraPawn != nullptr; }

protected:
	virtual void BeginPlay() override;

	virtual void InitializeController() override;
	virtual void InitializeUI() override;

public:
	/**
	 * 방 목록 창 열기
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void OpenRoomList();

	/**
	 * 방 생성 창 열기
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void OpenCreateRoom();

protected:
	/**
	 * 로비 전용 초기화
	 */
	void InitializeLobbyController();

	/**
	 * 로비 UI 생성
	 */
	void CreateLobbyUI();

private:
	/** 배치 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "Placement")
	TObjectPtr<UWjWorldPlacementComponent> PlacementComponent;

	/** 배치 모드 전환 전 원래 Pawn */
	TWeakObjectPtr<APawn> OriginalPawn;

	/** 배치 카메라 Pawn 인스턴스 */
	UPROPERTY()
	TObjectPtr<AWjWorldPlacementCameraPawn> PlacementCameraPawn;
};

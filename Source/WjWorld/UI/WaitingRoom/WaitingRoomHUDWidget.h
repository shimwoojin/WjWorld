// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "Core/Local/WaitingRoom/WjWorldGameStateWaitingRoom.h"
#include "WaitingRoomHUDWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

/**
 * 대기실 HUD 위젯
 * 
 * 기능:
 * - 플레이어 목록 표시 (GameState 동기화)
 * - 방 정보 표시 (GameState 동기화)
 * - 준비 버튼
 * - 게임 시작 버튼 (호스트만)
 * - 나가기 버튼
 */
UCLASS()
class WJWORLD_API UWaitingRoomHUDWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	//~ UI 위젯 바인딩 (Blueprint에서 설정)

	/** 방 이름 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RoomNameText;

	/** 게임 모드 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GameModeText;

	/** 플레이어 수 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlayerCountText;

	/** 플레이어 목록 컨테이너 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> PlayerListContainer;

	/** 준비 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReadyButton;

	/** 준비 버튼 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ReadyButtonText;

	/** 게임 시작 버튼 (호스트만) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartGameButton;

	/** 나가기 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LeaveButton;

protected:
	//~ 버튼 클릭 이벤트

	UFUNCTION()
	void OnReadyClicked();

	UFUNCTION()
	void OnStartGameClicked();

	UFUNCTION()
	void OnLeaveClicked();

	//~ GameState 이벤트 핸들러

	UFUNCTION()
	void OnRoomInfoChanged(const FRoomSettings& RoomSettings);

	UFUNCTION()
	void OnPlayerListChanged(const TArray<FPlayerDisplayInfo>& PlayerList);

	UFUNCTION()
	void OnPlayerReadyStateChanged(int32 PlayerID, bool bIsReady);

private:
	/** 방 정보 업데이트 */
	void UpdateRoomInfo();

	/** 플레이어 목록 업데이트 */
	void UpdatePlayerList();

	/** 준비 버튼 상태 업데이트 */
	void UpdateReadyButton();

	/** 게임 시작 버튼 상태 업데이트 */
	void UpdateStartGameButton();

	/** GameState 캐시 */
	UPROPERTY()
	TObjectPtr<AWjWorldGameStateWaitingRoom> CachedGameState;
};

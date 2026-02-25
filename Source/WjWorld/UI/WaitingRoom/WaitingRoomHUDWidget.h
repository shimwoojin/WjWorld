// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "Core/Local/WaitingRoom/WjWorldGameStateWaitingRoom.h"
#include "WaitingRoomHUDWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class UPlayerProfileWidget;
class UComboBoxString;
class UCanvasPanel;
class USettingsWidget;

/**
 * 대기실 HUD 위젯
 *
 * 기능:
 * - 플레이어 목록 표시 (GameState 동기화) - 클릭 시 프로필 팝업
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

	/** 맵 이름 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MapText;

	//~ 호스트 설정 변경 UI (호스트만 표시)

	/** 호스트 설정 패널 (호스트만 표시) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCanvasPanel> HostSettingsPanel;

	/** 게임 모드 선택 ComboBox */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UComboBoxString> GameModeComboBox;

	/** 맵 선택 ComboBox */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UComboBoxString> MapComboBox;

	/** 설정 적용 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ApplySettingsButton;

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

	/** 게임 시작 불가 사유 텍스트 (호스트에게만 표시) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StartGameStatusText;

	/** 나가기 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LeaveButton;

	/** 설정 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SettingsButton;

	/** 프로필 위젯 클래스 (Blueprint에서 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "Profile")
	TSubclassOf<UPlayerProfileWidget> ProfileWidgetClass;

	/** 설정 위젯 클래스 (Blueprint에서 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	TSubclassOf<USettingsWidget> SettingsWidgetClass;

protected:
	//~ 버튼 클릭 이벤트

	UFUNCTION()
	void OnReadyClicked();

	UFUNCTION()
	void OnStartGameClicked();

	UFUNCTION()
	void OnLeaveClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnApplySettingsClicked();

	UFUNCTION()
	void OnGameModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	//~ GameState 이벤트 핸들러

	UFUNCTION()
	void OnRoomInfoChanged(const FRoomSettings& RoomSettings);

	UFUNCTION()
	void OnPlayerListChanged(const TArray<FPlayerDisplayInfo>& PlayerList);

	UFUNCTION()
	void OnPlayerReadyStateChanged(int32 PlayerID, bool bIsReady);

private:
	/** 방 정보 업데이트 (설정 값을 직접 전달하거나 GameState에서 읽음) */
	void UpdateRoomInfo(const FRoomSettings* InSettings = nullptr);

	/** 플레이어 목록 업데이트 */
	void UpdatePlayerList();

	/** 준비 버튼 상태 업데이트 */
	void UpdateReadyButton();

	/** 게임 시작 버튼 상태 업데이트 */
	void UpdateStartGameButton();

	/** 호스트 설정 패널 초기화 */
	void InitializeHostSettingsPanel();

	/** 호스트 설정 패널 표시/숨기기 */
	void UpdateHostSettingsPanelVisibility();

	/** 맵 ComboBox 업데이트 (선택된 게임모드에 따라) */
	void UpdateMapComboBoxForGameMode(const FString& GameModeId);

	/** 현재 UI 설정을 GameState에 적용 (게임 시작 전 자동 호출) */
	void ApplyCurrentUISettings();

	/** 플레이어 버튼 클릭 공통 핸들러 (IsHovered로 어떤 버튼인지 판별) */
	UFUNCTION()
	void OnAnyPlayerButtonClicked();

	/** 플레이어 버튼 클릭 → 프로필 표시 */
	void ShowPlayerProfile(int32 PlayerID);

	/** GameState 캐시 */
	UPROPERTY()
	TObjectPtr<AWjWorldGameStateWaitingRoom> CachedGameState;

	/** 프로필 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<UPlayerProfileWidget> ProfileWidgetInstance;

	/** 캐시된 플레이어 목록 (버튼 인덱스 매핑용) */
	TArray<FPlayerDisplayInfo> CachedPlayerDisplayList;

	/** 버튼 → PlayerID 매핑 (안정적인 클릭 판별용) */
	UPROPERTY()
	TMap<UButton*, int32> PlayerButtonToIDMap;

	/** 맵 옵션 표시명 → 실제 값 매핑 (호스트 설정용) */
	TMap<FString, FString> MapOptionDisplayToValue;

	/** 설정 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<USettingsWidget> SettingsWidgetInstance;
};

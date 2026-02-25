// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "Network/SessionTypes.h"
#include "RoomListWindow.generated.h"

class UButton;
class UScrollBox;
class UComboBoxString;
class URoomListEntryWidget;
class UPasswordInputWidget;

/**
 * 방 목록 UI
 * 
 * 기능:
 * - 방 검색
 * - 방 목록 표시
 * - 방 입장
 */
UCLASS()
class WJWORLD_API URoomListWindow : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	/** 팝업 표시 */
	void ShowPopup();

	/** 팝업 표시 (네트워크 모드 지정) */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowPopupWithNetworkMode(ENetworkMode InNetworkMode);

	/** 팝업 닫기 */
	void ClosePopup();

	/** 네트워크 모드 설정 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetNetworkMode(ENetworkMode InNetworkMode) { CurrentNetworkMode = InNetworkMode; }

protected:
	//~ UI 위젯 바인딩

	/** 방 목록 스크롤박스 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> RoomListScrollBox;

	/** 네트워크 모드 선택 (LAN/Steam) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UComboBoxString> NetworkModeComboBox;

	/** 새로고침 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RefreshButton;

	/** 닫기 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	/** 방 목록 엔트리 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<URoomListEntryWidget> RoomListEntryWidgetClass;

	/** 비밀번호 입력 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPasswordInputWidget> PasswordInputWidgetClass;

protected:
	//~ 버튼 이벤트

	UFUNCTION()
	void OnRefreshClicked();

	UFUNCTION()
	void OnCloseClicked();

	UFUNCTION()
	void OnNetworkModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	//~ SessionManager 콜백

	UFUNCTION()
	void OnRoomsFound(bool bWasSuccessful, const TArray<FRoomInfo>& Rooms);

public:
	/**
	 * 비밀번호 검증 후 방 입장 시도
	 * RoomListEntryWidget에서 비공개 방 입장 시 호출
	 */
	void RequestJoinPrivateRoom(int32 RoomIndex);

private:
	/** 네트워크 모드 옵션 초기화 */
	void InitializeNetworkModeOptions();

	/** 방 목록 UI 업데이트 */
	void UpdateRoomList(const TArray<FRoomInfo>& Rooms);

	/** 방 검색 시작 */
	void StartSearching();

	/** 비밀번호 검증 후 입장 진행 */
	void JoinRoomWithPassword(int32 RoomIndex, const FString& Password);

	UFUNCTION()
	void OnPasswordSubmitted(const FString& Password);

	UFUNCTION()
	void OnPasswordCancelled();

	/** 현재 네트워크 모드 */
	ENetworkMode CurrentNetworkMode = ENetworkMode::LAN;

	/** 비밀번호 입력 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<UPasswordInputWidget> PasswordInputWidgetInstance;

	/** 비밀번호 입력 대기 중인 방 인덱스 */
	int32 PendingPasswordRoomIndex = -1;
};

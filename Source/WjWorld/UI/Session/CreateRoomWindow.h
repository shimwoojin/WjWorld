// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "Network/SessionTypes.h"
#include "CreateRoomWindow.generated.h"

class UEditableTextBox;
class UComboBoxString;
class UButton;
class UTextBlock;
class UCheckBox;

/**
 * 방 생성 UI 위젯
 * 
 * 기능:
 * - 방 이름, 게임 모드, 맵, 최대 인원 등 설정
 * - SessionManager를 통한 세션 생성
 * - 입력 유효성 검사
 */
UCLASS()
class WJWORLD_API UCreateRoomWindow : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	/** 팝업 표시 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowPopup();

	/** 팝업 닫기 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ClosePopup();

protected:
	//~ UI 위젯 바인딩 (Blueprint에서 설정)
	
	/** 방 이름 입력 필드 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> RoomNameTextBox;

	/** 게임 모드 선택 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> GameModeComboBox;

	/** 맵 선택 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> MapComboBox;

	/** 최대 인원 표시 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MaxPlayersText;

	/** 최대 인원 감소 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DecreasePlayersButton;

	/** 최대 인원 증가 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> IncreasePlayersButton;

	/** 비공개 방 체크박스 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> PrivateCheckBox;

	/** 비밀번호 입력 필드 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> PasswordTextBox;

	/** 게임 중 입장 허용 체크박스 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> AllowJoinInProgressCheckBox;

	/** 취소 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;

	/** 방 만들기 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CreateButton;

protected:
	//~ 버튼 클릭 이벤트

	UFUNCTION()
	void OnDecreasePlayersClicked();

	UFUNCTION()
	void OnIncreasePlayersClicked();

	UFUNCTION()
	void OnPrivateCheckBoxChanged(bool bIsChecked);

	UFUNCTION()
	void OnCancelClicked();

	UFUNCTION()
	void OnCreateClicked();

	//~ SessionManager 콜백

	UFUNCTION()
	void OnRoomCreated(bool bWasSuccessful);

private:
	/** 최대 인원 업데이트 */
	void UpdateMaxPlayersDisplay();

	/** 입력 유효성 검사 */
	bool ValidateInput(FString& OutErrorMessage);

	/** RoomSettings 생성 */
	FRoomSettings BuildRoomSettings();

	/** 게임 모드 옵션 초기화 */
	void InitializeGameModeOptions();

	/** 맵 옵션 초기화 */
	void InitializeMapOptions();

private:
	/** 현재 설정된 최대 인원 */
	UPROPERTY()
	int32 CurrentMaxPlayers = 8;

	/** 최소 인원 */
	static constexpr int32 MIN_PLAYERS = 2;

	/** 최대 인원 */
	static constexpr int32 MAX_PLAYERS = 8;
};

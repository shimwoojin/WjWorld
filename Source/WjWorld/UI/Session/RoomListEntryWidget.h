// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "Network/SessionTypes.h"
#include "RoomListEntryWidget.generated.h"

class UButton;
class UTextBlock;
class URoomListWindow;

/**
 * 방 목록 엔트리 위젯
 * 
 * 각 방의 정보를 표시하고 입장 버튼 제공
 */
UCLASS()
class WJWORLD_API URoomListEntryWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	/** 방 정보 설정 */
	UFUNCTION(BlueprintCallable, Category = "RoomList")
	void SetRoomInfo(const FRoomInfo& InRoomInfo);

protected:
	//~ UI 위젯 바인딩

	/** 방 이름 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RoomNameText;

	/** 게임 모드 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GameModeText;

	/** 플레이어 수 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerCountText;

	/** 핑 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PingText;

	/** 입장 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

protected:
	//~ 버튼 이벤트

	UFUNCTION()
	void OnJoinClicked();

	//~ SessionManager 콜백

	UFUNCTION()
	void OnRoomJoined(bool bWasSuccessful);

private:
	/** 현재 방 정보 */
	FRoomInfo CurrentRoomInfo;

	/** UI 업데이트 */
	void UpdateUI();
};

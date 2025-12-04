// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldGameModeBase.h"
#include "WjWorldGameModeLobby.generated.h"

class UCreateRoomWindow;
class URoomListWindow;

/**
 * 로비 게임 모드
 * 
 * 기능:
 * - 방 생성 UI 관리
 * - 방 목록 UI 관리
 */
UCLASS()
class WJWORLD_API AWjWorldGameModeLobby : public AWjWorldGameModeBase
{
	GENERATED_BODY()
	
public:
	AWjWorldGameModeLobby();

	virtual void BeginPlay() override;

	/** 방 생성 UI 표시 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ShowCreateRoomWindow();

	/** 방 목록 UI 표시 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ShowRoomListWindow();

protected:
	/** 방 생성 UI 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UCreateRoomWindow> CreateRoomWindowClass;

	/** 방 목록 UI 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<URoomListWindow> RoomListWindowClass;

private:
	/** 방 생성 UI 인스턴스 */
	UPROPERTY()
	TObjectPtr<UCreateRoomWindow> CreateRoomWindowInstance;

	/** 방 목록 UI 인스턴스 */
	UPROPERTY()
	TObjectPtr<URoomListWindow> RoomListWindowInstance;
};

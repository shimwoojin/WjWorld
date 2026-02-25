// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldHUDBase.h"
#include "WjWorldHUDWaitingRoom.generated.h"

class UWaitingRoomHUDWidget;

/**
 * 대기실 HUD
 * 
 * 기능:
 * - 플레이어 목록 표시
 * - 준비 상태 표시
 * - 채팅 (선택사항)
 * - 게임 시작 버튼 (호스트만)
 */
UCLASS()
class WJWORLD_API AWjWorldHUDWaitingRoom : public AWjWorldHUDBase
{
	GENERATED_BODY()
	
public:
	AWjWorldHUDWaitingRoom();

	virtual void BeginPlay() override;
	virtual bool TryCloseTopPopup() override;

	/** 대기실 HUD 표시 */
	void ShowWaitingRoomHUD();

	/** 대기실 HUD 숨기기 */
	void HideWaitingRoomHUD();

protected:
	/** 대기실 HUD 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UWaitingRoomHUDWidget> WaitingRoomHUDWidgetClass;

private:
	/** 대기실 HUD 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<UWaitingRoomHUDWidget> WaitingRoomHUDWidgetInstance;
};

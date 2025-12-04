// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldHUDBase.h"
#include "WjWorldHUDLobby.generated.h"

class ULobbyHUDWidget;

/**
 * 로비 HUD
 * 
 * 기능:
 * - 로비 UI 위젯 관리
 * - "방 만들기" 버튼 등 제공
 * - 추후 "방 찾기", "친구 목록" 등 추가 가능
 */
UCLASS()
class WJWORLD_API AWjWorldHUDLobby : public AWjWorldHUDBase
{
	GENERATED_BODY()
	
public:
	AWjWorldHUDLobby();
	
	virtual void BeginPlay() override;

	/** 로비 HUD 표시 */
	void ShowLobbyHUD();

	/** 로비 HUD 숨기기 */
	void HideLobbyHUD();

protected:
	/** 로비 HUD 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<ULobbyHUDWidget> LobbyHUDWidgetClass;

private:
	/** 로비 HUD 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<ULobbyHUDWidget> LobbyHUDWidgetInstance;
};

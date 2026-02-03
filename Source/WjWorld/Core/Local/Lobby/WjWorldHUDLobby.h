// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldHUDBase.h"
#include "WjWorldHUDLobby.generated.h"

class ULobbyHUDWidget;
class UPlacementHUDWidget;
class UWjWorldPlacementComponent;

/**
 * 로비 HUD
 *
 * 기능:
 * - 로비 UI 위젯 관리
 * - "방 만들기" 버튼 등 제공
 * - 배치 모드 HUD 전환
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

	/** 배치 모드 HUD 표시 (LobbyHUD 숨김) */
	void ShowPlacementHUD(UWjWorldPlacementComponent* PlacementComponent);

	/** 배치 모드 HUD 숨기기 (LobbyHUD 복원) */
	void HidePlacementHUD();

protected:
	/** 로비 HUD 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<ULobbyHUDWidget> LobbyHUDWidgetClass;

	/** 배치 HUD 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPlacementHUDWidget> PlacementHUDWidgetClass;

private:
	/** 로비 HUD 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<ULobbyHUDWidget> LobbyHUDWidgetInstance;

	/** 배치 HUD 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<UPlacementHUDWidget> PlacementHUDWidgetInstance;
};

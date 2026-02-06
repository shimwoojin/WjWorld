// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "GamePlay/Placement/WjWorldPlacementTypes.h"
#include "LobbyHUDWidget.generated.h"

class UButton;
class UPlayerProfileWidget;
class UCosmeticMainWindow;
class UPlacementContextSelectWidget;

/**
 * 로비 HUD 위젯
 *
 * 기능:
 * - "방 만들기" 버튼
 * - "방 찾기" 버튼
 * - "직접 입장" 버튼 (NULL Subsystem용)
 * - "설정" 버튼 (추후)
 * - "프로필" 버튼
 */
UCLASS()
class WJWORLD_API ULobbyHUDWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	//~ UI 위젯 바인딩 (Blueprint에서 설정)

	/** 방 만들기 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CreateRoomButton;

	/** 방 찾기 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> FindRoomButton;

	/** 직접 입장 버튼 (NULL Subsystem용 임시) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> DirectConnectButton;

	/** 설정 버튼 (추후) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SettingsButton;

	/** 프로필 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ProfileButton;

	/** 배치 모드 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> PlacementModeButton;

	/** 코스메틱 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CosmeticButton;

	/** 프로필 위젯 클래스 (Blueprint에서 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "Profile")
	TSubclassOf<UPlayerProfileWidget> ProfileWidgetClass;

	/** 코스메틱 윈도우 클래스 (Blueprint에서 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "Cosmetic")
	TSubclassOf<UCosmeticMainWindow> CosmeticWindowClass;

	/** 배치 컨텍스트 선택 위젯 클래스 (Blueprint에서 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "Placement")
	TSubclassOf<UPlacementContextSelectWidget> PlacementContextSelectClass;

protected:
	//~ 버튼 클릭 이벤트

	UFUNCTION()
	void OnCreateRoomClicked();

	UFUNCTION()
	void OnFindRoomClicked();

	UFUNCTION()
	void OnDirectConnectClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnProfileClicked();

	UFUNCTION()
	void OnPlacementModeClicked();

	UFUNCTION()
	void OnCosmeticClicked();

	/** 배치 컨텍스트 선택 핸들러 */
	UFUNCTION()
	void OnPlacementContextSelected(EPlacementContext SelectedContext);

private:
	UPROPERTY()
	TObjectPtr<UPlayerProfileWidget> ProfileWidgetInstance;

	UPROPERTY()
	TObjectPtr<UCosmeticMainWindow> CosmeticWindowInstance;

	UPROPERTY()
	TObjectPtr<UPlacementContextSelectWidget> PlacementContextSelectInstance;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldGameModeBase.h"
#include "WjWorldGameModeLobby.generated.h"

class UCreateRoomWindow;
class URoomListWindow;
class AWjWorldGameStateLobby;
class UWjWorldPlaceableObjectDataAsset;

/**
 * 로비 게임 모드
 *
 * 기능:
 * - 방 생성 UI 관리
 * - 방 목록 UI 관리
 * - 오브젝트 배치 모드 관리
 * - 멀티플레이 지원 (호스트 = 방장)
 */
UCLASS()
class WJWORLD_API AWjWorldGameModeLobby : public AWjWorldGameModeBase
{
	GENERATED_BODY()

public:
	AWjWorldGameModeLobby();

	virtual void BeginPlay() override;

	//~ 멀티플레이 오버라이드
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** 방 생성 UI 표시 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ShowCreateRoomWindow();

	/** 방 목록 UI 표시 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ShowRoomListWindow();

	/** 배치 모드 진입 (호스트만 가능) */
	UFUNCTION(BlueprintCallable, Category = "Lobby|Placement")
	void EnterPlacementMode();

	/** 배치 모드 종료 */
	UFUNCTION(BlueprintCallable, Category = "Lobby|Placement")
	void ExitPlacementMode();

	/** 현재 플레이어가 호스트인지 확인 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool IsHost(APlayerController* PC) const;

	/** GameStateLobby 참조 */
	AWjWorldGameStateLobby* GetLobbyGameState() const;

	/** 카탈로그 참조 */
	UWjWorldPlaceableObjectDataAsset* GetCatalog() const { return CachedCatalog; }

protected:
	/** 방 생성 UI 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UCreateRoomWindow> CreateRoomWindowClass;

	/** 방 목록 UI 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<URoomListWindow> RoomListWindowClass;

private:
	/** 카탈로그 로드 */
	void LoadCatalog();

	/** 호스트의 저장된 레이아웃을 GameState에 로드 (서버에서 호출) */
	void LoadHostLayoutToGameState();

	/** 새 플레이어 초기화 */
	void InitializeNewPlayer(APlayerController* NewPlayer);

	/** 방 생성 UI 인스턴스 */
	UPROPERTY()
	TObjectPtr<UCreateRoomWindow> CreateRoomWindowInstance;

	/** 방 목록 UI 인스턴스 */
	UPROPERTY()
	TObjectPtr<URoomListWindow> RoomListWindowInstance;

	/** 카탈로그 캐시 */
	UPROPERTY()
	TObjectPtr<UWjWorldPlaceableObjectDataAsset> CachedCatalog;
};

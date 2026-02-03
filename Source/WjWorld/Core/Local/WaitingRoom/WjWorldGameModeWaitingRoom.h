// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldGameModeBase.h"
#include "WjWorldGameModeWaitingRoom.generated.h"

class UWjWorldPlaceableObjectDataAsset;

/**
 * 대기실 게임 모드
 *
 * 기능:
 * - 플레이어 입장/퇴장 관리
 * - GameState 초기화
 * - 게임 시작 (호스트만)
 * - 배치 오브젝트 로드 (GameStateLobby 상속 활용)
 */
UCLASS()
class WJWORLD_API AWjWorldGameModeWaitingRoom : public AWjWorldGameModeBase
{
	GENERATED_BODY()

public:
	AWjWorldGameModeWaitingRoom();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** 게임 시작 (호스트만 가능) */
	UFUNCTION(BlueprintCallable, Category = "WaitingRoom")
	void StartGame();

private:
	/** 배치 카탈로그 로드 */
	void LoadPlacementCatalog();

	/** 호스트의 저장된 배치 레이아웃을 GameState에 로드 */
	void LoadHostLayoutToGameState();

	/** 카탈로그 캐시 */
	UPROPERTY()
	TObjectPtr<UWjWorldPlaceableObjectDataAsset> CachedPlacementCatalog;
};

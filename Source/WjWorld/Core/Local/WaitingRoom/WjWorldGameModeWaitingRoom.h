// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldGameModeBase.h"
#include "WjWorldGameModeWaitingRoom.generated.h"

/**
 * 대기실 게임 모드
 * 
 * 기능:
 * - 플레이어 입장/퇴장 관리
 * - GameState 초기화
 * - 게임 시작 (호스트만)
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
};

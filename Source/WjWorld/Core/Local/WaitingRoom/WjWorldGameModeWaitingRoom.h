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
 * - 준비 상태 관리
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

	/** 현재 대기실 인원 수 */
	UFUNCTION(BlueprintCallable, Category = "WaitingRoom")
	int32 GetPlayerCount() const { return PlayerCount; }

	/** 게임 시작 (호스트만 가능) */
	UFUNCTION(BlueprintCallable, Category = "WaitingRoom")
	void StartGame();

private:
	/** 현재 플레이어 수 */
	int32 PlayerCount;
};

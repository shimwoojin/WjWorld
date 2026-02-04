// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldCharacterBase.h"
#include "WjWorldCharacterWaitingRoom.generated.h"

/**
 * 대기실 캐릭터 클래스
 * 
 * 역할:
 * - 대기실 화면에서 사용되는 캐릭터
 * - 팀 선택 및 준비 상태 표현
 */
UCLASS()
class WJWORLD_API AWjWorldCharacterWaitingRoom : public AWjWorldCharacterBase
{
	GENERATED_BODY()

public:
	AWjWorldCharacterWaitingRoom();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	virtual void InitializeCharacter() override;
	virtual void SetupInputBindings(class UInputComponent* PlayerInputComponent) override;

public:
	/**
	 * 준비 상태에 따른 외형 변경
	 */
	UFUNCTION(BlueprintCallable, Category = "WaitingRoom|Character")
	void UpdateReadyState(bool bIsReady);

	/**
	 * 팀 변경에 따른 외형 업데이트
	 */
	UFUNCTION(BlueprintCallable, Category = "WaitingRoom|Character")
	void UpdateTeamAppearance(int32 TeamID);

protected:
	/**
	 * 대기실 전용 초기화
	 */
	void InitializeWaitingRoomCharacter();

	/**
	 * 현재 준비 상태
	 */
	UPROPERTY(BlueprintReadOnly, Category = "WaitingRoom")
	bool bIsReady = false;

	/**
	 * 현재 팀 ID
	 */
	UPROPERTY(BlueprintReadOnly, Category = "WaitingRoom")
	int32 CurrentTeamID = 0;
};

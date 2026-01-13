// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldCharacterBase.h"
#include "WjWorldCharacterLobby.generated.h"

/**
 * 로비 캐릭터 클래스
 * 
 * 역할:
 * - 로비 화면에서 사용되는 캐릭터
 * - 로비 전용 기능 제공 (캐릭터 커스터마이징 등)
 */
UCLASS()
class WJWORLD_API AWjWorldCharacterLobby : public AWjWorldCharacterBase
{
	GENERATED_BODY()

public:
	AWjWorldCharacterLobby();

protected:
	virtual void BeginPlay() override;

	virtual void InitializeCharacter() override;
	virtual void SetupInputBindings(class UInputComponent* PlayerInputComponent) override;

public:
	/**
	 * 캐릭터 외형 변경
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby|Character")
	void UpdateCharacterAppearance();

protected:
	/**
	 * 로비 전용 초기화
	 */
	void InitializeLobbyCharacter();
};

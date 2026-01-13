// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WjWorldPlayerControllerBase.generated.h"

class UInputMappingContext;
class UEnhancedInputComponent;

/**
 * 기본 플레이어 컨트롤러 클래스
 * 
 * 역할:
 * - 모든 플레이어 컨트롤러의 Base 클래스
 * - 공통 입력 처리 및 UI 관리
 */
UCLASS(abstract)
class WJWORLD_API AWjWorldPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	AWjWorldPlayerControllerBase();

protected:
	virtual void BeginPlay() override;

protected:
	/**
	 * 컨트롤러 초기화
	 */
	virtual void InitializeController();

	/**
	 * UI 초기화
	 */
	virtual void InitializeUI();

public:
	UFUNCTION(Exec)
	void CheckInputMode();

	UFUNCTION(Exec)
	void ChangeCharacterViewMode(int32 InViewMode);
};

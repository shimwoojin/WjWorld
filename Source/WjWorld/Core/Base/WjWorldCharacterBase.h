// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "GamePlay/Camera/WJCameraTypes.h"
#include "GameplayTagContainer.h"
#include "WjWorldCharacterBase.generated.h"

class UGameplayCameraComponent;
class UInputMappingContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterViewModeChanged, FGameplayTag, CameraMode);

/**
 * 기본 캐릭터 클래스
 * 
 * 역할:
 * - 모든 캐릭터의 Base 클래스
 * - 공통 기능 및 인터페이스 제공
 */
UCLASS(abstract)
class WJWORLD_API AWjWorldCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AWjWorldCharacterBase();

public:
	void SetCharacterViewMode(ECharacterCameraMode NewViewMode);
	void SetCharacterViewMode(const FGameplayTag& NewViewMode);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	/**
	 * 캐릭터 초기화
	 */
	virtual void InitializeCharacter();

	/**
	 * 입력 바인딩 설정
	 */
	virtual void SetupInputBindings(class UInputComponent* PlayerInputComponent);

	virtual void GasInputPressed(int32 InputID);
	virtual void GasInputReleased(int32 InputID);

	//~ Input Handlers
	/**
	 * 이동 입력 처리
	 */
	UFUNCTION()
	virtual void Move(const FInputActionValue& Value);

	/**
	 * 상호작용 입력 처리
	 */
	UFUNCTION()
	virtual void Interact(const FInputActionValue& Value);

public:
	UPROPERTY(BlueprintAssignable)
	FOnCharacterViewModeChanged OnCharacterViewModeChanged;

protected:
	/**
	 * GamePlay 카메라 컴포넌트
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGameplayCameraComponent> GamePlayCamera;

	/**
	 * 기본 Input Mapping Context
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

public:
	TObjectPtr<UGameplayCameraComponent> GetGamePlayCamera() { return GamePlayCamera; }
	TObjectPtr<UInputMappingContext> GetDefaultMappingContext() { return DefaultMappingContext; }

private:
	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	ECharacterCameraMode StartCameraMode = ECharacterCameraMode::TopDown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayTag CameraMode;
};

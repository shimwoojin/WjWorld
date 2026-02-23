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
class UWjWorldCosmeticComponent;

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
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
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

	/**
	 * 점프 입력 처리 (IA_Jump 자동 바인딩)
	 */
	UFUNCTION()
	void Jump_Started();

	UFUNCTION()
	void Jump_Completed();

	/** 네이티브 점프 허용 여부 (Play에서는 GAS가 처리하므로 false) */
	virtual bool CanNativeJump() const;

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
	 * 기본 스켈레탈 메시 (설정 안하면 DeveloperSettings 사용)
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	TSoftObjectPtr<USkeletalMesh> DefaultSkeletalMesh;

	/**
	 * 기본 애니메이션 블루프린트 클래스 (설정 안하면 DeveloperSettings 사용)
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	TSoftClassPtr<UAnimInstance> DefaultAnimBlueprintClass;

	/**
	 * 기본 Input Mapping Context (설정 안하면 DeveloperSettings 사용)
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> DefaultMappingContext;

	/**
	 * 코스메틱 컴포넌트
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cosmetic")
	TObjectPtr<UWjWorldCosmeticComponent> CosmeticComponent;

public:
	TObjectPtr<UGameplayCameraComponent> GetGamePlayCamera() { return GamePlayCamera; }
	UInputMappingContext* GetDefaultMappingContext() const;
	UWjWorldCosmeticComponent* GetCosmeticComponent() const { return CosmeticComponent; }
	const FGameplayTag& GetCameraMode() const { return CameraMode; }

private:
	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	ECharacterCameraMode StartCameraMode = ECharacterCameraMode::TopDown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayTag CameraMode;
};

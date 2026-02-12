// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "WjWorldPlacementCameraPawn.generated.h"

class UCameraComponent;
class UFloatingPawnMovement;
class UInputAction;
struct FInputActionValue;

/**
 * 배치 모드 자유 비행 카메라 Pawn
 *
 * Lobby 배치 모드 진입 시 원래 캐릭터 대신 빙의하여 자유 이동 지원.
 * - WASD: 수평 이동 (컨트롤러 Yaw 기준)
 * - Q/E: 수직 이동
 * - RMB 홀드 + 마우스: 카메라 회전
 * - 로컬 전용 (bReplicates = false)
 */
UCLASS()
class WJWORLD_API AWjWorldPlacementCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AWjWorldPlacementCameraPawn();

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void HandleMoveInput(const FInputActionValue& Value);
	void HandleLookInput(const FInputActionValue& Value);
	void HandleVerticalMoveInput(const FInputActionValue& Value);
	void HandleRightMousePressed(const FInputActionValue& Value);
	void HandleRightMouseReleased(const FInputActionValue& Value);

	/** DeveloperSettings에서 InputAction 로드 */
	void LoadInputSettings();

	bool bRightMouseHeld = false;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UFloatingPawnMovement> FloatingMovement;

	UPROPERTY()
	TObjectPtr<UInputAction> CameraMoveAction;

	UPROPERTY()
	TObjectPtr<UInputAction> CameraLookAction;

	UPROPERTY()
	TObjectPtr<UInputAction> CameraRightMouseAction;

	UPROPERTY()
	TObjectPtr<UInputAction> CameraVerticalMoveAction;
};

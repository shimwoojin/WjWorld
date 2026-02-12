// Fill out your copyright notice in the Description page of Project Settings.

#include "WjWorldPlacementCameraPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "WjWorldLogCategories.h"

AWjWorldPlacementCameraPawn::AWjWorldPlacementCameraPawn()
{
	bReplicates = false;
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SceneRoot);
	CameraComponent->bUsePawnControlRotation = true;

	FloatingMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));
	FloatingMovement->MaxSpeed = 1200.f;
	FloatingMovement->Acceleration = 4000.f;
	FloatingMovement->Deceleration = 8000.f;
}

void AWjWorldPlacementCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	LoadInputSettings();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementCameraPawn: EnhancedInputComponent not found"));
		return;
	}

	if (CameraMoveAction)
	{
		EIC->BindAction(CameraMoveAction, ETriggerEvent::Triggered, this, &AWjWorldPlacementCameraPawn::HandleMoveInput);
	}

	if (CameraLookAction)
	{
		EIC->BindAction(CameraLookAction, ETriggerEvent::Triggered, this, &AWjWorldPlacementCameraPawn::HandleLookInput);
	}

	if (CameraRightMouseAction)
	{
		EIC->BindAction(CameraRightMouseAction, ETriggerEvent::Started, this, &AWjWorldPlacementCameraPawn::HandleRightMousePressed);
		EIC->BindAction(CameraRightMouseAction, ETriggerEvent::Completed, this, &AWjWorldPlacementCameraPawn::HandleRightMouseReleased);
	}

	if (CameraVerticalMoveAction)
	{
		EIC->BindAction(CameraVerticalMoveAction, ETriggerEvent::Triggered, this, &AWjWorldPlacementCameraPawn::HandleVerticalMoveInput);
	}
}

void AWjWorldPlacementCameraPawn::LoadInputSettings()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		return;
	}

	if (!Settings->PlacementCameraMoveAction.IsNull())
	{
		CameraMoveAction = Settings->PlacementCameraMoveAction.LoadSynchronous();
	}
	if (!Settings->PlacementCameraLookAction.IsNull())
	{
		CameraLookAction = Settings->PlacementCameraLookAction.LoadSynchronous();
	}
	if (!Settings->PlacementCameraRightMouseAction.IsNull())
	{
		CameraRightMouseAction = Settings->PlacementCameraRightMouseAction.LoadSynchronous();
	}
	if (!Settings->PlacementCameraVerticalMoveAction.IsNull())
	{
		CameraVerticalMoveAction = Settings->PlacementCameraVerticalMoveAction.LoadSynchronous();
	}
}

void AWjWorldPlacementCameraPawn::HandleMoveInput(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	if (Input.IsNearlyZero())
	{
		return;
	}

	// 컨트롤러 Yaw 기준으로 Forward/Right 방향 계산
	const FRotator ControlRotation = GetControlRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, Input.Y);
	AddMovementInput(RightDir, Input.X);
}

void AWjWorldPlacementCameraPawn::HandleLookInput(const FInputActionValue& Value)
{
	if (!bRightMouseHeld)
	{
		return;
	}

	const FVector2D Input = Value.Get<FVector2D>();
	AddControllerYawInput(Input.X);
	AddControllerPitchInput(-Input.Y);
}

void AWjWorldPlacementCameraPawn::HandleVerticalMoveInput(const FInputActionValue& Value)
{
	const float Input = Value.Get<float>();
	if (FMath::IsNearlyZero(Input))
	{
		return;
	}

	AddMovementInput(FVector::UpVector, Input);
}

void AWjWorldPlacementCameraPawn::HandleRightMousePressed(const FInputActionValue& Value)
{
	bRightMouseHeld = true;
}

void AWjWorldPlacementCameraPawn::HandleRightMouseReleased(const FInputActionValue& Value)
{
	bRightMouseHeld = false;
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "WjWorldCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "WjWorldGameplayTag.h"
#include "Core/CameraAsset.h"

AWjWorldCharacterBase::AWjWorldCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Mesh 컴포넌트 설정
	if (GetMesh())
	{
		// Mesh 위치 및 회전 설정 (캡슐 컴포넌트 기준)
		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
		GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

		// 기본 SkeletalMesh 설정
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
			TEXT("/Game/Core/Characters/Mannequins/Meshes/SKM_Quinn_Simple")
		);
		if (MeshAsset.Succeeded())
		{
			GetMesh()->SetSkeletalMesh(MeshAsset.Object);
		}

		// 애니메이션 블루프린트 설정
		static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPClass(
			TEXT("/Game/Core/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed")
		);
		if (AnimBPClass.Succeeded())
		{
			GetMesh()->SetAnimInstanceClass(AnimBPClass.Class);
		}
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultIMC(
		TEXT("/Game/Core/Input/IMC_Default")
	);
	if (DefaultIMC.Succeeded())
	{
		DefaultMappingContext = DefaultIMC.Object;
	}

	// GamePlay 카메라 컴포넌트 생성
	GamePlayCamera = CreateDefaultSubobject<UGameplayCameraComponent>(TEXT("GamePlayCamera"));
	GamePlayCamera->SetupAttachment(RootComponent);
	//GamePlayCamera->bSetControlRotationWhenViewTarget = true;

	SetCharacterViewMode(ECharacterCameraMode::ThirdPerson);
}

void AWjWorldCharacterBase::SetCharacterViewMode(ECharacterCameraMode NewViewMode)
{
	FGameplayTag NewCameraMode = FGameplayTag::EmptyTag;

	switch (NewViewMode)
	{
	case ECharacterCameraMode::TopDown:
	{
		NewCameraMode = WjWorldGameplayTag::Camera_TopDown();
		break;
	}
	case ECharacterCameraMode::ThirdPerson:
	{
		NewCameraMode = WjWorldGameplayTag::Camera_ThirdPerson();
		break;
	}
	case ECharacterCameraMode::FirstPerson:
	{
		NewCameraMode = WjWorldGameplayTag::Camera_FirstPerson();
		break;
	}
	}

	SetCharacterViewMode(NewCameraMode);
}

void AWjWorldCharacterBase::SetCharacterViewMode(const FGameplayTag& NewViewMode)
{
	if (GamePlayCamera)
	{
		UCameraAsset* CameraAsset = GamePlayCamera->CameraReference.GetCameraAsset();
		if (CameraAsset)
		{
			FInstancedPropertyBag& DefaultParams = CameraAsset->GetDefaultParameters();
			auto Result = DefaultParams.GetValueStruct<FGameplayTag>(TEXT("CameraMode"));
			if (Result.HasError() == false)
			{
				FGameplayTag*& CameraModePtrRef = Result.GetValue();
				*CameraModePtrRef = NewViewMode;
			}
		}
	}
}

void AWjWorldCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeCharacter();
}

void AWjWorldCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWjWorldCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	APlayerController* PC = Cast<APlayerController>(GetController());
	if(PC == nullptr)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	SetupInputBindings(PlayerInputComponent);
}

void AWjWorldCharacterBase::InitializeCharacter()
{
	// Base implementation - override in derived classes
}

void AWjWorldCharacterBase::SetupInputBindings(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedInputComponent && DefaultMappingContext)
	{
		TSet<FString> AlreadyBoundActions;
		const TArray<FEnhancedActionKeyMapping>& Mappings = DefaultMappingContext->GetMappings();

		for (const auto& Mapping : Mappings)
		{
			if (!Mapping.Action)
			{
				continue;
			}

			FString ActionName = Mapping.Action.GetName();
			ActionName.RemoveFromStart(TEXT("IA_"));

			// TriggerEvent별로 함수 찾기
			TArray<TPair<ETriggerEvent, FString>> EventSuffixes = {
				{ETriggerEvent::Started, TEXT("_Started")},
				{ETriggerEvent::Ongoing, TEXT("_Ongoing")},
				{ETriggerEvent::Canceled, TEXT("_Canceled")},
				{ETriggerEvent::Completed, TEXT("_Completed")},
				{ETriggerEvent::Triggered, TEXT("")} // 기본 (접미사 없음)
			};

			for (const auto& EventSuffix : EventSuffixes)
			{
				FString FunctionName = ActionName + EventSuffix.Value;

				if (AlreadyBoundActions.Contains(FunctionName))
				{
					continue;
				}

				UFunction* Function = FindFunction(FName(*FunctionName));
				if (Function)
				{
					EnhancedInputComponent->BindAction(Mapping.Action, EventSuffix.Key, this, FName(*FunctionName));
					AlreadyBoundActions.Add(FunctionName);
				}
			}
		}
	}
}

void AWjWorldCharacterBase::Move(const FInputActionValue& Value)
{
	const FVector2D MoveVector = Value.Get<FVector2D>();
	
	if (Controller == nullptr || MoveVector.Length() <= 0.0f)
	{
		return;
	}
	
	//const FGameplayTag& CameraMode = GamePlayCamera->GetCurrentCameraMode();
	const FGameplayTag CameraMode = FGameplayTag::EmptyTag;

	if (CameraMode == WjWorldGameplayTag::Camera_ThirdPerson())
	{
		// ThirdPerson: 카메라의 Yaw 회전 기준으로 이동
		const FRotator CameraRotation = GamePlayCamera->GetComponentRotation();
		const FRotator YawRotation(0.0f, CameraRotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MoveVector.Y);
		AddMovementInput(RightDirection, MoveVector.X);
	}
	else if (CameraMode == WjWorldGameplayTag::Camera_FirstPerson())
	{
		// FirstPerson: 컴트롤러 회전 기준으로 이동
		const FRotator ControlRotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MoveVector.Y);
		AddMovementInput(RightDirection, MoveVector.X);
	}
	else if (CameraMode == WjWorldGameplayTag::Camera_TopDown())
	{
		// TopDown: 월드 좌표계 기준으로 이동
		// 카메라가 회전해도 WASD는 항상 월드 방향 기준
		const FRotator YawRotation(0.0f, 0.0f, 0.0f); // 월드 좌표계

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); // (1, 0, 0)
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);   // (0, 1, 0)

		AddMovementInput(ForwardDirection, MoveVector.Y); // W/S
		AddMovementInput(RightDirection, MoveVector.X);   // D/A
	}
}

void AWjWorldCharacterBase::Interact(const FInputActionValue& Value)
{
	// Base implementation - override in derived classes
}

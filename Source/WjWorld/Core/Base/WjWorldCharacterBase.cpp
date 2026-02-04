// Fill out your copyright notice in the Description page of Project Settings.

#include "WjWorldCharacterBase.h"
#include "WjWorldPlayerStateBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "Cosmetic/WjWorldCosmeticComponent.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "WjWorldGameplayTag.h"
#include "Core/CameraAsset.h"
#include "Directors/StateTreeCameraDirector.h"

#include "WjWorldLogCategories.h"

AWjWorldCharacterBase::AWjWorldCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Mesh 컴포넌트 위치 설정 (캡슐 컴포넌트 기준)
	if (GetMesh())
	{
		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
		GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	}

	// GamePlay 카메라 컴포넌트 생성
	GamePlayCamera = CreateDefaultSubobject<UGameplayCameraComponent>(TEXT("GamePlayCamera"));
	GamePlayCamera->SetupAttachment(RootComponent);

	// 코스메틱 컴포넌트 생성
	CosmeticComponent = CreateDefaultSubobject<UWjWorldCosmeticComponent>(TEXT("CosmeticComponent"));
}

void AWjWorldCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();

	// SkeletalMesh 설정 (UPROPERTY 우선, 없으면 DeveloperSettings)
	if (GetMesh())
	{
		USkeletalMesh* MeshToUse = nullptr;

		if (!DefaultSkeletalMesh.IsNull())
		{
			MeshToUse = DefaultSkeletalMesh.LoadSynchronous();
		}
		else if (Settings && !Settings->DefaultCharacterMesh.IsNull())
		{
			MeshToUse = Settings->DefaultCharacterMesh.LoadSynchronous();
		}

		if (MeshToUse)
		{
			GetMesh()->SetSkeletalMesh(MeshToUse);
		}

		// AnimBlueprint 설정
		UClass* AnimClassToUse = nullptr;

		if (!DefaultAnimBlueprintClass.IsNull())
		{
			AnimClassToUse = DefaultAnimBlueprintClass.LoadSynchronous();
		}
		else if (Settings && !Settings->DefaultAnimBlueprintClass.IsNull())
		{
			AnimClassToUse = Settings->DefaultAnimBlueprintClass.LoadSynchronous();
		}

		if (AnimClassToUse)
		{
			GetMesh()->SetAnimInstanceClass(AnimClassToUse);
		}
	}
}

UInputMappingContext* AWjWorldCharacterBase::GetDefaultMappingContext() const
{
	// UPROPERTY 우선, 없으면 DeveloperSettings
	if (!DefaultMappingContext.IsNull())
	{
		return DefaultMappingContext.LoadSynchronous();
	}

	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (Settings && !Settings->DefaultInputMappingContext.IsNull())
	{
		return Settings->DefaultInputMappingContext.LoadSynchronous();
	}

	return nullptr;
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
	CameraMode = NewViewMode;
	OnCharacterViewModeChanged.Broadcast(NewViewMode);
}

void AWjWorldCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			UInputMappingContext* IMC = GetDefaultMappingContext();
			if (IMC)
			{
				Subsystem->AddMappingContext(IMC, 0);
			}
		}
	}

	InitializeCharacter();
}

void AWjWorldCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWjWorldCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AWjWorldCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 코스메틱 카탈로그 설정
	if (CosmeticComponent)
	{
		if (UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>())
		{
			CosmeticComponent->SetCatalog(CosmeticSub->GetCatalog());
		}
	}

	// PlayerState에 Pawn 설정 알림 → 대기 중인 코스메틱 적용
	if (AWjWorldPlayerStateBase* PS = GetPlayerState<AWjWorldPlayerStateBase>())
	{
		PS->OnPawnSet(nullptr, this);
	}
}

void AWjWorldCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	SetupInputBindings(PlayerInputComponent);
}

void AWjWorldCharacterBase::InitializeCharacter()
{
	SetCharacterViewMode(StartCameraMode);
}

void AWjWorldCharacterBase::SetupInputBindings(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	const FString AbilityInputAction = TEXT("Ability");

	UInputMappingContext* IMC = GetDefaultMappingContext();

	if (EnhancedInputComponent && IMC)
	{
		TSet<FString> AlreadyBoundActions;
		const TArray<FEnhancedActionKeyMapping>& Mappings = IMC->GetMappings();

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

			if (ActionName.Contains(AbilityInputAction))
			{
				ActionName.RemoveFromStart(AbilityInputAction);
				int32 Param1 = FCString::Atoi(*ActionName);

				EnhancedInputComponent->BindAction(Mapping.Action, ETriggerEvent::Started, this, &AWjWorldCharacterBase::GasInputPressed, Param1);
				EnhancedInputComponent->BindAction(Mapping.Action, ETriggerEvent::Completed, this, &AWjWorldCharacterBase::GasInputReleased, Param1);
			}
			else
			{
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
}

void AWjWorldCharacterBase::GasInputPressed(int32 InputID)
{
	UE_LOG(LogWjWorldAbilities, Log, TEXT("GasInputPressed InputID : %d"), InputID);
}

void AWjWorldCharacterBase::GasInputReleased(int32 InputID)
{
	UE_LOG(LogWjWorldAbilities, Log, TEXT("GasInputReleased InputID : %d"), InputID);
}

void AWjWorldCharacterBase::Move(const FInputActionValue& Value)
{
	const FVector2D MoveVector = Value.Get<FVector2D>();
	
	if (Controller == nullptr || MoveVector.Length() <= 0.0f)
	{
		return;
	}
	
	if (CameraMode == WjWorldGameplayTag::Camera_ThirdPerson())
	{
		APlayerController* PC = GetController<APlayerController>();
		if (PC && PC->PlayerCameraManager)
		{
			//FRotator CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
			FRotator CameraRotation = GamePlayCamera->GetEvaluatedCameraRotation();

			const FRotator YawRotation(0.0f, CameraRotation.Yaw, 0.0f);

			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			AddMovementInput(ForwardDirection, MoveVector.Y);
			AddMovementInput(RightDirection, MoveVector.X);
		}
	}
	else if (CameraMode == WjWorldGameplayTag::Camera_FirstPerson())
	{
		const FRotator ControlRotation = GetActorRotation();
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

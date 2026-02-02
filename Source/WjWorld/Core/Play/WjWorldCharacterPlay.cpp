// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "Core/GameData/ApproachingWallPlayerDataComponent.h"
#include "AbilitySystem/WjWorldAbilitySystemComponent.h"
#include "Cosmetic/WjWorldCosmeticComponent.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "DataAsset/CharacterPlaySetupDataAsset.h"
#include "UI/Ability/AbilityPromptWidget.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

#include "WjWorldLogCategories.h"
#include "WjWorldGameplayTag.h"
#include "WjTypes.h"

AWjWorldCharacterPlay::AWjWorldCharacterPlay()
{
	CosmeticComponent = CreateDefaultSubobject<UWjWorldCosmeticComponent>(TEXT("CosmeticComponent"));

	// 어빌리티 프롬프트 WidgetComponent (캐릭터 머리 위에 Screen 공간으로 표시)
	AbilityPromptComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("AbilityPromptComponent"));
	AbilityPromptComponent->SetupAttachment(RootComponent);
	AbilityPromptComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	AbilityPromptComponent->SetWidgetSpace(EWidgetSpace::Screen);
	AbilityPromptComponent->SetDrawAtDesiredSize(true);
	AbilityPromptComponent->SetVisibility(false);
}

void AWjWorldCharacterPlay::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

UAbilitySystemComponent* AWjWorldCharacterPlay::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

UWjWorldAbilitySystemComponent* AWjWorldCharacterPlay::GetWJAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

void AWjWorldCharacterPlay::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWjWorldCharacterPlay, bIsEliminated);
}

void AWjWorldCharacterPlay::OnEliminated()
{
	if (bIsEliminated)
	{
		return; // 이미 사망한 경우 무시
	}

	bIsEliminated = true;

	// ASC에 State.Eliminated 태그 추가 → 어빌리티 활성화 차단
	if (AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->AddLooseGameplayTag(WjWorldGameplayTag::State_Eliminated());
		AbilitySystemComponent->CancelAllAbilities();
	}

	// 서버에서 PlayerState의 데이터 컴포넌트도 업데이트
	if (HasAuthority())
	{
		if (AWjWorldPlayerStatePlay* PS = GetPlayerState<AWjWorldPlayerStatePlay>())
		{
			if (UApproachingWallPlayerDataComponent* PlayerData = PS->GetGameData<UApproachingWallPlayerDataComponent>())
			{
				PlayerData->OnEliminated();
			}
		}
	}

	HandleEliminationEffects();
}

void AWjWorldCharacterPlay::OnRep_IsEliminated()
{
	if (bIsEliminated)
	{
		HandleEliminationEffects();
	}
}

void AWjWorldCharacterPlay::HandleEliminationEffects()
{
	// 움직임 비활성화
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->DisableMovement();
		MovementComp->StopMovementImmediately();
	}

	// 입력 비활성화
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}

	// Temp
	if (GetWorld())
	{
		FTimerHandle DestoryHandle;
		GetWorld()->GetTimerManager().SetTimer(DestoryHandle, [this]()
		{
			Destroy();
		}, 1.0f, false);
	}
	// TODO: 사망 이펙트, 애니메이션 등 추가 가능
}

void AWjWorldCharacterPlay::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	APlayerState* WJPlayerState = GetPlayerState<AWjWorldPlayerStatePlay>();
	IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(WJPlayerState);
	if (AbilitySystemInterface)
	{
		AbilitySystemComponent = Cast<UWjWorldAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent());
		if (AbilitySystemComponent.IsValid())
		{
			// 클라이언트에서도 ActorInfo 초기화 필요 (어빌리티 입력 처리를 위해)
			AbilitySystemComponent->InitAbilityActorInfo(WJPlayerState, this);

			// Confirm/Cancel InputID 설정
			AbilitySystemComponent->GenericConfirmInputID = static_cast<int32>(EWjWorldAbilityInputID::Confirm);
			AbilitySystemComponent->GenericCancelInputID = static_cast<int32>(EWjWorldAbilityInputID::Cancel);

			// 클라이언트 공통 초기화 (어빌리티 부여는 Initialize 내부에서 Authority 체크)
			FLoadSoftObjectPathAsyncDelegate SetupDataLoadDel;
			SetupDataLoadDel.BindUObject(this, &ThisClass::OnSetupDataAssetLoaded);
			SetupDataAsset.LoadAsync(SetupDataLoadDel);
		}
	}
}

void AWjWorldCharacterPlay::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	APlayerState* WJPlayerState = GetPlayerState<AWjWorldPlayerStatePlay>();
	IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(WJPlayerState);
	if (AbilitySystemInterface)
	{
		AbilitySystemComponent = Cast<UWjWorldAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent());
		check(AbilitySystemComponent.IsValid());
		AbilitySystemComponent->InitAbilityActorInfo(WJPlayerState, this);
		AbilitySystemComponent->SetOwnerActor(NewController);

		// Confirm/Cancel InputID 설정
		AbilitySystemComponent->GenericConfirmInputID = static_cast<int32>(EWjWorldAbilityInputID::Confirm);
		AbilitySystemComponent->GenericCancelInputID = static_cast<int32>(EWjWorldAbilityInputID::Cancel);

		FLoadSoftObjectPathAsyncDelegate SetupDataLoadDel;
		SetupDataLoadDel.BindUObject(this, &ThisClass::OnSetupDataAssetLoaded);
		SetupDataAsset.LoadAsync(SetupDataLoadDel);
	}

	// 코스메틱 로드아웃 설정 (서버에서)
	if (HasAuthority())
	{
		if (UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>())
		{
			if (CosmeticComponent)
			{
				CosmeticComponent->SetCatalog(CosmeticSub->GetCatalog());
			}

			if (AWjWorldPlayerStatePlay* PS = GetPlayerState<AWjWorldPlayerStatePlay>())
			{
				PS->SetCosmeticLoadout(CosmeticSub->GetLoadout());
			}
		}
	}
}

void AWjWorldCharacterPlay::OnSetupDataAssetLoaded(const FSoftObjectPath& Path, UObject* Object)
{
	UE_LOG(LogWjWorld, Log, TEXT("AWjWorldCharacterPlay::OnSetupDataAssetLoaded"));
	if (!::IsValid(this)) return;

	UCharacterPlaySetupDataAsset* SetupDA = Cast<UCharacterPlaySetupDataAsset>(Object);
	if (SetupDA)
	{
		FString ErrorMsg;
		SetupDA->Initialize(this, ErrorMsg);

		if (ErrorMsg.Len() > 0)
		{
			ensureMsgf(false, TEXT("ErrorMsg : %s"), *ErrorMsg);
		}
		else
		{
			UE_LOG(LogWjWorld, Log, TEXT("SetupDataAsset->Initialize Success"));
		}
	}
}

void AWjWorldCharacterPlay::GasInputPressed(int32 InputID)
{
	Super::GasInputPressed(InputID);
	if (AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->PressInputID(InputID);

		// Confirm/Cancel 입력 처리 (WaitConfirmCancel Task용)
		if (InputID == AbilitySystemComponent->GenericConfirmInputID)
		{
			AbilitySystemComponent->LocalInputConfirm();
		}
		else if (InputID == AbilitySystemComponent->GenericCancelInputID)
		{
			AbilitySystemComponent->LocalInputCancel();
		}
	}
}

void AWjWorldCharacterPlay::GasInputReleased(int32 InputID)
{
	Super::GasInputReleased(InputID);
	if (AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->ReleaseInputID(InputID);
	}
}

void AWjWorldCharacterPlay::ShowAbilityPrompt(const FText& ConfirmKeyName, const FText& CancelKeyName, const FText& Description)
{
	if (!AbilityPromptComponent)
	{
		return;
	}

	AbilityPromptComponent->SetVisibility(true);

	UAbilityPromptWidget* PromptWidget = Cast<UAbilityPromptWidget>(AbilityPromptComponent->GetWidget());
	if (PromptWidget)
	{
		PromptWidget->SetPromptInfo(ConfirmKeyName, CancelKeyName, Description);
	}
}

void AWjWorldCharacterPlay::HideAbilityPrompt()
{
	if (AbilityPromptComponent)
	{
		AbilityPromptComponent->SetVisibility(false);
	}
}

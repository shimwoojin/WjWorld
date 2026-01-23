// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "Core/GameData/ApproachingWallPlayerDataComponent.h"
#include "AbilitySystem/WjWorldAbilitySystemComponent.h"
#include "DataAsset/CharacterPlaySetupDataAsset.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

#include "WjWorldLogCategories.h"
#include "WjTypes.h"

AWjWorldCharacterPlay::AWjWorldCharacterPlay()
{

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
	DOREPLIFETIME(AWjWorldCharacterPlay, DeadStackCount);
}

void AWjWorldCharacterPlay::OnEliminated()
{
	if (bIsEliminated)
	{
		return; // 이미 사망한 경우 무시
	}

	bIsEliminated = true;

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

void AWjWorldCharacterPlay::AddDeadStackCount(int32 InCount)
{
	DeadStackCount += InCount;

	if (HasAuthority())
	{
		OnRep_DeadStackCountChanged();
	}
}

void AWjWorldCharacterPlay::RemoveDeadStackCount(int32 InCount)
{
	DeadStackCount = FMath::Max(0, DeadStackCount - InCount);

	if (HasAuthority())
	{
		OnRep_DeadStackCountChanged();
	}
}

void AWjWorldCharacterPlay::OnRep_IsEliminated()
{
	if (bIsEliminated)
	{
		HandleEliminationEffects();
	}
}

void AWjWorldCharacterPlay::OnRep_DeadStackCountChanged()
{
	OnDeadStackCountChanged.Broadcast(DeadStackCount);
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

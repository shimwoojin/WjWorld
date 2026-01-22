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

AWjWorldCharacterPlay::AWjWorldCharacterPlay()
{
	// 히트박스 컴포넌트 생성
	HitBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxComponent"));
	HitBoxComponent->SetupAttachment(GetCapsuleComponent());
	HitBoxComponent->SetBoxExtent(FVector(40.0f, 40.0f, 88.0f)); // 캡슐 크기에 맞게 조정
	HitBoxComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	HitBoxComponent->SetGenerateOverlapEvents(true);
}

void AWjWorldCharacterPlay::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (SetupDataAsset)
	{
		FString ErrorMsg;
		SetupDataAsset->Initialize(this, ErrorMsg);
		ensureMsgf(ErrorMsg.Len() == 0, TEXT("ErrorMsg : %s") ,*ErrorMsg);
	}
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

	// 충돌 비활성화 (다른 벽돌과 더 이상 충돌하지 않도록)
	if (HitBoxComponent)
	{
		HitBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	}
}
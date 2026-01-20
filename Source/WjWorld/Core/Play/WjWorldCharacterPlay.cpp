// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "AbilitySystem/WjWorldAbilitySystemComponent.h"
#include "DataAsset/CharacterPlaySetupDataAsset.h"

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

	//DOREPLIFETIME(AWjWorldCharacterPlay, AbilitySystemComponent);
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
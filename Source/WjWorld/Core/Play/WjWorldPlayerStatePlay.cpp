// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "AbilitySystem/WjWorldAbilitySystemComponent.h"

AWjWorldPlayerStatePlay::AWjWorldPlayerStatePlay()
{
	AbilitySystemComponent = CreateDefaultSubobject<UWjWorldAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
}

UAbilitySystemComponent* AWjWorldPlayerStatePlay::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UWjWorldAbilitySystemComponent* AWjWorldPlayerStatePlay::GetWJAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AWjWorldPlayerStatePlay::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AWjWorldPlayerStatePlay, AttributeSet);
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "Core/GameData/WjWorldGameDataComponent.h"
#include "AbilitySystem/WjWorldAbilitySystemComponent.h"

AWjWorldPlayerStatePlay::AWjWorldPlayerStatePlay()
{
	AbilitySystemComponent = CreateDefaultSubobject<UWjWorldAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	bReplicates = true;
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

void AWjWorldPlayerStatePlay::AddGameDataComponent(TSubclassOf<UWjWorldGameDataComponent> InDataComponentClass)
{
	if (InDataComponentClass)
	{
		PlayerDataComponent = NewObject<UWjWorldGameDataComponent>(this, InDataComponentClass);
		AddInstanceComponent(PlayerDataComponent);
		PlayerDataComponent->RegisterComponent();
		PlayerDataComponent->SetIsReplicated(true);
	}
}
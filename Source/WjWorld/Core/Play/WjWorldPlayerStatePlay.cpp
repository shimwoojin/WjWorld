// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "Core/Play/WjWorldHUDPlay.h"
#include "Core/GameData/WjWorldGameDataComponent.h"
#include "AbilitySystem/WjWorldAbilitySystemComponent.h"
#include "Cosmetic/WjWorldCosmeticComponent.h"

#include "Net/UnrealNetwork.h"

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

	DOREPLIFETIME(AWjWorldPlayerStatePlay, CosmeticLoadout);
}

void AWjWorldPlayerStatePlay::SetCosmeticLoadout(const FCosmeticLoadout& InLoadout)
{
	if (!HasAuthority())
	{
		return;
	}

	CosmeticLoadout = InLoadout;

	// 서버에서도 즉시 적용
	OnRep_CosmeticLoadout();
}

void AWjWorldPlayerStatePlay::OnRep_CosmeticLoadout()
{
	// 소유 Pawn의 CosmeticComponent에 로드아웃 적용
	if (APawn* OwnerPawn = GetPawn())
	{
		if (UWjWorldCosmeticComponent* CosmeticComp = OwnerPawn->FindComponentByClass<UWjWorldCosmeticComponent>())
		{
			CosmeticComp->ApplyLoadout(CosmeticLoadout);
		}
	}
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

// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSets/WjWorldCharacterAttributeSet.h"
#include "Net/UnrealNetwork.h"

void UWjWorldCharacterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UWjWorldCharacterAttributeSet, MaxSpawnBrickCharges, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWjWorldCharacterAttributeSet, SpawnBrickCharges, COND_None, REPNOTIFY_Always);
}

void UWjWorldCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// SpawnBrickCharges를 [0, MaxSpawnBrickCharges] 범위로 클램프
	if (Attribute == GetSpawnBrickChargesAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxSpawnBrickCharges());
	}
}

void UWjWorldCharacterAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UWjWorldCharacterAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UWjWorldCharacterAttributeSet::PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const
{
	Super::PostAttributeBaseChange(Attribute, OldValue, NewValue);
}

bool UWjWorldCharacterAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	return Super::PreGameplayEffectExecute(Data);
}

void UWjWorldCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UWjWorldCharacterAttributeSet::OnRep_MaxHP(const FGameplayAttributeData& OldMaxHP)
{
}

void UWjWorldCharacterAttributeSet::OnRep_HP(const FGameplayAttributeData& OldHP)
{
}

void UWjWorldCharacterAttributeSet::OnRep_MaxSpawnBrickCharges(const FGameplayAttributeData& OldMaxSpawnBrickCharges)
{
}

void UWjWorldCharacterAttributeSet::OnRep_SpawnBrickCharges(const FGameplayAttributeData& OldSpawnBrickCharges)
{
}

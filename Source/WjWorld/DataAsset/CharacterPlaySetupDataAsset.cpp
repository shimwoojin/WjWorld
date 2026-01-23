// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/CharacterPlaySetupDataAsset.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "AbilitySystemComponent.h"

void UCharacterPlaySetupDataAsset::Initialize(AWjWorldCharacterPlay* TargetCharacter, FString& OutErrorMsg)
{
	if (TargetCharacter == nullptr)
	{
		OutErrorMsg = TEXT("TargetCharacter is null");
		return;
	}

	// 서버 전용: 어빌리티 부여 (클라이언트로 리플리케이트됨)
	if (TargetCharacter->HasAuthority())
	{
		InitializeAbilities(TargetCharacter);
	}

	// 서버/클라이언트 공통 초기화
	InitializeCommon(TargetCharacter);

	if (OutErrorMsg.Len() == 0)
	{
		TargetCharacter->bSetupDataAssetCall = true;
		TargetCharacter->bSetupDataAssetFinal = true;
	}
}

void UCharacterPlaySetupDataAsset::InitializeAbilities(AWjWorldCharacterPlay* TargetCharacter)
{
	UAbilitySystemComponent* AbilitySystemComponent = TargetCharacter->GetAbilitySystemComponent();
	if (AbilitySystemComponent == nullptr) return;

	for (const auto& StartInputAbility : StartInputAbilities)
	{
		FGameplayAbilitySpec Spec(StartInputAbility.Value);
		Spec.InputID = static_cast<int32>(StartInputAbility.Key);
		AbilitySystemComponent->GiveAbility(Spec);
	}

	for (const auto& StartAbility : StartAbilities)
	{
		FGameplayAbilitySpec Spec(StartAbility);
		AbilitySystemComponent->GiveAbility(Spec);
	}
}

void UCharacterPlaySetupDataAsset::InitializeCommon(AWjWorldCharacterPlay* TargetCharacter)
{
	// 서버/클라이언트 공통 초기화 로직 (향후 확장용)
}

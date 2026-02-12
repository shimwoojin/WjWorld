// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_ToggleCameraView.h"

#include "WjWorldGameplayTag.h"
#include "Core/Base/WjWorldCharacterBase.h"

UGA_ToggleCameraView::UGA_ToggleCameraView()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(WjWorldGameplayTag::Ability_ToggleCameraView());
	SetAssetTags(AssetTags);

	// UI 메타데이터
	AbilityName = NSLOCTEXT("Abilities", "ToggleCameraView", "카메라 전환");
}

void UGA_ToggleCameraView::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		AWjWorldCharacterBase* Character = Cast<AWjWorldCharacterBase>(ActorInfo->AvatarActor.Get());
		if (Character)
		{
			if (Character->GetCameraMode() == WjWorldGameplayTag::Camera_TopDown())
			{
				Character->SetCharacterViewMode(ECharacterCameraMode::ThirdPerson);
			}
			else
			{
				Character->SetCharacterViewMode(ECharacterCameraMode::TopDown);
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

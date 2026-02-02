// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WjWorldGameplayAbilityBase.h"

#include "AbilitySystem/Effects/GE_AbilityCooldown.h"
#include "WjWorldGameplayTag.h"

UWjWorldGameplayAbilityBase::UWjWorldGameplayAbilityBase()
{
	// 제거 상태에서는 어빌리티 활성화 차단
	ActivationBlockedTags.AddTag(WjWorldGameplayTag::State_Eliminated());
}

void UWjWorldGameplayAbilityBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (CooldownDuration <= 0.f || CooldownTags.Num() == 0)
	{
		return;
	}

	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (!CooldownGE)
	{
		// GE_AbilityCooldown CDO 사용
		CooldownGE = UGE_AbilityCooldown::StaticClass()->GetDefaultObject<UGameplayEffect>();
	}

	if (CooldownGE)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			// SetByCaller로 Duration 설정
			SpecHandle.Data->SetSetByCallerMagnitude(WjWorldGameplayTag::Data_Cooldown(), CooldownDuration);

			// 쿨다운 태그를 DynamicGrantedTags에 추가
			SpecHandle.Data->DynamicGrantedTags.AppendTags(CooldownTags);

			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}
}

const FGameplayTagContainer* UWjWorldGameplayAbilityBase::GetCooldownTags() const
{
	// 부모 태그와 CooldownTags 합산
	MergedCooldownTags.Reset();

	const FGameplayTagContainer* ParentTags = Super::GetCooldownTags();
	if (ParentTags)
	{
		MergedCooldownTags.AppendTags(*ParentTags);
	}

	MergedCooldownTags.AppendTags(CooldownTags);

	return &MergedCooldownTags;
}

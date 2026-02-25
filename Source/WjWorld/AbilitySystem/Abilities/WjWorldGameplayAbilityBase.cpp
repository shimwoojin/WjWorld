// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/WjWorldGameplayAbilityBase.h"

#include "AbilitySystem/Effects/GE_AbilityCooldown.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "WjWorldGameplayTag.h"

UWjWorldGameplayAbilityBase::UWjWorldGameplayAbilityBase()
{
	// 제거 상태에서는 어빌리티 활성화 차단
	ActivationBlockedTags.AddTag(WjWorldGameplayTag::State_Eliminated());

	// 경직 상태에서는 어빌리티 활성화 차단
	ActivationBlockedTags.AddTag(WjWorldGameplayTag::State_Staggered());
}

bool UWjWorldGameplayAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// GameStatePlay 기반 체크 (GamePhase + AllowedAbilityTags)
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		UWorld* World = ActorInfo->AvatarActor->GetWorld();
		if (World)
		{
			AWjWorldGameStatePlay* GameState = World->GetGameState<AWjWorldGameStatePlay>();
			if (GameState)
			{
				// 게임 시작 전/후에는 어빌리티 사용 금지 (Playing 상태에서만 허용)
				if (GameState->GetGamePhase() != EGamePhase::Playing)
				{
					return false;
				}

				// AllowedAbilityTags 체크 (빈 컨테이너 = 전부 허용, 하위 호환)
				const FGameplayTagContainer& AllowedTags = GameState->GetAllowedAbilityTags();
				if (AllowedTags.Num() > 0)
				{
					// 이 어빌리티의 AssetTag가 AllowedTags에 포함되어야 활성화
					const FGameplayTagContainer& AssetTags = GetAssetTags();
					if (!AllowedTags.HasAny(AssetTags))
					{
						return false;
					}
				}
			}
		}
	}

	return true;
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

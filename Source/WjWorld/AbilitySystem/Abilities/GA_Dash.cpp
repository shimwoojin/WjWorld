// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_Dash.h"

#include "WjWorldGameplayTag.h"
#include "WjWorldLogCategories.h"

#include "GameFramework/Character.h"

UGA_Dash::UGA_Dash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(WjWorldGameplayTag::Ability_Dash());
	SetAssetTags(AssetTags);

	// SpawnBrickPreview 또는 LiftBrickCarry 상태에서는 활성화 불가
	ActivationBlockedTags.AddTag(WjWorldGameplayTag::State_SpawnBrickPreview());
	ActivationBlockedTags.AddTag(WjWorldGameplayTag::State_LiftBrickCarry());

	// 쿨다운 설정
	CooldownDuration = DashCooldownDuration;
	CooldownTags.AddTag(WjWorldGameplayTag::Cooldown_Dash());

	// UI 메타데이터
	AbilityName = NSLOCTEXT("Abilities", "Dash", "대시");
}

void UGA_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// EndAbility 호출용 캐시
	CachedHandle = Handle;
	CachedActorInfo = ActorInfo;
	CachedActivationInfo = ActivationInfo;

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
		if (!Character)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		// 전방으로 대시 (LaunchCharacter)
		FVector ForwardDir = Character->GetActorForwardVector();
		FVector LaunchVelocity = ForwardDir * (DashDistance / DashDuration);
		Character->LaunchCharacter(LaunchVelocity, true, true);

		UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_Dash: Launched %s forward with velocity %s"),
			*Character->GetName(), *LaunchVelocity.ToString());

		// 대시 종료 타이머
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(DashTimerHandle, this, &UGA_Dash::OnDashEnd, DashDuration, false);
		}
	}
}

void UGA_Dash::OnDashEnd()
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

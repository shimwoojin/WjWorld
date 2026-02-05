// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_Push.h"

#include "WjWorldGameplayTag.h"
#include "WjWorldLogCategories.h"

#include "Core/Play/WjWorldCharacterPlay.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Engine/OverlapResult.h"

UGA_Push::UGA_Push()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(WjWorldGameplayTag::Ability_Push());
	SetAssetTags(AssetTags);

	// SpawnBrickPreview 또는 LiftBrickCarry 상태에서는 활성화 불가
	ActivationBlockedTags.AddTag(WjWorldGameplayTag::State_SpawnBrickPreview());
	ActivationBlockedTags.AddTag(WjWorldGameplayTag::State_LiftBrickCarry());

	// 쿨다운 설정
	CooldownDuration = 1.5f;
	CooldownTags.AddTag(WjWorldGameplayTag::Cooldown_Push());

	// UI 메타데이터
	AbilityName = NSLOCTEXT("Abilities", "Push", "밀치기");
}

void UGA_Push::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// EndAbility 호출용 캐시
	CachedHandle = Handle;
	CachedActorInfo = ActorInfo;
	CachedActivationInfo = ActivationInfo;

	// GameplayCue 실행 (사운드/VFX)
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = GetAvatarActorFromActorInfo()->GetActorLocation();
		ASC->ExecuteGameplayCue(WjWorldGameplayTag::GameplayCue_Ability_Push(), CueParams);
	}

	// 밀치기 로직 즉시 실행
	ExecutePush(ActivationInfo);

	// 쿨다운 적용
	ApplyCooldown(Handle, ActorInfo, ActivationInfo);

	// Montage가 있으면 재생, 없으면 즉시 종료
	if (PushMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			PushMontage,
			MontagePlayRate,
			NAME_None,
			true,
			1.0f
		);

		MontageTask->OnCompleted.AddDynamic(this, &UGA_Push::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_Push::OnMontageBlendOut);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_Push::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_Push::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_Push::ExecutePush(const FGameplayAbilityActivationInfo& ActivationInfo)
{
	// 서버에서만 처리
	if (!HasAuthority(&ActivationInfo))
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor) return;

	AWjWorldCharacterPlay* OwnerCharacter = Cast<AWjWorldCharacterPlay>(AvatarActor);
	if (!OwnerCharacter) return;

	FVector Origin = AvatarActor->GetActorLocation();
	FVector ForwardDir = AvatarActor->GetActorForwardVector();

	// 전방 오버랩 중심 (캐릭터 전방에 약간 오프셋)
	FVector OverlapCenter = Origin + ForwardDir * (PushRange * 0.5f);

	TArray<FOverlapResult> Overlaps;
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(PushRange);

	if (GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		OverlapCenter,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
		CollisionShape))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AWjWorldCharacterPlay* HitCharacter = Cast<AWjWorldCharacterPlay>(Overlap.GetActor());
			if (!HitCharacter || HitCharacter == OwnerCharacter) continue;
			if (HitCharacter->IsEliminated()) continue;

			// 넉백 방향 계산: 공격자 → 피격자
			FVector PushDir = (HitCharacter->GetActorLocation() - Origin).GetSafeNormal2D();
			if (PushDir.IsNearlyZero())
			{
				PushDir = ForwardDir;
			}

			FVector LaunchVelocity = PushDir * PushForce + FVector(0.f, 0.f, PushUpForce);
			HitCharacter->LaunchCharacter(LaunchVelocity, true, true);

			// 킬 추적용 마지막 공격자 설정
			HitCharacter->SetLastAttacker(OwnerCharacter);

			UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_Push: Pushed %s with force (%s)"),
				*HitCharacter->GetName(), *LaunchVelocity.ToString());
		}
	}
}

void UGA_Push::OnMontageCompleted()
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

void UGA_Push::OnMontageBlendOut()
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

void UGA_Push::OnMontageInterrupted()
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, true);
}

void UGA_Push::OnMontageCancelled()
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, true);
}

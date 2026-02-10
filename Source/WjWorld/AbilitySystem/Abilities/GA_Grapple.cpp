// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_Grapple.h"

#include "WjWorldGameplayTag.h"
#include "WjWorldLogCategories.h"

#include "GameFramework/Character.h"
#include "GamePlay/JumpMap/JumpMapGrapplePointActor.h"

UGA_Grapple::UGA_Grapple()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(WjWorldGameplayTag::Ability_Grapple());
	SetAssetTags(AssetTags);

	// SpawnBrickPreview 또는 LiftBrickCarry 상태에서는 활성화 불가
	ActivationBlockedTags.AddTag(WjWorldGameplayTag::State_SpawnBrickPreview());
	ActivationBlockedTags.AddTag(WjWorldGameplayTag::State_LiftBrickCarry());

	// 쿨다운 설정
	CooldownDuration = GrappleCooldownDuration;
	CooldownTags.AddTag(WjWorldGameplayTag::Cooldown_Grapple());

	// UI 메타데이터
	AbilityName = NSLOCTEXT("Abilities", "Grapple", "그래플");
}

void UGA_Grapple::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// EndAbility 호출용 캐시
	CachedHandle = Handle;
	CachedActorInfo = ActorInfo;
	CachedActivationInfo = ActivationInfo;

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 카메라 방향으로 라인 트레이스
	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	FVector TraceEnd = CamLoc + CamRot.Vector() * GrappleRange;
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, CamLoc, TraceEnd, ECC_Visibility, Params))
	{
		AJumpMapGrapplePointActor* GrapplePoint = Cast<AJumpMapGrapplePointActor>(HitResult.GetActor());
		if (GrapplePoint)
		{
			if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
			{
				EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
				return;
			}

			GrappleTargetLocation = GrapplePoint->GetActorLocation();
			bIsPulling = true;

			// 캐릭터를 타겟 방향으로 발사
			FVector Direction = (GrappleTargetLocation - Character->GetActorLocation()).GetSafeNormal();
			Character->LaunchCharacter(Direction * GrapplePullSpeed, true, true);

			UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_Grapple: Pulling %s toward %s"),
				*Character->GetName(), *GrappleTargetLocation.ToString());

			// 도착 체크 타이머 (매 틱)
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UGA_Grapple::CheckArrival));
			}
			return;
		}
	}

	// 미스 → 쿨다운 없이 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_Grapple::CheckArrival()
{
	if (!bIsPulling)
	{
		return;
	}

	if (!CachedActorInfo || !CachedActorInfo->AvatarActor.IsValid())
	{
		bIsPulling = false;
		EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, true);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(CachedActorInfo->AvatarActor.Get());
	if (!Character)
	{
		bIsPulling = false;
		EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, true);
		return;
	}

	float Distance = FVector::Dist(Character->GetActorLocation(), GrappleTargetLocation);
	if (Distance <= ArrivalThreshold)
	{
		bIsPulling = false;
		EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
		return;
	}

	// 아직 이동 중 → 다음 틱에 다시 체크
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UGA_Grapple::CheckArrival));
	}
}

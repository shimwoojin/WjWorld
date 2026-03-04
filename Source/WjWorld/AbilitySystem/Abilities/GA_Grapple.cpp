// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_Grapple.h"

#include "WjWorldGameplayTag.h"
#include "WjWorldLogCategories.h"

#include "EngineUtils.h"
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

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
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

		// 카메라 방향 정보
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);
		FVector CamDir = CamRot.Vector();

		// 범위 내 + 카메라 방향에 가장 가까운 GrapplePoint 탐색
		AJumpMapGrapplePointActor* BestTarget = nullptr;
		float BestDot = 0.3f; // ~72도 원뿔 최소치
		FVector CharLoc = Character->GetActorLocation();

		for (TActorIterator<AJumpMapGrapplePointActor> It(GetWorld()); It; ++It)
		{
			AJumpMapGrapplePointActor* GP = *It;
			if (!GP->IsInRange(CharLoc))
			{
				continue;
			}

			FVector ToTarget = (GP->GetGrappleTargetLocation() - CamLoc).GetSafeNormal();
			float Dot = FVector::DotProduct(CamDir, ToTarget);
			if (Dot > BestDot)
			{
				BestDot = Dot;
				BestTarget = GP;
			}
		}

		if (BestTarget)
		{
			if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
			{
				EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
				return;
			}

			GrappleTargetLocation = BestTarget->GetGrappleTargetLocation();
			bIsPulling = true;
			PullElapsedTime = 0.f;

			// 캐릭터를 타겟 방향으로 발사
			FVector Direction = (GrappleTargetLocation - Character->GetActorLocation()).GetSafeNormal();
			Character->LaunchCharacter(Direction * GrapplePullSpeed, true, true);

			UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_Grapple: Pulling %s toward %s (dot=%.2f)"),
				*Character->GetName(), *GrappleTargetLocation.ToString(), BestDot);

			// 도착 체크 타이머 (매 틱)
			if (UWorld* World = GetWorld())
			{
				ArrivalCheckTimerHandle = World->GetTimerManager().SetTimerForNextTick(
					FTimerDelegate::CreateUObject(this, &UGA_Grapple::CheckArrival));
			}
			return;
		}

		// 미스 → 쿨다운 없이 종료
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_Grapple::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	bIsPulling = false;

	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ArrivalCheckTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
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

	// 타임아웃 체크 (무한 풀 방지)
	PullElapsedTime += GetWorld()->GetDeltaSeconds();
	if (PullElapsedTime >= MaxPullDuration)
	{
		UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_Grapple: Pull timed out after %.1fs"), PullElapsedTime);
		bIsPulling = false;
		EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
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
		ArrivalCheckTimerHandle = World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UGA_Grapple::CheckArrival));
	}
}

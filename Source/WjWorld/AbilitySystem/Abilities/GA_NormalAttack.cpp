// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_NormalAttack.h"

#include "WjWorldGameplayTag.h"
#include "WjWorldLogCategories.h"

#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"

#include "GamePlay/Wall/WjWorldBrickSpawner.h"
#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/Wall/WjWorldBrickActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Effects/GE_NormalAttackStagger.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"

UGA_NormalAttack::UGA_NormalAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(WjWorldGameplayTag::Ability_NormalAttack());
	SetAssetTags(AssetTags);

	// SpawnBrickPreview 또는 LiftBrickCarry 상태에서는 활성화 불가
	ActivationBlockedTags.AddTag(WjWorldGameplayTag::State_SpawnBrickPreview());
	ActivationBlockedTags.AddTag(WjWorldGameplayTag::State_LiftBrickCarry());

	// 쿨다운 설정
	CooldownDuration = 1.0f;
	CooldownTags.AddTag(WjWorldGameplayTag::Cooldown_NormalAttack());

	// UI 메타데이터
	AbilityName = NSLOCTEXT("Abilities", "NormalAttack", "공격");
}

void UGA_NormalAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// EndAbility 호출용 캐시
	CachedHandle = Handle;
	CachedActorInfo = ActorInfo;
	CachedActivationInfo = ActivationInfo;

	// WallDesc 캐시
	UWorld* World = GetWorld();
	AWjWorldGameModePlay* GameModePlay = World ? World->GetAuthGameMode<AWjWorldGameModePlay>() : nullptr;
	if (GameModePlay)
	{
		UWjWorldGameRuleApproachingWall* GameRule = GameModePlay->GetCurrentGameRule<UWjWorldGameRuleApproachingWall>();
		if (GameRule)
		{
			CachedWallDesc = GameRule->GetWallDesc();
		}
	}

	// GameplayCue 실행 (사운드/VFX)
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = GetAvatarActorFromActorInfo()->GetActorLocation();
		ASC->ExecuteGameplayCue(WjWorldGameplayTag::GameplayCue_Ability_NormalAttack(), CueParams);
	}

	// 공격 로직 즉시 실행 (Montage와 별개로)
	ExecuteAttack(ActivationInfo);

	// 쿨다운 적용
	ApplyCooldown(Handle, ActorInfo, ActivationInfo);

	// Montage가 있으면 재생, 없으면 즉시 종료
	if (AttackMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			AttackMontage,
			MontagePlayRate,
			NAME_None,
			true,  // bStopWhenAbilityEnds
			1.0f   // AnimRootMotionTranslationScale
		);

		MontageTask->OnCompleted.AddDynamic(this, &UGA_NormalAttack::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_NormalAttack::OnMontageBlendOut);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_NormalAttack::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_NormalAttack::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
	else
	{
		// Montage 없으면 즉시 종료
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_NormalAttack::ExecuteAttack(const FGameplayAbilityActivationInfo& ActivationInfo)
{
	// 서버에서만 공격 처리
	if (!HasAuthority(&ActivationInfo))
	{
		return;
	}

	FVector TargetLocation = CalculateTargetLocation();
	FVector HalfSize = CachedWallDesc.BrickSize * 0.3f;

	// BrickSize가 0이면 플레이어 감지용 기본 범위 사용
	if (HalfSize.IsNearlyZero())
	{
		HalfSize = FVector(75.f, 75.f, 90.f);
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionShape CollisionShape = FCollisionShape::MakeBox(HalfSize);

	if (GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		TargetLocation,
		FQuat::Identity,
		FCollisionObjectQueryParams::AllObjects,
		CollisionShape))
	{
		AActor* AvatarActor = GetAvatarActorFromActorInfo();

		for (const FOverlapResult& Overlap : Overlaps)
		{
			// 벽돌 감지
			AWjWorldBrickActor* BrickActor = Cast<AWjWorldBrickActor>(Overlap.GetActor());
			if (BrickActor)
			{
				UWjWorldBrickComponent* BrickComp = BrickActor->GetBrickComponent();
				if (!BrickComp) continue;

				const FWjWorldBrickProperties& Props = BrickComp->GetBrickProperties();

				switch (Props.BrickType)
				{
				case EWjWorldBrickType::Standard:
					UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_NormalAttack: Standard brick - cannot destroy"));
					break;

				case EWjWorldBrickType::Explosive:
					UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_NormalAttack: Explosive brick - HandleWallCollision"));
					BrickComp->HandleWallCollision(FVector::ZeroVector);
					break;

				case EWjWorldBrickType::Moving:
					break;

				case EWjWorldBrickType::Destructible:
					UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_NormalAttack: Destructible brick - applying damage"));
					BrickComp->ApplyDamage(1);
					break;
				}

				// 첫 번째 벽돌만 처리
				break;
			}

			// 플레이어 감지 — 경직 적용
			AWjWorldCharacterPlay* HitCharacter = Cast<AWjWorldCharacterPlay>(Overlap.GetActor());
			if (HitCharacter && HitCharacter != AvatarActor && !HitCharacter->IsEliminated())
			{
				UAbilitySystemComponent* TargetASC = HitCharacter->GetAbilitySystemComponent();
				if (TargetASC && !TargetASC->HasMatchingGameplayTag(WjWorldGameplayTag::State_Staggered()))
				{
					// 경직 GE 적용
					UClass* StaggerGEClass = StaggerEffectClass ? StaggerEffectClass.Get() : UGE_NormalAttackStagger::StaticClass();
					FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
					ContextHandle.AddSourceObject(AvatarActor);
					FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(StaggerGEClass, 1.f, ContextHandle);
					if (SpecHandle.IsValid())
					{
						TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					}

					// 경직 태그 + 이동 차단
					TargetASC->AddLooseGameplayTag(WjWorldGameplayTag::State_Staggered());

					if (UCharacterMovementComponent* MovementComp = HitCharacter->GetCharacterMovement())
					{
						MovementComp->DisableMovement();
						MovementComp->StopMovementImmediately();
					}

					// 1초 후 경직 해제
					FTimerHandle StaggerTimerHandle;
					TWeakObjectPtr<AWjWorldCharacterPlay> WeakHitChar = HitCharacter;
					GetWorld()->GetTimerManager().SetTimer(StaggerTimerHandle, [WeakHitChar]()
					{
						AWjWorldCharacterPlay* Char = WeakHitChar.Get();
						if (!Char || Char->IsEliminated()) return;

						UAbilitySystemComponent* ASC = Char->GetAbilitySystemComponent();
						if (ASC)
						{
							ASC->RemoveLooseGameplayTag(WjWorldGameplayTag::State_Staggered());
						}

						if (UCharacterMovementComponent* MC = Char->GetCharacterMovement())
						{
							MC->SetMovementMode(MOVE_Walking);
						}
					}, 1.0f, false);

					// 공격자 기록 (킬 추적용)
					HitCharacter->SetLastAttacker(Cast<AWjWorldCharacterPlay>(AvatarActor));

					UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_NormalAttack: Player %s staggered for 1s"), *HitCharacter->GetName());
				}
			}
		}
	}
}

void UGA_NormalAttack::OnMontageCompleted()
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

void UGA_NormalAttack::OnMontageBlendOut()
{
	// BlendOut도 정상 종료로 처리
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

void UGA_NormalAttack::OnMontageInterrupted()
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, true);
}

void UGA_NormalAttack::OnMontageCancelled()
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, true);
}

FVector UGA_NormalAttack::CalculateTargetLocation() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor) return FVector::ZeroVector;

	FVector AvatarLocation = AvatarActor->GetActorLocation();
	FRotator AvatarRotation = AvatarActor->GetActorRotation();

	// Yaw 기반 4방향 스냅 (GA_SpawnBrick과 동일 패턴)
	float NormalizedYaw = FRotator::NormalizeAxis(AvatarRotation.Yaw);
	FVector DirectionOffset = FVector::ZeroVector;

	if (NormalizedYaw >= -45.0f && NormalizedYaw < 45.0f)
	{
		DirectionOffset.X = CachedWallDesc.BrickSize.X;
	}
	else if (NormalizedYaw >= 45.0f && NormalizedYaw < 135.0f)
	{
		DirectionOffset.Y = CachedWallDesc.BrickSize.Y;
	}
	else if (NormalizedYaw >= -135.0f && NormalizedYaw < -45.0f)
	{
		DirectionOffset.Y = -CachedWallDesc.BrickSize.Y;
	}
	else
	{
		DirectionOffset.X = -CachedWallDesc.BrickSize.X;
	}

	FVector TargetPosition = AvatarLocation + DirectionOffset;

	// 그리드에 스냅
	FIntPoint GridIndex = UWjWorldBrickSpawner::CalculateBrickGridIndex(
		TargetPosition,
		CachedWallDesc.ColumnNum,
		CachedWallDesc.RowNum,
		CachedWallDesc.CenterOffset,
		CachedWallDesc.BrickSize
	);

	return UWjWorldBrickSpawner::CalculateBrickPosition(
		GridIndex.X,
		GridIndex.Y,
		CachedWallDesc.ColumnNum,
		CachedWallDesc.RowNum,
		CachedWallDesc.CenterOffset,
		CachedWallDesc.BrickSize
	);
}

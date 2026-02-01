// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_NormalAttack.h"

#include "WjWorldGameplayTag.h"
#include "WjWorldLogCategories.h"

#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"

#include "GamePlay/Wall/WjWorldBrickSpawner.h"
#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/Wall/WjWorldBrickActor.h"

#include "Engine/OverlapResult.h"

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
}

void UGA_NormalAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// WallDesc 캐시
	AWjWorldGameModePlay* GameModePlay = GetWorld()->GetAuthGameMode<AWjWorldGameModePlay>();
	if (GameModePlay)
	{
		UWjWorldGameRuleApproachingWall* GameRule = GameModePlay->GetCurrentGameRule<UWjWorldGameRuleApproachingWall>();
		if (GameRule)
		{
			CachedWallDesc = GameRule->GetWallDesc();
		}
	}

	// 서버에서만 공격 처리
	if (HasAuthority(&ActivationInfo))
	{
		FVector TargetLocation = CalculateTargetLocation();
		FVector HalfSize = CachedWallDesc.BrickSize * 0.3f;

		TArray<FOverlapResult> Overlaps;
		FCollisionShape CollisionShape = FCollisionShape::MakeBox(HalfSize);

		if (GetWorld()->OverlapMultiByObjectType(
			Overlaps,
			TargetLocation,
			FQuat::Identity,
			FCollisionObjectQueryParams::AllObjects,
			CollisionShape))
		{
			for (const FOverlapResult& Overlap : Overlaps)
			{
				AWjWorldBrickActor* BrickActor = Cast<AWjWorldBrickActor>(Overlap.GetActor());
				if (!BrickActor) continue;

				UWjWorldBrickComponent* BrickComp = BrickActor->GetBrickComponent();
				if (!BrickComp) continue;

				const FWjWorldBrickProperties& Props = BrickComp->GetBrickProperties();

				switch (Props.BrickType)
				{
				case EWjWorldBrickType::Standard:
					// 파괴 불가
					UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_NormalAttack: Standard brick - cannot destroy"));
					break;

				case EWjWorldBrickType::Explosive:
					// 폭발 처리
					UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_NormalAttack: Explosive brick - HandleWallCollision"));
					BrickComp->HandleWallCollision(FVector::ZeroVector);
					break;

				case EWjWorldBrickType::Moving:
					// 즉시 파괴
					UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_NormalAttack: Moving brick - destroying"));
					BrickComp->ReserveDestroyBrick(0.1f);
					break;

				case EWjWorldBrickType::Destructible:
					// HP 기반 데미지
					UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_NormalAttack: Destructible brick - applying damage"));
					BrickComp->ApplyDamage(1);
					break;
				}

				// 첫 번째 벽돌만 처리
				break;
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
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

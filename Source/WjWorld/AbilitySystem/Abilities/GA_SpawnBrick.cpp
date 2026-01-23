// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/GA_SpawnBrick.h"

#include "WjWorldLogCategories.h"
#include "WjWorldGameplayTag.h"

#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"

#include "GamePlay/Wall/WjWorldBrickSpawner.h"
#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/Wall/WjWorldBrickPreviewActor.h"

#include "Abilities/Tasks/AbilityTask_WaitConfirmCancel.h"
#include "TimerManager.h"

UGA_SpawnBrick::UGA_SpawnBrick()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Preview 중에 이 태그 부여
	ActivationOwnedTags.AddTag(WjWorldGameplayTag::State_SpawnBrickPreview());

	// 이 태그 가진 어빌리티 블록 (NormalAttack 등)
	BlockAbilitiesWithTag.AddTag(WjWorldGameplayTag::Ability_NormalAttack());
}

bool UGA_SpawnBrick::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UGA_SpawnBrick::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	// Preview는 로컬 클라이언트에서만 표시
	if (ActorInfo->IsLocallyControlled())
	{
		SpawnPreviewActor();

		// Preview 위치 업데이트 타이머 시작
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				PreviewUpdateTimerHandle,
				this,
				&UGA_SpawnBrick::UpdatePreviewLocation,
				0.016f, // ~60fps
				true
			);
		}
	}

	// Confirm/Cancel 대기 Task
	UAbilityTask_WaitConfirmCancel* WaitConfirmCancelTask = UAbilityTask_WaitConfirmCancel::WaitConfirmCancel(this);
	if (WaitConfirmCancelTask)
	{
		WaitConfirmCancelTask->OnConfirm.AddDynamic(this, &UGA_SpawnBrick::OnConfirmCallback);
		WaitConfirmCancelTask->OnCancel.AddDynamic(this, &UGA_SpawnBrick::OnCancelCallback);
		WaitConfirmCancelTask->ReadyForActivation();
	}
}

void UGA_SpawnBrick::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PreviewUpdateTimerHandle);
	}

	// Preview 정리
	DestroyPreviewActor();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SpawnBrick::SpawnPreviewActor()
{
	if (PreviewActor) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FVector SpawnLocation = CalculatePreviewLocation();
	PreviewActor = World->SpawnActor<AWjWorldBrickPreviewActor>(AWjWorldBrickPreviewActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);

	if (PreviewActor)
	{
		PreviewActor->InitializePreview(CachedWallDesc.BrickSize);
		PreviewActor->SetPreviewValid(CheckPreviewValid());
	}
}

void UGA_SpawnBrick::DestroyPreviewActor()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
}

FVector UGA_SpawnBrick::CalculatePreviewLocation() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor) return FVector::ZeroVector;

	FVector AvatarLocation = AvatarActor->GetActorLocation();
	FRotator AvatarRotation = AvatarActor->GetActorRotation();

	// Yaw를 45도 단위로 나눠서 4방향으로 스냅
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

	FVector PreviewPosition = AvatarLocation + DirectionOffset;

	// 그리드에 스냅
	FIntPoint GridIndex = UWjWorldBrickSpawner::CalculateBrickGridIndex(
		PreviewPosition,
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

bool UGA_SpawnBrick::CheckPreviewValid() const
{
	// TODO: 이미 벽돌이 있는 위치인지, 범위 밖인지 등 검사
	return true;
}

void UGA_SpawnBrick::OnConfirmCallback()
{
	UE_LOG(LogWjWorldAbilities, Log, TEXT("UGA_SpawnBrick::OnConfirmCallback"));

	if (CheckPreviewValid())
	{
		// 서버에서만 실제 스폰
		if (HasAuthority(&CurrentActivationInfo))
		{
			SpawnBrickAtPreviewLocation();
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_SpawnBrick::OnCancelCallback()
{
	UE_LOG(LogWjWorldAbilities, Log, TEXT("UGA_SpawnBrick::OnCancelCallback"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_SpawnBrick::SpawnBrickAtPreviewLocation()
{
	FVector SpawnPosition = CalculatePreviewLocation();
	FIntPoint SpawnIndexPoint = UWjWorldBrickSpawner::CalculateBrickGridIndex(
		SpawnPosition,
		CachedWallDesc.ColumnNum,
		CachedWallDesc.RowNum,
		CachedWallDesc.CenterOffset,
		CachedWallDesc.BrickSize
	);

	FWjWorldBrickProperties BrickProperties;
	BrickProperties.BrickType = EWjWorldBrickType::Destructible;
	BrickProperties.BrickMoveType = EWjWorldBrickMoveType::Standard;
	BrickProperties.Size = CachedWallDesc.BrickSize;
	BrickProperties.Color = FColor::Black;
	BrickProperties.SpawnedGridPosition = SpawnIndexPoint;
	BrickProperties.CenterOffset = CachedWallDesc.CenterOffset;
	BrickProperties.ColumnNum = CachedWallDesc.ColumnNum;
	BrickProperties.RowNum = CachedWallDesc.RowNum;

	UWjWorldBrickSpawner::SpawnBrickActor(GetWorld(), BrickProperties, SpawnIndexPoint.X, SpawnIndexPoint.Y);
}

void UGA_SpawnBrick::UpdatePreviewLocation()
{
	if (PreviewActor)
	{
		FVector NewLocation = CalculatePreviewLocation();
		PreviewActor->UpdatePreviewLocation(NewLocation);
		PreviewActor->SetPreviewValid(CheckPreviewValid());
	}
}

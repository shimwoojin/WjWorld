// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_LiftBrick.h"

#include "WjWorldGameplayTag.h"
#include "WjWorldLogCategories.h"

#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"

#include "GamePlay/Wall/WjWorldBrickSpawner.h"
#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/Wall/WjWorldBrickActor.h"
#include "GamePlay/Wall/WjWorldBrickPreviewActor.h"

#include "Abilities/Tasks/AbilityTask_WaitConfirmCancel.h"
#include "TimerManager.h"

#include "Engine/OverlapResult.h"

#include "Core/Play/WjWorldCharacterPlay.h"

UGA_LiftBrick::UGA_LiftBrick()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// 들고 있는 동안 이 태그 부여 (NormalAttack 차단)
	ActivationOwnedTags.AddTag(WjWorldGameplayTag::State_LiftBrickCarry());

	// SpawnBrickPreview 상태에서는 활성화 불가
	ActivationBlockedTags.AddTag(WjWorldGameplayTag::State_SpawnBrickPreview());

	// LiftBrick 태그가 있는 어빌리티 블록
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(WjWorldGameplayTag::Ability_LiftBrick());
	SetAssetTags(AssetTags);

	// 쿨다운 설정
	CooldownDuration = 2.0f;
	CooldownTags.AddTag(WjWorldGameplayTag::Cooldown_LiftBrick());

	// UI 메타데이터
	AbilityName = NSLOCTEXT("Abilities", "LiftBrick", "벽돌 들기");
}

void UGA_LiftBrick::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bHasLiftedBrick = false;

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

	// 서버에서 바라보는 방향의 벽돌 탐색 및 제거
	if (HasAuthority(&ActivationInfo))
	{
		FVector PickupLocation = CalculatePickupLocation();
		FVector HalfSize = CachedWallDesc.BrickSize * 0.3f;

		TArray<FOverlapResult> Overlaps;
		FCollisionShape CollisionShape = FCollisionShape::MakeBox(HalfSize);

		if (GetWorld()->OverlapMultiByObjectType(
			Overlaps,
			PickupLocation,
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

				// Moving 또는 Destructible 벽돌만 집을 수 있음
				if (Props.BrickType == EWjWorldBrickType::Moving || Props.BrickType == EWjWorldBrickType::Destructible)
				{
					// 원래 벽돌 정보 저장
					LiftedBrickProperties = Props;
					OriginalBrickLocation = BrickActor->GetActorLocation();
					OriginalGridIndex = Props.SpawnedGridPosition;
					bHasLiftedBrick = true;

					// 원래 벽돌 파괴
					BrickActor->Destroy();

					UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_LiftBrick: Picked up %s brick at %s"),
						Props.BrickType == EWjWorldBrickType::Moving ? TEXT("Moving") : TEXT("Destructible"),
						*OriginalBrickLocation.ToString());
					break;
				}
			}
		}

		if (!bHasLiftedBrick)
		{
			UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_LiftBrick: No liftable brick found"));
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}

	// Preview는 로컬 클라이언트에서만 표시
	if (ActorInfo->IsLocallyControlled())
	{
		SpawnPreviewActor();

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				PreviewUpdateTimerHandle,
				this,
				&UGA_LiftBrick::UpdatePreviewLocation,
				0.016f, // ~60fps
				true
			);
		}

		// 프롬프트 UI 표시
		if (AWjWorldCharacterPlay* CharacterPlay = Cast<AWjWorldCharacterPlay>(GetAvatarActorFromActorInfo()))
		{
			CharacterPlay->ShowAbilityPrompt(
				NSLOCTEXT("AbilityPrompt", "ConfirmKey", "좌클릭"),
				NSLOCTEXT("AbilityPrompt", "CancelKey", "우클릭"),
				GetPromptDescription()
			);
		}
	}

	// Confirm/Cancel 대기 Task
	UAbilityTask_WaitConfirmCancel* WaitConfirmCancelTask = UAbilityTask_WaitConfirmCancel::WaitConfirmCancel(this);
	if (WaitConfirmCancelTask)
	{
		WaitConfirmCancelTask->OnConfirm.AddDynamic(this, &UGA_LiftBrick::OnConfirmCallback);
		WaitConfirmCancelTask->OnCancel.AddDynamic(this, &UGA_LiftBrick::OnCancelCallback);
		WaitConfirmCancelTask->ReadyForActivation();
	}
}

void UGA_LiftBrick::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PreviewUpdateTimerHandle);
	}

	DestroyPreviewActor();

	// 프롬프트 UI 숨김
	if (ActorInfo && ActorInfo->IsLocallyControlled())
	{
		if (AWjWorldCharacterPlay* CharacterPlay = Cast<AWjWorldCharacterPlay>(GetAvatarActorFromActorInfo()))
		{
			CharacterPlay->HideAbilityPrompt();
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FVector UGA_LiftBrick::CalculatePickupLocation() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor) return FVector::ZeroVector;

	FVector AvatarLocation = AvatarActor->GetActorLocation();
	FRotator AvatarRotation = AvatarActor->GetActorRotation();

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

FVector UGA_LiftBrick::CalculatePreviewLocation() const
{
	// GA_SpawnBrick과 동일한 패턴
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor) return FVector::ZeroVector;

	FVector AvatarLocation = AvatarActor->GetActorLocation();
	FRotator AvatarRotation = AvatarActor->GetActorRotation();

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

bool UGA_LiftBrick::CheckPreviewValid() const
{
	FVector PreviewLocation = CalculatePreviewLocation();

	// 그리드 범위 체크
	FIntPoint GridIndex = UWjWorldBrickSpawner::CalculateBrickGridIndex(
		PreviewLocation,
		CachedWallDesc.ColumnNum,
		CachedWallDesc.RowNum,
		CachedWallDesc.CenterOffset,
		CachedWallDesc.BrickSize
	);

	if (GridIndex.X < 0 || GridIndex.X >= CachedWallDesc.ColumnNum ||
		GridIndex.Y < 0 || GridIndex.Y >= CachedWallDesc.RowNum)
	{
		return false;
	}

	// 이미 벽돌이 있는 위치인지 체크
	UWorld* World = GetWorld();
	if (!World) return false;

	FVector HalfSize = CachedWallDesc.BrickSize * 0.3f;
	TArray<FOverlapResult> Overlaps;
	FCollisionShape CollisionShape = FCollisionShape::MakeBox(HalfSize);

	if (World->OverlapMultiByObjectType(
		Overlaps,
		PreviewLocation,
		FQuat::Identity,
		FCollisionObjectQueryParams::AllObjects,
		CollisionShape))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (Cast<AWjWorldBrickActor>(Overlap.GetActor()))
			{
				return false;
			}
		}
	}

	return true;
}

void UGA_LiftBrick::SpawnPreviewActor()
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

void UGA_LiftBrick::DestroyPreviewActor()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
}

void UGA_LiftBrick::OnConfirmCallback()
{
	UE_LOG(LogWjWorldAbilities, Log, TEXT("UGA_LiftBrick::OnConfirmCallback"));

	if (HasAuthority(&CurrentActivationInfo) && bHasLiftedBrick)
	{
		if (CheckPreviewValid())
		{
			// 프리뷰 위치에 벽돌 스폰
			FVector SpawnLocation = CalculatePreviewLocation();
			SpawnBrickAtLocation(SpawnLocation);
		}
		else
		{
			// 유효하지 않으면 원래 위치에 복원
			SpawnBrickAtLocation(OriginalBrickLocation);
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_LiftBrick::OnCancelCallback()
{
	UE_LOG(LogWjWorldAbilities, Log, TEXT("UGA_LiftBrick::OnCancelCallback"));

	// Cancel 시 원래 위치에 벽돌 복원
	if (HasAuthority(&CurrentActivationInfo) && bHasLiftedBrick)
	{
		SpawnBrickAtLocation(OriginalBrickLocation);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_LiftBrick::UpdatePreviewLocation()
{
	if (PreviewActor)
	{
		FVector NewLocation = CalculatePreviewLocation();
		PreviewActor->UpdatePreviewLocation(NewLocation);
		PreviewActor->SetPreviewValid(CheckPreviewValid());
	}
}

FText UGA_LiftBrick::GetPromptDescription() const
{
	return NSLOCTEXT("AbilityPrompt", "LiftBrickDesc", "벽돌을 놓을 위치를 선택하세요");
}

void UGA_LiftBrick::SpawnBrickAtLocation(const FVector& Location)
{
	FIntPoint SpawnGridIndex = UWjWorldBrickSpawner::CalculateBrickGridIndex(
		Location,
		CachedWallDesc.ColumnNum,
		CachedWallDesc.RowNum,
		CachedWallDesc.CenterOffset,
		CachedWallDesc.BrickSize
	);

	FWjWorldBrickProperties SpawnProps = LiftedBrickProperties;
	SpawnProps.SpawnedGridPosition = SpawnGridIndex;

	UWjWorldBrickSpawner::SpawnBrickActor(GetWorld(), SpawnProps, SpawnGridIndex.X, SpawnGridIndex.Y);
}

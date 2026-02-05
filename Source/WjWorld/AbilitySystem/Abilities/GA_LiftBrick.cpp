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

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitConfirmCancel.h"
#include "TimerManager.h"

#include "Engine/OverlapResult.h"

#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/GameData/ApproachingWallGameDataComponent.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "GamePlay/Wall/WjWorldWallDescriptionDataAsset.h"

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

	// 클라이언트에서는 GameMode에 접근 불가하므로 GameState의 GameData에서 WallName을 가져와 로드
	if (CachedWallDesc.BrickSize.IsZero() || CachedWallDesc.ColumnNum == 0)
	{
		FString WallNameToLoad;

		// GameState에서 현재 Wall 이름 가져오기
		if (World)
		{
			if (AWjWorldGameStatePlay* GameState = World->GetGameState<AWjWorldGameStatePlay>())
			{
				if (UApproachingWallGameDataComponent* GameData = GameState->GetGameData<UApproachingWallGameDataComponent>())
				{
					WallNameToLoad = GameData->GetCurrentWallName();
				}
			}
		}

		const UWjWorldDeveloperSettings* DevSettings = GetDefault<UWjWorldDeveloperSettings>();
		if (DevSettings && !DevSettings->WallDescriptionAsset.IsNull())
		{
			UWjWorldWallDescriptionDataAsset* WallDescAsset = DevSettings->WallDescriptionAsset.LoadSynchronous();
			if (WallDescAsset)
			{
				// WallName으로 정확한 WallDescription 조회
				if (!WallNameToLoad.IsEmpty() && WallDescAsset->GetWallDescriptionByName(WallNameToLoad, CachedWallDesc))
				{
					if (CachedWallDesc.IsLayoutEmpty())
					{
						CachedWallDesc.LoadWallLayoutFromFile();
					}
					UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_LiftBrick: Loaded WallDesc '%s' from GameData"), *WallNameToLoad);
				}
				// WallName이 없으면 첫 번째 Description 사용 (폴백)
				else if (WallDescAsset->WallDescriptions.Num() > 0)
				{
					CachedWallDesc = WallDescAsset->WallDescriptions[0];
					if (CachedWallDesc.IsLayoutEmpty())
					{
						CachedWallDesc.LoadWallLayoutFromFile();
					}
					UE_LOG(LogWjWorldAbilities, Warning, TEXT("GA_LiftBrick: WallName not found, using first WallDesc as fallback"));
				}
			}
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

					// 캐릭터에 들고 있는 벽돌 시각화 (3자에게도 보임)
					if (AWjWorldCharacterPlay* CharacterPlay = Cast<AWjWorldCharacterPlay>(GetAvatarActorFromActorInfo()))
					{
						const UWjWorldDeveloperSettings* DevSettings = GetDefault<UWjWorldDeveloperSettings>();
						if (DevSettings && DevSettings->BrickMesh.IsValid())
						{
							UStaticMesh* BrickMesh = DevSettings->BrickMesh.LoadSynchronous();
							CharacterPlay->ShowLiftedBrick(BrickMesh, CachedWallDesc.BrickSize / 100.f, Props.GetColorWithBrickType());
						}
					}

					// GameplayCue 실행 (벽돌 집는 사운드)
					if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
					{
						FGameplayCueParameters CueParams;
						CueParams.Location = OriginalBrickLocation;
						ASC->ExecuteGameplayCue(WjWorldGameplayTag::GameplayCue_Ability_LiftBrick(), CueParams);
					}

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

		if (World)
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
	if (UWorld* WorldPtr = GetWorld())
	{
		WorldPtr->GetTimerManager().ClearTimer(PreviewUpdateTimerHandle);
	}

	DestroyPreviewActor();

	// 들고 있는 벽돌 시각화 숨김 (서버에서)
	if (HasAuthority(&ActivationInfo))
	{
		if (AWjWorldCharacterPlay* CharacterPlay = Cast<AWjWorldCharacterPlay>(GetAvatarActorFromActorInfo()))
		{
			CharacterPlay->HideLiftedBrick();
		}
	}

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
		FVector PlaceLocation;
		if (CheckPreviewValid())
		{
			// 프리뷰 위치에 벽돌 스폰
			PlaceLocation = CalculatePreviewLocation();
			SpawnBrickAtLocation(PlaceLocation);
		}
		else
		{
			// 유효하지 않으면 원래 위치에 복원
			PlaceLocation = OriginalBrickLocation;
			SpawnBrickAtLocation(PlaceLocation);
		}

		// GameplayCue 실행 (벽돌 놓는 사운드)
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			FGameplayCueParameters CueParams;
			CueParams.Location = PlaceLocation;
			ASC->ExecuteGameplayCue(WjWorldGameplayTag::GameplayCue_Ability_LiftBrick_Place(), CueParams);
		}
	}

	// 쿨다운 적용
	ApplyCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);

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

	// 쿨다운 적용 (Cancel 시에도 쿨다운 적용)
	ApplyCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);

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

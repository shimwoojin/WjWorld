// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/GA_SpawnBrick.h"

#include "WjWorldLogCategories.h"
#include "WjWorldGameplayTag.h"

#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"

#include "GamePlay/Wall/WjWorldBrickSpawner.h"
#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/Wall/WjWorldBrickActor.h"
#include "GamePlay/Wall/WjWorldBrickPreviewActor.h"

#include "Abilities/Tasks/AbilityTask_WaitConfirmCancel.h"
#include "TimerManager.h"

#include "Engine/OverlapResult.h"

#include "Setting/WjWorldDeveloperSettings.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Effects/GE_SpawnBrickChargeCost.h"
#include "AbilitySystem/AttributeSets/WjWorldCharacterAttributeSet.h"
#include "GameplayEffect.h"

#include "Core/Play/WjWorldCharacterPlay.h"

UGA_SpawnBrick::UGA_SpawnBrick()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Preview 중에 이 태그 부여
	ActivationOwnedTags.AddTag(WjWorldGameplayTag::State_SpawnBrickPreview());

	// 이 태그 가진 어빌리티 블록 (NormalAttack 등)
	BlockAbilitiesWithTag.AddTag(WjWorldGameplayTag::Ability_NormalAttack());

	// UI 메타데이터
	AbilityName = NSLOCTEXT("Abilities", "SpawnBrick", "벽돌 배치");
}

void UGA_SpawnBrick::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		return;
	}

	// 서버에서 어트리뷰트 초기값 설정
	if (ActorInfo->IsNetAuthority())
	{
		ASC->SetNumericAttributeBase(UWjWorldCharacterAttributeSet::GetMaxSpawnBrickChargesAttribute(), static_cast<float>(MaxCharges));
		ASC->SetNumericAttributeBase(UWjWorldCharacterAttributeSet::GetSpawnBrickChargesAttribute(), static_cast<float>(MaxCharges));
	}

	// 리필 GE 오브젝트 생성
	CreateRefillEffect();

	// SpawnBrickCharges 변경 델리게이트 등록
	ChargesChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(
		UWjWorldCharacterAttributeSet::GetSpawnBrickChargesAttribute()
	).AddUObject(this, &UGA_SpawnBrick::OnSpawnBrickChargesChanged);
}

void UGA_SpawnBrick::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	StopChargeRefill();

	// 델리게이트 해제
	if (ChargesChangedDelegateHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		if (ASC)
		{
			ASC->GetGameplayAttributeValueChangeDelegate(
				UWjWorldCharacterAttributeSet::GetSpawnBrickChargesAttribute()
			).Remove(ChargesChangedDelegateHandle);
		}
		ChargesChangedDelegateHandle.Reset();
	}

	Super::OnRemoveAbility(ActorInfo, Spec);
}

bool UGA_SpawnBrick::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 충전 잔량 체크
	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (ASC)
	{
		float CurrentCharges = ASC->GetNumericAttribute(UWjWorldCharacterAttributeSet::GetSpawnBrickChargesAttribute());
		if (CurrentCharges < 1.f)
		{
			return false;
		}
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

	// 이미 벽돌이 있는 위치인지 오버랩 체크
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

	// 서버에서는 안전 구역 범위 내인지 체크
	if (HasAuthority(&CurrentActivationInfo))
	{
		AWjWorldGameModePlay* GameModePlay = World->GetAuthGameMode<AWjWorldGameModePlay>();
		if (GameModePlay)
		{
			UWjWorldGameRuleApproachingWall* GameRule = GameModePlay->GetCurrentGameRule<UWjWorldGameRuleApproachingWall>();
			if (GameRule)
			{
				const TSet<FIntPoint>& SafeZonePoints = GameRule->GetCurrentSafeZonePoints();
				if (!SafeZonePoints.Contains(GridIndex))
				{
					return false;
				}
			}
		}
	}

	return true;
}

void UGA_SpawnBrick::OnConfirmCallback()
{
	UE_LOG(LogWjWorldAbilities, Log, TEXT("UGA_SpawnBrick::OnConfirmCallback"));

	if (CheckPreviewValid())
	{
		// 서버에서만 실제 스폰 + 충전 소모
		if (HasAuthority(&CurrentActivationInfo))
		{
			ApplyChargeCost();
			SpawnBrickAtPreviewLocation();
			StartChargeRefill();
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
	BrickProperties.BrickType = FMath::RandBool() ? EWjWorldBrickType::Moving : EWjWorldBrickType::Destructible;
	BrickProperties.BrickMoveType = EWjWorldBrickMoveType::Standard;
	BrickProperties.Size = CachedWallDesc.BrickSize;
	BrickProperties.Color = BrickProperties.GetColorWithBrickType();
	BrickProperties.SpawnedGridPosition = SpawnIndexPoint;
	BrickProperties.CenterOffset = CachedWallDesc.CenterOffset;
	BrickProperties.ColumnNum = CachedWallDesc.ColumnNum;
	BrickProperties.RowNum = CachedWallDesc.RowNum;

	// Destructible 벽돌은 DeveloperSettings에서 MaxHP 설정
	if (BrickProperties.BrickType == EWjWorldBrickType::Destructible)
	{
		BrickProperties.MaxHP = GetDefault<UWjWorldDeveloperSettings>()->DestructibleBrickDefaultHP;
	}

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

// ---- 충전 시스템 구현 ----

void UGA_SpawnBrick::ApplyChargeCost()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	const UGameplayEffect* CostGE = UGE_SpawnBrickChargeCost::StaticClass()->GetDefaultObject<UGameplayEffect>();
	if (CostGE)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CostGE->GetClass(), GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
		}
	}
}

void UGA_SpawnBrick::CreateRefillEffect()
{
	if (RefillEffect)
	{
		return;
	}

	RefillEffect = NewObject<UGameplayEffect>(this, FName(TEXT("GE_SpawnBrickChargeRefill")));
	RefillEffect->DurationPolicy = EGameplayEffectDurationType::Infinite;
	RefillEffect->Period = FScalableFloat(ChargeRefillInterval);
	RefillEffect->bExecutePeriodicEffectOnApplication = false;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UWjWorldCharacterAttributeSet::GetSpawnBrickChargesAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(1.f));
	RefillEffect->Modifiers.Add(Modifier);
}

void UGA_SpawnBrick::StartChargeRefill()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !RefillEffect)
	{
		return;
	}

	// 이미 리필 중이면 무시
	if (RefillEffectHandle.IsValid() && ASC->GetActiveGameplayEffect(RefillEffectHandle))
	{
		return;
	}

	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	FGameplayEffectSpec Spec(RefillEffect, ContextHandle, 1.f);
	RefillEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(Spec);

	// 리필 시작 시각 기록
	if (UWorld* World = GetWorld())
	{
		RefillStartWorldTime = World->GetTimeSeconds();
	}

	UE_LOG(LogWjWorldAbilities, Log, TEXT("UGA_SpawnBrick: 충전 리필 시작 (%.1f초 간격)"), ChargeRefillInterval);
}

void UGA_SpawnBrick::StopChargeRefill()
{
	if (!RefillEffectHandle.IsValid())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		ASC->RemoveActiveGameplayEffect(RefillEffectHandle);
	}

	RefillEffectHandle.Invalidate();
	RefillStartWorldTime = 0.0;

	UE_LOG(LogWjWorldAbilities, Log, TEXT("UGA_SpawnBrick: 충전 리필 중지 (충전 완료)"));
}

void UGA_SpawnBrick::OnSpawnBrickChargesChanged(const FOnAttributeChangeData& Data)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	// 리필로 충전이 증가했으면 리필 시작 시각 갱신 (다음 리필까지의 남은 시간 계산용)
	if (Data.NewValue > Data.OldValue && RefillEffectHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			RefillStartWorldTime = World->GetTimeSeconds();
		}
	}

	// 서버에서만 리필 관리
	if (!CurrentActorInfo || !CurrentActorInfo->IsNetAuthority())
	{
		return;
	}

	float MaxChargesValue = ASC->GetNumericAttribute(UWjWorldCharacterAttributeSet::GetMaxSpawnBrickChargesAttribute());

	// 충전이 최대치에 도달하면 리필 GE 제거
	if (Data.NewValue >= MaxChargesValue && RefillEffectHandle.IsValid())
	{
		StopChargeRefill();
	}
}

// ---- 충전 조회 메서드 ----

int32 UGA_SpawnBrick::GetCurrentCharges() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return 0;
	}
	return FMath::TruncToInt32(ASC->GetNumericAttribute(UWjWorldCharacterAttributeSet::GetSpawnBrickChargesAttribute()));
}

int32 UGA_SpawnBrick::GetMaxCharges() const
{
	return MaxCharges;
}

float UGA_SpawnBrick::GetChargeRefillTimeRemaining() const
{
	if (!RefillEffectHandle.IsValid() || RefillStartWorldTime <= 0.0)
	{
		return 0.f;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.f;
	}

	double Elapsed = World->GetTimeSeconds() - RefillStartWorldTime;
	float Remaining = ChargeRefillInterval - static_cast<float>(Elapsed);
	return FMath::Max(Remaining, 0.f);
}

FText UGA_SpawnBrick::GetPromptDescription() const
{
	return NSLOCTEXT("AbilityPrompt", "SpawnBrickDesc", "벽돌을 배치할 위치를 선택하세요");
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WjWorldGameplayAbilityBase.h"
#include "GamePlay/Wall/WjWorldWallDescriptionDataAsset.h"
#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GA_LiftBrick.generated.h"

class AWjWorldBrickPreviewActor;

/**
 * 바라보는 방향의 Moving/Destructible 벽돌을 집어서 재배치하는 어빌리티
 * - GA_SpawnBrick의 Preview + WaitConfirmCancel 패턴 사용
 * - 활성화 시: 바라보는 방향에서 Moving/Destructible 벽돌 탐색
 *   - 없으면 즉시 EndAbility(bWasCancelled=true)
 *   - 있으면 원래 벽돌 Destroy → 프리뷰 표시 → WaitConfirmCancel
 * - Confirm: 프리뷰 위치에 같은 타입의 벽돌 새로 스폰
 * - Cancel: 원래 위치에 같은 타입의 벽돌 복원
 */
UCLASS()
class WJWORLD_API UGA_LiftBrick : public UWjWorldGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_LiftBrick();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	FVector CalculatePickupLocation() const;
	FVector CalculatePreviewLocation() const;
	bool CheckPreviewValid() const;

	void SpawnPreviewActor();
	void DestroyPreviewActor();

	UFUNCTION()
	void OnConfirmCallback();

	UFUNCTION()
	void OnCancelCallback();

	UFUNCTION()
	void UpdatePreviewLocation();

	void SpawnBrickAtLocation(const FVector& Location);

private:
	UPROPERTY()
	TObjectPtr<AWjWorldBrickPreviewActor> PreviewActor;

	FWjWorldWallDescription CachedWallDesc;
	FWjWorldBrickProperties LiftedBrickProperties;
	FVector OriginalBrickLocation;
	FIntPoint OriginalGridIndex;
	bool bHasLiftedBrick = false;

	FTimerHandle PreviewUpdateTimerHandle;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WjWorldGameplayAbilityBase.h"
#include "GamePlay/Wall/WjWorldWallDescriptionDataAsset.h"
#include "GA_SpawnBrick.generated.h"

class AWjWorldBrickPreviewActor;

/**
 * R키로 활성화 → Preview 표시 → 좌클릭(Confirm)으로 스폰 / 우클릭(Cancel)으로 취소
 */
UCLASS()
class WJWORLD_API UGA_SpawnBrick : public UWjWorldGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_SpawnBrick();

public:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	// Preview 관련
	void SpawnPreviewActor();
	void DestroyPreviewActor();
	FVector CalculatePreviewLocation() const;
	bool CheckPreviewValid() const;

	// Confirm/Cancel 콜백
	UFUNCTION()
	void OnConfirmCallback();

	UFUNCTION()
	void OnCancelCallback();

	// 실제 벽돌 스폰 (서버에서만)
	void SpawnBrickAtPreviewLocation();

private:
	UPROPERTY()
	TObjectPtr<AWjWorldBrickPreviewActor> PreviewActor;

	// 캐시된 WallDesc (Preview 업데이트용)
	FWjWorldWallDescription CachedWallDesc;

	// Tick으로 Preview 위치 업데이트용 타이머
	FTimerHandle PreviewUpdateTimerHandle;

	UFUNCTION()
	void UpdatePreviewLocation();
};

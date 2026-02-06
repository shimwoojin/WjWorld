// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WjWorldGameplayAbilityBase.h"
#include "GamePlay/Wall/WjWorldWallDescriptionDataAsset.h"
#include "ActiveGameplayEffectHandle.h"
#include "GA_SpawnBrick.generated.h"

class AWjWorldBrickPreviewActor;
struct FOnAttributeChangeData;

/**
 * R키로 활성화 → Preview 표시 → 좌클릭(Confirm)으로 스폰 / 우클릭(Cancel)으로 취소
 * 충전 시스템: 최대 MaxCharges회 사용 가능, ChargeRefillInterval초마다 1회 리필
 */
UCLASS(Abstract)
class WJWORLD_API UGA_SpawnBrick : public UWjWorldGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_SpawnBrick();

	// 충전 시스템 오버라이드
	virtual bool IsChargeBased() const override { return true; }
	virtual int32 GetCurrentCharges() const override;
	virtual int32 GetMaxCharges() const override;
	virtual float GetChargeRefillTimeRemaining() const override;

	// 프롬프트 설명
	virtual FText GetPromptDescription() const override;

public:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
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

	// 클라이언트가 Confirm할 때 GridIndex를 서버로 전달
	UFUNCTION(Server, Reliable)
	void ServerSpawnBrickAtGridIndex(int32 GridX, int32 GridY);

	// ---- 충전 시스템 ----

	/** 충전 소모 GE 적용 (-1) */
	void ApplyChargeCost();

	/** 주기적 리필 GE 시작 (이미 활성 중이면 무시) */
	void StartChargeRefill();

	/** 리필 GE 중지 */
	void StopChargeRefill();

	/** 리필 GE 오브젝트 생성 (최초 1회) */
	void CreateRefillEffect();

	/** SpawnBrickCharges 어트리뷰트 변경 콜백 */
	void OnSpawnBrickChargesChanged(const FOnAttributeChangeData& Data);

private:
	UPROPERTY()
	TObjectPtr<AWjWorldBrickPreviewActor> PreviewActor;

	// 캐시된 WallDesc (Preview 업데이트용)
	FWjWorldWallDescription CachedWallDesc;

	// 캐시된 프리뷰 GridIndex (클라이언트에서 Confirm 시 서버로 전달)
	FIntPoint CachedPreviewGridIndex = FIntPoint(-1, -1);

	// Tick으로 Preview 위치 업데이트용 타이머
	FTimerHandle PreviewUpdateTimerHandle;

	UFUNCTION()
	void UpdatePreviewLocation();

	// ---- 충전 시스템 ----

	/** 최대 충전 횟수 (N) */
	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	int32 MaxCharges = 3;

	/** 충전 리필 간격 (M초) */
	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	float ChargeRefillInterval = 5.0f;

	/** 리필용 동적 GE 오브젝트 (캐시) */
	UPROPERTY()
	TObjectPtr<UGameplayEffect> RefillEffect;

	/** 현재 활성 리필 GE 핸들 */
	FActiveGameplayEffectHandle RefillEffectHandle;

	/** 어트리뷰트 변경 델리게이트 핸들 */
	FDelegateHandle ChargesChangedDelegateHandle;

	/** 리필 시작 시각 (GetWorld()->GetTimeSeconds() 기준) */
	double RefillStartWorldTime = 0.0;
};

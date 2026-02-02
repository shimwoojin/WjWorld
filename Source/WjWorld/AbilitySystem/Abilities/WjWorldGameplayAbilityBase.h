// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/Texture2D.h"
#include "WjWorldGameplayAbilityBase.generated.h"

/**
 * 프로젝트 어빌리티 베이스 클래스
 * - 쿨다운 지원 (CooldownDuration + CooldownTags → GE_AbilityCooldown)
 * - 제거 상태 차단 (State.Eliminated)
 */
UCLASS(Abstract)
class WJWORLD_API UWjWorldGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UWjWorldGameplayAbilityBase();

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	// UI 메타데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	FText AbilityName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UTexture2D> AbilityIcon;

	// 충전 시스템 (하위 클래스에서 선택적 오버라이드)
	virtual bool IsChargeBased() const { return false; }
	virtual int32 GetCurrentCharges() const { return 0; }
	virtual int32 GetMaxCharges() const { return 0; }
	virtual float GetChargeRefillTimeRemaining() const { return 0.f; }

	/** Confirm/Cancel 프롬프트 설명 텍스트 (빈 값이면 프롬프트 표시 안 함) */
	virtual FText GetPromptDescription() const { return FText::GetEmpty(); }

protected:
	// 쿨다운 시간 (0이면 쿨다운 없음)
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	float CooldownDuration = 0.f;

	// 쿨다운 중 부여될 태그 (CanActivateAbility에서 체크됨)
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	FGameplayTagContainer CooldownTags;

private:
	// GetCooldownTags() 반환용 (부모 태그 + CooldownTags 합산)
	mutable FGameplayTagContainer MergedCooldownTags;
};

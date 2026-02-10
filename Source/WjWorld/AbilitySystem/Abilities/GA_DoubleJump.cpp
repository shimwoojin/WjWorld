// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_DoubleJump.h"

#include "WjWorldGameplayTag.h"
#include "WjWorldLogCategories.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UGA_DoubleJump::UGA_DoubleJump()
{
	// 부모 AssetTag 교체
	FGameplayTagContainer NewAssetTags;
	NewAssetTags.AddTag(WjWorldGameplayTag::Ability_DoubleJump());
	SetAssetTags(NewAssetTags);

	// UI 메타데이터
	AbilityName = NSLOCTEXT("Abilities", "DoubleJump", "이중 점프");
}

bool UGA_DoubleJump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	// Base 어빌리티 체크 (제거 상태, 게임 페이즈, 쿨다운 등) - GA_Jump의 CanJump() 체크를 건너뜀
	if (!UWjWorldGameplayAbilityBase::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get(), ECastCheckedType::NullAllowed);
	if (!Character)
	{
		return false;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return false;
	}

	// 지상이면 항상 허용 (카운트 리셋)
	if (!MovementComp->IsFalling())
	{
		CurrentJumpCount = 0;
		return true;
	}

	// 공중이면 카운트 체크
	return CurrentJumpCount < MaxJumpCount;
}

void UGA_DoubleJump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// GA_Jump의 ActivateAbility 호출 (Super::ActivateAbility + CommitAbility + Jump)
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CurrentJumpCount++;

	UE_LOG(LogWjWorldAbilities, Log, TEXT("GA_DoubleJump: Jump count %d / %d"), CurrentJumpCount, MaxJumpCount);
}

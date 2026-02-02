// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "WjWorldAttributeSetBase.h"
#include "WjWorldCharacterAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class WJWORLD_API UWjWorldCharacterAttributeSet : public UWjWorldAttributeSetBase
{
	GENERATED_BODY()
	
public:
	ATTRIBUTE_ACCESSORS_BASIC(UWjWorldCharacterAttributeSet, MaxHP);
	ATTRIBUTE_ACCESSORS_BASIC(UWjWorldCharacterAttributeSet, HP);
	ATTRIBUTE_ACCESSORS_BASIC(UWjWorldCharacterAttributeSet, MaxSpawnBrickCharges);
	ATTRIBUTE_ACCESSORS_BASIC(UWjWorldCharacterAttributeSet, SpawnBrickCharges);

protected:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const override;
	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

protected:
	UFUNCTION()
	virtual void OnRep_MaxHP(const FGameplayAttributeData& OldMaxHP);

	UFUNCTION()
	virtual void OnRep_HP(const FGameplayAttributeData& OldHP);

	UFUNCTION()
	virtual void OnRep_MaxSpawnBrickCharges(const FGameplayAttributeData& OldMaxSpawnBrickCharges);

	UFUNCTION()
	virtual void OnRep_SpawnBrickCharges(const FGameplayAttributeData& OldSpawnBrickCharges);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Property", ReplicatedUsing = OnRep_MaxHP, Meta = (AllowPrivateAccess = "True"))
	FGameplayAttributeData MaxHP;

	UPROPERTY(BlueprintReadOnly, Category = "Property", ReplicatedUsing = OnRep_HP, Meta = (AllowPrivateAccess = "True"))
	FGameplayAttributeData HP;

	// SpawnBrick 충전 시스템
	UPROPERTY(BlueprintReadOnly, Category = "Property", ReplicatedUsing = OnRep_MaxSpawnBrickCharges, Meta = (AllowPrivateAccess = "True"))
	FGameplayAttributeData MaxSpawnBrickCharges;

	UPROPERTY(BlueprintReadOnly, Category = "Property", ReplicatedUsing = OnRep_SpawnBrickCharges, Meta = (AllowPrivateAccess = "True"))
	FGameplayAttributeData SpawnBrickCharges;
};

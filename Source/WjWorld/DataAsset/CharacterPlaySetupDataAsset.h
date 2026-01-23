// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WjTypes.h"
#include "CharacterPlaySetupDataAsset.generated.h"

class AWjWorldCharacterPlay;
class UGameplayAbility;

/**
 * 
 */
UCLASS()
class WJWORLD_API UCharacterPlaySetupDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	void Initialize(AWjWorldCharacterPlay* TargetCharacter, FString& OutErrorMsg);

private:
	// 서버 전용: 어빌리티 부여
	void InitializeAbilities(AWjWorldCharacterPlay* TargetCharacter);

	// 서버/클라이언트 공통 초기화
	void InitializeCommon(AWjWorldCharacterPlay* TargetCharacter);

public:
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> StartAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TMap<EWjWorldAbilityInputID, TSubclassOf<UGameplayAbility>> StartInputAbilities;
};

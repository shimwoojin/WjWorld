// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterPlaySetupDataAsset.generated.h"

class AWjWorldCharacterPlay;

/**
 * 
 */
UCLASS()
class WJWORLD_API UCharacterPlaySetupDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	void Initialize(AWjWorldCharacterPlay* TargetCharacter, FString& OutErrorMsg);
};

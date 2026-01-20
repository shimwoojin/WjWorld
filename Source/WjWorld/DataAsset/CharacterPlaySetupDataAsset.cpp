// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/CharacterPlaySetupDataAsset.h"
#include "Core/Play/WjWorldCharacterPlay.h"

void UCharacterPlaySetupDataAsset::Initialize(AWjWorldCharacterPlay* TargetCharacter, FString& OutErrorMsg)
{
	if (TargetCharacter == nullptr)
	{
		OutErrorMsg = TEXT("TargetCharacter is null");
	}

	if (OutErrorMsg.Len() == 0)
	{
		TargetCharacter->bSetupDataAssetCall = true;
		//TODO:
		//TargetCharacter->bSetupDataAssetFinal = true;
	}
}

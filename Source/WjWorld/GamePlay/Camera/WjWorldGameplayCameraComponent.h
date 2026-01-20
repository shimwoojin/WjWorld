// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "GameFramework/GameplayCameraComponent.h"
#include "GameplayTagContainer.h"
#include "Components/SceneComponent.h"
#include "WjWorldGameplayCameraComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCameraModeSetSignature, const FGameplayTag&, CameraMode);

struct FGameplayTag;

/**
 * 
 */
UCLASS()
class WJWORLD_API UWjWorldGameplayCameraComponent /*: public UGameplayCameraComponent*/ : public USceneComponent
{
	GENERATED_BODY()
	
public:
	const FGameplayTag& GetCurrentCameraMode() const;
	void SetCameraMode(const FGameplayTag& InCameraModeTag);

private:
	FGameplayTag CurrentCameraMode;
	
	UPROPERTY(BlueprintAssignable)
	FCameraModeSetSignature OnCameraModeSetted;
};

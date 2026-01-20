// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Camera/WjWorldGameplayCameraComponent.h"

const FGameplayTag& UWjWorldGameplayCameraComponent::GetCurrentCameraMode() const
{
	return CurrentCameraMode;
}

void UWjWorldGameplayCameraComponent::SetCameraMode(const FGameplayTag& InCameraModeTag)
{
	CurrentCameraMode = InCameraModeTag;
	OnCameraModeSetted.Broadcast(InCameraModeTag);
}

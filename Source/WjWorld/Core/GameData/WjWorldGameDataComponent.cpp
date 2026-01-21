// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameData/WjWorldGameDataComponent.h"

void UWjWorldGameDataComponent::GetValueInt(const FGameplayTag& InTag, int32& OutValue, bool& bFound) const
{
	const int32* Value = GetValue<int32>(InTag);
	bFound = Value != nullptr;

	if (bFound)
	{
		OutValue = *Value;
	}
}

void UWjWorldGameDataComponent::GetValueFloat(const FGameplayTag& InTag, float& OutValue, bool& bFound) const
{
	const float* Value = GetValue<float>(InTag);
	bFound = Value != nullptr;

	if (bFound)
	{
		OutValue = *Value;
	}
}

void UWjWorldGameDataComponent::GetValueString(const FGameplayTag& InTag, FString& OutValue, bool& bFound) const
{
	const FString* Value = GetValue<FString>(InTag);
	bFound = Value != nullptr;

	if (bFound)
	{
		OutValue = *Value;
	}
}

void UWjWorldGameDataComponent::SetValueInt(const FGameplayTag& InTag, int32 InValue)
{
	SetValue(InTag, InValue);
}

void UWjWorldGameDataComponent::SetValueFloat(const FGameplayTag& InTag, float InValue)
{
	SetValue(InTag, InValue);
}

void UWjWorldGameDataComponent::SetValueString(const FGameplayTag& InTag, const FString& InValue)
{
	SetValue(InTag, InValue);
}

void UWjWorldGameDataComponent::RemoveValueInt(const FGameplayTag& InTag)
{
	RemoveValue<int32>(InTag);
}

void UWjWorldGameDataComponent::RemoveValueFloat(const FGameplayTag& InTag)
{
	RemoveValue<float>(InTag);
}

void UWjWorldGameDataComponent::RemoveValueString(const FGameplayTag& InTag)
{
	RemoveValue<FString>(InTag);
}


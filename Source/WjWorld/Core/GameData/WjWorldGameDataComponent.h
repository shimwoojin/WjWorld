// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include <type_traits>
#include "WjWorldGameDataComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WJWORLD_API UWjWorldGameDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	template<typename T>
	const T* GetValue(const FGameplayTag& InTag) const;

	void GetValueInt(const FGameplayTag& InTag, int32& OutValue, bool& bFound) const;
	void GetValueFloat(const FGameplayTag& InTag, float& OutValue, bool& bFound) const;
	void GetValueString(const FGameplayTag& InTag, FString& OutValue, bool& bFound) const;

	template<typename T>
	void SetValue(const FGameplayTag& InTag, const T& InValue);

	void SetValueInt(const FGameplayTag& InTag, int32 InValue);
	void SetValueFloat(const FGameplayTag& InTag, float InValue);
	void SetValueString(const FGameplayTag& InTag, const FString& InValue);

	template<typename T>
	void RemoveValue(const FGameplayTag& InTag);

	void RemoveValueInt(const FGameplayTag& InTag);
	void RemoveValueFloat(const FGameplayTag& InTag);
	void RemoveValueString(const FGameplayTag& InTag);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, int32> IntDataMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, float> FloatDataMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, FString> StringDataMap;
};

template<typename T>
inline const T* UWjWorldGameDataComponent::GetValue(const FGameplayTag& InTag) const
{
	if constexpr (std::is_same_v<T, int32>)
	{
		if (IntDataMap.Contains(InTag))
		{
			return &IntDataMap[InTag];
		}
	}
	else if constexpr (std::is_same_v<T, float>)
	{
		if (FloatDataMap.Contains(InTag))
		{
			return &FloatDataMap[InTag];
		}
	}
	else if constexpr (std::is_same_v<T, FString>)
	{
		if (StringDataMap.Contains(InTag))
		{
			return &StringDataMap[InTag];
		}
	}

	return nullptr;
}

template<typename T>
inline void UWjWorldGameDataComponent::SetValue(const FGameplayTag& InTag, const T& InValue)
{
	if constexpr (std::is_same_v<T, int32>)
	{
		IntDataMap.Add(InTag, InValue);
	}
	else if constexpr (std::is_same_v<T, float>)
	{
		FloatDataMap.Add(InTag, InValue);
	}
	else if constexpr (std::is_same_v<T, FString>)
	{
		StringDataMap.Add(InTag, InValue);
	}
}

template<typename T>
inline void UWjWorldGameDataComponent::RemoveValue(const FGameplayTag& InTag)
{
	if constexpr (std::is_same_v<T, int32>)
	{
		IntDataMap.Remove(InTag);
	}
	else if constexpr (std::is_same_v<T, float>)
	{
		FloatDataMap.Remove(InTag);
	}
	else if constexpr (std::is_same_v<T, FString>)
	{
		StringDataMap.Remove(InTag);
	}
}

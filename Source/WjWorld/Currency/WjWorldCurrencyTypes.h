// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WjWorldCurrencyTypes.generated.h"

/**
 * 재화 종류
 * Coin = 인게임 플레이로 획득 (무료 재화)
 * Gem = Steam 결제로 구매 (유료 재화)
 */
UENUM(BlueprintType)
enum class ECurrencyType : uint8
{
	Coin	UMETA(DisplayName = "Coin"),
	Gem		UMETA(DisplayName = "Gem"),
};

/**
 * 재화 잔액 정보
 */
USTRUCT(BlueprintType)
struct WJWORLD_API FCurrencyBalance
{
	GENERATED_BODY()

	FCurrencyBalance() = default;

	FCurrencyBalance(ECurrencyType InType, int32 InAmount)
		: Type(InType), Amount(InAmount)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECurrencyType Type = ECurrencyType::Coin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;
};

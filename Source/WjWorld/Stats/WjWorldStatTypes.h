// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WjWorldStatTypes.generated.h"

/**
 * 미니게임별 스탯 이름 상수
 * Steam User Stats에 등록된 이름과 일치해야 함
 */
namespace WjWorldStats
{
	namespace ApproachingWall
	{
		inline const FName Wins("AW_Wins");
		inline const FName Losses("AW_Losses");
		inline const FName Kills("AW_Kills");
		inline const FName GamesPlayed("AW_GamesPlayed");
	}
	// 향후 미니게임 추가 시 새 namespace 추가
}

/**
 * 개별 스탯 항목 (UI 표시용)
 */
USTRUCT(BlueprintType)
struct FMinigameStatEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName StatName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;
};

/**
 * 미니게임 스탯 디스크립터
 * UI가 이 디스크립터를 순회하며 자동으로 섹션을 생성
 */
USTRUCT(BlueprintType)
struct FMinigameStatDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText MinigameName;

	/** FRoomSettings.GameMode과 매칭되는 ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString GameModeId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FMinigameStatEntry> Stats;
};

/** 등록된 전체 미니게임 디스크립터 반환 */
const TArray<FMinigameStatDescriptor>& GetAllMinigameDescriptors();

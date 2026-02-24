// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Core/GameRule/WjWorldGameRuleBase.h"
#include "WjWorldMinigameDataAsset.generated.h"

/**
 * 미니게임 맵/변형 옵션 (게임모드별)
 */
USTRUCT(BlueprintType)
struct FWjWorldMinigameMapOption
{
	GENERATED_BODY()

	/** UI 표시명 (예: "기본 배치", "랜덤") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minigame")
	FText DisplayName;

	/** 내부 값 (예: WallName "Map1", 또는 빈 문자열 for Random) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minigame")
	FString OptionValue;
};

/**
 * 미니게임 정의
 */
USTRUCT(BlueprintType)
struct FWjWorldMinigameDefinition
{
	GENERATED_BODY()

	/** UI 표시명 (예: "Approaching Wall") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minigame")
	FText DisplayName;

	/** 내부 ID (예: "ApproachingWall") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minigame")
	FName GameModeId;

	/** ServerTravel 경로 (예: "/Game/Map/03-1_ApproachingWall") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minigame")
	FString LevelPath;

	/** 게임 규칙 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minigame")
	TSubclassOf<UWjWorldGameRuleBase> GameRuleClass;

	/** 맵 옵션 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minigame")
	TArray<FWjWorldMinigameMapOption> MapOptions;

	/** 허용된 어빌리티 태그 (빈 컨테이너 = 전부 허용, 하위 호환) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minigame")
	FGameplayTagContainer AllowedAbilityTags;

	/** 스탯 네임스페이스 (예: "AW", "Sumo") - 스탯 키 접두사로 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minigame")
	FName StatNamespace;

	/** 최소 플레이어 수 (이 미만이면 게임 시작 불가, 기본 1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minigame", meta = (ClampMin = "1"))
	int32 MinimumPlayers = 1;
};

/**
 * 미니게임 카탈로그 데이터 에셋
 *
 * 모든 미니게임 정의를 포함하며, CreateRoomWindow와 WaitingRoom에서 참조.
 */
UCLASS(BlueprintType)
class WJWORLD_API UWjWorldMinigameDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 등록된 미니게임 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minigame")
	TArray<FWjWorldMinigameDefinition> Minigames;

	/** GameModeId로 미니게임 정의 찾기 */
	const FWjWorldMinigameDefinition* FindByGameModeId(FName GameModeId) const;
};

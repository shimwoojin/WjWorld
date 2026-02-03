// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WjWorldLayoutSaveGame.generated.h"

/**
 * 배치된 오브젝트 저장 항목
 */
USTRUCT()
struct FPlacedObjectSaveEntry
{
	GENERATED_BODY()

	/** 오브젝트 ID (카탈로그 참조) */
	UPROPERTY()
	FName ObjectId;

	/** 월드 트랜스폼 */
	UPROPERTY()
	FTransform Transform;
};

/**
 * 로비 오브젝트 배치 레이아웃 세이브 데이터
 */
UCLASS()
class WJWORLD_API UWjWorldLayoutSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** 배치된 오브젝트 목록 */
	UPROPERTY()
	TArray<FPlacedObjectSaveEntry> PlacedObjects;

	/** 세이브 버전 */
	UPROPERTY()
	int32 SaveVersion = 1;
};

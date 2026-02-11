// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WjWorldPlacementTypes.generated.h"

/**
 * 배치 컨텍스트 - 어떤 환경에서 배치하는지 구분
 */
UENUM(BlueprintType)
enum class EPlacementContext : uint8
{
	None,
	Lobby,              // 로비 데코레이션
	ApproachingWall,    // Approaching Wall 벽 레이아웃 + 맵 데코
	JumpMap             // JumpMap 플랫폼 + 장애물 + 함정
};

/**
 * 컨텍스트별 SaveSlot 이름 반환
 */
inline FString GetSaveSlotNameForContext(EPlacementContext Context)
{
	switch (Context)
	{
	case EPlacementContext::Lobby:
		return TEXT("LobbyLayout");
	case EPlacementContext::ApproachingWall:
		return TEXT("ApproachingWallLayout");
	case EPlacementContext::JumpMap:
		return TEXT("JumpMapLayout");
	default:
		return TEXT("DefaultLayout");
	}
}

/**
 * 컨텍스트 이름을 문자열로 반환 (디버그/로깅용)
 */
inline FString GetPlacementContextName(EPlacementContext Context)
{
	switch (Context)
	{
	case EPlacementContext::None:
		return TEXT("None");
	case EPlacementContext::Lobby:
		return TEXT("Lobby");
	case EPlacementContext::ApproachingWall:
		return TEXT("ApproachingWall");
	case EPlacementContext::JumpMap:
		return TEXT("JumpMap");
	default:
		return TEXT("Unknown");
	}
}

/**
 * AW 컨텍스트용 벽돌 그리드 설정
 */
struct FApproachingWallGridConfig
{
	/** 벽돌 크기 (기본값 100x100x100) */
	FVector BrickSize = FVector(100.0, 100.0, 100.0);

	/** 그리드 원점 (중앙 기준) */
	FVector GridOrigin = FVector::ZeroVector;

	/** 최대 그리드 크기 */
	int32 MaxGridRows = 20;
	int32 MaxGridColumns = 20;
};

/**
 * ObjectId를 CSV 벽돌 타입 값으로 변환
 * @return -1=Empty, 1=Standard, 2=Explosive, 3=Moving, 4=Destructible
 */
inline int32 ObjectIdToBrickTypeValue(FName ObjectId)
{
	FString IdStr = ObjectId.ToString();

	if (IdStr.Contains(TEXT("Standard")))
	{
		return 1;
	}
	else if (IdStr.Contains(TEXT("Explosive")))
	{
		return 2;
	}
	else if (IdStr.Contains(TEXT("Moving")))
	{
		return 3;
	}
	else if (IdStr.Contains(TEXT("Destructible")))
	{
		return 4;
	}

	// 기본값: Standard
	return 1;
}

/**
 * 유저 벽 레이아웃 디렉토리 경로 반환
 */
inline FString GetUserWallLayoutDirectory()
{
	// 상대 경로를 절대 경로로 변환 (독립 실행 빌드에서 작업 디렉토리에 무관하게 동일 경로 보장)
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("WallLayouts") / TEXT("User"));
}

/**
 * 유저 JumpMap 레이아웃 디렉토리 경로 반환
 */
inline FString GetUserJumpMapLayoutDirectory()
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("JumpMapLayouts") / TEXT("User"));
}

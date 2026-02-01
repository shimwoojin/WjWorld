// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldWallManager.h"
#include "GamePlay/Wall/WjWorldBrickSpawner.h"

#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"

#include "GameFramework/Character.h"

#include "WjWorldLogCategories.h"

UWjWorldWallManager* UWjWorldWallManager::CreateWallManager(UObject* Outer)
{
	if (Outer)
	{
		UWjWorldWallManager* WallManager = NewObject<UWjWorldWallManager>(Outer, UWjWorldWallManager::StaticClass());
		if (WallManager)
		{
			return WallManager;
		}
	}
	return nullptr;
}

float UWjWorldWallManager::GetBrickMovementAllowedTime(int32 BrickMoveSignalCount)
{
	float DeltaTime = BrickMoveAllowedTimeMax;

	float LevelRatio = FMath::Clamp(static_cast<float>(BrickMoveSignalCount) / static_cast<float>(MaxGameLevel), 0.0f, 1.0f);
	float InverseRatio = 1.0f - LevelRatio;
	DeltaTime = FMath::Lerp(BrickMoveAllowedTimeMin, BrickMoveAllowedTimeMax, InverseRatio);

	return DeltaTime;
}

void UWjWorldWallManager::SetGameRule(UWjWorldGameRuleApproachingWall* InGameRule)
{
	GameRule = InGameRule;
}

void UWjWorldWallManager::Tick(float DeltaTime)
{
	if (bIsWallMoving == false) return;

	WallMoveElapsedSeconds += DeltaTime;

	if(WallMoveElapsedSeconds >= WallMoveDurationSeconds)
	{
		// Wall Move End 처리
		OnWallMoveEnd();
		WallMoveElapsedSeconds = 0.0f;
	}
}

void UWjWorldWallManager::WallMoveStart(float WallMoveSeconds)
{
	WallMoveDurationSeconds = WallMoveSeconds;
	bIsWallMoving = true;
	WallMoveElapsedSeconds = 0.0f;
}

void UWjWorldWallManager::OnWallMoveEnd()
{
	bIsWallMoving = false;
	PushCharactersInSafeZone();
}

void UWjWorldWallManager::PushCharactersInSafeZone()
{
	if (!GameRule.IsValid()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const TSet<FIntPoint>& SafeZonePoints = GameRule->GetCurrentSafeZonePoints();
	const FWjWorldWallDescription& WallDesc = GameRule->GetWallDesc();

	if (SafeZonePoints.Num() == 0) return;

	// 모든 플레이어 확인
	for (auto Iter = World->GetPlayerControllerIterator(); Iter; ++Iter)
	{
		APlayerController* PC = Iter->Get();
		if (!PC) continue;

		ACharacter* Character = Cast<ACharacter>(PC->GetPawn());
		if (!Character) continue;

		FVector CharLocation = Character->GetActorLocation();

		// 현재 위치의 그리드 인덱스 계산
		FIntPoint CharGridIndex = UWjWorldBrickSpawner::CalculateBrickGridIndex(
			CharLocation,
			WallDesc.ColumnNum,
			WallDesc.RowNum,
			WallDesc.CenterOffset,
			WallDesc.BrickSize
		);

		// 안전 구역 안에 있으면 스킵
		if (SafeZonePoints.Contains(CharGridIndex))
		{
			continue;
		}

		// 안전 구역 밖에 있으면 가장 가까운 안전 구역으로 이동
		float MinDistSq = MAX_FLT;
		FIntPoint ClosestSafePoint = FIntPoint::ZeroValue;

		for (const FIntPoint& SafePoint : SafeZonePoints)
		{
			FVector SafeWorldPos = UWjWorldBrickSpawner::CalculateBrickPosition(
				SafePoint.X,
				SafePoint.Y,
				WallDesc.ColumnNum,
				WallDesc.RowNum,
				WallDesc.CenterOffset,
				WallDesc.BrickSize
			);

			float DistSq = FVector::DistSquared(CharLocation, SafeWorldPos);
			if (DistSq < MinDistSq)
			{
				MinDistSq = DistSq;
				ClosestSafePoint = SafePoint;
			}
		}

		FVector TargetLocation = UWjWorldBrickSpawner::CalculateBrickPosition(
			ClosestSafePoint.X,
			ClosestSafePoint.Y,
			WallDesc.ColumnNum,
			WallDesc.RowNum,
			WallDesc.CenterOffset,
			WallDesc.BrickSize
		);

		// Z축은 원래 캐릭터 위치 유지
		TargetLocation.Z = CharLocation.Z;

		Character->SetActorLocation(TargetLocation);

		UE_LOG(LogWjWorld, Log, TEXT("PushCharactersInSafeZone: Pushed character from grid (%d,%d) to (%d,%d)"),
			CharGridIndex.X, CharGridIndex.Y, ClosestSafePoint.X, ClosestSafePoint.Y);
	}
}

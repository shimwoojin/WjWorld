// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldBrickMovement.h"
#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/Wall/WjWorldBrickSpawner.h"
#include "GamePlay/Wall/WjWorldBrickActor.h"
#include "GamePlay/Wall/WjWorldWallManager.h"

#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"

#include "Kismet/KismetSystemLibrary.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

#include "WjWorldLogCategories.h"

#include "Engine/OverlapResult.h"

void UWjWorldBrickMovement::Initialize(UWjWorldBrickComponent* InBrickComponent, UWjWorldGameRuleApproachingWall* InGameRule)
{
	BrickComponent = InBrickComponent;
	GameRule = InGameRule;
	WallManger = InGameRule ? InGameRule->GetWallManager() : nullptr;

	if(BrickComponent.IsValid())
	{
		CurrentGridPosition = BrickComponent->GetBrickProperties().SpawnedGridPosition;
		TargetActor = BrickComponent->GetOwner();
	}
}

void UWjWorldBrickMovement::Tick(float DeltaTime)
{
	if (bIsMoving == false) return;

	MoveElapsedTime += DeltaTime;

	FVector NewLocation = FVector::ZeroVector;
	FVector BoxExtent = BrickComponent.IsValid() ? BrickComponent->GetBrickProperties().Size : FVector::ZeroVector;

	bool bWallManagerMoving = WallManger.IsValid() ? WallManger->IsWallMoving() : false;

	if (MoveElapsedTime >= MoveAllowTime
		|| bWallManagerMoving == false)
	{
		NewLocation = EndLocation;
		bIsMoving = false;
		MoveElapsedTime = 0.0f;
		if (TargetActor.IsValid())
		{
			TargetActor->SetActorLocation(NewLocation);
		}
	}
	else
	{
		float Alpha = MoveElapsedTime / MoveAllowTime;
		NewLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
		if (TargetActor.IsValid())
		{
			TargetActor->SetActorLocation(NewLocation);
		}
	}

	// Standard 타입 벽돌만 다른 벽돌과의 충돌 체크
	if (BrickComponent.IsValid() && BrickComponent->GetBrickProperties().BrickType == EWjWorldBrickType::Standard)
	{
		CheckCollisionWithOtherBricks();
	}

	// 이동 중인 벽돌이 Pawn과 겹치면 이동 방향으로 밀어냄
	if (BrickComponent.IsValid() && bIsMoving)
	{
		FVector BrickHalfSize = BoxExtent * 0.5f;

		TArray<FOverlapResult> PawnOverlaps;
		FCollisionShape PawnCheckShape = FCollisionShape::MakeBox(BrickHalfSize);

		if (GetWorld()->OverlapMultiByChannel(
			PawnOverlaps,
			NewLocation,
			FQuat::Identity,
			ECC_Pawn,
			PawnCheckShape))
		{
			for (const FOverlapResult& Overlap : PawnOverlaps)
			{
				ACharacter* Character = Cast<ACharacter>(Overlap.GetActor());
				if (!Character) continue;

				float CapsuleRadius = Character->GetCapsuleComponent()->GetScaledCapsuleRadius();

				FVector MoveDir = MovementVector.GetSafeNormal();
				if (MoveDir.IsNearlyZero()) continue;

				// 벽돌 반 크기 + 캡슐 반지름 + 여유값
				float PushDistance = 0.0f;
				if (FMath::Abs(MoveDir.X) > FMath::Abs(MoveDir.Y))
				{
					PushDistance = BrickHalfSize.X + CapsuleRadius + 5.0f;
				}
				else
				{
					PushDistance = BrickHalfSize.Y + CapsuleRadius + 5.0f;
				}

				FVector PushTarget = NewLocation + MoveDir * PushDistance;
				PushTarget.Z = Character->GetActorLocation().Z; // Z축 유지
				Character->SetActorLocation(PushTarget);
			}
		}
	}
}

void UWjWorldBrickMovement::MoveBrick(float InMoveAllowTime)
{
	if (!BrickComponent.IsValid()) return;

	bIsMoving = true;

	TargetActor = BrickComponent->GetOwner();
	StartLocation = BrickComponent->GetComponentLocation();
	EndLocation = StartLocation + GetMovementVector(true);
	MoveAllowTime = InMoveAllowTime;
	MoveElapsedTime = 0.0f;

	if(StartLocation.Equals(EndLocation))
	{
		bIsMoving = false;
		BrickComponent->ReserveDestroyBrick(InMoveAllowTime);
	}
}

FVector UWjWorldBrickMovement::GetMovementVector(bool bIsNew)
{
	if (!BrickComponent.IsValid()) return FVector();
	
	if (bIsNew)
	{
		LastMovementVector = MovementVector;
	}

	TArray<EWjWorldBrickMovementDirection> MovementDirs = GetNextDirections();
	const FWjWorldBrickProperties& BrickProperties = BrickComponent->GetBrickProperties();

	for(EWjWorldBrickMovementDirection Direction : MovementDirs)
	{
		switch (Direction)
		{
		case EWjWorldBrickMovementDirection::Up:
			++CurrentGridPosition.Y;
			break;
		case EWjWorldBrickMovementDirection::Right:
			++CurrentGridPosition.X;
			break;
		case EWjWorldBrickMovementDirection::Down:
			--CurrentGridPosition.Y;
			break;
		case EWjWorldBrickMovementDirection::Left:
			--CurrentGridPosition.X;
			break;
		}
	}

	EndLocation = UWjWorldBrickSpawner::CalculateBrickPosition
	(
		CurrentGridPosition.X,
		CurrentGridPosition.Y,
		BrickProperties.ColumnNum,
		BrickProperties.RowNum,
		BrickProperties.CenterOffset,
		BrickProperties.Size
	);

	MovementVector = EndLocation - StartLocation;

	return MovementVector;
}

TArray<EWjWorldBrickMovementDirection> UWjWorldBrickMovement::GetNextDirections()
{
	if (!BrickComponent.IsValid()) return TArray<EWjWorldBrickMovementDirection>();
	if (!GameRule.IsValid()) return TArray<EWjWorldBrickMovementDirection>();

	TArray<EWjWorldBrickMovementDirection> Result;
	const TSet<FIntPoint>& FloodFillPoints = GameRule->GetFloodFillPoints();

	struct FPointWithDirection
	{
		FIntPoint Point;
		EWjWorldBrickMovementDirection Direction;
	};

	TArray<FPointWithDirection> CheckPoints = {
		{ FIntPoint(CurrentGridPosition.X + 1, CurrentGridPosition.Y), EWjWorldBrickMovementDirection::Right },
		{ FIntPoint(CurrentGridPosition.X - 1, CurrentGridPosition.Y), EWjWorldBrickMovementDirection::Left },
		{ FIntPoint(CurrentGridPosition.X, CurrentGridPosition.Y + 1), EWjWorldBrickMovementDirection::Up },
		{ FIntPoint(CurrentGridPosition.X, CurrentGridPosition.Y - 1), EWjWorldBrickMovementDirection::Down }
	};

	for (const auto& CheckPoint : CheckPoints)
	{
		if (FloodFillPoints.Contains(CheckPoint.Point))
		{
			Result.Add(CheckPoint.Direction);
		}
	}

	return Result;
}

void UWjWorldBrickMovement::PushBrick(const FVector& Direction, float InMoveAllowTime)
{
	if (!BrickComponent.IsValid()) return;
	if (bIsMoving) return; // 이미 이동 중이면 무시

	// 연쇄 밀림 체크: 밀릴 수 있는지, 밀어야 할 벽돌 목록 수집
	TArray<UWjWorldBrickComponent*> BricksToMove;
	bool bCanPush = CanPushInDirection(Direction, BricksToMove);

	if (!bCanPush)
	{
		// 막혀서 밀 수 없음 -> 파괴
		UE_LOG(LogWjWorld, Log, TEXT("PushBrick blocked, destroying brick at %s"), *BrickComponent->GetComponentLocation().ToString());
		BrickComponent->ReserveDestroyBrick(0.0f);
		return;
	}

	// 연쇄 밀림: 뒤에서부터 밀어야 충돌 안 함
	for (int32 i = BricksToMove.Num() - 1; i >= 0; --i)
	{
		UWjWorldBrickComponent* BrickToMove = BricksToMove[i];
		if (BrickToMove && BrickToMove != BrickComponent.Get())
		{
			if (UWjWorldBrickMovement* OtherMovement = BrickToMove->GetBrickMovement())
			{
				OtherMovement->ExecutePush(Direction, InMoveAllowTime);
			}
		}
	}

	// 자신도 밀림
	ExecutePush(Direction, InMoveAllowTime);
}

void UWjWorldBrickMovement::ExecutePush(const FVector& Direction, float InMoveAllowTime)
{
	if (!BrickComponent.IsValid()) return;
	if (bIsMoving) return;

	bIsMoving = true;

	TargetActor = BrickComponent->GetOwner();
	StartLocation = BrickComponent->GetComponentLocation();

	// Direction을 정규화하고 벽돌 크기만큼 이동
	FVector NormalizedDir = Direction.GetSafeNormal();
	const FWjWorldBrickProperties& BrickProperties = BrickComponent->GetBrickProperties();

	// 이동 방향에 따라 적절한 거리 계산
	FVector MoveDistance = FVector::ZeroVector;
	if (FMath::Abs(NormalizedDir.X) > FMath::Abs(NormalizedDir.Y))
	{
		MoveDistance.X = FMath::Sign(NormalizedDir.X) * BrickProperties.Size.X;
	}
	else
	{
		MoveDistance.Y = FMath::Sign(NormalizedDir.Y) * BrickProperties.Size.Y;
	}

	EndLocation = StartLocation + MoveDistance;
	MoveAllowTime = InMoveAllowTime;
	MoveElapsedTime = 0.0f;
	MovementVector = MoveDistance;

	UE_LOG(LogWjWorld, Log, TEXT("ExecutePush: %s -> %s"), *StartLocation.ToString(), *EndLocation.ToString());
}

bool UWjWorldBrickMovement::CanPushInDirection(const FVector& Direction, TArray<UWjWorldBrickComponent*>& OutBricksToMove)
{
	if (!BrickComponent.IsValid()) return false;

	const FWjWorldBrickProperties& BrickProperties = BrickComponent->GetBrickProperties();
	FVector CurrentCheckLocation = BrickComponent->GetComponentLocation();

	// Direction을 정규화하고 벽돌 크기만큼 이동 거리 계산
	FVector NormalizedDir = Direction.GetSafeNormal();
	FVector MoveDistance = FVector::ZeroVector;
	if (FMath::Abs(NormalizedDir.X) > FMath::Abs(NormalizedDir.Y))
	{
		MoveDistance.X = FMath::Sign(NormalizedDir.X) * BrickProperties.Size.X;
	}
	else
	{
		MoveDistance.Y = FMath::Sign(NormalizedDir.Y) * BrickProperties.Size.Y;
	}

	// 자신을 목록에 추가
	OutBricksToMove.Add(BrickComponent.Get());

	// 다음 위치 체크
	FVector NextLocation = CurrentCheckLocation + MoveDistance;

	// 최대 체크 횟수 (무한 루프 방지)
	const int32 MaxChainLength = 20;
	int32 ChainCount = 0;

	while (ChainCount < MaxChainLength)
	{
		++ChainCount;

		// 다음 위치가 막혀있는지 체크 (Standard 벽돌, 맵 경계 등)
		if (IsLocationBlocked(NextLocation))
		{
			return false; // 막혀있으면 밀 수 없음
		}

		// 다음 위치에 Moving 타입 벽돌이 있는지 체크
		UWjWorldBrickComponent* BrickAtNext = FindBrickAtLocation(NextLocation);
		if (BrickAtNext)
		{
			if (BrickAtNext->GetBrickProperties().BrickType == EWjWorldBrickType::Moving)
			{
				// Moving 벽돌 발견 -> 연쇄 밀림 대상에 추가
				OutBricksToMove.Add(BrickAtNext);
				NextLocation = NextLocation + MoveDistance;
				continue;
			}
			else if (BrickAtNext->GetBrickProperties().BrickType == EWjWorldBrickType::Standard)
			{
				// Standard 벽돌에 막힘
				return false;
			}
			else
			{
				// Destructible, Explosive 등은 밀 수 있음 (해당 벽돌은 파괴됨)
				break;
			}
		}
		else
		{
			// 빈 공간 -> 밀 수 있음
			break;
		}
	}

	return true;
}

void UWjWorldBrickMovement::CheckCollisionWithOtherBricks()
{
	if (!BrickComponent.IsValid()) return;
	if (!TargetActor.IsValid()) return;

	const FWjWorldBrickProperties& BrickProperties = BrickComponent->GetBrickProperties();
	const FVector CurrentLocation = TargetActor->GetActorLocation();
	const FVector HalfSize = BrickProperties.Size * 0.4f; // 약간 작게 체크

	TArray<FOverlapResult> Overlaps;
	FCollisionShape CollisionShape = FCollisionShape::MakeBox(HalfSize);

	if (GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		CurrentLocation,
		FQuat::Identity,
		FCollisionObjectQueryParams::AllObjects,
		CollisionShape))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AWjWorldBrickActor* OtherBrickActor = Cast<AWjWorldBrickActor>(Overlap.GetActor());
			if (!OtherBrickActor) continue;
			if (OtherBrickActor == TargetActor.Get()) continue; // 자기 자신 제외

			UWjWorldBrickComponent* OtherBrickComponent = OtherBrickActor->GetBrickComponent();
			if (!OtherBrickComponent) continue;

			// Standard 타입이 아닌 벽돌만 충돌 처리
			if (OtherBrickComponent->GetBrickProperties().BrickType != EWjWorldBrickType::Standard)
			{
				NotifyCollisionToBrick(OtherBrickComponent, MovementVector);
			}
		}
	}
}

void UWjWorldBrickMovement::NotifyCollisionToBrick(UWjWorldBrickComponent* OtherBrick, const FVector& Direction)
{
	if (!OtherBrick) return;

	OtherBrick->HandleWallCollision(Direction);
}

UWjWorldBrickComponent* UWjWorldBrickMovement::FindBrickAtLocation(const FVector& Location)
{
	if (!BrickComponent.IsValid()) return nullptr;

	const FWjWorldBrickProperties& BrickProperties = BrickComponent->GetBrickProperties();
	const FVector HalfSize = BrickProperties.Size * 0.3f; // 약간 작게 체크

	TArray<FOverlapResult> Overlaps;
	FCollisionShape CollisionShape = FCollisionShape::MakeBox(HalfSize);

	if (GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		Location,
		FQuat::Identity,
		FCollisionObjectQueryParams::AllObjects,
		CollisionShape))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AWjWorldBrickActor* BrickActor = Cast<AWjWorldBrickActor>(Overlap.GetActor());
			if (!BrickActor) continue;
			if (BrickActor == BrickComponent->GetOwner()) continue; // 자기 자신 제외

			UWjWorldBrickComponent* FoundBrick = BrickActor->GetBrickComponent();
			if (FoundBrick)
			{
				return FoundBrick;
			}
		}
	}

	return nullptr;
}

bool UWjWorldBrickMovement::IsLocationBlocked(const FVector& Location)
{
	if (!BrickComponent.IsValid()) return true;
	if (!GameRule.IsValid()) return true;

	const FWjWorldBrickProperties& BrickProperties = BrickComponent->GetBrickProperties();

	// 그리드 인덱스로 변환하여 SafeZone 범위 체크
	FIntPoint GridIndex = UWjWorldBrickSpawner::CalculateBrickGridIndex(
		Location,
		BrickProperties.ColumnNum,
		BrickProperties.RowNum,
		BrickProperties.CenterOffset,
		BrickProperties.Size
	);

	// 맵 경계 체크
	if (GridIndex.X < 0 || GridIndex.X >= BrickProperties.ColumnNum ||
		GridIndex.Y < 0 || GridIndex.Y >= BrickProperties.RowNum)
	{
		return true; // 맵 밖으로 나가면 막힘
	}

	// 해당 위치에 Standard 벽돌이 있는지 체크
	UWjWorldBrickComponent* BrickAtLocation = FindBrickAtLocation(Location);
	if (BrickAtLocation && BrickAtLocation->GetBrickProperties().BrickType == EWjWorldBrickType::Standard)
	{
		return true; // Standard 벽돌에 막힘
	}

	return false;
}

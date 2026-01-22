// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldBrickMovement.h"
#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/Wall/WjWorldBrickSpawner.h"
#include "GamePlay/Wall/WjWorldWallManager.h"

#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"

#include "Kismet/KismetSystemLibrary.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

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

	//TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	//ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	//TArray<AActor*> Overlaps;
	//UKismetSystemLibrary::BoxOverlapActors(
	//	GetWorld(),
	//	NewLocation,
	//	BoxExtent,
	//	ObjectTypes,
	//	APawn::StaticClass(),
	//	{},
	//	Overlaps
	//);

	//for (AActor* OverlapActor : Overlaps)
	//{
	//	ACharacter* Character = Cast<ACharacter>(OverlapActor);
	//	if (Character == nullptr) continue;

	//	FVector NewPosition = NewLocation;
	//	float CapsuleRadius = Character->GetCapsuleComponent()->GetScaledCapsuleRadius();

	//	FVector MoveVectorTemp = MovementVector;
	//	MoveVectorTemp.Normalize();
	//	NewPosition += MoveVectorTemp * (CapsuleRadius + BoxExtent.X);

	//	Character->SetActorLocation(NewPosition);
	//}
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

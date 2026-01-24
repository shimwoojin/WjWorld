// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WjWorldBrickMovement.generated.h"

class UWjWorldBrickComponent;
class UWjWorldGameRuleApproachingWall;
class UWjWorldWallManager;

UENUM()
enum class EWjWorldBrickMovementDirection : uint8
{
	Up,
	Right,
	Down,
	Left
};

/**
 * 
 */
UCLASS()
class WJWORLD_API UWjWorldBrickMovement : public UObject
{
	GENERATED_BODY()
	
public:
	void Initialize(UWjWorldBrickComponent* InBrickComponent, UWjWorldGameRuleApproachingWall* InGameRule);

	void Tick(float DeltaTime);
	void MoveBrick(float InMoveAllowTime);
	void PushBrick(const FVector& Direction, float InMoveAllowTime);
	void ExecutePush(const FVector& Direction, float InMoveAllowTime);

	virtual FVector GetMovementVector(bool bIsNew = true);
	virtual TArray<EWjWorldBrickMovementDirection> GetNextDirections();

	FVector GetLastMovementVector() const { return LastMovementVector; }
	bool IsMoving() const { return bIsMoving; }

	// 연쇄 밀림 체크: 해당 방향으로 밀릴 수 있는지 확인 (재귀적으로 체크)
	bool CanPushInDirection(const FVector& Direction, TArray<UWjWorldBrickComponent*>& OutBricksToMove);

private:
	void CheckCollisionWithOtherBricks();
	void NotifyCollisionToBrick(class UWjWorldBrickComponent* OtherBrick, const FVector& Direction);
	UWjWorldBrickComponent* FindBrickAtLocation(const FVector& Location);
	bool IsLocationBlocked(const FVector& Location);

private:
	UPROPERTY()
	TWeakObjectPtr<UWjWorldBrickComponent> BrickComponent;

	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;

	UPROPERTY()
	TWeakObjectPtr<UWjWorldGameRuleApproachingWall> GameRule;

	UPROPERTY()
	TWeakObjectPtr<UWjWorldWallManager> WallManger;

	FIntPoint CurrentGridPosition;

	FVector StartLocation;
	FVector EndLocation;

	FVector MovementVector;
	FVector LastMovementVector;

	float MoveAllowTime = 1.0f;
	float MoveElapsedTime = 0.0f;

	bool bIsMoving = false;
};

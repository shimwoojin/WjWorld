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

	virtual FVector GetMovementVector(bool bIsNew = true);
	virtual TArray<EWjWorldBrickMovementDirection> GetNextDirections();

	FVector GetLastMovementVector() const { return LastMovementVector; }

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

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WjWorldBrickMovement.generated.h"

class UWjWorldBrickComponent;

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
	virtual FVector GetMovementVector(UWjWorldBrickComponent* BrickComponent, bool IsNew = true);
	virtual EWjWorldBrickMovementDirection GetNextDirection(UWjWorldBrickComponent* BrickComponent, bool IsNew = true);

private:
	FVector MovementVector;
	FVector LastMovementVector;

	EWjWorldBrickMovementDirection MovementDirection;
	EWjWorldBrickMovementDirection LastMovementDirection;
};

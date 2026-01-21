// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldBrickMovement.h"
#include "GamePlay/Wall/WjWorldBrickComponent.h"

FVector UWjWorldBrickMovement::GetMovementVector(UWjWorldBrickComponent* BrickComponent, bool IsNew)
{
	if (!BrickComponent) return FVector();

	if (IsNew)
	{
		LastMovementVector = MovementVector;
	}

	return MovementVector;
}

EWjWorldBrickMovementDirection UWjWorldBrickMovement::GetNextDirection(UWjWorldBrickComponent* BrickComponent, bool IsNew)
{
	if (!BrickComponent) return EWjWorldBrickMovementDirection();

	if (IsNew)
	{
		LastMovementDirection = MovementDirection;
	}

	return MovementDirection;
}

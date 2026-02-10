// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapMovingPlatformActor.h"

AJumpMapMovingPlatformActor::AJumpMapMovingPlatformActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AJumpMapMovingPlatformActor::BeginPlay()
{
	Super::BeginPlay();

	OriginalLocation = GetActorLocation();
	TargetLocation = OriginalLocation + MoveOffset;
}

void AJumpMapMovingPlatformActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 정지 상태 처리
	if (bIsPaused)
	{
		PauseTimer -= DeltaTime;
		if (PauseTimer <= 0.f)
		{
			bIsPaused = false;
			bMovingToTarget = !bMovingToTarget;
		}
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector& Destination = bMovingToTarget ? TargetLocation : OriginalLocation;

	const FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, Destination, DeltaTime, MoveSpeed);
	SetActorLocation(NewLocation);

	// 도착 체크
	if (FVector::DistSquared(NewLocation, Destination) < 1.f)
	{
		SetActorLocation(Destination);
		bIsPaused = true;
		PauseTimer = PauseTime;
	}
}

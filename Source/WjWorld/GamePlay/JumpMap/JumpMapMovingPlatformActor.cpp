// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapMovingPlatformActor.h"

AJumpMapMovingPlatformActor::AJumpMapMovingPlatformActor()
{
	JumpMapObjectId = TEXT("MovingPlatform");
	PrimaryActorTick.bCanEverTick = true;
}

void AJumpMapMovingPlatformActor::GetSerializableProperties(TMap<FString, FString>& OutProperties) const
{
	OutProperties.Add(TEXT("MoveOffset"), FString::Printf(TEXT("%.2f;%.2f;%.2f"), MoveOffset.X, MoveOffset.Y, MoveOffset.Z));
	OutProperties.Add(TEXT("MoveSpeed"), FString::SanitizeFloat(MoveSpeed));
	OutProperties.Add(TEXT("PauseTime"), FString::SanitizeFloat(PauseTime));
}

void AJumpMapMovingPlatformActor::ApplySerializedProperties(const TMap<FString, FString>& Properties)
{
	if (const FString* Value = Properties.Find(TEXT("MoveOffset")))
	{
		TArray<FString> Components;
		Value->ParseIntoArray(Components, TEXT(";"), true);
		if (Components.Num() >= 3)
		{
			MoveOffset = FVector(FCString::Atof(*Components[0]), FCString::Atof(*Components[1]), FCString::Atof(*Components[2]));
		}
	}
	if (const FString* Value = Properties.Find(TEXT("MoveSpeed")))
	{
		MoveSpeed = FCString::Atof(**Value);
	}
	if (const FString* Value = Properties.Find(TEXT("PauseTime")))
	{
		PauseTime = FCString::Atof(**Value);
	}
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

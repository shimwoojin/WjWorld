// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapMovingPlatformActor.h"
#include "Net/UnrealNetwork.h"

AJumpMapMovingPlatformActor::AJumpMapMovingPlatformActor()
{
	JumpMapObjectId = TEXT("MovingPlatform");
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
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

void AJumpMapMovingPlatformActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AJumpMapMovingPlatformActor, ServerElapsedTime);
}

void AJumpMapMovingPlatformActor::BeginPlay()
{
	Super::BeginPlay();

	OriginalLocation = GetActorLocation();
	TargetLocation = OriginalLocation + MoveOffset;

	// 이동 시간 및 주기 캐시
	float Distance = MoveOffset.Size();
	TravelTime = (MoveSpeed > 0.f) ? (Distance / MoveSpeed) : 0.f;
	CycleTime = TravelTime * 2.f + PauseTime * 2.f;
}

void AJumpMapMovingPlatformActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		// 서버: 경과 시간 누적
		ServerElapsedTime += DeltaTime;
	}

	// 서버/클라이언트 모두 동일한 시간 기반 위치 계산
	if (CycleTime > 0.f)
	{
		FVector NewLocation = CalculatePositionFromTime(ServerElapsedTime);
		SetActorLocation(NewLocation);
	}
}

FVector AJumpMapMovingPlatformActor::CalculatePositionFromTime(float Time) const
{
	if (CycleTime <= 0.f)
	{
		return OriginalLocation;
	}

	// 주기 내 현재 위상
	float Phase = FMath::Fmod(Time, CycleTime);
	if (Phase < 0.f) Phase += CycleTime;

	float Alpha = 0.f;

	if (Phase < TravelTime)
	{
		// 원점 → 타겟 이동 중
		Alpha = (TravelTime > 0.f) ? (Phase / TravelTime) : 1.f;
	}
	else if (Phase < TravelTime + PauseTime)
	{
		// 타겟에서 정지 중
		Alpha = 1.f;
	}
	else if (Phase < TravelTime * 2.f + PauseTime)
	{
		// 타겟 → 원점 이동 중
		float ReturnPhase = Phase - TravelTime - PauseTime;
		Alpha = (TravelTime > 0.f) ? (1.f - ReturnPhase / TravelTime) : 0.f;
	}
	else
	{
		// 원점에서 정지 중
		Alpha = 0.f;
	}

	return FMath::Lerp(OriginalLocation, TargetLocation, Alpha);
}

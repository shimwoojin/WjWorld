// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapMovingPlatformActor.h"
#include "Net/UnrealNetwork.h"

AJumpMapMovingPlatformActor::AJumpMapMovingPlatformActor()
{
	JumpMapObjectId = TEXT("MovingPlatform");
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);
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
	DOREPLIFETIME_CONDITION(AJumpMapMovingPlatformActor, OriginalLocation, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AJumpMapMovingPlatformActor, MoveOffset, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AJumpMapMovingPlatformActor, MoveSpeed, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AJumpMapMovingPlatformActor, PauseTime, COND_InitialOnly);
}

void AJumpMapMovingPlatformActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// 서버: 스폰 직후이므로 GetActorLocation()이 원래 스폰 위치
		OriginalLocation = GetActorLocation();
		RecalculateTimingCache();
	}
	// 클라이언트: OriginalLocation은 리플리케이션으로 수신 → OnRep_OriginalLocation에서 처리
}

void AJumpMapMovingPlatformActor::OnRep_OriginalLocation()
{
	RecalculateTimingCache();
}

void AJumpMapMovingPlatformActor::RecalculateTimingCache()
{
	TargetLocation = OriginalLocation + MoveOffset;

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

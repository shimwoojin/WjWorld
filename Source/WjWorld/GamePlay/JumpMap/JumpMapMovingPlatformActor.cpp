// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapMovingPlatformActor.h"
#include "Net/UnrealNetwork.h"

AJumpMapMovingPlatformActor::AJumpMapMovingPlatformActor()
{
	JumpMapObjectId = TEXT("MovingPlatform");
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// 커스텀 동기화 (ServerElapsedTime + CalculatePositionFromTime) 사용
	// bReplicateMovement = true (기본값) 이면 서버의 현재 위치가 ReplicatedMovement로
	// 클라에 전달되어 OriginalLocation이 스폰 위치가 아닌 현재 위치로 캡처됨
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
	DOREPLIFETIME_CONDITION(AJumpMapMovingPlatformActor, MoveOffset, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AJumpMapMovingPlatformActor, MoveSpeed, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AJumpMapMovingPlatformActor, PauseTime, COND_InitialOnly);
}

void AJumpMapMovingPlatformActor::BeginPlay()
{
	Super::BeginPlay();

	OriginalLocation = GetActorLocation();

	if (HasAuthority())
	{
		// 서버: 프로퍼티가 이미 설정된 상태 (ApplySerializedProperties 완료)
		RecalculateTimingCache();
	}
	else
	{
		// 클라이언트: 리플리케이션 프로퍼티가 아직 도착하지 않았을 수 있음
		// Tick에서 재계산 트리거
		bNeedsTimingRecalc = true;
		RecalculateTimingCache();
	}
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
	else if (bNeedsTimingRecalc)
	{
		// 클라이언트: 리플리케이션된 프로퍼티 수신 후 재계산
		RecalculateTimingCache();
		bNeedsTimingRecalc = false;
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

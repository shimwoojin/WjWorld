// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/JumpMap/JumpMapActorBase.h"
#include "JumpMapMovingPlatformActor.generated.h"

/**
 * JumpMap 이동 플랫폼 액터
 * - 시작 위치와 목표 위치 사이를 왕복 이동
 * - 양 끝에서 잠시 정지
 * - 서버 경과 시간 리플리케이션으로 클라이언트 동기화
 */
UCLASS(Blueprintable)
class WJWORLD_API AJumpMapMovingPlatformActor : public AJumpMapActorBase
{
	GENERATED_BODY()

public:
	AJumpMapMovingPlatformActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void GetSerializableProperties(TMap<FString, FString>& OutProperties) const override;
	virtual void ApplySerializedProperties(const TMap<FString, FString>& Properties) override;

protected:
	/** 시작 위치로부터의 이동 오프셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpMap|MovingPlatform")
	FVector MoveOffset = FVector(0.f, 0.f, 300.f);

	/** 이동 속도 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpMap|MovingPlatform")
	float MoveSpeed = 200.f;

	/** 양 끝에서 정지 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpMap|MovingPlatform")
	float PauseTime = 0.5f;

private:
	/** 시간 기반 위치 계산 (서버/클라이언트 동일 함수) */
	FVector CalculatePositionFromTime(float Time) const;

	FVector OriginalLocation;
	FVector TargetLocation;

	/** 서버 경과 시간 (리플리케이션) */
	UPROPERTY(Replicated)
	float ServerElapsedTime = 0.f;

	/** 왕복 1주기 시간 (캐시) */
	float CycleTime = 0.f;

	/** 이동 1방향 소요 시간 (캐시) */
	float TravelTime = 0.f;
};

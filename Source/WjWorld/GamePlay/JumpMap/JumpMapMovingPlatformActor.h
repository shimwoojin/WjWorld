// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/JumpMap/JumpMapActorBase.h"
#include "JumpMapMovingPlatformActor.generated.h"

/**
 * JumpMap 이동 플랫폼 액터
 * - 시작 위치와 목표 위치 사이를 왕복 이동
 * - 양 끝에서 잠시 정지
 */
UCLASS(Blueprintable)
class WJWORLD_API AJumpMapMovingPlatformActor : public AJumpMapActorBase
{
	GENERATED_BODY()

public:
	AJumpMapMovingPlatformActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

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
	FVector OriginalLocation;
	FVector TargetLocation;
	bool bMovingToTarget = true;
	float PauseTimer = 0.f;
	bool bIsPaused = false;
};

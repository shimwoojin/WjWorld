// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/JumpMap/JumpMapActorBase.h"
#include "JumpMapRotatingObstacleActor.generated.h"

class UBoxComponent;

/**
 * JumpMap 회전 장애물 액터
 * - 지속적으로 회전하며 플레이어에게 넉백 또는 킬 적용
 */
UCLASS(Blueprintable)
class WJWORLD_API AJumpMapRotatingObstacleActor : public AJumpMapActorBase
{
	GENERATED_BODY()

public:
	AJumpMapRotatingObstacleActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void GetSerializableProperties(TMap<FString, FString>& OutProperties) const override;
	virtual void ApplySerializedProperties(const TMap<FString, FString>& Properties) override;

protected:
	/** 회전 속도 (도/초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "JumpMap|RotatingObstacle")
	float RotationSpeed = 90.f;

	/** 회전축 (정규화됨) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "JumpMap|RotatingObstacle")
	FRotator RotationAxis = FRotator(0.f, 1.f, 0.f);

	/** true면 킬, false면 넉백 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "JumpMap|RotatingObstacle")
	bool bKillOnHit = false;

	/** 넉백 힘 (bKillOnHit=false일 때) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "JumpMap|RotatingObstacle")
	float KnockbackForce = 800.f;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> HitTrigger;

private:
	UFUNCTION()
	void OnHitOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 서버 경과 시간 (리플리케이션) */
	UPROPERTY(Replicated)
	float ServerElapsedTime = 0.f;

	/** BeginPlay 시점의 초기 회전 (캐시) */
	FRotator InitialRotation;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/JumpMap/JumpMapActorBase.h"
#include "JumpMapCheckpointActor.generated.h"

class UBoxComponent;

/**
 * JumpMap 체크포인트 액터
 * - 플레이어가 오버랩하면 현재 체크포인트로 등록
 * - CheckpointOrder가 높을수록 후반부 체크포인트
 */
UCLASS(Blueprintable)
class WJWORLD_API AJumpMapCheckpointActor : public AJumpMapActorBase
{
	GENERATED_BODY()

public:
	AJumpMapCheckpointActor();

	virtual void GetSerializableProperties(TMap<FString, FString>& OutProperties) const override;
	virtual void ApplySerializedProperties(const TMap<FString, FString>& Properties) override;

	/** 체크포인트 순서 (0부터 시작, 높을수록 후반부) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpMap|Checkpoint")
	int32 CheckpointOrder = 0;

	/** 리스폰 위치 (체크포인트 액터 위치 기준 오프셋) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpMap|Checkpoint")
	FVector RespawnOffset = FVector(0.f, 0.f, 100.f);

	/** 리스폰 위치 반환 */
	FVector GetRespawnLocation() const { return GetActorLocation() + RespawnOffset; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> CheckpointTrigger;

private:
	UFUNCTION()
	void OnCheckpointOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SumoFloorRingActor.generated.h"

UENUM(BlueprintType)
enum class ESumoRingState : uint8
{
	Active,
	Warning,
	Destroyed
};

/**
 * Sumo 미니게임 축소 플랫폼 링 액터
 * - 동심원 링으로 배치, RingOrder가 높을수록 외곽
 * - Warning(경고 색상) → Destroyed(비활성화) 단계로 진행
 * - 라운드 리셋 시 ResetRing()으로 복원
 */
UCLASS()
class WJWORLD_API ASumoFloorRingActor : public AActor
{
	GENERATED_BODY()

public:
	ASumoFloorRingActor();

	/** 링 순서 (0=중앙 파괴불가, 높을수록 외곽) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sumo|Ring")
	int32 RingOrder = 0;

	/** 경고 시작 (색상 전환) */
	void StartWarning();

	/** 링 파괴 (Collision 비활성화 + 숨기기) */
	void DestroyRing();

	/** 라운드 리셋 시 복원 */
	void ResetRing();

	ESumoRingState GetRingState() const { return RingState; }

	/** 링의 반지름 (파워업 스폰 위치 계산용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sumo|Ring")
	float RingRadius = 500.f;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_RingState();

	void ApplyRingStateVisual();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FloorMesh;

	UPROPERTY(ReplicatedUsing = OnRep_RingState)
	ESumoRingState RingState = ESumoRingState::Active;

	/** Warning 시 적용할 머티리얼 (BP에서 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "Sumo|Ring|Visual")
	TObjectPtr<UMaterialInterface> WarningMaterial;

	/** 원래 머티리얼 (BeginPlay에서 캐시) */
	UPROPERTY()
	TObjectPtr<UMaterialInterface> DefaultMaterial;
};

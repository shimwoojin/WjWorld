// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WjTypes.h"
#include "WjWorldTileActor.generated.h"

class UBoxComponent;
class UWjWorldGameRuleApproachingWall;
class UNiagaraSystem;

UCLASS()
class WJWORLD_API AWjWorldTileActor : public AActor
{
	GENERATED_BODY()
	
	constexpr static float HitBoxSize = 5.0f;

public:	
	AWjWorldTileActor();

	void InitializeTile(const FVector& InSize, const FVector& InCenterOffset);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	// 충돌 이벤트 핸들러
	UFUNCTION()
	void OnBrickOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBrickOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool CheckBombSignalOn() const;
	void Bomb();

	UFUNCTION(NetMulticast, Unreliable)
	void SpawnBombEffect();

protected:
	UPROPERTY()
	TObjectPtr<UBoxComponent> CenterHitBoxComponent;

	UPROPERTY()
	TObjectPtr<UBoxComponent> HitBoxComponents[EWjWorldDirection::Max];

	TWeakObjectPtr<UWjWorldGameRuleApproachingWall> GameRule;

	int32 bIsOverlapBricks[EWjWorldDirection::Max];

	bool bIsBombSignalOn = false;

	float ElapsedBombingTime = 0.0f;

	UPROPERTY(EditAnywhere)
	float BombChargingTime = 3.0f;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* NiagaraSystem;
};

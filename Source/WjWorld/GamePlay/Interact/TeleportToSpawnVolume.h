// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TeleportToSpawnVolume.generated.h"

class UBoxComponent;

/**
 * 오버랩 시 PlayerStart 위치로 텔레포트하는 볼륨
 * - 로비 맵 경계 등에 배치
 * - 서버에서만 오버랩 처리 (HasAuthority)
 */
UCLASS(Blueprintable)
class WJWORLD_API ATeleportToSpawnVolume : public AActor
{
	GENERATED_BODY()

public:
	ATeleportToSpawnVolume();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

private:
	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

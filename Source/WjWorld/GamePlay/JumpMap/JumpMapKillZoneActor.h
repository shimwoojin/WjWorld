// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/JumpMap/JumpMapActorBase.h"
#include "JumpMapKillZoneActor.generated.h"

class UBoxComponent;

/**
 * JumpMap 킬존 액터
 * - 플레이어가 오버랩하면 리스폰 처리
 * - 맵 하단이나 함정 위치에 배치
 */
UCLASS(Blueprintable)
class WJWORLD_API AJumpMapKillZoneActor : public AJumpMapActorBase
{
	GENERATED_BODY()

public:
	AJumpMapKillZoneActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> KillTrigger;

private:
	UFUNCTION()
	void OnKillOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

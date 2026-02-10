// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/JumpMap/JumpMapActorBase.h"
#include "JumpMapEndActor.generated.h"

class UBoxComponent;

/**
 * JumpMap 완주 지점 액터
 * - 플레이어가 오버랩하면 완주 처리
 */
UCLASS(Blueprintable)
class WJWORLD_API AJumpMapEndActor : public AJumpMapActorBase
{
	GENERATED_BODY()

public:
	AJumpMapEndActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> FinishTrigger;

private:
	UFUNCTION()
	void OnFinishOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

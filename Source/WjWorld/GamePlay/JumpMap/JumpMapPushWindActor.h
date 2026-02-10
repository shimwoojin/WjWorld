// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/JumpMap/JumpMapActorBase.h"
#include "JumpMapPushWindActor.generated.h"

class UBoxComponent;

/**
 * JumpMap 바람 장애물 액터
 * - 영역 내 캐릭터를 액터의 전방으로 밀어냄
 */
UCLASS(Blueprintable)
class WJWORLD_API AJumpMapPushWindActor : public AJumpMapActorBase
{
	GENERATED_BODY()

public:
	AJumpMapPushWindActor();

	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> WindZone;

	/** 바람 힘 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpMap|PushWind")
	float WindForce = 500.f;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WjWorldBrickActor.generated.h"

class UWjWorldBrickComponent;

UCLASS()
class WJWORLD_API AWjWorldBrickActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWjWorldBrickActor();

	UWjWorldBrickComponent* GetBrickComponent() const { return BrickComponent; }

	// VFX 스폰 (서버에서 호출 → 모든 클라이언트에서 실행)
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastSpawnDestroyEffect();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastSpawnExplosionEffect();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastSpawnDestructionFracture();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastSpawnDamageHitEffect();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostNetInit() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UWjWorldBrickComponent* BrickComponent;
};

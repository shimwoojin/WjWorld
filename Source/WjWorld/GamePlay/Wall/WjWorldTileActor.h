// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WjTypes.h"
#include "WjWorldTileActor.generated.h"

class UBoxComponent;
class UWjWorldGameRuleApproachingWall;
class UNiagaraSystem;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

UCLASS()
class WJWORLD_API AWjWorldTileActor : public AActor
{
	GENERATED_BODY()
	
	constexpr static float HitBoxSize = 5.0f;

	const static FColor BombSignalOnColorWarning;
	const static FColor BombSignalOnColorDanger;

	static const TCHAR* TileMeshPath;

public:	
	AWjWorldTileActor();

	void InitializeTile(const FVector& InSize, const FVector& InCenterOffset, bool bInIsWhiteTile);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void PostNetInit() override;

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

	UFUNCTION()
	void OnRep_IsBombSignalOn();

	void ApplyTileColor();


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> CenterHitBoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> TileMeshComponent;

	UPROPERTY()
	TObjectPtr<UBoxComponent> HitBoxComponents[EWjWorldDirection::Max];

	TWeakObjectPtr<UWjWorldGameRuleApproachingWall> GameRule;

	int32 bIsOverlapBricks[EWjWorldDirection::Max];

	UPROPERTY(ReplicatedUsing = OnRep_IsBombSignalOn)
	bool bIsBombSignalOn = false;

	float ElapsedBombingTime = 0.0f;

	UPROPERTY(EditAnywhere)
	float BombChargingTime = 3.0f;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* NiagaraSystem;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	UPROPERTY(Replicated)
	bool bIsWhiteTile = true;

	FLinearColor DefaultBaseColor;
};

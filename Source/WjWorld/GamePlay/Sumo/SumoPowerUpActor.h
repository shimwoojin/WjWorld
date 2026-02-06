// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SumoPowerUpActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class URotatingMovementComponent;
class AWjWorldCharacterPlay;

UENUM(BlueprintType)
enum class ESumoPowerUpType : uint8
{
	SpeedBoost,
	SuperPush,
	Shield
};

/**
 * Sumo 미니게임 파워업 픽업 액터
 * - 플랫폼 위 랜덤 위치에 스폰
 * - 오버랩으로 캐릭터 감지 → 버프 적용
 * - SpeedBoost: 5초간 이동속도 증가
 * - SuperPush: 다음 Push 넉백 2배
 * - Shield: 1회 제거 방지
 */
UCLASS()
class WJWORLD_API ASumoPowerUpActor : public AActor
{
	GENERATED_BODY()

public:
	ASumoPowerUpActor();

	/** 파워업 타입 설정 (메시도 함께 적용) */
	UFUNCTION(BlueprintCallable, Category = "Sumo|PowerUp")
	void SetPowerUpType(ESumoPowerUpType NewType);

	/** 현재 파워업 타입 */
	UPROPERTY(ReplicatedUsing = OnRep_PowerUpType, BlueprintReadOnly, Category = "Sumo|PowerUp")
	ESumoPowerUpType PowerUpType = ESumoPowerUpType::SpeedBoost;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_PowerUpType();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void ApplyPowerUp(AWjWorldCharacterPlay* Character);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URotatingMovementComponent> RotatingMovement;

	/** SpeedBoost 이동속도 배율 */
	UPROPERTY(EditDefaultsOnly, Category = "Sumo|PowerUp")
	float SpeedBoostMultiplier = 1.5f;

	/** SpeedBoost 지속 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Sumo|PowerUp")
	float SpeedBoostDuration = 5.f;

	//~ 타입별 메시 (에디터에서 설정)
	UPROPERTY(EditDefaultsOnly, Category = "Sumo|PowerUp|Mesh")
	TObjectPtr<UStaticMesh> SpeedBoostMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Sumo|PowerUp|Mesh")
	TObjectPtr<UStaticMesh> SuperPushMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Sumo|PowerUp|Mesh")
	TObjectPtr<UStaticMesh> ShieldMesh;

private:
	/** 현재 타입에 맞는 메시 적용 */
	void ApplyMeshForType();

	/** 캐릭터별 SpeedBoost 타이머 핸들 (중복 픽업 시 기존 타이머 취소용) */
	static TMap<TWeakObjectPtr<AWjWorldCharacterPlay>, FTimerHandle> SpeedBoostTimerHandles;
};

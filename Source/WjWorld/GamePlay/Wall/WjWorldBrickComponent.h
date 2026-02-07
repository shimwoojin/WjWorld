// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Core/Components/WjWorldGameplaySceneComponent.h"
#include "WjTypes.h"
#include "WjWorldBrickComponent.generated.h"

class UWjWorldBrickMovement;
class UBoxComponent;
class AWjWorldBrickActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWallCollision, UWjWorldBrickComponent*, CollidedBrick, const FVector&, WallDirection);

UENUM()
enum class EWjWorldBrickType : uint8
{
	Standard = 1,
	Explosive,
	Moving,
	Destructible
};

UENUM()
enum class EWjWorldBrickMoveType : uint8
{
	Standard,
};


USTRUCT(BlueprintType)
struct FWjWorldBrickProperties
{
	GENERATED_BODY()

	FColor GetColorWithBrickType() const
	{
		switch (BrickType)
		{
		case EWjWorldBrickType::Standard:
			return FColor::White;
		case EWjWorldBrickType::Explosive:
			return FColor::Red;
		case EWjWorldBrickType::Moving:
			return FColor::Blue;
		case EWjWorldBrickType::Destructible:
			return FColor::Black;
		default:
			return FColor::White;
		}
	}

	UPROPERTY(EditAnywhere)
	EWjWorldBrickType BrickType = EWjWorldBrickType::Standard;

	UPROPERTY(EditAnywhere)
	EWjWorldBrickMoveType BrickMoveType = EWjWorldBrickMoveType::Standard;

	UPROPERTY(EditAnywhere)
	FVector Size = FVector(100.0, 100.0, 100.0);

	UPROPERTY(EditAnywhere)
	FColor Color = FColor::White;

	UPROPERTY()
	FIntPoint SpawnedGridPosition = FIntPoint::ZeroValue;

	UPROPERTY()
	FVector CenterOffset = FVector::ZeroVector;

	UPROPERTY()
	int32 RowNum = 0;

	UPROPERTY()
	int32 ColumnNum = 0;

	UPROPERTY(EditAnywhere)
	int32 MaxHP = 1;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WJWORLD_API UWjWorldBrickComponent : public UWjWorldGameplaySceneComponent
{
	GENERATED_BODY()

public:
	/** DeveloperSettings에서 벽돌 메시 로드 */
	static UStaticMesh* GetBrickMesh();
	constexpr static float HitBoxSize = 5.0f;

public:	
	// Sets default values for this component's properties
	UWjWorldBrickComponent();

	void InitializeBrick(const FWjWorldBrickProperties& InBrickProperties);
	const FWjWorldBrickProperties& GetBrickProperties() const { return BrickProperties; }
	void ReserveDestroyBrick(float AfterSeconds);

	// HP 시스템
	void ApplyDamage(int32 DamageAmount);

	// 충돌 처리
	void HandleWallCollision(const FVector& WallDirection);
	void PushInDirection(const FVector& Direction, float MoveTime);

	// 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnWallCollision OnWallCollision;

	UWjWorldBrickMovement* GetBrickMovement() const { return BrickMovement; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;
	virtual void OnRegister() override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
private:
	void OnBrickMovementSignal(int32 BrickMoveSignalCount);
	void Explode();
	void DestroyBrick();

	UFUNCTION()
	void OnRep_CurrentHP();
	void UpdateDamageVisuals();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> CenterHitBoxComponent;

	/** 캐릭터 blocking collision용 BoxComponent (대각선 이동을 위해 약간 축소) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> BlockingCollisionComponent;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> BrickMeshComponent;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FWjWorldBrickProperties BrickProperties;

	UPROPERTY()
	TObjectPtr<UWjWorldBrickMovement> BrickMovement;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP)
	int32 CurrentHP = 1;

	FTimerHandle DestroyHandle;

	int32 CurrentBrickMoveSignalCount = 0;
};

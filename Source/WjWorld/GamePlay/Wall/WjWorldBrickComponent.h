// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Core/Components/WjWorldGameplaySceneComponent.h"
#include "WjTypes.h"
#include "WjWorldBrickComponent.generated.h"

class UWjWorldBrickMovement;
class UBoxComponent;

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
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WJWORLD_API UWjWorldBrickComponent : public UWjWorldGameplaySceneComponent
{
	GENERATED_BODY()

public:
	static const TCHAR* BrickMeshPath;

public:	
	// Sets default values for this component's properties
	UWjWorldBrickComponent();

	void InitializeBrick(const FWjWorldBrickProperties& InBrickProperties);
	const FWjWorldBrickProperties& GetBrickProperties() const { return BrickProperties; }
	void ReserveDestroyBrick(float AfterSeconds);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;
	virtual void OnRegister() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
private:
	void OnBrickMovementSignal(int32 BrickMoveSignalCount);

private:
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> BrickMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FWjWorldBrickProperties BrickProperties;

	UPROPERTY()
	TObjectPtr<UWjWorldBrickMovement> BrickMovement;

	FTimerHandle DestroyHandle;

	int32 CurrentBrickMoveSignalCount = 0;
};

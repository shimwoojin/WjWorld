// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WjWorldBrickComponent.generated.h"

class UWjWorldBrickMovement;

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
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WJWORLD_API UWjWorldBrickComponent : public USceneComponent
{
	GENERATED_BODY()

	const TCHAR* BrickMeshPath = TEXT("/Game/GamePlay/Wall/Mesh/Cube");

public:	
	// Sets default values for this component's properties
	UWjWorldBrickComponent();

	void InitializeBrick(const FWjWorldBrickProperties& InBrickProperties);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void OnRegister() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
private:
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> BrickMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FWjWorldBrickProperties BrickProperties;

	UPROPERTY()
	TObjectPtr<UWjWorldBrickMovement> BrickMovement;
};

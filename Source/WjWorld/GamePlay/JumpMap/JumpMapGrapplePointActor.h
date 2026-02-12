// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/JumpMap/JumpMapActorBase.h"
#include "JumpMapGrapplePointActor.generated.h"

class USphereComponent;

/**
 * JumpMap 그래플 포인트 액터
 * - GA_Grapple 어빌리티의 타겟 지점
 * - 범위 내에 있을 때만 그래플 가능
 */
UCLASS(Blueprintable)
class WJWORLD_API AJumpMapGrapplePointActor : public AJumpMapActorBase
{
	GENERATED_BODY()

public:
	AJumpMapGrapplePointActor();

	virtual void GetSerializableProperties(TMap<FString, FString>& OutProperties) const override;
	virtual void ApplySerializedProperties(const TMap<FString, FString>& Properties) override;

	/** 그래플 사거리 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpMap|Grapple")
	float GrappleRadius = 1500.f;

	/** 지정된 위치에서 그래플 가능 여부 */
	bool IsInRange(const FVector& FromLocation) const;

	/** 그래플 타겟 위치 반환 */
	FVector GetGrappleTargetLocation() const;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> GrappleRange;
};

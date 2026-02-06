// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldPlayerControllerBase.h"
#include "WjWorldPlayerControllerAWEditor.generated.h"

class UWjWorldPlacementComponent;

/**
 * Approaching Wall 에디터 플레이어 컨트롤러
 * PlacementComponent를 보유하여 배치 기능 제공
 */
UCLASS()
class WJWORLD_API AWjWorldPlayerControllerAWEditor : public AWjWorldPlayerControllerBase
{
	GENERATED_BODY()

public:
	AWjWorldPlayerControllerAWEditor();

	/** 배치 컴포넌트 getter */
	UFUNCTION(BlueprintCallable, Category = "AWEditor|Placement")
	UWjWorldPlacementComponent* GetPlacementComponent() const { return PlacementComponent; }

protected:
	virtual void BeginPlay() override;

private:
	/** 배치 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "Placement")
	TObjectPtr<UWjWorldPlacementComponent> PlacementComponent;
};

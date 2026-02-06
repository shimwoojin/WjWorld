// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldPlayerControllerBase.h"
#include "WjWorldPlayerControllerJumpMapEditor.generated.h"

class UWjWorldPlacementComponent;

/**
 * JumpMap 에디터 플레이어 컨트롤러
 * PlacementComponent를 보유하여 배치 기능 제공
 */
UCLASS()
class WJWORLD_API AWjWorldPlayerControllerJumpMapEditor : public AWjWorldPlayerControllerBase
{
	GENERATED_BODY()

public:
	AWjWorldPlayerControllerJumpMapEditor();

	/** 배치 컴포넌트 getter */
	UFUNCTION(BlueprintCallable, Category = "JumpMapEditor|Placement")
	UWjWorldPlacementComponent* GetPlacementComponent() const { return PlacementComponent; }

protected:
	virtual void BeginPlay() override;

private:
	/** 배치 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "Placement")
	TObjectPtr<UWjWorldPlacementComponent> PlacementComponent;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameData/WjWorldGameDataComponent.h"
#include "ApproachingWallPlayerDataComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerAliveChanged, bool, bIsAlive);

/**
 * Approaching Wall 미니게임 플레이어 데이터
 */
UCLASS()
class WJWORLD_API UApproachingWallPlayerDataComponent : public UWjWorldGameDataComponent
{
	GENERATED_BODY()

public:
	UApproachingWallPlayerDataComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 생존 상태
	UFUNCTION(BlueprintCallable, Category = "ApproachingWall")
	bool IsAlive() const { return bIsAlive; }

	UFUNCTION(BlueprintCallable, Category = "ApproachingWall")
	void SetAlive(bool bNewAlive);

	// 사망 시 호출
	UFUNCTION(BlueprintCallable, Category = "ApproachingWall")
	void OnEliminated();

	UPROPERTY(BlueprintAssignable, Category = "ApproachingWall")
	FOnPlayerAliveChanged OnPlayerAliveChanged;

protected:
	UFUNCTION()
	void OnRep_IsAlive();

private:
	UPROPERTY(ReplicatedUsing = OnRep_IsAlive)
	bool bIsAlive = true;
};

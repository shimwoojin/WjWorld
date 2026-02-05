// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameData/WjWorldGameDataComponent.h"
#include "SumoPlayerDataComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSumoPlayerAliveChanged, bool, bIsAlive);

/**
 * Sumo 미니게임 플레이어 데이터 (PlayerState에 부착)
 */
UCLASS()
class WJWORLD_API USumoPlayerDataComponent : public UWjWorldGameDataComponent
{
	GENERATED_BODY()

public:
	USumoPlayerDataComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Sumo")
	bool IsAlive() const { return bIsAlive; }

	UFUNCTION(BlueprintCallable, Category = "Sumo")
	void SetAlive(bool bNewAlive);

	UFUNCTION(BlueprintCallable, Category = "Sumo")
	void OnEliminated();

	UPROPERTY(BlueprintAssignable, Category = "Sumo")
	FOnSumoPlayerAliveChanged OnPlayerAliveChanged;

protected:
	UFUNCTION()
	void OnRep_IsAlive();

private:
	UPROPERTY(ReplicatedUsing = OnRep_IsAlive)
	bool bIsAlive = true;
};

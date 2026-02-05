// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameData/WjWorldGameDataComponent.h"
#include "SumoGameDataComponent.generated.h"

/**
 * Sumo 미니게임 전체 데이터 (GameState에 부착)
 */
UCLASS()
class WJWORLD_API USumoGameDataComponent : public UWjWorldGameDataComponent
{
	GENERATED_BODY()

public:
	int32 GetAlivePlayerCount() const { return AlivePlayerCount; }
	void SetAlivePlayerCount(int32 InCount) { AlivePlayerCount = InCount; }

	int32 GetTotalPlayerCount() const { return TotalPlayerCount; }
	void SetTotalPlayerCount(int32 InCount) { TotalPlayerCount = InCount; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated)
	int32 AlivePlayerCount = 0;

	UPROPERTY(Replicated)
	int32 TotalPlayerCount = 0;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameData/WjWorldGameDataComponent.h"
#include "JumpMapPlayerDataComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJumpMapCheckpointReached, int32, CheckpointIndex);

/**
 * JumpMap 미니게임 플레이어 데이터 (PlayerState에 부착)
 * 체크포인트, 완주 상태, 사망 횟수
 */
UCLASS()
class WJWORLD_API UJumpMapPlayerDataComponent : public UWjWorldGameDataComponent
{
	GENERATED_BODY()

public:
	UJumpMapPlayerDataComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool HasFinished() const { return bHasFinished; }
	void SetFinished(float InFinishTime);
	float GetFinishTime() const { return FinishTime; }

	int32 GetCurrentCheckpointIndex() const { return CurrentCheckpointIndex; }
	void SetCurrentCheckpointIndex(int32 Index);

	int32 GetDeathCount() const { return DeathCount; }
	void IncrementDeathCount();

	void ResetForNewGame();

	UPROPERTY(BlueprintAssignable, Category = "JumpMap")
	FOnJumpMapCheckpointReached OnCheckpointReached;

protected:
	UFUNCTION()
	void OnRep_CurrentCheckpointIndex();

private:
	UPROPERTY(Replicated)
	bool bHasFinished = false;

	UPROPERTY(Replicated)
	float FinishTime = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentCheckpointIndex)
	int32 CurrentCheckpointIndex = -1;

	UPROPERTY(Replicated)
	int32 DeathCount = 0;
};

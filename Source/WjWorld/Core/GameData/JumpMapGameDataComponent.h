// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameData/WjWorldGameDataComponent.h"
#include "JumpMapGameDataComponent.generated.h"

/** 플레이어 완주 정보 */
USTRUCT(BlueprintType)
struct FJumpMapPlayerFinish
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
	float FinishTime = 0.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJumpMapPlayerFinished, const FJumpMapPlayerFinish&, FinishInfo);

/**
 * JumpMap 미니게임 전체 데이터 (GameState에 부착)
 * 경과 시간, 완주 순서 리플리케이션
 */
UCLASS()
class WJWORLD_API UJumpMapGameDataComponent : public UWjWorldGameDataComponent
{
	GENERATED_BODY()

public:
	float GetElapsedTime() const { return ElapsedTime; }
	void SetElapsedTime(float InTime) { ElapsedTime = InTime; }

	float GetTimeLimit() const { return TimeLimit; }
	void SetTimeLimit(float InLimit) { TimeLimit = InLimit; }

	int32 GetFinishedPlayerCount() const { return FinishedPlayerCount; }
	void SetFinishedPlayerCount(int32 InCount) { FinishedPlayerCount = InCount; }

	int32 GetTotalPlayerCount() const { return TotalPlayerCount; }
	void SetTotalPlayerCount(int32 InCount) { TotalPlayerCount = InCount; }

	void AddPlayerFinish(const FJumpMapPlayerFinish& Finish);
	const TArray<FJumpMapPlayerFinish>& GetPlayerFinishOrder() const { return PlayerFinishOrder; }

	UPROPERTY(BlueprintAssignable, Category = "JumpMap")
	FOnJumpMapPlayerFinished OnPlayerFinished;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_PlayerFinishOrder();

private:
	UPROPERTY(Replicated)
	float ElapsedTime = 0.f;

	UPROPERTY(Replicated)
	float TimeLimit = 120.f;

	UPROPERTY(Replicated)
	int32 FinishedPlayerCount = 0;

	UPROPERTY(Replicated)
	int32 TotalPlayerCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerFinishOrder)
	TArray<FJumpMapPlayerFinish> PlayerFinishOrder;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameData/WjWorldGameDataComponent.h"
#include "SumoGameDataComponent.generated.h"

/** 플레이어 점수 항목 */
USTRUCT(BlueprintType)
struct FSumoPlayerScore
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
	int32 Score = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSumoKillFeed, const FString&, KillFeedText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSumoRoundChanged, int32, CurrentRound, int32, MaxRounds);

/**
 * Sumo 미니게임 전체 데이터 (GameState에 부착)
 * 킬피드, 라운드, 점수 리플리케이션
 */
UCLASS()
class WJWORLD_API USumoGameDataComponent : public UWjWorldGameDataComponent
{
	GENERATED_BODY()

public:
	// 생존자
	int32 GetAlivePlayerCount() const { return AlivePlayerCount; }
	void SetAlivePlayerCount(int32 InCount) { AlivePlayerCount = InCount; }

	int32 GetTotalPlayerCount() const { return TotalPlayerCount; }
	void SetTotalPlayerCount(int32 InCount) { TotalPlayerCount = InCount; }

	// 킬피드
	void SetKillFeedText(const FString& InText);
	FString GetLastKillFeedText() const { return LastKillFeedText; }

	UPROPERTY(BlueprintAssignable, Category = "Sumo")
	FOnSumoKillFeed OnKillFeedUpdated;

	// 라운드
	int32 GetCurrentRound() const { return CurrentRound; }
	void SetCurrentRound(int32 InRound);

	int32 GetMaxRounds() const { return MaxRounds; }
	void SetMaxRounds(int32 InMax) { MaxRounds = InMax; }

	UPROPERTY(BlueprintAssignable, Category = "Sumo")
	FOnSumoRoundChanged OnRoundChanged;

	// 점수
	const TArray<FSumoPlayerScore>& GetPlayerScores() const { return PlayerScores; }
	void SetPlayerScores(const TArray<FSumoPlayerScore>& InScores);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_LastKillFeed();

	UFUNCTION()
	void OnRep_CurrentRound();

	UFUNCTION()
	void OnRep_PlayerScores();

private:
	UPROPERTY(Replicated)
	int32 AlivePlayerCount = 0;

	UPROPERTY(Replicated)
	int32 TotalPlayerCount = 0;

	// 킬피드
	UPROPERTY(ReplicatedUsing = OnRep_LastKillFeed)
	FString LastKillFeedText;

	/** 동일 텍스트 갱신 감지용 카운터 */
	UPROPERTY(ReplicatedUsing = OnRep_LastKillFeed)
	int32 KillFeedCounter = 0;

	// 라운드
	UPROPERTY(ReplicatedUsing = OnRep_CurrentRound)
	int32 CurrentRound = 0;

	UPROPERTY(Replicated)
	int32 MaxRounds = 3;

	// 점수
	UPROPERTY(ReplicatedUsing = OnRep_PlayerScores)
	TArray<FSumoPlayerScore> PlayerScores;
};

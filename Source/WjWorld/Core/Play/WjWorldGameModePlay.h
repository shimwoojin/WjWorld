// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldGameModeBase.h"
#include "WjWorldGameModePlay.generated.h"

class UWjWorldGameRuleBase;

DECLARE_MULTICAST_DELEGATE(FOnGameStartDelegate);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameLevelChangeSignature, int32 /*Level*/);

/**
 * 
 */
UCLASS()
class WJWORLD_API AWjWorldGameModePlay : public AWjWorldGameModeBase
{
	GENERATED_BODY()
	
public:
	AWjWorldGameModePlay();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	void StartGame(float SecondsForStartCount);
	void EndGamePredict(float SecondsForEndCount);

	TObjectPtr<UWjWorldGameRuleBase> GetCurrentGameRule() const { return CurrentGameRule; }

	void OnGameLevelUp(int32 NewLevel);

protected:
	virtual void BeginDestroy() override;

public:
	FOnGameStartDelegate OnGameStart;

public:
	FOnGameLevelChangeSignature OnGameLevelChange;

private:
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	TSubclassOf<UWjWorldGameRuleBase> GameRuleClass;

	UPROPERTY()
	TObjectPtr<UWjWorldGameRuleBase> CurrentGameRule;

	FTimerHandle StartGameHandle;
};

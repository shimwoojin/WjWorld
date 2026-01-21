// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldGameModeBase.h"
#include "WjWorldGameModePlay.generated.h"

class UWjWorldGameRuleBase;

DECLARE_MULTICAST_DELEGATE(FOnGameStartDelegate);

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

protected:
	virtual void BeginDestroy() override;

public:
	FOnGameStartDelegate OnGameStart;

private:
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	TSubclassOf<UWjWorldGameRuleBase> GameRuleClass;

	UPROPERTY()
	TObjectPtr<UWjWorldGameRuleBase> CurrentGameRule;

	FTimerHandle StartGameHandle;
};

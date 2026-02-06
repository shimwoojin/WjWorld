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
	virtual void Logout(AController* Exiting) override;
	virtual void Tick(float DeltaSeconds) override;

	void StartGame(float SecondsForStartCount);
	void EndGamePredict(float SecondsForEndCount);

	TObjectPtr<UWjWorldGameRuleBase> GetCurrentGameRule() const { return CurrentGameRule; }

	template<typename T>
	TObjectPtr<T> GetCurrentGameRule() const { return Cast<T>(CurrentGameRule); }

	/** URL Options에서 파싱된 MapOption 값 */
	const FString& GetMapOption() const { return MapOption; }

	void OnGameLevelUp(int32 NewLevel);

protected:
	virtual void BeginDestroy() override;

public:
	FOnGameStartDelegate OnGameStart;

public:
	FOnGameLevelChangeSignature OnGameLevelChange;

private:
	/** 카탈로그에서 GameRule을 찾지 못했을 때 사용할 폴백 클래스 (Deprecated) */
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true, DeprecatedProperty, DeprecationMessage = "Use MinigameCatalog's GameRuleClass instead"))
	TSubclassOf<UWjWorldGameRuleBase> GameRuleClass;

	UPROPERTY()
	TObjectPtr<UWjWorldGameRuleBase> CurrentGameRule;

	FTimerHandle StartGameHandle;

	/** URL Options에서 파싱된 GameModeId (카탈로그 조회용) */
	FString GameModeId;

	/** URL Options에서 파싱된 MapOption */
	FString MapOption;
};

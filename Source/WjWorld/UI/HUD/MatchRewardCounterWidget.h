// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "MatchRewardCounterWidget.generated.h"

class UTextBlock;

/**
 * 매치 보상 일일 잔여 횟수 표시 위젯
 * 승리/패배 각각의 보상 카운트를 상시 표시
 * HUDBase에서 생성되어 Lobby/WaitingRoom/Play 모든 컨텍스트에서 동작
 */
UCLASS()
class WJWORLD_API UMatchRewardCounterWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	/** 승리 보상 카운트 (예: "3/5") */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WinCountText;

	/** 패배 보상 카운트 (예: "7/10") */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LossCountText;

	/** 승리 보상 금액 표시 (예: "50 Coin") */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WinRewardText;

	/** 패배 보상 금액 표시 (예: "10 Coin") */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LossRewardText;

private:
	UFUNCTION()
	void OnMatchRewardCountChanged(int32 WinCount, int32 MaxWin, int32 LossCount, int32 MaxLoss);

	void UpdateDisplay(int32 WinCount, int32 MaxWin, int32 LossCount, int32 MaxLoss);
};

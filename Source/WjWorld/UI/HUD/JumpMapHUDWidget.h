// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "JumpMapHUDWidget.generated.h"

class UTextBlock;
class UVerticalBox;
struct FJumpMapPlayerFinish;

/**
 * JumpMap 미니게임 전용 HUD 위젯
 * GameState의 JumpMapGameDataComponent에서 데이터를 읽어 표시
 * - 경과 시간 / 남은 시간
 * - 체크포인트 / 사망 횟수
 * - 완주 순위
 */
UCLASS()
class WJWORLD_API UJumpMapHUDWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimerText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TimeLimitText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CheckpointText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DeathCountText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> LeaderboardBox;

private:
	UFUNCTION()
	void OnPlayerFinished(const FJumpMapPlayerFinish& FinishInfo);

	void UpdateLeaderboard();
};

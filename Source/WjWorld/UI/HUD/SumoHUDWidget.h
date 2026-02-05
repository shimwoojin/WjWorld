// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "SumoHUDWidget.generated.h"

class UTextBlock;

/**
 * Sumo Knockoff 미니게임 전용 HUD 위젯
 * GameState의 SumoGameDataComponent에서 데이터를 읽어 표시
 * - 생존 플레이어 수
 * - 킬피드 (3초 페이드아웃)
 * - 라운드 정보
 */
UCLASS()
class WJWORLD_API USumoHUDWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	/** 생존 플레이어 수 표시 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AlivePlayerText;

	/** 킬피드 텍스트 (BindWidgetOptional: BP에서 생성 안 해도 됨) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> KillFeedText;

	/** 라운드 정보 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RoundText;

private:
	UFUNCTION()
	void OnKillFeedReceived(const FString& InText);

	void HideKillFeed();

	FTimerHandle KillFeedTimerHandle;

	/** 킬피드 표시 시간 */
	float KillFeedDisplayDuration = 3.f;
};

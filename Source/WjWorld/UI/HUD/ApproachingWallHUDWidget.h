// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "ApproachingWallHUDWidget.generated.h"

class UTextBlock;

/**
 * Approaching Wall 미니게임 전용 HUD 위젯
 * GameState의 ApproachingWallGameDataComponent에서 데이터를 읽어 표시
 */
UCLASS()
class WJWORLD_API UApproachingWallHUDWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AlivePlayerText;
};

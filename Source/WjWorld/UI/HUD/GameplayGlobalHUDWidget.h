// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "GameplayGlobalHUDWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class WJWORLD_API UGameplayGlobalHUDWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	void StartCountDown(float CountDown);
	void EndCountDown(float CountDown);
	void ShowGameResultText(const FString& ResultText, float Duration = 3.0f);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GameStartCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GameEndCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GameResultText;

private:
	float GameStartCountDownTime = 0.0f;
	float GameEndCountDownTime = 0.0f;
	float ResultTextDisplayTime = 0.0f;
};

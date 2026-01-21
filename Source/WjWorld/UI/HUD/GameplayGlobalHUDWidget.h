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

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr <UTextBlock> GameStartCountText;

private:
	float CountDownTime = 0.0f;
};

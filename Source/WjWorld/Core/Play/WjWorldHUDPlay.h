// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldHUDBase.h"
#include "WjWorldHUDPlay.generated.h"

class UGameplayGlobalHUDWidget;
class UWjWorldUserWidgetBase;
class UWjWorldGameRuleBase;

/**
 * 
 */
UCLASS()
class WJWORLD_API AWjWorldHUDPlay : public AWjWorldHUDBase
{
	GENERATED_BODY()
	
public:
	void StartGameStartCountDown(float CountDown);
	void ShowGameResultText(const FString& ResultText, float Duration = 3.0f);

	void ShowGameRuleHUDWidget(TSubclassOf<UWjWorldGameRuleBase> GameRuleClass);

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGameplayGlobalHUDWidget> GlobalHUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UGameplayGlobalHUDWidget> GlobalHUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TMap<TSubclassOf<UWjWorldGameRuleBase>, TSubclassOf<UWjWorldUserWidgetBase>> GameRuleHUDWidgetClasses;

	UPROPERTY()
	TObjectPtr<UWjWorldUserWidgetBase> GameRuleHUDWidget;
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldHUDPlay.h"
#include "Core/GameRule/WjWorldGameRuleBase.h"

#include "UI/HUD/GameplayGlobalHUDWidget.h"

AWjWorldHUDPlay::AWjWorldHUDPlay()
{
	bCreateChatWidget = true;
}

void AWjWorldHUDPlay::StartGameStartCountDown(float CountDown)
{
    if(GlobalHUDWidget)
    {
        GlobalHUDWidget->StartCountDown(CountDown);
	}
}

void AWjWorldHUDPlay::ShowGameResultText(const FString& ResultText, float Duration)
{
    if (GlobalHUDWidget)
    {
        GlobalHUDWidget->ShowGameResultText(ResultText, Duration);
    }
}

void AWjWorldHUDPlay::ShowGameRuleHUDWidget(TSubclassOf<UWjWorldGameRuleBase> GameRuleClass)
{
    if (GameRuleHUDWidget)
    {
        GameRuleHUDWidget->RemoveFromParent();
        GameRuleHUDWidget = nullptr;
    }

    if (GameRuleHUDWidgetClasses.Contains(GameRuleClass))
    {
        TSubclassOf<UWjWorldUserWidgetBase> WidgetClass = GameRuleHUDWidgetClasses[GameRuleClass];
        if (WidgetClass)
        {
            GameRuleHUDWidget = CreateWidget<UWjWorldUserWidgetBase>(GetWorld(), WidgetClass);
            if (GameRuleHUDWidget)
            {
                GameRuleHUDWidget->AddToViewport();
            }
        }
	}
}

void AWjWorldHUDPlay::BeginPlay()
{
	Super::BeginPlay();

	if (GlobalHUDWidgetClass)
	{
        GlobalHUDWidget = CreateWidget<UGameplayGlobalHUDWidget>(GetWorld(), GlobalHUDWidgetClass);
        if (GlobalHUDWidget)
        {
            GlobalHUDWidget->AddToViewport();
        }
	}
}

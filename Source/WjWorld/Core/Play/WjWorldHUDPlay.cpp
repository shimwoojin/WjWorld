// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldHUDPlay.h"

#include "UI/HUD/GameplayGlobalHUDWidget.h"

void AWjWorldHUDPlay::StartGameStartCountDown(float CountDown)
{
    if(GlobalHUDWidget)
    {
        GlobalHUDWidget->StartCountDown(CountDown);
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

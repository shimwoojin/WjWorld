// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Intro/WjWorldGameModeIntro.h"

#include "UI/Intro/IntroWindow.h"

AWjWorldGameModeIntro::AWjWorldGameModeIntro()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AWjWorldGameModeIntro::BeginPlay()
{
	Super::BeginPlay();
	// UI 생성 및 표시
	CreateAndShowIntroUI();
}

void AWjWorldGameModeIntro::OnVideoFinished()
{
}

void AWjWorldGameModeIntro::CreateAndShowIntroUI()
{
    if (IntroWidgetClass)
    {
        // 위젯 생성
        IntroWidget = CreateWidget<UIntroWindow>(GetWorld(), IntroWidgetClass);

        if (IntroWidget)
        {
            // 화면에 추가
            IntroWidget->AddToViewport();

            // 동영상 종료 이벤트 바인딩
            IntroWidget->OnVideoCompleted.AddDynamic(this, &AWjWorldGameModeIntro::OnVideoFinished);

            // 동영상 재생 시작
            IntroWidget->PlayIntroVideo();

            // 마우스 커서 표시 (필요시)
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (PC)
            {
                PC->bShowMouseCursor = true;
                PC->SetInputMode(FInputModeUIOnly());
            }
        }
    }
}

void AWjWorldGameModeIntro::DetermineNextLevel()
{
}

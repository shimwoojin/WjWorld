// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Intro/WjWorldGameModeIntro.h"
#include "UI/Intro/IntroWindow.h"
#include "Kismet/GameplayStatics.h"

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
    UE_LOG(LogTemp, Warning, TEXT("Intro video finished - Moving to Login"));

    // UI 제거
    if (IntroWidget)
    {
        IntroWidget->RemoveFromParent();
        IntroWidget = nullptr;
    }

    // 입력 모드 복원
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }

    // Login 레벨로 이동
    UGameplayStatics::OpenLevel(GetWorld(), TEXT("01_Login"));
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

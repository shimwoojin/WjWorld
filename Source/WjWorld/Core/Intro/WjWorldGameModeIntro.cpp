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
	// UI ���� �� ǥ��
	CreateAndShowIntroUI();
}

void AWjWorldGameModeIntro::OnVideoFinished()
{
}

void AWjWorldGameModeIntro::CreateAndShowIntroUI()
{
    if (IntroWidgetClass)
    {
        // ���� ����
        IntroWidget = CreateWidget<UIntroWindow>(GetWorld(), IntroWidgetClass);

        if (IntroWidget)
        {
            // ȭ�鿡 �߰�
            IntroWidget->AddToViewport();

            // ������ ���� �̺�Ʈ ���ε�
            IntroWidget->OnVideoCompleted.AddDynamic(this, &AWjWorldGameModeIntro::OnVideoFinished);

            // ������ ��� ����
            IntroWidget->PlayIntroVideo();

            // ���콺 Ŀ�� ǥ�� (�ʿ��)
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

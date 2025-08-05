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
	// UI ���� �� ǥ��
	CreateAndShowIntroUI();
}

void AWjWorldGameModeIntro::OnVideoFinished()
{
    UE_LOG(LogTemp, Warning, TEXT("Intro video finished - Moving to Login"));

    // UI ����
    if (IntroWidget)
    {
        IntroWidget->RemoveFromParent();
        IntroWidget = nullptr;
    }

    // �Է� ��� ����
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }

    // Login ������ �̵�
    UGameplayStatics::OpenLevel(GetWorld(), TEXT("01_Login"));
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

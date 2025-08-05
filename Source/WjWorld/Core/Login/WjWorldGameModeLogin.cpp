// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Login/WjWorldGameModeLogin.h"
#include "UI/Login/LoginWindow.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AWjWorldGameModeLogin::AWjWorldGameModeLogin()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWjWorldGameModeLogin::BeginPlay()
{
    Super::BeginPlay();

    // ���� ���� �� Login UI ����
    CreateAndShowLoginUI();
}

void AWjWorldGameModeLogin::CreateAndShowLoginUI()
{
    if (LoginWidgetClass)
    {
        // ���� ����
        LoginWidgetInstance = CreateWidget<ULoginWindow>(GetWorld(), LoginWidgetClass);

        if (LoginWidgetInstance)
        {
            // ȭ�鿡 �߰�
            LoginWidgetInstance->AddToViewport();

            // ���콺 Ŀ�� ǥ��
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (PC)
            {
                PC->bShowMouseCursor = true;
                PC->SetInputMode(FInputModeUIOnly());
            }

            // 5�� �� �ڵ����� �α��� �Ϸ� ó��
            GetWorld()->GetTimerManager().SetTimer(
                LoginTimerHandle,
                this,
                &AWjWorldGameModeLogin::OnLoginCompleted,
                LoginWaitTime,
                false
            );

            UE_LOG(LogTemp, Warning, TEXT("Login UI displayed - Auto login in %.1f seconds"), LoginWaitTime);
        }
    }
}

void AWjWorldGameModeLogin::OnLoginCompleted()
{
    UE_LOG(LogTemp, Warning, TEXT("Login completed - Moving to Lobby"));

    // UI ����
    if (LoginWidgetInstance)
    {
        LoginWidgetInstance->RemoveFromViewport();
        LoginWidgetInstance = nullptr;
    }

    // �Է� ��� ����
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }

    // Ÿ�̸� Ŭ����
    GetWorld()->GetTimerManager().ClearTimer(LoginTimerHandle);

    // Lobby ������ �̵�
    UGameplayStatics::OpenLevel(GetWorld(), TEXT("02-1_Lobby"));
}

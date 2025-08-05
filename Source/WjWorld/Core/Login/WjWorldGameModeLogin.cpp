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

    // 레벨 시작 후 Login UI 생성
    CreateAndShowLoginUI();
}

void AWjWorldGameModeLogin::CreateAndShowLoginUI()
{
    if (LoginWidgetClass)
    {
        // 위젯 생성
        LoginWidgetInstance = CreateWidget<ULoginWindow>(GetWorld(), LoginWidgetClass);

        if (LoginWidgetInstance)
        {
            // 화면에 추가
            LoginWidgetInstance->AddToViewport();

            // 마우스 커서 표시
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (PC)
            {
                PC->bShowMouseCursor = true;
                PC->SetInputMode(FInputModeUIOnly());
            }

            // 5초 후 자동으로 로그인 완료 처리
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

    // UI 제거
    if (LoginWidgetInstance)
    {
        LoginWidgetInstance->RemoveFromViewport();
        LoginWidgetInstance = nullptr;
    }

    // 입력 모드 복원
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }

    // 타이머 클리어
    GetWorld()->GetTimerManager().ClearTimer(LoginTimerHandle);

    // Lobby 레벨로 이동
    UGameplayStatics::OpenLevel(GetWorld(), TEXT("02-1_Lobby"));
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Login/LoginWindow.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Engine/World.h"

void ULoginWindow::UpdateLoginProgress(float Progress)
{
    if (LoginProgressBar)
    {
        LoginProgressBar->SetPercent(Progress);
    }
}

void ULoginWindow::UpdateLoginStatusText(const FString& StatusText)
{
    if (LoginStatusText)
    {
        LoginStatusText->SetText(FText::FromString(StatusText));
    }
}

void ULoginWindow::NativeConstruct()
{
    Super::NativeConstruct();

    // 초기 상태 설정
    UpdateLoginStatusText(TEXT("로그인 중..."));

    if (LoginProgressBar)
    {
        LoginProgressBar->SetPercent(0.0f);

        // 진행 바 애니메이션 시작
        GetWorld()->GetTimerManager().SetTimer(
            ProgressTimerHandle,
            this,
            &ULoginWindow::UpdateProgressBar,
            0.1f,
            true
        );
    }
}

void ULoginWindow::UpdateProgressBar()
{
    if (LoginProgressBar && CurrentProgress < 1.0f)
    {
        CurrentProgress += 0.02f; // 5초 동안 진행 (0.02 * 50 = 1.0)
        LoginProgressBar->SetPercent(CurrentProgress);

        if (CurrentProgress >= 1.0f)
        {
            GetWorld()->GetTimerManager().ClearTimer(ProgressTimerHandle);
        }
    }
}

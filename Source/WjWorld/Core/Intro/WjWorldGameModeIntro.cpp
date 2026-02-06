// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Intro/WjWorldGameModeIntro.h"
#include "Core/WjWorldGameInstance.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "UI/Intro/IntroWindow.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

AWjWorldGameModeIntro::AWjWorldGameModeIntro()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AWjWorldGameModeIntro::BeginPlay()
{
	Super::BeginPlay();

	// ⭐ 호스트 마이그레이션 중에 Intro로 온 경우 → Lobby로 바로 이동
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (GameInstance && GameInstance->IsMigrating())
	{
		UE_LOG(LogWjWorld, Warning, TEXT("GameModeIntro: Migration in progress - redirecting to Lobby"));

		// 마이그레이션 컨텍스트 리셋 (실패로 처리)
		GameInstance->GetMigrationContext().Reset();

		// Lobby로 이동
		const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
		UGameplayStatics::OpenLevel(GetWorld(), FName(*Settings->GetLobbyMapPath()));
		return;
	}

	// UI 생성 및 표시
	CreateAndShowIntroUI();
}

void AWjWorldGameModeIntro::OnVideoFinished()
{
    UE_LOG(LogWjWorld, Warning, TEXT("Intro video finished - Moving to Login"));

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
    if (!IntroWidgetClass)
    {
        UE_LOG(LogWjWorld, Warning, TEXT("IntroWidgetClass is not set - skipping intro"));
        OnVideoFinished();
        return;
    }

    // 위젯 생성
    IntroWidget = CreateWidget<UIntroWindow>(GetWorld(), IntroWidgetClass);

    if (!IntroWidget)
    {
        UE_LOG(LogWjWorld, Warning, TEXT("Failed to create IntroWidget - skipping intro"));
        OnVideoFinished();
        return;
    }

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

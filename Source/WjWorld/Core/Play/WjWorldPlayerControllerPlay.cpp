// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldPlayerControllerPlay.h"
#include "UI/Common/ConfirmDialogWidget.h"
#include "Core/WjWorldGameInstance.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

void AWjWorldPlayerControllerPlay::BeginPlay()
{
	Super::BeginPlay();

	SetInputMode(FInputModeGameOnly());
}

void AWjWorldPlayerControllerPlay::OnEscapePressed()
{
	// Play 모드에서는 LeaveDialog 토글이 우선
	if (bIsLeaveDialogOpen)
	{
		CloseLeaveGameDialog();
	}
	else
	{
		ShowLeaveGameDialog();
	}
}

void AWjWorldPlayerControllerPlay::ShowLeaveGameDialog()
{
	if (bIsLeaveDialogOpen)
	{
		return;
	}

	if (!LeaveGameDialogClass)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("PlayerControllerPlay: LeaveGameDialogClass is not set"));
		return;
	}

	// 다이얼로그 인스턴스 생성
	LeaveGameDialog = CreateWidget<UConfirmDialogWidget>(this, LeaveGameDialogClass);
	if (!LeaveGameDialog)
	{
		UE_LOG(LogWjWorld, Error, TEXT("PlayerControllerPlay: Failed to create LeaveGameDialog"));
		return;
	}

	LeaveGameDialog->SetMessage(FText::FromString(TEXT("게임을 나가시겠습니까?")));
	LeaveGameDialog->SetButtonLabels(
		FText::FromString(TEXT("나가기")),
		FText::FromString(TEXT("취소"))
	);

	LeaveGameDialog->OnConfirmed.AddDynamic(this, &AWjWorldPlayerControllerPlay::OnLeaveGameConfirmed);
	LeaveGameDialog->OnCancelled.AddDynamic(this, &AWjWorldPlayerControllerPlay::OnLeaveGameCancelled);

	LeaveGameDialog->ShowPopup();

	// GameAndUI 모드: 마우스 클릭 가능 + ESC 바인딩 유지
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);

	bIsLeaveDialogOpen = true;

	UE_LOG(LogWjWorld, Log, TEXT("PlayerControllerPlay: Leave game dialog shown"));
}

void AWjWorldPlayerControllerPlay::CloseLeaveGameDialog()
{
	if (!bIsLeaveDialogOpen || !LeaveGameDialog)
	{
		return;
	}

	LeaveGameDialog->OnConfirmed.RemoveDynamic(this, &AWjWorldPlayerControllerPlay::OnLeaveGameConfirmed);
	LeaveGameDialog->OnCancelled.RemoveDynamic(this, &AWjWorldPlayerControllerPlay::OnLeaveGameCancelled);

	LeaveGameDialog->ClosePopup();
	LeaveGameDialog = nullptr;

	// GameOnly 모드 복원
	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);

	bIsLeaveDialogOpen = false;

	UE_LOG(LogWjWorld, Log, TEXT("PlayerControllerPlay: Leave game dialog closed"));
}

void AWjWorldPlayerControllerPlay::OnLeaveGameConfirmed()
{
	UE_LOG(LogWjWorld, Log, TEXT("PlayerControllerPlay: Leave game confirmed"));

	// InputMode 복원 후 퇴장
	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);
	bIsLeaveDialogOpen = false;

	ExecuteLeaveGame();
}

void AWjWorldPlayerControllerPlay::OnLeaveGameCancelled()
{
	UE_LOG(LogWjWorld, Log, TEXT("PlayerControllerPlay: Leave game cancelled"));

	CloseLeaveGameDialog();
}

void AWjWorldPlayerControllerPlay::ExecuteLeaveGame()
{
	// WaitingRoomHUDWidget::OnLeaveClicked() 동일 패턴
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		bool bSuccess = GameInstance->LeaveRoom();
		if (bSuccess)
		{
			UE_LOG(LogWjWorld, Log, TEXT("PlayerControllerPlay: Leaving room..."));

			const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
			UGameplayStatics::OpenLevel(GetWorld(), FName(*Settings->GetLobbyMapPath()));
		}
		else
		{
			UE_LOG(LogWjWorld, Error, TEXT("PlayerControllerPlay: Failed to leave room"));
		}
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("PlayerControllerPlay: GameInstance is null"));
	}
}

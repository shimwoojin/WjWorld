// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Lobby/LobbyHUDWidget.h"
#include "UI/Profile/PlayerProfileWidget.h"
#include "Components/Button.h"
#include "Core/Local/Lobby/WjWorldGameModeLobby.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

void ULobbyHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 이벤트 바인딩
	if (CreateRoomButton)
	{
		CreateRoomButton->OnClicked.AddDynamic(this, &ULobbyHUDWidget::OnCreateRoomClicked);
	}

	if (FindRoomButton)
	{
		FindRoomButton->OnClicked.AddDynamic(this, &ULobbyHUDWidget::OnFindRoomClicked);
	}

	if (SettingsButton)
	{
		SettingsButton->OnClicked.AddDynamic(this, &ULobbyHUDWidget::OnSettingsClicked);
	}

	// Direct Connect 버튼
	if (DirectConnectButton)
	{
		DirectConnectButton->OnClicked.AddDynamic(this, &ULobbyHUDWidget::OnDirectConnectClicked);
	}

	// 프로필 버튼
	if (ProfileButton)
	{
		ProfileButton->OnClicked.AddDynamic(this, &ULobbyHUDWidget::OnProfileClicked);
	}

	UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: NativeConstruct completed"));
}

void ULobbyHUDWidget::OnCreateRoomClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: Create Room button clicked"));

	AWjWorldGameModeLobby* GameMode = Cast<AWjWorldGameModeLobby>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->ShowCreateRoomWindow();
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("LobbyHUDWidget: Failed to get WjWorldGameModeLobby"));
	}
}

void ULobbyHUDWidget::OnFindRoomClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: Find Room button clicked"));

	AWjWorldGameModeLobby* GameMode = Cast<AWjWorldGameModeLobby>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->ShowRoomListWindow();
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("LobbyHUDWidget: Failed to get WjWorldGameModeLobby"));
	}
}

void ULobbyHUDWidget::OnDirectConnectClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: Direct Connect button clicked"));

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogWjWorld, Error, TEXT("LobbyHUDWidget: PlayerController is null"));
		return;
	}

	// ⭐ 중요: ConsoleCommand "open"을 사용
	// ClientTravel은 자동으로 기본 맵을 붙이므로 대신 open 명령어 사용
	FString ConnectCommand = TEXT("open 127.0.0.1");
	
	UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: Executing console command: %s"), *ConnectCommand);
	PC->ConsoleCommand(ConnectCommand);
}

void ULobbyHUDWidget::OnSettingsClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: Settings button clicked"));
	// TODO: 설정 UI 표시
}

void ULobbyHUDWidget::OnProfileClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: Profile button clicked"));

	if (!ProfileWidgetClass)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("LobbyHUDWidget: ProfileWidgetClass is not set"));
		return;
	}

	// 이미 열려있으면 닫기
	if (ProfileWidgetInstance && ProfileWidgetInstance->GetVisibility() == ESlateVisibility::Visible)
	{
		ProfileWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 인스턴스가 없으면 생성
	if (!ProfileWidgetInstance)
	{
		ProfileWidgetInstance = CreateWidget<UPlayerProfileWidget>(GetOwningPlayer(), ProfileWidgetClass);
		if (ProfileWidgetInstance)
		{
			ProfileWidgetInstance->AddToViewport(100);
		}
	}

	if (ProfileWidgetInstance)
	{
		ProfileWidgetInstance->ShowLocalProfile();
	}
}

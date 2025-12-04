// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/WaitingRoom/WaitingRoomHUDWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Core/Local/WaitingRoom/WjWorldGameModeWaitingRoom.h"
#include "Core/WjWorldGameInstance.h"
#include "Core/Session/SessionManager.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

void UWaitingRoomHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 이벤트 바인딩
	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &UWaitingRoomHUDWidget::OnStartGameClicked);
	}

	if (LeaveButton)
	{
		LeaveButton->OnClicked.AddDynamic(this, &UWaitingRoomHUDWidget::OnLeaveClicked);
	}

	// 방 정보 업데이트
	UpdateRoomInfo();

	// 호스트가 아니면 게임 시작 버튼 비활성화
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (GameInstance && GameInstance->GetSessionManager())
	{
		bool bIsHost = GameInstance->GetSessionManager()->IsHost();
		if (StartGameButton)
		{
			StartGameButton->SetIsEnabled(bIsHost);
		}
		UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Is Host = %d"), bIsHost);
	}

	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: NativeConstruct completed"));
}

void UWaitingRoomHUDWidget::OnStartGameClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Start Game button clicked"));

	// GameMode의 StartGame 호출
	AWjWorldGameModeWaitingRoom* GameMode = Cast<AWjWorldGameModeWaitingRoom>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->StartGame();
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("WaitingRoomHUDWidget: Failed to get WjWorldGameModeWaitingRoom"));
	}
}

void UWaitingRoomHUDWidget::OnLeaveClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Leave button clicked"));

	// GameInstance를 통해 방 나가기
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		bool bSuccess = GameInstance->LeaveRoom();
		if (bSuccess)
		{
			UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Leaving room..."));
			
			// 로비로 이동
			UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Map/02-1_Lobby"));
		}
		else
		{
			UE_LOG(LogWjWorld, Error, TEXT("WaitingRoomHUDWidget: Failed to leave room"));
		}
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("WaitingRoomHUDWidget: GameInstance is null"));
	}
}

void UWaitingRoomHUDWidget::UpdateRoomInfo()
{
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (!GameInstance || !GameInstance->GetSessionManager())
	{
		return;
	}

	USessionManager* SessionManager = GameInstance->GetSessionManager();

	// 방 이름 표시
	if (RoomNameText)
	{
		FString RoomName = SessionManager->GetLastRoomSettings().RoomName;
		RoomNameText->SetText(FText::FromString(RoomName));
	}

	// 게임 모드 표시
	if (GameModeText)
	{
		FString GameMode = SessionManager->GetLastRoomSettings().GameMode;
		GameModeText->SetText(FText::FromString(GameMode));
	}

	// 플레이어 수 표시
	if (PlayerCountText)
	{
		AWjWorldGameModeWaitingRoom* GameMode = Cast<AWjWorldGameModeWaitingRoom>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			int32 CurrentPlayers = GameMode->GetPlayerCount();
			int32 MaxPlayers = SessionManager->GetLastRoomSettings().MaxPlayers;
			
			FString PlayerCountStr = FString::Printf(TEXT("%d / %d"), CurrentPlayers, MaxPlayers);
			PlayerCountText->SetText(FText::FromString(PlayerCountStr));
		}
	}

	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Room info updated"));
}

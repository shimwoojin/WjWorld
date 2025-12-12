// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/WaitingRoom/WaitingRoomHUDWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Core/Local/WaitingRoom/WjWorldGameModeWaitingRoom.h"
#include "Core/Base/WjWorldPlayerStateBase.h"
#include "Core/WjWorldGameInstance.h"
#include "Core/Session/SessionManager.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

void UWaitingRoomHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogWjWorld, Warning, TEXT("=== WaitingRoomHUDWidget: NativeConstruct START ==="));

	// 버튼 이벤트 바인딩
	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &UWaitingRoomHUDWidget::OnReadyClicked);
	}

	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &UWaitingRoomHUDWidget::OnStartGameClicked);
	}

	if (LeaveButton)
	{
		LeaveButton->OnClicked.AddDynamic(this, &UWaitingRoomHUDWidget::OnLeaveClicked);
	}

	// GameState 가져오기 및 이벤트 바인딩
	CachedGameState = GetWorld()->GetGameState<AWjWorldGameStateWaitingRoom>();
	if (CachedGameState)
	{
		// ⭐ 디버깅: GameState 정보 로그
		int32 PlayerCount = CachedGameState->GetPlayerCount();
		UE_LOG(LogWjWorld, Warning, TEXT("WaitingRoomHUDWidget: GameState found! PlayerCount = %d"), PlayerCount);
		
		// PlayerArray 내용 확인
		for (int32 i = 0; i < CachedGameState->PlayerArray.Num(); i++)
		{
			APlayerState* PS = CachedGameState->PlayerArray[i];
			if (PS)
			{
				UE_LOG(LogWjWorld, Warning, TEXT("  - PlayerState[%d]: %s (ID: %d)"), 
					i, *PS->GetPlayerName(), PS->GetPlayerId());
			}
		}

		// 델리게이트 바인딩
		CachedGameState->OnRoomInfoChanged.AddDynamic(this, &UWaitingRoomHUDWidget::OnRoomInfoChanged);
		CachedGameState->OnPlayerListChanged.AddDynamic(this, &UWaitingRoomHUDWidget::OnPlayerListChanged);

		// 초기 데이터 로드
		UpdateRoomInfo();
		UpdatePlayerList();
		UpdateStartGameButton();
		UpdateReadyButton();

		UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: GameState events bound"));
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("WaitingRoomHUDWidget: Failed to get GameStateWaitingRoom"));
	}

	// PlayerState 이벤트 바인딩
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		AWjWorldPlayerStateBase* PlayerState = PC->GetPlayerState<AWjWorldPlayerStateBase>();
		if (PlayerState)
		{
			PlayerState->OnReadyStateChanged.AddDynamic(this, &UWaitingRoomHUDWidget::OnPlayerReadyStateChanged);
			UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: PlayerState events bound - %s"), *PlayerState->GetPlayerName());
		}
		else
		{
			UE_LOG(LogWjWorld, Warning, TEXT("WaitingRoomHUDWidget: PlayerState is NULL"));
		}
	}

	UE_LOG(LogWjWorld, Warning, TEXT("=== WaitingRoomHUDWidget: NativeConstruct END ==="));
}

void UWaitingRoomHUDWidget::NativeDestruct()
{
	// 델리게이트 언바인딩
	if (CachedGameState)
	{
		CachedGameState->OnRoomInfoChanged.RemoveDynamic(this, &UWaitingRoomHUDWidget::OnRoomInfoChanged);
		CachedGameState->OnPlayerListChanged.RemoveDynamic(this, &UWaitingRoomHUDWidget::OnPlayerListChanged);
	}

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		AWjWorldPlayerStateBase* PlayerState = PC->GetPlayerState<AWjWorldPlayerStateBase>();
		if (PlayerState)
		{
			PlayerState->OnReadyStateChanged.RemoveDynamic(this, &UWaitingRoomHUDWidget::OnPlayerReadyStateChanged);
		}
	}

	Super::NativeDestruct();
}

void UWaitingRoomHUDWidget::OnReadyClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Ready button clicked"));

	// PlayerState의 준비 상태 토글
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		AWjWorldPlayerStateBase* PlayerState = PC->GetPlayerState<AWjWorldPlayerStateBase>();
		if (PlayerState)
		{
			PlayerState->ToggleReady();
		}
		else
		{
			UE_LOG(LogWjWorld, Error, TEXT("WaitingRoomHUDWidget: Failed to get PlayerState"));
		}
	}
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

void UWaitingRoomHUDWidget::OnRoomInfoChanged(const FRoomSettings& RoomSettings)
{
	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Room info changed event received"));
	UpdateRoomInfo();
}

void UWaitingRoomHUDWidget::OnPlayerListChanged(const TArray<FPlayerDisplayInfo>& PlayerList)
{
	UE_LOG(LogWjWorld, Warning, TEXT("WaitingRoomHUDWidget: Player list changed event received - %d players"), PlayerList.Num());
	
	// ⭐ 플레이어 목록 업데이트
	UpdatePlayerList();
	
	// ⭐ 플레이어 수도 함께 업데이트 (중요!)
	UpdateRoomInfo();
	
	// 게임 시작 버튼 상태 업데이트
	UpdateStartGameButton();
}

void UWaitingRoomHUDWidget::OnPlayerReadyStateChanged(int32 PlayerID, bool bIsReady)
{
	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Player %d ready state changed to %d"), PlayerID, bIsReady);
	UpdateReadyButton();
	UpdatePlayerList();
	UpdateStartGameButton();
}

void UWaitingRoomHUDWidget::UpdateRoomInfo()
{
	if (!CachedGameState)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("WaitingRoomHUDWidget::UpdateRoomInfo - CachedGameState is NULL"));
		return;
	}

	const FRoomSettings& Settings = CachedGameState->GetRoomSettings();

	// 방 이름 표시
	if (RoomNameText)
	{
		RoomNameText->SetText(FText::FromString(Settings.RoomName));
	}

	// 게임 모드 표시
	if (GameModeText)
	{
		GameModeText->SetText(FText::FromString(Settings.GameMode));
	}

	// ⭐ 플레이어 수 표시 (실시간으로 가져오기)
	if (PlayerCountText)
	{
		int32 CurrentPlayers = CachedGameState->GetPlayerCount();
		int32 MaxPlayers = Settings.MaxPlayers;
		
		FString PlayerCountStr = FString::Printf(TEXT("%d / %d"), CurrentPlayers, MaxPlayers);
		PlayerCountText->SetText(FText::FromString(PlayerCountStr));
		
		UE_LOG(LogWjWorld, Warning, TEXT("WaitingRoomHUDWidget::UpdateRoomInfo - PlayerCount: %s (Current=%d, Max=%d)"), 
			*PlayerCountStr, CurrentPlayers, MaxPlayers);
	}

	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Room info updated"));
}

void UWaitingRoomHUDWidget::UpdatePlayerList()
{
	if (!CachedGameState || !PlayerListContainer)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("WaitingRoomHUDWidget::UpdatePlayerList - GameState or Container is NULL"));
		return;
	}

	// 기존 목록 제거
	PlayerListContainer->ClearChildren();

	// 플레이어 목록 가져오기
	TArray<FPlayerDisplayInfo> PlayerList = CachedGameState->GetPlayerList();

	UE_LOG(LogWjWorld, Warning, TEXT("WaitingRoomHUDWidget: Updating player list - %d players"), PlayerList.Num());

	// 간단한 텍스트로 표시 (나중에 커스텀 위젯으로 교체 가능)
	for (const FPlayerDisplayInfo& Info : PlayerList)
	{
		UTextBlock* PlayerText = NewObject<UTextBlock>(this);
		if (PlayerText)
		{
			// 플레이어 정보 포맷: "[Host] PlayerName (Ready)" 또는 "PlayerName"
			FString DisplayText = Info.PlayerName;
			
			if (Info.bIsHost)
			{
				DisplayText = FString::Printf(TEXT("[Host] %s"), *DisplayText);
			}
			
			if (Info.bIsReady)
			{
				DisplayText += TEXT(" (Ready)");
			}

			PlayerText->SetText(FText::FromString(DisplayText));
			PlayerListContainer->AddChild(PlayerText);

			UE_LOG(LogWjWorld, Warning, TEXT("  - %s"), *DisplayText);
		}
	}
}

void UWaitingRoomHUDWidget::UpdateReadyButton()
{
	if (!ReadyButton || !ReadyButtonText)
	{
		return;
	}

	// 현재 플레이어의 준비 상태 확인
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		AWjWorldPlayerStateBase* PlayerState = PC->GetPlayerState<AWjWorldPlayerStateBase>();
		if (PlayerState)
		{
			bool bIsReady = PlayerState->IsReady();
			
			// 버튼 텍스트 변경
			if (bIsReady)
			{
				ReadyButtonText->SetText(FText::FromString(TEXT("Cancel Ready")));
			}
			else
			{
				ReadyButtonText->SetText(FText::FromString(TEXT("Ready")));
			}
		}
	}
}

void UWaitingRoomHUDWidget::UpdateStartGameButton()
{
	if (!StartGameButton || !CachedGameState)
	{
		return;
	}

	// 호스트 여부 확인
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (!GameInstance || !GameInstance->GetSessionManager())
	{
		return;
	}

	bool bIsHost = GameInstance->GetSessionManager()->IsHost();
	
	if (bIsHost)
	{
		// 호스트는 모든 플레이어가 준비되었을 때만 게임 시작 가능
		bool bAllReady = CachedGameState->AreAllPlayersReady();
		int32 PlayerCount = CachedGameState->GetPlayerCount();
		
		// 최소 1명 이상 있어야 시작 가능
		bool bCanStart = bAllReady && (PlayerCount > 0);
		
		StartGameButton->SetIsEnabled(bCanStart);
		
		UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Start button enabled = %d (AllReady=%d, Count=%d)"),
			bCanStart, bAllReady, PlayerCount);
	}
	else
	{
		// 호스트가 아니면 버튼 비활성화
		StartGameButton->SetIsEnabled(false);
	}
}

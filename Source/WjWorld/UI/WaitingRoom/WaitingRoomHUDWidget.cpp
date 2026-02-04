// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/WaitingRoom/WaitingRoomHUDWidget.h"
#include "UI/Profile/PlayerProfileWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Core/Local/WaitingRoom/WjWorldGameModeWaitingRoom.h"
#include "Core/Base/WjWorldPlayerStateBase.h"
#include "Core/WjWorldGameInstance.h"
#include "Core/Session/SessionManager.h"
#include "Setting/WjWorldDeveloperSettings.h"
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
			const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
			UGameplayStatics::OpenLevel(GetWorld(), FName(*Settings->GetLobbyMapPath()));
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

	// 플레이어 목록 가져오기 & 캐시
	CachedPlayerDisplayList = CachedGameState->GetPlayerList();

	UE_LOG(LogWjWorld, Warning, TEXT("WaitingRoomHUDWidget: Updating player list - %d players"), CachedPlayerDisplayList.Num());

	// 버튼으로 표시 (클릭 시 프로필 팝업)
	for (const FPlayerDisplayInfo& Info : CachedPlayerDisplayList)
	{
		UButton* PlayerButton = NewObject<UButton>(this);
		if (!PlayerButton)
		{
			continue;
		}

		// 버튼 안에 텍스트 추가
		UTextBlock* PlayerText = NewObject<UTextBlock>(this);
		if (PlayerText)
		{
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
			PlayerButton->AddChild(PlayerText);
		}

		// 모든 버튼에 공통 핸들러 바인딩 (IsHovered로 어떤 버튼인지 판별)
		PlayerButton->OnClicked.AddDynamic(this, &UWaitingRoomHUDWidget::OnAnyPlayerButtonClicked);

		PlayerListContainer->AddChild(PlayerButton);

		UE_LOG(LogWjWorld, Warning, TEXT("  - %s (ID: %d)"), *Info.PlayerName, Info.PlayerID);
	}
}

void UWaitingRoomHUDWidget::OnAnyPlayerButtonClicked()
{
	if (!PlayerListContainer)
	{
		return;
	}

	// 어떤 버튼이 클릭되었는지 IsHovered()로 판별
	for (int32 i = 0; i < PlayerListContainer->GetChildrenCount(); ++i)
	{
		UButton* Btn = Cast<UButton>(PlayerListContainer->GetChildAt(i));
		if (Btn && Btn->IsHovered() && CachedPlayerDisplayList.IsValidIndex(i))
		{
			ShowPlayerProfile(CachedPlayerDisplayList[i].PlayerID);
			return;
		}
	}
}

void UWaitingRoomHUDWidget::ShowPlayerProfile(int32 PlayerID)
{
	if (!CachedGameState)
	{
		return;
	}

	// PlayerID로 PlayerState 검색
	APlayerState* TargetPS = nullptr;
	for (APlayerState* PS : CachedGameState->PlayerArray)
	{
		if (PS && PS->GetPlayerId() == PlayerID)
		{
			TargetPS = PS;
			break;
		}
	}

	if (!TargetPS)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("WaitingRoomHUDWidget: PlayerState not found for ID %d"), PlayerID);
		return;
	}

	// 프로필 위젯 생성
	if (!ProfileWidgetClass)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("WaitingRoomHUDWidget: ProfileWidgetClass is not set"));
		return;
	}

	if (!ProfileWidgetInstance)
	{
		ProfileWidgetInstance = CreateWidget<UPlayerProfileWidget>(GetOwningPlayer(), ProfileWidgetClass);
		if (ProfileWidgetInstance)
		{
			ProfileWidgetInstance->AddToViewport(100);
		}
	}

	if (!ProfileWidgetInstance)
	{
		return;
	}

	// 자기 자신인지 확인
	APlayerController* LocalPC = GetOwningPlayer();
	if (LocalPC && LocalPC->PlayerState && LocalPC->PlayerState->GetPlayerId() == PlayerID)
	{
		// 자기 자신 → ShowLocalProfile
		ProfileWidgetInstance->ShowLocalProfile();
	}
	else
	{
		// 타 플레이어
		AWjWorldPlayerStateBase* BasePS = Cast<AWjWorldPlayerStateBase>(TargetPS);
		FUniqueNetIdRepl UniqueId = TargetPS->GetUniqueId();
		FText PlayerName = FText::FromString(TargetPS->GetPlayerName());
		FCosmeticLoadout Loadout;
		if (BasePS)
		{
			Loadout = BasePS->GetCosmeticLoadout();
		}

		ProfileWidgetInstance->ShowPlayerProfile(UniqueId, PlayerName, Loadout);
	}

	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: ShowPlayerProfile for ID %d"), PlayerID);
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

// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/WaitingRoom/WaitingRoomHUDWidget.h"
#include "UI/Profile/PlayerProfileWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/ComboBoxString.h"
#include "Components/CanvasPanel.h"
#include "Core/Local/WaitingRoom/WjWorldGameModeWaitingRoom.h"
#include "Core/Base/WjWorldPlayerStateBase.h"
#include "Core/WjWorldGameInstance.h"
#include "Core/Session/SessionManager.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "DataAsset/WjWorldMinigameDataAsset.h"
#include "GamePlay/Wall/WjWorldWallDescriptionDataAsset.h"
#include "GamePlay/JumpMap/JumpMapLayoutDataAsset.h"
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

	if (ApplySettingsButton)
	{
		ApplySettingsButton->OnClicked.AddDynamic(this, &UWaitingRoomHUDWidget::OnApplySettingsClicked);
	}

	if (GameModeComboBox)
	{
		GameModeComboBox->OnSelectionChanged.AddDynamic(this, &UWaitingRoomHUDWidget::OnGameModeSelectionChanged);
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

		// 호스트 설정 패널 초기화
		InitializeHostSettingsPanel();
		UpdateHostSettingsPanelVisibility();

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

	if (GameModeComboBox)
	{
		GameModeComboBox->OnSelectionChanged.RemoveDynamic(this, &UWaitingRoomHUDWidget::OnGameModeSelectionChanged);
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

	// ⭐ 게임 시작 전에 현재 UI 설정을 자동으로 적용 (Apply Settings 버튼을 안 눌러도 반영되도록)
	ApplyCurrentUISettings();

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
	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Room info changed event received - GameMode: %s, Map: %s"),
		*RoomSettings.GameMode, *RoomSettings.MapName);

	// ⭐ 직접 전달받은 설정으로 UI 업데이트 (타이밍 이슈 방지)
	UpdateRoomInfo(&RoomSettings);

	// 호스트 설정 패널의 ComboBox 선택도 업데이트 (초기화 타이밍 이슈 해결)
	if (GameModeComboBox && !RoomSettings.GameMode.IsEmpty())
	{
		GameModeComboBox->SetSelectedOption(RoomSettings.GameMode);
		UpdateMapComboBoxForGameMode(RoomSettings.GameMode);

		// 맵 선택도 업데이트
		if (MapComboBox && !RoomSettings.MapName.IsEmpty())
		{
			for (const auto& Pair : MapOptionDisplayToValue)
			{
				if (Pair.Value == RoomSettings.MapName)
				{
					MapComboBox->SetSelectedOption(Pair.Key);
					break;
				}
			}
		}
	}
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

void UWaitingRoomHUDWidget::UpdateRoomInfo(const FRoomSettings* InSettings)
{
	// 설정이 직접 전달되면 사용, 아니면 GameState에서 읽음
	FRoomSettings Settings;
	if (InSettings)
	{
		Settings = *InSettings;
	}
	else if (CachedGameState)
	{
		Settings = CachedGameState->GetRoomSettings();
	}
	else
	{
		UE_LOG(LogWjWorld, Warning, TEXT("WaitingRoomHUDWidget::UpdateRoomInfo - CachedGameState is NULL"));
		return;
	}

	UE_LOG(LogWjWorld, Warning, TEXT("WaitingRoomHUDWidget::UpdateRoomInfo - Updating with: RoomName='%s', GameMode='%s', MapName='%s'"),
		*Settings.RoomName, *Settings.GameMode, *Settings.MapName);

	// 방 이름 표시
	if (RoomNameText)
	{
		RoomNameText->SetText(FText::FromString(Settings.RoomName));
	}

	// 게임 모드 표시
	if (GameModeText)
	{
		GameModeText->SetText(FText::FromString(Settings.GameMode));
		UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: GameModeText set to '%s'"), *Settings.GameMode);
	}

	// 맵 이름 표시
	if (MapText)
	{
		FString MapDisplayName = Settings.MapName;
		// User_ 접두사 제거하여 표시
		MapDisplayName.RemoveFromStart(TEXT("User_"));
		MapText->SetText(FText::FromString(MapDisplayName));
		UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: MapText set to '%s'"), *MapDisplayName);
	}

	// ⭐ 플레이어 수 표시 (실시간으로 가져오기)
	if (PlayerCountText && CachedGameState)
	{
		int32 CurrentPlayers = CachedGameState->GetPlayerCount();
		int32 MaxPlayers = Settings.MaxPlayers;

		FString PlayerCountStr = FString::Printf(TEXT("%d / %d"), CurrentPlayers, MaxPlayers);
		PlayerCountText->SetText(FText::FromString(PlayerCountStr));

		UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: PlayerCountText set to '%s'"), *PlayerCountStr);
	}

	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Room info updated successfully"));
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
	PlayerButtonToIDMap.Empty();

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

		// 버튼 → PlayerID 매핑 저장
		PlayerButtonToIDMap.Add(PlayerButton, Info.PlayerID);

		// 모든 버튼에 공통 핸들러 바인딩
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

	// 클릭된 버튼 찾기 (IsHovered + HasMouseCapture 조합으로 더 안정적으로 판별)
	for (const auto& Pair : PlayerButtonToIDMap)
	{
		UButton* Btn = Pair.Key;
		if (Btn && (Btn->IsHovered() || Btn->HasMouseCapture()))
		{
			UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Player button clicked - PlayerID: %d"), Pair.Value);
			ShowPlayerProfile(Pair.Value);
			return;
		}
	}

	UE_LOG(LogWjWorld, Warning, TEXT("WaitingRoomHUDWidget: OnAnyPlayerButtonClicked - Could not find clicked button"));
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

void UWaitingRoomHUDWidget::InitializeHostSettingsPanel()
{
	if (!GameModeComboBox || !MapComboBox)
	{
		return;
	}

	const UWjWorldDeveloperSettings* DevSettings = GetDefault<UWjWorldDeveloperSettings>();
	if (!DevSettings || DevSettings->MinigameCatalog.IsNull())
	{
		return;
	}

	UWjWorldMinigameDataAsset* Catalog = DevSettings->MinigameCatalog.LoadSynchronous();
	if (!Catalog)
	{
		return;
	}

	GameModeComboBox->ClearOptions();
	for (const FWjWorldMinigameDefinition& Def : Catalog->Minigames)
	{
		GameModeComboBox->AddOption(Def.GameModeId.ToString());
	}

	if (CachedGameState)
	{
		const FRoomSettings& Settings = CachedGameState->GetRoomSettings();
		GameModeComboBox->SetSelectedOption(Settings.GameMode);
		UpdateMapComboBoxForGameMode(Settings.GameMode);

		if (!Settings.MapName.IsEmpty())
		{
			for (const auto& Pair : MapOptionDisplayToValue)
			{
				if (Pair.Value == Settings.MapName)
				{
					MapComboBox->SetSelectedOption(Pair.Key);
					break;
				}
			}
		}
	}

	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Host settings panel initialized"));
}

void UWaitingRoomHUDWidget::UpdateHostSettingsPanelVisibility()
{
	if (!HostSettingsPanel)
	{
		return;
	}

	bool bIsHost = false;

	// 1. SessionManager에서 호스트 여부 확인
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (GameInstance && GameInstance->GetSessionManager())
	{
		bIsHost = GameInstance->GetSessionManager()->IsHost();
	}

	// 2. 추가 검증: Listen Server인지 확인 (클라이언트에서 잘못된 bIsHost 값 방지)
	UWorld* World = GetWorld();
	if (World)
	{
		ENetMode NetMode = World->GetNetMode();
		// 클라이언트(NM_Client)는 절대 호스트가 아님
		if (NetMode == NM_Client)
		{
			bIsHost = false;
		}
		// Listen Server(NM_ListenServer)만 호스트임
		else if (NetMode == NM_ListenServer || NetMode == NM_DedicatedServer)
		{
			// SessionManager 값 유지
		}
		else if (NetMode == NM_Standalone)
		{
			// 싱글플레이는 호스트 설정 불필요
			bIsHost = false;
		}
	}

	// 호스트만 설정 패널 표시 (클라이언트는 Display Text로 확인)
	HostSettingsPanel->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: HostSettingsPanel visible=%s (bIsHost=%d, NetMode=%d)"),
		bIsHost ? TEXT("Yes") : TEXT("No"), bIsHost, World ? (int32)World->GetNetMode() : -1);
}

void UWaitingRoomHUDWidget::UpdateMapComboBoxForGameMode(const FString& GameModeId)
{
	if (!MapComboBox)
	{
		return;
	}

	MapComboBox->ClearOptions();
	MapOptionDisplayToValue.Empty();

	const UWjWorldDeveloperSettings* DevSettings = GetDefault<UWjWorldDeveloperSettings>();
	if (!DevSettings || DevSettings->MinigameCatalog.IsNull())
	{
		return;
	}

	UWjWorldMinigameDataAsset* Catalog = DevSettings->MinigameCatalog.LoadSynchronous();
	if (!Catalog)
	{
		return;
	}

	const FWjWorldMinigameDefinition* Def = Catalog->FindByGameModeId(FName(*GameModeId));
	if (Def)
	{
		for (const FWjWorldMinigameMapOption& Option : Def->MapOptions)
		{
			FString DisplayName = Option.DisplayName.ToString();
			FString OptionValue = Option.OptionValue;

			MapComboBox->AddOption(DisplayName);
			MapOptionDisplayToValue.Add(DisplayName, OptionValue);
		}
	}

	// ApproachingWall인 경우 유저 레이아웃도 추가
	if (GameModeId == TEXT("ApproachingWall"))
	{
		if (!DevSettings->WallDescriptionAsset.IsNull())
		{
			UWjWorldWallDescriptionDataAsset* WallAsset = DevSettings->WallDescriptionAsset.LoadSynchronous();
			if (WallAsset)
			{
				TArray<FWjWorldWallDescription> UserLayouts;
				WallAsset->ScanUserWallLayouts(UserLayouts);

				for (const FWjWorldWallDescription& Layout : UserLayouts)
				{
					FString DisplayName = Layout.WallName;
					DisplayName.RemoveFromStart(TEXT("User_"));
					FString DisplayStr = FString::Printf(TEXT("[User] %s"), *DisplayName);

					MapComboBox->AddOption(DisplayStr);
					MapOptionDisplayToValue.Add(DisplayStr, Layout.WallName);
				}

				UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Added %d AW user layouts"), UserLayouts.Num());
			}
		}
	}

	// JumpMap인 경우 유저 레이아웃도 추가
	if (GameModeId == TEXT("JumpMap"))
	{
		if (!DevSettings->JumpMapLayoutDataAsset.IsNull())
		{
			UJumpMapLayoutDataAsset* LayoutAsset = DevSettings->JumpMapLayoutDataAsset.LoadSynchronous();
			if (LayoutAsset)
			{
				TArray<FJumpMapLayout> UserLayouts;
				LayoutAsset->ScanUserJumpMapLayouts(UserLayouts);

				for (const FJumpMapLayout& Layout : UserLayouts)
				{
					FString DisplayName = Layout.LayoutName;
					DisplayName.RemoveFromStart(TEXT("User_"));
					FString DisplayStr = FString::Printf(TEXT("[User] %s"), *DisplayName);

					MapComboBox->AddOption(DisplayStr);
					MapOptionDisplayToValue.Add(DisplayStr, Layout.LayoutName);
				}

				UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Added %d JumpMap user layouts"), UserLayouts.Num());
			}
		}
	}

	if (MapComboBox->GetOptionCount() > 0)
	{
		MapComboBox->SetSelectedIndex(0);
	}

	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Map options updated for GameMode '%s' - %d options"),
		*GameModeId, MapComboBox->GetOptionCount());
}

void UWaitingRoomHUDWidget::OnGameModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct)
	{
		return;
	}

	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Game mode selection changed to '%s'"), *SelectedItem);
	UpdateMapComboBoxForGameMode(SelectedItem);
}

void UWaitingRoomHUDWidget::OnApplySettingsClicked()
{
	UE_LOG(LogWjWorld, Warning, TEXT("=== WaitingRoomHUDWidget::OnApplySettingsClicked START ==="));

	if (!CachedGameState)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WaitingRoomHUDWidget: CachedGameState is NULL!"));
		return;
	}
	if (!GameModeComboBox)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WaitingRoomHUDWidget: GameModeComboBox is NULL!"));
		return;
	}
	if (!MapComboBox)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WaitingRoomHUDWidget: MapComboBox is NULL!"));
		return;
	}

	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (!GameInstance || !GameInstance->GetSessionManager() || !GameInstance->GetSessionManager()->IsHost())
	{
		UE_LOG(LogWjWorld, Warning, TEXT("WaitingRoomHUDWidget: Only host can change settings (IsHost=%d)"),
			GameInstance && GameInstance->GetSessionManager() ? GameInstance->GetSessionManager()->IsHost() : false);
		return;
	}

	FRoomSettings OldSettings = CachedGameState->GetRoomSettings();
	FRoomSettings NewSettings = OldSettings;

	FString SelectedGameMode = GameModeComboBox->GetSelectedOption();
	NewSettings.GameMode = SelectedGameMode;

	FString SelectedMapDisplay = MapComboBox->GetSelectedOption();
	if (const FString* MapValue = MapOptionDisplayToValue.Find(SelectedMapDisplay))
	{
		NewSettings.MapName = *MapValue;
	}
	else
	{
		NewSettings.MapName = SelectedMapDisplay;
	}

	UE_LOG(LogWjWorld, Warning, TEXT("WaitingRoomHUDWidget: Applying settings change:"));
	UE_LOG(LogWjWorld, Warning, TEXT("  GameMode: '%s' -> '%s'"), *OldSettings.GameMode, *NewSettings.GameMode);
	UE_LOG(LogWjWorld, Warning, TEXT("  MapName: '%s' -> '%s'"), *OldSettings.MapName, *NewSettings.MapName);

	CachedGameState->UpdateRoomSettings(NewSettings);

	// ⭐ 호스트 로컬에서 Display Text 직접 업데이트 (델리게이트 타이밍 안전장치)
	UpdateRoomInfo(&NewSettings);

	UE_LOG(LogWjWorld, Warning, TEXT("=== WaitingRoomHUDWidget::OnApplySettingsClicked END ==="));
}

void UWaitingRoomHUDWidget::ApplyCurrentUISettings()
{
	// 호스트가 아니면 무시
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (!GameInstance || !GameInstance->GetSessionManager() || !GameInstance->GetSessionManager()->IsHost())
	{
		return;
	}

	// 필요한 컴포넌트가 없으면 무시
	if (!CachedGameState || !GameModeComboBox || !MapComboBox)
	{
		return;
	}

	FRoomSettings NewSettings = CachedGameState->GetRoomSettings();

	// 현재 UI에서 선택된 게임 모드
	FString SelectedGameMode = GameModeComboBox->GetSelectedOption();
	if (!SelectedGameMode.IsEmpty())
	{
		NewSettings.GameMode = SelectedGameMode;
	}

	// 현재 UI에서 선택된 맵
	FString SelectedMapDisplay = MapComboBox->GetSelectedOption();
	if (const FString* MapValue = MapOptionDisplayToValue.Find(SelectedMapDisplay))
	{
		NewSettings.MapName = *MapValue;
	}
	else if (!SelectedMapDisplay.IsEmpty())
	{
		NewSettings.MapName = SelectedMapDisplay;
	}

	UE_LOG(LogWjWorld, Log, TEXT("WaitingRoomHUDWidget: Auto-applying UI settings before game start - GameMode: '%s', Map: '%s'"),
		*NewSettings.GameMode, *NewSettings.MapName);

	CachedGameState->UpdateRoomSettings(NewSettings);
}

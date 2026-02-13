// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Session/CreateRoomWindow.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
#include "Core/WjWorldGameInstance.h"
#include "Core/Session/SessionManager.h"
#include "DataAsset/WjWorldMinigameDataAsset.h"
#include "GamePlay/Wall/WjWorldWallDescriptionDataAsset.h"
#include "GamePlay/JumpMap/JumpMapLayoutDataAsset.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

void UCreateRoomWindow::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 이벤트 바인딩
	if (DecreasePlayersButton)
	{
		DecreasePlayersButton->OnClicked.AddDynamic(this, &UCreateRoomWindow::OnDecreasePlayersClicked);
	}

	if (IncreasePlayersButton)
	{
		IncreasePlayersButton->OnClicked.AddDynamic(this, &UCreateRoomWindow::OnIncreasePlayersClicked);
	}

	if (PrivateCheckBox)
	{
		PrivateCheckBox->OnCheckStateChanged.AddDynamic(this, &UCreateRoomWindow::OnPrivateCheckBoxChanged);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UCreateRoomWindow::OnCancelClicked);
	}

	if (CreateButton)
	{
		CreateButton->OnClicked.AddDynamic(this, &UCreateRoomWindow::OnCreateClicked);
	}

	// 카탈로그 로드 및 옵션 초기화
	const UWjWorldDeveloperSettings* DevSettings = GetDefault<UWjWorldDeveloperSettings>();
	if (DevSettings && !DevSettings->MinigameCatalog.IsNull())
	{
		MinigameCatalog = DevSettings->MinigameCatalog.LoadSynchronous();
	}

	InitializeNetworkModeOptions();
	InitializeGameModeOptions();
	InitializeMapOptions();

	// 초기 인원 표시
	UpdateMaxPlayersDisplay();

	// 비밀번호 필드 초기 비활성화
	if (PasswordTextBox)
	{
		PasswordTextBox->SetIsEnabled(false);
	}

	// SessionManager 델리게이트 바인딩
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (GameInstance && GameInstance->GetSessionManager())
	{
		// ⭐ 중복 바인딩 방지
		GameInstance->GetSessionManager()->OnRoomCreatedEvent.RemoveDynamic(this, &UCreateRoomWindow::OnRoomCreated);
		GameInstance->GetSessionManager()->OnRoomCreatedEvent.AddDynamic(this, &UCreateRoomWindow::OnRoomCreated);
	}

	UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: NativeConstruct completed"));
}

void UCreateRoomWindow::ShowPopup()
{
	// 화면에 추가
	AddToViewport(100); // 높은 Z-Order로 최상단 표시

	// 입력 모드 변경
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(RoomNameTextBox->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}

	// 기본값 설정
	if (RoomNameTextBox)
	{
		RoomNameTextBox->SetText(FText::FromString(TEXT("New Room")));
	}

	if (AllowJoinInProgressCheckBox)
	{
		AllowJoinInProgressCheckBox->SetIsChecked(true);
	}

	UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Popup shown"));
}

void UCreateRoomWindow::ClosePopup()
{
	// 화면에서 제거
	RemoveFromParent();

	// 입력 모드 복원
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->SetInputMode(FInputModeGameAndUI());
		PC->bShowMouseCursor = true;
	}

	UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Popup closed"));
}

void UCreateRoomWindow::OnDecreasePlayersClicked()
{
	if (CurrentMaxPlayers > MIN_PLAYERS)
	{
		CurrentMaxPlayers--;
		UpdateMaxPlayersDisplay();
		UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Max players decreased to %d"), CurrentMaxPlayers);
	}
}

void UCreateRoomWindow::OnIncreasePlayersClicked()
{
	if (CurrentMaxPlayers < MAX_PLAYERS)
	{
		CurrentMaxPlayers++;
		UpdateMaxPlayersDisplay();
		UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Max players increased to %d"), CurrentMaxPlayers);
	}
}

void UCreateRoomWindow::OnPrivateCheckBoxChanged(bool bIsChecked)
{
	// 비공개 방이면 비밀번호 입력 활성화
	if (PasswordTextBox)
	{
		PasswordTextBox->SetIsEnabled(bIsChecked);
		
		if (!bIsChecked)
		{
			PasswordTextBox->SetText(FText::GetEmpty());
		}
	}

	UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Private room %s"), bIsChecked ? TEXT("enabled") : TEXT("disabled"));
}

void UCreateRoomWindow::OnCancelClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Cancel clicked"));
	ClosePopup();
}

void UCreateRoomWindow::OnCreateClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Create clicked"));

	// 입력 유효성 검사
	FString ErrorMessage;
	if (!ValidateInput(ErrorMessage))
	{
		UE_LOG(LogWjWorld, Warning, TEXT("CreateRoomWindow: Validation failed - %s"), *ErrorMessage);
		// TODO: 에러 메시지 UI 표시
		return;
	}

	// RoomSettings 생성
	FRoomSettings Settings = BuildRoomSettings();

	// GameInstance를 통해 방 생성
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		bool bSuccess = GameInstance->CreateRoom(Settings);
		if (bSuccess)
		{
			UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Room creation initiated"));
		}
		else
		{
			UE_LOG(LogWjWorld, Error, TEXT("CreateRoomWindow: Failed to initiate room creation"));
		}
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("CreateRoomWindow: GameInstance is null"));
	}
}

void UCreateRoomWindow::OnRoomCreated(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Room created successfully!"));

		// 팝업 닫기
		ClosePopup();

		// Lobby 맵을 Listen Server로 열되, GameMode는 WaitingRoomGameMode로 오버라이드
		const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
		UGameplayStatics::OpenLevel(GetWorld(), FName(*Settings->GetWaitingRoomOpenLevelURL()));
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("CreateRoomWindow: Room creation failed"));
		// TODO: 에러 메시지 UI 표시
	}
}

void UCreateRoomWindow::UpdateMaxPlayersDisplay()
{
	if (MaxPlayersText)
	{
		FString DisplayText = FString::Printf(TEXT("%d명"), CurrentMaxPlayers);
		MaxPlayersText->SetText(FText::FromString(DisplayText));
	}
}

bool UCreateRoomWindow::ValidateInput(FString& OutErrorMessage)
{
	// 방 이름 검사
	if (!RoomNameTextBox)
	{
		OutErrorMessage = TEXT("Room name field is missing");
		return false;
	}

	FString RoomName = RoomNameTextBox->GetText().ToString();
	if (RoomName.IsEmpty())
	{
		OutErrorMessage = TEXT("방 이름을 입력해주세요");
		return false;
	}

	if (RoomName.Len() > 20)
	{
		OutErrorMessage = TEXT("방 이름은 20자 이내로 입력해주세요");
		return false;
	}

	// 비공개 방인 경우 비밀번호 검사
	if (PrivateCheckBox && PrivateCheckBox->IsChecked())
	{
		if (PasswordTextBox)
		{
			FString Password = PasswordTextBox->GetText().ToString();
			if (Password.IsEmpty())
			{
				OutErrorMessage = TEXT("비공개 방은 비밀번호를 입력해주세요");
				return false;
			}

			if (Password.Len() < 4)
			{
				OutErrorMessage = TEXT("비밀번호는 4자 이상 입력해주세요");
				return false;
			}
		}
	}

	// 게임 모드 검사
	if (!GameModeComboBox || GameModeComboBox->GetSelectedOption().IsEmpty())
	{
		OutErrorMessage = TEXT("게임 모드를 선택해주세요");
		return false;
	}

	// 맵 검사
	if (!MapComboBox || MapComboBox->GetSelectedOption().IsEmpty())
	{
		OutErrorMessage = TEXT("맵을 선택해주세요");
		return false;
	}

	return true;
}

FRoomSettings UCreateRoomWindow::BuildRoomSettings()
{
	FRoomSettings Settings;

	// 방 이름
	if (RoomNameTextBox)
	{
		Settings.RoomName = RoomNameTextBox->GetText().ToString();
	}

	// 게임 모드: DisplayName → GameModeId 변환
	if (GameModeComboBox)
	{
		FString SelectedDisplay = GameModeComboBox->GetSelectedOption();
		if (const FName* FoundId = GameModeDisplayToId.Find(SelectedDisplay))
		{
			Settings.GameMode = FoundId->ToString();
		}
		else
		{
			Settings.GameMode = SelectedDisplay;
		}
	}

	// 맵: DisplayName → OptionValue 변환
	if (MapComboBox)
	{
		FString SelectedDisplay = MapComboBox->GetSelectedOption();
		if (const FString* FoundValue = MapOptionDisplayToValue.Find(SelectedDisplay))
		{
			Settings.MapName = *FoundValue;
		}
		else
		{
			Settings.MapName = SelectedDisplay;
		}
	}

	// 최대 인원
	Settings.MaxPlayers = CurrentMaxPlayers;

	// 비공개 방
	if (PrivateCheckBox)
	{
		Settings.bIsPrivate = PrivateCheckBox->IsChecked();
	}

	// 비밀번호
	if (Settings.bIsPrivate && PasswordTextBox)
	{
		Settings.Password = PasswordTextBox->GetText().ToString();
	}

	// 게임 중 입장 허용
	if (AllowJoinInProgressCheckBox)
	{
		Settings.bAllowJoinInProgress = AllowJoinInProgressCheckBox->IsChecked();
	}

	// 네트워크 모드
	if (NetworkModeComboBox)
	{
		FString SelectedMode = NetworkModeComboBox->GetSelectedOption();
		Settings.NetworkMode = (SelectedMode == TEXT("Steam")) ? ENetworkMode::Steam : ENetworkMode::LAN;
	}
	else
	{
		Settings.NetworkMode = ENetworkMode::LAN; // 기본값
	}

	UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Built settings - Name: %s, Mode: %s, Map: %s, Players: %d, Network: %s"),
		*Settings.RoomName, *Settings.GameMode, *Settings.MapName, Settings.MaxPlayers,
		Settings.NetworkMode == ENetworkMode::Steam ? TEXT("Steam") : TEXT("LAN"));

	return Settings;
}

void UCreateRoomWindow::InitializeGameModeOptions()
{
	if (!GameModeComboBox)
	{
		return;
	}

	GameModeComboBox->ClearOptions();
	GameModeDisplayToId.Empty();

	if (MinigameCatalog)
	{
		for (const FWjWorldMinigameDefinition& Def : MinigameCatalog->Minigames)
		{
			FString DisplayStr = Def.DisplayName.ToString();
			GameModeComboBox->AddOption(DisplayStr);
			GameModeDisplayToId.Add(DisplayStr, Def.GameModeId);
		}

		// 첫 번째 항목 기본 선택
		if (MinigameCatalog->Minigames.Num() > 0)
		{
			GameModeComboBox->SetSelectedOption(MinigameCatalog->Minigames[0].DisplayName.ToString());
		}
	}

	// 게임모드 변경 콜백 바인딩
	GameModeComboBox->OnSelectionChanged.AddDynamic(this, &UCreateRoomWindow::OnGameModeSelectionChanged);

	UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Game mode options initialized (%d items)"), GameModeDisplayToId.Num());
}

void UCreateRoomWindow::OnGameModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	// 선택된 게임모드에 맞는 맵 옵션으로 갱신
	InitializeMapOptions();

	UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Game mode changed to '%s'"), *SelectedItem);
}

void UCreateRoomWindow::InitializeMapOptions()
{
	if (!MapComboBox)
	{
		return;
	}

	MapComboBox->ClearOptions();
	MapOptionDisplayToValue.Empty();

	FName CurrentGameModeId = NAME_None;

	if (MinigameCatalog && GameModeComboBox)
	{
		FString SelectedGameMode = GameModeComboBox->GetSelectedOption();
		if (const FName* GameModeId = GameModeDisplayToId.Find(SelectedGameMode))
		{
			CurrentGameModeId = *GameModeId;

			if (const FWjWorldMinigameDefinition* Def = MinigameCatalog->FindByGameModeId(*GameModeId))
			{
				// 1. 기본 맵 옵션 추가
				for (const FWjWorldMinigameMapOption& MapOption : Def->MapOptions)
				{
					FString DisplayStr = MapOption.DisplayName.ToString();
					MapComboBox->AddOption(DisplayStr);
					MapOptionDisplayToValue.Add(DisplayStr, MapOption.OptionValue);
				}

				// 첫 번째 항목 기본 선택
				if (Def->MapOptions.Num() > 0)
				{
					MapComboBox->SetSelectedOption(Def->MapOptions[0].DisplayName.ToString());
				}
			}
		}
	}

	// 2. 유저 커스텀 맵 옵션 추가
	if (!CurrentGameModeId.IsNone())
	{
		AddUserMapOptions(CurrentGameModeId);
	}

	UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Map options initialized (%d items)"), MapOptionDisplayToValue.Num());
}

void UCreateRoomWindow::AddUserMapOptions(FName GameModeId)
{
	if (!MapComboBox)
	{
		return;
	}

	const UWjWorldDeveloperSettings* DevSettings = GetDefault<UWjWorldDeveloperSettings>();
	if (!DevSettings)
	{
		return;
	}

	// ApproachingWall: WallDescriptionDataAsset에서 유저 레이아웃 스캔
	if (GameModeId == FName("ApproachingWall"))
	{
		if (!DevSettings->WallDescriptionAsset.IsNull())
		{
			UWjWorldWallDescriptionDataAsset* WallAsset = DevSettings->WallDescriptionAsset.LoadSynchronous();
			if (WallAsset)
			{
				TArray<FWjWorldWallDescription> UserLayouts;
				int32 Count = WallAsset->ScanUserWallLayouts(UserLayouts);

				for (const FWjWorldWallDescription& Layout : UserLayouts)
				{
					// "[User] 레이아웃명" 형식으로 표시 (WallName에서 User_ 접두사 제거)
					FString DisplayName = Layout.WallName;
					DisplayName.RemoveFromStart(TEXT("User_"));
					FString DisplayStr = FString::Printf(TEXT("[User] %s"), *DisplayName);
					MapComboBox->AddOption(DisplayStr);
					// OptionValue는 WallName 그대로 (이미 User_ 접두사 포함)
					MapOptionDisplayToValue.Add(DisplayStr, Layout.WallName);
				}

				if (Count > 0)
				{
					UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Added %d user layouts for ApproachingWall"), Count);
				}
			}
		}
	}
	else if (GameModeId == FName("JumpMap"))
	{
		if (!DevSettings->JumpMapLayoutDataAsset.IsNull())
		{
			UJumpMapLayoutDataAsset* JumpMapAsset = DevSettings->JumpMapLayoutDataAsset.LoadSynchronous();
			if (JumpMapAsset)
			{
				TArray<FJumpMapLayout> UserLayouts;
				int32 Count = JumpMapAsset->ScanUserJumpMapLayouts(UserLayouts);

				for (const FJumpMapLayout& Layout : UserLayouts)
				{
					FString DisplayName = Layout.LayoutName;
					DisplayName.RemoveFromStart(TEXT("User_"));
					FString DisplayStr = FString::Printf(TEXT("[User] %s"), *DisplayName);
					MapComboBox->AddOption(DisplayStr);
					MapOptionDisplayToValue.Add(DisplayStr, Layout.LayoutName);
				}

				if (Count > 0)
				{
					UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Added %d user layouts for JumpMap"), Count);
				}
			}
		}
	}
}

void UCreateRoomWindow::InitializeNetworkModeOptions()
{
	if (!NetworkModeComboBox)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("CreateRoomWindow: NetworkModeComboBox not found (optional)"));
		return;
	}

	NetworkModeComboBox->ClearOptions();

	// LAN 옵션 추가
	NetworkModeComboBox->AddOption(TEXT("LAN"));

	// Steam 옵션 추가 (Steam 빌드에서만 활성화)
#if WITH_STEAM
	NetworkModeComboBox->AddOption(TEXT("Steam"));
#endif

	// 기본값은 LAN
	NetworkModeComboBox->SetSelectedOption(TEXT("LAN"));

	UE_LOG(LogWjWorld, Log, TEXT("CreateRoomWindow: Network mode options initialized"));
}

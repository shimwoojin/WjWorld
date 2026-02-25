// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Session/RoomListWindow.h"
#include "UI/Session/RoomListEntryWidget.h"
#include "UI/Session/PasswordInputWidget.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/ComboBoxString.h"
#include "Core/WjWorldGameInstance.h"
#include "Core/Session/SessionManager.h"
#include "WjWorldLogCategories.h"

void URoomListWindow::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 이벤트 바인딩
	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &URoomListWindow::OnRefreshClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &URoomListWindow::OnCloseClicked);
	}

	// 네트워크 모드 옵션 초기화
	InitializeNetworkModeOptions();

	// SessionManager 델리게이트 바인딩
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (GameInstance && GameInstance->GetSessionManager())
	{
		GameInstance->GetSessionManager()->OnRoomsFoundEvent.AddDynamic(this, &URoomListWindow::OnRoomsFound);
	}

	UE_LOG(LogWjWorld, Log, TEXT("RoomListWindow: NativeConstruct completed"));
}

void URoomListWindow::ShowPopup()
{
#if WITH_STEAM
	ShowPopupWithNetworkMode(ENetworkMode::Steam);
#else
	ShowPopupWithNetworkMode(ENetworkMode::LAN);
#endif
}

void URoomListWindow::ShowPopupWithNetworkMode(ENetworkMode InNetworkMode)
{
	CurrentNetworkMode = InNetworkMode;

	// 콤보박스 선택 동기화
	if (NetworkModeComboBox)
	{
		FString ModeStr = (InNetworkMode == ENetworkMode::Steam) ? TEXT("Steam") : TEXT("LAN");
		NetworkModeComboBox->SetSelectedOption(ModeStr);
	}

	// 화면에 추가
	AddToViewport(100); // 높은 Z-Order

	// 입력 모드 변경
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}

	// 자동으로 방 검색 시작
	StartSearching();

	UE_LOG(LogWjWorld, Log, TEXT("RoomListWindow: Popup shown (NetworkMode: %s)"),
		CurrentNetworkMode == ENetworkMode::Steam ? TEXT("Steam") : TEXT("LAN"));
}

void URoomListWindow::ClosePopup()
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

	UE_LOG(LogWjWorld, Log, TEXT("RoomListWindow: Popup closed"));
}

void URoomListWindow::OnRefreshClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("RoomListWindow: Refresh button clicked"));
	StartSearching();
}

void URoomListWindow::OnCloseClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("RoomListWindow: Close button clicked"));
	ClosePopup();
}

void URoomListWindow::OnRoomsFound(bool bWasSuccessful, const TArray<FRoomInfo>& Rooms)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogWjWorld, Log, TEXT("RoomListWindow: Found %d rooms"), Rooms.Num());
		UpdateRoomList(Rooms);
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("RoomListWindow: Failed to find rooms"));
		// TODO: 에러 메시지 표시
	}
}

void URoomListWindow::UpdateRoomList(const TArray<FRoomInfo>& Rooms)
{
	if (!RoomListScrollBox)
	{
		UE_LOG(LogWjWorld, Error, TEXT("RoomListWindow: RoomListScrollBox is null"));
		return;
	}

	if (!RoomListEntryWidgetClass)
	{
		UE_LOG(LogWjWorld, Error, TEXT("RoomListWindow: RoomListEntryWidgetClass is not set"));
		return;
	}

	// 기존 목록 제거
	RoomListScrollBox->ClearChildren();

	// 방이 없으면 메시지 표시
	if (Rooms.Num() == 0)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("RoomListWindow: No rooms found"));
		// TODO: "방이 없습니다" 텍스트 표시
		return;
	}

	// 각 방에 대한 엔트리 위젯 생성
	for (const FRoomInfo& RoomInfo : Rooms)
	{
		URoomListEntryWidget* EntryWidget = CreateWidget<URoomListEntryWidget>(GetWorld(), RoomListEntryWidgetClass);
		if (EntryWidget)
		{
			EntryWidget->SetRoomInfo(RoomInfo);
			RoomListScrollBox->AddChild(EntryWidget);
		}
	}

	UE_LOG(LogWjWorld, Log, TEXT("RoomListWindow: Room list updated with %d entries"), Rooms.Num());
}

void URoomListWindow::OnNetworkModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	// 프로그램적 변경은 무시 (ShowPopupWithNetworkMode에서 설정할 때)
	if (SelectionType == ESelectInfo::Direct)
	{
		return;
	}

	ENetworkMode NewMode = (SelectedItem == TEXT("Steam")) ? ENetworkMode::Steam : ENetworkMode::LAN;

	if (CurrentNetworkMode != NewMode)
	{
		CurrentNetworkMode = NewMode;
		UE_LOG(LogWjWorld, Log, TEXT("RoomListWindow: Network mode changed to %s"), *SelectedItem);

		// 모드 변경 시 자동으로 다시 검색
		StartSearching();
	}
}

void URoomListWindow::InitializeNetworkModeOptions()
{
	if (!NetworkModeComboBox)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("RoomListWindow: NetworkModeComboBox not found (optional)"));
		return;
	}

	NetworkModeComboBox->ClearOptions();

	// LAN 옵션 추가
	NetworkModeComboBox->AddOption(TEXT("LAN"));

	// Steam 옵션 추가 (Steam 빌드에서만 활성화)
#if WITH_STEAM
	NetworkModeComboBox->AddOption(TEXT("Steam"));
	// Steam 빌드에서는 Steam이 기본
	NetworkModeComboBox->SetSelectedOption(TEXT("Steam"));
	CurrentNetworkMode = ENetworkMode::Steam;
#else
	NetworkModeComboBox->SetSelectedOption(TEXT("LAN"));
#endif

	// 콜백 바인딩
	NetworkModeComboBox->OnSelectionChanged.AddDynamic(this, &URoomListWindow::OnNetworkModeSelectionChanged);

	UE_LOG(LogWjWorld, Log, TEXT("RoomListWindow: Network mode options initialized"));
}

void URoomListWindow::StartSearching()
{
	UE_LOG(LogWjWorld, Log, TEXT("RoomListWindow: Starting room search (NetworkMode: %s)..."),
		CurrentNetworkMode == ENetworkMode::Steam ? TEXT("Steam") : TEXT("LAN"));

	// GameInstance를 통해 방 검색
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		bool bSuccess = GameInstance->FindRooms(CurrentNetworkMode);
		if (bSuccess)
		{
			UE_LOG(LogWjWorld, Log, TEXT("RoomListWindow: Room search initiated"));
			// TODO: 로딩 인디케이터 표시
		}
		else
		{
			UE_LOG(LogWjWorld, Error, TEXT("RoomListWindow: Failed to initiate room search"));
		}
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("RoomListWindow: GameInstance is null"));
	}
}

void URoomListWindow::RequestJoinPrivateRoom(int32 RoomIndex)
{
	PendingPasswordRoomIndex = RoomIndex;

	// 비밀번호 입력 위젯 생성
	if (!PasswordInputWidgetInstance && PasswordInputWidgetClass)
	{
		PasswordInputWidgetInstance = CreateWidget<UPasswordInputWidget>(GetWorld(), PasswordInputWidgetClass);
		if (PasswordInputWidgetInstance)
		{
			PasswordInputWidgetInstance->OnPasswordSubmitted.AddDynamic(this, &URoomListWindow::OnPasswordSubmitted);
			PasswordInputWidgetInstance->OnPasswordCancelled.AddDynamic(this, &URoomListWindow::OnPasswordCancelled);
		}
	}

	if (PasswordInputWidgetInstance)
	{
		PasswordInputWidgetInstance->SetMessage(FText::FromString(TEXT("Enter password to join room.")));
		PasswordInputWidgetInstance->ShowPopup();
		UE_LOG(LogWjWorld, Log, TEXT("RoomListWindow: Password input shown for room index %d"), RoomIndex);
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("RoomListWindow: PasswordInputWidgetClass is not set"));
	}
}

void URoomListWindow::OnPasswordSubmitted(const FString& Password)
{
	JoinRoomWithPassword(PendingPasswordRoomIndex, Password);
}

void URoomListWindow::OnPasswordCancelled()
{
	PendingPasswordRoomIndex = -1;
	UE_LOG(LogWjWorld, Log, TEXT("RoomListWindow: Password input cancelled"));
}

void URoomListWindow::JoinRoomWithPassword(int32 RoomIndex, const FString& Password)
{
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (!GameInstance || !GameInstance->GetSessionManager())
	{
		UE_LOG(LogWjWorld, Error, TEXT("RoomListWindow: GameInstance or SessionManager is null"));
		return;
	}

	USessionManager* SessionManager = GameInstance->GetSessionManager();

	// 세션에 저장된 비밀번호 추출
	FString StoredPassword;
	bool bHasPassword = SessionManager->GetSessionPassword(RoomIndex, StoredPassword);

	if (bHasPassword && Password != StoredPassword)
	{
		// 비밀번호 불일치
		UE_LOG(LogWjWorld, Warning, TEXT("RoomListWindow: Wrong password for room index %d"), RoomIndex);

		if (PasswordInputWidgetInstance)
		{
			PasswordInputWidgetInstance->SetErrorMessage(FText::FromString(TEXT("Wrong password.")));
		}
		return;
	}

	// 비밀번호 일치 → 팝업 닫고 입장
	if (PasswordInputWidgetInstance)
	{
		PasswordInputWidgetInstance->ClosePopup();
	}
	PendingPasswordRoomIndex = -1;

	UE_LOG(LogWjWorld, Log, TEXT("RoomListWindow: Password verified, joining room index %d"), RoomIndex);
	GameInstance->JoinRoom(RoomIndex);
}

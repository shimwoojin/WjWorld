// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Session/RoomListWindow.h"
#include "UI/Session/RoomListEntryWidget.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
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
	ShowPopupWithNetworkMode(ENetworkMode::LAN);
}

void URoomListWindow::ShowPopupWithNetworkMode(ENetworkMode InNetworkMode)
{
	CurrentNetworkMode = InNetworkMode;

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
